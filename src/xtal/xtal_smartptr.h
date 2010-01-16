/** \file src/xtal/xtal_smartptr.h
* \brief src/xtal/xtal_smartptr.h
*/

#ifndef XTAL_SMARTPTR_H_INCLUDE_GUARD
#define XTAL_SMARTPTR_H_INCLUDE_GUARD

#pragma once

namespace xtal{

/**
* \brief T型へのポインタを保持するためのスマートポインタ
*/
template<class T>
class SmartPtr : public SmartPtr<Any>{
public:

	SmartPtr(){}

	template<class Deleter>
	SmartPtr(const T* p, const Deleter& deleter)
		:SmartPtr<Any>((T*)p, deleter){}

	SmartPtr(const T* p)
		:SmartPtr<Any>(p){}

public:

	/// nullを受け取るコンストラクタ
	SmartPtr(const NullPtr&){}

	SmartPtr<T>& operator =(const SmartPtr<Null>& null){
		SmartPtr<Any>::operator =(null);
		return *this;
	}

public:

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
	SmartPtr(const XNew<U>& m)
		:SmartPtr<Any>(m){
		// 継承関係をここでチェックしている。
		// ここでコンパイルエラーになる場合、
		// ptr_cast関数等を使用して型を変換する必要がある。
		T* n = (U*)0; 
		XTAL_UNUSED_VAR(n);	
	}

	template<class U>
	SmartPtr<T>& operator =(const XNew<U>& p){
		// 継承関係をここでチェックしている。
		// ここでコンパイルエラーになる場合、
		// ptr_cast関数等を使用して型を変換する必要がある。
		T* n = (U*)0; 
		XTAL_UNUSED_VAR(n);
		
		SmartPtr<Any>::operator =(p);
		return *this;
	}


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
	T* get() const{ return Extract<InheritedN<T>::value, T>::extract(*this); }

	/**
	* \brief ->演算子
	* スマートポインタとして振舞うために。
	*/
	T* operator ->() const{ return get(); }
	
	/**
	* \brief *演算子
	* スマートポインタとして振舞うために。
	*/
	T& operator *() const{ return *get(); }
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

template<class T >
XNew<T> xxnew(){
	XNew<T> n;
	new(n.ptr()) T;
	return n;
}

template<class T >
inline SmartPtr<T> xnew(){
	return xxnew<T>();
}

//{REPEAT{{
/*
template<class T, class A0 #COMMA_REPEAT#class A`i+1`#>
XNew<T> xxnew(const A0& a0 #COMMA_REPEAT#const A`i+1`& a`i+1`#){
	XNew<T> n;
	new(n.ptr()) T(a0 #COMMA_REPEAT#a`i+1`#);
	return n;
}

template<class T, class A0 #COMMA_REPEAT#class A`i+1`#>
inline SmartPtr<T> xnew(const A0& a0 #COMMA_REPEAT#const A`i+1`& a`i+1`#){
	return xxnew<T>(a0 #COMMA_REPEAT#a`i+1`#);
}

*/

template<class T, class A0 >
XNew<T> xxnew(const A0& a0 ){
	XNew<T> n;
	new(n.ptr()) T(a0 );
	return n;
}

template<class T, class A0 >
inline SmartPtr<T> xnew(const A0& a0 ){
	return xxnew<T>(a0 );
}


template<class T, class A0 , class A1>
XNew<T> xxnew(const A0& a0 , const A1& a1){
	XNew<T> n;
	new(n.ptr()) T(a0 , a1);
	return n;
}

template<class T, class A0 , class A1>
inline SmartPtr<T> xnew(const A0& a0 , const A1& a1){
	return xxnew<T>(a0 , a1);
}


template<class T, class A0 , class A1, class A2>
XNew<T> xxnew(const A0& a0 , const A1& a1, const A2& a2){
	XNew<T> n;
	new(n.ptr()) T(a0 , a1, a2);
	return n;
}

template<class T, class A0 , class A1, class A2>
inline SmartPtr<T> xnew(const A0& a0 , const A1& a1, const A2& a2){
	return xxnew<T>(a0 , a1, a2);
}


template<class T, class A0 , class A1, class A2, class A3>
XNew<T> xxnew(const A0& a0 , const A1& a1, const A2& a2, const A3& a3){
	XNew<T> n;
	new(n.ptr()) T(a0 , a1, a2, a3);
	return n;
}

template<class T, class A0 , class A1, class A2, class A3>
inline SmartPtr<T> xnew(const A0& a0 , const A1& a1, const A2& a2, const A3& a3){
	return xxnew<T>(a0 , a1, a2, a3);
}


template<class T, class A0 , class A1, class A2, class A3, class A4>
XNew<T> xxnew(const A0& a0 , const A1& a1, const A2& a2, const A3& a3, const A4& a4){
	XNew<T> n;
	new(n.ptr()) T(a0 , a1, a2, a3, a4);
	return n;
}

template<class T, class A0 , class A1, class A2, class A3, class A4>
inline SmartPtr<T> xnew(const A0& a0 , const A1& a1, const A2& a2, const A3& a3, const A4& a4){
	return xxnew<T>(a0 , a1, a2, a3, a4);
}


template<class T, class A0 , class A1, class A2, class A3, class A4, class A5>
XNew<T> xxnew(const A0& a0 , const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5){
	XNew<T> n;
	new(n.ptr()) T(a0 , a1, a2, a3, a4, a5);
	return n;
}

template<class T, class A0 , class A1, class A2, class A3, class A4, class A5>
inline SmartPtr<T> xnew(const A0& a0 , const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5){
	return xxnew<T>(a0 , a1, a2, a3, a4, a5);
}


template<class T, class A0 , class A1, class A2, class A3, class A4, class A5, class A6>
XNew<T> xxnew(const A0& a0 , const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6){
	XNew<T> n;
	new(n.ptr()) T(a0 , a1, a2, a3, a4, a5, a6);
	return n;
}

template<class T, class A0 , class A1, class A2, class A3, class A4, class A5, class A6>
inline SmartPtr<T> xnew(const A0& a0 , const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6){
	return xxnew<T>(a0 , a1, a2, a3, a4, a5, a6);
}


template<class T, class A0 , class A1, class A2, class A3, class A4, class A5, class A6, class A7>
XNew<T> xxnew(const A0& a0 , const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7){
	XNew<T> n;
	new(n.ptr()) T(a0 , a1, a2, a3, a4, a5, a6, a7);
	return n;
}

template<class T, class A0 , class A1, class A2, class A3, class A4, class A5, class A6, class A7>
inline SmartPtr<T> xnew(const A0& a0 , const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7){
	return xxnew<T>(a0 , a1, a2, a3, a4, a5, a6, a7);
}


template<class T, class A0 , class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8>
XNew<T> xxnew(const A0& a0 , const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8){
	XNew<T> n;
	new(n.ptr()) T(a0 , a1, a2, a3, a4, a5, a6, a7, a8);
	return n;
}

template<class T, class A0 , class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8>
inline SmartPtr<T> xnew(const A0& a0 , const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8){
	return xxnew<T>(a0 , a1, a2, a3, a4, a5, a6, a7, a8);
}


template<class T, class A0 , class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9>
XNew<T> xxnew(const A0& a0 , const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8, const A9& a9){
	XNew<T> n;
	new(n.ptr()) T(a0 , a1, a2, a3, a4, a5, a6, a7, a8, a9);
	return n;
}

template<class T, class A0 , class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9>
inline SmartPtr<T> xnew(const A0& a0 , const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8, const A9& a9){
	return xxnew<T>(a0 , a1, a2, a3, a4, a5, a6, a7, a8, a9);
}


template<class T, class A0 , class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10>
XNew<T> xxnew(const A0& a0 , const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8, const A9& a9, const A10& a10){
	XNew<T> n;
	new(n.ptr()) T(a0 , a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
	return n;
}

template<class T, class A0 , class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10>
inline SmartPtr<T> xnew(const A0& a0 , const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8, const A9& a9, const A10& a10){
	return xxnew<T>(a0 , a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
}


template<class T, class A0 , class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11>
XNew<T> xxnew(const A0& a0 , const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8, const A9& a9, const A10& a10, const A11& a11){
	XNew<T> n;
	new(n.ptr()) T(a0 , a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11);
	return n;
}

template<class T, class A0 , class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11>
inline SmartPtr<T> xnew(const A0& a0 , const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8, const A9& a9, const A10& a10, const A11& a11){
	return xxnew<T>(a0 , a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11);
}


template<class T, class A0 , class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12>
XNew<T> xxnew(const A0& a0 , const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8, const A9& a9, const A10& a10, const A11& a11, const A12& a12){
	XNew<T> n;
	new(n.ptr()) T(a0 , a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12);
	return n;
}

template<class T, class A0 , class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12>
inline SmartPtr<T> xnew(const A0& a0 , const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8, const A9& a9, const A10& a10, const A11& a11, const A12& a12){
	return xxnew<T>(a0 , a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12);
}


template<class T, class A0 , class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13>
XNew<T> xxnew(const A0& a0 , const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8, const A9& a9, const A10& a10, const A11& a11, const A12& a12, const A13& a13){
	XNew<T> n;
	new(n.ptr()) T(a0 , a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13);
	return n;
}

template<class T, class A0 , class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13>
inline SmartPtr<T> xnew(const A0& a0 , const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8, const A9& a9, const A10& a10, const A11& a11, const A12& a12, const A13& a13){
	return xxnew<T>(a0 , a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13);
}


//}}REPEAT}

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
* \brief T*をconst SmartPtr<T>&に変換する関数
*/
template<class T>
inline const SmartPtr<T>& to_smartptr(const T* p){
	return ToSmartPtr<InheritedN<T>::value, T>::to(p);
}

/**
* \brief SmartPtr<T>型のnullを取得する関数
*/
template<class T>
inline const SmartPtr<T>& nul(){
	return *reinterpret_cast<SmartPtr<T>*>(&null);
}

//////////////////////////////////////////////////////////////

// SmartPtrの重ね着をコンパイルエラーとするための定義
template<class T>
class SmartPtr< SmartPtr<T> >;

///////////////////////////////////////////////////////////////

template<>
struct SmartPtrCtor1<String>{
	typedef const char_t* type;
	static StringPtr call(type v);
};

template<>
struct SmartPtrCtor2<String>{
	typedef const char8_t* type;
	static StringPtr call(type v);
};

template<>
struct SmartPtrCtor3<String>{
	typedef const StringLiteral& type;
	static StringPtr call(type v);
};

template<>
struct SmartPtrCtor1<ID>{
	typedef const char_t* type;
	static IDPtr call(type v);
};

template<>
struct SmartPtrCtor2<ID>{
	typedef const StringPtr& type;
	static IDPtr call(type v);
};

template<>
struct SmartPtrCtor3<ID>{
	typedef const char8_t* type;
	static IDPtr call(type v);
};

template<>
struct SmartPtrCtor4<ID>{
	typedef const StringLiteral& type;
	static IDPtr call(type v);
};

}

#endif // XTAL_SMARTPTR_H_INCLUDE_GUARD
