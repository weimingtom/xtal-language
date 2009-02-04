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

	if(const ClassPtr& klass = ptr_as<Class>(target)){
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
		p->def_method(Xid(initialize), &Exception::initialize)->params(Xid(message), empty_id);
		p->def_method(Xid(to_s), &Exception::to_s);
		p->def_method(Xid(message), &Exception::message);
		p->def_method(Xid(backtrace), &Exception::backtrace);
		p->def_method(Xid(append_backtrace), &Exception::append_backtrace);

		builtin()->def(Xid(Exception), p);
	}
}

void initialize_except_script(){
	Xemb((

builtin::StandardError: class(Exception){}
builtin::RuntimeError: class(StandardError){}
builtin::IOError: class(StandardError){}
builtin::LogicError: class(StandardError){}
builtin::CastError: class(StandardError){}
builtin::ArgumentError: class(StandardError){}
builtin::YieldError: class(StandardError){}
builtin::InstanceVariableError: class(StandardError){}
builtin::UnsupportedError: class(StandardError){}
builtin::RedefinedError: class(StandardError){}
builtin::AccessibilityError: class(StandardError){}
builtin::AssertionFailed: class(StandardError){}
builtin::CompileError: class(StandardError){
initialize: method(message, errors:[]){
		Exception::initialize(%f"%s\n%s"(message, errors.join("\t\n")));
	}		
}

	),
"\x78\x74\x61\x6c\x01\x00\x00\x00\x00\x00\x00\x4b\x39\x00\x01\x89\x00\x01\x00\x0f\x0b\x2f\x00\x00\x00\x00\x00\x02\x0b\x25\x01\x25\x00\x37\x00\x03\x39\x00\x01\x89\x00\x02\x00\x0f\x0b\x2f\x00\x00\x00\x00\x00\x04\x01\x25\x01\x25\x00\x37\x00\x05\x39\x00\x01\x89"
"\x00\x03\x00\x0f\x0b\x2f\x00\x00\x00\x00\x00\x04\x01\x25\x01\x25\x00\x37\x00\x06\x25\x00\x8b\x00\x03\x08\x00\x00\x00\x00\x00\x02\x00\x00\x00\x12\x00\x20\x00\x00\x00\x00\x00\x04\x00\x00\x00\x12\x00\x38\x00\x00\x00\x00\x00\x06\x00\x00\x00\x12\x00\x00\x00\x00"
"\x04\x00\x00\x00\x00\x03\x06\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x08\x00\x00\x00\x05\x00\x02\x00\x00\x00\x00\x00\x00\x01\x00\x00\x20\x00\x00\x00\x05\x00\x04\x00\x00\x00\x00\x00\x00\x01\x00\x00\x38\x00\x00\x00\x05\x00\x06\x00\x00\x00\x00\x00\x00\x01\x00"
"\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x14\x00\x00\x00\x00\x11\x00\x00\x00\x00\x00\x00\x00\x06\x00\x00\x00\x00\x00\x00\x00\x03\x00\x00\x00\x03\x00\x00\x00\x06\x00\x00\x00\x08\x00\x00\x00\x04\x00\x00\x00\x10\x00\x00\x00\x05\x00\x00"
"\x00\x13\x00\x00\x00\x06\x00\x00\x00\x18\x00\x00\x00\x0b\x00\x00\x00\x18\x00\x00\x00\x08\x00\x00\x00\x1b\x00\x00\x00\x0b\x00\x00\x00\x20\x00\x00\x00\x09\x00\x00\x00\x28\x00\x00\x00\x0a\x00\x00\x00\x2b\x00\x00\x00\x0b\x00\x00\x00\x30\x00\x00\x00\x10\x00\x00"
"\x00\x30\x00\x00\x00\x0d\x00\x00\x00\x33\x00\x00\x00\x10\x00\x00\x00\x38\x00\x00\x00\x0e\x00\x00\x00\x40\x00\x00\x00\x0f\x00\x00\x00\x43\x00\x00\x00\x10\x00\x00\x00\x48\x00\x00\x00\x11\x00\x00\x00\x00\x01\x0b\x00\x00\x00\x03\x09\x00\x00\x00\x06\x73\x6f\x75"
"\x72\x63\x65\x09\x00\x00\x00\x11\x74\x6f\x6f\x6c\x2f\x74\x65\x6d\x70\x2f\x69\x6e\x2e\x78\x74\x61\x6c\x09\x00\x00\x00\x0b\x69\x64\x65\x6e\x74\x69\x66\x69\x65\x72\x73\x0a\x00\x00\x00\x07\x09\x00\x00\x00\x00\x09\x00\x00\x00\x05\x4d\x75\x74\x65\x78\x09\x00\x00"
"\x00\x04\x6c\x6f\x63\x6b\x09\x00\x00\x00\x0b\x62\x6c\x6f\x63\x6b\x5f\x66\x69\x72\x73\x74\x09\x00\x00\x00\x06\x75\x6e\x6c\x6f\x63\x6b\x09\x00\x00\x00\x0a\x62\x6c\x6f\x63\x6b\x5f\x6e\x65\x78\x74\x09\x00\x00\x00\x0b\x62\x6c\x6f\x63\x6b\x5f\x62\x72\x65\x61\x6b"
"\x09\x00\x00\x00\x06\x76\x61\x6c\x75\x65\x73\x0a\x00\x00\x00\x01\x03"
)->call();
}


}
