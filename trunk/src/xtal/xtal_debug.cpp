#include "xtal.h"
#include "xtal_macro.h"

namespace xtal{ namespace debug{

void Info::visit_members(Visitor& m){
	Base::visit_members(m);
	m & file_name_ & fun_name_ & local_variables_;
}

namespace{
	int_t enable_count_;
	AnyPtr break_point_hook_;
	AnyPtr call_hook_;
	AnyPtr return_hook_;
	AnyPtr throw_hook_;
}

void enable(){
	enable_count_++;
	if(enable_count_>1)
		enable_count_ = 1;
}

void disable(){
	enable_count_--;
}

bool is_enabled(){
	return enable_count_>0;
}

void set_break_point_hook(const AnyPtr& hook){
	break_point_hook_ = hook;
}

void set_call_hook(const AnyPtr& hook){
	call_hook_ = hook;
}

void set_return_hook(const AnyPtr& hook){
	return_hook_ = hook;
}

void set_throw_hook(const AnyPtr& hook){
	throw_hook_ = hook;
}

const AnyPtr& break_point_hook(){
	return break_point_hook_;
}

const AnyPtr& call_hook(){
	return call_hook_;
}

const AnyPtr& return_hook(){
	return return_hook_;
}

const AnyPtr& throw_hook(){
	return throw_hook_;
}

}

void uninitialize_debug(){
	using namespace debug;
	break_point_hook_ = null;
	call_hook_ = null;
	return_hook_ = null;
	throw_hook_ = null;
}

void initialize_debug(){
	using namespace debug;
	register_uninitializer(&uninitialize_debug);

	enable_count_ = 0;

	ClassPtr debug = xnew<Singleton>(Xid(debug));
	debug->fun(Xid(enable), &enable);
	debug->fun(Xid(disable), &disable);
	debug->fun(Xid(is_enabled), &is_enabled);
	debug->fun(Xid(break_point_hook), &break_point_hook);
	debug->fun(Xid(call_hook), &call_hook);
	debug->fun(Xid(return_hook), &return_hook);
	debug->fun(Xid(throw_hook), &throw_hook);
	debug->fun(Xid(set_break_point_hook), &set_break_point_hook);
	debug->fun(Xid(set_call_hook), &set_call_hook);
	debug->fun(Xid(set_return_hook), &set_return_hook);
	debug->fun(Xid(set_throw_hook), &set_throw_hook);

	{
		ClassPtr p = new_cpp_class<Info>(Xid(Info));
		p->method(Xid(clone), &Info::clone);
		p->method(Xid(kind), &Info::kind);
		p->method(Xid(line), &Info::line);
		p->method(Xid(fun_name), &Info::fun_name);
		p->method(Xid(file_name), &Info::file_name);
		p->method(Xid(set_kind), &Info::set_kind);
		p->method(Xid(set_line), &Info::line);
		p->method(Xid(set_fun_name), &Info::set_fun_name);
		p->method(Xid(set_file_name), &Info::set_file_name);

		p->def(Xid(BREAKPOINT), BREAKPOINT);
		p->def(Xid(CALL), BREAKPOINT_CALL);
		p->def(Xid(RETURN), BREAKPOINT_RETURN);
		p->def(Xid(RETURN_THROW), BREAKPOINT_THROW);

		debug->def(Xid(Info), p);
	}

	builtin()->def(Xid(debug), debug);
}

}
