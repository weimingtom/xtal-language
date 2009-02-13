
#pragma once

namespace xtal{

class Exception : public Base{
public:

	Exception(const AnyPtr& message = empty_string);

	void initialize(const AnyPtr& message = empty_string);

	void append_backtrace(const AnyPtr& file, int_t line, const AnyPtr& function_name = empty_string);

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
AnyPtr argument_error(const AnyPtr& object, int_t no, const ClassPtr& required, const ClassPtr& type);
AnyPtr unsupported_error(const AnyPtr& target, const IDPtr& primary_key, const AnyPtr& secondary_key);

typedef void (*except_handler_t)(const AnyPtr& except, const char* file, int line);
except_handler_t except_handler();
void set_except_handler(except_handler_t handler);

}
