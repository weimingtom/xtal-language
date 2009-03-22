#include "xtal.h"
#include "xtal_macro.h"

namespace xtal{

void DebugInfo::visit_members(Visitor& m){
	Base::visit_members(m);
	m & file_name_ & fun_name_ & local_variables_frame_;
}

void Debug::enable(){
	enable_count_++;

	set_break_point_hook(break_point_hook_);
	set_call_hook(call_hook_);
	set_return_hook(return_hook_);
	set_throw_hook(throw_hook_);
	set_assert_hook(assert_hook_);
}

void Debug::disable(){
	enable_count_--;

	hook_setting_bit_ = 0;
}

bool Debug::is_enabled(){
	return enable_count_>0;
}

void Debug::set_break_point_hook(const AnyPtr& hook){
	break_point_hook_ = hook;
	if(hook){
		hook_setting_bit_ |= 1<<BREAKPOINT;
	}
	else{
		hook_setting_bit_ &= ~(1<<BREAKPOINT);
	}
}

void Debug::set_call_hook(const AnyPtr& hook){
	call_hook_ = hook;
	if(hook){
		hook_setting_bit_ |= 1<<BREAKPOINT_CALL;
	}
	else{
		hook_setting_bit_ &= ~(1<<BREAKPOINT_CALL);
	}
}

void Debug::set_return_hook(const AnyPtr& hook){
	return_hook_ = hook;
	if(hook){
		hook_setting_bit_ |= 1<<BREAKPOINT_RETURN;
	}
	else{
		hook_setting_bit_ &= ~(1<<BREAKPOINT_RETURN);
	}
}

void Debug::set_throw_hook(const AnyPtr& hook){
	throw_hook_ = hook;
	if(hook){
		hook_setting_bit_ |= 1<<BREAKPOINT_THROW;
	}
	else{
		hook_setting_bit_ &= ~(1<<BREAKPOINT_THROW);
	}
}

void Debug::set_assert_hook(const AnyPtr& hook){
	assert_hook_ = hook;
	if(hook){
		hook_setting_bit_ |= 1<<BREAKPOINT_ASSERT;
	}
	else{
		hook_setting_bit_ &= ~(1<<BREAKPOINT_ASSERT);
	}
}

const AnyPtr& Debug::break_point_hook(){
	return break_point_hook_;
}

const AnyPtr& Debug::call_hook(){
	return call_hook_;
}

const AnyPtr& Debug::return_hook(){
	return return_hook_;
}

const AnyPtr& Debug::throw_hook(){
	return throw_hook_;
}

const AnyPtr& Debug::assert_hook(){
	return assert_hook_;
}

void enable_debug(){
	debug()->enable();
}

void disable_debug(){
	debug()->disable();
}

}
