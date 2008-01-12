#include "xtal.h"
#include "xtal_macro.h"

namespace xtal{

inline const IDPtr& isp(const Innocence& v){
	return (const IDPtr&)v;
}

template<class T>
inline T check_zero(T value){
	if(value==0){
		XTAL_THROW(builtin()->member("RuntimeError")(Xt("Xtal Runtime Error 1024")), return 1);
	}
	return value;
}

VMachine::VMachine(){	
	myself_ = this;

	stack_.reserve(32);

	end_code_ = InstExit::NUMBER;
	throw_unsupported_error_code_ = InstThrowUnsupportedError::NUMBER;
	check_unsupported_code_ = InstCheckUnsupported::NUMBER;
	cleanup_call_code_ = InstCleanupCall::NUMBER;
	throw_undefined_code_ = InstThrowUndefined::NUMBER;
	resume_pc_ = 0;
}

void VMachine::setup_call(int_t need_result_count){
	push_ff(&end_code_, need_result_count, 0, 0, null);
}

void VMachine::setup_call(int_t need_result_count, const Param& a1){
	push_ff(&end_code_, need_result_count, 0, 0, null);
	push_arg(a1);
}

void VMachine::setup_call(int_t need_result_count, const Param& a1, const Param& a2){
	push_ff(&end_code_, need_result_count, 0, 0, null);
	push_arg(a1); 
	push_arg(a2);
}

void VMachine::setup_call(int_t need_result_count, const Param& a1, const Param& a2, const Param& a3){
	push_ff(&end_code_, need_result_count, 0, 0, null);
	push_arg(a1); 
	push_arg(a2); 
	push_arg(a3);
}

void VMachine::setup_call(int_t need_result_count, const Param& a1, const Param& a2, const Param& a3, const Param& a4){
	push_ff(&end_code_, need_result_count, 0, 0, null);
	push_arg(a1); 
	push_arg(a2); 
	push_arg(a3); 
	push_arg(a4);
}

void VMachine::setup_call(int_t need_result_count, const Param& a1, const Param& a2, const Param& a3, const Param& a4, const Param& a5){
	push_ff(&end_code_, need_result_count, 0, 0, null);
	push_arg(a1); 
	push_arg(a2); 
	push_arg(a3); 
	push_arg(a4);
	push_arg(a5);
}

void VMachine::push_arg(const AnyPtr& value){
	XTAL_ASSERT(named_arg_count() == 0);
	ff().ordered_arg_count++;
	push(value);
}
	
void VMachine::push_arg(const IDPtr& name, const AnyPtr& value){
	ff().named_arg_count++;
	push(name);
	push(value);
}
	
void VMachine::push_arg(const Param& value){ 
	if(value.name){ 
		push_arg(value.name, value.value); 
	}
	else{ 
		push_arg(value.value); 
	} 
}

void VMachine::push_ordered_args(const ArrayPtr& p){ 
	Xfor(v, p){
		push_arg(v);
	}
}

void VMachine::push_named_args(const MapPtr& p){ 
	Xfor2(k, v, p){
		push_arg(cast<IDPtr>(k), v);
	}
}
	
const AnyPtr& VMachine::result(int_t pos){
	const inst_t* temp;
	{
		FunFrame& f = ff();

		if(*f.called_pc==InstCleanupCall::NUMBER){
			if(pos<f.need_result_count){
				return get(f.need_result_count-pos-1);
			}
			else{
				return undefined;
			}
		}

		temp = f.poped_pc;
		f.poped_pc = &end_code_;
		execute_inner(f.called_pc);
	}

	fun_frames_.upsize(1);

	{
		FunFrame& f = ff();

		f.poped_pc = temp;
		f.called_pc = &cleanup_call_code_;

		if(pos<f.need_result_count){
			return get(f.need_result_count-pos-1);
		}
		else{
			return undefined;
		}
	}
}
	
AnyPtr VMachine::result_and_cleanup_call(int_t pos){
	const AnyPtr& ret = result(pos);
	downsize(ff().need_result_count);
	pop_ff();
	return ret;
}
	
void VMachine::cleanup_call(){
	result(0);
	downsize(ff().need_result_count);
	pop_ff();
}

const AnyPtr& VMachine::arg(int_t pos){
	FunFrame& f = ff();
	if(pos<f.ordered_arg_count)
		return get(f.args_stack_size()-1-pos);
	return undefined;
}

const AnyPtr& VMachine::arg(const IDPtr& name){
	FunFrame& f = ff();
	for(int_t i = 0, sz = f.named_arg_count; i<sz; ++i){
		if(raweq(get(sz*2-1-(i*2+0)), name)){
			return get(sz*2-1-(i*2+1));
		}
	}
	return undefined;
}

const AnyPtr& VMachine::arg(int_t pos, const IDPtr& name){
	FunFrame& f = ff();
	if(pos<f.ordered_arg_count){
		return get(f.args_stack_size()-1-pos);
	}
	return arg(name);
}

const AnyPtr& VMachine::arg(int_t pos, Fun* names){
	FunFrame& f = ff();
	if(pos<f.ordered_arg_count){
		return get(f.args_stack_size()-1-pos);
	}
	return arg(names->param_name_at(pos));
}

const AnyPtr& VMachine::arg_default(int_t pos, const AnyPtr& def){
	FunFrame& f = ff();
	if(pos<f.ordered_arg_count){
		return get(f.args_stack_size()-1-pos);
	}
	return def;
}
	
const AnyPtr& VMachine::arg_default(const IDPtr& name, const AnyPtr& def){
	FunFrame& f = ff();
	for(int_t i = 0, sz = f.named_arg_count; i<sz; ++i){
		if(raweq(get(sz*2-1-(i*2+0)), name)){
			return get(sz*2-1-(i*2+1));
		}
	}
	return def;
}
	
const AnyPtr& VMachine::arg_default(int_t pos, const IDPtr& name, const AnyPtr& def){
	FunFrame& f = ff();
	if(pos<f.ordered_arg_count){
		return get(f.args_stack_size()-1-pos);
	}
	return arg_default(name, def);
}

void VMachine::return_result(){
	FunFrame& f = ff();

	downsize(f.args_stack_size());
	for(int_t i=0, sz=f.need_result_count; i<sz; ++i){
		push(undefined);
	}

	f.called_pc = &cleanup_call_code_;
}

void VMachine::return_result(const AnyPtr& value1){
	FunFrame& f = ff();

	downsize(f.args_stack_size());
	push(value1);
	if(f.need_result_count!=1){
		adjust_result(1);
	}

	f.called_pc = &cleanup_call_code_;
}
	
void VMachine::return_result(const AnyPtr& value1, const AnyPtr& value2){
	FunFrame& f = ff();

	downsize(f.args_stack_size());
	push(value1);
	push(value2);
	adjust_result(2);

	f.called_pc = &cleanup_call_code_;
}

void VMachine::return_result(const AnyPtr& value1, const AnyPtr& value2, const AnyPtr& value3){
	FunFrame& f = ff();

	downsize(f.args_stack_size());
	push(value1);
	push(value2);
	push(value3);
	adjust_result(3);

	f.called_pc = &cleanup_call_code_;
}
	
void VMachine::return_result(const AnyPtr& value1, const AnyPtr& value2, const AnyPtr& value3, const AnyPtr& value4){
	FunFrame& f = ff();

	downsize(f.args_stack_size());
	push(value1);
	push(value2);
	push(value3);
	push(value4);
	adjust_result(4);

	f.called_pc = &cleanup_call_code_;
}

void VMachine::return_result_array(const ArrayPtr& values){
	FunFrame& f = ff();

	downsize(f.args_stack_size());
	int_t size = values->size();
	for(int_t i=0; i<size; ++i){
		push(values->at(i));
	}
	adjust_result(size);

	f.called_pc = &cleanup_call_code_;
}

void VMachine::replace_result(int_t pos, const AnyPtr& v){
	result(0);
	set(ff().need_result_count-pos-1, v);
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
				vm->push(FiberPtr(fun));
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
}

const VMachinePtr& VMachine::inner_setup_call(const inst_t* pc, int_t need_result_count){
	push_ff(pc, need_result_count, 0, 0, null);
	return myself();
}

const VMachinePtr& VMachine::inner_setup_call(const inst_t* pc, int_t need_result_count, const AnyPtr& a1){
	push(a1);
	push_ff(pc, need_result_count, 1, 0, null);
	return myself();
}

const VMachinePtr& VMachine::inner_setup_call(const inst_t* pc, int_t need_result_count, const AnyPtr& a1, const AnyPtr& a2){
	push(a1); push(a2);
	push_ff(pc, need_result_count, 2, 0, null);
	return myself();
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

const inst_t* VMachine::send1(const inst_t* pc, const IDPtr& name){
	XTAL_GLOBAL_INTERPRETER_LOCK{
		ff().temp3_ = name;
		ff().temp2_ = null;
		Innocence target = pop_and_save1();
		Innocence self = ff().self();
		push_ff(pc, 1, 0, 0, ap(self));
		ap(target)->rawsend(myself(), name, null, ff().self());
	}
	return ff().called_pc;
}

const inst_t* VMachine::send2(const inst_t* pc, const IDPtr& name){
	XTAL_GLOBAL_INTERPRETER_LOCK{
		ff().temp3_ = name;
		ff().temp2_ = get()->get_class();
		const AnyPtr& temp = pop();
		Innocence target = pop_and_save1();
		push(temp);
		Innocence self = ff().self();
		push_ff(pc, 1, 1, 0, ap(self));
		ap(target)->rawsend(myself(), name, get()->get_class(), ff().self());
	}
	return ff().called_pc;
}

const inst_t* VMachine::send2_r(const inst_t* pc, const IDPtr& name){
	XTAL_GLOBAL_INTERPRETER_LOCK{
		ff().temp3_ = name;
		ff().temp2_ = get()->get_class();
		Innocence target = pop_and_save1();
		Innocence self = ff().self();
		push_ff(pc, 1, 1, 0, ap(self));
		ap(target)->rawsend(myself(), name, get()->get_class(), ff().self());
	}
	return ff().called_pc;
}

const inst_t* VMachine::send2_q(const inst_t* pc, const IDPtr& name){
	XTAL_GLOBAL_INTERPRETER_LOCK{
		ff().temp3_ = name;
		ff().temp2_ = get(1)->get_class();
		const AnyPtr& temp = pop();
		Innocence target = pop_and_save1();
		push(temp);
		Innocence self = ff().self();
		push_ff(pc, 1, 1, 0, ap(self));
		ff().called_pc = &check_unsupported_code_;
		ap(target)->rawsend(myself(), name, get()->get_class(), ff().self());
	}
	return ff().called_pc;
}

void VMachine::return_result_instance_variable(int_t number, ClassCore* core){
	return_result((ff().instance_variables->variable(number, core)));
}

#define XTAL_VM_NODEFAULT } XTAL_ASSERT(false); XTAL_NODEFAULT
#define XTAL_VM_CASE_FIRST(key) case Inst##key::NUMBER: { Inst##key& inst = *(Inst##key*)pc;
#define XTAL_VM_CASE(key) } XTAL_ASSERT(false); case Inst##key::NUMBER: /*printf("%s\n", #key);*/ { typedef Inst##key Inst; Inst& inst = *(Inst*)pc;
#define XTAL_VM_SWITCH switch(*pc)
#define XTAL_VM_DEF_INST(key) typedef Inst##key Inst; Inst& inst = *(Inst*)pc
#define XTAL_VM_CONTINUE(x) pc = (x); goto begin
#define XTAL_VM_EXCEPT(e) except_[0] = e; goto except_catch
#define XTAL_VM_RETURN return

#ifdef XTAL_NO_EXCEPTIONS
#	define XTAL_VM_CHECK_EXCEPT if(ap(except_[0])){ goto except_catch; }
#else
#	define XTAL_VM_CHECK_EXCEPT
#endif

void VMachine::execute_inner(const inst_t* start){

	const inst_t* pc = start;

	int_t stack_size = stack_.size() - ff().args_stack_size();
	int_t fun_frames_size = fun_frames_.size();

XTAL_GLOBAL_INTERPRETER_UNLOCK{
retry:
XTAL_TRY{
begin:

// C++の例外機能を使わない場合、計算毎に例外が発生していないか調べる。
XTAL_VM_CHECK_EXCEPT;

XTAL_VM_SWITCH{

//{OPS{{
	XTAL_VM_CASE_FIRST(Undefined){ // 2
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
		push(Innocence(true)); 
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(PushFalse){ // 3
		push(Innocence(false)); 
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(PushInt1Byte){ // 3
		push(Innocence((int_t)inst.value)); 
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(PushInt2Byte){ // 3
		push(Innocence((int_t)inst.value)); 
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(PushFloat1Byte){ // 3
		push(Innocence((float_t)inst.value)); 
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(PushFloat2Byte){ // 3
		push(Innocence((float_t)inst.value)); 
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
		Innocence temp = get(); 
		set(get(1)); 
		set(1, temp); 
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(Insert2){ // 6
		Innocence temp = get(); 
		set(get(1)); 
		set(1, get(2)); 
		set(2, temp); 
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(Insert3){ XTAL_VM_CONTINUE(FunInsert3(pc)); /*
		Innocence temp = get(); 
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
		XTAL_VM_CONTINUE(pc + (pop() ? inst.ISIZE : inst.address));
	}

	XTAL_VM_CASE(Unless){ // 2
		XTAL_VM_CONTINUE(pc + (!pop() ? inst.ISIZE : inst.address));
	}

	XTAL_VM_CASE(Goto){ // 2
		XTAL_VM_CONTINUE(pc + inst.address); 
	}

	XTAL_VM_CASE(LocalVariableInc){ XTAL_VM_CONTINUE(FunLocalVariableInc(pc)); /*
		Innocence a = local_variable(inst.number);
		switch(type(a)){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ set_local_variable(inst.number, Innocence(ivalue(a)+1)); XTAL_VM_CONTINUE(pc + inst.ISIZE + InstSetLocalVariable1Byte::ISIZE); }
			XTAL_CASE(TYPE_FLOAT){ set_local_variable(inst.number, Innocence(fvalue(a)+1)); XTAL_VM_CONTINUE(pc + inst.ISIZE + InstSetLocalVariable1Byte::ISIZE); }
		}
		ap(a)->rawsend(inner_setup_call(pc + inst.ISIZE, 1), Xid(op_inc));
		XTAL_VM_CONTINUE(ff().called_pc);
	}*/ }

	XTAL_VM_CASE(LocalVariableIncDirect){ // 9
		Innocence& a = ff().variables_[inst.number];
		switch(type(a)){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ a = Innocence(ivalue(a)+1); XTAL_VM_CONTINUE(pc + inst.ISIZE + InstSetLocalVariable1ByteDirect::ISIZE); }
			XTAL_CASE(TYPE_FLOAT){ a = Innocence(fvalue(a)+1); XTAL_VM_CONTINUE(pc + inst.ISIZE + InstSetLocalVariable1ByteDirect::ISIZE); }
		}
		ap(a)->rawsend(inner_setup_call(pc + inst.ISIZE, 1), Xid(op_inc));
		XTAL_VM_CONTINUE(ff().called_pc);
	}

	XTAL_VM_CASE(LocalVariableDec){ XTAL_VM_CONTINUE(FunLocalVariableDec(pc)); /*
		Innocence a = local_variable(inst.number);
		switch(type(a)){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ set_local_variable(inst.number, Innocence(ivalue(a)-1)); XTAL_VM_CONTINUE(pc + inst.ISIZE + InstSetLocalVariable1Byte::ISIZE); }
			XTAL_CASE(TYPE_FLOAT){ set_local_variable(inst.number, Innocence(fvalue(a)-1)); XTAL_VM_CONTINUE(pc + inst.ISIZE + InstSetLocalVariable1Byte::ISIZE); }
		}
		ap(a)->rawsend(inner_setup_call(pc + inst.ISIZE, 1), Xid(op_dec));
		XTAL_VM_CONTINUE(ff().called_pc);
	}*/ }

	XTAL_VM_CASE(LocalVariableDecDirect){ // 9
		Innocence& a = ff().variables_[inst.number];
		switch(type(a)){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ a = Innocence(ivalue(a)-1); XTAL_VM_CONTINUE(pc + inst.ISIZE + InstSetLocalVariable1ByteDirect::ISIZE); }
			XTAL_CASE(TYPE_FLOAT){ a = Innocence(fvalue(a)-1); XTAL_VM_CONTINUE(pc + inst.ISIZE + InstSetLocalVariable1ByteDirect::ISIZE); }
		}
		ap(a)->rawsend(inner_setup_call(pc + inst.ISIZE, 1), Xid(op_dec));
		XTAL_VM_CONTINUE(ff().called_pc);
	}

	XTAL_VM_CASE(LocalVariableInc2Byte){ XTAL_VM_CONTINUE(FunLocalVariableInc2Byte(pc)); /*
		Innocence a = local_variable(inst.number);
		switch(type(a)){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ set_local_variable(inst.number, Innocence(ivalue(a)+1)); XTAL_VM_CONTINUE(pc + inst.ISIZE + InstSetLocalVariable2Byte::ISIZE); }
			XTAL_CASE(TYPE_FLOAT){ set_local_variable(inst.number, Innocence(fvalue(a)+1)); XTAL_VM_CONTINUE(pc + inst.ISIZE + InstSetLocalVariable2Byte::ISIZE); }
		}
		ap(a)->rawsend(inner_setup_call(pc + inst.ISIZE, 1), Xid(op_inc));
		XTAL_VM_CONTINUE(ff().called_pc);
	}*/ }

	XTAL_VM_CASE(LocalVariableDec2Byte){ XTAL_VM_CONTINUE(FunLocalVariableDec2Byte(pc)); /*
		Innocence a = local_variable(inst.number);
		switch(type(a)){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ set_local_variable(inst.number, Innocence(ivalue(a)-1)); XTAL_VM_CONTINUE(pc + inst.ISIZE + InstSetLocalVariable2Byte::ISIZE); }
			XTAL_CASE(TYPE_FLOAT){ set_local_variable(inst.number, Innocence(fvalue(a)-1)); XTAL_VM_CONTINUE(pc + inst.ISIZE + InstSetLocalVariable2Byte::ISIZE); }
		}
		ap(a)->rawsend(inner_setup_call(pc + inst.ISIZE, 1), Xid(op_dec));
		XTAL_VM_CONTINUE(ff().called_pc);
	}*/ }

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
			push(f.instance_variables->variable(inst.number, f.fun()->code()->class_core(inst.core_number)));
		}
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(SetInstanceVariable){ // 5
		FunFrame& f = ff();
		XTAL_GLOBAL_INTERPRETER_LOCK{ 
			f.instance_variables->set_variable(inst.number, f.fun()->code()->class_core(inst.core_number), get());
			downsize(1);
		}
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(CleanupCall){ // 2
		XTAL_VM_CONTINUE(pop_ff());
	}

	XTAL_VM_CASE(Return){ // 3
		if(ff().need_result_count!=inst.results){
			adjust_result(inst.results); 
		}
		XTAL_VM_CONTINUE(pop_ff());  
	}

	XTAL_VM_CASE(Yield){ // 7
		yield_result_count_ = inst.results;	
		if(ff().yieldable){
			resume_pc_ = pc + inst.ISIZE;
			XTAL_VM_RETURN;
		}
		else{
			downsize(yield_result_count_);
			XTAL_GLOBAL_INTERPRETER_LOCK{ 
				except_[0] = builtin()->member("YieldError")(Xt("Xtal Runtime Error 1012"));
			}
			XTAL_VM_EXCEPT(except_[0]);
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
			ff().temp2_ = null;
			Innocence primary_key = identifier_ex(inst.identifier_number);
			Innocence self = ff().self();
			Innocence target = pop_and_save1();
			push_ff(pc + inst.ISIZE, 1, 0, 0, ap(self));
			ap(target)->rawsend(myself(), isp(primary_key), null, ff().self());
		}
		XTAL_VM_CONTINUE(ff().called_pc); 	
	}*/ }

	XTAL_VM_CASE(PropertyNS){ XTAL_VM_CONTINUE(FunPropertyNS(pc)); /*
		XTAL_GLOBAL_INTERPRETER_LOCK{
			Innocence secondary_key = pop_and_save2();
			Innocence primary_key = identifier_ex(inst.identifier_number);
			Innocence self = ff().self();
			Innocence target = pop_and_save1();
			push_ff(pc + inst.ISIZE, 1, 0, 0, ap(self));
			ap(target)->rawsend(myself(), isp(primary_key), ap(secondary_key), ff().self());
		}
		XTAL_VM_CONTINUE(ff().called_pc); 	
	}*/ }

	XTAL_VM_CASE(SetProperty){ XTAL_VM_CONTINUE(FunSetProperty(pc)); /*
		XTAL_GLOBAL_INTERPRETER_LOCK{
			ff().temp2_ = null;
			Innocence primary_key = identifier_ex(inst.identifier_number);
			Innocence self = ff().self();
			Innocence target = pop_and_save1();
			push_ff(pc + inst.ISIZE, 0, 1, 0, ap(self));
			ap(target)->rawsend(myself(), isp(primary_key), null, ff().self());
		}
		XTAL_VM_CONTINUE(ff().called_pc); 	
	}*/ }

	XTAL_VM_CASE(SetPropertyNS){ XTAL_VM_CONTINUE(FunSetPropertyNS(pc)); /*
		XTAL_GLOBAL_INTERPRETER_LOCK{
			Innocence secondary_key = pop_and_save2();
			Innocence primary_key = identifier_ex(inst.identifier_number);
			Innocence self = ff().self();
			Innocence target = pop_and_save1();
			push_ff(pc + inst.ISIZE, 0, 1, 0, ap(self));
			ap(target)->rawsend(myself(), isp(primary_key), ap(secondary_key), ff().self());
		}
		XTAL_VM_CONTINUE(ff().called_pc); 	
	}*/ }

	XTAL_VM_CASE(Call){ XTAL_VM_CONTINUE(FunCall(pc)); /*
		XTAL_GLOBAL_INTERPRETER_LOCK{
			ff().temp2_ = null;
			ff().temp3_ = Xid(op_call);
			Innocence self = ff().self();
			Innocence target = pop_and_save1();
			push_ff(pc + inst.ISIZE, (InstCall&)inst, ap(self));
			ap(target)->call(myself());
		}
		XTAL_VM_CONTINUE(ff().called_pc);	
	}*/ }

	XTAL_VM_CASE(Send){ XTAL_VM_CONTINUE(FunSend(pc)); /*
		XTAL_GLOBAL_INTERPRETER_LOCK{
			ff().temp2_ = null;
			Innocence primary_key = identifier_ex(inst.identifier_number);
			Innocence self = ff().self();
			Innocence target = pop_and_save1();
			push_ff(pc + inst.ISIZE, (InstCall&)inst, ap(self));
			ap(target)->rawsend(myself(), isp(primary_key), null, ff().self());
		}
		XTAL_VM_CONTINUE(ff().called_pc); 	
	}*/ }

	XTAL_VM_CASE(SendNS){ XTAL_VM_CONTINUE(FunSendNS(pc)); /*
		XTAL_GLOBAL_INTERPRETER_LOCK{
			Innocence secondary_key = pop_and_save2();
			Innocence primary_key = identifier_ex(inst.identifier_number);
			Innocence self = ff().self();
			Innocence target = pop_and_save1();
			push_ff(pc + inst.ISIZE, (InstCall&)inst, ap(self));
			ap(target)->rawsend(myself(), isp(primary_key), ap(secondary_key), ff().self());
		}
		XTAL_VM_CONTINUE(ff().called_pc); 	
	}*/ }

	XTAL_VM_CASE(SendQ){ XTAL_VM_CONTINUE(FunSendQ(pc)); /*
		XTAL_GLOBAL_INTERPRETER_LOCK{
			ff().temp2_ = null;
			Innocence primary_key = identifier_ex(inst.identifier_number);
			Innocence self = ff().self();
			Innocence target = pop_and_save1();
			push_ff(pc + inst.ISIZE, (InstCall&)inst, ap(self));
			ff().called_pc = &check_unsupported_code_;
			ap(target)->rawsend(myself(), isp(primary_key), null, ff().self());
		}
		XTAL_VM_CONTINUE(ff().called_pc); 	
	}*/ }

	XTAL_VM_CASE(SendQNS){ XTAL_VM_CONTINUE(FunSendQNS(pc)); /*
		XTAL_GLOBAL_INTERPRETER_LOCK{
			Innocence secondary_key = pop_and_save2();
			Innocence primary_key = identifier_ex(inst.identifier_number);
			Innocence self = ff().self();
			Innocence target = pop_and_save1();
			push_ff(pc + inst.ISIZE, (InstCall&)inst, ap(self));
			ff().called_pc = &check_unsupported_code_;
			ap(target)->rawsend(myself(), isp(primary_key), ap(secondary_key), ff().self());
		}
		XTAL_VM_CONTINUE(ff().called_pc); 	
	}*/ }

	XTAL_VM_CASE(Member){ // 8
		XTAL_GLOBAL_INTERPRETER_LOCK{
			Innocence primary_key = identifier_ex(inst.identifier_number);
			Innocence target = get();
			Innocence ret = ap(target)->member(isp(primary_key), null, ff().self());
			XTAL_VM_CHECK_EXCEPT;
			if(rawne(ret, undefined)){
				set(ret);
			}
			else{
				XTAL_VM_EXCEPT(unsupported_error(ap(target), isp(primary_key), null));
			}
		}
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(MemberNS){ // 9
		XTAL_GLOBAL_INTERPRETER_LOCK{
			Innocence secondary_key = pop_and_save2();
			Innocence primary_key = identifier_ex(inst.identifier_number);
			Innocence target = get();
			Innocence ret = ap(target)->member(isp(primary_key), ap(secondary_key), ff().self());
			XTAL_VM_CHECK_EXCEPT;
			if(rawne(ret, undefined)){
				set(ret);
			}
			else{
				XTAL_VM_EXCEPT(unsupported_error(ap(target), isp(primary_key), ap(secondary_key)));
			}
		}
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(MemberQ){ // 5
		XTAL_GLOBAL_INTERPRETER_LOCK{
			Innocence primary_key = identifier_ex(inst.identifier_number);
			Innocence target = get();
			set(ap(target)->member(isp(primary_key), null, ff().self()));
		}
		XTAL_VM_CONTINUE(pc + inst.ISIZE);  
	}

	XTAL_VM_CASE(MemberQNS){ // 6
		XTAL_GLOBAL_INTERPRETER_LOCK{
			Innocence secondary_key = pop_and_save2();
			Innocence primary_key = identifier_ex(inst.identifier_number);
			Innocence target = get();
			set(ap(target)->member(isp(primary_key), ap(secondary_key), ff().self()));		
		}
		XTAL_VM_CONTINUE(pc + inst.ISIZE);  
	}

	XTAL_VM_CASE(DefineMember){ // 7
		XTAL_GLOBAL_INTERPRETER_LOCK{
			Innocence primary_key = identifier_ex(inst.identifier_number);
			Innocence value = get();
			Innocence target = get(1);
			ap(target)->def(isp(primary_key), ap(value));
			downsize(2);
		}
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(DefineMemberNS){ XTAL_VM_CONTINUE(FunDefineMemberNS(pc)); /*
		XTAL_GLOBAL_INTERPRETER_LOCK{
			Innocence secondary_key = get();
			Innocence primary_key = identifier_ex(inst.identifier_number);
			Innocence value = get(1);
			Innocence target = get(2);
			ap(target)->def(isp(primary_key), ap(value), ap(secondary_key), KIND_PUBLIC); 
			downsize(3);
		}
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}*/ }

	XTAL_VM_CASE(GlobalVariable){ // 11
		XTAL_GLOBAL_INTERPRETER_LOCK{
			Innocence ret = code()->filelocal()->member(identifier(inst.identifier_number));
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
				XTAL_VM_CONTINUE(pc + inst.address);
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
		FunFrame& f = ff(); 
		XTAL_GLOBAL_INTERPRETER_LOCK{
			ScopeCore* core = f.fun()->code()->scope_core(inst.core_number);
			const FramePtr& outer = (core->flags&ScopeCore::FLAG_SCOPE_CHAIN) ? ff().outer() : static_ptr_cast<Frame>(null);
			f.outer(xnew<Frame>(outer, code(), core));
		}
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}

	XTAL_VM_CASE(BlockBeginDirect){ // 4
		FunFrame& f = ff(); 
		f.variables_.upsize(f.fun()->code()->scope_core(inst.core_number)->variable_size);
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}

	XTAL_VM_CASE(BlockEnd){ // 3
		ff().outer(ff().outer()->outer()); 
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}

	XTAL_VM_CASE(BlockEndDirect){ // 4
		FunFrame& f = ff();
		f.variables_.downsize(f.fun()->code()->scope_core(inst.core_number)->variable_size);
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}

	XTAL_VM_CASE(TryBegin){ // 9
		FunFrame& f = ff(); 
		ExceptFrame& ef = except_frames_.push();
		ef.core = &f.fun()->code()->except_core_table_[inst.core_number];
		ef.fun_frame_count = fun_frames_.size();
		ef.stack_count = this->stack_size();
		ef.variable_size = f.variables_.size();
		ef.outer = f.outer();
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}

	XTAL_VM_CASE(TryEnd){ // 2
		XTAL_VM_CONTINUE(except_frames_.pop().core->finally_pc + code()->data()); 
	}

	XTAL_VM_CASE(PushGoto){ // 3
		push(Innocence((int_t)((pc+inst.address)-code()->data()))); 
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(PopGoto){ // 2
		XTAL_VM_CONTINUE(code()->data()+ivalue(pop()));
	}

	XTAL_VM_CASE(IfEq){ XTAL_VM_CONTINUE(FunIfEq(pc)); /*
		switch(type(get(1))){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ pc = ivalue(get(1)) == ivalue(get()) ? pc+inst.ISIZE+InstIf::ISIZE : pc+inst.address; downsize(2); XTAL_VM_CONTINUE(pc); }
				XTAL_CASE(TYPE_FLOAT){ pc = ivalue(get(1)) == fvalue(get()) ? pc+inst.ISIZE+InstIf::ISIZE : pc+inst.address; downsize(2); XTAL_VM_CONTINUE(pc); }
			}}
			XTAL_CASE(TYPE_FLOAT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ pc = fvalue(get(1)) == ivalue(get()) ? pc+inst.ISIZE+InstIf::ISIZE : pc+inst.address; downsize(2); XTAL_VM_CONTINUE(pc); }
				XTAL_CASE(TYPE_FLOAT){ pc = fvalue(get(1)) == fvalue(get()) ? pc+inst.ISIZE+InstIf::ISIZE : pc+inst.address; downsize(2); XTAL_VM_CONTINUE(pc); }
			}}
		}
		if(raweq(get(1), get(0))){
			downsize(2);
			XTAL_VM_CONTINUE(pc+inst.ISIZE+InstIf::ISIZE);
		}
		else{
			XTAL_VM_CONTINUE(send2_q(pc+inst.ISIZE, Xid(op_eq)));
		}
	}*/ }

	XTAL_VM_CASE(IfNe){ XTAL_VM_CONTINUE(FunIfNe(pc)); /*
		switch(type(get(1))){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ pc = ivalue(get(1)) != ivalue(get()) ? pc+inst.ISIZE+inst.ISIZE+InstUnless::ISIZE : pc+inst.address; downsize(2); XTAL_VM_CONTINUE(pc); }
				XTAL_CASE(TYPE_FLOAT){ pc = ivalue(get(1)) != fvalue(get()) ? pc+inst.ISIZE+inst.ISIZE+InstUnless::ISIZE : pc+inst.address; downsize(2); XTAL_VM_CONTINUE(pc); }
			}}
			XTAL_CASE(TYPE_FLOAT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ pc = fvalue(get(1)) != ivalue(get()) ? pc+inst.ISIZE+inst.ISIZE+InstUnless::ISIZE : pc+inst.address; downsize(2); XTAL_VM_CONTINUE(pc); }
				XTAL_CASE(TYPE_FLOAT){ pc = fvalue(get(1)) != fvalue(get()) ? pc+inst.ISIZE+inst.ISIZE+InstUnless::ISIZE : pc+inst.address; downsize(2); XTAL_VM_CONTINUE(pc); }
			}}
		}
		if(rawne(get(1), get(0))){
			downsize(2);
			XTAL_VM_CONTINUE(pc+inst.ISIZE+inst.ISIZE+InstUnless::ISIZE);
		}
		else{
			XTAL_VM_CONTINUE(send2_q(pc+inst.ISIZE, Xid(op_eq)));
		}
	}*/ }

	XTAL_VM_CASE(IfLt){ XTAL_VM_CONTINUE(FunIfLt(pc)); /*
		switch(type(get(1))){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ pc = ivalue(get(1)) < ivalue(get()) ? pc+inst.ISIZE+InstIf::ISIZE : pc+inst.address; downsize(2); XTAL_VM_CONTINUE(pc); }
				XTAL_CASE(TYPE_FLOAT){ pc = ivalue(get(1)) < fvalue(get()) ? pc+inst.ISIZE+InstIf::ISIZE : pc+inst.address; downsize(2); XTAL_VM_CONTINUE(pc); }
			}}
			XTAL_CASE(TYPE_FLOAT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ pc = fvalue(get(1)) < ivalue(get()) ? pc+inst.ISIZE+InstIf::ISIZE : pc+inst.address; downsize(2); XTAL_VM_CONTINUE(pc); }
				XTAL_CASE(TYPE_FLOAT){ pc = fvalue(get(1)) < fvalue(get()) ? pc+inst.ISIZE+InstIf::ISIZE : pc+inst.address; downsize(2); XTAL_VM_CONTINUE(pc); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc+inst.ISIZE, Xid(op_lt)));
	}*/ }

	XTAL_VM_CASE(IfLe){ XTAL_VM_CONTINUE(FunIfLe(pc)); /*
		switch(type(get(1))){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ pc = ivalue(get(1)) <= ivalue(get()) ? pc+inst.ISIZE+InstUnless::ISIZE : pc+inst.address; downsize(2); XTAL_VM_CONTINUE(pc); }
				XTAL_CASE(TYPE_FLOAT){ pc = ivalue(get(1)) <= fvalue(get()) ? pc+inst.ISIZE+inst.ISIZE+InstUnless::ISIZE : pc+inst.address; downsize(2); XTAL_VM_CONTINUE(pc); }
			}}
			XTAL_CASE(TYPE_FLOAT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ pc = fvalue(get(1)) <= ivalue(get()) ? pc+inst.ISIZE+inst.ISIZE+InstUnless::ISIZE : pc+inst.address; downsize(2); XTAL_VM_CONTINUE(pc); }
				XTAL_CASE(TYPE_FLOAT){ pc = fvalue(get(1)) <= fvalue(get()) ? pc+inst.ISIZE+inst.ISIZE+InstUnless::ISIZE : pc+inst.address; downsize(2); XTAL_VM_CONTINUE(pc); }
			}}
		}
		XTAL_VM_CONTINUE(send2_r(pc+inst.ISIZE, Xid(op_lt)));
	}*/ }

	XTAL_VM_CASE(IfGt){ XTAL_VM_CONTINUE(FunIfGt(pc)); /*
		switch(type(get(1))){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ pc = ivalue(get(1)) > ivalue(get()) ? pc+inst.ISIZE+InstIf::ISIZE : pc+inst.address; downsize(2); XTAL_VM_CONTINUE(pc); }
				XTAL_CASE(TYPE_FLOAT){ pc = ivalue(get(1)) > fvalue(get()) ? pc+inst.ISIZE+InstIf::ISIZE : pc+inst.address; downsize(2); XTAL_VM_CONTINUE(pc); }
			}}
			XTAL_CASE(TYPE_FLOAT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ pc = fvalue(get(1)) > ivalue(get()) ? pc+inst.ISIZE+InstIf::ISIZE : pc+inst.address; downsize(2); XTAL_VM_CONTINUE(pc); }
				XTAL_CASE(TYPE_FLOAT){ pc = fvalue(get(1)) > fvalue(get()) ? pc+inst.ISIZE+InstIf::ISIZE : pc+inst.address; downsize(2); XTAL_VM_CONTINUE(pc); }
			}}
		}
		XTAL_VM_CONTINUE(send2_r(pc+inst.ISIZE, Xid(op_lt)));
	}*/ }

	XTAL_VM_CASE(IfGe){ XTAL_VM_CONTINUE(FunIfGe(pc)); /*
		switch(type(get(1))){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ pc = ivalue(get(1)) >= ivalue(get()) ? pc+inst.ISIZE+InstUnless::ISIZE : pc+inst.address; downsize(2); XTAL_VM_CONTINUE(pc); }
				XTAL_CASE(TYPE_FLOAT){ pc = ivalue(get(1)) >= fvalue(get()) ? pc+inst.ISIZE+InstUnless::ISIZE : pc+inst.address; downsize(2); XTAL_VM_CONTINUE(pc); }
			}}
			XTAL_CASE(TYPE_FLOAT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ pc = fvalue(get(1)) >= ivalue(get()) ? pc+inst.ISIZE+InstUnless::ISIZE : pc+inst.address; downsize(2); XTAL_VM_CONTINUE(pc); }
				XTAL_CASE(TYPE_FLOAT){ pc = fvalue(get(1)) >= fvalue(get()) ? pc+inst.ISIZE+InstUnless::ISIZE : pc+inst.address; downsize(2); XTAL_VM_CONTINUE(pc); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc+inst.ISIZE, Xid(op_lt)));
	}*/ }

	XTAL_VM_CASE(IfRawEq){ // 4
		pc += raweq(get(1), get()) ? (int)(inst.ISIZE+InstIf::ISIZE) : (int)inst.address;
		downsize(2);
		XTAL_VM_CONTINUE(pc);
	}

	XTAL_VM_CASE(IfRawNe){ // 4
		pc += !raweq(get(1), get()) ? (int)(inst.ISIZE+InstIf::ISIZE) : (int)inst.address;
		downsize(2);
		XTAL_VM_CONTINUE(pc);
	}

	XTAL_VM_CASE(IfIn){ // 2
		XTAL_VM_CONTINUE(send2(pc+inst.ISIZE, Xid(op_in)));
	}

	XTAL_VM_CASE(IfNin){ // 2
		XTAL_VM_CONTINUE(send2(pc+inst.ISIZE, Xid(op_in)));
	}

	XTAL_VM_CASE(IfIs){ // 4
		pc += get(1)->is(get()) ? (int)(inst.ISIZE+InstIf::ISIZE) : (int)inst.address;
		downsize(2);
		XTAL_VM_CONTINUE(pc);
	}

	XTAL_VM_CASE(IfNis){ // 4
		pc += !get(1)->is(get()) ? (int)(inst.ISIZE+InstIf::ISIZE) : (int)inst.address;
		downsize(2);
		XTAL_VM_CONTINUE(pc);
	}

	XTAL_VM_CASE(IfArgIsUndefined){ // 3
		if(raweq(local_variable(inst.arg), undefined)){
			XTAL_VM_CONTINUE(pc + inst.ISIZE);
		}
		else{
			XTAL_VM_CONTINUE(pc + inst.address); 
		}
	}

	XTAL_VM_CASE(IfArgIsUndefinedDirect){ // 3
		if(raweq(ff().variable(inst.arg), undefined)){
			XTAL_VM_CONTINUE(pc + inst.ISIZE);
		}
		else{
			XTAL_VM_CONTINUE(pc + inst.address); 
		}
	}

	XTAL_VM_CASE(Pos){ XTAL_VM_CONTINUE(FunPos(pc)); /*
		switch(type(get())){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ XTAL_VM_CONTINUE(pc + 1); }
			XTAL_CASE(TYPE_FLOAT){ XTAL_VM_CONTINUE(pc + 1); }
		}
		XTAL_VM_CONTINUE(send1(pc, Xid(op_pos)));
	}*/ }

	XTAL_VM_CASE(Neg){ XTAL_VM_CONTINUE(FunNeg(pc)); /*
		switch(type(get())){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ set(Innocence(-ivalue(get()))); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
			XTAL_CASE(TYPE_FLOAT){ set(Innocence(-fvalue(get()))); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
		}
		XTAL_VM_CONTINUE(send1(pc+inst.ISIZE, Xid(op_neg)));
	}*/ }

	XTAL_VM_CASE(Com){ XTAL_VM_CONTINUE(FunCom(pc)); /*
		switch(type(get())){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ set(Innocence(~ivalue(get()))); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
		}
		XTAL_VM_CONTINUE(send1(pc+inst.ISIZE, Xid(op_com)));
	}*/ }

	XTAL_VM_CASE(Not){ // 3
		set(Innocence(!get())); 
		XTAL_VM_CONTINUE(pc+inst.ISIZE); 
	}

	XTAL_VM_CASE(At){ // 7
		FunFrame& f = ff();
		XTAL_GLOBAL_INTERPRETER_LOCK{
			Innocence key = pop_and_save2();
			Innocence target = pop_and_save1();
			inner_setup_call(pc+inst.ISIZE, 1, ap(key));
			ap(target)->rawsend(myself(), Xid(op_at), ap(key)->get_class());
		}
		XTAL_VM_CONTINUE(ff().called_pc);
	}

	XTAL_VM_CASE(SetAt){ XTAL_VM_CONTINUE(FunSetAt(pc)); /*
		FunFrame& f = ff();
		XTAL_GLOBAL_INTERPRETER_LOCK{
			Innocence key = pop_and_save2();
			Innocence target = pop_and_save1();
			Innocence value = pop();
			inner_setup_call(pc+inst.ISIZE, 0, ap(key), ap(value));
			ap(target)->rawsend(myself(), Xid(op_set_at), ap(key)->get_class());
		}
		XTAL_VM_CONTINUE(ff().called_pc);
	}*/ }

	XTAL_VM_CASE(Add){ XTAL_VM_CONTINUE(FunAdd(pc)); /*
		switch(type(get(1))){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, Innocence(ivalue(get(1)) + ivalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
				XTAL_CASE(TYPE_FLOAT){ set(1, Innocence(ivalue(get(1)) + fvalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
			}}
			XTAL_CASE(TYPE_FLOAT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, Innocence(fvalue(get(1)) + ivalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
				XTAL_CASE(TYPE_FLOAT){ set(1, Innocence(fvalue(get(1)) + fvalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc+inst.ISIZE, Xid(op_add)));
	}*/ }

	XTAL_VM_CASE(Sub){ XTAL_VM_CONTINUE(FunSub(pc)); /*
		switch(type(get(1))){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, Innocence(ivalue(get(1)) - ivalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
				XTAL_CASE(TYPE_FLOAT){ set(1, Innocence(ivalue(get(1)) - fvalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
			}}
			XTAL_CASE(TYPE_FLOAT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, Innocence(fvalue(get(1)) - ivalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
				XTAL_CASE(TYPE_FLOAT){ set(1, Innocence(fvalue(get(1)) - fvalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc+inst.ISIZE, Xid(op_sub)));
	}*/ }

	XTAL_VM_CASE(Cat){ // 2
		XTAL_VM_CONTINUE(send2(pc+inst.ISIZE, Xid(op_cat)));
	}

	XTAL_VM_CASE(Mul){ XTAL_VM_CONTINUE(FunMul(pc)); /*
		switch(type(get(1))){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, Innocence(ivalue(get(1)) * ivalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
				XTAL_CASE(TYPE_FLOAT){ set(1, Innocence(ivalue(get(1)) * fvalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
			}}
			XTAL_CASE(TYPE_FLOAT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, Innocence(fvalue(get(1)) * ivalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
				XTAL_CASE(TYPE_FLOAT){ set(1, Innocence(fvalue(get(1)) * fvalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc+inst.ISIZE, Xid(op_mul)));
	}*/ }

	XTAL_VM_CASE(Div){ XTAL_VM_CONTINUE(FunDiv(pc)); /*
		switch(type(get(1))){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, Innocence(ivalue(get(1)) / check_zero(ivalue(get())))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
				XTAL_CASE(TYPE_FLOAT){ set(1, Innocence(ivalue(get(1)) / check_zero(fvalue(get())))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
			}}
			XTAL_CASE(TYPE_FLOAT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, Innocence(fvalue(get(1)) / check_zero(ivalue(get())))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
				XTAL_CASE(TYPE_FLOAT){ set(1, Innocence(fvalue(get(1)) / check_zero(fvalue(get())))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc+inst.ISIZE, Xid(op_div)));
	}*/ }

	XTAL_VM_CASE(Mod){ XTAL_VM_CONTINUE(FunMod(pc)); /*
		using namespace std;
		switch(type(get(1))){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, Innocence(ivalue(get(1)) % check_zero(ivalue(get())))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
				XTAL_CASE(TYPE_FLOAT){ set(1, Innocence(fmodf((float_t)ivalue(get(1)), fvalue(get())))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
			}}
			XTAL_CASE(TYPE_FLOAT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, Innocence(fmodf(fvalue(get(1)), (float_t)ivalue(get())))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
				XTAL_CASE(TYPE_FLOAT){ set(1, Innocence(fmodf(fvalue(get(1)), fvalue(get())))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc+inst.ISIZE, Xid(op_mod)));
	}*/ }

	XTAL_VM_CASE(And){ XTAL_VM_CONTINUE(FunAnd(pc)); /*
		switch(type(get(1))){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, Innocence(ivalue(get(1)) & ivalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc+inst.ISIZE, Xid(op_and)));
	}*/ }

	XTAL_VM_CASE(Or){ XTAL_VM_CONTINUE(FunOr(pc)); /*
		switch(type(get(1))){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, Innocence(ivalue(get(1)) | ivalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc+inst.ISIZE, Xid(op_or)));
	}*/ }

	XTAL_VM_CASE(Xor){ XTAL_VM_CONTINUE(FunXor(pc)); /*
		switch(type(get(1))){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, Innocence(ivalue(get(1)) ^ ivalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc+inst.ISIZE, Xid(op_xor)));
	}*/ }

	XTAL_VM_CASE(Shl){ XTAL_VM_CONTINUE(FunShl(pc)); /*
		switch(type(get(1))){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, Innocence(ivalue(get(1)) << ivalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc+inst.ISIZE, Xid(op_shl)));
	}*/ }

	XTAL_VM_CASE(Shr){ XTAL_VM_CONTINUE(FunShr(pc)); /*
		switch(type(get(1))){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, Innocence(ivalue(get(1)) >> ivalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc+inst.ISIZE, Xid(op_shr)));
	}*/ }

	XTAL_VM_CASE(Ushr){ XTAL_VM_CONTINUE(FunUshr(pc)); /*
		switch(type(get(1))){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, Innocence((int_t)((uint_t)ivalue(get(1)) >> ivalue(get())))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc+inst.ISIZE, Xid(op_ushr)));
	}*/ }

	XTAL_VM_CASE(Eq){ XTAL_VM_CONTINUE(FunEq(pc)); /*
		switch(type(get(1))){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, Innocence(ivalue(get(1)) == ivalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
				XTAL_CASE(TYPE_FLOAT){ set(1, Innocence(ivalue(get(1)) == fvalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
			}}
			XTAL_CASE(TYPE_FLOAT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, Innocence(fvalue(get(1)) == ivalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
				XTAL_CASE(TYPE_FLOAT){ set(1, Innocence(fvalue(get(1)) == fvalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
			}}
		}
		if(raweq(get(1), get(0))){
			set(1, Innocence(true)); downsize(1);
			XTAL_VM_CONTINUE(pc+inst.ISIZE);
		}
		else{
			XTAL_VM_CONTINUE(send2_q(pc+inst.ISIZE, Xid(op_eq)));
		}
	}*/ }

	XTAL_VM_CASE(Ne){ XTAL_VM_CONTINUE(FunNe(pc)); /*
		switch(type(get(1))){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, Innocence(ivalue(get(1)) != ivalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE+InstNot::ISIZE); }
				XTAL_CASE(TYPE_FLOAT){ set(1, Innocence(ivalue(get(1)) != fvalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE+InstNot::ISIZE); }
			}}
			XTAL_CASE(TYPE_FLOAT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, Innocence(fvalue(get(1)) != ivalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE+InstNot::ISIZE); }
				XTAL_CASE(TYPE_FLOAT){ set(1, Innocence(fvalue(get(1)) != fvalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE+InstNot::ISIZE);}
			}}
		}
		if(rawne(get(1), get(0))){
			set(1, Innocence(true)); downsize(1);
			XTAL_VM_CONTINUE(pc+inst.ISIZE+InstNot::ISIZE);
		}
		else{
			XTAL_VM_CONTINUE(send2_q(pc+inst.ISIZE, Xid(op_eq)));
		}
	}*/ }

	XTAL_VM_CASE(Lt){ XTAL_VM_CONTINUE(FunLt(pc)); /*
		switch(type(get(1))){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, Innocence(ivalue(get(1)) < ivalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
				XTAL_CASE(TYPE_FLOAT){ set(1, Innocence(ivalue(get(1)) < fvalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
			}}
			XTAL_CASE(TYPE_FLOAT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, Innocence(fvalue(get(1)) < ivalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
				XTAL_CASE(TYPE_FLOAT){ set(1, Innocence(fvalue(get(1)) < fvalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc+inst.ISIZE, Xid(op_lt)));
	}*/ }

	XTAL_VM_CASE(Le){ XTAL_VM_CONTINUE(FunLe(pc)); /*
		switch(type(get(1))){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, Innocence(ivalue(get(1)) <= ivalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE+InstNot::ISIZE); }
				XTAL_CASE(TYPE_FLOAT){ set(1, Innocence(ivalue(get(1)) <= fvalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE+InstNot::ISIZE); }
			}}
			XTAL_CASE(TYPE_FLOAT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, Innocence(fvalue(get(1)) <= ivalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE+InstNot::ISIZE); }
				XTAL_CASE(TYPE_FLOAT){ set(1, Innocence(fvalue(get(1)) <= fvalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE+InstNot::ISIZE); }
			}}
		}
		XTAL_VM_CONTINUE(send2_r(pc+inst.ISIZE, Xid(op_lt)));
	}*/ }

	XTAL_VM_CASE(Gt){ XTAL_VM_CONTINUE(FunGt(pc)); /*
		switch(type(get(1))){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, Innocence(ivalue(get(1)) > ivalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
				XTAL_CASE(TYPE_FLOAT){ set(1, Innocence(ivalue(get(1)) > fvalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
			}}
			XTAL_CASE(TYPE_FLOAT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, Innocence(fvalue(get(1)) > ivalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
				XTAL_CASE(TYPE_FLOAT){ set(1, Innocence(fvalue(get(1)) > fvalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
			}}
		}
		XTAL_VM_CONTINUE(send2_r(pc+inst.ISIZE, Xid(op_lt)));
	}*/ }

	XTAL_VM_CASE(Ge){ XTAL_VM_CONTINUE(FunGe(pc)); /*
		switch(type(get(1))){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, Innocence(ivalue(get(1)) >= ivalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE+InstNot::ISIZE); }
				XTAL_CASE(TYPE_FLOAT){ set(1, Innocence(ivalue(get(1)) >= fvalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE+InstNot::ISIZE); }
			}}
			XTAL_CASE(TYPE_FLOAT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, Innocence(fvalue(get(1)) >= ivalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE+InstNot::ISIZE); }
				XTAL_CASE(TYPE_FLOAT){ set(1, Innocence(fvalue(get(1)) >= fvalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE+InstNot::ISIZE); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc+inst.ISIZE, Xid(op_lt)));
	}*/ }

	XTAL_VM_CASE(RawEq){ // 4
		set(1, Innocence(raweq(get(1), get())));
		downsize(1);
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}

	XTAL_VM_CASE(RawNe){ // 4
		set(1, Innocence(rawne(get(1), get())));
		downsize(1);
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}

	XTAL_VM_CASE(In){ // 2
		XTAL_VM_CONTINUE(send2(pc+inst.ISIZE, Xid(op_in)));
	}

	XTAL_VM_CASE(Nin){ // 2
		XTAL_VM_CONTINUE(send2(pc+inst.ISIZE, Xid(op_in)));
	}

	XTAL_VM_CASE(Is){ // 4
		set(1, Innocence(get(1)->is(get())));
		downsize(1);
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}

	XTAL_VM_CASE(Nis){ // 4
		set(1, Innocence(!get(1)->is(get())));
		downsize(1);
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}

	XTAL_VM_CASE(Inc){ XTAL_VM_CONTINUE(FunInc(pc)); /*
		switch(type(get())){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ set(Innocence(ivalue(get())+1)); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
			XTAL_CASE(TYPE_FLOAT){ set(Innocence(fvalue(get())+1)); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
		}
		XTAL_VM_CONTINUE(send1(pc+inst.ISIZE, Xid(op_inc)));
	}*/ }

	XTAL_VM_CASE(Dec){ XTAL_VM_CONTINUE(FunDec(pc)); /*
		switch(type(get())){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ set(Innocence(ivalue(get())-1)); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
			XTAL_CASE(TYPE_FLOAT){ set(Innocence(fvalue(get())-1)); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
		}
		XTAL_VM_CONTINUE(send1(pc+inst.ISIZE, Xid(op_dec)));
	}*/ }

	XTAL_VM_CASE(AddAssign){ XTAL_VM_CONTINUE(FunAddAssign(pc)); /*
		switch(type(get(1))){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, Innocence(ivalue(get(1)) + ivalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
				XTAL_CASE(TYPE_FLOAT){ set(1, Innocence(ivalue(get(1)) + fvalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
			}}
			XTAL_CASE(TYPE_FLOAT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, Innocence(fvalue(get(1)) + ivalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
				XTAL_CASE(TYPE_FLOAT){ set(1, Innocence(fvalue(get(1)) + fvalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc+inst.ISIZE, Xid(op_add_assign)));
	}*/ }

	XTAL_VM_CASE(SubAssign){ XTAL_VM_CONTINUE(FunSubAssign(pc)); /*
		switch(type(get(1))){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, Innocence(ivalue(get(1)) - ivalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
				XTAL_CASE(TYPE_FLOAT){ set(1, Innocence(ivalue(get(1)) - fvalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
			}}
			XTAL_CASE(TYPE_FLOAT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, Innocence(fvalue(get(1)) - ivalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
				XTAL_CASE(TYPE_FLOAT){ set(1, Innocence(fvalue(get(1)) - fvalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc+inst.ISIZE, Xid(op_sub_assign)));
	}*/ }

	XTAL_VM_CASE(CatAssign){ // 2
		XTAL_VM_CONTINUE(send2(pc+inst.ISIZE, Xid(op_cat_assign)));
	}

	XTAL_VM_CASE(MulAssign){ XTAL_VM_CONTINUE(FunMulAssign(pc)); /*
		switch(type(get(1))){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, Innocence(ivalue(get(1)) * ivalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
				XTAL_CASE(TYPE_FLOAT){ set(1, Innocence(ivalue(get(1)) * fvalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
			}}
			XTAL_CASE(TYPE_FLOAT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, Innocence(fvalue(get(1)) * ivalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
				XTAL_CASE(TYPE_FLOAT){ set(1, Innocence(fvalue(get(1)) * fvalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc+inst.ISIZE, Xid(op_mul_assign)));
	}*/ }

	XTAL_VM_CASE(DivAssign){ XTAL_VM_CONTINUE(FunDivAssign(pc)); /*
		switch(type(get(1))){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, Innocence(ivalue(get(1)) / check_zero(ivalue(get())))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
				XTAL_CASE(TYPE_FLOAT){ set(1, Innocence(ivalue(get(1)) / check_zero(fvalue(get())))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
			}}
			XTAL_CASE(TYPE_FLOAT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, Innocence(fvalue(get(1)) / check_zero(ivalue(get())))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
				XTAL_CASE(TYPE_FLOAT){ set(1, Innocence(fvalue(get(1)) / check_zero(fvalue(get())))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc+inst.ISIZE, Xid(op_div_assign)));
	}*/ }

	XTAL_VM_CASE(ModAssign){ XTAL_VM_CONTINUE(FunModAssign(pc)); /*
		using namespace std;
		switch(type(get(1))){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, Innocence(ivalue(get(1)) % check_zero(ivalue(get())))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
				XTAL_CASE(TYPE_FLOAT){ set(1, Innocence(fmodf((float_t)ivalue(get(1)), fvalue(get())))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
			}}
			XTAL_CASE(TYPE_FLOAT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, Innocence(fmodf(fvalue(get(1)),(float_t)ivalue(get())))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
				XTAL_CASE(TYPE_FLOAT){ set(1, Innocence(fmodf(fvalue(get(1)), fvalue(get())))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc+inst.ISIZE, Xid(op_mod_assign)));
	}*/ }

	XTAL_VM_CASE(AndAssign){ XTAL_VM_CONTINUE(FunAndAssign(pc)); /*
		switch(type(get(1))){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, Innocence(ivalue(get(1)) & ivalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc+inst.ISIZE, Xid(op_and_assign)));
	}*/ }

	XTAL_VM_CASE(OrAssign){ XTAL_VM_CONTINUE(FunOrAssign(pc)); /*
		switch(type(get(1))){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, Innocence(ivalue(get(1)) | ivalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc+inst.ISIZE, Xid(op_or_assign)));
	}*/ }

	XTAL_VM_CASE(XorAssign){ XTAL_VM_CONTINUE(FunXorAssign(pc)); /*
		switch(type(get(1))){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, Innocence(ivalue(get(1)) ^ ivalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc+inst.ISIZE, Xid(op_xor_assign)));
	}*/ }

	XTAL_VM_CASE(ShlAssign){ XTAL_VM_CONTINUE(FunShlAssign(pc)); /*
		switch(type(get(1))){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, Innocence(ivalue(get(1)) << ivalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc+inst.ISIZE, Xid(op_shl_assign)));
	}*/ }

	XTAL_VM_CASE(ShrAssign){ XTAL_VM_CONTINUE(FunShrAssign(pc)); /*
		switch(type(get(1))){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, Innocence(ivalue(get(1)) >> ivalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc+inst.ISIZE, Xid(op_shr_assign)));
	}*/ }

	XTAL_VM_CASE(UshrAssign){ XTAL_VM_CONTINUE(FunUshrAssign(pc)); /*
		switch(type(get(1))){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, Innocence((int_t)((uint_t)ivalue(get(1)) >> ivalue(get())))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc+inst.ISIZE, Xid(op_ushr_assign)));
	}*/ }

	XTAL_VM_CASE(Range){ // 6
		XTAL_GLOBAL_INTERPRETER_LOCK{
			Innocence rhs = pop_and_save2();
			Innocence lhs = pop_and_save1();
			inner_setup_call(pc+inst.ISIZE, 1, ap(rhs), ap(Innocence((int_t)inst.kind)));
			ap(lhs)->rawsend(myself(), Xid(op_range), ap(rhs)->get_class());
		}
		XTAL_VM_CONTINUE(ff().called_pc);
	}

	XTAL_VM_CASE(ClassBegin){ XTAL_VM_CONTINUE(FunClassBegin(pc)); /*
		XTAL_GLOBAL_INTERPRETER_LOCK{
			ClassCore* core = code()->class_core(inst.core_number);
			const FramePtr& outer = (core->flags&ClassCore::FLAG_SCOPE_CHAIN) ? ff().outer() : static_ptr_cast<Frame>(null);
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
			ClassPtr p = cast<ClassPtr>(ff().outer());
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
			cast<ArrayPtr>(get(1))->push_back(get()); 
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
			cast<MapPtr>(get(2))->set_at(get(1), get()); 
			downsize(2);	
		}
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}

	XTAL_VM_CASE(MapSetDefault){ // 4
		XTAL_GLOBAL_INTERPRETER_LOCK{
			cast<MapPtr>(get(1))->set_default_value(get()); 
			downsize(1);	
		}
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}

	XTAL_VM_CASE(MakeFun){ XTAL_VM_CONTINUE(FunMakeFun(pc)); /*
		int_t table_n = inst.core_number, end = inst.address;
		XTAL_GLOBAL_INTERPRETER_LOCK{
			FunCore* core = code()->fun_core(table_n);
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

				XTAL_CASE(0){ ret = xnew<InstanceVariableGetter>(inst.number, code()->class_core(inst.core_number)); }
				XTAL_CASE(1){ ret = xnew<InstanceVariableSetter>(inst.number, code()->class_core(inst.core_number)); }
			}
			push(ret);
		}
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}*/ }

	XTAL_VM_CASE(Throw){ // 6
		XTAL_GLOBAL_INTERPRETER_LOCK{
			AnyPtr except = pop();
			if(!except){
				except = ap(except_[0]);
			}

			if(!except->is(builtin()->member("Exception"))){
				except_[0] = builtin()->member("RuntimeError")(except);
			}
			else{
				except_[0] = except;
			}
		}
		XTAL_VM_EXCEPT(except_[0]);
	}

	XTAL_VM_CASE(ThrowUnsupportedError){ // 4
		XTAL_GLOBAL_INTERPRETER_LOCK{
			if(ap(prev_ff().temp2_)){
				except_[0] = unsupported_error(ap(prev_ff().temp1_)->get_class(), isp(prev_ff().temp3_), ap(prev_ff().temp2_));
			}
			else{
				except_[0] = unsupported_error(ap(prev_ff().temp1_)->get_class(), isp(prev_ff().temp3_), null);
			}
		}
		XTAL_VM_EXCEPT(except_[0]);
	}

	XTAL_VM_CASE(ThrowUndefined){ // 2
		XTAL_VM_EXCEPT(undefined); 
	}

	XTAL_VM_CASE(Assert){ // 10
		bool failure = false;
		XTAL_GLOBAL_INTERPRETER_LOCK{
			AnyPtr expr = get(2);
			if(!expr){
				failure = true;
				AnyPtr expr_string = get(1) ? get(1) : AnyPtr("");
				AnyPtr message = get() ? get() : AnyPtr("");
				except_[0] = builtin()->member("AssertionFailed")(message, expr_string);
			}
			downsize(3);
		}

		if(failure){
			XTAL_VM_EXCEPT(except_[0]); 
		}

		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}

	XTAL_VM_CASE(BreakPoint){ XTAL_VM_CONTINUE(FunBreakPoint(pc)); /*
		if(debug::is_enabled()){
			XTAL_GLOBAL_INTERPRETER_LOCK{
				int_t kind = inst.type;
				
				if(!debug_info_) debug_info_ = xnew<debug::Info>();
				debug_info_->set_kind(kind);
				debug_info_->set_line(code()->compliant_lineno(pc));
				debug_info_->set_file_name(code()->source_file_name());
				debug_info_->set_fun_name(fun()->object_name());
				debug_info_->set_local_variables(ff().outer());

				struct guard{
					guard(){ debug::disable(); }
					~guard(){ debug::enable(); }
				} g;
			
				switch(kind){
					XTAL_NODEFAULT;
					
					XTAL_CASE(BREAKPOINT_LINE){
						if(AnyPtr hook = debug::line_hook()){
							hook(debug_info_);
						}
					}

					XTAL_CASE(BREAKPOINT_CALL){
						if(AnyPtr hook = debug::call_hook()){
							hook(debug_info_);
						}				
					}

					XTAL_CASE(BREAKPOINT_RETURN){
						if(AnyPtr hook = debug::return_hook()){
							hook(debug_info_);
						}				
					}
				}
			}
		}
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}*/ }

	XTAL_VM_CASE(MAX){ // 2
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}

//}}OPS}

	XTAL_VM_NODEFAULT;
}

// 例外が投げられたらここに到達する
except_catch:
	except_[0] = append_backtrace(pc, ap(except_[0]));
	pc = catch_body(pc, stack_size, fun_frames_size);
	if(pc){
		goto begin;
	}
	goto rethrow;

}
XTAL_CATCH(e){
	except_[0] = append_backtrace(pc, e);
	pc = catch_body(pc, stack_size, fun_frames_size);
	if(pc){
		goto retry;
	}
	goto rethrow;	
}
#ifndef XTAL_NO_EXCEPTIONS
catch(...){
	except_[0] = null;
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

#undef XTAL_VM_RETURN
#define XTAL_VM_RETURN last_except_ = null; return 0

#undef XTAL_VM_EXCEPT
#define XTAL_VM_EXCEPT(x) last_except_ = (x); return 0

#ifdef XTAL_NO_EXCEPTIONS
#	undef XTAL_VM_CHECK_EXCEPT
#	define XTAL_VM_CHECK_EXCEPT if(ap(except_[0])){ return 0; }
#endif

//{FUNS{{
const inst_t* VMachine::FunInsert3(const inst_t* pc){
		XTAL_VM_DEF_INST(Insert3);
		Innocence temp = get(); 
		set(get(1)); 
		set(1, get(2)); 
		set(2, get(3)); 
		set(3, temp); 
		XTAL_VM_CONTINUE(pc + inst.ISIZE);;;;;;
}

const inst_t* VMachine::FunLocalVariableInc(const inst_t* pc){
		XTAL_VM_DEF_INST(LocalVariableInc);
		Innocence a = local_variable(inst.number);
		switch(type(a)){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ set_local_variable(inst.number, Innocence(ivalue(a)+1)); XTAL_VM_CONTINUE(pc + inst.ISIZE + InstSetLocalVariable1Byte::ISIZE); }
			XTAL_CASE(TYPE_FLOAT){ set_local_variable(inst.number, Innocence(fvalue(a)+1)); XTAL_VM_CONTINUE(pc + inst.ISIZE + InstSetLocalVariable1Byte::ISIZE); }
		}
		ap(a)->rawsend(inner_setup_call(pc + inst.ISIZE, 1), Xid(op_inc));
		XTAL_VM_CONTINUE(ff().called_pc);
}

const inst_t* VMachine::FunLocalVariableDec(const inst_t* pc){
		XTAL_VM_DEF_INST(LocalVariableDec);
		Innocence a = local_variable(inst.number);
		switch(type(a)){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ set_local_variable(inst.number, Innocence(ivalue(a)-1)); XTAL_VM_CONTINUE(pc + inst.ISIZE + InstSetLocalVariable1Byte::ISIZE); }
			XTAL_CASE(TYPE_FLOAT){ set_local_variable(inst.number, Innocence(fvalue(a)-1)); XTAL_VM_CONTINUE(pc + inst.ISIZE + InstSetLocalVariable1Byte::ISIZE); }
		}
		ap(a)->rawsend(inner_setup_call(pc + inst.ISIZE, 1), Xid(op_dec));
		XTAL_VM_CONTINUE(ff().called_pc);
}

const inst_t* VMachine::FunLocalVariableInc2Byte(const inst_t* pc){
		XTAL_VM_DEF_INST(LocalVariableInc2Byte);
		Innocence a = local_variable(inst.number);
		switch(type(a)){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ set_local_variable(inst.number, Innocence(ivalue(a)+1)); XTAL_VM_CONTINUE(pc + inst.ISIZE + InstSetLocalVariable2Byte::ISIZE); }
			XTAL_CASE(TYPE_FLOAT){ set_local_variable(inst.number, Innocence(fvalue(a)+1)); XTAL_VM_CONTINUE(pc + inst.ISIZE + InstSetLocalVariable2Byte::ISIZE); }
		}
		ap(a)->rawsend(inner_setup_call(pc + inst.ISIZE, 1), Xid(op_inc));
		XTAL_VM_CONTINUE(ff().called_pc);
}

const inst_t* VMachine::FunLocalVariableDec2Byte(const inst_t* pc){
		XTAL_VM_DEF_INST(LocalVariableDec2Byte);
		Innocence a = local_variable(inst.number);
		switch(type(a)){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ set_local_variable(inst.number, Innocence(ivalue(a)-1)); XTAL_VM_CONTINUE(pc + inst.ISIZE + InstSetLocalVariable2Byte::ISIZE); }
			XTAL_CASE(TYPE_FLOAT){ set_local_variable(inst.number, Innocence(fvalue(a)-1)); XTAL_VM_CONTINUE(pc + inst.ISIZE + InstSetLocalVariable2Byte::ISIZE); }
		}
		ap(a)->rawsend(inner_setup_call(pc + inst.ISIZE, 1), Xid(op_dec));
		XTAL_VM_CONTINUE(ff().called_pc);
}

const inst_t* VMachine::FunProperty(const inst_t* pc){
		XTAL_VM_DEF_INST(Property);
		XTAL_GLOBAL_INTERPRETER_LOCK{
			ff().temp2_ = null;
			Innocence primary_key = identifier_ex(inst.identifier_number);
			Innocence self = ff().self();
			Innocence target = pop_and_save1();
			push_ff(pc + inst.ISIZE, 1, 0, 0, ap(self));
			ap(target)->rawsend(myself(), isp(primary_key), null, ff().self());
		}
		XTAL_VM_CONTINUE(ff().called_pc); 	
}

const inst_t* VMachine::FunPropertyNS(const inst_t* pc){
		XTAL_VM_DEF_INST(PropertyNS);
		XTAL_GLOBAL_INTERPRETER_LOCK{
			Innocence secondary_key = pop_and_save2();
			Innocence primary_key = identifier_ex(inst.identifier_number);
			Innocence self = ff().self();
			Innocence target = pop_and_save1();
			push_ff(pc + inst.ISIZE, 1, 0, 0, ap(self));
			ap(target)->rawsend(myself(), isp(primary_key), ap(secondary_key), ff().self());
		}
		XTAL_VM_CONTINUE(ff().called_pc); 	
}

const inst_t* VMachine::FunSetProperty(const inst_t* pc){
		XTAL_VM_DEF_INST(SetProperty);
		XTAL_GLOBAL_INTERPRETER_LOCK{
			ff().temp2_ = null;
			Innocence primary_key = identifier_ex(inst.identifier_number);
			Innocence self = ff().self();
			Innocence target = pop_and_save1();
			push_ff(pc + inst.ISIZE, 0, 1, 0, ap(self));
			ap(target)->rawsend(myself(), isp(primary_key), null, ff().self());
		}
		XTAL_VM_CONTINUE(ff().called_pc); 	
}

const inst_t* VMachine::FunSetPropertyNS(const inst_t* pc){
		XTAL_VM_DEF_INST(SetPropertyNS);
		XTAL_GLOBAL_INTERPRETER_LOCK{
			Innocence secondary_key = pop_and_save2();
			Innocence primary_key = identifier_ex(inst.identifier_number);
			Innocence self = ff().self();
			Innocence target = pop_and_save1();
			push_ff(pc + inst.ISIZE, 0, 1, 0, ap(self));
			ap(target)->rawsend(myself(), isp(primary_key), ap(secondary_key), ff().self());
		}
		XTAL_VM_CONTINUE(ff().called_pc); 	
}

const inst_t* VMachine::FunCall(const inst_t* pc){
		XTAL_VM_DEF_INST(Call);
		XTAL_GLOBAL_INTERPRETER_LOCK{
			ff().temp2_ = null;
			ff().temp3_ = Xid(op_call);
			Innocence self = ff().self();
			Innocence target = pop_and_save1();
			push_ff(pc + inst.ISIZE, (InstCall&)inst, ap(self));
			ap(target)->call(myself());
		}
		XTAL_VM_CONTINUE(ff().called_pc);	
}

const inst_t* VMachine::FunSend(const inst_t* pc){
		XTAL_VM_DEF_INST(Send);
		XTAL_GLOBAL_INTERPRETER_LOCK{
			ff().temp2_ = null;
			Innocence primary_key = identifier_ex(inst.identifier_number);
			Innocence self = ff().self();
			Innocence target = pop_and_save1();
			push_ff(pc + inst.ISIZE, (InstCall&)inst, ap(self));
			ap(target)->rawsend(myself(), isp(primary_key), null, ff().self());
		}
		XTAL_VM_CONTINUE(ff().called_pc); 	
}

const inst_t* VMachine::FunSendNS(const inst_t* pc){
		XTAL_VM_DEF_INST(SendNS);
		XTAL_GLOBAL_INTERPRETER_LOCK{
			Innocence secondary_key = pop_and_save2();
			Innocence primary_key = identifier_ex(inst.identifier_number);
			Innocence self = ff().self();
			Innocence target = pop_and_save1();
			push_ff(pc + inst.ISIZE, (InstCall&)inst, ap(self));
			ap(target)->rawsend(myself(), isp(primary_key), ap(secondary_key), ff().self());
		}
		XTAL_VM_CONTINUE(ff().called_pc); 	
}

const inst_t* VMachine::FunSendQ(const inst_t* pc){
		XTAL_VM_DEF_INST(SendQ);
		XTAL_GLOBAL_INTERPRETER_LOCK{
			ff().temp2_ = null;
			Innocence primary_key = identifier_ex(inst.identifier_number);
			Innocence self = ff().self();
			Innocence target = pop_and_save1();
			push_ff(pc + inst.ISIZE, (InstCall&)inst, ap(self));
			ff().called_pc = &check_unsupported_code_;
			ap(target)->rawsend(myself(), isp(primary_key), null, ff().self());
		}
		XTAL_VM_CONTINUE(ff().called_pc); 	
}

const inst_t* VMachine::FunSendQNS(const inst_t* pc){
		XTAL_VM_DEF_INST(SendQNS);
		XTAL_GLOBAL_INTERPRETER_LOCK{
			Innocence secondary_key = pop_and_save2();
			Innocence primary_key = identifier_ex(inst.identifier_number);
			Innocence self = ff().self();
			Innocence target = pop_and_save1();
			push_ff(pc + inst.ISIZE, (InstCall&)inst, ap(self));
			ff().called_pc = &check_unsupported_code_;
			ap(target)->rawsend(myself(), isp(primary_key), ap(secondary_key), ff().self());
		}
		XTAL_VM_CONTINUE(ff().called_pc); 	
}

const inst_t* VMachine::FunDefineMemberNS(const inst_t* pc){
		XTAL_VM_DEF_INST(DefineMemberNS);
		XTAL_GLOBAL_INTERPRETER_LOCK{
			Innocence secondary_key = get();
			Innocence primary_key = identifier_ex(inst.identifier_number);
			Innocence value = get(1);
			Innocence target = get(2);
			ap(target)->def(isp(primary_key), ap(value), ap(secondary_key), KIND_PUBLIC); 
			downsize(3);
		}
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
}

const inst_t* VMachine::FunIfEq(const inst_t* pc){
		XTAL_VM_DEF_INST(IfEq);
		switch(type(get(1))){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ pc = ivalue(get(1)) == ivalue(get()) ? pc+inst.ISIZE+InstIf::ISIZE : pc+inst.address; downsize(2); XTAL_VM_CONTINUE(pc); }
				XTAL_CASE(TYPE_FLOAT){ pc = ivalue(get(1)) == fvalue(get()) ? pc+inst.ISIZE+InstIf::ISIZE : pc+inst.address; downsize(2); XTAL_VM_CONTINUE(pc); }
			}}
			XTAL_CASE(TYPE_FLOAT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ pc = fvalue(get(1)) == ivalue(get()) ? pc+inst.ISIZE+InstIf::ISIZE : pc+inst.address; downsize(2); XTAL_VM_CONTINUE(pc); }
				XTAL_CASE(TYPE_FLOAT){ pc = fvalue(get(1)) == fvalue(get()) ? pc+inst.ISIZE+InstIf::ISIZE : pc+inst.address; downsize(2); XTAL_VM_CONTINUE(pc); }
			}}
		}
		if(raweq(get(1), get(0))){
			downsize(2);
			XTAL_VM_CONTINUE(pc+inst.ISIZE+InstIf::ISIZE);
		}
		else{
			XTAL_VM_CONTINUE(send2_q(pc+inst.ISIZE, Xid(op_eq)));
		}
}

const inst_t* VMachine::FunIfNe(const inst_t* pc){
		XTAL_VM_DEF_INST(IfNe);
		switch(type(get(1))){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ pc = ivalue(get(1)) != ivalue(get()) ? pc+inst.ISIZE+inst.ISIZE+InstUnless::ISIZE : pc+inst.address; downsize(2); XTAL_VM_CONTINUE(pc); }
				XTAL_CASE(TYPE_FLOAT){ pc = ivalue(get(1)) != fvalue(get()) ? pc+inst.ISIZE+inst.ISIZE+InstUnless::ISIZE : pc+inst.address; downsize(2); XTAL_VM_CONTINUE(pc); }
			}}
			XTAL_CASE(TYPE_FLOAT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ pc = fvalue(get(1)) != ivalue(get()) ? pc+inst.ISIZE+inst.ISIZE+InstUnless::ISIZE : pc+inst.address; downsize(2); XTAL_VM_CONTINUE(pc); }
				XTAL_CASE(TYPE_FLOAT){ pc = fvalue(get(1)) != fvalue(get()) ? pc+inst.ISIZE+inst.ISIZE+InstUnless::ISIZE : pc+inst.address; downsize(2); XTAL_VM_CONTINUE(pc); }
			}}
		}
		if(rawne(get(1), get(0))){
			downsize(2);
			XTAL_VM_CONTINUE(pc+inst.ISIZE+inst.ISIZE+InstUnless::ISIZE);
		}
		else{
			XTAL_VM_CONTINUE(send2_q(pc+inst.ISIZE, Xid(op_eq)));
		}
}

const inst_t* VMachine::FunIfLt(const inst_t* pc){
		XTAL_VM_DEF_INST(IfLt);
		switch(type(get(1))){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ pc = ivalue(get(1)) < ivalue(get()) ? pc+inst.ISIZE+InstIf::ISIZE : pc+inst.address; downsize(2); XTAL_VM_CONTINUE(pc); }
				XTAL_CASE(TYPE_FLOAT){ pc = ivalue(get(1)) < fvalue(get()) ? pc+inst.ISIZE+InstIf::ISIZE : pc+inst.address; downsize(2); XTAL_VM_CONTINUE(pc); }
			}}
			XTAL_CASE(TYPE_FLOAT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ pc = fvalue(get(1)) < ivalue(get()) ? pc+inst.ISIZE+InstIf::ISIZE : pc+inst.address; downsize(2); XTAL_VM_CONTINUE(pc); }
				XTAL_CASE(TYPE_FLOAT){ pc = fvalue(get(1)) < fvalue(get()) ? pc+inst.ISIZE+InstIf::ISIZE : pc+inst.address; downsize(2); XTAL_VM_CONTINUE(pc); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc+inst.ISIZE, Xid(op_lt)));
}

const inst_t* VMachine::FunIfLe(const inst_t* pc){
		XTAL_VM_DEF_INST(IfLe);
		switch(type(get(1))){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ pc = ivalue(get(1)) <= ivalue(get()) ? pc+inst.ISIZE+InstUnless::ISIZE : pc+inst.address; downsize(2); XTAL_VM_CONTINUE(pc); }
				XTAL_CASE(TYPE_FLOAT){ pc = ivalue(get(1)) <= fvalue(get()) ? pc+inst.ISIZE+inst.ISIZE+InstUnless::ISIZE : pc+inst.address; downsize(2); XTAL_VM_CONTINUE(pc); }
			}}
			XTAL_CASE(TYPE_FLOAT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ pc = fvalue(get(1)) <= ivalue(get()) ? pc+inst.ISIZE+inst.ISIZE+InstUnless::ISIZE : pc+inst.address; downsize(2); XTAL_VM_CONTINUE(pc); }
				XTAL_CASE(TYPE_FLOAT){ pc = fvalue(get(1)) <= fvalue(get()) ? pc+inst.ISIZE+inst.ISIZE+InstUnless::ISIZE : pc+inst.address; downsize(2); XTAL_VM_CONTINUE(pc); }
			}}
		}
		XTAL_VM_CONTINUE(send2_r(pc+inst.ISIZE, Xid(op_lt)));
}

const inst_t* VMachine::FunIfGt(const inst_t* pc){
		XTAL_VM_DEF_INST(IfGt);
		switch(type(get(1))){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ pc = ivalue(get(1)) > ivalue(get()) ? pc+inst.ISIZE+InstIf::ISIZE : pc+inst.address; downsize(2); XTAL_VM_CONTINUE(pc); }
				XTAL_CASE(TYPE_FLOAT){ pc = ivalue(get(1)) > fvalue(get()) ? pc+inst.ISIZE+InstIf::ISIZE : pc+inst.address; downsize(2); XTAL_VM_CONTINUE(pc); }
			}}
			XTAL_CASE(TYPE_FLOAT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ pc = fvalue(get(1)) > ivalue(get()) ? pc+inst.ISIZE+InstIf::ISIZE : pc+inst.address; downsize(2); XTAL_VM_CONTINUE(pc); }
				XTAL_CASE(TYPE_FLOAT){ pc = fvalue(get(1)) > fvalue(get()) ? pc+inst.ISIZE+InstIf::ISIZE : pc+inst.address; downsize(2); XTAL_VM_CONTINUE(pc); }
			}}
		}
		XTAL_VM_CONTINUE(send2_r(pc+inst.ISIZE, Xid(op_lt)));
}

const inst_t* VMachine::FunIfGe(const inst_t* pc){
		XTAL_VM_DEF_INST(IfGe);
		switch(type(get(1))){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ pc = ivalue(get(1)) >= ivalue(get()) ? pc+inst.ISIZE+InstUnless::ISIZE : pc+inst.address; downsize(2); XTAL_VM_CONTINUE(pc); }
				XTAL_CASE(TYPE_FLOAT){ pc = ivalue(get(1)) >= fvalue(get()) ? pc+inst.ISIZE+InstUnless::ISIZE : pc+inst.address; downsize(2); XTAL_VM_CONTINUE(pc); }
			}}
			XTAL_CASE(TYPE_FLOAT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ pc = fvalue(get(1)) >= ivalue(get()) ? pc+inst.ISIZE+InstUnless::ISIZE : pc+inst.address; downsize(2); XTAL_VM_CONTINUE(pc); }
				XTAL_CASE(TYPE_FLOAT){ pc = fvalue(get(1)) >= fvalue(get()) ? pc+inst.ISIZE+InstUnless::ISIZE : pc+inst.address; downsize(2); XTAL_VM_CONTINUE(pc); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc+inst.ISIZE, Xid(op_lt)));
}

const inst_t* VMachine::FunPos(const inst_t* pc){
		XTAL_VM_DEF_INST(Pos);
		switch(type(get())){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ XTAL_VM_CONTINUE(pc + 1); }
			XTAL_CASE(TYPE_FLOAT){ XTAL_VM_CONTINUE(pc + 1); }
		}
		XTAL_VM_CONTINUE(send1(pc, Xid(op_pos)));
}

const inst_t* VMachine::FunNeg(const inst_t* pc){
		XTAL_VM_DEF_INST(Neg);
		switch(type(get())){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ set(Innocence(-ivalue(get()))); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
			XTAL_CASE(TYPE_FLOAT){ set(Innocence(-fvalue(get()))); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
		}
		XTAL_VM_CONTINUE(send1(pc+inst.ISIZE, Xid(op_neg)));
}

const inst_t* VMachine::FunCom(const inst_t* pc){
		XTAL_VM_DEF_INST(Com);
		switch(type(get())){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ set(Innocence(~ivalue(get()))); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
		}
		XTAL_VM_CONTINUE(send1(pc+inst.ISIZE, Xid(op_com)));
}

const inst_t* VMachine::FunSetAt(const inst_t* pc){
		XTAL_VM_DEF_INST(SetAt);
		FunFrame& f = ff();
		XTAL_GLOBAL_INTERPRETER_LOCK{
			Innocence key = pop_and_save2();
			Innocence target = pop_and_save1();
			Innocence value = pop();
			inner_setup_call(pc+inst.ISIZE, 0, ap(key), ap(value));
			ap(target)->rawsend(myself(), Xid(op_set_at), ap(key)->get_class());
		}
		XTAL_VM_CONTINUE(ff().called_pc);
}

const inst_t* VMachine::FunAdd(const inst_t* pc){
		XTAL_VM_DEF_INST(Add);
		switch(type(get(1))){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, Innocence(ivalue(get(1)) + ivalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
				XTAL_CASE(TYPE_FLOAT){ set(1, Innocence(ivalue(get(1)) + fvalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
			}}
			XTAL_CASE(TYPE_FLOAT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, Innocence(fvalue(get(1)) + ivalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
				XTAL_CASE(TYPE_FLOAT){ set(1, Innocence(fvalue(get(1)) + fvalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc+inst.ISIZE, Xid(op_add)));
}

const inst_t* VMachine::FunSub(const inst_t* pc){
		XTAL_VM_DEF_INST(Sub);
		switch(type(get(1))){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, Innocence(ivalue(get(1)) - ivalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
				XTAL_CASE(TYPE_FLOAT){ set(1, Innocence(ivalue(get(1)) - fvalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
			}}
			XTAL_CASE(TYPE_FLOAT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, Innocence(fvalue(get(1)) - ivalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
				XTAL_CASE(TYPE_FLOAT){ set(1, Innocence(fvalue(get(1)) - fvalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc+inst.ISIZE, Xid(op_sub)));
}

const inst_t* VMachine::FunMul(const inst_t* pc){
		XTAL_VM_DEF_INST(Mul);
		switch(type(get(1))){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, Innocence(ivalue(get(1)) * ivalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
				XTAL_CASE(TYPE_FLOAT){ set(1, Innocence(ivalue(get(1)) * fvalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
			}}
			XTAL_CASE(TYPE_FLOAT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, Innocence(fvalue(get(1)) * ivalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
				XTAL_CASE(TYPE_FLOAT){ set(1, Innocence(fvalue(get(1)) * fvalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc+inst.ISIZE, Xid(op_mul)));
}

const inst_t* VMachine::FunDiv(const inst_t* pc){
		XTAL_VM_DEF_INST(Div);
		switch(type(get(1))){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, Innocence(ivalue(get(1)) / check_zero(ivalue(get())))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
				XTAL_CASE(TYPE_FLOAT){ set(1, Innocence(ivalue(get(1)) / check_zero(fvalue(get())))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
			}}
			XTAL_CASE(TYPE_FLOAT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, Innocence(fvalue(get(1)) / check_zero(ivalue(get())))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
				XTAL_CASE(TYPE_FLOAT){ set(1, Innocence(fvalue(get(1)) / check_zero(fvalue(get())))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc+inst.ISIZE, Xid(op_div)));
}

const inst_t* VMachine::FunMod(const inst_t* pc){
		XTAL_VM_DEF_INST(Mod);
		using namespace std;
		switch(type(get(1))){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, Innocence(ivalue(get(1)) % check_zero(ivalue(get())))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
				XTAL_CASE(TYPE_FLOAT){ set(1, Innocence(fmodf((float_t)ivalue(get(1)), fvalue(get())))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
			}}
			XTAL_CASE(TYPE_FLOAT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, Innocence(fmodf(fvalue(get(1)), (float_t)ivalue(get())))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
				XTAL_CASE(TYPE_FLOAT){ set(1, Innocence(fmodf(fvalue(get(1)), fvalue(get())))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc+inst.ISIZE, Xid(op_mod)));
}

const inst_t* VMachine::FunAnd(const inst_t* pc){
		XTAL_VM_DEF_INST(And);
		switch(type(get(1))){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, Innocence(ivalue(get(1)) & ivalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc+inst.ISIZE, Xid(op_and)));
}

const inst_t* VMachine::FunOr(const inst_t* pc){
		XTAL_VM_DEF_INST(Or);
		switch(type(get(1))){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, Innocence(ivalue(get(1)) | ivalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc+inst.ISIZE, Xid(op_or)));
}

const inst_t* VMachine::FunXor(const inst_t* pc){
		XTAL_VM_DEF_INST(Xor);
		switch(type(get(1))){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, Innocence(ivalue(get(1)) ^ ivalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc+inst.ISIZE, Xid(op_xor)));
}

const inst_t* VMachine::FunShl(const inst_t* pc){
		XTAL_VM_DEF_INST(Shl);
		switch(type(get(1))){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, Innocence(ivalue(get(1)) << ivalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc+inst.ISIZE, Xid(op_shl)));
}

const inst_t* VMachine::FunShr(const inst_t* pc){
		XTAL_VM_DEF_INST(Shr);
		switch(type(get(1))){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, Innocence(ivalue(get(1)) >> ivalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc+inst.ISIZE, Xid(op_shr)));
}

const inst_t* VMachine::FunUshr(const inst_t* pc){
		XTAL_VM_DEF_INST(Ushr);
		switch(type(get(1))){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, Innocence((int_t)((uint_t)ivalue(get(1)) >> ivalue(get())))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc+inst.ISIZE, Xid(op_ushr)));
}

const inst_t* VMachine::FunEq(const inst_t* pc){
		XTAL_VM_DEF_INST(Eq);
		switch(type(get(1))){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, Innocence(ivalue(get(1)) == ivalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
				XTAL_CASE(TYPE_FLOAT){ set(1, Innocence(ivalue(get(1)) == fvalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
			}}
			XTAL_CASE(TYPE_FLOAT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, Innocence(fvalue(get(1)) == ivalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
				XTAL_CASE(TYPE_FLOAT){ set(1, Innocence(fvalue(get(1)) == fvalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
			}}
		}
		if(raweq(get(1), get(0))){
			set(1, Innocence(true)); downsize(1);
			XTAL_VM_CONTINUE(pc+inst.ISIZE);
		}
		else{
			XTAL_VM_CONTINUE(send2_q(pc+inst.ISIZE, Xid(op_eq)));
		}
}

const inst_t* VMachine::FunNe(const inst_t* pc){
		XTAL_VM_DEF_INST(Ne);
		switch(type(get(1))){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, Innocence(ivalue(get(1)) != ivalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE+InstNot::ISIZE); }
				XTAL_CASE(TYPE_FLOAT){ set(1, Innocence(ivalue(get(1)) != fvalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE+InstNot::ISIZE); }
			}}
			XTAL_CASE(TYPE_FLOAT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, Innocence(fvalue(get(1)) != ivalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE+InstNot::ISIZE); }
				XTAL_CASE(TYPE_FLOAT){ set(1, Innocence(fvalue(get(1)) != fvalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE+InstNot::ISIZE);}
			}}
		}
		if(rawne(get(1), get(0))){
			set(1, Innocence(true)); downsize(1);
			XTAL_VM_CONTINUE(pc+inst.ISIZE+InstNot::ISIZE);
		}
		else{
			XTAL_VM_CONTINUE(send2_q(pc+inst.ISIZE, Xid(op_eq)));
		}
}

const inst_t* VMachine::FunLt(const inst_t* pc){
		XTAL_VM_DEF_INST(Lt);
		switch(type(get(1))){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, Innocence(ivalue(get(1)) < ivalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
				XTAL_CASE(TYPE_FLOAT){ set(1, Innocence(ivalue(get(1)) < fvalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
			}}
			XTAL_CASE(TYPE_FLOAT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, Innocence(fvalue(get(1)) < ivalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
				XTAL_CASE(TYPE_FLOAT){ set(1, Innocence(fvalue(get(1)) < fvalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc+inst.ISIZE, Xid(op_lt)));
}

const inst_t* VMachine::FunLe(const inst_t* pc){
		XTAL_VM_DEF_INST(Le);
		switch(type(get(1))){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, Innocence(ivalue(get(1)) <= ivalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE+InstNot::ISIZE); }
				XTAL_CASE(TYPE_FLOAT){ set(1, Innocence(ivalue(get(1)) <= fvalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE+InstNot::ISIZE); }
			}}
			XTAL_CASE(TYPE_FLOAT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, Innocence(fvalue(get(1)) <= ivalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE+InstNot::ISIZE); }
				XTAL_CASE(TYPE_FLOAT){ set(1, Innocence(fvalue(get(1)) <= fvalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE+InstNot::ISIZE); }
			}}
		}
		XTAL_VM_CONTINUE(send2_r(pc+inst.ISIZE, Xid(op_lt)));
}

const inst_t* VMachine::FunGt(const inst_t* pc){
		XTAL_VM_DEF_INST(Gt);
		switch(type(get(1))){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, Innocence(ivalue(get(1)) > ivalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
				XTAL_CASE(TYPE_FLOAT){ set(1, Innocence(ivalue(get(1)) > fvalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
			}}
			XTAL_CASE(TYPE_FLOAT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, Innocence(fvalue(get(1)) > ivalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
				XTAL_CASE(TYPE_FLOAT){ set(1, Innocence(fvalue(get(1)) > fvalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
			}}
		}
		XTAL_VM_CONTINUE(send2_r(pc+inst.ISIZE, Xid(op_lt)));
}

const inst_t* VMachine::FunGe(const inst_t* pc){
		XTAL_VM_DEF_INST(Ge);
		switch(type(get(1))){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, Innocence(ivalue(get(1)) >= ivalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE+InstNot::ISIZE); }
				XTAL_CASE(TYPE_FLOAT){ set(1, Innocence(ivalue(get(1)) >= fvalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE+InstNot::ISIZE); }
			}}
			XTAL_CASE(TYPE_FLOAT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, Innocence(fvalue(get(1)) >= ivalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE+InstNot::ISIZE); }
				XTAL_CASE(TYPE_FLOAT){ set(1, Innocence(fvalue(get(1)) >= fvalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE+InstNot::ISIZE); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc+inst.ISIZE, Xid(op_lt)));
}

const inst_t* VMachine::FunInc(const inst_t* pc){
		XTAL_VM_DEF_INST(Inc);
		switch(type(get())){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ set(Innocence(ivalue(get())+1)); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
			XTAL_CASE(TYPE_FLOAT){ set(Innocence(fvalue(get())+1)); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
		}
		XTAL_VM_CONTINUE(send1(pc+inst.ISIZE, Xid(op_inc)));
}

const inst_t* VMachine::FunDec(const inst_t* pc){
		XTAL_VM_DEF_INST(Dec);
		switch(type(get())){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ set(Innocence(ivalue(get())-1)); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
			XTAL_CASE(TYPE_FLOAT){ set(Innocence(fvalue(get())-1)); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
		}
		XTAL_VM_CONTINUE(send1(pc+inst.ISIZE, Xid(op_dec)));
}

const inst_t* VMachine::FunAddAssign(const inst_t* pc){
		XTAL_VM_DEF_INST(AddAssign);
		switch(type(get(1))){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, Innocence(ivalue(get(1)) + ivalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
				XTAL_CASE(TYPE_FLOAT){ set(1, Innocence(ivalue(get(1)) + fvalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
			}}
			XTAL_CASE(TYPE_FLOAT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, Innocence(fvalue(get(1)) + ivalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
				XTAL_CASE(TYPE_FLOAT){ set(1, Innocence(fvalue(get(1)) + fvalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc+inst.ISIZE, Xid(op_add_assign)));
}

const inst_t* VMachine::FunSubAssign(const inst_t* pc){
		XTAL_VM_DEF_INST(SubAssign);
		switch(type(get(1))){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, Innocence(ivalue(get(1)) - ivalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
				XTAL_CASE(TYPE_FLOAT){ set(1, Innocence(ivalue(get(1)) - fvalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
			}}
			XTAL_CASE(TYPE_FLOAT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, Innocence(fvalue(get(1)) - ivalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
				XTAL_CASE(TYPE_FLOAT){ set(1, Innocence(fvalue(get(1)) - fvalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc+inst.ISIZE, Xid(op_sub_assign)));
}

const inst_t* VMachine::FunMulAssign(const inst_t* pc){
		XTAL_VM_DEF_INST(MulAssign);
		switch(type(get(1))){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, Innocence(ivalue(get(1)) * ivalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
				XTAL_CASE(TYPE_FLOAT){ set(1, Innocence(ivalue(get(1)) * fvalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
			}}
			XTAL_CASE(TYPE_FLOAT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, Innocence(fvalue(get(1)) * ivalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
				XTAL_CASE(TYPE_FLOAT){ set(1, Innocence(fvalue(get(1)) * fvalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc+inst.ISIZE, Xid(op_mul_assign)));
}

const inst_t* VMachine::FunDivAssign(const inst_t* pc){
		XTAL_VM_DEF_INST(DivAssign);
		switch(type(get(1))){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, Innocence(ivalue(get(1)) / check_zero(ivalue(get())))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
				XTAL_CASE(TYPE_FLOAT){ set(1, Innocence(ivalue(get(1)) / check_zero(fvalue(get())))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
			}}
			XTAL_CASE(TYPE_FLOAT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, Innocence(fvalue(get(1)) / check_zero(ivalue(get())))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
				XTAL_CASE(TYPE_FLOAT){ set(1, Innocence(fvalue(get(1)) / check_zero(fvalue(get())))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc+inst.ISIZE, Xid(op_div_assign)));
}

const inst_t* VMachine::FunModAssign(const inst_t* pc){
		XTAL_VM_DEF_INST(ModAssign);
		using namespace std;
		switch(type(get(1))){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, Innocence(ivalue(get(1)) % check_zero(ivalue(get())))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
				XTAL_CASE(TYPE_FLOAT){ set(1, Innocence(fmodf((float_t)ivalue(get(1)), fvalue(get())))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
			}}
			XTAL_CASE(TYPE_FLOAT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, Innocence(fmodf(fvalue(get(1)),(float_t)ivalue(get())))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
				XTAL_CASE(TYPE_FLOAT){ set(1, Innocence(fmodf(fvalue(get(1)), fvalue(get())))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc+inst.ISIZE, Xid(op_mod_assign)));
}

const inst_t* VMachine::FunAndAssign(const inst_t* pc){
		XTAL_VM_DEF_INST(AndAssign);
		switch(type(get(1))){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, Innocence(ivalue(get(1)) & ivalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc+inst.ISIZE, Xid(op_and_assign)));
}

const inst_t* VMachine::FunOrAssign(const inst_t* pc){
		XTAL_VM_DEF_INST(OrAssign);
		switch(type(get(1))){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, Innocence(ivalue(get(1)) | ivalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc+inst.ISIZE, Xid(op_or_assign)));
}

const inst_t* VMachine::FunXorAssign(const inst_t* pc){
		XTAL_VM_DEF_INST(XorAssign);
		switch(type(get(1))){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, Innocence(ivalue(get(1)) ^ ivalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc+inst.ISIZE, Xid(op_xor_assign)));
}

const inst_t* VMachine::FunShlAssign(const inst_t* pc){
		XTAL_VM_DEF_INST(ShlAssign);
		switch(type(get(1))){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, Innocence(ivalue(get(1)) << ivalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc+inst.ISIZE, Xid(op_shl_assign)));
}

const inst_t* VMachine::FunShrAssign(const inst_t* pc){
		XTAL_VM_DEF_INST(ShrAssign);
		switch(type(get(1))){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, Innocence(ivalue(get(1)) >> ivalue(get()))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc+inst.ISIZE, Xid(op_shr_assign)));
}

const inst_t* VMachine::FunUshrAssign(const inst_t* pc){
		XTAL_VM_DEF_INST(UshrAssign);
		switch(type(get(1))){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(type(get())){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, Innocence((int_t)((uint_t)ivalue(get(1)) >> ivalue(get())))); downsize(1); XTAL_VM_CONTINUE(pc+inst.ISIZE); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc+inst.ISIZE, Xid(op_ushr_assign)));
}

const inst_t* VMachine::FunClassBegin(const inst_t* pc){
		XTAL_VM_DEF_INST(ClassBegin);
		XTAL_GLOBAL_INTERPRETER_LOCK{
			ClassCore* core = code()->class_core(inst.core_number);
			const FramePtr& outer = (core->flags&ClassCore::FLAG_SCOPE_CHAIN) ? ff().outer() : static_ptr_cast<Frame>(null);
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
		int_t table_n = inst.core_number, end = inst.address;
		XTAL_GLOBAL_INTERPRETER_LOCK{
			FunCore* core = code()->fun_core(table_n);
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

				XTAL_CASE(0){ ret = xnew<InstanceVariableGetter>(inst.number, code()->class_core(inst.core_number)); }
				XTAL_CASE(1){ ret = xnew<InstanceVariableSetter>(inst.number, code()->class_core(inst.core_number)); }
			}
			push(ret);
		}
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
}

const inst_t* VMachine::FunBreakPoint(const inst_t* pc){
		XTAL_VM_DEF_INST(BreakPoint);
		if(debug::is_enabled()){
			XTAL_GLOBAL_INTERPRETER_LOCK{
				int_t kind = inst.type;
				
				if(!debug_info_) debug_info_ = xnew<debug::Info>();
				debug_info_->set_kind(kind);
				debug_info_->set_line(code()->compliant_lineno(pc));
				debug_info_->set_file_name(code()->source_file_name());
				debug_info_->set_fun_name(fun()->object_name());
				debug_info_->set_local_variables(ff().outer());

				struct guard{
					guard(){ debug::disable(); }
					~guard(){ debug::enable(); }
				} g;
			
				switch(kind){
					XTAL_NODEFAULT;
					
					XTAL_CASE(BREAKPOINT_LINE){
						if(AnyPtr hook = debug::line_hook()){
							hook(debug_info_);
						}
					}

					XTAL_CASE(BREAKPOINT_CALL){
						if(AnyPtr hook = debug::call_hook()){
							hook(debug_info_);
						}				
					}

					XTAL_CASE(BREAKPOINT_RETURN){
						if(AnyPtr hook = debug::return_hook()){
							hook(debug_info_);
						}				
					}
				}
			}
		}
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
}

//}}FUNS}
	
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
	
	FunCore* core = fun->core();
	if(int_t size = core->variable_size){
		if(core->flags & FunCore::FLAG_ON_HEAP){
			f.outer(xnew<Frame>(f.outer(), fun->code(), core));
			Frame* frame = f.outer().get();
			for(int_t n = 0; n<size; ++n){
				frame->set_member_direct(n, arg(n, fun));
			}
		}
		else{
			f.variables_.upsize(size);
			Innocence* vars=&f.variables_[size-1];
			for(int_t n = 0; n<size; ++n){
				vars[n] = arg(n, fun);
			}
		}
	}
	
	int_t max_stack = core->max_stack;
	stack_.upsize(max_stack);
	stack_.downsize(max_stack + f.args_stack_size());
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

	FunCore* core = fun->core();
	int_t size = core->variable_size;
	adjust_result(f.ordered_arg_count, size);
	f.named_arg_count = 0;
	f.ordered_arg_count = size;

	if(size){
		if(core->flags & FunCore::FLAG_ON_HEAP){
			f.outer(xnew<Frame>(f.outer(), fun->code(), core));
			Frame* frame = f.outer().get();
			for(int_t n = 0; n<size; ++n){
				frame->set_member_direct(n, get(size-1-n));
			}
		}
		else{
			f.variables_.upsize(size);	
			Innocence* vars=&f.variables_[size-1];
			for(int_t n = 0; n<size; ++n){
				vars[n] = get(size-1-n);
			}
		}
	}	
	
	int_t max_stack = core->max_stack;
	stack_.upsize(max_stack);
	stack_.downsize(max_stack + size);
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
				// 最後の要素が配列ではないので、voidで埋めとく
				for(int_t i = n; i<need_result_count; ++i){
					push(undefined);
				}
			}
		}
	}
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

void VMachine::set_local_variable(int_t pos, const Innocence& value){
	pos -= ff().variables_.size();
	Frame* outer = ff().outer().get();
	XTAL_GLOBAL_INTERPRETER_LOCK{
		while(1){
			int_t variables_size = outer->block_size();
			if(pos<variables_size){
				outer->set_member_direct(variables_size-1-pos, ap(value));
				return;
			}
			pos-=variables_size;
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

ArgumentsPtr VMachine::make_arguments(){
	ArgumentsPtr p = xnew<Arguments>();

	for(int_t i = 0, size = ordered_arg_count(); i<size; ++i){
		p->ordered_->push_back(arg(i));
	}

	for(int_t i = 0, size = named_arg_count(); i<size; ++i){
		p->named_->set_at(get(size*2-1-(i*2+0)), get(size*2-1-(i*2+1)));
	}
	return p;
}

ArgumentsPtr VMachine::make_args(Fun* fun){
	ArgumentsPtr p = xnew<Arguments>();

	for(int_t i = fun->param_size(), size = ff().ordered_arg_count; i<size; ++i){
		p->ordered_->push_back(get(ff().ordered_arg_count+ff().named_arg_count*2-1-i));
	}

	IDPtr name;
	for(int_t i = 0, size = ff().named_arg_count; i<size; ++i){
		name = (IDPtr&)get(size*2-1-(i*2+0));
		for(int_t j = 0; j<fun->param_size(); ++j){
			if(fun->param_name_at(j)==name){
				name = null;
				break;
			}
		}
		if(name){
			p->named_->set_at(name, get(size*2-1-(i*2+1)));
		}
	}

	return p;
}

AnyPtr VMachine::append_backtrace(const inst_t* pc, const AnyPtr& e){
	if(e){
		AnyPtr ep = e;
		if(!ep->is(builtin()->member("Exception"))){
			ep = builtin()->member("RuntimeError")(ep);
		}
		if(fun() && code()){
			if((pc != code()->data()+code()->size()-1)){
				ep->send("append_backtrace")(
					code()->source_file_name(),
					code()->compliant_lineno(pc),
					fun()->object_name());
			}
		}
		else{
			/*ep->send("append_backtrace")(
				"C++ function",
				-1,
				ap(ff().temp1_)->object_name()
			);*/
		}
		return ep;
	}
	return e;
}

void VMachine::hook_return(const inst_t* pc){
	if(debug::is_enabled() && fun() && code()){
		if(!debug_info_) debug_info_ = xnew<debug::Info>();
		debug_info_->set_kind(BREAKPOINT_RETURN);
		debug_info_->set_line(code()->compliant_lineno(pc));
		debug_info_->set_file_name(code()->source_file_name());
		debug_info_->set_fun_name(fun()->object_name());
		debug_info_->set_local_variables(ff().outer());

		struct guard{
			guard(){ debug::disable(); }
			~guard(){ debug::enable(); }
		} g;
	
		if(AnyPtr hook = debug::return_hook()){
			hook(debug_info_);
		}				
	}
}

const inst_t* VMachine::catch_body(const inst_t* pc, int_t stack_size, int_t fun_frames_size){
	XTAL_GLOBAL_INTERPRETER_LOCK{

		AnyPtr e = except();

		// try .. catch .. finally文で囲われていない
		if(except_frames_.empty()){
			while((size_t)fun_frames_size<fun_frames_.size()){
				hook_return(pc);
				pc = pop_ff();
				e = append_backtrace(pc, e);
			}
			stack_.downsize_n(stack_size);
			except_[0] = e;
			return 0;
		}

		ExceptFrame& ef = except_frames_.top();
		while((size_t)ef.fun_frame_count<fun_frames_.size()){
			hook_return(pc);
			pc = pop_ff();
			e = append_backtrace(pc, e);

			if(pc==&end_code_){
				stack_.downsize_n(stack_size);
				except_[0] = e;
				return 0;
			}
		}

		ff().variables_.downsize_n(ef.variable_size);
		if(ap(ef.outer)){
			while(!raweq(ef.outer, ff().outer())){
				ff().outer(ff().outer()->outer());
			}
		}

		stack_.downsize_n(ef.stack_count);
		if(ef.core->catch_pc && e){
			pc = ef.core->catch_pc + code()->data();
			push(AnyPtr(ef.core->end_pc));
			push(e);
			except_[1] = null;
			except_[0] = null;
		}
		else{
			pc = ef.core->finally_pc + code()->data();
			push(e);
			push(AnyPtr(code()->size()-1));
			except_[1] = except_[0];
			except_[0] = null;
		}

		except_frames_.downsize(1);
		return pc;
	}
	return 0;
}


void VMachine::visit_members(Visitor& m){
	GCObserver::visit_members(m);
	m & debug_info_ & except_[0] & except_[1] & except_[2];
	
	for(int_t i=0, size=stack_.size(); i<size; ++i){
		m & stack_[i];
	}

	for(int_t i=0, size=fun_frames_.size(); i<size; ++i){
		m & fun_frames_[i];
	}
}

void VMachine::before_gc(){
	stack_.fill_over();
	fun_frames_.fill_over();

	inc_ref_count_force(except_[0]);
	inc_ref_count_force(except_[1]);
	inc_ref_count_force(except_[2]);

	for(int_t i=0, size=stack_.size(); i<size; ++i){
		inc_ref_count_force(stack_[i]);
	}

	for(int_t i=0, size=fun_frames_.size(); i<size; ++i){
		fun_frames_[i].inc_ref();
	}
}

void VMachine::after_gc(){
	dec_ref_count_force(except_[0]);
	dec_ref_count_force(except_[1]);
	dec_ref_count_force(except_[2]);

	for(int_t i=0, size=stack_.size(); i<size; ++i){
		dec_ref_count_force(stack_[i]);
	}

	for(int_t i=0, size=fun_frames_.size(); i<size; ++i){
		fun_frames_[i].dec_ref();
	}
}


void VMachine::print_info(){
	std::printf("stack size %d\n", stack_.size());
	for(uint_t i=0; i<stack_.size(); ++i){
		std::printf("\tstack value %d = %d\n", i, ap(stack_[i])->to_s()->c_str());
	}

	std::printf("fun_frames size %d\n", fun_frames_.size());
	std::printf("except_frames size %d\n", except_frames_.size());
}


void VMachine::FunFrame::inc_ref(){
	inc_ref_count_force(fun_);
	inc_ref_count_force(outer_);
	
	for(int_t i=0, size=variables_.size(); i<size; ++i){
		inc_ref_count_force(variables_[i]);
	}
	
	inc_ref_count_force(self_);
	inc_ref_count_force(arguments_);
	inc_ref_count_force(hint_);

	inc_ref_count_force(temp1_);
	inc_ref_count_force(temp2_);
	inc_ref_count_force(temp3_);
}

void VMachine::FunFrame::dec_ref(){
	dec_ref_count_force(fun_);
	dec_ref_count_force(outer_);
	
	for(int_t i=0, size=variables_.size(); i<size; ++i){
		dec_ref_count_force(variables_[i]);
	}
	
	dec_ref_count_force(self_);
	dec_ref_count_force(arguments_);
	dec_ref_count_force(hint_);

	dec_ref_count_force(temp1_);
	dec_ref_count_force(temp2_);
	dec_ref_count_force(temp3_);
}
	
void visit_members(Visitor& m, const VMachine::FunFrame& v){
	m & v.fun_ & v.outer_ & v.arguments_ & v.hint_ & v.self_ & v.temp1_ & v.temp2_ & v.temp3_;
	for(int_t i=0, size=v.variables_.size(); i<size; ++i){
		m & v.variable(i);
	}
}

}//namespace

