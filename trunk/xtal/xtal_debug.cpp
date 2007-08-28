#include "xtal.h"

namespace xtal{

namespace debug{

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

AnyPtr line_hook(){
	return line_hook_;
}

AnyPtr call_hook(){
	return call_hook_;
}

AnyPtr return_hook(){
	return return_hook_;
}

}

void InitDebug(){
	using namespace debug;

	add_long_life_var(&line_hook_);
	add_long_life_var(&call_hook_);
	add_long_life_var(&return_hook_);
	enable_count_ = 0;

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
	}
}

}
