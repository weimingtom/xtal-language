#include "xtal.h"
#include "xtal_macro.h"

namespace xtal{

VMachine::VMachine(){	
	myself_ = this;

	id_ = core()->id_op_list();

	stack_.reserve(32);

	end_code_ = InstExit::NUMBER;
	throw_code_ = InstThrow::NUMBER;
	throw_unsupported_error_code_ = InstThrowUnsupportedError::NUMBER;
	throw_undefined_code_ = InstThrowUndefined::NUMBER;
	check_unsupported_code_ = InstCheckUnsupported::NUMBER;
	cleanup_call_code_ = InstCleanupCall::NUMBER;
	resume_pc_ = 0;
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
	for(int_t i=f.ordered_arg_count; i<num; ++i){
		stack_.insert(offset, arg_default(params[i]));
		f.ordered_arg_count++;
	}
	stack_.downsize(offset);
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

void VMachine::return_result_mv(const MultiValuePtr& values){
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

void VMachine::present_for_vm(Fiber* fun, VMachine* vm, bool add_succ_or_fail_result){
	// Œ‹‰Ê‚ðvm‚É“n‚·
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
	push_ff(&end_code_, vm->need_result_count(), vm->ordered_arg_count(), vm->named_arg_count(), vm->arg_this());
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

ArgumentsPtr VMachine::make_args(Method* fun){
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
		if(!ep->is(builtin()->member(Xid(Exception)))){
			ep = RuntimeError()->call(ep);
		}
		if(fun() &&  fun()->code()){
			if((pc !=  fun()->code()->data()+ fun()->code()->size()-1)){
				ep->send(Xid(append_backtrace),
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
	if(!debug_info_){ debug_info_ = xnew<DebugInfo>(); }
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
		debug_info_->set_message(pop()->to_s());
	}
	else{
		debug_info_->set_message(empty_string);
	}

	debug_info_->set_local_variables(ff().outer());
}

void VMachine::debug_hook(const inst_t* pc, int_t kind){
	XTAL_GLOBAL_INTERPRETER_LOCK{
		{
			struct guard{
				const SmartPtr<Debug>& debug_;
				guard(const SmartPtr<Debug>& debug):debug_(debug){ debug_->disable(); }
				~guard(){ debug_->enable(); }
			} g(debug_);
		
			switch(kind){
				XTAL_CASE(BREAKPOINT){
					if(const AnyPtr& hook = debug_->break_point_hook()){
						make_debug_info(pc, kind);
						hook->call(debug_info_);
					}
				}

				XTAL_CASE(BREAKPOINT_RETURN){
					if(const AnyPtr& hook = debug_->return_hook()){
						make_debug_info(pc, kind);
						hook->call(debug_info_);
					}
				}

				XTAL_CASE(BREAKPOINT_CALL){
					if(const AnyPtr& hook = debug_->call_hook()){
						make_debug_info(pc, kind);
						hook->call(debug_info_);
					}
				}

				XTAL_CASE(BREAKPOINT_THROW){
					if(const AnyPtr& hook = debug_->throw_hook()){
						make_debug_info(pc, kind);
						hook->call(debug_info_);
					}
				}

				XTAL_CASE(BREAKPOINT_ASSERT){
					if(const AnyPtr& hook = debug_->assert_hook()){
						make_debug_info(pc, kind);
						hook->call(debug_info_);
					}
					else{
						set_except(builtin()->member(Xid(AssertionFailed))->call(pop()->to_s()));
					}
				}
			}
		}
	}
}

const inst_t* VMachine::catch_body(const inst_t* pc, int_t stack_size, int_t fun_frames_size){
	XTAL_GLOBAL_INTERPRETER_LOCK{

		AnyPtr e = except();

		// try .. catch .. finally•¶‚ÅˆÍ‚í‚ê‚Ä‚¢‚È‚¢
		if(except_frames_.empty()){
			while((size_t)fun_frames_size<fun_frames_.size()){
				debug_hook(pc, BREAKPOINT_RETURN);
				pc = pop_ff();
				e = append_backtrace(pc, e);
			}
			stack_.downsize_n(stack_size);
			set_except_0(e);
			return 0;
		}

		ExceptFrame& ef = except_frames_.top();
		while((size_t)ef.fun_frame_count<fun_frames_.size()){
			debug_hook(pc, BREAKPOINT_RETURN);
			pc = pop_ff();
			e = append_backtrace(pc, e);

			if(pc==&end_code_){
				stack_.downsize_n(stack_size);
				set_except_0(e);
				return 0;
			}
		}

		ff().variables_.downsize_n(ef.variable_size);
		if(ap(ef.outer)){
			while(!raweq(ef.outer, ff().outer())){
				ff().outer(ff().outer()->outer());
			}
		}

		stack_.resize(ef.stack_count);
		if(ef.core->catch_pc && e){
			pc = ef.core->catch_pc +  fun()->code()->data();
			push(AnyPtr(ef.core->end_pc));
			push(e);
			except_[1] = null;
			except_[0] = null;
		}
		else{
			pc = ef.core->finally_pc +  fun()->code()->data();
			push(e);
			push(AnyPtr(fun()->code()->size()-1));
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
	m & debug_info_ & debug_ & except_[0] & except_[1] & except_[2];

	for(int_t i=0, size=stack_.size(); i<size; ++i){
		m & stack_[i];
	}

	for(int_t i=0, size=fun_frames_.size(); i<size; ++i){
		m & fun_frames_[i];
	}

	for(int_t i=0, size=except_frames_.size(); i<size; ++i){
		m & except_frames_[i].outer;
	}
}

void VMachine::before_gc(){
	inc_ref_count_force(debug_info_);
	inc_ref_count_force(debug_);

	inc_ref_count_force(except_[0]);
	inc_ref_count_force(except_[1]);
	inc_ref_count_force(except_[2]);

	for(int_t i=0, size=stack_.size(); i<size; ++i){
		inc_ref_count_force(stack_[i]);
	}

	for(int_t i=0, size=fun_frames_.size(); i<size; ++i){
		fun_frames_[i].inc_ref();
	}

	for(int_t i=0, size=except_frames_.size(); i<size; ++i){
		inc_ref_count_force(except_frames_[i].outer);
	}

	// Žg‚í‚ê‚Ä‚¢‚È‚¢•”•ª‚ðnull‚Å“h‚è‚Â‚Ô‚·
	for(int_t i=stack_.size(), size=stack_.capacity(); i<size; ++i){
		stack_.reverse_at_unchecked(i).set_null();
	}

	for(int_t i=fun_frames_.size(), size=fun_frames_.capacity(); i<size; ++i){
		fun_frames_.reverse_at_unchecked(i).set_null();
		for(int_t j=0, jsize=fun_frames_.reverse_at_unchecked(i).variables_.capacity(); j<jsize; ++j){
			fun_frames_.reverse_at_unchecked(i).variables_.reverse_at_unchecked(j).set_null();
		}
	}

	for(int_t i=except_frames_.size(), size=except_frames_.capacity(); i<size; ++i){
		except_frames_.reverse_at_unchecked(i).outer.set_null();
	}

}

void VMachine::after_gc(){
	dec_ref_count_force(debug_info_);
	dec_ref_count_force(debug_);

	dec_ref_count_force(except_[0]);
	dec_ref_count_force(except_[1]);
	dec_ref_count_force(except_[2]);

	for(int_t i=0, size=stack_.size(); i<size; ++i){
		dec_ref_count_force(stack_[i]);
	}

	for(int_t i=0, size=fun_frames_.size(); i<size; ++i){
		fun_frames_[i].dec_ref();
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

	inc_ref_count_force(target_);
	inc_ref_count_force(primary_key_);
	inc_ref_count_force(secondary_key_);
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

	dec_ref_count_force(target_);
	dec_ref_count_force(primary_key_);
	dec_ref_count_force(secondary_key_);
}
	
void visit_members(Visitor& m, const VMachine::FunFrame& v){
	m & v.fun_ & v.outer_ & v.arguments_ & v.hint_ & v.self_ & v.target_ & v.primary_key_ & v.secondary_key_;
	for(int_t i=0, size=v.variables_.size(); i<size; ++i){
		m & v.variable(i);
	}
}

}//namespace

