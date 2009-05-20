/** \file src/xtal/xtal_anyinline.h
* \brief src/xtal/xtal_anyinline.h
*/

#ifndef XTAL_ANYINLINE_H_INCLUDE_GUARD
#define XTAL_ANYINLINE_H_INCLUDE_GUARD

#pragma once

namespace xtal{

void xmemmove(int_t* s1, const int_t* s2, size_t n);
void xmemcpy(int_t* s1, const int_t* s2, size_t n);
void xmemset(int_t* s, int_t c, size_t n);

template<class T>
void  xmemmove(T* s1, const T* s2, size_t n){
	XTAL_ASSERT(n%sizeof(int_t)==0);
	xmemmove((int_t*)s1, (const int_t*)s2, n/sizeof(int_t));
}

template<class T>
void  xmemcpy(T* s1, const T* s2, size_t n){
	XTAL_ASSERT(n%sizeof(int_t)==0);
	typedef char staticassert[static_count_bits<sizeof(int_t)>::value==1 ? 1 : -1];
	xmemcpy((int_t*)s1, (const int_t*)s2, n/sizeof(int_t));
}

template<class T>
void  xmemset(T* s, int_t c, size_t n){
	XTAL_ASSERT(n%sizeof(int_t)==0);
	typedef char staticassert[static_count_bits<sizeof(int_t)>::value==1 ? 1 : -1];
	xmemset((int_t*)s1, c, n/sizeof(int_t));
}


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
