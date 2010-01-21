/** \file src/xtal/xtal_bind.h
* \brief src/xtal/xtal_bind.h
*/

#ifndef XTAL_BIND_H_INCLUDE_GUARD
#define XTAL_BIND_H_INCLUDE_GUARD

#pragma once

namespace xtal{


//{REPEAT{{
/*
template<class C #COMMA_REPEAT#class A`i`#>
struct param_types_holder_base`n`{
	static CppClassSymbolData** const values[`n`+1];
};

template<class C #COMMA_REPEAT#class A`i`#>
CppClassSymbolData** const param_types_holder_base`n`<C #COMMA_REPEAT#A`i`#>::values[`n`+1] = {
	&CppClassSymbol<C>::value,
	#REPEAT#&CppClassSymbol<A`i`>::value,#
};

template<class C #COMMA_REPEAT#class A`i`#>
struct param_types_holder`n` : 
public param_types_holder_base`n`<typename CppClassSymbol<C>::type #COMMA_REPEAT#typename CppClassSymbol<A`i`>::type#>{};
*/

template<class C >
struct param_types_holder_base0{
	static CppClassSymbolData** const values[0+1];
};

template<class C >
CppClassSymbolData** const param_types_holder_base0<C >::values[0+1] = {
	&CppClassSymbol<C>::value,
	
};

template<class C >
struct param_types_holder0 : 
public param_types_holder_base0<typename CppClassSymbol<C>::type >{};

template<class C , class A0>
struct param_types_holder_base1{
	static CppClassSymbolData** const values[1+1];
};

template<class C , class A0>
CppClassSymbolData** const param_types_holder_base1<C , A0>::values[1+1] = {
	&CppClassSymbol<C>::value,
	&CppClassSymbol<A0>::value,
};

template<class C , class A0>
struct param_types_holder1 : 
public param_types_holder_base1<typename CppClassSymbol<C>::type , typename CppClassSymbol<A0>::type>{};

template<class C , class A0, class A1>
struct param_types_holder_base2{
	static CppClassSymbolData** const values[2+1];
};

template<class C , class A0, class A1>
CppClassSymbolData** const param_types_holder_base2<C , A0, A1>::values[2+1] = {
	&CppClassSymbol<C>::value,
	&CppClassSymbol<A0>::value,&CppClassSymbol<A1>::value,
};

template<class C , class A0, class A1>
struct param_types_holder2 : 
public param_types_holder_base2<typename CppClassSymbol<C>::type , typename CppClassSymbol<A0>::type, typename CppClassSymbol<A1>::type>{};

template<class C , class A0, class A1, class A2>
struct param_types_holder_base3{
	static CppClassSymbolData** const values[3+1];
};

template<class C , class A0, class A1, class A2>
CppClassSymbolData** const param_types_holder_base3<C , A0, A1, A2>::values[3+1] = {
	&CppClassSymbol<C>::value,
	&CppClassSymbol<A0>::value,&CppClassSymbol<A1>::value,&CppClassSymbol<A2>::value,
};

template<class C , class A0, class A1, class A2>
struct param_types_holder3 : 
public param_types_holder_base3<typename CppClassSymbol<C>::type , typename CppClassSymbol<A0>::type, typename CppClassSymbol<A1>::type, typename CppClassSymbol<A2>::type>{};

template<class C , class A0, class A1, class A2, class A3>
struct param_types_holder_base4{
	static CppClassSymbolData** const values[4+1];
};

template<class C , class A0, class A1, class A2, class A3>
CppClassSymbolData** const param_types_holder_base4<C , A0, A1, A2, A3>::values[4+1] = {
	&CppClassSymbol<C>::value,
	&CppClassSymbol<A0>::value,&CppClassSymbol<A1>::value,&CppClassSymbol<A2>::value,&CppClassSymbol<A3>::value,
};

template<class C , class A0, class A1, class A2, class A3>
struct param_types_holder4 : 
public param_types_holder_base4<typename CppClassSymbol<C>::type , typename CppClassSymbol<A0>::type, typename CppClassSymbol<A1>::type, typename CppClassSymbol<A2>::type, typename CppClassSymbol<A3>::type>{};

template<class C , class A0, class A1, class A2, class A3, class A4>
struct param_types_holder_base5{
	static CppClassSymbolData** const values[5+1];
};

template<class C , class A0, class A1, class A2, class A3, class A4>
CppClassSymbolData** const param_types_holder_base5<C , A0, A1, A2, A3, A4>::values[5+1] = {
	&CppClassSymbol<C>::value,
	&CppClassSymbol<A0>::value,&CppClassSymbol<A1>::value,&CppClassSymbol<A2>::value,&CppClassSymbol<A3>::value,&CppClassSymbol<A4>::value,
};

template<class C , class A0, class A1, class A2, class A3, class A4>
struct param_types_holder5 : 
public param_types_holder_base5<typename CppClassSymbol<C>::type , typename CppClassSymbol<A0>::type, typename CppClassSymbol<A1>::type, typename CppClassSymbol<A2>::type, typename CppClassSymbol<A3>::type, typename CppClassSymbol<A4>::type>{};

template<class C , class A0, class A1, class A2, class A3, class A4, class A5>
struct param_types_holder_base6{
	static CppClassSymbolData** const values[6+1];
};

template<class C , class A0, class A1, class A2, class A3, class A4, class A5>
CppClassSymbolData** const param_types_holder_base6<C , A0, A1, A2, A3, A4, A5>::values[6+1] = {
	&CppClassSymbol<C>::value,
	&CppClassSymbol<A0>::value,&CppClassSymbol<A1>::value,&CppClassSymbol<A2>::value,&CppClassSymbol<A3>::value,&CppClassSymbol<A4>::value,&CppClassSymbol<A5>::value,
};

template<class C , class A0, class A1, class A2, class A3, class A4, class A5>
struct param_types_holder6 : 
public param_types_holder_base6<typename CppClassSymbol<C>::type , typename CppClassSymbol<A0>::type, typename CppClassSymbol<A1>::type, typename CppClassSymbol<A2>::type, typename CppClassSymbol<A3>::type, typename CppClassSymbol<A4>::type, typename CppClassSymbol<A5>::type>{};

template<class C , class A0, class A1, class A2, class A3, class A4, class A5, class A6>
struct param_types_holder_base7{
	static CppClassSymbolData** const values[7+1];
};

template<class C , class A0, class A1, class A2, class A3, class A4, class A5, class A6>
CppClassSymbolData** const param_types_holder_base7<C , A0, A1, A2, A3, A4, A5, A6>::values[7+1] = {
	&CppClassSymbol<C>::value,
	&CppClassSymbol<A0>::value,&CppClassSymbol<A1>::value,&CppClassSymbol<A2>::value,&CppClassSymbol<A3>::value,&CppClassSymbol<A4>::value,&CppClassSymbol<A5>::value,&CppClassSymbol<A6>::value,
};

template<class C , class A0, class A1, class A2, class A3, class A4, class A5, class A6>
struct param_types_holder7 : 
public param_types_holder_base7<typename CppClassSymbol<C>::type , typename CppClassSymbol<A0>::type, typename CppClassSymbol<A1>::type, typename CppClassSymbol<A2>::type, typename CppClassSymbol<A3>::type, typename CppClassSymbol<A4>::type, typename CppClassSymbol<A5>::type, typename CppClassSymbol<A6>::type>{};

template<class C , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7>
struct param_types_holder_base8{
	static CppClassSymbolData** const values[8+1];
};

template<class C , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7>
CppClassSymbolData** const param_types_holder_base8<C , A0, A1, A2, A3, A4, A5, A6, A7>::values[8+1] = {
	&CppClassSymbol<C>::value,
	&CppClassSymbol<A0>::value,&CppClassSymbol<A1>::value,&CppClassSymbol<A2>::value,&CppClassSymbol<A3>::value,&CppClassSymbol<A4>::value,&CppClassSymbol<A5>::value,&CppClassSymbol<A6>::value,&CppClassSymbol<A7>::value,
};

template<class C , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7>
struct param_types_holder8 : 
public param_types_holder_base8<typename CppClassSymbol<C>::type , typename CppClassSymbol<A0>::type, typename CppClassSymbol<A1>::type, typename CppClassSymbol<A2>::type, typename CppClassSymbol<A3>::type, typename CppClassSymbol<A4>::type, typename CppClassSymbol<A5>::type, typename CppClassSymbol<A6>::type, typename CppClassSymbol<A7>::type>{};

template<class C , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8>
struct param_types_holder_base9{
	static CppClassSymbolData** const values[9+1];
};

template<class C , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8>
CppClassSymbolData** const param_types_holder_base9<C , A0, A1, A2, A3, A4, A5, A6, A7, A8>::values[9+1] = {
	&CppClassSymbol<C>::value,
	&CppClassSymbol<A0>::value,&CppClassSymbol<A1>::value,&CppClassSymbol<A2>::value,&CppClassSymbol<A3>::value,&CppClassSymbol<A4>::value,&CppClassSymbol<A5>::value,&CppClassSymbol<A6>::value,&CppClassSymbol<A7>::value,&CppClassSymbol<A8>::value,
};

template<class C , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8>
struct param_types_holder9 : 
public param_types_holder_base9<typename CppClassSymbol<C>::type , typename CppClassSymbol<A0>::type, typename CppClassSymbol<A1>::type, typename CppClassSymbol<A2>::type, typename CppClassSymbol<A3>::type, typename CppClassSymbol<A4>::type, typename CppClassSymbol<A5>::type, typename CppClassSymbol<A6>::type, typename CppClassSymbol<A7>::type, typename CppClassSymbol<A8>::type>{};

template<class C , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9>
struct param_types_holder_base10{
	static CppClassSymbolData** const values[10+1];
};

template<class C , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9>
CppClassSymbolData** const param_types_holder_base10<C , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9>::values[10+1] = {
	&CppClassSymbol<C>::value,
	&CppClassSymbol<A0>::value,&CppClassSymbol<A1>::value,&CppClassSymbol<A2>::value,&CppClassSymbol<A3>::value,&CppClassSymbol<A4>::value,&CppClassSymbol<A5>::value,&CppClassSymbol<A6>::value,&CppClassSymbol<A7>::value,&CppClassSymbol<A8>::value,&CppClassSymbol<A9>::value,
};

template<class C , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9>
struct param_types_holder10 : 
public param_types_holder_base10<typename CppClassSymbol<C>::type , typename CppClassSymbol<A0>::type, typename CppClassSymbol<A1>::type, typename CppClassSymbol<A2>::type, typename CppClassSymbol<A3>::type, typename CppClassSymbol<A4>::type, typename CppClassSymbol<A5>::type, typename CppClassSymbol<A6>::type, typename CppClassSymbol<A7>::type, typename CppClassSymbol<A8>::type, typename CppClassSymbol<A9>::type>{};

template<class C , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10>
struct param_types_holder_base11{
	static CppClassSymbolData** const values[11+1];
};

template<class C , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10>
CppClassSymbolData** const param_types_holder_base11<C , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10>::values[11+1] = {
	&CppClassSymbol<C>::value,
	&CppClassSymbol<A0>::value,&CppClassSymbol<A1>::value,&CppClassSymbol<A2>::value,&CppClassSymbol<A3>::value,&CppClassSymbol<A4>::value,&CppClassSymbol<A5>::value,&CppClassSymbol<A6>::value,&CppClassSymbol<A7>::value,&CppClassSymbol<A8>::value,&CppClassSymbol<A9>::value,&CppClassSymbol<A10>::value,
};

template<class C , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10>
struct param_types_holder11 : 
public param_types_holder_base11<typename CppClassSymbol<C>::type , typename CppClassSymbol<A0>::type, typename CppClassSymbol<A1>::type, typename CppClassSymbol<A2>::type, typename CppClassSymbol<A3>::type, typename CppClassSymbol<A4>::type, typename CppClassSymbol<A5>::type, typename CppClassSymbol<A6>::type, typename CppClassSymbol<A7>::type, typename CppClassSymbol<A8>::type, typename CppClassSymbol<A9>::type, typename CppClassSymbol<A10>::type>{};

template<class C , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11>
struct param_types_holder_base12{
	static CppClassSymbolData** const values[12+1];
};

template<class C , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11>
CppClassSymbolData** const param_types_holder_base12<C , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11>::values[12+1] = {
	&CppClassSymbol<C>::value,
	&CppClassSymbol<A0>::value,&CppClassSymbol<A1>::value,&CppClassSymbol<A2>::value,&CppClassSymbol<A3>::value,&CppClassSymbol<A4>::value,&CppClassSymbol<A5>::value,&CppClassSymbol<A6>::value,&CppClassSymbol<A7>::value,&CppClassSymbol<A8>::value,&CppClassSymbol<A9>::value,&CppClassSymbol<A10>::value,&CppClassSymbol<A11>::value,
};

template<class C , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11>
struct param_types_holder12 : 
public param_types_holder_base12<typename CppClassSymbol<C>::type , typename CppClassSymbol<A0>::type, typename CppClassSymbol<A1>::type, typename CppClassSymbol<A2>::type, typename CppClassSymbol<A3>::type, typename CppClassSymbol<A4>::type, typename CppClassSymbol<A5>::type, typename CppClassSymbol<A6>::type, typename CppClassSymbol<A7>::type, typename CppClassSymbol<A8>::type, typename CppClassSymbol<A9>::type, typename CppClassSymbol<A10>::type, typename CppClassSymbol<A11>::type>{};

template<class C , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12>
struct param_types_holder_base13{
	static CppClassSymbolData** const values[13+1];
};

template<class C , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12>
CppClassSymbolData** const param_types_holder_base13<C , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12>::values[13+1] = {
	&CppClassSymbol<C>::value,
	&CppClassSymbol<A0>::value,&CppClassSymbol<A1>::value,&CppClassSymbol<A2>::value,&CppClassSymbol<A3>::value,&CppClassSymbol<A4>::value,&CppClassSymbol<A5>::value,&CppClassSymbol<A6>::value,&CppClassSymbol<A7>::value,&CppClassSymbol<A8>::value,&CppClassSymbol<A9>::value,&CppClassSymbol<A10>::value,&CppClassSymbol<A11>::value,&CppClassSymbol<A12>::value,
};

template<class C , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12>
struct param_types_holder13 : 
public param_types_holder_base13<typename CppClassSymbol<C>::type , typename CppClassSymbol<A0>::type, typename CppClassSymbol<A1>::type, typename CppClassSymbol<A2>::type, typename CppClassSymbol<A3>::type, typename CppClassSymbol<A4>::type, typename CppClassSymbol<A5>::type, typename CppClassSymbol<A6>::type, typename CppClassSymbol<A7>::type, typename CppClassSymbol<A8>::type, typename CppClassSymbol<A9>::type, typename CppClassSymbol<A10>::type, typename CppClassSymbol<A11>::type, typename CppClassSymbol<A12>::type>{};

//}}REPEAT}


//{REPEAT{{
/*

/////

template<class R #COMMA_REPEAT#class A`i`#>
struct cfun_holder_base<R (*)(#REPEAT_COMMA#A`i`#)>{
	typedef R (*fun_t)(#REPEAT_COMMA#A`i`#);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args);
};

template<class R #COMMA_REPEAT#class A`i`#>
void cfun_holder_base<R (*)(#REPEAT_COMMA#A`i`#)>::call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
	XTAL_RETURN_TO_VM((*(fun_t*)data)(
		#REPEAT_COMMA#unchecked_cast<A`i`>((AnyPtr&)args[`i+1`])#
	));
}

template<class R #COMMA_REPEAT#class A`i`#>
struct cfun_holder<R (*)(#REPEAT_COMMA#A`i`#)> : 
cfun_holder_base<R (*)(#REPEAT_COMMA#typename CommonType<A`i`>::type#)>{
	enum{ PARAM_N = `n`, METHOD = 0, EXTENDABLE = 0 };
	typedef param_types_holder`n`<void #COMMA_REPEAT#A`i`#> fun_param_holder;
};

/////

template<class C, class R #COMMA_REPEAT#class A`i`#>
struct cmemfun_holder_base<R (C::*)(#REPEAT_COMMA#A`i`#)>{
	typedef R (C::*fun_t)(#REPEAT_COMMA#A`i`#);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args);
};

template<class C, class R #COMMA_REPEAT#class A`i`#>
void cmemfun_holder_base<R (C::*)(#REPEAT_COMMA#A`i`#)>::call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
	XTAL_RETURN_TO_VM((unchecked_cast<C*>((AnyPtr&)args[0])->**(fun_t*)data)(
		#REPEAT_COMMA#unchecked_cast<A`i`>((AnyPtr&)args[`i+1`])#
	));
}

template<class C, class R #COMMA_REPEAT#class A`i`#>
struct cmemfun_holder<R (C::*)(#REPEAT_COMMA#A`i`#)> : 
cmemfun_holder_base<R (CommonThisType<C>::type::*)(#REPEAT_COMMA#typename CommonType<A`i`>::type#)>{
	enum{ PARAM_N = `n`, METHOD = 1, EXTENDABLE = 0 };
	typedef param_types_holder`n`<C #COMMA_REPEAT#A`i`#> fun_param_holder;
};

template<class C, class R #COMMA_REPEAT#class A`i`#>
struct cmemfun_holder<R (C::*)(#REPEAT_COMMA#A`i`#) const> : 
cmemfun_holder_base<R (CommonThisType<C>::type::*)(#REPEAT_COMMA#typename CommonType<A`i`>::type#)>{
	enum{ PARAM_N = `n`, METHOD = 1, EXTENDABLE = 0 };
	typedef param_types_holder`n`<C #COMMA_REPEAT#A`i`#> fun_param_holder;
};

/////

template<class C, class R #COMMA_REPEAT#class A`i`#>
struct cmemfun_holder_base<R (*)(C #COMMA_REPEAT#A`i`#)>{
	typedef R (*fun_t)(C #COMMA_REPEAT#A`i`#);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args);
};

template<class C, class R #COMMA_REPEAT#class A`i`#>
void cmemfun_holder_base<R (*)(C #COMMA_REPEAT#A`i`#)>::call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
	XTAL_RETURN_TO_VM((*(fun_t*)data)(
		unchecked_cast<C>((AnyPtr&)args[0])
		#COMMA_REPEAT#unchecked_cast<A`i`>((AnyPtr&)args[`i+1`])#
	));
}

template<class C, class R #COMMA_REPEAT#class A`i`#>
struct cmemfun_holder<R (*)(C #COMMA_REPEAT#A`i`#)> :
cmemfun_holder_base<R (*)(typename CommonType<C>::type #COMMA_REPEAT#typename CommonType<A`i`>::type#)>{
	enum{ PARAM_N = `n`, METHOD = 1, EXTENDABLE = 0 };
	typedef param_types_holder`n`<C #COMMA_REPEAT#A`i`#> fun_param_holder;
};

/////

template<class T #COMMA_REPEAT#class A`i`#>
struct ctor_fun<T #COMMA_REPEAT#A`i`#>{
	enum{ PARAM_N = `n`, METHOD = 0, EXTENDABLE = 0 };
	typedef param_types_holder`n`<void #COMMA_REPEAT#A`i`#> fun_param_holder;
	typedef char fun_t;

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		vm->return_result(xnew<T>(
			#REPEAT_COMMA#unchecked_cast<A`i`>((AnyPtr&)args[`i+1`])#
		));
	}
};

#if defined(_WIN32) && defined(_M_IX86)
template<class R #COMMA_REPEAT#class A`i`#>
struct cfun_holder<R (__stdcall *)(#REPEAT_COMMA#A`i`#)>{
	enum{ PARAM_N = `n`, METHOD = 0, EXTENDABLE = 0 };
	typedef param_types_holder`n`<void #COMMA_REPEAT#A`i`#> fun_param_holder;
	typedef R (__stdcall *fun_t)(#REPEAT_COMMA#A`i`#);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		XTAL_RETURN_TO_VM((*(fun_t*)data)(
			#REPEAT_COMMA#unchecked_cast<A`i`>((AnyPtr&)args[`i+1`])#
		));
	}
};

template<class C, class R #COMMA_REPEAT#class A`i`#>
struct cmemfun_holder<R (__stdcall *)(C #COMMA_REPEAT#A`i`#)>{
	enum{ PARAM_N = `n`, METHOD = 1, EXTENDABLE = 0 };
	typedef param_types_holder`n`<C #COMMA_REPEAT#A`i`#> fun_param_holder;
	typedef R (__stdcall *fun_t)(C #COMMA_REPEAT#A`i`#);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		XTAL_RETURN_TO_VM((*(fun_t*)data)(
			unchecked_cast<C>((AnyPtr&)args[0])
			#COMMA_REPEAT#unchecked_cast<A`i`>((AnyPtr&)args[`i+1`])#
		));
	}
};
#endif

*/


/////

template<class R >
struct cfun_holder_base<R (*)()>{
	typedef R (*fun_t)();

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args);
};

template<class R >
void cfun_holder_base<R (*)()>::call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
	XTAL_RETURN_TO_VM((*(fun_t*)data)(
		
	));
}

template<class R >
struct cfun_holder<R (*)()> : 
cfun_holder_base<R (*)()>{
	enum{ PARAM_N = 0, METHOD = 0, EXTENDABLE = 0 };
	typedef param_types_holder0<void > fun_param_holder;
};

/////

template<class C, class R >
struct cmemfun_holder_base<R (C::*)()>{
	typedef R (C::*fun_t)();

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args);
};

template<class C, class R >
void cmemfun_holder_base<R (C::*)()>::call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
	XTAL_RETURN_TO_VM((unchecked_cast<C*>((AnyPtr&)args[0])->**(fun_t*)data)(
		
	));
}

template<class C, class R >
struct cmemfun_holder<R (C::*)()> : 
cmemfun_holder_base<R (CommonThisType<C>::type::*)()>{
	enum{ PARAM_N = 0, METHOD = 1, EXTENDABLE = 0 };
	typedef param_types_holder0<C > fun_param_holder;
};

template<class C, class R >
struct cmemfun_holder<R (C::*)() const> : 
cmemfun_holder_base<R (CommonThisType<C>::type::*)()>{
	enum{ PARAM_N = 0, METHOD = 1, EXTENDABLE = 0 };
	typedef param_types_holder0<C > fun_param_holder;
};

/////

template<class C, class R >
struct cmemfun_holder_base<R (*)(C )>{
	typedef R (*fun_t)(C );

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args);
};

template<class C, class R >
void cmemfun_holder_base<R (*)(C )>::call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
	XTAL_RETURN_TO_VM((*(fun_t*)data)(
		unchecked_cast<C>((AnyPtr&)args[0])
		
	));
}

template<class C, class R >
struct cmemfun_holder<R (*)(C )> :
cmemfun_holder_base<R (*)(typename CommonType<C>::type )>{
	enum{ PARAM_N = 0, METHOD = 1, EXTENDABLE = 0 };
	typedef param_types_holder0<C > fun_param_holder;
};

/////

template<class T >
struct ctor_fun<T >{
	enum{ PARAM_N = 0, METHOD = 0, EXTENDABLE = 0 };
	typedef param_types_holder0<void > fun_param_holder;
	typedef char fun_t;

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		vm->return_result(xnew<T>(
			
		));
	}
};

#if defined(_WIN32) && defined(_M_IX86)
template<class R >
struct cfun_holder<R (__stdcall *)()>{
	enum{ PARAM_N = 0, METHOD = 0, EXTENDABLE = 0 };
	typedef param_types_holder0<void > fun_param_holder;
	typedef R (__stdcall *fun_t)();

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		XTAL_RETURN_TO_VM((*(fun_t*)data)(
			
		));
	}
};

template<class C, class R >
struct cmemfun_holder<R (__stdcall *)(C )>{
	enum{ PARAM_N = 0, METHOD = 1, EXTENDABLE = 0 };
	typedef param_types_holder0<C > fun_param_holder;
	typedef R (__stdcall *fun_t)(C );

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		XTAL_RETURN_TO_VM((*(fun_t*)data)(
			unchecked_cast<C>((AnyPtr&)args[0])
			
		));
	}
};
#endif



/////

template<class R , class A0>
struct cfun_holder_base<R (*)(A0)>{
	typedef R (*fun_t)(A0);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args);
};

template<class R , class A0>
void cfun_holder_base<R (*)(A0)>::call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
	XTAL_RETURN_TO_VM((*(fun_t*)data)(
		unchecked_cast<A0>((AnyPtr&)args[1])
	));
}

template<class R , class A0>
struct cfun_holder<R (*)(A0)> : 
cfun_holder_base<R (*)(typename CommonType<A0>::type)>{
	enum{ PARAM_N = 1, METHOD = 0, EXTENDABLE = 0 };
	typedef param_types_holder1<void , A0> fun_param_holder;
};

/////

template<class C, class R , class A0>
struct cmemfun_holder_base<R (C::*)(A0)>{
	typedef R (C::*fun_t)(A0);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args);
};

template<class C, class R , class A0>
void cmemfun_holder_base<R (C::*)(A0)>::call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
	XTAL_RETURN_TO_VM((unchecked_cast<C*>((AnyPtr&)args[0])->**(fun_t*)data)(
		unchecked_cast<A0>((AnyPtr&)args[1])
	));
}

template<class C, class R , class A0>
struct cmemfun_holder<R (C::*)(A0)> : 
cmemfun_holder_base<R (CommonThisType<C>::type::*)(typename CommonType<A0>::type)>{
	enum{ PARAM_N = 1, METHOD = 1, EXTENDABLE = 0 };
	typedef param_types_holder1<C , A0> fun_param_holder;
};

template<class C, class R , class A0>
struct cmemfun_holder<R (C::*)(A0) const> : 
cmemfun_holder_base<R (CommonThisType<C>::type::*)(typename CommonType<A0>::type)>{
	enum{ PARAM_N = 1, METHOD = 1, EXTENDABLE = 0 };
	typedef param_types_holder1<C , A0> fun_param_holder;
};

/////

template<class C, class R , class A0>
struct cmemfun_holder_base<R (*)(C , A0)>{
	typedef R (*fun_t)(C , A0);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args);
};

template<class C, class R , class A0>
void cmemfun_holder_base<R (*)(C , A0)>::call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
	XTAL_RETURN_TO_VM((*(fun_t*)data)(
		unchecked_cast<C>((AnyPtr&)args[0])
		, unchecked_cast<A0>((AnyPtr&)args[1])
	));
}

template<class C, class R , class A0>
struct cmemfun_holder<R (*)(C , A0)> :
cmemfun_holder_base<R (*)(typename CommonType<C>::type , typename CommonType<A0>::type)>{
	enum{ PARAM_N = 1, METHOD = 1, EXTENDABLE = 0 };
	typedef param_types_holder1<C , A0> fun_param_holder;
};

/////

template<class T , class A0>
struct ctor_fun<T , A0>{
	enum{ PARAM_N = 1, METHOD = 0, EXTENDABLE = 0 };
	typedef param_types_holder1<void , A0> fun_param_holder;
	typedef char fun_t;

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		vm->return_result(xnew<T>(
			unchecked_cast<A0>((AnyPtr&)args[1])
		));
	}
};

#if defined(_WIN32) && defined(_M_IX86)
template<class R , class A0>
struct cfun_holder<R (__stdcall *)(A0)>{
	enum{ PARAM_N = 1, METHOD = 0, EXTENDABLE = 0 };
	typedef param_types_holder1<void , A0> fun_param_holder;
	typedef R (__stdcall *fun_t)(A0);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		XTAL_RETURN_TO_VM((*(fun_t*)data)(
			unchecked_cast<A0>((AnyPtr&)args[1])
		));
	}
};

template<class C, class R , class A0>
struct cmemfun_holder<R (__stdcall *)(C , A0)>{
	enum{ PARAM_N = 1, METHOD = 1, EXTENDABLE = 0 };
	typedef param_types_holder1<C , A0> fun_param_holder;
	typedef R (__stdcall *fun_t)(C , A0);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		XTAL_RETURN_TO_VM((*(fun_t*)data)(
			unchecked_cast<C>((AnyPtr&)args[0])
			, unchecked_cast<A0>((AnyPtr&)args[1])
		));
	}
};
#endif



/////

template<class R , class A0, class A1>
struct cfun_holder_base<R (*)(A0, A1)>{
	typedef R (*fun_t)(A0, A1);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args);
};

template<class R , class A0, class A1>
void cfun_holder_base<R (*)(A0, A1)>::call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
	XTAL_RETURN_TO_VM((*(fun_t*)data)(
		unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2])
	));
}

template<class R , class A0, class A1>
struct cfun_holder<R (*)(A0, A1)> : 
cfun_holder_base<R (*)(typename CommonType<A0>::type, typename CommonType<A1>::type)>{
	enum{ PARAM_N = 2, METHOD = 0, EXTENDABLE = 0 };
	typedef param_types_holder2<void , A0, A1> fun_param_holder;
};

/////

template<class C, class R , class A0, class A1>
struct cmemfun_holder_base<R (C::*)(A0, A1)>{
	typedef R (C::*fun_t)(A0, A1);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args);
};

template<class C, class R , class A0, class A1>
void cmemfun_holder_base<R (C::*)(A0, A1)>::call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
	XTAL_RETURN_TO_VM((unchecked_cast<C*>((AnyPtr&)args[0])->**(fun_t*)data)(
		unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2])
	));
}

template<class C, class R , class A0, class A1>
struct cmemfun_holder<R (C::*)(A0, A1)> : 
cmemfun_holder_base<R (CommonThisType<C>::type::*)(typename CommonType<A0>::type, typename CommonType<A1>::type)>{
	enum{ PARAM_N = 2, METHOD = 1, EXTENDABLE = 0 };
	typedef param_types_holder2<C , A0, A1> fun_param_holder;
};

template<class C, class R , class A0, class A1>
struct cmemfun_holder<R (C::*)(A0, A1) const> : 
cmemfun_holder_base<R (CommonThisType<C>::type::*)(typename CommonType<A0>::type, typename CommonType<A1>::type)>{
	enum{ PARAM_N = 2, METHOD = 1, EXTENDABLE = 0 };
	typedef param_types_holder2<C , A0, A1> fun_param_holder;
};

/////

template<class C, class R , class A0, class A1>
struct cmemfun_holder_base<R (*)(C , A0, A1)>{
	typedef R (*fun_t)(C , A0, A1);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args);
};

template<class C, class R , class A0, class A1>
void cmemfun_holder_base<R (*)(C , A0, A1)>::call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
	XTAL_RETURN_TO_VM((*(fun_t*)data)(
		unchecked_cast<C>((AnyPtr&)args[0])
		, unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2])
	));
}

template<class C, class R , class A0, class A1>
struct cmemfun_holder<R (*)(C , A0, A1)> :
cmemfun_holder_base<R (*)(typename CommonType<C>::type , typename CommonType<A0>::type, typename CommonType<A1>::type)>{
	enum{ PARAM_N = 2, METHOD = 1, EXTENDABLE = 0 };
	typedef param_types_holder2<C , A0, A1> fun_param_holder;
};

/////

template<class T , class A0, class A1>
struct ctor_fun<T , A0, A1>{
	enum{ PARAM_N = 2, METHOD = 0, EXTENDABLE = 0 };
	typedef param_types_holder2<void , A0, A1> fun_param_holder;
	typedef char fun_t;

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		vm->return_result(xnew<T>(
			unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2])
		));
	}
};

#if defined(_WIN32) && defined(_M_IX86)
template<class R , class A0, class A1>
struct cfun_holder<R (__stdcall *)(A0, A1)>{
	enum{ PARAM_N = 2, METHOD = 0, EXTENDABLE = 0 };
	typedef param_types_holder2<void , A0, A1> fun_param_holder;
	typedef R (__stdcall *fun_t)(A0, A1);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		XTAL_RETURN_TO_VM((*(fun_t*)data)(
			unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2])
		));
	}
};

template<class C, class R , class A0, class A1>
struct cmemfun_holder<R (__stdcall *)(C , A0, A1)>{
	enum{ PARAM_N = 2, METHOD = 1, EXTENDABLE = 0 };
	typedef param_types_holder2<C , A0, A1> fun_param_holder;
	typedef R (__stdcall *fun_t)(C , A0, A1);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		XTAL_RETURN_TO_VM((*(fun_t*)data)(
			unchecked_cast<C>((AnyPtr&)args[0])
			, unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2])
		));
	}
};
#endif



/////

template<class R , class A0, class A1, class A2>
struct cfun_holder_base<R (*)(A0, A1, A2)>{
	typedef R (*fun_t)(A0, A1, A2);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args);
};

template<class R , class A0, class A1, class A2>
void cfun_holder_base<R (*)(A0, A1, A2)>::call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
	XTAL_RETURN_TO_VM((*(fun_t*)data)(
		unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3])
	));
}

template<class R , class A0, class A1, class A2>
struct cfun_holder<R (*)(A0, A1, A2)> : 
cfun_holder_base<R (*)(typename CommonType<A0>::type, typename CommonType<A1>::type, typename CommonType<A2>::type)>{
	enum{ PARAM_N = 3, METHOD = 0, EXTENDABLE = 0 };
	typedef param_types_holder3<void , A0, A1, A2> fun_param_holder;
};

/////

template<class C, class R , class A0, class A1, class A2>
struct cmemfun_holder_base<R (C::*)(A0, A1, A2)>{
	typedef R (C::*fun_t)(A0, A1, A2);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args);
};

template<class C, class R , class A0, class A1, class A2>
void cmemfun_holder_base<R (C::*)(A0, A1, A2)>::call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
	XTAL_RETURN_TO_VM((unchecked_cast<C*>((AnyPtr&)args[0])->**(fun_t*)data)(
		unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3])
	));
}

template<class C, class R , class A0, class A1, class A2>
struct cmemfun_holder<R (C::*)(A0, A1, A2)> : 
cmemfun_holder_base<R (CommonThisType<C>::type::*)(typename CommonType<A0>::type, typename CommonType<A1>::type, typename CommonType<A2>::type)>{
	enum{ PARAM_N = 3, METHOD = 1, EXTENDABLE = 0 };
	typedef param_types_holder3<C , A0, A1, A2> fun_param_holder;
};

template<class C, class R , class A0, class A1, class A2>
struct cmemfun_holder<R (C::*)(A0, A1, A2) const> : 
cmemfun_holder_base<R (CommonThisType<C>::type::*)(typename CommonType<A0>::type, typename CommonType<A1>::type, typename CommonType<A2>::type)>{
	enum{ PARAM_N = 3, METHOD = 1, EXTENDABLE = 0 };
	typedef param_types_holder3<C , A0, A1, A2> fun_param_holder;
};

/////

template<class C, class R , class A0, class A1, class A2>
struct cmemfun_holder_base<R (*)(C , A0, A1, A2)>{
	typedef R (*fun_t)(C , A0, A1, A2);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args);
};

template<class C, class R , class A0, class A1, class A2>
void cmemfun_holder_base<R (*)(C , A0, A1, A2)>::call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
	XTAL_RETURN_TO_VM((*(fun_t*)data)(
		unchecked_cast<C>((AnyPtr&)args[0])
		, unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3])
	));
}

template<class C, class R , class A0, class A1, class A2>
struct cmemfun_holder<R (*)(C , A0, A1, A2)> :
cmemfun_holder_base<R (*)(typename CommonType<C>::type , typename CommonType<A0>::type, typename CommonType<A1>::type, typename CommonType<A2>::type)>{
	enum{ PARAM_N = 3, METHOD = 1, EXTENDABLE = 0 };
	typedef param_types_holder3<C , A0, A1, A2> fun_param_holder;
};

/////

template<class T , class A0, class A1, class A2>
struct ctor_fun<T , A0, A1, A2>{
	enum{ PARAM_N = 3, METHOD = 0, EXTENDABLE = 0 };
	typedef param_types_holder3<void , A0, A1, A2> fun_param_holder;
	typedef char fun_t;

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		vm->return_result(xnew<T>(
			unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3])
		));
	}
};

#if defined(_WIN32) && defined(_M_IX86)
template<class R , class A0, class A1, class A2>
struct cfun_holder<R (__stdcall *)(A0, A1, A2)>{
	enum{ PARAM_N = 3, METHOD = 0, EXTENDABLE = 0 };
	typedef param_types_holder3<void , A0, A1, A2> fun_param_holder;
	typedef R (__stdcall *fun_t)(A0, A1, A2);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		XTAL_RETURN_TO_VM((*(fun_t*)data)(
			unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3])
		));
	}
};

template<class C, class R , class A0, class A1, class A2>
struct cmemfun_holder<R (__stdcall *)(C , A0, A1, A2)>{
	enum{ PARAM_N = 3, METHOD = 1, EXTENDABLE = 0 };
	typedef param_types_holder3<C , A0, A1, A2> fun_param_holder;
	typedef R (__stdcall *fun_t)(C , A0, A1, A2);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		XTAL_RETURN_TO_VM((*(fun_t*)data)(
			unchecked_cast<C>((AnyPtr&)args[0])
			, unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3])
		));
	}
};
#endif



/////

template<class R , class A0, class A1, class A2, class A3>
struct cfun_holder_base<R (*)(A0, A1, A2, A3)>{
	typedef R (*fun_t)(A0, A1, A2, A3);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args);
};

template<class R , class A0, class A1, class A2, class A3>
void cfun_holder_base<R (*)(A0, A1, A2, A3)>::call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
	XTAL_RETURN_TO_VM((*(fun_t*)data)(
		unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4])
	));
}

template<class R , class A0, class A1, class A2, class A3>
struct cfun_holder<R (*)(A0, A1, A2, A3)> : 
cfun_holder_base<R (*)(typename CommonType<A0>::type, typename CommonType<A1>::type, typename CommonType<A2>::type, typename CommonType<A3>::type)>{
	enum{ PARAM_N = 4, METHOD = 0, EXTENDABLE = 0 };
	typedef param_types_holder4<void , A0, A1, A2, A3> fun_param_holder;
};

/////

template<class C, class R , class A0, class A1, class A2, class A3>
struct cmemfun_holder_base<R (C::*)(A0, A1, A2, A3)>{
	typedef R (C::*fun_t)(A0, A1, A2, A3);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args);
};

template<class C, class R , class A0, class A1, class A2, class A3>
void cmemfun_holder_base<R (C::*)(A0, A1, A2, A3)>::call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
	XTAL_RETURN_TO_VM((unchecked_cast<C*>((AnyPtr&)args[0])->**(fun_t*)data)(
		unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4])
	));
}

template<class C, class R , class A0, class A1, class A2, class A3>
struct cmemfun_holder<R (C::*)(A0, A1, A2, A3)> : 
cmemfun_holder_base<R (CommonThisType<C>::type::*)(typename CommonType<A0>::type, typename CommonType<A1>::type, typename CommonType<A2>::type, typename CommonType<A3>::type)>{
	enum{ PARAM_N = 4, METHOD = 1, EXTENDABLE = 0 };
	typedef param_types_holder4<C , A0, A1, A2, A3> fun_param_holder;
};

template<class C, class R , class A0, class A1, class A2, class A3>
struct cmemfun_holder<R (C::*)(A0, A1, A2, A3) const> : 
cmemfun_holder_base<R (CommonThisType<C>::type::*)(typename CommonType<A0>::type, typename CommonType<A1>::type, typename CommonType<A2>::type, typename CommonType<A3>::type)>{
	enum{ PARAM_N = 4, METHOD = 1, EXTENDABLE = 0 };
	typedef param_types_holder4<C , A0, A1, A2, A3> fun_param_holder;
};

/////

template<class C, class R , class A0, class A1, class A2, class A3>
struct cmemfun_holder_base<R (*)(C , A0, A1, A2, A3)>{
	typedef R (*fun_t)(C , A0, A1, A2, A3);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args);
};

template<class C, class R , class A0, class A1, class A2, class A3>
void cmemfun_holder_base<R (*)(C , A0, A1, A2, A3)>::call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
	XTAL_RETURN_TO_VM((*(fun_t*)data)(
		unchecked_cast<C>((AnyPtr&)args[0])
		, unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4])
	));
}

template<class C, class R , class A0, class A1, class A2, class A3>
struct cmemfun_holder<R (*)(C , A0, A1, A2, A3)> :
cmemfun_holder_base<R (*)(typename CommonType<C>::type , typename CommonType<A0>::type, typename CommonType<A1>::type, typename CommonType<A2>::type, typename CommonType<A3>::type)>{
	enum{ PARAM_N = 4, METHOD = 1, EXTENDABLE = 0 };
	typedef param_types_holder4<C , A0, A1, A2, A3> fun_param_holder;
};

/////

template<class T , class A0, class A1, class A2, class A3>
struct ctor_fun<T , A0, A1, A2, A3>{
	enum{ PARAM_N = 4, METHOD = 0, EXTENDABLE = 0 };
	typedef param_types_holder4<void , A0, A1, A2, A3> fun_param_holder;
	typedef char fun_t;

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		vm->return_result(xnew<T>(
			unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4])
		));
	}
};

#if defined(_WIN32) && defined(_M_IX86)
template<class R , class A0, class A1, class A2, class A3>
struct cfun_holder<R (__stdcall *)(A0, A1, A2, A3)>{
	enum{ PARAM_N = 4, METHOD = 0, EXTENDABLE = 0 };
	typedef param_types_holder4<void , A0, A1, A2, A3> fun_param_holder;
	typedef R (__stdcall *fun_t)(A0, A1, A2, A3);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		XTAL_RETURN_TO_VM((*(fun_t*)data)(
			unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4])
		));
	}
};

template<class C, class R , class A0, class A1, class A2, class A3>
struct cmemfun_holder<R (__stdcall *)(C , A0, A1, A2, A3)>{
	enum{ PARAM_N = 4, METHOD = 1, EXTENDABLE = 0 };
	typedef param_types_holder4<C , A0, A1, A2, A3> fun_param_holder;
	typedef R (__stdcall *fun_t)(C , A0, A1, A2, A3);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		XTAL_RETURN_TO_VM((*(fun_t*)data)(
			unchecked_cast<C>((AnyPtr&)args[0])
			, unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4])
		));
	}
};
#endif



/////

template<class R , class A0, class A1, class A2, class A3, class A4>
struct cfun_holder_base<R (*)(A0, A1, A2, A3, A4)>{
	typedef R (*fun_t)(A0, A1, A2, A3, A4);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args);
};

template<class R , class A0, class A1, class A2, class A3, class A4>
void cfun_holder_base<R (*)(A0, A1, A2, A3, A4)>::call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
	XTAL_RETURN_TO_VM((*(fun_t*)data)(
		unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4]), unchecked_cast<A4>((AnyPtr&)args[5])
	));
}

template<class R , class A0, class A1, class A2, class A3, class A4>
struct cfun_holder<R (*)(A0, A1, A2, A3, A4)> : 
cfun_holder_base<R (*)(typename CommonType<A0>::type, typename CommonType<A1>::type, typename CommonType<A2>::type, typename CommonType<A3>::type, typename CommonType<A4>::type)>{
	enum{ PARAM_N = 5, METHOD = 0, EXTENDABLE = 0 };
	typedef param_types_holder5<void , A0, A1, A2, A3, A4> fun_param_holder;
};

/////

template<class C, class R , class A0, class A1, class A2, class A3, class A4>
struct cmemfun_holder_base<R (C::*)(A0, A1, A2, A3, A4)>{
	typedef R (C::*fun_t)(A0, A1, A2, A3, A4);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args);
};

template<class C, class R , class A0, class A1, class A2, class A3, class A4>
void cmemfun_holder_base<R (C::*)(A0, A1, A2, A3, A4)>::call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
	XTAL_RETURN_TO_VM((unchecked_cast<C*>((AnyPtr&)args[0])->**(fun_t*)data)(
		unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4]), unchecked_cast<A4>((AnyPtr&)args[5])
	));
}

template<class C, class R , class A0, class A1, class A2, class A3, class A4>
struct cmemfun_holder<R (C::*)(A0, A1, A2, A3, A4)> : 
cmemfun_holder_base<R (CommonThisType<C>::type::*)(typename CommonType<A0>::type, typename CommonType<A1>::type, typename CommonType<A2>::type, typename CommonType<A3>::type, typename CommonType<A4>::type)>{
	enum{ PARAM_N = 5, METHOD = 1, EXTENDABLE = 0 };
	typedef param_types_holder5<C , A0, A1, A2, A3, A4> fun_param_holder;
};

template<class C, class R , class A0, class A1, class A2, class A3, class A4>
struct cmemfun_holder<R (C::*)(A0, A1, A2, A3, A4) const> : 
cmemfun_holder_base<R (CommonThisType<C>::type::*)(typename CommonType<A0>::type, typename CommonType<A1>::type, typename CommonType<A2>::type, typename CommonType<A3>::type, typename CommonType<A4>::type)>{
	enum{ PARAM_N = 5, METHOD = 1, EXTENDABLE = 0 };
	typedef param_types_holder5<C , A0, A1, A2, A3, A4> fun_param_holder;
};

/////

template<class C, class R , class A0, class A1, class A2, class A3, class A4>
struct cmemfun_holder_base<R (*)(C , A0, A1, A2, A3, A4)>{
	typedef R (*fun_t)(C , A0, A1, A2, A3, A4);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args);
};

template<class C, class R , class A0, class A1, class A2, class A3, class A4>
void cmemfun_holder_base<R (*)(C , A0, A1, A2, A3, A4)>::call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
	XTAL_RETURN_TO_VM((*(fun_t*)data)(
		unchecked_cast<C>((AnyPtr&)args[0])
		, unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4]), unchecked_cast<A4>((AnyPtr&)args[5])
	));
}

template<class C, class R , class A0, class A1, class A2, class A3, class A4>
struct cmemfun_holder<R (*)(C , A0, A1, A2, A3, A4)> :
cmemfun_holder_base<R (*)(typename CommonType<C>::type , typename CommonType<A0>::type, typename CommonType<A1>::type, typename CommonType<A2>::type, typename CommonType<A3>::type, typename CommonType<A4>::type)>{
	enum{ PARAM_N = 5, METHOD = 1, EXTENDABLE = 0 };
	typedef param_types_holder5<C , A0, A1, A2, A3, A4> fun_param_holder;
};

/////

template<class T , class A0, class A1, class A2, class A3, class A4>
struct ctor_fun<T , A0, A1, A2, A3, A4>{
	enum{ PARAM_N = 5, METHOD = 0, EXTENDABLE = 0 };
	typedef param_types_holder5<void , A0, A1, A2, A3, A4> fun_param_holder;
	typedef char fun_t;

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		vm->return_result(xnew<T>(
			unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4]), unchecked_cast<A4>((AnyPtr&)args[5])
		));
	}
};

#if defined(_WIN32) && defined(_M_IX86)
template<class R , class A0, class A1, class A2, class A3, class A4>
struct cfun_holder<R (__stdcall *)(A0, A1, A2, A3, A4)>{
	enum{ PARAM_N = 5, METHOD = 0, EXTENDABLE = 0 };
	typedef param_types_holder5<void , A0, A1, A2, A3, A4> fun_param_holder;
	typedef R (__stdcall *fun_t)(A0, A1, A2, A3, A4);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		XTAL_RETURN_TO_VM((*(fun_t*)data)(
			unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4]), unchecked_cast<A4>((AnyPtr&)args[5])
		));
	}
};

template<class C, class R , class A0, class A1, class A2, class A3, class A4>
struct cmemfun_holder<R (__stdcall *)(C , A0, A1, A2, A3, A4)>{
	enum{ PARAM_N = 5, METHOD = 1, EXTENDABLE = 0 };
	typedef param_types_holder5<C , A0, A1, A2, A3, A4> fun_param_holder;
	typedef R (__stdcall *fun_t)(C , A0, A1, A2, A3, A4);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		XTAL_RETURN_TO_VM((*(fun_t*)data)(
			unchecked_cast<C>((AnyPtr&)args[0])
			, unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4]), unchecked_cast<A4>((AnyPtr&)args[5])
		));
	}
};
#endif



/////

template<class R , class A0, class A1, class A2, class A3, class A4, class A5>
struct cfun_holder_base<R (*)(A0, A1, A2, A3, A4, A5)>{
	typedef R (*fun_t)(A0, A1, A2, A3, A4, A5);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args);
};

template<class R , class A0, class A1, class A2, class A3, class A4, class A5>
void cfun_holder_base<R (*)(A0, A1, A2, A3, A4, A5)>::call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
	XTAL_RETURN_TO_VM((*(fun_t*)data)(
		unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4]), unchecked_cast<A4>((AnyPtr&)args[5]), unchecked_cast<A5>((AnyPtr&)args[6])
	));
}

template<class R , class A0, class A1, class A2, class A3, class A4, class A5>
struct cfun_holder<R (*)(A0, A1, A2, A3, A4, A5)> : 
cfun_holder_base<R (*)(typename CommonType<A0>::type, typename CommonType<A1>::type, typename CommonType<A2>::type, typename CommonType<A3>::type, typename CommonType<A4>::type, typename CommonType<A5>::type)>{
	enum{ PARAM_N = 6, METHOD = 0, EXTENDABLE = 0 };
	typedef param_types_holder6<void , A0, A1, A2, A3, A4, A5> fun_param_holder;
};

/////

template<class C, class R , class A0, class A1, class A2, class A3, class A4, class A5>
struct cmemfun_holder_base<R (C::*)(A0, A1, A2, A3, A4, A5)>{
	typedef R (C::*fun_t)(A0, A1, A2, A3, A4, A5);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args);
};

template<class C, class R , class A0, class A1, class A2, class A3, class A4, class A5>
void cmemfun_holder_base<R (C::*)(A0, A1, A2, A3, A4, A5)>::call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
	XTAL_RETURN_TO_VM((unchecked_cast<C*>((AnyPtr&)args[0])->**(fun_t*)data)(
		unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4]), unchecked_cast<A4>((AnyPtr&)args[5]), unchecked_cast<A5>((AnyPtr&)args[6])
	));
}

template<class C, class R , class A0, class A1, class A2, class A3, class A4, class A5>
struct cmemfun_holder<R (C::*)(A0, A1, A2, A3, A4, A5)> : 
cmemfun_holder_base<R (CommonThisType<C>::type::*)(typename CommonType<A0>::type, typename CommonType<A1>::type, typename CommonType<A2>::type, typename CommonType<A3>::type, typename CommonType<A4>::type, typename CommonType<A5>::type)>{
	enum{ PARAM_N = 6, METHOD = 1, EXTENDABLE = 0 };
	typedef param_types_holder6<C , A0, A1, A2, A3, A4, A5> fun_param_holder;
};

template<class C, class R , class A0, class A1, class A2, class A3, class A4, class A5>
struct cmemfun_holder<R (C::*)(A0, A1, A2, A3, A4, A5) const> : 
cmemfun_holder_base<R (CommonThisType<C>::type::*)(typename CommonType<A0>::type, typename CommonType<A1>::type, typename CommonType<A2>::type, typename CommonType<A3>::type, typename CommonType<A4>::type, typename CommonType<A5>::type)>{
	enum{ PARAM_N = 6, METHOD = 1, EXTENDABLE = 0 };
	typedef param_types_holder6<C , A0, A1, A2, A3, A4, A5> fun_param_holder;
};

/////

template<class C, class R , class A0, class A1, class A2, class A3, class A4, class A5>
struct cmemfun_holder_base<R (*)(C , A0, A1, A2, A3, A4, A5)>{
	typedef R (*fun_t)(C , A0, A1, A2, A3, A4, A5);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args);
};

template<class C, class R , class A0, class A1, class A2, class A3, class A4, class A5>
void cmemfun_holder_base<R (*)(C , A0, A1, A2, A3, A4, A5)>::call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
	XTAL_RETURN_TO_VM((*(fun_t*)data)(
		unchecked_cast<C>((AnyPtr&)args[0])
		, unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4]), unchecked_cast<A4>((AnyPtr&)args[5]), unchecked_cast<A5>((AnyPtr&)args[6])
	));
}

template<class C, class R , class A0, class A1, class A2, class A3, class A4, class A5>
struct cmemfun_holder<R (*)(C , A0, A1, A2, A3, A4, A5)> :
cmemfun_holder_base<R (*)(typename CommonType<C>::type , typename CommonType<A0>::type, typename CommonType<A1>::type, typename CommonType<A2>::type, typename CommonType<A3>::type, typename CommonType<A4>::type, typename CommonType<A5>::type)>{
	enum{ PARAM_N = 6, METHOD = 1, EXTENDABLE = 0 };
	typedef param_types_holder6<C , A0, A1, A2, A3, A4, A5> fun_param_holder;
};

/////

template<class T , class A0, class A1, class A2, class A3, class A4, class A5>
struct ctor_fun<T , A0, A1, A2, A3, A4, A5>{
	enum{ PARAM_N = 6, METHOD = 0, EXTENDABLE = 0 };
	typedef param_types_holder6<void , A0, A1, A2, A3, A4, A5> fun_param_holder;
	typedef char fun_t;

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		vm->return_result(xnew<T>(
			unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4]), unchecked_cast<A4>((AnyPtr&)args[5]), unchecked_cast<A5>((AnyPtr&)args[6])
		));
	}
};

#if defined(_WIN32) && defined(_M_IX86)
template<class R , class A0, class A1, class A2, class A3, class A4, class A5>
struct cfun_holder<R (__stdcall *)(A0, A1, A2, A3, A4, A5)>{
	enum{ PARAM_N = 6, METHOD = 0, EXTENDABLE = 0 };
	typedef param_types_holder6<void , A0, A1, A2, A3, A4, A5> fun_param_holder;
	typedef R (__stdcall *fun_t)(A0, A1, A2, A3, A4, A5);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		XTAL_RETURN_TO_VM((*(fun_t*)data)(
			unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4]), unchecked_cast<A4>((AnyPtr&)args[5]), unchecked_cast<A5>((AnyPtr&)args[6])
		));
	}
};

template<class C, class R , class A0, class A1, class A2, class A3, class A4, class A5>
struct cmemfun_holder<R (__stdcall *)(C , A0, A1, A2, A3, A4, A5)>{
	enum{ PARAM_N = 6, METHOD = 1, EXTENDABLE = 0 };
	typedef param_types_holder6<C , A0, A1, A2, A3, A4, A5> fun_param_holder;
	typedef R (__stdcall *fun_t)(C , A0, A1, A2, A3, A4, A5);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		XTAL_RETURN_TO_VM((*(fun_t*)data)(
			unchecked_cast<C>((AnyPtr&)args[0])
			, unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4]), unchecked_cast<A4>((AnyPtr&)args[5]), unchecked_cast<A5>((AnyPtr&)args[6])
		));
	}
};
#endif



/////

template<class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6>
struct cfun_holder_base<R (*)(A0, A1, A2, A3, A4, A5, A6)>{
	typedef R (*fun_t)(A0, A1, A2, A3, A4, A5, A6);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args);
};

template<class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6>
void cfun_holder_base<R (*)(A0, A1, A2, A3, A4, A5, A6)>::call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
	XTAL_RETURN_TO_VM((*(fun_t*)data)(
		unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4]), unchecked_cast<A4>((AnyPtr&)args[5]), unchecked_cast<A5>((AnyPtr&)args[6]), unchecked_cast<A6>((AnyPtr&)args[7])
	));
}

template<class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6>
struct cfun_holder<R (*)(A0, A1, A2, A3, A4, A5, A6)> : 
cfun_holder_base<R (*)(typename CommonType<A0>::type, typename CommonType<A1>::type, typename CommonType<A2>::type, typename CommonType<A3>::type, typename CommonType<A4>::type, typename CommonType<A5>::type, typename CommonType<A6>::type)>{
	enum{ PARAM_N = 7, METHOD = 0, EXTENDABLE = 0 };
	typedef param_types_holder7<void , A0, A1, A2, A3, A4, A5, A6> fun_param_holder;
};

/////

template<class C, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6>
struct cmemfun_holder_base<R (C::*)(A0, A1, A2, A3, A4, A5, A6)>{
	typedef R (C::*fun_t)(A0, A1, A2, A3, A4, A5, A6);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args);
};

template<class C, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6>
void cmemfun_holder_base<R (C::*)(A0, A1, A2, A3, A4, A5, A6)>::call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
	XTAL_RETURN_TO_VM((unchecked_cast<C*>((AnyPtr&)args[0])->**(fun_t*)data)(
		unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4]), unchecked_cast<A4>((AnyPtr&)args[5]), unchecked_cast<A5>((AnyPtr&)args[6]), unchecked_cast<A6>((AnyPtr&)args[7])
	));
}

template<class C, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6>
struct cmemfun_holder<R (C::*)(A0, A1, A2, A3, A4, A5, A6)> : 
cmemfun_holder_base<R (CommonThisType<C>::type::*)(typename CommonType<A0>::type, typename CommonType<A1>::type, typename CommonType<A2>::type, typename CommonType<A3>::type, typename CommonType<A4>::type, typename CommonType<A5>::type, typename CommonType<A6>::type)>{
	enum{ PARAM_N = 7, METHOD = 1, EXTENDABLE = 0 };
	typedef param_types_holder7<C , A0, A1, A2, A3, A4, A5, A6> fun_param_holder;
};

template<class C, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6>
struct cmemfun_holder<R (C::*)(A0, A1, A2, A3, A4, A5, A6) const> : 
cmemfun_holder_base<R (CommonThisType<C>::type::*)(typename CommonType<A0>::type, typename CommonType<A1>::type, typename CommonType<A2>::type, typename CommonType<A3>::type, typename CommonType<A4>::type, typename CommonType<A5>::type, typename CommonType<A6>::type)>{
	enum{ PARAM_N = 7, METHOD = 1, EXTENDABLE = 0 };
	typedef param_types_holder7<C , A0, A1, A2, A3, A4, A5, A6> fun_param_holder;
};

/////

template<class C, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6>
struct cmemfun_holder_base<R (*)(C , A0, A1, A2, A3, A4, A5, A6)>{
	typedef R (*fun_t)(C , A0, A1, A2, A3, A4, A5, A6);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args);
};

template<class C, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6>
void cmemfun_holder_base<R (*)(C , A0, A1, A2, A3, A4, A5, A6)>::call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
	XTAL_RETURN_TO_VM((*(fun_t*)data)(
		unchecked_cast<C>((AnyPtr&)args[0])
		, unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4]), unchecked_cast<A4>((AnyPtr&)args[5]), unchecked_cast<A5>((AnyPtr&)args[6]), unchecked_cast<A6>((AnyPtr&)args[7])
	));
}

template<class C, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6>
struct cmemfun_holder<R (*)(C , A0, A1, A2, A3, A4, A5, A6)> :
cmemfun_holder_base<R (*)(typename CommonType<C>::type , typename CommonType<A0>::type, typename CommonType<A1>::type, typename CommonType<A2>::type, typename CommonType<A3>::type, typename CommonType<A4>::type, typename CommonType<A5>::type, typename CommonType<A6>::type)>{
	enum{ PARAM_N = 7, METHOD = 1, EXTENDABLE = 0 };
	typedef param_types_holder7<C , A0, A1, A2, A3, A4, A5, A6> fun_param_holder;
};

/////

template<class T , class A0, class A1, class A2, class A3, class A4, class A5, class A6>
struct ctor_fun<T , A0, A1, A2, A3, A4, A5, A6>{
	enum{ PARAM_N = 7, METHOD = 0, EXTENDABLE = 0 };
	typedef param_types_holder7<void , A0, A1, A2, A3, A4, A5, A6> fun_param_holder;
	typedef char fun_t;

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		vm->return_result(xnew<T>(
			unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4]), unchecked_cast<A4>((AnyPtr&)args[5]), unchecked_cast<A5>((AnyPtr&)args[6]), unchecked_cast<A6>((AnyPtr&)args[7])
		));
	}
};

#if defined(_WIN32) && defined(_M_IX86)
template<class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6>
struct cfun_holder<R (__stdcall *)(A0, A1, A2, A3, A4, A5, A6)>{
	enum{ PARAM_N = 7, METHOD = 0, EXTENDABLE = 0 };
	typedef param_types_holder7<void , A0, A1, A2, A3, A4, A5, A6> fun_param_holder;
	typedef R (__stdcall *fun_t)(A0, A1, A2, A3, A4, A5, A6);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		XTAL_RETURN_TO_VM((*(fun_t*)data)(
			unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4]), unchecked_cast<A4>((AnyPtr&)args[5]), unchecked_cast<A5>((AnyPtr&)args[6]), unchecked_cast<A6>((AnyPtr&)args[7])
		));
	}
};

template<class C, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6>
struct cmemfun_holder<R (__stdcall *)(C , A0, A1, A2, A3, A4, A5, A6)>{
	enum{ PARAM_N = 7, METHOD = 1, EXTENDABLE = 0 };
	typedef param_types_holder7<C , A0, A1, A2, A3, A4, A5, A6> fun_param_holder;
	typedef R (__stdcall *fun_t)(C , A0, A1, A2, A3, A4, A5, A6);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		XTAL_RETURN_TO_VM((*(fun_t*)data)(
			unchecked_cast<C>((AnyPtr&)args[0])
			, unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4]), unchecked_cast<A4>((AnyPtr&)args[5]), unchecked_cast<A5>((AnyPtr&)args[6]), unchecked_cast<A6>((AnyPtr&)args[7])
		));
	}
};
#endif



/////

template<class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7>
struct cfun_holder_base<R (*)(A0, A1, A2, A3, A4, A5, A6, A7)>{
	typedef R (*fun_t)(A0, A1, A2, A3, A4, A5, A6, A7);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args);
};

template<class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7>
void cfun_holder_base<R (*)(A0, A1, A2, A3, A4, A5, A6, A7)>::call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
	XTAL_RETURN_TO_VM((*(fun_t*)data)(
		unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4]), unchecked_cast<A4>((AnyPtr&)args[5]), unchecked_cast<A5>((AnyPtr&)args[6]), unchecked_cast<A6>((AnyPtr&)args[7]), unchecked_cast<A7>((AnyPtr&)args[8])
	));
}

template<class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7>
struct cfun_holder<R (*)(A0, A1, A2, A3, A4, A5, A6, A7)> : 
cfun_holder_base<R (*)(typename CommonType<A0>::type, typename CommonType<A1>::type, typename CommonType<A2>::type, typename CommonType<A3>::type, typename CommonType<A4>::type, typename CommonType<A5>::type, typename CommonType<A6>::type, typename CommonType<A7>::type)>{
	enum{ PARAM_N = 8, METHOD = 0, EXTENDABLE = 0 };
	typedef param_types_holder8<void , A0, A1, A2, A3, A4, A5, A6, A7> fun_param_holder;
};

/////

template<class C, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7>
struct cmemfun_holder_base<R (C::*)(A0, A1, A2, A3, A4, A5, A6, A7)>{
	typedef R (C::*fun_t)(A0, A1, A2, A3, A4, A5, A6, A7);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args);
};

template<class C, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7>
void cmemfun_holder_base<R (C::*)(A0, A1, A2, A3, A4, A5, A6, A7)>::call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
	XTAL_RETURN_TO_VM((unchecked_cast<C*>((AnyPtr&)args[0])->**(fun_t*)data)(
		unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4]), unchecked_cast<A4>((AnyPtr&)args[5]), unchecked_cast<A5>((AnyPtr&)args[6]), unchecked_cast<A6>((AnyPtr&)args[7]), unchecked_cast<A7>((AnyPtr&)args[8])
	));
}

template<class C, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7>
struct cmemfun_holder<R (C::*)(A0, A1, A2, A3, A4, A5, A6, A7)> : 
cmemfun_holder_base<R (CommonThisType<C>::type::*)(typename CommonType<A0>::type, typename CommonType<A1>::type, typename CommonType<A2>::type, typename CommonType<A3>::type, typename CommonType<A4>::type, typename CommonType<A5>::type, typename CommonType<A6>::type, typename CommonType<A7>::type)>{
	enum{ PARAM_N = 8, METHOD = 1, EXTENDABLE = 0 };
	typedef param_types_holder8<C , A0, A1, A2, A3, A4, A5, A6, A7> fun_param_holder;
};

template<class C, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7>
struct cmemfun_holder<R (C::*)(A0, A1, A2, A3, A4, A5, A6, A7) const> : 
cmemfun_holder_base<R (CommonThisType<C>::type::*)(typename CommonType<A0>::type, typename CommonType<A1>::type, typename CommonType<A2>::type, typename CommonType<A3>::type, typename CommonType<A4>::type, typename CommonType<A5>::type, typename CommonType<A6>::type, typename CommonType<A7>::type)>{
	enum{ PARAM_N = 8, METHOD = 1, EXTENDABLE = 0 };
	typedef param_types_holder8<C , A0, A1, A2, A3, A4, A5, A6, A7> fun_param_holder;
};

/////

template<class C, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7>
struct cmemfun_holder_base<R (*)(C , A0, A1, A2, A3, A4, A5, A6, A7)>{
	typedef R (*fun_t)(C , A0, A1, A2, A3, A4, A5, A6, A7);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args);
};

template<class C, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7>
void cmemfun_holder_base<R (*)(C , A0, A1, A2, A3, A4, A5, A6, A7)>::call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
	XTAL_RETURN_TO_VM((*(fun_t*)data)(
		unchecked_cast<C>((AnyPtr&)args[0])
		, unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4]), unchecked_cast<A4>((AnyPtr&)args[5]), unchecked_cast<A5>((AnyPtr&)args[6]), unchecked_cast<A6>((AnyPtr&)args[7]), unchecked_cast<A7>((AnyPtr&)args[8])
	));
}

template<class C, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7>
struct cmemfun_holder<R (*)(C , A0, A1, A2, A3, A4, A5, A6, A7)> :
cmemfun_holder_base<R (*)(typename CommonType<C>::type , typename CommonType<A0>::type, typename CommonType<A1>::type, typename CommonType<A2>::type, typename CommonType<A3>::type, typename CommonType<A4>::type, typename CommonType<A5>::type, typename CommonType<A6>::type, typename CommonType<A7>::type)>{
	enum{ PARAM_N = 8, METHOD = 1, EXTENDABLE = 0 };
	typedef param_types_holder8<C , A0, A1, A2, A3, A4, A5, A6, A7> fun_param_holder;
};

/////

template<class T , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7>
struct ctor_fun<T , A0, A1, A2, A3, A4, A5, A6, A7>{
	enum{ PARAM_N = 8, METHOD = 0, EXTENDABLE = 0 };
	typedef param_types_holder8<void , A0, A1, A2, A3, A4, A5, A6, A7> fun_param_holder;
	typedef char fun_t;

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		vm->return_result(xnew<T>(
			unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4]), unchecked_cast<A4>((AnyPtr&)args[5]), unchecked_cast<A5>((AnyPtr&)args[6]), unchecked_cast<A6>((AnyPtr&)args[7]), unchecked_cast<A7>((AnyPtr&)args[8])
		));
	}
};

#if defined(_WIN32) && defined(_M_IX86)
template<class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7>
struct cfun_holder<R (__stdcall *)(A0, A1, A2, A3, A4, A5, A6, A7)>{
	enum{ PARAM_N = 8, METHOD = 0, EXTENDABLE = 0 };
	typedef param_types_holder8<void , A0, A1, A2, A3, A4, A5, A6, A7> fun_param_holder;
	typedef R (__stdcall *fun_t)(A0, A1, A2, A3, A4, A5, A6, A7);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		XTAL_RETURN_TO_VM((*(fun_t*)data)(
			unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4]), unchecked_cast<A4>((AnyPtr&)args[5]), unchecked_cast<A5>((AnyPtr&)args[6]), unchecked_cast<A6>((AnyPtr&)args[7]), unchecked_cast<A7>((AnyPtr&)args[8])
		));
	}
};

template<class C, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7>
struct cmemfun_holder<R (__stdcall *)(C , A0, A1, A2, A3, A4, A5, A6, A7)>{
	enum{ PARAM_N = 8, METHOD = 1, EXTENDABLE = 0 };
	typedef param_types_holder8<C , A0, A1, A2, A3, A4, A5, A6, A7> fun_param_holder;
	typedef R (__stdcall *fun_t)(C , A0, A1, A2, A3, A4, A5, A6, A7);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		XTAL_RETURN_TO_VM((*(fun_t*)data)(
			unchecked_cast<C>((AnyPtr&)args[0])
			, unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4]), unchecked_cast<A4>((AnyPtr&)args[5]), unchecked_cast<A5>((AnyPtr&)args[6]), unchecked_cast<A6>((AnyPtr&)args[7]), unchecked_cast<A7>((AnyPtr&)args[8])
		));
	}
};
#endif



/////

template<class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8>
struct cfun_holder_base<R (*)(A0, A1, A2, A3, A4, A5, A6, A7, A8)>{
	typedef R (*fun_t)(A0, A1, A2, A3, A4, A5, A6, A7, A8);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args);
};

template<class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8>
void cfun_holder_base<R (*)(A0, A1, A2, A3, A4, A5, A6, A7, A8)>::call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
	XTAL_RETURN_TO_VM((*(fun_t*)data)(
		unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4]), unchecked_cast<A4>((AnyPtr&)args[5]), unchecked_cast<A5>((AnyPtr&)args[6]), unchecked_cast<A6>((AnyPtr&)args[7]), unchecked_cast<A7>((AnyPtr&)args[8]), unchecked_cast<A8>((AnyPtr&)args[9])
	));
}

template<class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8>
struct cfun_holder<R (*)(A0, A1, A2, A3, A4, A5, A6, A7, A8)> : 
cfun_holder_base<R (*)(typename CommonType<A0>::type, typename CommonType<A1>::type, typename CommonType<A2>::type, typename CommonType<A3>::type, typename CommonType<A4>::type, typename CommonType<A5>::type, typename CommonType<A6>::type, typename CommonType<A7>::type, typename CommonType<A8>::type)>{
	enum{ PARAM_N = 9, METHOD = 0, EXTENDABLE = 0 };
	typedef param_types_holder9<void , A0, A1, A2, A3, A4, A5, A6, A7, A8> fun_param_holder;
};

/////

template<class C, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8>
struct cmemfun_holder_base<R (C::*)(A0, A1, A2, A3, A4, A5, A6, A7, A8)>{
	typedef R (C::*fun_t)(A0, A1, A2, A3, A4, A5, A6, A7, A8);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args);
};

template<class C, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8>
void cmemfun_holder_base<R (C::*)(A0, A1, A2, A3, A4, A5, A6, A7, A8)>::call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
	XTAL_RETURN_TO_VM((unchecked_cast<C*>((AnyPtr&)args[0])->**(fun_t*)data)(
		unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4]), unchecked_cast<A4>((AnyPtr&)args[5]), unchecked_cast<A5>((AnyPtr&)args[6]), unchecked_cast<A6>((AnyPtr&)args[7]), unchecked_cast<A7>((AnyPtr&)args[8]), unchecked_cast<A8>((AnyPtr&)args[9])
	));
}

template<class C, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8>
struct cmemfun_holder<R (C::*)(A0, A1, A2, A3, A4, A5, A6, A7, A8)> : 
cmemfun_holder_base<R (CommonThisType<C>::type::*)(typename CommonType<A0>::type, typename CommonType<A1>::type, typename CommonType<A2>::type, typename CommonType<A3>::type, typename CommonType<A4>::type, typename CommonType<A5>::type, typename CommonType<A6>::type, typename CommonType<A7>::type, typename CommonType<A8>::type)>{
	enum{ PARAM_N = 9, METHOD = 1, EXTENDABLE = 0 };
	typedef param_types_holder9<C , A0, A1, A2, A3, A4, A5, A6, A7, A8> fun_param_holder;
};

template<class C, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8>
struct cmemfun_holder<R (C::*)(A0, A1, A2, A3, A4, A5, A6, A7, A8) const> : 
cmemfun_holder_base<R (CommonThisType<C>::type::*)(typename CommonType<A0>::type, typename CommonType<A1>::type, typename CommonType<A2>::type, typename CommonType<A3>::type, typename CommonType<A4>::type, typename CommonType<A5>::type, typename CommonType<A6>::type, typename CommonType<A7>::type, typename CommonType<A8>::type)>{
	enum{ PARAM_N = 9, METHOD = 1, EXTENDABLE = 0 };
	typedef param_types_holder9<C , A0, A1, A2, A3, A4, A5, A6, A7, A8> fun_param_holder;
};

/////

template<class C, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8>
struct cmemfun_holder_base<R (*)(C , A0, A1, A2, A3, A4, A5, A6, A7, A8)>{
	typedef R (*fun_t)(C , A0, A1, A2, A3, A4, A5, A6, A7, A8);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args);
};

template<class C, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8>
void cmemfun_holder_base<R (*)(C , A0, A1, A2, A3, A4, A5, A6, A7, A8)>::call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
	XTAL_RETURN_TO_VM((*(fun_t*)data)(
		unchecked_cast<C>((AnyPtr&)args[0])
		, unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4]), unchecked_cast<A4>((AnyPtr&)args[5]), unchecked_cast<A5>((AnyPtr&)args[6]), unchecked_cast<A6>((AnyPtr&)args[7]), unchecked_cast<A7>((AnyPtr&)args[8]), unchecked_cast<A8>((AnyPtr&)args[9])
	));
}

template<class C, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8>
struct cmemfun_holder<R (*)(C , A0, A1, A2, A3, A4, A5, A6, A7, A8)> :
cmemfun_holder_base<R (*)(typename CommonType<C>::type , typename CommonType<A0>::type, typename CommonType<A1>::type, typename CommonType<A2>::type, typename CommonType<A3>::type, typename CommonType<A4>::type, typename CommonType<A5>::type, typename CommonType<A6>::type, typename CommonType<A7>::type, typename CommonType<A8>::type)>{
	enum{ PARAM_N = 9, METHOD = 1, EXTENDABLE = 0 };
	typedef param_types_holder9<C , A0, A1, A2, A3, A4, A5, A6, A7, A8> fun_param_holder;
};

/////

template<class T , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8>
struct ctor_fun<T , A0, A1, A2, A3, A4, A5, A6, A7, A8>{
	enum{ PARAM_N = 9, METHOD = 0, EXTENDABLE = 0 };
	typedef param_types_holder9<void , A0, A1, A2, A3, A4, A5, A6, A7, A8> fun_param_holder;
	typedef char fun_t;

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		vm->return_result(xnew<T>(
			unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4]), unchecked_cast<A4>((AnyPtr&)args[5]), unchecked_cast<A5>((AnyPtr&)args[6]), unchecked_cast<A6>((AnyPtr&)args[7]), unchecked_cast<A7>((AnyPtr&)args[8]), unchecked_cast<A8>((AnyPtr&)args[9])
		));
	}
};

#if defined(_WIN32) && defined(_M_IX86)
template<class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8>
struct cfun_holder<R (__stdcall *)(A0, A1, A2, A3, A4, A5, A6, A7, A8)>{
	enum{ PARAM_N = 9, METHOD = 0, EXTENDABLE = 0 };
	typedef param_types_holder9<void , A0, A1, A2, A3, A4, A5, A6, A7, A8> fun_param_holder;
	typedef R (__stdcall *fun_t)(A0, A1, A2, A3, A4, A5, A6, A7, A8);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		XTAL_RETURN_TO_VM((*(fun_t*)data)(
			unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4]), unchecked_cast<A4>((AnyPtr&)args[5]), unchecked_cast<A5>((AnyPtr&)args[6]), unchecked_cast<A6>((AnyPtr&)args[7]), unchecked_cast<A7>((AnyPtr&)args[8]), unchecked_cast<A8>((AnyPtr&)args[9])
		));
	}
};

template<class C, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8>
struct cmemfun_holder<R (__stdcall *)(C , A0, A1, A2, A3, A4, A5, A6, A7, A8)>{
	enum{ PARAM_N = 9, METHOD = 1, EXTENDABLE = 0 };
	typedef param_types_holder9<C , A0, A1, A2, A3, A4, A5, A6, A7, A8> fun_param_holder;
	typedef R (__stdcall *fun_t)(C , A0, A1, A2, A3, A4, A5, A6, A7, A8);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		XTAL_RETURN_TO_VM((*(fun_t*)data)(
			unchecked_cast<C>((AnyPtr&)args[0])
			, unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4]), unchecked_cast<A4>((AnyPtr&)args[5]), unchecked_cast<A5>((AnyPtr&)args[6]), unchecked_cast<A6>((AnyPtr&)args[7]), unchecked_cast<A7>((AnyPtr&)args[8]), unchecked_cast<A8>((AnyPtr&)args[9])
		));
	}
};
#endif



/////

template<class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9>
struct cfun_holder_base<R (*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9)>{
	typedef R (*fun_t)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args);
};

template<class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9>
void cfun_holder_base<R (*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9)>::call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
	XTAL_RETURN_TO_VM((*(fun_t*)data)(
		unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4]), unchecked_cast<A4>((AnyPtr&)args[5]), unchecked_cast<A5>((AnyPtr&)args[6]), unchecked_cast<A6>((AnyPtr&)args[7]), unchecked_cast<A7>((AnyPtr&)args[8]), unchecked_cast<A8>((AnyPtr&)args[9]), unchecked_cast<A9>((AnyPtr&)args[10])
	));
}

template<class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9>
struct cfun_holder<R (*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9)> : 
cfun_holder_base<R (*)(typename CommonType<A0>::type, typename CommonType<A1>::type, typename CommonType<A2>::type, typename CommonType<A3>::type, typename CommonType<A4>::type, typename CommonType<A5>::type, typename CommonType<A6>::type, typename CommonType<A7>::type, typename CommonType<A8>::type, typename CommonType<A9>::type)>{
	enum{ PARAM_N = 10, METHOD = 0, EXTENDABLE = 0 };
	typedef param_types_holder10<void , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9> fun_param_holder;
};

/////

template<class C, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9>
struct cmemfun_holder_base<R (C::*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9)>{
	typedef R (C::*fun_t)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args);
};

template<class C, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9>
void cmemfun_holder_base<R (C::*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9)>::call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
	XTAL_RETURN_TO_VM((unchecked_cast<C*>((AnyPtr&)args[0])->**(fun_t*)data)(
		unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4]), unchecked_cast<A4>((AnyPtr&)args[5]), unchecked_cast<A5>((AnyPtr&)args[6]), unchecked_cast<A6>((AnyPtr&)args[7]), unchecked_cast<A7>((AnyPtr&)args[8]), unchecked_cast<A8>((AnyPtr&)args[9]), unchecked_cast<A9>((AnyPtr&)args[10])
	));
}

template<class C, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9>
struct cmemfun_holder<R (C::*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9)> : 
cmemfun_holder_base<R (CommonThisType<C>::type::*)(typename CommonType<A0>::type, typename CommonType<A1>::type, typename CommonType<A2>::type, typename CommonType<A3>::type, typename CommonType<A4>::type, typename CommonType<A5>::type, typename CommonType<A6>::type, typename CommonType<A7>::type, typename CommonType<A8>::type, typename CommonType<A9>::type)>{
	enum{ PARAM_N = 10, METHOD = 1, EXTENDABLE = 0 };
	typedef param_types_holder10<C , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9> fun_param_holder;
};

template<class C, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9>
struct cmemfun_holder<R (C::*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9) const> : 
cmemfun_holder_base<R (CommonThisType<C>::type::*)(typename CommonType<A0>::type, typename CommonType<A1>::type, typename CommonType<A2>::type, typename CommonType<A3>::type, typename CommonType<A4>::type, typename CommonType<A5>::type, typename CommonType<A6>::type, typename CommonType<A7>::type, typename CommonType<A8>::type, typename CommonType<A9>::type)>{
	enum{ PARAM_N = 10, METHOD = 1, EXTENDABLE = 0 };
	typedef param_types_holder10<C , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9> fun_param_holder;
};

/////

template<class C, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9>
struct cmemfun_holder_base<R (*)(C , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9)>{
	typedef R (*fun_t)(C , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args);
};

template<class C, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9>
void cmemfun_holder_base<R (*)(C , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9)>::call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
	XTAL_RETURN_TO_VM((*(fun_t*)data)(
		unchecked_cast<C>((AnyPtr&)args[0])
		, unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4]), unchecked_cast<A4>((AnyPtr&)args[5]), unchecked_cast<A5>((AnyPtr&)args[6]), unchecked_cast<A6>((AnyPtr&)args[7]), unchecked_cast<A7>((AnyPtr&)args[8]), unchecked_cast<A8>((AnyPtr&)args[9]), unchecked_cast<A9>((AnyPtr&)args[10])
	));
}

template<class C, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9>
struct cmemfun_holder<R (*)(C , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9)> :
cmemfun_holder_base<R (*)(typename CommonType<C>::type , typename CommonType<A0>::type, typename CommonType<A1>::type, typename CommonType<A2>::type, typename CommonType<A3>::type, typename CommonType<A4>::type, typename CommonType<A5>::type, typename CommonType<A6>::type, typename CommonType<A7>::type, typename CommonType<A8>::type, typename CommonType<A9>::type)>{
	enum{ PARAM_N = 10, METHOD = 1, EXTENDABLE = 0 };
	typedef param_types_holder10<C , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9> fun_param_holder;
};

/////

template<class T , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9>
struct ctor_fun<T , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9>{
	enum{ PARAM_N = 10, METHOD = 0, EXTENDABLE = 0 };
	typedef param_types_holder10<void , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9> fun_param_holder;
	typedef char fun_t;

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		vm->return_result(xnew<T>(
			unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4]), unchecked_cast<A4>((AnyPtr&)args[5]), unchecked_cast<A5>((AnyPtr&)args[6]), unchecked_cast<A6>((AnyPtr&)args[7]), unchecked_cast<A7>((AnyPtr&)args[8]), unchecked_cast<A8>((AnyPtr&)args[9]), unchecked_cast<A9>((AnyPtr&)args[10])
		));
	}
};

#if defined(_WIN32) && defined(_M_IX86)
template<class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9>
struct cfun_holder<R (__stdcall *)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9)>{
	enum{ PARAM_N = 10, METHOD = 0, EXTENDABLE = 0 };
	typedef param_types_holder10<void , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9> fun_param_holder;
	typedef R (__stdcall *fun_t)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		XTAL_RETURN_TO_VM((*(fun_t*)data)(
			unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4]), unchecked_cast<A4>((AnyPtr&)args[5]), unchecked_cast<A5>((AnyPtr&)args[6]), unchecked_cast<A6>((AnyPtr&)args[7]), unchecked_cast<A7>((AnyPtr&)args[8]), unchecked_cast<A8>((AnyPtr&)args[9]), unchecked_cast<A9>((AnyPtr&)args[10])
		));
	}
};

template<class C, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9>
struct cmemfun_holder<R (__stdcall *)(C , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9)>{
	enum{ PARAM_N = 10, METHOD = 1, EXTENDABLE = 0 };
	typedef param_types_holder10<C , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9> fun_param_holder;
	typedef R (__stdcall *fun_t)(C , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		XTAL_RETURN_TO_VM((*(fun_t*)data)(
			unchecked_cast<C>((AnyPtr&)args[0])
			, unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4]), unchecked_cast<A4>((AnyPtr&)args[5]), unchecked_cast<A5>((AnyPtr&)args[6]), unchecked_cast<A6>((AnyPtr&)args[7]), unchecked_cast<A7>((AnyPtr&)args[8]), unchecked_cast<A8>((AnyPtr&)args[9]), unchecked_cast<A9>((AnyPtr&)args[10])
		));
	}
};
#endif



/////

template<class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10>
struct cfun_holder_base<R (*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10)>{
	typedef R (*fun_t)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args);
};

template<class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10>
void cfun_holder_base<R (*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10)>::call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
	XTAL_RETURN_TO_VM((*(fun_t*)data)(
		unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4]), unchecked_cast<A4>((AnyPtr&)args[5]), unchecked_cast<A5>((AnyPtr&)args[6]), unchecked_cast<A6>((AnyPtr&)args[7]), unchecked_cast<A7>((AnyPtr&)args[8]), unchecked_cast<A8>((AnyPtr&)args[9]), unchecked_cast<A9>((AnyPtr&)args[10]), unchecked_cast<A10>((AnyPtr&)args[11])
	));
}

template<class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10>
struct cfun_holder<R (*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10)> : 
cfun_holder_base<R (*)(typename CommonType<A0>::type, typename CommonType<A1>::type, typename CommonType<A2>::type, typename CommonType<A3>::type, typename CommonType<A4>::type, typename CommonType<A5>::type, typename CommonType<A6>::type, typename CommonType<A7>::type, typename CommonType<A8>::type, typename CommonType<A9>::type, typename CommonType<A10>::type)>{
	enum{ PARAM_N = 11, METHOD = 0, EXTENDABLE = 0 };
	typedef param_types_holder11<void , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10> fun_param_holder;
};

/////

template<class C, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10>
struct cmemfun_holder_base<R (C::*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10)>{
	typedef R (C::*fun_t)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args);
};

template<class C, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10>
void cmemfun_holder_base<R (C::*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10)>::call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
	XTAL_RETURN_TO_VM((unchecked_cast<C*>((AnyPtr&)args[0])->**(fun_t*)data)(
		unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4]), unchecked_cast<A4>((AnyPtr&)args[5]), unchecked_cast<A5>((AnyPtr&)args[6]), unchecked_cast<A6>((AnyPtr&)args[7]), unchecked_cast<A7>((AnyPtr&)args[8]), unchecked_cast<A8>((AnyPtr&)args[9]), unchecked_cast<A9>((AnyPtr&)args[10]), unchecked_cast<A10>((AnyPtr&)args[11])
	));
}

template<class C, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10>
struct cmemfun_holder<R (C::*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10)> : 
cmemfun_holder_base<R (CommonThisType<C>::type::*)(typename CommonType<A0>::type, typename CommonType<A1>::type, typename CommonType<A2>::type, typename CommonType<A3>::type, typename CommonType<A4>::type, typename CommonType<A5>::type, typename CommonType<A6>::type, typename CommonType<A7>::type, typename CommonType<A8>::type, typename CommonType<A9>::type, typename CommonType<A10>::type)>{
	enum{ PARAM_N = 11, METHOD = 1, EXTENDABLE = 0 };
	typedef param_types_holder11<C , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10> fun_param_holder;
};

template<class C, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10>
struct cmemfun_holder<R (C::*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10) const> : 
cmemfun_holder_base<R (CommonThisType<C>::type::*)(typename CommonType<A0>::type, typename CommonType<A1>::type, typename CommonType<A2>::type, typename CommonType<A3>::type, typename CommonType<A4>::type, typename CommonType<A5>::type, typename CommonType<A6>::type, typename CommonType<A7>::type, typename CommonType<A8>::type, typename CommonType<A9>::type, typename CommonType<A10>::type)>{
	enum{ PARAM_N = 11, METHOD = 1, EXTENDABLE = 0 };
	typedef param_types_holder11<C , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10> fun_param_holder;
};

/////

template<class C, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10>
struct cmemfun_holder_base<R (*)(C , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10)>{
	typedef R (*fun_t)(C , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args);
};

template<class C, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10>
void cmemfun_holder_base<R (*)(C , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10)>::call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
	XTAL_RETURN_TO_VM((*(fun_t*)data)(
		unchecked_cast<C>((AnyPtr&)args[0])
		, unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4]), unchecked_cast<A4>((AnyPtr&)args[5]), unchecked_cast<A5>((AnyPtr&)args[6]), unchecked_cast<A6>((AnyPtr&)args[7]), unchecked_cast<A7>((AnyPtr&)args[8]), unchecked_cast<A8>((AnyPtr&)args[9]), unchecked_cast<A9>((AnyPtr&)args[10]), unchecked_cast<A10>((AnyPtr&)args[11])
	));
}

template<class C, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10>
struct cmemfun_holder<R (*)(C , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10)> :
cmemfun_holder_base<R (*)(typename CommonType<C>::type , typename CommonType<A0>::type, typename CommonType<A1>::type, typename CommonType<A2>::type, typename CommonType<A3>::type, typename CommonType<A4>::type, typename CommonType<A5>::type, typename CommonType<A6>::type, typename CommonType<A7>::type, typename CommonType<A8>::type, typename CommonType<A9>::type, typename CommonType<A10>::type)>{
	enum{ PARAM_N = 11, METHOD = 1, EXTENDABLE = 0 };
	typedef param_types_holder11<C , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10> fun_param_holder;
};

/////

template<class T , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10>
struct ctor_fun<T , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10>{
	enum{ PARAM_N = 11, METHOD = 0, EXTENDABLE = 0 };
	typedef param_types_holder11<void , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10> fun_param_holder;
	typedef char fun_t;

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		vm->return_result(xnew<T>(
			unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4]), unchecked_cast<A4>((AnyPtr&)args[5]), unchecked_cast<A5>((AnyPtr&)args[6]), unchecked_cast<A6>((AnyPtr&)args[7]), unchecked_cast<A7>((AnyPtr&)args[8]), unchecked_cast<A8>((AnyPtr&)args[9]), unchecked_cast<A9>((AnyPtr&)args[10]), unchecked_cast<A10>((AnyPtr&)args[11])
		));
	}
};

#if defined(_WIN32) && defined(_M_IX86)
template<class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10>
struct cfun_holder<R (__stdcall *)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10)>{
	enum{ PARAM_N = 11, METHOD = 0, EXTENDABLE = 0 };
	typedef param_types_holder11<void , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10> fun_param_holder;
	typedef R (__stdcall *fun_t)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		XTAL_RETURN_TO_VM((*(fun_t*)data)(
			unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4]), unchecked_cast<A4>((AnyPtr&)args[5]), unchecked_cast<A5>((AnyPtr&)args[6]), unchecked_cast<A6>((AnyPtr&)args[7]), unchecked_cast<A7>((AnyPtr&)args[8]), unchecked_cast<A8>((AnyPtr&)args[9]), unchecked_cast<A9>((AnyPtr&)args[10]), unchecked_cast<A10>((AnyPtr&)args[11])
		));
	}
};

template<class C, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10>
struct cmemfun_holder<R (__stdcall *)(C , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10)>{
	enum{ PARAM_N = 11, METHOD = 1, EXTENDABLE = 0 };
	typedef param_types_holder11<C , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10> fun_param_holder;
	typedef R (__stdcall *fun_t)(C , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		XTAL_RETURN_TO_VM((*(fun_t*)data)(
			unchecked_cast<C>((AnyPtr&)args[0])
			, unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4]), unchecked_cast<A4>((AnyPtr&)args[5]), unchecked_cast<A5>((AnyPtr&)args[6]), unchecked_cast<A6>((AnyPtr&)args[7]), unchecked_cast<A7>((AnyPtr&)args[8]), unchecked_cast<A8>((AnyPtr&)args[9]), unchecked_cast<A9>((AnyPtr&)args[10]), unchecked_cast<A10>((AnyPtr&)args[11])
		));
	}
};
#endif



/////

template<class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11>
struct cfun_holder_base<R (*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11)>{
	typedef R (*fun_t)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args);
};

template<class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11>
void cfun_holder_base<R (*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11)>::call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
	XTAL_RETURN_TO_VM((*(fun_t*)data)(
		unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4]), unchecked_cast<A4>((AnyPtr&)args[5]), unchecked_cast<A5>((AnyPtr&)args[6]), unchecked_cast<A6>((AnyPtr&)args[7]), unchecked_cast<A7>((AnyPtr&)args[8]), unchecked_cast<A8>((AnyPtr&)args[9]), unchecked_cast<A9>((AnyPtr&)args[10]), unchecked_cast<A10>((AnyPtr&)args[11]), unchecked_cast<A11>((AnyPtr&)args[12])
	));
}

template<class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11>
struct cfun_holder<R (*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11)> : 
cfun_holder_base<R (*)(typename CommonType<A0>::type, typename CommonType<A1>::type, typename CommonType<A2>::type, typename CommonType<A3>::type, typename CommonType<A4>::type, typename CommonType<A5>::type, typename CommonType<A6>::type, typename CommonType<A7>::type, typename CommonType<A8>::type, typename CommonType<A9>::type, typename CommonType<A10>::type, typename CommonType<A11>::type)>{
	enum{ PARAM_N = 12, METHOD = 0, EXTENDABLE = 0 };
	typedef param_types_holder12<void , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11> fun_param_holder;
};

/////

template<class C, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11>
struct cmemfun_holder_base<R (C::*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11)>{
	typedef R (C::*fun_t)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args);
};

template<class C, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11>
void cmemfun_holder_base<R (C::*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11)>::call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
	XTAL_RETURN_TO_VM((unchecked_cast<C*>((AnyPtr&)args[0])->**(fun_t*)data)(
		unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4]), unchecked_cast<A4>((AnyPtr&)args[5]), unchecked_cast<A5>((AnyPtr&)args[6]), unchecked_cast<A6>((AnyPtr&)args[7]), unchecked_cast<A7>((AnyPtr&)args[8]), unchecked_cast<A8>((AnyPtr&)args[9]), unchecked_cast<A9>((AnyPtr&)args[10]), unchecked_cast<A10>((AnyPtr&)args[11]), unchecked_cast<A11>((AnyPtr&)args[12])
	));
}

template<class C, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11>
struct cmemfun_holder<R (C::*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11)> : 
cmemfun_holder_base<R (CommonThisType<C>::type::*)(typename CommonType<A0>::type, typename CommonType<A1>::type, typename CommonType<A2>::type, typename CommonType<A3>::type, typename CommonType<A4>::type, typename CommonType<A5>::type, typename CommonType<A6>::type, typename CommonType<A7>::type, typename CommonType<A8>::type, typename CommonType<A9>::type, typename CommonType<A10>::type, typename CommonType<A11>::type)>{
	enum{ PARAM_N = 12, METHOD = 1, EXTENDABLE = 0 };
	typedef param_types_holder12<C , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11> fun_param_holder;
};

template<class C, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11>
struct cmemfun_holder<R (C::*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11) const> : 
cmemfun_holder_base<R (CommonThisType<C>::type::*)(typename CommonType<A0>::type, typename CommonType<A1>::type, typename CommonType<A2>::type, typename CommonType<A3>::type, typename CommonType<A4>::type, typename CommonType<A5>::type, typename CommonType<A6>::type, typename CommonType<A7>::type, typename CommonType<A8>::type, typename CommonType<A9>::type, typename CommonType<A10>::type, typename CommonType<A11>::type)>{
	enum{ PARAM_N = 12, METHOD = 1, EXTENDABLE = 0 };
	typedef param_types_holder12<C , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11> fun_param_holder;
};

/////

template<class C, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11>
struct cmemfun_holder_base<R (*)(C , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11)>{
	typedef R (*fun_t)(C , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args);
};

template<class C, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11>
void cmemfun_holder_base<R (*)(C , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11)>::call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
	XTAL_RETURN_TO_VM((*(fun_t*)data)(
		unchecked_cast<C>((AnyPtr&)args[0])
		, unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4]), unchecked_cast<A4>((AnyPtr&)args[5]), unchecked_cast<A5>((AnyPtr&)args[6]), unchecked_cast<A6>((AnyPtr&)args[7]), unchecked_cast<A7>((AnyPtr&)args[8]), unchecked_cast<A8>((AnyPtr&)args[9]), unchecked_cast<A9>((AnyPtr&)args[10]), unchecked_cast<A10>((AnyPtr&)args[11]), unchecked_cast<A11>((AnyPtr&)args[12])
	));
}

template<class C, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11>
struct cmemfun_holder<R (*)(C , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11)> :
cmemfun_holder_base<R (*)(typename CommonType<C>::type , typename CommonType<A0>::type, typename CommonType<A1>::type, typename CommonType<A2>::type, typename CommonType<A3>::type, typename CommonType<A4>::type, typename CommonType<A5>::type, typename CommonType<A6>::type, typename CommonType<A7>::type, typename CommonType<A8>::type, typename CommonType<A9>::type, typename CommonType<A10>::type, typename CommonType<A11>::type)>{
	enum{ PARAM_N = 12, METHOD = 1, EXTENDABLE = 0 };
	typedef param_types_holder12<C , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11> fun_param_holder;
};

/////

template<class T , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11>
struct ctor_fun<T , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11>{
	enum{ PARAM_N = 12, METHOD = 0, EXTENDABLE = 0 };
	typedef param_types_holder12<void , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11> fun_param_holder;
	typedef char fun_t;

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		vm->return_result(xnew<T>(
			unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4]), unchecked_cast<A4>((AnyPtr&)args[5]), unchecked_cast<A5>((AnyPtr&)args[6]), unchecked_cast<A6>((AnyPtr&)args[7]), unchecked_cast<A7>((AnyPtr&)args[8]), unchecked_cast<A8>((AnyPtr&)args[9]), unchecked_cast<A9>((AnyPtr&)args[10]), unchecked_cast<A10>((AnyPtr&)args[11]), unchecked_cast<A11>((AnyPtr&)args[12])
		));
	}
};

#if defined(_WIN32) && defined(_M_IX86)
template<class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11>
struct cfun_holder<R (__stdcall *)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11)>{
	enum{ PARAM_N = 12, METHOD = 0, EXTENDABLE = 0 };
	typedef param_types_holder12<void , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11> fun_param_holder;
	typedef R (__stdcall *fun_t)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		XTAL_RETURN_TO_VM((*(fun_t*)data)(
			unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4]), unchecked_cast<A4>((AnyPtr&)args[5]), unchecked_cast<A5>((AnyPtr&)args[6]), unchecked_cast<A6>((AnyPtr&)args[7]), unchecked_cast<A7>((AnyPtr&)args[8]), unchecked_cast<A8>((AnyPtr&)args[9]), unchecked_cast<A9>((AnyPtr&)args[10]), unchecked_cast<A10>((AnyPtr&)args[11]), unchecked_cast<A11>((AnyPtr&)args[12])
		));
	}
};

template<class C, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11>
struct cmemfun_holder<R (__stdcall *)(C , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11)>{
	enum{ PARAM_N = 12, METHOD = 1, EXTENDABLE = 0 };
	typedef param_types_holder12<C , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11> fun_param_holder;
	typedef R (__stdcall *fun_t)(C , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		XTAL_RETURN_TO_VM((*(fun_t*)data)(
			unchecked_cast<C>((AnyPtr&)args[0])
			, unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4]), unchecked_cast<A4>((AnyPtr&)args[5]), unchecked_cast<A5>((AnyPtr&)args[6]), unchecked_cast<A6>((AnyPtr&)args[7]), unchecked_cast<A7>((AnyPtr&)args[8]), unchecked_cast<A8>((AnyPtr&)args[9]), unchecked_cast<A9>((AnyPtr&)args[10]), unchecked_cast<A10>((AnyPtr&)args[11]), unchecked_cast<A11>((AnyPtr&)args[12])
		));
	}
};
#endif



/////

template<class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12>
struct cfun_holder_base<R (*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12)>{
	typedef R (*fun_t)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args);
};

template<class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12>
void cfun_holder_base<R (*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12)>::call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
	XTAL_RETURN_TO_VM((*(fun_t*)data)(
		unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4]), unchecked_cast<A4>((AnyPtr&)args[5]), unchecked_cast<A5>((AnyPtr&)args[6]), unchecked_cast<A6>((AnyPtr&)args[7]), unchecked_cast<A7>((AnyPtr&)args[8]), unchecked_cast<A8>((AnyPtr&)args[9]), unchecked_cast<A9>((AnyPtr&)args[10]), unchecked_cast<A10>((AnyPtr&)args[11]), unchecked_cast<A11>((AnyPtr&)args[12]), unchecked_cast<A12>((AnyPtr&)args[13])
	));
}

template<class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12>
struct cfun_holder<R (*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12)> : 
cfun_holder_base<R (*)(typename CommonType<A0>::type, typename CommonType<A1>::type, typename CommonType<A2>::type, typename CommonType<A3>::type, typename CommonType<A4>::type, typename CommonType<A5>::type, typename CommonType<A6>::type, typename CommonType<A7>::type, typename CommonType<A8>::type, typename CommonType<A9>::type, typename CommonType<A10>::type, typename CommonType<A11>::type, typename CommonType<A12>::type)>{
	enum{ PARAM_N = 13, METHOD = 0, EXTENDABLE = 0 };
	typedef param_types_holder13<void , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12> fun_param_holder;
};

/////

template<class C, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12>
struct cmemfun_holder_base<R (C::*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12)>{
	typedef R (C::*fun_t)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args);
};

template<class C, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12>
void cmemfun_holder_base<R (C::*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12)>::call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
	XTAL_RETURN_TO_VM((unchecked_cast<C*>((AnyPtr&)args[0])->**(fun_t*)data)(
		unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4]), unchecked_cast<A4>((AnyPtr&)args[5]), unchecked_cast<A5>((AnyPtr&)args[6]), unchecked_cast<A6>((AnyPtr&)args[7]), unchecked_cast<A7>((AnyPtr&)args[8]), unchecked_cast<A8>((AnyPtr&)args[9]), unchecked_cast<A9>((AnyPtr&)args[10]), unchecked_cast<A10>((AnyPtr&)args[11]), unchecked_cast<A11>((AnyPtr&)args[12]), unchecked_cast<A12>((AnyPtr&)args[13])
	));
}

template<class C, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12>
struct cmemfun_holder<R (C::*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12)> : 
cmemfun_holder_base<R (CommonThisType<C>::type::*)(typename CommonType<A0>::type, typename CommonType<A1>::type, typename CommonType<A2>::type, typename CommonType<A3>::type, typename CommonType<A4>::type, typename CommonType<A5>::type, typename CommonType<A6>::type, typename CommonType<A7>::type, typename CommonType<A8>::type, typename CommonType<A9>::type, typename CommonType<A10>::type, typename CommonType<A11>::type, typename CommonType<A12>::type)>{
	enum{ PARAM_N = 13, METHOD = 1, EXTENDABLE = 0 };
	typedef param_types_holder13<C , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12> fun_param_holder;
};

template<class C, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12>
struct cmemfun_holder<R (C::*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12) const> : 
cmemfun_holder_base<R (CommonThisType<C>::type::*)(typename CommonType<A0>::type, typename CommonType<A1>::type, typename CommonType<A2>::type, typename CommonType<A3>::type, typename CommonType<A4>::type, typename CommonType<A5>::type, typename CommonType<A6>::type, typename CommonType<A7>::type, typename CommonType<A8>::type, typename CommonType<A9>::type, typename CommonType<A10>::type, typename CommonType<A11>::type, typename CommonType<A12>::type)>{
	enum{ PARAM_N = 13, METHOD = 1, EXTENDABLE = 0 };
	typedef param_types_holder13<C , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12> fun_param_holder;
};

/////

template<class C, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12>
struct cmemfun_holder_base<R (*)(C , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12)>{
	typedef R (*fun_t)(C , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args);
};

template<class C, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12>
void cmemfun_holder_base<R (*)(C , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12)>::call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
	XTAL_RETURN_TO_VM((*(fun_t*)data)(
		unchecked_cast<C>((AnyPtr&)args[0])
		, unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4]), unchecked_cast<A4>((AnyPtr&)args[5]), unchecked_cast<A5>((AnyPtr&)args[6]), unchecked_cast<A6>((AnyPtr&)args[7]), unchecked_cast<A7>((AnyPtr&)args[8]), unchecked_cast<A8>((AnyPtr&)args[9]), unchecked_cast<A9>((AnyPtr&)args[10]), unchecked_cast<A10>((AnyPtr&)args[11]), unchecked_cast<A11>((AnyPtr&)args[12]), unchecked_cast<A12>((AnyPtr&)args[13])
	));
}

template<class C, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12>
struct cmemfun_holder<R (*)(C , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12)> :
cmemfun_holder_base<R (*)(typename CommonType<C>::type , typename CommonType<A0>::type, typename CommonType<A1>::type, typename CommonType<A2>::type, typename CommonType<A3>::type, typename CommonType<A4>::type, typename CommonType<A5>::type, typename CommonType<A6>::type, typename CommonType<A7>::type, typename CommonType<A8>::type, typename CommonType<A9>::type, typename CommonType<A10>::type, typename CommonType<A11>::type, typename CommonType<A12>::type)>{
	enum{ PARAM_N = 13, METHOD = 1, EXTENDABLE = 0 };
	typedef param_types_holder13<C , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12> fun_param_holder;
};

/////

template<class T , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12>
struct ctor_fun<T , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12>{
	enum{ PARAM_N = 13, METHOD = 0, EXTENDABLE = 0 };
	typedef param_types_holder13<void , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12> fun_param_holder;
	typedef char fun_t;

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		vm->return_result(xnew<T>(
			unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4]), unchecked_cast<A4>((AnyPtr&)args[5]), unchecked_cast<A5>((AnyPtr&)args[6]), unchecked_cast<A6>((AnyPtr&)args[7]), unchecked_cast<A7>((AnyPtr&)args[8]), unchecked_cast<A8>((AnyPtr&)args[9]), unchecked_cast<A9>((AnyPtr&)args[10]), unchecked_cast<A10>((AnyPtr&)args[11]), unchecked_cast<A11>((AnyPtr&)args[12]), unchecked_cast<A12>((AnyPtr&)args[13])
		));
	}
};

#if defined(_WIN32) && defined(_M_IX86)
template<class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12>
struct cfun_holder<R (__stdcall *)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12)>{
	enum{ PARAM_N = 13, METHOD = 0, EXTENDABLE = 0 };
	typedef param_types_holder13<void , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12> fun_param_holder;
	typedef R (__stdcall *fun_t)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		XTAL_RETURN_TO_VM((*(fun_t*)data)(
			unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4]), unchecked_cast<A4>((AnyPtr&)args[5]), unchecked_cast<A5>((AnyPtr&)args[6]), unchecked_cast<A6>((AnyPtr&)args[7]), unchecked_cast<A7>((AnyPtr&)args[8]), unchecked_cast<A8>((AnyPtr&)args[9]), unchecked_cast<A9>((AnyPtr&)args[10]), unchecked_cast<A10>((AnyPtr&)args[11]), unchecked_cast<A11>((AnyPtr&)args[12]), unchecked_cast<A12>((AnyPtr&)args[13])
		));
	}
};

template<class C, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12>
struct cmemfun_holder<R (__stdcall *)(C , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12)>{
	enum{ PARAM_N = 13, METHOD = 1, EXTENDABLE = 0 };
	typedef param_types_holder13<C , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12> fun_param_holder;
	typedef R (__stdcall *fun_t)(C , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		XTAL_RETURN_TO_VM((*(fun_t*)data)(
			unchecked_cast<C>((AnyPtr&)args[0])
			, unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4]), unchecked_cast<A4>((AnyPtr&)args[5]), unchecked_cast<A5>((AnyPtr&)args[6]), unchecked_cast<A6>((AnyPtr&)args[7]), unchecked_cast<A7>((AnyPtr&)args[8]), unchecked_cast<A8>((AnyPtr&)args[9]), unchecked_cast<A9>((AnyPtr&)args[10]), unchecked_cast<A10>((AnyPtr&)args[11]), unchecked_cast<A11>((AnyPtr&)args[12]), unchecked_cast<A12>((AnyPtr&)args[13])
		));
	}
};
#endif


//}}REPEAT}

template<>
struct cfun_holder<void (*)(const VMachinePtr&)>{
	enum{ PARAM_N = 0, METHOD = 0, EXTENDABLE = 1 };
	typedef param_types_holder0<void > fun_param_holder;
	typedef void (*fun_t)(const VMachinePtr&);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		(*(fun_t*)data)(vm);
	}
};

template<class T>
struct ctor_fun<T , const VMachinePtr&>{
	enum{ PARAM_N = 0, METHOD = 0, EXTENDABLE = 1 };
	typedef param_types_holder0<void > fun_param_holder;
	typedef char fun_t;

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		vm->return_result(xnew<T>(vm));
	}
};

template<>
struct cmemfun_holder<void (*)(const VMachinePtr&)>{
	enum{ PARAM_N = 0, METHOD = 1, EXTENDABLE = 1 };
	typedef param_types_holder0<void > fun_param_holder;
	typedef void (*fun_t)(const VMachinePtr&);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		(*(fun_t*)data)(vm);
	}
};

template<class C>
struct cmemfun_holder<void (*)(C, const VMachinePtr&)>{
	enum{ PARAM_N = 0, METHOD = 1, EXTENDABLE = 1 };
	typedef param_types_holder1<C, void > fun_param_holder;
	typedef void (*fun_t)(C, const VMachinePtr&);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		(*(fun_t*)data)(unchecked_cast<C>((AnyPtr&)args[0]), vm);
	}
};

template<class C>
struct cmemfun_holder<void (C::*)(const VMachinePtr&)>{
	enum{ PARAM_N = 0, METHOD = 1, EXTENDABLE = 1 };
	typedef param_types_holder1<C, void > fun_param_holder;
	typedef void (C::*fun_t)(const VMachinePtr&);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		(unchecked_cast<C*>((AnyPtr&)args[0])->**(fun_t*)data)(vm);
	}
};

template<class C>
struct cmemfun_holder<void (C::*)(const VMachinePtr&) const>{
	enum{ PARAM_N = 0, METHOD = 1, EXTENDABLE = 1 };
	typedef param_types_holder1<C, void > fun_param_holder;
	typedef void (C::*fun_t)(const VMachinePtr&) const;

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		(unchecked_cast<C*>((AnyPtr&)args[0])->**(fun_t*)data)(vm);
	}
};

///////////////////////////////////////////////////////

template<class C, class T>
struct getter_holder{
	enum{ PARAM_N = 0, METHOD = 1, EXTENDABLE = 0 };
	typedef param_types_holder0<C> fun_param_holder;
	typedef T C::* fun_t;
	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		T& v = unchecked_cast<C*>((AnyPtr&)args[0])->*(*(fun_t*)data);
		vm->return_result(&v);
	}
};

template<class C, class T>
struct getter_holder<C, SmartPtr<T> >{
	enum{ PARAM_N = 0, METHOD = 1, EXTENDABLE = 0 };
	typedef param_types_holder0<C> fun_param_holder;
	typedef SmartPtr<T> C::* fun_t;
	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		SmartPtr<T>& v = unchecked_cast<C*>((AnyPtr&)args[0])->*(*(fun_t*)data);
		vm->return_result(v);
	}
};

template<class C, class T>
struct setter_holder{
	enum{ PARAM_N = 1, METHOD = 1, EXTENDABLE = 0 };
	typedef param_types_holder1<C, T> fun_param_holder;
	typedef T C::* fun_t;
	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		T& v = unchecked_cast<C*>((AnyPtr&)args[0])->*(*(fun_t*)data);
		v = unchecked_cast<T>((AnyPtr&)args[1]);	
		vm->return_result(&v);
	}
};

template<class C, class T>
struct setter_holder<C, SmartPtr<T> >{
	enum{ PARAM_N = 1, METHOD = 1, EXTENDABLE = 0 };
	typedef param_types_holder1<C, T> fun_param_holder;
	typedef SmartPtr<T> C::* fun_t;
	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		SmartPtr<T>& v = unchecked_cast<C*>((AnyPtr&)args[0])->*(*(fun_t*)data);
		v = unchecked_cast<SmartPtr<T> >((AnyPtr&)args[1]);	
		vm->return_result(v);
	}
};

}

#endif
