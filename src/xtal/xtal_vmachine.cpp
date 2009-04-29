#include "xtal.h"
#include "xtal_macro.h"

#include "xtal_stringspace.h"

namespace xtal{

inline const IDPtr& isp(const Any& v){
	return (const IDPtr&)v;
}

inline const CodePtr& VMachine::code(){ return ff().code(); }
inline const CodePtr& VMachine::prev_code(){ return prev_ff().code(); }

inline const IDPtr& VMachine::identifier(int_t n){ return code()->identifier(n); }
inline const IDPtr& VMachine::prev_identifier(int_t n){ return prev_code()->identifier(n); }

void VMachine::push_call(const inst_t* pc, int_t need_result_count, 
		int_t ordered_arg_count, int_t named_arg_count, 
		const IDPtr& primary_key, const AnyPtr& secondary_key, const AnyPtr& self){
	XTAL_ASSERT(!raweq(secondary_key, null)); // セカンダリキーが無いときはnullでなくundefinedを指定するようになったので、検出用assert

	FunFrame* fp = fun_frames_.push();
	if(!fp){
		void* p = xmalloc(sizeof(FunFrame));
		fp = new(p) FunFrame();
		fun_frames_.top() = fp;
	}

	FunFrame& f = *fp;

	f.need_result_count = need_result_count;
	f.ordered_arg_count = ordered_arg_count;
	f.named_arg_count = named_arg_count;
	f.result_count = 0;
	f.called_pc = &throw_unsupported_error_code_;
	f.poped_pc = pc;
	f.instance_variables = &empty_instance_variables;
	f.variable_size = 0;

	f.secondary_key(secondary_key);
	f.primary_key(primary_key);
	f.target(pop());

	f.self(self);
	f.fun(null);
	f.code(null);
	f.arguments(null);
	f.hint(null);
	f.outer(null);

	scopes_.push(0);
}

void VMachine::push_call(const inst_t* pc, const InstSend& inst){
	FunFrame* fp;
	if((inst.flags&CALL_FLAG_TAIL)==0){
		fp = fun_frames_.push();
		if(!fp){
			void* p = xmalloc(sizeof(FunFrame));
			fp = new(p) FunFrame();
			fun_frames_.top() = fp;
		}
		fp->poped_pc = pc;

		fp->self(prev_ff().self());
		fp->fun(null);
		fp->code(null);
		fp->arguments(null);
		fp->hint(null);
		fp->outer(null);

		fp->need_result_count = inst.need_result;
		fp->result_count = 0;
		fp->instance_variables = &empty_instance_variables;
		fp->variable_size = 0;
	}
	else{
		pop_ff();
		fp = fun_frames_.push();
	}

	FunFrame& f = *fp;

	f.ordered_arg_count = inst.ordered;
	f.named_arg_count = inst.named;

	if((inst.flags&CALL_FLAG_Q)==0){
		f.called_pc = &throw_unsupported_error_code_;
	}
	else{
		f.called_pc = &check_unsupported_code_;
	}

	if((inst.flags&CALL_FLAG_NS)==0){
		f.secondary_key(undefined);
	}
	else{
		f.secondary_key(pop());
	}

	if((inst.flags&CALL_FLAG_TAIL)==0){
		if(int_t n = inst.identifier_number){ f.primary_key(prev_identifier(n));  }
		else{ 
			f.primary_key(pop()->to_s()->intern()); 
		}
	}
	else{
		if(int_t n = inst.identifier_number){ f.primary_key(identifier(n));  }
		else{ f.primary_key(pop()->to_s()->intern()); }
	}

	f.target(pop());

	if((inst.flags&CALL_FLAG_ARGS)!=0){
		ArgumentsPtr args = ptr_cast<Arguments>(pop());
		push_args(args, inst.named);
		f.ordered_arg_count = args->ordered_size()+inst.ordered;
		f.named_arg_count = args->named_size()+inst.named;
	}

	scopes_.push(0);
}

void VMachine::push_call(const inst_t* pc, const InstCall& inst){
	FunFrame* fp;
	if((inst.flags&CALL_FLAG_TAIL)==0){
		fp = fun_frames_.push();
		if(!fp){
			void* p = xmalloc(sizeof(FunFrame));
			fp = new(p) FunFrame();
			fun_frames_.top() = fp;
		}
		fp->poped_pc = pc;

		fp->self(prev_ff().self());
		fp->fun(null);
		fp->code(null);
		fp->arguments(null);
		fp->hint(null);
		fp->outer(null);
	
		fp->need_result_count = inst.need_result;
		fp->result_count = 0;
		fp->instance_variables = &empty_instance_variables;
		fp->variable_size = 0;
	}
	else{
		pop_ff();
		fp = fun_frames_.push();
	}

	FunFrame& f = *fp;

	f.ordered_arg_count = inst.ordered;
	f.named_arg_count = inst.named;

	f.called_pc = &throw_unsupported_error_code_;

	f.secondary_key(undefined);
	f.primary_key(id_[IDOp::id_op_call]);
	f.target(pop());

	if((inst.flags&CALL_FLAG_ARGS)!=0){
		ArgumentsPtr args = ptr_cast<Arguments>(pop());
		push_args(args, inst.named);
		f.ordered_arg_count = args->ordered_size()+inst.ordered;
		f.named_arg_count = args->named_size()+inst.named;
	}

	scopes_.push(0);
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

	args->add_named(myself());
}

void VMachine::return_result_instance_variable(int_t number, ClassInfo* info){
	return_result((ff().instance_variables->variable(number, info)));
}

void VMachine::carry_over(Method* fun){
	FunFrame& f = ff();
	
	f.fun(fun);
	f.code(fun->code());
	f.outer(fun->outer());
	f.called_pc = fun->source();
	f.yieldable = f.poped_pc==&end_code_ ? false : prev_ff().yieldable;
	
	if(type(f.self())==TYPE_BASE){
		f.instance_variables = pvalue(f.self())->instance_variables();
	}

	if(fun->extendable_param()){
		f.arguments(inner_make_arguments(fun));
	}
	
	FunInfo* info = fun->info();
	if(int_t size = info->variable_size){
		variables_.upsize(size);
		f.variable_size += size;
		Any* vars = &variables_[size-1];
		for(int_t n = 0; n<size; ++n){
			vars[n] = arg(n, fun);
		}
	}
	scopes_.top() = info;
	
	int_t max_stack = info->max_stack;
	stack_.upsize(max_stack);
	stack_.downsize(max_stack + f.args_stack_size());
	f.ordered_arg_count = f.named_arg_count = 0;

	hook_setting_bit_ = debug::hook_setting_bit();
	check_debug_hook(f.called_pc, BREAKPOINT_CALL);
}

void VMachine::mv_carry_over(Method* fun){
	FunFrame& f = ff();
	
	f.fun(fun);
	f.code(fun->code());
	f.outer(fun->outer());
	f.called_pc = fun->source();
	f.yieldable = f.poped_pc==&end_code_ ? false : prev_ff().yieldable;

	if(type(f.self())==TYPE_BASE){
		f.instance_variables = pvalue(f.self())->instance_variables();
	}

	if(fun->extendable_param()){
		f.arguments(inner_make_arguments(fun));
	}
	
	// 名前付き引数は邪魔
	stack_.downsize(f.named_arg_count*2);

	FunInfo* info = fun->info();
	int_t size = info->variable_size;
	adjust_result(f.ordered_arg_count, size);
	f.ordered_arg_count = size;
	f.named_arg_count = 0;
	
	if(size){
		variables_.upsize(size);
		f.variable_size += size;
		Any* vars = &variables_[size-1];
		for(int_t n = 0; n<size; ++n){
			vars[n] = get(size-1-n);
		}
	}	
	scopes_.top() = info;
	
	int_t max_stack = info->max_stack;
	stack_.upsize(max_stack);
	stack_.downsize(max_stack + size);
	f.ordered_arg_count = 0;

	hook_setting_bit_ = debug::hook_setting_bit();
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
		XTAL_GLOBAL_INTERPRETER_LOCK{
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
	}
	else{
		// 要求している戻り値の数の方が、関数が返す戻り値より多い

		XTAL_GLOBAL_INTERPRETER_LOCK{
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
}

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

const inst_t* VMachine::push_except(const inst_t* pc){
	push(except_[0]);
	except_[0] = null;
	throw_pc_ = pc;
	return &throw_code_;
}
	
const inst_t* VMachine::push_except(const inst_t* pc, const AnyPtr& e){
	push(e);
	except_[0] = null;
	throw_pc_ = pc;
	return &throw_code_;
}

const inst_t* VMachine::inner_send_from_stack(const inst_t* pc, int_t need_result_count, const IDPtr& primary_key, int_t ntarget){
	Any secondary_key = undefined;
	Any target = get(ntarget);
	downsize(1);

	push(target);
	push_call(pc, need_result_count, 0, 0, primary_key, ap(secondary_key), ff().self());
	FunFrame& f = ff();
	XTAL_GLOBAL_INTERPRETER_LOCK{
		f.target()->rawsend(myself(), f.primary_key(), f.secondary_key(), f.self());
	}
	return XTAL_VM_CHECK_EXCEPT_PC(ff().called_pc);
}

const inst_t* VMachine::inner_send_from_stack(const inst_t* pc, int_t need_result_count, const IDPtr& primary_key, int_t ntarget, int_t na0){
	Any secondary_key = undefined;
	Any target = get(ntarget);
	Any a0 = get(na0);
	downsize(2);

	push(a0);
	push(target);
	push_call(pc, need_result_count, 1, 0, primary_key, ap(secondary_key), ff().self());
	FunFrame& f = ff();
	XTAL_GLOBAL_INTERPRETER_LOCK{
		f.target()->rawsend(myself(), f.primary_key(), f.secondary_key(), f.self());
	}
	return XTAL_VM_CHECK_EXCEPT_PC(ff().called_pc);
}

const inst_t* VMachine::inner_send_from_stack_q(const inst_t* pc, int_t need_result_count, const IDPtr& primary_key, int_t ntarget, int_t na0){
	Any secondary_key = undefined;
	Any target = get(ntarget);
	Any a0 = get(na0);
	downsize(2);

	push(a0);
	push(target);
	push_call(pc, need_result_count, 1, 0, primary_key, ap(secondary_key), ff().self());
	FunFrame& f = ff();
	f.called_pc = &check_unsupported_code_;
	XTAL_GLOBAL_INTERPRETER_LOCK{
		f.target()->rawsend(myself(), f.primary_key(), f.secondary_key(), f.self());
	}
	return XTAL_VM_CHECK_EXCEPT_PC(ff().called_pc);
}

const inst_t* VMachine::inner_send_from_stack(const inst_t* pc, int_t need_result_count, const IDPtr& primary_key, int_t ntarget, int_t na0, int_t na1){
	Any secondary_key = undefined;
	Any target = get(ntarget);
	Any a0 = get(na0);
	Any a1 = get(na1);
	downsize(3);

	push(a0); push(a1);
	push(target);
	push_call(pc, need_result_count, 2, 0, primary_key, ap(secondary_key), ff().self());
	FunFrame& f = ff();
	XTAL_GLOBAL_INTERPRETER_LOCK{
		f.target()->rawsend(myself(), f.primary_key(), f.secondary_key(), f.self());
	}
	return XTAL_VM_CHECK_EXCEPT_PC(ff().called_pc);
}

const inst_t* VMachine::inner_send_from_stack_q(const inst_t* pc, int_t need_result_count, const IDPtr& primary_key, int_t ntarget, int_t na0, int_t na1){
	Any secondary_key = undefined;
	Any target = get(ntarget);
	Any a0 = get(na0);
	Any a1 = get(na1);
	downsize(3);

	push(a0); push(a1);
	push(target);
	push_call(pc, need_result_count, 2, 0, primary_key, ap(secondary_key), ff().self());
	FunFrame& f = ff();
	f.called_pc = &check_unsupported_code_;
	XTAL_GLOBAL_INTERPRETER_LOCK{
		f.target()->rawsend(myself(), f.primary_key(), f.secondary_key(), f.self());
	}
	return XTAL_VM_CHECK_EXCEPT_PC(ff().called_pc);
}

const FramePtr& VMachine::make_outer_outer(){
	if(!scopes_.empty() && ff().variable_size){
		if(ScopeInfo* scope = scopes_.pop()){
			FramePtr outer = xnew<Frame>(null, code(), scope);
			for(uint_t i=0; i<scope->variable_size; ++i){
				outer->set_member_direct(scope->variable_size-1-i, ap(variables_[i]));
			}
			variables_.downsize(scope->variable_size);
			ff().variable_size -= scope->variable_size;
			if(debug::is_enabled() || (scope->flags&ScopeInfo::FLAG_SCOPE_CHAIN)){
				outer->set_outer(make_outer_outer());
			}
			ff().outer(outer);
		}
		scopes_.push(0);
	}
	return ff().outer();
}

const FramePtr& VMachine::make_outer(ScopeInfo* scope){
	if(debug::is_enabled() || (scope->flags&ScopeInfo::FLAG_SCOPE_CHAIN)){
		ff().outer(make_outer_outer());
	}
	return ff().outer();
}

void VMachine::set_local_variable_outer(uint_t pos, const Any& value){
	Frame* outer = ff().outer().get();
	XTAL_GLOBAL_INTERPRETER_LOCK{
		for(;;){
			uint_t variables_size = outer->block_size();
			if(pos<variables_size){
				outer->set_member_direct(variables_size-1-pos, ap(value));
				return;
			}
			pos -= variables_size;
			outer = outer->outer().get();
		}
	}
}

AnyPtr& VMachine::local_variable_outer(uint_t pos){
	Frame* outer = ff().outer().get();
	XTAL_GLOBAL_INTERPRETER_LOCK{
		for(;;){
			uint_t variables_size = outer->block_size();
			if(pos<variables_size){
				return outer->member_direct(variables_size-1-pos);
			}
			pos -= variables_size;
			outer = outer->outer().get();
		}
	}
	return undefined;
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
		pop_ff();
		return;
	}

	hook_setting_bit_ = debug::hook_setting_bit();

	thread_yield_count_ = 1000;

	XTAL_ASSERT(cur.stack_size>=0);

XTAL_GLOBAL_INTERPRETER_UNLOCK{

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

	XTAL_VM_CASE(PushInt2Byte){ // 3
		push(Any((int_t)inst.value)); 
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(PushFloat1Byte){ // 3
		push(Any((float_t)inst.value)); 
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(PushFloat2Byte){ // 3
		push(Any((float_t)inst.value)); 
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(PushCallee){ // 3
		push(fun()); 
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(PushArgs){ // 3
		push(ff().arguments());
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

	XTAL_VM_CASE(Insert3){ // 7
		Any temp = get(); 
		set(get(1)); 
		set(1, get(2)); 
		set(2, get(3)); 
		set(3, temp); 
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}

	XTAL_VM_CASE(AdjustResult){ // 3
		adjust_result(inst.result_count, inst.need_result_count);
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(LocalVariableInc){ // 2
		XTAL_VM_CONTINUE(OpAddConstantInt(pc + inst.ISIZE + InstSetLocalVariable1Byte::ISIZE, pc + inst.ISIZE, 
			IDOp::id_op_inc, local_variable(inst.number), 1));
	}

	XTAL_VM_CASE(LocalVariableDec){ // 2
		XTAL_VM_CONTINUE(OpAddConstantInt(pc + inst.ISIZE + InstSetLocalVariable1Byte::ISIZE, pc + inst.ISIZE, 
			IDOp::id_op_dec, local_variable(inst.number), -1));
	}

	XTAL_VM_CASE(LocalVariableInc2Byte){ // 2
		XTAL_VM_CONTINUE(OpAddConstantInt(pc + inst.ISIZE + InstSetLocalVariable2Byte::ISIZE, pc + inst.ISIZE, 
			IDOp::id_op_inc, local_variable(inst.number), 1));
	}

	XTAL_VM_CASE(LocalVariableDec2Byte){ // 2
		XTAL_VM_CONTINUE(OpAddConstantInt(pc + inst.ISIZE + InstSetLocalVariable2Byte::ISIZE, pc + inst.ISIZE, 
			IDOp::id_op_dec, local_variable(inst.number), -1));
	}

	XTAL_VM_CASE(LocalVariable1Byte){ // 3
		push(local_variable(inst.number)); 
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(LocalVariable1ByteX2){ // 4
		push(local_variable(inst.number1)); 
		push(local_variable(inst.number2)); 
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(LocalVariable1ByteX3){ // 5
		push(local_variable(inst.number1)); 
		push(local_variable(inst.number2)); 
		push(local_variable(inst.number3)); 
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(LocalVariable2Byte){ // 3
		push(local_variable(inst.number)); 
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(SetLocalVariable1Byte){ // 4
		set_local_variable(inst.number, get());
		downsize(1);
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(SetLocalVariable2Byte){ // 4
		set_local_variable(inst.number, get()); 
		downsize(1);
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(InstanceVariable){ // 4
		FunFrame& f = ff();
		XTAL_GLOBAL_INTERPRETER_LOCK{
			push(f.instance_variables->variable(inst.number, code()->class_info(inst.info_number)));
		}
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(SetInstanceVariable){ // 5
		FunFrame& f = ff();
		XTAL_GLOBAL_INTERPRETER_LOCK{ 
			f.instance_variables->set_variable(inst.number, code()->class_info(inst.info_number), get());
			downsize(1);
		}
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(CleanupCall){ // 2
		XTAL_VM_CONTINUE(pop_ff());
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
			XTAL_GLOBAL_INTERPRETER_LOCK{ 
				XTAL_VM_THROW_EXCEPT(cpp_class<YieldError>()->call(Xt("Xtal Runtime Error 1012")));
			}
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
			adjust_result(1);
		}

		XTAL_VM_CONTINUE(pop_ff());
	}

	XTAL_VM_CASE(Property){ // 13
		XTAL_GLOBAL_INTERPRETER_LOCK{
			const ClassPtr& cls = get().get_class();
			const AnyPtr& ret = ap(cls)->member(identifier(inst.identifier_number), undefined, ff().self(), true);
			XTAL_VM_CHECK_EXCEPT;
			if(raweq(ret.get_class(), cpp_class<InstanceVariableGetter>())){
				const AnyPtr& self = get();
				set(self->instance_variables()->variable(unchecked_ptr_cast<InstanceVariableGetter>(ret)->number_, unchecked_ptr_cast<InstanceVariableGetter>(ret)->info_));
				XTAL_VM_CHECK_EXCEPT;
				XTAL_VM_CONTINUE(pc + inst.ISIZE); 	
			}

			push_call(pc + inst.ISIZE, inst.need_result, 0, 0, identifier(inst.identifier_number), undefined, ff().self());
			FunFrame& f = ff();
			f.target()->rawsend(myself(), f.primary_key(), f.secondary_key(), f.self());
			XTAL_VM_CHECK_EXCEPT;
		}
		XTAL_VM_CONTINUE(ff().called_pc); 	
	}

	XTAL_VM_CASE(SetProperty){ // 14
		XTAL_GLOBAL_INTERPRETER_LOCK{
			const ClassPtr& cls = get().get_class();
			const AnyPtr& ret = ap(cls)->member(identifier(inst.identifier_number), undefined, ff().self(), true);
			XTAL_VM_CHECK_EXCEPT;
			if(raweq(ret.get_class(), cpp_class<InstanceVariableSetter>())){
				const AnyPtr& self = get();
				self->instance_variables()->set_variable(unchecked_ptr_cast<InstanceVariableSetter>(ret)->number_, unchecked_ptr_cast<InstanceVariableSetter>(ret)->info_, get(1));
				downsize(2);
				XTAL_VM_CHECK_EXCEPT;
				XTAL_VM_CONTINUE(pc + inst.ISIZE); 	
			}

			push_call(pc + inst.ISIZE, 0, 1, 0, identifier(inst.identifier_number), undefined, ff().self());
			FunFrame& f = ff();
			f.target()->rawsend(myself(), f.primary_key(), f.secondary_key(), f.self());
			XTAL_VM_CHECK_EXCEPT;
		}
		XTAL_VM_CONTINUE(ff().called_pc); 	
	}

	XTAL_VM_CASE(Call){ // 6
		XTAL_GLOBAL_INTERPRETER_LOCK{
			push_call(pc + inst.ISIZE, inst);
			FunFrame& f = ff();
			f.target()->rawcall(myself());
			XTAL_VM_CHECK_EXCEPT;
		}
		XTAL_VM_CONTINUE(ff().called_pc);	
	}

	XTAL_VM_CASE(Send){ // 6
		XTAL_GLOBAL_INTERPRETER_LOCK{
			push_call(pc + inst.ISIZE, inst);
			FunFrame& f = ff();
			f.target()->rawsend(myself(), f.primary_key(), f.secondary_key(), f.self());
			XTAL_VM_CHECK_EXCEPT;
		}
		XTAL_VM_CONTINUE(ff().called_pc); 	
	}

	XTAL_VM_CASE(Member){ // 12
		XTAL_GLOBAL_INTERPRETER_LOCK{
			FunFrame& f = ff();
			f.secondary_key((inst.flags&CALL_FLAG_NS) ? pop() : undefined);
			if(int_t n = inst.identifier_number){ f.primary_key(identifier(n)); }
			else{ f.primary_key(pop()->to_s()->intern()); }
			f.target(get());
			Any ret = f.target()->member(f.primary_key(), f.secondary_key(), f.self());
			XTAL_VM_CHECK_EXCEPT;

			if(inst.flags&CALL_FLAG_Q){
				set(ret);
			}
			else{
				if(rawne(ret, undefined)){
					set(ret);
				}
				else{
					XTAL_VM_THROW_EXCEPT(unsupported_error(f.target(), f.primary_key(), f.secondary_key()));
				}
			}
		}
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(DefineMember){ // 10
		XTAL_GLOBAL_INTERPRETER_LOCK{
			FunFrame& f = ff();
			f.secondary_key((inst.flags&CALL_FLAG_NS) ? pop() : undefined);
			if(int_t n = inst.identifier_number){ f.primary_key(identifier(n)); }
			else{ f.primary_key(pop()->to_s()->intern()); }
			AnyPtr value = pop();
			f.target(pop());
			f.target()->def(f.primary_key(), ap(value), f.secondary_key(), KIND_PUBLIC);
			XTAL_VM_CHECK_EXCEPT;
		}
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(BlockBegin){ // 11
		FunFrame& f = ff(); 
		ScopeInfo* info = f.code()->scope_info(inst.info_number);
		uint_t size = info->variable_size;
		variables_.upsize(size);
		f.variable_size += size;
		for(uint_t i=0; i<size; ++i){
			set_nullt(variables_[i]);
		}
		scopes_.push(info);
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}

	XTAL_VM_CASE(BlockEnd){ // 6
		FunFrame& f = ff();
		if(ScopeInfo* scope = scopes_.pop()){
			variables_.downsize(scope->variable_size);
			f.variable_size -= scope->variable_size;
		}
		else{
			f.outer(f.outer()->outer());
		}
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}

	XTAL_VM_CASE(TryBegin){ // 9
		FunFrame& f = ff(); 
		ExceptFrame& ef = except_frames_.push();
		ef.info = f.code()->except_info(inst.info_number);
		ef.fun_frame_size = fun_frames_.size();
		ef.stack_size = stack_.size();
		ef.scope_size = scopes_.size();
		ef.outer = f.outer();
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}

	XTAL_VM_CASE(TryEnd){ // 2
		XTAL_VM_CONTINUE(except_frames_.pop().info->finally_pc + code()->data()); 
	}

	XTAL_VM_CASE(PushGoto){ // 3
		push(Any((int_t)((pc+inst.OFFSET_address+inst.address)-code()->data()))); 
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(PopGoto){ // 2
		XTAL_VM_CONTINUE(code()->data()+ivalue(pop()));
	}

	XTAL_VM_CASE(Goto){ // 3
		XTAL_CHECK_YIELD;
		XTAL_VM_CONTINUE(pc + inst.OFFSET_address + inst.address); 
	}

	XTAL_VM_CASE(If){ // 2
		XTAL_VM_CONTINUE(pc + (pop() ? 
			inst.OFFSET_address_true + inst.address_true : 
			inst.OFFSET_address_false + inst.address_false));
	}

	XTAL_VM_CASE(IfEq){ // 14
		const AnyPtr& b = get(); uint_t btype = type(b)-TYPE_INT;
		const AnyPtr& a = get(1); uint_t atype = type(a)-TYPE_INT;
		uint_t abtype = atype | btype;
		
		typedef InstIf InstType2; 
		InstType2& inst2 = *(InstType2*)(pc+inst.ISIZE);

		if(abtype==0){
			downsize(2); 
			XTAL_VM_CONTINUE((
				ivalue(a) == ivalue(b) ? 
				inst2.OFFSET_address_true+inst2.address_true : 
				inst2.OFFSET_address_false+inst2.address_false) + pc + inst.ISIZE);
		}

		if(abtype==1){
			f2 ab = to_f2(atype, a, btype, b);
			downsize(2); 
			XTAL_VM_CONTINUE((
				ab.a == ab.b ? 
				inst2.OFFSET_address_true+inst2.address_true : 
				inst2.OFFSET_address_false+inst2.address_false) + pc + inst.ISIZE);
		}

		XTAL_VM_CONTINUE(inner_send_from_stack_q(pc+inst.ISIZE, 1, id_[IDOp::id_op_eq], 1, 0));
	}

	XTAL_VM_CASE(IfLt){ // 14
		const AnyPtr& b = get(); uint_t btype = type(b)-TYPE_INT;
		const AnyPtr& a = get(1); uint_t atype = type(a)-TYPE_INT;
		uint_t abtype = atype | btype;

		typedef InstIf InstType2; 
		InstType2& inst2 = *(InstType2*)(pc+inst.ISIZE);

		if(abtype==0){
			downsize(2); 
			XTAL_VM_CONTINUE((
				ivalue(a) < ivalue(b) ? 
				inst2.OFFSET_address_true+inst2.address_true : 
				inst2.OFFSET_address_false+inst2.address_false) + pc + inst.ISIZE);
		}

		if(abtype==1){
			f2 ab = to_f2(atype, a, btype, b);
			downsize(2); 
			XTAL_VM_CONTINUE(
				(ab.a < ab.b ? 
				inst2.OFFSET_address_true+inst2.address_true : 
				inst2.OFFSET_address_false+inst2.address_false) + pc + inst.ISIZE);
		}
		
		XTAL_VM_CONTINUE(inner_send_from_stack(pc+inst.ISIZE, 1, id_[IDOp::id_op_lt], 1, 0));
	}

	XTAL_VM_CASE(IfRawEq){ // 6
		typedef InstIf InstType2; 
		InstType2& inst2 = *(InstType2*)(pc+inst.ISIZE);

		pc = (raweq(get(1), get()) ? 
			(int)(inst2.OFFSET_address_true+inst2.address_true) : 
			(int)(inst2.OFFSET_address_false+inst2.address_false)) + pc + inst.ISIZE;

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
			(int)(inst2.OFFSET_address_true+inst2.address_true) : 
			(int)(inst2.OFFSET_address_false+inst2.address_false)) + pc + inst.ISIZE;
		downsize(2);
		XTAL_VM_CONTINUE(pc);
	}

	XTAL_VM_CASE(IfArgIsUndefined){ // 3
		if(raweq(local_variable(inst.arg), undefined)){
			XTAL_VM_CONTINUE(pc + inst.ISIZE);
		}
		else{
			XTAL_VM_CONTINUE(pc + inst.OFFSET_address + inst.address); 
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
		//else if(type(get(1))==TYPE_MAP){
		//	Any ret = unchecked_ptr_cast<Map>(get(1))->op_at(ivalue(get()));
		//	XTAL_VM_CHECK_EXCEPT;
		//	downsize(1);
		//	set(ret);
		//	XTAL_VM_CONTINUE(pc+inst.ISIZE); 
		//}
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
		//else if(type(get(2))==TYPE_MAP){
		//	unchecked_ptr_cast<Map>(get(2))->op_set_at(get(1), get());
		//	XTAL_VM_CHECK_EXCEPT;
		//	downsize(3);
		//	XTAL_VM_CONTINUE(pc+inst.ISIZE); 
		//}
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
		XTAL_GLOBAL_INTERPRETER_LOCK{
			Any rhs = pop();
			Any lhs = pop();
			push(rhs); 
			push(Any((int_t)inst.kind));
			push(lhs);
			push_call(pc+inst.ISIZE, 1, 2, 0, id_[IDOp::id_op_range], undefined, ff().self());
			ap(lhs)->rawsend(myself(), id_[IDOp::id_op_range], undefined);
			XTAL_VM_CHECK_EXCEPT;
		}
		XTAL_VM_CONTINUE(ff().called_pc);
	}

	XTAL_VM_CASE(GlobalVariable){ // 5
		XTAL_GLOBAL_INTERPRETER_LOCK{
			const AnyPtr& ret = code()->filelocal()->member(identifier(inst.identifier_number));
			if(rawne(ret, undefined)){
				push(ret);
			}
			else{
				XTAL_VM_THROW_EXCEPT(unsupported_error(code()->filelocal(), identifier(inst.identifier_number), undefined));
			}
		}
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}

	XTAL_VM_CASE(SetGlobalVariable){ // 4
		XTAL_GLOBAL_INTERPRETER_LOCK{
			if(!code()->filelocal()->set_member(identifier(inst.identifier_number), get(), undefined)){
				XTAL_VM_THROW_EXCEPT(unsupported_error(code()->filelocal(), identifier(inst.identifier_number), undefined));
			}
			downsize(1);
		}
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}

	XTAL_VM_CASE(DefineGlobalVariable){ // 5
		XTAL_GLOBAL_INTERPRETER_LOCK{
			code()->filelocal()->def(identifier(inst.identifier_number), get(), undefined, KIND_PUBLIC);
			XTAL_VM_CHECK_EXCEPT;
			downsize(1);
		}
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}

	XTAL_VM_CASE(Once){ // 5
		XTAL_GLOBAL_INTERPRETER_LOCK{
			const AnyPtr& ret = code()->once_value(inst.value_number);
			if(rawne(ret, undefined)){
				push(ret);
				XTAL_VM_CONTINUE(pc + inst.OFFSET_address + inst.address);
			}
			else{
				XTAL_VM_CONTINUE(pc + inst.ISIZE);
			}
		}
	}

	XTAL_VM_CASE(SetOnce){ // 3
		XTAL_GLOBAL_INTERPRETER_LOCK{
			code()->set_once_value(inst.value_number, pop()); 
		}
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(ClassBegin){ // 19
		XTAL_GLOBAL_INTERPRETER_LOCK{
			ClassInfo* info = code()->class_info(inst.info_number);
			const FramePtr& outer = make_outer(info);
			scopes_.push(0);
			ClassPtr cp;

			switch(info->kind){
				XTAL_CASE(KIND_CLASS){
					cp = xnew<Class>(outer, code(), info);
				}

				XTAL_CASE(KIND_SINGLETON){
					cp = xnew<Singleton>(outer, code(), info);
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

			push(null);
			push_call(pc + inst.ISIZE, 0, 0, 0, null, undefined, cp);

			ff().fun(prev_fun());
			ff().code(prev_fun()->code());
			ff().outer(cp);
		}
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}

	XTAL_VM_CASE(ClassEnd){ // 13
		if(raweq(ff().outer()->get_class(), ff().outer())){
			Singleton* singleton = (Singleton*)pvalue(ff().outer());
			singleton->init_singleton(myself());
		}

		push(ff().outer());
		ff().outer(ff().outer()->outer());
		pop_ff();
		scopes_.pop();
		XTAL_VM_CONTINUE(pc + inst.ISIZE);;;;;;
	}

	XTAL_VM_CASE(DefineClassMember){ // 5
		XTAL_GLOBAL_INTERPRETER_LOCK{
			const ClassPtr& p = cast<const ClassPtr&>(ff().outer());
			p->set_member_direct(p->block_size()-1-inst.number, identifier(inst.identifier_number), get(1), get(), inst.accessibility);
			downsize(2);
		}
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}

	XTAL_VM_CASE(MakeArray){ // 3
		XTAL_GLOBAL_INTERPRETER_LOCK{
			push(xnew<Array>());
		}
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}

	XTAL_VM_CASE(ArrayAppend){ // 4
		XTAL_GLOBAL_INTERPRETER_LOCK{
			cast<const ArrayPtr&>(get(1))->push_back(get()); 
			downsize(1);
		}
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}

	XTAL_VM_CASE(MakeMap){ // 3
		XTAL_GLOBAL_INTERPRETER_LOCK{
			push(xnew<Map>());
		}
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}

	XTAL_VM_CASE(MapInsert){ // 4
		XTAL_GLOBAL_INTERPRETER_LOCK{
			cast<const MapPtr&>(get(2))->set_at(get(1), get()); 
			downsize(2);	
		}
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}

	XTAL_VM_CASE(MapSetDefault){ // 4
		XTAL_GLOBAL_INTERPRETER_LOCK{
			cast<const MapPtr&>(get(1))->set_default_value(get()); 
			downsize(1);	
		}
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}

	XTAL_VM_CASE(MakeFun){ // 10
		int_t table_n = inst.info_number, end = inst.OFFSET_address + inst.address;
		XTAL_GLOBAL_INTERPRETER_LOCK{
			FunInfo* info = code()->fun_info(table_n);
			const FramePtr& outer = make_outer(info);
			switch(info->kind){
				XTAL_NODEFAULT;
				XTAL_CASE(KIND_FUN){ push(xnew<Fun>(outer, ff().self(), code(), info)); }
				XTAL_CASE(KIND_LAMBDA){ push(xnew<Lambda>(outer, ff().self(), code(), info)); }
				XTAL_CASE(KIND_METHOD){ push(xnew<Method>(outer, code(), info)); }
				XTAL_CASE(KIND_FIBER){ push(xnew<Fiber>(outer, ff().self(), code(), info)); }
			}
		}
		XTAL_VM_CONTINUE(pc + end);
	}

	XTAL_VM_CASE(MakeInstanceVariableAccessor){ // 7
		XTAL_GLOBAL_INTERPRETER_LOCK{
			AnyPtr ret;
			switch(inst.type){
				XTAL_NODEFAULT;

				XTAL_CASE(0){ ret = xnew<InstanceVariableGetter>(inst.number, code()->class_info(inst.info_number)); }
				XTAL_CASE(1){ ret = xnew<InstanceVariableSetter>(inst.number, code()->class_info(inst.info_number)); }
			}
			push(ret);
		}
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(Throw){ // 6
		XTAL_GLOBAL_INTERPRETER_LOCK{
			//make_outer_outer();

			AnyPtr except = pop();
			if(!except){
				except = ap(except_[0]);
			}

			if(!except->is(cpp_class<Exception>())){
				XTAL_VM_EXCEPT(cpp_class<RuntimeError>()->call(except));
			}
			else{
				XTAL_VM_EXCEPT(except);
			}
		}
	}

	XTAL_VM_CASE(ThrowUnsupportedError){ // 4
		XTAL_GLOBAL_INTERPRETER_LOCK{
			FunFrame& f = ff();
			AnyPtr target_class = f.target() ? f.target()->get_class() : unchecked_ptr_cast<Class>(null);
			XTAL_VM_THROW_EXCEPT(unsupported_error(target_class, f.primary_key(), f.secondary_key()));
		}
	}

	XTAL_VM_CASE(IfDebug){ // 2
		XTAL_VM_CONTINUE(pc + (debug::is_enabled() ? inst.ISIZE : inst.OFFSET_address + inst.address));
	}

	XTAL_VM_CASE(Assert){ // 4
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
	set_except(append_backtrace(pc, ap(e)));

	// Xtalソース内でキャッチ等あるか調べる
	pc = catch_body(pc, cur);

	// Xtalソース内でキャッチ等されたなら最初に戻る
	if(pc){
		goto begin;
	}

	// Xtalで記述された部分から抜け出す
	pop_ff();
	return;
}

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
		f2 ab = to_f2(atype, a, btype, b);
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
		f2 ab = to_f2(atype, a, btype, b);
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
		f2 ab = to_f2(atype, a, btype, b);
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
		f2 ab = to_f2(atype, a, btype, b);

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
		f2 ab = to_f2(atype, a, btype, b);

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

