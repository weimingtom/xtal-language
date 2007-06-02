
#include "xtal.h"

#include "funimpl.h"
#include "vmachineimpl.h"

namespace xtal{

void InitFun(){
	TClass<Fun> fun("Fun");
	
	TClass<Method> met("Method");
	met.inherit(fun);
}

FunImpl::FunImpl(const Frame& outer, const Any& athis, const Code& code, FunCore* core)
	:outer_(outer), this_(athis), code_(code), core_(core){
	set_class(TClass<Fun>::get());
}

void FunImpl::check_arg(const VMachine& vm){
	int_t n = vm.ordered_arg_count();
	if(n<core_->min_param_count || (!core_->used_args_object && n>core_->max_param_count)){
		if(core_->min_param_count==0 && core_->max_param_count==0){
			XTAL_THROW(builtin().member("ArgumentError")(
				Xt("Xtal Runtime Error 1007")(
					Named("name", object_name()),
					Named("value", n)
				)
			));
		}else{
			if(core_->used_args_object){
				XTAL_THROW(builtin().member("ArgumentError")(
					Xt("Xtal Runtime Error 1005")(
						Named("name", object_name()),
						Named("min", core_->min_param_count),
						Named("value", n)
					)
				));
			}else{
				XTAL_THROW(builtin().member("ArgumentError")(
					Xt("Xtal Runtime Error 1006")(
						Named("name", object_name()),
						Named("min", core_->min_param_count),
						Named("max", core_->max_param_count),
						Named("value", n)
					)
				));
			}
		}
	}
}

void FunImpl::call(const VMachine& vm){
	check_arg(vm);
	vm.set_arg_this(this_);
	vm.impl()->carry_over(Fun(this));
}

void MethodImpl::call(const VMachine& vm){
	check_arg(vm);
	vm.impl()->carry_over(Fun(this));
}

class VMachineMgrImpl : public AnyImpl{

	Array vms_;

	virtual void visit_members(Visitor& m){
		AnyImpl::visit_members(m);
		m & vms_;
	}

public:

	VMachine take_over(){
		if(vms_.empty()){
			vms_.push_back(VMachine());
		}
		VMachine vm((const VMachine&)vms_.back());
		vms_.pop_back();
		return vm;
	}

	void take_back(const VMachine& vm){
		vm.reset();
		vms_.push_back(vm);
	}
};

static VMachineMgrImpl* vm_mgr(){
	static LLVar<Any> p;
	if(!p){ new(p) VMachineMgrImpl(); }
	return (VMachineMgrImpl*)p.impl();
}

FiberImpl::FiberImpl(const Frame& outer, const Any& th, const Code& code, FunCore* core)
	:FunImpl(outer, th, code, core), vm_(null), resume_pc_(0){
	set_class(TClass<Fiber>::get());
}

FiberImpl::~FiberImpl(){

}

void FiberImpl::halt(){
	if(resume_pc_!=0){
		vm_.impl()->exit_fiber();
		resume_pc_ = 0;
		vm_mgr()->take_back(vm_);
		vm_ = null;
	}
}

void FiberImpl::call_helper(const VMachine& vm, bool add_succ_or_fail_result){
	vm.set_arg_this(this_);
	if(resume_pc_==0){
		if(vm_==null){ vm_ = vm_mgr()->take_over(); }
		resume_pc_ = vm_.impl()->start_fiber(Fiber(this), vm.impl(), add_succ_or_fail_result);
	}else{ 
		resume_pc_ = vm_.impl()->resume_fiber(Fiber(this), resume_pc_, vm.impl(), add_succ_or_fail_result);
	}
	if(resume_pc_==0){
		vm_mgr()->take_back(vm_);
		vm_ = null;
	}
}

Fun::Fun(const Frame& outer, const Any& athis, const Code& code, FunCore* core){
	new(*this) FunImpl(outer, athis, code, core);
}

const Frame& Fun::outer() const{
	return impl()->outer();
}

const Code& Fun::code() const{
	return impl()->code();
}

int_t Fun::pc() const{
	return impl()->pc();
}

const u8* Fun::source() const{
	return impl()->source();
}

const ID& Fun::param_name_at(size_t i) const{
	return impl()->param_name_at(i);
}

int_t Fun::param_size() const{
	return impl()->param_size();
}

bool Fun::used_args_object() const{
	return impl()->used_args_object();
}

int_t Fun::defined_file_line_number() const{
	return impl()->defined_file_line_number();
}

void Fun::set_core(FunCore* fc) const{
	impl()->set_core(fc);
}

FunCore* Fun::core() const{
	return impl()->core();
}

Method::Method(const Frame& outer, const Code& code, FunCore* core)
	:Fun(null){
	new(*this) MethodImpl(outer, code, core);
}


void InitFiber(){
	TClass<Fiber> p("Fiber");
	p.inherit(TClass<Fun>::get());
	p.inherit(Iterator());
	p.method("restart", &Fiber::restart);
	p.method("iter_first", &Fiber::iter_next);
	p.method("iter_next", &Fiber::iter_next);
	p.method("halt", &Fiber::halt);
	p.method("finalize", &Fiber::halt);
}

Fiber::Fiber(const Frame& outer, const Any& th, const Code& code, FunCore* core)
	:Fun(null){ 
	new(*this) FiberImpl(outer, th, code, core);
}

Any Fiber::restart(){
	impl()->halt();
	return *this;
}

void Fiber::iter_next(const VMachine& vm){
	impl()->iter_next(vm);
}

void Fiber::halt(){
	impl()->halt();
}

}
