
#pragma once

namespace xtal{

class Exception : public Base{
public:

	Exception(const AnyPtr& message = "");

	void initialize(const AnyPtr& message = "");

	void append_backtrace(const AnyPtr& file, int_t line, const AnyPtr& function_name="");

	StringPtr to_s();

	StringPtr message(){
		return message_;
	}

	AnyPtr backtrace(){
		return backtrace_->each();
	}

private:

	virtual void visit_members(Visitor& m){
		Base::visit_members(m);
		m & message_ & backtrace_;
	}

	StringPtr message_;
	ArrayPtr backtrace_;
};

AnyPtr cast_error(const AnyPtr& from, const AnyPtr& to);
AnyPtr argument_error(const AnyPtr& from, const AnyPtr& to, int_t param_num, const AnyPtr& param_name);
AnyPtr unsupported_error(const AnyPtr& name, const AnyPtr& member);

typedef void (*except_handler_t)(const AnyPtr& except, const char* file, int line);
except_handler_t except_handler();
void set_except_handler(except_handler_t handler);

}
