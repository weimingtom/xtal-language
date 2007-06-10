
#include "xtal.h"
#include "xtal_cast.h"
#include "xtal_frame.h"

namespace xtal{

void* cast_helper_helper_extend_anyimpl(const Any& a, const Class& cls){
	if(a.is(cls)){ return (void*)a.impl(); }
	XTAL_THROW(cast_error(a, cls.object_name()));
	return 0;
}

void* cast_helper_helper_extend_any(const Any& a, const Class& cls){
	if(a.is(cls)){ return (void*)&a; }
	XTAL_THROW(cast_error(a, cls.object_name()));
	return 0;
}

void* cast_helper_helper_other(const Any& a, const Class& cls){
	if(a.is(cls)){ return ((UserData<int>&)a).get(); }
	XTAL_THROW(cast_error(a, cls.object_name()));
	return 0;
}

void* arg_cast_helper_helper_extend_anyimpl(const Any& a, int param_num, const Any& param_name, const Class& cls){
	if(a.is(cls)){ return (void*)a.impl(); }
	XTAL_THROW(argument_error(a, cls.object_name(), param_num, param_name));
	return 0;
}

void* arg_cast_helper_helper_extend_any(const Any& a, int param_num, const Any& param_name, const Class& cls){
	if(a.is(cls)){ return (void*)&a; }
	XTAL_THROW(argument_error(a, cls.object_name(), param_num, param_name));
	return 0;
}

void* arg_cast_helper_helper_other(const Any& a, int param_num, const Any& param_name, const Class& cls){
	if(a.is(cls)){ return ((UserData<int>&)a).get(); }
	XTAL_THROW(argument_error(a, cls.object_name(), param_num, param_name));
	return 0;
}

const ID* CastHelper<const ID*>::as(const Any& a){ 
	if(const String* p = xtal::as<const String*>(a)){
		if(p->is_interned()){
			return (const ID*)p;
		}
	}
	return 0;
}

const ID* CastHelper<const ID*>::cast(const Any& a){
	if(const ID* p = as(a)){
		return p;
	}
	XTAL_THROW(cast_error(a, "interned String"));
	return 0;
}

const ID* CastHelper<const ID*>::arg_cast(const Any& a, int param_num, const Any& param_name){
	if(const ID* p = as(a)){
		return p;
	}
	XTAL_THROW(argument_error(a, "interned String", param_num, param_name));
	return 0;
}


const char* CastHelper<const char*>::as(const Any& a){ 
	if(const String* p = xtal::as<const String*>(a)){
		return p->c_str();
	}
	return 0;
}

const char* CastHelper<const char*>::cast(const Any& a){
	if(const char* p = as(a)){
		return p;
	}
	XTAL_THROW(cast_error(a, TClass<String>::get().object_name()));
	return 0;
}

const char* CastHelper<const char*>::arg_cast(const Any& a, int param_num, const Any& param_name){
	if(const char* p = as(a)){
		return p;
	}
	XTAL_THROW(argument_error(a, TClass<String>::get().object_name(), param_num, param_name));
	return 0;
}


int_t CastHelper<int_t>::as(const Any& a){
	switch(a.type()){
		XTAL_DEFAULT;
		XTAL_CASE(TYPE_INT){ return a.ivalue(); }
		XTAL_CASE(TYPE_FLOAT){ return static_cast<int_t>(a.fvalue()); }
	}
	return 0;
}
	
int_t CastHelper<int_t>::cast(const Any& a){
	switch(a.type()){
		XTAL_DEFAULT{ XTAL_THROW(cast_error(a, String("Int"))); }
		XTAL_CASE(TYPE_INT){ return a.ivalue(); }
		XTAL_CASE(TYPE_FLOAT){ return static_cast<int_t>(a.fvalue()); }
	}
	return 0;
}

int_t CastHelper<int_t>::arg_cast(const Any& a, int param_num, const Any& param_name){
	switch(a.type()){
		XTAL_DEFAULT{ XTAL_THROW(argument_error(a, String("Int"), param_num, param_name)); }
		XTAL_CASE(TYPE_INT){ return a.ivalue(); }
		XTAL_CASE(TYPE_FLOAT){ return static_cast<int_t>(a.fvalue()); }
	}
	return 0;
}
	
float_t CastHelper<float_t>::as(const Any& a){
	switch(a.type()){
		XTAL_DEFAULT;
		XTAL_CASE(TYPE_INT){ return static_cast<float_t>(a.ivalue()); }
		XTAL_CASE(TYPE_FLOAT){ return a.fvalue(); }
	}
	return 0;
}
	
float_t CastHelper<float_t>::cast(const Any& a){
	switch(a.type()){
		XTAL_DEFAULT{ XTAL_THROW(cast_error(a, String("Float"))); }
		XTAL_CASE(TYPE_INT){ return static_cast<float_t>(a.ivalue()); }
		XTAL_CASE(TYPE_FLOAT){ return a.fvalue(); }
	}
	return 0;
}

float_t CastHelper<float_t>::arg_cast(const Any& a, int param_num, const Any& param_name){
	switch(a.type()){
		XTAL_DEFAULT{ XTAL_THROW(argument_error(a, String("Float"), param_num, param_name)); }
		XTAL_CASE(TYPE_INT){ return static_cast<float_t>(a.ivalue()); }
		XTAL_CASE(TYPE_FLOAT){ return a.fvalue(); }
	}
	return 0;
}

}
