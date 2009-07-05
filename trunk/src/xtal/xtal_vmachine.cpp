#include "xtal.h"
#include "xtal_macro.h"

#include "xtal_stringspace.h"
#include "xtal_details.h"

namespace xtal{

#undef XTAL_USE_COMPUTED_GOTO

//#define XTAL_VM_DEF_INST(key) typedef const Inst##key InstType
//#define inst (*(InstType*)pc)
#define XTAL_VM_DEF_INST(key) const Inst##key& inst = *(const Inst##key*)pc

#define XTAL_VM_RETURN return
#define XTAL_VM_CHECK_EXCEPT_PC(pc) (ap(except_[0]) ? push_except(pc) : (pc))
#define XTAL_VM_THROW_EXCEPT(e) XTAL_VM_CONTINUE(push_except(pc, e))
#define XTAL_VM_CHECK_EXCEPT if(ap(except_[0])){ XTAL_VM_CONTINUE(push_except(pc)); }

#ifdef XTAL_USE_COMPUTED_GOTO
#	define XTAL_COPY_LABEL_ADDRESS(key) &&Label##key
#	define XTAL_VM_CASE_FIRST(key) Label##key: { XTAL_VM_DEF_INST(key);
#	define XTAL_VM_CASE(key) } Label##key: { XTAL_VM_DEF_INST(key);
#	define XTAL_VM_LOOP goto *labels[*pc];
#	define XTAL_VM_LOOP_END }
#	define XTAL_VM_CONTINUE(x) { pc = (x); goto *labels[*pc]; }
#	define XTAL_VM_CONTINUE0 goto *labels[*pc]
#else
#	define XTAL_VM_CASE_FIRST(key) case Inst##key::NUMBER: { XTAL_VM_DEF_INST(key);
#	define XTAL_VM_CASE(key) } case Inst##key::NUMBER: /*printf("%s\n", #key);*/ { XTAL_VM_DEF_INST(key);
#	define XTAL_VM_LOOP for(;;)switch(*pc){
#	define XTAL_VM_LOOP_END } XTAL_NODEFAULT; }
#	define XTAL_VM_CONTINUE(x) { pc = (x); continue; }
#	define XTAL_VM_CONTINUE0 continue
#endif

#ifdef XTAL_NO_THREAD
#	define XTAL_CHECK_YIELD
#else
#	define XTAL_CHECK_YIELD if(--thread_yield_count_<0){ yield_thread(); thread_yield_count_ = 1000; }
#endif

#define XTAL_VM_FUN

const ClassPtr& Any::get_class() const{
	int_t t = type(*this);
	if(t==TYPE_BASE){ return pvalue(*this)->get_class(); }
	return cpp_class(*classdata[t]);
}

bool Any::is(const AnyPtr& klass) const{
	const ClassPtr& my_class = get_class();
	if(raweq(my_class, klass)) return true;
	return cache_is(my_class, klass);
}

const AnyPtr& MemberCacheTable::cache(const AnyPtr& target_class, const IDPtr& primary_key, const AnyPtr& secondary_key, int_t& accessibility){
	uint_t itarget_class = rawvalue(target_class).uvalue>>2;
	uint_t iprimary_key = rawvalue(primary_key).uvalue;
	uint_t isecondary_key = rawvalue(secondary_key).uvalue;

	uint_t hash = (itarget_class ^ iprimary_key ^ isecondary_key) ^ (iprimary_key>>3)*3 ^ isecondary_key*7;
	Unit& unit = table_[calc_index(hash)];

	if(((mutate_count_^unit.mutate_count) | 
		rawbitxor(primary_key, unit.primary_key) | 
		rawbitxor(target_class, unit.target_class) | 
		rawbitxor(secondary_key, unit.secondary_key))==0){
		hit_++;
		accessibility = unit.accessibility;
		return ap(unit.member);
	}
	else{

		miss_++;

		if(type(target_class)!=TYPE_BASE){
			return undefined;
		}

		bool nocache = false;
		const AnyPtr& ret = pvalue(target_class)->rawmember(primary_key, ap(secondary_key), true, accessibility, nocache);
		//if(rawne(ret, undefined)){
			unit.member = ret;
			if(!nocache){
				unit.target_class = target_class;
				unit.primary_key = primary_key;
				unit.secondary_key = secondary_key;
				unit.accessibility = accessibility;
				unit.mutate_count = mutate_count_;
			}
			else{
				unit.mutate_count = mutate_count_-1;
			}
			return ap(unit.member);
		//}
		//return undefined;
	}
}

bool IsCacheTable::cache(const AnyPtr& target_class, const AnyPtr& klass){
	uint_t itarget_class = rawvalue(target_class).uvalue;
	uint_t iklass = rawvalue(klass).uvalue;

	uint_t hash = (itarget_class>>3) ^ (iklass>>2);
	Unit& unit = table_[hash & CACHE_MASK];
	
	if(mutate_count_==unit.mutate_count && 
		raweq(target_class, unit.target_class) && 
		raweq(klass, unit.klass)){
		hit_++;
		return unit.result;
	}
	else{
		miss_++;

		bool ret = unchecked_ptr_cast<Class>(ap(target_class))->is_inherited(ap(klass));

		// キャッシュに保存
		//if(ret){
			unit.target_class = target_class;
			unit.klass = klass;
			unit.mutate_count = mutate_count_;
			unit.result = ret;
		//}
		return ret;
	}
}

bool CtorCacheTable::cache(const AnyPtr& target_class, int_t kind){
	uint_t itarget_class = rawvalue(target_class).uvalue;

	uint_t hash = (itarget_class>>3) ^ kind*37;
	Unit& unit = table_[hash & CACHE_MASK];
	
	if(mutate_count_==unit.mutate_count && 
		raweq(target_class, unit.target_class) &&
		kind==kind){
		hit_++;
		return true;
	}
	else{
		miss_++;

		// キャッシュに保存
		{
			unit.target_class = target_class;
			unit.kind = kind;
			unit.mutate_count = mutate_count_;
		}
		return false;
	}
}

////////////////////////////////////////////////

void VMachine::push_args(const ArgumentsPtr& args, int_t stack_base, int_t ordered_arg_count, int_t named_arg_count){
	if(!named_arg_count){
		for(uint_t i = 0; i<args->ordered_size(); ++i){
			set_local_variable(stack_base+ordered_arg_count+i, args->op_at_int(i));
		}
	}
	else{
		int_t usize = args->ordered_size();
		int_t offset = named_arg_count*2;
		for(int_t i = 0; i<offset; ++i){
			set_local_variable(stack_base+ordered_arg_count+i+usize, local_variable(ordered_arg_count+i));
		}

		for(int_t i = 0; i<usize; ++i){
			set_local_variable(stack_base+ordered_arg_count+i, args->op_at_int(i));
		}
	}

	int_t i=0;
	Xfor2(key, value, args->named_arguments()){
		set_local_variable(stack_base+ordered_arg_count+named_arg_count*2+i*2+0, key);
		set_local_variable(stack_base+ordered_arg_count+named_arg_count*2+i*2+1, value);
		++i;
	}
}

void VMachine::carry_over(Method* fun){
	FunFrame& f = ff();
	
	f.set_fun(to_smartptr(fun));
	f.called_pc = fun->source();
	f.yieldable = f.poped_pc==&end_code_ ? false : prev_ff().yieldable;
	f.instance_variables = f.self()->instance_variables();
	f.processed = 1;

	FunInfo* info = fun->info();
	
	if(fun->extendable_param()){
		set_local_variable(info->variable_size-1, inner_make_arguments(fun));
	}
	else{
		if(f.ordered_arg_count!=fun->param_size()){
			adjust_args(fun, fun->param_size());
		}
	}

	push_scope(info);

	f.ordered_arg_count = f.named_arg_count = 0;

	check_debug_hook(f.called_pc, BREAKPOINT_CALL);
}

void VMachine::mv_carry_over(Method* fun){
	FunFrame& f = ff();
	
	f.set_fun(to_smartptr(fun));
	f.called_pc = fun->source();
	f.yieldable = f.poped_pc==&end_code_ ? false : prev_ff().yieldable;
	f.instance_variables = f.self()->instance_variables();
	f.processed = 1;

	FunInfo* info = fun->info();

	int_t size = info->variable_size;
	if(f.ordered_arg_count!=size){
		adjust_values2(0, f.ordered_arg_count, size);
	}

	f.ordered_arg_count = size;
	f.named_arg_count = 0;

	push_scope(info);

	check_debug_hook(f.called_pc, BREAKPOINT_CALL);
}

void VMachine::adjust_values(int_t stack_base, int_t n, int_t need_result_count){
	// 関数が返す戻り値の数と要求している戻り値の数が等しい
	if(need_result_count==n){
		return;
	}

	// 戻り値なんて要求してない
	if(need_result_count==0){
		stack_.downsize(n);
		return;
	}

	// 関数が返す戻り値が一つも無いのでundefinedで埋める
	if(n==0){
		for(int_t i = 0; i<need_result_count; ++i){
			stack_.push(undefined);
		}
		return;
	}

	// この時点で、nもneed_result_countも1以上


	// 要求している戻り値の数の方が、関数が返す戻り値より少ない
	if(need_result_count<n){

		// 余った戻り値を多値に直す。
		int_t size = n-need_result_count+1;
		ValuesPtr ret;
		if(type(stack_.get(0))==TYPE_VALUES){
			ret = unchecked_ptr_cast<Values>(ap(stack_.get(0)));
		}
		else{
			ret = xnew<Values>(ap(stack_.get(0)));
		}

		for(int_t i=1; i<size; ++i){
			ret = xnew<Values>(ap(stack_.get(i)), ret);
		}
		stack_.downsize(size);
		stack_.push(ret);
	}
	else{
		// 要求している戻り値の数の方が、関数が返す戻り値より多い

		if(type(stack_.get(0))==TYPE_VALUES){
			// 最後の要素の多値を展開し埋め込む
			ValuesPtr mv = unchecked_ptr_cast<Values>(ap(stack_.get(0)));
			stack_.downsize(1);

			const ValuesPtr* cur = &mv;
			int_t size = 0;
			while(true){
				if((*cur)->tail()){
					if(need_result_count==n+size){
						stack_.push(*cur);
						++size;
						break;
					}

					stack_.push((*cur)->head());
					++size;		
					cur = &(*cur)->tail();
				}
				else{
					stack_.push((*cur)->head());
					++size;		
					while(need_result_count>=n+size){
						stack_.push(undefined);
						++size;
					}
					break;
				}
			}
		}
		else{
			// 最後の要素が多値ではないので、undefinedで埋めとく
			for(int_t i = n; i<need_result_count; ++i){
				stack_.push(undefined);
			}
		}
	}
}

void VMachine::adjust_values2(int_t stack_base, int_t n, int_t need_result_count){

	// 関数が返す戻り値の数と要求している戻り値の数が等しい
	if(need_result_count==n){
		return;
	}

	// 戻り値なんて要求してない
	if(need_result_count==0){
		return;
	}

	// 関数が返す戻り値が一つも無いのでundefinedで埋める
	if(n==0){
		for(int_t i = 0; i<need_result_count; ++i){
			set_local_variable(stack_base+i, undefined);
		}
		return;
	}

	// この時点で、nもneed_result_countも1以上


	// 要求している戻り値の数の方が、関数が返す戻り値より少ない
	if(need_result_count<n){

		// 余った戻り値を多値に直す。
		int_t size = n-need_result_count+1;
		ValuesPtr ret;
		AnyPtr top = local_variable(stack_base+n-1);
		if(type(top)==TYPE_VALUES){
			ret = unchecked_ptr_cast<Values>(top);
		}
		else{
			ret = xnew<Values>(top);
		}

		for(int_t i=1; i<size; ++i){
			ret = xnew<Values>(local_variable(stack_base+n-1-i), ret);
		}

		set_local_variable(stack_base+need_result_count-1, ret);
	}
	else{
		// 要求している戻り値の数の方が、関数が返す戻り値より多い

		if(type(local_variable(stack_base+n-1))==TYPE_VALUES){
			// 最後の要素の多値を展開し埋め込む
			ValuesPtr mv = unchecked_ptr_cast<Values>(local_variable(stack_base+n-1));

			const ValuesPtr* cur = &mv;
			int_t size = 0;
			while(true){
				if((*cur)->tail()){
					if(need_result_count==n+size){
						set_local_variable(stack_base+n-1+size, *cur);
						++size;
						break;
					}

					set_local_variable(stack_base+n-1+size, (*cur)->head());
					++size;		
					cur = &(*cur)->tail();
				}
				else{
					set_local_variable(stack_base+n-1+size, (*cur)->head());
					++size;		
					while(need_result_count>=n+size){
						set_local_variable(stack_base+n-1+size, undefined);
						++size;
					}
					break;
				}
			}
		}
		else{
			// 最後の要素が多値ではないので、undefinedで埋めとく
			for(int_t i = n; i<need_result_count; ++i){
				set_local_variable(stack_base+i, undefined);
			}
		}
	}
}

void VMachine::upsize_variables(uint_t upsize){
	int_t top = variables_top();

	if(variables_.size()<top+upsize+128){
		variables_.resize(top+upsize+256);
		set_variables_top(top);

		for(uint_t i=0; i<scopes_.size(); ++i){
			Scope& scope = scopes_[i]; 
			if(scope.flags!=Scope::CLASS){
				XTAL_ASSERT(scope.pos+scope.size<=variables_.size());
				scopes_[i].frame->members_.attach((AnyPtr*)variables_.data()+scope.pos, scope.size);
			}
		}
	}

	variables_top_ += upsize;
}

void VMachine::push_ff(const AnyPtr& self, 
	const inst_t* npc, 
	int_t result, int_t need_result_count, 
	int_t stack_base, int_t ordered_arg_count, int_t named_arg_count){

	FunFrame* fp = fun_frames_.push();
	if(!fp){ 
		fun_frames_.top() = fp = new(xmalloc(sizeof(FunFrame))) FunFrame();
	}

	FunFrame& f = *fp;
	f.processed = 0;
	f.need_result_count = need_result_count;
	f.ordered_arg_count = ordered_arg_count;
	f.named_arg_count = named_arg_count;
	f.result_count = 0;
	f.called_pc = 0;
	f.poped_pc = npc;
	f.instance_variables = (InstanceVariables*)&empty_instance_variables;
	f.result = result;
	f.prev_stack_base = variables_top();
	f.stack_base = stack_base + f.prev_stack_base;
	f.scope_size = scopes_.size();
	f.set_self(self);
	f.set_fun();

	upsize_variables(stack_base);
}

void VMachine::pop_ff(){
	FunFrame& f = *fun_frames_.top();
	set_variables_top(f.prev_stack_base);
	result_base_ = f.result;

	if(f.need_result_count!=f.result_count){
		adjust_values(f.result_count); 
	}

	for(uint_t i=0, sz=f.need_result_count; i<sz; ++i){
		set_local_variable(result_base_+i, stack_[sz-1-i]);
	}

	stack_.downsize(f.need_result_count);

	fun_frames_.downsize(1);
	fun_frames_.top()->processed = 2;
	fun_frames_.top()->called_pc = f.poped_pc; 
}

void VMachine::pop_ff2(){
	FunFrame& f = *fun_frames_.top();
	fun_frames_.downsize(1);
	fun_frames_.top()->processed = 2;
	fun_frames_.top()->called_pc = f.poped_pc; 
}

FramePtr& VMachine::push_scope(ScopeInfo* info){
	Scope& scope = scopes_.push();
	if(!scope.frame){
		scope.pos = 0;
		scope.size = 0;
		scope.flags = Scope::NONE;
		scope.frame = xnew<Frame>();
		scope.frame->orphan_ = false;
	}

	upsize_variables(info->variable_size);

	scope.frame->set_info(info);
	scope.frame->set_code(code());
	scope.pos = variables_top()-info->variable_size;
	scope.size = info->variable_size;
	scope.frame->members_.attach((AnyPtr*)variables_.data()+scope.pos, scope.size);
	scope.flags = Scope::NONE;

	return scope.frame;
}

void VMachine::pop_scope(){
	Scope& scope = scopes_.top();
	variables_top_ -= scope.frame->info()->variable_size;

	if(scope.flags==Scope::NONE){
		scope.frame->members_.detach();
	}
	else if(scope.flags==Scope::FRAME){
		scope.frame->members_.reflesh();
		scope.frame->add_ref_count_members(1);
		scope.frame->orphan_ = true;
		scope.frame = null;
	}
	else{
		scope.frame->add_ref_count_members(1);
		scope.frame->orphan_ = true;
		scope.frame = null;
	}

	scopes_.downsize(1);
}

const inst_t* VMachine::check_accessibility(const inst_t* pc, const AnyPtr& cls, const IDPtr& primary_key, const AnyPtr& secondary_key, const AnyPtr& self, int_t accessibility){
	if(accessibility & KIND_PRIVATE){
		if(rawne(self->get_class(), cls)){
			return push_except(pc, cpp_class<AccessibilityError>()->call(Xt("Xtal Runtime Error 1017")->call(
				Named(Xid(object), cls->object_name()), 
				Named(Xid(name), primary_key), 
				Named(Xid(secondary_key), secondary_key), 
				Named(Xid(accessibility), Xid(private))))
			);
		}
	}
	else if(accessibility & KIND_PROTECTED){
		if(!self->is(cls)){
			return push_except(pc, cpp_class<AccessibilityError>()->call(Xt("Xtal Runtime Error 1017")->call(
				Named(Xid(object), cls->object_name()), 
				Named(Xid(primary_key), primary_key), 
				Named(Xid(secondary_key), secondary_key), 
				Named(Xid(accessibility), Xid(protected))))
			);
		}
	}
	return 0;
}

const inst_t* VMachine::inner_send(const inst_t* npc, const IDPtr& primary_key, int_t stack_base, int_t result, int_t target){
	return send(local_variable(target), primary_key, undefined, ff().self(), 0, npc, result, 1, stack_base, 0, 0, 0);
}

const inst_t* VMachine::inner_send(const inst_t* npc, const IDPtr& primary_key, int_t stack_base, int_t result, int_t target, int_t value){
	set_local_variable(stack_base, local_variable(value));
	return send(local_variable(target), primary_key, undefined, ff().self(), 0, npc, result, 1, stack_base, 1, 0, 0);
}

const inst_t* VMachine::inner_send_q(const inst_t* npc, const IDPtr& primary_key, int_t stack_base, int_t result, int_t target, int_t value){
	set_local_variable(stack_base, local_variable(value));
	return send(local_variable(target), primary_key, undefined, ff().self(), 0, npc, result, 1, stack_base, 1, 0, MEMBER_FLAG_Q_BIT);
}

enum{
	FLAG_PROPERTY = 1<<16,
	FLAG_SET_PROPERTY = 1<<17,
};

const inst_t* VMachine::send(Any target, Any primary_key, Any secondary_key, const AnyPtr& self,
	const inst_t* pc, const inst_t* npc,
	int_t result, int_t need_result_count, 
	int_t stack_base, int_t ordered, int_t named, int_t flags){
	
	const ClassPtr& cls = target.get_class();

	int_t accessibility = 0;
	Any mem = cache_member(cls, (IDPtr&)primary_key, ap(secondary_key), accessibility);

	if(accessibility){
		if(const inst_t* epc = check_accessibility(pc, cls, (IDPtr&)primary_key, ap(secondary_key), self, accessibility)){
			return epc;
		}
	}

	if((flags&FLAG_PROPERTY) && type(mem)==TYPE_IVAR_GETTER){
		const SmartPtr<InstanceVariableGetter>& p = unchecked_ptr_cast<InstanceVariableGetter>(ap(mem));
		set_local_variable(result, target.instance_variables()->variable(p->number_, p->info_));
		return XTAL_VM_CHECK_EXCEPT_PC(npc);
	}

	if((flags&FLAG_SET_PROPERTY) && type(mem)==TYPE_IVAR_SETTER){
		const SmartPtr<InstanceVariableSetter>& p = unchecked_ptr_cast<InstanceVariableSetter>(ap(mem));
		target.instance_variables()->set_variable(p->number_, p->info_, local_variable(stack_base));
		return XTAL_VM_CHECK_EXCEPT_PC(npc);
	}

	if(is_undefined(mem)){
		if(flags&MEMBER_FLAG_Q_BIT){
			for(int_t i=0; i<need_result_count; ++i){
				set_local_variable(result+i, undefined);
			}
			return npc;
		}

		return push_except(pc, unsupported_error(cls, (IDPtr&)primary_key, ap(secondary_key)));
	}

	return call(mem, ap(target), pc, npc, result, need_result_count, stack_base, ordered, named, flags);
}

const inst_t* VMachine::call(Any mem, const AnyPtr& self, 
	const inst_t* pc, const inst_t* npc,
	int_t result, int_t need_result_count, 
	int_t stack_base, int_t ordered, int_t named, int_t flags){

	if(flags&CALL_FLAG_ARGS_BIT){
		if(ArgumentsPtr args = ptr_cast<Arguments>(local_variable(stack_base+ordered+named*2))){
			push_args(args, stack_base, ordered, named);
			ordered += args->ordered_size();
			named += args->named_size();
		}
	}

	push_ff(self, npc, result, need_result_count, stack_base, ordered, named);

	switch(type(mem)){
		XTAL_DEFAULT{
			ap(mem)->rawcall(to_smartptr(this));
		}

		XTAL_CASE(TYPE_UNDEFINED){

		}

		XTAL_CASE(TYPE_BASE){ 
			pvalue(ap(mem))->rawcall(to_smartptr(this)); 
		}

		XTAL_CASE(TYPE_NATIVE_METHOD){ 
			unchecked_ptr_cast<NativeMethod>(ap(mem))->rawcall(to_smartptr(this)); 
		}

		XTAL_CASE(TYPE_NATIVE_FUN){ 
			unchecked_ptr_cast<NativeFun>(ap(mem))->rawcall(to_smartptr(this)); 
		}

		XTAL_CASE(TYPE_IVAR_GETTER){ 
			unchecked_ptr_cast<InstanceVariableGetter>(ap(mem))->rawcall(to_smartptr(this)); 
		}

		XTAL_CASE(TYPE_IVAR_SETTER){ 
			unchecked_ptr_cast<InstanceVariableSetter>(ap(mem))->rawcall(to_smartptr(this)); 
		}
	}

	if(ff().processed==0){
		if(ap(except_[0])){ 
			return push_except(pc, ap(except_[0]));
		}

		return push_except(pc, unsupported_error(ap(mem)->get_class(), id_[IDOp::id_op_call], undefined));
	}

	return XTAL_VM_CHECK_EXCEPT_PC(ff().called_pc);
}

const FramePtr& VMachine::make_outer_outer(uint_t i){
	if(i<scopes_.size()-ff().scope_size){
		Scope& scope = scopes_[i];
		if(scope.flags==0){
			scope.flags = Scope::FRAME;
			if(!disable_debug_ || scope.frame->info()->flags&ScopeInfo::FLAG_SCOPE_CHAIN){
				scope.frame->set_outer(make_outer_outer(i+1));
			}
		}
		return scope.frame;
	}

	if(ff().fun()){
		return ff().fun()->outer();
	}

	return unchecked_ptr_cast<Frame>(null);
}

const FramePtr& VMachine::make_outer(ScopeInfo* scope){
	if(!disable_debug_ || scope->flags&ScopeInfo::FLAG_SCOPE_CHAIN){
		return make_outer_outer();
	}
	return unchecked_ptr_cast<Frame>(null);
}

void VMachine::set_local_variable_out_of_fun(uint_t pos, uint_t depth, const Any& value){
	uint_t size = scopes_.size()-ff().scope_size;
	if(depth<size){
		scopes_[depth].frame->set_member_direct_unref(pos, value);
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
		return scopes_[depth].frame->member_direct(pos);
	}

	depth -= size;

	Frame* out = outer().get();
	while(depth){
		out = out->outer().get();
		depth--;
	}

	return out->member_direct(pos);
}

#if 1
void VMachine::execute_inner(const inst_t* start){

	const inst_t* pc = start;

	CallState call_state;

	ExceptFrame cur;
	cur.info = 0;
	cur.stack_size = stack_.size() - ff().args_stack_size();
	cur.fun_frame_size = fun_frames_.size();
	cur.scope_size = scopes_.size();
	cur.variables_top = variables_top();

	hook_setting_bit_ = debug::hook_setting_bit();
	thread_yield_count_ = 1000;

	XTAL_ASSERT(cur.stack_size>=0);
	
#ifdef XTAL_USE_COMPUTED_GOTO
	static const void* labels[] = {
	//{LABELS{{
		XTAL_COPY_LABEL_ADDRESS(0, LabelNop);
		XTAL_COPY_LABEL_ADDRESS(1, LabelLoadNull);
		XTAL_COPY_LABEL_ADDRESS(2, LabelLoadUndefined);
		XTAL_COPY_LABEL_ADDRESS(3, LabelLoadTrue);
		XTAL_COPY_LABEL_ADDRESS(4, LabelLoadFalse);
		XTAL_COPY_LABEL_ADDRESS(5, LabelLoadTrueAndSkip);
		XTAL_COPY_LABEL_ADDRESS(6, LabelLoadInt1Byte);
		XTAL_COPY_LABEL_ADDRESS(7, LabelLoadFloat1Byte);
		XTAL_COPY_LABEL_ADDRESS(8, LabelLoadCallee);
		XTAL_COPY_LABEL_ADDRESS(9, LabelLoadThis);
		XTAL_COPY_LABEL_ADDRESS(10, LabelLoadCurrentContext);
		XTAL_COPY_LABEL_ADDRESS(11, LabelCopy);
		XTAL_COPY_LABEL_ADDRESS(12, LabelPush);
		XTAL_COPY_LABEL_ADDRESS(13, LabelPop);
		XTAL_COPY_LABEL_ADDRESS(14, LabelAdjustValues);
		XTAL_COPY_LABEL_ADDRESS(15, LabelLocalVariable2Byte);
		XTAL_COPY_LABEL_ADDRESS(16, LabelSetLocalVariable2Byte);
		XTAL_COPY_LABEL_ADDRESS(17, LabelInstanceVariable);
		XTAL_COPY_LABEL_ADDRESS(18, LabelSetInstanceVariable);
		XTAL_COPY_LABEL_ADDRESS(19, LabelFilelocalVariable);
		XTAL_COPY_LABEL_ADDRESS(20, LabelReturn);
		XTAL_COPY_LABEL_ADDRESS(21, LabelYield);
		XTAL_COPY_LABEL_ADDRESS(22, LabelExit);
		XTAL_COPY_LABEL_ADDRESS(23, LabelValue);
		XTAL_COPY_LABEL_ADDRESS(24, LabelCall);
		XTAL_COPY_LABEL_ADDRESS(25, LabelSend);
		XTAL_COPY_LABEL_ADDRESS(26, LabelProperty);
		XTAL_COPY_LABEL_ADDRESS(27, LabelSetProperty);
		XTAL_COPY_LABEL_ADDRESS(28, LabelMember);
		XTAL_COPY_LABEL_ADDRESS(29, LabelScopeBegin);
		XTAL_COPY_LABEL_ADDRESS(30, LabelScopeEnd);
		XTAL_COPY_LABEL_ADDRESS(31, LabelPos);
		XTAL_COPY_LABEL_ADDRESS(32, LabelNeg);
		XTAL_COPY_LABEL_ADDRESS(33, LabelCom);
		XTAL_COPY_LABEL_ADDRESS(34, LabelNot);
		XTAL_COPY_LABEL_ADDRESS(35, LabelInc);
		XTAL_COPY_LABEL_ADDRESS(36, LabelDec);
		XTAL_COPY_LABEL_ADDRESS(37, LabelArith);
		XTAL_COPY_LABEL_ADDRESS(38, LabelBitwise);
		XTAL_COPY_LABEL_ADDRESS(39, LabelAt);
		XTAL_COPY_LABEL_ADDRESS(40, LabelSetAt);
		XTAL_COPY_LABEL_ADDRESS(41, LabelGoto);
		XTAL_COPY_LABEL_ADDRESS(42, LabelIf);
		XTAL_COPY_LABEL_ADDRESS(43, LabelIfEq);
		XTAL_COPY_LABEL_ADDRESS(44, LabelIfLt);
		XTAL_COPY_LABEL_ADDRESS(45, LabelIfRawEq);
		XTAL_COPY_LABEL_ADDRESS(46, LabelIfIn);
		XTAL_COPY_LABEL_ADDRESS(47, LabelIfIs);
		XTAL_COPY_LABEL_ADDRESS(48, LabelIfUndefined);
		XTAL_COPY_LABEL_ADDRESS(49, LabelIfDebug);
		XTAL_COPY_LABEL_ADDRESS(50, LabelRange);
		XTAL_COPY_LABEL_ADDRESS(51, LabelOnce);
		XTAL_COPY_LABEL_ADDRESS(52, LabelSetOnce);
		XTAL_COPY_LABEL_ADDRESS(53, LabelMakeArray);
		XTAL_COPY_LABEL_ADDRESS(54, LabelArrayAppend);
		XTAL_COPY_LABEL_ADDRESS(55, LabelMakeMap);
		XTAL_COPY_LABEL_ADDRESS(56, LabelMapInsert);
		XTAL_COPY_LABEL_ADDRESS(57, LabelMapSetDefault);
		XTAL_COPY_LABEL_ADDRESS(58, LabelClassBegin);
		XTAL_COPY_LABEL_ADDRESS(59, LabelClassEnd);
		XTAL_COPY_LABEL_ADDRESS(60, LabelDefineClassMember);
		XTAL_COPY_LABEL_ADDRESS(61, LabelDefineMember);
		XTAL_COPY_LABEL_ADDRESS(62, LabelMakeFun);
		XTAL_COPY_LABEL_ADDRESS(63, LabelMakeInstanceVariableAccessor);
		XTAL_COPY_LABEL_ADDRESS(64, LabelTryBegin);
		XTAL_COPY_LABEL_ADDRESS(65, LabelTryEnd);
		XTAL_COPY_LABEL_ADDRESS(66, LabelPushGoto);
		XTAL_COPY_LABEL_ADDRESS(67, LabelPopGoto);
		XTAL_COPY_LABEL_ADDRESS(68, LabelThrow);
		XTAL_COPY_LABEL_ADDRESS(69, LabelAssert);
		XTAL_COPY_LABEL_ADDRESS(70, LabelBreakPoint);
		XTAL_COPY_LABEL_ADDRESS(71, LabelMAX);
//}}LABELS}
		};
#endif

XTAL_VM_LOOP

//{OPS{{
	XTAL_VM_CASE_FIRST(Nop){ // 2
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(LoadNull){ // 3
		set_local_variable(inst.result, null);
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(LoadUndefined){ // 3
		set_local_variable(inst.result, undefined);
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(LoadTrue){ // 3
		set_local_variable(inst.result, Any(true));
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(LoadFalse){ // 3
		set_local_variable(inst.result, Any(false));
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(LoadTrueAndSkip){ // 3
		set_local_variable(inst.result, Any(true));
		XTAL_VM_CONTINUE(pc + inst.ISIZE + InstLoadFalse::ISIZE); 
	}

	XTAL_VM_CASE(LoadInt1Byte){ // 3
		set_local_variable(inst.result, Any((int_t)inst.value));
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(LoadFloat1Byte){ // 3
		set_local_variable(inst.result, Any((float_t)inst.value));
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(LoadCallee){ // 3
		set_local_variable(inst.result, fun());
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(LoadThis){ // 3
		set_local_variable(inst.result, ff().self());
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(LoadCurrentContext){ // 3
		set_local_variable(inst.result, make_outer_outer());
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}

	XTAL_VM_CASE(Copy){ // 3
		set_local_variable(inst.result, local_variable(inst.target));
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(Push){ // 3
		stack_.push(local_variable(inst.target));
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(Pop){ // 3
		set_local_variable(inst.result, stack_.pop());
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(AdjustValues){ // 3
		adjust_values2(inst.stack_base, inst.result_count, inst.need_result_count);
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(LocalVariable2Byte){ // 3
		set_local_variable(inst.result, local_variable_out_of_fun(inst.number, inst.depth)); 
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(SetLocalVariable2Byte){ // 3
		set_local_variable_out_of_fun(inst.number, inst.depth, local_variable(inst.target)); 
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(InstanceVariable){ // 3
		set_local_variable(inst.result, ff().instance_variables->variable(inst.number, code()->class_info(inst.info_number)));
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(SetInstanceVariable){ // 3
		ff().instance_variables->set_variable(inst.number, code()->class_info(inst.info_number), local_variable(inst.value));
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}

	XTAL_VM_CASE(FilelocalVariable){ // 5
		const AnyPtr& ret = code()->filelocal()->member(identifier(inst.identifier_number));
		if(rawne(ret, undefined)){
			set_local_variable(inst.result, ret);
			XTAL_VM_CONTINUE(pc + inst.ISIZE);
		}
		else{
			XTAL_VM_THROW_EXCEPT(filelocal_unsupported_error(code(), identifier(inst.identifier_number)));
		}
	}

	XTAL_VM_CASE(Return){ // 7
		check_debug_hook(pc, BREAKPOINT_RETURN);

		FunFrame& f = ff();
		f.result_count += inst.result_count;
		pop_scope();
		pop_ff();
		XTAL_VM_CONTINUE(ff().called_pc);
	}

	XTAL_VM_CASE(Yield){ // 7
		yield_target_count_ = inst.target_count;	
		yield_result_ = inst.result;
		yield_need_result_count_ = inst.need_result_count;

		if(ff().yieldable){
			resume_pc_ = pc + inst.ISIZE;
			XTAL_VM_RETURN;
		}
		else{
			XTAL_VM_THROW_EXCEPT(cpp_class<YieldError>()->call(Xt("Xtal Runtime Error 1012")));
		}
	}

	XTAL_VM_CASE(Exit){ // 3
		resume_pc_ = 0; 
		XTAL_VM_RETURN;
	}

	XTAL_VM_CASE(Value){ // 3
		set_local_variable(inst.result, code()->value(inst.value_number)); 
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(Call){ // 2
		XTAL_VM_CONTINUE(call(local_variable(inst.target), (inst.flags&CALL_FLAG_THIS) ? local_variable(inst.self) : ff().self(),
			pc, pc + inst.ISIZE,
			inst.result, inst.need_result, inst.stack_base, inst.ordered, inst.named, inst.flags
		));
	}

	XTAL_VM_CASE(Send){ // 3
		int_t flags = inst.flags;
		XTAL_VM_CONTINUE(send(local_variable(inst.target), 
			(flags&MEMBER_FLAG_P_BIT) ? (IDPtr&)local_variable(inst.primary) : identifier(inst.primary),
			(flags&MEMBER_FLAG_S_BIT) ? local_variable(inst.secondary) : undefined, ff().self(),
			pc, pc + inst.ISIZE,
			inst.result, inst.need_result, inst.stack_base, inst.ordered, inst.named, flags
		));
	}

	XTAL_VM_CASE(Property){ // 2
		XTAL_VM_CONTINUE(send(local_variable(inst.target), identifier(inst.primary), undefined, ff().self(),
			pc, pc + inst.ISIZE,
			inst.result, 1, inst.stack_base, 0, 0, FLAG_PROPERTY
		));
	}

	XTAL_VM_CASE(SetProperty){ // 2
		XTAL_VM_CONTINUE(send(local_variable(inst.target), identifier(inst.primary), undefined, ff().self(),
			pc, pc + inst.ISIZE,
			inst.stack_base, 0, inst.stack_base, 1, 0, FLAG_SET_PROPERTY
		));
	}

	XTAL_VM_CASE(Member){ // 13
		XTAL_VM_FUN;
		Any primary = (inst.flags&MEMBER_FLAG_P_BIT) ? (const IDPtr&)local_variable(inst.primary) : identifier(inst.primary);
		Any secondary = (inst.flags&MEMBER_FLAG_S_BIT) ? local_variable(inst.secondary) : undefined;
		const AnyPtr& cls = local_variable(inst.target);

		int_t accessibility = 0;
		Any mem = cache_member(cls, (IDPtr&)primary, ap(secondary), accessibility);

		if(accessibility){
			if(const inst_t* epc = check_accessibility(pc, cls, (IDPtr&)primary, ap(secondary), ff().self(), accessibility)){
				XTAL_VM_CONTINUE(epc);
			}	
		}

		if(is_undefined(mem)){
			if(inst.flags&MEMBER_FLAG_Q_BIT){
				set_local_variable(inst.result, undefined);
			}
			else{
				XTAL_VM_CHECK_EXCEPT;
				XTAL_VM_THROW_EXCEPT(unsupported_error(cls, (IDPtr&)primary, ap(secondary)));
			}
		}
		else{
			set_local_variable(inst.result, ap(mem));
		}

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

	XTAL_VM_CASE(Pos){ // 6
		const AnyPtr& a = local_variable(inst.target); uint_t atype = type(a)-TYPE_INT;
		if(atype<2){
			set_local_variable(inst.result, a);
			XTAL_VM_CONTINUE(pc+inst.ISIZE);
		}
			
		XTAL_VM_CONTINUE(inner_send(pc+inst.ISIZE, id_[IDOp::id_op_pos], inst.stack_base, inst.result, inst.target));
	}

	XTAL_VM_CASE(Neg){ // 8
		const AnyPtr& a = local_variable(inst.target); uint_t atype = type(a)-TYPE_INT;
		if(atype==0){
			set_ivalue(local_variable(inst.result), -ivalue(a));
			XTAL_VM_CONTINUE(pc+inst.ISIZE);
		}

		if(atype==1){
			set_fvalue(local_variable(inst.result), -fvalue(a));
			XTAL_VM_CONTINUE(pc+inst.ISIZE);
		}
	
		XTAL_VM_CONTINUE(inner_send(pc+inst.ISIZE, id_[IDOp::id_op_neg], inst.stack_base, inst.result, inst.target));
	}

	XTAL_VM_CASE(Com){ // 6
		const AnyPtr& a = local_variable(inst.target); uint_t atype = type(a)-TYPE_INT;
		if(atype==0){
			set_ivalue(local_variable(inst.result), ~ivalue(a));
			XTAL_VM_CONTINUE(pc+inst.ISIZE);
		}

		XTAL_VM_CONTINUE(inner_send(pc+inst.ISIZE, id_[IDOp::id_op_com], inst.stack_base, inst.result, inst.target));
	}

	XTAL_VM_CASE(Not){ // 3
		set_local_variable(inst.result, Any(!local_variable(inst.target))); 
		XTAL_VM_CONTINUE(pc+inst.ISIZE); 
	}

	XTAL_VM_CASE(Inc){ // 8
		AnyPtr& a = local_variable(inst.target);
		uint_t atype = type(a)-TYPE_INT;
		if(atype==0){
			set_ivalue(local_variable(inst.result), ivalue(a)+1);
			XTAL_VM_CONTINUE(pc + inst.ISIZE); 
		}
		
		if(atype==1){
			set_fvalue(local_variable(inst.result), fvalue(a)+1);
			XTAL_VM_CONTINUE(pc + inst.ISIZE); 
		}

		XTAL_VM_CONTINUE(inner_send(pc+inst.ISIZE, id_[IDOp::id_op_inc], inst.stack_base, inst.result, inst.target));
	}

	XTAL_VM_CASE(Dec){ // 8
		AnyPtr& a = local_variable(inst.target);
		uint_t atype = type(a)-TYPE_INT;
		if(atype==0){
			set_ivalue(local_variable(inst.result), ivalue(a)-1);
			XTAL_VM_CONTINUE(pc + inst.ISIZE); 
		}
		
		if(atype==1){
			set_fvalue(local_variable(inst.result), fvalue(a)-1);
			XTAL_VM_CONTINUE(pc + inst.ISIZE); 
		}

		XTAL_VM_CONTINUE(inner_send(pc+inst.ISIZE, id_[IDOp::id_op_dec], inst.stack_base, inst.result, inst.target));
	}

	XTAL_VM_CASE(Arith){ // 49
		XTAL_VM_FUN;
		AnyPtr& a = local_variable(inst.lhs); uint_t atype = rawtype(a)-TYPE_INT;
		AnyPtr& b = local_variable(inst.rhs); uint_t btype = rawtype(b)-TYPE_INT;
		uint_t kind = inst.kind&0x7;
				
		int_t ia, ib, iret;
		float_t fa, fb, fret;

		if((atype|btype)>1){
			goto send;
		}

		switch(atype | btype<<1){
			XTAL_NODEFAULT;
			XTAL_CASE(0 | 0<<1){ ia = ivalue(a); ib = ivalue(b); goto icalc; } 
			XTAL_CASE(0| 1<<1){ fa = (float_t)ivalue(a); fb = fvalue(b); goto fcalc; }
			XTAL_CASE(1 | 0<<1){ fa = fvalue(a); fb = (float_t)ivalue(b);  goto fcalc; }
			XTAL_CASE(1 | 1<<1){ fa = fvalue(a); fb = fvalue(b); goto fcalc; }
		}

icalc:
		switch(kind){
			XTAL_NODEFAULT;
			XTAL_CASE(ARITH_ADD){ iret = ia + ib; } 
			XTAL_CASE(ARITH_SUB){ iret = ia - ib; }
			XTAL_CASE(ARITH_CAT){ goto send; }
			XTAL_CASE(ARITH_MUL){ iret = ia * ib; }
			XTAL_CASE(ARITH_DIV){ if(ib==0) goto zerodiv; iret = ia / ib; }
			XTAL_CASE(ARITH_MOD){ if(ib==0) goto zerodiv; iret = ia % ib; }
		}
		set_ivalue(local_variable(inst.result), iret);
		XTAL_VM_CONTINUE(pc + inst.ISIZE);

fcalc:
		using namespace std;
		switch(kind){
			XTAL_NODEFAULT;
			XTAL_CASE(ARITH_ADD){ fret = fa + fb; } 
			XTAL_CASE(ARITH_SUB){ fret = fa - fb; }
			XTAL_CASE(ARITH_CAT){ goto send; }
			XTAL_CASE(ARITH_MUL){ fret = fa * fb; }
			XTAL_CASE(ARITH_DIV){ if(fb==0) goto zerodiv; fret = fa / fb; }
			XTAL_CASE(ARITH_MOD){ if(fb==0) goto zerodiv; fret = fmodf(fa, fb); }
		}
		set_fvalue(local_variable(inst.result), fret);
		XTAL_VM_CONTINUE(pc + inst.ISIZE);

send:
		XTAL_VM_CONTINUE(inner_send(pc + inst.ISIZE, id_[inst.kind>=ARITH_ADD_ASSIGN ? IDOp::id_op_add_assign+kind : IDOp::id_op_add+kind], inst.stack_base, inst.result, inst.lhs, inst.rhs));

zerodiv:
		stack_.push(cpp_class<RuntimeError>()->call(Xt("Xtal Runtime Error 1024")));
		XTAL_VM_CONTINUE(&throw_code_);
	}

	XTAL_VM_CASE(Bitwise){ // 18
		XTAL_VM_FUN;
		AnyPtr& a = local_variable(inst.lhs);
		AnyPtr& b = local_variable(inst.rhs);
		uint_t kind = inst.kind&0x7;

		if(rawtype(a)==TYPE_INT && rawtype(b)==TYPE_INT){
			int_t ia, ib, iret;
			ia = ivalue(a); ib = ivalue(b);
			switch(kind){
				XTAL_NODEFAULT;
				XTAL_CASE(BITWISE_AND){ iret = ia & ib; }
				XTAL_CASE(BITWISE_OR){ iret = ia | ib; }
				XTAL_CASE(BITWISE_XOR){ iret = ia ^ ib; }
				XTAL_CASE(BITWISE_SHL){ iret = ia << ib; }
				XTAL_CASE(BITWISE_SHR){ iret = ia >> ib; }
				XTAL_CASE(BITWISE_USHR){ iret = (uint_t)ia >> ib; }
			}

			set_ivalue(local_variable(inst.result), iret);
			XTAL_VM_CONTINUE(pc + inst.ISIZE);
		}

		XTAL_VM_CONTINUE(inner_send(pc + inst.ISIZE, id_[inst.kind>=BITWISE_AND_ASSIGN ? IDOp::id_op_and_assign+kind : IDOp::id_op_and+kind], inst.stack_base, inst.result, inst.lhs, inst.rhs));
	}

	XTAL_VM_CASE(At){ // 15
		XTAL_VM_FUN;
		AnyPtr& b = local_variable(inst.index); uint_t btype = type(b);
		AnyPtr& a = local_variable(inst.target); uint_t atype = type(a);

		if(atype==TYPE_ARRAY && btype==TYPE_INT){
			Any ret = unchecked_ptr_cast<Array>(a)->op_at(ivalue(b));
			XTAL_VM_CHECK_EXCEPT;
			set_local_variable(inst.result, ret);
			XTAL_VM_CONTINUE(pc+inst.ISIZE); 
		}
		else if(raweq(a->get_class(), cpp_class<Map>())){
			Any ret = unchecked_ptr_cast<Map>(a)->op_at(b);
			XTAL_VM_CHECK_EXCEPT;
			set_local_variable(inst.result, ret);
			XTAL_VM_CONTINUE(pc+inst.ISIZE); 
		}
		else{
			XTAL_VM_CONTINUE(inner_send(pc+inst.ISIZE, id_[IDOp::id_op_at], inst.stack_base, inst.result, inst.target, inst.index));
		}
	}

	XTAL_VM_CASE(SetAt){ // 16
		XTAL_VM_FUN;
		AnyPtr& b = local_variable(inst.index); uint_t btype = type(b);
		AnyPtr& a = local_variable(inst.target); uint_t atype = type(a);
		AnyPtr& c = local_variable(inst.value);

		if(atype==TYPE_ARRAY && btype==TYPE_INT){
			unchecked_ptr_cast<Array>(a)->op_set_at(ivalue(b), c);
			XTAL_VM_CHECK_EXCEPT;
			XTAL_VM_CONTINUE(pc+inst.ISIZE); 
		}
		else if(raweq(a->get_class(), cpp_class<Map>())){
			unchecked_ptr_cast<Map>(a)->op_set_at(b, c);
			XTAL_VM_CHECK_EXCEPT;
			XTAL_VM_CONTINUE(pc+inst.ISIZE); 
		}
		else{
			set_local_variable(inst.stack_base+0, b);
			set_local_variable(inst.stack_base+1, c);
			XTAL_VM_CONTINUE(send(a, id_[IDOp::id_op_set_at], undefined, ff().self(),
				pc, pc+inst.ISIZE,
				inst.stack_base, 0, inst.stack_base, 2, 0, 0));
		}
	}

	XTAL_VM_CASE(Goto){ // 3
		XTAL_CHECK_YIELD;
		XTAL_VM_CONTINUE(pc + inst.address); 
	}

	XTAL_VM_CASE(If){ // 2
		XTAL_VM_CONTINUE(pc + (local_variable(inst.target) ? inst.address_true : inst.address_false));
	}

	XTAL_VM_CASE(IfEq){ // 14
		XTAL_VM_FUN;
		const AnyPtr& b = local_variable(inst.rhs); uint_t btype = type(b)-TYPE_INT;
		const AnyPtr& a = local_variable(inst.lhs); uint_t atype = type(a)-TYPE_INT;
		uint_t abtype = atype | btype;
		
		typedef InstIf InstType2; 
		InstType2& inst2 = *(InstType2*)(pc+inst.ISIZE);

		if(abtype==0){
			XTAL_VM_CONTINUE((
				ivalue(a) == ivalue(b) ? 
				inst2.address_true : 
				inst2.address_false) + pc + inst.ISIZE);
		}

		if(abtype==1){
			f2 ab;
			to_f2(ab, atype, a, btype, b);
			XTAL_VM_CONTINUE((
				ab.a == ab.b ? 
				inst2.address_true : 
				inst2.address_false) + pc + inst.ISIZE);
		}

		XTAL_VM_CONTINUE(inner_send_q(pc+inst.ISIZE, id_[IDOp::id_op_eq], inst.stack_base, inst2.target, inst.lhs, inst.rhs));
	}

	XTAL_VM_CASE(IfLt){ // 14
		XTAL_VM_FUN;
		const AnyPtr& b = local_variable(inst.rhs); uint_t btype = type(b)-TYPE_INT;
		const AnyPtr& a = local_variable(inst.lhs); uint_t atype = type(a)-TYPE_INT;
		uint_t abtype = atype | btype;

		typedef InstIf InstType2; 
		InstType2& inst2 = *(InstType2*)(pc+inst.ISIZE);

		if(abtype==0){
			XTAL_VM_CONTINUE((
				ivalue(a) < ivalue(b) ? 
				inst2.address_true : 
				inst2.address_false) + pc + inst.ISIZE);
		}

		if(abtype==1){
			f2 ab;
			to_f2(ab, atype, a, btype, b);
			XTAL_VM_CONTINUE(
				(ab.a < ab.b ? 
				inst2.address_true : 
				inst2.address_false) + pc + inst.ISIZE);
		}
		
		XTAL_VM_CONTINUE(inner_send_q(pc+inst.ISIZE, id_[IDOp::id_op_lt], inst.stack_base, inst2.target, inst.lhs, inst.rhs));
	}

	XTAL_VM_CASE(IfRawEq){ // 7
		typedef InstIf InstType2; 
		InstType2& inst2 = *(InstType2*)(pc+inst.ISIZE);

		const AnyPtr& b = local_variable(inst.rhs);
		const AnyPtr& a = local_variable(inst.lhs);

		pc = (raweq(a, b) ? 
			(int)(inst2.address_true) : 
			(int)(inst2.address_false)) + pc + inst.ISIZE;

		XTAL_VM_CONTINUE(pc);
	}

	XTAL_VM_CASE(IfIn){ // 4
		typedef InstIf InstType2; 
		InstType2& inst2 = *(InstType2*)(pc+inst.ISIZE);
		XTAL_VM_CONTINUE(inner_send_q(pc+inst.ISIZE, id_[IDOp::id_op_in], inst.stack_base, inst2.target, inst.lhs, inst.rhs));
	}

	XTAL_VM_CASE(IfIs){ // 7
		typedef InstIf InstType2; 
		InstType2& inst2 = *(InstType2*)(pc+inst.ISIZE);

		const AnyPtr& a = local_variable(inst.lhs);
		const AnyPtr& b = local_variable(inst.rhs);

		pc = (a->is(b) ? 
			(int)(inst2.address_true) : 
			(int)(inst2.address_false)) + pc + inst.ISIZE;
		XTAL_VM_CONTINUE(pc);
	}

	XTAL_VM_CASE(IfUndefined){ // 2
		XTAL_VM_CONTINUE(pc + (raweq(local_variable(inst.target), undefined) ? inst.address_true : inst.address_false));
	}

	XTAL_VM_CASE(IfDebug){ // 2
		XTAL_VM_CONTINUE(pc + (debug::is_enabled() ? inst.ISIZE : inst.address));
	}

	XTAL_VM_CASE(Range){ // 7
		XTAL_VM_FUN;
		AnyPtr& b = local_variable(inst.rhs);
		AnyPtr& a = local_variable(inst.lhs);

		set_local_variable(inst.stack_base+0, b);
		set_local_variable(inst.stack_base+1, inst.kind);

		XTAL_VM_CONTINUE(send(a, id_[IDOp::id_op_range], undefined, ff().self(),
			pc, pc+inst.ISIZE,
			inst.result, 1, inst.stack_base, 2, 0, 0));
	}

	XTAL_VM_CASE(Once){ // 5
		const AnyPtr& ret = code()->once_value(inst.value_number);
		if(rawne(ret, undefined)){
			set_local_variable(inst.result, ret);
			XTAL_VM_CONTINUE(pc + inst.address);
		}
		else{
			XTAL_VM_CONTINUE(pc + inst.ISIZE);
		}
	}

	XTAL_VM_CASE(SetOnce){ // 3
		code()->set_once_value(inst.value_number, local_variable(inst.target)); 
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(MakeArray){ // 3
		set_local_variable(inst.result, xnew<Array>());
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}

	XTAL_VM_CASE(ArrayAppend){ // 3
		ptr_cast<Array>(local_variable(inst.target))->push_back(local_variable(inst.value)); 
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}

	XTAL_VM_CASE(MakeMap){ // 3
		set_local_variable(inst.result, xnew<Map>());
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}

	XTAL_VM_CASE(MapInsert){ // 3
		ptr_cast<Map>(local_variable(inst.target))->set_at(local_variable(inst.key), local_variable(inst.value)); 
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}

	XTAL_VM_CASE(MapSetDefault){ // 3
		ptr_cast<Map>(local_variable(inst.target))->set_default_value(local_variable(inst.value)); 
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}

	XTAL_VM_CASE(ClassBegin){ // 22
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
			AnyPtr popped = local_variable(inst.mixin_base+i);
			if(const ClassPtr& cls = ptr_cast<Class>(popped)){
				cp->inherit_first(cls);
			}
			else{
				XTAL_VM_THROW_EXCEPT(cpp_class<RuntimeError>()->call());
			}
		}

		push_ff(cp, pc + inst.ISIZE, 0, 0, 0, 0, 0);

		Scope& scope = scopes_.push();
		scope.frame = cp;
		scope.pos = 0;
		scope.size = 0;
		scope.flags = Scope::CLASS;
		cp->orphan_ = false;

		ff().set_fun(prev_fun());
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}

	XTAL_VM_CASE(ClassEnd){ // 12
		XTAL_VM_FUN;
		Scope& scope = scopes_.top();
		if(raweq(scope.frame->get_class(), scope.frame)){
			Class* singleton = (Class*)pvalue(scope.frame);
			singleton->init_singleton(to_smartptr(this));
		}

		set_local_variable(inst.result, scope.frame);
		scope.frame->add_ref_count_members(1);
		scope.frame->orphan_ = true;
		scope.frame = null;
		scopes_.downsize(1);

		fun_frames_.downsize(1);
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}

	XTAL_VM_CASE(DefineClassMember){ // 4
		XTAL_VM_FUN;
		if(const ClassPtr& p = ptr_cast<Class>(scopes_.top().frame)){
			p->set_member_direct(inst.number, identifier(inst.primary), local_variable(inst.value), local_variable(inst.secondary), inst.accessibility);
		}
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}

	XTAL_VM_CASE(DefineMember){ // 9
		XTAL_VM_FUN;
		IDPtr primary = (inst.flags&MEMBER_FLAG_P_BIT) ? (const IDPtr&)local_variable(inst.primary) : identifier(inst.primary);
		AnyPtr secondary = (inst.flags&MEMBER_FLAG_S_BIT) ? local_variable(inst.secondary) : undefined;
		AnyPtr cls = local_variable(inst.target);
		AnyPtr value = local_variable(inst.value);
		cls->def(primary, ap(value), secondary, KIND_PUBLIC);
		XTAL_VM_CHECK_EXCEPT;
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(MakeFun){ // 11
		XTAL_VM_FUN;
		int_t table_n = inst.info_number, end = inst.address;
		FunInfo* info = code()->fun_info(table_n);
		const FramePtr& outer = make_outer(info);
		switch(info->kind){
			XTAL_NODEFAULT;
			XTAL_CASE(KIND_FUN){ set_local_variable(inst.result, xnew<Fun>(outer, ff().self(), code(), info)); }
			XTAL_CASE(KIND_LAMBDA){ set_local_variable(inst.result, xnew<Lambda>(outer, ff().self(), code(), info)); }
			XTAL_CASE(KIND_METHOD){ set_local_variable(inst.result, xnew<Method>(outer, code(), info)); }
			XTAL_CASE(KIND_FIBER){ set_local_variable(inst.result, xnew<Fiber>(outer, ff().self(), code(), info)); }
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
		set_local_variable(inst.result, ret);
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(TryBegin){ // 10
		XTAL_VM_FUN;
		FunFrame& f = ff(); 
		ExceptFrame& ef = except_frames_.push();
		ef.info = code()->except_info(inst.info_number);
		ef.fun_frame_size = fun_frames_.size();
		ef.stack_size = stack_.size();
		ef.scope_size = scopes_.size();
		ef.variables_top = variables_top();
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}

	XTAL_VM_CASE(TryEnd){ // 2
		XTAL_VM_CONTINUE(except_frames_.pop().info->finally_pc + code()->data()); 
	}

	XTAL_VM_CASE(PushGoto){ // 3
		stack_.push(Any((int_t)((pc+inst.address)-code()->data()))); 
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(PopGoto){ // 2
		XTAL_VM_CONTINUE(code()->data()+ivalue(ap(stack_.pop())));
	}

	XTAL_VM_CASE(Throw){ // 12
		AnyPtr except = ap(stack_.pop());
		if(!except){
			except = ap(except_[0]);
		}

		if(pc!=&throw_code_){
			throw_pc_ = pc;
		}

		if(!except->is(cpp_class<Exception>())){
			except = cpp_class<RuntimeError>()->call(except);
		}

		except_[0] = except; 
		check_debug_hook(pc==&throw_code_ ? throw_pc_ : pc, BREAKPOINT_THROW); 

		// 例外にバックトレースを追加する
		AnyPtr e = catch_except();
		set_except(append_backtrace(throw_pc_, ap(e)));

		// Xtalソース内でキャッチ等あるか調べる
		pc = catch_body(throw_pc_, cur);

		// Xtalソース内でキャッチ等されたなら最初に戻る
		if(pc){
			XTAL_VM_CONTINUE(pc);
		}
		
		// されてないならループ脱出
		return;
	}

	XTAL_VM_CASE(Assert){ // 6
		XTAL_VM_FUN;
		assertion_message_ = ptr_cast<String>(local_variable(inst.message));
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

XTAL_VM_LOOP_END

}

#endif

#undef XTAL_VM_CONTINUE
#undef XTAL_VM_CONTINUE0
#undef XTAL_VM_THROW_EXCEPT
#undef XTAL_VM_CHECK_EXCEPT

#define XTAL_VM_CONTINUE(x) return (x)
#define XTAL_VM_CONTINUE0 return pc
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
#undef XTAL_VM_CONTINUE0
#undef XTAL_VM_RETURN

#undef XTAL_VM_EXCEPT
#undef XTAL_VM_CHECK_EXCEPT_PC
#undef XTAL_VM_THROW_EXCEPT
#undef XTAL_VM_CHECK_EXCEPT

#undef XTAL_CHECK_YIELD

//#undef inst

}//namespace

