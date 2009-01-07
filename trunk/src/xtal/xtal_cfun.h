
#pragma once

namespace xtal{

class CFun;	

struct Result{
	static void return_result(const VMachinePtr& vm){
		vm->return_result();
	}

	static void return_result2(const VMachinePtr& vm, const AnyPtr& ret, Bool<true>){
		vm->return_result(ret);
	}

	template<class T>
	static void return_result2(const VMachinePtr& vm, const T& ret, Bool<false>){
		vm->return_result(xnew<T>(ret));
	}

	template<class T>
	static void return_result(const VMachinePtr& vm, const T& ret){
		return_result2(vm, ret, Bool<Convertible<T, AnyPtr>::value>());
	}
};

struct ReturnThis{
	static void return_result(const VMachinePtr& vm){
		vm->return_result(vm->get_arg_this());
	}

	template<class T>
	static void return_result(const VMachinePtr& vm, const T&){
		return_result(vm);
	}
};

struct ReturnUndefined{
	static void return_result(const VMachinePtr& vm){
		vm->return_result();
	}

	template<class T>
	static void return_result(const VMachinePtr& vm, const T&){
		return_result(vm);
	}
};

struct ReturnNone{
	static void return_result(const VMachinePtr& vm){}

	template<class T>
	static void return_result(const VMachinePtr& vm, const T&){}
};

struct ParamInfo{
	Named* params;
	int_t min_param_count;
	int_t max_param_count;
};


template<class T, int N>
struct arg_getter{

	static const AnyPtr&
	get(const VMachinePtr& vm, const ParamInfo& p){
		return vm->arg_default(N, p.params[N].name, p.params[N].value);
	}

	static typename CastResult<T>::type 
	cast(const Innocence& p){
		return unchecked_cast<T>(ap(p));
	}

	static uint_t check(const Innocence& p){
		return !can_cast<T>(ap(p)) << (N+1);
	}
};

template<class T>
struct arg_this_getter{

	static const AnyPtr&
	get(const VMachinePtr& vm, const ParamInfo& p){
		return vm->get_arg_this();
	}

	static typename CastResult<T>::type 
	cast(const Innocence& p){
		return unchecked_cast<T>(ap(p));
	}

	static uint_t check(const Innocence& p){
		return !can_cast<T>(ap(p)) << 0;
	}
};

struct arg_getter_vm{
	static const AnyPtr& 
	get(const VMachinePtr& vm, const ParamInfo& p){
		return vm;
	}

	static const VMachinePtr& 
	cast(const Innocence& p){
		return *(VMachinePtr*)&p;
	}

	static uint_t check(const Innocence& p){
		return 0;
	}
};

void check_args(const VMachinePtr& vm, const ParamInfo& p, uint_t flags);


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


template<class T, class R, class Policy>
struct fun0{
	static void f(const VMachinePtr& vm, const ParamInfo& p, void* data){
		(*(T*)data)(
		), ReturnPolicyTest<Policy>(vm), ReturnPolicyVoidTest();
	}
};
	
template<class T, class R, class A0, class Policy>
struct fun1{
	static void f(const VMachinePtr& vm, const ParamInfo& p, void* data){
		Innocence v0 = A0::get(vm, p);

		if(uint_t flags = A0::check(v0)){
			check_args(vm, p, flags);
		}
		else{
			(*(T*)data)(
				A0::cast(v0)
			), ReturnPolicyTest<Policy>(vm), ReturnPolicyVoidTest();
		}
	}
};

template<class T, class R, class A0, class A1, class Policy>
struct fun2{
	static void f(const VMachinePtr& vm, const ParamInfo& p, void* data){
		Innocence v0 = A0::get(vm, p);
		Innocence v1 = A1::get(vm, p);

		if(uint_t flags = A0::check(v0) | A1::check(v1)){
			check_args(vm, p, flags);
		}
		else{
			(*(T*)data)(
				A0::cast(v0), 
				A1::cast(v1)
			), ReturnPolicyTest<Policy>(vm), ReturnPolicyVoidTest();
		}
	}
};

template<class T, class R, class A0, class A1, class A2, class Policy>
struct fun3{
	static void f(const VMachinePtr& vm, const ParamInfo& p, void* data){
		Innocence v0 = A0::get(vm, p);
		Innocence v1 = A1::get(vm, p);
		Innocence v2 = A2::get(vm, p);

		if(uint_t flags = A0::check(v0) | A1::check(v1) | A2::check(v2)){
			check_args(vm, p, flags);
		}
		else{
			(*(T*)data)(
				A0::cast(v0), 
				A1::cast(v1), 
				A2::cast(v2)
			), ReturnPolicyTest<Policy>(vm), ReturnPolicyVoidTest();
		}
	}
};

template<class T, class R, class A0, class A1, class A2, class A3, class Policy>
struct fun4{
	static void f(const VMachinePtr& vm, const ParamInfo& p, void* data){
		Innocence v0 = A0::get(vm, p);
		Innocence v1 = A1::get(vm, p);
		Innocence v2 = A2::get(vm, p);
		Innocence v3 = A3::get(vm, p);

		if(uint_t flags = A0::check(v0) | A1::check(v1) | A2::check(v2) | A3::check(v3)){
			check_args(vm, p, flags);
		}
		else{
			(*(T*)data)(
				A0::cast(v0), 
				A1::cast(v1), 
				A2::cast(v2), 
				A3::cast(v3)
			), ReturnPolicyTest<Policy>(vm), ReturnPolicyVoidTest();
		}
	}
};

template<class T, class R, class A0, class A1, class A2, class A3, class A4, class Policy>
struct fun5{
	static void f(const VMachinePtr& vm, const ParamInfo& p, void* data){
		Innocence v0 = A0::get(vm, p);
		Innocence v1 = A1::get(vm, p);
		Innocence v2 = A2::get(vm, p);
		Innocence v3 = A3::get(vm, p);
		Innocence v4 = A4::get(vm, p);

		if(uint_t flags = A0::check(v0) | A1::check(v1) | A2::check(v2) | A3::check(v3) | A4::check(v4)){
			check_args(vm, p, flags);
		}
		else{
			(*(T*)data)(
				A0::cast(v0), 
				A1::cast(v1), 
				A2::cast(v2), 
				A3::cast(v3),
				A4::cast(v4)
			), ReturnPolicyTest<Policy>(vm), ReturnPolicyVoidTest();
		}
	}
};

template<class T, class R, class A0, class A1, class A2, class A3, class A4, class A5, class Policy>
struct fun6{
	static void f(const VMachinePtr& vm, const ParamInfo& p, void* data){
		Innocence v0 = A0::get(vm, p);
		Innocence v1 = A1::get(vm, p);
		Innocence v2 = A2::get(vm, p);
		Innocence v3 = A3::get(vm, p);
		Innocence v4 = A4::get(vm, p);
		Innocence v5 = A5::get(vm, p);

		if(uint_t flags = A0::check(v0) | A1::check(v1) | A2::check(v2) | A3::check(v3) | A4::check(v4) | A5::check(v5)){
			check_args(vm, p, flags);
		}
		else{
			(*(T*)data)(
				A0::cast(v0), 
				A1::cast(v1), 
				A2::cast(v2), 
				A3::cast(v3),
				A4::cast(v4),
				A5::cast(v5)
			), ReturnPolicyTest<Policy>(vm), ReturnPolicyVoidTest();
		}
	}
};

///////////////////////////////////////////////////////

template<class C, class R>
struct memfun0{
	typedef R (C::*fun_t)();
	fun_t fun;
	template<class T> memfun0(T f):fun((fun_t)f){}
	R operator()(C* self){ return (self->*fun)(); }
};

template<class C, class R, class A0>
struct memfun1{
	typedef R (C::*fun_t)(A0);
	fun_t fun;
	template<class T> memfun1(T f):fun((fun_t)f){}
	R operator()(C* self, A0 a0){ return (self->*fun)(a0); }
};

template<class C, class R, class A0, class A1>
struct memfun2{
	typedef R (C::*fun_t)(A0, A1);
	fun_t fun;
	template<class T> memfun2(T f):fun((fun_t)f){}
	R operator()(C* self, A0 a0, A1 a1){ return (self->*fun)(a0, a1); }
};

template<class C, class R, class A0, class A1, class A2>
struct memfun3{
	typedef R (C::*fun_t)(A0, A1, A2);
	fun_t fun;
	template<class T> memfun3(T f):fun((fun_t)f){}
	R operator()(C* self, A0 a0, A1 a1, A2 a2){ return (self->*fun)(a0, a1, a2); }
};

template<class C, class R, class A0, class A1, class A2, class A3>
struct memfun4{
	typedef R (C::*fun_t)(A0, A1, A2, A3);
	fun_t fun;
	template<class T> memfun4(T f):fun((fun_t)f){}
	R operator()(C* self, A0 a0, A1 a1, A2 a2, A3 a3){ return (self->*fun)(a0, a1, a2, a3); }
};

template<class C, class R, class A0, class A1, class A2, class A3, class A4>
struct memfun5{
	typedef R (C::*fun_t)(A0, A1, A2, A3, A4);
	fun_t fun;
	template<class T> memfun5(T f):fun((fun_t)f){}
	R operator()(C* self, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4){ return (self->*fun)(a0, a1, a2, a3, a4); }
};

///////////////////////////////////////////////////////

template<class C, class T>
struct getter_fun{
	T C::* var;
	getter_fun(T C::* var):var(var){}
	const T& operator()(C* self){ return self->*var; }
};

template<class C, class T>
struct setter_fun{
	T C::* var;
	setter_fun(T C::* var):var(var){}
	const T& operator()(C* self, const T& v){ return self->*var = v; }
};

///////////////////////////////////////////////////////

template<int N>
struct ctor_fun;

template<>
struct ctor_fun<0>{
	template<class T, class A0, class A1, class A2, class A3, class A4>
	struct inner{ static SmartPtr<T> make(){ return xnew<T>(); } };
};

template<>
struct ctor_fun<1>{
	template<class T, class A0, class A1, class A2, class A3, class A4>
	struct inner{ static SmartPtr<T> make(A0 a0){ return xnew<T>(a0); } };
};

template<>
struct ctor_fun<2>{
	template<class T, class A0, class A1, class A2, class A3, class A4>
	struct inner{ static SmartPtr<T> make(A0 a0, A1 a1){ return xnew<T>(a0, a1); } };
};

template<>
struct ctor_fun<3>{
	template<class T, class A0, class A1, class A2, class A3, class A4>
	struct inner{ static SmartPtr<T> make(A0 a0, A1 a1, A2 a2){ return xnew<T>(a0, a1, a2); } };
};

template<>
struct ctor_fun<4>{
	template<class T, class A0, class A1, class A2, class A3, class A4>
	struct inner{ static SmartPtr<T> make(A0 a0, A1 a1, A2 a2, A3 a3){ return xnew<T>(a0, a1, a2, a3); } };
};

template<>
struct ctor_fun<5>{
	template<class T, class A0, class A1, class A2, class A3, class A4>
	struct inner{ static SmartPtr<T> make(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4){ return xnew<T>(a0, a1, a2, a3, a4); } };
};

///////////////////////////////////////////////////////

struct CFunEssence{
	typedef void (*fun_t)(const VMachinePtr&, const ParamInfo&, void* data);

	CFunEssence(fun_t f, const void* val, int_t val_size, int_t param_n, bool args = false);

	fun_t f;
	void* val;
	int_t param_n;
	bool args;
	//bool (*)(can_cast_fun)(const Any&);
};

///////////////////////////////////////////////////////

template<class R, class Policy>
inline CFunEssence fun_impl(R (*f)(), Policy){
	return CFunEssence(&fun0<R (*)(), R, Policy>::f, &f, sizeof(f), 0);
}

template<class R, class A0, class Policy>
inline CFunEssence fun_impl(I2T<1>, R (*f)(A0), Policy){
	return CFunEssence(&fun1<R (*)(A0), R, arg_getter_vm, typename If<IsSame<R, void>::value, ReturnNone, Policy>::type>::f, &f, sizeof(f), 0, true);
}

template<class R, class A0, class Policy>
inline CFunEssence fun_impl(I2T<0>, R (*f)(A0), Policy){
	return CFunEssence(&fun1<R (*)(A0), R, arg_getter<A0, 0>, Policy>::f, &f, sizeof(f), 1);
}

template<class R, class A0, class Policy>
inline CFunEssence fun_impl(R (*f)(A0), Policy policy){
	return fun_impl(I2T<IsSame<A0, const VMachinePtr&>::value>(), f, policy);
}

template<class R, class A0, class A1, class Policy>
inline CFunEssence fun_impl(R (*f)(A0, A1), Policy){
	return CFunEssence(&fun2<R (*)(A0, A1), R, arg_getter<A0, 0>, arg_getter<A1, 1>, Policy>::f, &f, sizeof(f), 2);
}

template<class R, class A0, class A1, class A2, class Policy>
inline CFunEssence fun_impl(R (*f)(A0, A1, A2), Policy){
	return CFunEssence(&fun3<R (*)(A0, A1, A2), R, arg_getter<A0, 0>, arg_getter<A1, 1>, arg_getter<A2, 2>, Policy>::f, &f, sizeof(f), 3);
}


template<class R, class A0, class A1, class A2, class A3, class Policy>
inline CFunEssence fun_impl(R (*f)(A0, A1, A2, A3), Policy){
	return CFunEssence(&fun4<R (*)(A0, A1, A2, A3), R, arg_getter<A0, 0>, arg_getter<A1, 1>, arg_getter<A2, 2>, arg_getter<A3, 3>, Policy>::f, &f, sizeof(f), 4);
}

template<class R, class A0, class A1, class A2, class A3, class A4, class Policy>
inline CFunEssence fun_impl(R (*f)(A0, A1, A2, A3, A4), Policy){
	return CFunEssence(&fun5<R (*)(A0, A1, A2, A3, A4), R, arg_getter<A0, 0>, arg_getter<A1, 1>, arg_getter<A2, 2>, arg_getter<A3, 3>, arg_getter<A4, 4>, Policy>::f, &f, sizeof(f), 5);
}

//////////////////////////////////////////////////////////////

#ifdef _WIN32

template<class R, class Policy>
inline CFunEssence fun_impl(R (__stdcall *f)(), Policy){
	return CFunEssence(&fun0<R __stdcall (*)(), R, Policy>::f, &f, sizeof(f), 0);
}

template<class R, class A0, class Policy>
inline CFunEssence fun_impl(I2T<1>, R (__stdcall *f)(A0), Policy){
	return CFunEssence(&fun1<R __stdcall (*)(A0), R, arg_getter_vm, typename If<IsSame<R, void>::value, ReturnNone, Policy>::type>::f, &f, sizeof(f), 0, true);
}

template<class R, class A0, class Policy>
inline CFunEssence fun_impl(I2T<0>, R (__stdcall *f)(A0), Policy){
	return CFunEssence(&fun1<R __stdcall (*)(A0), R, arg_getter<A0, 0>, Policy>::f, &f, sizeof(f), 1);
}

template<class R, class A0, class Policy>
inline CFunEssence fun_impl(R (__stdcall *f)(A0), Policy policy){
	return fun_impl(I2T<IsSame<A0, const VMachinePtr&>::value>(), f, policy);
}

template<class R, class A0, class A1, class Policy>
inline CFunEssence fun_impl(R (__stdcall *f)(A0, A1), Policy){
	return CFunEssence(&fun2<R __stdcall (*)(A0, A1), R, arg_getter<A0, 0>, arg_getter<A1, 1>, Policy>::f, &f, sizeof(f), 2);
}

template<class R, class A0, class A1, class A2, class Policy>
inline CFunEssence fun_impl(R (__stdcall *f)(A0, A1, A2), Policy){
	return CFunEssence(&fun3<R __stdcall (*)(A0, A1, A2), R, arg_getter<A0, 0>, arg_getter<A1, 1>, arg_getter<A2, 2>, Policy>::f, &f, sizeof(f), 3);
}


template<class R, class A0, class A1, class A2, class A3, class Policy>
inline CFunEssence fun_impl(R (__stdcall *f)(A0, A1, A2, A3), Policy){
	return CFunEssence(&fun4<R __stdcall (*)(A0, A1, A2, A3), R, arg_getter<A0, 0>, arg_getter<A1, 1>, arg_getter<A2, 2>, arg_getter<A3, 3>, Policy>::f, &f, sizeof(f), 4);
}

template<class R, class A0, class A1, class A2, class A3, class A4, class Policy>
inline CFunEssence fun_impl(R (__stdcall *f)(A0, A1, A2, A3, A4), Policy){
	return CFunEssence(&fun5<R __stdcall (*)(A0, A1, A2, A3, A4), R, arg_getter<A0, 0>, arg_getter<A1, 1>, arg_getter<A2, 2>, arg_getter<A3, 3>, arg_getter<A4, 4>, Policy>::f, &f, sizeof(f), 5);
}

#endif

//////////////////////////////////////////////////////////////

template<class C, class R, class Policy>
inline CFunEssence method_impl(R (C::*f)(), Policy){
	typedef memfun0<C, R> memfun;
	memfun data(f);
	return CFunEssence(&fun1<memfun, R, arg_this_getter<C*>, Policy>::f, &data, sizeof(data), 0);
}

template<class C, class R, class A0, class Policy>
inline CFunEssence method_impl(I2T<1>, R (C::*f)(A0), Policy){
	typedef memfun1<C, R, A0> memfun;
	memfun data(f);
	return CFunEssence(&fun2<memfun, R, arg_this_getter<C*>, arg_getter_vm, typename If<IsSame<R, void>::value, ReturnNone, Policy>::type>::f, &data, sizeof(data), 0, true);
}

template<class C, class R, class A0, class Policy>
inline CFunEssence method_impl(I2T<0>, R (C::*f)(A0), Policy){
	typedef memfun1<C, R, A0> memfun;
	memfun data(f);
	return CFunEssence(&fun2<memfun, R, arg_this_getter<C*>, arg_getter<A0, 0>, Policy>::f, &data, sizeof(data), 1);
}

template<class C, class R, class A0, class Policy>
inline CFunEssence method_impl(R (C::*f)(A0), Policy policy){
	return method_impl(I2T<IsSame<A0, const VMachinePtr&>::value>(), f, policy);
}

template<class C, class R, class A0, class A1, class Policy>
inline CFunEssence method_impl(R (C::*f)(A0, A1), Policy){
	typedef memfun2<C, R, A0, A1> memfun;
	memfun data(f);
	return CFunEssence(&fun3<memfun, R, arg_this_getter<C*>, arg_getter<A0, 0>, arg_getter<A1, 1>, Policy>::f, &data, sizeof(data), 2);
}

template<class C, class R, class A0, class A1, class A2, class Policy>
inline CFunEssence method_impl(R (C::*f)(A0, A1, A2), Policy){
	typedef memfun3<C, R, A0, A1, A2> memfun;
	memfun data(f);
	return CFunEssence(&fun4<memfun, R, arg_this_getter<C*>, arg_getter<A0, 0>, arg_getter<A1, 1>, arg_getter<A2, 2>, Policy>::f, &data, sizeof(data), 3);
}

template<class C, class R, class A0, class A1, class A2, class A3, class Policy>
inline CFunEssence method_impl(R (C::*f)(A0, A1, A2, A3), Policy){
	typedef memfun4<C, R, A0, A1, A2, A3> memfun;
	memfun data(f);
	return CFunEssence(&fun5<memfun, R, arg_this_getter<C*>, arg_getter<A0, 0>, arg_getter<A1, 1>, arg_getter<A2, 2>, arg_getter<A3, 3>, Policy>::f, &data, sizeof(data), 4);
}

template<class C, class R, class A0, class A1, class A2, class A3, class A4, class Policy>
inline CFunEssence method_impl(R (C::*f)(A0, A1, A2, A3, A4), Policy){
	typedef memfun5<C, R, A0, A1, A2, A3, A4> memfun;
	memfun data(f);
	return CFunEssence(&fun6<memfun, R, arg_this_getter<C*>, arg_getter<A0, 0>, arg_getter<A1, 1>, arg_getter<A2, 2>, arg_getter<A3, 3>, arg_getter<A4, 4>, Policy>::f, &data, sizeof(data), 5);
}

//////////////////////////////////////////////////////////////

template<class C, class R, class Policy>
inline CFunEssence method_impl(R (C::*f)() const, Policy){
	typedef memfun0<C, R> memfun;
	memfun data(f);
	return CFunEssence(&fun1<memfun, R, arg_this_getter<C*>, Policy>::f, &data, sizeof(data), 0);
}

template<class C, class R, class A0, class Policy>
inline CFunEssence method_impl(I2T<1>, R (C::*f)(A0) const, Policy){
	typedef memfun1<C, R, A0> memfun;
	memfun data(f);
	return CFunEssence(&fun2<memfun, R, arg_this_getter<C*>, arg_getter_vm, typename If<IsSame<R, void>::value, ReturnNone, Policy>::type>::f, &data, sizeof(data), 0, true);
}

template<class C, class R, class A0, class Policy>
inline CFunEssence method_impl(I2T<0>, R (C::*f)(A0) const, Policy){
	typedef memfun1<C, R, A0> memfun;
	memfun data(f);
	return CFunEssence(&fun2<memfun, R, arg_this_getter<C*>, arg_getter<A0, 0>, Policy>::f, &data, sizeof(data), 1);
}

template<class C, class R, class A0, class Policy>
inline CFunEssence method_impl(R (C::*f)(A0) const, Policy policy){
	return method_impl(I2T<IsSame<A0, const VMachinePtr&>::value>(), f, policy);
}

template<class C, class R, class A0, class A1, class Policy>
inline CFunEssence method_impl(R (C::*f)(A0, A1) const, Policy){
	typedef memfun2<C, R, A0, A1> memfun;
	memfun data(f);
	return CFunEssence(&fun3<memfun, R, arg_this_getter<C*>, arg_getter<A0, 0>, arg_getter<A1, 1>, Policy>::f, &data, sizeof(data), 2);
}

template<class C, class R, class A0, class A1, class A2, class Policy>
inline CFunEssence method_impl(R (C::*f)(A0, A1, A2) const, Policy){
	typedef memfun3<C, R, A0, A1, A2> memfun;
	memfun data(f);
	return CFunEssence(&fun4<memfun, R, arg_this_getter<C*>, arg_getter<A0, 0>, arg_getter<A1, 1>, arg_getter<A2, 2>, Policy>::f, &data, sizeof(data), 3);
}

template<class C, class R, class A0, class A1, class A2, class A3, class Policy>
inline CFunEssence method_impl(R (C::*f)(A0, A1, A2, A3) const, Policy){
	typedef memfun4<C, R, A0, A1, A2, A3> memfun;
	memfun data(f);
	return CFunEssence(&fun5<memfun, R, arg_this_getter<C*>, arg_getter<A0, 0>, arg_getter<A1, 1>, arg_getter<A2, 2>, arg_getter<A3, 3>, Policy>::f, &data, sizeof(data), 4);
}

template<class C, class R, class A0, class A1, class A2, class A3, class A4, class Policy>
inline CFunEssence method_impl(R (C::*f)(A0, A1, A2, A3, A4) const, Policy){
	typedef memfun5<C, R, A0, A1, A2, A3, A4> memfun;
	memfun data(f);
	return CFunEssence(&fun6<memfun, R, arg_this_getter<C*>, arg_getter<A0, 0>, arg_getter<A1, 1>, arg_getter<A2, 2>, arg_getter<A3, 3>, arg_getter<A4, 4>, Policy>::f, &data, sizeof(data), 5);
}

//////////////////////////////////////////////////////////////

template<class C, class R, class Policy>
inline CFunEssence method_impl(R (*f)(C), Policy){
	return CFunEssence(&fun1<R (*)(C), R, arg_this_getter<C>, Policy>::f, &f, sizeof(f), 0);
}

template<class C, class R, class A0, class Policy>
inline CFunEssence method_impl(I2T<1>, R (*f)(C, A0), Policy){
	return CFunEssence(&fun2<R (*)(C, A0), R, arg_this_getter<C>, arg_getter_vm, typename If<IsSame<R, void>::value, ReturnNone, Policy>::type>::f, &f, sizeof(f), 0, true);
}

template<class C, class R, class A0, class Policy>
inline CFunEssence method_impl(I2T<0>, R (*f)(C, A0), Policy){
	return CFunEssence(&fun2<R (*)(C, A0), R, arg_this_getter<C>, arg_getter<A0, 0>, Policy>::f, &f, sizeof(f), 1);
}

template<class C, class R, class A0, class Policy>
inline CFunEssence method_impl(R (*f)(C, A0), Policy policy){
	return method_impl(I2T<IsSame<A0, const VMachinePtr&>::value>(), f, policy);
}

template<class C, class R, class A0, class A1, class Policy>
inline CFunEssence method_impl(R (*f)(C, A0, A1), Policy){
	return CFunEssence(&fun3<R (*)(C, A0, A1), R, arg_this_getter<C>, arg_getter<A0, 0>, arg_getter<A1, 1>, Policy>::f, &f, sizeof(f), 2);
}

template<class C, class R, class A0, class A1, class A2, class Policy>
inline CFunEssence method_impl(R (*f)(C, A0, A1, A2), Policy){
	return CFunEssence(&fun4<R (*)(C, A0, A1, A2), R, arg_this_getter<C>, arg_getter<A0, 0>, arg_getter<A1, 1>, arg_getter<A2, 2>, Policy>::f, &f, sizeof(f), 3);
}

template<class C, class R, class A0, class A1, class A2, class A3, class Policy>
inline CFunEssence method_impl(R (*f)(C, A0, A1, A2, A3), Policy){
	return CFunEssence(&fun5<R (*)(C, A0, A1, A2, A3), R, arg_this_getter<C>, arg_getter<A0, 0>, arg_getter<A1, 1>, arg_getter<A2, 2>, arg_getter<A3, 3>, Policy>::f, &f, sizeof(f), 4);
}

template<class C, class R, class A0, class A1, class A2, class A3, class A4, class Policy>
inline CFunEssence method_impl(R (*f)(C, A0, A1, A2, A3, A4), Policy){
	return CFunEssence(&fun6<R (*)(C, A0, A1, A2, A3, A4), R, arg_this_getter<C>, arg_getter<A0, 0>, arg_getter<A1, 1>, arg_getter<A2, 2>, arg_getter<A3, 3>, arg_getter<A4, 4>, Policy>::f, &f, sizeof(f), 5);
}

//////////////////////////////////////////////////////////////

template<class T, class C, class Policy>
inline CFunEssence getter_impl(T C::* v, Policy){
	typedef getter_fun<C, T> getter;
	getter data(v);
	return CFunEssence(&fun1<getter, const T&, arg_this_getter<C*>, Policy>::f, &data, sizeof(data), 0);
}
	
template<class T, class C, class Policy>
inline CFunEssence setter_impl(T C::* v, Policy){
	typedef setter_fun<C, T> setter;
	setter data(v);
	return CFunEssence(&fun2<setter, const T&, arg_this_getter<C*>, arg_getter<const T&, 0>, Policy>::f, &data, sizeof(data), 1);
}

//////////////////////////////////////////////////////////////

class CFun : public HaveName{
public:
	typedef void (*fun_t)(const VMachinePtr&, const ParamInfo&, void* data);

	CFun(fun_t f, void* val, int_t param_n);
	
	virtual ~CFun();

	CFunPtr param(
		const Named2& value0 = null_named, 
		const Named2& value1 = null_named,
		const Named2& value2 = null_named,
		const Named2& value3 = null_named,
		const Named2& value4 = null_named
	);

	virtual void visit_members(Visitor& m);

	void check_arg(const VMachinePtr& vm);

	virtual void call(const VMachinePtr& vm);

protected:
	void* val_;
	fun_t fun_;
	ParamInfo pi_;
	int_t param_n_;
};

class CFunArgs : public CFun{
public:

	CFunArgs(fun_t f, void* val, int_t param_n);

	virtual void call(const VMachinePtr& vm);
};

CFunPtr new_cfun(const CFunEssence& essence);

//////////////////////////////////////////////////////////////

/**
* @brief C++の関数をXtalから呼び出せるオブジェクトに変換するための関数
*
*/
template<class Fun>
inline CFunPtr fun(Fun f){
	return new_cfun(fun_impl(f, result));
}

/**
* @brief C++の関数をXtalから呼び出せるオブジェクトに変換するための関数
*
*/
template<class Fun, class Policy>
inline CFunPtr fun(Fun f, Policy policy){
	return new_cfun(fun_impl(f, policy));
}


/**
* @brief C++のメンバ関数をXtalから呼び出せるオブジェクトに変換するための関数
*
* 普通の関数をメソッドとして変換したい場合、第一引数をその型にすること。
*/
template<class Fun>
inline CFunPtr method(Fun f){
	return new_cfun(method_impl(f, result));
}

/**
* @brief C++のメンバ関数をXtalから呼び出せるオブジェクトに変換するための関数
*
* 普通の関数をメソッドとして変換したい場合、第一引数をその型にすること。
*/
template<class Fun, class Policy>
inline CFunPtr method(Fun f, Policy policy){
	return new_cfun(method_impl(f, policy));
}

/**
* @brief C++のコンストラクタをXtalから呼び出せるオブジェクトに変換するための擬似関数
*
*/
template<class T, class A0=void, class A1=void, class A2=void, class A3=void, class A4=void>
class ctor : public CFunPtr{
	enum{
		N = 
		IsNotVoid<A0>::value + 
		IsNotVoid<A1>::value + 
		IsNotVoid<A2>::value + 
		IsNotVoid<A3>::value + 
		IsNotVoid<A4>::value
	};
public:
	
	ctor()
		:CFunPtr(fun(&ctor_fun<N>::template inner<T, A0, A1, A2, A3, A4>::make)){}
};

	
/**
* @brief メンバ変数へのポインタからゲッター関数を生成する
*
*/
template<class T, class C, class Policy>
inline CFunPtr getter(T C::* v, Policy policy){
	return new_cfun(getter_impl(v, policy));
}
	
/**
* @brief メンバ変数へのポインタからセッター関数を生成する
*
*/
template<class T, class C, class Policy>
inline CFunPtr setter(T C::* v, Policy policy){
	return new_cfun(setter_impl(v, policy));
}


/**
* @birief 2重ディスパッチメソッド
*/
class DualDispatchMethod : public HaveName{
public:

	DualDispatchMethod(const IDPtr& primary_key)
		:primary_key_(primary_key){}

	virtual void call(const VMachinePtr& vm){
		vm->get_arg_this()->rawsend(vm, primary_key_, vm->arg(0)->get_class(), vm->get_arg_this());
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

	virtual void call(const VMachinePtr& vm){
		klass_->member(primary_key_, vm->arg(0)->get_class(), vm->get_arg_this())->call(vm);
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
