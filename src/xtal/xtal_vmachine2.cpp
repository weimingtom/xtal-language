#include "xtal.h"
#include "xtal_macro.h"

namespace xtal{

VMachine::VMachine(){
	id_ = id_op_list();

	stack_.reserve(32);

	end_code_ = InstExit::NUMBER;
	throw_code_ = InstThrow::NUMBER;
	throw_unsupported_error_code_ = InstThrowUnsupportedError::NUMBER;
	throw_undefined_code_ = InstThrowUndefined::NUMBER;
	check_unsupported_code_ = InstCheckUnsupported::NUMBER;
	cleanup_call_code_ = InstCleanupCall::NUMBER;
	resume_pc_ = 0;
}

VMachine::~VMachine(){
	fun_frames_.resize(fun_frames_.capacity());
	for(int_t i=0, size=fun_frames_.capacity(); i<size; ++i){
		if(fun_frames_[i]){
			fun_frames_[i]->~FunFrame();
			xfree(fun_frames_[i], sizeof(FunFrame));
		}
	}
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

void VMachine::push_ff(int_t need_result_count){
	push(null);
	push_call(&end_code_, need_result_count, 0, 0, null, undefined, null);
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
	
void VMachine::push_ordered_args(const ArrayPtr& p){ 
	Xfor(v, p){
		push_arg(v);
	}
}

void VMachine::push_named_args(const MapPtr& p){ 
	Xfor2(k, v, p){
		push_arg(cast<const IDPtr&>(k), v);
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
	scopes_.push(0);

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
	scopes_.push(0);

	{
		FunFrame& f = ff();

		f.poped_pc = temp;
		f.called_pc = &cleanup_call_code_;
		
		if(type(except_[0])!=TYPE_NULL){
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

const AnyPtr& VMachine::arg(int_t pos, Method* names){
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

const AnyPtr& VMachine::arg_default(const Named& name_and_def){
	FunFrame& f = ff();
	for(int_t i = 0, sz = f.named_arg_count; i<sz; ++i){
		if(raweq(get(sz*2-1-(i*2+0)), name_and_def.name)){
			return get(sz*2-1-(i*2+1));
		}
	}
	return name_and_def.value;
}

const AnyPtr& VMachine::arg_default(int_t pos, const Named& name_and_def){
	FunFrame& f = ff();
	if(pos<f.ordered_arg_count){
		return get(f.args_stack_size()-1-pos);
	}
	return arg_default(name_and_def);
}

void VMachine::adjust_args(const Named* params, int_t num){
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

void VMachine::return_result_mv(const MultiValuePtr& values){
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
		return;
	}

	// åãâ ÇvmÇ…ìnÇ∑
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
}

const inst_t* VMachine::start_fiber(Fiber* fun, VMachine* vm, bool add_succ_or_fail_result){
	yield_result_count_ = 0;
	push(null);
	push_call(&end_code_, vm->need_result_count(), vm->ordered_arg_count(), vm->named_arg_count(), null, undefined, vm->arg_this());
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
	yield_result_count_ = 0;
	ff().called_pc = resume_pc_;
	resume_pc_ = 0;
	execute_inner(&throw_undefined_code_);
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
	else if(type(a)==TYPE_MULTI_VALUE){
		downsize(1);
		f.ordered_arg_count = push_mv(unchecked_ptr_cast<MultiValue>(a));
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

		if(fun() &&  fun()->code()){
			if((pc !=  fun()->code()->data()+ fun()->code()->size()-1)){
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
		debug_info_ = xnew<DebugInfo>(); 
	}

	debug_info_->set_kind(kind);
	if(fun()){
		debug_info_->set_line( fun()->code()->compliant_lineno(pc));
		debug_info_->set_file_name( fun()->code()->source_file_name());
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

	debug_info_->set_variables_frame(make_outer_outer());
}

void VMachine::debug_hook(const inst_t* pc, int_t kind){
	XTAL_GLOBAL_INTERPRETER_LOCK{
		if(!debug_->is_enabled()){
			return;
		}

		{
			struct guard{
				const SmartPtr<Debug>* debug_;
				int_t count_;
				guard(const SmartPtr<Debug>& debug):debug_(&debug){ count_ = (*debug_)->disable_force(); }
				~guard(){ (*debug_)->enable_force(count_); }
			} g(debug_);

			AnyPtr e = ap(except_[0]);
			except_[0] = null;

			make_debug_info(pc, kind);

			switch(kind){
				XTAL_CASE(BREAKPOINT){
					if(const AnyPtr& hook = debug_->break_point_hook()){
						hook->call(debug_info_);
					}
				}

				XTAL_CASE(BREAKPOINT_RETURN){
					if(const AnyPtr& hook = debug_->return_hook()){
						hook->call(debug_info_);
					}
				}

				XTAL_CASE(BREAKPOINT_CALL){
					if(const AnyPtr& hook = debug_->call_hook()){
						hook->call(debug_info_);
					}
				}

				XTAL_CASE(BREAKPOINT_THROW){
					if(const AnyPtr& hook = debug_->throw_hook()){
						hook->call(debug_info_);
					}
				}

				XTAL_CASE(BREAKPOINT_ASSERT){
					if(const AnyPtr& hook = debug_->assert_hook()){
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
}

const inst_t* VMachine::catch_body(const inst_t* pc, const ExceptFrame& nef){
	XTAL_GLOBAL_INTERPRETER_LOCK{
		AnyPtr e = catch_except();

		ExceptFrame ef = except_frames_.empty() ? nef : except_frames_.top();

		// XtalÇÃä÷êîÇíEèoÇµÇƒÇ¢Ç≠
		while((size_t)ef.fun_frame_size<fun_frames_.size()){
			debug_hook(pc, BREAKPOINT_RETURN);
			pc = pop_ff();
			e = append_backtrace(pc, e);

			// CÇÃä÷êîÇ…Ç‘Ç¬Ç©Ç¡ÇΩ
			if(pc==&end_code_){
				ef.info = 0;
				break;
			}
		}

		while(scopes_.size()<ef.scope_size){
			scopes_.push(0);
		}

		while(scopes_.size()!=ef.scope_size){
			if(ScopeInfo* scope = scopes_.pop()){
				variables_.downsize(scope->variable_size);
			}
		}
		ff().outer(ap(ef.outer));

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
		return 0;
	}
	//return 0;
}

void VMachine::visit_members(Visitor& m){
	GCObserver::visit_members(m);
	m & debug_info_ & debug_ & except_[0] & except_[1] & except_[2];

	for(int_t i=0, size=stack_.size(); i<size; ++i){
		m & stack_[i];
	}

	for(int_t i=0, size=fun_frames_.size(); i<size; ++i){
		if(fun_frames_[i]){
			m & *fun_frames_[i];
		}
	}

	for(int_t i=0, size=variables_.size(); i<size; ++i){
		m & variables_[i];
	}

	for(int_t i=0, size=except_frames_.size(); i<size; ++i){
		m & except_frames_[i].outer;
	}
}

void VMachine::before_gc(){
	if(fun_frames_.empty()){
		//return;
	}

	inc_ref_count_force(debug_info_);
	inc_ref_count_force(debug_);

	inc_ref_count_force(except_[0]);
	inc_ref_count_force(except_[1]);
	inc_ref_count_force(except_[2]);


	for(int_t i=0, size=stack_.size(); i<size; ++i){
		inc_ref_count_force(stack_[i]);
	}

	for(int_t i=0, size=fun_frames_.size(); i<size; ++i){
		if(fun_frames_[i]){
			fun_frames_[i]->inc_ref();
		}
	}

	for(int_t i=0, size=except_frames_.size(); i<size; ++i){
		inc_ref_count_force(except_frames_[i].outer);
	}

	//*
	// égÇÌÇÍÇƒÇ¢Ç»Ç¢ïîï™ÇnullÇ≈ìhÇËÇ¬Ç‘Ç∑
	for(int_t i=stack_.size(), size=stack_.capacity(); i<size; ++i){
		set_null(stack_.reverse_at_unchecked(i));
	}

	for(int_t i=fun_frames_.size(), size=fun_frames_.capacity(); i<size; ++i){
		if(fun_frames_.reverse_at_unchecked(i)){
			fun_frames_.reverse_at_unchecked(i)->set_null();
		}
	}

	for(int_t j=variables_.size(), jsize=variables_.capacity(); j<jsize; ++j){
		set_null(variables_.reverse_at_unchecked(j));
	}

	for(int_t i=0, size=variables_.size(); i<size; ++i){
		inc_ref_count_force(variables_[i]);
	}
		
	for(int_t i=except_frames_.size(), size=except_frames_.capacity(); i<size; ++i){
		set_null(except_frames_.reverse_at_unchecked(i).outer);
	}
	//*/
}

void VMachine::after_gc(){
	if(fun_frames_.empty()){
		//return;
	}

	dec_ref_count_force(debug_info_);
	dec_ref_count_force(debug_);

	dec_ref_count_force(except_[0]);
	dec_ref_count_force(except_[1]);
	dec_ref_count_force(except_[2]);

	for(int_t i=0, size=stack_.size(); i<size; ++i){
		dec_ref_count_force(stack_[i]);
	}

	for(int_t i=0, size=fun_frames_.size(); i<size; ++i){
		if(fun_frames_[i]){
			fun_frames_[i]->dec_ref();
		}
	}

	for(int_t i=0, size=variables_.size(); i<size; ++i){
		dec_ref_count_force(variables_[i]);
	}

	for(int_t i=0, size=except_frames_.size(); i<size; ++i){
		dec_ref_count_force(except_frames_[i].outer);
	}
}


void VMachine::print_info(){
	std::printf("stack size %d\n", stack_.size());
	for(uint_t i=0; i<stack_.size(); ++i){
		std::printf("\tstack value %d = %s\n", i, ap(stack_[i])->to_s()->c_str());
	}

	std::printf("fun_frames size %d\n", fun_frames_.size());
	std::printf("except_frames size %d\n", except_frames_.size());
	std::printf("variables size %d\n", variables_.size());
	std::printf("scopes size %d\n", scopes_.size());
}

void VMachine::FunFrame::inc_ref(){
	inc_ref_count_force(fun_);
	inc_ref_count_force(outer_);
	
	inc_ref_count_force(self_);
	inc_ref_count_force(arguments_);
	inc_ref_count_force(hint_);

	inc_ref_count_force(target_);
	inc_ref_count_force(primary_key_);
	inc_ref_count_force(secondary_key_);
}

void VMachine::FunFrame::dec_ref(){
	dec_ref_count_force(fun_);
	dec_ref_count_force(outer_);
	
	dec_ref_count_force(self_);
	dec_ref_count_force(arguments_);
	dec_ref_count_force(hint_);

	dec_ref_count_force(target_);
	dec_ref_count_force(primary_key_);
	dec_ref_count_force(secondary_key_);
}
	
void visit_members(Visitor& m, const VMachine::FunFrame& v){
	m & v.fun_ & v.outer_ & v.arguments_ & v.hint_ & v.self_ & v.target_ & v.primary_key_ & v.secondary_key_;
}

}//namespace

