#include "xtal.h"

namespace xtal{ namespace debug{

void Info::visit_members(Visitor& m){
	Base::visit_members(m);
	m & file_name_ & fun_name_ & local_variables_;
}

namespace{
	int_t enable_count_;
	AnyPtr line_hook_;
	AnyPtr call_hook_;
	AnyPtr return_hook_;
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

void set_line_hook(const AnyPtr& hook){
	line_hook_ = hook;
}

void set_call_hook(const AnyPtr& hook){
	call_hook_ = hook;
}

void set_return_hook(const AnyPtr& hook){
	return_hook_ = hook;
}

const AnyPtr& line_hook(){
	return line_hook_;
}

const AnyPtr& call_hook(){
	return call_hook_;
}

const AnyPtr& return_hook(){
	return return_hook_;
}

}

void uninitialize_debug(){
	using namespace debug;
	line_hook_ = null;
	call_hook_ = null;
	return_hook_ = null;
}

void initialize_debug(){
	using namespace debug;
	register_uninitializer(&uninitialize_debug);

	enable_count_ = 0;

	ClassPtr debug = xnew<Singleton>("debug");
	debug->fun("enable", &enable);
	debug->fun("disable", &disable);
	debug->fun("is_enabled", &is_enabled);
	debug->fun("line_hook", &line_hook);
	debug->fun("call_hook", &call_hook);
	debug->fun("return_hook", &return_hook);
	debug->fun("set_line_hook", &set_line_hook);
	debug->fun("set_call_hook", &set_call_hook);
	debug->fun("set_return_hook", &set_return_hook);

	{
		ClassPtr p = new_cpp_class<Info>("Info");
		p->method("clone", &Info::clone);
		p->method("kind", &Info::kind);
		p->method("line", &Info::line);
		p->method("fun_name", &Info::fun_name);
		p->method("file_name", &Info::file_name);
		p->method("set_kind", &Info::set_kind);
		p->method("set_line", &Info::line);
		p->method("set_fun_name", &Info::set_fun_name);
		p->method("set_file_name", &Info::set_file_name);

		p->def("LINE", BREAKPOINT_LINE);
		p->def("CALL", BREAKPOINT_CALL);
		p->def("RETURN", BREAKPOINT_RETURN);

		debug->def("Info", p);
	}

	builtin()->def("debug", debug);
}

}
