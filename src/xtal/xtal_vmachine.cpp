#include "xtal.h"
#include "xtal_macro.h"

namespace xtal{

inline const IDPtr& isp(const Any& v){
	return (const IDPtr&)v;
}

template<class T>
inline T check_zero(T value){
	if(value==0){
		XTAL_THROW(RuntimeError()->call(Xt("Xtal Runtime Error 1024")), return 1);
	}
	return value;
}

inline const CodePtr& VMachine::code(){ return fun()->code(); }
inline const CodePtr& VMachine::prev_code(){ return prev_fun()->code(); }

inline const IDPtr& VMachine::identifier(int_t n){ return code()->identifier(n); }
inline const IDPtr& VMachine::prev_identifier(int_t n){ return prev_code()->identifier(n); }
inline const IDPtr& VMachine::identifier_or_pop(int_t n){ 
	if(n!=0){ return  unchecked_ptr_cast<ID>(ap(identifier(n)));  }
	else{ return unchecked_ptr_cast<ID>(ap(pop()->to_s()->intern())); }
}

void VMachine::recycle_call(){
	FunFrame& f = ff();
	downsize(f.ordered_arg_count+f.named_arg_count*2);
	f.ordered_arg_count = 0;
	f.named_arg_count = 0;
	f.called_pc = &throw_unsupported_error_code_;
}

void VMachine::recycle_call(const AnyPtr& a1){
	recycle_call();
	push_arg(a1);
}

void VMachine::execute(Fun* fun, const inst_t* start_pc){
	setup_call(0);
	carry_over(fun);
	const inst_t* temp;

	{
		FunFrame& f = ff();

		temp = f.poped_pc;
		f.poped_pc = &end_code_;

		execute_inner(start_pc ? start_pc : f.called_pc);
	}

	fun_frames_.upsize(1);

	{
		FunFrame& f = ff();

		f.poped_pc = temp;
		f.called_pc = &cleanup_call_code_;
	}

}

void VMachine::present_for_vm(Fiber* fun, VMachine* vm, bool add_succ_or_fail_result){
	// 結果をvmに渡す
	if(vm->need_result()){
		if(add_succ_or_fail_result){
			if(resume_pc_!=0){
				vm->push(from_this(fun));
			}
			else{
				vm->push(null);
			}
			vm->push(this, yield_result_count_);
			downsize(yield_result_count_);
			vm->adjust_result(yield_result_count_+1);
		}
		else{
			vm->push(this, yield_result_count_);
			downsize(yield_result_count_);
			vm->adjust_result(yield_result_count_);
		}
	}
}

const inst_t* VMachine::start_fiber(Fiber* fun, VMachine* vm, bool add_succ_or_fail_result){
	yield_result_count_ = 0;
	push_ff(&end_code_, vm->need_result_count(), vm->ordered_arg_count(), vm->named_arg_count(), vm->get_arg_this());
	move(vm, vm->ordered_arg_count()+vm->named_arg_count()*2);
	resume_pc_ = 0;
	carry_over(fun);
	ff().yieldable = true;
	execute_inner(ff().called_pc);
	present_for_vm(fun, vm, add_succ_or_fail_result);
	vm->ff().called_pc = &cleanup_call_code_;
	return resume_pc_;
}

const inst_t* VMachine::resume_fiber(Fiber* fun, const inst_t* pc, VMachine* vm, bool add_succ_or_fail_result){
	yield_result_count_ = 0;
	ff().called_pc = pc;
	resume_pc_ = 0;
	move(vm, vm->ordered_arg_count()+vm->named_arg_count()*2);
	execute_inner(ff().called_pc);
	present_for_vm(fun, vm, add_succ_or_fail_result);
	vm->ff().called_pc = &cleanup_call_code_;
	return resume_pc_;
}

void VMachine::exit_fiber(){
	XTAL_TRY{
		yield_result_count_ = 0;
		ff().called_pc = resume_pc_;
		resume_pc_ = 0;
		execute_inner(&throw_undefined_code_);
	}
	XTAL_CATCH(e){
		(void)e;
	}
	reset();
}

void VMachine::reset(){
	stack_.resize(0);
	except_frames_.resize(0);
	fun_frames_.resize(0);

	except_[0] = null;
	except_[1] = null;
	except_[2] = null;
	debug_info_ = null;
	debug_ = null;
}

void VMachine::recycle_ff(const inst_t* pc, int_t ordered_arg_count, int_t named_arg_count, const AnyPtr& self){
	FunFrame& f = ff();
	f.ordered_arg_count = ordered_arg_count;
	f.named_arg_count = named_arg_count;
	f.self(self);
	//f.poped_pc = pc;
	f.called_pc = &throw_unsupported_error_code_;
}

void VMachine::push_ff(const inst_t* pc, int_t need_result_count, int_t ordered_arg_count, int_t named_arg_count, const AnyPtr& self){
	FunFrame& f = fun_frames_.push();
	f.need_result_count = need_result_count;
	f.ordered_arg_count = ordered_arg_count;
	f.named_arg_count = named_arg_count;
	f.called_pc = &throw_unsupported_error_code_;
	f.poped_pc = pc;
	f.instance_variables = &empty_instance_variables;
	f.self(self);
	f.set_null();
}

void VMachine::push_ff(const inst_t* pc, const InstCall& inst, const AnyPtr& self){
	if(inst.flags&CALL_FLAG_ARGS){
		ArgumentsPtr args = ptr_cast<Arguments>(pop());
		push_args(args, inst.named);
		if(inst.flags&CALL_FLAG_TAIL){
			recycle_ff(pc, args->ordered_->size()+inst.ordered, args->named_->size()+inst.named, self);
		}
		else{
			push_ff(pc, inst.need_result, args->ordered_->size()+inst.ordered, args->named_->size()+inst.named, self);
		}
		return;
	}

	if(inst.flags&CALL_FLAG_TAIL){
		FunFrame& f = ff();
		f.ordered_arg_count = inst.ordered;
		f.named_arg_count = inst.named;
		f.self(self);
		//f.poped_pc = pc;
		f.called_pc = &throw_unsupported_error_code_;
	}
	else{
		FunFrame& f = fun_frames_.push();
		f.need_result_count = inst.need_result;
		f.ordered_arg_count = inst.ordered;
		f.named_arg_count = inst.named;
		f.called_pc = &throw_unsupported_error_code_;
		f.poped_pc = pc;
		f.instance_variables = &empty_instance_variables;
		f.self(self);
		f.set_null();
	}
}

void VMachine::push_args(const ArgumentsPtr& args, int_t named_arg_count){
	if(!named_arg_count){
		for(uint_t i = 0; i<args->ordered_->size(); ++i){
			push(args->ordered_->at(i));
		}
	}
	else{
		int_t usize = args->ordered_->size();
		upsize(usize);
		int_t offset = named_arg_count*2;
		for(int_t i = 0; i<offset; ++i){
			set(i+usize, get(i));
		}

		for(int_t i = 0; i<usize; ++i){
			set(offset-1-i, args->ordered_->at(i));
		}
	}

	args->named_->push_all(myself());
}

void VMachine::return_result_instance_variable(int_t number, ClassInfo* core){
	return_result((ff().instance_variables->variable(number, core)));
}

void VMachine::adjust_arg(int_t n){
	FunFrame& f = ff();
	stack_.downsize(f.named_arg_count*2);
	adjust_result(f.ordered_arg_count, n);
	f.named_arg_count = 0;
	f.ordered_arg_count = n;
}

void VMachine::flatten_arg(){
	FunFrame& f = ff();
	adjust_arg(1);
	if(MultiValuePtr mv = arg(0)->flatten_mv()){
		downsize(1);
		for(uint_t i=0; i<mv->size(); ++i){
			push(mv->at(i));
		}
		f.ordered_arg_count = mv->size();
	}
	else{
		downsize(1);
		f.ordered_arg_count = 0;
	}
}

void VMachine::flatten_all_arg(){
	FunFrame& f = ff();
	adjust_arg(1);
	if(MultiValuePtr mv = arg(0)->flatten_all_mv()){
		downsize(1);
		for(uint_t i=0; i<mv->size(); ++i){
			push(mv->at(i));
		}
		f.ordered_arg_count = mv->size();
	}
	else{
		downsize(1);
		f.ordered_arg_count = 0;
	}
}

void VMachine::set_local_variable(int_t pos, const Any& value){
	pos -= ff().variables_.size();
	Frame* outer = ff().outer().get();
	XTAL_GLOBAL_INTERPRETER_LOCK{
		while(1){
			int_t variables_size = outer->block_size();
			if(pos<variables_size){
				outer->set_member_direct(variables_size-1-pos, ap(value));
				return;
			}
			pos -= variables_size;
			outer = outer->outer().get();
		}
	}
}

const AnyPtr& VMachine::local_variable(int_t pos){
	pos -= ff().variables_.size();
	Frame* outer = ff().outer().get();
	XTAL_GLOBAL_INTERPRETER_LOCK{
		for(;;){
			int_t variables_size = outer->block_size();
			if(pos<variables_size){
				return outer->member_direct(variables_size-1-pos);
			}
			pos -= variables_size;
			outer = outer->outer().get();
		}
	}
	return undefined;
}

#define XTAL_VM_NODEFAULT } XTAL_ASSERT(false); XTAL_NODEFAULT
#define XTAL_VM_CASE_FIRST(key) case Inst##key::NUMBER: { Inst##key& inst = *(Inst##key*)pc;
#define XTAL_VM_CASE(key) } XTAL_ASSERT(false); case Inst##key::NUMBER: /*printf("%s\n", #key);*/ { typedef Inst##key Inst; Inst& inst = *(Inst*)pc;
#define XTAL_VM_SWITCH switch(*pc)
#define XTAL_VM_DEF_INST(key) typedef Inst##key Inst; Inst& inst = *(Inst*)pc
#define XTAL_VM_CONTINUE(x) do{ pc = (x); goto begin; }while(0)
#define XTAL_VM_EXCEPT(e) do{ set_except_0(e); if(debug_enable_ && debug_->is_enabled()){ debug_hook(pc, BREAKPOINT_THROW); } goto except_catch; }while(0)
#define XTAL_VM_RETURN return

#ifdef XTAL_NO_EXCEPTIONS
#	define XTAL_VM_CHECK_EXCEPT_PC(pc) (ap(except_[0]) ? push_except() : (pc))
#	define XTAL_VM_CHECK_EXCEPT if(ap(except_[0])){ goto except_catch; }
#else
#	define XTAL_VM_CHECK_EXCEPT_PC(pc) (pc)
#	define XTAL_VM_CHECK_EXCEPT
#endif

const inst_t* VMachine::push_except(){
	push(except_[0]);
	return &throw_code_;
}

const inst_t* VMachine::inner_send(const inst_t* pc, int_t need_result_count, const IDPtr& primary_key, const Any& secondary_key, const Any& target){
	XTAL_GLOBAL_INTERPRETER_LOCK{
		Any self = ff().self();
		push_ff(pc, need_result_count, 0, 0, null);
		set_unsuported_error_info(target, primary_key, secondary_key);
		ap(target)->rawsend(myself(), primary_key, ap(secondary_key), ap(self));
	}
	return XTAL_VM_CHECK_EXCEPT_PC(ff().called_pc);
}

const inst_t* VMachine::inner_send_from_stack(const inst_t* pc, int_t need_result_count, const IDPtr& primary_key, int_t ntarget){
	Any secondary_key = null;
	Any target = get(ntarget);
	downsize(1);
	return inner_send(pc, need_result_count, primary_key, secondary_key, target);
}

const inst_t* VMachine::inner_send(const inst_t* pc, int_t need_result_count, const IDPtr& primary_key, const Any& secondary_key, const Any& target, const Any& a0){
	XTAL_GLOBAL_INTERPRETER_LOCK{
		Any self = ff().self();
		push(a0);
		push_ff(pc, need_result_count, 1, 0, null);
		set_unsuported_error_info(target, primary_key, secondary_key);
		ap(target)->rawsend(myself(), primary_key, ap(secondary_key), ap(self));
	}
	return XTAL_VM_CHECK_EXCEPT_PC(ff().called_pc);
}

const inst_t* VMachine::inner_send_from_stack(const inst_t* pc, int_t need_result_count, const IDPtr& primary_key, int_t ntarget, int_t na0){
	Any secondary_key = null;
	Any target = get(ntarget);
	Any a0 = get(na0);
	downsize(2);
	return inner_send(pc, need_result_count, primary_key, secondary_key, target, a0);
}

const inst_t* VMachine::inner_send_q(const inst_t* pc, int_t need_result_count, const IDPtr& primary_key, const Any& secondary_key, const Any& target, const Any& a0){
	XTAL_GLOBAL_INTERPRETER_LOCK{
		Any self = ff().self();
		push(a0);
		push_ff(pc, need_result_count, 1, 0, null);
		ff().called_pc = &check_unsupported_code_;
		set_unsuported_error_info(target, primary_key, secondary_key);
		ap(target)->rawsend(myself(), primary_key, ap(secondary_key), ap(self));
	}
	return XTAL_VM_CHECK_EXCEPT_PC(ff().called_pc);
}

const inst_t* VMachine::inner_send_from_stack_q(const inst_t* pc, int_t need_result_count, const IDPtr& primary_key, int_t ntarget, int_t na0){
	Any secondary_key = null;
	Any target = get(ntarget);
	Any a0 = get(na0);
	downsize(2);
	return inner_send_q(pc, need_result_count, primary_key, secondary_key, target, a0);
}

const inst_t* VMachine::inner_send(const inst_t* pc, int_t need_result_count, const IDPtr& primary_key, const Any& secondary_key, const Any& target, const Any& a0, const Any& a1){
	XTAL_GLOBAL_INTERPRETER_LOCK{
		Any self = ff().self();
		push(a0); push(a1);
		push_ff(pc, need_result_count, 2, 0, null);
		set_unsuported_error_info(target, primary_key, secondary_key);
		ap(target)->rawsend(myself(), primary_key, ap(secondary_key), ap(self));
	}
	return XTAL_VM_CHECK_EXCEPT_PC(ff().called_pc);
}

const inst_t* VMachine::inner_send_from_stack(const inst_t* pc, int_t need_result_count, const IDPtr& primary_key, int_t ntarget, int_t na0, int_t na1){
	Any secondary_key = null;
	Any target = get(ntarget);
	Any a0 = get(na0);
	Any a1 = get(na1);
	downsize(3);
	return inner_send(pc, need_result_count, primary_key, secondary_key, target, a0, a1);
}

const inst_t* VMachine::inner_send_q(const inst_t* pc, int_t need_result_count, const IDPtr& primary_key, const Any& secondary_key, const Any& target, const Any& a0, const Any& a1){
	XTAL_GLOBAL_INTERPRETER_LOCK{
		Any self = ff().self();
		push(a0); push(a1);
		push_ff(pc, need_result_count, 2, 0, null);
		ff().called_pc = &check_unsupported_code_;
		set_unsuported_error_info(target, primary_key, secondary_key);
		ap(target)->rawsend(myself(), primary_key, ap(secondary_key), ap(self));
	}
	return XTAL_VM_CHECK_EXCEPT_PC(ff().called_pc);
}

const inst_t* VMachine::inner_send_from_stack_q(const inst_t* pc, int_t need_result_count, const IDPtr& primary_key, int_t ntarget, int_t na0, int_t na1){
	Any secondary_key = null;
	Any target = get(ntarget);
	Any a0 = get(na0);
	Any a1 = get(na1);
	downsize(3);
	return inner_send_q(pc, need_result_count, primary_key, secondary_key, target, a0, a1);
}

void VMachine::execute_inner(const inst_t* start){

	const inst_t* pc = start;
	int_t stack_size = stack_.size() - ff().args_stack_size();
	int_t fun_frames_size = fun_frames_.size();

	if(const AnyPtr& d = builtin()->member(Xid(debug))){
		debug_ = unchecked_ptr_cast<Debug>(d);
		debug_enable_ = debug_->is_enabled();
	}
	else{
		debug_enable_ = false;
	}

	XTAL_ASSERT(stack_size>=0);

XTAL_GLOBAL_INTERPRETER_UNLOCK{
retry:
XTAL_TRY{
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
		push(fun_frames_[0].arguments());
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}

	XTAL_VM_CASE(PushThis){ // 3
		push(ff().self()); 
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(PushCurrentContext){ // 3
		push(ff().outer());
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

	XTAL_VM_CASE(Insert3){ XTAL_VM_CONTINUE(FunInsert3(pc)); /*
		Any temp = get(); 
		set(get(1)); 
		set(1, get(2)); 
		set(2, get(3)); 
		set(3, temp); 
		XTAL_VM_CONTINUE(pc + inst.ISIZE);;;;;;
	}*/ }

	XTAL_VM_CASE(AdjustResult){ // 3
		adjust_result(inst.result_count, inst.need_result_count);
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(If){ // 2
		XTAL_VM_CONTINUE(pc + (pop() ? inst.ISIZE : inst.OFFSET_address + inst.address));
	}

	XTAL_VM_CASE(Unless){ // 2
		XTAL_VM_CONTINUE(pc + (!pop() ? inst.ISIZE : inst.OFFSET_address + inst.address));
	}

	XTAL_VM_CASE(Goto){ // 2
		XTAL_VM_CONTINUE(pc + inst.OFFSET_address + inst.address); 
	}

	XTAL_VM_CASE(LocalVariableInc){ // 8
		Any a = local_variable(inst.number);
		uint_t atype = type(a)-TYPE_INT;
		if(atype<2){
			if(atype==0){
				a.set_i(ivalue(a)+1);
			}
			else{
				a.set_f(fvalue(a)+1);
			}
			set_local_variable(inst.number, a); 
			XTAL_VM_CONTINUE(pc + inst.ISIZE + InstSetLocalVariable1Byte::ISIZE);
		}
		else{
			XTAL_VM_CONTINUE(inner_send(pc+inst.ISIZE, 1, id_[Core::id_op_inc], null, a));
		}
	}

	XTAL_VM_CASE(LocalVariableIncDirect){ // 7
		Any& a = ff().variables_[inst.number];
		uint_t atype = type(a)-TYPE_INT;
		if(atype<2){
			if(atype==0){
				a.set_i(ivalue(a)+1);
			}
			else{
				a.set_f(fvalue(a)+1);
			}
			XTAL_VM_CONTINUE(pc + inst.ISIZE + InstSetLocalVariable1ByteDirect::ISIZE);
		}
		else{
			XTAL_VM_CONTINUE(inner_send(pc+inst.ISIZE, 1, id_[Core::id_op_inc], null, a));
		}
	}

	XTAL_VM_CASE(LocalVariableDec){ // 8
		Any a = local_variable(inst.number);
		uint_t atype = type(a)-TYPE_INT;
		if(atype<2){
			if(atype==0){
				a.set_i(ivalue(a)-1);
			}
			else{
				a.set_f(fvalue(a)-1);
			}
			set_local_variable(inst.number, a); 
			XTAL_VM_CONTINUE(pc + inst.ISIZE + InstSetLocalVariable1Byte::ISIZE);
		}
		else{
			XTAL_VM_CONTINUE(inner_send(pc+inst.ISIZE, 1, id_[Core::id_op_dec], null, a));
		}
	}

	XTAL_VM_CASE(LocalVariableDecDirect){ // 7
		Any& a = ff().variables_[inst.number];
		uint_t atype = type(a)-TYPE_INT;
		if(atype<2){
			if(atype==0){
				a.set_i(ivalue(a)-1);
			}
			else{
				a.set_f(fvalue(a)-1);
			}
			XTAL_VM_CONTINUE(pc + inst.ISIZE + InstSetLocalVariable1ByteDirect::ISIZE);
		}
		else{
			XTAL_VM_CONTINUE(inner_send(pc+inst.ISIZE, 1, id_[Core::id_op_dec], null, a));
		}
	}

	XTAL_VM_CASE(LocalVariableInc2Byte){ // 8
		Any a = local_variable(inst.number);
		uint_t atype = type(a)-TYPE_INT;
		if(atype<2){
			if(atype==0){
				a.set_i(ivalue(a)+1);
			}
			else{
				a.set_f(fvalue(a)+1);
			}
			set_local_variable(inst.number, a); 
			XTAL_VM_CONTINUE(pc + inst.ISIZE + InstSetLocalVariable2Byte::ISIZE);
		}
		else{
			XTAL_VM_CONTINUE(inner_send(pc+inst.ISIZE, 1, id_[Core::id_op_inc], null, a));
		}
	}

	XTAL_VM_CASE(LocalVariableDec2Byte){ // 8
		Any a = local_variable(inst.number);
		uint_t atype = type(a)-TYPE_INT;
		if(atype<2){
			if(atype==0){
				a.set_i(ivalue(a)-1);
			}
			else{
				a.set_f(fvalue(a)-1);
			}
			set_local_variable(inst.number, a); 
			XTAL_VM_CONTINUE(pc + inst.ISIZE + InstSetLocalVariable2Byte::ISIZE);
		}
		else{
			XTAL_VM_CONTINUE(inner_send(pc+inst.ISIZE, 1, id_[Core::id_op_dec], null, a));
		}
	}

	XTAL_VM_CASE(LocalVariable1Byte){ // 3
		push(local_variable(inst.number)); 
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(LocalVariable1ByteDirect){ // 3
		push(ff().variable(inst.number)); 
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

	XTAL_VM_CASE(SetLocalVariable1ByteDirect){ // 4
		ff().variable(inst.number, get());
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
			push(f.instance_variables->variable(inst.number, f.fun()->code()->class_info(inst.core_number)));
		}
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(SetInstanceVariable){ // 5
		FunFrame& f = ff();
		XTAL_GLOBAL_INTERPRETER_LOCK{ 
			f.instance_variables->set_variable(inst.number, f.fun()->code()->class_info(inst.core_number), get());
			downsize(1);
		}
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(CleanupCall){ // 2
		XTAL_VM_CONTINUE(pop_ff());
	}

	XTAL_VM_CASE(Return){ // 4
		if(debug_enable_ && debug_->is_enabled()){
			debug_hook(pc, BREAKPOINT_RETURN);
		}

		if(ff().need_result_count!=inst.results){
			adjust_result(inst.results); 
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
				XTAL_VM_EXCEPT(builtin()->member(Xid(YieldError))->call(Xt("Xtal Runtime Error 1012")));
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

	XTAL_VM_CASE(Property){ XTAL_VM_CONTINUE(FunProperty(pc)); /*
		XTAL_GLOBAL_INTERPRETER_LOCK{
			Any secondary_key = (inst.flags&CALL_FLAG_NS) ? pop() : null;
			Any primary_key = identifier_or_pop(inst.identifier_number);
			Any self = ff().self();
			Any target = pop();
			push_ff(pc + inst.ISIZE, 1, 0, 0, ap(self));
			set_unsuported_error_info(target, primary_key, secondary_key);
			ap(target)->rawsend(myself(), isp(primary_key), ap(secondary_key), ff().self());
		}
		XTAL_VM_CONTINUE(XTAL_VM_CHECK_EXCEPT_PC(ff().called_pc)); 	
	}*/ }

	XTAL_VM_CASE(SetProperty){ XTAL_VM_CONTINUE(FunSetProperty(pc)); /*
		XTAL_GLOBAL_INTERPRETER_LOCK{
			Any secondary_key = (inst.flags&CALL_FLAG_NS) ? pop() : null;
			Any primary_key = identifier_or_pop(inst.identifier_number);
			Any self = ff().self();
			Any target = pop();
			push_ff(pc + inst.ISIZE, 0, 1, 0, ap(self));
			set_unsuported_error_info(target, primary_key, secondary_key);
			ap(target)->rawsend(myself(), isp(primary_key), ap(secondary_key), ff().self());
		}
		XTAL_VM_CONTINUE(XTAL_VM_CHECK_EXCEPT_PC(ff().called_pc)); 	
	}*/ }

	XTAL_VM_CASE(Call){ XTAL_VM_CONTINUE(FunCall(pc)); /*
		XTAL_GLOBAL_INTERPRETER_LOCK{
			Any secondary_key = null;
			Any primary_key = id_[Core::id_op_call];
			Any self = ff().self();
			Any target = pop();
			push_ff(pc + inst.ISIZE, (InstCall&)inst, ap(self));
			set_unsuported_error_info(target, primary_key, secondary_key);
			ap(target)->rawcall(myself());
		}
		XTAL_VM_CONTINUE(XTAL_VM_CHECK_EXCEPT_PC(ff().called_pc));	
	}*/ }

	XTAL_VM_CASE(Send){ XTAL_VM_CONTINUE(FunSend(pc)); /*
		XTAL_GLOBAL_INTERPRETER_LOCK{
			Any secondary_key = (inst.flags&CALL_FLAG_NS) ? pop() : null;
			Any primary_key = identifier_or_pop(inst.identifier_number);
			Any self = ff().self();
			Any target = pop();
			push_ff(pc + inst.ISIZE, (InstCall&)inst, ap(self));
			set_unsuported_error_info(target, primary_key, secondary_key);
			if(inst.flags&CALL_FLAG_Q) ff().called_pc = &check_unsupported_code_;
			ap(target)->rawsend(myself(), isp(primary_key), ap(secondary_key), ff().self());
		}
		XTAL_VM_CONTINUE(XTAL_VM_CHECK_EXCEPT_PC(ff().called_pc)); 	
	}*/ }

	XTAL_VM_CASE(Member){ // 11
		XTAL_GLOBAL_INTERPRETER_LOCK{
			Any secondary_key = (inst.flags&CALL_FLAG_NS) ? pop() : null;
			Any primary_key = identifier_or_pop(inst.identifier_number);
			Any target = get();
			set_unsuported_error_info(target, primary_key, secondary_key);
			Any ret = ap(target)->member(isp(primary_key), ap(secondary_key), ff().self());

			if(inst.flags&CALL_FLAG_Q){
				set(ret);
			}
			else{
				XTAL_VM_CHECK_EXCEPT;
				if(rawne(ret, undefined)){
					set(ret);
				}
				else{
					XTAL_VM_EXCEPT(unsupported_error(ap(target), isp(primary_key), ap(secondary_key)));
				}
			}
		}
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(DefineMember){ XTAL_VM_CONTINUE(FunDefineMember(pc)); /*
		XTAL_GLOBAL_INTERPRETER_LOCK{
			Any secondary_key = (inst.flags&CALL_FLAG_NS) ? pop() : null;
			Any primary_key = identifier_or_pop(inst.identifier_number);
			AnyPtr value = pop();
			Any target = pop();
			set_unsuported_error_info(target, primary_key, secondary_key);
			ap(target)->def(isp(primary_key), ap(value), ap(secondary_key), KIND_PUBLIC);
		}
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}*/ }

	XTAL_VM_CASE(GlobalVariable){ // 11
		XTAL_GLOBAL_INTERPRETER_LOCK{
			Any ret = code()->filelocal()->member(identifier(inst.identifier_number));
			XTAL_VM_CHECK_EXCEPT;
			if(rawne(ret, undefined)){
				push(ret);
			}
			else{
				XTAL_VM_EXCEPT(unsupported_error(code()->filelocal(), identifier(inst.identifier_number), null));
			}
		}
		XTAL_VM_CONTINUE(pc + inst.ISIZE);;;;;;
	}

	XTAL_VM_CASE(SetGlobalVariable){ // 4
		XTAL_GLOBAL_INTERPRETER_LOCK{
			code()->filelocal()->set_member(identifier(inst.identifier_number), get(), null);
			downsize(1);
		}
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}

	XTAL_VM_CASE(DefineGlobalVariable){ // 4
		XTAL_GLOBAL_INTERPRETER_LOCK{
			code()->filelocal()->def(identifier(inst.identifier_number), get(), null, KIND_PUBLIC);
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

	XTAL_VM_CASE(BlockBegin){ // 6
		XTAL_GLOBAL_INTERPRETER_LOCK{
			FunFrame& f = ff(); 
			ScopeInfo* core = f.fun()->code()->scope_info(inst.core_number);
			const FramePtr& outer = (core->flags&ScopeInfo::FLAG_SCOPE_CHAIN) ? ff().outer() : unchecked_ptr_cast<Frame>(null);
			f.outer(xnew<Frame>(outer, code(), core));
		}
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}

	XTAL_VM_CASE(BlockBeginDirect){ // 4
		FunFrame& f = ff(); 
		f.variables_.upsize(f.fun()->code()->scope_info(inst.core_number)->variable_size);
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}

	XTAL_VM_CASE(BlockEnd){ // 3
		ff().outer(ff().outer()->outer()); 
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}

	XTAL_VM_CASE(BlockEndDirect){ // 4
		FunFrame& f = ff();
		f.variables_.downsize(f.fun()->code()->scope_info(inst.core_number)->variable_size);
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}

	XTAL_VM_CASE(TryBegin){ // 9
		FunFrame& f = ff(); 
		ExceptFrame& ef = except_frames_.push();
		ef.core = &f.fun()->code()->except_info_table_[inst.core_number];
		ef.fun_frame_count = fun_frames_.size();
		ef.stack_count = this->stack_size();
		ef.variable_size = f.variables_.size();
		ef.outer = f.outer();
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}

	XTAL_VM_CASE(TryEnd){ // 2
		XTAL_VM_CONTINUE(except_frames_.pop().core->finally_pc + code()->data()); 
	}

	XTAL_VM_CASE(PushGoto){ // 5
		//printf("-----------------------\n%d\n", (int_t)((pc+inst.address)-code()->data()));
		//XTAL_ASSERT((int_t)((pc+inst.OFFSET_address+inst.address)-code()->data())>=0);
		push(Any((int_t)((pc+inst.OFFSET_address+inst.address)-code()->data()))); 
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(PopGoto){ // 2
		XTAL_VM_CONTINUE(code()->data()+ivalue(pop()));
	}

	XTAL_VM_CASE(IfEq){ XTAL_VM_CONTINUE(FunIfEq(pc)); /*
		const AnyPtr& b = get(); uint_t btype = type(b)-TYPE_INT;
		const AnyPtr& a = get(1); uint_t atype = type(a)-TYPE_INT;
		uint_t abtype = atype | btype;
		if(abtype<2){
			bool ret;
			if(abtype==0){
				ret = ivalue(a) == ivalue(b);
			}
			else{
				float_t afvalue = atype==0 ? (float_t)ivalue(a) : fvalue(a);
				float_t bfvalue = btype==0 ? (float_t)ivalue(b) : fvalue(b);
				ret = afvalue == bfvalue;
			}
			downsize(2); 
			XTAL_VM_CONTINUE(ret ? pc+inst.ISIZE+InstIf::ISIZE : pc+inst.OFFSET_address+inst.address);
		}
		else{
			if(raweq(a, b)){
				downsize(2);
				XTAL_VM_CONTINUE(pc+inst.ISIZE+InstIf::ISIZE);
			}
			else{
				XTAL_VM_CONTINUE(inner_send_from_stack_q(pc+inst.ISIZE, 1, id_[Core::id_op_eq], 1, 0));
			}
		}
	}*/ }

	XTAL_VM_CASE(IfNe){ XTAL_VM_CONTINUE(FunIfNe(pc)); /*
		const AnyPtr& b = get(); uint_t btype = type(b)-TYPE_INT;
		const AnyPtr& a = get(1); uint_t atype = type(a)-TYPE_INT;
		uint_t abtype = atype | btype;
		if(abtype<2){
			bool ret;
			if(abtype==0){
				ret = ivalue(a) != ivalue(b);
			}
			else{
				float_t afvalue = atype==0 ? (float_t)ivalue(a) : fvalue(a);
				float_t bfvalue = btype==0 ? (float_t)ivalue(b) : fvalue(b);
				ret = afvalue != bfvalue;
			}
			downsize(2); 
			XTAL_VM_CONTINUE(ret ? pc+inst.ISIZE+InstUnless::ISIZE : pc+inst.OFFSET_address+inst.address);
		}
		else{
			if(rawne(a, b)){
				downsize(2);
				XTAL_VM_CONTINUE(pc+inst.ISIZE+InstUnless::ISIZE);
			}
			else{
				XTAL_VM_CONTINUE(inner_send_from_stack_q(pc+inst.ISIZE, 1, id_[Core::id_op_eq], 1, 0));
			}
		}
	}*/ }

	XTAL_VM_CASE(IfLt){ XTAL_VM_CONTINUE(FunIfLt(pc)); /*
		const AnyPtr& b = get(); uint_t btype = type(b)-TYPE_INT;
		const AnyPtr& a = get(1); uint_t atype = type(a)-TYPE_INT;
		uint_t abtype = atype | btype;
		if(abtype<2){
			bool ret;
			if(abtype==0){
				ret = ivalue(a) < ivalue(b);
			}
			else{
				float_t afvalue = atype==0 ? (float_t)ivalue(a) : fvalue(a);
				float_t bfvalue = btype==0 ? (float_t)ivalue(b) : fvalue(b);
				ret = afvalue < bfvalue;
			}
			downsize(2); 
			XTAL_VM_CONTINUE(ret ? pc+inst.ISIZE+InstIf::ISIZE : pc+inst.OFFSET_address+inst.address);
		}
		else{
			XTAL_VM_CONTINUE(inner_send_from_stack(pc+inst.ISIZE, 1, id_[Core::id_op_lt], 1, 0));
		}
	}*/ }

	XTAL_VM_CASE(IfLe){ XTAL_VM_CONTINUE(FunIfLe(pc)); /*
		const AnyPtr& b = get(); uint_t btype = type(b)-TYPE_INT;
		const AnyPtr& a = get(1); uint_t atype = type(a)-TYPE_INT;
		uint_t abtype = atype | btype;
		if(abtype<2){
			bool ret;
			if(abtype==0){
				ret = ivalue(a) <= ivalue(b);
			}
			else{
				float_t afvalue = atype==0 ? (float_t)ivalue(a) : fvalue(a);
				float_t bfvalue = btype==0 ? (float_t)ivalue(b) : fvalue(b);
				ret = afvalue <= bfvalue;
			}
			downsize(2); 
			XTAL_VM_CONTINUE(ret ? pc+inst.ISIZE+InstUnless::ISIZE : pc+inst.OFFSET_address+inst.address);
		}
		else{
			XTAL_VM_CONTINUE(inner_send_from_stack(pc+inst.ISIZE, 1, id_[Core::id_op_lt], 0, 1));
		}
	}*/ }

	XTAL_VM_CASE(IfGt){ XTAL_VM_CONTINUE(FunIfGt(pc)); /*
		const AnyPtr& b = get(); uint_t btype = type(b)-TYPE_INT;
		const AnyPtr& a = get(1); uint_t atype = type(a)-TYPE_INT;
		uint_t abtype = atype | btype;
		if(abtype<2){
			bool ret;
			if(abtype==0){
				ret = ivalue(a) > ivalue(b);
			}
			else{
				float_t afvalue = atype==0 ? (float_t)ivalue(a) : fvalue(a);
				float_t bfvalue = btype==0 ? (float_t)ivalue(b) : fvalue(b);
				ret = afvalue > bfvalue;
			}
			downsize(2); 
			XTAL_VM_CONTINUE(ret ? pc+inst.ISIZE+InstIf::ISIZE : pc+inst.OFFSET_address+inst.address);
		}
		else{
			XTAL_VM_CONTINUE(inner_send_from_stack(pc+inst.ISIZE, 1, id_[Core::id_op_lt], 0, 1));
		}
	}*/ }

	XTAL_VM_CASE(IfGe){ XTAL_VM_CONTINUE(FunIfGe(pc)); /*
		const AnyPtr& b = get(); uint_t btype = type(b)-TYPE_INT;
		const AnyPtr& a = get(1); uint_t atype = type(a)-TYPE_INT;
		uint_t abtype = atype | btype;
		if(abtype<2){
			bool ret;
			if(abtype==0){
				ret = ivalue(a) >= ivalue(b);
			}
			else{
				float_t afvalue = atype==0 ? (float_t)ivalue(a) : fvalue(a);
				float_t bfvalue = btype==0 ? (float_t)ivalue(b) : fvalue(b);
				ret = afvalue >= bfvalue;
			}
			downsize(2); 
			XTAL_VM_CONTINUE(ret ? pc+inst.ISIZE+InstUnless::ISIZE : pc+inst.OFFSET_address+inst.address);
		}
		else{
			XTAL_VM_CONTINUE(inner_send_from_stack(pc+inst.ISIZE, 1, id_[Core::id_op_lt], 1, 0));
		}
	}*/ }

	XTAL_VM_CASE(IfRawEq){ // 4
		pc += raweq(get(1), get()) ? (int)(inst.ISIZE+InstIf::ISIZE) : (int)(inst.OFFSET_address+inst.address);
		downsize(2);
		XTAL_VM_CONTINUE(pc);
	}

	XTAL_VM_CASE(IfRawNe){ // 4
		pc += !raweq(get(1), get()) ? (int)(inst.ISIZE+InstIf::ISIZE) : (int)(inst.OFFSET_address+inst.address);
		downsize(2);
		XTAL_VM_CONTINUE(pc);
	}

	XTAL_VM_CASE(IfIn){ // 2
		XTAL_VM_CONTINUE(inner_send_from_stack(pc+inst.ISIZE, 1, id_[Core::id_op_in], 1, 0));
	}

	XTAL_VM_CASE(IfNin){ // 2
		XTAL_VM_CONTINUE(inner_send_from_stack(pc+inst.ISIZE, 1, id_[Core::id_op_in], 1, 0));
	}

	XTAL_VM_CASE(IfIs){ // 4
		pc += get(1)->is(get()) ? (int)(inst.ISIZE+InstIf::ISIZE) : (int)(inst.OFFSET_address+inst.address);
		downsize(2);
		XTAL_VM_CONTINUE(pc);
	}

	XTAL_VM_CASE(IfNis){ // 4
		pc += !get(1)->is(get()) ? (int)(inst.ISIZE+InstIf::ISIZE) : (int)(inst.OFFSET_address+inst.address);
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

	XTAL_VM_CASE(IfArgIsUndefinedDirect){ // 3
		if(raweq(ff().variable(inst.arg), undefined)){
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
			XTAL_VM_CONTINUE(inner_send_from_stack(pc+inst.ISIZE, 1, id_[Core::id_op_pos], 0));
		}
	}

	XTAL_VM_CASE(Neg){ // 7
		const AnyPtr& a = get(); uint_t atype = type(a)-TYPE_INT;
		if(atype<2){
			if(atype==0){
				set(Any(-ivalue(a)));
			}
			else{
				set(Any(-fvalue(a)));
			}
			XTAL_VM_CONTINUE(pc+inst.ISIZE);
		}
		else{
			XTAL_VM_CONTINUE(inner_send_from_stack(pc+inst.ISIZE, 1, id_[Core::id_op_neg], 0));
		}
	}

	XTAL_VM_CASE(Com){ // 6
		const AnyPtr& a = get(); uint_t atype = type(a)-TYPE_INT;
		if(atype==0){
			set(Any(~ivalue(a)));
			XTAL_VM_CONTINUE(pc+inst.ISIZE);
		}
		else{
			XTAL_VM_CONTINUE(inner_send_from_stack(pc+inst.ISIZE, 1, id_[Core::id_op_com], 0));
		}
	}

	XTAL_VM_CASE(Not){ // 3
		set(Any(!get())); 
		XTAL_VM_CONTINUE(pc+inst.ISIZE); 
	}

	XTAL_VM_CASE(At){ // 2
		XTAL_VM_CONTINUE(inner_send_from_stack(pc+inst.ISIZE, 1, id_[Core::id_op_at], 1, 0));
	}

	XTAL_VM_CASE(SetAt){ // 2
		XTAL_VM_CONTINUE(inner_send_from_stack(pc+inst.ISIZE, 0, id_[Core::id_op_set_at], 1, 0, 2));
	}

	XTAL_VM_CASE(Add){ // 2
		XTAL_VM_CONTINUE(OpAdd(pc+inst.ISIZE, Core::id_op_add));
	}

	XTAL_VM_CASE(Sub){ // 2
		XTAL_VM_CONTINUE(OpSub(pc+inst.ISIZE, Core::id_op_sub));
	}

	XTAL_VM_CASE(Cat){ // 2
		XTAL_VM_CONTINUE(inner_send_from_stack(pc+inst.ISIZE, 1, id_[Core::id_op_cat], 1, 0));
	}

	XTAL_VM_CASE(Mul){ // 2
		XTAL_VM_CONTINUE(OpMul(pc+inst.ISIZE, Core::id_op_mul));
	}

	XTAL_VM_CASE(Div){ // 2
		XTAL_VM_CONTINUE(OpDiv(pc+inst.ISIZE, Core::id_op_div));
	}

	XTAL_VM_CASE(Mod){ // 2
		XTAL_VM_CONTINUE(OpMod(pc+inst.ISIZE, Core::id_op_mod));
	}

	XTAL_VM_CASE(And){ // 2
		XTAL_VM_CONTINUE(OpAnd(pc+inst.ISIZE, Core::id_op_and));
	}

	XTAL_VM_CASE(Or){ // 2
		XTAL_VM_CONTINUE(OpOr(pc+inst.ISIZE, Core::id_op_or));
	}

	XTAL_VM_CASE(Xor){ // 2
		XTAL_VM_CONTINUE(OpXor(pc+inst.ISIZE, Core::id_op_xor));
	}

	XTAL_VM_CASE(Shl){ // 2
		XTAL_VM_CONTINUE(OpShl(pc+inst.ISIZE, Core::id_op_shl));
	}

	XTAL_VM_CASE(Shr){ // 2
		XTAL_VM_CONTINUE(OpShr(pc+inst.ISIZE, Core::id_op_shr));
	}

	XTAL_VM_CASE(Ushr){ // 2
		XTAL_VM_CONTINUE(OpUshr(pc+inst.ISIZE, Core::id_op_ushr));
	}

	XTAL_VM_CASE(Eq){ XTAL_VM_CONTINUE(FunEq(pc)); /*
		const AnyPtr& b = get(); uint_t btype = type(b)-TYPE_INT;
		const AnyPtr& a = get(1); uint_t atype = type(a)-TYPE_INT;
		uint_t abtype = atype | btype;
		if(abtype<2){
			bool ret;
			if(abtype==0){
				ret = ivalue(a) == ivalue(b);
			}
			else{
				float_t afvalue = atype==0 ? (float_t)ivalue(a) : fvalue(a);
				float_t bfvalue = btype==0 ? (float_t)ivalue(b) : fvalue(b);
				ret = afvalue == bfvalue;
			}
			set(1, Any(ret));
			downsize(1); 
			XTAL_VM_CONTINUE(pc+inst.ISIZE);
		}
		else{
			if(raweq(a, b)){
				set(1, Any(true)); 
				downsize(1);
				XTAL_VM_CONTINUE(pc+inst.ISIZE);
			}
			else{
				XTAL_VM_CONTINUE(inner_send_from_stack_q(pc+inst.ISIZE, 1, id_[Core::id_op_eq], 1, 0));
			}
		}
	}*/ }

	XTAL_VM_CASE(Ne){ XTAL_VM_CONTINUE(FunNe(pc)); /*
		const AnyPtr& b = get(); uint_t btype = type(b)-TYPE_INT;
		const AnyPtr& a = get(1); uint_t atype = type(a)-TYPE_INT;
		uint_t abtype = atype | btype;
		if(abtype<2){
			bool ret;
			if(abtype==0){
				ret = ivalue(a) != ivalue(b);
			}
			else{
				float_t afvalue = atype==0 ? (float_t)ivalue(a) : fvalue(a);
				float_t bfvalue = btype==0 ? (float_t)ivalue(b) : fvalue(b);
				ret = afvalue != bfvalue;
			}
			set(1, Any(ret));
			downsize(1); 
			XTAL_VM_CONTINUE(pc+inst.ISIZE+InstNot::ISIZE);
		}
		else{
			if(rawne(a, b)){
				set(1, Any(true)); 
				downsize(1);
				XTAL_VM_CONTINUE(pc+inst.ISIZE+InstNot::ISIZE);
			}
			else{
				XTAL_VM_CONTINUE(inner_send_from_stack_q(pc+inst.ISIZE, 1, id_[Core::id_op_eq], 1, 0));
			}
		}
	}*/ }

	XTAL_VM_CASE(Lt){ XTAL_VM_CONTINUE(FunLt(pc)); /*
		const AnyPtr& b = get(); uint_t btype = type(b)-TYPE_INT;
		const AnyPtr& a = get(1); uint_t atype = type(a)-TYPE_INT;
		uint_t abtype = atype | btype;
		if(abtype<2){
			bool ret;
			if(abtype==0){
				ret = ivalue(a) < ivalue(b);
			}
			else{
				float_t afvalue = atype==0 ? (float_t)ivalue(a) : fvalue(a);
				float_t bfvalue = btype==0 ? (float_t)ivalue(b) : fvalue(b);
				ret = afvalue < bfvalue;
			}
			set(1, Any(ret));
			downsize(1); 
			XTAL_VM_CONTINUE(pc+inst.ISIZE);
		}
		else{
			XTAL_VM_CONTINUE(inner_send_from_stack(pc+inst.ISIZE, 1, id_[Core::id_op_lt], 1, 0));
		}
	}*/ }

	XTAL_VM_CASE(Le){ XTAL_VM_CONTINUE(FunLe(pc)); /*
		const AnyPtr& b = get(); uint_t btype = type(b)-TYPE_INT;
		const AnyPtr& a = get(1); uint_t atype = type(a)-TYPE_INT;
		uint_t abtype = atype | btype;
		if(abtype<2){
			bool ret;
			if(abtype==0){
				ret = ivalue(a) <= ivalue(b);
			}
			else{
				float_t afvalue = atype==0 ? (float_t)ivalue(a) : fvalue(a);
				float_t bfvalue = btype==0 ? (float_t)ivalue(b) : fvalue(b);
				ret = afvalue <= bfvalue;
			}
			set(1, Any(ret));
			downsize(1); 
			XTAL_VM_CONTINUE(pc+inst.ISIZE+InstNot::ISIZE);
		}
		else{
			XTAL_VM_CONTINUE(inner_send_from_stack(pc+inst.ISIZE, 1, id_[Core::id_op_lt], 0, 1));
		}
	}*/ }

	XTAL_VM_CASE(Gt){ XTAL_VM_CONTINUE(FunGt(pc)); /*
		const AnyPtr& b = get(); uint_t btype = type(b)-TYPE_INT;
		const AnyPtr& a = get(1); uint_t atype = type(a)-TYPE_INT;
		uint_t abtype = atype | btype;
		if(abtype<2){
			bool ret;
			if(abtype==0){
				ret = ivalue(a) > ivalue(b);
			}
			else{
				float_t afvalue = atype==0 ? (float_t)ivalue(a) : fvalue(a);
				float_t bfvalue = btype==0 ? (float_t)ivalue(b) : fvalue(b);
				ret = afvalue > bfvalue;
			}
			set(1, Any(ret));
			downsize(1); 
			XTAL_VM_CONTINUE(pc+inst.ISIZE);
		}
		else{
			XTAL_VM_CONTINUE(inner_send_from_stack(pc+inst.ISIZE, 1, id_[Core::id_op_lt], 1, 0));
		}
	}*/ }

	XTAL_VM_CASE(Ge){ XTAL_VM_CONTINUE(FunGe(pc)); /*
		const AnyPtr& b = get(); uint_t btype = type(b)-TYPE_INT;
		const AnyPtr& a = get(1); uint_t atype = type(a)-TYPE_INT;
		uint_t abtype = atype | btype;
		if(abtype<2){
			bool ret;
			if(abtype==0){
				ret = ivalue(a) >= ivalue(b);
			}
			else{
				float_t afvalue = atype==0 ? (float_t)ivalue(a) : fvalue(a);
				float_t bfvalue = btype==0 ? (float_t)ivalue(b) : fvalue(b);
				ret = afvalue >= bfvalue;
			}
			set(1, Any(ret));
			downsize(1); 
			XTAL_VM_CONTINUE(pc+inst.ISIZE+InstNot::ISIZE);
		}
		else{
			XTAL_VM_CONTINUE(inner_send_from_stack(pc+inst.ISIZE, 1, id_[Core::id_op_lt], 0, 1));
		}
	}*/ }

	XTAL_VM_CASE(RawEq){ // 4
		set(1, Any(raweq(get(1), get())));
		downsize(1);
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}

	XTAL_VM_CASE(RawNe){ // 4
		set(1, Any(rawne(get(1), get())));
		downsize(1);
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}

	XTAL_VM_CASE(In){ // 2
		XTAL_VM_CONTINUE(inner_send_from_stack(pc+inst.ISIZE, 1, id_[Core::id_op_in], 1, 0));
	}

	XTAL_VM_CASE(Nin){ // 2
		XTAL_VM_CONTINUE(inner_send_from_stack(pc+inst.ISIZE, 1, id_[Core::id_op_in], 1, 0));
	}

	XTAL_VM_CASE(Is){ // 4
		set(1, Any(get(1)->is(get())));
		downsize(1);
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}

	XTAL_VM_CASE(Nis){ // 4
		set(1, Any(!get(1)->is(get())));
		downsize(1);
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}

	XTAL_VM_CASE(Inc){ // 2
		XTAL_VM_CONTINUE(OpInc(pc+inst.ISIZE, Core::id_op_inc));
	}

	XTAL_VM_CASE(Dec){ // 2
		XTAL_VM_CONTINUE(OpDec(pc+inst.ISIZE, Core::id_op_dec));
	}

	XTAL_VM_CASE(AddAssign){ // 2
		XTAL_VM_CONTINUE(OpAdd(pc+inst.ISIZE, Core::id_op_add_assign));
	}

	XTAL_VM_CASE(SubAssign){ // 2
		XTAL_VM_CONTINUE(OpSub(pc+inst.ISIZE, Core::id_op_sub_assign));
	}

	XTAL_VM_CASE(CatAssign){ // 2
		XTAL_VM_CONTINUE(inner_send_from_stack(pc+inst.ISIZE, 1, id_[Core::id_op_cat_assign], 1, 0));
	}

	XTAL_VM_CASE(MulAssign){ // 2
		XTAL_VM_CONTINUE(OpMul(pc+inst.ISIZE, Core::id_op_mul_assign));
	}

	XTAL_VM_CASE(DivAssign){ // 2
		XTAL_VM_CONTINUE(OpDiv(pc+inst.ISIZE, Core::id_op_div_assign));
	}

	XTAL_VM_CASE(ModAssign){ // 2
		XTAL_VM_CONTINUE(OpMod(pc+inst.ISIZE, Core::id_op_mod_assign));
	}

	XTAL_VM_CASE(AndAssign){ // 2
		XTAL_VM_CONTINUE(OpAnd(pc+inst.ISIZE, Core::id_op_and_assign));
	}

	XTAL_VM_CASE(OrAssign){ // 2
		XTAL_VM_CONTINUE(OpOr(pc+inst.ISIZE, Core::id_op_or_assign));
	}

	XTAL_VM_CASE(XorAssign){ // 2
		XTAL_VM_CONTINUE(OpXor(pc+inst.ISIZE, Core::id_op_xor_assign));
	}

	XTAL_VM_CASE(ShlAssign){ // 2
		XTAL_VM_CONTINUE(OpShl(pc+inst.ISIZE, Core::id_op_shl_assign));
	}

	XTAL_VM_CASE(ShrAssign){ // 2
		XTAL_VM_CONTINUE(OpShr(pc+inst.ISIZE, Core::id_op_shr_assign));
	}

	XTAL_VM_CASE(UshrAssign){ // 2
		XTAL_VM_CONTINUE(OpUshr(pc+inst.ISIZE, Core::id_op_ushr_assign));
	}

	XTAL_VM_CASE(Range){ XTAL_VM_CONTINUE(FunRange(pc)); /*
		XTAL_GLOBAL_INTERPRETER_LOCK{
			Any rhs = pop();
			Any lhs = pop();
			push(ap(rhs)); push(ap(Any((int_t)inst.kind)));
			push_ff(pc+inst.ISIZE, 1, 2, 0, null);
			ap(lhs)->rawsend(myself(), id_[Core::id_op_range], null);
		}
		XTAL_VM_CONTINUE(XTAL_VM_CHECK_EXCEPT_PC(ff().called_pc));
	}*/ }

	XTAL_VM_CASE(ClassBegin){ XTAL_VM_CONTINUE(FunClassBegin(pc)); /*
		XTAL_GLOBAL_INTERPRETER_LOCK{
			ClassInfo* core = code()->class_info(inst.core_number);
			const FramePtr& outer = (core->flags&ClassInfo::FLAG_SCOPE_CHAIN) ? ff().outer() : unchecked_ptr_cast<Frame>(null);
			ClassPtr cp;

			switch(core->kind){
				XTAL_CASE(KIND_CLASS){
					cp = xnew<Class>(outer, code(), core);
				}

				XTAL_CASE(KIND_SINGLETON){
					cp = xnew<Singleton>(outer, code(), core);
				}
			}
			
			int_t n = core->mixins;
			for(int_t i = 0; i<n; ++i){
				cp->inherit_strict(ptr_cast<Class>(pop()));
			}

			push_ff(pc + inst.ISIZE, 0, 0, 0, cp);
			ff().fun(prev_fun());

			ff().outer(cp);
		}
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}*/ }

	XTAL_VM_CASE(ClassEnd){ XTAL_VM_CONTINUE(FunClassEnd(pc)); /*
		if(raweq(ff().outer()->get_class(), ff().outer())){
			Singleton* singleton = (Singleton*)pvalue(ff().outer());
			singleton->init_singleton(myself());
		}

		push(ff().outer());
		ff().outer(ff().outer()->outer());
		pop_ff();
		XTAL_VM_CONTINUE(pc + inst.ISIZE);;;;;;
	}*/ }

	XTAL_VM_CASE(DefineClassMember){ // 5
		XTAL_GLOBAL_INTERPRETER_LOCK{
			const ClassPtr& p = cast<const ClassPtr&>(ff().outer());
			p->set_class_member(p->block_size()-1-inst.number, identifier(inst.identifier_number), get(1), get(), inst.accessibility);
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

	XTAL_VM_CASE(MakeFun){ XTAL_VM_CONTINUE(FunMakeFun(pc)); /*
		int_t table_n = inst.core_number, end = inst.OFFSET_address + inst.address;
		XTAL_GLOBAL_INTERPRETER_LOCK{
			FunInfo* core = code()->fun_info(table_n);
			const FramePtr& outer = ff().outer();
			switch(core->kind){
				XTAL_NODEFAULT;
				XTAL_CASE(KIND_FUN){ push(xnew<Fun>(outer, ff().self(), code(), core)); }
				XTAL_CASE(KIND_LAMBDA){ push(xnew<Lambda>(outer, ff().self(), code(), core)); }
				XTAL_CASE(KIND_METHOD){ push(xnew<Method>(outer, code(), core)); }
				XTAL_CASE(KIND_FIBER){ push(xnew<Fiber>(outer, ff().self(), code(), core)); }
			}
		}
		XTAL_VM_CONTINUE(pc + end);
	}*/ }

	XTAL_VM_CASE(MakeInstanceVariableAccessor){ XTAL_VM_CONTINUE(FunMakeInstanceVariableAccessor(pc)); /*
		XTAL_GLOBAL_INTERPRETER_LOCK{
			AnyPtr ret;
			switch(inst.type){
				XTAL_NODEFAULT;

				XTAL_CASE(0){ ret = xnew<InstanceVariableGetter>(inst.number, code()->class_info(inst.core_number)); }
				XTAL_CASE(1){ ret = xnew<InstanceVariableSetter>(inst.number, code()->class_info(inst.core_number)); }
			}
			push(ret);
		}
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}*/ }

	XTAL_VM_CASE(Throw){ // 5
		XTAL_GLOBAL_INTERPRETER_LOCK{
			AnyPtr except = pop();
			if(!except){
				except = ap(except_[0]);
			}

			if(!except->is(builtin()->member(Xid(Exception)))){
				XTAL_VM_EXCEPT(RuntimeError()->call(except));
			}
			else{
				XTAL_VM_EXCEPT(except);
			}
		}
	}

	XTAL_VM_CASE(ThrowUnsupportedError){ // 4
		XTAL_GLOBAL_INTERPRETER_LOCK{
			FunFrame& f = ff();
			if(ap(f.secondary_key_)){
				XTAL_VM_EXCEPT(unsupported_error(ap(f.target_)->get_class(), isp(f.primary_key_), ap(f.secondary_key_)));
			}
			else{
				XTAL_VM_EXCEPT(unsupported_error(ap(f.target_)->get_class(), isp(f.primary_key_), null));
			}
		}
	}

	XTAL_VM_CASE(ThrowUndefined){ // 2
		XTAL_VM_EXCEPT(undefined); 
	}

	XTAL_VM_CASE(IfDebug){ // 2
		XTAL_VM_CONTINUE(pc + ((debug_enable_ || debug_->is_enabled()) ? inst.ISIZE : inst.OFFSET_address + inst.address));
	}

	XTAL_VM_CASE(Assert){ // 4
		XTAL_GLOBAL_INTERPRETER_LOCK{
			AnyPtr message = pop();
			XTAL_VM_EXCEPT(builtin()->member(Xid(AssertionFailed))->call(message)); 
		}
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}

	XTAL_VM_CASE(BreakPoint){ // 3
		if(debug_enable_ && debug_->is_enabled()){
			debug_hook(pc, BREAKPOINT);
		}
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
	set_except_0(append_backtrace(pc, ap(except_[0])));
	pc = catch_body(pc, stack_size, fun_frames_size);
	if(pc){
		goto begin;
	}
	goto rethrow;

}
XTAL_CATCH(e){
	set_except_0(append_backtrace(pc, e));
	pc = catch_body(pc, stack_size, fun_frames_size);
	if(pc){
		goto retry;
	}
	goto rethrow;	
}
#ifndef XTAL_NO_EXCEPTIONS
catch(...){
	set_except_0(null);
	catch_body(start, stack_size, fun_frames_size);
	pop_ff();
	throw;
}
#endif
}

return;

rethrow:
	pop_ff();
	XTAL_THROW(ap(except_[0]), return);	
}

#undef XTAL_VM_CONTINUE
#define XTAL_VM_CONTINUE(x) return (x)

/*
#undef XTAL_VM_RETURN
#define XTAL_VM_RETURN last_except_ = null; return 0

#undef XTAL_VM_EXCEPT
#define XTAL_VM_EXCEPT(x) last_except_ = (x); return 0

#ifdef XTAL_NO_EXCEPTIONS
#	undef XTAL_VM_CHECK_EXCEPT
#	define XTAL_VM_CHECK_EXCEPT if(ap(except_[0])){ return 0; }
#endif
*/

//{FUNS{{
const inst_t* VMachine::FunInsert3(const inst_t* pc){
		XTAL_VM_DEF_INST(Insert3);
		Any temp = get(); 
		set(get(1)); 
		set(1, get(2)); 
		set(2, get(3)); 
		set(3, temp); 
		XTAL_VM_CONTINUE(pc + inst.ISIZE);;;;;;
}

const inst_t* VMachine::FunProperty(const inst_t* pc){
		XTAL_VM_DEF_INST(Property);
		XTAL_GLOBAL_INTERPRETER_LOCK{
			Any secondary_key = (inst.flags&CALL_FLAG_NS) ? pop() : null;
			Any primary_key = identifier_or_pop(inst.identifier_number);
			Any self = ff().self();
			Any target = pop();
			push_ff(pc + inst.ISIZE, 1, 0, 0, ap(self));
			set_unsuported_error_info(target, primary_key, secondary_key);
			ap(target)->rawsend(myself(), isp(primary_key), ap(secondary_key), ff().self());
		}
		XTAL_VM_CONTINUE(XTAL_VM_CHECK_EXCEPT_PC(ff().called_pc)); 	
}

const inst_t* VMachine::FunSetProperty(const inst_t* pc){
		XTAL_VM_DEF_INST(SetProperty);
		XTAL_GLOBAL_INTERPRETER_LOCK{
			Any secondary_key = (inst.flags&CALL_FLAG_NS) ? pop() : null;
			Any primary_key = identifier_or_pop(inst.identifier_number);
			Any self = ff().self();
			Any target = pop();
			push_ff(pc + inst.ISIZE, 0, 1, 0, ap(self));
			set_unsuported_error_info(target, primary_key, secondary_key);
			ap(target)->rawsend(myself(), isp(primary_key), ap(secondary_key), ff().self());
		}
		XTAL_VM_CONTINUE(XTAL_VM_CHECK_EXCEPT_PC(ff().called_pc)); 	
}

const inst_t* VMachine::FunCall(const inst_t* pc){
		XTAL_VM_DEF_INST(Call);
		XTAL_GLOBAL_INTERPRETER_LOCK{
			Any secondary_key = null;
			Any primary_key = id_[Core::id_op_call];
			Any self = ff().self();
			Any target = pop();
			push_ff(pc + inst.ISIZE, (InstCall&)inst, ap(self));
			set_unsuported_error_info(target, primary_key, secondary_key);
			ap(target)->rawcall(myself());
		}
		XTAL_VM_CONTINUE(XTAL_VM_CHECK_EXCEPT_PC(ff().called_pc));	
}

const inst_t* VMachine::FunSend(const inst_t* pc){
		XTAL_VM_DEF_INST(Send);
		XTAL_GLOBAL_INTERPRETER_LOCK{
			Any secondary_key = (inst.flags&CALL_FLAG_NS) ? pop() : null;
			Any primary_key = identifier_or_pop(inst.identifier_number);
			Any self = ff().self();
			Any target = pop();
			push_ff(pc + inst.ISIZE, (InstCall&)inst, ap(self));
			set_unsuported_error_info(target, primary_key, secondary_key);
			if(inst.flags&CALL_FLAG_Q) ff().called_pc = &check_unsupported_code_;
			ap(target)->rawsend(myself(), isp(primary_key), ap(secondary_key), ff().self());
		}
		XTAL_VM_CONTINUE(XTAL_VM_CHECK_EXCEPT_PC(ff().called_pc)); 	
}

const inst_t* VMachine::FunDefineMember(const inst_t* pc){
		XTAL_VM_DEF_INST(DefineMember);
		XTAL_GLOBAL_INTERPRETER_LOCK{
			Any secondary_key = (inst.flags&CALL_FLAG_NS) ? pop() : null;
			Any primary_key = identifier_or_pop(inst.identifier_number);
			AnyPtr value = pop();
			Any target = pop();
			set_unsuported_error_info(target, primary_key, secondary_key);
			ap(target)->def(isp(primary_key), ap(value), ap(secondary_key), KIND_PUBLIC);
		}
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
}

const inst_t* VMachine::FunIfEq(const inst_t* pc){
		XTAL_VM_DEF_INST(IfEq);
		const AnyPtr& b = get(); uint_t btype = type(b)-TYPE_INT;
		const AnyPtr& a = get(1); uint_t atype = type(a)-TYPE_INT;
		uint_t abtype = atype | btype;
		if(abtype<2){
			bool ret;
			if(abtype==0){
				ret = ivalue(a) == ivalue(b);
			}
			else{
				float_t afvalue = atype==0 ? (float_t)ivalue(a) : fvalue(a);
				float_t bfvalue = btype==0 ? (float_t)ivalue(b) : fvalue(b);
				ret = afvalue == bfvalue;
			}
			downsize(2); 
			XTAL_VM_CONTINUE(ret ? pc+inst.ISIZE+InstIf::ISIZE : pc+inst.OFFSET_address+inst.address);
		}
		else{
			if(raweq(a, b)){
				downsize(2);
				XTAL_VM_CONTINUE(pc+inst.ISIZE+InstIf::ISIZE);
			}
			else{
				XTAL_VM_CONTINUE(inner_send_from_stack_q(pc+inst.ISIZE, 1, id_[Core::id_op_eq], 1, 0));
			}
		}
}

const inst_t* VMachine::FunIfNe(const inst_t* pc){
		XTAL_VM_DEF_INST(IfNe);
		const AnyPtr& b = get(); uint_t btype = type(b)-TYPE_INT;
		const AnyPtr& a = get(1); uint_t atype = type(a)-TYPE_INT;
		uint_t abtype = atype | btype;
		if(abtype<2){
			bool ret;
			if(abtype==0){
				ret = ivalue(a) != ivalue(b);
			}
			else{
				float_t afvalue = atype==0 ? (float_t)ivalue(a) : fvalue(a);
				float_t bfvalue = btype==0 ? (float_t)ivalue(b) : fvalue(b);
				ret = afvalue != bfvalue;
			}
			downsize(2); 
			XTAL_VM_CONTINUE(ret ? pc+inst.ISIZE+InstUnless::ISIZE : pc+inst.OFFSET_address+inst.address);
		}
		else{
			if(rawne(a, b)){
				downsize(2);
				XTAL_VM_CONTINUE(pc+inst.ISIZE+InstUnless::ISIZE);
			}
			else{
				XTAL_VM_CONTINUE(inner_send_from_stack_q(pc+inst.ISIZE, 1, id_[Core::id_op_eq], 1, 0));
			}
		}
}

const inst_t* VMachine::FunIfLt(const inst_t* pc){
		XTAL_VM_DEF_INST(IfLt);
		const AnyPtr& b = get(); uint_t btype = type(b)-TYPE_INT;
		const AnyPtr& a = get(1); uint_t atype = type(a)-TYPE_INT;
		uint_t abtype = atype | btype;
		if(abtype<2){
			bool ret;
			if(abtype==0){
				ret = ivalue(a) < ivalue(b);
			}
			else{
				float_t afvalue = atype==0 ? (float_t)ivalue(a) : fvalue(a);
				float_t bfvalue = btype==0 ? (float_t)ivalue(b) : fvalue(b);
				ret = afvalue < bfvalue;
			}
			downsize(2); 
			XTAL_VM_CONTINUE(ret ? pc+inst.ISIZE+InstIf::ISIZE : pc+inst.OFFSET_address+inst.address);
		}
		else{
			XTAL_VM_CONTINUE(inner_send_from_stack(pc+inst.ISIZE, 1, id_[Core::id_op_lt], 1, 0));
		}
}

const inst_t* VMachine::FunIfLe(const inst_t* pc){
		XTAL_VM_DEF_INST(IfLe);
		const AnyPtr& b = get(); uint_t btype = type(b)-TYPE_INT;
		const AnyPtr& a = get(1); uint_t atype = type(a)-TYPE_INT;
		uint_t abtype = atype | btype;
		if(abtype<2){
			bool ret;
			if(abtype==0){
				ret = ivalue(a) <= ivalue(b);
			}
			else{
				float_t afvalue = atype==0 ? (float_t)ivalue(a) : fvalue(a);
				float_t bfvalue = btype==0 ? (float_t)ivalue(b) : fvalue(b);
				ret = afvalue <= bfvalue;
			}
			downsize(2); 
			XTAL_VM_CONTINUE(ret ? pc+inst.ISIZE+InstUnless::ISIZE : pc+inst.OFFSET_address+inst.address);
		}
		else{
			XTAL_VM_CONTINUE(inner_send_from_stack(pc+inst.ISIZE, 1, id_[Core::id_op_lt], 0, 1));
		}
}

const inst_t* VMachine::FunIfGt(const inst_t* pc){
		XTAL_VM_DEF_INST(IfGt);
		const AnyPtr& b = get(); uint_t btype = type(b)-TYPE_INT;
		const AnyPtr& a = get(1); uint_t atype = type(a)-TYPE_INT;
		uint_t abtype = atype | btype;
		if(abtype<2){
			bool ret;
			if(abtype==0){
				ret = ivalue(a) > ivalue(b);
			}
			else{
				float_t afvalue = atype==0 ? (float_t)ivalue(a) : fvalue(a);
				float_t bfvalue = btype==0 ? (float_t)ivalue(b) : fvalue(b);
				ret = afvalue > bfvalue;
			}
			downsize(2); 
			XTAL_VM_CONTINUE(ret ? pc+inst.ISIZE+InstIf::ISIZE : pc+inst.OFFSET_address+inst.address);
		}
		else{
			XTAL_VM_CONTINUE(inner_send_from_stack(pc+inst.ISIZE, 1, id_[Core::id_op_lt], 0, 1));
		}
}

const inst_t* VMachine::FunIfGe(const inst_t* pc){
		XTAL_VM_DEF_INST(IfGe);
		const AnyPtr& b = get(); uint_t btype = type(b)-TYPE_INT;
		const AnyPtr& a = get(1); uint_t atype = type(a)-TYPE_INT;
		uint_t abtype = atype | btype;
		if(abtype<2){
			bool ret;
			if(abtype==0){
				ret = ivalue(a) >= ivalue(b);
			}
			else{
				float_t afvalue = atype==0 ? (float_t)ivalue(a) : fvalue(a);
				float_t bfvalue = btype==0 ? (float_t)ivalue(b) : fvalue(b);
				ret = afvalue >= bfvalue;
			}
			downsize(2); 
			XTAL_VM_CONTINUE(ret ? pc+inst.ISIZE+InstUnless::ISIZE : pc+inst.OFFSET_address+inst.address);
		}
		else{
			XTAL_VM_CONTINUE(inner_send_from_stack(pc+inst.ISIZE, 1, id_[Core::id_op_lt], 1, 0));
		}
}

const inst_t* VMachine::FunEq(const inst_t* pc){
		XTAL_VM_DEF_INST(Eq);
		const AnyPtr& b = get(); uint_t btype = type(b)-TYPE_INT;
		const AnyPtr& a = get(1); uint_t atype = type(a)-TYPE_INT;
		uint_t abtype = atype | btype;
		if(abtype<2){
			bool ret;
			if(abtype==0){
				ret = ivalue(a) == ivalue(b);
			}
			else{
				float_t afvalue = atype==0 ? (float_t)ivalue(a) : fvalue(a);
				float_t bfvalue = btype==0 ? (float_t)ivalue(b) : fvalue(b);
				ret = afvalue == bfvalue;
			}
			set(1, Any(ret));
			downsize(1); 
			XTAL_VM_CONTINUE(pc+inst.ISIZE);
		}
		else{
			if(raweq(a, b)){
				set(1, Any(true)); 
				downsize(1);
				XTAL_VM_CONTINUE(pc+inst.ISIZE);
			}
			else{
				XTAL_VM_CONTINUE(inner_send_from_stack_q(pc+inst.ISIZE, 1, id_[Core::id_op_eq], 1, 0));
			}
		}
}

const inst_t* VMachine::FunNe(const inst_t* pc){
		XTAL_VM_DEF_INST(Ne);
		const AnyPtr& b = get(); uint_t btype = type(b)-TYPE_INT;
		const AnyPtr& a = get(1); uint_t atype = type(a)-TYPE_INT;
		uint_t abtype = atype | btype;
		if(abtype<2){
			bool ret;
			if(abtype==0){
				ret = ivalue(a) != ivalue(b);
			}
			else{
				float_t afvalue = atype==0 ? (float_t)ivalue(a) : fvalue(a);
				float_t bfvalue = btype==0 ? (float_t)ivalue(b) : fvalue(b);
				ret = afvalue != bfvalue;
			}
			set(1, Any(ret));
			downsize(1); 
			XTAL_VM_CONTINUE(pc+inst.ISIZE+InstNot::ISIZE);
		}
		else{
			if(rawne(a, b)){
				set(1, Any(true)); 
				downsize(1);
				XTAL_VM_CONTINUE(pc+inst.ISIZE+InstNot::ISIZE);
			}
			else{
				XTAL_VM_CONTINUE(inner_send_from_stack_q(pc+inst.ISIZE, 1, id_[Core::id_op_eq], 1, 0));
			}
		}
}

const inst_t* VMachine::FunLt(const inst_t* pc){
		XTAL_VM_DEF_INST(Lt);
		const AnyPtr& b = get(); uint_t btype = type(b)-TYPE_INT;
		const AnyPtr& a = get(1); uint_t atype = type(a)-TYPE_INT;
		uint_t abtype = atype | btype;
		if(abtype<2){
			bool ret;
			if(abtype==0){
				ret = ivalue(a) < ivalue(b);
			}
			else{
				float_t afvalue = atype==0 ? (float_t)ivalue(a) : fvalue(a);
				float_t bfvalue = btype==0 ? (float_t)ivalue(b) : fvalue(b);
				ret = afvalue < bfvalue;
			}
			set(1, Any(ret));
			downsize(1); 
			XTAL_VM_CONTINUE(pc+inst.ISIZE);
		}
		else{
			XTAL_VM_CONTINUE(inner_send_from_stack(pc+inst.ISIZE, 1, id_[Core::id_op_lt], 1, 0));
		}
}

const inst_t* VMachine::FunLe(const inst_t* pc){
		XTAL_VM_DEF_INST(Le);
		const AnyPtr& b = get(); uint_t btype = type(b)-TYPE_INT;
		const AnyPtr& a = get(1); uint_t atype = type(a)-TYPE_INT;
		uint_t abtype = atype | btype;
		if(abtype<2){
			bool ret;
			if(abtype==0){
				ret = ivalue(a) <= ivalue(b);
			}
			else{
				float_t afvalue = atype==0 ? (float_t)ivalue(a) : fvalue(a);
				float_t bfvalue = btype==0 ? (float_t)ivalue(b) : fvalue(b);
				ret = afvalue <= bfvalue;
			}
			set(1, Any(ret));
			downsize(1); 
			XTAL_VM_CONTINUE(pc+inst.ISIZE+InstNot::ISIZE);
		}
		else{
			XTAL_VM_CONTINUE(inner_send_from_stack(pc+inst.ISIZE, 1, id_[Core::id_op_lt], 0, 1));
		}
}

const inst_t* VMachine::FunGt(const inst_t* pc){
		XTAL_VM_DEF_INST(Gt);
		const AnyPtr& b = get(); uint_t btype = type(b)-TYPE_INT;
		const AnyPtr& a = get(1); uint_t atype = type(a)-TYPE_INT;
		uint_t abtype = atype | btype;
		if(abtype<2){
			bool ret;
			if(abtype==0){
				ret = ivalue(a) > ivalue(b);
			}
			else{
				float_t afvalue = atype==0 ? (float_t)ivalue(a) : fvalue(a);
				float_t bfvalue = btype==0 ? (float_t)ivalue(b) : fvalue(b);
				ret = afvalue > bfvalue;
			}
			set(1, Any(ret));
			downsize(1); 
			XTAL_VM_CONTINUE(pc+inst.ISIZE);
		}
		else{
			XTAL_VM_CONTINUE(inner_send_from_stack(pc+inst.ISIZE, 1, id_[Core::id_op_lt], 1, 0));
		}
}

const inst_t* VMachine::FunGe(const inst_t* pc){
		XTAL_VM_DEF_INST(Ge);
		const AnyPtr& b = get(); uint_t btype = type(b)-TYPE_INT;
		const AnyPtr& a = get(1); uint_t atype = type(a)-TYPE_INT;
		uint_t abtype = atype | btype;
		if(abtype<2){
			bool ret;
			if(abtype==0){
				ret = ivalue(a) >= ivalue(b);
			}
			else{
				float_t afvalue = atype==0 ? (float_t)ivalue(a) : fvalue(a);
				float_t bfvalue = btype==0 ? (float_t)ivalue(b) : fvalue(b);
				ret = afvalue >= bfvalue;
			}
			set(1, Any(ret));
			downsize(1); 
			XTAL_VM_CONTINUE(pc+inst.ISIZE+InstNot::ISIZE);
		}
		else{
			XTAL_VM_CONTINUE(inner_send_from_stack(pc+inst.ISIZE, 1, id_[Core::id_op_lt], 0, 1));
		}
}

const inst_t* VMachine::FunRange(const inst_t* pc){
		XTAL_VM_DEF_INST(Range);
		XTAL_GLOBAL_INTERPRETER_LOCK{
			Any rhs = pop();
			Any lhs = pop();
			push(ap(rhs)); push(ap(Any((int_t)inst.kind)));
			push_ff(pc+inst.ISIZE, 1, 2, 0, null);
			ap(lhs)->rawsend(myself(), id_[Core::id_op_range], null);
		}
		XTAL_VM_CONTINUE(XTAL_VM_CHECK_EXCEPT_PC(ff().called_pc));
}

const inst_t* VMachine::FunClassBegin(const inst_t* pc){
		XTAL_VM_DEF_INST(ClassBegin);
		XTAL_GLOBAL_INTERPRETER_LOCK{
			ClassInfo* core = code()->class_info(inst.core_number);
			const FramePtr& outer = (core->flags&ClassInfo::FLAG_SCOPE_CHAIN) ? ff().outer() : unchecked_ptr_cast<Frame>(null);
			ClassPtr cp;

			switch(core->kind){
				XTAL_CASE(KIND_CLASS){
					cp = xnew<Class>(outer, code(), core);
				}

				XTAL_CASE(KIND_SINGLETON){
					cp = xnew<Singleton>(outer, code(), core);
				}
			}
			
			int_t n = core->mixins;
			for(int_t i = 0; i<n; ++i){
				cp->inherit_strict(ptr_cast<Class>(pop()));
			}

			push_ff(pc + inst.ISIZE, 0, 0, 0, cp);
			ff().fun(prev_fun());

			ff().outer(cp);
		}
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
}

const inst_t* VMachine::FunClassEnd(const inst_t* pc){
		XTAL_VM_DEF_INST(ClassEnd);
		if(raweq(ff().outer()->get_class(), ff().outer())){
			Singleton* singleton = (Singleton*)pvalue(ff().outer());
			singleton->init_singleton(myself());
		}

		push(ff().outer());
		ff().outer(ff().outer()->outer());
		pop_ff();
		XTAL_VM_CONTINUE(pc + inst.ISIZE);;;;;;
}

const inst_t* VMachine::FunMakeFun(const inst_t* pc){
		XTAL_VM_DEF_INST(MakeFun);
		int_t table_n = inst.core_number, end = inst.OFFSET_address + inst.address;
		XTAL_GLOBAL_INTERPRETER_LOCK{
			FunInfo* core = code()->fun_info(table_n);
			const FramePtr& outer = ff().outer();
			switch(core->kind){
				XTAL_NODEFAULT;
				XTAL_CASE(KIND_FUN){ push(xnew<Fun>(outer, ff().self(), code(), core)); }
				XTAL_CASE(KIND_LAMBDA){ push(xnew<Lambda>(outer, ff().self(), code(), core)); }
				XTAL_CASE(KIND_METHOD){ push(xnew<Method>(outer, code(), core)); }
				XTAL_CASE(KIND_FIBER){ push(xnew<Fiber>(outer, ff().self(), code(), core)); }
			}
		}
		XTAL_VM_CONTINUE(pc + end);
}

const inst_t* VMachine::FunMakeInstanceVariableAccessor(const inst_t* pc){
		XTAL_VM_DEF_INST(MakeInstanceVariableAccessor);
		XTAL_GLOBAL_INTERPRETER_LOCK{
			AnyPtr ret;
			switch(inst.type){
				XTAL_NODEFAULT;

				XTAL_CASE(0){ ret = xnew<InstanceVariableGetter>(inst.number, code()->class_info(inst.core_number)); }
				XTAL_CASE(1){ ret = xnew<InstanceVariableSetter>(inst.number, code()->class_info(inst.core_number)); }
			}
			push(ret);
		}
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
}

//}}FUNS}
	
const inst_t* VMachine::OpAdd(const inst_t* pc, int_t op){
	const AnyPtr& b = get(); uint_t btype = type(b)-TYPE_INT;
	const AnyPtr& a = get(1); uint_t atype = type(a)-TYPE_INT;
	uint_t abtype = atype | btype;
	if(abtype<2){
		Any ret;
		if(abtype==0){
			ret.set_i(ivalue(a) + ivalue(b));
		}
		else{
			float_t afvalue = atype==0 ? (float_t)ivalue(a) : fvalue(a);
			float_t bfvalue = btype==0 ? (float_t)ivalue(b) : fvalue(b);
			ret.set_f(afvalue + bfvalue);
		}
		set(1, ret);
		downsize(1); 
		return pc;	
	}
	else{
		return inner_send_from_stack(pc, 1, id_[op], 1, 0);
	}
}

const inst_t* VMachine::OpSub(const inst_t* pc, int_t op){
	const AnyPtr& b = get(); uint_t btype = type(b)-TYPE_INT;
	const AnyPtr& a = get(1); uint_t atype = type(a)-TYPE_INT;
	uint_t abtype = atype | btype;
	if(abtype<2){
		Any ret;
		if(abtype==0){
			ret.set_i(ivalue(a) - ivalue(b));
		}
		else{
			float_t afvalue = atype==0 ? (float_t)ivalue(a) : fvalue(a);
			float_t bfvalue = btype==0 ? (float_t)ivalue(b) : fvalue(b);
			ret.set_f(afvalue - bfvalue);
		}
		set(1, ret);
		downsize(1); 
		return pc;
	}
	else{
		return inner_send_from_stack(pc, 1, id_[op], 1, 0);
	}
}

const inst_t* VMachine::OpMul(const inst_t* pc, int_t op){
	const AnyPtr& b = get(); uint_t btype = type(b)-TYPE_INT;
	const AnyPtr& a = get(1); uint_t atype = type(a)-TYPE_INT;
	uint_t abtype = atype | btype;
	if(abtype<2){
		Any ret;
		if(abtype==0){
			ret.set_i(ivalue(a) * ivalue(b));
		}
		else{
			float_t afvalue = atype==0 ? (float_t)ivalue(a) : fvalue(a);
			float_t bfvalue = btype==0 ? (float_t)ivalue(b) : fvalue(b);
			ret.set_f(afvalue * bfvalue);
		}
		set(1, ret);
		downsize(1); 
		return pc;
	}
	else{
		return inner_send_from_stack(pc, 1, id_[op], 1, 0);
	}
}

const inst_t* VMachine::OpDiv(const inst_t* pc, int_t op){
	const AnyPtr& b = get(); uint_t btype = type(b)-TYPE_INT;
	const AnyPtr& a = get(1); uint_t atype = type(a)-TYPE_INT;
	uint_t abtype = atype | btype;
	if(abtype<2){
		Any ret;
		if(abtype==0){
			ret.set_i(ivalue(a) / check_zero(ivalue(b)));
		}
		else{
			float_t afvalue = atype==0 ? (float_t)ivalue(a) : fvalue(a);
			float_t bfvalue = btype==0 ? (float_t)ivalue(b) : fvalue(b);
			ret.set_f(afvalue / check_zero(bfvalue));
		}
		set(1, ret);
		downsize(1); 
		return pc;
	}
	else{
		return inner_send_from_stack(pc, 1, id_[op], 1, 0);
	}
}

const inst_t* VMachine::OpMod(const inst_t* pc, int_t op){
	const AnyPtr& b = get(); uint_t btype = type(b)-TYPE_INT;
	const AnyPtr& a = get(1); uint_t atype = type(a)-TYPE_INT;
	uint_t abtype = atype | btype;
	if(abtype<2){
		Any ret;
		if(abtype==0){
			ret.set_i(ivalue(a) % check_zero(ivalue(b)));
		}
		else{
			float_t afvalue = atype==0 ? (float_t)ivalue(a) : fvalue(a);
			float_t bfvalue = btype==0 ? (float_t)ivalue(b) : fvalue(b);
			ret.set_f(fmodf(afvalue, check_zero(bfvalue)));
		}
		set(1, ret);
		downsize(1); 
		return pc;
	}
	else{
		return inner_send_from_stack(pc, 1, id_[op], 1, 0);
	}
}

const inst_t* VMachine::OpAnd(const inst_t* pc, int_t op){
	const AnyPtr& b = get(); uint_t btype = type(b)-TYPE_INT;
	const AnyPtr& a = get(1); uint_t atype = type(a)-TYPE_INT;
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
	const AnyPtr& b = get(); uint_t btype = type(b)-TYPE_INT;
	const AnyPtr& a = get(1); uint_t atype = type(a)-TYPE_INT;
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
	const AnyPtr& b = get(); uint_t btype = type(b)-TYPE_INT;
	const AnyPtr& a = get(1); uint_t atype = type(a)-TYPE_INT;
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
	const AnyPtr& b = get(); uint_t btype = type(b)-TYPE_INT;
	const AnyPtr& a = get(1); uint_t atype = type(a)-TYPE_INT;
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
	const AnyPtr& b = get(); uint_t btype = type(b)-TYPE_INT;
	const AnyPtr& a = get(1); uint_t atype = type(a)-TYPE_INT;
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
	const AnyPtr& b = get(); uint_t btype = type(b)-TYPE_INT;
	const AnyPtr& a = get(1); uint_t atype = type(a)-TYPE_INT;
	uint_t abtype = atype | btype;
	if(abtype==0){
		set(1, Any((uint_t)ivalue(a) >> ivalue(b)));
		downsize(1);
		return pc;
	}
	else{
		return inner_send_from_stack(pc, 1, id_[op], 1, 0);
	}
}

const inst_t* VMachine::OpInc(const inst_t* pc, int_t op){
	const AnyPtr& a = get(); uint_t atype = type(a)-TYPE_INT;
	if(atype<2){
		if(atype==0){
			set(Any(ivalue(a)+1));
		}
		else{
			set(Any(fvalue(a)+1));
		}
		return pc;
	}
	else{
		return inner_send_from_stack(pc, 1, id_[op], 0);
	}
}

const inst_t* VMachine::OpDec(const inst_t* pc, int_t op){
	const AnyPtr& a = get(); uint_t atype = type(a)-TYPE_INT;
	if(atype<2){
		if(atype==0){
			set(Any(ivalue(a)-1));
		}
		else{
			set(Any(fvalue(a)-1));
		}
		return pc;
	}
	else{
		return inner_send_from_stack(pc, 1, id_[op], 0);
	}
}

void VMachine::carry_over(Fun* fun){
	FunFrame& f = ff();
	
	f.fun(fun);
	f.outer(fun->outer());
	f.variables_.clear();
	f.called_pc = fun->source();
	f.yieldable = f.poped_pc==&end_code_ ? false : prev_ff().yieldable;
	
	if(type(f.self())==TYPE_BASE){
		f.instance_variables = pvalue(f.self())->instance_variables();
	}

	if(fun->extendable_param()){
		f.arguments(make_args(fun));
	}
	
	FunInfo* info = fun->info();
	if(int_t size = info->variable_size){
		if(info->flags & FunInfo::FLAG_ON_HEAP){
			f.outer(xnew<Frame>(f.outer(), fun->code(), info));
			Frame* frame = f.outer().get();
			for(int_t n = 0; n<size; ++n){
				frame->set_member_direct(n, arg(n, fun));
			}
		}
		else{
			f.variables_.upsize(size);
			Any* vars=&f.variables_[size-1];
			for(int_t n = 0; n<size; ++n){
				vars[n] = arg(n, fun);
			}
		}
	}
	
	int_t max_stack = info->max_stack;
	stack_.upsize(max_stack);
	stack_.downsize(max_stack + f.args_stack_size());
	f.ordered_arg_count = f.named_arg_count = 0;

	debug_enable_ = debug_ && debug_->is_enabled();
	if(debug_enable_){
		debug_hook(f.called_pc, BREAKPOINT_CALL);
	}
}

void VMachine::mv_carry_over(Fun* fun){
	FunFrame& f = ff();
	
	f.fun(fun);
	f.outer(fun->outer());
	f.variables_.clear();
	f.called_pc = fun->source();
	f.yieldable = f.poped_pc==&end_code_ ? false : prev_ff().yieldable;

	if(type(f.self())==TYPE_BASE){
		f.instance_variables = pvalue(f.self())->instance_variables();
	}

	if(fun->extendable_param()){
		f.arguments(make_args(fun));
	}
	
	// 名前付き引数は邪魔
	stack_.downsize(f.named_arg_count*2);

	FunInfo* core = fun->info();
	int_t size = core->variable_size;
	adjust_result(f.ordered_arg_count, size);
	f.ordered_arg_count = size;
	f.named_arg_count = 0;
	
	if(size){
		if(core->flags & FunInfo::FLAG_ON_HEAP){
			f.outer(xnew<Frame>(f.outer(), fun->code(), core));
			Frame* frame = f.outer().get();
			for(int_t n = 0; n<size; ++n){
				frame->set_member_direct(n, get(size-1-n));
			}
		}
		else{
			f.variables_.upsize(size);	
			Any* vars=&f.variables_[size-1];
			for(int_t n = 0; n<size; ++n){
				vars[n] = get(size-1-n);
			}
		}
	}	
	
	int_t max_stack = core->max_stack;
	stack_.upsize(max_stack);
	stack_.downsize(max_stack + size);
	f.ordered_arg_count = 0;

	debug_enable_ = debug_ && debug_->is_enabled();
	if(debug_enable_){
		debug_hook(f.called_pc, BREAKPOINT_CALL);
	}
}

void VMachine::adjust_result(int_t n, int_t need_result_count){

	// 戻り値の数と要求している戻り値の数が等しい
	if(need_result_count==n){
		return;
	}

	// 戻り値なんて要求してない
	if(need_result_count==0){
		downsize(n);
		return;
	}

	// 戻り値が一つも無いのでundefinedで埋める
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
			MultiValuePtr ret(xnew<MultiValue>(size));
			for(int_t i=0; i<size; ++i){
				ret->set_at(i, get(size-1-i));
			}
			downsize(size);
			push(ret);
		}	
	}
	else{
		// 要求している戻り値の数の方が、関数が返す戻り値より多い

		XTAL_GLOBAL_INTERPRETER_LOCK{
			if(const MultiValuePtr& temp = xtal::ptr_as<MultiValue>(get())){
				// 最後の要素の多値を展開し埋め込む
				MultiValuePtr mv(temp);
				downsize(1);

				int_t len = mv->size();
				for(int_t i=0; i<len; ++i){
					push(mv->at(i));
				}

				adjust_result(len-1, need_result_count-n);
			}
			else{
				// 最後の要素が配列ではないので、undefinedで埋めとく
				for(int_t i = n; i<need_result_count; ++i){
					push(undefined);
				}
			}
		}
	}
}

}//namespace

