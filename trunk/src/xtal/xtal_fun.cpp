#include "xtal.h"
#include "xtal_macro.h"

namespace xtal{

Arguments::Arguments(const AnyPtr& ordered, const AnyPtr& named){
	if(ordered){ ordered_ = ptr_cast<Array>(ordered); }
	else{ ordered_ = xnew<Array>(); }
	if(named){ named_ = ptr_cast<Map>(named); }
	else{ named_ = xnew<Map>(); }
}

void Arguments::visit_members(Visitor& m){
	Base::visit_members(m);
	m & ordered_ & named_;
}

InstanceVariableGetter::InstanceVariableGetter(int_t number, ClassInfo* info)
	:number_(number), info_(info){
}

void InstanceVariableGetter::rawcall(const VMachinePtr& vm){
	const AnyPtr& self = vm->arg_this();
	InstanceVariables* p;
	if(type(self)==TYPE_BASE){
		p = pvalue(self)->instance_variables();
	}
	else{
		p = &empty_instance_variables;
	}
	vm->return_result(p->variable(number_, info_));
}

InstanceVariableSetter::InstanceVariableSetter(int_t number, ClassInfo* info)
	:number_(number), info_(info){
}

void InstanceVariableSetter::rawcall(const VMachinePtr& vm){
	const AnyPtr& self = vm->arg_this();
	InstanceVariables* p;
	if(type(self)==TYPE_BASE){
		p = pvalue(self)->instance_variables();
	}
	else{
		p = &empty_instance_variables;
	}
	p->set_variable(number_, info_, vm->arg(0));
	vm->return_result();
}


Method::Method(const FramePtr& outer, const CodePtr& code, FunInfo* core)
	:outer_(outer), code_(code), info_(core){
}

bool Method::check_arg(const VMachinePtr& vm){
	int_t n = vm->ordered_arg_count();
	if(n<info_->min_param_count || (!(info_->flags&FunInfo::FLAG_EXTENDABLE_PARAM) && n>info_->max_param_count)){
		if(info_->min_param_count==0 && info_->max_param_count==0){
			XTAL_SET_EXCEPT(builtin()->member(Xid(ArgumentError))->call(
				Xt("Xtal Runtime Error 1007")->call(
					Named(Xid(object), object_name()),
					Named(Xid(value), n)
				)
			));
			return false;
		}
		else{
			if(info_->flags&FunInfo::FLAG_EXTENDABLE_PARAM){
				XTAL_SET_EXCEPT(builtin()->member(Xid(ArgumentError))->call(
					Xt("Xtal Runtime Error 1005")->call(
						Named(Xid(object), object_name()),
						Named(Xid(min), info_->min_param_count),
						Named(Xid(value), n)
					)
				));
				return false;
			}
			else{
				XTAL_SET_EXCEPT(builtin()->member(Xid(ArgumentError))->call(
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

Fiber::Fiber(const FramePtr& outer, const AnyPtr& th, const CodePtr& code, FunInfo* core)
	:Fun(outer, th, code, core), vm_(null), resume_pc_(0), alive_(true){
	set_finalizer_flag();
}

void Fiber::finalize(){
	halt();
}

void Fiber::halt(){
	if(resume_pc_!=0){
		vm_->exit_fiber();
		resume_pc_ = 0;
		core()->vm_take_back(vm_);
		vm_ = null;
		alive_ = false;
	}
}

void Fiber::call_helper(const VMachinePtr& vm, bool add_succ_or_fail_result){
	if(alive_){
		vm->set_arg_this(this_);
		if(resume_pc_==0){
			if(!vm_){ vm_ = core()->vm_take_over(); }
			resume_pc_ = vm_->start_fiber(this, vm.get(), add_succ_or_fail_result);
		}
		else{ 
			resume_pc_ = vm_->resume_fiber(this, resume_pc_, vm.get(), add_succ_or_fail_result);
		}
		if(resume_pc_==0){
			core()->vm_take_back(vm_);
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
	return from_this(this);
}

}
