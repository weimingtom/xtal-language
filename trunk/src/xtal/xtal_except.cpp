#include "xtal.h"
#include "xtal_macro.h"

namespace xtal{

void initialize_except(){
	{
		ClassPtr p = new_cpp_class<Exception>();
		p->def("new", ctor<Exception>()->param(Named("message", "")));
		p->method("initialize", &Exception::initialize)->param(Named("message", ""));
		p->method("to_s", &Exception::to_s);
		p->method("message", &Exception::message);
		p->method("backtrace", &Exception::backtrace);
		p->method("append_backtrace", &Exception::append_backtrace);

		builtin()->def("Exception", p);
	}
}

Exception::Exception(const AnyPtr& message){
	initialize(message);
}

void Exception::initialize(const AnyPtr& message){
	message_ = message->to_s();
	backtrace_ = xnew<Array>();
}

void Exception::append_backtrace(const AnyPtr& file, int_t line, const AnyPtr& function_name){
	backtrace_->push_back(Xf("\t%(file)s:%(line)d: in %(function_name)s")(
		Named("file", file), Named("line", line), Named("function_name", function_name)));
}

StringPtr Exception::to_s(){
	MemoryStreamPtr mm = xnew<MemoryStream>();
	mm->put_s(get_class()->object_name());
	mm->put_s(": ");
	mm->put_s(message_->to_s());
	mm->put_s("\n");
	mm->put_s(backtrace_->join("\n"));
	return mm->to_s();
}

AnyPtr cast_error(const AnyPtr& from, const AnyPtr& to){
	return builtin()->member("CastError")(Xt("Xtal Runtime Error 1004")(
		Named("type", from->get_class()->object_name()), Named("required", to)
	));
}

AnyPtr argument_error(const AnyPtr& from, const AnyPtr& to, int_t param_num, const AnyPtr& param_name){
	if(param_name){
		return builtin()->member("ArgumentError")(Xt("Xtal Runtime Error 1001")(
			Named("n", param_num+1), 
			Named("param_name", param_name), 
			Named("type", from->get_class()->object_name()), 
			Named("required", to)
		));
	}else{
		return builtin()->member("ArgumentError")(Xt("Xtal Runtime Error 1001")(
			Named("n", param_num+1), 
			Named("param_name", xnew<String>("")), 
			Named("type", from->get_class()->object_name()), 
			Named("required", to)
		));	}
}

AnyPtr unsupported_error(const AnyPtr& name, const AnyPtr& member, const AnyPtr& ns){
	if(raweq(ns, null)){
		return builtin()->member("UnsupportedError")(Xt("Xtal Runtime Error 1015")(
			Named("object", name), Named("name", (member ? member : AnyPtr("()")))
		));
	}else{
		return builtin()->member("UnsupportedError")(Xt("Xtal Runtime Error 1021")(
			Named("object", name), Named("name", (member ? member : AnyPtr("()"))), Named("ns", ns)
		));
	}
}
namespace{
	void default_except_handler(const AnyPtr& except, const char* file, int line){
#ifdef XTAL_NO_EXCEPT
		printf("%s(%d):%s\n", file, line, except->to_s()->c_str());
		exit(1);
#endif
	}

	except_handler_t except_handler_ = &default_except_handler;
	AnyPtr except_;
}


except_handler_t except_handler(){
	return except_handler_;
}

void set_except_handler(except_handler_t handler){
	except_handler_ = handler;
}

}
