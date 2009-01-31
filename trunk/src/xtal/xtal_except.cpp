#include "xtal.h"
#include "xtal_macro.h"

namespace xtal{

Exception::Exception(const AnyPtr& message){
	initialize(message);
}

void Exception::initialize(const AnyPtr& message){
	message_ = message->to_s();
	backtrace_ = xnew<Array>();
}

void Exception::append_backtrace(const AnyPtr& file, int_t line, const AnyPtr& function_name){
	backtrace_->push_back(Xf("\t%(file)s:%(line)d: in %(function_name)s")->call(
		Named(Xid(file), file), Named(Xid(line), line), Named(Xid(function_name), function_name)));
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
	return builtin()->member(Xid(CastError))->call(Xt("Xtal Runtime Error 1004")->call(
		Named(Xid(type), from->get_class()->object_name()), Named(Xid(required), to)
	));
}

AnyPtr argument_error(const AnyPtr& object, int_t no){
	return builtin()->member(Xid(ArgumentError))->call(Xt("Xtal Runtime Error 1001")->call(
		Named(Xid(object), object), 
		Named(Xid(no), no)
	));
}

AnyPtr unsupported_error(const AnyPtr& target, const IDPtr& primary_key, const AnyPtr& secondary_key){

	IDPtr pick;

	if(ClassPtr klass = ptr_as<Class>(target)){
		pick = klass->find_near_member(primary_key, secondary_key);
		if(raweq(pick, primary_key)){
			pick = null;
		}
	}

	if(pick){
		if(secondary_key){
			return UnsupportedError()->call(Xt("Xtal Runtime Error 1021")->call(
				Named(Xid(object), Xf("%s::%s#%s")->call(target->object_name(), primary_key, secondary_key)),
				Named(Xid(pick), pick)
			));	
		}
		else{
			return UnsupportedError()->call(Xt("Xtal Runtime Error 1021")->call(
				Named(Xid(object), Xf("%s::%s")->call(target->object_name(), primary_key)),
				Named(Xid(pick), pick)
			));	
		}
	}
	else{
		if(secondary_key){
			return UnsupportedError()->call(Xt("Xtal Runtime Error 1015")->call(
				Named(Xid(object), Xf("%s::%s#%s")->call(target->object_name(), primary_key, secondary_key))
			));
		}
		else{
			return UnsupportedError()->call(Xt("Xtal Runtime Error 1015")->call(
				Named(Xid(object), Xf("%s::%s")->call(target->object_name(), primary_key))
			));		
		}
	}
}

namespace{
	void default_except_handler(const AnyPtr& except, const char* file, int line){
//#ifdef XTAL_NO_EXCEPTIONS
//		printf("%s(%d):%s\n", file, line, except->to_s()->c_str());
//		exit(1);
//#endif
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


void initialize_except(){
	{
		ClassPtr p = new_cpp_class<Exception>();
		p->def(Xid(new), ctor<Exception>()->params(Xid(message), empty_id));
		p->method(Xid(initialize), &Exception::initialize)->params(Xid(message), empty_id);
		p->method(Xid(to_s), &Exception::to_s);
		p->method(Xid(message), &Exception::message);
		p->method(Xid(backtrace), &Exception::backtrace);
		p->method(Xid(append_backtrace), &Exception::append_backtrace);

		builtin()->def(Xid(Exception), p);
	}
}


}
