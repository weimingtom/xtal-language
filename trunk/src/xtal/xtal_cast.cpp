
#include "xtal.h"
#include "xtal_macro.h"


namespace xtal{

const char_t* cast_const_char_t_ptr(const AnyPtr& a){
	return unchecked_ptr_cast<String>(a)->c_str(); 
}

const IDPtr& cast_IDPtr(const AnyPtr& a){
	if(a){
		const StringPtr& s = unchecked_ptr_cast<String>(a);
		return s->is_interned() ? *(IDPtr*)&a : *(IDPtr*)&s->intern();
	}
	return *(IDPtr*)&a;
}

}
