
#pragma once

namespace xtal{

class CFun;	

struct ReturnResult{
	static void return_result(const VMachinePtr& vm){
		vm->return_result();
	}

	static void return_result2(const VMachinePtr& vm, const AnyPtr& ret, TypeBool<1>){
		vm->return_result(ret);
	}
		
	template<class T>
	static void return_result2(const VMachinePtr& vm, const T& ret, TypeBool<0>){
		vm->return_result(xnew<T>(ret));
	}

	template<class T>
	static void return_result(const VMachinePtr& vm, const T& ret){
		return_result2(vm, ret, TypeBool<Convertible<T, AnyPtr>::value>());
	}
};

struct ReturnNone{
	static void return_result(const VMachinePtr& vm){}

	template<class T>
	static void return_result(const VMachinePtr& vm, const T& v){
		ReturnResult::return_result(vm, v);
	}
};

struct ParamInfo{
	Named* params;
	int_t min_param_count;
	int_t max_param_count;
};

struct ParamInfoAndVM{
	const ParamInfo& p;
	const VMachinePtr& vm;
	uint_t flags;
	const void* data;

	ParamInfoAndVM(const ParamInfo& p, const VMachinePtr& vm, const void* data)
		:p(p), vm(vm), flags(0), data(data){}
};

template<class T, int N>
struct arg_getter{
	static const AnyPtr&
	get(ParamInfoAndVM& pvm){
		const AnyPtr& p = pvm.vm->arg_default(N, pvm.p.params[N]);
		pvm.flags |= (can_cast<T>(p)^1) << (N+1);
		return p;
	}

	static typename CastResult<T>::type 
	cast(const AnyPtr& p){
		return unchecked_cast<T>(p);
	}
};

template<class T>
struct arg_this_getter{
	static const AnyPtr&
	get(ParamInfoAndVM& pvm){
		const AnyPtr& p = pvm.vm->get_arg_this();
		pvm.flags |= (can_cast<T>(p)^1);
		return p;
	}

	static typename CastResult<T>::type 
	cast(const AnyPtr& p){
		return unchecked_cast<T>(p);
	}
};

struct arg_getter_vm{
	static const AnyPtr& 
	get(ParamInfoAndVM& pvm){
		return pvm.vm;
	}

	static const VMachinePtr& 
	cast(const AnyPtr& p){
		return *(VMachinePtr*)&p;
	}
};

template<class Policy>
struct ReturnPolicyTest{
	const VMachinePtr& vm;

	ReturnPolicyTest(const VMachinePtr& vm)
		:vm(vm){}
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

template<class Fun, Fun fun>
struct cfun_holder_static{
	enum{ PARAMS = 0, PARAMS2 = 0 };
};

template<class Fun>
struct cfun_holder{
	enum{ PARAMS = 0, PARAMS2 = 0 };
};

template<class Fun, Fun fun>
struct cmemfun_holder_static{
	enum{ PARAMS = 0, PARAMS2 = 0 };
};

template<class Fun>
struct cmemfun_holder{
	enum{ PARAMS = 0, PARAMS2 = 0 };
};

template<class T, class A0=void, class A1=void, class A2=void, class A3=void, class A4=void, class A5=void, class A6=void, class A7=void, class A8=void, class A9=void>
struct ctor_fun{
	enum{ PARAMS = 0, PARAMS2 = 0 };
};

//{REPEAT{{
/*
template<class R #COMMA_REPEAT#class A`i`#, R (*fun)(#REPEAT_COMMA#A`i`#)>
struct cfun_holder_static<R (*)(#REPEAT_COMMA#A`i`#), fun>{
	enum{ PARAMS = `n`, PARAMS2 = `n` };
	typedef R (*fun_t)(#REPEAT_COMMA#A`i`#);
	#REPEAT#typedef arg_getter<A`i`, `i`> ARG`i`;#
	typedef ReturnResult Result;
	R operator()(#REPEAT_COMMA#A`i` a`i`#){ 
		return (*fun)(#REPEAT_COMMA#a`i`#); 
	}
};

template<class R #COMMA_REPEAT#class A`i`#>
struct cfun_holder<R (*)(#REPEAT_COMMA#A`i`#)>{
	enum{ PARAMS = `n`, PARAMS2 = `n` };
	typedef R (*fun_t)(#REPEAT_COMMA#A`i`#);
	#REPEAT#typedef arg_getter<A`i`, `i`> ARG`i`;#
	typedef ReturnResult Result;
	fun_t fun;
	cfun_holder(const fun_t& f):fun(f){}
	R operator()(#REPEAT_COMMA#A`i` a`i`#){ 
		return (*fun)(#REPEAT_COMMA#a`i`#); 
	}
};

#ifdef _WIN32
template<class R #COMMA_REPEAT#class A`i`#>
struct cfun_holder<R (__stdcall *)(#REPEAT_COMMA#A`i`#)>{
	enum{ PARAMS = `n`, PARAMS2 = `n` };
	typedef R (__stdcall *fun_t)(#REPEAT_COMMA#A`i`#);
	#REPEAT#typedef arg_getter<A`i`, `i`> ARG`i`;#
	typedef ReturnResult Result;
	fun_t fun;
	cfun_holder(const fun_t& f):fun(f){}
	R operator()(#REPEAT_COMMA#A`i` a`i`#){ 
		return (*fun)(#REPEAT_COMMA#a`i`#); 
	}
};
#endif

template<class C, class R #COMMA_REPEAT#class A`i`#, R (C::*fun)(#REPEAT_COMMA#A`i`#)>
struct cmemfun_holder_static<R (C::*)(#REPEAT_COMMA#A`i`#), fun>{
	enum{ PARAMS = `n+1`, PARAMS2 = `n` };
	typedef R (C::*fun_t)(#REPEAT_COMMA#A`i`#);
	typedef arg_this_getter<C*> ARG0;
	#REPEAT#typedef arg_getter<A`i`, `i`> ARG`i+1`;#
	typedef ReturnResult Result;
	R operator()(C* self #COMMA_REPEAT#A`i` a`i`#){ 
		return (self->*fun)(#REPEAT_COMMA#a`i`#); 
	}
};

template<class C, class R #COMMA_REPEAT#class A`i`#>
struct cmemfun_holder<R (C::*)(#REPEAT_COMMA#A`i`#)>{
	enum{ PARAMS = `n+1`, PARAMS2 = `n` };
	typedef R (C::*fun_t)(#REPEAT_COMMA#A`i`#);
	typedef arg_this_getter<C*> ARG0;
	#REPEAT#typedef arg_getter<A`i`, `i`> ARG`i+1`;#
	typedef ReturnResult Result;
	fun_t fun;
	cmemfun_holder(const fun_t& f):fun(f){}
	R operator()(C* self #COMMA_REPEAT#A`i` a`i`#){ 
		return (self->*fun)(#REPEAT_COMMA#a`i`#); 
	}
};

template<class C, class R #COMMA_REPEAT#class A`i`#>
struct cmemfun_holder<R (C::*)(#REPEAT_COMMA#A`i`#) const>{
	enum{ PARAMS = `n+1`, PARAMS2 = `n` };
	typedef R (C::*fun_t)(#REPEAT_COMMA#A`i`#) const;
	typedef arg_this_getter<C*> ARG0;
	#REPEAT#typedef arg_getter<A`i`, `i`> ARG`i+1`;#
	typedef ReturnResult Result;
	fun_t fun;
	cmemfun_holder(const fun_t& f):fun(f){}
	R operator()(C* self #COMMA_REPEAT#A`i` a`i`#){ 
		return (self->*fun)(#REPEAT_COMMA#a`i`#); 
	}
};

template<class C, class R #COMMA_REPEAT#class A`i`#, R (*fun)(#REPEAT_COMMA#A`i`#)>
struct cmemfun_holder_static<R (*)(C #COMMA_REPEAT#A`i`#), fun>{
	enum{ PARAMS = `n+1`, PARAMS2 = `n` };
	typedef R (*fun_t)(#REPEAT_COMMA#A`i`#);
	typedef arg_this_getter<C> ARG0;
	#REPEAT#typedef arg_getter<A`i`, `i`> ARG`i+1`;#
	typedef ReturnResult Result;
	R operator()(C c #COMMA_REPEAT#A`i` a`i`#){ 
		return (*fun)(c #COMMA_REPEAT#a`i`#); 
	}
};

template<class C, class R #COMMA_REPEAT#class A`i`#>
struct cmemfun_holder<R (*)(C #COMMA_REPEAT#A`i`#)>{
	enum{ PARAMS = `n+1`, PARAMS2 = `n` };
	typedef R (*fun_t)(C #COMMA_REPEAT#A`i`#);
	typedef arg_this_getter<C> ARG0;
	#REPEAT#typedef arg_getter<A`i`, `i`> ARG`i+1`;#
	typedef ReturnResult Result;
	fun_t fun;
	cmemfun_holder(const fun_t& f):fun(f){}
	R operator()(C c #COMMA_REPEAT#A`i` a`i`#){ 
		return (*fun)(c #COMMA_REPEAT#a`i`#); 
	}
};

#ifdef _WIN32
template<class C, class R #COMMA_REPEAT#class A`i`#>
struct cmemfun_holder<R (__stdcall *)(C #COMMA_REPEAT#A`i`#)>{
	enum{ PARAMS = `n+1`, PARAMS2 = `n` };
	typedef R (__stdcall *fun_t)(C #COMMA_REPEAT#A`i`#);
	typedef arg_this_getter<C> ARG0;
	#REPEAT#typedef arg_getter<A`i`, `i`> ARG`i+1`;#
	typedef ReturnResult Result;
	fun_t fun;
	cmemfun_holder(const fun_t& f):fun(f){}
	R operator()(C c #COMMA_REPEAT#A`i` a`i`#){ 
		return (*fun)(c #COMMA_REPEAT#a`i`#); 
	}
};
#endif

template<class T #COMMA_REPEAT#class A`i`#>
struct ctor_fun<T #COMMA_REPEAT#A`i`#>{
	enum{ PARAMS = `n`, PARAMS2 = `n` };
	#REPEAT#typedef arg_getter<A`i`, `i`> ARG`i`;#
	typedef ReturnResult Result;
	SmartPtr<T> operator()(#REPEAT_COMMA#A`i` a`i`#){
		return xnew<T>(#REPEAT_COMMA#a`i`#);
	}
};
*/

template<class R , R (*fun)()>
struct cfun_holder_static<R (*)(), fun>{
	enum{ PARAMS = 0, PARAMS2 = 0 };
	typedef R (*fun_t)();
	
	typedef ReturnResult Result;
	R operator()(){ 
		return (*fun)(); 
	}
};

template<class R >
struct cfun_holder<R (*)()>{
	enum{ PARAMS = 0, PARAMS2 = 0 };
	typedef R (*fun_t)();
	
	typedef ReturnResult Result;
	fun_t fun;
	cfun_holder(const fun_t& f):fun(f){}
	R operator()(){ 
		return (*fun)(); 
	}
};

#ifdef _WIN32
template<class R >
struct cfun_holder<R (__stdcall *)()>{
	enum{ PARAMS = 0, PARAMS2 = 0 };
	typedef R (__stdcall *fun_t)();
	
	typedef ReturnResult Result;
	fun_t fun;
	cfun_holder(const fun_t& f):fun(f){}
	R operator()(){ 
		return (*fun)(); 
	}
};
#endif

template<class C, class R , R (C::*fun)()>
struct cmemfun_holder_static<R (C::*)(), fun>{
	enum{ PARAMS = 1, PARAMS2 = 0 };
	typedef R (C::*fun_t)();
	typedef arg_this_getter<C*> ARG0;
	
	typedef ReturnResult Result;
	R operator()(C* self ){ 
		return (self->*fun)(); 
	}
};

template<class C, class R >
struct cmemfun_holder<R (C::*)()>{
	enum{ PARAMS = 1, PARAMS2 = 0 };
	typedef R (C::*fun_t)();
	typedef arg_this_getter<C*> ARG0;
	
	typedef ReturnResult Result;
	fun_t fun;
	cmemfun_holder(const fun_t& f):fun(f){}
	R operator()(C* self ){ 
		return (self->*fun)(); 
	}
};

template<class C, class R >
struct cmemfun_holder<R (C::*)() const>{
	enum{ PARAMS = 1, PARAMS2 = 0 };
	typedef R (C::*fun_t)() const;
	typedef arg_this_getter<C*> ARG0;
	
	typedef ReturnResult Result;
	fun_t fun;
	cmemfun_holder(const fun_t& f):fun(f){}
	R operator()(C* self ){ 
		return (self->*fun)(); 
	}
};

template<class C, class R , R (*fun)()>
struct cmemfun_holder_static<R (*)(C ), fun>{
	enum{ PARAMS = 1, PARAMS2 = 0 };
	typedef R (*fun_t)();
	typedef arg_this_getter<C> ARG0;
	
	typedef ReturnResult Result;
	R operator()(C c ){ 
		return (*fun)(c ); 
	}
};

template<class C, class R >
struct cmemfun_holder<R (*)(C )>{
	enum{ PARAMS = 1, PARAMS2 = 0 };
	typedef R (*fun_t)(C );
	typedef arg_this_getter<C> ARG0;
	
	typedef ReturnResult Result;
	fun_t fun;
	cmemfun_holder(const fun_t& f):fun(f){}
	R operator()(C c ){ 
		return (*fun)(c ); 
	}
};

#ifdef _WIN32
template<class C, class R >
struct cmemfun_holder<R (__stdcall *)(C )>{
	enum{ PARAMS = 1, PARAMS2 = 0 };
	typedef R (__stdcall *fun_t)(C );
	typedef arg_this_getter<C> ARG0;
	
	typedef ReturnResult Result;
	fun_t fun;
	cmemfun_holder(const fun_t& f):fun(f){}
	R operator()(C c ){ 
		return (*fun)(c ); 
	}
};
#endif

template<class T >
struct ctor_fun<T >{
	enum{ PARAMS = 0, PARAMS2 = 0 };
	
	typedef ReturnResult Result;
	SmartPtr<T> operator()(){
		return xnew<T>();
	}
};

template<class R , class A0, R (*fun)(A0)>
struct cfun_holder_static<R (*)(A0), fun>{
	enum{ PARAMS = 1, PARAMS2 = 1 };
	typedef R (*fun_t)(A0);
	typedef arg_getter<A0, 0> ARG0;
	typedef ReturnResult Result;
	R operator()(A0 a0){ 
		return (*fun)(a0); 
	}
};

template<class R , class A0>
struct cfun_holder<R (*)(A0)>{
	enum{ PARAMS = 1, PARAMS2 = 1 };
	typedef R (*fun_t)(A0);
	typedef arg_getter<A0, 0> ARG0;
	typedef ReturnResult Result;
	fun_t fun;
	cfun_holder(const fun_t& f):fun(f){}
	R operator()(A0 a0){ 
		return (*fun)(a0); 
	}
};

#ifdef _WIN32
template<class R , class A0>
struct cfun_holder<R (__stdcall *)(A0)>{
	enum{ PARAMS = 1, PARAMS2 = 1 };
	typedef R (__stdcall *fun_t)(A0);
	typedef arg_getter<A0, 0> ARG0;
	typedef ReturnResult Result;
	fun_t fun;
	cfun_holder(const fun_t& f):fun(f){}
	R operator()(A0 a0){ 
		return (*fun)(a0); 
	}
};
#endif

template<class C, class R , class A0, R (C::*fun)(A0)>
struct cmemfun_holder_static<R (C::*)(A0), fun>{
	enum{ PARAMS = 2, PARAMS2 = 1 };
	typedef R (C::*fun_t)(A0);
	typedef arg_this_getter<C*> ARG0;
	typedef arg_getter<A0, 0> ARG1;
	typedef ReturnResult Result;
	R operator()(C* self , A0 a0){ 
		return (self->*fun)(a0); 
	}
};

template<class C, class R , class A0>
struct cmemfun_holder<R (C::*)(A0)>{
	enum{ PARAMS = 2, PARAMS2 = 1 };
	typedef R (C::*fun_t)(A0);
	typedef arg_this_getter<C*> ARG0;
	typedef arg_getter<A0, 0> ARG1;
	typedef ReturnResult Result;
	fun_t fun;
	cmemfun_holder(const fun_t& f):fun(f){}
	R operator()(C* self , A0 a0){ 
		return (self->*fun)(a0); 
	}
};

template<class C, class R , class A0>
struct cmemfun_holder<R (C::*)(A0) const>{
	enum{ PARAMS = 2, PARAMS2 = 1 };
	typedef R (C::*fun_t)(A0) const;
	typedef arg_this_getter<C*> ARG0;
	typedef arg_getter<A0, 0> ARG1;
	typedef ReturnResult Result;
	fun_t fun;
	cmemfun_holder(const fun_t& f):fun(f){}
	R operator()(C* self , A0 a0){ 
		return (self->*fun)(a0); 
	}
};

template<class C, class R , class A0, R (*fun)(A0)>
struct cmemfun_holder_static<R (*)(C , A0), fun>{
	enum{ PARAMS = 2, PARAMS2 = 1 };
	typedef R (*fun_t)(A0);
	typedef arg_this_getter<C> ARG0;
	typedef arg_getter<A0, 0> ARG1;
	typedef ReturnResult Result;
	R operator()(C c , A0 a0){ 
		return (*fun)(c , a0); 
	}
};

template<class C, class R , class A0>
struct cmemfun_holder<R (*)(C , A0)>{
	enum{ PARAMS = 2, PARAMS2 = 1 };
	typedef R (*fun_t)(C , A0);
	typedef arg_this_getter<C> ARG0;
	typedef arg_getter<A0, 0> ARG1;
	typedef ReturnResult Result;
	fun_t fun;
	cmemfun_holder(const fun_t& f):fun(f){}
	R operator()(C c , A0 a0){ 
		return (*fun)(c , a0); 
	}
};

#ifdef _WIN32
template<class C, class R , class A0>
struct cmemfun_holder<R (__stdcall *)(C , A0)>{
	enum{ PARAMS = 2, PARAMS2 = 1 };
	typedef R (__stdcall *fun_t)(C , A0);
	typedef arg_this_getter<C> ARG0;
	typedef arg_getter<A0, 0> ARG1;
	typedef ReturnResult Result;
	fun_t fun;
	cmemfun_holder(const fun_t& f):fun(f){}
	R operator()(C c , A0 a0){ 
		return (*fun)(c , a0); 
	}
};
#endif

template<class T , class A0>
struct ctor_fun<T , A0>{
	enum{ PARAMS = 1, PARAMS2 = 1 };
	typedef arg_getter<A0, 0> ARG0;
	typedef ReturnResult Result;
	SmartPtr<T> operator()(A0 a0){
		return xnew<T>(a0);
	}
};

template<class R , class A0, class A1, R (*fun)(A0, A1)>
struct cfun_holder_static<R (*)(A0, A1), fun>{
	enum{ PARAMS = 2, PARAMS2 = 2 };
	typedef R (*fun_t)(A0, A1);
	typedef arg_getter<A0, 0> ARG0;typedef arg_getter<A1, 1> ARG1;
	typedef ReturnResult Result;
	R operator()(A0 a0, A1 a1){ 
		return (*fun)(a0, a1); 
	}
};

template<class R , class A0, class A1>
struct cfun_holder<R (*)(A0, A1)>{
	enum{ PARAMS = 2, PARAMS2 = 2 };
	typedef R (*fun_t)(A0, A1);
	typedef arg_getter<A0, 0> ARG0;typedef arg_getter<A1, 1> ARG1;
	typedef ReturnResult Result;
	fun_t fun;
	cfun_holder(const fun_t& f):fun(f){}
	R operator()(A0 a0, A1 a1){ 
		return (*fun)(a0, a1); 
	}
};

#ifdef _WIN32
template<class R , class A0, class A1>
struct cfun_holder<R (__stdcall *)(A0, A1)>{
	enum{ PARAMS = 2, PARAMS2 = 2 };
	typedef R (__stdcall *fun_t)(A0, A1);
	typedef arg_getter<A0, 0> ARG0;typedef arg_getter<A1, 1> ARG1;
	typedef ReturnResult Result;
	fun_t fun;
	cfun_holder(const fun_t& f):fun(f){}
	R operator()(A0 a0, A1 a1){ 
		return (*fun)(a0, a1); 
	}
};
#endif

template<class C, class R , class A0, class A1, R (C::*fun)(A0, A1)>
struct cmemfun_holder_static<R (C::*)(A0, A1), fun>{
	enum{ PARAMS = 3, PARAMS2 = 2 };
	typedef R (C::*fun_t)(A0, A1);
	typedef arg_this_getter<C*> ARG0;
	typedef arg_getter<A0, 0> ARG1;typedef arg_getter<A1, 1> ARG2;
	typedef ReturnResult Result;
	R operator()(C* self , A0 a0, A1 a1){ 
		return (self->*fun)(a0, a1); 
	}
};

template<class C, class R , class A0, class A1>
struct cmemfun_holder<R (C::*)(A0, A1)>{
	enum{ PARAMS = 3, PARAMS2 = 2 };
	typedef R (C::*fun_t)(A0, A1);
	typedef arg_this_getter<C*> ARG0;
	typedef arg_getter<A0, 0> ARG1;typedef arg_getter<A1, 1> ARG2;
	typedef ReturnResult Result;
	fun_t fun;
	cmemfun_holder(const fun_t& f):fun(f){}
	R operator()(C* self , A0 a0, A1 a1){ 
		return (self->*fun)(a0, a1); 
	}
};

template<class C, class R , class A0, class A1>
struct cmemfun_holder<R (C::*)(A0, A1) const>{
	enum{ PARAMS = 3, PARAMS2 = 2 };
	typedef R (C::*fun_t)(A0, A1) const;
	typedef arg_this_getter<C*> ARG0;
	typedef arg_getter<A0, 0> ARG1;typedef arg_getter<A1, 1> ARG2;
	typedef ReturnResult Result;
	fun_t fun;
	cmemfun_holder(const fun_t& f):fun(f){}
	R operator()(C* self , A0 a0, A1 a1){ 
		return (self->*fun)(a0, a1); 
	}
};

template<class C, class R , class A0, class A1, R (*fun)(A0, A1)>
struct cmemfun_holder_static<R (*)(C , A0, A1), fun>{
	enum{ PARAMS = 3, PARAMS2 = 2 };
	typedef R (*fun_t)(A0, A1);
	typedef arg_this_getter<C> ARG0;
	typedef arg_getter<A0, 0> ARG1;typedef arg_getter<A1, 1> ARG2;
	typedef ReturnResult Result;
	R operator()(C c , A0 a0, A1 a1){ 
		return (*fun)(c , a0, a1); 
	}
};

template<class C, class R , class A0, class A1>
struct cmemfun_holder<R (*)(C , A0, A1)>{
	enum{ PARAMS = 3, PARAMS2 = 2 };
	typedef R (*fun_t)(C , A0, A1);
	typedef arg_this_getter<C> ARG0;
	typedef arg_getter<A0, 0> ARG1;typedef arg_getter<A1, 1> ARG2;
	typedef ReturnResult Result;
	fun_t fun;
	cmemfun_holder(const fun_t& f):fun(f){}
	R operator()(C c , A0 a0, A1 a1){ 
		return (*fun)(c , a0, a1); 
	}
};

#ifdef _WIN32
template<class C, class R , class A0, class A1>
struct cmemfun_holder<R (__stdcall *)(C , A0, A1)>{
	enum{ PARAMS = 3, PARAMS2 = 2 };
	typedef R (__stdcall *fun_t)(C , A0, A1);
	typedef arg_this_getter<C> ARG0;
	typedef arg_getter<A0, 0> ARG1;typedef arg_getter<A1, 1> ARG2;
	typedef ReturnResult Result;
	fun_t fun;
	cmemfun_holder(const fun_t& f):fun(f){}
	R operator()(C c , A0 a0, A1 a1){ 
		return (*fun)(c , a0, a1); 
	}
};
#endif

template<class T , class A0, class A1>
struct ctor_fun<T , A0, A1>{
	enum{ PARAMS = 2, PARAMS2 = 2 };
	typedef arg_getter<A0, 0> ARG0;typedef arg_getter<A1, 1> ARG1;
	typedef ReturnResult Result;
	SmartPtr<T> operator()(A0 a0, A1 a1){
		return xnew<T>(a0, a1);
	}
};

template<class R , class A0, class A1, class A2, R (*fun)(A0, A1, A2)>
struct cfun_holder_static<R (*)(A0, A1, A2), fun>{
	enum{ PARAMS = 3, PARAMS2 = 3 };
	typedef R (*fun_t)(A0, A1, A2);
	typedef arg_getter<A0, 0> ARG0;typedef arg_getter<A1, 1> ARG1;typedef arg_getter<A2, 2> ARG2;
	typedef ReturnResult Result;
	R operator()(A0 a0, A1 a1, A2 a2){ 
		return (*fun)(a0, a1, a2); 
	}
};

template<class R , class A0, class A1, class A2>
struct cfun_holder<R (*)(A0, A1, A2)>{
	enum{ PARAMS = 3, PARAMS2 = 3 };
	typedef R (*fun_t)(A0, A1, A2);
	typedef arg_getter<A0, 0> ARG0;typedef arg_getter<A1, 1> ARG1;typedef arg_getter<A2, 2> ARG2;
	typedef ReturnResult Result;
	fun_t fun;
	cfun_holder(const fun_t& f):fun(f){}
	R operator()(A0 a0, A1 a1, A2 a2){ 
		return (*fun)(a0, a1, a2); 
	}
};

#ifdef _WIN32
template<class R , class A0, class A1, class A2>
struct cfun_holder<R (__stdcall *)(A0, A1, A2)>{
	enum{ PARAMS = 3, PARAMS2 = 3 };
	typedef R (__stdcall *fun_t)(A0, A1, A2);
	typedef arg_getter<A0, 0> ARG0;typedef arg_getter<A1, 1> ARG1;typedef arg_getter<A2, 2> ARG2;
	typedef ReturnResult Result;
	fun_t fun;
	cfun_holder(const fun_t& f):fun(f){}
	R operator()(A0 a0, A1 a1, A2 a2){ 
		return (*fun)(a0, a1, a2); 
	}
};
#endif

template<class C, class R , class A0, class A1, class A2, R (C::*fun)(A0, A1, A2)>
struct cmemfun_holder_static<R (C::*)(A0, A1, A2), fun>{
	enum{ PARAMS = 4, PARAMS2 = 3 };
	typedef R (C::*fun_t)(A0, A1, A2);
	typedef arg_this_getter<C*> ARG0;
	typedef arg_getter<A0, 0> ARG1;typedef arg_getter<A1, 1> ARG2;typedef arg_getter<A2, 2> ARG3;
	typedef ReturnResult Result;
	R operator()(C* self , A0 a0, A1 a1, A2 a2){ 
		return (self->*fun)(a0, a1, a2); 
	}
};

template<class C, class R , class A0, class A1, class A2>
struct cmemfun_holder<R (C::*)(A0, A1, A2)>{
	enum{ PARAMS = 4, PARAMS2 = 3 };
	typedef R (C::*fun_t)(A0, A1, A2);
	typedef arg_this_getter<C*> ARG0;
	typedef arg_getter<A0, 0> ARG1;typedef arg_getter<A1, 1> ARG2;typedef arg_getter<A2, 2> ARG3;
	typedef ReturnResult Result;
	fun_t fun;
	cmemfun_holder(const fun_t& f):fun(f){}
	R operator()(C* self , A0 a0, A1 a1, A2 a2){ 
		return (self->*fun)(a0, a1, a2); 
	}
};

template<class C, class R , class A0, class A1, class A2>
struct cmemfun_holder<R (C::*)(A0, A1, A2) const>{
	enum{ PARAMS = 4, PARAMS2 = 3 };
	typedef R (C::*fun_t)(A0, A1, A2) const;
	typedef arg_this_getter<C*> ARG0;
	typedef arg_getter<A0, 0> ARG1;typedef arg_getter<A1, 1> ARG2;typedef arg_getter<A2, 2> ARG3;
	typedef ReturnResult Result;
	fun_t fun;
	cmemfun_holder(const fun_t& f):fun(f){}
	R operator()(C* self , A0 a0, A1 a1, A2 a2){ 
		return (self->*fun)(a0, a1, a2); 
	}
};

template<class C, class R , class A0, class A1, class A2, R (*fun)(A0, A1, A2)>
struct cmemfun_holder_static<R (*)(C , A0, A1, A2), fun>{
	enum{ PARAMS = 4, PARAMS2 = 3 };
	typedef R (*fun_t)(A0, A1, A2);
	typedef arg_this_getter<C> ARG0;
	typedef arg_getter<A0, 0> ARG1;typedef arg_getter<A1, 1> ARG2;typedef arg_getter<A2, 2> ARG3;
	typedef ReturnResult Result;
	R operator()(C c , A0 a0, A1 a1, A2 a2){ 
		return (*fun)(c , a0, a1, a2); 
	}
};

template<class C, class R , class A0, class A1, class A2>
struct cmemfun_holder<R (*)(C , A0, A1, A2)>{
	enum{ PARAMS = 4, PARAMS2 = 3 };
	typedef R (*fun_t)(C , A0, A1, A2);
	typedef arg_this_getter<C> ARG0;
	typedef arg_getter<A0, 0> ARG1;typedef arg_getter<A1, 1> ARG2;typedef arg_getter<A2, 2> ARG3;
	typedef ReturnResult Result;
	fun_t fun;
	cmemfun_holder(const fun_t& f):fun(f){}
	R operator()(C c , A0 a0, A1 a1, A2 a2){ 
		return (*fun)(c , a0, a1, a2); 
	}
};

#ifdef _WIN32
template<class C, class R , class A0, class A1, class A2>
struct cmemfun_holder<R (__stdcall *)(C , A0, A1, A2)>{
	enum{ PARAMS = 4, PARAMS2 = 3 };
	typedef R (__stdcall *fun_t)(C , A0, A1, A2);
	typedef arg_this_getter<C> ARG0;
	typedef arg_getter<A0, 0> ARG1;typedef arg_getter<A1, 1> ARG2;typedef arg_getter<A2, 2> ARG3;
	typedef ReturnResult Result;
	fun_t fun;
	cmemfun_holder(const fun_t& f):fun(f){}
	R operator()(C c , A0 a0, A1 a1, A2 a2){ 
		return (*fun)(c , a0, a1, a2); 
	}
};
#endif

template<class T , class A0, class A1, class A2>
struct ctor_fun<T , A0, A1, A2>{
	enum{ PARAMS = 3, PARAMS2 = 3 };
	typedef arg_getter<A0, 0> ARG0;typedef arg_getter<A1, 1> ARG1;typedef arg_getter<A2, 2> ARG2;
	typedef ReturnResult Result;
	SmartPtr<T> operator()(A0 a0, A1 a1, A2 a2){
		return xnew<T>(a0, a1, a2);
	}
};

template<class R , class A0, class A1, class A2, class A3, R (*fun)(A0, A1, A2, A3)>
struct cfun_holder_static<R (*)(A0, A1, A2, A3), fun>{
	enum{ PARAMS = 4, PARAMS2 = 4 };
	typedef R (*fun_t)(A0, A1, A2, A3);
	typedef arg_getter<A0, 0> ARG0;typedef arg_getter<A1, 1> ARG1;typedef arg_getter<A2, 2> ARG2;typedef arg_getter<A3, 3> ARG3;
	typedef ReturnResult Result;
	R operator()(A0 a0, A1 a1, A2 a2, A3 a3){ 
		return (*fun)(a0, a1, a2, a3); 
	}
};

template<class R , class A0, class A1, class A2, class A3>
struct cfun_holder<R (*)(A0, A1, A2, A3)>{
	enum{ PARAMS = 4, PARAMS2 = 4 };
	typedef R (*fun_t)(A0, A1, A2, A3);
	typedef arg_getter<A0, 0> ARG0;typedef arg_getter<A1, 1> ARG1;typedef arg_getter<A2, 2> ARG2;typedef arg_getter<A3, 3> ARG3;
	typedef ReturnResult Result;
	fun_t fun;
	cfun_holder(const fun_t& f):fun(f){}
	R operator()(A0 a0, A1 a1, A2 a2, A3 a3){ 
		return (*fun)(a0, a1, a2, a3); 
	}
};

#ifdef _WIN32
template<class R , class A0, class A1, class A2, class A3>
struct cfun_holder<R (__stdcall *)(A0, A1, A2, A3)>{
	enum{ PARAMS = 4, PARAMS2 = 4 };
	typedef R (__stdcall *fun_t)(A0, A1, A2, A3);
	typedef arg_getter<A0, 0> ARG0;typedef arg_getter<A1, 1> ARG1;typedef arg_getter<A2, 2> ARG2;typedef arg_getter<A3, 3> ARG3;
	typedef ReturnResult Result;
	fun_t fun;
	cfun_holder(const fun_t& f):fun(f){}
	R operator()(A0 a0, A1 a1, A2 a2, A3 a3){ 
		return (*fun)(a0, a1, a2, a3); 
	}
};
#endif

template<class C, class R , class A0, class A1, class A2, class A3, R (C::*fun)(A0, A1, A2, A3)>
struct cmemfun_holder_static<R (C::*)(A0, A1, A2, A3), fun>{
	enum{ PARAMS = 5, PARAMS2 = 4 };
	typedef R (C::*fun_t)(A0, A1, A2, A3);
	typedef arg_this_getter<C*> ARG0;
	typedef arg_getter<A0, 0> ARG1;typedef arg_getter<A1, 1> ARG2;typedef arg_getter<A2, 2> ARG3;typedef arg_getter<A3, 3> ARG4;
	typedef ReturnResult Result;
	R operator()(C* self , A0 a0, A1 a1, A2 a2, A3 a3){ 
		return (self->*fun)(a0, a1, a2, a3); 
	}
};

template<class C, class R , class A0, class A1, class A2, class A3>
struct cmemfun_holder<R (C::*)(A0, A1, A2, A3)>{
	enum{ PARAMS = 5, PARAMS2 = 4 };
	typedef R (C::*fun_t)(A0, A1, A2, A3);
	typedef arg_this_getter<C*> ARG0;
	typedef arg_getter<A0, 0> ARG1;typedef arg_getter<A1, 1> ARG2;typedef arg_getter<A2, 2> ARG3;typedef arg_getter<A3, 3> ARG4;
	typedef ReturnResult Result;
	fun_t fun;
	cmemfun_holder(const fun_t& f):fun(f){}
	R operator()(C* self , A0 a0, A1 a1, A2 a2, A3 a3){ 
		return (self->*fun)(a0, a1, a2, a3); 
	}
};

template<class C, class R , class A0, class A1, class A2, class A3>
struct cmemfun_holder<R (C::*)(A0, A1, A2, A3) const>{
	enum{ PARAMS = 5, PARAMS2 = 4 };
	typedef R (C::*fun_t)(A0, A1, A2, A3) const;
	typedef arg_this_getter<C*> ARG0;
	typedef arg_getter<A0, 0> ARG1;typedef arg_getter<A1, 1> ARG2;typedef arg_getter<A2, 2> ARG3;typedef arg_getter<A3, 3> ARG4;
	typedef ReturnResult Result;
	fun_t fun;
	cmemfun_holder(const fun_t& f):fun(f){}
	R operator()(C* self , A0 a0, A1 a1, A2 a2, A3 a3){ 
		return (self->*fun)(a0, a1, a2, a3); 
	}
};

template<class C, class R , class A0, class A1, class A2, class A3, R (*fun)(A0, A1, A2, A3)>
struct cmemfun_holder_static<R (*)(C , A0, A1, A2, A3), fun>{
	enum{ PARAMS = 5, PARAMS2 = 4 };
	typedef R (*fun_t)(A0, A1, A2, A3);
	typedef arg_this_getter<C> ARG0;
	typedef arg_getter<A0, 0> ARG1;typedef arg_getter<A1, 1> ARG2;typedef arg_getter<A2, 2> ARG3;typedef arg_getter<A3, 3> ARG4;
	typedef ReturnResult Result;
	R operator()(C c , A0 a0, A1 a1, A2 a2, A3 a3){ 
		return (*fun)(c , a0, a1, a2, a3); 
	}
};

template<class C, class R , class A0, class A1, class A2, class A3>
struct cmemfun_holder<R (*)(C , A0, A1, A2, A3)>{
	enum{ PARAMS = 5, PARAMS2 = 4 };
	typedef R (*fun_t)(C , A0, A1, A2, A3);
	typedef arg_this_getter<C> ARG0;
	typedef arg_getter<A0, 0> ARG1;typedef arg_getter<A1, 1> ARG2;typedef arg_getter<A2, 2> ARG3;typedef arg_getter<A3, 3> ARG4;
	typedef ReturnResult Result;
	fun_t fun;
	cmemfun_holder(const fun_t& f):fun(f){}
	R operator()(C c , A0 a0, A1 a1, A2 a2, A3 a3){ 
		return (*fun)(c , a0, a1, a2, a3); 
	}
};

#ifdef _WIN32
template<class C, class R , class A0, class A1, class A2, class A3>
struct cmemfun_holder<R (__stdcall *)(C , A0, A1, A2, A3)>{
	enum{ PARAMS = 5, PARAMS2 = 4 };
	typedef R (__stdcall *fun_t)(C , A0, A1, A2, A3);
	typedef arg_this_getter<C> ARG0;
	typedef arg_getter<A0, 0> ARG1;typedef arg_getter<A1, 1> ARG2;typedef arg_getter<A2, 2> ARG3;typedef arg_getter<A3, 3> ARG4;
	typedef ReturnResult Result;
	fun_t fun;
	cmemfun_holder(const fun_t& f):fun(f){}
	R operator()(C c , A0 a0, A1 a1, A2 a2, A3 a3){ 
		return (*fun)(c , a0, a1, a2, a3); 
	}
};
#endif

template<class T , class A0, class A1, class A2, class A3>
struct ctor_fun<T , A0, A1, A2, A3>{
	enum{ PARAMS = 4, PARAMS2 = 4 };
	typedef arg_getter<A0, 0> ARG0;typedef arg_getter<A1, 1> ARG1;typedef arg_getter<A2, 2> ARG2;typedef arg_getter<A3, 3> ARG3;
	typedef ReturnResult Result;
	SmartPtr<T> operator()(A0 a0, A1 a1, A2 a2, A3 a3){
		return xnew<T>(a0, a1, a2, a3);
	}
};

template<class R , class A0, class A1, class A2, class A3, class A4, R (*fun)(A0, A1, A2, A3, A4)>
struct cfun_holder_static<R (*)(A0, A1, A2, A3, A4), fun>{
	enum{ PARAMS = 5, PARAMS2 = 5 };
	typedef R (*fun_t)(A0, A1, A2, A3, A4);
	typedef arg_getter<A0, 0> ARG0;typedef arg_getter<A1, 1> ARG1;typedef arg_getter<A2, 2> ARG2;typedef arg_getter<A3, 3> ARG3;typedef arg_getter<A4, 4> ARG4;
	typedef ReturnResult Result;
	R operator()(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4){ 
		return (*fun)(a0, a1, a2, a3, a4); 
	}
};

template<class R , class A0, class A1, class A2, class A3, class A4>
struct cfun_holder<R (*)(A0, A1, A2, A3, A4)>{
	enum{ PARAMS = 5, PARAMS2 = 5 };
	typedef R (*fun_t)(A0, A1, A2, A3, A4);
	typedef arg_getter<A0, 0> ARG0;typedef arg_getter<A1, 1> ARG1;typedef arg_getter<A2, 2> ARG2;typedef arg_getter<A3, 3> ARG3;typedef arg_getter<A4, 4> ARG4;
	typedef ReturnResult Result;
	fun_t fun;
	cfun_holder(const fun_t& f):fun(f){}
	R operator()(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4){ 
		return (*fun)(a0, a1, a2, a3, a4); 
	}
};

#ifdef _WIN32
template<class R , class A0, class A1, class A2, class A3, class A4>
struct cfun_holder<R (__stdcall *)(A0, A1, A2, A3, A4)>{
	enum{ PARAMS = 5, PARAMS2 = 5 };
	typedef R (__stdcall *fun_t)(A0, A1, A2, A3, A4);
	typedef arg_getter<A0, 0> ARG0;typedef arg_getter<A1, 1> ARG1;typedef arg_getter<A2, 2> ARG2;typedef arg_getter<A3, 3> ARG3;typedef arg_getter<A4, 4> ARG4;
	typedef ReturnResult Result;
	fun_t fun;
	cfun_holder(const fun_t& f):fun(f){}
	R operator()(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4){ 
		return (*fun)(a0, a1, a2, a3, a4); 
	}
};
#endif

template<class C, class R , class A0, class A1, class A2, class A3, class A4, R (C::*fun)(A0, A1, A2, A3, A4)>
struct cmemfun_holder_static<R (C::*)(A0, A1, A2, A3, A4), fun>{
	enum{ PARAMS = 6, PARAMS2 = 5 };
	typedef R (C::*fun_t)(A0, A1, A2, A3, A4);
	typedef arg_this_getter<C*> ARG0;
	typedef arg_getter<A0, 0> ARG1;typedef arg_getter<A1, 1> ARG2;typedef arg_getter<A2, 2> ARG3;typedef arg_getter<A3, 3> ARG4;typedef arg_getter<A4, 4> ARG5;
	typedef ReturnResult Result;
	R operator()(C* self , A0 a0, A1 a1, A2 a2, A3 a3, A4 a4){ 
		return (self->*fun)(a0, a1, a2, a3, a4); 
	}
};

template<class C, class R , class A0, class A1, class A2, class A3, class A4>
struct cmemfun_holder<R (C::*)(A0, A1, A2, A3, A4)>{
	enum{ PARAMS = 6, PARAMS2 = 5 };
	typedef R (C::*fun_t)(A0, A1, A2, A3, A4);
	typedef arg_this_getter<C*> ARG0;
	typedef arg_getter<A0, 0> ARG1;typedef arg_getter<A1, 1> ARG2;typedef arg_getter<A2, 2> ARG3;typedef arg_getter<A3, 3> ARG4;typedef arg_getter<A4, 4> ARG5;
	typedef ReturnResult Result;
	fun_t fun;
	cmemfun_holder(const fun_t& f):fun(f){}
	R operator()(C* self , A0 a0, A1 a1, A2 a2, A3 a3, A4 a4){ 
		return (self->*fun)(a0, a1, a2, a3, a4); 
	}
};

template<class C, class R , class A0, class A1, class A2, class A3, class A4>
struct cmemfun_holder<R (C::*)(A0, A1, A2, A3, A4) const>{
	enum{ PARAMS = 6, PARAMS2 = 5 };
	typedef R (C::*fun_t)(A0, A1, A2, A3, A4) const;
	typedef arg_this_getter<C*> ARG0;
	typedef arg_getter<A0, 0> ARG1;typedef arg_getter<A1, 1> ARG2;typedef arg_getter<A2, 2> ARG3;typedef arg_getter<A3, 3> ARG4;typedef arg_getter<A4, 4> ARG5;
	typedef ReturnResult Result;
	fun_t fun;
	cmemfun_holder(const fun_t& f):fun(f){}
	R operator()(C* self , A0 a0, A1 a1, A2 a2, A3 a3, A4 a4){ 
		return (self->*fun)(a0, a1, a2, a3, a4); 
	}
};

template<class C, class R , class A0, class A1, class A2, class A3, class A4, R (*fun)(A0, A1, A2, A3, A4)>
struct cmemfun_holder_static<R (*)(C , A0, A1, A2, A3, A4), fun>{
	enum{ PARAMS = 6, PARAMS2 = 5 };
	typedef R (*fun_t)(A0, A1, A2, A3, A4);
	typedef arg_this_getter<C> ARG0;
	typedef arg_getter<A0, 0> ARG1;typedef arg_getter<A1, 1> ARG2;typedef arg_getter<A2, 2> ARG3;typedef arg_getter<A3, 3> ARG4;typedef arg_getter<A4, 4> ARG5;
	typedef ReturnResult Result;
	R operator()(C c , A0 a0, A1 a1, A2 a2, A3 a3, A4 a4){ 
		return (*fun)(c , a0, a1, a2, a3, a4); 
	}
};

template<class C, class R , class A0, class A1, class A2, class A3, class A4>
struct cmemfun_holder<R (*)(C , A0, A1, A2, A3, A4)>{
	enum{ PARAMS = 6, PARAMS2 = 5 };
	typedef R (*fun_t)(C , A0, A1, A2, A3, A4);
	typedef arg_this_getter<C> ARG0;
	typedef arg_getter<A0, 0> ARG1;typedef arg_getter<A1, 1> ARG2;typedef arg_getter<A2, 2> ARG3;typedef arg_getter<A3, 3> ARG4;typedef arg_getter<A4, 4> ARG5;
	typedef ReturnResult Result;
	fun_t fun;
	cmemfun_holder(const fun_t& f):fun(f){}
	R operator()(C c , A0 a0, A1 a1, A2 a2, A3 a3, A4 a4){ 
		return (*fun)(c , a0, a1, a2, a3, a4); 
	}
};

#ifdef _WIN32
template<class C, class R , class A0, class A1, class A2, class A3, class A4>
struct cmemfun_holder<R (__stdcall *)(C , A0, A1, A2, A3, A4)>{
	enum{ PARAMS = 6, PARAMS2 = 5 };
	typedef R (__stdcall *fun_t)(C , A0, A1, A2, A3, A4);
	typedef arg_this_getter<C> ARG0;
	typedef arg_getter<A0, 0> ARG1;typedef arg_getter<A1, 1> ARG2;typedef arg_getter<A2, 2> ARG3;typedef arg_getter<A3, 3> ARG4;typedef arg_getter<A4, 4> ARG5;
	typedef ReturnResult Result;
	fun_t fun;
	cmemfun_holder(const fun_t& f):fun(f){}
	R operator()(C c , A0 a0, A1 a1, A2 a2, A3 a3, A4 a4){ 
		return (*fun)(c , a0, a1, a2, a3, a4); 
	}
};
#endif

template<class T , class A0, class A1, class A2, class A3, class A4>
struct ctor_fun<T , A0, A1, A2, A3, A4>{
	enum{ PARAMS = 5, PARAMS2 = 5 };
	typedef arg_getter<A0, 0> ARG0;typedef arg_getter<A1, 1> ARG1;typedef arg_getter<A2, 2> ARG2;typedef arg_getter<A3, 3> ARG3;typedef arg_getter<A4, 4> ARG4;
	typedef ReturnResult Result;
	SmartPtr<T> operator()(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4){
		return xnew<T>(a0, a1, a2, a3, a4);
	}
};

//}}REPEAT}


template<class R, R (*fun)(const VMachinePtr&)>
struct cfun_holder_static<R (*)(const VMachinePtr&), fun>{
	enum{ PARAMS = 1, PARAMS2 = -1 };
	typedef R (*fun_t)(const VMachinePtr&);
	typedef arg_getter_vm ARG0;
	typedef ReturnNone Result;
	R operator()(const VMachinePtr& a0){ 
		return (*fun)(a0); 
	}
};

template<class R>
struct cfun_holder<R (*)(const VMachinePtr&)>{
	enum{ PARAMS = 1, PARAMS2 = -1 };
	typedef R (*fun_t)(const VMachinePtr&);
	typedef arg_getter_vm ARG0; 
	typedef ReturnNone Result;
	fun_t fun;
	cfun_holder(const fun_t& f):fun(f){}
	R operator()(const VMachinePtr& a0){ 
		return (*fun)(a0); 
	}
};

template<class T>
struct ctor_fun<T , const VMachinePtr&>{
	enum{ PARAMS = 1, PARAMS2 = -1 };
	typedef arg_getter_vm ARG0; 
	typedef ReturnNone Result;
	SmartPtr<T> operator()(const VMachinePtr& a0){ 
		return xnew<T>(a0);
	}
};

template<class R, R (*fun)(const VMachinePtr&)>
struct cmemfun_holder_static<R (*)(const VMachinePtr&), fun>{
	enum{ PARAMS = 1, PARAMS2 = -1 };
	typedef R (*fun_t)(const VMachinePtr&);
	typedef arg_getter_vm ARG0;
	typedef ReturnNone Result;
	R operator()(const VMachinePtr& a0){ 
		return (*fun)(a0); 
	}
};

template<class R>
struct cmemfun_holder<R (*)(const VMachinePtr&)>{
	enum{ PARAMS = 1, PARAMS2 = -1 };
	typedef R (*fun_t)(const VMachinePtr&);
	typedef arg_getter_vm ARG0; 
	typedef ReturnNone Result;
	fun_t fun;
	cmemfun_holder(const fun_t& f):fun(f){}
	R operator()(const VMachinePtr& a0){ 
		return (*fun)(a0); 
	}
};

template<class R, class C, R (*fun)(C, const VMachinePtr&)>
struct cmemfun_holder_static<R (*)(C, const VMachinePtr&), fun>{
	enum{ PARAMS = 1, PARAMS2 = -1 };
	typedef R (*fun_t)(C, const VMachinePtr&);
	typedef arg_this_getter<C> ARG0;
	typedef arg_getter_vm ARG1;
	typedef ReturnNone Result;
	R operator()(C self, const VMachinePtr& a0){ 
		return (*fun)(self, a0); 
	}
};

template<class R, class C>
struct cmemfun_holder<R (*)(C, const VMachinePtr&)>{
	enum{ PARAMS = 1, PARAMS2 = -1 };
	typedef R (*fun_t)(C, const VMachinePtr&);
	typedef arg_this_getter<C> ARG0;
	typedef arg_getter_vm ARG1;
	typedef ReturnNone Result;
	fun_t fun;
	cmemfun_holder(const fun_t& f):fun(f){}
	R operator()(C self, const VMachinePtr& a0){ 
		return (*fun)(self, a0); 
	}
};

template<class R, class C, R (C::*fun)(const VMachinePtr&)>
struct cmemfun_holder_static<R (C::*)(const VMachinePtr&), fun>{
	enum{ PARAMS = 2, PARAMS2 = -1 };
	typedef R (C::*fun_t)(const VMachinePtr&);
	typedef arg_this_getter<C*> ARG0;
	typedef arg_getter_vm ARG1; 
	typedef ReturnNone Result;
	R operator()(C* self , const VMachinePtr& a0){ 
		return (self->*fun)(a0); 
	}
};

template<class C>
struct cmemfun_holder<void (C::*)(const VMachinePtr&)>{
	enum{ PARAMS = 2, PARAMS2 = -1 };
	typedef void (C::*fun_t)(const VMachinePtr&);
	typedef arg_this_getter<C*> ARG0;
	typedef arg_getter_vm ARG1; 
	typedef ReturnNone Result;
	fun_t fun;
	cmemfun_holder(const fun_t& f):fun(f){}
	void operator()(C* self , const VMachinePtr& a0){ 
		return (self->*fun)(a0); 
	}
};

template<class C>
struct cmemfun_holder<void (C::*)(const VMachinePtr&) const>{
	enum{ PARAMS = 2, PARAMS2 = -1 };
	typedef void (C::*fun_t)(const VMachinePtr&) const;
	typedef arg_this_getter<C*> ARG0;
	typedef arg_getter_vm ARG1; 
	typedef ReturnNone Result;
	fun_t fun;
	cmemfun_holder(const fun_t& f):fun(f){}
	void operator()(C* self , const VMachinePtr& a0){ 
		return (self->*fun)(a0); 
	}
};

///////////////////////////////////////////////////////

template<class C, class T>
struct getter_holder{
	enum{ PARAMS = 1, PARAMS2 = 0 };
	typedef ReturnResult Result;
	typedef arg_this_getter<C*> ARG0;
	T C::* var;
	getter_holder(T C::* var):var(var){}
	const T& operator()(C* self){ return self->*var; }
};

template<class C, class T>
struct setter_holder{
	enum{ PARAMS = 2, PARAMS2 = 1 };
	typedef ReturnResult Result;
	typedef arg_this_getter<C*> ARG0;
	typedef arg_getter<const T&, 0> ARG1;
	T C::* var;
	setter_holder(T C::* var):var(var){}
	const T& operator()(C* self, const T& v){ return self->*var = v; }
};

///////////////////////////////////////////////////////

template<int N, class Fun>
struct cfun{};

//{REPEAT{{
/*
template<class Fun>
struct cfun<`n`, Fun>{
	static void f(ParamInfoAndVM& pvm){
		#REPEAT#typedef typename Fun::ARG`i` A`i`;#
		#REPEAT#const AnyPtr& a`i` = A`i`::get(pvm);#
		if(pvm.flags==0){
			(*(Fun*)pvm.data)(
				#REPEAT_COMMA#A`i`::cast(a`i`)#
			), ReturnPolicyTest<typename Fun::Result>(pvm.vm), ReturnPolicyVoidTest();
		}
	}
};
*/

template<class Fun>
struct cfun<0, Fun>{
	static void f(ParamInfoAndVM& pvm){
		
		
		if(pvm.flags==0){
			(*(Fun*)pvm.data)(
				
			), ReturnPolicyTest<typename Fun::Result>(pvm.vm), ReturnPolicyVoidTest();
		}
	}
};

template<class Fun>
struct cfun<1, Fun>{
	static void f(ParamInfoAndVM& pvm){
		typedef typename Fun::ARG0 A0;
		const AnyPtr& a0 = A0::get(pvm);
		if(pvm.flags==0){
			(*(Fun*)pvm.data)(
				A0::cast(a0)
			), ReturnPolicyTest<typename Fun::Result>(pvm.vm), ReturnPolicyVoidTest();
		}
	}
};

template<class Fun>
struct cfun<2, Fun>{
	static void f(ParamInfoAndVM& pvm){
		typedef typename Fun::ARG0 A0;typedef typename Fun::ARG1 A1;
		const AnyPtr& a0 = A0::get(pvm);const AnyPtr& a1 = A1::get(pvm);
		if(pvm.flags==0){
			(*(Fun*)pvm.data)(
				A0::cast(a0), A1::cast(a1)
			), ReturnPolicyTest<typename Fun::Result>(pvm.vm), ReturnPolicyVoidTest();
		}
	}
};

template<class Fun>
struct cfun<3, Fun>{
	static void f(ParamInfoAndVM& pvm){
		typedef typename Fun::ARG0 A0;typedef typename Fun::ARG1 A1;typedef typename Fun::ARG2 A2;
		const AnyPtr& a0 = A0::get(pvm);const AnyPtr& a1 = A1::get(pvm);const AnyPtr& a2 = A2::get(pvm);
		if(pvm.flags==0){
			(*(Fun*)pvm.data)(
				A0::cast(a0), A1::cast(a1), A2::cast(a2)
			), ReturnPolicyTest<typename Fun::Result>(pvm.vm), ReturnPolicyVoidTest();
		}
	}
};

template<class Fun>
struct cfun<4, Fun>{
	static void f(ParamInfoAndVM& pvm){
		typedef typename Fun::ARG0 A0;typedef typename Fun::ARG1 A1;typedef typename Fun::ARG2 A2;typedef typename Fun::ARG3 A3;
		const AnyPtr& a0 = A0::get(pvm);const AnyPtr& a1 = A1::get(pvm);const AnyPtr& a2 = A2::get(pvm);const AnyPtr& a3 = A3::get(pvm);
		if(pvm.flags==0){
			(*(Fun*)pvm.data)(
				A0::cast(a0), A1::cast(a1), A2::cast(a2), A3::cast(a3)
			), ReturnPolicyTest<typename Fun::Result>(pvm.vm), ReturnPolicyVoidTest();
		}
	}
};

template<class Fun>
struct cfun<5, Fun>{
	static void f(ParamInfoAndVM& pvm){
		typedef typename Fun::ARG0 A0;typedef typename Fun::ARG1 A1;typedef typename Fun::ARG2 A2;typedef typename Fun::ARG3 A3;typedef typename Fun::ARG4 A4;
		const AnyPtr& a0 = A0::get(pvm);const AnyPtr& a1 = A1::get(pvm);const AnyPtr& a2 = A2::get(pvm);const AnyPtr& a3 = A3::get(pvm);const AnyPtr& a4 = A4::get(pvm);
		if(pvm.flags==0){
			(*(Fun*)pvm.data)(
				A0::cast(a0), A1::cast(a1), A2::cast(a2), A3::cast(a3), A4::cast(a4)
			), ReturnPolicyTest<typename Fun::Result>(pvm.vm), ReturnPolicyVoidTest();
		}
	}
};

//}}REPEAT}

//////////////////////////////////////////////////////////////

class CFun : public HaveName{
public:
	typedef void (*fun_t)(ParamInfoAndVM& pvm);

	CFun(fun_t f, const void* val, int_t val_size, int_t param_n);
	
	virtual ~CFun();

public:

	const CFunPtr& param(
		const Named2& value0 = null_named, 
		const Named2& value1 = null_named,
		const Named2& value2 = null_named,
		const Named2& value3 = null_named,
		const Named2& value4 = null_named,
		const Named2& value5 = null_named,
		const Named2& value6 = null_named,
		const Named2& value7 = null_named,
		const Named2& value8 = null_named,
		const Named2& value9 = null_named,
		const Named2& value10 = null_named
	);

public:
//{REPEAT{{
/*
	const CFunPtr& params(#REPEAT_COMMA#const IDPtr& key`i`, const Any& value`i`#);
*/

	const CFunPtr& params();

	const CFunPtr& params(const IDPtr& key0, const Any& value0);

	const CFunPtr& params(const IDPtr& key0, const Any& value0, const IDPtr& key1, const Any& value1);

	const CFunPtr& params(const IDPtr& key0, const Any& value0, const IDPtr& key1, const Any& value1, const IDPtr& key2, const Any& value2);

	const CFunPtr& params(const IDPtr& key0, const Any& value0, const IDPtr& key1, const Any& value1, const IDPtr& key2, const Any& value2, const IDPtr& key3, const Any& value3);

	const CFunPtr& params(const IDPtr& key0, const Any& value0, const IDPtr& key1, const Any& value1, const IDPtr& key2, const Any& value2, const IDPtr& key3, const Any& value3, const IDPtr& key4, const Any& value4);

//}}REPEAT}
public:

	virtual void visit_members(Visitor& m);

	void check_arg_num(const VMachinePtr& vm);
	void check_args_type(ParamInfoAndVM& pvm);

	virtual void rawcall(const VMachinePtr& vm);

	const CFunPtr& bind_this(const AnyPtr& t){
		this_ = t;
		return from_this(this);
	}

protected:
	void* val_;
	fun_t fun_;
	ParamInfo pi_;
	int_t param_n_;
	AnyPtr this_;
};

CFunPtr new_cfun(void (*fun)(ParamInfoAndVM& pvm), const void* val, int_t val_size, int_t param_n);

//////////////////////////////////////////////////////////////

/**
* @brief C++の関数をXtalから呼び出せるオブジェクトに変換するための関数
*
*/
template<class Fun>
inline CFunPtr fun(const Fun& f){
	typedef cfun_holder<Fun> fun_t;
	fun_t fun(f);
	return new_cfun(&cfun<fun_t::PARAMS, fun_t>::f, &fun, sizeof(fun), fun_t::PARAMS2);
}

/**
* @brief C++の関数をXtalから呼び出せるオブジェクトに変換するための関数
*
*/
template<class Fun, Fun fun_s>
inline CFunPtr fun_static(){
	typedef cfun_holder_static<Fun, fun_s> fun_t;
	fun_t fun;
	return new_cfun(&cfun<fun_t::PARAMS, fun_t>::f, &fun, 0, fun_t::PARAMS2);
}

/**
* @brief C++のメンバ関数をXtalから呼び出せるオブジェクトに変換するための関数
*
* 普通の関数をメソッドとして変換したい場合、第一引数をその型にすること。
*/
template<class Fun>
inline CFunPtr method(const Fun& f){
	typedef cmemfun_holder<Fun> fun_t;
	fun_t fun(f);
	return new_cfun(&cfun<fun_t::PARAMS, fun_t>::f, &fun, sizeof(fun), fun_t::PARAMS2);
}

/**
* @brief C++のメンバ関数をXtalから呼び出せるオブジェクトに変換するための関数
*
* 普通の関数をメソッドとして変換したい場合、第一引数をその型にすること。
*/
template<class Fun, Fun f>
inline CFunPtr method_static(){
	typedef cmemfun_holder_static<Fun, f> fun_t;
	fun_t fun;
	return new_cfun(&cfun<fun_t::PARAMS, fun_t>::f, &fun, 0, fun_t::PARAMS2);
}

/**
* @brief C++のコンストラクタをXtalから呼び出せるオブジェクトに変換するための擬似関数
*
*/
template<class T, class A0=void, class A1=void, class A2=void, class A3=void, class A4=void, class A5=void, class A6=void, class A7=void, class A8=void, class A9=void>
struct ctor : public CFunPtr{
	typedef ctor_fun<T, A0, A1, A2, A3, A4, A5, A6, A7, A8, A9> fun_t;
	ctor():CFunPtr(new_cfun(&cfun<fun_t::PARAMS, fun_t>::f, 0, 0, fun_t::PARAMS2)){}
};

	
/**
* @brief メンバ変数へのポインタからゲッター関数を生成する
*
*/
template<class T, class C>
inline CFunPtr getter(T C::* v){
	typedef getter_holder<C, T> fun_t;
	fun_t fun(v);
	return new_cfun(&cfun<fun_t::PARAMS, fun_t>::f, &fun, sizeof(fun), fun_t::PARAMS2);
}
	
/**
* @brief メンバ変数へのポインタからセッター関数を生成する
*
*/
template<class T, class C>
inline CFunPtr setter(T C::* v){
	typedef setter_holder<C, T> fun_t;
	fun_t fun(v);
	return new_cfun(&cfun<fun_t::PARAMS, fun_t>::f, &fun, sizeof(fun), fun_t::PARAMS2);
}


/**
* @birief 2重ディスパッチメソッド
*/
class DualDispatchMethod : public HaveName{
public:

	DualDispatchMethod(const IDPtr& primary_key)
		:primary_key_(primary_key){}

	virtual void rawcall(const VMachinePtr& vm){
		if(vm->ordered_arg_count()>0){
			vm->get_arg_this()->rawsend(vm, primary_key_, vm->arg(0)->get_class(), vm->get_arg_this());
		}
	}

private:
	IDPtr primary_key_;
};

/**
* @brief 2重ディスパッチメソッドオブジェクトを生成する
*
*/
DualDispatchMethodPtr dual_dispatch_method(const IDPtr& primary_key);

/**
* @birief 2重ディスパッチ関数
*/
class DualDispatchFun : public HaveName{
public:

	DualDispatchFun(const ClassPtr& klass, const IDPtr& primary_key)
		:klass_(klass), primary_key_(primary_key){}

	virtual void rawcall(const VMachinePtr& vm){
		klass_->member(primary_key_, vm->arg(0)->get_class(), vm->get_arg_this())->rawcall(vm);
	}

private:

	virtual void visit_members(Visitor& m){
		HaveName::visit_members(m);
		m & klass_;
	}

	IDPtr primary_key_;
	AnyPtr klass_;
};

/**
* @brief 2重ディスパッチメソッドオブジェクトを生成する
*
*/
DualDispatchFunPtr dual_dispatch_fun(const ClassPtr& klass, const IDPtr& primary_key);

}
