
#pragma once

#include "any.h"
#include "code.h"
#include "vmachine.h"
#include "cast.h"
#include "userdata.h"
#include "string.h"

namespace xtal{

class CFunImpl;	

struct Result{
	static void return_result(const VMachine& vm){
		vm.return_result();
	}

	template<class T>
	static void return_result2(const VMachine& vm, const T& ret, Bool<true>){
		vm.return_result(ret);
	}

	template<class T>
	static void return_result2(const VMachine& vm, const T& ret, Bool<false>){
		vm.return_result(new_userdata<T>(ret));
	}

	template<class T>
	static void return_result(const VMachine& vm, const T& ret){
		return_result2(vm, ret, Bool<Convertible<T, Any>::value>());
	}
};

struct ReturnThis{
	static void return_result(const VMachine& vm){
		vm.return_result(vm.get_arg_this());
	}
	template<class T>
	static void return_result(const VMachine& vm, const T&){
		vm.return_result(vm.get_arg_this());
	}
};

struct ReturnVoid{
	static void return_result(const VMachine& vm){
		vm.return_result();
	}
	template<class T>
	static void return_result(const VMachine& vm, const T&){
		vm.return_result();
	}
};

extern Result result;
extern ReturnThis return_this;
extern ReturnVoid return_void;

struct ParamInfo{
	Named* params;
	int_t min_param_count;
	int_t max_param_count;
	UncountedAny fun;
};

template<class T>
inline typename CastResult<T>::type 
arg_get(const VMachine& vm, const ParamInfo& p, int_t i){
	return arg_cast<T>(vm.arg_default(i, p.params[i].name, p.params[i].value), i, p.params[i].name);
}

void check_arg(const VMachine& vm, const ParamInfo& p);

namespace detail{

template<class T, class R, class Policy>
struct fun0{
	
	static void f(const VMachine& vm, const ParamInfo& p, void* data);
	
	static void f2(const VMachine& vm, const ParamInfo& p, T& fun, ...){
		Policy::return_result(vm, fun(
		));
	}
	
	static void f2(const VMachine& vm, const ParamInfo& p, T& fun, T2T<void>){
		fun(
		);
		Policy::return_result(vm);
	}
};
	
template<class T, class R, class Policy>
void fun0<T, R, Policy>::f(const VMachine& vm, const ParamInfo& p, void* data){
	check_arg(vm, p);
	f2(vm, p, *(T*)data, T2T<R>());
}

template<class T, class R, class A0, class Policy>
struct fun1{
	
	static void f(const VMachine& vm, const ParamInfo& p, void* data);
	
	static void f2(const VMachine& vm, const ParamInfo& p, T& fun, T2T<void>, T2T<const VMachine&>){
		fun(vm);
	}	
		
	static void f2(const VMachine& vm, const ParamInfo& p, T& fun, ...){
		f3(vm, p, fun, T2T<A0>());
	}

	static void f3(const VMachine& vm, const ParamInfo& p, T& fun, T2T<const VMachine&>){
		Policy::return_result(vm, fun(
			vm
		));
	}

	static void f3(const VMachine& vm, const ParamInfo& p, T& fun, ...){
		check_arg(vm, p);
		Policy::return_result(vm, fun(
			arg_get<A0>(vm, p, 0)
		));
	}
	
	static void f2(const VMachine& vm, const ParamInfo& p, T& fun, T2T<void>, ...){
		check_arg(vm, p);
		fun(
			arg_get<A0>(vm, p, 0)
		);
		Policy::return_result(vm);
	}
};

template<class T, class R, class A0, class Policy>
void fun1<T, R, A0, Policy>::f(const VMachine& vm, const ParamInfo& p, void* data){
	f2(vm, p, *(T*)data, T2T<R>(), T2T<A0>());
}
	
template<class T, class R, class A0, class A1, class Policy>
struct fun2{
	
	static void f(const VMachine& vm, const ParamInfo& p, void* data);
	
	static void f2(const VMachine& vm, const ParamInfo& p, T& fun, ...){
		Policy::return_result(vm, fun(
			arg_get<A0>(vm, p, 0), 
			arg_get<A1>(vm, p, 1)
		));
	}
	
	static void f2(const VMachine& vm, const ParamInfo& p, T& fun, T2T<void>){
		fun(
			arg_get<A0>(vm, p, 0), 
			arg_get<A1>(vm, p, 1)
		);
		Policy::return_result(vm);
	}
};

template<class T, class R, class A0, class A1, class Policy>	
void fun2<T, R, A0, A1, Policy>::f(const VMachine& vm, const ParamInfo& p, void* data){
	check_arg(vm, p);
	f2(vm, p, *(T*)data, T2T<R>());
}
	
template<class T, class R, class A0, class A1, class A2, class Policy>
struct fun3{
	
	static void f(const VMachine& vm, const ParamInfo& p, void* data){
		check_arg(vm, p);
		f2(vm, p, *(T*)data, T2T<R>());
	}
	
	static void f2(const VMachine& vm, const ParamInfo& p, T& fun, ...){
		Policy::return_result(vm, fun(
			arg_get<A0>(vm, p, 0), 
			arg_get<A1>(vm, p, 1), 
			arg_get<A2>(vm, p, 2)
		));
	}
	
	static void f2(const VMachine& vm, const ParamInfo& p, T& fun, T2T<void>){
		fun(
			arg_get<A0>(vm, p, 0), 
			arg_get<A1>(vm, p, 1), 
			arg_get<A2>(vm, p, 2)
		);
		Policy::return_result(vm);
	}
};

template<class T, class R, class A0, class A1, class A2, class A3, class Policy>
struct fun4{
	
	static void f(const VMachine& vm, const ParamInfo& p, void* data){
		check_arg(vm, p);
		f2(vm, p, *(T*)data, T2T<R>());
	}
	
	static void f2(const VMachine& vm, const ParamInfo& p, T& fun, ...){
		Policy::return_result(vm, fun(
			arg_get<A0>(vm, p, 0), 
			arg_get<A1>(vm, p, 1), 
			arg_get<A2>(vm, p, 2), 
			arg_get<A3>(vm, p, 3)
		));
	}
	
	static void f2(const VMachine& vm, const ParamInfo& p, T& fun, T2T<void>){
		fun(
			arg_get<A0>(vm, p, 0), 
			arg_get<A1>(vm, p, 1), 
			arg_get<A2>(vm, p, 2), 
			arg_get<A3>(vm, p, 3)
		);
		Policy::return_result(vm);
	}
};

template<class T, class R, class A0, class A1, class A2, class A3, class A4, class Policy>
struct fun5{
	
	static void f(const VMachine& vm, const ParamInfo& p, void* data){
		check_arg(vm, p);
		f2(vm, p, *(T*)data, T2T<R>());
	}
	
	static void f2(const VMachine& vm, const ParamInfo& p, T& fun, ...){
		Policy::return_result(vm, fun(
			arg_get<A0>(vm, p, 0), 
			arg_get<A1>(vm, p, 1), 
			arg_get<A2>(vm, p, 2), 
			arg_get<A3>(vm, p, 3),
			arg_get<A4>(vm, p, 4)
		));
	}
	
	static void f2(const VMachine& vm, const ParamInfo& p, T& fun, T2T<void>){
		fun(
			arg_get<A0>(vm, p, 0), 
			arg_get<A1>(vm, p, 1), 
			arg_get<A2>(vm, p, 2), 
			arg_get<A3>(vm, p, 3),
			arg_get<A4>(vm, p, 4)
		);
		Policy::return_result(vm);
	}
};

template<class T, class C, class R, class Policy>
struct method0{
	
	static void f(const VMachine& vm, const ParamInfo& p, void* data);
	
	static void f2(const VMachine& vm, const ParamInfo& p, T& fun, ...){
		Policy::return_result(vm, fun(
			cast<C>(vm.get_arg_this())
		));
	}
	
	static void f2(const VMachine& vm, const ParamInfo& p, T& fun, T2T<void>){
		fun(
			cast<C>(vm.get_arg_this())
		);
		Policy::return_result(vm);
	}
};
	
template<class T, class C, class R, class Policy>
void method0<T, C, R, Policy>::f(const VMachine& vm, const ParamInfo& p, void* data){
	check_arg(vm, p);
	f2(vm, p, *(T*)data, T2T<R>());
}


template<class T, class C, class R, class A0, class Policy>
struct method1{
	
	static void f(const VMachine& vm, const ParamInfo& p, void* data);
	
	static void f2(const VMachine& vm, const ParamInfo& p, T& fun, T2T<void>, T2T<const VMachine&>){
		fun(cast<C>(vm.get_arg_this()), vm);
	}	
	
	static void f2(const VMachine& vm, const ParamInfo& p, T& fun, ...){
		f3(vm, p, fun, T2T<A0>());
	}
	
	static void f3(const VMachine& vm, const ParamInfo& p, T& fun, T2T<const VMachine&>){
		Policy::return_result(vm, fun(
			cast<C>(vm.get_arg_this()), 
			vm
		));
	}

	static void f3(const VMachine& vm, const ParamInfo& p, T& fun, ...){
		check_arg(vm, p);
		Policy::return_result(vm, fun(
			cast<C>(vm.get_arg_this()), 
			arg_get<A0>(vm, p, 0)
		));
	}

	static void f2(const VMachine& vm, const ParamInfo& p, T& fun, T2T<void>, ...){
		check_arg(vm, p);
		fun(
			cast<C>(vm.get_arg_this()), 
			arg_get<A0>(vm, p, 0)
		);
		Policy::return_result(vm);
	}
};

template<class T, class C, class R, class A0, class Policy>
void method1<T, C, R, A0, Policy>::f(const VMachine& vm, const ParamInfo& p, void* data){
	f2(vm, p, *(T*)data, T2T<R>(), T2T<A0>());
}

template<class T, class C, class R, class A0, class A1, class Policy>
struct method2{
	
	static void f(const VMachine& vm, const ParamInfo& p, void* data);
	
	static void f2(const VMachine& vm, const ParamInfo& p, T& fun, ...){
		Policy::return_result(vm, fun(
			cast<C>(vm.get_arg_this()), 
			arg_get<A0>(vm, p, 0), 
			arg_get<A1>(vm, p, 1)
		));
	}
	
	static void f2(const VMachine& vm, const ParamInfo& p, T& fun, T2T<void>){
		fun(
			cast<C>(vm.get_arg_this()), 
			arg_get<A0>(vm, p, 0), 
			arg_get<A1>(vm, p, 1)
		);
		Policy::return_result(vm);
	}
};

template<class T, class C, class R, class A0, class A1, class Policy>
void method2<T, C, R, A0, A1, Policy>::f(const VMachine& vm, const ParamInfo& p, void* data){
	check_arg(vm, p);
	f2(vm, p, *(T*)data, T2T<R>());
}
	
template<class T, class C, class R, class A0, class A1, class A2, class Policy>
struct method3{
	
	static void f(const VMachine& vm, const ParamInfo& p, void* data){
		check_arg(vm, p);
		f2(vm, p, *(T*)data, T2T<R>());
	}
	
	static void f2(const VMachine& vm, const ParamInfo& p, T& fun, ...){
		Policy::return_result(vm, fun(
			cast<C>(vm.get_arg_this()), 
			arg_get<A0>(vm, p, 0), 
			arg_get<A1>(vm, p, 1), 
			arg_get<A2>(vm, p, 2)
		));
	}
	
	static void f2(const VMachine& vm, const ParamInfo& p, T& fun, T2T<void>){
		fun(
			cast<C>(vm.get_arg_this()), 
			arg_get<A0>(vm, p, 0), 
			arg_get<A1>(vm, p, 1), 
			arg_get<A2>(vm, p, 2)
		);
		Policy::return_result(vm);
	}
};

template<class T, class C, class R, class A0, class A1, class A2, class A3, class Policy>
struct method4{
	
	static void f(const VMachine& vm, const ParamInfo& p, void* data){
		check_arg(vm, p);
		f2(vm, p, *(T*)data, T2T<R>());
	}
	
	static void f2(const VMachine& vm, const ParamInfo& p, T& fun, ...){
		Policy::return_result(vm, fun(
			cast<C>(vm.get_arg_this()), 
			arg_get<A0>(vm, p, 0), 
			arg_get<A1>(vm, p, 1), 
			arg_get<A2>(vm, p, 2), 
			arg_get<A3>(vm, p, 3)
		));
	}
	
	static void f2(const VMachine& vm, const ParamInfo& p, T& fun, T2T<void>){
		fun(
			cast<C>(vm.get_arg_this()), 
			arg_get<A0>(vm, p, 0), 
			arg_get<A1>(vm, p, 1), 
			arg_get<A2>(vm, p, 2), 
			arg_get<A3>(vm, p, 3)
		);
		Policy::return_result(vm);
	}
};

template<class T, class C, class R, class A0, class A1, class A2, class A3, class A4, class Policy>
struct method5{
	
	static void f(const VMachine& vm, const ParamInfo& p, void* data){
		check_arg(vm, p);
		f2(vm, p, *(T*)data, T2T<R>());
	}
	
	static void f2(const VMachine& vm, const ParamInfo& p, T& fun, ...){
		Policy::return_result(vm, fun(
			cast<C>(vm.get_arg_this()), 
			arg_get<A0>(vm, p, 0), 
			arg_get<A1>(vm, p, 1), 
			arg_get<A2>(vm, p, 2), 
			arg_get<A3>(vm, p, 3),
			arg_get<A4>(vm, p, 4)
		));
	}
	
	static void f2(const VMachine& vm, const ParamInfo& p, T& fun, T2T<void>){
		fun(
			cast<C>(vm.get_arg_this()), 
			arg_get<A0>(vm, p, 0), 
			arg_get<A1>(vm, p, 1), 
			arg_get<A2>(vm, p, 2), 
			arg_get<A3>(vm, p, 3),
			arg_get<A4>(vm, p, 4)
		);
		Policy::return_result(vm);
	}
};

template<class C, class R>
struct memfun0{
	R (C::*fun)();
	memfun0(R (C::*fun)()):fun(fun){}
	memfun0(R (C::*fun)() const):fun((R (C::*)())fun){}
	R operator()(C* self){ return (self->*fun)(); }
};

template<class C, class R, class A0>
struct memfun1{
	R (C::*fun)(A0);
	memfun1(R (C::*fun)(A0)):fun(fun){}
	memfun1(R (C::*fun)(A0) const):fun((R (C::*)(A0))fun){}
	R operator()(C* self, A0 a0){ return (self->*fun)(a0); }
};

template<class C, class R, class A0, class A1>
struct memfun2{
	R (C::*fun)(A0, A1);
	memfun2(R (C::*fun)(A0, A1)):fun(fun){}
	memfun2(R (C::*fun)(A0, A1) const):fun((R (C::*)(A0, A1))fun){}
	R operator()(C* self, A0 a0, A1 a1){ return (self->*fun)(a0, a1); }
};

template<class C, class R, class A0, class A1, class A2>
struct memfun3{
	R (C::*fun)(A0, A1, A2);
	memfun3(R (C::*fun)(A0, A1, A2)):fun(fun){}
	memfun3(R (C::*fun)(A0, A1, A2) const):fun((R (C::*)(A0, A1, A2))fun){}
	R operator()(C* self, A0 a0, A1 a1, A2 a2){ return (self->*fun)(a0, a1, a2); }
};

template<class C, class R, class A0, class A1, class A2, class A3>
struct memfun4{
	R (C::*fun)(A0, A1, A2, A3);
	memfun4(R (C::*fun)(A0, A1, A2, A3)):fun(fun){}
	memfun4(R (C::*fun)(A0, A1, A2, A3) const):fun((R (C::*)(A0, A1, A2, A3))fun){}
	R operator()(C* self, A0 a0, A1 a1, A2 a2, A3 a3){ return (self->*fun)(a0, a1, a2, a3); }
};

template<class C, class R, class A0, class A1, class A2, class A3, class A4>
struct memfun5{
	R (C::*fun)(A0, A1, A2, A3, A4);
	memfun5(R (C::*fun)(A0, A1, A2, A3, A4)):fun(fun){}
	memfun5(R (C::*fun)(A0, A1, A2, A3, A4) const):fun((R (C::*)(A0, A1, A2, A3, A4))fun){}
	R operator()(C* self, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4){ return (self->*fun)(a0, a1, a2, a3, a4); }
};

template<class C, class T>
struct getter{
	T C::* var;
	getter(T C::* var):var(var){}
	const T& operator()(C* self){ return self->*var; }
};

template<class C, class T>
struct setter{
	T C::* var;
	setter(T C::* var):var(var){}
	const T& operator()(C* self,const T& v){ return self->*var = v; }
};

template<int N, int M>
struct ctor_fun;

template<>
struct ctor_fun<0, 2>{
	template<class T, class A0, class A1, class A2, class A3, class A4>
	struct inner{ static UserData<T> make(){ 
		return new_userdata<T>(); 
	} };
};

template<>
struct ctor_fun<1, 2>{
	template<class T, class A0, class A1, class A2, class A3, class A4>
	struct inner{ static UserData<T> make(A0 a0){ 
		return new_userdata<T>(a0); 
	} };
};

template<>
struct ctor_fun<2, 2>{
	template<class T, class A0, class A1, class A2, class A3, class A4>
	struct inner{ static UserData<T> make(A0 a0, A1 a1){ 
		return new_userdata<T>(a0, a1); 
	} };
};

template<>
struct ctor_fun<3, 2>{
	template<class T, class A0, class A1, class A2, class A3, class A4>
	struct inner{ static UserData<T> make(A0 a0, A1 a1, A2 a2){ 
		return new_userdata<T>(a0, a1, a2); 
	} };
};

template<>
struct ctor_fun<4, 2>{
	template<class T, class A0, class A1, class A2, class A3, class A4>
	struct inner{ static UserData<T> make(A0 a0, A1 a1, A2 a2, A3 a3){ 
		return new_userdata<T>(a0, a1, a2, a3); 
	} };
};

template<>
struct ctor_fun<5, 2>{
	template<class T, class A0, class A1, class A2, class A3, class A4>
	struct inner{ static UserData<T> make(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4){ 
		return new_userdata<T>(a0, a1, a2, a3, a4); 
	} };
};

template<>
struct ctor_fun<0, 1>{
	template<class T, class A0, class A1, class A2, class A3, class A4>
	struct inner{ static Any make(){ Any ret; return new(ret) T(); return ret; } };
};

template<>
struct ctor_fun<1, 1>{
	template<class T, class A0, class A1, class A2, class A3, class A4>
	struct inner{ static Any make(A0 a0){ 
		Any ret; return new(ret) T(a0); return ret; 
	} };
};

template<>
struct ctor_fun<2, 1>{
	template<class T, class A0, class A1, class A2, class A3, class A4>
	struct inner{ static Any make(A0 a0, A1 a1){ 
		Any ret; return new(ret) T(a0, a1); return ret; 
	} };
};

template<>
struct ctor_fun<3, 1>{
	template<class T, class A0, class A1, class A2, class A3, class A4>
	struct inner{ static Any make(A0 a0, A1 a1, A2 a2){ 
		Any ret; return new(ret) T(a0, a1, a2); return ret; 
	} };
};

template<>
struct ctor_fun<4, 1>{
	template<class T, class A0, class A1, class A2, class A3, class A4>
	struct inner{ static Any make(A0 a0, A1 a1, A2 a2, A3 a3){ 
		Any ret; return new(ret) T(a0, a1, a2, a3); return ret; 
	} };
};

template<>
struct ctor_fun<5, 1>{
	template<class T, class A0, class A1, class A2, class A3, class A4>
	struct inner{ static Any make(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4){ 
		Any ret; return new(ret) T(a0, a1, a2, a3, a4); return ret; 
	} };
};

template<>
struct ctor_fun<0, 0>{
	template<class T, class A0, class A1, class A2, class A3, class A4>
	struct inner{ static T make(){ 
		return T(); 
	} };
};

template<>
struct ctor_fun<1, 0>{
	template<class T, class A0, class A1, class A2, class A3, class A4>
	struct inner{ static T make(A0 a0){ 
		return T(a0); 
	} };
};

template<>
struct ctor_fun<2, 0>{
	template<class T, class A0, class A1, class A2, class A3, class A4>
	struct inner{ static T make(A0 a0, A1 a1){ 
		return T(a0, a1); 
	} };
};

template<>
struct ctor_fun<3, 0>{
	template<class T, class A0, class A1, class A2, class A3, class A4>
	struct inner{ static T make(A0 a0, A1 a1, A2 a2){ 
		return T(a0, a1, a2); 
	} };
};

template<>
struct ctor_fun<4, 0>{
	template<class T, class A0, class A1, class A2, class A3, class A4>
	struct inner{ static T make(A0 a0, A1 a1, A2 a2, A3 a3){ 
		return T(a0, a1, a2, a3); 
	} };
};

template<>
struct ctor_fun<5, 0>{
	template<class T, class A0, class A1, class A2, class A3, class A4>
	struct inner{ static T make(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4){ 
		return T(a0, a1, a2, a3, a4); 
	} };
};


template<class A0=void, class A1=void, class A2=void, class A3=void, class A4=void>
struct ctor{
	enum{
		N = 
		IsNotVoid<A0>::value + 
		IsNotVoid<A1>::value + 
		IsNotVoid<A2>::value + 
		IsNotVoid<A3>::value + 
		IsNotVoid<A4>::value
	};	
};

template<class T>
struct TType{ 
	typedef char (&any)[1];
	typedef char (&anyimpl)[2];
	typedef char (&unknown)[3];
	static any testT(Any*);
	static anyimpl testT(AnyImpl*);
	static unknown testT(void*);
	enum{ 
		temp = sizeof(testT((T*)0)),
		value = temp==sizeof(any) ? 0 :
				temp==sizeof(anyimpl) ? 1 : 2
	}; 
};

}

/**
* @brief C++の関数をXtalから呼び出せるようにするためのクラス
*
*/
class CFun : public Any{
public:

	CFun(void (*fun)(const VMachine& vm, const ParamInfo& p, void* data), const void* val, int_t val_size, int_t param_n);
	
	CFun(const Null&)
		:Any(null){}
		
	explicit CFun(CFunImpl* p)
		:Any((AnyImpl*)p){}

public:

	/**
	* @brief 仮引数の名前、デフォルト引数を指定する。
	*
	*/
	const CFun& param(
		const Named& value0=null, 
		const Named& value1=null,
		const Named& value2=null,
		const Named& value3=null,
		const Named& value4=null
	) const;

public:

	CFunImpl* impl() const{ return (CFunImpl*)Any::impl(); }
};


namespace detail{

template<class R, class Policy>
CFun fun(R (*f)(), const Policy&){
	return CFun(&detail::fun0<R (*)(), R, Policy>::f, &f, sizeof(f), 0);
}

template<class R, class A0, class Policy>
CFun fun(R (*f)(A0), const Policy&){
	return CFun(&detail::fun1<R (*)(A0), R, A0, Policy>::f, &f, sizeof(f), 1);
}

template<class R, class A0, class A1, class Policy>
CFun fun(R (*f)(A0, A1), const Policy&){
	return CFun(&detail::fun2<R (*)(A0, A1), R, A0, A1, Policy>::f, &f, sizeof(f), 2);
}

template<class R, class A0, class A1, class A2, class Policy>
CFun fun(R (*f)(A0, A1, A2), const Policy&){
	return CFun(&detail::fun3<R (*)(A0, A1, A2), R, A0, A1, A2, Policy>::f, &f, sizeof(f), 3);
}


template<class R, class A0, class A1, class A2, class A3, class Policy>
CFun fun(R (*f)(A0, A1, A2, A3), const Policy&){
	return CFun(&detail::fun4<R (*)(A0, A1, A2, A3), R, A0, A1, A2, A3, Policy>::f, &f, sizeof(f), 4);
}

template<class R, class A0, class A1, class A2, class A3, class A4, class Policy>
CFun fun(R (*f)(A0, A1, A2, A3, A4), const Policy&){
	return CFun(&detail::fun5<R (*)(A0, A1, A2, A3, A4), R, A0, A1, A2, A3, A4, Policy>::f, &f, sizeof(f), 5);
}

template<class C, class R, class Policy>
CFun method(R (C::*f)(), const Policy&){
	typedef detail::memfun0<C, R> memfun;
	memfun data(f);
	return CFun(&detail::method0<memfun, C*, R, Policy>::f, &data, sizeof(data), 0);
}

template<class C, class R, class A0, class Policy>
CFun method(R (C::*f)(A0), const Policy&){
	typedef detail::memfun1<C, R, A0> memfun;
	memfun data(f);
	return CFun(&detail::method1<memfun, C*, R, A0, Policy>::f, &data, sizeof(data), 1);
}

template<class C, class R, class A0, class A1, class Policy>
CFun method(R (C::*f)(A0, A1), const Policy&){
	typedef detail::memfun2<C, R, A0, A1> memfun;
	memfun data(f);
	return CFun(&detail::method2<memfun, C*, R, A0, A1, Policy>::f, &data, sizeof(data), 2);
}

template<class C, class R, class A0, class A1, class A2, class Policy>
CFun method(R (C::*f)(A0, A1, A2), const Policy&){
	typedef detail::memfun3<C, R, A0, A1, A2> memfun;
	memfun data(f);
	return CFun(&detail::method3<memfun, C*, R, A0, A1, A2, Policy>::f, &data, sizeof(data), 3);
}

template<class C, class R, class A0, class A1, class A2, class A3, class Policy>
CFun method(R (C::*f)(A0, A1, A2, A3), const Policy&){
	typedef detail::memfun4<C, R, A0, A1, A2, A3> memfun;
	memfun data(f);
	return CFun(&detail::method4<memfun, C*, R, A0, A1, A2, A3, Policy>::f, &data, sizeof(data), 4);
}

template<class C, class R, class A0, class A1, class A2, class A3, class A4, class Policy>
CFun method(R (C::*f)(A0, A1, A2, A3, A4), const Policy&){
	typedef detail::memfun5<C, R, A0, A1, A2, A3, A4> memfun;
	memfun data(f);
	return CFun(&detail::method5<memfun, C*, R, A0, A1, A2, A3, A4, Policy>::f, &data, sizeof(data), 5);
}

template<class C, class R, class Policy>
CFun method(R (C::*f)() const, const Policy&){
	typedef detail::memfun0<C, R> memfun;
	memfun data(f);
	return CFun(&detail::method0<memfun, C*, R, Policy>::f, &data, sizeof(data), 0);
}

template<class C, class R, class A0, class Policy>
CFun method(R (C::*f)(A0) const, const Policy&){
	typedef detail::memfun1<C, R, A0> memfun;
	memfun data(f);
	return CFun(&detail::method1<memfun, C*, R, A0, Policy>::f, &data, sizeof(data), 1);
}

template<class C, class R, class A0, class A1, class Policy>
CFun method(R (C::*f)(A0, A1) const, const Policy&){
	typedef detail::memfun2<C, R, A0, A1> memfun;
	memfun data(f);
	return CFun(&detail::method2<memfun, C*, R, A0, A1, Policy>::f, &data, sizeof(data), 2);
}

template<class C, class R, class A0, class A1, class A2, class Policy>
CFun method(R (C::*f)(A0, A1, A2) const, const Policy&){
	typedef detail::memfun3<C, R, A0, A1, A2> memfun;
	memfun data(f);
	return CFun(&detail::method3<memfun, C*, R, A0, A1, A2, Policy>::f, &data, sizeof(data), 3);
}

template<class C, class R, class A0, class A1, class A2, class A3, class Policy>
CFun method(R (C::*f)(A0, A1, A2, A3) const, const Policy&){
	typedef detail::memfun4<C, R, A0, A1, A2, A3> memfun;
	memfun data(f);
	return CFun(&detail::method4<memfun, C*, R, A0, A1, A2, A3, Policy>::f, &data, sizeof(data), 4);
}

template<class C, class R, class A0, class A1, class A2, class A3, class A4, class Policy>
CFun method(R (C::*f)(A0, A1, A2, A3, A4) const, const Policy&){
	typedef detail::memfun5<C, R, A0, A1, A2, A3, A4> memfun;
	memfun data(f);
	return CFun(&detail::method5<memfun, C*, R, A0, A1, A2, A3, A4, Policy>::f, &data, sizeof(data), 5);
}



template<class C, class R, class A0, class Policy>
CFun method(R (*f)(C, A0), const Policy&){
	return CFun(&detail::method1<R (*)(C, A0), C, R, A0, Policy>::f, &f, sizeof(f), 1);
}

template<class C, class R, class A0, class A1, class Policy>
CFun method(R (*f)(C, A0, A1), const Policy&){
	return CFun(&detail::method2<R (*)(C, A0, A1), C, R, A0, A1, Policy>::f, &f, sizeof(f), 2);
}

template<class C, class R, class A0, class A1, class A2, class Policy>
CFun method(R (*f)(C, A0, A1, A2), const Policy&){
	return CFun(&detail::method3<R (*)(C, A0, A1, A2), C, R, A0, A1, A2, Policy>::f, &f, sizeof(f), 3);
}

template<class C, class R, class A0, class A1, class A2, class A3, class Policy>
CFun method(R (*f)(C, A0, A1, A2, A3), const Policy&){
	return CFun(&detail::method4<R (*)(C, A0, A1, A2, A3), C, R, A0, A1, A2, A3, Policy>::f, &f, sizeof(f), 4);
}

template<class C, class R, class A0, class A1, class A2, class A3, class A4, class Policy>
CFun method(R (*f)(C, A0, A1, A2, A3, A4), const Policy&){
	return CFun(&detail::method5<R (*)(C, A0, A1, A2, A3, A4), C, R, A0, A1, A2, A3, A4, Policy>::f, &f, sizeof(f), 5);
}

}

/**
* @brief C++の関数をXtalから呼び出せるオブジェクトに変換するための関数
*
*/
template<class Fun>
CFun fun(Fun f){
	return detail::fun(f, result);
}

/**
* @brief C++の関数をXtalから呼び出せるオブジェクトに変換するための関数
*
*/
template<class Fun, class Policy>
CFun fun(Fun f, const Policy& policy){
	return detail::fun(f, policy);
}


/**
* @brief C++の関数、メンバをXtalから呼び出せるオブジェクトに変換するための関数
*
* 普通の関数をメソッドとして変換したい場合、第一引数をその型にすること。
*/
template<class Fun>
CFun method(Fun f){
	return detail::method(f, result);
}

/**
* @brief C++の関数、メンバをXtalから呼び出せるオブジェクトに変換するための関数
*
* 普通の関数をメソッドとして変換したい場合、第一引数をその型にすること。
*/
template<class Fun, class Policy>
CFun method(Fun f, const Policy& policy){
	return detail::method(f, policy);
}


template<class T, class A0=void, class A1=void, class A2=void, class A3=void, class A4=void>
class New : public CFun{
	enum{
		N = 
		IsNotVoid<A0>::value + 
		IsNotVoid<A1>::value + 
		IsNotVoid<A2>::value + 
		IsNotVoid<A3>::value + 
		IsNotVoid<A4>::value
	};
public:
	
	New()
		:CFun(fun(&detail::ctor_fun<detail::ctor<A0, A1, A2, A3, A4>::N, detail::TType<T>::value>::template inner<T, A0, A1, A2, A3, A4>::make)){}
};

}
