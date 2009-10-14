#include "xtal.h"
#include "xtal_macro.h"

namespace xtal{

Arguments::Arguments(const AnyPtr& ordered, const AnyPtr& named){
	if(ordered){ ordered_ = ptr_cast<Array>(ordered); }
	else{ ordered_ = null; }
	if(named){ named_ = ptr_cast<Map>(named); }
	else{ named_ = null; }
}
void Arguments::add_ordered(const AnyPtr& v){
	if(!ordered_){ ordered_ = xnew<Array>(); }
	ordered_->push_back(v);
}

void Arguments::add_named(const AnyPtr& k, const AnyPtr& v){
	if(!named_){ named_ = xnew<Map>(); }
	named_->set_at(k, v);
}
	
void Arguments::add_named(const VMachinePtr& vm){
	if(!named_){ named_ = xnew<Map>(); }
	named_->push_all(vm);
}

uint_t Arguments::ordered_size(){
	if(ordered_){
		return ordered_->size();
	}
	return 0;
}

uint_t Arguments::named_size(){
	if(named_){
		return named_->size();
	}
	return 0;
}

const AnyPtr& Arguments::op_at_int(uint_t index){
	if(ordered_ && index<ordered_->size()){
		return ordered_->at(index);
	}
	return undefined;
}

const AnyPtr& Arguments::op_at_string(const StringPtr& key){
	if(named_){
		return named_->at(key); 
	}
	return undefined;
}

uint_t Arguments::length(){
	if(ordered_){
		return ordered_->length();
	}
	return 0;
}

AnyPtr Arguments::ordered_arguments(){
	if(ordered_){
		return ordered_->each();
	}
	return null;
}

AnyPtr Arguments::named_arguments(){
	if(named_){
		return named_->each();
	}
	return null;
}

void Arguments::visit_members(Visitor& m){
	Base::visit_members(m);
	m & ordered_ & named_;
}

InstanceVariableGetter::InstanceVariableGetter(int_t number, ClassInfo* info)
	:number_(number), info_(info){
	set_pvalue(*this, TYPE, this);
}

void InstanceVariableGetter::rawcall(const VMachinePtr& vm){
	const AnyPtr& self = vm->arg_this();
	vm->return_result(self->instance_variables()->variable(number_, info_));
}

InstanceVariableSetter::InstanceVariableSetter(int_t number, ClassInfo* info)
	:number_(number), info_(info){
	set_pvalue(*this, TYPE, this);
}

void InstanceVariableSetter::rawcall(const VMachinePtr& vm){
	const AnyPtr& self = vm->arg_this();
	self->instance_variables()->set_variable(number_, info_, vm->arg(0));
	vm->return_result();
}


Method::Method(const FramePtr& outer, const CodePtr& code, FunInfo* info)
	:outer_(outer), code_(code), info_(info){
}

bool Method::check_arg(const VMachinePtr& vm){
	int_t n = vm->ordered_arg_count();
	if(n<info_->min_param_count || (!(info_->flags&FunInfo::FLAG_EXTENDABLE_PARAM) && n>info_->max_param_count)){
		if(info_->min_param_count==0 && info_->max_param_count==0){
			XTAL_SET_EXCEPT(cpp_class<ArgumentError>()->call(
				Xt("Xtal Runtime Error 1007")->call(
					Named(Xid(object), object_name()),
					Named(Xid(value), n)
				)
			));
			return false;
		}
		else{
			if(info_->flags&FunInfo::FLAG_EXTENDABLE_PARAM){
				XTAL_SET_EXCEPT(cpp_class<ArgumentError>()->call(
					Xt("Xtal Runtime Error 1005")->call(
						Named(Xid(object), object_name()),
						Named(Xid(min), info_->min_param_count),
						Named(Xid(value), n)
					)
				));
				return false;
			}
			else{
				XTAL_SET_EXCEPT(cpp_class<ArgumentError>()->call(
					Xt("Xtal Runtime Error 1006")->call(
						Named(Xid(object), object_name()),
						Named(Xid(min), info_->min_param_count),
						Named(Xid(max), info_->max_param_count),
						Named(Xid(value), n)
					)
				));
				return false;
			}
		}
	}
	return true;
}

void Method::rawcall(const VMachinePtr& vm){
	if(vm->ordered_arg_count()!=info_->max_param_count){
		if(!check_arg(vm)){
			return;
		}
	}

	vm->carry_over(this);
}

void Fun::rawcall(const VMachinePtr& vm){
	if(vm->ordered_arg_count()!=info_->max_param_count){
		if(!check_arg(vm)){
			return;
		}
	}

	vm->set_arg_this(this_);
	vm->carry_over(this);
}

void Lambda::rawcall(const VMachinePtr& vm){
	vm->set_arg_this(this_);
	vm->mv_carry_over(this);
}

Fiber::Fiber(const FramePtr& outer, const AnyPtr& th, const CodePtr& code, FunInfo* info)
	:Fun(outer, th, code, info), vm_(null), resume_pc_(0), alive_(true){
	set_finalizer_flag();
}

void Fiber::finalize(){
	halt();
}

void Fiber::halt(){
	if(resume_pc_!=0){
		vm_->exit_fiber();
		resume_pc_ = 0;
		vmachine_take_back(vm_);
		vm_ = null;
		alive_ = false;
	}
}

void Fiber::call_helper(const VMachinePtr& vm, bool add_succ_or_fail_result){
	if(alive_){
		vm->set_arg_this(this_);
		if(resume_pc_==0){
			if(!vm_){ vm_ = vmachine_take_over(); }
			resume_pc_ = vm_->start_fiber(this, vm.get(), add_succ_or_fail_result);
		}
		else{ 
			resume_pc_ = vm_->resume_fiber(this, resume_pc_, vm.get(), add_succ_or_fail_result);
		}

		if(resume_pc_==0){
			vmachine_take_back(vm_);
			vm_ = null;
			alive_ = false;
		}
	}
	else{
		vm->return_result();
	}
}

AnyPtr Fiber::reset(){
	halt();
	alive_ = true;
	return to_smartptr(this);
}

void BindedThis::rawcall(const VMachinePtr& vm){
	vm->set_arg_this(this_);
	fun_->rawcall(vm);
}

}
