#include "xtal.h"
#include "xtal_macro.h"

namespace xtal{

void DebugInfo::visit_members(Visitor& m){
	Base::visit_members(m);
	m & file_name_ & fun_name_ & local_variables_;
}

void Debug::enable(){
	enable_count_++;
	if(enable_count_>1){
		enable_count_ = 1;
	}
}

void Debug::disable(){
	enable_count_--;
}

bool Debug::is_enabled(){
	return enable_count_>0;
}

void Debug::set_break_point_hook(const AnyPtr& hook){
	break_point_hook_ = hook;
}

void Debug::set_call_hook(const AnyPtr& hook){
	call_hook_ = hook;
}

void Debug::set_return_hook(const AnyPtr& hook){
	return_hook_ = hook;
}

void Debug::set_throw_hook(const AnyPtr& hook){
	throw_hook_ = hook;
}

void Debug::set_assert_hook(const AnyPtr& hook){
	assert_hook_ = hook;
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

const SmartPtr<Debug>& debug(){
	return ptr_cast<Debug>(builtin()->member(Xid(debug)));
}

void enable_debug(){
	debug()->enable();
}

void disable_debug(){
	debug()->disable();
}

}
