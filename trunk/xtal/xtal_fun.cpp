
#include "xtal.h"

#include "xtal_fun.h"
#include "xtal_vmachine.h"
#include "xtal_frame.h"
#include "xtal_code.h"
#include "xtal_macro.h"

namespace xtal{

class VMachineMgr : public Base{

	ArrayPtr vms_;

	virtual void visit_members(Visitor& m){
		Base::visit_members(m);
		m & vms_;
	}

public:

	VMachineMgr(){
		vms_ = xnew<Array>();
	}

	VMachinePtr take_over(){
		if(vms_->empty()){
			vms_->push_back(xnew<VMachine>());
		}
		VMachinePtr vm = static_ptr_cast<VMachine>(vms_->back());
		vms_->pop_back();
		return vm;
	}

	void take_back(const VMachinePtr& vm){
		vm->reset();
		vms_->push_back(vm);
	}
};

namespace{
	SmartPtr<VMachineMgr> vm_mgr_;

	void uninitialize_fun(){
		vm_mgr_ = null;
	}
}

void initialize_fun(){
	register_uninitializer(&uninitialize_fun);

	vm_mgr_ = xnew<VMachineMgr>();

	{
		ClassPtr p = new_cpp_class<Fun>("Fun");
	}

	{
		ClassPtr p = new_cpp_class<Method>("Method");
		p->inherit(get_cpp_class<Fun>());
	}

	{
		ClassPtr p = new_cpp_class<Fiber>("Fiber");
		p->inherit(get_cpp_class<Fun>());
		p->inherit(Iterator());
		p->method("reset", &Fiber::reset);
		p->method("block_first", &Fiber::block_next);
		p->method("block_next", &Fiber::block_next);
		p->method("halt", &Fiber::halt);
		p->method("is_alive", &Fiber::is_alive);
	}

	{
		ClassPtr p = new_cpp_class<Lambda>("Lambda");
		p->inherit(get_cpp_class<Fun>());
	}

	{
		ClassPtr p = new_cpp_class<Arguments>("Arguments");
		p->method("size", &Arguments::length);
		p->method("length", &Arguments::length);
		p->method("op_at", &Arguments::op_at);
		p->method("ordered_arguments", &Arguments::ordered_arguments);
		p->method("named_arguments", &Arguments::named_arguments);
	}

	builtin()->def("Arguments", get_cpp_class<Arguments>());
	builtin()->def("Fun", get_cpp_class<Fun>());
	builtin()->def("Fiber", get_cpp_class<Fiber>());
}

void Arguments::visit_members(Visitor& m){
	Base::visit_members(m);
	m & ordered_ & named_;
}

InstanceVariableGetter::InstanceVariableGetter(int_t number, ClassCore* core)
	:number_(number), core_(core){
}

void InstanceVariableGetter::call(const VMachinePtr& vm){
	const AnyPtr& self = vm->get_arg_this();
	InstanceVariables* p;
	if(type(self)==TYPE_BASE){
		p = pvalue(self)->instance_variables();
	}else{
		p = &empty_instance_variables;
	}
	vm->return_result(p->variable(number_, core_));
}

InstanceVariableSetter::InstanceVariableSetter(int_t number, ClassCore* core)
	:number_(number), core_(core){
}

void InstanceVariableSetter::call(const VMachinePtr& vm){
	const AnyPtr& self = vm->get_arg_this();
	InstanceVariables* p;
	if(type(self)==TYPE_BASE){
		p = pvalue(self)->instance_variables();
	}else{
		p = &empty_instance_variables;
	}
	p->set_variable(number_, core_, vm->arg(0));
	vm->return_result();
}


Fun::Fun(const FramePtr& outer, const AnyPtr& athis, const CodePtr& code, FunCore* core)
	:outer_(outer), this_(athis), code_(code), core_(core){
}

void Fun::check_arg(const VMachinePtr& vm){
	int_t n = vm->ordered_arg_count();
	if(n<core_->min_param_count || (!(core_->flags&FunCore::FLAG_EXTENDABLE_PARAM) && n>core_->max_param_count)){
		if(core_->min_param_count==0 && core_->max_param_count==0){
			XTAL_THROW(builtin()->member("ArgumentError")(
				Xt("Xtal Runtime Error 1007")(
					Named("name", object_name()),
					Named("value", n)
				)
			), return);
		}else{
			if(core_->flags&FunCore::FLAG_EXTENDABLE_PARAM){
				XTAL_THROW(builtin()->member("ArgumentError")(
					Xt("Xtal Runtime Error 1005")(
						Named("name", object_name()),
						Named("min", core_->min_param_count),
						Named("value", n)
					)
				), return);
			}else{
				XTAL_THROW(builtin()->member("ArgumentError")(
					Xt("Xtal Runtime Error 1006")(
						Named("name", object_name()),
						Named("min", core_->min_param_count),
						Named("max", core_->max_param_count),
						Named("value", n)
					)
				), return);
			}
		}
	}
}

const inst_t* Fun::source(){ 
	return code_->data()+core_->pc; 
}

const InternedStringPtr& Fun::param_name_at(size_t i){ 
	return code_->identifier(i+core_->variable_identifier_offset); 
}

void Fun::call(const VMachinePtr& vm){
	if(vm->ordered_arg_count()!=core_->max_param_count)
		check_arg(vm);
	vm->set_arg_this(this_);
	vm->carry_over(this);
}

void Lambda::call(const VMachinePtr& vm){
	vm->set_arg_this(this_);
	vm->mv_carry_over(this);
}


void Method::call(const VMachinePtr& vm){
	if(vm->ordered_arg_count()!=core_->max_param_count)
		check_arg(vm);
	vm->carry_over(this);
}


Fiber::Fiber(const FramePtr& outer, const AnyPtr& th, const CodePtr& code, FunCore* core)
	:Fun(outer, th, code, core), vm_(null), resume_pc_(0), alive_(true){
}


void Fiber::halt(){
	if(resume_pc_!=0){
		vm_->exit_fiber();
		resume_pc_ = 0;
		vm_mgr_->take_back(vm_);
		vm_ = null;
		alive_ = false;
	}
}

void Fiber::call_helper(const VMachinePtr& vm, bool add_succ_or_fail_result){
	if(alive_){
		vm->set_arg_this(this_);
		if(resume_pc_==0){
			if(!vm_){ vm_ = vm_mgr_->take_over(); }
			resume_pc_ = vm_->start_fiber(this, vm.get(), add_succ_or_fail_result);
		}else{ 
			resume_pc_ = vm_->resume_fiber(this, resume_pc_, vm.get(), add_succ_or_fail_result);
		}
		if(resume_pc_==0){
			vm_mgr_->take_back(vm_);
			vm_ = null;
			alive_ = false;
		}
	}else{
		vm->return_result();
	}
}

AnyPtr Fiber::reset(){
	halt();
	alive_ = true;
	return FiberPtr::from_this(this);
}

}
