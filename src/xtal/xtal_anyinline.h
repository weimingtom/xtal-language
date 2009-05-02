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

}

#endif // XTAL_ANYINLINE_H_INCLUDE_GUARD
