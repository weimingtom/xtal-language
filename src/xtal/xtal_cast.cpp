#include "xtal.h"
#include "xtal_macro.h"

namespace xtal{

CppClassSymbolData::CppClassSymbolData(){
	static unsigned int counter = 1;
	static CppClassSymbolData* prev_data = 0;
	value = counter++;
	prev = prev_data;
	prebind = 0;
	bind = 0;
	name = 0;
	prev_data = this;
}

CppClassBindTemp::CppClassBindTemp(bind_class_fun_t& dest, bind_class_fun_t src, const char_t*& name, const char_t* given){
	std::memcpy(&dest, &src, sizeof(src));
	std::memcpy(&dummy, &src, sizeof(dummy));
	name = given;
}

CppVarSymbolData::CppVarSymbolData(bind_var_fun_t fun){
	static unsigned int counter = 1;
	static CppVarSymbolData* prev_data = 0;
	value = counter++;
	prev = prev_data;
	maker = fun;
	prev_data = this;
}

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
		XTAL_CASE(TYPE_INT){ return false; }
		XTAL_CASE(TYPE_FLOAT){ return true; }
	}
}

float_t Float_unchecked_cast(const AnyPtr& a){
	switch(type(a)){
		XTAL_DEFAULT{ return 0; }
		XTAL_CASE(TYPE_INT){ return 0; }
		XTAL_CASE(TYPE_FLOAT){ return fvalue(a); }
	}
}

////////////////////////////////////////////////////////////

}
