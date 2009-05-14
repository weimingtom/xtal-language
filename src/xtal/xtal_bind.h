/** \file src/xtal/xtal_bind.h
* \brief src/xtal/xtal_bind.h
*/

#ifndef XTAL_BIND_H_INCLUDE_GUARD
#define XTAL_BIND_H_INCLUDE_GUARD

#pragma once

namespace xtal{

//{REPEAT{{
/*

template<class R #COMMA_REPEAT#class A`i`#>
struct cfun_holder_base`n`{
	enum{ PARAMS = `n`, PARAM_N = `n`, METHOD = 0, EXTENDABLE = 0 };
	#REPEAT#typedef ArgGetter<A`i`, `i`> ARG`i`;#
	typedef ReturnResult Result;
	typedef param_types_holder`n`<void #COMMA_REPEAT#A`i`#> fun_param_holder;
};

template<class R #COMMA_REPEAT#class A`i`#>
struct cfun_holder<R (*)(#REPEAT_COMMA#A`i`#)> : public cfun_holder_base`n`<R #COMMA_REPEAT#A`i`#>{
	typedef R (*fun_t)(#REPEAT_COMMA#A`i`#);
	static R call(const void* fun #COMMA_REPEAT#A`i` a`i`#){ 
		return (*(fun_t*)fun)(#REPEAT_COMMA#a`i`#); 
	}
};

#if defined(_WIN32) && defined(_M_IX86)
template<class R #COMMA_REPEAT#class A`i`#>
struct cfun_holder<R (__stdcall *)(#REPEAT_COMMA#A`i`#)> : public cfun_holder_base`n`<R #COMMA_REPEAT#A`i`#>{
	typedef R (__stdcall *fun_t)(#REPEAT_COMMA#A`i`#);
	static R call(const void* fun #COMMA_REPEAT#A`i` a`i`#){ 
		return (*(fun_t*)fun)(#REPEAT_COMMA#a`i`#); 
	}
};

#endif

template<class C, class R #COMMA_REPEAT#class A`i`#>
struct cmemfun_holder_base`n`{
	enum{ PARAMS = `n+1`, PARAM_N = `n`, METHOD = 1, EXTENDABLE = 0 };
	typedef ArgThisGetter<C> ARG0;
	#REPEAT#typedef ArgGetter<A`i`, `i`> ARG`i+1`;#
	typedef ReturnResult Result;
	typedef param_types_holder`n`<C #COMMA_REPEAT#A`i`#> fun_param_holder;
};

template<class C, class R #COMMA_REPEAT#class A`i`#>
struct cmemfun_holder<R (C::*)(#REPEAT_COMMA#A`i`#)> : public cmemfun_holder_base`n`<C*, R #COMMA_REPEAT#A`i`#>{
	typedef R (C::*fun_t)(#REPEAT_COMMA#A`i`#);
	static R call(const void* fun, C* self #COMMA_REPEAT#A`i` a`i`#){ 
		return (self->**(fun_t*)fun)(#REPEAT_COMMA#a`i`#); 
	}
};

template<class C, class R #COMMA_REPEAT#class A`i`#>
struct cmemfun_holder<R (C::*)(#REPEAT_COMMA#A`i`#) const> : public cmemfun_holder_base`n`<C*, R #COMMA_REPEAT#A`i`#>{
	typedef R (C::*fun_t)(#REPEAT_COMMA#A`i`#) const;
	static R call(const void* fun, C* self #COMMA_REPEAT#A`i` a`i`#){ 
		return (self->**(fun_t*)fun)(#REPEAT_COMMA#a`i`#); 
	}
};

template<class C, class R #COMMA_REPEAT#class A`i`#>
struct cmemfun_holder<R (*)(C #COMMA_REPEAT#A`i`#)> : public cmemfun_holder_base`n`<C, R #COMMA_REPEAT#A`i`#>{
	typedef R (*fun_t)(C #COMMA_REPEAT#A`i`#);
	static R call(const void* fun, C c #COMMA_REPEAT#A`i` a`i`#){ 
		return (*(fun_t*)fun)(c #COMMA_REPEAT#a`i`#); 
	}
};

#if defined(_WIN32) && defined(_M_IX86)
template<class C, class R #COMMA_REPEAT#class A`i`#>
struct cmemfun_holder<R (__stdcall *)(C #COMMA_REPEAT#A`i`#)> : public cmemfun_holder_base`n`<C, R #COMMA_REPEAT#A`i`#>{
	typedef R (__stdcall *fun_t)(C #COMMA_REPEAT#A`i`#);
	static R call(const void* fun, C c #COMMA_REPEAT#A`i` a`i`#){ 
		return (*(fun_t*)fun)(c #COMMA_REPEAT#a`i`#); 
	}
};
#endif

template<class T #COMMA_REPEAT#class A`i`#>
struct ctor_fun<T #COMMA_REPEAT#A`i`#>{
	enum{ PARAMS = `n`, PARAM_N = `n`, METHOD = 0, EXTENDABLE = 0 };
	#REPEAT#typedef ArgGetter<A`i`, `i`> ARG`i`;#
	typedef ReturnResult Result;
	typedef param_types_holder`n`<void #COMMA_REPEAT#A`i`#> fun_param_holder;
	typedef char fun_t;
	static SmartPtr<T> call(const void* fun #COMMA_REPEAT#A`i` a`i`#){
		return xnew<T>(#REPEAT_COMMA#a`i`#);
	}
};

*/


template<class R >
struct cfun_holder_base0{
	enum{ PARAMS = 0, PARAM_N = 0, METHOD = 0, EXTENDABLE = 0 };
	
	typedef ReturnResult Result;
	typedef param_types_holder0<void > fun_param_holder;
};

template<class R >
struct cfun_holder<R (*)()> : public cfun_holder_base0<R >{
	typedef R (*fun_t)();
	static R call(const void* fun ){ 
		return (*(fun_t*)fun)(); 
	}
};

#if defined(_WIN32) && defined(_M_IX86)
template<class R >
struct cfun_holder<R (__stdcall *)()> : public cfun_holder_base0<R >{
	typedef R (__stdcall *fun_t)();
	static R call(const void* fun ){ 
		return (*(fun_t*)fun)(); 
	}
};

#endif

template<class C, class R >
struct cmemfun_holder_base0{
	enum{ PARAMS = 1, PARAM_N = 0, METHOD = 1, EXTENDABLE = 0 };
	typedef ArgThisGetter<C> ARG0;
	
	typedef ReturnResult Result;
	typedef param_types_holder0<C > fun_param_holder;
};

template<class C, class R >
struct cmemfun_holder<R (C::*)()> : public cmemfun_holder_base0<C*, R >{
	typedef R (C::*fun_t)();
	static R call(const void* fun, C* self ){ 
		return (self->**(fun_t*)fun)(); 
	}
};

template<class C, class R >
struct cmemfun_holder<R (C::*)() const> : public cmemfun_holder_base0<C*, R >{
	typedef R (C::*fun_t)() const;
	static R call(const void* fun, C* self ){ 
		return (self->**(fun_t*)fun)(); 
	}
};

template<class C, class R >
struct cmemfun_holder<R (*)(C )> : public cmemfun_holder_base0<C, R >{
	typedef R (*fun_t)(C );
	static R call(const void* fun, C c ){ 
		return (*(fun_t*)fun)(c ); 
	}
};

#if defined(_WIN32) && defined(_M_IX86)
template<class C, class R >
struct cmemfun_holder<R (__stdcall *)(C )> : public cmemfun_holder_base0<C, R >{
	typedef R (__stdcall *fun_t)(C );
	static R call(const void* fun, C c ){ 
		return (*(fun_t*)fun)(c ); 
	}
};
#endif

template<class T >
struct ctor_fun<T >{
	enum{ PARAMS = 0, PARAM_N = 0, METHOD = 0, EXTENDABLE = 0 };
	
	typedef ReturnResult Result;
	typedef param_types_holder0<void > fun_param_holder;
	typedef char fun_t;
	static SmartPtr<T> call(const void* fun ){
		return xnew<T>();
	}
};



template<class R , class A0>
struct cfun_holder_base1{
	enum{ PARAMS = 1, PARAM_N = 1, METHOD = 0, EXTENDABLE = 0 };
	typedef ArgGetter<A0, 0> ARG0;
	typedef ReturnResult Result;
	typedef param_types_holder1<void , A0> fun_param_holder;
};

template<class R , class A0>
struct cfun_holder<R (*)(A0)> : public cfun_holder_base1<R , A0>{
	typedef R (*fun_t)(A0);
	static R call(const void* fun , A0 a0){ 
		return (*(fun_t*)fun)(a0); 
	}
};

#if defined(_WIN32) && defined(_M_IX86)
template<class R , class A0>
struct cfun_holder<R (__stdcall *)(A0)> : public cfun_holder_base1<R , A0>{
	typedef R (__stdcall *fun_t)(A0);
	static R call(const void* fun , A0 a0){ 
		return (*(fun_t*)fun)(a0); 
	}
};

#endif

template<class C, class R , class A0>
struct cmemfun_holder_base1{
	enum{ PARAMS = 2, PARAM_N = 1, METHOD = 1, EXTENDABLE = 0 };
	typedef ArgThisGetter<C> ARG0;
	typedef ArgGetter<A0, 0> ARG1;
	typedef ReturnResult Result;
	typedef param_types_holder1<C , A0> fun_param_holder;
};

template<class C, class R , class A0>
struct cmemfun_holder<R (C::*)(A0)> : public cmemfun_holder_base1<C*, R , A0>{
	typedef R (C::*fun_t)(A0);
	static R call(const void* fun, C* self , A0 a0){ 
		return (self->**(fun_t*)fun)(a0); 
	}
};

template<class C, class R , class A0>
struct cmemfun_holder<R (C::*)(A0) const> : public cmemfun_holder_base1<C*, R , A0>{
	typedef R (C::*fun_t)(A0) const;
	static R call(const void* fun, C* self , A0 a0){ 
		return (self->**(fun_t*)fun)(a0); 
	}
};

template<class C, class R , class A0>
struct cmemfun_holder<R (*)(C , A0)> : public cmemfun_holder_base1<C, R , A0>{
	typedef R (*fun_t)(C , A0);
	static R call(const void* fun, C c , A0 a0){ 
		return (*(fun_t*)fun)(c , a0); 
	}
};

#if defined(_WIN32) && defined(_M_IX86)
template<class C, class R , class A0>
struct cmemfun_holder<R (__stdcall *)(C , A0)> : public cmemfun_holder_base1<C, R , A0>{
	typedef R (__stdcall *fun_t)(C , A0);
	static R call(const void* fun, C c , A0 a0){ 
		return (*(fun_t*)fun)(c , a0); 
	}
};
#endif

template<class T , class A0>
struct ctor_fun<T , A0>{
	enum{ PARAMS = 1, PARAM_N = 1, METHOD = 0, EXTENDABLE = 0 };
	typedef ArgGetter<A0, 0> ARG0;
	typedef ReturnResult Result;
	typedef param_types_holder1<void , A0> fun_param_holder;
	typedef char fun_t;
	static SmartPtr<T> call(const void* fun , A0 a0){
		return xnew<T>(a0);
	}
};



template<class R , class A0, class A1>
struct cfun_holder_base2{
	enum{ PARAMS = 2, PARAM_N = 2, METHOD = 0, EXTENDABLE = 0 };
	typedef ArgGetter<A0, 0> ARG0;typedef ArgGetter<A1, 1> ARG1;
	typedef ReturnResult Result;
	typedef param_types_holder2<void , A0, A1> fun_param_holder;
};

template<class R , class A0, class A1>
struct cfun_holder<R (*)(A0, A1)> : public cfun_holder_base2<R , A0, A1>{
	typedef R (*fun_t)(A0, A1);
	static R call(const void* fun , A0 a0, A1 a1){ 
		return (*(fun_t*)fun)(a0, a1); 
	}
};

#if defined(_WIN32) && defined(_M_IX86)
template<class R , class A0, class A1>
struct cfun_holder<R (__stdcall *)(A0, A1)> : public cfun_holder_base2<R , A0, A1>{
	typedef R (__stdcall *fun_t)(A0, A1);
	static R call(const void* fun , A0 a0, A1 a1){ 
		return (*(fun_t*)fun)(a0, a1); 
	}
};

#endif

template<class C, class R , class A0, class A1>
struct cmemfun_holder_base2{
	enum{ PARAMS = 3, PARAM_N = 2, METHOD = 1, EXTENDABLE = 0 };
	typedef ArgThisGetter<C> ARG0;
	typedef ArgGetter<A0, 0> ARG1;typedef ArgGetter<A1, 1> ARG2;
	typedef ReturnResult Result;
	typedef param_types_holder2<C , A0, A1> fun_param_holder;
};

template<class C, class R , class A0, class A1>
struct cmemfun_holder<R (C::*)(A0, A1)> : public cmemfun_holder_base2<C*, R , A0, A1>{
	typedef R (C::*fun_t)(A0, A1);
	static R call(const void* fun, C* self , A0 a0, A1 a1){ 
		return (self->**(fun_t*)fun)(a0, a1); 
	}
};

template<class C, class R , class A0, class A1>
struct cmemfun_holder<R (C::*)(A0, A1) const> : public cmemfun_holder_base2<C*, R , A0, A1>{
	typedef R (C::*fun_t)(A0, A1) const;
	static R call(const void* fun, C* self , A0 a0, A1 a1){ 
		return (self->**(fun_t*)fun)(a0, a1); 
	}
};

template<class C, class R , class A0, class A1>
struct cmemfun_holder<R (*)(C , A0, A1)> : public cmemfun_holder_base2<C, R , A0, A1>{
	typedef R (*fun_t)(C , A0, A1);
	static R call(const void* fun, C c , A0 a0, A1 a1){ 
		return (*(fun_t*)fun)(c , a0, a1); 
	}
};

#if defined(_WIN32) && defined(_M_IX86)
template<class C, class R , class A0, class A1>
struct cmemfun_holder<R (__stdcall *)(C , A0, A1)> : public cmemfun_holder_base2<C, R , A0, A1>{
	typedef R (__stdcall *fun_t)(C , A0, A1);
	static R call(const void* fun, C c , A0 a0, A1 a1){ 
		return (*(fun_t*)fun)(c , a0, a1); 
	}
};
#endif

template<class T , class A0, class A1>
struct ctor_fun<T , A0, A1>{
	enum{ PARAMS = 2, PARAM_N = 2, METHOD = 0, EXTENDABLE = 0 };
	typedef ArgGetter<A0, 0> ARG0;typedef ArgGetter<A1, 1> ARG1;
	typedef ReturnResult Result;
	typedef param_types_holder2<void , A0, A1> fun_param_holder;
	typedef char fun_t;
	static SmartPtr<T> call(const void* fun , A0 a0, A1 a1){
		return xnew<T>(a0, a1);
	}
};



template<class R , class A0, class A1, class A2>
struct cfun_holder_base3{
	enum{ PARAMS = 3, PARAM_N = 3, METHOD = 0, EXTENDABLE = 0 };
	typedef ArgGetter<A0, 0> ARG0;typedef ArgGetter<A1, 1> ARG1;typedef ArgGetter<A2, 2> ARG2;
	typedef ReturnResult Result;
	typedef param_types_holder3<void , A0, A1, A2> fun_param_holder;
};

template<class R , class A0, class A1, class A2>
struct cfun_holder<R (*)(A0, A1, A2)> : public cfun_holder_base3<R , A0, A1, A2>{
	typedef R (*fun_t)(A0, A1, A2);
	static R call(const void* fun , A0 a0, A1 a1, A2 a2){ 
		return (*(fun_t*)fun)(a0, a1, a2); 
	}
};

#if defined(_WIN32) && defined(_M_IX86)
template<class R , class A0, class A1, class A2>
struct cfun_holder<R (__stdcall *)(A0, A1, A2)> : public cfun_holder_base3<R , A0, A1, A2>{
	typedef R (__stdcall *fun_t)(A0, A1, A2);
	static R call(const void* fun , A0 a0, A1 a1, A2 a2){ 
		return (*(fun_t*)fun)(a0, a1, a2); 
	}
};

#endif

template<class C, class R , class A0, class A1, class A2>
struct cmemfun_holder_base3{
	enum{ PARAMS = 4, PARAM_N = 3, METHOD = 1, EXTENDABLE = 0 };
	typedef ArgThisGetter<C> ARG0;
	typedef ArgGetter<A0, 0> ARG1;typedef ArgGetter<A1, 1> ARG2;typedef ArgGetter<A2, 2> ARG3;
	typedef ReturnResult Result;
	typedef param_types_holder3<C , A0, A1, A2> fun_param_holder;
};

template<class C, class R , class A0, class A1, class A2>
struct cmemfun_holder<R (C::*)(A0, A1, A2)> : public cmemfun_holder_base3<C*, R , A0, A1, A2>{
	typedef R (C::*fun_t)(A0, A1, A2);
	static R call(const void* fun, C* self , A0 a0, A1 a1, A2 a2){ 
		return (self->**(fun_t*)fun)(a0, a1, a2); 
	}
};

template<class C, class R , class A0, class A1, class A2>
struct cmemfun_holder<R (C::*)(A0, A1, A2) const> : public cmemfun_holder_base3<C*, R , A0, A1, A2>{
	typedef R (C::*fun_t)(A0, A1, A2) const;
	static R call(const void* fun, C* self , A0 a0, A1 a1, A2 a2){ 
		return (self->**(fun_t*)fun)(a0, a1, a2); 
	}
};

template<class C, class R , class A0, class A1, class A2>
struct cmemfun_holder<R (*)(C , A0, A1, A2)> : public cmemfun_holder_base3<C, R , A0, A1, A2>{
	typedef R (*fun_t)(C , A0, A1, A2);
	static R call(const void* fun, C c , A0 a0, A1 a1, A2 a2){ 
		return (*(fun_t*)fun)(c , a0, a1, a2); 
	}
};

#if defined(_WIN32) && defined(_M_IX86)
template<class C, class R , class A0, class A1, class A2>
struct cmemfun_holder<R (__stdcall *)(C , A0, A1, A2)> : public cmemfun_holder_base3<C, R , A0, A1, A2>{
	typedef R (__stdcall *fun_t)(C , A0, A1, A2);
	static R call(const void* fun, C c , A0 a0, A1 a1, A2 a2){ 
		return (*(fun_t*)fun)(c , a0, a1, a2); 
	}
};
#endif

template<class T , class A0, class A1, class A2>
struct ctor_fun<T , A0, A1, A2>{
	enum{ PARAMS = 3, PARAM_N = 3, METHOD = 0, EXTENDABLE = 0 };
	typedef ArgGetter<A0, 0> ARG0;typedef ArgGetter<A1, 1> ARG1;typedef ArgGetter<A2, 2> ARG2;
	typedef ReturnResult Result;
	typedef param_types_holder3<void , A0, A1, A2> fun_param_holder;
	typedef char fun_t;
	static SmartPtr<T> call(const void* fun , A0 a0, A1 a1, A2 a2){
		return xnew<T>(a0, a1, a2);
	}
};



template<class R , class A0, class A1, class A2, class A3>
struct cfun_holder_base4{
	enum{ PARAMS = 4, PARAM_N = 4, METHOD = 0, EXTENDABLE = 0 };
	typedef ArgGetter<A0, 0> ARG0;typedef ArgGetter<A1, 1> ARG1;typedef ArgGetter<A2, 2> ARG2;typedef ArgGetter<A3, 3> ARG3;
	typedef ReturnResult Result;
	typedef param_types_holder4<void , A0, A1, A2, A3> fun_param_holder;
};

template<class R , class A0, class A1, class A2, class A3>
struct cfun_holder<R (*)(A0, A1, A2, A3)> : public cfun_holder_base4<R , A0, A1, A2, A3>{
	typedef R (*fun_t)(A0, A1, A2, A3);
	static R call(const void* fun , A0 a0, A1 a1, A2 a2, A3 a3){ 
		return (*(fun_t*)fun)(a0, a1, a2, a3); 
	}
};

#if defined(_WIN32) && defined(_M_IX86)
template<class R , class A0, class A1, class A2, class A3>
struct cfun_holder<R (__stdcall *)(A0, A1, A2, A3)> : public cfun_holder_base4<R , A0, A1, A2, A3>{
	typedef R (__stdcall *fun_t)(A0, A1, A2, A3);
	static R call(const void* fun , A0 a0, A1 a1, A2 a2, A3 a3){ 
		return (*(fun_t*)fun)(a0, a1, a2, a3); 
	}
};

#endif

template<class C, class R , class A0, class A1, class A2, class A3>
struct cmemfun_holder_base4{
	enum{ PARAMS = 5, PARAM_N = 4, METHOD = 1, EXTENDABLE = 0 };
	typedef ArgThisGetter<C> ARG0;
	typedef ArgGetter<A0, 0> ARG1;typedef ArgGetter<A1, 1> ARG2;typedef ArgGetter<A2, 2> ARG3;typedef ArgGetter<A3, 3> ARG4;
	typedef ReturnResult Result;
	typedef param_types_holder4<C , A0, A1, A2, A3> fun_param_holder;
};

template<class C, class R , class A0, class A1, class A2, class A3>
struct cmemfun_holder<R (C::*)(A0, A1, A2, A3)> : public cmemfun_holder_base4<C*, R , A0, A1, A2, A3>{
	typedef R (C::*fun_t)(A0, A1, A2, A3);
	static R call(const void* fun, C* self , A0 a0, A1 a1, A2 a2, A3 a3){ 
		return (self->**(fun_t*)fun)(a0, a1, a2, a3); 
	}
};

template<class C, class R , class A0, class A1, class A2, class A3>
struct cmemfun_holder<R (C::*)(A0, A1, A2, A3) const> : public cmemfun_holder_base4<C*, R , A0, A1, A2, A3>{
	typedef R (C::*fun_t)(A0, A1, A2, A3) const;
	static R call(const void* fun, C* self , A0 a0, A1 a1, A2 a2, A3 a3){ 
		return (self->**(fun_t*)fun)(a0, a1, a2, a3); 
	}
};

template<class C, class R , class A0, class A1, class A2, class A3>
struct cmemfun_holder<R (*)(C , A0, A1, A2, A3)> : public cmemfun_holder_base4<C, R , A0, A1, A2, A3>{
	typedef R (*fun_t)(C , A0, A1, A2, A3);
	static R call(const void* fun, C c , A0 a0, A1 a1, A2 a2, A3 a3){ 
		return (*(fun_t*)fun)(c , a0, a1, a2, a3); 
	}
};

#if defined(_WIN32) && defined(_M_IX86)
template<class C, class R , class A0, class A1, class A2, class A3>
struct cmemfun_holder<R (__stdcall *)(C , A0, A1, A2, A3)> : public cmemfun_holder_base4<C, R , A0, A1, A2, A3>{
	typedef R (__stdcall *fun_t)(C , A0, A1, A2, A3);
	static R call(const void* fun, C c , A0 a0, A1 a1, A2 a2, A3 a3){ 
		return (*(fun_t*)fun)(c , a0, a1, a2, a3); 
	}
};
#endif

template<class T , class A0, class A1, class A2, class A3>
struct ctor_fun<T , A0, A1, A2, A3>{
	enum{ PARAMS = 4, PARAM_N = 4, METHOD = 0, EXTENDABLE = 0 };
	typedef ArgGetter<A0, 0> ARG0;typedef ArgGetter<A1, 1> ARG1;typedef ArgGetter<A2, 2> ARG2;typedef ArgGetter<A3, 3> ARG3;
	typedef ReturnResult Result;
	typedef param_types_holder4<void , A0, A1, A2, A3> fun_param_holder;
	typedef char fun_t;
	static SmartPtr<T> call(const void* fun , A0 a0, A1 a1, A2 a2, A3 a3){
		return xnew<T>(a0, a1, a2, a3);
	}
};



template<class R , class A0, class A1, class A2, class A3, class A4>
struct cfun_holder_base5{
	enum{ PARAMS = 5, PARAM_N = 5, METHOD = 0, EXTENDABLE = 0 };
	typedef ArgGetter<A0, 0> ARG0;typedef ArgGetter<A1, 1> ARG1;typedef ArgGetter<A2, 2> ARG2;typedef ArgGetter<A3, 3> ARG3;typedef ArgGetter<A4, 4> ARG4;
	typedef ReturnResult Result;
	typedef param_types_holder5<void , A0, A1, A2, A3, A4> fun_param_holder;
};

template<class R , class A0, class A1, class A2, class A3, class A4>
struct cfun_holder<R (*)(A0, A1, A2, A3, A4)> : public cfun_holder_base5<R , A0, A1, A2, A3, A4>{
	typedef R (*fun_t)(A0, A1, A2, A3, A4);
	static R call(const void* fun , A0 a0, A1 a1, A2 a2, A3 a3, A4 a4){ 
		return (*(fun_t*)fun)(a0, a1, a2, a3, a4); 
	}
};

#if defined(_WIN32) && defined(_M_IX86)
template<class R , class A0, class A1, class A2, class A3, class A4>
struct cfun_holder<R (__stdcall *)(A0, A1, A2, A3, A4)> : public cfun_holder_base5<R , A0, A1, A2, A3, A4>{
	typedef R (__stdcall *fun_t)(A0, A1, A2, A3, A4);
	static R call(const void* fun , A0 a0, A1 a1, A2 a2, A3 a3, A4 a4){ 
		return (*(fun_t*)fun)(a0, a1, a2, a3, a4); 
	}
};

#endif

template<class C, class R , class A0, class A1, class A2, class A3, class A4>
struct cmemfun_holder_base5{
	enum{ PARAMS = 6, PARAM_N = 5, METHOD = 1, EXTENDABLE = 0 };
	typedef ArgThisGetter<C> ARG0;
	typedef ArgGetter<A0, 0> ARG1;typedef ArgGetter<A1, 1> ARG2;typedef ArgGetter<A2, 2> ARG3;typedef ArgGetter<A3, 3> ARG4;typedef ArgGetter<A4, 4> ARG5;
	typedef ReturnResult Result;
	typedef param_types_holder5<C , A0, A1, A2, A3, A4> fun_param_holder;
};

template<class C, class R , class A0, class A1, class A2, class A3, class A4>
struct cmemfun_holder<R (C::*)(A0, A1, A2, A3, A4)> : public cmemfun_holder_base5<C*, R , A0, A1, A2, A3, A4>{
	typedef R (C::*fun_t)(A0, A1, A2, A3, A4);
	static R call(const void* fun, C* self , A0 a0, A1 a1, A2 a2, A3 a3, A4 a4){ 
		return (self->**(fun_t*)fun)(a0, a1, a2, a3, a4); 
	}
};

template<class C, class R , class A0, class A1, class A2, class A3, class A4>
struct cmemfun_holder<R (C::*)(A0, A1, A2, A3, A4) const> : public cmemfun_holder_base5<C*, R , A0, A1, A2, A3, A4>{
	typedef R (C::*fun_t)(A0, A1, A2, A3, A4) const;
	static R call(const void* fun, C* self , A0 a0, A1 a1, A2 a2, A3 a3, A4 a4){ 
		return (self->**(fun_t*)fun)(a0, a1, a2, a3, a4); 
	}
};

template<class C, class R , class A0, class A1, class A2, class A3, class A4>
struct cmemfun_holder<R (*)(C , A0, A1, A2, A3, A4)> : public cmemfun_holder_base5<C, R , A0, A1, A2, A3, A4>{
	typedef R (*fun_t)(C , A0, A1, A2, A3, A4);
	static R call(const void* fun, C c , A0 a0, A1 a1, A2 a2, A3 a3, A4 a4){ 
		return (*(fun_t*)fun)(c , a0, a1, a2, a3, a4); 
	}
};

#if defined(_WIN32) && defined(_M_IX86)
template<class C, class R , class A0, class A1, class A2, class A3, class A4>
struct cmemfun_holder<R (__stdcall *)(C , A0, A1, A2, A3, A4)> : public cmemfun_holder_base5<C, R , A0, A1, A2, A3, A4>{
	typedef R (__stdcall *fun_t)(C , A0, A1, A2, A3, A4);
	static R call(const void* fun, C c , A0 a0, A1 a1, A2 a2, A3 a3, A4 a4){ 
		return (*(fun_t*)fun)(c , a0, a1, a2, a3, a4); 
	}
};
#endif

template<class T , class A0, class A1, class A2, class A3, class A4>
struct ctor_fun<T , A0, A1, A2, A3, A4>{
	enum{ PARAMS = 5, PARAM_N = 5, METHOD = 0, EXTENDABLE = 0 };
	typedef ArgGetter<A0, 0> ARG0;typedef ArgGetter<A1, 1> ARG1;typedef ArgGetter<A2, 2> ARG2;typedef ArgGetter<A3, 3> ARG3;typedef ArgGetter<A4, 4> ARG4;
	typedef ReturnResult Result;
	typedef param_types_holder5<void , A0, A1, A2, A3, A4> fun_param_holder;
	typedef char fun_t;
	static SmartPtr<T> call(const void* fun , A0 a0, A1 a1, A2 a2, A3 a3, A4 a4){
		return xnew<T>(a0, a1, a2, a3, a4);
	}
};


//}}REPEAT}

template<class R>
struct cfun_holder<R (*)(const VMachinePtr&)>{
	enum{ PARAMS = 1, PARAM_N = 0, METHOD = 0, EXTENDABLE = 1 };
	typedef ArgGetterVM ARG0; 
	typedef ReturnNone Result;
	typedef param_types_holder0<void > fun_param_holder;
	typedef R (*fun_t)(const VMachinePtr&);
	static R call(const void* fun, const VMachinePtr& a0){ 
		return (*(fun_t*)fun)(a0); 
	}
};

template<class T>
struct ctor_fun<T , const VMachinePtr&>{
	enum{ PARAMS = 1, PARAM_N = 0, METHOD = 0, EXTENDABLE = 1 };
	typedef ArgGetterVM ARG0; 
	typedef ReturnNone Result;
	typedef param_types_holder0<void > fun_param_holder;
	typedef char fun_t;
	static SmartPtr<T> call(const void* fun, const VMachinePtr& a0){ 
		return xnew<T>(a0);
	}
};

template<class R>
struct cmemfun_holder<R (*)(const VMachinePtr&)>{
	enum{ PARAMS = 1, PARAM_N = 0, METHOD = 1, EXTENDABLE = 1 };
	typedef ArgGetterVM ARG0; 
	typedef ReturnNone Result;
	typedef param_types_holder0<void > fun_param_holder;
	typedef R (*fun_t)(const VMachinePtr&);
	static R call(const void* fun, const VMachinePtr& a0){ 
		return (*(fun_t*)fun)(a0); 
	}
};

template<class R, class C>
struct cmemfun_holder<R (*)(C, const VMachinePtr&)>{
	enum{ PARAMS = 2, PARAM_N = 0, METHOD = 1, EXTENDABLE = 1 };
	typedef ArgThisGetter<C> ARG0;
	typedef ArgGetterVM ARG1;
	typedef ReturnNone Result;
	typedef param_types_holder1<C, void > fun_param_holder;
	typedef R (*fun_t)(C, const VMachinePtr&);
	static R call(const void* fun, C self, const VMachinePtr& a0){ 
		return (*(fun_t*)fun)(self, a0); 
	}
};

template<class C>
struct cmemfun_holder<void (C::*)(const VMachinePtr&)>{
	enum{ PARAMS = 2, PARAM_N = 0, METHOD = 1, EXTENDABLE = 1 };
	typedef ArgThisGetter<C*> ARG0;
	typedef ArgGetterVM ARG1; 
	typedef ReturnNone Result;
	typedef param_types_holder1<C, void > fun_param_holder;
	typedef void (C::*fun_t)(const VMachinePtr&);
	static void call(const void* fun, C* self , const VMachinePtr& a0){ 
		return (self->**(fun_t*)fun)(a0); 
	}
};

template<class C>
struct cmemfun_holder<void (C::*)(const VMachinePtr&) const>{
	enum{ PARAMS = 2, PARAM_N = 0, METHOD = 1, EXTENDABLE = 1 };
	typedef ArgThisGetter<C*> ARG0;
	typedef ArgGetterVM ARG1; 
	typedef ReturnNone Result;
	typedef param_types_holder1<C, void > fun_param_holder;
	typedef void (C::*fun_t)(const VMachinePtr&) const;
	static void call(const void* fun, C* self , const VMachinePtr& a0){ 
		return (self->**(fun_t*)fun)(a0); 
	}
};

}

#endif
