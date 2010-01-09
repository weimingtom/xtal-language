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
	enum{ PARAM_N = `n`, METHOD = 0, EXTENDABLE = 0 };
	typedef param_types_holder`n`<void #COMMA_REPEAT#A`i`#> fun_param_holder;
};

template<class R #COMMA_REPEAT#class A`i`#>
struct cfun_holder<R (*)(#REPEAT_COMMA#A`i`#)> : public cfun_holder_base`n`<R #COMMA_REPEAT#A`i`#>{
	typedef R (*fun_t)(#REPEAT_COMMA#A`i`#);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		(*(fun_t*)data)(
			#REPEAT_COMMA#unchecked_cast<A`i`>((AnyPtr&)args[`i+1`])#
		), ReturnPolicyTest<ReturnResult>(vm), ReturnPolicyVoidTest();
	}
};

#if defined(_WIN32) && defined(_M_IX86)
template<class R #COMMA_REPEAT#class A`i`#>
struct cfun_holder<R (__stdcall *)(#REPEAT_COMMA#A`i`#)> : public cfun_holder_base`n`<R #COMMA_REPEAT#A`i`#>{
	typedef R (__stdcall *fun_t)(#REPEAT_COMMA#A`i`#);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		(*(fun_t*)data)(
			#REPEAT_COMMA#unchecked_cast<A`i`>((AnyPtr&)args[`i+1`])#
		), ReturnPolicyTest<ReturnResult>(vm), ReturnPolicyVoidTest();
	}
};

#endif

template<class C, class R #COMMA_REPEAT#class A`i`#>
struct cmemfun_holder_base`n`{
	enum{ PARAM_N = `n`, METHOD = 1, EXTENDABLE = 0 };
	typedef param_types_holder`n`<C #COMMA_REPEAT#A`i`#> fun_param_holder;
};

template<class C, class R #COMMA_REPEAT#class A`i`#>
struct cmemfun_holder<R (C::*)(#REPEAT_COMMA#A`i`#)> : public cmemfun_holder_base`n`<C*, R #COMMA_REPEAT#A`i`#>{
	typedef R (C::*fun_t)(#REPEAT_COMMA#A`i`#);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		(unchecked_cast<C*>((AnyPtr&)args[0])->**(fun_t*)data)(
			#REPEAT_COMMA#unchecked_cast<A`i`>((AnyPtr&)args[`i+1`])#
		), ReturnPolicyTest<ReturnResult>(vm), ReturnPolicyVoidTest();
	}
};

template<class C, class R #COMMA_REPEAT#class A`i`#>
struct cmemfun_holder<R (C::*)(#REPEAT_COMMA#A`i`#) const> : public cmemfun_holder_base`n`<C*, R #COMMA_REPEAT#A`i`#>{
	typedef R (C::*fun_t)(#REPEAT_COMMA#A`i`#) const;

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		(unchecked_cast<C*>((AnyPtr&)args[0])->**(fun_t*)data)(
			#REPEAT_COMMA#unchecked_cast<A`i`>((AnyPtr&)args[`i+1`])#
		), ReturnPolicyTest<ReturnResult>(vm), ReturnPolicyVoidTest();
	}
};

template<class C, class R #COMMA_REPEAT#class A`i`#>
struct cmemfun_holder<R (*)(C #COMMA_REPEAT#A`i`#)> : public cmemfun_holder_base`n`<C, R #COMMA_REPEAT#A`i`#>{
	typedef R (*fun_t)(C #COMMA_REPEAT#A`i`#);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		(*(fun_t*)data)(
			unchecked_cast<C>((AnyPtr&)args[0])
			#COMMA_REPEAT#unchecked_cast<A`i`>((AnyPtr&)args[`i+1`])#
		), ReturnPolicyTest<ReturnResult>(vm), ReturnPolicyVoidTest();
	}
};

#if defined(_WIN32) && defined(_M_IX86)
template<class C, class R #COMMA_REPEAT#class A`i`#>
struct cmemfun_holder<R (__stdcall *)(C #COMMA_REPEAT#A`i`#)> : public cmemfun_holder_base`n`<C, R #COMMA_REPEAT#A`i`#>{
	typedef R (__stdcall *fun_t)(C #COMMA_REPEAT#A`i`#);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		(*(fun_t*)data)(
			unchecked_cast<C>((AnyPtr&)args[0])
			#COMMA_REPEAT#unchecked_cast<A`i`>((AnyPtr&)args[`i+1`])#
		), ReturnPolicyTest<ReturnResult>(vm), ReturnPolicyVoidTest();
	}
};
#endif

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

*/


template<class R >
struct cfun_holder_base0{
	enum{ PARAM_N = 0, METHOD = 0, EXTENDABLE = 0 };
	typedef param_types_holder0<void > fun_param_holder;
};

template<class R >
struct cfun_holder<R (*)()> : public cfun_holder_base0<R >{
	typedef R (*fun_t)();

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		(*(fun_t*)data)(
			
		), ReturnPolicyTest<ReturnResult>(vm), ReturnPolicyVoidTest();
	}
};

#if defined(_WIN32) && defined(_M_IX86)
template<class R >
struct cfun_holder<R (__stdcall *)()> : public cfun_holder_base0<R >{
	typedef R (__stdcall *fun_t)();

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		(*(fun_t*)data)(
			
		), ReturnPolicyTest<ReturnResult>(vm), ReturnPolicyVoidTest();
	}
};

#endif

template<class C, class R >
struct cmemfun_holder_base0{
	enum{ PARAM_N = 0, METHOD = 1, EXTENDABLE = 0 };
	typedef param_types_holder0<C > fun_param_holder;
};

template<class C, class R >
struct cmemfun_holder<R (C::*)()> : public cmemfun_holder_base0<C*, R >{
	typedef R (C::*fun_t)();

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		(unchecked_cast<C*>((AnyPtr&)args[0])->**(fun_t*)data)(
			
		), ReturnPolicyTest<ReturnResult>(vm), ReturnPolicyVoidTest();
	}
};

template<class C, class R >
struct cmemfun_holder<R (C::*)() const> : public cmemfun_holder_base0<C*, R >{
	typedef R (C::*fun_t)() const;

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		(unchecked_cast<C*>((AnyPtr&)args[0])->**(fun_t*)data)(
			
		), ReturnPolicyTest<ReturnResult>(vm), ReturnPolicyVoidTest();
	}
};

template<class C, class R >
struct cmemfun_holder<R (*)(C )> : public cmemfun_holder_base0<C, R >{
	typedef R (*fun_t)(C );

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		(*(fun_t*)data)(
			unchecked_cast<C>((AnyPtr&)args[0])
			
		), ReturnPolicyTest<ReturnResult>(vm), ReturnPolicyVoidTest();
	}
};

#if defined(_WIN32) && defined(_M_IX86)
template<class C, class R >
struct cmemfun_holder<R (__stdcall *)(C )> : public cmemfun_holder_base0<C, R >{
	typedef R (__stdcall *fun_t)(C );

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		(*(fun_t*)data)(
			unchecked_cast<C>((AnyPtr&)args[0])
			
		), ReturnPolicyTest<ReturnResult>(vm), ReturnPolicyVoidTest();
	}
};
#endif

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



template<class R , class A0>
struct cfun_holder_base1{
	enum{ PARAM_N = 1, METHOD = 0, EXTENDABLE = 0 };
	typedef param_types_holder1<void , A0> fun_param_holder;
};

template<class R , class A0>
struct cfun_holder<R (*)(A0)> : public cfun_holder_base1<R , A0>{
	typedef R (*fun_t)(A0);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		(*(fun_t*)data)(
			unchecked_cast<A0>((AnyPtr&)args[1])
		), ReturnPolicyTest<ReturnResult>(vm), ReturnPolicyVoidTest();
	}
};

#if defined(_WIN32) && defined(_M_IX86)
template<class R , class A0>
struct cfun_holder<R (__stdcall *)(A0)> : public cfun_holder_base1<R , A0>{
	typedef R (__stdcall *fun_t)(A0);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		(*(fun_t*)data)(
			unchecked_cast<A0>((AnyPtr&)args[1])
		), ReturnPolicyTest<ReturnResult>(vm), ReturnPolicyVoidTest();
	}
};

#endif

template<class C, class R , class A0>
struct cmemfun_holder_base1{
	enum{ PARAM_N = 1, METHOD = 1, EXTENDABLE = 0 };
	typedef param_types_holder1<C , A0> fun_param_holder;
};

template<class C, class R , class A0>
struct cmemfun_holder<R (C::*)(A0)> : public cmemfun_holder_base1<C*, R , A0>{
	typedef R (C::*fun_t)(A0);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		(unchecked_cast<C*>((AnyPtr&)args[0])->**(fun_t*)data)(
			unchecked_cast<A0>((AnyPtr&)args[1])
		), ReturnPolicyTest<ReturnResult>(vm), ReturnPolicyVoidTest();
	}
};

template<class C, class R , class A0>
struct cmemfun_holder<R (C::*)(A0) const> : public cmemfun_holder_base1<C*, R , A0>{
	typedef R (C::*fun_t)(A0) const;

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		(unchecked_cast<C*>((AnyPtr&)args[0])->**(fun_t*)data)(
			unchecked_cast<A0>((AnyPtr&)args[1])
		), ReturnPolicyTest<ReturnResult>(vm), ReturnPolicyVoidTest();
	}
};

template<class C, class R , class A0>
struct cmemfun_holder<R (*)(C , A0)> : public cmemfun_holder_base1<C, R , A0>{
	typedef R (*fun_t)(C , A0);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		(*(fun_t*)data)(
			unchecked_cast<C>((AnyPtr&)args[0])
			, unchecked_cast<A0>((AnyPtr&)args[1])
		), ReturnPolicyTest<ReturnResult>(vm), ReturnPolicyVoidTest();
	}
};

#if defined(_WIN32) && defined(_M_IX86)
template<class C, class R , class A0>
struct cmemfun_holder<R (__stdcall *)(C , A0)> : public cmemfun_holder_base1<C, R , A0>{
	typedef R (__stdcall *fun_t)(C , A0);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		(*(fun_t*)data)(
			unchecked_cast<C>((AnyPtr&)args[0])
			, unchecked_cast<A0>((AnyPtr&)args[1])
		), ReturnPolicyTest<ReturnResult>(vm), ReturnPolicyVoidTest();
	}
};
#endif

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



template<class R , class A0, class A1>
struct cfun_holder_base2{
	enum{ PARAM_N = 2, METHOD = 0, EXTENDABLE = 0 };
	typedef param_types_holder2<void , A0, A1> fun_param_holder;
};

template<class R , class A0, class A1>
struct cfun_holder<R (*)(A0, A1)> : public cfun_holder_base2<R , A0, A1>{
	typedef R (*fun_t)(A0, A1);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		(*(fun_t*)data)(
			unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2])
		), ReturnPolicyTest<ReturnResult>(vm), ReturnPolicyVoidTest();
	}
};

#if defined(_WIN32) && defined(_M_IX86)
template<class R , class A0, class A1>
struct cfun_holder<R (__stdcall *)(A0, A1)> : public cfun_holder_base2<R , A0, A1>{
	typedef R (__stdcall *fun_t)(A0, A1);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		(*(fun_t*)data)(
			unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2])
		), ReturnPolicyTest<ReturnResult>(vm), ReturnPolicyVoidTest();
	}
};

#endif

template<class C, class R , class A0, class A1>
struct cmemfun_holder_base2{
	enum{ PARAM_N = 2, METHOD = 1, EXTENDABLE = 0 };
	typedef param_types_holder2<C , A0, A1> fun_param_holder;
};

template<class C, class R , class A0, class A1>
struct cmemfun_holder<R (C::*)(A0, A1)> : public cmemfun_holder_base2<C*, R , A0, A1>{
	typedef R (C::*fun_t)(A0, A1);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		(unchecked_cast<C*>((AnyPtr&)args[0])->**(fun_t*)data)(
			unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2])
		), ReturnPolicyTest<ReturnResult>(vm), ReturnPolicyVoidTest();
	}
};

template<class C, class R , class A0, class A1>
struct cmemfun_holder<R (C::*)(A0, A1) const> : public cmemfun_holder_base2<C*, R , A0, A1>{
	typedef R (C::*fun_t)(A0, A1) const;

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		(unchecked_cast<C*>((AnyPtr&)args[0])->**(fun_t*)data)(
			unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2])
		), ReturnPolicyTest<ReturnResult>(vm), ReturnPolicyVoidTest();
	}
};

template<class C, class R , class A0, class A1>
struct cmemfun_holder<R (*)(C , A0, A1)> : public cmemfun_holder_base2<C, R , A0, A1>{
	typedef R (*fun_t)(C , A0, A1);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		(*(fun_t*)data)(
			unchecked_cast<C>((AnyPtr&)args[0])
			, unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2])
		), ReturnPolicyTest<ReturnResult>(vm), ReturnPolicyVoidTest();
	}
};

#if defined(_WIN32) && defined(_M_IX86)
template<class C, class R , class A0, class A1>
struct cmemfun_holder<R (__stdcall *)(C , A0, A1)> : public cmemfun_holder_base2<C, R , A0, A1>{
	typedef R (__stdcall *fun_t)(C , A0, A1);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		(*(fun_t*)data)(
			unchecked_cast<C>((AnyPtr&)args[0])
			, unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2])
		), ReturnPolicyTest<ReturnResult>(vm), ReturnPolicyVoidTest();
	}
};
#endif

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



template<class R , class A0, class A1, class A2>
struct cfun_holder_base3{
	enum{ PARAM_N = 3, METHOD = 0, EXTENDABLE = 0 };
	typedef param_types_holder3<void , A0, A1, A2> fun_param_holder;
};

template<class R , class A0, class A1, class A2>
struct cfun_holder<R (*)(A0, A1, A2)> : public cfun_holder_base3<R , A0, A1, A2>{
	typedef R (*fun_t)(A0, A1, A2);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		(*(fun_t*)data)(
			unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3])
		), ReturnPolicyTest<ReturnResult>(vm), ReturnPolicyVoidTest();
	}
};

#if defined(_WIN32) && defined(_M_IX86)
template<class R , class A0, class A1, class A2>
struct cfun_holder<R (__stdcall *)(A0, A1, A2)> : public cfun_holder_base3<R , A0, A1, A2>{
	typedef R (__stdcall *fun_t)(A0, A1, A2);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		(*(fun_t*)data)(
			unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3])
		), ReturnPolicyTest<ReturnResult>(vm), ReturnPolicyVoidTest();
	}
};

#endif

template<class C, class R , class A0, class A1, class A2>
struct cmemfun_holder_base3{
	enum{ PARAM_N = 3, METHOD = 1, EXTENDABLE = 0 };
	typedef param_types_holder3<C , A0, A1, A2> fun_param_holder;
};

template<class C, class R , class A0, class A1, class A2>
struct cmemfun_holder<R (C::*)(A0, A1, A2)> : public cmemfun_holder_base3<C*, R , A0, A1, A2>{
	typedef R (C::*fun_t)(A0, A1, A2);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		(unchecked_cast<C*>((AnyPtr&)args[0])->**(fun_t*)data)(
			unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3])
		), ReturnPolicyTest<ReturnResult>(vm), ReturnPolicyVoidTest();
	}
};

template<class C, class R , class A0, class A1, class A2>
struct cmemfun_holder<R (C::*)(A0, A1, A2) const> : public cmemfun_holder_base3<C*, R , A0, A1, A2>{
	typedef R (C::*fun_t)(A0, A1, A2) const;

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		(unchecked_cast<C*>((AnyPtr&)args[0])->**(fun_t*)data)(
			unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3])
		), ReturnPolicyTest<ReturnResult>(vm), ReturnPolicyVoidTest();
	}
};

template<class C, class R , class A0, class A1, class A2>
struct cmemfun_holder<R (*)(C , A0, A1, A2)> : public cmemfun_holder_base3<C, R , A0, A1, A2>{
	typedef R (*fun_t)(C , A0, A1, A2);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		(*(fun_t*)data)(
			unchecked_cast<C>((AnyPtr&)args[0])
			, unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3])
		), ReturnPolicyTest<ReturnResult>(vm), ReturnPolicyVoidTest();
	}
};

#if defined(_WIN32) && defined(_M_IX86)
template<class C, class R , class A0, class A1, class A2>
struct cmemfun_holder<R (__stdcall *)(C , A0, A1, A2)> : public cmemfun_holder_base3<C, R , A0, A1, A2>{
	typedef R (__stdcall *fun_t)(C , A0, A1, A2);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		(*(fun_t*)data)(
			unchecked_cast<C>((AnyPtr&)args[0])
			, unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3])
		), ReturnPolicyTest<ReturnResult>(vm), ReturnPolicyVoidTest();
	}
};
#endif

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



template<class R , class A0, class A1, class A2, class A3>
struct cfun_holder_base4{
	enum{ PARAM_N = 4, METHOD = 0, EXTENDABLE = 0 };
	typedef param_types_holder4<void , A0, A1, A2, A3> fun_param_holder;
};

template<class R , class A0, class A1, class A2, class A3>
struct cfun_holder<R (*)(A0, A1, A2, A3)> : public cfun_holder_base4<R , A0, A1, A2, A3>{
	typedef R (*fun_t)(A0, A1, A2, A3);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		(*(fun_t*)data)(
			unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4])
		), ReturnPolicyTest<ReturnResult>(vm), ReturnPolicyVoidTest();
	}
};

#if defined(_WIN32) && defined(_M_IX86)
template<class R , class A0, class A1, class A2, class A3>
struct cfun_holder<R (__stdcall *)(A0, A1, A2, A3)> : public cfun_holder_base4<R , A0, A1, A2, A3>{
	typedef R (__stdcall *fun_t)(A0, A1, A2, A3);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		(*(fun_t*)data)(
			unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4])
		), ReturnPolicyTest<ReturnResult>(vm), ReturnPolicyVoidTest();
	}
};

#endif

template<class C, class R , class A0, class A1, class A2, class A3>
struct cmemfun_holder_base4{
	enum{ PARAM_N = 4, METHOD = 1, EXTENDABLE = 0 };
	typedef param_types_holder4<C , A0, A1, A2, A3> fun_param_holder;
};

template<class C, class R , class A0, class A1, class A2, class A3>
struct cmemfun_holder<R (C::*)(A0, A1, A2, A3)> : public cmemfun_holder_base4<C*, R , A0, A1, A2, A3>{
	typedef R (C::*fun_t)(A0, A1, A2, A3);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		(unchecked_cast<C*>((AnyPtr&)args[0])->**(fun_t*)data)(
			unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4])
		), ReturnPolicyTest<ReturnResult>(vm), ReturnPolicyVoidTest();
	}
};

template<class C, class R , class A0, class A1, class A2, class A3>
struct cmemfun_holder<R (C::*)(A0, A1, A2, A3) const> : public cmemfun_holder_base4<C*, R , A0, A1, A2, A3>{
	typedef R (C::*fun_t)(A0, A1, A2, A3) const;

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		(unchecked_cast<C*>((AnyPtr&)args[0])->**(fun_t*)data)(
			unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4])
		), ReturnPolicyTest<ReturnResult>(vm), ReturnPolicyVoidTest();
	}
};

template<class C, class R , class A0, class A1, class A2, class A3>
struct cmemfun_holder<R (*)(C , A0, A1, A2, A3)> : public cmemfun_holder_base4<C, R , A0, A1, A2, A3>{
	typedef R (*fun_t)(C , A0, A1, A2, A3);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		(*(fun_t*)data)(
			unchecked_cast<C>((AnyPtr&)args[0])
			, unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4])
		), ReturnPolicyTest<ReturnResult>(vm), ReturnPolicyVoidTest();
	}
};

#if defined(_WIN32) && defined(_M_IX86)
template<class C, class R , class A0, class A1, class A2, class A3>
struct cmemfun_holder<R (__stdcall *)(C , A0, A1, A2, A3)> : public cmemfun_holder_base4<C, R , A0, A1, A2, A3>{
	typedef R (__stdcall *fun_t)(C , A0, A1, A2, A3);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		(*(fun_t*)data)(
			unchecked_cast<C>((AnyPtr&)args[0])
			, unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4])
		), ReturnPolicyTest<ReturnResult>(vm), ReturnPolicyVoidTest();
	}
};
#endif

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



template<class R , class A0, class A1, class A2, class A3, class A4>
struct cfun_holder_base5{
	enum{ PARAM_N = 5, METHOD = 0, EXTENDABLE = 0 };
	typedef param_types_holder5<void , A0, A1, A2, A3, A4> fun_param_holder;
};

template<class R , class A0, class A1, class A2, class A3, class A4>
struct cfun_holder<R (*)(A0, A1, A2, A3, A4)> : public cfun_holder_base5<R , A0, A1, A2, A3, A4>{
	typedef R (*fun_t)(A0, A1, A2, A3, A4);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		(*(fun_t*)data)(
			unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4]), unchecked_cast<A4>((AnyPtr&)args[5])
		), ReturnPolicyTest<ReturnResult>(vm), ReturnPolicyVoidTest();
	}
};

#if defined(_WIN32) && defined(_M_IX86)
template<class R , class A0, class A1, class A2, class A3, class A4>
struct cfun_holder<R (__stdcall *)(A0, A1, A2, A3, A4)> : public cfun_holder_base5<R , A0, A1, A2, A3, A4>{
	typedef R (__stdcall *fun_t)(A0, A1, A2, A3, A4);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		(*(fun_t*)data)(
			unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4]), unchecked_cast<A4>((AnyPtr&)args[5])
		), ReturnPolicyTest<ReturnResult>(vm), ReturnPolicyVoidTest();
	}
};

#endif

template<class C, class R , class A0, class A1, class A2, class A3, class A4>
struct cmemfun_holder_base5{
	enum{ PARAM_N = 5, METHOD = 1, EXTENDABLE = 0 };
	typedef param_types_holder5<C , A0, A1, A2, A3, A4> fun_param_holder;
};

template<class C, class R , class A0, class A1, class A2, class A3, class A4>
struct cmemfun_holder<R (C::*)(A0, A1, A2, A3, A4)> : public cmemfun_holder_base5<C*, R , A0, A1, A2, A3, A4>{
	typedef R (C::*fun_t)(A0, A1, A2, A3, A4);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		(unchecked_cast<C*>((AnyPtr&)args[0])->**(fun_t*)data)(
			unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4]), unchecked_cast<A4>((AnyPtr&)args[5])
		), ReturnPolicyTest<ReturnResult>(vm), ReturnPolicyVoidTest();
	}
};

template<class C, class R , class A0, class A1, class A2, class A3, class A4>
struct cmemfun_holder<R (C::*)(A0, A1, A2, A3, A4) const> : public cmemfun_holder_base5<C*, R , A0, A1, A2, A3, A4>{
	typedef R (C::*fun_t)(A0, A1, A2, A3, A4) const;

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		(unchecked_cast<C*>((AnyPtr&)args[0])->**(fun_t*)data)(
			unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4]), unchecked_cast<A4>((AnyPtr&)args[5])
		), ReturnPolicyTest<ReturnResult>(vm), ReturnPolicyVoidTest();
	}
};

template<class C, class R , class A0, class A1, class A2, class A3, class A4>
struct cmemfun_holder<R (*)(C , A0, A1, A2, A3, A4)> : public cmemfun_holder_base5<C, R , A0, A1, A2, A3, A4>{
	typedef R (*fun_t)(C , A0, A1, A2, A3, A4);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		(*(fun_t*)data)(
			unchecked_cast<C>((AnyPtr&)args[0])
			, unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4]), unchecked_cast<A4>((AnyPtr&)args[5])
		), ReturnPolicyTest<ReturnResult>(vm), ReturnPolicyVoidTest();
	}
};

#if defined(_WIN32) && defined(_M_IX86)
template<class C, class R , class A0, class A1, class A2, class A3, class A4>
struct cmemfun_holder<R (__stdcall *)(C , A0, A1, A2, A3, A4)> : public cmemfun_holder_base5<C, R , A0, A1, A2, A3, A4>{
	typedef R (__stdcall *fun_t)(C , A0, A1, A2, A3, A4);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		(*(fun_t*)data)(
			unchecked_cast<C>((AnyPtr&)args[0])
			, unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4]), unchecked_cast<A4>((AnyPtr&)args[5])
		), ReturnPolicyTest<ReturnResult>(vm), ReturnPolicyVoidTest();
	}
};
#endif

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



template<class R , class A0, class A1, class A2, class A3, class A4, class A5>
struct cfun_holder_base6{
	enum{ PARAM_N = 6, METHOD = 0, EXTENDABLE = 0 };
	typedef param_types_holder6<void , A0, A1, A2, A3, A4, A5> fun_param_holder;
};

template<class R , class A0, class A1, class A2, class A3, class A4, class A5>
struct cfun_holder<R (*)(A0, A1, A2, A3, A4, A5)> : public cfun_holder_base6<R , A0, A1, A2, A3, A4, A5>{
	typedef R (*fun_t)(A0, A1, A2, A3, A4, A5);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		(*(fun_t*)data)(
			unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4]), unchecked_cast<A4>((AnyPtr&)args[5]), unchecked_cast<A5>((AnyPtr&)args[6])
		), ReturnPolicyTest<ReturnResult>(vm), ReturnPolicyVoidTest();
	}
};

#if defined(_WIN32) && defined(_M_IX86)
template<class R , class A0, class A1, class A2, class A3, class A4, class A5>
struct cfun_holder<R (__stdcall *)(A0, A1, A2, A3, A4, A5)> : public cfun_holder_base6<R , A0, A1, A2, A3, A4, A5>{
	typedef R (__stdcall *fun_t)(A0, A1, A2, A3, A4, A5);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		(*(fun_t*)data)(
			unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4]), unchecked_cast<A4>((AnyPtr&)args[5]), unchecked_cast<A5>((AnyPtr&)args[6])
		), ReturnPolicyTest<ReturnResult>(vm), ReturnPolicyVoidTest();
	}
};

#endif

template<class C, class R , class A0, class A1, class A2, class A3, class A4, class A5>
struct cmemfun_holder_base6{
	enum{ PARAM_N = 6, METHOD = 1, EXTENDABLE = 0 };
	typedef param_types_holder6<C , A0, A1, A2, A3, A4, A5> fun_param_holder;
};

template<class C, class R , class A0, class A1, class A2, class A3, class A4, class A5>
struct cmemfun_holder<R (C::*)(A0, A1, A2, A3, A4, A5)> : public cmemfun_holder_base6<C*, R , A0, A1, A2, A3, A4, A5>{
	typedef R (C::*fun_t)(A0, A1, A2, A3, A4, A5);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		(unchecked_cast<C*>((AnyPtr&)args[0])->**(fun_t*)data)(
			unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4]), unchecked_cast<A4>((AnyPtr&)args[5]), unchecked_cast<A5>((AnyPtr&)args[6])
		), ReturnPolicyTest<ReturnResult>(vm), ReturnPolicyVoidTest();
	}
};

template<class C, class R , class A0, class A1, class A2, class A3, class A4, class A5>
struct cmemfun_holder<R (C::*)(A0, A1, A2, A3, A4, A5) const> : public cmemfun_holder_base6<C*, R , A0, A1, A2, A3, A4, A5>{
	typedef R (C::*fun_t)(A0, A1, A2, A3, A4, A5) const;

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		(unchecked_cast<C*>((AnyPtr&)args[0])->**(fun_t*)data)(
			unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4]), unchecked_cast<A4>((AnyPtr&)args[5]), unchecked_cast<A5>((AnyPtr&)args[6])
		), ReturnPolicyTest<ReturnResult>(vm), ReturnPolicyVoidTest();
	}
};

template<class C, class R , class A0, class A1, class A2, class A3, class A4, class A5>
struct cmemfun_holder<R (*)(C , A0, A1, A2, A3, A4, A5)> : public cmemfun_holder_base6<C, R , A0, A1, A2, A3, A4, A5>{
	typedef R (*fun_t)(C , A0, A1, A2, A3, A4, A5);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		(*(fun_t*)data)(
			unchecked_cast<C>((AnyPtr&)args[0])
			, unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4]), unchecked_cast<A4>((AnyPtr&)args[5]), unchecked_cast<A5>((AnyPtr&)args[6])
		), ReturnPolicyTest<ReturnResult>(vm), ReturnPolicyVoidTest();
	}
};

#if defined(_WIN32) && defined(_M_IX86)
template<class C, class R , class A0, class A1, class A2, class A3, class A4, class A5>
struct cmemfun_holder<R (__stdcall *)(C , A0, A1, A2, A3, A4, A5)> : public cmemfun_holder_base6<C, R , A0, A1, A2, A3, A4, A5>{
	typedef R (__stdcall *fun_t)(C , A0, A1, A2, A3, A4, A5);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		(*(fun_t*)data)(
			unchecked_cast<C>((AnyPtr&)args[0])
			, unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4]), unchecked_cast<A4>((AnyPtr&)args[5]), unchecked_cast<A5>((AnyPtr&)args[6])
		), ReturnPolicyTest<ReturnResult>(vm), ReturnPolicyVoidTest();
	}
};
#endif

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



template<class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6>
struct cfun_holder_base7{
	enum{ PARAM_N = 7, METHOD = 0, EXTENDABLE = 0 };
	typedef param_types_holder7<void , A0, A1, A2, A3, A4, A5, A6> fun_param_holder;
};

template<class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6>
struct cfun_holder<R (*)(A0, A1, A2, A3, A4, A5, A6)> : public cfun_holder_base7<R , A0, A1, A2, A3, A4, A5, A6>{
	typedef R (*fun_t)(A0, A1, A2, A3, A4, A5, A6);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		(*(fun_t*)data)(
			unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4]), unchecked_cast<A4>((AnyPtr&)args[5]), unchecked_cast<A5>((AnyPtr&)args[6]), unchecked_cast<A6>((AnyPtr&)args[7])
		), ReturnPolicyTest<ReturnResult>(vm), ReturnPolicyVoidTest();
	}
};

#if defined(_WIN32) && defined(_M_IX86)
template<class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6>
struct cfun_holder<R (__stdcall *)(A0, A1, A2, A3, A4, A5, A6)> : public cfun_holder_base7<R , A0, A1, A2, A3, A4, A5, A6>{
	typedef R (__stdcall *fun_t)(A0, A1, A2, A3, A4, A5, A6);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		(*(fun_t*)data)(
			unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4]), unchecked_cast<A4>((AnyPtr&)args[5]), unchecked_cast<A5>((AnyPtr&)args[6]), unchecked_cast<A6>((AnyPtr&)args[7])
		), ReturnPolicyTest<ReturnResult>(vm), ReturnPolicyVoidTest();
	}
};

#endif

template<class C, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6>
struct cmemfun_holder_base7{
	enum{ PARAM_N = 7, METHOD = 1, EXTENDABLE = 0 };
	typedef param_types_holder7<C , A0, A1, A2, A3, A4, A5, A6> fun_param_holder;
};

template<class C, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6>
struct cmemfun_holder<R (C::*)(A0, A1, A2, A3, A4, A5, A6)> : public cmemfun_holder_base7<C*, R , A0, A1, A2, A3, A4, A5, A6>{
	typedef R (C::*fun_t)(A0, A1, A2, A3, A4, A5, A6);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		(unchecked_cast<C*>((AnyPtr&)args[0])->**(fun_t*)data)(
			unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4]), unchecked_cast<A4>((AnyPtr&)args[5]), unchecked_cast<A5>((AnyPtr&)args[6]), unchecked_cast<A6>((AnyPtr&)args[7])
		), ReturnPolicyTest<ReturnResult>(vm), ReturnPolicyVoidTest();
	}
};

template<class C, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6>
struct cmemfun_holder<R (C::*)(A0, A1, A2, A3, A4, A5, A6) const> : public cmemfun_holder_base7<C*, R , A0, A1, A2, A3, A4, A5, A6>{
	typedef R (C::*fun_t)(A0, A1, A2, A3, A4, A5, A6) const;

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		(unchecked_cast<C*>((AnyPtr&)args[0])->**(fun_t*)data)(
			unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4]), unchecked_cast<A4>((AnyPtr&)args[5]), unchecked_cast<A5>((AnyPtr&)args[6]), unchecked_cast<A6>((AnyPtr&)args[7])
		), ReturnPolicyTest<ReturnResult>(vm), ReturnPolicyVoidTest();
	}
};

template<class C, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6>
struct cmemfun_holder<R (*)(C , A0, A1, A2, A3, A4, A5, A6)> : public cmemfun_holder_base7<C, R , A0, A1, A2, A3, A4, A5, A6>{
	typedef R (*fun_t)(C , A0, A1, A2, A3, A4, A5, A6);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		(*(fun_t*)data)(
			unchecked_cast<C>((AnyPtr&)args[0])
			, unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4]), unchecked_cast<A4>((AnyPtr&)args[5]), unchecked_cast<A5>((AnyPtr&)args[6]), unchecked_cast<A6>((AnyPtr&)args[7])
		), ReturnPolicyTest<ReturnResult>(vm), ReturnPolicyVoidTest();
	}
};

#if defined(_WIN32) && defined(_M_IX86)
template<class C, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6>
struct cmemfun_holder<R (__stdcall *)(C , A0, A1, A2, A3, A4, A5, A6)> : public cmemfun_holder_base7<C, R , A0, A1, A2, A3, A4, A5, A6>{
	typedef R (__stdcall *fun_t)(C , A0, A1, A2, A3, A4, A5, A6);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		(*(fun_t*)data)(
			unchecked_cast<C>((AnyPtr&)args[0])
			, unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4]), unchecked_cast<A4>((AnyPtr&)args[5]), unchecked_cast<A5>((AnyPtr&)args[6]), unchecked_cast<A6>((AnyPtr&)args[7])
		), ReturnPolicyTest<ReturnResult>(vm), ReturnPolicyVoidTest();
	}
};
#endif

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



template<class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7>
struct cfun_holder_base8{
	enum{ PARAM_N = 8, METHOD = 0, EXTENDABLE = 0 };
	typedef param_types_holder8<void , A0, A1, A2, A3, A4, A5, A6, A7> fun_param_holder;
};

template<class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7>
struct cfun_holder<R (*)(A0, A1, A2, A3, A4, A5, A6, A7)> : public cfun_holder_base8<R , A0, A1, A2, A3, A4, A5, A6, A7>{
	typedef R (*fun_t)(A0, A1, A2, A3, A4, A5, A6, A7);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		(*(fun_t*)data)(
			unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4]), unchecked_cast<A4>((AnyPtr&)args[5]), unchecked_cast<A5>((AnyPtr&)args[6]), unchecked_cast<A6>((AnyPtr&)args[7]), unchecked_cast<A7>((AnyPtr&)args[8])
		), ReturnPolicyTest<ReturnResult>(vm), ReturnPolicyVoidTest();
	}
};

#if defined(_WIN32) && defined(_M_IX86)
template<class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7>
struct cfun_holder<R (__stdcall *)(A0, A1, A2, A3, A4, A5, A6, A7)> : public cfun_holder_base8<R , A0, A1, A2, A3, A4, A5, A6, A7>{
	typedef R (__stdcall *fun_t)(A0, A1, A2, A3, A4, A5, A6, A7);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		(*(fun_t*)data)(
			unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4]), unchecked_cast<A4>((AnyPtr&)args[5]), unchecked_cast<A5>((AnyPtr&)args[6]), unchecked_cast<A6>((AnyPtr&)args[7]), unchecked_cast<A7>((AnyPtr&)args[8])
		), ReturnPolicyTest<ReturnResult>(vm), ReturnPolicyVoidTest();
	}
};

#endif

template<class C, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7>
struct cmemfun_holder_base8{
	enum{ PARAM_N = 8, METHOD = 1, EXTENDABLE = 0 };
	typedef param_types_holder8<C , A0, A1, A2, A3, A4, A5, A6, A7> fun_param_holder;
};

template<class C, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7>
struct cmemfun_holder<R (C::*)(A0, A1, A2, A3, A4, A5, A6, A7)> : public cmemfun_holder_base8<C*, R , A0, A1, A2, A3, A4, A5, A6, A7>{
	typedef R (C::*fun_t)(A0, A1, A2, A3, A4, A5, A6, A7);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		(unchecked_cast<C*>((AnyPtr&)args[0])->**(fun_t*)data)(
			unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4]), unchecked_cast<A4>((AnyPtr&)args[5]), unchecked_cast<A5>((AnyPtr&)args[6]), unchecked_cast<A6>((AnyPtr&)args[7]), unchecked_cast<A7>((AnyPtr&)args[8])
		), ReturnPolicyTest<ReturnResult>(vm), ReturnPolicyVoidTest();
	}
};

template<class C, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7>
struct cmemfun_holder<R (C::*)(A0, A1, A2, A3, A4, A5, A6, A7) const> : public cmemfun_holder_base8<C*, R , A0, A1, A2, A3, A4, A5, A6, A7>{
	typedef R (C::*fun_t)(A0, A1, A2, A3, A4, A5, A6, A7) const;

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		(unchecked_cast<C*>((AnyPtr&)args[0])->**(fun_t*)data)(
			unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4]), unchecked_cast<A4>((AnyPtr&)args[5]), unchecked_cast<A5>((AnyPtr&)args[6]), unchecked_cast<A6>((AnyPtr&)args[7]), unchecked_cast<A7>((AnyPtr&)args[8])
		), ReturnPolicyTest<ReturnResult>(vm), ReturnPolicyVoidTest();
	}
};

template<class C, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7>
struct cmemfun_holder<R (*)(C , A0, A1, A2, A3, A4, A5, A6, A7)> : public cmemfun_holder_base8<C, R , A0, A1, A2, A3, A4, A5, A6, A7>{
	typedef R (*fun_t)(C , A0, A1, A2, A3, A4, A5, A6, A7);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		(*(fun_t*)data)(
			unchecked_cast<C>((AnyPtr&)args[0])
			, unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4]), unchecked_cast<A4>((AnyPtr&)args[5]), unchecked_cast<A5>((AnyPtr&)args[6]), unchecked_cast<A6>((AnyPtr&)args[7]), unchecked_cast<A7>((AnyPtr&)args[8])
		), ReturnPolicyTest<ReturnResult>(vm), ReturnPolicyVoidTest();
	}
};

#if defined(_WIN32) && defined(_M_IX86)
template<class C, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7>
struct cmemfun_holder<R (__stdcall *)(C , A0, A1, A2, A3, A4, A5, A6, A7)> : public cmemfun_holder_base8<C, R , A0, A1, A2, A3, A4, A5, A6, A7>{
	typedef R (__stdcall *fun_t)(C , A0, A1, A2, A3, A4, A5, A6, A7);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		(*(fun_t*)data)(
			unchecked_cast<C>((AnyPtr&)args[0])
			, unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4]), unchecked_cast<A4>((AnyPtr&)args[5]), unchecked_cast<A5>((AnyPtr&)args[6]), unchecked_cast<A6>((AnyPtr&)args[7]), unchecked_cast<A7>((AnyPtr&)args[8])
		), ReturnPolicyTest<ReturnResult>(vm), ReturnPolicyVoidTest();
	}
};
#endif

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



template<class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8>
struct cfun_holder_base9{
	enum{ PARAM_N = 9, METHOD = 0, EXTENDABLE = 0 };
	typedef param_types_holder9<void , A0, A1, A2, A3, A4, A5, A6, A7, A8> fun_param_holder;
};

template<class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8>
struct cfun_holder<R (*)(A0, A1, A2, A3, A4, A5, A6, A7, A8)> : public cfun_holder_base9<R , A0, A1, A2, A3, A4, A5, A6, A7, A8>{
	typedef R (*fun_t)(A0, A1, A2, A3, A4, A5, A6, A7, A8);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		(*(fun_t*)data)(
			unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4]), unchecked_cast<A4>((AnyPtr&)args[5]), unchecked_cast<A5>((AnyPtr&)args[6]), unchecked_cast<A6>((AnyPtr&)args[7]), unchecked_cast<A7>((AnyPtr&)args[8]), unchecked_cast<A8>((AnyPtr&)args[9])
		), ReturnPolicyTest<ReturnResult>(vm), ReturnPolicyVoidTest();
	}
};

#if defined(_WIN32) && defined(_M_IX86)
template<class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8>
struct cfun_holder<R (__stdcall *)(A0, A1, A2, A3, A4, A5, A6, A7, A8)> : public cfun_holder_base9<R , A0, A1, A2, A3, A4, A5, A6, A7, A8>{
	typedef R (__stdcall *fun_t)(A0, A1, A2, A3, A4, A5, A6, A7, A8);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		(*(fun_t*)data)(
			unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4]), unchecked_cast<A4>((AnyPtr&)args[5]), unchecked_cast<A5>((AnyPtr&)args[6]), unchecked_cast<A6>((AnyPtr&)args[7]), unchecked_cast<A7>((AnyPtr&)args[8]), unchecked_cast<A8>((AnyPtr&)args[9])
		), ReturnPolicyTest<ReturnResult>(vm), ReturnPolicyVoidTest();
	}
};

#endif

template<class C, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8>
struct cmemfun_holder_base9{
	enum{ PARAM_N = 9, METHOD = 1, EXTENDABLE = 0 };
	typedef param_types_holder9<C , A0, A1, A2, A3, A4, A5, A6, A7, A8> fun_param_holder;
};

template<class C, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8>
struct cmemfun_holder<R (C::*)(A0, A1, A2, A3, A4, A5, A6, A7, A8)> : public cmemfun_holder_base9<C*, R , A0, A1, A2, A3, A4, A5, A6, A7, A8>{
	typedef R (C::*fun_t)(A0, A1, A2, A3, A4, A5, A6, A7, A8);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		(unchecked_cast<C*>((AnyPtr&)args[0])->**(fun_t*)data)(
			unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4]), unchecked_cast<A4>((AnyPtr&)args[5]), unchecked_cast<A5>((AnyPtr&)args[6]), unchecked_cast<A6>((AnyPtr&)args[7]), unchecked_cast<A7>((AnyPtr&)args[8]), unchecked_cast<A8>((AnyPtr&)args[9])
		), ReturnPolicyTest<ReturnResult>(vm), ReturnPolicyVoidTest();
	}
};

template<class C, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8>
struct cmemfun_holder<R (C::*)(A0, A1, A2, A3, A4, A5, A6, A7, A8) const> : public cmemfun_holder_base9<C*, R , A0, A1, A2, A3, A4, A5, A6, A7, A8>{
	typedef R (C::*fun_t)(A0, A1, A2, A3, A4, A5, A6, A7, A8) const;

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		(unchecked_cast<C*>((AnyPtr&)args[0])->**(fun_t*)data)(
			unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4]), unchecked_cast<A4>((AnyPtr&)args[5]), unchecked_cast<A5>((AnyPtr&)args[6]), unchecked_cast<A6>((AnyPtr&)args[7]), unchecked_cast<A7>((AnyPtr&)args[8]), unchecked_cast<A8>((AnyPtr&)args[9])
		), ReturnPolicyTest<ReturnResult>(vm), ReturnPolicyVoidTest();
	}
};

template<class C, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8>
struct cmemfun_holder<R (*)(C , A0, A1, A2, A3, A4, A5, A6, A7, A8)> : public cmemfun_holder_base9<C, R , A0, A1, A2, A3, A4, A5, A6, A7, A8>{
	typedef R (*fun_t)(C , A0, A1, A2, A3, A4, A5, A6, A7, A8);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		(*(fun_t*)data)(
			unchecked_cast<C>((AnyPtr&)args[0])
			, unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4]), unchecked_cast<A4>((AnyPtr&)args[5]), unchecked_cast<A5>((AnyPtr&)args[6]), unchecked_cast<A6>((AnyPtr&)args[7]), unchecked_cast<A7>((AnyPtr&)args[8]), unchecked_cast<A8>((AnyPtr&)args[9])
		), ReturnPolicyTest<ReturnResult>(vm), ReturnPolicyVoidTest();
	}
};

#if defined(_WIN32) && defined(_M_IX86)
template<class C, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8>
struct cmemfun_holder<R (__stdcall *)(C , A0, A1, A2, A3, A4, A5, A6, A7, A8)> : public cmemfun_holder_base9<C, R , A0, A1, A2, A3, A4, A5, A6, A7, A8>{
	typedef R (__stdcall *fun_t)(C , A0, A1, A2, A3, A4, A5, A6, A7, A8);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		(*(fun_t*)data)(
			unchecked_cast<C>((AnyPtr&)args[0])
			, unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4]), unchecked_cast<A4>((AnyPtr&)args[5]), unchecked_cast<A5>((AnyPtr&)args[6]), unchecked_cast<A6>((AnyPtr&)args[7]), unchecked_cast<A7>((AnyPtr&)args[8]), unchecked_cast<A8>((AnyPtr&)args[9])
		), ReturnPolicyTest<ReturnResult>(vm), ReturnPolicyVoidTest();
	}
};
#endif

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



template<class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9>
struct cfun_holder_base10{
	enum{ PARAM_N = 10, METHOD = 0, EXTENDABLE = 0 };
	typedef param_types_holder10<void , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9> fun_param_holder;
};

template<class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9>
struct cfun_holder<R (*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9)> : public cfun_holder_base10<R , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9>{
	typedef R (*fun_t)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		(*(fun_t*)data)(
			unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4]), unchecked_cast<A4>((AnyPtr&)args[5]), unchecked_cast<A5>((AnyPtr&)args[6]), unchecked_cast<A6>((AnyPtr&)args[7]), unchecked_cast<A7>((AnyPtr&)args[8]), unchecked_cast<A8>((AnyPtr&)args[9]), unchecked_cast<A9>((AnyPtr&)args[10])
		), ReturnPolicyTest<ReturnResult>(vm), ReturnPolicyVoidTest();
	}
};

#if defined(_WIN32) && defined(_M_IX86)
template<class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9>
struct cfun_holder<R (__stdcall *)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9)> : public cfun_holder_base10<R , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9>{
	typedef R (__stdcall *fun_t)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		(*(fun_t*)data)(
			unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4]), unchecked_cast<A4>((AnyPtr&)args[5]), unchecked_cast<A5>((AnyPtr&)args[6]), unchecked_cast<A6>((AnyPtr&)args[7]), unchecked_cast<A7>((AnyPtr&)args[8]), unchecked_cast<A8>((AnyPtr&)args[9]), unchecked_cast<A9>((AnyPtr&)args[10])
		), ReturnPolicyTest<ReturnResult>(vm), ReturnPolicyVoidTest();
	}
};

#endif

template<class C, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9>
struct cmemfun_holder_base10{
	enum{ PARAM_N = 10, METHOD = 1, EXTENDABLE = 0 };
	typedef param_types_holder10<C , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9> fun_param_holder;
};

template<class C, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9>
struct cmemfun_holder<R (C::*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9)> : public cmemfun_holder_base10<C*, R , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9>{
	typedef R (C::*fun_t)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		(unchecked_cast<C*>((AnyPtr&)args[0])->**(fun_t*)data)(
			unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4]), unchecked_cast<A4>((AnyPtr&)args[5]), unchecked_cast<A5>((AnyPtr&)args[6]), unchecked_cast<A6>((AnyPtr&)args[7]), unchecked_cast<A7>((AnyPtr&)args[8]), unchecked_cast<A8>((AnyPtr&)args[9]), unchecked_cast<A9>((AnyPtr&)args[10])
		), ReturnPolicyTest<ReturnResult>(vm), ReturnPolicyVoidTest();
	}
};

template<class C, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9>
struct cmemfun_holder<R (C::*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9) const> : public cmemfun_holder_base10<C*, R , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9>{
	typedef R (C::*fun_t)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9) const;

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		(unchecked_cast<C*>((AnyPtr&)args[0])->**(fun_t*)data)(
			unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4]), unchecked_cast<A4>((AnyPtr&)args[5]), unchecked_cast<A5>((AnyPtr&)args[6]), unchecked_cast<A6>((AnyPtr&)args[7]), unchecked_cast<A7>((AnyPtr&)args[8]), unchecked_cast<A8>((AnyPtr&)args[9]), unchecked_cast<A9>((AnyPtr&)args[10])
		), ReturnPolicyTest<ReturnResult>(vm), ReturnPolicyVoidTest();
	}
};

template<class C, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9>
struct cmemfun_holder<R (*)(C , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9)> : public cmemfun_holder_base10<C, R , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9>{
	typedef R (*fun_t)(C , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		(*(fun_t*)data)(
			unchecked_cast<C>((AnyPtr&)args[0])
			, unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4]), unchecked_cast<A4>((AnyPtr&)args[5]), unchecked_cast<A5>((AnyPtr&)args[6]), unchecked_cast<A6>((AnyPtr&)args[7]), unchecked_cast<A7>((AnyPtr&)args[8]), unchecked_cast<A8>((AnyPtr&)args[9]), unchecked_cast<A9>((AnyPtr&)args[10])
		), ReturnPolicyTest<ReturnResult>(vm), ReturnPolicyVoidTest();
	}
};

#if defined(_WIN32) && defined(_M_IX86)
template<class C, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9>
struct cmemfun_holder<R (__stdcall *)(C , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9)> : public cmemfun_holder_base10<C, R , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9>{
	typedef R (__stdcall *fun_t)(C , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		(*(fun_t*)data)(
			unchecked_cast<C>((AnyPtr&)args[0])
			, unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4]), unchecked_cast<A4>((AnyPtr&)args[5]), unchecked_cast<A5>((AnyPtr&)args[6]), unchecked_cast<A6>((AnyPtr&)args[7]), unchecked_cast<A7>((AnyPtr&)args[8]), unchecked_cast<A8>((AnyPtr&)args[9]), unchecked_cast<A9>((AnyPtr&)args[10])
		), ReturnPolicyTest<ReturnResult>(vm), ReturnPolicyVoidTest();
	}
};
#endif

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



template<class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10>
struct cfun_holder_base11{
	enum{ PARAM_N = 11, METHOD = 0, EXTENDABLE = 0 };
	typedef param_types_holder11<void , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10> fun_param_holder;
};

template<class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10>
struct cfun_holder<R (*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10)> : public cfun_holder_base11<R , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10>{
	typedef R (*fun_t)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		(*(fun_t*)data)(
			unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4]), unchecked_cast<A4>((AnyPtr&)args[5]), unchecked_cast<A5>((AnyPtr&)args[6]), unchecked_cast<A6>((AnyPtr&)args[7]), unchecked_cast<A7>((AnyPtr&)args[8]), unchecked_cast<A8>((AnyPtr&)args[9]), unchecked_cast<A9>((AnyPtr&)args[10]), unchecked_cast<A10>((AnyPtr&)args[11])
		), ReturnPolicyTest<ReturnResult>(vm), ReturnPolicyVoidTest();
	}
};

#if defined(_WIN32) && defined(_M_IX86)
template<class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10>
struct cfun_holder<R (__stdcall *)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10)> : public cfun_holder_base11<R , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10>{
	typedef R (__stdcall *fun_t)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		(*(fun_t*)data)(
			unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4]), unchecked_cast<A4>((AnyPtr&)args[5]), unchecked_cast<A5>((AnyPtr&)args[6]), unchecked_cast<A6>((AnyPtr&)args[7]), unchecked_cast<A7>((AnyPtr&)args[8]), unchecked_cast<A8>((AnyPtr&)args[9]), unchecked_cast<A9>((AnyPtr&)args[10]), unchecked_cast<A10>((AnyPtr&)args[11])
		), ReturnPolicyTest<ReturnResult>(vm), ReturnPolicyVoidTest();
	}
};

#endif

template<class C, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10>
struct cmemfun_holder_base11{
	enum{ PARAM_N = 11, METHOD = 1, EXTENDABLE = 0 };
	typedef param_types_holder11<C , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10> fun_param_holder;
};

template<class C, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10>
struct cmemfun_holder<R (C::*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10)> : public cmemfun_holder_base11<C*, R , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10>{
	typedef R (C::*fun_t)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		(unchecked_cast<C*>((AnyPtr&)args[0])->**(fun_t*)data)(
			unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4]), unchecked_cast<A4>((AnyPtr&)args[5]), unchecked_cast<A5>((AnyPtr&)args[6]), unchecked_cast<A6>((AnyPtr&)args[7]), unchecked_cast<A7>((AnyPtr&)args[8]), unchecked_cast<A8>((AnyPtr&)args[9]), unchecked_cast<A9>((AnyPtr&)args[10]), unchecked_cast<A10>((AnyPtr&)args[11])
		), ReturnPolicyTest<ReturnResult>(vm), ReturnPolicyVoidTest();
	}
};

template<class C, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10>
struct cmemfun_holder<R (C::*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10) const> : public cmemfun_holder_base11<C*, R , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10>{
	typedef R (C::*fun_t)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10) const;

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		(unchecked_cast<C*>((AnyPtr&)args[0])->**(fun_t*)data)(
			unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4]), unchecked_cast<A4>((AnyPtr&)args[5]), unchecked_cast<A5>((AnyPtr&)args[6]), unchecked_cast<A6>((AnyPtr&)args[7]), unchecked_cast<A7>((AnyPtr&)args[8]), unchecked_cast<A8>((AnyPtr&)args[9]), unchecked_cast<A9>((AnyPtr&)args[10]), unchecked_cast<A10>((AnyPtr&)args[11])
		), ReturnPolicyTest<ReturnResult>(vm), ReturnPolicyVoidTest();
	}
};

template<class C, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10>
struct cmemfun_holder<R (*)(C , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10)> : public cmemfun_holder_base11<C, R , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10>{
	typedef R (*fun_t)(C , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		(*(fun_t*)data)(
			unchecked_cast<C>((AnyPtr&)args[0])
			, unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4]), unchecked_cast<A4>((AnyPtr&)args[5]), unchecked_cast<A5>((AnyPtr&)args[6]), unchecked_cast<A6>((AnyPtr&)args[7]), unchecked_cast<A7>((AnyPtr&)args[8]), unchecked_cast<A8>((AnyPtr&)args[9]), unchecked_cast<A9>((AnyPtr&)args[10]), unchecked_cast<A10>((AnyPtr&)args[11])
		), ReturnPolicyTest<ReturnResult>(vm), ReturnPolicyVoidTest();
	}
};

#if defined(_WIN32) && defined(_M_IX86)
template<class C, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10>
struct cmemfun_holder<R (__stdcall *)(C , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10)> : public cmemfun_holder_base11<C, R , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10>{
	typedef R (__stdcall *fun_t)(C , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		(*(fun_t*)data)(
			unchecked_cast<C>((AnyPtr&)args[0])
			, unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4]), unchecked_cast<A4>((AnyPtr&)args[5]), unchecked_cast<A5>((AnyPtr&)args[6]), unchecked_cast<A6>((AnyPtr&)args[7]), unchecked_cast<A7>((AnyPtr&)args[8]), unchecked_cast<A8>((AnyPtr&)args[9]), unchecked_cast<A9>((AnyPtr&)args[10]), unchecked_cast<A10>((AnyPtr&)args[11])
		), ReturnPolicyTest<ReturnResult>(vm), ReturnPolicyVoidTest();
	}
};
#endif

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



template<class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11>
struct cfun_holder_base12{
	enum{ PARAM_N = 12, METHOD = 0, EXTENDABLE = 0 };
	typedef param_types_holder12<void , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11> fun_param_holder;
};

template<class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11>
struct cfun_holder<R (*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11)> : public cfun_holder_base12<R , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11>{
	typedef R (*fun_t)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		(*(fun_t*)data)(
			unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4]), unchecked_cast<A4>((AnyPtr&)args[5]), unchecked_cast<A5>((AnyPtr&)args[6]), unchecked_cast<A6>((AnyPtr&)args[7]), unchecked_cast<A7>((AnyPtr&)args[8]), unchecked_cast<A8>((AnyPtr&)args[9]), unchecked_cast<A9>((AnyPtr&)args[10]), unchecked_cast<A10>((AnyPtr&)args[11]), unchecked_cast<A11>((AnyPtr&)args[12])
		), ReturnPolicyTest<ReturnResult>(vm), ReturnPolicyVoidTest();
	}
};

#if defined(_WIN32) && defined(_M_IX86)
template<class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11>
struct cfun_holder<R (__stdcall *)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11)> : public cfun_holder_base12<R , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11>{
	typedef R (__stdcall *fun_t)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		(*(fun_t*)data)(
			unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4]), unchecked_cast<A4>((AnyPtr&)args[5]), unchecked_cast<A5>((AnyPtr&)args[6]), unchecked_cast<A6>((AnyPtr&)args[7]), unchecked_cast<A7>((AnyPtr&)args[8]), unchecked_cast<A8>((AnyPtr&)args[9]), unchecked_cast<A9>((AnyPtr&)args[10]), unchecked_cast<A10>((AnyPtr&)args[11]), unchecked_cast<A11>((AnyPtr&)args[12])
		), ReturnPolicyTest<ReturnResult>(vm), ReturnPolicyVoidTest();
	}
};

#endif

template<class C, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11>
struct cmemfun_holder_base12{
	enum{ PARAM_N = 12, METHOD = 1, EXTENDABLE = 0 };
	typedef param_types_holder12<C , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11> fun_param_holder;
};

template<class C, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11>
struct cmemfun_holder<R (C::*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11)> : public cmemfun_holder_base12<C*, R , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11>{
	typedef R (C::*fun_t)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		(unchecked_cast<C*>((AnyPtr&)args[0])->**(fun_t*)data)(
			unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4]), unchecked_cast<A4>((AnyPtr&)args[5]), unchecked_cast<A5>((AnyPtr&)args[6]), unchecked_cast<A6>((AnyPtr&)args[7]), unchecked_cast<A7>((AnyPtr&)args[8]), unchecked_cast<A8>((AnyPtr&)args[9]), unchecked_cast<A9>((AnyPtr&)args[10]), unchecked_cast<A10>((AnyPtr&)args[11]), unchecked_cast<A11>((AnyPtr&)args[12])
		), ReturnPolicyTest<ReturnResult>(vm), ReturnPolicyVoidTest();
	}
};

template<class C, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11>
struct cmemfun_holder<R (C::*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11) const> : public cmemfun_holder_base12<C*, R , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11>{
	typedef R (C::*fun_t)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11) const;

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		(unchecked_cast<C*>((AnyPtr&)args[0])->**(fun_t*)data)(
			unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4]), unchecked_cast<A4>((AnyPtr&)args[5]), unchecked_cast<A5>((AnyPtr&)args[6]), unchecked_cast<A6>((AnyPtr&)args[7]), unchecked_cast<A7>((AnyPtr&)args[8]), unchecked_cast<A8>((AnyPtr&)args[9]), unchecked_cast<A9>((AnyPtr&)args[10]), unchecked_cast<A10>((AnyPtr&)args[11]), unchecked_cast<A11>((AnyPtr&)args[12])
		), ReturnPolicyTest<ReturnResult>(vm), ReturnPolicyVoidTest();
	}
};

template<class C, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11>
struct cmemfun_holder<R (*)(C , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11)> : public cmemfun_holder_base12<C, R , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11>{
	typedef R (*fun_t)(C , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		(*(fun_t*)data)(
			unchecked_cast<C>((AnyPtr&)args[0])
			, unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4]), unchecked_cast<A4>((AnyPtr&)args[5]), unchecked_cast<A5>((AnyPtr&)args[6]), unchecked_cast<A6>((AnyPtr&)args[7]), unchecked_cast<A7>((AnyPtr&)args[8]), unchecked_cast<A8>((AnyPtr&)args[9]), unchecked_cast<A9>((AnyPtr&)args[10]), unchecked_cast<A10>((AnyPtr&)args[11]), unchecked_cast<A11>((AnyPtr&)args[12])
		), ReturnPolicyTest<ReturnResult>(vm), ReturnPolicyVoidTest();
	}
};

#if defined(_WIN32) && defined(_M_IX86)
template<class C, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11>
struct cmemfun_holder<R (__stdcall *)(C , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11)> : public cmemfun_holder_base12<C, R , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11>{
	typedef R (__stdcall *fun_t)(C , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		(*(fun_t*)data)(
			unchecked_cast<C>((AnyPtr&)args[0])
			, unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4]), unchecked_cast<A4>((AnyPtr&)args[5]), unchecked_cast<A5>((AnyPtr&)args[6]), unchecked_cast<A6>((AnyPtr&)args[7]), unchecked_cast<A7>((AnyPtr&)args[8]), unchecked_cast<A8>((AnyPtr&)args[9]), unchecked_cast<A9>((AnyPtr&)args[10]), unchecked_cast<A10>((AnyPtr&)args[11]), unchecked_cast<A11>((AnyPtr&)args[12])
		), ReturnPolicyTest<ReturnResult>(vm), ReturnPolicyVoidTest();
	}
};
#endif

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



template<class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12>
struct cfun_holder_base13{
	enum{ PARAM_N = 13, METHOD = 0, EXTENDABLE = 0 };
	typedef param_types_holder13<void , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12> fun_param_holder;
};

template<class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12>
struct cfun_holder<R (*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12)> : public cfun_holder_base13<R , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12>{
	typedef R (*fun_t)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		(*(fun_t*)data)(
			unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4]), unchecked_cast<A4>((AnyPtr&)args[5]), unchecked_cast<A5>((AnyPtr&)args[6]), unchecked_cast<A6>((AnyPtr&)args[7]), unchecked_cast<A7>((AnyPtr&)args[8]), unchecked_cast<A8>((AnyPtr&)args[9]), unchecked_cast<A9>((AnyPtr&)args[10]), unchecked_cast<A10>((AnyPtr&)args[11]), unchecked_cast<A11>((AnyPtr&)args[12]), unchecked_cast<A12>((AnyPtr&)args[13])
		), ReturnPolicyTest<ReturnResult>(vm), ReturnPolicyVoidTest();
	}
};

#if defined(_WIN32) && defined(_M_IX86)
template<class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12>
struct cfun_holder<R (__stdcall *)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12)> : public cfun_holder_base13<R , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12>{
	typedef R (__stdcall *fun_t)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		(*(fun_t*)data)(
			unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4]), unchecked_cast<A4>((AnyPtr&)args[5]), unchecked_cast<A5>((AnyPtr&)args[6]), unchecked_cast<A6>((AnyPtr&)args[7]), unchecked_cast<A7>((AnyPtr&)args[8]), unchecked_cast<A8>((AnyPtr&)args[9]), unchecked_cast<A9>((AnyPtr&)args[10]), unchecked_cast<A10>((AnyPtr&)args[11]), unchecked_cast<A11>((AnyPtr&)args[12]), unchecked_cast<A12>((AnyPtr&)args[13])
		), ReturnPolicyTest<ReturnResult>(vm), ReturnPolicyVoidTest();
	}
};

#endif

template<class C, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12>
struct cmemfun_holder_base13{
	enum{ PARAM_N = 13, METHOD = 1, EXTENDABLE = 0 };
	typedef param_types_holder13<C , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12> fun_param_holder;
};

template<class C, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12>
struct cmemfun_holder<R (C::*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12)> : public cmemfun_holder_base13<C*, R , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12>{
	typedef R (C::*fun_t)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		(unchecked_cast<C*>((AnyPtr&)args[0])->**(fun_t*)data)(
			unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4]), unchecked_cast<A4>((AnyPtr&)args[5]), unchecked_cast<A5>((AnyPtr&)args[6]), unchecked_cast<A6>((AnyPtr&)args[7]), unchecked_cast<A7>((AnyPtr&)args[8]), unchecked_cast<A8>((AnyPtr&)args[9]), unchecked_cast<A9>((AnyPtr&)args[10]), unchecked_cast<A10>((AnyPtr&)args[11]), unchecked_cast<A11>((AnyPtr&)args[12]), unchecked_cast<A12>((AnyPtr&)args[13])
		), ReturnPolicyTest<ReturnResult>(vm), ReturnPolicyVoidTest();
	}
};

template<class C, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12>
struct cmemfun_holder<R (C::*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12) const> : public cmemfun_holder_base13<C*, R , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12>{
	typedef R (C::*fun_t)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12) const;

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		(unchecked_cast<C*>((AnyPtr&)args[0])->**(fun_t*)data)(
			unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4]), unchecked_cast<A4>((AnyPtr&)args[5]), unchecked_cast<A5>((AnyPtr&)args[6]), unchecked_cast<A6>((AnyPtr&)args[7]), unchecked_cast<A7>((AnyPtr&)args[8]), unchecked_cast<A8>((AnyPtr&)args[9]), unchecked_cast<A9>((AnyPtr&)args[10]), unchecked_cast<A10>((AnyPtr&)args[11]), unchecked_cast<A11>((AnyPtr&)args[12]), unchecked_cast<A12>((AnyPtr&)args[13])
		), ReturnPolicyTest<ReturnResult>(vm), ReturnPolicyVoidTest();
	}
};

template<class C, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12>
struct cmemfun_holder<R (*)(C , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12)> : public cmemfun_holder_base13<C, R , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12>{
	typedef R (*fun_t)(C , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		(*(fun_t*)data)(
			unchecked_cast<C>((AnyPtr&)args[0])
			, unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4]), unchecked_cast<A4>((AnyPtr&)args[5]), unchecked_cast<A5>((AnyPtr&)args[6]), unchecked_cast<A6>((AnyPtr&)args[7]), unchecked_cast<A7>((AnyPtr&)args[8]), unchecked_cast<A8>((AnyPtr&)args[9]), unchecked_cast<A9>((AnyPtr&)args[10]), unchecked_cast<A10>((AnyPtr&)args[11]), unchecked_cast<A11>((AnyPtr&)args[12]), unchecked_cast<A12>((AnyPtr&)args[13])
		), ReturnPolicyTest<ReturnResult>(vm), ReturnPolicyVoidTest();
	}
};

#if defined(_WIN32) && defined(_M_IX86)
template<class C, class R , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12>
struct cmemfun_holder<R (__stdcall *)(C , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12)> : public cmemfun_holder_base13<C, R , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12>{
	typedef R (__stdcall *fun_t)(C , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12);

	static void call(const VMachinePtr& vm, const void* data, UninitializedAny* args){ 
		(*(fun_t*)data)(
			unchecked_cast<C>((AnyPtr&)args[0])
			, unchecked_cast<A0>((AnyPtr&)args[1]), unchecked_cast<A1>((AnyPtr&)args[2]), unchecked_cast<A2>((AnyPtr&)args[3]), unchecked_cast<A3>((AnyPtr&)args[4]), unchecked_cast<A4>((AnyPtr&)args[5]), unchecked_cast<A5>((AnyPtr&)args[6]), unchecked_cast<A6>((AnyPtr&)args[7]), unchecked_cast<A7>((AnyPtr&)args[8]), unchecked_cast<A8>((AnyPtr&)args[9]), unchecked_cast<A9>((AnyPtr&)args[10]), unchecked_cast<A10>((AnyPtr&)args[11]), unchecked_cast<A11>((AnyPtr&)args[12]), unchecked_cast<A12>((AnyPtr&)args[13])
		), ReturnPolicyTest<ReturnResult>(vm), ReturnPolicyVoidTest();
	}
};
#endif

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
		(*(fun_t*)data)(vm)
		, ReturnPolicyTest<ReturnResult>(vm), ReturnPolicyVoidTest();
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

}

#endif
