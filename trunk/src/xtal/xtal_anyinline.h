/** \file src/xtal/xtal_anyinline.h
* \brief src/xtal/xtal_anyinline.h
*/

#ifndef XTAL_ANYINLINE_H_INCLUDE_GUARD
#define XTAL_ANYINLINE_H_INCLUDE_GUARD

#pragma once

namespace xtal{

inline const ClassPtr& Base::get_class(){ 
	return to_smartptr(class_); 
}

inline void inc_ref_count_force(const Any& v){
	if(type(v)==TYPE_BASE){
		pvalue(v)->inc_ref_count();
	}
	else if(type(v)>TYPE_BASE){
		rcpvalue(v)->inc_ref_count();		
	}
}

inline void dec_ref_count_force(const Any& v){
	if(type(v)==TYPE_BASE){
		pvalue(v)->dec_ref_count();
	}
	else if(type(v)>TYPE_BASE){
		rcpvalue(v)->dec_ref_count();		
	}
}

inline InstanceVariables* Any::instance_variables() const{
	if(type(*this)==TYPE_BASE){
		return pvalue(*this)->instance_variables();
	}
	return &empty_instance_variables;
}

struct f2{
	float_t a, b;
};

inline f2 to_f2(int_t atype, const AnyPtr& a, int_t btype, const AnyPtr& b){
	f2 ret;
	if(atype==0){
		ret.a = (float_t)ivalue(a);
		ret.b = fvalue(b);
	}
	else if(btype==0){
		ret.a = fvalue(a);
		ret.b = (float_t)ivalue(b);
	}
	else{
		ret.a = fvalue(a);
		ret.b = fvalue(b);
	}
	return ret;
}

}

#endif // XTAL_ANYINLINE_H_INCLUDE_GUARD
