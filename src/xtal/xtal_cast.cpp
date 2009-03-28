#include "xtal.h"
#include "xtal_macro.h"

namespace xtal{

////////////////////////////////////////////////////////////////////////

const char_t* CastHelper<const char_t*>::unchecked_cast(const AnyPtr& a){ 
	return ptr_cast<String>(a)->c_str(); 
}

bool CastHelper<const IDPtr*>::can_cast(const AnyPtr& a){
	if(String* p = xtal::cast<String*>(a)){
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

bool Int_can_cast(const AnyPtr& a){
	switch(type(a)){
		XTAL_DEFAULT{ return false; }
		XTAL_CASE(TYPE_INT){ return true; }
		XTAL_CASE(TYPE_FLOAT){ return true; }
	}
}

int_t Int_unchecked_cast(const AnyPtr& a){
	switch(type(a)){
		XTAL_DEFAULT{ return 0; }
		XTAL_CASE(TYPE_INT){ return ivalue(a); }
		XTAL_CASE(TYPE_FLOAT){ return static_cast<uint_t>(fvalue(a)); }
	}
}

////////////////////////////////////////////////////////////

bool Float_can_cast(const AnyPtr& a){
	switch(type(a)){
		XTAL_DEFAULT{ return false; }
		XTAL_CASE(TYPE_INT){ return true; }
		XTAL_CASE(TYPE_FLOAT){ return true; }
	}
}

float_t Float_unchecked_cast(const AnyPtr& a){
	switch(type(a)){
		XTAL_DEFAULT{ return 0; }
		XTAL_CASE(TYPE_INT){ return static_cast<float_t>(ivalue(a)); }
		XTAL_CASE(TYPE_FLOAT){ return fvalue(a); }
	}
}

////////////////////////////////////////////////////////////

}
