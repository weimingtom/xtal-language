
#pragma once

namespace xtal{

/**
* @brief T型へのポインタを保持するためのスマートポインタ
*/
template<class T>
class SmartPtr : public SmartPtr<Any>{
public:
	
	SmartPtr(){}

	template<class Deleter>
	SmartPtr(T* p, Deleter deleter)
		:SmartPtr<Any>(p, deleter){}

	template<class U>
	SmartPtr(const SmartPtr<U>& p)
		:SmartPtr<Any>(p){

		// 継承関係をここでチェックしている。
		// ここでコンパイルエラーになる場合、ptr_cast関数、ptr_as関数等を使用して変換する必要がある。
		T* n = (U*)0; 
	}

	//SmartPtr(const SmartPtr<T>& p)
	//	:SmartPtr<Any>(p){}

	SmartPtr<T>& operator =(const Null&){
		dec_ref_count();
		set_null_force(*this);
		return *this;
	}

	/// nullを受け取るコンストラクタ
	//SmartPtr(const Null&){}

	/// 特別なコンストラクタ1
	SmartPtr(typename SmartPtrCtor1<T>::type v);

	/// 特別なコンストラクタ2
	SmartPtr(typename SmartPtrCtor2<T>::type v);

	/// 特別なコンストラクタ3
	SmartPtr(typename SmartPtrCtor3<T>::type v);

public:

	SmartPtr(SmartPtrSelector<INHERITED_BASE>, T* p){ 
		set_p((Base*)p); 
		p->inc_ref_count();
	}

	SmartPtr(SmartPtrSelector<INHERITED_ANY>, T* p){ 
		*(Any*)this = *(Any*)p; 
		inc_ref_count();
	}

private:

	/**
	* @brief 暗黙の変換を抑えるためのコンストラクタ。
	*
	* AnyPtrからの暗黙の変換を拒否するために、privateで定義されている。
	* AnyPtrからSmartPtr<T>に変換するにはptr_cast関数、ptr_as関数を使用すること。
	*/
	//SmartPtr(const AnyPtr&);

	/**
	* @brief 暗黙の変換を抑えるためのコンストラクタ。
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
	SmartPtr(SmartPtrSelector<INHERITED_OTHER>, T* p);

private:

	T* get2(SmartPtrSelector<INHERITED_BASE>) const{ 
		XTAL_ASSERT(type(*this)!=TYPE_NULL); // このアサーションで止まる場合、nullポインタが格納されている
		return (T*)pvalue(*this); 
	}

	T* get2(SmartPtrSelector<INHERITED_ANY>) const{ 
		return (T*)this; 
	}

	T* get2(SmartPtrSelector<INHERITED_OTHER>) const{ 
		return (T*)((UserTypeHolder<T>*)pvalue(*this))->ptr; 
	}

public:

	/**
	* @brief ->演算子
	* スマートポインタとして扱うために。
	*/
	T* operator ->() const{ return get(); }
	
	/**
	* @brief *演算子
	* スマートポインタとして扱うために。
	*/
	T& operator *() const{ return *get(); }
	
	/**
	* @brief T型へのポインタを取得する。
	*/
	T* get() const{ return get2(SmartPtrSelector<InheritedN<T>::value>()); }

public:

	template<class U>
	SmartPtr(XNewEssence0<U> x)
		:SmartPtr<Any>(x){
		T* n = (U*)0; // inherited test
	}
	
	template<class U>
	SmartPtr& operator= (XNewEssence0<U> x){
		T* n = (U*)0; // inherited test
		SmartPtr<Any>::operator =(x);
		return *this;
	}

public:

	template<class U, class A0>
	SmartPtr(const XNewEssence1<U, A0>& x)
		:SmartPtr<Any>(x){
		T* n = (U*)0; // inherited test
	}
	
	template<class U, class A0>
	SmartPtr& operator= (const XNewEssence1<U, A0>& x){
		T* n = (U*)0; // inherited test
		SmartPtr<Any>::operator =(x);
		return *this;
	}

public:

	template<class U, class A0, class A1>
	SmartPtr(const XNewEssence2<U, A0, A1>& x)
		:SmartPtr<Any>(x){
		T* n = (U*)0; // inherited test
	}
	
	template<class U, class A0, class A1>
	SmartPtr& operator= (const XNewEssence2<U, A0, A1>& x){
		T* n = (U*)0; // inherited test
		SmartPtr<Any>::operator =(x);
		return *this;
	}

public:

	SmartPtr(SmartPtrSelector<INHERITED_BASE>)
		:SmartPtr<Any>(new T(), new_cpp_class<T>(), with_class_t()){}

	SmartPtr(SmartPtrSelector<INHERITED_ANY>)
		:SmartPtr<Any>(T()){}

	SmartPtr(SmartPtrSelector<INHERITED_OTHER>)
		:SmartPtr<Any>(new UserTypeHolderSub<T>(), new_cpp_class<T>(), with_class_t()){
		UserTypeHolderSub<T>* p = ((UserTypeHolderSub<T>*)pvalue(*this));
		p->ptr = (T*)p->buf;
		new(p->ptr) T;
	}

/////////////////////

	template<class A0>
	SmartPtr(SmartPtrSelector<INHERITED_BASE>, const A0& a0)
		:SmartPtr<Any>(new T(a0), new_cpp_class<T>(), with_class_t()){}

	template<class A0>
	SmartPtr(SmartPtrSelector<INHERITED_ANY>, const A0& a0)
		:SmartPtr<Any>(T(a0)){}

	template<class A0>
	SmartPtr(SmartPtrSelector<INHERITED_OTHER>, const A0& a0)
		:SmartPtr<Any>(new UserTypeHolderSub<T>(), new_cpp_class<T>(), with_class_t()){
		UserTypeHolderSub<T>* p = ((UserTypeHolderSub<T>*)pvalue(*this));
		p->ptr = (T*)p->buf;
		new(p->ptr) T(a0);
	}

/////////////////////

	template<class A0, class A1>
	SmartPtr(SmartPtrSelector<INHERITED_BASE>, const A0& a0, const A1& a1)
		:SmartPtr<Any>(new T(a0, a1), new_cpp_class<T>(), with_class_t()){}

	template<class A0, class A1>
	SmartPtr(SmartPtrSelector<INHERITED_ANY>, const A0& a0, const A1& a1)
		:SmartPtr<Any>(T(a0, a1)){}

	template<class A0, class A1>
	SmartPtr(SmartPtrSelector<INHERITED_OTHER>, const A0& a0, const A1& a1)
		:SmartPtr<Any>(new UserTypeHolderSub<T>(), new_cpp_class<T>(), with_class_t()){
		UserTypeHolderSub<T>* p = ((UserTypeHolderSub<T>*)pvalue(*this));
		p->ptr = (T*)p->buf;
		new(p->ptr) T(a0, a1);
	}

/////////////////////

	template<class A0, class A1, class A2>
	SmartPtr(SmartPtrSelector<INHERITED_BASE>, const A0& a0, const A1& a1, const A2& a2)
		:SmartPtr<Any>(new T(a0, a1, a2), new_cpp_class<T>(), with_class_t()){}

	template<class A0, class A1, class A2>
	SmartPtr(SmartPtrSelector<INHERITED_ANY>, const A0& a0, const A1& a1, const A2& a2)
		:SmartPtr<Any>(T(a0, a1, a2)){}

	template<class A0, class A1, class A2>
	SmartPtr(SmartPtrSelector<INHERITED_OTHER>, const A0& a0, const A1& a1, const A2& a2)
		:SmartPtr<Any>(new UserTypeHolderSub<T>(), new_cpp_class<T>(), with_class_t()){
		UserTypeHolderSub<T>* p = ((UserTypeHolderSub<T>*)pvalue(*this));
		p->ptr = (T*)p->buf;
		new(p->ptr) T(a0, a1, a2);
	}

/////////////////////

	template<class A0, class A1, class A2, class A3>
	SmartPtr(SmartPtrSelector<INHERITED_BASE>, const A0& a0, const A1& a1, const A2& a2, const A3& a3)
		:SmartPtr<Any>(new T(a0, a1, a2, a3), new_cpp_class<T>(), with_class_t()){}

	template<class A0, class A1, class A2, class A3>
	SmartPtr(SmartPtrSelector<INHERITED_ANY>, const A0& a0, const A1& a1, const A2& a2, const A3& a3)
		:SmartPtr<Any>(T(a0, a1, a2, a3)){}

	template<class A0, class A1, class A2, class A3>
	SmartPtr(SmartPtrSelector<INHERITED_OTHER>, const A0& a0, const A1& a1, const A2& a2, const A3& a3)
		:SmartPtr<Any>(new UserTypeHolderSub<T>(), new_cpp_class<T>(), with_class_t()){
		UserTypeHolderSub<T>* p = ((UserTypeHolderSub<T>*)pvalue(*this));
		p->ptr = (T*)&p->fun;
		new(p->ptr) T(a0, a1, a2, a3);
	}

/////////////////////

	template<class A0, class A1, class A2, class A3, class A4>
	SmartPtr(SmartPtrSelector<INHERITED_BASE>, const A0& a0, const A1& a1, const A2& a2, const A3& a3, const A4& a4)
		:SmartPtr<Any>(new T(a0, a1, a2, a3, a4), new_cpp_class<T>(), with_class_t()){}

	template<class A0, class A1, class A2, class A3, class A4>
	SmartPtr(SmartPtrSelector<INHERITED_ANY>, const A0& a0, const A1& a1, const A2& a2, const A3& a3, const A4& a4)
		:SmartPtr<Any>(T(a0, a1, a2, a3, a4)){}

	template<class A0, class A1, class A2, class A3, class A4>
	SmartPtr(SmartPtrSelector<INHERITED_OTHER>, const A0& a0, const A1& a1, const A2& a2, const A3& a3, const A4& a4)
		:SmartPtr<Any>(new UserTypeHolderSub<T>(), new_cpp_class<T>(), with_class_t()){
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

/**
* @brief Tオブジェクトを生成する
*/
template<class T>
inline SmartPtr<T> xnew(){
	return SmartPtr<T>(SmartPtrSelector<InheritedN<T>::value>());
}

/**
* @brief Tオブジェクトを生成する
*/
template<class T, class A0>
inline SmartPtr<T> xnew(const A0& a0){
	return SmartPtr<T>(SmartPtrSelector<InheritedN<T>::value>(), a0);
}

/**
* @brief Tオブジェクトを生成する
*/
template<class T, class A0, class A1>
inline SmartPtr<T> xnew(const A0& a0, const A1& a1){
	return SmartPtr<T>(SmartPtrSelector<InheritedN<T>::value>(), a0, a1);
}

/**
* @brief Tオブジェクトを生成する
*/
template<class T, class A0, class A1, class A2>
inline SmartPtr<T> xnew(const A0& a0, const A1& a1, const A2& a2){
	return SmartPtr<T>(SmartPtrSelector<InheritedN<T>::value>(), a0, a1, a2);
}

/**
* @brief Tオブジェクトを生成する
*/
template<class T, class A0, class A1, class A2, class A3>
inline SmartPtr<T> xnew(const A0& a0, const A1& a1, const A2& a2, const A3& a3){
	return SmartPtr<T>(SmartPtrSelector<InheritedN<T>::value>(), a0, a1, a2, a3);
}

/**
* @brief Tオブジェクトを生成する
*/
template<class T, class A0, class A1, class A2, class A3, class A4>
inline SmartPtr<T> xnew(const A0& a0, const A1& a1, const A2& a2, const A3& a3, const A4& a4){
	return SmartPtr<T>(SmartPtrSelector<InheritedN<T>::value>(), a0, a1, a2, a3, a4);
}

//////////////////////////////////////////////////////////////

/**
* @brief Tオブジェクトを生成する
*/
template<class T>
inline XNewEssence0<T> xnew_lazy(){
	return XNewEssence0<T>();
}

/**
* @brief Tオブジェクトを生成する
*/
template<class T, class A0>
inline XNewEssence1<T, A0> xnew_lazy(const A0& a0){
	return XNewEssence1<T, A0>(a0);
}

/**
* @brief Tオブジェクトを生成する
*/
template<class T, class A0, class A1>
inline XNewEssence2<T, A0, A1> xnew_lazy(const A0& a0, const A1& a1){
	return XNewEssence2<T, A0, A1>(a0, a1);
}

//////////////////////////////////////////////////////////////

template<class T>
inline const SmartPtr<T>& from_this(SmartPtrSelector<INHERITED_BASE>, const T* p){
	return *static_cast<SmartPtr<T>*>(static_cast<Any*>(const_cast<T*>(p)));
}

template<class T>
inline const SmartPtr<T>& from_this(SmartPtrSelector<INHERITED_ANY>, const T* p){
	return *static_cast<SmartPtr<T>*>(static_cast<Any*>(const_cast<T*>(p)));
}

/**
* @brief thisポインタをSmartPtr<T>に変換する関数
*/
template<class T>
inline const SmartPtr<T>& from_this(const T* p){
	return from_this(SmartPtrSelector<InheritedN<T>::value>(), p);
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
	typedef const avoid<char>::type* type;
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
	typedef const avoid<char>::type* type;
	static AnyPtr call(type v);
};

void visit_members(Visitor& m, const AnyPtr& p);

class Visitor{
	int_t value_;
public:	

	Visitor(int_t value){
		value_ = value;
	}

	int_t value(){
		return value_;
	}

	template<class T> Visitor operator &(const T& value){
		visit_members(*this, value);
		return *this;
	}

	template<class T> Visitor operator ()(const T& value){
		visit_members(*this, value);
		return *this;
	}
};

template<class F, class S>
void visit_members(Visitor& m, const std::pair<F, S>& value){
	m & value.first & value.second;
}

inline void visit_members(Visitor& m, Base* p){
	m & ap(Any(p));
}

inline void visit_members(Visitor& m, const Any& p){
	m & ap(p);
}

}
