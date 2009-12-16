/** \file src/xtal/xtal_cfun.h
* \brief src/xtal/xtal_cfun.h
*/

#ifndef XTAL_CFUN_H_INCLUDE_GUARD
#define XTAL_CFUN_H_INCLUDE_GUARD

#pragma once

namespace xtal{

class NativeMethod;	

struct ReturnResult{
	static void return_result(const VMachinePtr& vm){
		vm->return_result();
	}

	template<class T>
	static void return_result(const VMachinePtr& vm, const T& ret){
		return_result2(vm, ret, TypeIntType<Convertible<T, AnyPtr>::value>());
	}

	template<class T>
	static void return_result2(const VMachinePtr& vm, const T& ret, TypeIntType<1>){
		vm->return_result(ret);
	}
		
	template<class T>
	static void return_result2(const VMachinePtr& vm, const T& ret, TypeIntType<0>){
		vm->return_result(xnew<T>(ret));
	}
};

struct ReturnNone{
	static void return_result(const VMachinePtr& vm){}

	template<class T>
	static void return_result(const VMachinePtr& vm, const T& v){
		ReturnResult::return_result(vm, v);
	}
};

template<class T, int N>
struct ArgGetter{
	static typename CastResult<T>::type 
	cast(const VMachinePtr& vm, UninitializedAny* args){
		return unchecked_cast<T>((AnyPtr&)args[N+1]);
	}
};

template<class T>
struct ArgThisGetter{
	static typename CastResult<T>::type 
	cast(const VMachinePtr& vm, UninitializedAny* args){
		return unchecked_cast<T>((AnyPtr&)args[0]);
	}
};

struct ArgGetterVM{
	static const VMachinePtr& 
	cast(const VMachinePtr& vm, UninitializedAny* args){
		return vm;
	}
};

template<class Policy>
struct ReturnPolicyTest{
	const VMachinePtr& vm;

	ReturnPolicyTest(const VMachinePtr& vm)
		:vm(vm){}

private:
	void operator =(const ReturnPolicyTest&);
};

struct ReturnPolicyVoidTest{};

template<class T, class Policy>
inline int operator ,(const T& val, ReturnPolicyTest<Policy> rp){
	Policy::return_result(rp.vm, val);
	return 0;
}

template<class Policy>
inline int operator ,(ReturnPolicyTest<Policy> rp, ReturnPolicyVoidTest){
	Policy::return_result(rp.vm);
	return 0;
}

///////////////////////////////////////////////////////

template<int N, class Fun>
struct cfun{};

//{REPEAT{{
/*
template<class Fun>
struct cfun<`n`, Fun>{
	static void f(const VMachinePtr& vm, const void* data, UninitializedAny* args){
		#REPEAT#typedef typename Fun::ARG`i` A`i`;#
		Fun::call(data
			#COMMA_REPEAT#A`i`::cast(vm, args)#
		), ReturnPolicyTest<typename Fun::Result>(vm), ReturnPolicyVoidTest();
	}
};
*/

template<class Fun>
struct cfun<0, Fun>{
	static void f(const VMachinePtr& vm, const void* data, UninitializedAny* args){
		
		Fun::call(data
			
		), ReturnPolicyTest<typename Fun::Result>(vm), ReturnPolicyVoidTest();
	}
};

template<class Fun>
struct cfun<1, Fun>{
	static void f(const VMachinePtr& vm, const void* data, UninitializedAny* args){
		typedef typename Fun::ARG0 A0;
		Fun::call(data
			, A0::cast(vm, args)
		), ReturnPolicyTest<typename Fun::Result>(vm), ReturnPolicyVoidTest();
	}
};

template<class Fun>
struct cfun<2, Fun>{
	static void f(const VMachinePtr& vm, const void* data, UninitializedAny* args){
		typedef typename Fun::ARG0 A0;typedef typename Fun::ARG1 A1;
		Fun::call(data
			, A0::cast(vm, args), A1::cast(vm, args)
		), ReturnPolicyTest<typename Fun::Result>(vm), ReturnPolicyVoidTest();
	}
};

template<class Fun>
struct cfun<3, Fun>{
	static void f(const VMachinePtr& vm, const void* data, UninitializedAny* args){
		typedef typename Fun::ARG0 A0;typedef typename Fun::ARG1 A1;typedef typename Fun::ARG2 A2;
		Fun::call(data
			, A0::cast(vm, args), A1::cast(vm, args), A2::cast(vm, args)
		), ReturnPolicyTest<typename Fun::Result>(vm), ReturnPolicyVoidTest();
	}
};

template<class Fun>
struct cfun<4, Fun>{
	static void f(const VMachinePtr& vm, const void* data, UninitializedAny* args){
		typedef typename Fun::ARG0 A0;typedef typename Fun::ARG1 A1;typedef typename Fun::ARG2 A2;typedef typename Fun::ARG3 A3;
		Fun::call(data
			, A0::cast(vm, args), A1::cast(vm, args), A2::cast(vm, args), A3::cast(vm, args)
		), ReturnPolicyTest<typename Fun::Result>(vm), ReturnPolicyVoidTest();
	}
};

template<class Fun>
struct cfun<5, Fun>{
	static void f(const VMachinePtr& vm, const void* data, UninitializedAny* args){
		typedef typename Fun::ARG0 A0;typedef typename Fun::ARG1 A1;typedef typename Fun::ARG2 A2;typedef typename Fun::ARG3 A3;typedef typename Fun::ARG4 A4;
		Fun::call(data
			, A0::cast(vm, args), A1::cast(vm, args), A2::cast(vm, args), A3::cast(vm, args), A4::cast(vm, args)
		), ReturnPolicyTest<typename Fun::Result>(vm), ReturnPolicyVoidTest();
	}
};

template<class Fun>
struct cfun<6, Fun>{
	static void f(const VMachinePtr& vm, const void* data, UninitializedAny* args){
		typedef typename Fun::ARG0 A0;typedef typename Fun::ARG1 A1;typedef typename Fun::ARG2 A2;typedef typename Fun::ARG3 A3;typedef typename Fun::ARG4 A4;typedef typename Fun::ARG5 A5;
		Fun::call(data
			, A0::cast(vm, args), A1::cast(vm, args), A2::cast(vm, args), A3::cast(vm, args), A4::cast(vm, args), A5::cast(vm, args)
		), ReturnPolicyTest<typename Fun::Result>(vm), ReturnPolicyVoidTest();
	}
};

template<class Fun>
struct cfun<7, Fun>{
	static void f(const VMachinePtr& vm, const void* data, UninitializedAny* args){
		typedef typename Fun::ARG0 A0;typedef typename Fun::ARG1 A1;typedef typename Fun::ARG2 A2;typedef typename Fun::ARG3 A3;typedef typename Fun::ARG4 A4;typedef typename Fun::ARG5 A5;typedef typename Fun::ARG6 A6;
		Fun::call(data
			, A0::cast(vm, args), A1::cast(vm, args), A2::cast(vm, args), A3::cast(vm, args), A4::cast(vm, args), A5::cast(vm, args), A6::cast(vm, args)
		), ReturnPolicyTest<typename Fun::Result>(vm), ReturnPolicyVoidTest();
	}
};

template<class Fun>
struct cfun<8, Fun>{
	static void f(const VMachinePtr& vm, const void* data, UninitializedAny* args){
		typedef typename Fun::ARG0 A0;typedef typename Fun::ARG1 A1;typedef typename Fun::ARG2 A2;typedef typename Fun::ARG3 A3;typedef typename Fun::ARG4 A4;typedef typename Fun::ARG5 A5;typedef typename Fun::ARG6 A6;typedef typename Fun::ARG7 A7;
		Fun::call(data
			, A0::cast(vm, args), A1::cast(vm, args), A2::cast(vm, args), A3::cast(vm, args), A4::cast(vm, args), A5::cast(vm, args), A6::cast(vm, args), A7::cast(vm, args)
		), ReturnPolicyTest<typename Fun::Result>(vm), ReturnPolicyVoidTest();
	}
};

template<class Fun>
struct cfun<9, Fun>{
	static void f(const VMachinePtr& vm, const void* data, UninitializedAny* args){
		typedef typename Fun::ARG0 A0;typedef typename Fun::ARG1 A1;typedef typename Fun::ARG2 A2;typedef typename Fun::ARG3 A3;typedef typename Fun::ARG4 A4;typedef typename Fun::ARG5 A5;typedef typename Fun::ARG6 A6;typedef typename Fun::ARG7 A7;typedef typename Fun::ARG8 A8;
		Fun::call(data
			, A0::cast(vm, args), A1::cast(vm, args), A2::cast(vm, args), A3::cast(vm, args), A4::cast(vm, args), A5::cast(vm, args), A6::cast(vm, args), A7::cast(vm, args), A8::cast(vm, args)
		), ReturnPolicyTest<typename Fun::Result>(vm), ReturnPolicyVoidTest();
	}
};

template<class Fun>
struct cfun<10, Fun>{
	static void f(const VMachinePtr& vm, const void* data, UninitializedAny* args){
		typedef typename Fun::ARG0 A0;typedef typename Fun::ARG1 A1;typedef typename Fun::ARG2 A2;typedef typename Fun::ARG3 A3;typedef typename Fun::ARG4 A4;typedef typename Fun::ARG5 A5;typedef typename Fun::ARG6 A6;typedef typename Fun::ARG7 A7;typedef typename Fun::ARG8 A8;typedef typename Fun::ARG9 A9;
		Fun::call(data
			, A0::cast(vm, args), A1::cast(vm, args), A2::cast(vm, args), A3::cast(vm, args), A4::cast(vm, args), A5::cast(vm, args), A6::cast(vm, args), A7::cast(vm, args), A8::cast(vm, args), A9::cast(vm, args)
		), ReturnPolicyTest<typename Fun::Result>(vm), ReturnPolicyVoidTest();
	}
};

template<class Fun>
struct cfun<11, Fun>{
	static void f(const VMachinePtr& vm, const void* data, UninitializedAny* args){
		typedef typename Fun::ARG0 A0;typedef typename Fun::ARG1 A1;typedef typename Fun::ARG2 A2;typedef typename Fun::ARG3 A3;typedef typename Fun::ARG4 A4;typedef typename Fun::ARG5 A5;typedef typename Fun::ARG6 A6;typedef typename Fun::ARG7 A7;typedef typename Fun::ARG8 A8;typedef typename Fun::ARG9 A9;typedef typename Fun::ARG10 A10;
		Fun::call(data
			, A0::cast(vm, args), A1::cast(vm, args), A2::cast(vm, args), A3::cast(vm, args), A4::cast(vm, args), A5::cast(vm, args), A6::cast(vm, args), A7::cast(vm, args), A8::cast(vm, args), A9::cast(vm, args), A10::cast(vm, args)
		), ReturnPolicyTest<typename Fun::Result>(vm), ReturnPolicyVoidTest();
	}
};

template<class Fun>
struct cfun<12, Fun>{
	static void f(const VMachinePtr& vm, const void* data, UninitializedAny* args){
		typedef typename Fun::ARG0 A0;typedef typename Fun::ARG1 A1;typedef typename Fun::ARG2 A2;typedef typename Fun::ARG3 A3;typedef typename Fun::ARG4 A4;typedef typename Fun::ARG5 A5;typedef typename Fun::ARG6 A6;typedef typename Fun::ARG7 A7;typedef typename Fun::ARG8 A8;typedef typename Fun::ARG9 A9;typedef typename Fun::ARG10 A10;typedef typename Fun::ARG11 A11;
		Fun::call(data
			, A0::cast(vm, args), A1::cast(vm, args), A2::cast(vm, args), A3::cast(vm, args), A4::cast(vm, args), A5::cast(vm, args), A6::cast(vm, args), A7::cast(vm, args), A8::cast(vm, args), A9::cast(vm, args), A10::cast(vm, args), A11::cast(vm, args)
		), ReturnPolicyTest<typename Fun::Result>(vm), ReturnPolicyVoidTest();
	}
};

template<class Fun>
struct cfun<13, Fun>{
	static void f(const VMachinePtr& vm, const void* data, UninitializedAny* args){
		typedef typename Fun::ARG0 A0;typedef typename Fun::ARG1 A1;typedef typename Fun::ARG2 A2;typedef typename Fun::ARG3 A3;typedef typename Fun::ARG4 A4;typedef typename Fun::ARG5 A5;typedef typename Fun::ARG6 A6;typedef typename Fun::ARG7 A7;typedef typename Fun::ARG8 A8;typedef typename Fun::ARG9 A9;typedef typename Fun::ARG10 A10;typedef typename Fun::ARG11 A11;typedef typename Fun::ARG12 A12;
		Fun::call(data
			, A0::cast(vm, args), A1::cast(vm, args), A2::cast(vm, args), A3::cast(vm, args), A4::cast(vm, args), A5::cast(vm, args), A6::cast(vm, args), A7::cast(vm, args), A8::cast(vm, args), A9::cast(vm, args), A10::cast(vm, args), A11::cast(vm, args), A12::cast(vm, args)
		), ReturnPolicyTest<typename Fun::Result>(vm), ReturnPolicyVoidTest();
	}
};

//}}REPEAT}

///////////////////////////////////////////////////////

template<class Fun>
struct cfun_holder{
	enum{ PARAMS = 0, PARAMS2 = 0 };
};

template<class Fun>
struct cmemfun_holder{
	enum{ PARAMS = 0, PARAMS2 = 0 };
};

template<class Fun, Fun fun>
struct cmemfun_holder_static : cmemfun_holder<Fun>{
	enum{ PARAMS = 0, PARAMS2 = 0 };
};

template<class T, 
class A0=void, class A1=void, class A2=void, class A3=void, class A4=void, 
class A5=void, class A6=void, class A7=void, class A8=void, class A9=void,
class A10=void, class A11=void, class A12=void, class A13=void, class A14=void>
struct ctor_fun{
	enum{ PARAMS = 0, PARAMS2 = 0 };
};

struct param_types_holder_n{
	void (*fun)(const VMachinePtr& vm, const void* data, UninitializedAny* args); // 関数
	CppClassSymbolData*** param_types; // thisと引数の型を表すクラスシンボルへのポインタ
	u8 size;
	u8 param_n; // 引数の数
	u8 extendable; // 可変長かどうか
};

template<class Func>
struct fun_param_holder{
	enum{
		PARAMS = Func::PARAMS,
		PARAM_N = Func::PARAM_N,
		EXTENDABLE = Func::EXTENDABLE,
	};

	typedef typename Func::fun_param_holder fph;
	typedef typename Func::fun_t fun_t;
	typedef cfun<PARAMS, Func> cfun_t;

	static param_types_holder_n value;
};

template<class Fun>
param_types_holder_n fun_param_holder<Fun>::value = {
	&cfun_t::f,
	fph::values,
	sizeof(fun_t),
	PARAM_N,
	EXTENDABLE,
};

//{REPEAT{{
/*

template<class C #COMMA_REPEAT#class A`i`#>
struct param_types_holder`n`{
	static CppClassSymbolData** values[`n`+1];
};

template<class C #COMMA_REPEAT#class A`i`#>
CppClassSymbolData** param_types_holder`n`<C #COMMA_REPEAT#A`i`#>::values[`n`+1] = {
	&CppClassSymbol<C>::value,
	#REPEAT#&CppClassSymbol<A`i`>::value,#
};
*/


template<class C >
struct param_types_holder0{
	static CppClassSymbolData** values[0+1];
};

template<class C >
CppClassSymbolData** param_types_holder0<C >::values[0+1] = {
	&CppClassSymbol<C>::value,
	
};


template<class C , class A0>
struct param_types_holder1{
	static CppClassSymbolData** values[1+1];
};

template<class C , class A0>
CppClassSymbolData** param_types_holder1<C , A0>::values[1+1] = {
	&CppClassSymbol<C>::value,
	&CppClassSymbol<A0>::value,
};


template<class C , class A0, class A1>
struct param_types_holder2{
	static CppClassSymbolData** values[2+1];
};

template<class C , class A0, class A1>
CppClassSymbolData** param_types_holder2<C , A0, A1>::values[2+1] = {
	&CppClassSymbol<C>::value,
	&CppClassSymbol<A0>::value,&CppClassSymbol<A1>::value,
};


template<class C , class A0, class A1, class A2>
struct param_types_holder3{
	static CppClassSymbolData** values[3+1];
};

template<class C , class A0, class A1, class A2>
CppClassSymbolData** param_types_holder3<C , A0, A1, A2>::values[3+1] = {
	&CppClassSymbol<C>::value,
	&CppClassSymbol<A0>::value,&CppClassSymbol<A1>::value,&CppClassSymbol<A2>::value,
};


template<class C , class A0, class A1, class A2, class A3>
struct param_types_holder4{
	static CppClassSymbolData** values[4+1];
};

template<class C , class A0, class A1, class A2, class A3>
CppClassSymbolData** param_types_holder4<C , A0, A1, A2, A3>::values[4+1] = {
	&CppClassSymbol<C>::value,
	&CppClassSymbol<A0>::value,&CppClassSymbol<A1>::value,&CppClassSymbol<A2>::value,&CppClassSymbol<A3>::value,
};


template<class C , class A0, class A1, class A2, class A3, class A4>
struct param_types_holder5{
	static CppClassSymbolData** values[5+1];
};

template<class C , class A0, class A1, class A2, class A3, class A4>
CppClassSymbolData** param_types_holder5<C , A0, A1, A2, A3, A4>::values[5+1] = {
	&CppClassSymbol<C>::value,
	&CppClassSymbol<A0>::value,&CppClassSymbol<A1>::value,&CppClassSymbol<A2>::value,&CppClassSymbol<A3>::value,&CppClassSymbol<A4>::value,
};


template<class C , class A0, class A1, class A2, class A3, class A4, class A5>
struct param_types_holder6{
	static CppClassSymbolData** values[6+1];
};

template<class C , class A0, class A1, class A2, class A3, class A4, class A5>
CppClassSymbolData** param_types_holder6<C , A0, A1, A2, A3, A4, A5>::values[6+1] = {
	&CppClassSymbol<C>::value,
	&CppClassSymbol<A0>::value,&CppClassSymbol<A1>::value,&CppClassSymbol<A2>::value,&CppClassSymbol<A3>::value,&CppClassSymbol<A4>::value,&CppClassSymbol<A5>::value,
};


template<class C , class A0, class A1, class A2, class A3, class A4, class A5, class A6>
struct param_types_holder7{
	static CppClassSymbolData** values[7+1];
};

template<class C , class A0, class A1, class A2, class A3, class A4, class A5, class A6>
CppClassSymbolData** param_types_holder7<C , A0, A1, A2, A3, A4, A5, A6>::values[7+1] = {
	&CppClassSymbol<C>::value,
	&CppClassSymbol<A0>::value,&CppClassSymbol<A1>::value,&CppClassSymbol<A2>::value,&CppClassSymbol<A3>::value,&CppClassSymbol<A4>::value,&CppClassSymbol<A5>::value,&CppClassSymbol<A6>::value,
};


template<class C , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7>
struct param_types_holder8{
	static CppClassSymbolData** values[8+1];
};

template<class C , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7>
CppClassSymbolData** param_types_holder8<C , A0, A1, A2, A3, A4, A5, A6, A7>::values[8+1] = {
	&CppClassSymbol<C>::value,
	&CppClassSymbol<A0>::value,&CppClassSymbol<A1>::value,&CppClassSymbol<A2>::value,&CppClassSymbol<A3>::value,&CppClassSymbol<A4>::value,&CppClassSymbol<A5>::value,&CppClassSymbol<A6>::value,&CppClassSymbol<A7>::value,
};


template<class C , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8>
struct param_types_holder9{
	static CppClassSymbolData** values[9+1];
};

template<class C , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8>
CppClassSymbolData** param_types_holder9<C , A0, A1, A2, A3, A4, A5, A6, A7, A8>::values[9+1] = {
	&CppClassSymbol<C>::value,
	&CppClassSymbol<A0>::value,&CppClassSymbol<A1>::value,&CppClassSymbol<A2>::value,&CppClassSymbol<A3>::value,&CppClassSymbol<A4>::value,&CppClassSymbol<A5>::value,&CppClassSymbol<A6>::value,&CppClassSymbol<A7>::value,&CppClassSymbol<A8>::value,
};


template<class C , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9>
struct param_types_holder10{
	static CppClassSymbolData** values[10+1];
};

template<class C , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9>
CppClassSymbolData** param_types_holder10<C , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9>::values[10+1] = {
	&CppClassSymbol<C>::value,
	&CppClassSymbol<A0>::value,&CppClassSymbol<A1>::value,&CppClassSymbol<A2>::value,&CppClassSymbol<A3>::value,&CppClassSymbol<A4>::value,&CppClassSymbol<A5>::value,&CppClassSymbol<A6>::value,&CppClassSymbol<A7>::value,&CppClassSymbol<A8>::value,&CppClassSymbol<A9>::value,
};


template<class C , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10>
struct param_types_holder11{
	static CppClassSymbolData** values[11+1];
};

template<class C , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10>
CppClassSymbolData** param_types_holder11<C , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10>::values[11+1] = {
	&CppClassSymbol<C>::value,
	&CppClassSymbol<A0>::value,&CppClassSymbol<A1>::value,&CppClassSymbol<A2>::value,&CppClassSymbol<A3>::value,&CppClassSymbol<A4>::value,&CppClassSymbol<A5>::value,&CppClassSymbol<A6>::value,&CppClassSymbol<A7>::value,&CppClassSymbol<A8>::value,&CppClassSymbol<A9>::value,&CppClassSymbol<A10>::value,
};


template<class C , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11>
struct param_types_holder12{
	static CppClassSymbolData** values[12+1];
};

template<class C , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11>
CppClassSymbolData** param_types_holder12<C , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11>::values[12+1] = {
	&CppClassSymbol<C>::value,
	&CppClassSymbol<A0>::value,&CppClassSymbol<A1>::value,&CppClassSymbol<A2>::value,&CppClassSymbol<A3>::value,&CppClassSymbol<A4>::value,&CppClassSymbol<A5>::value,&CppClassSymbol<A6>::value,&CppClassSymbol<A7>::value,&CppClassSymbol<A8>::value,&CppClassSymbol<A9>::value,&CppClassSymbol<A10>::value,&CppClassSymbol<A11>::value,
};


template<class C , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12>
struct param_types_holder13{
	static CppClassSymbolData** values[13+1];
};

template<class C , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12>
CppClassSymbolData** param_types_holder13<C , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12>::values[13+1] = {
	&CppClassSymbol<C>::value,
	&CppClassSymbol<A0>::value,&CppClassSymbol<A1>::value,&CppClassSymbol<A2>::value,&CppClassSymbol<A3>::value,&CppClassSymbol<A4>::value,&CppClassSymbol<A5>::value,&CppClassSymbol<A6>::value,&CppClassSymbol<A7>::value,&CppClassSymbol<A8>::value,&CppClassSymbol<A9>::value,&CppClassSymbol<A10>::value,&CppClassSymbol<A11>::value,&CppClassSymbol<A12>::value,
};

//}}REPEAT}

///////////////////////////////////////////////////////

template<class C, class T>
struct getter_holder{
	enum{ PARAMS = 1, PARAM_N = 0, METHOD = 1, EXTENDABLE = 0 };
	typedef ReturnResult Result;
	typedef ArgThisGetter<C*> ARG0;
	typedef param_types_holder0<C> fun_param_holder;
	typedef T C::* fun_t;
	//static const T* call(const void* fun, C* self){ return &(self->**(fun_t*)fun); }
	static const T& call(const void* fun, C* self){ return (self->**(fun_t*)fun); }
};

template<class C, class T>
struct setter_holder{
	enum{ PARAMS = 2, PARAM_N = 1, METHOD = 1, EXTENDABLE = 0 };
	typedef ReturnResult Result;
	typedef ArgThisGetter<C*> ARG0;
	typedef ArgGetter<typename CastResult<T>::type, 0> ARG1;
	typedef param_types_holder1<C, T> fun_param_holder;
	typedef T C::* fun_t;
	//static const T* call(const void* fun, C* self, const T& v){ self->**(fun_t*)fun = v; return &(self->**(fun_t*)fun); }
	static const T& call(const void* fun, C* self, const T& v){ self->**(fun_t*)fun = v; return (self->**(fun_t*)fun); }
};

//////////////////////////////////////////////////////////////

class NativeMethod : public HaveParentRefCountingBase{
public:
	enum{ TYPE = TYPE_NATIVE_METHOD };

	typedef void (*fun_t)(const VMachinePtr& vm, const void* data, UninitializedAny* args);

	NativeMethod(const param_types_holder_n& pth, const void* val = 0);
	
	~NativeMethod();

public:

	const NativeFunPtr& param(int_t i, const IDPtr& key, const AnyPtr& value);

public:

	void visit_members(Visitor& m);

	void rawcall(const VMachinePtr& vm);

protected:
	fun_t fun_;
	void* data_;

	u8 param_n_;
	u8 min_param_count_;
	u8 max_param_count_;
	u8 val_size_;
};

class NativeFun : public NativeMethod{
public:
	enum{ TYPE = TYPE_NATIVE_FUN };

	NativeFun(const param_types_holder_n& pth, const void* val, const AnyPtr& this_);

public:

	void visit_members(Visitor& m);

	void rawcall(const VMachinePtr& vm);

private:
	AnyPtr this_;
};

NativeFunPtr new_native_fun(const param_types_holder_n& pth, const void* val);
NativeFunPtr new_native_fun(const param_types_holder_n& pth, const void* val, const AnyPtr& this_);

//////////////////////////////////////////////////////////////

/*
* \brief C++のコンストラクタをXtalから呼び出せるオブジェクトに変換するための擬似関数
*
*/
template<class T, 
class A0=void, class A1=void, class A2=void, class A3=void, class A4=void, 
class A5=void, class A6=void, class A7=void, class A8=void, class A9=void,
class A10=void, class A11=void, class A12=void, class A13=void, class A14=void>
struct ctor : public NativeFunPtr{
	typedef ctor_fun<T, A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14> fun_t;
	ctor(){
		NativeFunPtr::operator =(new_native_fun(fun_param_holder<fun_t>::value, 0));
	}
};
	
/**
* @brief 2重ディスパッチメソッド
*/
class DoubleDispatchMethod : public HaveParentBase{
public:

	DoubleDispatchMethod(const IDPtr& primary_key)
		:primary_key_(primary_key){}

	virtual void rawcall(const VMachinePtr& vm){
		if(vm->ordered_arg_count()>0){
			vm->arg_this()->rawsend(vm, primary_key_, vm->arg(0)->get_class());
		}
	}

private:
	IDPtr primary_key_;
};

/**
* @brief 2重ディスパッチ関数
*/
class DoubleDispatchFun : public HaveParentBase{
public:

	DoubleDispatchFun(const ClassPtr& klass, const IDPtr& primary_key)
		:klass_(klass), primary_key_(primary_key){}

	virtual void rawcall(const VMachinePtr& vm){
		klass_->member(primary_key_, vm->arg(0)->get_class())->rawcall(vm);
	}

private:

	virtual void visit_members(Visitor& m){
		HaveParentBase::visit_members(m);
		m & klass_;
	}

	AnyPtr klass_;
	IDPtr primary_key_;
};

/// \name ネイティブ関数をXtalで呼び出せるオブジェクトに変換するための関数群
//@{

/**
* \brief C++の関数をXtalから呼び出せるオブジェクトに変換するための関数
*
*/
template<class Fun>
inline NativeFunPtr fun(const Fun& f){
	return new_native_fun(fun_param_holder<cfun_holder<Fun> >::value, &f);
}

/**
* \brief C++のメンバ関数をXtalから呼び出せるオブジェクトに変換するための関数
*
* 普通の関数をメソッドとして変換したい場合、第一引数をその型にすること。
*/
template<class Fun>
inline NativeFunPtr method(const Fun& f){
	return new_native_fun(fun_param_holder<cmemfun_holder<Fun> >::value, &f);
}

/**
* \brief メンバ変数へのポインタからゲッター関数を生成する
*
*/
template<class T, class C>
inline NativeFunPtr getter(T C::* f){
	return new_native_fun(fun_param_holder<getter_holder<C, T> >::value, &f);
}
	
/**
* \brief メンバ変数へのポインタからセッター関数を生成する
*
*/
template<class T, class C>
inline NativeFunPtr setter(T C::* f){
	return new_native_fun(fun_param_holder<setter_holder<C, T> >::value(), &f);
}

/**
* \brief 2重ディスパッチメソッドオブジェクトを生成する
*
*/
DoubleDispatchMethodPtr double_dispatch_method(const IDPtr& primary_key);

/**
* \brief 2重ディスパッチメソッドオブジェクトを生成する
*
*/
DoubleDispatchFunPtr double_dispatch_fun(const ClassPtr& klass, const IDPtr& primary_key);

//@}

}

#endif // XTAL_CFUN_H_INCLUDE_GUARD
