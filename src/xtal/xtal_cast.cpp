#include "xtal.h"
#include "xtal_macro.h"

namespace xtal{

void cast_failed(const AnyPtr& a, const ClassPtr& cls){
	XTAL_SET_EXCEPT(cast_error(a, cls->object_name()));
}

////////////////////////////////////////////////////////////////////////

const char_t* CastHelper<const char_t*>::unchecked_cast(const AnyPtr& a){ 
	return ptr_cast<String>(a)->c_str(); 
}

bool CastHelper<const IDPtr*>::can_cast(const AnyPtr& a){
	if(String* p = xtal::as<String*>(a)){
		if(p->is_interned()){
			return true;
		}
	}
	return false;
}

const IDPtr* CastHelper<const IDPtr*>::unchecked_cast(const AnyPtr& a){ 
	return (const IDPtr*)&a;
}

////////////////////////////////////////////////////////////

bool CastHelper<int_t>::can_cast(const AnyPtr& a){
	switch(type(a)){
		XTAL_DEFAULT{ return false; }
		XTAL_CASE(TYPE_INT){ return true; }
		XTAL_CASE(TYPE_FLOAT){ return true; }
	}
	return false;
}

int_t CastHelper<int_t>::unchecked_cast(const AnyPtr& a){
	switch(type(a)){
		XTAL_NODEFAULT;
		XTAL_CASE(TYPE_INT){ return ivalue(a); }
		XTAL_CASE(TYPE_FLOAT){ return static_cast<int_t>(fvalue(a)); }
	}
	return 0;
}

////////////////////////////////////////////////////////////

bool CastHelper<float_t>::can_cast(const AnyPtr& a){
	switch(type(a)){
		XTAL_DEFAULT{ return false; }
		XTAL_CASE(TYPE_INT){ return true; }
		XTAL_CASE(TYPE_FLOAT){ return true; }
	}
	return false;
}

float_t CastHelper<float_t>::unchecked_cast(const AnyPtr& a){
	switch(type(a)){
		XTAL_NODEFAULT;
		XTAL_CASE(TYPE_INT){ return static_cast<float_t>(ivalue(a)); }
		XTAL_CASE(TYPE_FLOAT){ return fvalue(a); }
	}
	return 0;
}

////////////////////////////////////////////////////////////

}
