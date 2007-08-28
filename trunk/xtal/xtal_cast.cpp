
#include "xtal.h"
#include "xtal_cast.h"
#include "xtal_frame.h"
#include "xtal_frame.h"

namespace xtal{

void* cast_helper_helper_extend_anyimpl(const AnyPtr& a, const ClassPtr& cls){
	if(a->is(cls)){ return (void*)pvalue(a); }
	XTAL_THROW(cast_error(a, cls->object_name()), return 0);
}

void* cast_helper_helper_extend_any(const AnyPtr& a, const ClassPtr& cls){
	if(a->is(cls)){ return (void*)&a; }
	XTAL_THROW(cast_error(a, cls->object_name()), return 0);
}

void* cast_helper_helper_other(const AnyPtr& a, const ClassPtr& cls){
	if(a->is(cls)){ return ((SmartPtr<int>&)a).get(); }
	XTAL_THROW(cast_error(a, cls->object_name()), return 0);
}

void* arg_cast_helper_helper_extend_anyimpl(const AnyPtr& a, int_t param_num, const AnyPtr& param_name, const ClassPtr& cls){
	if(a->is(cls)){ return (void*)pvalue(a); }
	XTAL_THROW(argument_error(a, cls->object_name(), param_num, param_name), return 0);
}

void* arg_cast_helper_helper_extend_any(const AnyPtr& a, int_t param_num, const AnyPtr& param_name, const ClassPtr& cls){
	if(a->is(cls)){ return (void*)&a; }
	XTAL_THROW(argument_error(a, cls->object_name(), param_num, param_name), return 0);
}

void* arg_cast_helper_helper_other(const AnyPtr& a, int_t param_num, const AnyPtr& param_name, const ClassPtr& cls){
	if(a->is(cls)){ return ((SmartPtr<int>&)a).get(); }
	XTAL_THROW(argument_error(a, cls->object_name(), param_num, param_name), return 0);
}

const InternedStringPtr* CastHelper<const InternedStringPtr*>::as(const AnyPtr& a){ 
	if(String* p = xtal::as<String*>(a)){
		if(p->is_interned()){
			return (const InternedStringPtr*)p;
		}
	}
	return 0;
}

const InternedStringPtr* CastHelper<const InternedStringPtr*>::cast(const AnyPtr& a){
	if(const InternedStringPtr* p = as(a)){
		return p;
	}
	XTAL_THROW(cast_error(a, "interned String"), return 0);
}

const InternedStringPtr* CastHelper<const InternedStringPtr*>::arg_cast(const AnyPtr& a, int_t param_num, const AnyPtr& param_name){
	if(const InternedStringPtr* p = as(a)){
		return p;
	}
	XTAL_THROW(argument_error(a, "interned String", param_num, param_name), return 0);
}


const char_t* CastHelper<const char_t*>::as(const AnyPtr& a){ 
	if(String* p = xtal::as<String*>(a)){
		return p->c_str();
	}
	return 0;
}

const char_t* CastHelper<const char_t*>::cast(const AnyPtr& a){
	if(const char* p = as(a)){
		return p;
	}
	XTAL_THROW(cast_error(a, get_cpp_class<String>()->object_name()), return 0);
}

const char_t* CastHelper<const char_t*>::arg_cast(const AnyPtr& a, int_t param_num, const AnyPtr& param_name){
	if(const char_t* p = as(a)){
		return p;
	}
	XTAL_THROW(argument_error(a, get_cpp_class<String>()->object_name(), param_num, param_name), return 0);
}


int_t CastHelper<int_t>::as(const AnyPtr& a){
	switch(type(a)){
		XTAL_DEFAULT;
		XTAL_CASE(TYPE_INT){ return ivalue(a); }
		XTAL_CASE(TYPE_FLOAT){ return static_cast<int_t>(fvalue(a)); }
	}
	return 0;
}
	
int_t CastHelper<int_t>::cast(const AnyPtr& a){
	switch(type(a)){
		XTAL_DEFAULT{ XTAL_THROW(cast_error(a, xnew<String>("Int")), return 0); }
		XTAL_CASE(TYPE_INT){ return ivalue(a); }
		XTAL_CASE(TYPE_FLOAT){ return static_cast<int_t>(fvalue(a)); }
	}
	return 0;
}

int_t CastHelper<int_t>::arg_cast(const AnyPtr& a, int_t param_num, const AnyPtr& param_name){
	switch(type(a)){
		XTAL_DEFAULT{ XTAL_THROW(argument_error(a, xnew<String>("Int"), param_num, param_name), return 0); }
		XTAL_CASE(TYPE_INT){ return ivalue(a); }
		XTAL_CASE(TYPE_FLOAT){ return static_cast<int_t>(fvalue(a)); }
	}
	return 0;
}
	
float_t CastHelper<float_t>::as(const AnyPtr& a){
	switch(type(a)){
		XTAL_DEFAULT;
		XTAL_CASE(TYPE_INT){ return static_cast<float_t>(ivalue(a)); }
		XTAL_CASE(TYPE_FLOAT){ return fvalue(a); }
	}
	return 0;
}
	
float_t CastHelper<float_t>::cast(const AnyPtr& a){
	switch(type(a)){
		XTAL_DEFAULT{ XTAL_THROW(cast_error(a, xnew<String>("Float")), return 0); }
		XTAL_CASE(TYPE_INT){ return static_cast<float_t>(ivalue(a)); }
		XTAL_CASE(TYPE_FLOAT){ return fvalue(a); }
	}
	return 0;
}

float_t CastHelper<float_t>::arg_cast(const AnyPtr& a, int_t param_num, const AnyPtr& param_name){
	switch(type(a)){
		XTAL_DEFAULT{ XTAL_THROW(argument_error(a, xnew<String>("Float"), param_num, param_name), return 0); }
		XTAL_CASE(TYPE_INT){ return static_cast<float_t>(ivalue(a)); }
		XTAL_CASE(TYPE_FLOAT){ return fvalue(a); }
	}
	return 0;
}

}
