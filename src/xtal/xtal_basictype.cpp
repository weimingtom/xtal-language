#include "xtal.h"
#include "xtal_macro.h"

namespace xtal{

Null null;
Nop nop;

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
			return Xf("%d")(p)->to_s();
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
			return Xf("%g")(p)->to_s();
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

	builtin()->def("MissingNS", new_cpp_class<MissingNS>());
	builtin()->def("String", new_cpp_class<String>());
	builtin()->def("Int", new_cpp_class<Int>());
	builtin()->def("Float", new_cpp_class<Float>());
	builtin()->def("Null", new_cpp_class<Null>());
	builtin()->def("Nop", new_cpp_class<Nop>());
	builtin()->def("True", new_cpp_class<True>());
	builtin()->def("False", new_cpp_class<False>());
}

}
