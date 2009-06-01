#include "xtal.h"
#include "xtal_macro.h"

namespace xtal{

VMachine::VMachine(){
	id_ = id_op_list();

	stack_.reserve(32);

	end_code_ = InstExit::NUMBER;
	throw_code_ = InstThrow::NUMBER;
	throw_unsupported_error_code_ = InstThrowUnsupportedError::NUMBER;
	check_unsupported_code_ = InstCheckUnsupported::NUMBER;
	cleanup_call_code_ = InstCleanupCall::NUMBER;
	resume_pc_ = 0;
	disable_debug_ = false;

	variables_.resize(10000);
	variables_top_ = 0;
}

VMachine::~VMachine(){
	variables_.clear_unref();
	fun_frames_.resize(fun_frames_.capacity());
	for(int_t i=0, size=fun_frames_.capacity(); i<size; ++i){
		if(fun_frames_[i]){
			fun_frames_[i]->~FunFrame();
			xfree(fun_frames_[i], sizeof(FunFrame));
		}
	}

	while(scopes_.size()){
		pop_scope();
	}
}

void VMachine::reset(){
	stack_.resize(0);
	except_frames_.resize(0);
	fun_frames_.resize(0);
	
	while(scopes_.size()){
		pop_scope();
	}

	variables_top_ = 0;

	except_[0] = null;
	except_[1] = null;
	except_[2] = null;
	debug_info_ = null;
}

VMachine::FunFrame::FunFrame(){
}

void VMachine::FunFrame::set_null(){
	xtal::set_null(fun_); 
	xtal::set_null(self_);
	xtal::set_null(target_);
	xtal::set_null(primary_key_);
	xtal::set_null(secondary_key_);
}

//{REPEAT{{
/*
void VMachine::setup_call(int_t need_result_count, const Param& a0 #COMMA_REPEAT#const Param& a`i+1`#){
	push_ff();
	set_ff(&end_code_, &throw_unsupported_error_code_, need_result_count, 0, 0, undefined);
	push_arg(a0);
	#REPEAT#push_arg(a`i+1`);# 
}
*/

void VMachine::setup_call(int_t need_result_count, const Param& a0 ){
	push_ff();
	set_ff(&end_code_, &throw_unsupported_error_code_, need_result_count, 0, 0, undefined);
	push_arg(a0);
	 
}

void VMachine::setup_call(int_t need_result_count, const Param& a0 , const Param& a1){
	push_ff();
	set_ff(&end_code_, &throw_unsupported_error_code_, need_result_count, 0, 0, undefined);
	push_arg(a0);
	push_arg(a1); 
}

void VMachine::setup_call(int_t need_result_count, const Param& a0 , const Param& a1, const Param& a2){
	push_ff();
	set_ff(&end_code_, &throw_unsupported_error_code_, need_result_count, 0, 0, undefined);
	push_arg(a0);
	push_arg(a1);push_arg(a2); 
}

void VMachine::setup_call(int_t need_result_count, const Param& a0 , const Param& a1, const Param& a2, const Param& a3){
	push_ff();
	set_ff(&end_code_, &throw_unsupported_error_code_, need_result_count, 0, 0, undefined);
	push_arg(a0);
	push_arg(a1);push_arg(a2);push_arg(a3); 
}

void VMachine::setup_call(int_t need_result_count, const Param& a0 , const Param& a1, const Param& a2, const Param& a3, const Param& a4){
	push_ff();
	set_ff(&end_code_, &throw_unsupported_error_code_, need_result_count, 0, 0, undefined);
	push_arg(a0);
	push_arg(a1);push_arg(a2);push_arg(a3);push_arg(a4); 
}

void VMachine::setup_call(int_t need_result_count, const Param& a0 , const Param& a1, const Param& a2, const Param& a3, const Param& a4, const Param& a5){
	push_ff();
	set_ff(&end_code_, &throw_unsupported_error_code_, need_result_count, 0, 0, undefined);
	push_arg(a0);
	push_arg(a1);push_arg(a2);push_arg(a3);push_arg(a4);push_arg(a5); 
}

//}}REPEAT}

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

void VMachine::push_arg(const Param& p){
	switch(p.type>>Param::SHIFT){
		XTAL_NODEFAULT;

		XTAL_CASE(0){
			Any temp;
			set_type_value(temp, p.type, p.value);
			push(ap(temp));
			ff().ordered_arg_count++;
		}

		XTAL_CASE(Param::NAMED>>Param::SHIFT){
			push(*p.name.name);
			push(*p.name.value);
			ff().named_arg_count++;
		}

		XTAL_CASE(Param::STR>>Param::SHIFT){
			push(xnew<String>(p.str));
			ff().ordered_arg_count++;
		}

		XTAL_CASE(Param::STR8>>Param::SHIFT){
			push(xnew<String>(p.str8));
			ff().ordered_arg_count++;
		}
	}
}
	
void VMachine::push_ordered_args(const ArrayPtr& p){ 
	Xfor(v, p){
		push_arg(v);
	}
}

void VMachine::push_named_args(const MapPtr& p){ 
	Xfor2(k, v, p){
		push_arg(ptr_cast<ID>(k), v);
	}
}
	
void VMachine::execute(Method* fun, const inst_t* start_pc){
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

const AnyPtr& VMachine::result(int_t pos){
	const inst_t* temp;

	{
		FunFrame& f = ff();

		// ��O���N���A����Ă��Ȃ��Ȃ���s�ł��Ȃ�
		if(ap(except_[0])){
			downsize(f.args_stack_size());
			upsize(f.need_result_count);
			return undefined;
		}

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
		
		if(ap(except_[0])){
			f.need_result_count = 0;
			return undefined;
		}

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
	pop_ff2();
	return ret;
}
	
void VMachine::cleanup_call(){
	result(0);
	downsize(ff().need_result_count);
	pop_ff2();
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

void VMachine::adjust_args(const NamedParam* params, int_t num){
	FunFrame& f = ff();
	int_t offset = f.named_arg_count*2;
	int_t k = 0;
	for(int_t j=f.ordered_arg_count; j<num; ++j){
		bool hit = false;
		for(int_t i = 0, sz = f.named_arg_count; i<sz; ++i){
			if(raweq(get(sz*2-1-(i*2+0)+k), params[j].name)){
				stack_.push(get(sz*2-1-(i*2+1)+k));
				hit = true;
				break;
			}
		}

		if(!hit){
			stack_.push(params[j].value);
		}

		k++;
	}

	stack_.erase(k+offset-1, offset);
	f.ordered_arg_count += k;
	f.named_arg_count = 0;
}

void VMachine::adjust_args(Method* names, int_t num){
	FunFrame& f = ff();
	int_t offset = f.named_arg_count*2;
	int_t k = 0;
	for(int_t j=f.ordered_arg_count; j<num; ++j){
		bool hit = false;
		for(int_t i = 0, sz = f.named_arg_count; i<sz; ++i){
			if(raweq(get(sz*2-1-(i*2+0)+k), names->param_name_at(j))){
				stack_.push(get(sz*2-1-(i*2+1)+k));
				hit = true;
				break;
			}
		}

		if(!hit){
			stack_.push(undefined);
		}

		k++;
	}

	stack_.erase(k+offset-1, offset);
	f.ordered_arg_count += k;
	f.named_arg_count = 0;
}

void VMachine::return_result(){
	FunFrame& f = ff();

	downsize(f.args_stack_size());
	if(f.need_result_count!=f.result_count+0){
		adjust_result(f.result_count+0);
	}

	f.called_pc = &cleanup_call_code_;
}

void VMachine::return_result(const AnyPtr& value1){
	FunFrame& f = ff();

	downsize(f.args_stack_size());
	push(value1);
	if(f.need_result_count!=f.result_count+1){
		adjust_result(f.result_count+1);
	}

	f.called_pc = &cleanup_call_code_;
}
	
void VMachine::return_result(const AnyPtr& value1, const AnyPtr& value2){
	FunFrame& f = ff();

	downsize(f.args_stack_size());
	push(value1);
	push(value2);
	adjust_result(f.result_count+2);

	f.called_pc = &cleanup_call_code_;
}

void VMachine::return_result(const AnyPtr& value1, const AnyPtr& value2, const AnyPtr& value3){
	FunFrame& f = ff();

	downsize(f.args_stack_size());
	push(value1);
	push(value2);
	push(value3);
	adjust_result(f.result_count+3);

	f.called_pc = &cleanup_call_code_;
}
	
void VMachine::return_result(const AnyPtr& value1, const AnyPtr& value2, const AnyPtr& value3, const AnyPtr& value4){
	FunFrame& f = ff();

	downsize(f.args_stack_size());
	push(value1);
	push(value2);
	push(value3);
	push(value4);
	adjust_result(f.result_count+4);

	f.called_pc = &cleanup_call_code_;
}

void VMachine::return_result_mv(const ValuesPtr& values){
	FunFrame& f = ff();
	downsize(f.args_stack_size());
	adjust_result(push_mv(values));
	f.called_pc = &cleanup_call_code_;
}

void VMachine::return_result_args(const VMachinePtr& vm){
	FunFrame& f = ff();
	vm->downsize(vm->named_arg_count()*2);
	push(vm.get(), vm->ordered_arg_count());
	vm->upsize(vm->named_arg_count()*2);
	adjust_result(f.result_count+vm->ordered_arg_count());
	f.called_pc = &cleanup_call_code_;
}

void VMachine::prereturn_result(const AnyPtr& v){
	ff().result_count++;
	stack_.insert(ff().args_stack_size(), v);
}

void VMachine::present_for_vm(Fiber* fun, VMachine* vm, bool add_succ_or_fail_result){
	if(const AnyPtr& e = catch_except()){
		vm->set_except(e);
		vm->push(undefined);
		vm->adjust_result(add_succ_or_fail_result?1:0);
		return;
	}

	// ���ʂ�vm�ɓn��
	if(vm->need_result()){
		if(add_succ_or_fail_result){
			if(resume_pc_!=0){
				vm->push(to_smartptr(fun));
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
	vm->ff().called_pc = &cleanup_call_code_;
}

const inst_t* VMachine::start_fiber(Fiber* fun, VMachine* vm, bool add_succ_or_fail_result){
	yield_result_count_ = 0;
	push_ff();
	set_ff(&end_code_, &throw_unsupported_error_code_, vm->need_result_count(), vm->ordered_arg_count(), vm->named_arg_count(), vm->arg_this());
	move(vm, vm->ordered_arg_count()+vm->named_arg_count()*2);
	resume_pc_ = 0;
	carry_over(fun);
	ff().yieldable = true;
	execute_inner(ff().called_pc);
	present_for_vm(fun, vm, add_succ_or_fail_result);
	return resume_pc_;
}

const inst_t* VMachine::resume_fiber(Fiber* fun, const inst_t* pc, VMachine* vm, bool add_succ_or_fail_result){
	yield_result_count_ = 0;
	ff().called_pc = pc;
	resume_pc_ = 0;
	move(vm, vm->ordered_arg_count()+vm->named_arg_count()*2);
	execute_inner(ff().called_pc);
	present_for_vm(fun, vm, add_succ_or_fail_result);
	return resume_pc_;
}

void VMachine::exit_fiber(){
	yield_result_count_ = 0;
	ff().called_pc = resume_pc_;
	resume_pc_ = 0;

	push(undefined);
	execute_inner(&throw_code_);

	reset();
}

void VMachine::flatten_args(){
	FunFrame& f = ff();

	stack_.downsize(f.named_arg_count*2);
	adjust_result(f.ordered_arg_count, 1);
	f.named_arg_count = 0;
	f.ordered_arg_count = 1;

	AnyPtr a = arg(0);
	if(type(a)==TYPE_UNDEFINED){
		downsize(1);
		f.ordered_arg_count = 0;
	}
	else if(type(a)==TYPE_VALUES){
		downsize(1);
		f.ordered_arg_count = push_mv(unchecked_ptr_cast<Values>(a));
	}
}

ArgumentsPtr VMachine::make_arguments(){
	ArgumentsPtr p = xnew<Arguments>();

	for(int_t i = 0, size = ordered_arg_count(); i<size; ++i){
		p->add_ordered(arg(i));
	}

	for(int_t i = 0, size = named_arg_count(); i<size; ++i){
		p->add_named(get(size*2-1-(i*2+0)), get(size*2-1-(i*2+1)));
	}
	return p;
}

ArgumentsPtr VMachine::inner_make_arguments(Method* fun){
	ArgumentsPtr p = xnew<Arguments>();

	ArrayPtr ordered;
	for(int_t i = fun->param_size(), size = ff().ordered_arg_count; i<size; ++i){
		p->add_ordered(get(ff().ordered_arg_count+ff().named_arg_count*2-1-i));
	}

	MapPtr named;
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
			p->add_named(name, get(size*2-1-(i*2+1)));
		}
	}

	return p;
}

AnyPtr VMachine::append_backtrace(const inst_t* pc, const AnyPtr& e){
	if(e){
		AnyPtr ep = e;
		if(!ep->is(cpp_class<Exception>())){
			ep = cpp_class<RuntimeError>()->call(ep);
		}

		if(fun() && fun()->code()){
			if((pc !=  fun()->code()->data() + fun()->code()->size()-1)){
				unchecked_ptr_cast<Exception>(ep)->append_backtrace(
					 fun()->code()->source_file_name(),
					 fun()->code()->compliant_lineno(pc),
					fun()->object_name());
			}
		}
		else{
			/*ep->send(Xid(append_backtrace))(
				"C++ function",
				-1,
				ap(ff().temp1_)->object_name()
			);*/
		}
		return ep;
	}
	return e;
}

void VMachine::set_except_0(const Any& e){
	except_[0] = e;
}
	
void VMachine::make_debug_info(const inst_t* pc, int_t kind){
	if(!debug_info_){ 
		debug_info_ = xnew<debug::HookInfo>(); 
	}

	debug_info_->set_kind(kind);
	if(fun()){
		debug_info_->set_line(fun()->code()->compliant_lineno(pc));
		debug_info_->set_file_name(fun()->code()->source_file_name());
		debug_info_->set_fun_name(fun()->object_name());
	}
	else{
		debug_info_->set_line(0);
		debug_info_->set_file_name("?");
		debug_info_->set_fun_name("?");
	}

	if(kind==BREAKPOINT_ASSERT){
		debug_info_->set_assertion_message(pop()->to_s());
	}
	else{
		debug_info_->set_assertion_message(empty_string);
	}

	if(kind==BREAKPOINT_THROW){
		debug_info_->set_exception(ap(except_[0]));
	}
	else{
		debug_info_->set_exception(null);
	}

	make_outer_outer();
	debug_info_->set_variables_frame(scopes_.top());
	scopes_.top()->recycle_ = false;

	debug_info_->set_vm(to_smartptr(this));
	debug_info_->funframe_ = fun_frames_.size()-2;
}

debug::CallerInfoPtr VMachine::caller(uint_t n){
	if(n>=fun_frames_.size()){
		return null;
	}

	FunFrame& f = *fun_frames_.reverse_at(n);
	FunFrame& pf = *fun_frames_.reverse_at(n+1);

	if(!f.fun()){
		return null;
	}

	debug::CallerInfoPtr ret = xnew<debug::CallerInfo>();
	ret->set_line(f.fun()->code()->compliant_lineno(f.poped_pc));
	ret->set_fun_name(f.fun()->object_name());
	ret->set_file_name(f.fun()->code()->source_file_name());
	make_outer_outer();
	ret->set_variables_frame(scopes_.reverse_at(pf.scope_size-1));
	return ret;
}

void VMachine::debug_hook(const inst_t* pc, int_t kind){
	{
		struct guard{
			bool& disable_debug_;
			guard(bool& disable_debug):disable_debug_(disable_debug){ disable_debug_ = true; }
			~guard(){ disable_debug_ = false; }
		} g(disable_debug_);

		AnyPtr e = ap(except_[0]);
		except_[0] = null;

		make_debug_info(pc, kind);

		switch(kind){
			XTAL_CASE(BREAKPOINT){
				if(const AnyPtr& hook = debug::break_point_hook()){
					hook->call(debug_info_);
				}
			}

			XTAL_CASE(BREAKPOINT_RETURN){
				if(const AnyPtr& hook = debug::return_hook()){
					hook->call(debug_info_);
				}
			}

			XTAL_CASE(BREAKPOINT_CALL){
				if(const AnyPtr& hook = debug::call_hook()){
					hook->call(debug_info_);
				}
			}

			XTAL_CASE(BREAKPOINT_THROW){
				if(const AnyPtr& hook = debug::throw_hook()){
					hook->call(debug_info_);
				}
			}

			XTAL_CASE(BREAKPOINT_ASSERT){
				if(const AnyPtr& hook = debug::assert_hook()){
					hook->call(debug_info_);
				}
				else{
					set_except(cpp_class<AssertionFailed>()->call(debug_info_->assertion_message()));
					e = ap(except_[0]);
				}
			}
		}

		except_[0] = e;
	}
}

const inst_t* VMachine::catch_body(const inst_t* pc, const ExceptFrame& nef){
	AnyPtr e = catch_except();

	ExceptFrame ef = except_frames_.empty() ? nef : except_frames_.top();
	uint_t fs = fun_frames_.size();

	// Xtal�̊֐���E�o���Ă���
	while((size_t)ef.fun_frame_size<fun_frames_.size()){
		while(scopes_.size()>ff().scope_size+1){
			pop_scope();
		}

		check_debug_hook(pc, BREAKPOINT_RETURN);
		pc = pop_ff();
		e = append_backtrace(pc, e);

		// C�̊֐��ɂԂ�����
		if(pc==&end_code_){
			ef.info = 0;
			break;
		}
	}

	//XTAL_ASSERT(scopes_.size()>=ef.scope_size);

	while(scopes_.size()<ef.scope_size){
		push_scope();
	}

	while(scopes_.size()>ef.scope_size){
		pop_scope();
	}

	stack_.resize(ef.stack_size);

	if(ef.info){
		if(ef.info->catch_pc && e){
			pc = ef.info->catch_pc +  fun()->code()->data();
			push(AnyPtr(ef.info->end_pc));
			push(e);
		}
		else{
			pc = ef.info->finally_pc +  fun()->code()->data();
			push(e);
			push(AnyPtr(fun()->code()->size()-1));
		}

		except_frames_.downsize(1);
		return pc;
	}
	else{
		set_except_0(e);
	}

	while(scopes_.size()>ff().scope_size){
		pop_scope();
	}
	pop_ff2();

	return 0;
}

const AnyPtr& VMachine::catch_except(){
	except_[2] = except();
	except_[0] = null;
	return ap(except_[2]);
}

void VMachine::set_except(const AnyPtr& e){
	if(!ap(except_[0])){
		except_[0] = e;
	}
	else{
		XTAL_ASSERT(false); // ��O���n���h�������Ɏ��̗�O��ݒ肵��
	}
}

const inst_t* VMachine::push_except(const inst_t* pc){
	push(except_[0]);
	except_[0] = null;
	throw_pc_ = pc+1;
	return &throw_code_;
}
	
const inst_t* VMachine::push_except(const inst_t* pc, const AnyPtr& e){
	push(e);
	except_[0] = null;
	throw_pc_ = pc+1;
	return &throw_code_;
}

void VMachine::visit_members(Visitor& m){
	GCObserver::visit_members(m);
	m & debug_info_ & except_[0] & except_[1] & except_[2];

	for(int_t i=0, size=stack_.size(); i<size; ++i){
		m & stack_[i];
	}

	for(int_t i=0, size=fun_frames_.size(); i<size; ++i){
		if(fun_frames_[i]){
			m & *fun_frames_[i];
		}
	}

	for(int_t i=0, size=scopes_.size(); i<size; ++i){
		m & scopes_[i];
		//for(int_t j=0, jsize=scopes_[i]->members_.size(); j<jsize; ++j){
		//	m & scopes_[i]->members_.at(j);
		//}
	}

	for(int_t i=0, size=variables_.size(); i<size; ++i){
		//m & variables_.at(i);
	}
}

void VMachine::add_ref_count_members(int_t n){
	add_ref_count_force(debug_info_, n);

	add_ref_count_force(except_[0], n);
	add_ref_count_force(except_[1], n);
	add_ref_count_force(except_[2], n);


	for(int_t i=0, size=stack_.size(); i<size; ++i){
		add_ref_count_force(stack_[i], n);
	}

	for(int_t i=0, size=fun_frames_.size(); i<size; ++i){
		if(FunFrame* f = fun_frames_[i]){
			add_ref_count_force(f->fun_, n);
			add_ref_count_force(f->self_, n);
			add_ref_count_force(f->target_, n);
			add_ref_count_force(f->primary_key_, n);
			add_ref_count_force(f->secondary_key_, n);
		}
	}

	for(int_t i=0, size=scopes_.size(); i<size; ++i){
		if(scopes_[i]){
			scopes_[i]->add_ref_count_members(n);
		}
	}

	for(int_t i=0, size=variables_.size(); i<size; ++i){
		//add_ref_count_force(variables_.at(i), n);
	}
}

void VMachine::before_gc(){
	if(stack_.empty() && except_frames_.empty() && fun_frames_.empty() && scopes_.empty()){
	//	return;
	}

	add_ref_count_members(1);

	//*
	// �g���Ă��Ȃ�������null�œh��Ԃ�
	for(int_t i=stack_.size(), size=stack_.capacity(); i<size; ++i){
		set_null(stack_.reverse_at_unchecked(i));
	}

	for(int_t i=fun_frames_.size(), size=fun_frames_.capacity(); i<size; ++i){
		if(fun_frames_.reverse_at_unchecked(i)){
			fun_frames_.reverse_at_unchecked(i)->set_null();
		}
	}

	//*/
}

void VMachine::after_gc(){
	if(stack_.empty() && except_frames_.empty() && fun_frames_.empty() && scopes_.empty()){
	//	return;
	}

	add_ref_count_members(-1);
}


void VMachine::print_info(){
#ifdef XTAL_DEBUG_PRINT
	std::printf("stack size %d\n", stack_.size());
	for(uint_t i=0; i<stack_.size(); ++i){
//		std::printf("\tstack value %d = %s\n", i, ap(stack_[i])->to_s()->c_str());
	}

	std::printf("fun_frames size %d\n", fun_frames_.size());
	std::printf("except_frames size %d\n", except_frames_.size());
	std::printf("scopes size %d\n", scopes_.size());
#endif
}
	
void visit_members(Visitor& m, VMachine::FunFrame& v){
	m & v.fun_ & v.self_ & v.target_ & v.primary_key_ & v.secondary_key_;
}

void VMachine::make_procedure(const VMachinePtr& vm){
	/*
	reset();

	{
		FunFrame* f = vm->fun_frames_.pop();
		ScopeInfo* s = vm->scopes_.pop();
		vm->make_outer_outer();
		vm->fun_frames_.push(f);
		vm->scopes_.push(s);
	}

	resume_pc_ = vm->resume_pc_;
	yield_result_count_ = vm->yield_result_count_;

	stack_ = vm->stack_;

	fun_frames_.resize(vm->fun_frames_.size());
	for(uint_t i=0; i<vm->fun_frames_.size(); ++i){
		if(fun_frames_[i]){
			*fun_frames_[i] = *vm->fun_frames_[i];
		}
		else{
			void* p = xmalloc(sizeof(FunFrame));
			fun_frames_[i] = new(p) FunFrame(*vm->fun_frames_[i]);
		}
	}

	variables_ = vm->variables_;
	scopes_ = vm->scopes_;
	except_frames_ = vm->except_frames_;
	
	except_[0] = vm->except_[0];
	except_[1] = vm->except_[1];
	except_[2] = vm->except_[2];
	*/
}

void VMachine::swap_procedure(const VMachinePtr& vm){
	/*
	using namespace std;

	std::swap(resume_pc_, vm->resume_pc_);
	std::swap(yield_result_count_, vm->yield_result_count_);

	swap(stack_, vm->stack_);
	swap(fun_frames_, vm->fun_frames_);
	swap(variables_, vm->variables_);
	swap(scopes_, vm->scopes_);
	swap(except_frames_, vm->except_frames_);

	swap(except_[0], vm->except_[0]);
	swap(except_[1], vm->except_[1]);
	swap(except_[2], vm->except_[2]);

	swap(debug_info_, vm->debug_info_);
	*/
}


}//namespace
