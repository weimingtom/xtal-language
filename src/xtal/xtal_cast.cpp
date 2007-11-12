#include "xtal.h"

namespace xtal{

const void* cast_helper_helper_smartptr(const AnyPtr& a, const ClassPtr& cls){
	if(a->is(cls)){ return &a; }
	XTAL_THROW(cast_error(a, cls->object_name()), return 0);
}

const void* cast_helper_helper_base(const AnyPtr& a, const ClassPtr& cls){
	if(a->is(cls)){ return pvalue(a); }
	XTAL_THROW(cast_error(a, cls->object_name()), return 0);
}

const void* cast_helper_helper_innocence(const AnyPtr& a, const ClassPtr& cls){
	if(a->is(cls)){ return &a; }
	XTAL_THROW(cast_error(a, cls->object_name()), return 0);
}

const void* cast_helper_helper_other(const AnyPtr& a, const ClassPtr& cls){
	if(a->is(cls)){ return ((SmartPtr<int>&)a).get(); }
	XTAL_THROW(cast_error(a, cls->object_name()), return 0);
}

////////////////////////////////////////////////////////////////////////

const IDPtr* CastHelper<const IDPtr*>::as(const AnyPtr& a){ 
	if(String* p = xtal::as<String*>(a)){
		if(p->is_interned()){
			return (const IDPtr*)p;
		}
	}
	return 0;
}

const IDPtr* CastHelper<const IDPtr*>::cast(const AnyPtr& a){
	if(const IDPtr* p = as(a)){
		return p;
	}
	XTAL_THROW(cast_error(a, "interned String"), return 0);
}

bool CastHelper<const IDPtr*>::can_cast(const AnyPtr& a){
	return as(a)!=0;
}

const IDPtr* CastHelper<const IDPtr*>::nocheck_cast(const AnyPtr& a){ 
	return (const IDPtr*)&a;
}

////////////////////////////////////////////////////////////

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

bool CastHelper<const char_t*>::can_cast(const AnyPtr& a){
	return as(a)!=0;
}

const char_t* CastHelper<const char_t*>::nocheck_cast(const AnyPtr& a){ 
	return static_ptr_cast<String>(a)->c_str();
}

////////////////////////////////////////////////////////////

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

bool CastHelper<int_t>::can_cast(const AnyPtr& a){
	switch(type(a)){
		XTAL_DEFAULT{ return false; }
		XTAL_CASE(TYPE_INT){ return true; }
		XTAL_CASE(TYPE_FLOAT){ return true; }
	}
	return false;
}

int_t CastHelper<int_t>::nocheck_cast(const AnyPtr& a){
	switch(type(a)){
		XTAL_NODEFAULT;
		XTAL_CASE(TYPE_INT){ return ivalue(a); }
		XTAL_CASE(TYPE_FLOAT){ return static_cast<int_t>(fvalue(a)); }
	}
	return 0;
}

////////////////////////////////////////////////////////////

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

bool CastHelper<float_t>::can_cast(const AnyPtr& a){
	switch(type(a)){
		XTAL_DEFAULT{ return false; }
		XTAL_CASE(TYPE_INT){ return true; }
		XTAL_CASE(TYPE_FLOAT){ return true; }
	}
	return false;
}

float_t CastHelper<float_t>::nocheck_cast(const AnyPtr& a){
	switch(type(a)){
		XTAL_NODEFAULT;
		XTAL_CASE(TYPE_INT){ return static_cast<float_t>(ivalue(a)); }
		XTAL_CASE(TYPE_FLOAT){ return fvalue(a); }
	}
	return 0;
}

////////////////////////////////////////////////////////////

}
