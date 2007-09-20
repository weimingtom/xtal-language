#include "xtal.h"

namespace xtal{

Null null;

namespace{

	int_t Int_to_i(const AnyPtr& p){
		if(type(p)==TYPE_INT){
			return ivalue(p);
		}

		XTAL_THROW(cast_error(p->get_class()->object_name(), "Int"), return 0);
	}

	float_t Int_to_f(const AnyPtr& p){
		if(type(p)==TYPE_INT){
			return (float_t)ivalue(p);
		}

		XTAL_THROW(cast_error(p->get_class()->object_name(), "Int"), return 0);
	}

	StringPtr Int_to_s(const AnyPtr& p){
		if(type(p)==TYPE_INT){
			char buf[32];
			sprintf(buf, "%d", ivalue(p));
			return xnew<String>(buf);
		}

		XTAL_THROW(cast_error(p->get_class()->object_name(), "Int"), return null);
	}


	int_t Float_to_i(const AnyPtr& p){
		if(type(p)==TYPE_FLOAT){
			return (int_t)fvalue(p);
		}

		XTAL_THROW(cast_error(p->get_class()->object_name(), "Float"), return 0);
	}

	float_t Float_to_f(const AnyPtr& p){
		if(type(p)==TYPE_FLOAT){
			return fvalue(p);
		}

		XTAL_THROW(cast_error(p->get_class()->object_name(), "Float"), return 0);
	}

	StringPtr Float_to_s(const AnyPtr& p){
		if(type(p)==TYPE_FLOAT){
			char buf[32];
			sprintf(buf, "%g", fvalue(p));
			return xnew<String>(buf);
		}

		XTAL_THROW(cast_error(p->get_class()->object_name(), "Float"), return null);
	}
}


void initialize_basic_type(){
	{
		ClassPtr p = new_cpp_class<Int>("Int");	
		p->method("to_i", &Int_to_i);
		p->method("to_f", &Int_to_f);
		p->method("to_s", &Int_to_s);
	}

	{
		ClassPtr p = new_cpp_class<Float>("Float");		
		p->method("to_i", &Float_to_i);
		p->method("to_f", &Float_to_f);
		p->method("to_s", &Float_to_s);
	}

	builtin()->def("String", new_cpp_class<String>());
	builtin()->def("Int", new_cpp_class<Int>());
	builtin()->def("Float", new_cpp_class<Float>());
	builtin()->def("Null", new_cpp_class<Null>());
	builtin()->def("True", new_cpp_class<True>());
	builtin()->def("False", new_cpp_class<False>());
}

}
