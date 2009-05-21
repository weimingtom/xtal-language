/** \file src/xtal/xtal_anyinline.h
* \brief src/xtal/xtal_anyinline.h
*/

#ifndef XTAL_ANYINLINE_H_INCLUDE_GUARD
#define XTAL_ANYINLINE_H_INCLUDE_GUARD

#pragma once

namespace xtal{

/*
void xmemmove(int_t* s1, const int_t* s2, size_t n);
void xmemcpy(int_t* s1, const int_t* s2, size_t n);
void xmemset(int_t* s, int_t c, size_t n);

template<class T>
void  xmemmove(T* s1, const T* s2, size_t n){
	xmemmove((int_t*)s1, (const int_t*)s2, n*(sizeof(T)/sizeof(int_t)));
}

template<class T>
void  xmemcpy(T* s1, const T* s2, size_t n){
	xmemcpy((int_t*)s1, (const int_t*)s2, n*(sizeof(T)/sizeof(int_t)));
}

template<class T>
void  xmemset(T* s, int_t c, size_t n){
	xmemset((int_t*)s, c, n*(sizeof(T)/sizeof(int_t)));
}
*/

inline const ClassPtr& Base::get_class(){ 
	return to_smartptr(class_); 
}

inline void inc_ref_count_force(const Any& v){
	uint_t t = type(v);

	if(t<TYPE_BASE){
		
	}
	else if(t==TYPE_BASE){
		pvalue(v)->inc_ref_count();
	}
	else{
		rcpvalue(v)->inc_ref_count();		
	}
}

inline void dec_ref_count_force(const Any& v){
	uint_t t = type(v);

	if(t<TYPE_BASE){
		
	}
	else if(t==TYPE_BASE){
		pvalue(v)->dec_ref_count();
	}
	else{
		rcpvalue(v)->dec_ref_count();		
	}
}

inline InstanceVariables* Any::instance_variables() const{
	if(type(*this)==TYPE_BASE){
		return pvalue(*this)->instance_variables();
	}
	return &empty_instance_variables;
}

inline const ClassPtr& Any::get_class() const{
	int_t t = type(*this);
	if(t==TYPE_BASE){ return pvalue(*this)->get_class(); }
	return cpp_class(*classdata[t]);
}

inline bool Any::is(const AnyPtr& klass) const{
	const ClassPtr& my_class = get_class();
	if(raweq(my_class, klass)) return true;
	return cache_is(my_class, klass);
}

struct f2{
	float_t a, b;
};

inline void to_f2(f2& ret, int_t atype, const AnyPtr& a, int_t btype, const AnyPtr& b){
	/*	if(atype==btype){
		ret.a = fvalue(a); 
		ret.b = fvalue(b); 
	}
	else if(atype==0){
		ret.a = (float_t)ivalue(a); 
		ret.b = fvalue(b); 
	}
	else{
		ret.a = fvalue(a); 
		ret.b = (float_t)ivalue(b); 
	}
	*/
	float_t aa[2] = {(float_t)ivalue(a), fvalue(a)};
	float_t bb[2] = {(float_t)ivalue(b), fvalue(b)};
	ret.a = aa[atype];
	ret.b = bb[btype];
}

}

#endif // XTAL_ANYINLINE_H_INCLUDE_GUARD
