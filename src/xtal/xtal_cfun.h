/** \file src/xtal/xtal_cfun.h
* \brief src/xtal/xtal_cfun.h
*/

#ifndef XTAL_CFUN_H_INCLUDE_GUARD
#define XTAL_CFUN_H_INCLUDE_GUARD

#pragma once

namespace xtal{

class NativeMethod;	

/////////////////////////////////////////////

struct FunctorParam{
	Any result;
	const void* fun;
	VMachine* vm;
	Any args[16];
};

template<class R>
struct FunctorParamR : FunctorParam{
	typedef const R& type;
	void return_result(type r){ vm->return_result(r); }
};

template<class R> 
struct FunctorParamR<R&> : FunctorParam{
	typedef R& type;
	void return_result(type r){ vm->return_result(&r); }
};

template<class T> 
struct FunctorParamR<SmartPtr<T>&> : FunctorParam{
	typedef SmartPtr<T>& type;
	void return_result(type r){ vm->return_result(r); }
};

template<class T> 
struct FunctorParamR<const SmartPtr<T>&> : FunctorParam{
	typedef const SmartPtr<T>& type;
	void return_result(type r){ vm->return_result(r); }
};

template<> 
struct FunctorParamR<void> : FunctorParam{
	typedef void**** type;
};

#define XTAL_FP_HELPER(Type, XType, InitFun) \
template<> struct FunctorParamR<Type> : FunctorParam{\
	typedef Type type;\
	void return_result(type r){ result.value_.InitFun((XType::value_type)r); }\
};\
template<> struct FunctorParamR<Type&> : FunctorParamR<Type>{};\
template<> struct FunctorParamR<const Type&> : FunctorParamR<Type>{}

XTAL_FP_HELPER(bool, Bool, init_bool);

XTAL_FP_HELPER(char, Int, init_int);
XTAL_FP_HELPER(signed char, Int, init_int);
XTAL_FP_HELPER(unsigned char, Int, init_int);
XTAL_FP_HELPER(short, Int, init_int);
XTAL_FP_HELPER(unsigned short, Int, init_int);
XTAL_FP_HELPER(int, Int, init_int);
XTAL_FP_HELPER(unsigned int, Int, init_int);
XTAL_FP_HELPER(long, Int, init_int);
XTAL_FP_HELPER(unsigned long, Int, init_int);
XTAL_FP_HELPER(float, Float, init_float);
XTAL_FP_HELPER(double, Float, init_float);
XTAL_FP_HELPER(long double, Float, init_float);

#undef XTAL_FP_HELPER


template<class R>
inline void operator, (typename FunctorParamR<R>::type val, FunctorParamR<R>& ret){
	ret.return_result(val);
}

typedef void (*native_functon_t)(FunctorParam&);

template<class R>
inline FunctorParamR<R>& rcast(FunctorParam& a){
	return (FunctorParamR<R>&)a;
}

////////////////////////////////////////////

struct param_types_holder_n{
	native_functon_t fun; // 関数
	CppClassSymbolData* const * param_types; // thisと引数の型を表すクラスシンボルへのポインタ
	u8 size; // 関数のサイズ
	u8 param_n; // 引数の数
	u8 extendable; // 可変長かどうか
	u8 vm; // 仮想マシンを透過するかどうか
};

template<class TFun>
struct fun_param_holder{
	typedef typename TFun::fun_param_holder fph;
	typedef typename TFun::fun_type fun_type;

	static const param_types_holder_n value;
};

template<class TFun>
const param_types_holder_n fun_param_holder<TFun>::value = {
	&TFun::call,
	fph::values,
	sizeof(fun_type),
	TFun::arity,
	TFun::extendable,
	TFun::vm,
};

//////////////////////////////////////////////

template<class C, class T>
struct getter_functor;

template<class C, class T>
struct setter_functor;

template<class TFun>
struct functor{};

template<class TFun, TFun fun>
struct static_functor{};

///////////////////////////////////////////////

template<int Arity, class TFun, int Method, class R>
struct nfun_base{};

template<class TFun, int Method>
struct nfun
	: nfun_base<TFun::arity, TFun, Method, typename TFun::result_type>{};

template<class TFun>
struct dfun
	: nfun<TFun, 0>{};

template<class TFun>
struct dmemfun
	: nfun<TFun, 1>{};

template<class C, class T>
struct getter_functor;

template<class C, class T>
struct setter_functor;


template<class T, 
class A0=void, class A1=void, class A2=void, class A3=void, class A4=void, 
class A5=void, class A6=void, class A7=void, class A8=void, class A9=void,
class A10=void, class A11=void, class A12=void, class A13=void, class A14=void>
struct ctor_functor{};

//////////////////////////////////////////////////////////////

class StatelessNativeMethod : public Any{
public:
	StatelessNativeMethod(const param_types_holder_n& pth);
public:
	void on_rawcall(const VMachinePtr& vm);
};

class NativeMethod : public RefCountingBase{
public:
	enum{ TYPE = TYPE_NATIVE_METHOD };

	NativeMethod(const param_types_holder_n& pth, const void* val = 0);
	
	~NativeMethod();

public:

	const NativeFunPtr& param(int_t i, const IDPtr& key, const AnyPtr& value);

	const NativeFunPtr& add_param(const IDPtr& key, const AnyPtr& value);

public:

	const NamedParam* params();

	void on_visit_members(Visitor& m);

	void on_rawcall(const VMachinePtr& vm);

protected:
	const param_types_holder_n* pth_;
	void* data_;

	u8 min_param_count_;
	u8 max_param_count_;
	u8 val_size_;
};

class NativeFun : public NativeMethod{
public:
	enum{ TYPE = TYPE_NATIVE_FUN };

	NativeFun(const param_types_holder_n& pth, const void* val, const AnyPtr& this_);

public:

	void on_visit_members(Visitor& m);

	void on_rawcall(const VMachinePtr& vm);

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
	typedef dfun<ctor_functor<T, A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14> > fun_t;
	ctor(){
		NativeFunPtr::operator =(new_native_fun(fun_param_holder<fun_t>::value, 0));
	}
};
	
/**
* @brief 2重ディスパッチメソッド
*/
class DoubleDispatchMethod : public Base{
public:

	DoubleDispatchMethod(const IDPtr& primary_key)
		:primary_key_(primary_key){}

	void on_rawcall(const VMachinePtr& vm){
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
class DoubleDispatchFun : public Base{
public:

	DoubleDispatchFun(const ClassPtr& klass, const IDPtr& primary_key)
		:klass_(klass), primary_key_(primary_key){}

	void on_rawcall(const VMachinePtr& vm){
		klass_->member(primary_key_, vm->arg(0)->get_class())->rawcall(vm);
	}

	void on_visit_members(Visitor& m){
		Base::on_visit_members(m);
		m & klass_;
	}

private:
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
	return new_native_fun(fun_param_holder<dfun<Fun> >::value, &f);
}

/**
* \brief C++のメンバ関数をXtalから呼び出せるオブジェクトに変換するための関数
*
* 普通の関数をメソッドとして変換したい場合、第一引数をその型にすること。
*/
template<class Fun>
inline NativeFunPtr method(const Fun& f){
	return new_native_fun(fun_param_holder<dmemfun<Fun> >::value, &f);
}

/**
* \brief メンバ変数へのポインタからゲッター関数を生成する
*
*/
template<class T, class C>
inline NativeFunPtr getter(T C::* f){
	return new_native_fun(fun_param_holder<getter_functor<C, T> >::value, &f);
}
	
/**
* \brief メンバ変数へのポインタからセッター関数を生成する
*
*/
template<class T, class C>
inline NativeFunPtr setter(T C::* f){
	return new_native_fun(fun_param_holder<getter_functor<C, T> >::value(), &f);
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
