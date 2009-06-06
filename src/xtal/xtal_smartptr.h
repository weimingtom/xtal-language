/** \file src/xtal/xtal_smartptr.h
* \brief src/xtal/xtal_smartptr.h
*/

#ifndef XTAL_SMARTPTR_H_INCLUDE_GUARD
#define XTAL_SMARTPTR_H_INCLUDE_GUARD

#pragma once

namespace xtal{

template<int N, class T>
struct ExtractSmartPtr{};

template<class T>
struct ExtractSmartPtr<INHERITED_BASE, T>{
	static T* extract(const Any& a){
		XTAL_ASSERT(type(a)!=TYPE_NULL); // このアサーションで止まる場合、nullポインタが格納されている
		XTAL_ASSERT(type(a)!=TYPE_UNDEFINED); // このアサーションで止まる場合、undefinedが格納されている
		return (T*)pvalue(a);
	}
};

template<class T>
struct ExtractSmartPtr<INHERITED_RCBASE, T>{
	static T* extract(const Any& a){
		XTAL_ASSERT(type(a)!=TYPE_NULL); // このアサーションで止まる場合、nullポインタが格納されている
		XTAL_ASSERT(type(a)!=TYPE_UNDEFINED); // このアサーションで止まる場合、undefinedが格納されている
		return (T*)rcpvalue(a);
	}
};

template<class T>
struct ExtractSmartPtr<INHERITED_ANY, T>{
	static T* extract(const Any& a){
		return (T*)&a;
	}
};

template<class T>
struct ExtractSmartPtr<INHERITED_OTHER, T>{
	static T* extract(const Any& a){
		return (T*)((UserTypeHolder<T>*)pvalue(a))->ptr; 
	}
};

/**
* \brief T型へのポインタを保持するためのスマートポインタ
*/
template<class T>
class SmartPtr : public SmartPtr<Any>{
public:
	
	SmartPtr(){}

	template<class Deleter>
	SmartPtr(T* p, const Deleter& deleter)
		:SmartPtr<Any>(p, deleter){}

	template<class U>
	SmartPtr(const SmartPtr<U>& p)
		:SmartPtr<Any>(p){

		// 継承関係をここでチェックしている。
		// ここでコンパイルエラーになる場合、
		// ptr_cast関数等を使用して型を変換する必要がある。
		T* n = (U*)0; 
		XTAL_UNUSED_VAR(n);
	}
	
	template<class U>
	SmartPtr<T>& operator =(const SmartPtr<U>& p){
		// 継承関係をここでチェックしている。
		// ここでコンパイルエラーになる場合、
		// ptr_cast関数等を使用して型を変換する必要がある。
		T* n = (U*)0; 
		XTAL_UNUSED_VAR(n);
		
		SmartPtr<Any>::operator =(p);
		return *this;
	}
	
	SmartPtr<T>& operator =(const NullPtr& null){
		SmartPtr<Any>::operator =(null);
		return *this;
	}

	/// nullを受け取るコンストラクタ
	SmartPtr(const NullPtr&){}

	/// 特別なコンストラクタ1
	SmartPtr(typename SmartPtrCtor1<T>::type v);

	/// 特別なコンストラクタ2
	SmartPtr(typename SmartPtrCtor2<T>::type v);

	/// 特別なコンストラクタ3
	SmartPtr(typename SmartPtrCtor3<T>::type v);

	/// 特別なコンストラクタ4
	SmartPtr(typename SmartPtrCtor4<T>::type v);

public:

	template<class U>
	void assign_direct(const SmartPtr<U>& a){
		AnyPtr::assign_direct(a);

		// 継承関係をここでチェックしている。
		// ここでコンパイルエラーになる場合、
		// ptr_cast関数等を使用して型を変換する必要がある。
		T* n = (U*)0; 
		XTAL_UNUSED_VAR(n);
	}

private:

	/**
	* \brief 暗黙の変換を抑えるためのコンストラクタ。
	*
	* AnyPtrからの暗黙の変換を拒否するために、privateで定義されている。
	* AnyPtrからSmartPtr<T>に変換するにはptr_cast関数、ptr_cast関数を使用すること。
	*/
	//SmartPtr(const AnyPtr&);

	/**
	* \brief 暗黙の変換を抑えるためのコンストラクタ。
	*
	* 得体の知れないポインタ型からの暗黙の変換を拒否するために、privateで定義されている。
	*
	* Baseクラスを継承していないAというクラスを持たせたい場合、
	* SmartPtr<A> p = xnew<A>(); 
	* とxnewで作り出すか、
	* SmartPtr<A> p = SmartPtr<A>(new A, deleter);
	* とコンストラクタにnewで生成したポインタとdeleterを渡すか
	* static A static_a;
	* SmartPtr<A> p = SmartPtr<A>(&static_a, undeleter);
	* と寿命が長いオブジェクトへのポインタとundeleterを渡すか、
	* または独自のdeleterを定義して渡す方法をとること。
	*/
	SmartPtr(void*);

public:

	/**
	* \brief T型へのポインタを取得する。
	*/
	T* get() const{ return ExtractSmartPtr<InheritedN<T>::value, T>::extract(*this); }

	/**
	* \brief ->演算子
	* スマートポインタとして扱うために。
	*/
	T* operator ->() const{ return get(); }
	
	/**
	* \brief *演算子
	* スマートポインタとして扱うために。
	*/
	T& operator *() const{ return *get(); }
	
public:

	SmartPtr(SmartPtrSelector<INHERITED_BASE>)
		:SmartPtr<Any>(new T(), cpp_class<T>(), special_ctor_t()){}

	SmartPtr(SmartPtrSelector<INHERITED_RCBASE>)
		:SmartPtr<Any>(new T(), T::TYPE, special_ctor_t()){}

	SmartPtr(SmartPtrSelector<INHERITED_ANY>)
		:SmartPtr<Any>(T(), special_ctor_t()){}

	SmartPtr(SmartPtrSelector<INHERITED_OTHER>)
		:SmartPtr<Any>(new UserTypeHolderSub<T>(), cpp_class<T>(), special_ctor_t()){
		UserTypeHolderSub<T>* p = ((UserTypeHolderSub<T>*)pvalue(*this));
		p->ptr = (T*)p->buf;
		new(p->ptr) T;
	}

/////////////////////

	template<class A0>
	SmartPtr(SmartPtrSelector<INHERITED_BASE>, const A0& a0)
		:SmartPtr<Any>(new T(a0), cpp_class<T>(), special_ctor_t()){}

	template<class A0>
	SmartPtr(SmartPtrSelector<INHERITED_RCBASE>, const A0& a0)
		:SmartPtr<Any>(new T(a0), T::TYPE, special_ctor_t()){}

	template<class A0>
	SmartPtr(SmartPtrSelector<INHERITED_ANY>, const A0& a0)
		:SmartPtr<Any>(T(a0), special_ctor_t()){}

	template<class A0>
	SmartPtr(SmartPtrSelector<INHERITED_OTHER>, const A0& a0)
		:SmartPtr<Any>(new UserTypeHolderSub<T>(), cpp_class<T>(), special_ctor_t()){
		UserTypeHolderSub<T>* p = ((UserTypeHolderSub<T>*)pvalue(*this));
		p->ptr = (T*)p->buf;
		new(p->ptr) T(a0);
	}

/////////////////////

	template<class A0, class A1>
	SmartPtr(SmartPtrSelector<INHERITED_BASE>, const A0& a0, const A1& a1)
		:SmartPtr<Any>(new T(a0, a1), cpp_class<T>(), special_ctor_t()){}

	template<class A0, class A1>
	SmartPtr(SmartPtrSelector<INHERITED_RCBASE>, const A0& a0, const A1& a1)
		:SmartPtr<Any>(new T(a0, a1), T::TYPE, special_ctor_t()){}

	template<class A0, class A1>
	SmartPtr(SmartPtrSelector<INHERITED_ANY>, const A0& a0, const A1& a1)
		:SmartPtr<Any>(T(a0, a1), special_ctor_t()){}

	template<class A0, class A1>
	SmartPtr(SmartPtrSelector<INHERITED_OTHER>, const A0& a0, const A1& a1)
		:SmartPtr<Any>(new UserTypeHolderSub<T>(), cpp_class<T>(), special_ctor_t()){
		UserTypeHolderSub<T>* p = ((UserTypeHolderSub<T>*)pvalue(*this));
		p->ptr = (T*)p->buf;
		new(p->ptr) T(a0, a1);
	}

/////////////////////

	template<class A0, class A1, class A2>
	SmartPtr(SmartPtrSelector<INHERITED_BASE>, const A0& a0, const A1& a1, const A2& a2)
		:SmartPtr<Any>(new T(a0, a1, a2), cpp_class<T>(), special_ctor_t()){}

	template<class A0, class A1, class A2>
	SmartPtr(SmartPtrSelector<INHERITED_RCBASE>, const A0& a0, const A1& a1, const A2& a2)
		:SmartPtr<Any>(new T(a0, a1, a2), T::TYPE, special_ctor_t()){}

	template<class A0, class A1, class A2>
	SmartPtr(SmartPtrSelector<INHERITED_ANY>, const A0& a0, const A1& a1, const A2& a2)
		:SmartPtr<Any>(T(a0, a1, a2), special_ctor_t()){}

	template<class A0, class A1, class A2>
	SmartPtr(SmartPtrSelector<INHERITED_OTHER>, const A0& a0, const A1& a1, const A2& a2)
		:SmartPtr<Any>(new UserTypeHolderSub<T>(), cpp_class<T>(), special_ctor_t()){
		UserTypeHolderSub<T>* p = ((UserTypeHolderSub<T>*)pvalue(*this));
		p->ptr = (T*)p->buf;
		new(p->ptr) T(a0, a1, a2);
	}

/////////////////////

	template<class A0, class A1, class A2, class A3>
	SmartPtr(SmartPtrSelector<INHERITED_BASE>, const A0& a0, const A1& a1, const A2& a2, const A3& a3)
		:SmartPtr<Any>(new T(a0, a1, a2, a3), cpp_class<T>(), special_ctor_t()){}

	template<class A0, class A1, class A2, class A3>
	SmartPtr(SmartPtrSelector<INHERITED_RCBASE>, const A0& a0, const A1& a1, const A2& a2, const A3& a3)
		:SmartPtr<Any>(new T(a0, a1, a2, a3), T::TYPE, special_ctor_t()){}

	template<class A0, class A1, class A2, class A3>
	SmartPtr(SmartPtrSelector<INHERITED_ANY>, const A0& a0, const A1& a1, const A2& a2, const A3& a3)
		:SmartPtr<Any>(T(a0, a1, a2, a3), special_ctor_t()){}

	template<class A0, class A1, class A2, class A3>
	SmartPtr(SmartPtrSelector<INHERITED_OTHER>, const A0& a0, const A1& a1, const A2& a2, const A3& a3)
		:SmartPtr<Any>(new UserTypeHolderSub<T>(), cpp_class<T>(), special_ctor_t()){
		UserTypeHolderSub<T>* p = ((UserTypeHolderSub<T>*)pvalue(*this));
		p->ptr = (T*)p->buf;
		new(p->ptr) T(a0, a1, a2, a3);
	}

/////////////////////

	template<class A0, class A1, class A2, class A3, class A4>
	SmartPtr(SmartPtrSelector<INHERITED_BASE>, const A0& a0, const A1& a1, const A2& a2, const A3& a3, const A4& a4)
		:SmartPtr<Any>(new T(a0, a1, a2, a3, a4), cpp_class<T>(), special_ctor_t()){}

	template<class A0, class A1, class A2, class A3, class A4>
	SmartPtr(SmartPtrSelector<INHERITED_RCBASE>, const A0& a0, const A1& a1, const A2& a2, const A3& a3, const A4& a4)
		:SmartPtr<Any>(new T(a0, a1, a2, a3, a4), T::TYPE, special_ctor_t()){}

	template<class A0, class A1, class A2, class A3, class A4>
	SmartPtr(SmartPtrSelector<INHERITED_ANY>, const A0& a0, const A1& a1, const A2& a2, const A3& a3, const A4& a4)
		:SmartPtr<Any>(T(a0, a1, a2, a3, a4), special_ctor_t()){}

	template<class A0, class A1, class A2, class A3, class A4>
	SmartPtr(SmartPtrSelector<INHERITED_OTHER>, const A0& a0, const A1& a1, const A2& a2, const A3& a3, const A4& a4)
		:SmartPtr<Any>(new UserTypeHolderSub<T>(), cpp_class<T>(), special_ctor_t()){
		UserTypeHolderSub<T>* p = ((UserTypeHolderSub<T>*)pvalue(*this));
		p->ptr = (T*)p->buf;
		new(p->ptr) T(a0, a1, a2, a3, a4);
	}

};

template<class T>
SmartPtr<T>::SmartPtr(typename SmartPtrCtor1<T>::type v)
	:SmartPtr<Any>(SmartPtrCtor1<T>::call(v)){}

template<class T>
SmartPtr<T>::SmartPtr(typename SmartPtrCtor2<T>::type v)
	:SmartPtr<Any>(SmartPtrCtor2<T>::call(v)){}

template<class T>
SmartPtr<T>::SmartPtr(typename SmartPtrCtor3<T>::type v)
	:SmartPtr<Any>(SmartPtrCtor3<T>::call(v)){}

template<class T>
SmartPtr<T>::SmartPtr(typename SmartPtrCtor4<T>::type v)
	:SmartPtr<Any>(SmartPtrCtor4<T>::call(v)){}


/// \name オブジェクト生成
//@{

//{REPEAT{{
/*
*/
//}}REPEAT}

/**
* \brief Tオブジェクトを生成する
*/
template<class T>
inline SmartPtr<T> xnew(){
	return SmartPtr<T>(SmartPtrSelector<InheritedN<T>::value>());
}

/**
* \brief Tオブジェクトを生成する
*/
template<class T, class A0>
inline SmartPtr<T> xnew(const A0& a0){
	return SmartPtr<T>(SmartPtrSelector<InheritedN<T>::value>(), a0);
}

/**
* \brief Tオブジェクトを生成する
*/
template<class T, class A0, class A1>
inline SmartPtr<T> xnew(const A0& a0, const A1& a1){
	return SmartPtr<T>(SmartPtrSelector<InheritedN<T>::value>(), a0, a1);
}

/**
* \brief Tオブジェクトを生成する
*/
template<class T, class A0, class A1, class A2>
inline SmartPtr<T> xnew(const A0& a0, const A1& a1, const A2& a2){
	return SmartPtr<T>(SmartPtrSelector<InheritedN<T>::value>(), a0, a1, a2);
}

/**
* \brief Tオブジェクトを生成する
*/
template<class T, class A0, class A1, class A2, class A3>
inline SmartPtr<T> xnew(const A0& a0, const A1& a1, const A2& a2, const A3& a3){
	return SmartPtr<T>(SmartPtrSelector<InheritedN<T>::value>(), a0, a1, a2, a3);
}

/**
* \brief Tオブジェクトを生成する
*/
template<class T, class A0, class A1, class A2, class A3, class A4>
inline SmartPtr<T> xnew(const A0& a0, const A1& a1, const A2& a2, const A3& a3, const A4& a4){
	return SmartPtr<T>(SmartPtrSelector<InheritedN<T>::value>(), a0, a1, a2, a3, a4);
}

//@}

//////////////////////////////////////////////////////////////

template<int N, class T>
struct ToSmartPtr{};

template<class T>
struct ToSmartPtr<INHERITED_BASE, T>{
	static const SmartPtr<T>& to(const T* p){
		return *static_cast<SmartPtr<T>*>(static_cast<Any*>(const_cast<T*>(p)));
	}
};

template<class T>
struct ToSmartPtr<INHERITED_RCBASE, T>{
	static const SmartPtr<T>& to(const T* p){
		return *static_cast<SmartPtr<T>*>(static_cast<Any*>(const_cast<T*>(p)));
	}
};

template<class T>
struct ToSmartPtr<INHERITED_ANY, T>{
	static const SmartPtr<T>& to(const T* p){
		return *static_cast<SmartPtr<T>*>(static_cast<Any*>(const_cast<T*>(p)));
	}
};

/**
* \brief T*をSmartPtr<T>に変換する関数
*/
template<class T>
inline const SmartPtr<T>& to_smartptr(const T* p){
	return ToSmartPtr<InheritedN<T>::value, T>::to(p);
}

//////////////////////////////////////////////////////////////

// SmartPtrの重ね着をコンパイルエラーとするための定義
template<class T>
class SmartPtr< SmartPtr<T> >;

///////////////////////////////////////////////////////////////

template<>
struct SmartPtrCtor1<String>{
	typedef const char_t* type;
	static AnyPtr call(type v);
};

template<>
struct SmartPtrCtor2<String>{
	typedef const char8_t* type;
	static AnyPtr call(type v);
};

template<>
struct SmartPtrCtor3<String>{
	typedef const StringLiteral& type;
	static AnyPtr call(type v);
};

template<>
struct SmartPtrCtor1<ID>{
	typedef const char_t* type;
	static AnyPtr call(type v);
};

template<>
struct SmartPtrCtor2<ID>{
	typedef const StringPtr& type;
	static AnyPtr call(type v);
};

template<>
struct SmartPtrCtor3<ID>{
	typedef const char8_t* type;
	static AnyPtr call(type v);
};

template<>
struct SmartPtrCtor4<ID>{
	typedef const StringLiteral& type;
	static AnyPtr call(type v);
};

}

#endif // XTAL_SMARTPTR_H_INCLUDE_GUARD
