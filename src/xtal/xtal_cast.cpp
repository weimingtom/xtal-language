#include "xtal.h"
#include "xtal_macro.h"

namespace xtal{
	
const char_t* cast_const_char_t_ptr(const AnyPtr& a){
	return ptr_cast<String>(a)->c_str(); 
}

const IDPtr& cast_IDPtr(const AnyPtr& a){
	if(a){
		const StringPtr& s = unchecked_ptr_cast<String>(a);
		return s->is_interned() ? *(IDPtr*)&a : *(IDPtr*)&s->intern();
	}
	return *(IDPtr*)&a;
}

int_t cast_Int(const AnyPtr& a){
	switch(type(a)){
		XTAL_DEFAULT{ return 0; }
		XTAL_CASE(TYPE_INT){ return ivalue(a); }
		XTAL_CASE(TYPE_FLOAT){ return static_cast<int_t>(fvalue(a)); }
	}
}

float_t cast_Float(const AnyPtr& a){
	switch(type(a)){
		XTAL_DEFAULT{ return 0; }
		XTAL_CASE(TYPE_INT){ return static_cast<float_t>(ivalue(a)); }
		XTAL_CASE(TYPE_FLOAT){ return fvalue(a); }
	}
}

}
