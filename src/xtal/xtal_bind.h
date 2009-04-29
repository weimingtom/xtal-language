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
struct cfun_holder<R (*)(#REPEAT_COMMA#A`i`#)>{
	enum{ PARAMS = `n`, PARAM_N = `n`, METHOD = 0, EXTENDABLE = 0 };
	typedef R (*fun_t)(#REPEAT_COMMA#A`i`#);
	#REPEAT#typedef ArgGetter<A`i`, `i`> ARG`i`;#
	typedef ReturnResult Result;
	typedef param_types_holder`n`<void #COMMA_REPEAT#A`i`#> fun_param_holder;
	fun_t fun;
	cfun_holder(const fun_t& f):fun(f){}
	R operator()(#REPEAT_COMMA#A`i` a`i`#){ 
		return (*fun)(#REPEAT_COMMA#a`i`#); 
	}
};

#if defined(_WIN32) && defined(_M_IX86)
template<class R #COMMA_REPEAT#class A`i`#>
struct cfun_holder<R (__stdcall *)(#REPEAT_COMMA#A`i`#)>{
	enum{ PARAMS = `n`, PARAM_N = `n`, METHOD = 0, EXTENDABLE = 0 };
	typedef R (__stdcall *fun_t)(#REPEAT_COMMA#A`i`#);
	#REPEAT#typedef ArgGetter<A`i`, `i`> ARG`i`;#
	typedef ReturnResult Result;
	typedef param_types_holder`n`<void #COMMA_REPEAT#A`i`#> fun_param_holder;
	fun_t fun;
	cfun_holder(const fun_t& f):fun(f){}
	R operator()(#REPEAT_COMMA#A`i` a`i`#){ 
		return (*fun)(#REPEAT_COMMA#a`i`#); 
	}
};

#endif

template<class C, class R #COMMA_REPEAT#class A`i`#>
struct cmemfun_holder<R (C::*)(#REPEAT_COMMA#A`i`#)>{
	enum{ PARAMS = `n+1`, PARAM_N = `n`, METHOD = 1, EXTENDABLE = 0 };
	typedef R (C::*fun_t)(#REPEAT_COMMA#A`i`#);
	typedef ArgThisGetter<C*> ARG0;
	#REPEAT#typedef ArgGetter<A`i`, `i`> ARG`i+1`;#
	typedef ReturnResult Result;
	typedef param_types_holder`n`<C #COMMA_REPEAT#A`i`#> fun_param_holder;
	fun_t fun;
	cmemfun_holder(const fun_t& f):fun(f){}
	R operator()(C* self #COMMA_REPEAT#A`i` a`i`#){ 
		return (self->*fun)(#REPEAT_COMMA#a`i`#); 
	}
};

template<class C, class R #COMMA_REPEAT#class A`i`#>
struct cmemfun_holder<R (C::*)(#REPEAT_COMMA#A`i`#) const>{
	enum{ PARAMS = `n+1`, PARAM_N = `n`, METHOD = 1, EXTENDABLE = 0 };
	typedef R (C::*fun_t)(#REPEAT_COMMA#A`i`#) const;
	typedef ArgThisGetter<C*> ARG0;
	#REPEAT#typedef ArgGetter<A`i`, `i`> ARG`i+1`;#
	typedef ReturnResult Result;
	typedef param_types_holder`n`<C #COMMA_REPEAT#A`i`#> fun_param_holder;
	fun_t fun;
	cmemfun_holder(const fun_t& f):fun(f){}
	R operator()(C* self #COMMA_REPEAT#A`i` a`i`#){ 
		return (self->*fun)(#REPEAT_COMMA#a`i`#); 
	}
};

template<class C, class R #COMMA_REPEAT#class A`i`#>
struct cmemfun_holder<R (*)(C #COMMA_REPEAT#A`i`#)>{
	enum{ PARAMS = `n+1`, PARAM_N = `n`, METHOD = 1, EXTENDABLE = 0 };
	typedef R (*fun_t)(C #COMMA_REPEAT#A`i`#);
	typedef ArgThisGetter<C> ARG0;
	#REPEAT#typedef ArgGetter<A`i`, `i`> ARG`i+1`;#
	typedef ReturnResult Result;
	typedef param_types_holder`n`<C #COMMA_REPEAT#A`i`#> fun_param_holder;
	fun_t fun;
	cmemfun_holder(const fun_t& f):fun(f){}
	R operator()(C c #COMMA_REPEAT#A`i` a`i`#){ 
		return (*fun)(c #COMMA_REPEAT#a`i`#); 
	}
};

#if defined(_WIN32) && defined(_M_IX86)
template<class C, class R #COMMA_REPEAT#class A`i`#>
struct cmemfun_holder<R (__stdcall *)(C #COMMA_REPEAT#A`i`#)>{
	enum{ PARAMS = `n+1`, PARAM_N = `n`, METHOD = 1, EXTENDABLE = 0 };
	typedef R (__stdcall *fun_t)(C #COMMA_REPEAT#A`i`#);
	typedef ArgThisGetter<C> ARG0;
	#REPEAT#typedef ArgGetter<A`i`, `i`> ARG`i+1`;#
	typedef ReturnResult Result;
	typedef param_types_holder`n`<C #COMMA_REPEAT#A`i`#> fun_param_holder;
	fun_t fun;
	cmemfun_holder(const fun_t& f):fun(f){}
	R operator()(C c #COMMA_REPEAT#A`i` a`i`#){ 
		return (*fun)(c #COMMA_REPEAT#a`i`#); 
	}
};
#endif

template<class T #COMMA_REPEAT#class A`i`#>
struct ctor_fun<T #COMMA_REPEAT#A`i`#>{
	enum{ PARAMS = `n`, PARAM_N = `n`, METHOD = 0, EXTENDABLE = 0 };
	#REPEAT#typedef ArgGetter<A`i`, `i`> ARG`i`;#
	typedef ReturnResult Result;
	typedef param_types_holder`n`<void #COMMA_REPEAT#A`i`#> fun_param_holder;
	ctor_fun(){}
	SmartPtr<T> operator()(#REPEAT_COMMA#A`i` a`i`#){
		return xnew<T>(#REPEAT_COMMA#a`i`#);
	}
};
*/


template<class R >
struct cfun_holder<R (*)()>{
	enum{ PARAMS = 0, PARAM_N = 0, METHOD = 0, EXTENDABLE = 0 };
	typedef R (*fun_t)();
	
	typedef ReturnResult Result;
	typedef param_types_holder0<void > fun_param_holder;
	fun_t fun;
	cfun_holder(const fun_t& f):fun(f){}
	R operator()(){ 
		return (*fun)(); 
	}
};

#if defined(_WIN32) && defined(_M_IX86)
template<class R >
struct cfun_holder<R (__stdcall *)()>{
	enum{ PARAMS = 0, PARAM_N = 0, METHOD = 0, EXTENDABLE = 0 };
	typedef R (__stdcall *fun_t)();
	
	typedef ReturnResult Result;
	typedef param_types_holder0<void > fun_param_holder;
	fun_t fun;
	cfun_holder(const fun_t& f):fun(f){}
	R operator()(){ 
		return (*fun)(); 
	}
};

#endif

template<class C, class R >
struct cmemfun_holder<R (C::*)()>{
	enum{ PARAMS = 1, PARAM_N = 0, METHOD = 1, EXTENDABLE = 0 };
	typedef R (C::*fun_t)();
	typedef ArgThisGetter<C*> ARG0;
	
	typedef ReturnResult Result;
	typedef param_types_holder0<C > fun_param_holder;
	fun_t fun;
	cmemfun_holder(const fun_t& f):fun(f){}
	R operator()(C* self ){ 
		return (self->*fun)(); 
	}
};

template<class C, class R >
struct cmemfun_holder<R (C::*)() const>{
	enum{ PARAMS = 1, PARAM_N = 0, METHOD = 1, EXTENDABLE = 0 };
	typedef R (C::*fun_t)() const;
	typedef ArgThisGetter<C*> ARG0;
	
	typedef ReturnResult Result;
	typedef param_types_holder0<C > fun_param_holder;
	fun_t fun;
	cmemfun_holder(const fun_t& f):fun(f){}
	R operator()(C* self ){ 
		return (self->*fun)(); 
	}
};

template<class C, class R >
struct cmemfun_holder<R (*)(C )>{
	enum{ PARAMS = 1, PARAM_N = 0, METHOD = 1, EXTENDABLE = 0 };
	typedef R (*fun_t)(C );
	typedef ArgThisGetter<C> ARG0;
	
	typedef ReturnResult Result;
	typedef param_types_holder0<C > fun_param_holder;
	fun_t fun;
	cmemfun_holder(const fun_t& f):fun(f){}
	R operator()(C c ){ 
		return (*fun)(c ); 
	}
};

#if defined(_WIN32) && defined(_M_IX86)
template<class C, class R >
struct cmemfun_holder<R (__stdcall *)(C )>{
	enum{ PARAMS = 1, PARAM_N = 0, METHOD = 1, EXTENDABLE = 0 };
	typedef R (__stdcall *fun_t)(C );
	typedef ArgThisGetter<C> ARG0;
	
	typedef ReturnResult Result;
	typedef param_types_holder0<C > fun_param_holder;
	fun_t fun;
	cmemfun_holder(const fun_t& f):fun(f){}
	R operator()(C c ){ 
		return (*fun)(c ); 
	}
};
#endif

template<class T >
struct ctor_fun<T >{
	enum{ PARAMS = 0, PARAM_N = 0, METHOD = 0, EXTENDABLE = 0 };
	
	typedef ReturnResult Result;
	typedef param_types_holder0<void > fun_param_holder;
	ctor_fun(){}
	SmartPtr<T> operator()(){
		return xnew<T>();
	}
};


template<class R , class A0>
struct cfun_holder<R (*)(A0)>{
	enum{ PARAMS = 1, PARAM_N = 1, METHOD = 0, EXTENDABLE = 0 };
	typedef R (*fun_t)(A0);
	typedef ArgGetter<A0, 0> ARG0;
	typedef ReturnResult Result;
	typedef param_types_holder1<void , A0> fun_param_holder;
	fun_t fun;
	cfun_holder(const fun_t& f):fun(f){}
	R operator()(A0 a0){ 
		return (*fun)(a0); 
	}
};

#if defined(_WIN32) && defined(_M_IX86)
template<class R , class A0>
struct cfun_holder<R (__stdcall *)(A0)>{
	enum{ PARAMS = 1, PARAM_N = 1, METHOD = 0, EXTENDABLE = 0 };
	typedef R (__stdcall *fun_t)(A0);
	typedef ArgGetter<A0, 0> ARG0;
	typedef ReturnResult Result;
	typedef param_types_holder1<void , A0> fun_param_holder;
	fun_t fun;
	cfun_holder(const fun_t& f):fun(f){}
	R operator()(A0 a0){ 
		return (*fun)(a0); 
	}
};

#endif

template<class C, class R , class A0>
struct cmemfun_holder<R (C::*)(A0)>{
	enum{ PARAMS = 2, PARAM_N = 1, METHOD = 1, EXTENDABLE = 0 };
	typedef R (C::*fun_t)(A0);
	typedef ArgThisGetter<C*> ARG0;
	typedef ArgGetter<A0, 0> ARG1;
	typedef ReturnResult Result;
	typedef param_types_holder1<C , A0> fun_param_holder;
	fun_t fun;
	cmemfun_holder(const fun_t& f):fun(f){}
	R operator()(C* self , A0 a0){ 
		return (self->*fun)(a0); 
	}
};

template<class C, class R , class A0>
struct cmemfun_holder<R (C::*)(A0) const>{
	enum{ PARAMS = 2, PARAM_N = 1, METHOD = 1, EXTENDABLE = 0 };
	typedef R (C::*fun_t)(A0) const;
	typedef ArgThisGetter<C*> ARG0;
	typedef ArgGetter<A0, 0> ARG1;
	typedef ReturnResult Result;
	typedef param_types_holder1<C , A0> fun_param_holder;
	fun_t fun;
	cmemfun_holder(const fun_t& f):fun(f){}
	R operator()(C* self , A0 a0){ 
		return (self->*fun)(a0); 
	}
};

template<class C, class R , class A0>
struct cmemfun_holder<R (*)(C , A0)>{
	enum{ PARAMS = 2, PARAM_N = 1, METHOD = 1, EXTENDABLE = 0 };
	typedef R (*fun_t)(C , A0);
	typedef ArgThisGetter<C> ARG0;
	typedef ArgGetter<A0, 0> ARG1;
	typedef ReturnResult Result;
	typedef param_types_holder1<C , A0> fun_param_holder;
	fun_t fun;
	cmemfun_holder(const fun_t& f):fun(f){}
	R operator()(C c , A0 a0){ 
		return (*fun)(c , a0); 
	}
};

#if defined(_WIN32) && defined(_M_IX86)
template<class C, class R , class A0>
struct cmemfun_holder<R (__stdcall *)(C , A0)>{
	enum{ PARAMS = 2, PARAM_N = 1, METHOD = 1, EXTENDABLE = 0 };
	typedef R (__stdcall *fun_t)(C , A0);
	typedef ArgThisGetter<C> ARG0;
	typedef ArgGetter<A0, 0> ARG1;
	typedef ReturnResult Result;
	typedef param_types_holder1<C , A0> fun_param_holder;
	fun_t fun;
	cmemfun_holder(const fun_t& f):fun(f){}
	R operator()(C c , A0 a0){ 
		return (*fun)(c , a0); 
	}
};
#endif

template<class T , class A0>
struct ctor_fun<T , A0>{
	enum{ PARAMS = 1, PARAM_N = 1, METHOD = 0, EXTENDABLE = 0 };
	typedef ArgGetter<A0, 0> ARG0;
	typedef ReturnResult Result;
	typedef param_types_holder1<void , A0> fun_param_holder;
	ctor_fun(){}
	SmartPtr<T> operator()(A0 a0){
		return xnew<T>(a0);
	}
};


template<class R , class A0, class A1>
struct cfun_holder<R (*)(A0, A1)>{
	enum{ PARAMS = 2, PARAM_N = 2, METHOD = 0, EXTENDABLE = 0 };
	typedef R (*fun_t)(A0, A1);
	typedef ArgGetter<A0, 0> ARG0;typedef ArgGetter<A1, 1> ARG1;
	typedef ReturnResult Result;
	typedef param_types_holder2<void , A0, A1> fun_param_holder;
	fun_t fun;
	cfun_holder(const fun_t& f):fun(f){}
	R operator()(A0 a0, A1 a1){ 
		return (*fun)(a0, a1); 
	}
};

#if defined(_WIN32) && defined(_M_IX86)
template<class R , class A0, class A1>
struct cfun_holder<R (__stdcall *)(A0, A1)>{
	enum{ PARAMS = 2, PARAM_N = 2, METHOD = 0, EXTENDABLE = 0 };
	typedef R (__stdcall *fun_t)(A0, A1);
	typedef ArgGetter<A0, 0> ARG0;typedef ArgGetter<A1, 1> ARG1;
	typedef ReturnResult Result;
	typedef param_types_holder2<void , A0, A1> fun_param_holder;
	fun_t fun;
	cfun_holder(const fun_t& f):fun(f){}
	R operator()(A0 a0, A1 a1){ 
		return (*fun)(a0, a1); 
	}
};

#endif

template<class C, class R , class A0, class A1>
struct cmemfun_holder<R (C::*)(A0, A1)>{
	enum{ PARAMS = 3, PARAM_N = 2, METHOD = 1, EXTENDABLE = 0 };
	typedef R (C::*fun_t)(A0, A1);
	typedef ArgThisGetter<C*> ARG0;
	typedef ArgGetter<A0, 0> ARG1;typedef ArgGetter<A1, 1> ARG2;
	typedef ReturnResult Result;
	typedef param_types_holder2<C , A0, A1> fun_param_holder;
	fun_t fun;
	cmemfun_holder(const fun_t& f):fun(f){}
	R operator()(C* self , A0 a0, A1 a1){ 
		return (self->*fun)(a0, a1); 
	}
};

template<class C, class R , class A0, class A1>
struct cmemfun_holder<R (C::*)(A0, A1) const>{
	enum{ PARAMS = 3, PARAM_N = 2, METHOD = 1, EXTENDABLE = 0 };
	typedef R (C::*fun_t)(A0, A1) const;
	typedef ArgThisGetter<C*> ARG0;
	typedef ArgGetter<A0, 0> ARG1;typedef ArgGetter<A1, 1> ARG2;
	typedef ReturnResult Result;
	typedef param_types_holder2<C , A0, A1> fun_param_holder;
	fun_t fun;
	cmemfun_holder(const fun_t& f):fun(f){}
	R operator()(C* self , A0 a0, A1 a1){ 
		return (self->*fun)(a0, a1); 
	}
};

template<class C, class R , class A0, class A1>
struct cmemfun_holder<R (*)(C , A0, A1)>{
	enum{ PARAMS = 3, PARAM_N = 2, METHOD = 1, EXTENDABLE = 0 };
	typedef R (*fun_t)(C , A0, A1);
	typedef ArgThisGetter<C> ARG0;
	typedef ArgGetter<A0, 0> ARG1;typedef ArgGetter<A1, 1> ARG2;
	typedef ReturnResult Result;
	typedef param_types_holder2<C , A0, A1> fun_param_holder;
	fun_t fun;
	cmemfun_holder(const fun_t& f):fun(f){}
	R operator()(C c , A0 a0, A1 a1){ 
		return (*fun)(c , a0, a1); 
	}
};

#if defined(_WIN32) && defined(_M_IX86)
template<class C, class R , class A0, class A1>
struct cmemfun_holder<R (__stdcall *)(C , A0, A1)>{
	enum{ PARAMS = 3, PARAM_N = 2, METHOD = 1, EXTENDABLE = 0 };
	typedef R (__stdcall *fun_t)(C , A0, A1);
	typedef ArgThisGetter<C> ARG0;
	typedef ArgGetter<A0, 0> ARG1;typedef ArgGetter<A1, 1> ARG2;
	typedef ReturnResult Result;
	typedef param_types_holder2<C , A0, A1> fun_param_holder;
	fun_t fun;
	cmemfun_holder(const fun_t& f):fun(f){}
	R operator()(C c , A0 a0, A1 a1){ 
		return (*fun)(c , a0, a1); 
	}
};
#endif

template<class T , class A0, class A1>
struct ctor_fun<T , A0, A1>{
	enum{ PARAMS = 2, PARAM_N = 2, METHOD = 0, EXTENDABLE = 0 };
	typedef ArgGetter<A0, 0> ARG0;typedef ArgGetter<A1, 1> ARG1;
	typedef ReturnResult Result;
	typedef param_types_holder2<void , A0, A1> fun_param_holder;
	ctor_fun(){}
	SmartPtr<T> operator()(A0 a0, A1 a1){
		return xnew<T>(a0, a1);
	}
};


template<class R , class A0, class A1, class A2>
struct cfun_holder<R (*)(A0, A1, A2)>{
	enum{ PARAMS = 3, PARAM_N = 3, METHOD = 0, EXTENDABLE = 0 };
	typedef R (*fun_t)(A0, A1, A2);
	typedef ArgGetter<A0, 0> ARG0;typedef ArgGetter<A1, 1> ARG1;typedef ArgGetter<A2, 2> ARG2;
	typedef ReturnResult Result;
	typedef param_types_holder3<void , A0, A1, A2> fun_param_holder;
	fun_t fun;
	cfun_holder(const fun_t& f):fun(f){}
	R operator()(A0 a0, A1 a1, A2 a2){ 
		return (*fun)(a0, a1, a2); 
	}
};

#if defined(_WIN32) && defined(_M_IX86)
template<class R , class A0, class A1, class A2>
struct cfun_holder<R (__stdcall *)(A0, A1, A2)>{
	enum{ PARAMS = 3, PARAM_N = 3, METHOD = 0, EXTENDABLE = 0 };
	typedef R (__stdcall *fun_t)(A0, A1, A2);
	typedef ArgGetter<A0, 0> ARG0;typedef ArgGetter<A1, 1> ARG1;typedef ArgGetter<A2, 2> ARG2;
	typedef ReturnResult Result;
	typedef param_types_holder3<void , A0, A1, A2> fun_param_holder;
	fun_t fun;
	cfun_holder(const fun_t& f):fun(f){}
	R operator()(A0 a0, A1 a1, A2 a2){ 
		return (*fun)(a0, a1, a2); 
	}
};

#endif

template<class C, class R , class A0, class A1, class A2>
struct cmemfun_holder<R (C::*)(A0, A1, A2)>{
	enum{ PARAMS = 4, PARAM_N = 3, METHOD = 1, EXTENDABLE = 0 };
	typedef R (C::*fun_t)(A0, A1, A2);
	typedef ArgThisGetter<C*> ARG0;
	typedef ArgGetter<A0, 0> ARG1;typedef ArgGetter<A1, 1> ARG2;typedef ArgGetter<A2, 2> ARG3;
	typedef ReturnResult Result;
	typedef param_types_holder3<C , A0, A1, A2> fun_param_holder;
	fun_t fun;
	cmemfun_holder(const fun_t& f):fun(f){}
	R operator()(C* self , A0 a0, A1 a1, A2 a2){ 
		return (self->*fun)(a0, a1, a2); 
	}
};

template<class C, class R , class A0, class A1, class A2>
struct cmemfun_holder<R (C::*)(A0, A1, A2) const>{
	enum{ PARAMS = 4, PARAM_N = 3, METHOD = 1, EXTENDABLE = 0 };
	typedef R (C::*fun_t)(A0, A1, A2) const;
	typedef ArgThisGetter<C*> ARG0;
	typedef ArgGetter<A0, 0> ARG1;typedef ArgGetter<A1, 1> ARG2;typedef ArgGetter<A2, 2> ARG3;
	typedef ReturnResult Result;
	typedef param_types_holder3<C , A0, A1, A2> fun_param_holder;
	fun_t fun;
	cmemfun_holder(const fun_t& f):fun(f){}
	R operator()(C* self , A0 a0, A1 a1, A2 a2){ 
		return (self->*fun)(a0, a1, a2); 
	}
};

template<class C, class R , class A0, class A1, class A2>
struct cmemfun_holder<R (*)(C , A0, A1, A2)>{
	enum{ PARAMS = 4, PARAM_N = 3, METHOD = 1, EXTENDABLE = 0 };
	typedef R (*fun_t)(C , A0, A1, A2);
	typedef ArgThisGetter<C> ARG0;
	typedef ArgGetter<A0, 0> ARG1;typedef ArgGetter<A1, 1> ARG2;typedef ArgGetter<A2, 2> ARG3;
	typedef ReturnResult Result;
	typedef param_types_holder3<C , A0, A1, A2> fun_param_holder;
	fun_t fun;
	cmemfun_holder(const fun_t& f):fun(f){}
	R operator()(C c , A0 a0, A1 a1, A2 a2){ 
		return (*fun)(c , a0, a1, a2); 
	}
};

#if defined(_WIN32) && defined(_M_IX86)
template<class C, class R , class A0, class A1, class A2>
struct cmemfun_holder<R (__stdcall *)(C , A0, A1, A2)>{
	enum{ PARAMS = 4, PARAM_N = 3, METHOD = 1, EXTENDABLE = 0 };
	typedef R (__stdcall *fun_t)(C , A0, A1, A2);
	typedef ArgThisGetter<C> ARG0;
	typedef ArgGetter<A0, 0> ARG1;typedef ArgGetter<A1, 1> ARG2;typedef ArgGetter<A2, 2> ARG3;
	typedef ReturnResult Result;
	typedef param_types_holder3<C , A0, A1, A2> fun_param_holder;
	fun_t fun;
	cmemfun_holder(const fun_t& f):fun(f){}
	R operator()(C c , A0 a0, A1 a1, A2 a2){ 
		return (*fun)(c , a0, a1, a2); 
	}
};
#endif

template<class T , class A0, class A1, class A2>
struct ctor_fun<T , A0, A1, A2>{
	enum{ PARAMS = 3, PARAM_N = 3, METHOD = 0, EXTENDABLE = 0 };
	typedef ArgGetter<A0, 0> ARG0;typedef ArgGetter<A1, 1> ARG1;typedef ArgGetter<A2, 2> ARG2;
	typedef ReturnResult Result;
	typedef param_types_holder3<void , A0, A1, A2> fun_param_holder;
	ctor_fun(){}
	SmartPtr<T> operator()(A0 a0, A1 a1, A2 a2){
		return xnew<T>(a0, a1, a2);
	}
};


template<class R , class A0, class A1, class A2, class A3>
struct cfun_holder<R (*)(A0, A1, A2, A3)>{
	enum{ PARAMS = 4, PARAM_N = 4, METHOD = 0, EXTENDABLE = 0 };
	typedef R (*fun_t)(A0, A1, A2, A3);
	typedef ArgGetter<A0, 0> ARG0;typedef ArgGetter<A1, 1> ARG1;typedef ArgGetter<A2, 2> ARG2;typedef ArgGetter<A3, 3> ARG3;
	typedef ReturnResult Result;
	typedef param_types_holder4<void , A0, A1, A2, A3> fun_param_holder;
	fun_t fun;
	cfun_holder(const fun_t& f):fun(f){}
	R operator()(A0 a0, A1 a1, A2 a2, A3 a3){ 
		return (*fun)(a0, a1, a2, a3); 
	}
};

#if defined(_WIN32) && defined(_M_IX86)
template<class R , class A0, class A1, class A2, class A3>
struct cfun_holder<R (__stdcall *)(A0, A1, A2, A3)>{
	enum{ PARAMS = 4, PARAM_N = 4, METHOD = 0, EXTENDABLE = 0 };
	typedef R (__stdcall *fun_t)(A0, A1, A2, A3);
	typedef ArgGetter<A0, 0> ARG0;typedef ArgGetter<A1, 1> ARG1;typedef ArgGetter<A2, 2> ARG2;typedef ArgGetter<A3, 3> ARG3;
	typedef ReturnResult Result;
	typedef param_types_holder4<void , A0, A1, A2, A3> fun_param_holder;
	fun_t fun;
	cfun_holder(const fun_t& f):fun(f){}
	R operator()(A0 a0, A1 a1, A2 a2, A3 a3){ 
		return (*fun)(a0, a1, a2, a3); 
	}
};

#endif

template<class C, class R , class A0, class A1, class A2, class A3>
struct cmemfun_holder<R (C::*)(A0, A1, A2, A3)>{
	enum{ PARAMS = 5, PARAM_N = 4, METHOD = 1, EXTENDABLE = 0 };
	typedef R (C::*fun_t)(A0, A1, A2, A3);
	typedef ArgThisGetter<C*> ARG0;
	typedef ArgGetter<A0, 0> ARG1;typedef ArgGetter<A1, 1> ARG2;typedef ArgGetter<A2, 2> ARG3;typedef ArgGetter<A3, 3> ARG4;
	typedef ReturnResult Result;
	typedef param_types_holder4<C , A0, A1, A2, A3> fun_param_holder;
	fun_t fun;
	cmemfun_holder(const fun_t& f):fun(f){}
	R operator()(C* self , A0 a0, A1 a1, A2 a2, A3 a3){ 
		return (self->*fun)(a0, a1, a2, a3); 
	}
};

template<class C, class R , class A0, class A1, class A2, class A3>
struct cmemfun_holder<R (C::*)(A0, A1, A2, A3) const>{
	enum{ PARAMS = 5, PARAM_N = 4, METHOD = 1, EXTENDABLE = 0 };
	typedef R (C::*fun_t)(A0, A1, A2, A3) const;
	typedef ArgThisGetter<C*> ARG0;
	typedef ArgGetter<A0, 0> ARG1;typedef ArgGetter<A1, 1> ARG2;typedef ArgGetter<A2, 2> ARG3;typedef ArgGetter<A3, 3> ARG4;
	typedef ReturnResult Result;
	typedef param_types_holder4<C , A0, A1, A2, A3> fun_param_holder;
	fun_t fun;
	cmemfun_holder(const fun_t& f):fun(f){}
	R operator()(C* self , A0 a0, A1 a1, A2 a2, A3 a3){ 
		return (self->*fun)(a0, a1, a2, a3); 
	}
};

template<class C, class R , class A0, class A1, class A2, class A3>
struct cmemfun_holder<R (*)(C , A0, A1, A2, A3)>{
	enum{ PARAMS = 5, PARAM_N = 4, METHOD = 1, EXTENDABLE = 0 };
	typedef R (*fun_t)(C , A0, A1, A2, A3);
	typedef ArgThisGetter<C> ARG0;
	typedef ArgGetter<A0, 0> ARG1;typedef ArgGetter<A1, 1> ARG2;typedef ArgGetter<A2, 2> ARG3;typedef ArgGetter<A3, 3> ARG4;
	typedef ReturnResult Result;
	typedef param_types_holder4<C , A0, A1, A2, A3> fun_param_holder;
	fun_t fun;
	cmemfun_holder(const fun_t& f):fun(f){}
	R operator()(C c , A0 a0, A1 a1, A2 a2, A3 a3){ 
		return (*fun)(c , a0, a1, a2, a3); 
	}
};

#if defined(_WIN32) && defined(_M_IX86)
template<class C, class R , class A0, class A1, class A2, class A3>
struct cmemfun_holder<R (__stdcall *)(C , A0, A1, A2, A3)>{
	enum{ PARAMS = 5, PARAM_N = 4, METHOD = 1, EXTENDABLE = 0 };
	typedef R (__stdcall *fun_t)(C , A0, A1, A2, A3);
	typedef ArgThisGetter<C> ARG0;
	typedef ArgGetter<A0, 0> ARG1;typedef ArgGetter<A1, 1> ARG2;typedef ArgGetter<A2, 2> ARG3;typedef ArgGetter<A3, 3> ARG4;
	typedef ReturnResult Result;
	typedef param_types_holder4<C , A0, A1, A2, A3> fun_param_holder;
	fun_t fun;
	cmemfun_holder(const fun_t& f):fun(f){}
	R operator()(C c , A0 a0, A1 a1, A2 a2, A3 a3){ 
		return (*fun)(c , a0, a1, a2, a3); 
	}
};
#endif

template<class T , class A0, class A1, class A2, class A3>
struct ctor_fun<T , A0, A1, A2, A3>{
	enum{ PARAMS = 4, PARAM_N = 4, METHOD = 0, EXTENDABLE = 0 };
	typedef ArgGetter<A0, 0> ARG0;typedef ArgGetter<A1, 1> ARG1;typedef ArgGetter<A2, 2> ARG2;typedef ArgGetter<A3, 3> ARG3;
	typedef ReturnResult Result;
	typedef param_types_holder4<void , A0, A1, A2, A3> fun_param_holder;
	ctor_fun(){}
	SmartPtr<T> operator()(A0 a0, A1 a1, A2 a2, A3 a3){
		return xnew<T>(a0, a1, a2, a3);
	}
};


template<class R , class A0, class A1, class A2, class A3, class A4>
struct cfun_holder<R (*)(A0, A1, A2, A3, A4)>{
	enum{ PARAMS = 5, PARAM_N = 5, METHOD = 0, EXTENDABLE = 0 };
	typedef R (*fun_t)(A0, A1, A2, A3, A4);
	typedef ArgGetter<A0, 0> ARG0;typedef ArgGetter<A1, 1> ARG1;typedef ArgGetter<A2, 2> ARG2;typedef ArgGetter<A3, 3> ARG3;typedef ArgGetter<A4, 4> ARG4;
	typedef ReturnResult Result;
	typedef param_types_holder5<void , A0, A1, A2, A3, A4> fun_param_holder;
	fun_t fun;
	cfun_holder(const fun_t& f):fun(f){}
	R operator()(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4){ 
		return (*fun)(a0, a1, a2, a3, a4); 
	}
};

#if defined(_WIN32) && defined(_M_IX86)
template<class R , class A0, class A1, class A2, class A3, class A4>
struct cfun_holder<R (__stdcall *)(A0, A1, A2, A3, A4)>{
	enum{ PARAMS = 5, PARAM_N = 5, METHOD = 0, EXTENDABLE = 0 };
	typedef R (__stdcall *fun_t)(A0, A1, A2, A3, A4);
	typedef ArgGetter<A0, 0> ARG0;typedef ArgGetter<A1, 1> ARG1;typedef ArgGetter<A2, 2> ARG2;typedef ArgGetter<A3, 3> ARG3;typedef ArgGetter<A4, 4> ARG4;
	typedef ReturnResult Result;
	typedef param_types_holder5<void , A0, A1, A2, A3, A4> fun_param_holder;
	fun_t fun;
	cfun_holder(const fun_t& f):fun(f){}
	R operator()(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4){ 
		return (*fun)(a0, a1, a2, a3, a4); 
	}
};

#endif

template<class C, class R , class A0, class A1, class A2, class A3, class A4>
struct cmemfun_holder<R (C::*)(A0, A1, A2, A3, A4)>{
	enum{ PARAMS = 6, PARAM_N = 5, METHOD = 1, EXTENDABLE = 0 };
	typedef R (C::*fun_t)(A0, A1, A2, A3, A4);
	typedef ArgThisGetter<C*> ARG0;
	typedef ArgGetter<A0, 0> ARG1;typedef ArgGetter<A1, 1> ARG2;typedef ArgGetter<A2, 2> ARG3;typedef ArgGetter<A3, 3> ARG4;typedef ArgGetter<A4, 4> ARG5;
	typedef ReturnResult Result;
	typedef param_types_holder5<C , A0, A1, A2, A3, A4> fun_param_holder;
	fun_t fun;
	cmemfun_holder(const fun_t& f):fun(f){}
	R operator()(C* self , A0 a0, A1 a1, A2 a2, A3 a3, A4 a4){ 
		return (self->*fun)(a0, a1, a2, a3, a4); 
	}
};

template<class C, class R , class A0, class A1, class A2, class A3, class A4>
struct cmemfun_holder<R (C::*)(A0, A1, A2, A3, A4) const>{
	enum{ PARAMS = 6, PARAM_N = 5, METHOD = 1, EXTENDABLE = 0 };
	typedef R (C::*fun_t)(A0, A1, A2, A3, A4) const;
	typedef ArgThisGetter<C*> ARG0;
	typedef ArgGetter<A0, 0> ARG1;typedef ArgGetter<A1, 1> ARG2;typedef ArgGetter<A2, 2> ARG3;typedef ArgGetter<A3, 3> ARG4;typedef ArgGetter<A4, 4> ARG5;
	typedef ReturnResult Result;
	typedef param_types_holder5<C , A0, A1, A2, A3, A4> fun_param_holder;
	fun_t fun;
	cmemfun_holder(const fun_t& f):fun(f){}
	R operator()(C* self , A0 a0, A1 a1, A2 a2, A3 a3, A4 a4){ 
		return (self->*fun)(a0, a1, a2, a3, a4); 
	}
};

template<class C, class R , class A0, class A1, class A2, class A3, class A4>
struct cmemfun_holder<R (*)(C , A0, A1, A2, A3, A4)>{
	enum{ PARAMS = 6, PARAM_N = 5, METHOD = 1, EXTENDABLE = 0 };
	typedef R (*fun_t)(C , A0, A1, A2, A3, A4);
	typedef ArgThisGetter<C> ARG0;
	typedef ArgGetter<A0, 0> ARG1;typedef ArgGetter<A1, 1> ARG2;typedef ArgGetter<A2, 2> ARG3;typedef ArgGetter<A3, 3> ARG4;typedef ArgGetter<A4, 4> ARG5;
	typedef ReturnResult Result;
	typedef param_types_holder5<C , A0, A1, A2, A3, A4> fun_param_holder;
	fun_t fun;
	cmemfun_holder(const fun_t& f):fun(f){}
	R operator()(C c , A0 a0, A1 a1, A2 a2, A3 a3, A4 a4){ 
		return (*fun)(c , a0, a1, a2, a3, a4); 
	}
};

#if defined(_WIN32) && defined(_M_IX86)
template<class C, class R , class A0, class A1, class A2, class A3, class A4>
struct cmemfun_holder<R (__stdcall *)(C , A0, A1, A2, A3, A4)>{
	enum{ PARAMS = 6, PARAM_N = 5, METHOD = 1, EXTENDABLE = 0 };
	typedef R (__stdcall *fun_t)(C , A0, A1, A2, A3, A4);
	typedef ArgThisGetter<C> ARG0;
	typedef ArgGetter<A0, 0> ARG1;typedef ArgGetter<A1, 1> ARG2;typedef ArgGetter<A2, 2> ARG3;typedef ArgGetter<A3, 3> ARG4;typedef ArgGetter<A4, 4> ARG5;
	typedef ReturnResult Result;
	typedef param_types_holder5<C , A0, A1, A2, A3, A4> fun_param_holder;
	fun_t fun;
	cmemfun_holder(const fun_t& f):fun(f){}
	R operator()(C c , A0 a0, A1 a1, A2 a2, A3 a3, A4 a4){ 
		return (*fun)(c , a0, a1, a2, a3, a4); 
	}
};
#endif

template<class T , class A0, class A1, class A2, class A3, class A4>
struct ctor_fun<T , A0, A1, A2, A3, A4>{
	enum{ PARAMS = 5, PARAM_N = 5, METHOD = 0, EXTENDABLE = 0 };
	typedef ArgGetter<A0, 0> ARG0;typedef ArgGetter<A1, 1> ARG1;typedef ArgGetter<A2, 2> ARG2;typedef ArgGetter<A3, 3> ARG3;typedef ArgGetter<A4, 4> ARG4;
	typedef ReturnResult Result;
	typedef param_types_holder5<void , A0, A1, A2, A3, A4> fun_param_holder;
	ctor_fun(){}
	SmartPtr<T> operator()(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4){
		return xnew<T>(a0, a1, a2, a3, a4);
	}
};

//}}REPEAT}

template<class R>
struct cfun_holder<R (*)(const VMachinePtr&)>{
	enum{ PARAMS = 1, PARAM_N = 0, METHOD = 0, EXTENDABLE = 1 };
	typedef R (*fun_t)(const VMachinePtr&);
	typedef ArgGetterVM ARG0; 
	typedef ReturnNone Result;
	typedef param_types_holder0<void > fun_param_holder;
	fun_t fun;
	cfun_holder(const fun_t& f):fun(f){}
	R operator()(const VMachinePtr& a0){ 
		return (*fun)(a0); 
	}
};

template<class T>
struct ctor_fun<T , const VMachinePtr&>{
	enum{ PARAMS = 1, PARAM_N = 0, METHOD = 0, EXTENDABLE = 1 };
	typedef ArgGetterVM ARG0; 
	typedef ReturnNone Result;
	typedef param_types_holder0<void > fun_param_holder;
	ctor_fun(){}
	SmartPtr<T> operator()(const VMachinePtr& a0){ 
		return xnew<T>(a0);
	}
};

template<class R>
struct cmemfun_holder<R (*)(const VMachinePtr&)>{
	enum{ PARAMS = 1, PARAM_N = 0, METHOD = 1, EXTENDABLE = 1 };
	typedef R (*fun_t)(const VMachinePtr&);
	typedef ArgGetterVM ARG0; 
	typedef ReturnNone Result;
	typedef param_types_holder0<void > fun_param_holder;
	fun_t fun;
	cmemfun_holder(const fun_t& f):fun(f){}
	R operator()(const VMachinePtr& a0){ 
		return (*fun)(a0); 
	}
};

template<class R, class C>
struct cmemfun_holder<R (*)(C, const VMachinePtr&)>{
	enum{ PARAMS = 2, PARAM_N = 0, METHOD = 1, EXTENDABLE = 1 };
	typedef R (*fun_t)(C, const VMachinePtr&);
	typedef ArgThisGetter<C> ARG0;
	typedef ArgGetterVM ARG1;
	typedef ReturnNone Result;
	typedef param_types_holder1<C, void > fun_param_holder;
	fun_t fun;
	cmemfun_holder(const fun_t& f):fun(f){}
	R operator()(C self, const VMachinePtr& a0){ 
		return (*fun)(self, a0); 
	}
};

template<class C>
struct cmemfun_holder<void (C::*)(const VMachinePtr&)>{
	enum{ PARAMS = 2, PARAM_N = 0, METHOD = 1, EXTENDABLE = 1 };
	typedef void (C::*fun_t)(const VMachinePtr&);
	typedef ArgThisGetter<C*> ARG0;
	typedef ArgGetterVM ARG1; 
	typedef ReturnNone Result;
	typedef param_types_holder1<C, void > fun_param_holder;
	fun_t fun;
	cmemfun_holder(const fun_t& f):fun(f){}
	void operator()(C* self , const VMachinePtr& a0){ 
		return (self->*fun)(a0); 
	}
};

template<class C>
struct cmemfun_holder<void (C::*)(const VMachinePtr&) const>{
	enum{ PARAMS = 2, PARAM_N = 0, METHOD = 1, EXTENDABLE = 1 };
	typedef void (C::*fun_t)(const VMachinePtr&) const;
	typedef ArgThisGetter<C*> ARG0;
	typedef ArgGetterVM ARG1; 
	typedef ReturnNone Result;
	typedef param_types_holder1<C, void > fun_param_holder;
	fun_t fun;
	cmemfun_holder(const fun_t& f):fun(f){}
	void operator()(C* self , const VMachinePtr& a0){ 
		return (self->*fun)(a0); 
	}
};

}

#endif
