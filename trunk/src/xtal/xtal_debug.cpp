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

const SmartPtr<Debug>& debug(){
	return ptr_cast<Debug>(builtin()->member(Xid(debug)));
}


void initialize_debug(){
	ClassPtr debug = new_cpp_singleton<Debug>();
	debug->method(Xid(enable), &Debug::enable);
	debug->method(Xid(disable), &Debug::disable);
	debug->method(Xid(is_enabled), &Debug::is_enabled);
	debug->method(Xid(break_point_hook), &Debug::break_point_hook);
	debug->method(Xid(call_hook), &Debug::call_hook);
	debug->method(Xid(return_hook), &Debug::return_hook);
	debug->method(Xid(throw_hook), &Debug::throw_hook);
	debug->method(Xid(set_break_point_hook), &Debug::set_break_point_hook);
	debug->method(Xid(set_call_hook), &Debug::set_call_hook);
	debug->method(Xid(set_return_hook), &Debug::set_return_hook);
	debug->method(Xid(set_throw_hook), &Debug::set_throw_hook);

	{
		ClassPtr p = new_cpp_class<DebugInfo>(Xid(DebugInfo));
		p->method(Xid(clone), &DebugInfo::clone);
		p->method(Xid(kind), &DebugInfo::kind);
		p->method(Xid(line), &DebugInfo::line);
		p->method(Xid(fun_name), &DebugInfo::fun_name);
		p->method(Xid(file_name), &DebugInfo::file_name);
		p->method(Xid(set_kind), &DebugInfo::set_kind);
		p->method(Xid(set_line), &DebugInfo::line);
		p->method(Xid(set_fun_name), &DebugInfo::set_fun_name);
		p->method(Xid(set_file_name), &DebugInfo::set_file_name);

		p->def(Xid(BREAKPOINT), BREAKPOINT);
		p->def(Xid(CALL), BREAKPOINT_CALL);
		p->def(Xid(RETURN), BREAKPOINT_RETURN);
		p->def(Xid(RETURN_THROW), BREAKPOINT_THROW);

		debug->def(Xid(DebugInfo), p);
	}

	builtin()->def(Xid(debug), debug);
}

}
