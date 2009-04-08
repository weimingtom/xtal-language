#include "xtal.h"
#include "xtal_macro.h"

namespace xtal{ namespace debug{

void HookInfo::visit_members(Visitor& m){
	Base::visit_members(m);
	m & file_name_ & fun_name_ & variables_frame_;
}

class DebugData{
public:
	DebugData(){
		enable_count_ = 0;
		hook_setting_bit_ = 0;
	}

	int_t enable_count_;
	uint_t hook_setting_bit_;
	AnyPtr break_point_hook_;
	AnyPtr call_hook_;
	AnyPtr return_hook_;
	AnyPtr throw_hook_;
	AnyPtr assert_hook_;
};


void enable(){
	DebugData& d = cpp_var<DebugData>();

	d.enable_count_++;

	set_break_point_hook(d.break_point_hook_);
	set_call_hook(d.call_hook_);
	set_return_hook(d.return_hook_);
	set_throw_hook(d.throw_hook_);
	set_assert_hook(d.assert_hook_);
}

void disable(){
	DebugData& d = cpp_var<DebugData>();
	d.enable_count_--;

	d.hook_setting_bit_ = 0;
}

bool is_enabled(){
	DebugData& d = cpp_var<DebugData>();
	return d.enable_count_>0;
}

void enable_force(int_t count){
	DebugData& d = cpp_var<DebugData>();
	d.enable_count_ = count;
}

int_t disable_force(){
	DebugData& d = cpp_var<DebugData>();
	int_t temp = d.enable_count_;
	d.enable_count_ = 0;
	return temp;
}

uint_t hook_setting_bit(){
	DebugData& d = cpp_var<DebugData>();
	return d.hook_setting_bit_;
}

void set_break_point_hook(const AnyPtr& hook){
	DebugData& d = cpp_var<DebugData>();
	d.break_point_hook_ = hook;
	if(hook){
		d.hook_setting_bit_ |= 1<<BREAKPOINT;
	}
	else{
		d.hook_setting_bit_ &= ~(1<<BREAKPOINT);
	}
}

void set_call_hook(const AnyPtr& hook){
	DebugData& d = cpp_var<DebugData>();
	d.call_hook_ = hook;
	if(hook){
		d.hook_setting_bit_ |= 1<<BREAKPOINT_CALL;
	}
	else{
		d.hook_setting_bit_ &= ~(1<<BREAKPOINT_CALL);
	}
}

void set_return_hook(const AnyPtr& hook){
	DebugData& d = cpp_var<DebugData>();
	d.return_hook_ = hook;
	if(hook){
		d.hook_setting_bit_ |= 1<<BREAKPOINT_RETURN;
	}
	else{
		d.hook_setting_bit_ &= ~(1<<BREAKPOINT_RETURN);
	}
}

void set_throw_hook(const AnyPtr& hook){
	DebugData& d = cpp_var<DebugData>();
	d.throw_hook_ = hook;
	if(hook){
		d.hook_setting_bit_ |= 1<<BREAKPOINT_THROW;
	}
	else{
		d.hook_setting_bit_ &= ~(1<<BREAKPOINT_THROW);
	}
}

void set_assert_hook(const AnyPtr& hook){
	DebugData& d = cpp_var<DebugData>();
	d.assert_hook_ = hook;
	if(hook){
		d.hook_setting_bit_ |= 1<<BREAKPOINT_ASSERT;
	}
	else{
		d.hook_setting_bit_ &= ~(1<<BREAKPOINT_ASSERT);
	}
}

const AnyPtr& break_point_hook(){
	DebugData& d = cpp_var<DebugData>();
	return d.break_point_hook_;
}

const AnyPtr& call_hook(){
	DebugData& d = cpp_var<DebugData>();
	return d.call_hook_;
}

const AnyPtr& return_hook(){
	DebugData& d = cpp_var<DebugData>();
	return d.return_hook_;
}

const AnyPtr& throw_hook(){
	DebugData& d = cpp_var<DebugData>();
	return d.throw_hook_;
}

const AnyPtr& assert_hook(){
	DebugData& d = cpp_var<DebugData>();
	return d.assert_hook_;
}

}

}
