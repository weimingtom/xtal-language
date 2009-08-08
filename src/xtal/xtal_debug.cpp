#include "xtal.h"
#include "xtal_macro.h"

namespace xtal{ namespace debug{

void CallerInfo::visit_members(Visitor& m){
	Base::visit_members(m);
	m & file_name_ & fun_name_ & variables_frame_;
}

void HookInfo::visit_members(Visitor& m){
	Base::visit_members(m);
	m & /*vm_ & */file_name_ & fun_name_ & assertion_message_ & exception_ & variables_frame_;
}

CallerInfoPtr HookInfo::caller(uint_t n){
	return ptr_cast<VMachine>(ap(vm_))->caller(funframe_-n);
}

class DebugData{
public:
	DebugData(){
		enable_count_ = 0;
		hook_setting_bit_ = 0;
	}

	int_t enable_count_;
	uint_t hook_setting_bit_;

	AnyPtr hooks_[BREAKPOINT_MAX];
};

namespace{

void debugenable(const SmartPtr<DebugData>& d){
	bind_all();
	for(int_t i=0, size=BREAKPOINT_MAX; i<size; ++i){
		set_hook(i, d->hooks_[i]);
	}
	vmachine()->set_hook_setting_bit(d->hook_setting_bit_);
}

void bitchange(const SmartPtr<DebugData>& d, bool b, int_t type){
	if(b){
		d->hook_setting_bit_ |= 1<<type;
	}
	else{
		d->hook_setting_bit_ &= ~(1<<type);
	}
}

}

void enable(){
	const SmartPtr<DebugData>& d = cpp_var<DebugData>();
	d->enable_count_++;
	debugenable(d);
}

void disable(){
	const SmartPtr<DebugData>& d = cpp_var<DebugData>();
	d->enable_count_--;

	if(d->enable_count_<=0){
		d->hook_setting_bit_ = 0;
		vmachine()->set_hook_setting_bit(0);
	}
}

bool is_enabled(){
	const SmartPtr<DebugData>& d = cpp_var<DebugData>();
	return d->enable_count_>0;
}

void enable_force(int_t count){
	const SmartPtr<DebugData>& d = cpp_var<DebugData>();
	d->enable_count_ = count;
	debugenable(d);
}

int_t disable_force(){
	const SmartPtr<DebugData>& d = cpp_var<DebugData>();
	int_t temp = d->enable_count_;
	d->enable_count_ = 0;

	if(d->enable_count_<=0){
		d->hook_setting_bit_ = 0;
		vmachine()->set_hook_setting_bit(0);
	}

	return temp;
}

uint_t hook_setting_bit(){
	const SmartPtr<DebugData>& d = cpp_var<DebugData>();
	return d->hook_setting_bit_;
}

void set_hook(int_t hooktype, const AnyPtr& hook){
	const SmartPtr<DebugData>& d = cpp_var<DebugData>();
	d->hooks_[hooktype] = hook;
	bitchange(d, hook, hooktype);
}

const AnyPtr& hook(int_t hooktype){
	const SmartPtr<DebugData>& d = cpp_var<DebugData>();
	return d->hooks_[hooktype];
}

void set_line_hook(const AnyPtr& hook){
	set_hook(BREAKPOINT_LINE, hook);
}

void set_call_hook(const AnyPtr& hook){
	set_hook(BREAKPOINT_CALL, hook);
}

void set_return_hook(const AnyPtr& hook){
	set_hook(BREAKPOINT_RETURN, hook);
}

void set_throw_hook(const AnyPtr& hook){
	set_hook(BREAKPOINT_THROW, hook);
}

void set_assert_hook(const AnyPtr& hook){
	set_hook(BREAKPOINT_ASSERT, hook);
}

const AnyPtr& line_hook(){
	return hook(BREAKPOINT_LINE);
}

const AnyPtr& call_hook(){
	return hook(BREAKPOINT_CALL);
}

const AnyPtr& return_hook(){
	return hook(BREAKPOINT_RETURN);
}

const AnyPtr& throw_hook(){
	return hook(BREAKPOINT_THROW);
}

const AnyPtr& assert_hook(){
	return hook(BREAKPOINT_ASSERT);
}

}

}
