/** \file src/xtal/xtal_bind.h
* \brief src/xtal/xtal_bind.h
*/

#ifndef XTAL_BIND_H_INCLUDE_GUARD
#define XTAL_BIND_H_INCLUDE_GUARD

#pragma once

namespace xtal{

template<class C = void >
struct param_types_holderM1;

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

struct AM1{};

//{REPEAT{{
/*
template<class TFun, int Method, class R #COMMA_REPEAT#class A`i`#>
struct nfun_base`n`{
	typedef TFun fun_type;
	typedef typename If<Method,
		param_types_holder`n-1`<#REPEAT_COMMA#A`i`#>,
		param_types_holder`n`<Any #COMMA_REPEAT#A`i`#>
		>::type fun_param_holder;
	enum{ extendable = (Method==1 && `n`==1) ? 0 : IsSame<Arguments, typename CppClassSymbol<A`n-1`>::type>::value };
	enum{ ArgBase = 1-Method, method = Method, arity = `n`-Method, vm = 0 };
	static void call(VMachine* vm, UninitializedAny* args, const void* data){ 
		ReturnResult<R>::call(vm,
			(*(fun_type*)data)(
				#REPEAT_COMMA#unchecked_cast<A`i`>((AnyPtr&)args[ArgBase+`i`])#
			)
		);
	}
};

template<class TFun, int Method #COMMA_REPEAT#class A`i`#>
struct nfun_base`n`<TFun, Method, void #COMMA_REPEAT#A`i`#>{
	typedef TFun fun_type;
	typedef typename If<Method,
		param_types_holder`n-1`<#REPEAT_COMMA#A`i`#>,
		param_types_holder`n`<Any #COMMA_REPEAT#A`i`#>
		>::type fun_param_holder;
	enum{ extendable = (Method==1 && `n`==1) ? 0 : IsSame<Arguments, typename CppClassSymbol<A`n-1`>::type>::value };
	enum{ ArgBase = 1-Method, method = Method, arity = `n`-Method, vm = 0 };
	static void call(VMachine* vm, UninitializedAny* args, const void* data){ 
		(*(fun_type*)data)(
			#REPEAT_COMMA#unchecked_cast<A`i`>((AnyPtr&)args[ArgBase+`i`])#
		);
		vm->return_result();
	}
};

template<class TFun, int Method, class R>
struct nfun_base<`n`, TFun, Method, R>
	: nfun_base`n`<TFun, Method, R #COMMA_REPEAT#typename TFun::arg`i+1`_type#>{};

*/

template<class TFun, int Method, class R >
struct nfun_base0{
	typedef TFun fun_type;
	typedef typename If<Method,
		param_types_holderM1<>,
		param_types_holder0<Any >
		>::type fun_param_holder;
	enum{ extendable = (Method==1 && 0==1) ? 0 : IsSame<Arguments, typename CppClassSymbol<AM1>::type>::value };
	enum{ ArgBase = 1-Method, method = Method, arity = 0-Method, vm = 0 };
	static void call(VMachine* vm, UninitializedAny* args, const void* data){ 
		ReturnResult<R>::call(vm,
			(*(fun_type*)data)(
				
			)
		);
	}
};

template<class TFun, int Method >
struct nfun_base0<TFun, Method, void >{
	typedef TFun fun_type;
	typedef typename If<Method,
		param_types_holderM1<>,
		param_types_holder0<Any >
		>::type fun_param_holder;
	enum{ extendable = (Method==1 && 0==1) ? 0 : IsSame<Arguments, typename CppClassSymbol<AM1>::type>::value };
	enum{ ArgBase = 1-Method, method = Method, arity = 0-Method, vm = 0 };
	static void call(VMachine* vm, UninitializedAny* args, const void* data){ 
		(*(fun_type*)data)(
			
		);
		vm->return_result();
	}
};

template<class TFun, int Method, class R>
struct nfun_base<0, TFun, Method, R>
	: nfun_base0<TFun, Method, R >{};


template<class TFun, int Method, class R , class A0>
struct nfun_base1{
	typedef TFun fun_type;
	typedef typename If<Method,
		param_types_holder0<A0>,
		param_types_holder1<Any , A0>
		>::type fun_param_holder;
	enum{ extendable = (Method==1 && 1==1) ? 0 : IsSame<Arguments, typename CppClassSymbol<A0>::type>::value };
	enum{ ArgBase = 1-Method, method = Method, arity = 1-Method, vm = 0 };
	static void call(VMachine* vm, UninitializedAny* args, const void* data){ 
		ReturnResult<R>::call(vm,
			(*(fun_type*)data)(
				unchecked_cast<A0>((AnyPtr&)args[ArgBase+0])
			)
		);
	}
};

template<class TFun, int Method , class A0>
struct nfun_base1<TFun, Method, void , A0>{
	typedef TFun fun_type;
	typedef typename If<Method,
		param_types_holder0<A0>,
		param_types_holder1<Any , A0>
		>::type fun_param_holder;
	enum{ extendable = (Method==1 && 1==1) ? 0 : IsSame<Arguments, typename CppClassSymbol<A0>::type>::value };
	enum{ ArgBase = 1-Method, method = Method, arity = 1-Method, vm = 0 };
	static void call(VMachine* vm, UninitializedAny* args, const void* data){ 
		(*(fun_type*)data)(
			unchecked_cast<A0>((AnyPtr&)args[ArgBase+0])
		);
		vm->return_result();
	}
};

template<class TFun, int Method, class R>
struct nfun_base<1, TFun, Method, R>
	: nfun_base1<TFun, Method, R , typename TFun::arg1_type>{};


template<class TFun, int Method, class R , class A0, class A1>
struct nfun_base2{
	typedef TFun fun_type;
	typedef typename If<Method,
		param_types_holder1<A0, A1>,
		param_types_holder2<Any , A0, A1>
		>::type fun_param_holder;
	enum{ extendable = (Method==1 && 2==1) ? 0 : IsSame<Arguments, typename CppClassSymbol<A1>::type>::value };
	enum{ ArgBase = 1-Method, method = Method, arity = 2-Method, vm = 0 };
	static void call(VMachine* vm, UninitializedAny* args, const void* data){ 
		ReturnResult<R>::call(vm,
			(*(fun_type*)data)(
				unchecked_cast<A0>((AnyPtr&)args[ArgBase+0]), unchecked_cast<A1>((AnyPtr&)args[ArgBase+1])
			)
		);
	}
};

template<class TFun, int Method , class A0, class A1>
struct nfun_base2<TFun, Method, void , A0, A1>{
	typedef TFun fun_type;
	typedef typename If<Method,
		param_types_holder1<A0, A1>,
		param_types_holder2<Any , A0, A1>
		>::type fun_param_holder;
	enum{ extendable = (Method==1 && 2==1) ? 0 : IsSame<Arguments, typename CppClassSymbol<A1>::type>::value };
	enum{ ArgBase = 1-Method, method = Method, arity = 2-Method, vm = 0 };
	static void call(VMachine* vm, UninitializedAny* args, const void* data){ 
		(*(fun_type*)data)(
			unchecked_cast<A0>((AnyPtr&)args[ArgBase+0]), unchecked_cast<A1>((AnyPtr&)args[ArgBase+1])
		);
		vm->return_result();
	}
};

template<class TFun, int Method, class R>
struct nfun_base<2, TFun, Method, R>
	: nfun_base2<TFun, Method, R , typename TFun::arg1_type, typename TFun::arg2_type>{};


template<class TFun, int Method, class R , class A0, class A1, class A2>
struct nfun_base3{
	typedef TFun fun_type;
	typedef typename If<Method,
		param_types_holder2<A0, A1, A2>,
		param_types_holder3<Any , A0, A1, A2>
		>::type fun_param_holder;
	enum{ extendable = (Method==1 && 3==1) ? 0 : IsSame<Arguments, typename CppClassSymbol<A2>::type>::value };
	enum{ ArgBase = 1-Method, method = Method, arity = 3-Method, vm = 0 };
	static void call(VMachine* vm, UninitializedAny* args, const void* data){ 
		ReturnResult<R>::call(vm,
			(*(fun_type*)data)(
				unchecked_cast<A0>((AnyPtr&)args[ArgBase+0]), unchecked_cast<A1>((AnyPtr&)args[ArgBase+1]), unchecked_cast<A2>((AnyPtr&)args[ArgBase+2])
			)
		);
	}
};

template<class TFun, int Method , class A0, class A1, class A2>
struct nfun_base3<TFun, Method, void , A0, A1, A2>{
	typedef TFun fun_type;
	typedef typename If<Method,
		param_types_holder2<A0, A1, A2>,
		param_types_holder3<Any , A0, A1, A2>
		>::type fun_param_holder;
	enum{ extendable = (Method==1 && 3==1) ? 0 : IsSame<Arguments, typename CppClassSymbol<A2>::type>::value };
	enum{ ArgBase = 1-Method, method = Method, arity = 3-Method, vm = 0 };
	static void call(VMachine* vm, UninitializedAny* args, const void* data){ 
		(*(fun_type*)data)(
			unchecked_cast<A0>((AnyPtr&)args[ArgBase+0]), unchecked_cast<A1>((AnyPtr&)args[ArgBase+1]), unchecked_cast<A2>((AnyPtr&)args[ArgBase+2])
		);
		vm->return_result();
	}
};

template<class TFun, int Method, class R>
struct nfun_base<3, TFun, Method, R>
	: nfun_base3<TFun, Method, R , typename TFun::arg1_type, typename TFun::arg2_type, typename TFun::arg3_type>{};


template<class TFun, int Method, class R , class A0, class A1, class A2, class A3>
struct nfun_base4{
	typedef TFun fun_type;
	typedef typename If<Method,
		param_types_holder3<A0, A1, A2, A3>,
		param_types_holder4<Any , A0, A1, A2, A3>
		>::type fun_param_holder;
	enum{ extendable = (Method==1 && 4==1) ? 0 : IsSame<Arguments, typename CppClassSymbol<A3>::type>::value };
	enum{ ArgBase = 1-Method, method = Method, arity = 4-Method, vm = 0 };
	static void call(VMachine* vm, UninitializedAny* args, const void* data){ 
		ReturnResult<R>::call(vm,
			(*(fun_type*)data)(
				unchecked_cast<A0>((AnyPtr&)args[ArgBase+0]), unchecked_cast<A1>((AnyPtr&)args[ArgBase+1]), unchecked_cast<A2>((AnyPtr&)args[ArgBase+2]), unchecked_cast<A3>((AnyPtr&)args[ArgBase+3])
			)
		);
	}
};

template<class TFun, int Method , class A0, class A1, class A2, class A3>
struct nfun_base4<TFun, Method, void , A0, A1, A2, A3>{
	typedef TFun fun_type;
	typedef typename If<Method,
		param_types_holder3<A0, A1, A2, A3>,
		param_types_holder4<Any , A0, A1, A2, A3>
		>::type fun_param_holder;
	enum{ extendable = (Method==1 && 4==1) ? 0 : IsSame<Arguments, typename CppClassSymbol<A3>::type>::value };
	enum{ ArgBase = 1-Method, method = Method, arity = 4-Method, vm = 0 };
	static void call(VMachine* vm, UninitializedAny* args, const void* data){ 
		(*(fun_type*)data)(
			unchecked_cast<A0>((AnyPtr&)args[ArgBase+0]), unchecked_cast<A1>((AnyPtr&)args[ArgBase+1]), unchecked_cast<A2>((AnyPtr&)args[ArgBase+2]), unchecked_cast<A3>((AnyPtr&)args[ArgBase+3])
		);
		vm->return_result();
	}
};

template<class TFun, int Method, class R>
struct nfun_base<4, TFun, Method, R>
	: nfun_base4<TFun, Method, R , typename TFun::arg1_type, typename TFun::arg2_type, typename TFun::arg3_type, typename TFun::arg4_type>{};


template<class TFun, int Method, class R , class A0, class A1, class A2, class A3, class A4>
struct nfun_base5{
	typedef TFun fun_type;
	typedef typename If<Method,
		param_types_holder4<A0, A1, A2, A3, A4>,
		param_types_holder5<Any , A0, A1, A2, A3, A4>
		>::type fun_param_holder;
	enum{ extendable = (Method==1 && 5==1) ? 0 : IsSame<Arguments, typename CppClassSymbol<A4>::type>::value };
	enum{ ArgBase = 1-Method, method = Method, arity = 5-Method, vm = 0 };
	static void call(VMachine* vm, UninitializedAny* args, const void* data){ 
		ReturnResult<R>::call(vm,
			(*(fun_type*)data)(
				unchecked_cast<A0>((AnyPtr&)args[ArgBase+0]), unchecked_cast<A1>((AnyPtr&)args[ArgBase+1]), unchecked_cast<A2>((AnyPtr&)args[ArgBase+2]), unchecked_cast<A3>((AnyPtr&)args[ArgBase+3]), unchecked_cast<A4>((AnyPtr&)args[ArgBase+4])
			)
		);
	}
};

template<class TFun, int Method , class A0, class A1, class A2, class A3, class A4>
struct nfun_base5<TFun, Method, void , A0, A1, A2, A3, A4>{
	typedef TFun fun_type;
	typedef typename If<Method,
		param_types_holder4<A0, A1, A2, A3, A4>,
		param_types_holder5<Any , A0, A1, A2, A3, A4>
		>::type fun_param_holder;
	enum{ extendable = (Method==1 && 5==1) ? 0 : IsSame<Arguments, typename CppClassSymbol<A4>::type>::value };
	enum{ ArgBase = 1-Method, method = Method, arity = 5-Method, vm = 0 };
	static void call(VMachine* vm, UninitializedAny* args, const void* data){ 
		(*(fun_type*)data)(
			unchecked_cast<A0>((AnyPtr&)args[ArgBase+0]), unchecked_cast<A1>((AnyPtr&)args[ArgBase+1]), unchecked_cast<A2>((AnyPtr&)args[ArgBase+2]), unchecked_cast<A3>((AnyPtr&)args[ArgBase+3]), unchecked_cast<A4>((AnyPtr&)args[ArgBase+4])
		);
		vm->return_result();
	}
};

template<class TFun, int Method, class R>
struct nfun_base<5, TFun, Method, R>
	: nfun_base5<TFun, Method, R , typename TFun::arg1_type, typename TFun::arg2_type, typename TFun::arg3_type, typename TFun::arg4_type, typename TFun::arg5_type>{};


template<class TFun, int Method, class R , class A0, class A1, class A2, class A3, class A4, class A5>
struct nfun_base6{
	typedef TFun fun_type;
	typedef typename If<Method,
		param_types_holder5<A0, A1, A2, A3, A4, A5>,
		param_types_holder6<Any , A0, A1, A2, A3, A4, A5>
		>::type fun_param_holder;
	enum{ extendable = (Method==1 && 6==1) ? 0 : IsSame<Arguments, typename CppClassSymbol<A5>::type>::value };
	enum{ ArgBase = 1-Method, method = Method, arity = 6-Method, vm = 0 };
	static void call(VMachine* vm, UninitializedAny* args, const void* data){ 
		ReturnResult<R>::call(vm,
			(*(fun_type*)data)(
				unchecked_cast<A0>((AnyPtr&)args[ArgBase+0]), unchecked_cast<A1>((AnyPtr&)args[ArgBase+1]), unchecked_cast<A2>((AnyPtr&)args[ArgBase+2]), unchecked_cast<A3>((AnyPtr&)args[ArgBase+3]), unchecked_cast<A4>((AnyPtr&)args[ArgBase+4]), unchecked_cast<A5>((AnyPtr&)args[ArgBase+5])
			)
		);
	}
};

template<class TFun, int Method , class A0, class A1, class A2, class A3, class A4, class A5>
struct nfun_base6<TFun, Method, void , A0, A1, A2, A3, A4, A5>{
	typedef TFun fun_type;
	typedef typename If<Method,
		param_types_holder5<A0, A1, A2, A3, A4, A5>,
		param_types_holder6<Any , A0, A1, A2, A3, A4, A5>
		>::type fun_param_holder;
	enum{ extendable = (Method==1 && 6==1) ? 0 : IsSame<Arguments, typename CppClassSymbol<A5>::type>::value };
	enum{ ArgBase = 1-Method, method = Method, arity = 6-Method, vm = 0 };
	static void call(VMachine* vm, UninitializedAny* args, const void* data){ 
		(*(fun_type*)data)(
			unchecked_cast<A0>((AnyPtr&)args[ArgBase+0]), unchecked_cast<A1>((AnyPtr&)args[ArgBase+1]), unchecked_cast<A2>((AnyPtr&)args[ArgBase+2]), unchecked_cast<A3>((AnyPtr&)args[ArgBase+3]), unchecked_cast<A4>((AnyPtr&)args[ArgBase+4]), unchecked_cast<A5>((AnyPtr&)args[ArgBase+5])
		);
		vm->return_result();
	}
};

template<class TFun, int Method, class R>
struct nfun_base<6, TFun, Method, R>
	: nfun_base6<TFun, Method, R , typename TFun::arg1_type, typename TFun::arg2_type, typename TFun::arg3_type, typename TFun::arg4_type, typename TFun::arg5_type, typename TFun::arg6_type>{};


template<class TFun, int Method, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6>
struct nfun_base7{
	typedef TFun fun_type;
	typedef typename If<Method,
		param_types_holder6<A0, A1, A2, A3, A4, A5, A6>,
		param_types_holder7<Any , A0, A1, A2, A3, A4, A5, A6>
		>::type fun_param_holder;
	enum{ extendable = (Method==1 && 7==1) ? 0 : IsSame<Arguments, typename CppClassSymbol<A6>::type>::value };
	enum{ ArgBase = 1-Method, method = Method, arity = 7-Method, vm = 0 };
	static void call(VMachine* vm, UninitializedAny* args, const void* data){ 
		ReturnResult<R>::call(vm,
			(*(fun_type*)data)(
				unchecked_cast<A0>((AnyPtr&)args[ArgBase+0]), unchecked_cast<A1>((AnyPtr&)args[ArgBase+1]), unchecked_cast<A2>((AnyPtr&)args[ArgBase+2]), unchecked_cast<A3>((AnyPtr&)args[ArgBase+3]), unchecked_cast<A4>((AnyPtr&)args[ArgBase+4]), unchecked_cast<A5>((AnyPtr&)args[ArgBase+5]), unchecked_cast<A6>((AnyPtr&)args[ArgBase+6])
			)
		);
	}
};

template<class TFun, int Method , class A0, class A1, class A2, class A3, class A4, class A5, class A6>
struct nfun_base7<TFun, Method, void , A0, A1, A2, A3, A4, A5, A6>{
	typedef TFun fun_type;
	typedef typename If<Method,
		param_types_holder6<A0, A1, A2, A3, A4, A5, A6>,
		param_types_holder7<Any , A0, A1, A2, A3, A4, A5, A6>
		>::type fun_param_holder;
	enum{ extendable = (Method==1 && 7==1) ? 0 : IsSame<Arguments, typename CppClassSymbol<A6>::type>::value };
	enum{ ArgBase = 1-Method, method = Method, arity = 7-Method, vm = 0 };
	static void call(VMachine* vm, UninitializedAny* args, const void* data){ 
		(*(fun_type*)data)(
			unchecked_cast<A0>((AnyPtr&)args[ArgBase+0]), unchecked_cast<A1>((AnyPtr&)args[ArgBase+1]), unchecked_cast<A2>((AnyPtr&)args[ArgBase+2]), unchecked_cast<A3>((AnyPtr&)args[ArgBase+3]), unchecked_cast<A4>((AnyPtr&)args[ArgBase+4]), unchecked_cast<A5>((AnyPtr&)args[ArgBase+5]), unchecked_cast<A6>((AnyPtr&)args[ArgBase+6])
		);
		vm->return_result();
	}
};

template<class TFun, int Method, class R>
struct nfun_base<7, TFun, Method, R>
	: nfun_base7<TFun, Method, R , typename TFun::arg1_type, typename TFun::arg2_type, typename TFun::arg3_type, typename TFun::arg4_type, typename TFun::arg5_type, typename TFun::arg6_type, typename TFun::arg7_type>{};


template<class TFun, int Method, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7>
struct nfun_base8{
	typedef TFun fun_type;
	typedef typename If<Method,
		param_types_holder7<A0, A1, A2, A3, A4, A5, A6, A7>,
		param_types_holder8<Any , A0, A1, A2, A3, A4, A5, A6, A7>
		>::type fun_param_holder;
	enum{ extendable = (Method==1 && 8==1) ? 0 : IsSame<Arguments, typename CppClassSymbol<A7>::type>::value };
	enum{ ArgBase = 1-Method, method = Method, arity = 8-Method, vm = 0 };
	static void call(VMachine* vm, UninitializedAny* args, const void* data){ 
		ReturnResult<R>::call(vm,
			(*(fun_type*)data)(
				unchecked_cast<A0>((AnyPtr&)args[ArgBase+0]), unchecked_cast<A1>((AnyPtr&)args[ArgBase+1]), unchecked_cast<A2>((AnyPtr&)args[ArgBase+2]), unchecked_cast<A3>((AnyPtr&)args[ArgBase+3]), unchecked_cast<A4>((AnyPtr&)args[ArgBase+4]), unchecked_cast<A5>((AnyPtr&)args[ArgBase+5]), unchecked_cast<A6>((AnyPtr&)args[ArgBase+6]), unchecked_cast<A7>((AnyPtr&)args[ArgBase+7])
			)
		);
	}
};

template<class TFun, int Method , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7>
struct nfun_base8<TFun, Method, void , A0, A1, A2, A3, A4, A5, A6, A7>{
	typedef TFun fun_type;
	typedef typename If<Method,
		param_types_holder7<A0, A1, A2, A3, A4, A5, A6, A7>,
		param_types_holder8<Any , A0, A1, A2, A3, A4, A5, A6, A7>
		>::type fun_param_holder;
	enum{ extendable = (Method==1 && 8==1) ? 0 : IsSame<Arguments, typename CppClassSymbol<A7>::type>::value };
	enum{ ArgBase = 1-Method, method = Method, arity = 8-Method, vm = 0 };
	static void call(VMachine* vm, UninitializedAny* args, const void* data){ 
		(*(fun_type*)data)(
			unchecked_cast<A0>((AnyPtr&)args[ArgBase+0]), unchecked_cast<A1>((AnyPtr&)args[ArgBase+1]), unchecked_cast<A2>((AnyPtr&)args[ArgBase+2]), unchecked_cast<A3>((AnyPtr&)args[ArgBase+3]), unchecked_cast<A4>((AnyPtr&)args[ArgBase+4]), unchecked_cast<A5>((AnyPtr&)args[ArgBase+5]), unchecked_cast<A6>((AnyPtr&)args[ArgBase+6]), unchecked_cast<A7>((AnyPtr&)args[ArgBase+7])
		);
		vm->return_result();
	}
};

template<class TFun, int Method, class R>
struct nfun_base<8, TFun, Method, R>
	: nfun_base8<TFun, Method, R , typename TFun::arg1_type, typename TFun::arg2_type, typename TFun::arg3_type, typename TFun::arg4_type, typename TFun::arg5_type, typename TFun::arg6_type, typename TFun::arg7_type, typename TFun::arg8_type>{};


template<class TFun, int Method, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8>
struct nfun_base9{
	typedef TFun fun_type;
	typedef typename If<Method,
		param_types_holder8<A0, A1, A2, A3, A4, A5, A6, A7, A8>,
		param_types_holder9<Any , A0, A1, A2, A3, A4, A5, A6, A7, A8>
		>::type fun_param_holder;
	enum{ extendable = (Method==1 && 9==1) ? 0 : IsSame<Arguments, typename CppClassSymbol<A8>::type>::value };
	enum{ ArgBase = 1-Method, method = Method, arity = 9-Method, vm = 0 };
	static void call(VMachine* vm, UninitializedAny* args, const void* data){ 
		ReturnResult<R>::call(vm,
			(*(fun_type*)data)(
				unchecked_cast<A0>((AnyPtr&)args[ArgBase+0]), unchecked_cast<A1>((AnyPtr&)args[ArgBase+1]), unchecked_cast<A2>((AnyPtr&)args[ArgBase+2]), unchecked_cast<A3>((AnyPtr&)args[ArgBase+3]), unchecked_cast<A4>((AnyPtr&)args[ArgBase+4]), unchecked_cast<A5>((AnyPtr&)args[ArgBase+5]), unchecked_cast<A6>((AnyPtr&)args[ArgBase+6]), unchecked_cast<A7>((AnyPtr&)args[ArgBase+7]), unchecked_cast<A8>((AnyPtr&)args[ArgBase+8])
			)
		);
	}
};

template<class TFun, int Method , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8>
struct nfun_base9<TFun, Method, void , A0, A1, A2, A3, A4, A5, A6, A7, A8>{
	typedef TFun fun_type;
	typedef typename If<Method,
		param_types_holder8<A0, A1, A2, A3, A4, A5, A6, A7, A8>,
		param_types_holder9<Any , A0, A1, A2, A3, A4, A5, A6, A7, A8>
		>::type fun_param_holder;
	enum{ extendable = (Method==1 && 9==1) ? 0 : IsSame<Arguments, typename CppClassSymbol<A8>::type>::value };
	enum{ ArgBase = 1-Method, method = Method, arity = 9-Method, vm = 0 };
	static void call(VMachine* vm, UninitializedAny* args, const void* data){ 
		(*(fun_type*)data)(
			unchecked_cast<A0>((AnyPtr&)args[ArgBase+0]), unchecked_cast<A1>((AnyPtr&)args[ArgBase+1]), unchecked_cast<A2>((AnyPtr&)args[ArgBase+2]), unchecked_cast<A3>((AnyPtr&)args[ArgBase+3]), unchecked_cast<A4>((AnyPtr&)args[ArgBase+4]), unchecked_cast<A5>((AnyPtr&)args[ArgBase+5]), unchecked_cast<A6>((AnyPtr&)args[ArgBase+6]), unchecked_cast<A7>((AnyPtr&)args[ArgBase+7]), unchecked_cast<A8>((AnyPtr&)args[ArgBase+8])
		);
		vm->return_result();
	}
};

template<class TFun, int Method, class R>
struct nfun_base<9, TFun, Method, R>
	: nfun_base9<TFun, Method, R , typename TFun::arg1_type, typename TFun::arg2_type, typename TFun::arg3_type, typename TFun::arg4_type, typename TFun::arg5_type, typename TFun::arg6_type, typename TFun::arg7_type, typename TFun::arg8_type, typename TFun::arg9_type>{};


template<class TFun, int Method, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9>
struct nfun_base10{
	typedef TFun fun_type;
	typedef typename If<Method,
		param_types_holder9<A0, A1, A2, A3, A4, A5, A6, A7, A8, A9>,
		param_types_holder10<Any , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9>
		>::type fun_param_holder;
	enum{ extendable = (Method==1 && 10==1) ? 0 : IsSame<Arguments, typename CppClassSymbol<A9>::type>::value };
	enum{ ArgBase = 1-Method, method = Method, arity = 10-Method, vm = 0 };
	static void call(VMachine* vm, UninitializedAny* args, const void* data){ 
		ReturnResult<R>::call(vm,
			(*(fun_type*)data)(
				unchecked_cast<A0>((AnyPtr&)args[ArgBase+0]), unchecked_cast<A1>((AnyPtr&)args[ArgBase+1]), unchecked_cast<A2>((AnyPtr&)args[ArgBase+2]), unchecked_cast<A3>((AnyPtr&)args[ArgBase+3]), unchecked_cast<A4>((AnyPtr&)args[ArgBase+4]), unchecked_cast<A5>((AnyPtr&)args[ArgBase+5]), unchecked_cast<A6>((AnyPtr&)args[ArgBase+6]), unchecked_cast<A7>((AnyPtr&)args[ArgBase+7]), unchecked_cast<A8>((AnyPtr&)args[ArgBase+8]), unchecked_cast<A9>((AnyPtr&)args[ArgBase+9])
			)
		);
	}
};

template<class TFun, int Method , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9>
struct nfun_base10<TFun, Method, void , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9>{
	typedef TFun fun_type;
	typedef typename If<Method,
		param_types_holder9<A0, A1, A2, A3, A4, A5, A6, A7, A8, A9>,
		param_types_holder10<Any , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9>
		>::type fun_param_holder;
	enum{ extendable = (Method==1 && 10==1) ? 0 : IsSame<Arguments, typename CppClassSymbol<A9>::type>::value };
	enum{ ArgBase = 1-Method, method = Method, arity = 10-Method, vm = 0 };
	static void call(VMachine* vm, UninitializedAny* args, const void* data){ 
		(*(fun_type*)data)(
			unchecked_cast<A0>((AnyPtr&)args[ArgBase+0]), unchecked_cast<A1>((AnyPtr&)args[ArgBase+1]), unchecked_cast<A2>((AnyPtr&)args[ArgBase+2]), unchecked_cast<A3>((AnyPtr&)args[ArgBase+3]), unchecked_cast<A4>((AnyPtr&)args[ArgBase+4]), unchecked_cast<A5>((AnyPtr&)args[ArgBase+5]), unchecked_cast<A6>((AnyPtr&)args[ArgBase+6]), unchecked_cast<A7>((AnyPtr&)args[ArgBase+7]), unchecked_cast<A8>((AnyPtr&)args[ArgBase+8]), unchecked_cast<A9>((AnyPtr&)args[ArgBase+9])
		);
		vm->return_result();
	}
};

template<class TFun, int Method, class R>
struct nfun_base<10, TFun, Method, R>
	: nfun_base10<TFun, Method, R , typename TFun::arg1_type, typename TFun::arg2_type, typename TFun::arg3_type, typename TFun::arg4_type, typename TFun::arg5_type, typename TFun::arg6_type, typename TFun::arg7_type, typename TFun::arg8_type, typename TFun::arg9_type, typename TFun::arg10_type>{};


template<class TFun, int Method, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10>
struct nfun_base11{
	typedef TFun fun_type;
	typedef typename If<Method,
		param_types_holder10<A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10>,
		param_types_holder11<Any , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10>
		>::type fun_param_holder;
	enum{ extendable = (Method==1 && 11==1) ? 0 : IsSame<Arguments, typename CppClassSymbol<A10>::type>::value };
	enum{ ArgBase = 1-Method, method = Method, arity = 11-Method, vm = 0 };
	static void call(VMachine* vm, UninitializedAny* args, const void* data){ 
		ReturnResult<R>::call(vm,
			(*(fun_type*)data)(
				unchecked_cast<A0>((AnyPtr&)args[ArgBase+0]), unchecked_cast<A1>((AnyPtr&)args[ArgBase+1]), unchecked_cast<A2>((AnyPtr&)args[ArgBase+2]), unchecked_cast<A3>((AnyPtr&)args[ArgBase+3]), unchecked_cast<A4>((AnyPtr&)args[ArgBase+4]), unchecked_cast<A5>((AnyPtr&)args[ArgBase+5]), unchecked_cast<A6>((AnyPtr&)args[ArgBase+6]), unchecked_cast<A7>((AnyPtr&)args[ArgBase+7]), unchecked_cast<A8>((AnyPtr&)args[ArgBase+8]), unchecked_cast<A9>((AnyPtr&)args[ArgBase+9]), unchecked_cast<A10>((AnyPtr&)args[ArgBase+10])
			)
		);
	}
};

template<class TFun, int Method , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10>
struct nfun_base11<TFun, Method, void , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10>{
	typedef TFun fun_type;
	typedef typename If<Method,
		param_types_holder10<A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10>,
		param_types_holder11<Any , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10>
		>::type fun_param_holder;
	enum{ extendable = (Method==1 && 11==1) ? 0 : IsSame<Arguments, typename CppClassSymbol<A10>::type>::value };
	enum{ ArgBase = 1-Method, method = Method, arity = 11-Method, vm = 0 };
	static void call(VMachine* vm, UninitializedAny* args, const void* data){ 
		(*(fun_type*)data)(
			unchecked_cast<A0>((AnyPtr&)args[ArgBase+0]), unchecked_cast<A1>((AnyPtr&)args[ArgBase+1]), unchecked_cast<A2>((AnyPtr&)args[ArgBase+2]), unchecked_cast<A3>((AnyPtr&)args[ArgBase+3]), unchecked_cast<A4>((AnyPtr&)args[ArgBase+4]), unchecked_cast<A5>((AnyPtr&)args[ArgBase+5]), unchecked_cast<A6>((AnyPtr&)args[ArgBase+6]), unchecked_cast<A7>((AnyPtr&)args[ArgBase+7]), unchecked_cast<A8>((AnyPtr&)args[ArgBase+8]), unchecked_cast<A9>((AnyPtr&)args[ArgBase+9]), unchecked_cast<A10>((AnyPtr&)args[ArgBase+10])
		);
		vm->return_result();
	}
};

template<class TFun, int Method, class R>
struct nfun_base<11, TFun, Method, R>
	: nfun_base11<TFun, Method, R , typename TFun::arg1_type, typename TFun::arg2_type, typename TFun::arg3_type, typename TFun::arg4_type, typename TFun::arg5_type, typename TFun::arg6_type, typename TFun::arg7_type, typename TFun::arg8_type, typename TFun::arg9_type, typename TFun::arg10_type, typename TFun::arg11_type>{};


template<class TFun, int Method, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11>
struct nfun_base12{
	typedef TFun fun_type;
	typedef typename If<Method,
		param_types_holder11<A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11>,
		param_types_holder12<Any , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11>
		>::type fun_param_holder;
	enum{ extendable = (Method==1 && 12==1) ? 0 : IsSame<Arguments, typename CppClassSymbol<A11>::type>::value };
	enum{ ArgBase = 1-Method, method = Method, arity = 12-Method, vm = 0 };
	static void call(VMachine* vm, UninitializedAny* args, const void* data){ 
		ReturnResult<R>::call(vm,
			(*(fun_type*)data)(
				unchecked_cast<A0>((AnyPtr&)args[ArgBase+0]), unchecked_cast<A1>((AnyPtr&)args[ArgBase+1]), unchecked_cast<A2>((AnyPtr&)args[ArgBase+2]), unchecked_cast<A3>((AnyPtr&)args[ArgBase+3]), unchecked_cast<A4>((AnyPtr&)args[ArgBase+4]), unchecked_cast<A5>((AnyPtr&)args[ArgBase+5]), unchecked_cast<A6>((AnyPtr&)args[ArgBase+6]), unchecked_cast<A7>((AnyPtr&)args[ArgBase+7]), unchecked_cast<A8>((AnyPtr&)args[ArgBase+8]), unchecked_cast<A9>((AnyPtr&)args[ArgBase+9]), unchecked_cast<A10>((AnyPtr&)args[ArgBase+10]), unchecked_cast<A11>((AnyPtr&)args[ArgBase+11])
			)
		);
	}
};

template<class TFun, int Method , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11>
struct nfun_base12<TFun, Method, void , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11>{
	typedef TFun fun_type;
	typedef typename If<Method,
		param_types_holder11<A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11>,
		param_types_holder12<Any , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11>
		>::type fun_param_holder;
	enum{ extendable = (Method==1 && 12==1) ? 0 : IsSame<Arguments, typename CppClassSymbol<A11>::type>::value };
	enum{ ArgBase = 1-Method, method = Method, arity = 12-Method, vm = 0 };
	static void call(VMachine* vm, UninitializedAny* args, const void* data){ 
		(*(fun_type*)data)(
			unchecked_cast<A0>((AnyPtr&)args[ArgBase+0]), unchecked_cast<A1>((AnyPtr&)args[ArgBase+1]), unchecked_cast<A2>((AnyPtr&)args[ArgBase+2]), unchecked_cast<A3>((AnyPtr&)args[ArgBase+3]), unchecked_cast<A4>((AnyPtr&)args[ArgBase+4]), unchecked_cast<A5>((AnyPtr&)args[ArgBase+5]), unchecked_cast<A6>((AnyPtr&)args[ArgBase+6]), unchecked_cast<A7>((AnyPtr&)args[ArgBase+7]), unchecked_cast<A8>((AnyPtr&)args[ArgBase+8]), unchecked_cast<A9>((AnyPtr&)args[ArgBase+9]), unchecked_cast<A10>((AnyPtr&)args[ArgBase+10]), unchecked_cast<A11>((AnyPtr&)args[ArgBase+11])
		);
		vm->return_result();
	}
};

template<class TFun, int Method, class R>
struct nfun_base<12, TFun, Method, R>
	: nfun_base12<TFun, Method, R , typename TFun::arg1_type, typename TFun::arg2_type, typename TFun::arg3_type, typename TFun::arg4_type, typename TFun::arg5_type, typename TFun::arg6_type, typename TFun::arg7_type, typename TFun::arg8_type, typename TFun::arg9_type, typename TFun::arg10_type, typename TFun::arg11_type, typename TFun::arg12_type>{};


template<class TFun, int Method, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12>
struct nfun_base13{
	typedef TFun fun_type;
	typedef typename If<Method,
		param_types_holder12<A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12>,
		param_types_holder13<Any , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12>
		>::type fun_param_holder;
	enum{ extendable = (Method==1 && 13==1) ? 0 : IsSame<Arguments, typename CppClassSymbol<A12>::type>::value };
	enum{ ArgBase = 1-Method, method = Method, arity = 13-Method, vm = 0 };
	static void call(VMachine* vm, UninitializedAny* args, const void* data){ 
		ReturnResult<R>::call(vm,
			(*(fun_type*)data)(
				unchecked_cast<A0>((AnyPtr&)args[ArgBase+0]), unchecked_cast<A1>((AnyPtr&)args[ArgBase+1]), unchecked_cast<A2>((AnyPtr&)args[ArgBase+2]), unchecked_cast<A3>((AnyPtr&)args[ArgBase+3]), unchecked_cast<A4>((AnyPtr&)args[ArgBase+4]), unchecked_cast<A5>((AnyPtr&)args[ArgBase+5]), unchecked_cast<A6>((AnyPtr&)args[ArgBase+6]), unchecked_cast<A7>((AnyPtr&)args[ArgBase+7]), unchecked_cast<A8>((AnyPtr&)args[ArgBase+8]), unchecked_cast<A9>((AnyPtr&)args[ArgBase+9]), unchecked_cast<A10>((AnyPtr&)args[ArgBase+10]), unchecked_cast<A11>((AnyPtr&)args[ArgBase+11]), unchecked_cast<A12>((AnyPtr&)args[ArgBase+12])
			)
		);
	}
};

template<class TFun, int Method , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12>
struct nfun_base13<TFun, Method, void , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12>{
	typedef TFun fun_type;
	typedef typename If<Method,
		param_types_holder12<A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12>,
		param_types_holder13<Any , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12>
		>::type fun_param_holder;
	enum{ extendable = (Method==1 && 13==1) ? 0 : IsSame<Arguments, typename CppClassSymbol<A12>::type>::value };
	enum{ ArgBase = 1-Method, method = Method, arity = 13-Method, vm = 0 };
	static void call(VMachine* vm, UninitializedAny* args, const void* data){ 
		(*(fun_type*)data)(
			unchecked_cast<A0>((AnyPtr&)args[ArgBase+0]), unchecked_cast<A1>((AnyPtr&)args[ArgBase+1]), unchecked_cast<A2>((AnyPtr&)args[ArgBase+2]), unchecked_cast<A3>((AnyPtr&)args[ArgBase+3]), unchecked_cast<A4>((AnyPtr&)args[ArgBase+4]), unchecked_cast<A5>((AnyPtr&)args[ArgBase+5]), unchecked_cast<A6>((AnyPtr&)args[ArgBase+6]), unchecked_cast<A7>((AnyPtr&)args[ArgBase+7]), unchecked_cast<A8>((AnyPtr&)args[ArgBase+8]), unchecked_cast<A9>((AnyPtr&)args[ArgBase+9]), unchecked_cast<A10>((AnyPtr&)args[ArgBase+10]), unchecked_cast<A11>((AnyPtr&)args[ArgBase+11]), unchecked_cast<A12>((AnyPtr&)args[ArgBase+12])
		);
		vm->return_result();
	}
};

template<class TFun, int Method, class R>
struct nfun_base<13, TFun, Method, R>
	: nfun_base13<TFun, Method, R , typename TFun::arg1_type, typename TFun::arg2_type, typename TFun::arg3_type, typename TFun::arg4_type, typename TFun::arg5_type, typename TFun::arg6_type, typename TFun::arg7_type, typename TFun::arg8_type, typename TFun::arg9_type, typename TFun::arg10_type, typename TFun::arg11_type, typename TFun::arg12_type, typename TFun::arg13_type>{};


//}}REPEAT}

template<class TFun>
struct nfun_base1<TFun, 0, void , const VMachinePtr&>{
	typedef TFun fun_type;
	typedef param_types_holder1<Any, Any> fun_param_holder;
	enum{ method = 0, arity = 1, extendable = 0, vm = 1 };
	static void call(VMachine* vm, UninitializedAny* args, const void* data){ 
		(*(TFun*)data)(
			to_smartptr(vm)
		);
	}
};

template<class TFun, class C>
struct nfun_base2<TFun, 1, void , C, const VMachinePtr&>{
	typedef TFun fun_type;
	typedef param_types_holder1<C, Any> fun_param_holder;
	enum{ method = 1, arity = 1, extendable = 0, vm = 1 };
	static void call(VMachine* vm, UninitializedAny* args, const void* data){ 
		(*(TFun*)data)(
			unchecked_cast<C>((AnyPtr&)args[0]),
			to_smartptr(vm)
		);
	}
};

template<class TFun, class R>
struct nfun_base1<TFun, 0, R, const VMachinePtr&>{
	typedef TFun fun_type;
	typedef param_types_holder1<Any, Any> fun_param_holder;
	enum{ method = 0, arity = 1, extendable = 0, vm = 1 };
	static void call(VMachine* vm, UninitializedAny* args, const void* data){ 
		ReturnResult<R>::call(vm, (*(TFun*)data)(
			to_smartptr(vm)
		));
	}
};

template<class TFun, class R, class C>
struct nfun_base2<TFun, 1, R , C, const VMachinePtr&>{
	typedef TFun fun_type;
	typedef param_types_holder1<C, Any> fun_param_holder;
	enum{ method = 1, arity = 1, extendable = 0, vm = 1 };
	static void call(VMachine* vm, UninitializedAny* args, const void* data){ 
		ReturnResult<R>::call(vm, (*(TFun*)data)(
			unchecked_cast<C>((AnyPtr&)args[0]),
			to_smartptr(vm)
		));
	}
};



//{REPEAT{{
/*
template<int Method, class R #COMMA_REPEAT#class A`i`#>
struct nfun<R (*)(#REPEAT_COMMA#A`i`#), Method>
	: nfun_base<`n`, functor<R (*)(#REPEAT_COMMA#A`i`#)>, Method, R>{};

// 関数を関数オブジェクト化する
template<class R #COMMA_REPEAT#class A`i`#>
struct functor<R (*)(#REPEAT_COMMA#A`i`#)>{
	typedef R result_type; #REPEAT#typedef A`i` arg`i+1`_type; #
	typedef R (*fun_type)(#REPEAT_COMMA#A`i`#);
	enum{ arity = `n`};

	fun_type fun_;
	functor(fun_type fun):fun_(fun){}

	R operator()(#REPEAT_COMMA#A`i` a`i`#) const{
		return fun_(#REPEAT_COMMA#a`i`#);
	}
};

template<int Method, class R, class C #COMMA_REPEAT#class A`i`#>
struct nfun<R (C::*)(#REPEAT_COMMA#A`i`#), Method>
	: nfun_base<`n+1`, functor<R (C::*)(#REPEAT_COMMA#A`i`#)>, Method, R>{};

// メンバ関数を関数オブジェクト化する
template<class R, class C #COMMA_REPEAT#class A`i`#>
struct functor<R (C::*)(#REPEAT_COMMA#A`i`#)>{
	typedef R result_type; typedef C* arg1_type; #REPEAT#typedef A`i` arg`i+2`_type; #
	typedef R (C::*fun_type)(#REPEAT_COMMA#A`i`#);
	enum{ arity = `n+1`};

	fun_type fun_;
	functor(fun_type fun):fun_(fun){}

	R operator()(C* self #COMMA_REPEAT#A`i` a`i`#) const{
		return (self->*fun_)(#REPEAT_COMMA#a`i`#);
	}
};

template<int Method, class R, class C #COMMA_REPEAT#class A`i`#>
struct nfun<R (C::*)(#REPEAT_COMMA#A`i`#) const, Method>
	: nfun_base<`n+1`, functor<R (C::*)(#REPEAT_COMMA#A`i`#) const>, Method, R>{};

// constメンバ関数を関数オブジェクト化する
template<class R, class C #COMMA_REPEAT#class A`i`#>
struct functor<R (C::*)(#REPEAT_COMMA#A`i`#) const>{
	typedef R result_type; typedef C* arg1_type; #REPEAT#typedef A`i` arg`i+2`_type; #
	typedef R (C::*fun_type)(#REPEAT_COMMA#A`i`#) const;
	enum{ arity = `n+1`};

	fun_type fun_;
	functor(fun_type fun):fun_(fun){}

	R operator()(C* self #COMMA_REPEAT#A`i` a`i`#) const{
		return (self->*fun_)(#REPEAT_COMMA#a`i`#);
	}
};

////

// 関数を関数オブジェクト化する
template<class R #COMMA_REPEAT#class A`i`#, R (*fun_)(#REPEAT_COMMA#A`i`#)>
struct static_functor<R (*)(#REPEAT_COMMA#A`i`#), fun_>{
	typedef R result_type; #REPEAT#typedef A`i` arg`i+1`_type; #
	enum{ arity = `n`};

	R operator()(#REPEAT_COMMA#A`i` a`i`#) const{
		return fun_(#REPEAT_COMMA#a`i`#);
	}
};

// メンバ関数を関数オブジェクト化する
template<class R, class C #COMMA_REPEAT#class A`i`#, R (C::*fun_)(#REPEAT_COMMA#A`i`#)>
struct static_functor<R (C::*)(#REPEAT_COMMA#A`i`#), fun_>{
	typedef R result_type; typedef C* arg1_type; #REPEAT#typedef A`i` arg`i+2`_type; #
	enum{ arity = `n+1`};

	R operator()(C* self #COMMA_REPEAT#A`i` a`i`#) const{
		return (self->*fun_)(#REPEAT_COMMA#a`i`#);
	}
};

// constメンバ関数を関数オブジェクト化する
template<class R, class C #COMMA_REPEAT#class A`i`#, R (C::*fun_)(#REPEAT_COMMA#A`i`#) const>
struct static_functor<R (C::*)(#REPEAT_COMMA#A`i`#) const, fun_>{
	typedef R result_type; typedef C* arg1_type; #REPEAT#typedef A`i` arg`i+2`_type; #
	enum{ arity = `n+1`};

	R operator()(C* self #COMMA_REPEAT#A`i` a`i`#) const{
		return (self->*fun_)(#REPEAT_COMMA#a`i`#);
	}
};

// コンストラクタを関数オブジェクト化する
template<class T #COMMA_REPEAT#class A`i`#>
struct ctor_functor<T #COMMA_REPEAT#A`i`#>{
	typedef SmartPtr<T> result_type; #REPEAT#typedef A`i` arg`i+1`_type; #
	enum{ arity = `n`};

	SmartPtr<T> operator()(#REPEAT_COMMA#A`i` a`i`#) const{
		return xnew<T>(#REPEAT_COMMA#a`i`#);
	}
};

// stdcall版
#if defined(_WIN32) && defined(_M_IX86)

template<int Method, class R #COMMA_REPEAT#class A`i`#>
struct nfun<R (__stdcall *)(#REPEAT_COMMA#A`i`#), Method>
	: nfun_base<`n`, functor<R (__stdcall *)(#REPEAT_COMMA#A`i`#)>, Method, R>{};

// 関数を関数オブジェクト化する
template<class R #COMMA_REPEAT#class A`i`#>
struct functor<R (__stdcall *)(#REPEAT_COMMA#A`i`#)>{
	typedef R result_type; #REPEAT#typedef A`i` arg`i+1`_type; #
	typedef R (__stdcall *fun_type)(#REPEAT_COMMA#A`i`#);
	enum{ arity = `n`};

	fun_type fun_;
	functor(fun_type fun):fun_(fun){}

	R operator()(#REPEAT_COMMA#A`i` a`i`#) const{
		return fun_(#REPEAT_COMMA#a`i`#);
	}
};

// 関数を関数オブジェクト化する
template<class R #COMMA_REPEAT#class A`i`#, R (__stdcall *fun_)(#REPEAT_COMMA#A`i`#)>
struct static_functor<R (__stdcall *)(#REPEAT_COMMA#A`i`#), fun_>{
	typedef R result_type; #REPEAT#typedef A`i` arg`i+1`_type; #
	enum{ arity = `n`};

	R operator()(#REPEAT_COMMA#A`i` a`i`#) const{
		return fun_(#REPEAT_COMMA#a`i`#);
	}
};

#endif

*/

template<int Method, class R >
struct nfun<R (*)(), Method>
	: nfun_base<0, functor<R (*)()>, Method, R>{};

// 関数を関数オブジェクト化する
template<class R >
struct functor<R (*)()>{
	typedef R result_type; 
	typedef R (*fun_type)();
	enum{ arity = 0};

	fun_type fun_;
	functor(fun_type fun):fun_(fun){}

	R operator()() const{
		return fun_();
	}
};

template<int Method, class R, class C >
struct nfun<R (C::*)(), Method>
	: nfun_base<1, functor<R (C::*)()>, Method, R>{};

// メンバ関数を関数オブジェクト化する
template<class R, class C >
struct functor<R (C::*)()>{
	typedef R result_type; typedef C* arg1_type; 
	typedef R (C::*fun_type)();
	enum{ arity = 1};

	fun_type fun_;
	functor(fun_type fun):fun_(fun){}

	R operator()(C* self ) const{
		return (self->*fun_)();
	}
};

template<int Method, class R, class C >
struct nfun<R (C::*)() const, Method>
	: nfun_base<1, functor<R (C::*)() const>, Method, R>{};

// constメンバ関数を関数オブジェクト化する
template<class R, class C >
struct functor<R (C::*)() const>{
	typedef R result_type; typedef C* arg1_type; 
	typedef R (C::*fun_type)() const;
	enum{ arity = 1};

	fun_type fun_;
	functor(fun_type fun):fun_(fun){}

	R operator()(C* self ) const{
		return (self->*fun_)();
	}
};

////

// 関数を関数オブジェクト化する
template<class R , R (*fun_)()>
struct static_functor<R (*)(), fun_>{
	typedef R result_type; 
	enum{ arity = 0};

	R operator()() const{
		return fun_();
	}
};

// メンバ関数を関数オブジェクト化する
template<class R, class C , R (C::*fun_)()>
struct static_functor<R (C::*)(), fun_>{
	typedef R result_type; typedef C* arg1_type; 
	enum{ arity = 1};

	R operator()(C* self ) const{
		return (self->*fun_)();
	}
};

// constメンバ関数を関数オブジェクト化する
template<class R, class C , R (C::*fun_)() const>
struct static_functor<R (C::*)() const, fun_>{
	typedef R result_type; typedef C* arg1_type; 
	enum{ arity = 1};

	R operator()(C* self ) const{
		return (self->*fun_)();
	}
};

// コンストラクタを関数オブジェクト化する
template<class T >
struct ctor_functor<T >{
	typedef SmartPtr<T> result_type; 
	enum{ arity = 0};

	SmartPtr<T> operator()() const{
		return xnew<T>();
	}
};

// stdcall版
#if defined(_WIN32) && defined(_M_IX86)

template<int Method, class R >
struct nfun<R (__stdcall *)(), Method>
	: nfun_base<0, functor<R (__stdcall *)()>, Method, R>{};

// 関数を関数オブジェクト化する
template<class R >
struct functor<R (__stdcall *)()>{
	typedef R result_type; 
	typedef R (__stdcall *fun_type)();
	enum{ arity = 0};

	fun_type fun_;
	functor(fun_type fun):fun_(fun){}

	R operator()() const{
		return fun_();
	}
};

// 関数を関数オブジェクト化する
template<class R , R (__stdcall *fun_)()>
struct static_functor<R (__stdcall *)(), fun_>{
	typedef R result_type; 
	enum{ arity = 0};

	R operator()() const{
		return fun_();
	}
};

#endif


template<int Method, class R , class A0>
struct nfun<R (*)(A0), Method>
	: nfun_base<1, functor<R (*)(A0)>, Method, R>{};

// 関数を関数オブジェクト化する
template<class R , class A0>
struct functor<R (*)(A0)>{
	typedef R result_type; typedef A0 arg1_type; 
	typedef R (*fun_type)(A0);
	enum{ arity = 1};

	fun_type fun_;
	functor(fun_type fun):fun_(fun){}

	R operator()(A0 a0) const{
		return fun_(a0);
	}
};

template<int Method, class R, class C , class A0>
struct nfun<R (C::*)(A0), Method>
	: nfun_base<2, functor<R (C::*)(A0)>, Method, R>{};

// メンバ関数を関数オブジェクト化する
template<class R, class C , class A0>
struct functor<R (C::*)(A0)>{
	typedef R result_type; typedef C* arg1_type; typedef A0 arg2_type; 
	typedef R (C::*fun_type)(A0);
	enum{ arity = 2};

	fun_type fun_;
	functor(fun_type fun):fun_(fun){}

	R operator()(C* self , A0 a0) const{
		return (self->*fun_)(a0);
	}
};

template<int Method, class R, class C , class A0>
struct nfun<R (C::*)(A0) const, Method>
	: nfun_base<2, functor<R (C::*)(A0) const>, Method, R>{};

// constメンバ関数を関数オブジェクト化する
template<class R, class C , class A0>
struct functor<R (C::*)(A0) const>{
	typedef R result_type; typedef C* arg1_type; typedef A0 arg2_type; 
	typedef R (C::*fun_type)(A0) const;
	enum{ arity = 2};

	fun_type fun_;
	functor(fun_type fun):fun_(fun){}

	R operator()(C* self , A0 a0) const{
		return (self->*fun_)(a0);
	}
};

////

// 関数を関数オブジェクト化する
template<class R , class A0, R (*fun_)(A0)>
struct static_functor<R (*)(A0), fun_>{
	typedef R result_type; typedef A0 arg1_type; 
	enum{ arity = 1};

	R operator()(A0 a0) const{
		return fun_(a0);
	}
};

// メンバ関数を関数オブジェクト化する
template<class R, class C , class A0, R (C::*fun_)(A0)>
struct static_functor<R (C::*)(A0), fun_>{
	typedef R result_type; typedef C* arg1_type; typedef A0 arg2_type; 
	enum{ arity = 2};

	R operator()(C* self , A0 a0) const{
		return (self->*fun_)(a0);
	}
};

// constメンバ関数を関数オブジェクト化する
template<class R, class C , class A0, R (C::*fun_)(A0) const>
struct static_functor<R (C::*)(A0) const, fun_>{
	typedef R result_type; typedef C* arg1_type; typedef A0 arg2_type; 
	enum{ arity = 2};

	R operator()(C* self , A0 a0) const{
		return (self->*fun_)(a0);
	}
};

// コンストラクタを関数オブジェクト化する
template<class T , class A0>
struct ctor_functor<T , A0>{
	typedef SmartPtr<T> result_type; typedef A0 arg1_type; 
	enum{ arity = 1};

	SmartPtr<T> operator()(A0 a0) const{
		return xnew<T>(a0);
	}
};

// stdcall版
#if defined(_WIN32) && defined(_M_IX86)

template<int Method, class R , class A0>
struct nfun<R (__stdcall *)(A0), Method>
	: nfun_base<1, functor<R (__stdcall *)(A0)>, Method, R>{};

// 関数を関数オブジェクト化する
template<class R , class A0>
struct functor<R (__stdcall *)(A0)>{
	typedef R result_type; typedef A0 arg1_type; 
	typedef R (__stdcall *fun_type)(A0);
	enum{ arity = 1};

	fun_type fun_;
	functor(fun_type fun):fun_(fun){}

	R operator()(A0 a0) const{
		return fun_(a0);
	}
};

// 関数を関数オブジェクト化する
template<class R , class A0, R (__stdcall *fun_)(A0)>
struct static_functor<R (__stdcall *)(A0), fun_>{
	typedef R result_type; typedef A0 arg1_type; 
	enum{ arity = 1};

	R operator()(A0 a0) const{
		return fun_(a0);
	}
};

#endif


template<int Method, class R , class A0, class A1>
struct nfun<R (*)(A0, A1), Method>
	: nfun_base<2, functor<R (*)(A0, A1)>, Method, R>{};

// 関数を関数オブジェクト化する
template<class R , class A0, class A1>
struct functor<R (*)(A0, A1)>{
	typedef R result_type; typedef A0 arg1_type; typedef A1 arg2_type; 
	typedef R (*fun_type)(A0, A1);
	enum{ arity = 2};

	fun_type fun_;
	functor(fun_type fun):fun_(fun){}

	R operator()(A0 a0, A1 a1) const{
		return fun_(a0, a1);
	}
};

template<int Method, class R, class C , class A0, class A1>
struct nfun<R (C::*)(A0, A1), Method>
	: nfun_base<3, functor<R (C::*)(A0, A1)>, Method, R>{};

// メンバ関数を関数オブジェクト化する
template<class R, class C , class A0, class A1>
struct functor<R (C::*)(A0, A1)>{
	typedef R result_type; typedef C* arg1_type; typedef A0 arg2_type; typedef A1 arg3_type; 
	typedef R (C::*fun_type)(A0, A1);
	enum{ arity = 3};

	fun_type fun_;
	functor(fun_type fun):fun_(fun){}

	R operator()(C* self , A0 a0, A1 a1) const{
		return (self->*fun_)(a0, a1);
	}
};

template<int Method, class R, class C , class A0, class A1>
struct nfun<R (C::*)(A0, A1) const, Method>
	: nfun_base<3, functor<R (C::*)(A0, A1) const>, Method, R>{};

// constメンバ関数を関数オブジェクト化する
template<class R, class C , class A0, class A1>
struct functor<R (C::*)(A0, A1) const>{
	typedef R result_type; typedef C* arg1_type; typedef A0 arg2_type; typedef A1 arg3_type; 
	typedef R (C::*fun_type)(A0, A1) const;
	enum{ arity = 3};

	fun_type fun_;
	functor(fun_type fun):fun_(fun){}

	R operator()(C* self , A0 a0, A1 a1) const{
		return (self->*fun_)(a0, a1);
	}
};

////

// 関数を関数オブジェクト化する
template<class R , class A0, class A1, R (*fun_)(A0, A1)>
struct static_functor<R (*)(A0, A1), fun_>{
	typedef R result_type; typedef A0 arg1_type; typedef A1 arg2_type; 
	enum{ arity = 2};

	R operator()(A0 a0, A1 a1) const{
		return fun_(a0, a1);
	}
};

// メンバ関数を関数オブジェクト化する
template<class R, class C , class A0, class A1, R (C::*fun_)(A0, A1)>
struct static_functor<R (C::*)(A0, A1), fun_>{
	typedef R result_type; typedef C* arg1_type; typedef A0 arg2_type; typedef A1 arg3_type; 
	enum{ arity = 3};

	R operator()(C* self , A0 a0, A1 a1) const{
		return (self->*fun_)(a0, a1);
	}
};

// constメンバ関数を関数オブジェクト化する
template<class R, class C , class A0, class A1, R (C::*fun_)(A0, A1) const>
struct static_functor<R (C::*)(A0, A1) const, fun_>{
	typedef R result_type; typedef C* arg1_type; typedef A0 arg2_type; typedef A1 arg3_type; 
	enum{ arity = 3};

	R operator()(C* self , A0 a0, A1 a1) const{
		return (self->*fun_)(a0, a1);
	}
};

// コンストラクタを関数オブジェクト化する
template<class T , class A0, class A1>
struct ctor_functor<T , A0, A1>{
	typedef SmartPtr<T> result_type; typedef A0 arg1_type; typedef A1 arg2_type; 
	enum{ arity = 2};

	SmartPtr<T> operator()(A0 a0, A1 a1) const{
		return xnew<T>(a0, a1);
	}
};

// stdcall版
#if defined(_WIN32) && defined(_M_IX86)

template<int Method, class R , class A0, class A1>
struct nfun<R (__stdcall *)(A0, A1), Method>
	: nfun_base<2, functor<R (__stdcall *)(A0, A1)>, Method, R>{};

// 関数を関数オブジェクト化する
template<class R , class A0, class A1>
struct functor<R (__stdcall *)(A0, A1)>{
	typedef R result_type; typedef A0 arg1_type; typedef A1 arg2_type; 
	typedef R (__stdcall *fun_type)(A0, A1);
	enum{ arity = 2};

	fun_type fun_;
	functor(fun_type fun):fun_(fun){}

	R operator()(A0 a0, A1 a1) const{
		return fun_(a0, a1);
	}
};

// 関数を関数オブジェクト化する
template<class R , class A0, class A1, R (__stdcall *fun_)(A0, A1)>
struct static_functor<R (__stdcall *)(A0, A1), fun_>{
	typedef R result_type; typedef A0 arg1_type; typedef A1 arg2_type; 
	enum{ arity = 2};

	R operator()(A0 a0, A1 a1) const{
		return fun_(a0, a1);
	}
};

#endif


template<int Method, class R , class A0, class A1, class A2>
struct nfun<R (*)(A0, A1, A2), Method>
	: nfun_base<3, functor<R (*)(A0, A1, A2)>, Method, R>{};

// 関数を関数オブジェクト化する
template<class R , class A0, class A1, class A2>
struct functor<R (*)(A0, A1, A2)>{
	typedef R result_type; typedef A0 arg1_type; typedef A1 arg2_type; typedef A2 arg3_type; 
	typedef R (*fun_type)(A0, A1, A2);
	enum{ arity = 3};

	fun_type fun_;
	functor(fun_type fun):fun_(fun){}

	R operator()(A0 a0, A1 a1, A2 a2) const{
		return fun_(a0, a1, a2);
	}
};

template<int Method, class R, class C , class A0, class A1, class A2>
struct nfun<R (C::*)(A0, A1, A2), Method>
	: nfun_base<4, functor<R (C::*)(A0, A1, A2)>, Method, R>{};

// メンバ関数を関数オブジェクト化する
template<class R, class C , class A0, class A1, class A2>
struct functor<R (C::*)(A0, A1, A2)>{
	typedef R result_type; typedef C* arg1_type; typedef A0 arg2_type; typedef A1 arg3_type; typedef A2 arg4_type; 
	typedef R (C::*fun_type)(A0, A1, A2);
	enum{ arity = 4};

	fun_type fun_;
	functor(fun_type fun):fun_(fun){}

	R operator()(C* self , A0 a0, A1 a1, A2 a2) const{
		return (self->*fun_)(a0, a1, a2);
	}
};

template<int Method, class R, class C , class A0, class A1, class A2>
struct nfun<R (C::*)(A0, A1, A2) const, Method>
	: nfun_base<4, functor<R (C::*)(A0, A1, A2) const>, Method, R>{};

// constメンバ関数を関数オブジェクト化する
template<class R, class C , class A0, class A1, class A2>
struct functor<R (C::*)(A0, A1, A2) const>{
	typedef R result_type; typedef C* arg1_type; typedef A0 arg2_type; typedef A1 arg3_type; typedef A2 arg4_type; 
	typedef R (C::*fun_type)(A0, A1, A2) const;
	enum{ arity = 4};

	fun_type fun_;
	functor(fun_type fun):fun_(fun){}

	R operator()(C* self , A0 a0, A1 a1, A2 a2) const{
		return (self->*fun_)(a0, a1, a2);
	}
};

////

// 関数を関数オブジェクト化する
template<class R , class A0, class A1, class A2, R (*fun_)(A0, A1, A2)>
struct static_functor<R (*)(A0, A1, A2), fun_>{
	typedef R result_type; typedef A0 arg1_type; typedef A1 arg2_type; typedef A2 arg3_type; 
	enum{ arity = 3};

	R operator()(A0 a0, A1 a1, A2 a2) const{
		return fun_(a0, a1, a2);
	}
};

// メンバ関数を関数オブジェクト化する
template<class R, class C , class A0, class A1, class A2, R (C::*fun_)(A0, A1, A2)>
struct static_functor<R (C::*)(A0, A1, A2), fun_>{
	typedef R result_type; typedef C* arg1_type; typedef A0 arg2_type; typedef A1 arg3_type; typedef A2 arg4_type; 
	enum{ arity = 4};

	R operator()(C* self , A0 a0, A1 a1, A2 a2) const{
		return (self->*fun_)(a0, a1, a2);
	}
};

// constメンバ関数を関数オブジェクト化する
template<class R, class C , class A0, class A1, class A2, R (C::*fun_)(A0, A1, A2) const>
struct static_functor<R (C::*)(A0, A1, A2) const, fun_>{
	typedef R result_type; typedef C* arg1_type; typedef A0 arg2_type; typedef A1 arg3_type; typedef A2 arg4_type; 
	enum{ arity = 4};

	R operator()(C* self , A0 a0, A1 a1, A2 a2) const{
		return (self->*fun_)(a0, a1, a2);
	}
};

// コンストラクタを関数オブジェクト化する
template<class T , class A0, class A1, class A2>
struct ctor_functor<T , A0, A1, A2>{
	typedef SmartPtr<T> result_type; typedef A0 arg1_type; typedef A1 arg2_type; typedef A2 arg3_type; 
	enum{ arity = 3};

	SmartPtr<T> operator()(A0 a0, A1 a1, A2 a2) const{
		return xnew<T>(a0, a1, a2);
	}
};

// stdcall版
#if defined(_WIN32) && defined(_M_IX86)

template<int Method, class R , class A0, class A1, class A2>
struct nfun<R (__stdcall *)(A0, A1, A2), Method>
	: nfun_base<3, functor<R (__stdcall *)(A0, A1, A2)>, Method, R>{};

// 関数を関数オブジェクト化する
template<class R , class A0, class A1, class A2>
struct functor<R (__stdcall *)(A0, A1, A2)>{
	typedef R result_type; typedef A0 arg1_type; typedef A1 arg2_type; typedef A2 arg3_type; 
	typedef R (__stdcall *fun_type)(A0, A1, A2);
	enum{ arity = 3};

	fun_type fun_;
	functor(fun_type fun):fun_(fun){}

	R operator()(A0 a0, A1 a1, A2 a2) const{
		return fun_(a0, a1, a2);
	}
};

// 関数を関数オブジェクト化する
template<class R , class A0, class A1, class A2, R (__stdcall *fun_)(A0, A1, A2)>
struct static_functor<R (__stdcall *)(A0, A1, A2), fun_>{
	typedef R result_type; typedef A0 arg1_type; typedef A1 arg2_type; typedef A2 arg3_type; 
	enum{ arity = 3};

	R operator()(A0 a0, A1 a1, A2 a2) const{
		return fun_(a0, a1, a2);
	}
};

#endif


template<int Method, class R , class A0, class A1, class A2, class A3>
struct nfun<R (*)(A0, A1, A2, A3), Method>
	: nfun_base<4, functor<R (*)(A0, A1, A2, A3)>, Method, R>{};

// 関数を関数オブジェクト化する
template<class R , class A0, class A1, class A2, class A3>
struct functor<R (*)(A0, A1, A2, A3)>{
	typedef R result_type; typedef A0 arg1_type; typedef A1 arg2_type; typedef A2 arg3_type; typedef A3 arg4_type; 
	typedef R (*fun_type)(A0, A1, A2, A3);
	enum{ arity = 4};

	fun_type fun_;
	functor(fun_type fun):fun_(fun){}

	R operator()(A0 a0, A1 a1, A2 a2, A3 a3) const{
		return fun_(a0, a1, a2, a3);
	}
};

template<int Method, class R, class C , class A0, class A1, class A2, class A3>
struct nfun<R (C::*)(A0, A1, A2, A3), Method>
	: nfun_base<5, functor<R (C::*)(A0, A1, A2, A3)>, Method, R>{};

// メンバ関数を関数オブジェクト化する
template<class R, class C , class A0, class A1, class A2, class A3>
struct functor<R (C::*)(A0, A1, A2, A3)>{
	typedef R result_type; typedef C* arg1_type; typedef A0 arg2_type; typedef A1 arg3_type; typedef A2 arg4_type; typedef A3 arg5_type; 
	typedef R (C::*fun_type)(A0, A1, A2, A3);
	enum{ arity = 5};

	fun_type fun_;
	functor(fun_type fun):fun_(fun){}

	R operator()(C* self , A0 a0, A1 a1, A2 a2, A3 a3) const{
		return (self->*fun_)(a0, a1, a2, a3);
	}
};

template<int Method, class R, class C , class A0, class A1, class A2, class A3>
struct nfun<R (C::*)(A0, A1, A2, A3) const, Method>
	: nfun_base<5, functor<R (C::*)(A0, A1, A2, A3) const>, Method, R>{};

// constメンバ関数を関数オブジェクト化する
template<class R, class C , class A0, class A1, class A2, class A3>
struct functor<R (C::*)(A0, A1, A2, A3) const>{
	typedef R result_type; typedef C* arg1_type; typedef A0 arg2_type; typedef A1 arg3_type; typedef A2 arg4_type; typedef A3 arg5_type; 
	typedef R (C::*fun_type)(A0, A1, A2, A3) const;
	enum{ arity = 5};

	fun_type fun_;
	functor(fun_type fun):fun_(fun){}

	R operator()(C* self , A0 a0, A1 a1, A2 a2, A3 a3) const{
		return (self->*fun_)(a0, a1, a2, a3);
	}
};

////

// 関数を関数オブジェクト化する
template<class R , class A0, class A1, class A2, class A3, R (*fun_)(A0, A1, A2, A3)>
struct static_functor<R (*)(A0, A1, A2, A3), fun_>{
	typedef R result_type; typedef A0 arg1_type; typedef A1 arg2_type; typedef A2 arg3_type; typedef A3 arg4_type; 
	enum{ arity = 4};

	R operator()(A0 a0, A1 a1, A2 a2, A3 a3) const{
		return fun_(a0, a1, a2, a3);
	}
};

// メンバ関数を関数オブジェクト化する
template<class R, class C , class A0, class A1, class A2, class A3, R (C::*fun_)(A0, A1, A2, A3)>
struct static_functor<R (C::*)(A0, A1, A2, A3), fun_>{
	typedef R result_type; typedef C* arg1_type; typedef A0 arg2_type; typedef A1 arg3_type; typedef A2 arg4_type; typedef A3 arg5_type; 
	enum{ arity = 5};

	R operator()(C* self , A0 a0, A1 a1, A2 a2, A3 a3) const{
		return (self->*fun_)(a0, a1, a2, a3);
	}
};

// constメンバ関数を関数オブジェクト化する
template<class R, class C , class A0, class A1, class A2, class A3, R (C::*fun_)(A0, A1, A2, A3) const>
struct static_functor<R (C::*)(A0, A1, A2, A3) const, fun_>{
	typedef R result_type; typedef C* arg1_type; typedef A0 arg2_type; typedef A1 arg3_type; typedef A2 arg4_type; typedef A3 arg5_type; 
	enum{ arity = 5};

	R operator()(C* self , A0 a0, A1 a1, A2 a2, A3 a3) const{
		return (self->*fun_)(a0, a1, a2, a3);
	}
};

// コンストラクタを関数オブジェクト化する
template<class T , class A0, class A1, class A2, class A3>
struct ctor_functor<T , A0, A1, A2, A3>{
	typedef SmartPtr<T> result_type; typedef A0 arg1_type; typedef A1 arg2_type; typedef A2 arg3_type; typedef A3 arg4_type; 
	enum{ arity = 4};

	SmartPtr<T> operator()(A0 a0, A1 a1, A2 a2, A3 a3) const{
		return xnew<T>(a0, a1, a2, a3);
	}
};

// stdcall版
#if defined(_WIN32) && defined(_M_IX86)

template<int Method, class R , class A0, class A1, class A2, class A3>
struct nfun<R (__stdcall *)(A0, A1, A2, A3), Method>
	: nfun_base<4, functor<R (__stdcall *)(A0, A1, A2, A3)>, Method, R>{};

// 関数を関数オブジェクト化する
template<class R , class A0, class A1, class A2, class A3>
struct functor<R (__stdcall *)(A0, A1, A2, A3)>{
	typedef R result_type; typedef A0 arg1_type; typedef A1 arg2_type; typedef A2 arg3_type; typedef A3 arg4_type; 
	typedef R (__stdcall *fun_type)(A0, A1, A2, A3);
	enum{ arity = 4};

	fun_type fun_;
	functor(fun_type fun):fun_(fun){}

	R operator()(A0 a0, A1 a1, A2 a2, A3 a3) const{
		return fun_(a0, a1, a2, a3);
	}
};

// 関数を関数オブジェクト化する
template<class R , class A0, class A1, class A2, class A3, R (__stdcall *fun_)(A0, A1, A2, A3)>
struct static_functor<R (__stdcall *)(A0, A1, A2, A3), fun_>{
	typedef R result_type; typedef A0 arg1_type; typedef A1 arg2_type; typedef A2 arg3_type; typedef A3 arg4_type; 
	enum{ arity = 4};

	R operator()(A0 a0, A1 a1, A2 a2, A3 a3) const{
		return fun_(a0, a1, a2, a3);
	}
};

#endif


template<int Method, class R , class A0, class A1, class A2, class A3, class A4>
struct nfun<R (*)(A0, A1, A2, A3, A4), Method>
	: nfun_base<5, functor<R (*)(A0, A1, A2, A3, A4)>, Method, R>{};

// 関数を関数オブジェクト化する
template<class R , class A0, class A1, class A2, class A3, class A4>
struct functor<R (*)(A0, A1, A2, A3, A4)>{
	typedef R result_type; typedef A0 arg1_type; typedef A1 arg2_type; typedef A2 arg3_type; typedef A3 arg4_type; typedef A4 arg5_type; 
	typedef R (*fun_type)(A0, A1, A2, A3, A4);
	enum{ arity = 5};

	fun_type fun_;
	functor(fun_type fun):fun_(fun){}

	R operator()(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4) const{
		return fun_(a0, a1, a2, a3, a4);
	}
};

template<int Method, class R, class C , class A0, class A1, class A2, class A3, class A4>
struct nfun<R (C::*)(A0, A1, A2, A3, A4), Method>
	: nfun_base<6, functor<R (C::*)(A0, A1, A2, A3, A4)>, Method, R>{};

// メンバ関数を関数オブジェクト化する
template<class R, class C , class A0, class A1, class A2, class A3, class A4>
struct functor<R (C::*)(A0, A1, A2, A3, A4)>{
	typedef R result_type; typedef C* arg1_type; typedef A0 arg2_type; typedef A1 arg3_type; typedef A2 arg4_type; typedef A3 arg5_type; typedef A4 arg6_type; 
	typedef R (C::*fun_type)(A0, A1, A2, A3, A4);
	enum{ arity = 6};

	fun_type fun_;
	functor(fun_type fun):fun_(fun){}

	R operator()(C* self , A0 a0, A1 a1, A2 a2, A3 a3, A4 a4) const{
		return (self->*fun_)(a0, a1, a2, a3, a4);
	}
};

template<int Method, class R, class C , class A0, class A1, class A2, class A3, class A4>
struct nfun<R (C::*)(A0, A1, A2, A3, A4) const, Method>
	: nfun_base<6, functor<R (C::*)(A0, A1, A2, A3, A4) const>, Method, R>{};

// constメンバ関数を関数オブジェクト化する
template<class R, class C , class A0, class A1, class A2, class A3, class A4>
struct functor<R (C::*)(A0, A1, A2, A3, A4) const>{
	typedef R result_type; typedef C* arg1_type; typedef A0 arg2_type; typedef A1 arg3_type; typedef A2 arg4_type; typedef A3 arg5_type; typedef A4 arg6_type; 
	typedef R (C::*fun_type)(A0, A1, A2, A3, A4) const;
	enum{ arity = 6};

	fun_type fun_;
	functor(fun_type fun):fun_(fun){}

	R operator()(C* self , A0 a0, A1 a1, A2 a2, A3 a3, A4 a4) const{
		return (self->*fun_)(a0, a1, a2, a3, a4);
	}
};

////

// 関数を関数オブジェクト化する
template<class R , class A0, class A1, class A2, class A3, class A4, R (*fun_)(A0, A1, A2, A3, A4)>
struct static_functor<R (*)(A0, A1, A2, A3, A4), fun_>{
	typedef R result_type; typedef A0 arg1_type; typedef A1 arg2_type; typedef A2 arg3_type; typedef A3 arg4_type; typedef A4 arg5_type; 
	enum{ arity = 5};

	R operator()(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4) const{
		return fun_(a0, a1, a2, a3, a4);
	}
};

// メンバ関数を関数オブジェクト化する
template<class R, class C , class A0, class A1, class A2, class A3, class A4, R (C::*fun_)(A0, A1, A2, A3, A4)>
struct static_functor<R (C::*)(A0, A1, A2, A3, A4), fun_>{
	typedef R result_type; typedef C* arg1_type; typedef A0 arg2_type; typedef A1 arg3_type; typedef A2 arg4_type; typedef A3 arg5_type; typedef A4 arg6_type; 
	enum{ arity = 6};

	R operator()(C* self , A0 a0, A1 a1, A2 a2, A3 a3, A4 a4) const{
		return (self->*fun_)(a0, a1, a2, a3, a4);
	}
};

// constメンバ関数を関数オブジェクト化する
template<class R, class C , class A0, class A1, class A2, class A3, class A4, R (C::*fun_)(A0, A1, A2, A3, A4) const>
struct static_functor<R (C::*)(A0, A1, A2, A3, A4) const, fun_>{
	typedef R result_type; typedef C* arg1_type; typedef A0 arg2_type; typedef A1 arg3_type; typedef A2 arg4_type; typedef A3 arg5_type; typedef A4 arg6_type; 
	enum{ arity = 6};

	R operator()(C* self , A0 a0, A1 a1, A2 a2, A3 a3, A4 a4) const{
		return (self->*fun_)(a0, a1, a2, a3, a4);
	}
};

// コンストラクタを関数オブジェクト化する
template<class T , class A0, class A1, class A2, class A3, class A4>
struct ctor_functor<T , A0, A1, A2, A3, A4>{
	typedef SmartPtr<T> result_type; typedef A0 arg1_type; typedef A1 arg2_type; typedef A2 arg3_type; typedef A3 arg4_type; typedef A4 arg5_type; 
	enum{ arity = 5};

	SmartPtr<T> operator()(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4) const{
		return xnew<T>(a0, a1, a2, a3, a4);
	}
};

// stdcall版
#if defined(_WIN32) && defined(_M_IX86)

template<int Method, class R , class A0, class A1, class A2, class A3, class A4>
struct nfun<R (__stdcall *)(A0, A1, A2, A3, A4), Method>
	: nfun_base<5, functor<R (__stdcall *)(A0, A1, A2, A3, A4)>, Method, R>{};

// 関数を関数オブジェクト化する
template<class R , class A0, class A1, class A2, class A3, class A4>
struct functor<R (__stdcall *)(A0, A1, A2, A3, A4)>{
	typedef R result_type; typedef A0 arg1_type; typedef A1 arg2_type; typedef A2 arg3_type; typedef A3 arg4_type; typedef A4 arg5_type; 
	typedef R (__stdcall *fun_type)(A0, A1, A2, A3, A4);
	enum{ arity = 5};

	fun_type fun_;
	functor(fun_type fun):fun_(fun){}

	R operator()(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4) const{
		return fun_(a0, a1, a2, a3, a4);
	}
};

// 関数を関数オブジェクト化する
template<class R , class A0, class A1, class A2, class A3, class A4, R (__stdcall *fun_)(A0, A1, A2, A3, A4)>
struct static_functor<R (__stdcall *)(A0, A1, A2, A3, A4), fun_>{
	typedef R result_type; typedef A0 arg1_type; typedef A1 arg2_type; typedef A2 arg3_type; typedef A3 arg4_type; typedef A4 arg5_type; 
	enum{ arity = 5};

	R operator()(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4) const{
		return fun_(a0, a1, a2, a3, a4);
	}
};

#endif


template<int Method, class R , class A0, class A1, class A2, class A3, class A4, class A5>
struct nfun<R (*)(A0, A1, A2, A3, A4, A5), Method>
	: nfun_base<6, functor<R (*)(A0, A1, A2, A3, A4, A5)>, Method, R>{};

// 関数を関数オブジェクト化する
template<class R , class A0, class A1, class A2, class A3, class A4, class A5>
struct functor<R (*)(A0, A1, A2, A3, A4, A5)>{
	typedef R result_type; typedef A0 arg1_type; typedef A1 arg2_type; typedef A2 arg3_type; typedef A3 arg4_type; typedef A4 arg5_type; typedef A5 arg6_type; 
	typedef R (*fun_type)(A0, A1, A2, A3, A4, A5);
	enum{ arity = 6};

	fun_type fun_;
	functor(fun_type fun):fun_(fun){}

	R operator()(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5) const{
		return fun_(a0, a1, a2, a3, a4, a5);
	}
};

template<int Method, class R, class C , class A0, class A1, class A2, class A3, class A4, class A5>
struct nfun<R (C::*)(A0, A1, A2, A3, A4, A5), Method>
	: nfun_base<7, functor<R (C::*)(A0, A1, A2, A3, A4, A5)>, Method, R>{};

// メンバ関数を関数オブジェクト化する
template<class R, class C , class A0, class A1, class A2, class A3, class A4, class A5>
struct functor<R (C::*)(A0, A1, A2, A3, A4, A5)>{
	typedef R result_type; typedef C* arg1_type; typedef A0 arg2_type; typedef A1 arg3_type; typedef A2 arg4_type; typedef A3 arg5_type; typedef A4 arg6_type; typedef A5 arg7_type; 
	typedef R (C::*fun_type)(A0, A1, A2, A3, A4, A5);
	enum{ arity = 7};

	fun_type fun_;
	functor(fun_type fun):fun_(fun){}

	R operator()(C* self , A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5) const{
		return (self->*fun_)(a0, a1, a2, a3, a4, a5);
	}
};

template<int Method, class R, class C , class A0, class A1, class A2, class A3, class A4, class A5>
struct nfun<R (C::*)(A0, A1, A2, A3, A4, A5) const, Method>
	: nfun_base<7, functor<R (C::*)(A0, A1, A2, A3, A4, A5) const>, Method, R>{};

// constメンバ関数を関数オブジェクト化する
template<class R, class C , class A0, class A1, class A2, class A3, class A4, class A5>
struct functor<R (C::*)(A0, A1, A2, A3, A4, A5) const>{
	typedef R result_type; typedef C* arg1_type; typedef A0 arg2_type; typedef A1 arg3_type; typedef A2 arg4_type; typedef A3 arg5_type; typedef A4 arg6_type; typedef A5 arg7_type; 
	typedef R (C::*fun_type)(A0, A1, A2, A3, A4, A5) const;
	enum{ arity = 7};

	fun_type fun_;
	functor(fun_type fun):fun_(fun){}

	R operator()(C* self , A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5) const{
		return (self->*fun_)(a0, a1, a2, a3, a4, a5);
	}
};

////

// 関数を関数オブジェクト化する
template<class R , class A0, class A1, class A2, class A3, class A4, class A5, R (*fun_)(A0, A1, A2, A3, A4, A5)>
struct static_functor<R (*)(A0, A1, A2, A3, A4, A5), fun_>{
	typedef R result_type; typedef A0 arg1_type; typedef A1 arg2_type; typedef A2 arg3_type; typedef A3 arg4_type; typedef A4 arg5_type; typedef A5 arg6_type; 
	enum{ arity = 6};

	R operator()(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5) const{
		return fun_(a0, a1, a2, a3, a4, a5);
	}
};

// メンバ関数を関数オブジェクト化する
template<class R, class C , class A0, class A1, class A2, class A3, class A4, class A5, R (C::*fun_)(A0, A1, A2, A3, A4, A5)>
struct static_functor<R (C::*)(A0, A1, A2, A3, A4, A5), fun_>{
	typedef R result_type; typedef C* arg1_type; typedef A0 arg2_type; typedef A1 arg3_type; typedef A2 arg4_type; typedef A3 arg5_type; typedef A4 arg6_type; typedef A5 arg7_type; 
	enum{ arity = 7};

	R operator()(C* self , A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5) const{
		return (self->*fun_)(a0, a1, a2, a3, a4, a5);
	}
};

// constメンバ関数を関数オブジェクト化する
template<class R, class C , class A0, class A1, class A2, class A3, class A4, class A5, R (C::*fun_)(A0, A1, A2, A3, A4, A5) const>
struct static_functor<R (C::*)(A0, A1, A2, A3, A4, A5) const, fun_>{
	typedef R result_type; typedef C* arg1_type; typedef A0 arg2_type; typedef A1 arg3_type; typedef A2 arg4_type; typedef A3 arg5_type; typedef A4 arg6_type; typedef A5 arg7_type; 
	enum{ arity = 7};

	R operator()(C* self , A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5) const{
		return (self->*fun_)(a0, a1, a2, a3, a4, a5);
	}
};

// コンストラクタを関数オブジェクト化する
template<class T , class A0, class A1, class A2, class A3, class A4, class A5>
struct ctor_functor<T , A0, A1, A2, A3, A4, A5>{
	typedef SmartPtr<T> result_type; typedef A0 arg1_type; typedef A1 arg2_type; typedef A2 arg3_type; typedef A3 arg4_type; typedef A4 arg5_type; typedef A5 arg6_type; 
	enum{ arity = 6};

	SmartPtr<T> operator()(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5) const{
		return xnew<T>(a0, a1, a2, a3, a4, a5);
	}
};

// stdcall版
#if defined(_WIN32) && defined(_M_IX86)

template<int Method, class R , class A0, class A1, class A2, class A3, class A4, class A5>
struct nfun<R (__stdcall *)(A0, A1, A2, A3, A4, A5), Method>
	: nfun_base<6, functor<R (__stdcall *)(A0, A1, A2, A3, A4, A5)>, Method, R>{};

// 関数を関数オブジェクト化する
template<class R , class A0, class A1, class A2, class A3, class A4, class A5>
struct functor<R (__stdcall *)(A0, A1, A2, A3, A4, A5)>{
	typedef R result_type; typedef A0 arg1_type; typedef A1 arg2_type; typedef A2 arg3_type; typedef A3 arg4_type; typedef A4 arg5_type; typedef A5 arg6_type; 
	typedef R (__stdcall *fun_type)(A0, A1, A2, A3, A4, A5);
	enum{ arity = 6};

	fun_type fun_;
	functor(fun_type fun):fun_(fun){}

	R operator()(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5) const{
		return fun_(a0, a1, a2, a3, a4, a5);
	}
};

// 関数を関数オブジェクト化する
template<class R , class A0, class A1, class A2, class A3, class A4, class A5, R (__stdcall *fun_)(A0, A1, A2, A3, A4, A5)>
struct static_functor<R (__stdcall *)(A0, A1, A2, A3, A4, A5), fun_>{
	typedef R result_type; typedef A0 arg1_type; typedef A1 arg2_type; typedef A2 arg3_type; typedef A3 arg4_type; typedef A4 arg5_type; typedef A5 arg6_type; 
	enum{ arity = 6};

	R operator()(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5) const{
		return fun_(a0, a1, a2, a3, a4, a5);
	}
};

#endif


template<int Method, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6>
struct nfun<R (*)(A0, A1, A2, A3, A4, A5, A6), Method>
	: nfun_base<7, functor<R (*)(A0, A1, A2, A3, A4, A5, A6)>, Method, R>{};

// 関数を関数オブジェクト化する
template<class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6>
struct functor<R (*)(A0, A1, A2, A3, A4, A5, A6)>{
	typedef R result_type; typedef A0 arg1_type; typedef A1 arg2_type; typedef A2 arg3_type; typedef A3 arg4_type; typedef A4 arg5_type; typedef A5 arg6_type; typedef A6 arg7_type; 
	typedef R (*fun_type)(A0, A1, A2, A3, A4, A5, A6);
	enum{ arity = 7};

	fun_type fun_;
	functor(fun_type fun):fun_(fun){}

	R operator()(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6) const{
		return fun_(a0, a1, a2, a3, a4, a5, a6);
	}
};

template<int Method, class R, class C , class A0, class A1, class A2, class A3, class A4, class A5, class A6>
struct nfun<R (C::*)(A0, A1, A2, A3, A4, A5, A6), Method>
	: nfun_base<8, functor<R (C::*)(A0, A1, A2, A3, A4, A5, A6)>, Method, R>{};

// メンバ関数を関数オブジェクト化する
template<class R, class C , class A0, class A1, class A2, class A3, class A4, class A5, class A6>
struct functor<R (C::*)(A0, A1, A2, A3, A4, A5, A6)>{
	typedef R result_type; typedef C* arg1_type; typedef A0 arg2_type; typedef A1 arg3_type; typedef A2 arg4_type; typedef A3 arg5_type; typedef A4 arg6_type; typedef A5 arg7_type; typedef A6 arg8_type; 
	typedef R (C::*fun_type)(A0, A1, A2, A3, A4, A5, A6);
	enum{ arity = 8};

	fun_type fun_;
	functor(fun_type fun):fun_(fun){}

	R operator()(C* self , A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6) const{
		return (self->*fun_)(a0, a1, a2, a3, a4, a5, a6);
	}
};

template<int Method, class R, class C , class A0, class A1, class A2, class A3, class A4, class A5, class A6>
struct nfun<R (C::*)(A0, A1, A2, A3, A4, A5, A6) const, Method>
	: nfun_base<8, functor<R (C::*)(A0, A1, A2, A3, A4, A5, A6) const>, Method, R>{};

// constメンバ関数を関数オブジェクト化する
template<class R, class C , class A0, class A1, class A2, class A3, class A4, class A5, class A6>
struct functor<R (C::*)(A0, A1, A2, A3, A4, A5, A6) const>{
	typedef R result_type; typedef C* arg1_type; typedef A0 arg2_type; typedef A1 arg3_type; typedef A2 arg4_type; typedef A3 arg5_type; typedef A4 arg6_type; typedef A5 arg7_type; typedef A6 arg8_type; 
	typedef R (C::*fun_type)(A0, A1, A2, A3, A4, A5, A6) const;
	enum{ arity = 8};

	fun_type fun_;
	functor(fun_type fun):fun_(fun){}

	R operator()(C* self , A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6) const{
		return (self->*fun_)(a0, a1, a2, a3, a4, a5, a6);
	}
};

////

// 関数を関数オブジェクト化する
template<class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, R (*fun_)(A0, A1, A2, A3, A4, A5, A6)>
struct static_functor<R (*)(A0, A1, A2, A3, A4, A5, A6), fun_>{
	typedef R result_type; typedef A0 arg1_type; typedef A1 arg2_type; typedef A2 arg3_type; typedef A3 arg4_type; typedef A4 arg5_type; typedef A5 arg6_type; typedef A6 arg7_type; 
	enum{ arity = 7};

	R operator()(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6) const{
		return fun_(a0, a1, a2, a3, a4, a5, a6);
	}
};

// メンバ関数を関数オブジェクト化する
template<class R, class C , class A0, class A1, class A2, class A3, class A4, class A5, class A6, R (C::*fun_)(A0, A1, A2, A3, A4, A5, A6)>
struct static_functor<R (C::*)(A0, A1, A2, A3, A4, A5, A6), fun_>{
	typedef R result_type; typedef C* arg1_type; typedef A0 arg2_type; typedef A1 arg3_type; typedef A2 arg4_type; typedef A3 arg5_type; typedef A4 arg6_type; typedef A5 arg7_type; typedef A6 arg8_type; 
	enum{ arity = 8};

	R operator()(C* self , A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6) const{
		return (self->*fun_)(a0, a1, a2, a3, a4, a5, a6);
	}
};

// constメンバ関数を関数オブジェクト化する
template<class R, class C , class A0, class A1, class A2, class A3, class A4, class A5, class A6, R (C::*fun_)(A0, A1, A2, A3, A4, A5, A6) const>
struct static_functor<R (C::*)(A0, A1, A2, A3, A4, A5, A6) const, fun_>{
	typedef R result_type; typedef C* arg1_type; typedef A0 arg2_type; typedef A1 arg3_type; typedef A2 arg4_type; typedef A3 arg5_type; typedef A4 arg6_type; typedef A5 arg7_type; typedef A6 arg8_type; 
	enum{ arity = 8};

	R operator()(C* self , A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6) const{
		return (self->*fun_)(a0, a1, a2, a3, a4, a5, a6);
	}
};

// コンストラクタを関数オブジェクト化する
template<class T , class A0, class A1, class A2, class A3, class A4, class A5, class A6>
struct ctor_functor<T , A0, A1, A2, A3, A4, A5, A6>{
	typedef SmartPtr<T> result_type; typedef A0 arg1_type; typedef A1 arg2_type; typedef A2 arg3_type; typedef A3 arg4_type; typedef A4 arg5_type; typedef A5 arg6_type; typedef A6 arg7_type; 
	enum{ arity = 7};

	SmartPtr<T> operator()(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6) const{
		return xnew<T>(a0, a1, a2, a3, a4, a5, a6);
	}
};

// stdcall版
#if defined(_WIN32) && defined(_M_IX86)

template<int Method, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6>
struct nfun<R (__stdcall *)(A0, A1, A2, A3, A4, A5, A6), Method>
	: nfun_base<7, functor<R (__stdcall *)(A0, A1, A2, A3, A4, A5, A6)>, Method, R>{};

// 関数を関数オブジェクト化する
template<class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6>
struct functor<R (__stdcall *)(A0, A1, A2, A3, A4, A5, A6)>{
	typedef R result_type; typedef A0 arg1_type; typedef A1 arg2_type; typedef A2 arg3_type; typedef A3 arg4_type; typedef A4 arg5_type; typedef A5 arg6_type; typedef A6 arg7_type; 
	typedef R (__stdcall *fun_type)(A0, A1, A2, A3, A4, A5, A6);
	enum{ arity = 7};

	fun_type fun_;
	functor(fun_type fun):fun_(fun){}

	R operator()(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6) const{
		return fun_(a0, a1, a2, a3, a4, a5, a6);
	}
};

// 関数を関数オブジェクト化する
template<class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, R (__stdcall *fun_)(A0, A1, A2, A3, A4, A5, A6)>
struct static_functor<R (__stdcall *)(A0, A1, A2, A3, A4, A5, A6), fun_>{
	typedef R result_type; typedef A0 arg1_type; typedef A1 arg2_type; typedef A2 arg3_type; typedef A3 arg4_type; typedef A4 arg5_type; typedef A5 arg6_type; typedef A6 arg7_type; 
	enum{ arity = 7};

	R operator()(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6) const{
		return fun_(a0, a1, a2, a3, a4, a5, a6);
	}
};

#endif


template<int Method, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7>
struct nfun<R (*)(A0, A1, A2, A3, A4, A5, A6, A7), Method>
	: nfun_base<8, functor<R (*)(A0, A1, A2, A3, A4, A5, A6, A7)>, Method, R>{};

// 関数を関数オブジェクト化する
template<class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7>
struct functor<R (*)(A0, A1, A2, A3, A4, A5, A6, A7)>{
	typedef R result_type; typedef A0 arg1_type; typedef A1 arg2_type; typedef A2 arg3_type; typedef A3 arg4_type; typedef A4 arg5_type; typedef A5 arg6_type; typedef A6 arg7_type; typedef A7 arg8_type; 
	typedef R (*fun_type)(A0, A1, A2, A3, A4, A5, A6, A7);
	enum{ arity = 8};

	fun_type fun_;
	functor(fun_type fun):fun_(fun){}

	R operator()(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7) const{
		return fun_(a0, a1, a2, a3, a4, a5, a6, a7);
	}
};

template<int Method, class R, class C , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7>
struct nfun<R (C::*)(A0, A1, A2, A3, A4, A5, A6, A7), Method>
	: nfun_base<9, functor<R (C::*)(A0, A1, A2, A3, A4, A5, A6, A7)>, Method, R>{};

// メンバ関数を関数オブジェクト化する
template<class R, class C , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7>
struct functor<R (C::*)(A0, A1, A2, A3, A4, A5, A6, A7)>{
	typedef R result_type; typedef C* arg1_type; typedef A0 arg2_type; typedef A1 arg3_type; typedef A2 arg4_type; typedef A3 arg5_type; typedef A4 arg6_type; typedef A5 arg7_type; typedef A6 arg8_type; typedef A7 arg9_type; 
	typedef R (C::*fun_type)(A0, A1, A2, A3, A4, A5, A6, A7);
	enum{ arity = 9};

	fun_type fun_;
	functor(fun_type fun):fun_(fun){}

	R operator()(C* self , A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7) const{
		return (self->*fun_)(a0, a1, a2, a3, a4, a5, a6, a7);
	}
};

template<int Method, class R, class C , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7>
struct nfun<R (C::*)(A0, A1, A2, A3, A4, A5, A6, A7) const, Method>
	: nfun_base<9, functor<R (C::*)(A0, A1, A2, A3, A4, A5, A6, A7) const>, Method, R>{};

// constメンバ関数を関数オブジェクト化する
template<class R, class C , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7>
struct functor<R (C::*)(A0, A1, A2, A3, A4, A5, A6, A7) const>{
	typedef R result_type; typedef C* arg1_type; typedef A0 arg2_type; typedef A1 arg3_type; typedef A2 arg4_type; typedef A3 arg5_type; typedef A4 arg6_type; typedef A5 arg7_type; typedef A6 arg8_type; typedef A7 arg9_type; 
	typedef R (C::*fun_type)(A0, A1, A2, A3, A4, A5, A6, A7) const;
	enum{ arity = 9};

	fun_type fun_;
	functor(fun_type fun):fun_(fun){}

	R operator()(C* self , A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7) const{
		return (self->*fun_)(a0, a1, a2, a3, a4, a5, a6, a7);
	}
};

////

// 関数を関数オブジェクト化する
template<class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, R (*fun_)(A0, A1, A2, A3, A4, A5, A6, A7)>
struct static_functor<R (*)(A0, A1, A2, A3, A4, A5, A6, A7), fun_>{
	typedef R result_type; typedef A0 arg1_type; typedef A1 arg2_type; typedef A2 arg3_type; typedef A3 arg4_type; typedef A4 arg5_type; typedef A5 arg6_type; typedef A6 arg7_type; typedef A7 arg8_type; 
	enum{ arity = 8};

	R operator()(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7) const{
		return fun_(a0, a1, a2, a3, a4, a5, a6, a7);
	}
};

// メンバ関数を関数オブジェクト化する
template<class R, class C , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, R (C::*fun_)(A0, A1, A2, A3, A4, A5, A6, A7)>
struct static_functor<R (C::*)(A0, A1, A2, A3, A4, A5, A6, A7), fun_>{
	typedef R result_type; typedef C* arg1_type; typedef A0 arg2_type; typedef A1 arg3_type; typedef A2 arg4_type; typedef A3 arg5_type; typedef A4 arg6_type; typedef A5 arg7_type; typedef A6 arg8_type; typedef A7 arg9_type; 
	enum{ arity = 9};

	R operator()(C* self , A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7) const{
		return (self->*fun_)(a0, a1, a2, a3, a4, a5, a6, a7);
	}
};

// constメンバ関数を関数オブジェクト化する
template<class R, class C , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, R (C::*fun_)(A0, A1, A2, A3, A4, A5, A6, A7) const>
struct static_functor<R (C::*)(A0, A1, A2, A3, A4, A5, A6, A7) const, fun_>{
	typedef R result_type; typedef C* arg1_type; typedef A0 arg2_type; typedef A1 arg3_type; typedef A2 arg4_type; typedef A3 arg5_type; typedef A4 arg6_type; typedef A5 arg7_type; typedef A6 arg8_type; typedef A7 arg9_type; 
	enum{ arity = 9};

	R operator()(C* self , A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7) const{
		return (self->*fun_)(a0, a1, a2, a3, a4, a5, a6, a7);
	}
};

// コンストラクタを関数オブジェクト化する
template<class T , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7>
struct ctor_functor<T , A0, A1, A2, A3, A4, A5, A6, A7>{
	typedef SmartPtr<T> result_type; typedef A0 arg1_type; typedef A1 arg2_type; typedef A2 arg3_type; typedef A3 arg4_type; typedef A4 arg5_type; typedef A5 arg6_type; typedef A6 arg7_type; typedef A7 arg8_type; 
	enum{ arity = 8};

	SmartPtr<T> operator()(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7) const{
		return xnew<T>(a0, a1, a2, a3, a4, a5, a6, a7);
	}
};

// stdcall版
#if defined(_WIN32) && defined(_M_IX86)

template<int Method, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7>
struct nfun<R (__stdcall *)(A0, A1, A2, A3, A4, A5, A6, A7), Method>
	: nfun_base<8, functor<R (__stdcall *)(A0, A1, A2, A3, A4, A5, A6, A7)>, Method, R>{};

// 関数を関数オブジェクト化する
template<class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7>
struct functor<R (__stdcall *)(A0, A1, A2, A3, A4, A5, A6, A7)>{
	typedef R result_type; typedef A0 arg1_type; typedef A1 arg2_type; typedef A2 arg3_type; typedef A3 arg4_type; typedef A4 arg5_type; typedef A5 arg6_type; typedef A6 arg7_type; typedef A7 arg8_type; 
	typedef R (__stdcall *fun_type)(A0, A1, A2, A3, A4, A5, A6, A7);
	enum{ arity = 8};

	fun_type fun_;
	functor(fun_type fun):fun_(fun){}

	R operator()(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7) const{
		return fun_(a0, a1, a2, a3, a4, a5, a6, a7);
	}
};

// 関数を関数オブジェクト化する
template<class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, R (__stdcall *fun_)(A0, A1, A2, A3, A4, A5, A6, A7)>
struct static_functor<R (__stdcall *)(A0, A1, A2, A3, A4, A5, A6, A7), fun_>{
	typedef R result_type; typedef A0 arg1_type; typedef A1 arg2_type; typedef A2 arg3_type; typedef A3 arg4_type; typedef A4 arg5_type; typedef A5 arg6_type; typedef A6 arg7_type; typedef A7 arg8_type; 
	enum{ arity = 8};

	R operator()(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7) const{
		return fun_(a0, a1, a2, a3, a4, a5, a6, a7);
	}
};

#endif


template<int Method, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8>
struct nfun<R (*)(A0, A1, A2, A3, A4, A5, A6, A7, A8), Method>
	: nfun_base<9, functor<R (*)(A0, A1, A2, A3, A4, A5, A6, A7, A8)>, Method, R>{};

// 関数を関数オブジェクト化する
template<class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8>
struct functor<R (*)(A0, A1, A2, A3, A4, A5, A6, A7, A8)>{
	typedef R result_type; typedef A0 arg1_type; typedef A1 arg2_type; typedef A2 arg3_type; typedef A3 arg4_type; typedef A4 arg5_type; typedef A5 arg6_type; typedef A6 arg7_type; typedef A7 arg8_type; typedef A8 arg9_type; 
	typedef R (*fun_type)(A0, A1, A2, A3, A4, A5, A6, A7, A8);
	enum{ arity = 9};

	fun_type fun_;
	functor(fun_type fun):fun_(fun){}

	R operator()(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8) const{
		return fun_(a0, a1, a2, a3, a4, a5, a6, a7, a8);
	}
};

template<int Method, class R, class C , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8>
struct nfun<R (C::*)(A0, A1, A2, A3, A4, A5, A6, A7, A8), Method>
	: nfun_base<10, functor<R (C::*)(A0, A1, A2, A3, A4, A5, A6, A7, A8)>, Method, R>{};

// メンバ関数を関数オブジェクト化する
template<class R, class C , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8>
struct functor<R (C::*)(A0, A1, A2, A3, A4, A5, A6, A7, A8)>{
	typedef R result_type; typedef C* arg1_type; typedef A0 arg2_type; typedef A1 arg3_type; typedef A2 arg4_type; typedef A3 arg5_type; typedef A4 arg6_type; typedef A5 arg7_type; typedef A6 arg8_type; typedef A7 arg9_type; typedef A8 arg10_type; 
	typedef R (C::*fun_type)(A0, A1, A2, A3, A4, A5, A6, A7, A8);
	enum{ arity = 10};

	fun_type fun_;
	functor(fun_type fun):fun_(fun){}

	R operator()(C* self , A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8) const{
		return (self->*fun_)(a0, a1, a2, a3, a4, a5, a6, a7, a8);
	}
};

template<int Method, class R, class C , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8>
struct nfun<R (C::*)(A0, A1, A2, A3, A4, A5, A6, A7, A8) const, Method>
	: nfun_base<10, functor<R (C::*)(A0, A1, A2, A3, A4, A5, A6, A7, A8) const>, Method, R>{};

// constメンバ関数を関数オブジェクト化する
template<class R, class C , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8>
struct functor<R (C::*)(A0, A1, A2, A3, A4, A5, A6, A7, A8) const>{
	typedef R result_type; typedef C* arg1_type; typedef A0 arg2_type; typedef A1 arg3_type; typedef A2 arg4_type; typedef A3 arg5_type; typedef A4 arg6_type; typedef A5 arg7_type; typedef A6 arg8_type; typedef A7 arg9_type; typedef A8 arg10_type; 
	typedef R (C::*fun_type)(A0, A1, A2, A3, A4, A5, A6, A7, A8) const;
	enum{ arity = 10};

	fun_type fun_;
	functor(fun_type fun):fun_(fun){}

	R operator()(C* self , A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8) const{
		return (self->*fun_)(a0, a1, a2, a3, a4, a5, a6, a7, a8);
	}
};

////

// 関数を関数オブジェクト化する
template<class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, R (*fun_)(A0, A1, A2, A3, A4, A5, A6, A7, A8)>
struct static_functor<R (*)(A0, A1, A2, A3, A4, A5, A6, A7, A8), fun_>{
	typedef R result_type; typedef A0 arg1_type; typedef A1 arg2_type; typedef A2 arg3_type; typedef A3 arg4_type; typedef A4 arg5_type; typedef A5 arg6_type; typedef A6 arg7_type; typedef A7 arg8_type; typedef A8 arg9_type; 
	enum{ arity = 9};

	R operator()(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8) const{
		return fun_(a0, a1, a2, a3, a4, a5, a6, a7, a8);
	}
};

// メンバ関数を関数オブジェクト化する
template<class R, class C , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, R (C::*fun_)(A0, A1, A2, A3, A4, A5, A6, A7, A8)>
struct static_functor<R (C::*)(A0, A1, A2, A3, A4, A5, A6, A7, A8), fun_>{
	typedef R result_type; typedef C* arg1_type; typedef A0 arg2_type; typedef A1 arg3_type; typedef A2 arg4_type; typedef A3 arg5_type; typedef A4 arg6_type; typedef A5 arg7_type; typedef A6 arg8_type; typedef A7 arg9_type; typedef A8 arg10_type; 
	enum{ arity = 10};

	R operator()(C* self , A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8) const{
		return (self->*fun_)(a0, a1, a2, a3, a4, a5, a6, a7, a8);
	}
};

// constメンバ関数を関数オブジェクト化する
template<class R, class C , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, R (C::*fun_)(A0, A1, A2, A3, A4, A5, A6, A7, A8) const>
struct static_functor<R (C::*)(A0, A1, A2, A3, A4, A5, A6, A7, A8) const, fun_>{
	typedef R result_type; typedef C* arg1_type; typedef A0 arg2_type; typedef A1 arg3_type; typedef A2 arg4_type; typedef A3 arg5_type; typedef A4 arg6_type; typedef A5 arg7_type; typedef A6 arg8_type; typedef A7 arg9_type; typedef A8 arg10_type; 
	enum{ arity = 10};

	R operator()(C* self , A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8) const{
		return (self->*fun_)(a0, a1, a2, a3, a4, a5, a6, a7, a8);
	}
};

// コンストラクタを関数オブジェクト化する
template<class T , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8>
struct ctor_functor<T , A0, A1, A2, A3, A4, A5, A6, A7, A8>{
	typedef SmartPtr<T> result_type; typedef A0 arg1_type; typedef A1 arg2_type; typedef A2 arg3_type; typedef A3 arg4_type; typedef A4 arg5_type; typedef A5 arg6_type; typedef A6 arg7_type; typedef A7 arg8_type; typedef A8 arg9_type; 
	enum{ arity = 9};

	SmartPtr<T> operator()(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8) const{
		return xnew<T>(a0, a1, a2, a3, a4, a5, a6, a7, a8);
	}
};

// stdcall版
#if defined(_WIN32) && defined(_M_IX86)

template<int Method, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8>
struct nfun<R (__stdcall *)(A0, A1, A2, A3, A4, A5, A6, A7, A8), Method>
	: nfun_base<9, functor<R (__stdcall *)(A0, A1, A2, A3, A4, A5, A6, A7, A8)>, Method, R>{};

// 関数を関数オブジェクト化する
template<class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8>
struct functor<R (__stdcall *)(A0, A1, A2, A3, A4, A5, A6, A7, A8)>{
	typedef R result_type; typedef A0 arg1_type; typedef A1 arg2_type; typedef A2 arg3_type; typedef A3 arg4_type; typedef A4 arg5_type; typedef A5 arg6_type; typedef A6 arg7_type; typedef A7 arg8_type; typedef A8 arg9_type; 
	typedef R (__stdcall *fun_type)(A0, A1, A2, A3, A4, A5, A6, A7, A8);
	enum{ arity = 9};

	fun_type fun_;
	functor(fun_type fun):fun_(fun){}

	R operator()(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8) const{
		return fun_(a0, a1, a2, a3, a4, a5, a6, a7, a8);
	}
};

// 関数を関数オブジェクト化する
template<class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, R (__stdcall *fun_)(A0, A1, A2, A3, A4, A5, A6, A7, A8)>
struct static_functor<R (__stdcall *)(A0, A1, A2, A3, A4, A5, A6, A7, A8), fun_>{
	typedef R result_type; typedef A0 arg1_type; typedef A1 arg2_type; typedef A2 arg3_type; typedef A3 arg4_type; typedef A4 arg5_type; typedef A5 arg6_type; typedef A6 arg7_type; typedef A7 arg8_type; typedef A8 arg9_type; 
	enum{ arity = 9};

	R operator()(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8) const{
		return fun_(a0, a1, a2, a3, a4, a5, a6, a7, a8);
	}
};

#endif


template<int Method, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9>
struct nfun<R (*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9), Method>
	: nfun_base<10, functor<R (*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9)>, Method, R>{};

// 関数を関数オブジェクト化する
template<class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9>
struct functor<R (*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9)>{
	typedef R result_type; typedef A0 arg1_type; typedef A1 arg2_type; typedef A2 arg3_type; typedef A3 arg4_type; typedef A4 arg5_type; typedef A5 arg6_type; typedef A6 arg7_type; typedef A7 arg8_type; typedef A8 arg9_type; typedef A9 arg10_type; 
	typedef R (*fun_type)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9);
	enum{ arity = 10};

	fun_type fun_;
	functor(fun_type fun):fun_(fun){}

	R operator()(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9) const{
		return fun_(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9);
	}
};

template<int Method, class R, class C , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9>
struct nfun<R (C::*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9), Method>
	: nfun_base<11, functor<R (C::*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9)>, Method, R>{};

// メンバ関数を関数オブジェクト化する
template<class R, class C , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9>
struct functor<R (C::*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9)>{
	typedef R result_type; typedef C* arg1_type; typedef A0 arg2_type; typedef A1 arg3_type; typedef A2 arg4_type; typedef A3 arg5_type; typedef A4 arg6_type; typedef A5 arg7_type; typedef A6 arg8_type; typedef A7 arg9_type; typedef A8 arg10_type; typedef A9 arg11_type; 
	typedef R (C::*fun_type)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9);
	enum{ arity = 11};

	fun_type fun_;
	functor(fun_type fun):fun_(fun){}

	R operator()(C* self , A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9) const{
		return (self->*fun_)(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9);
	}
};

template<int Method, class R, class C , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9>
struct nfun<R (C::*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9) const, Method>
	: nfun_base<11, functor<R (C::*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9) const>, Method, R>{};

// constメンバ関数を関数オブジェクト化する
template<class R, class C , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9>
struct functor<R (C::*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9) const>{
	typedef R result_type; typedef C* arg1_type; typedef A0 arg2_type; typedef A1 arg3_type; typedef A2 arg4_type; typedef A3 arg5_type; typedef A4 arg6_type; typedef A5 arg7_type; typedef A6 arg8_type; typedef A7 arg9_type; typedef A8 arg10_type; typedef A9 arg11_type; 
	typedef R (C::*fun_type)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9) const;
	enum{ arity = 11};

	fun_type fun_;
	functor(fun_type fun):fun_(fun){}

	R operator()(C* self , A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9) const{
		return (self->*fun_)(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9);
	}
};

////

// 関数を関数オブジェクト化する
template<class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, R (*fun_)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9)>
struct static_functor<R (*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9), fun_>{
	typedef R result_type; typedef A0 arg1_type; typedef A1 arg2_type; typedef A2 arg3_type; typedef A3 arg4_type; typedef A4 arg5_type; typedef A5 arg6_type; typedef A6 arg7_type; typedef A7 arg8_type; typedef A8 arg9_type; typedef A9 arg10_type; 
	enum{ arity = 10};

	R operator()(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9) const{
		return fun_(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9);
	}
};

// メンバ関数を関数オブジェクト化する
template<class R, class C , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, R (C::*fun_)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9)>
struct static_functor<R (C::*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9), fun_>{
	typedef R result_type; typedef C* arg1_type; typedef A0 arg2_type; typedef A1 arg3_type; typedef A2 arg4_type; typedef A3 arg5_type; typedef A4 arg6_type; typedef A5 arg7_type; typedef A6 arg8_type; typedef A7 arg9_type; typedef A8 arg10_type; typedef A9 arg11_type; 
	enum{ arity = 11};

	R operator()(C* self , A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9) const{
		return (self->*fun_)(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9);
	}
};

// constメンバ関数を関数オブジェクト化する
template<class R, class C , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, R (C::*fun_)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9) const>
struct static_functor<R (C::*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9) const, fun_>{
	typedef R result_type; typedef C* arg1_type; typedef A0 arg2_type; typedef A1 arg3_type; typedef A2 arg4_type; typedef A3 arg5_type; typedef A4 arg6_type; typedef A5 arg7_type; typedef A6 arg8_type; typedef A7 arg9_type; typedef A8 arg10_type; typedef A9 arg11_type; 
	enum{ arity = 11};

	R operator()(C* self , A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9) const{
		return (self->*fun_)(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9);
	}
};

// コンストラクタを関数オブジェクト化する
template<class T , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9>
struct ctor_functor<T , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9>{
	typedef SmartPtr<T> result_type; typedef A0 arg1_type; typedef A1 arg2_type; typedef A2 arg3_type; typedef A3 arg4_type; typedef A4 arg5_type; typedef A5 arg6_type; typedef A6 arg7_type; typedef A7 arg8_type; typedef A8 arg9_type; typedef A9 arg10_type; 
	enum{ arity = 10};

	SmartPtr<T> operator()(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9) const{
		return xnew<T>(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9);
	}
};

// stdcall版
#if defined(_WIN32) && defined(_M_IX86)

template<int Method, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9>
struct nfun<R (__stdcall *)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9), Method>
	: nfun_base<10, functor<R (__stdcall *)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9)>, Method, R>{};

// 関数を関数オブジェクト化する
template<class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9>
struct functor<R (__stdcall *)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9)>{
	typedef R result_type; typedef A0 arg1_type; typedef A1 arg2_type; typedef A2 arg3_type; typedef A3 arg4_type; typedef A4 arg5_type; typedef A5 arg6_type; typedef A6 arg7_type; typedef A7 arg8_type; typedef A8 arg9_type; typedef A9 arg10_type; 
	typedef R (__stdcall *fun_type)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9);
	enum{ arity = 10};

	fun_type fun_;
	functor(fun_type fun):fun_(fun){}

	R operator()(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9) const{
		return fun_(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9);
	}
};

// 関数を関数オブジェクト化する
template<class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, R (__stdcall *fun_)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9)>
struct static_functor<R (__stdcall *)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9), fun_>{
	typedef R result_type; typedef A0 arg1_type; typedef A1 arg2_type; typedef A2 arg3_type; typedef A3 arg4_type; typedef A4 arg5_type; typedef A5 arg6_type; typedef A6 arg7_type; typedef A7 arg8_type; typedef A8 arg9_type; typedef A9 arg10_type; 
	enum{ arity = 10};

	R operator()(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9) const{
		return fun_(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9);
	}
};

#endif


template<int Method, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10>
struct nfun<R (*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10), Method>
	: nfun_base<11, functor<R (*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10)>, Method, R>{};

// 関数を関数オブジェクト化する
template<class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10>
struct functor<R (*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10)>{
	typedef R result_type; typedef A0 arg1_type; typedef A1 arg2_type; typedef A2 arg3_type; typedef A3 arg4_type; typedef A4 arg5_type; typedef A5 arg6_type; typedef A6 arg7_type; typedef A7 arg8_type; typedef A8 arg9_type; typedef A9 arg10_type; typedef A10 arg11_type; 
	typedef R (*fun_type)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10);
	enum{ arity = 11};

	fun_type fun_;
	functor(fun_type fun):fun_(fun){}

	R operator()(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10) const{
		return fun_(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
	}
};

template<int Method, class R, class C , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10>
struct nfun<R (C::*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10), Method>
	: nfun_base<12, functor<R (C::*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10)>, Method, R>{};

// メンバ関数を関数オブジェクト化する
template<class R, class C , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10>
struct functor<R (C::*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10)>{
	typedef R result_type; typedef C* arg1_type; typedef A0 arg2_type; typedef A1 arg3_type; typedef A2 arg4_type; typedef A3 arg5_type; typedef A4 arg6_type; typedef A5 arg7_type; typedef A6 arg8_type; typedef A7 arg9_type; typedef A8 arg10_type; typedef A9 arg11_type; typedef A10 arg12_type; 
	typedef R (C::*fun_type)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10);
	enum{ arity = 12};

	fun_type fun_;
	functor(fun_type fun):fun_(fun){}

	R operator()(C* self , A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10) const{
		return (self->*fun_)(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
	}
};

template<int Method, class R, class C , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10>
struct nfun<R (C::*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10) const, Method>
	: nfun_base<12, functor<R (C::*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10) const>, Method, R>{};

// constメンバ関数を関数オブジェクト化する
template<class R, class C , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10>
struct functor<R (C::*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10) const>{
	typedef R result_type; typedef C* arg1_type; typedef A0 arg2_type; typedef A1 arg3_type; typedef A2 arg4_type; typedef A3 arg5_type; typedef A4 arg6_type; typedef A5 arg7_type; typedef A6 arg8_type; typedef A7 arg9_type; typedef A8 arg10_type; typedef A9 arg11_type; typedef A10 arg12_type; 
	typedef R (C::*fun_type)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10) const;
	enum{ arity = 12};

	fun_type fun_;
	functor(fun_type fun):fun_(fun){}

	R operator()(C* self , A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10) const{
		return (self->*fun_)(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
	}
};

////

// 関数を関数オブジェクト化する
template<class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, R (*fun_)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10)>
struct static_functor<R (*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10), fun_>{
	typedef R result_type; typedef A0 arg1_type; typedef A1 arg2_type; typedef A2 arg3_type; typedef A3 arg4_type; typedef A4 arg5_type; typedef A5 arg6_type; typedef A6 arg7_type; typedef A7 arg8_type; typedef A8 arg9_type; typedef A9 arg10_type; typedef A10 arg11_type; 
	enum{ arity = 11};

	R operator()(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10) const{
		return fun_(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
	}
};

// メンバ関数を関数オブジェクト化する
template<class R, class C , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, R (C::*fun_)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10)>
struct static_functor<R (C::*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10), fun_>{
	typedef R result_type; typedef C* arg1_type; typedef A0 arg2_type; typedef A1 arg3_type; typedef A2 arg4_type; typedef A3 arg5_type; typedef A4 arg6_type; typedef A5 arg7_type; typedef A6 arg8_type; typedef A7 arg9_type; typedef A8 arg10_type; typedef A9 arg11_type; typedef A10 arg12_type; 
	enum{ arity = 12};

	R operator()(C* self , A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10) const{
		return (self->*fun_)(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
	}
};

// constメンバ関数を関数オブジェクト化する
template<class R, class C , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, R (C::*fun_)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10) const>
struct static_functor<R (C::*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10) const, fun_>{
	typedef R result_type; typedef C* arg1_type; typedef A0 arg2_type; typedef A1 arg3_type; typedef A2 arg4_type; typedef A3 arg5_type; typedef A4 arg6_type; typedef A5 arg7_type; typedef A6 arg8_type; typedef A7 arg9_type; typedef A8 arg10_type; typedef A9 arg11_type; typedef A10 arg12_type; 
	enum{ arity = 12};

	R operator()(C* self , A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10) const{
		return (self->*fun_)(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
	}
};

// コンストラクタを関数オブジェクト化する
template<class T , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10>
struct ctor_functor<T , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10>{
	typedef SmartPtr<T> result_type; typedef A0 arg1_type; typedef A1 arg2_type; typedef A2 arg3_type; typedef A3 arg4_type; typedef A4 arg5_type; typedef A5 arg6_type; typedef A6 arg7_type; typedef A7 arg8_type; typedef A8 arg9_type; typedef A9 arg10_type; typedef A10 arg11_type; 
	enum{ arity = 11};

	SmartPtr<T> operator()(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10) const{
		return xnew<T>(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
	}
};

// stdcall版
#if defined(_WIN32) && defined(_M_IX86)

template<int Method, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10>
struct nfun<R (__stdcall *)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10), Method>
	: nfun_base<11, functor<R (__stdcall *)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10)>, Method, R>{};

// 関数を関数オブジェクト化する
template<class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10>
struct functor<R (__stdcall *)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10)>{
	typedef R result_type; typedef A0 arg1_type; typedef A1 arg2_type; typedef A2 arg3_type; typedef A3 arg4_type; typedef A4 arg5_type; typedef A5 arg6_type; typedef A6 arg7_type; typedef A7 arg8_type; typedef A8 arg9_type; typedef A9 arg10_type; typedef A10 arg11_type; 
	typedef R (__stdcall *fun_type)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10);
	enum{ arity = 11};

	fun_type fun_;
	functor(fun_type fun):fun_(fun){}

	R operator()(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10) const{
		return fun_(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
	}
};

// 関数を関数オブジェクト化する
template<class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, R (__stdcall *fun_)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10)>
struct static_functor<R (__stdcall *)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10), fun_>{
	typedef R result_type; typedef A0 arg1_type; typedef A1 arg2_type; typedef A2 arg3_type; typedef A3 arg4_type; typedef A4 arg5_type; typedef A5 arg6_type; typedef A6 arg7_type; typedef A7 arg8_type; typedef A8 arg9_type; typedef A9 arg10_type; typedef A10 arg11_type; 
	enum{ arity = 11};

	R operator()(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10) const{
		return fun_(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
	}
};

#endif


template<int Method, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11>
struct nfun<R (*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11), Method>
	: nfun_base<12, functor<R (*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11)>, Method, R>{};

// 関数を関数オブジェクト化する
template<class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11>
struct functor<R (*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11)>{
	typedef R result_type; typedef A0 arg1_type; typedef A1 arg2_type; typedef A2 arg3_type; typedef A3 arg4_type; typedef A4 arg5_type; typedef A5 arg6_type; typedef A6 arg7_type; typedef A7 arg8_type; typedef A8 arg9_type; typedef A9 arg10_type; typedef A10 arg11_type; typedef A11 arg12_type; 
	typedef R (*fun_type)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11);
	enum{ arity = 12};

	fun_type fun_;
	functor(fun_type fun):fun_(fun){}

	R operator()(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11) const{
		return fun_(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11);
	}
};

template<int Method, class R, class C , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11>
struct nfun<R (C::*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11), Method>
	: nfun_base<13, functor<R (C::*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11)>, Method, R>{};

// メンバ関数を関数オブジェクト化する
template<class R, class C , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11>
struct functor<R (C::*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11)>{
	typedef R result_type; typedef C* arg1_type; typedef A0 arg2_type; typedef A1 arg3_type; typedef A2 arg4_type; typedef A3 arg5_type; typedef A4 arg6_type; typedef A5 arg7_type; typedef A6 arg8_type; typedef A7 arg9_type; typedef A8 arg10_type; typedef A9 arg11_type; typedef A10 arg12_type; typedef A11 arg13_type; 
	typedef R (C::*fun_type)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11);
	enum{ arity = 13};

	fun_type fun_;
	functor(fun_type fun):fun_(fun){}

	R operator()(C* self , A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11) const{
		return (self->*fun_)(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11);
	}
};

template<int Method, class R, class C , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11>
struct nfun<R (C::*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11) const, Method>
	: nfun_base<13, functor<R (C::*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11) const>, Method, R>{};

// constメンバ関数を関数オブジェクト化する
template<class R, class C , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11>
struct functor<R (C::*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11) const>{
	typedef R result_type; typedef C* arg1_type; typedef A0 arg2_type; typedef A1 arg3_type; typedef A2 arg4_type; typedef A3 arg5_type; typedef A4 arg6_type; typedef A5 arg7_type; typedef A6 arg8_type; typedef A7 arg9_type; typedef A8 arg10_type; typedef A9 arg11_type; typedef A10 arg12_type; typedef A11 arg13_type; 
	typedef R (C::*fun_type)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11) const;
	enum{ arity = 13};

	fun_type fun_;
	functor(fun_type fun):fun_(fun){}

	R operator()(C* self , A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11) const{
		return (self->*fun_)(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11);
	}
};

////

// 関数を関数オブジェクト化する
template<class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, R (*fun_)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11)>
struct static_functor<R (*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11), fun_>{
	typedef R result_type; typedef A0 arg1_type; typedef A1 arg2_type; typedef A2 arg3_type; typedef A3 arg4_type; typedef A4 arg5_type; typedef A5 arg6_type; typedef A6 arg7_type; typedef A7 arg8_type; typedef A8 arg9_type; typedef A9 arg10_type; typedef A10 arg11_type; typedef A11 arg12_type; 
	enum{ arity = 12};

	R operator()(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11) const{
		return fun_(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11);
	}
};

// メンバ関数を関数オブジェクト化する
template<class R, class C , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, R (C::*fun_)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11)>
struct static_functor<R (C::*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11), fun_>{
	typedef R result_type; typedef C* arg1_type; typedef A0 arg2_type; typedef A1 arg3_type; typedef A2 arg4_type; typedef A3 arg5_type; typedef A4 arg6_type; typedef A5 arg7_type; typedef A6 arg8_type; typedef A7 arg9_type; typedef A8 arg10_type; typedef A9 arg11_type; typedef A10 arg12_type; typedef A11 arg13_type; 
	enum{ arity = 13};

	R operator()(C* self , A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11) const{
		return (self->*fun_)(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11);
	}
};

// constメンバ関数を関数オブジェクト化する
template<class R, class C , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, R (C::*fun_)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11) const>
struct static_functor<R (C::*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11) const, fun_>{
	typedef R result_type; typedef C* arg1_type; typedef A0 arg2_type; typedef A1 arg3_type; typedef A2 arg4_type; typedef A3 arg5_type; typedef A4 arg6_type; typedef A5 arg7_type; typedef A6 arg8_type; typedef A7 arg9_type; typedef A8 arg10_type; typedef A9 arg11_type; typedef A10 arg12_type; typedef A11 arg13_type; 
	enum{ arity = 13};

	R operator()(C* self , A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11) const{
		return (self->*fun_)(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11);
	}
};

// コンストラクタを関数オブジェクト化する
template<class T , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11>
struct ctor_functor<T , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11>{
	typedef SmartPtr<T> result_type; typedef A0 arg1_type; typedef A1 arg2_type; typedef A2 arg3_type; typedef A3 arg4_type; typedef A4 arg5_type; typedef A5 arg6_type; typedef A6 arg7_type; typedef A7 arg8_type; typedef A8 arg9_type; typedef A9 arg10_type; typedef A10 arg11_type; typedef A11 arg12_type; 
	enum{ arity = 12};

	SmartPtr<T> operator()(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11) const{
		return xnew<T>(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11);
	}
};

// stdcall版
#if defined(_WIN32) && defined(_M_IX86)

template<int Method, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11>
struct nfun<R (__stdcall *)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11), Method>
	: nfun_base<12, functor<R (__stdcall *)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11)>, Method, R>{};

// 関数を関数オブジェクト化する
template<class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11>
struct functor<R (__stdcall *)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11)>{
	typedef R result_type; typedef A0 arg1_type; typedef A1 arg2_type; typedef A2 arg3_type; typedef A3 arg4_type; typedef A4 arg5_type; typedef A5 arg6_type; typedef A6 arg7_type; typedef A7 arg8_type; typedef A8 arg9_type; typedef A9 arg10_type; typedef A10 arg11_type; typedef A11 arg12_type; 
	typedef R (__stdcall *fun_type)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11);
	enum{ arity = 12};

	fun_type fun_;
	functor(fun_type fun):fun_(fun){}

	R operator()(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11) const{
		return fun_(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11);
	}
};

// 関数を関数オブジェクト化する
template<class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, R (__stdcall *fun_)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11)>
struct static_functor<R (__stdcall *)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11), fun_>{
	typedef R result_type; typedef A0 arg1_type; typedef A1 arg2_type; typedef A2 arg3_type; typedef A3 arg4_type; typedef A4 arg5_type; typedef A5 arg6_type; typedef A6 arg7_type; typedef A7 arg8_type; typedef A8 arg9_type; typedef A9 arg10_type; typedef A10 arg11_type; typedef A11 arg12_type; 
	enum{ arity = 12};

	R operator()(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11) const{
		return fun_(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11);
	}
};

#endif


template<int Method, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12>
struct nfun<R (*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12), Method>
	: nfun_base<13, functor<R (*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12)>, Method, R>{};

// 関数を関数オブジェクト化する
template<class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12>
struct functor<R (*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12)>{
	typedef R result_type; typedef A0 arg1_type; typedef A1 arg2_type; typedef A2 arg3_type; typedef A3 arg4_type; typedef A4 arg5_type; typedef A5 arg6_type; typedef A6 arg7_type; typedef A7 arg8_type; typedef A8 arg9_type; typedef A9 arg10_type; typedef A10 arg11_type; typedef A11 arg12_type; typedef A12 arg13_type; 
	typedef R (*fun_type)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12);
	enum{ arity = 13};

	fun_type fun_;
	functor(fun_type fun):fun_(fun){}

	R operator()(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12) const{
		return fun_(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12);
	}
};

template<int Method, class R, class C , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12>
struct nfun<R (C::*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12), Method>
	: nfun_base<14, functor<R (C::*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12)>, Method, R>{};

// メンバ関数を関数オブジェクト化する
template<class R, class C , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12>
struct functor<R (C::*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12)>{
	typedef R result_type; typedef C* arg1_type; typedef A0 arg2_type; typedef A1 arg3_type; typedef A2 arg4_type; typedef A3 arg5_type; typedef A4 arg6_type; typedef A5 arg7_type; typedef A6 arg8_type; typedef A7 arg9_type; typedef A8 arg10_type; typedef A9 arg11_type; typedef A10 arg12_type; typedef A11 arg13_type; typedef A12 arg14_type; 
	typedef R (C::*fun_type)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12);
	enum{ arity = 14};

	fun_type fun_;
	functor(fun_type fun):fun_(fun){}

	R operator()(C* self , A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12) const{
		return (self->*fun_)(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12);
	}
};

template<int Method, class R, class C , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12>
struct nfun<R (C::*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12) const, Method>
	: nfun_base<14, functor<R (C::*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12) const>, Method, R>{};

// constメンバ関数を関数オブジェクト化する
template<class R, class C , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12>
struct functor<R (C::*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12) const>{
	typedef R result_type; typedef C* arg1_type; typedef A0 arg2_type; typedef A1 arg3_type; typedef A2 arg4_type; typedef A3 arg5_type; typedef A4 arg6_type; typedef A5 arg7_type; typedef A6 arg8_type; typedef A7 arg9_type; typedef A8 arg10_type; typedef A9 arg11_type; typedef A10 arg12_type; typedef A11 arg13_type; typedef A12 arg14_type; 
	typedef R (C::*fun_type)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12) const;
	enum{ arity = 14};

	fun_type fun_;
	functor(fun_type fun):fun_(fun){}

	R operator()(C* self , A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12) const{
		return (self->*fun_)(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12);
	}
};

////

// 関数を関数オブジェクト化する
template<class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, R (*fun_)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12)>
struct static_functor<R (*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12), fun_>{
	typedef R result_type; typedef A0 arg1_type; typedef A1 arg2_type; typedef A2 arg3_type; typedef A3 arg4_type; typedef A4 arg5_type; typedef A5 arg6_type; typedef A6 arg7_type; typedef A7 arg8_type; typedef A8 arg9_type; typedef A9 arg10_type; typedef A10 arg11_type; typedef A11 arg12_type; typedef A12 arg13_type; 
	enum{ arity = 13};

	R operator()(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12) const{
		return fun_(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12);
	}
};

// メンバ関数を関数オブジェクト化する
template<class R, class C , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, R (C::*fun_)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12)>
struct static_functor<R (C::*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12), fun_>{
	typedef R result_type; typedef C* arg1_type; typedef A0 arg2_type; typedef A1 arg3_type; typedef A2 arg4_type; typedef A3 arg5_type; typedef A4 arg6_type; typedef A5 arg7_type; typedef A6 arg8_type; typedef A7 arg9_type; typedef A8 arg10_type; typedef A9 arg11_type; typedef A10 arg12_type; typedef A11 arg13_type; typedef A12 arg14_type; 
	enum{ arity = 14};

	R operator()(C* self , A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12) const{
		return (self->*fun_)(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12);
	}
};

// constメンバ関数を関数オブジェクト化する
template<class R, class C , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, R (C::*fun_)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12) const>
struct static_functor<R (C::*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12) const, fun_>{
	typedef R result_type; typedef C* arg1_type; typedef A0 arg2_type; typedef A1 arg3_type; typedef A2 arg4_type; typedef A3 arg5_type; typedef A4 arg6_type; typedef A5 arg7_type; typedef A6 arg8_type; typedef A7 arg9_type; typedef A8 arg10_type; typedef A9 arg11_type; typedef A10 arg12_type; typedef A11 arg13_type; typedef A12 arg14_type; 
	enum{ arity = 14};

	R operator()(C* self , A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12) const{
		return (self->*fun_)(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12);
	}
};

// コンストラクタを関数オブジェクト化する
template<class T , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12>
struct ctor_functor<T , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12>{
	typedef SmartPtr<T> result_type; typedef A0 arg1_type; typedef A1 arg2_type; typedef A2 arg3_type; typedef A3 arg4_type; typedef A4 arg5_type; typedef A5 arg6_type; typedef A6 arg7_type; typedef A7 arg8_type; typedef A8 arg9_type; typedef A9 arg10_type; typedef A10 arg11_type; typedef A11 arg12_type; typedef A12 arg13_type; 
	enum{ arity = 13};

	SmartPtr<T> operator()(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12) const{
		return xnew<T>(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12);
	}
};

// stdcall版
#if defined(_WIN32) && defined(_M_IX86)

template<int Method, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12>
struct nfun<R (__stdcall *)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12), Method>
	: nfun_base<13, functor<R (__stdcall *)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12)>, Method, R>{};

// 関数を関数オブジェクト化する
template<class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12>
struct functor<R (__stdcall *)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12)>{
	typedef R result_type; typedef A0 arg1_type; typedef A1 arg2_type; typedef A2 arg3_type; typedef A3 arg4_type; typedef A4 arg5_type; typedef A5 arg6_type; typedef A6 arg7_type; typedef A7 arg8_type; typedef A8 arg9_type; typedef A9 arg10_type; typedef A10 arg11_type; typedef A11 arg12_type; typedef A12 arg13_type; 
	typedef R (__stdcall *fun_type)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12);
	enum{ arity = 13};

	fun_type fun_;
	functor(fun_type fun):fun_(fun){}

	R operator()(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12) const{
		return fun_(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12);
	}
};

// 関数を関数オブジェクト化する
template<class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, R (__stdcall *fun_)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12)>
struct static_functor<R (__stdcall *)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12), fun_>{
	typedef R result_type; typedef A0 arg1_type; typedef A1 arg2_type; typedef A2 arg3_type; typedef A3 arg4_type; typedef A4 arg5_type; typedef A5 arg6_type; typedef A6 arg7_type; typedef A7 arg8_type; typedef A8 arg9_type; typedef A9 arg10_type; typedef A10 arg11_type; typedef A11 arg12_type; typedef A12 arg13_type; 
	enum{ arity = 13};

	R operator()(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12) const{
		return fun_(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12);
	}
};

#endif


//}}REPEAT}


///////////////////////////////////////////////////////

template<class C, class T>
struct getter_functor{
	typedef T& result_type; typedef C* arg1_type;
	enum{ arity = 1 };
	typedef T C::* fun_t;

	fun_t fun_;
	getter_functor(fun_t fun): fun_(fun){}

	T& operator()(C* a0){
		T& ret = a0->*fun_;
		return ret;
	}
};

template<class C, class T>
struct setter_functor{
	typedef T& result_type; typedef C* arg1_type; typedef const T& arg2_type;
	enum{ arity = 2 };
	typedef T C::* fun_t;

	fun_t fun_;
	setter_functor(fun_t fun): fun_(fun){}

	T& operator()(C* a0, const T& a1){
		T& ret = a0->*fun_;
		ret = a1;
		return ret;
	}
};

//////////////////////////////////////////

#define Xdef_method(fun) it->def_and_return(XTAL_L(#fun), XTAL_METHOD(&Self::fun))
#define Xdef_method2(fun, secondary_key) it->def_and_return(XTAL_L(#fun), secondary_key, XTAL_METHOD(&Self::fun))
#define Xdef_method_alias(fun, impl) it->def_and_return(XTAL_L(#fun), XTAL_METHOD(impl))
#define Xdef_method_alias2(fun, secondary_key, impl) it->def_and_return(XTAL_L(#fun), secondary_key, XTAL_METHOD(impl))

}

#endif
