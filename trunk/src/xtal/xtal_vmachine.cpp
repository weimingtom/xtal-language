#include "xtal.h"
#include "xtal_macro.h"

#include "xtal_stringspace.h"

namespace xtal{

#define XTAL_VM_NODEFAULT } XTAL_ASSERT(false); XTAL_NODEFAULT
#define XTAL_VM_CASE_FIRST(key) case Inst##key::NUMBER: { typedef Inst##key InstType; InstType& inst = *(InstType*)pc;
#define XTAL_VM_CASE(key) } XTAL_ASSERT(false); case Inst##key::NUMBER: /*printf("%s\n", #key);*/ { typedef Inst##key InstType; InstType& inst = *(InstType*)pc;
#define XTAL_VM_SWITCH switch(*pc)
#define XTAL_VM_DEF_INST(key) typedef Inst##key InstType; InstType& inst = *(InstType*)pc
#define XTAL_VM_CONTINUE(x) do{ pc = (x); goto begin; }while(0)
#define XTAL_VM_RETURN return

#define XTAL_VM_EXCEPT(e) do{ except_[0] = e; check_debug_hook(pc==&throw_code_ ? throw_pc_ : pc, BREAKPOINT_THROW); goto except_catch; }while(0)
#define XTAL_VM_CHECK_EXCEPT_PC(pc) (ap(except_[0]) ? push_except(pc) : (pc))
#define XTAL_VM_THROW_EXCEPT(e) XTAL_VM_CONTINUE(push_except(pc, e))
#define XTAL_VM_CHECK_EXCEPT if(ap(except_[0])){ XTAL_VM_CONTINUE(push_except(pc)); }

#ifdef XTAL_NO_THREAD
#	define XTAL_CHECK_YIELD
#else
#	define XTAL_CHECK_YIELD if(--thread_yield_count_<0){ yield_thread(); thread_yield_count_ = 1000; }
#endif

#define XTAL_VM_FUN

void VMachine::set_ff(const inst_t* pc, const inst_t* cpc, int_t need_result_count, 
		int_t ordered_arg_count, int_t named_arg_count, const AnyPtr& self){

	FunFrame& f = *fun_frames_.top();

	f.need_result_count = need_result_count;
	f.ordered_arg_count = ordered_arg_count;
	f.named_arg_count = named_arg_count;
	f.result_count = 0;
	f.called_pc = cpc;
	f.poped_pc = pc;
	f.instance_variables = &empty_instance_variables;
	f.scope_size = scopes_.size();

	f.self(self);
	f.fun(null);
}

void VMachine::push_args(const ArgumentsPtr& args, int_t named_arg_count){
	if(!named_arg_count){
		for(uint_t i = 0; i<args->ordered_size(); ++i){
			push(args->op_at_int(i));
		}
	}
	else{
		int_t usize = args->ordered_size();
		upsize(usize);
		int_t offset = named_arg_count*2;
		for(int_t i = 0; i<offset; ++i){
			set(i+usize, get(i));
		}

		for(int_t i = 0; i<usize; ++i){
			set(offset-1-i, args->op_at_int(i));
		}
	}

	args->add_named(to_smartptr(this));
}

void VMachine::carry_over(Method* fun){
	FunFrame& f = ff();
	
	f.fun(fun);
	f.called_pc = fun->source();
	f.yieldable = f.poped_pc==&end_code_ ? false : prev_ff().yieldable;
	f.instance_variables = f.self()->instance_variables();

	FunInfo* info = fun->info();
	FramePtr& scope = push_scope(info);
	if(fun->extendable_param()){
		if(int_t size = info->variable_size){
			int_t m = size-1;
			for(int_t n=0; n<m; ++n){
				scope->set_member_direct_unref(n, arg(n, fun));
			}				
			scope->set_member_direct_unref(m, inner_make_arguments(fun));
		}
	}
	else{
		if(f.ordered_arg_count!=fun->param_size()){
			adjust_args(fun, fun->param_size());
		}

		int_t size = info->variable_size;
		int_t c = f.args_stack_size()-1;
		int_t m = size;
		for(int_t n=0; n<m; ++n){
			scope->set_member_direct_unref(n, get(c-n));		
		}
	}

	int_t max_stack = info->max_stack;
	stack_.upsize(max_stack);
	stack_.downsize(max_stack + f.args_stack_size());
	f.ordered_arg_count = f.named_arg_count = 0;

	check_debug_hook(f.called_pc, BREAKPOINT_CALL);
}

void VMachine::mv_carry_over(Method* fun){
	FunFrame& f = ff();
	
	f.fun(fun);
	f.called_pc = fun->source();
	f.yieldable = f.poped_pc==&end_code_ ? false : prev_ff().yieldable;
	f.instance_variables = f.self()->instance_variables();

	// 名前付き引数は邪魔
	stack_.downsize(f.named_arg_count*2);

	FunInfo* info = fun->info();

	int_t size = info->variable_size;
	adjust_result(f.ordered_arg_count, size);
	f.ordered_arg_count = size;
	f.named_arg_count = 0;

	FramePtr& scope = push_scope(info);
	int_t c = f.args_stack_size()-1;
	int_t m = size;
	for(int_t n=0; n<m; ++n){
		scope->set_member_direct_unref(n, get(c-n));
	}	
	
	int_t max_stack = info->max_stack;
	stack_.upsize(max_stack);
	stack_.downsize(max_stack + size);
	f.ordered_arg_count = 0;

	check_debug_hook(f.called_pc, BREAKPOINT_CALL);
}

void VMachine::adjust_result(int_t n, int_t need_result_count){

	// 関数が返す戻り値の数と要求している戻り値の数が等しい
	if(need_result_count==n){
		return;
	}

	// 戻り値なんて要求してない
	if(need_result_count==0){
		downsize(n);
		return;
	}

	// 関数が返す戻り値が一つも無いのでundefinedで埋める
	if(n==0){
		for(int_t i = 0; i<need_result_count; ++i){
			push(undefined);
		}
		return;
	}

	// この時点で、nもneed_result_countも1以上


	// 要求している戻り値の数の方が、関数が返す戻り値より少ない
	if(need_result_count<n){

		// 余った戻り値を多値に直す。
		int_t size = n-need_result_count+1;
		ValuesPtr ret;
		if(type(get(0))==TYPE_VALUES){
			ret = unchecked_ptr_cast<Values>(get(0));
		}
		else{
			ret = xnew<Values>(get(0));
		}

		for(int_t i=1; i<size; ++i){
			ret = xnew<Values>(get(i), ret);
		}
		downsize(size);
		push(ret);
	}
	else{
		// 要求している戻り値の数の方が、関数が返す戻り値より多い

		if(type(get(0))==TYPE_VALUES){
			// 最後の要素の多値を展開し埋め込む
			ValuesPtr mv = unchecked_ptr_cast<Values>(get(0));
			downsize(1);

			const ValuesPtr* cur = &mv;
			int_t size = 0;
			while(true){
				if((*cur)->tail()){
					if(need_result_count==n+size){
						push(*cur);
						++size;
						break;
					}

					push((*cur)->head());
					++size;		
					cur = &(*cur)->tail();
				}
				else{
					push((*cur)->head());
					++size;		
					while(need_result_count>=n+size){
						push(undefined);
						++size;
					}
					break;
				}
			}
		}
		else{
			// 最後の要素が多値ではないので、undefinedで埋めとく
			for(int_t i = n; i<need_result_count; ++i){
				push(undefined);
			}
		}
	}
}

const inst_t* VMachine::call(const inst_t* pc, const inst_t* npc, const inst_t* cpc,
	int_t result, int_t ordered, int_t named){

	set_ff(npc, cpc,
		result, ordered, named, prev_ff().self());

	const AnyPtr& target = ff().target();

	switch(type(target)){
		XTAL_DEFAULT{}

		XTAL_CASE(TYPE_BASE){ 
			pvalue(target)->rawcall(to_smartptr(this)); 
		}

		XTAL_CASE(TYPE_NATIVE_METHOD){ 
			unchecked_ptr_cast<NativeMethod>(target)->rawcall(to_smartptr(this)); 
			return XTAL_VM_CHECK_EXCEPT_PC(pop_ff2());
		}

		XTAL_CASE(TYPE_NATIVE_FUN){ 
			unchecked_ptr_cast<NativeFun>(target)->rawcall(to_smartptr(this)); 
			return XTAL_VM_CHECK_EXCEPT_PC(pop_ff2());
		}

		XTAL_CASE(TYPE_IVAR_GETTER){ 
			unchecked_ptr_cast<InstanceVariableGetter>(target)->rawcall(to_smartptr(this)); 
			return XTAL_VM_CHECK_EXCEPT_PC(pop_ff2());
		}

		XTAL_CASE(TYPE_IVAR_SETTER){ 
			unchecked_ptr_cast<InstanceVariableSetter>(target)->rawcall(to_smartptr(this)); 
			return XTAL_VM_CHECK_EXCEPT_PC(pop_ff2());
		}
	}

	if(!processed()){
		set_unsuported_error_info(target, id_[IDOp::id_op_call], undefined);
	}

	return XTAL_VM_CHECK_EXCEPT_PC(ff().called_pc);
}

const inst_t* VMachine::check_accessibility(const inst_t* pc, const AnyPtr& cls, const AnyPtr& self, int_t accessibility){
	if(accessibility & KIND_PRIVATE){
		if(rawne(self->get_class(), cls)){
			return push_except(pc, cpp_class<AccessibilityError>()->call(Xt("Xtal Runtime Error 1017")->call(
				Named(Xid(object), cls->object_name()), 
				Named(Xid(name), ff().primary_key()), 
				Named(Xid(secondary_key), ff().secondary_key()), 
				Named(Xid(accessibility), Xid(private))))
			);
		}
	}
	else if(accessibility & KIND_PROTECTED){
		if(!self->is(cls)){
			return push_except(pc, cpp_class<AccessibilityError>()->call(Xt("Xtal Runtime Error 1017")->call(
				Named(Xid(object), cls->object_name()), 
				Named(Xid(primary_key), ff().primary_key()), 
				Named(Xid(secondary_key), ff().secondary_key()), 
				Named(Xid(accessibility), Xid(protected))))
			);
		}
	}
	return 0;
}

const inst_t* VMachine::send_inner(const ClassPtr& cls, const AnyPtr& ret){
	switch(type(ret)){
		XTAL_DEFAULT{
			ret->rawcall(to_smartptr(this));
			return XTAL_VM_CHECK_EXCEPT_PC(ff().called_pc);
		}

		XTAL_CASE(TYPE_UNDEFINED){

		}

		XTAL_CASE(TYPE_BASE){ 
			pvalue(ret)->rawcall(to_smartptr(this)); 
			return XTAL_VM_CHECK_EXCEPT_PC(ff().called_pc);
		}

		XTAL_CASE(TYPE_NATIVE_METHOD){ 
			unchecked_ptr_cast<NativeMethod>(ret)->rawcall(to_smartptr(this)); 
			return XTAL_VM_CHECK_EXCEPT_PC(pop_ff2());
		}

		XTAL_CASE(TYPE_NATIVE_FUN){ 
			unchecked_ptr_cast<NativeFun>(ret)->rawcall(to_smartptr(this)); 
			return XTAL_VM_CHECK_EXCEPT_PC(pop_ff2());
		}

		XTAL_CASE(TYPE_IVAR_GETTER){ 
			unchecked_ptr_cast<InstanceVariableGetter>(ret)->rawcall(to_smartptr(this)); 
			return XTAL_VM_CHECK_EXCEPT_PC(pop_ff2());
		}

		XTAL_CASE(TYPE_IVAR_SETTER){ 
			unchecked_ptr_cast<InstanceVariableSetter>(ret)->rawcall(to_smartptr(this)); 
			return XTAL_VM_CHECK_EXCEPT_PC(pop_ff2());
		}
	}
		
	{
		const AnyPtr& ret = ap(cls)->member(Xid(send_missing), undefined, true);
		if(rawne(ret, undefined)){
			ArgumentsPtr args = make_arguments();
			recycle_call();
			push_arg(ff().primary_key());
			push_arg(ff().secondary_key());
			push_arg(args);
			ret->rawcall(to_smartptr(this));
			return XTAL_VM_CHECK_EXCEPT_PC(ff().called_pc);
		}
		else{
			return XTAL_VM_CHECK_EXCEPT_PC(ff().called_pc);
		}
	}	
}

const inst_t* VMachine::send(const inst_t* pc, const inst_t* npc, const inst_t* cpc,
	int_t result, int_t ordered, int_t named){
	
	FunFrame& f = ff();
	const ClassPtr& cls = f.target()->get_class();

	int_t accessibility = 0;
	const AnyPtr& ret = cache_member(cls, f.primary_key(), f.secondary_key(), accessibility);

	if(accessibility){
		if(const inst_t* epc = check_accessibility(pc, cls, prev_ff().self(), accessibility)){
			return epc;
		}
	}
	
	set_ff(npc, cpc, result, ordered, named, prev_ff().self());
	set_arg_this(f.target());
	return send_inner(cls, ret);
}

const inst_t* VMachine::vproperty(const inst_t* pc, const inst_t* npc, const inst_t* cpc, int_t primary_key){
	const ClassPtr& cls = get()->get_class();
	int_t accessibility = 0;
	const IDPtr& id = identifier(primary_key);
	const AnyPtr& ret = cache_member(cls, id, accessibility);

	if(accessibility){
		if(const inst_t* epc = check_accessibility(pc, cls, ff().self(), accessibility)){
			return epc;
		}
	}

	if(type(ret)==TYPE_IVAR_GETTER){
		set(get().instance_variables()->variable(
			unchecked_ptr_cast<InstanceVariableGetter>(ret)->number_, 
			unchecked_ptr_cast<InstanceVariableGetter>(ret)->info_));
		return XTAL_VM_CHECK_EXCEPT_PC(npc);
	}

	FunFrame& f = push_ff();
	f.secondary_key(undefined);
	f.primary_key(id);
	f.target(pop());

	set_ff(npc, cpc, 1, 0, 0, prev_ff().self());
	set_arg_this(f.target());
	return send_inner(cls, ret);
}

const inst_t* VMachine::set_vproperty(const inst_t* pc, const inst_t* npc, const inst_t* cpc, int_t primary_key){
	const ClassPtr& cls = get()->get_class();
	int_t accessibility = 0;
	const IDPtr& id = identifier(primary_key);
	const AnyPtr& ret = cache_member(cls, id, accessibility);

	if(accessibility){
		if(const inst_t* epc = check_accessibility(pc, cls, ff().self(), accessibility)){
			return epc;
		}
	}

	if(type(ret)==TYPE_IVAR_SETTER){
		get().instance_variables()->set_variable(
			unchecked_ptr_cast<InstanceVariableSetter>(ret)->number_, 
			unchecked_ptr_cast<InstanceVariableSetter>(ret)->info_,
			get(1));
		downsize(2);
		return XTAL_VM_CHECK_EXCEPT_PC(npc);
	}

	FunFrame& f = push_ff();
	f.secondary_key(undefined);
	f.primary_key(id);
	f.target(pop());

	set_ff(npc, cpc, 0, 1, 0, prev_ff().self());
	set_arg_this(f.target());
	return send_inner(cls, ret);
}

const inst_t* VMachine::inner_send_from_stack(const inst_t* npc, int_t need_result_count, const IDPtr& primary_key, int_t ntarget){
	FunFrame& f = push_ff();
	f.secondary_key(undefined);
	f.primary_key(primary_key);
	f.target(get(ntarget));
	downsize(1);

	return XTAL_VM_CHECK_EXCEPT_PC(
		send(0, npc, &throw_unsupported_error_code_,
			need_result_count, 0, 0));
}

const inst_t* VMachine::inner_send_from_stack(const inst_t* npc, int_t need_result_count, const IDPtr& primary_key, int_t ntarget, int_t na0){
	FunFrame& f = push_ff();
	f.secondary_key(undefined);
	f.primary_key(primary_key);
	f.target(get(ntarget));
	Any a0 = get(na0);
	downsize(2);
	push(a0);

	return XTAL_VM_CHECK_EXCEPT_PC(
		send(0, npc, &throw_unsupported_error_code_,
			need_result_count, 1, 0));
}

const inst_t* VMachine::inner_send_from_stack_q(const inst_t* npc, int_t need_result_count, const IDPtr& primary_key, int_t ntarget, int_t na0){
	FunFrame& f = push_ff();
	f.secondary_key(undefined);
	f.primary_key(primary_key);
	f.target(get(ntarget));
	Any a0 = get(na0);
	downsize(2);
	push(a0);

	return XTAL_VM_CHECK_EXCEPT_PC(
		send(0, npc, &check_unsupported_code_,
			need_result_count, 1, 0));
	return XTAL_VM_CHECK_EXCEPT_PC(ff().called_pc);
}

const inst_t* VMachine::inner_send_from_stack(const inst_t* npc, int_t need_result_count, const IDPtr& primary_key, int_t ntarget, int_t na0, int_t na1){
	FunFrame& f = push_ff();
	f.secondary_key(undefined);
	f.primary_key(primary_key);
	f.target(get(ntarget));
	Any a0 = get(na0);
	Any a1 = get(na1);
	downsize(3);
	push(a0); push(a1);

	return XTAL_VM_CHECK_EXCEPT_PC(
		send(0, npc, &throw_unsupported_error_code_,
			need_result_count, 2, 0));
}

const inst_t* VMachine::inner_send_from_stack_q(const inst_t* npc, int_t need_result_count, const IDPtr& primary_key, int_t ntarget, int_t na0, int_t na1){
	FunFrame& f = push_ff();
	f.secondary_key(undefined);
	f.primary_key(primary_key);
	f.target(get(ntarget));
	Any a0 = get(na0);
	Any a1 = get(na1);
	downsize(3);
	push(a0); push(a1);

	return XTAL_VM_CHECK_EXCEPT_PC(
		send(0, npc, &check_unsupported_code_,
			need_result_count, 2, 0));
}

const FramePtr& VMachine::make_outer_outer(uint_t i){
	if(i<scopes_.size()-ff().scope_size){
		FramePtr& scope = scopes_[i];
		if(!scope->code()){
			scope->set_code(code());
			if(scope->info()->flags&ScopeInfo::FLAG_SCOPE_CHAIN){
				scope->set_outer(make_outer_outer(i+1));
			}
		}
		return scope;
	}
	return outer();
}

const FramePtr& VMachine::make_outer(ScopeInfo* scope){
	if(scope->flags&ScopeInfo::FLAG_SCOPE_CHAIN){
		return make_outer_outer();
	}
	return unchecked_ptr_cast<Frame>(null);
}

void VMachine::set_local_variable_out_of_fun(uint_t pos, uint_t depth, const Any& value){
	uint_t size = scopes_.size()-ff().scope_size;
	if(depth<size){
		scopes_[depth]->set_member_direct_unref(pos, ap(value));
		return;
	}

	depth -= size;

	Frame* out = outer().get();
	while(depth){
		out = out->outer().get();
		depth--;
	}

	out->set_member_direct(pos, ap(value));
}

AnyPtr& VMachine::local_variable_out_of_fun(uint_t pos, uint_t depth){
	uint_t size = scopes_.size()-ff().scope_size;
	if(depth<size){
		return scopes_[depth]->member_direct(pos);
	}

	depth -= size;

	Frame* out = outer().get();
	while(depth){
		out = out->outer().get();
		depth--;
	}

	return out->member_direct(pos);
}

void VMachine::execute_inner(const inst_t* start){

	// レジスタ割付されたらいいな
	register const inst_t* pc = start;

	ExceptFrame cur;
	cur.info = 0;
	cur.stack_size = stack_.size() - ff().args_stack_size();
	cur.fun_frame_size = fun_frames_.size();
	cur.scope_size = scopes_.size();

	// 例外がクリアされていないなら実行できない
	if(ap(except_[0])){
		stack_.downsize_n(cur.stack_size);
		pop_ff2();
		return;
	}

	hook_setting_bit_ = debug::hook_setting_bit();

	thread_yield_count_ = 1000;

	XTAL_ASSERT(cur.stack_size>=0);

{

begin:

XTAL_VM_SWITCH{

//{OPS{{
	XTAL_VM_CASE_FIRST(Nop){ // 2
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(PushNull){ // 3
		push(null); 
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(PushUndefined){ // 3
		push(undefined); 
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(PushTrue){ // 3
		push(Any(true)); 
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(PushFalse){ // 3
		push(Any(false)); 
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(PushTrueAndSkip){ // 3
		push(Any(true)); 
		XTAL_VM_CONTINUE(pc + inst.ISIZE + InstPushFalse::ISIZE); 
	}

	XTAL_VM_CASE(PushInt1Byte){ // 3
		push(Any((int_t)inst.value)); 
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(PushFloat1Byte){ // 3
		push(Any((float_t)inst.value)); 
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(PushCallee){ // 3
		push(fun()); 
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(PushThis){ // 3
		push(ff().self()); 
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(PushCurrentContext){ // 3
		push(make_outer_outer());
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}

	XTAL_VM_CASE(Pop){ // 3
		downsize(1); 
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(Dup){ // 3
		dup(); 
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(Insert1){ // 5
		Any temp = get(); 
		set(get(1)); 
		set(1, temp); 
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(Insert2){ // 6
		Any temp = get(); 
		set(get(1)); 
		set(1, get(2)); 
		set(2, temp); 
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(AdjustResult){ // 3
		adjust_result(inst.result_count, inst.need_result_count);
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(LocalVariableInc1Byte){ // 2
		XTAL_VM_CONTINUE(OpAddConstantInt(pc + inst.ISIZE + InstSetLocalVariable1Byte::ISIZE, pc + inst.ISIZE, 
			IDOp::id_op_inc, local_variable(inst.number, inst.depth), 1));
	}

	XTAL_VM_CASE(LocalVariableDec1Byte){ // 2
		XTAL_VM_CONTINUE(OpAddConstantInt(pc + inst.ISIZE + InstSetLocalVariable1Byte::ISIZE, pc + inst.ISIZE, 
			IDOp::id_op_dec, local_variable(inst.number, inst.depth), -1));
	}

	XTAL_VM_CASE(LocalVariable1Byte){ // 3
		push(local_variable(inst.number, inst.depth)); 
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(LocalVariable1ByteX2){ // 4
		push(local_variable(inst.number1, inst.depth1)); 
		push(local_variable(inst.number2, inst.depth2)); 
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(LocalVariable2Byte){ // 3
		push(local_variable_out_of_fun(inst.number, inst.depth)); 
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(SetLocalVariable1Byte){ // 4
		set_local_variable(inst.number, inst.depth, get());
		downsize(1);
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(SetLocalVariable1ByteX2){ // 5
		set_local_variable(inst.number1, inst.depth1, get());
		set_local_variable(inst.number2, inst.depth2, get(1));
		downsize(2);
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(SetLocalVariable2Byte){ // 4
		set_local_variable_out_of_fun(inst.number, inst.depth, get()); 
		downsize(1);
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(InstanceVariable){ // 3
		push(ff().instance_variables->variable(inst.number, code()->class_info(inst.info_number)));
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(SetInstanceVariable){ // 4
		ff().instance_variables->set_variable(inst.number, code()->class_info(inst.info_number), get());
		downsize(1);
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(FilelocalVariable){ // 5
		const AnyPtr& ret = code()->filelocal()->member(identifier(inst.identifier_number));
		if(rawne(ret, undefined)){
			push(ret);
		}
		else{
			XTAL_VM_THROW_EXCEPT(filelocal_unsupported_error(code(), identifier(inst.identifier_number)));
		}
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}

	XTAL_VM_CASE(CleanupCall){ // 2
		XTAL_VM_CONTINUE(pop_ff2());
	}

	XTAL_VM_CASE(Return){ // 5
		check_debug_hook(pc, BREAKPOINT_RETURN);

		FunFrame& f = ff();

		if(f.need_result_count!=f.result_count+inst.results){
			adjust_result(f.result_count+inst.results); 
		}

		XTAL_VM_CONTINUE(pop_ff());
	}

	XTAL_VM_CASE(Yield){ // 6
		yield_result_count_ = inst.results;	
		if(ff().yieldable){
			resume_pc_ = pc + inst.ISIZE;
			XTAL_VM_RETURN;
		}
		else{
			downsize(yield_result_count_);
			XTAL_VM_THROW_EXCEPT(cpp_class<YieldError>()->call(Xt("Xtal Runtime Error 1012")));
		}
	}

	XTAL_VM_CASE(Exit){ // 3
		resume_pc_ = 0; 
		XTAL_VM_RETURN;
	}

	XTAL_VM_CASE(Value){ // 3
		push(code()->value(inst.value_number)); 
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(CheckUnsupported){ // 6
		FunFrame& f = ff();

		downsize(f.args_stack_size());
		push(undefined);
		if(f.need_result_count!=1){
			adjust_result(1, f.need_result_count);
		}

		XTAL_VM_CONTINUE(pop_ff2());
	}

	XTAL_VM_CASE(Property){ // 2
		XTAL_VM_CONTINUE(vproperty(pc, pc+inst.ISIZE, &throw_unsupported_error_code_, inst.identifier_number));
	}

	XTAL_VM_CASE(SetProperty){ // 2
		XTAL_VM_CONTINUE(set_vproperty(pc, pc+inst.ISIZE, &throw_unsupported_error_code_, inst.identifier_number));
	}

	XTAL_VM_CASE(Call){ // 8
		FunFrame& f = push_ff();
		f.secondary_key(undefined);
		f.primary_key(id_[IDOp::id_op_call]);
		f.target(pop());

		if((inst.flags&CALL_FLAG_ARGS)!=0){
			if(ArgumentsPtr args = ptr_cast<Arguments>(pop())){
				push_args(args, inst.named);
				XTAL_VM_CONTINUE(call(pc, pc + inst.ISIZE, &throw_unsupported_error_code_, 
					inst.need_result, args->ordered_size()+inst.ordered, args->named_size()+inst.named));
			}
		}

		XTAL_VM_CONTINUE(call(pc, pc + inst.ISIZE, &throw_unsupported_error_code_, 
			inst.need_result, inst.ordered, inst.named));
	}

	XTAL_VM_CASE(Send){ // 11
		FunFrame& f = push_ff();

		if((inst.flags&CALL_FLAG_NS)==0){ f.secondary_key(undefined); }
		else{ f.secondary_key(pop()); }
		if(int_t n = inst.identifier_number){ f.primary_key(prev_identifier(n)); }
		else{ f.primary_key(pop()->to_s()->intern()); }
		f.target(pop());

		const inst_t* cpc = (inst.flags&CALL_FLAG_Q)==0 ? 
			&throw_unsupported_error_code_ : &check_unsupported_code_;

		if((inst.flags&CALL_FLAG_ARGS)!=0){
			if(ArgumentsPtr args = ptr_cast<Arguments>(pop())){
				push_args(args, inst.named);

				XTAL_VM_CONTINUE(send(pc, pc+inst.ISIZE, cpc,
					inst.need_result, args->ordered_size()+inst.ordered, args->named_size()+inst.named));		 	
			}
		}

		XTAL_VM_CONTINUE(send(pc, pc+inst.ISIZE, cpc,
			inst.need_result, inst.ordered, inst.named));		 	
	}

	XTAL_VM_CASE(Member){ // 14
		FunFrame& f = ff();

		if((inst.flags&CALL_FLAG_NS)==0){ f.secondary_key(undefined); }
		else{ f.secondary_key(pop()); }
		if(int_t n = inst.identifier_number){ f.primary_key(identifier(n)); }
		else{ f.primary_key(pop()->to_s()->intern()); }
		f.target(pop());

		int_t accessibility = 0;
		const AnyPtr& ret = cache_member(f.target(), f.primary_key(), f.secondary_key(), accessibility);

		if(accessibility){
			if(const inst_t* epc = check_accessibility(pc, f.target(), f.self(), accessibility)){
				XTAL_VM_CONTINUE(epc);
			}	
		}

		if(inst.flags&CALL_FLAG_Q){
			push(ret);
		}
		else{
			if(rawne(ret, undefined)){
				push(ret);
			}
			else{
				XTAL_VM_THROW_EXCEPT(unsupported_error(f.target(), f.primary_key(), f.secondary_key()));
			}
		}
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(DefineMember){ // 11
		XTAL_VM_FUN;
		FunFrame& f = ff();
		f.secondary_key((inst.flags&CALL_FLAG_NS) ? pop() : undefined);
		if(int_t n = inst.identifier_number){ f.primary_key(identifier(n)); }
		else{ f.primary_key(pop()->to_s()->intern()); }
		AnyPtr value = pop();
		f.target(pop());
		f.target()->def(f.primary_key(), ap(value), f.secondary_key(), KIND_PUBLIC);
		XTAL_VM_CHECK_EXCEPT;
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(ScopeBegin){ // 3
		push_scope(code()->scope_info(inst.info_number));
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}

	XTAL_VM_CASE(ScopeEnd){ // 3
		pop_scope();
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}

	XTAL_VM_CASE(TryBegin){ // 8
		FunFrame& f = ff(); 
		ExceptFrame& ef = except_frames_.push();
		ef.info = code()->except_info(inst.info_number);
		ef.fun_frame_size = fun_frames_.size();
		ef.stack_size = stack_.size();
		ef.scope_size = scopes_.size();
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}

	XTAL_VM_CASE(TryEnd){ // 2
		XTAL_VM_CONTINUE(except_frames_.pop().info->finally_pc + code()->data()); 
	}

	XTAL_VM_CASE(PushGoto){ // 3
		push(Any((int_t)((pc+inst.address)-code()->data()))); 
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(PopGoto){ // 2
		XTAL_VM_CONTINUE(code()->data()+ivalue(pop()));
	}

	XTAL_VM_CASE(Goto){ // 3
		XTAL_CHECK_YIELD;
		XTAL_VM_CONTINUE(pc + inst.address); 
	}

	XTAL_VM_CASE(If){ // 2
		XTAL_VM_CONTINUE(pc + (pop() ? inst.address_true : inst.address_false));
	}

	XTAL_VM_CASE(IfEq){ // 15
		const AnyPtr& b = get(); uint_t btype = type(b)-TYPE_INT;
		const AnyPtr& a = get(1); uint_t atype = type(a)-TYPE_INT;
		uint_t abtype = atype | btype;
		
		typedef InstIf InstType2; 
		InstType2& inst2 = *(InstType2*)(pc+inst.ISIZE);

		if(abtype==0){
			downsize(2); 
			XTAL_VM_CONTINUE((
				ivalue(a) == ivalue(b) ? 
				inst2.address_true : 
				inst2.address_false) + pc + inst.ISIZE);
		}

		if(abtype==1){
			f2 ab;
			to_f2(ab, atype, a, btype, b);
			downsize(2); 
			XTAL_VM_CONTINUE((
				ab.a == ab.b ? 
				inst2.address_true : 
				inst2.address_false) + pc + inst.ISIZE);
		}

		XTAL_VM_CONTINUE(inner_send_from_stack_q(pc+inst.ISIZE, 1, id_[IDOp::id_op_eq], 1, 0));
	}

	XTAL_VM_CASE(IfLt){ // 15
		const AnyPtr& b = get(); uint_t btype = type(b)-TYPE_INT;
		const AnyPtr& a = get(1); uint_t atype = type(a)-TYPE_INT;
		uint_t abtype = atype | btype;

		typedef InstIf InstType2; 
		InstType2& inst2 = *(InstType2*)(pc+inst.ISIZE);

		if(abtype==0){
			downsize(2); 
			XTAL_VM_CONTINUE((
				ivalue(a) < ivalue(b) ? 
				inst2.address_true : 
				inst2.address_false) + pc + inst.ISIZE);
		}

		if(abtype==1){
			f2 ab;
			to_f2(ab, atype, a, btype, b);
			downsize(2); 
			XTAL_VM_CONTINUE(
				(ab.a < ab.b ? 
				inst2.address_true : 
				inst2.address_false) + pc + inst.ISIZE);
		}
		
		XTAL_VM_CONTINUE(inner_send_from_stack(pc+inst.ISIZE, 1, id_[IDOp::id_op_lt], 1, 0));
	}

	XTAL_VM_CASE(IfRawEq){ // 6
		typedef InstIf InstType2; 
		InstType2& inst2 = *(InstType2*)(pc+inst.ISIZE);

		pc = (raweq(get(1), get()) ? 
			(int)(inst2.address_true) : 
			(int)(inst2.address_false)) + pc + inst.ISIZE;

		downsize(2);
		XTAL_VM_CONTINUE(pc);
	}

	XTAL_VM_CASE(IfIn){ // 2
		XTAL_VM_CONTINUE(inner_send_from_stack(pc+inst.ISIZE, 1, id_[IDOp::id_op_in], 1, 0));
	}

	XTAL_VM_CASE(IfIs){ // 6
		typedef InstIf InstType2; 
		InstType2& inst2 = *(InstType2*)(pc+inst.ISIZE);

		pc = (get(1)->is(get()) ? 
			(int)(inst2.address_true) : 
			(int)(inst2.address_false)) + pc + inst.ISIZE;
		downsize(2);
		XTAL_VM_CONTINUE(pc);
	}

	XTAL_VM_CASE(IfArgIsUndefined){ // 3
		if(raweq(local_variable(inst.arg, 0), undefined)){
			XTAL_VM_CONTINUE(pc + inst.ISIZE);
		}
		else{
			XTAL_VM_CONTINUE(pc + inst.address); 
		}
	}

	XTAL_VM_CASE(Pos){ // 5
		const AnyPtr& a = get(); uint_t atype = type(a)-TYPE_INT;
		if(atype<2){
			XTAL_VM_CONTINUE(pc+inst.ISIZE);
		}
		else{
			XTAL_VM_CONTINUE(inner_send_from_stack(pc+inst.ISIZE, 1, id_[IDOp::id_op_pos], 0));
		}
	}

	XTAL_VM_CASE(Neg){ // 8
		const AnyPtr& a = get(); uint_t atype = type(a)-TYPE_INT;
		if(atype==0){
			set(Any(-ivalue(a)));
			XTAL_VM_CONTINUE(pc+inst.ISIZE);
		}

		if(atype==1){
			set(Any(-fvalue(a)));
			XTAL_VM_CONTINUE(pc+inst.ISIZE);
		}
	
		XTAL_VM_CONTINUE(inner_send_from_stack(pc+inst.ISIZE, 1, id_[IDOp::id_op_neg], 0));
	}

	XTAL_VM_CASE(Com){ // 6
		const AnyPtr& a = get(); uint_t atype = type(a)-TYPE_INT;
		if(atype==0){
			set(Any(~ivalue(a)));
			XTAL_VM_CONTINUE(pc+inst.ISIZE);
		}
		else{
			XTAL_VM_CONTINUE(inner_send_from_stack(pc+inst.ISIZE, 1, id_[IDOp::id_op_com], 0));
		}
	}

	XTAL_VM_CASE(Not){ // 3
		set(Any(!get())); 
		XTAL_VM_CONTINUE(pc+inst.ISIZE); 
	}

	XTAL_VM_CASE(At){ // 16
		AnyPtr& b = get(); uint_t btype = type(b);
		AnyPtr& a = get(1); uint_t atype = type(a);

		if(atype==TYPE_ARRAY && btype==TYPE_INT){
			Any ret = unchecked_ptr_cast<Array>(a)->op_at(ivalue(b));
			XTAL_VM_CHECK_EXCEPT;
			downsize(1);
			set(ret);
			XTAL_VM_CONTINUE(pc+inst.ISIZE); 
		}
		else if(raweq(a->get_class(), cpp_class<Map>())){
			Any ret = unchecked_ptr_cast<Map>(a)->op_at(b);
			XTAL_VM_CHECK_EXCEPT;
			downsize(1);
			set(ret);
			XTAL_VM_CONTINUE(pc+inst.ISIZE); 
		}
		else{
			XTAL_VM_CONTINUE(inner_send_from_stack(pc+inst.ISIZE, 1, id_[IDOp::id_op_at], 1, 0));
		}
	}

	XTAL_VM_CASE(SetAt){ // 14
		AnyPtr& b = get(0); uint_t btype = type(b);
		AnyPtr& a = get(1); uint_t atype = type(a);

		if(atype==TYPE_ARRAY && btype==TYPE_INT){
			unchecked_ptr_cast<Array>(a)->op_set_at(ivalue(b), get(2));
			XTAL_VM_CHECK_EXCEPT;
			downsize(3);
			XTAL_VM_CONTINUE(pc+inst.ISIZE); 
		}
		else if(raweq(a->get_class(), cpp_class<Map>())){
			unchecked_ptr_cast<Map>(a)->op_set_at(b, get(2));
			XTAL_VM_CHECK_EXCEPT;
			downsize(3);
			XTAL_VM_CONTINUE(pc+inst.ISIZE); 
		}
		else{
			XTAL_VM_CONTINUE(inner_send_from_stack(pc+inst.ISIZE, 0, id_[IDOp::id_op_set_at], 1, 0, 2));
		}
	}

	XTAL_VM_CASE(Add){ // 2
		XTAL_VM_CONTINUE(OpAdd(pc+inst.ISIZE, IDOp::id_op_add));
	}

	XTAL_VM_CASE(Sub){ // 2
		XTAL_VM_CONTINUE(OpSub(pc+inst.ISIZE, IDOp::id_op_sub));
	}

	XTAL_VM_CASE(Cat){ // 2
		XTAL_VM_CONTINUE(inner_send_from_stack(pc+inst.ISIZE, 1, id_[IDOp::id_op_cat], 1, 0));
	}

	XTAL_VM_CASE(Mul){ // 2
		XTAL_VM_CONTINUE(OpMul(pc+inst.ISIZE, IDOp::id_op_mul));
	}

	XTAL_VM_CASE(Div){ // 2
		XTAL_VM_CONTINUE(OpDiv(pc+inst.ISIZE, IDOp::id_op_div));
	}

	XTAL_VM_CASE(Mod){ // 2
		XTAL_VM_CONTINUE(OpMod(pc+inst.ISIZE, IDOp::id_op_mod));
	}

	XTAL_VM_CASE(And){ // 2
		XTAL_VM_CONTINUE(OpAnd(pc+inst.ISIZE, IDOp::id_op_and));
	}

	XTAL_VM_CASE(Or){ // 2
		XTAL_VM_CONTINUE(OpOr(pc+inst.ISIZE, IDOp::id_op_or));
	}

	XTAL_VM_CASE(Xor){ // 2
		XTAL_VM_CONTINUE(OpXor(pc+inst.ISIZE, IDOp::id_op_xor));
	}

	XTAL_VM_CASE(Shl){ // 2
		XTAL_VM_CONTINUE(OpShl(pc+inst.ISIZE, IDOp::id_op_shl));
	}

	XTAL_VM_CASE(Shr){ // 2
		XTAL_VM_CONTINUE(OpShr(pc+inst.ISIZE, IDOp::id_op_shr));
	}

	XTAL_VM_CASE(Ushr){ // 2
		XTAL_VM_CONTINUE(OpUshr(pc+inst.ISIZE, IDOp::id_op_ushr));
	}

	XTAL_VM_CASE(Inc){ // 2
		XTAL_VM_CONTINUE(OpAddConstantInt(pc+inst.ISIZE, IDOp::id_op_inc, 1));
	}

	XTAL_VM_CASE(Dec){ // 2
		XTAL_VM_CONTINUE(OpAddConstantInt(pc+inst.ISIZE, IDOp::id_op_dec, -1));
	}

	XTAL_VM_CASE(AddAssign){ // 2
		XTAL_VM_CONTINUE(OpAdd(pc+inst.ISIZE, IDOp::id_op_add_assign));
	}

	XTAL_VM_CASE(SubAssign){ // 2
		XTAL_VM_CONTINUE(OpSub(pc+inst.ISIZE, IDOp::id_op_sub_assign));
	}

	XTAL_VM_CASE(CatAssign){ // 2
		XTAL_VM_CONTINUE(inner_send_from_stack(pc+inst.ISIZE, 1, id_[IDOp::id_op_cat_assign], 1, 0));
	}

	XTAL_VM_CASE(MulAssign){ // 2
		XTAL_VM_CONTINUE(OpMul(pc+inst.ISIZE, IDOp::id_op_mul_assign));
	}

	XTAL_VM_CASE(DivAssign){ // 2
		XTAL_VM_CONTINUE(OpDiv(pc+inst.ISIZE, IDOp::id_op_div_assign));
	}

	XTAL_VM_CASE(ModAssign){ // 2
		XTAL_VM_CONTINUE(OpMod(pc+inst.ISIZE, IDOp::id_op_mod_assign));
	}

	XTAL_VM_CASE(AndAssign){ // 2
		XTAL_VM_CONTINUE(OpAnd(pc+inst.ISIZE, IDOp::id_op_and_assign));
	}

	XTAL_VM_CASE(OrAssign){ // 2
		XTAL_VM_CONTINUE(OpOr(pc+inst.ISIZE, IDOp::id_op_or_assign));
	}

	XTAL_VM_CASE(XorAssign){ // 2
		XTAL_VM_CONTINUE(OpXor(pc+inst.ISIZE, IDOp::id_op_xor_assign));
	}

	XTAL_VM_CASE(ShlAssign){ // 2
		XTAL_VM_CONTINUE(OpShl(pc+inst.ISIZE, IDOp::id_op_shl_assign));
	}

	XTAL_VM_CASE(ShrAssign){ // 2
		XTAL_VM_CONTINUE(OpShr(pc+inst.ISIZE, IDOp::id_op_shr_assign));
	}

	XTAL_VM_CASE(UshrAssign){ // 2
		XTAL_VM_CONTINUE(OpUshr(pc+inst.ISIZE, IDOp::id_op_ushr_assign));
	}

	XTAL_VM_CASE(Range){ // 10
		Any rhs = pop();
		Any lhs = pop();
		push(rhs); 
		push(Any((int_t)inst.kind));

		FunFrame& f = push_ff();
		f.secondary_key(undefined);
		f.primary_key(id_[IDOp::id_op_range]);
		f.target(lhs);

		XTAL_VM_CONTINUE(send(pc, pc+inst.ISIZE, &throw_unsupported_error_code_,
			1, 2, 0));
	}

	XTAL_VM_CASE(Once){ // 5
		const AnyPtr& ret = code()->once_value(inst.value_number);
		if(rawne(ret, undefined)){
			push(ret);
			XTAL_VM_CONTINUE(pc + inst.address);
		}
		else{
			XTAL_VM_CONTINUE(pc + inst.ISIZE);
		}
	}

	XTAL_VM_CASE(SetOnce){ // 3
		code()->set_once_value(inst.value_number, pop()); 
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(ClassBegin){ // 21
		XTAL_VM_FUN;
		ClassInfo* info = code()->class_info(inst.info_number);
		const FramePtr& outer = make_outer(info);
		ClassPtr cp = xnew<Class>(outer, code(), info);

		switch(info->kind){
			XTAL_NODEFAULT;

			XTAL_CASE(KIND_CLASS){

			}

			XTAL_CASE(KIND_SINGLETON){
				cp->set_singleton();
			}
		}
		
		int_t n = info->mixins;
		for(int_t i = 0; i<n; ++i){
			AnyPtr popped = pop();
			if(const ClassPtr& cls = ptr_cast<Class>(popped)){
				cp->inherit_first(cls);
			}
			else{
				XTAL_VM_THROW_EXCEPT(cpp_class<RuntimeError>()->call());
			}
		}

		push_ff();
		set_ff(pc + inst.ISIZE, &throw_unsupported_error_code_, 0, 0, 0, cp);
		scopes_.push(cp);
		cp->orphan_ = false;

		ff().fun(prev_fun());
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}

	XTAL_VM_CASE(ClassEnd){ // 8
		XTAL_VM_FUN;
		FramePtr& scope = scopes_.top();
		if(raweq(scope->get_class(), scope)){
			Class* singleton = (Class*)pvalue(scope);
			singleton->init_singleton(to_smartptr(this));
		}

		push(scope);
		scope->add_ref_count_members(1);
		scope->orphan_ = true;
		scope = null;
		scopes_.downsize(1);

		pop_ff2();
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}

	XTAL_VM_CASE(DefineClassMember){ // 6
		XTAL_VM_FUN;
		if(const ClassPtr& p = ptr_cast<Class>(scopes_.top())){
			p->set_member_direct(inst.number, identifier(inst.identifier_number), get(1), get(), inst.accessibility);
		}
		downsize(2);
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}

	XTAL_VM_CASE(MakeArray){ // 3
		push(xnew<Array>());
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}

	XTAL_VM_CASE(ArrayAppend){ // 4
		cast<const ArrayPtr&>(get(1))->push_back(get()); 
		downsize(1);
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}

	XTAL_VM_CASE(MakeMap){ // 3
		push(xnew<Map>());
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}

	XTAL_VM_CASE(MapInsert){ // 4
		cast<const MapPtr&>(get(2))->set_at(get(1), get()); 
		downsize(2);	
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}

	XTAL_VM_CASE(MapSetDefault){ // 4
		cast<const MapPtr&>(get(1))->set_default_value(get()); 
		downsize(1);	
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}

	XTAL_VM_CASE(MakeFun){ // 11
		XTAL_VM_FUN;
		int_t table_n = inst.info_number, end = inst.address;
		FunInfo* info = code()->fun_info(table_n);
		const FramePtr& outer = make_outer(info);
		switch(info->kind){
			XTAL_NODEFAULT;
			XTAL_CASE(KIND_FUN){ push(xnew<Fun>(outer, ff().self(), code(), info)); }
			XTAL_CASE(KIND_LAMBDA){ push(xnew<Lambda>(outer, ff().self(), code(), info)); }
			XTAL_CASE(KIND_METHOD){ push(xnew<Method>(outer, code(), info)); }
			XTAL_CASE(KIND_FIBER){ push(xnew<Fiber>(outer, ff().self(), code(), info)); }
		}
		XTAL_VM_CONTINUE(pc + end);
	}

	XTAL_VM_CASE(MakeInstanceVariableAccessor){ // 8
		XTAL_VM_FUN;
		AnyPtr ret;
		switch(inst.type){
			XTAL_NODEFAULT;
			XTAL_CASE(0){ ret = xnew<InstanceVariableGetter>(inst.number, code()->class_info(inst.info_number)); }
			XTAL_CASE(1){ ret = xnew<InstanceVariableSetter>(inst.number, code()->class_info(inst.info_number)); }
		}
		push(ret);
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(Throw){ // 6
		AnyPtr except = pop();
		if(!except){
			except = ap(except_[0]);
		}

		if(pc!=&throw_code_){
			throw_pc_ = pc;
		}

		if(!except->is(cpp_class<Exception>())){
			XTAL_VM_EXCEPT(cpp_class<RuntimeError>()->call(except));
		}
		else{
			XTAL_VM_EXCEPT(except);
		}
	}

	XTAL_VM_CASE(ThrowUnsupportedError){ // 6
		XTAL_VM_FUN;
		FunFrame& f = ff();
		AnyPtr target_class = f.target() ? f.target()->get_class() : unchecked_ptr_cast<Class>(null);
		pop_ff2();
		XTAL_VM_THROW_EXCEPT(unsupported_error(target_class, f.primary_key(), f.secondary_key()));
	}

	XTAL_VM_CASE(IfDebug){ // 2
		XTAL_VM_CONTINUE(pc + (debug::is_enabled() ? inst.ISIZE : inst.address));
	}

	XTAL_VM_CASE(Assert){ // 5
		XTAL_VM_FUN;
		debug_hook(pc, BREAKPOINT_ASSERT);
		if(ap(except_[0])){
			XTAL_VM_THROW_EXCEPT(ap(except_[0]));
		}
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}

	XTAL_VM_CASE(BreakPoint){ // 3
		check_debug_hook(pc, BREAKPOINT);
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}

	XTAL_VM_CASE(MAX){ // 2
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}

//}}OPS}

	XTAL_VM_NODEFAULT;
}

// 例外が投げられたらここに到達する
except_catch:
	// 例外にバックトレースを追加する
	AnyPtr e = catch_except();
	set_except(append_backtrace(throw_pc_, ap(e)));

	// Xtalソース内でキャッチ等あるか調べる
	pc = catch_body(throw_pc_, cur);

	// Xtalソース内でキャッチ等されたなら最初に戻る
	if(pc){
		goto begin;
	}

	return;
}

}

const inst_t* VMachine::OpAddConstantInt(const inst_t* pc1, const inst_t* pc2, int_t op, Any& a, int_t constant){
	uint_t atype = type(a)-TYPE_INT;
	if(atype==0){
		set_ivalue(a, ivalue(a)+constant);
		return pc1;
	}
	
	if(atype==1){
		set_fvalue(a, fvalue(a)+constant);
		return pc1;
	}

	push(a);
	return inner_send_from_stack(pc2, 1, id_[op], 0);
}

const inst_t* VMachine::OpAddConstantInt(const inst_t* pc, int_t op, int_t constant){
	Any& a = get(); uint_t atype = type(a)-TYPE_INT;
	if(atype==0){
		set_ivalue(a, ivalue(a)+constant);
		return pc;
	}

	if(atype==1){
		set_fvalue(a, fvalue(a)+constant);
		return pc;
	}

	return inner_send_from_stack(pc, 1, id_[op], 0);
}

const inst_t* VMachine::OpAdd(const inst_t* pc, int_t op){
	AnyPtr& b = get(); uint_t btype = type(b)-TYPE_INT;
	AnyPtr& a = get(1); uint_t atype = type(a)-TYPE_INT;
	uint_t abtype = atype | btype;

	if(abtype==0){
		set_ivalue(a, ivalue(a) + ivalue(b));
		downsize(1); 
		return pc;	
	}

	if(abtype==1){
		f2 ab;
		to_f2(ab, atype, a, btype, b);
		set_fvalue(a, ab.a + ab.b);
		downsize(1);
		return pc;	
	}

	return inner_send_from_stack(pc, 1, id_[op], 1, 0);
}

const inst_t* VMachine::OpSub(const inst_t* pc, int_t op){
	AnyPtr& b = get(); uint_t btype = type(b)-TYPE_INT;
	AnyPtr& a = get(1); uint_t atype = type(a)-TYPE_INT;
	uint_t abtype = atype | btype;

	if(abtype==0){
		set_ivalue(a, ivalue(a) - ivalue(b));
		downsize(1); 
		return pc;	
	}

	if(abtype==1){
		f2 ab;
		to_f2(ab, atype, a, btype, b);
		set_fvalue(a, ab.a - ab.b);
		downsize(1);
		return pc;	
	}

	return inner_send_from_stack(pc, 1, id_[op], 1, 0);
}

const inst_t* VMachine::OpMul(const inst_t* pc, int_t op){
	AnyPtr& b = get(); uint_t btype = type(b)-TYPE_INT;
	AnyPtr& a = get(1); uint_t atype = type(a)-TYPE_INT;
	uint_t abtype = atype | btype;

	if(abtype==0){
		set_ivalue(a, ivalue(a) * ivalue(b));
		downsize(1); 
		return pc;	
	}

	if(abtype==1){
		f2 ab;
		to_f2(ab, atype, a, btype, b);
		set_fvalue(a, ab.a * ab.b);
		downsize(1);
		return pc;	
	}

	return inner_send_from_stack(pc, 1, id_[op], 1, 0);
}

const inst_t* VMachine::OpDiv(const inst_t* pc, int_t op){
	AnyPtr& b = get(); uint_t btype = type(b)-TYPE_INT;
	AnyPtr& a = get(1); uint_t atype = type(a)-TYPE_INT;
	uint_t abtype = atype | btype;

	if(abtype==0){
		int_t bivalue = ivalue(b);
		if(bivalue==0){
			push(cpp_class<RuntimeError>()->call(Xt("Xtal Runtime Error 1024")));
			return &throw_code_;
		}

		set_ivalue(a, ivalue(a) / bivalue);
		downsize(1); 
		return pc;	
	}

	if(abtype==1){
		f2 ab;
		to_f2(ab, atype, a, btype, b);

		if(ab.b==0){
			push(cpp_class<RuntimeError>()->call(Xt("Xtal Runtime Error 1024")));
			return &throw_code_;
		}

		set_fvalue(a, ab.a / ab.b);
		downsize(1);
		return pc;	
	}

	return inner_send_from_stack(pc, 1, id_[op], 1, 0);
}

const inst_t* VMachine::OpMod(const inst_t* pc, int_t op){
	AnyPtr& b = get(); uint_t btype = type(b)-TYPE_INT;
	AnyPtr& a = get(1); uint_t atype = type(a)-TYPE_INT;
	uint_t abtype = atype | btype;

	if(abtype==0){
		int_t bivalue = ivalue(b);
		if(bivalue==0){
			push(cpp_class<RuntimeError>()->call(Xt("Xtal Runtime Error 1024")));
			return &throw_code_;
		}

		set_ivalue(a, ivalue(a) % bivalue);
		downsize(1); 
		return pc;	
	}

	if(abtype==1){
		f2 ab;
		to_f2(ab, atype, a, btype, b);

		if(ab.b==0){
			push(cpp_class<RuntimeError>()->call(Xt("Xtal Runtime Error 1024")));
			return &throw_code_;
		}

		using namespace std;
		set_fvalue(a, fmodf(ab.a, ab.b));
		downsize(1);
		return pc;	
	}

	return inner_send_from_stack(pc, 1, id_[op], 1, 0);
}

const inst_t* VMachine::OpAnd(const inst_t* pc, int_t op){
	AnyPtr& b = get(); uint_t btype = type(b)-TYPE_INT;
	AnyPtr& a = get(1); uint_t atype = type(a)-TYPE_INT;
	uint_t abtype = atype | btype;
	if(abtype==0){
		set(1, Any(ivalue(a) & ivalue(b)));
		downsize(1);
		return pc;
	}
	else{
		return inner_send_from_stack(pc, 1, id_[op], 1, 0);
	}
}

const inst_t* VMachine::OpOr(const inst_t* pc, int_t op){
	AnyPtr& b = get(); uint_t btype = type(b)-TYPE_INT;
	AnyPtr& a = get(1); uint_t atype = type(a)-TYPE_INT;
	uint_t abtype = atype | btype;
	if(abtype==0){
		set(1, Any(ivalue(a) | ivalue(b)));
		downsize(1);
		return pc;
	}
	else{
		return inner_send_from_stack(pc, 1, id_[op], 1, 0);
	}
}

const inst_t* VMachine::OpXor(const inst_t* pc, int_t op){
	AnyPtr& b = get(); uint_t btype = type(b)-TYPE_INT;
	AnyPtr& a = get(1); uint_t atype = type(a)-TYPE_INT;
	uint_t abtype = atype | btype;
	if(abtype==0){
		set(1, Any(ivalue(a) ^ ivalue(b)));
		downsize(1);
		return pc;
	}
	else{
		return inner_send_from_stack(pc, 1, id_[op], 1, 0);
	}
}

const inst_t* VMachine::OpShl(const inst_t* pc, int_t op){
	AnyPtr& b = get(); uint_t btype = type(b)-TYPE_INT;
	AnyPtr& a = get(1); uint_t atype = type(a)-TYPE_INT;
	uint_t abtype = atype | btype;
	if(abtype==0){
		set(1, Any(ivalue(a) << ivalue(b)));
		downsize(1);
		return pc;
	}
	else{
		return inner_send_from_stack(pc, 1, id_[op], 1, 0);
	}
}

const inst_t* VMachine::OpShr(const inst_t* pc, int_t op){
	AnyPtr& b = get(); uint_t btype = type(b)-TYPE_INT;
	AnyPtr& a = get(1); uint_t atype = type(a)-TYPE_INT;
	uint_t abtype = atype | btype;
	if(abtype==0){
		set(1, Any(ivalue(a) >> ivalue(b)));
		downsize(1);
		return pc;
	}
	else{
		return inner_send_from_stack(pc, 1, id_[op], 1, 0);
	}
}

const inst_t* VMachine::OpUshr(const inst_t* pc, int_t op){
	AnyPtr& b = get(); uint_t btype = type(b)-TYPE_INT;
	AnyPtr& a = get(1); uint_t atype = type(a)-TYPE_INT;
	uint_t abtype = atype | btype;
	if(abtype==0){
		set(1, Any((int_t)((uint_t)ivalue(a) >> ivalue(b))));
		downsize(1);
		return pc;
	}
	else{
		return inner_send_from_stack(pc, 1, id_[op], 1, 0);
	}
}

#undef XTAL_VM_CONTINUE
#undef XTAL_VM_THROW_EXCEPT
#undef XTAL_VM_CHECK_EXCEPT

#define XTAL_VM_CONTINUE(x) return (x)
#define XTAL_VM_THROW_EXCEPT(e) XTAL_VM_CONTINUE(push_except(pc, e))
#define XTAL_VM_CHECK_EXCEPT if(ap(except_[0])){ XTAL_VM_CONTINUE(push_except(pc)); }

//{FUNS{{
//}}FUNS}

#undef XTAL_VM_NODEFAULT
#undef XTAL_VM_CASE_FIRST
#undef XTAL_VM_CASE
#undef XTAL_VM_SWITCH
#undef XTAL_VM_DEF_INST
#undef XTAL_VM_CONTINUE
#undef XTAL_VM_RETURN

#undef XTAL_VM_EXCEPT
#undef XTAL_VM_CHECK_EXCEPT_PC
#undef XTAL_VM_THROW_EXCEPT
#undef XTAL_VM_CHECK_EXCEPT

#undef XTAL_CHECK_YIELD

}//namespace

