
#pragma once

namespace xtal{

template<uint_t Size>
struct UserTypeBuffer{
	union{
		u8 buf[Size];
		int_t dummy;
	};

	template<class T>
	void operator()(T* p){
		p->~T();
	}
};

template<class T>
struct UserTypeHolder : public Base{
	UserTypeHolder(){}
	UserTypeHolder(T* p):ptr(p){}
	T* ptr;
};

template<class T, class Deleter = UserTypeBuffer<sizeof(T)> >
struct UserTypeHolderSub : public UserTypeHolder<T>{
	UserTypeHolderSub(){}
	UserTypeHolderSub(T* p, Deleter f):UserTypeHolder<T>(p), fun(f){}
	virtual ~UserTypeHolderSub(){ fun(this->ptr); }
	Deleter fun;
};

enum InheritedEnum{
	INHERITED_BASE,
	INHERITED_INNOCENCE,
	INHERITED_OTHER
};

template<class T>
struct InheritedN{
	enum{
		value = IsInherited<T, Base>::value ? INHERITED_BASE : 
			IsInherited<T, Any>::value ? INHERITED_INNOCENCE : INHERITED_OTHER
	};
};

template<class T>
struct SmartPtrCtor1{
	struct type{};
	static int_t call(type){ return 0; };
};

template<class T>
struct SmartPtrCtor2{
	struct type{};
	static int_t call(type){ return 0; };
};

template<class T>
struct SmartPtrCtor3{
	struct type{};
	static int_t call(type){ return 0; };
};

template <int N>
struct SmartPtrSelector{};

template<class T>
struct XNewEssence0{};

template<class T, class A0>
struct XNewEssence1{
	A0 a0;
	XNewEssence1(const A0& a0)
		:a0(a0){}
};

template<class T, class A0, class A1>
struct XNewEssence2{
	A0 a0;
	A1 a1;
	XNewEssence2(const A0& a0, const A1& a1)
		:a0(a0), a1(a1){}
};

struct undeleter_t{
	template<class T>
	void operator()(T* p){}
};

struct deleter_t{
	template<class T>
	void operator()(T* p){
		delete p;
	}
};

extern undeleter_t undeleter;
extern deleter_t deleter;

/**
* @brief 何の型のオブジェクトでも保持する特殊化されたスマートポインタ
*/
template<>
class SmartPtr<Any> : public Any{
public:
	
	SmartPtr(){}

	template<class T, class Deleter>
	SmartPtr(T* p, Deleter deleter){
		UserTypeHolderSub<T, Deleter>* holder = new UserTypeHolderSub<T, Deleter>(p, deleter);
		set_p_with_class(holder, new_cpp_class<T>());
	}

	SmartPtr(const SmartPtr<Any>& p)
		:Any(p){
		inc_ref_count();
	}

	SmartPtr<Any>& operator =(const Null&){
		dec_ref_count();
		set_null_force(*this);
		return *this;
	}

	SmartPtr<Any>& operator =(const Undefined&){
		dec_ref_count();
		set_undefined_force(*this);
		return *this;
	}

	SmartPtr<Any>& operator =(const SmartPtr<Any>& p);

	explicit SmartPtr(PrimitiveType type)
		:Any(type){}

	explicit SmartPtr(Base* p)
		:Any(p){
		p->inc_ref_count();
	}

	~SmartPtr(){
		dec_ref_count();
		//*(Any*)this = Any();
	}

protected:

	explicit SmartPtr(const Any& innocence)
		:Any(innocence){}

	struct with_class_t{};

	SmartPtr(Base* p, const ClassPtr& c, with_class_t);

	SmartPtr(Singleton* p, const ClassPtr& c, with_class_t);

	SmartPtr(CppSingleton* p, const ClassPtr& c, with_class_t);

	void set_p_with_class(Base* p, const ClassPtr& c);

	void inc_ref_count(){
		if(type(*this)==TYPE_BASE){
			pvalue(*this)->inc_ref_count();
		}
	}

	void dec_ref_count(){
		if(type(*this)==TYPE_BASE){
			pvalue(*this)->dec_ref_count();
		}
	}

public:

	/**
	* @brief 整数値から構築するコンストラクタ。
	*
	*/
	SmartPtr(int_t v){ set_i(v); }
	
	/**
	* @brief 浮動小数点数値から構築するコンストラクタ。
	*
	*/
	SmartPtr(float_t v){ set_f(v); }
	
	/**
	* @brief 文字列から構築するコンストラクタ。
	*
	*/
	SmartPtr(bool b){ set_b(b); }

	/**
	* @brief 文字列から構築するコンストラクタ。
	*
	*/
	SmartPtr(const char_t* str)
		:Any(str){
		inc_ref_count();
	}

	/**
	* @brief 文字列から構築するコンストラクタ。
	*
	*/
	SmartPtr(const avoid<char>::type* str)
		:Any(str){
		inc_ref_count();
	}

	// 基本型の整数、浮動小数点数から構築するコンストラクタ
	SmartPtr(avoid<int>::type v){ set_i((int_t)v); }
	SmartPtr(avoid<long>::type v){ set_i((int_t)v); }
	SmartPtr(avoid<short>::type v){ set_i((int_t)v); }
	SmartPtr(avoid<char>::type v){ set_i((int_t)v); }
	SmartPtr(avoid<unsigned int>::type v){ set_i((int_t)v); }
	SmartPtr(avoid<unsigned long>::type v){ set_i((int_t)v); }
	SmartPtr(avoid<unsigned short>::type v){ set_i((int_t)v); }
	SmartPtr(avoid<unsigned char>::type v){ set_i((int_t)v); }
	SmartPtr(avoid<signed char>::type v){ set_i((int_t)v); }
	SmartPtr(avoid<float>::type v){ set_f((float_t)v); }
	SmartPtr(avoid<double>::type v){ set_f((float_t)v); }
	SmartPtr(avoid<long double>::type v){ set_f((float_t)v); }

public:

	/**
	* @brief ->演算子
	* スマートポインタとして扱うためにオーバーロードする。
	*/
	Any* operator ->() const{ return get(); }
	
	/**
	* @brief *演算子
	* スマートポインタとして扱うためにオーバーロードする。
	*/
	Any& operator *() const{ return *get(); }
	
	/**
	* @brief T型へのポインタを取得する。
	*/
	Any* get() const{ return (Any*)this; }

public:

	friend inline const AnyPtr& ap(const Any& v){
		return (const AnyPtr&)v;
	}

	friend inline SmartPtr<Any>& ap_copy(SmartPtr<Any>& a, const SmartPtr<Any>& b){
		a.dec_ref_count();
		*(Any*)&a = b;
		a.inc_ref_count();
		return a;
	}

private:

	struct dummy_bool_tag{ void safe_true(dummy_bool_tag){} };
	typedef void (dummy_bool_tag::*safe_bool)(dummy_bool_tag);

public:
	
	/**
	* @brief bool型に自動変換する。
	*
	*/
	operator safe_bool() const{
		return type((*this)->self())>TYPE_FALSE ? &dummy_bool_tag::safe_true : (safe_bool)0;
	}

	bool operator !() const{
		return type((*this)->self())<=TYPE_FALSE;
	}

public:

	template<class U>
	SmartPtr(XNewEssence0<U>)
		:Any(Any::noinit_t()){
		gene<U>(SmartPtrSelector<InheritedN<U>::value>());
	}
	
	template<class U>
	SmartPtr& operator= (XNewEssence0<U>){
		dec_ref_count();
		gene<U>(SmartPtrSelector<InheritedN<U>::value>());
		return *this;
	}

public:

	template<class U, class A0>
	SmartPtr(const XNewEssence1<U, A0>& x)
		:Any(Any::noinit_t()){
		gene<U>(SmartPtrSelector<InheritedN<U>::value>(), x.a0);
	}
	
	template<class U, class A0>
	SmartPtr& operator= (const XNewEssence1<U, A0>& x){
		dec_ref_count();
		gene<U>(SmartPtrSelector<InheritedN<U>::value>(), x.a0);
		return *this;
	}

public:

	template<class U, class A0, class A1>
	SmartPtr(const XNewEssence2<U, A0, A1>& x)
		:Any(Any::noinit_t()){
		gene<U>(SmartPtrSelector<InheritedN<U>::value>(), x.a0, x.a1);
	}
	
	template<class U, class A0, class A1>
	SmartPtr& operator= (const XNewEssence2<U, A0, A1>& x){
		dec_ref_count();
		gene<U>(SmartPtrSelector<InheritedN<U>::value>(), x.a0, x.a1);
		return *this;
	}

protected:

	template<class U>
	void gene(SmartPtrSelector<INHERITED_BASE>){
		set_p_with_class(new U(), new_cpp_class<U>());
	}

	template<class U>
	void gene(SmartPtrSelector<INHERITED_INNOCENCE>){
		Any::operator =(U());
	}

	template<class U>
	void gene(SmartPtrSelector<INHERITED_OTHER>){
		set_p_with_class(new UserTypeHolderSub<U>(), new_cpp_class<U>());
		UserTypeHolderSub<U>* p = ((UserTypeHolderSub<U>*)pvalue(*this));
		p->ptr = (U*)&p->fun;
		new(p->ptr) U;
	}

protected:

	template<class U, class A0>
	void gene(SmartPtrSelector<INHERITED_BASE>, const A0& a0){
		set_p_with_class(new U(a0), new_cpp_class<U>());
	}

	template<class U, class A0>
	void gene(SmartPtrSelector<INHERITED_INNOCENCE>, const A0& a0){
		Any::operator =(U(a0));
	}

	template<class U, class A0>
	void gene(SmartPtrSelector<INHERITED_OTHER>, const A0& a0){
		set_p_with_class(new UserTypeHolderSub<U>(), new_cpp_class<U>());
		UserTypeHolderSub<U>* p = ((UserTypeHolderSub<U>*)pvalue(*this));
		p->ptr = (U*)&p->fun;
		new(p->ptr) U(a0);
	}

protected:

	template<class U, class A0, class A1>
	void gene(SmartPtrSelector<INHERITED_BASE>, const A0& a0, const A1& a1){
		set_p_with_class(new U(a0, a1), new_cpp_class<U>());
	}

	template<class U, class A0, class A1>
	void gene(SmartPtrSelector<INHERITED_INNOCENCE>, const A0& a0, const A1& a1){
		Any::operator =(U(a0, a1));
	}

	template<class U, class A0, class A1>
	void gene(SmartPtrSelector<INHERITED_OTHER>, const A0& a0, const A1& a1){
		set_p_with_class(new UserTypeHolderSub<U>(), new_cpp_class<U>());
		UserTypeHolderSub<U>* p = ((UserTypeHolderSub<U>*)pvalue(*this));
		p->ptr = (U*)&p->fun;
		new(p->ptr) U(a0, a1);
	}

private:

	/**
	* @brief 暗黙の変換を抑えるためのコンストラクタ。
	*
	* 得体の知れないポインタからの構築を拒否するため、このコンストラクタはprivateで実装も存在しない。
	*/
	SmartPtr(void*);

};


class Null : public AnyPtr{};
class Undefined : public AnyPtr{ public: Undefined():AnyPtr(TYPE_UNDEFINED){} };

inline bool operator ==(const AnyPtr& a, const Null&){ return raweq(a, null); }
inline bool operator !=(const AnyPtr& a, const Null&){ return rawne(a, null); }
inline bool operator ==(const Null&, const AnyPtr& a){ return raweq(a, null); }
inline bool operator !=(const Null&, const AnyPtr& a){ return rawne(a, null); }

inline bool operator ==(const AnyPtr& a, const Undefined&){ return raweq(a, undefined); }
inline bool operator !=(const AnyPtr& a, const Undefined&){ return rawne(a, undefined); }
inline bool operator ==(const Undefined&, const AnyPtr& a){ return raweq(a, undefined); }
inline bool operator !=(const Undefined&, const AnyPtr& a){ return rawne(a, undefined); }


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
		T* n = (U*)0; // inherited test
	}

	SmartPtr(const SmartPtr<T>& p)
		:SmartPtr<Any>(p){}

	SmartPtr<T>& operator =(const Null&){
		dec_ref_count();
		set_null_force(*this);
		return *this;
	}

	/// nullを受け取るコンストラクタ
	SmartPtr(const Null&){}

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

	SmartPtr(SmartPtrSelector<INHERITED_INNOCENCE>, T* p){ 
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
	SmartPtr(const AnyPtr&);

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
	* この三つの方法のどれかをする必要がある。
	*/
	SmartPtr(void*);
	SmartPtr(SmartPtrSelector<INHERITED_OTHER>, T* p);

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

private:

	T* get2(SmartPtrSelector<INHERITED_BASE>) const{ 
		XTAL_ASSERT(type(*this)!=TYPE_NULL); // このアサーションで止まる場合、nullポインタが格納されている
		return (T*)pvalue(*this); 
	}

	T* get2(SmartPtrSelector<INHERITED_INNOCENCE>) const{ 
		return (T*)this; 
	}

	T* get2(SmartPtrSelector<INHERITED_OTHER>) const{ 
		return (T*)((UserTypeHolder<T>*)pvalue(*this))->ptr; 
	}

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

	SmartPtr(SmartPtrSelector<INHERITED_INNOCENCE>)
		:SmartPtr<Any>(T()){}

	SmartPtr(SmartPtrSelector<INHERITED_OTHER>)
		:SmartPtr<Any>(new UserTypeHolderSub<T>(), new_cpp_class<T>(), with_class_t()){
		UserTypeHolderSub<T>* p = ((UserTypeHolderSub<T>*)pvalue(*this));
		p->ptr = (T*)&p->fun;
		new(p->ptr) T;
	}

/////////////////////

	template<class A0>
	SmartPtr(SmartPtrSelector<INHERITED_BASE>, const A0& a0)
		:SmartPtr<Any>(new T(a0), new_cpp_class<T>(), with_class_t()){}

	template<class A0>
	SmartPtr(SmartPtrSelector<INHERITED_INNOCENCE>, const A0& a0)
		:SmartPtr<Any>(T(a0)){}

	template<class A0>
	SmartPtr(SmartPtrSelector<INHERITED_OTHER>, const A0& a0)
		:SmartPtr<Any>(new UserTypeHolderSub<T>(), new_cpp_class<T>(), with_class_t()){
		UserTypeHolderSub<T>* p = ((UserTypeHolderSub<T>*)pvalue(*this));
		p->ptr = (T*)&p->fun;
		new(p->ptr) T(a0);
	}

/////////////////////

	template<class A0, class A1>
	SmartPtr(SmartPtrSelector<INHERITED_BASE>, const A0& a0, const A1& a1)
		:SmartPtr<Any>(new T(a0, a1), new_cpp_class<T>(), with_class_t()){}

	template<class A0, class A1>
	SmartPtr(SmartPtrSelector<INHERITED_INNOCENCE>, const A0& a0, const A1& a1)
		:SmartPtr<Any>(T(a0, a1)){}

	template<class A0, class A1>
	SmartPtr(SmartPtrSelector<INHERITED_OTHER>, const A0& a0, const A1& a1)
		:SmartPtr<Any>(new UserTypeHolderSub<T>(), new_cpp_class<T>(), with_class_t()){
		UserTypeHolderSub<T>* p = ((UserTypeHolderSub<T>*)pvalue(*this));
		p->ptr = (T*)&p->fun;
		new(p->ptr) T(a0, a1);
	}

/////////////////////

	template<class A0, class A1, class A2>
	SmartPtr(SmartPtrSelector<INHERITED_BASE>, const A0& a0, const A1& a1, const A2& a2)
		:SmartPtr<Any>(new T(a0, a1, a2), new_cpp_class<T>(), with_class_t()){}

	template<class A0, class A1, class A2>
	SmartPtr(SmartPtrSelector<INHERITED_INNOCENCE>, const A0& a0, const A1& a1, const A2& a2)
		:SmartPtr<Any>(T(a0, a1, a2)){}

	template<class A0, class A1, class A2>
	SmartPtr(SmartPtrSelector<INHERITED_OTHER>, const A0& a0, const A1& a1, const A2& a2)
		:SmartPtr<Any>(new UserTypeHolderSub<T>(), new_cpp_class<T>(), with_class_t()){
		UserTypeHolderSub<T>* p = ((UserTypeHolderSub<T>*)pvalue(*this));
		p->ptr = (T*)&p->fun;
		new(p->ptr) T(a0, a1, a2);
	}

/////////////////////

	template<class A0, class A1, class A2, class A3>
	SmartPtr(SmartPtrSelector<INHERITED_BASE>, const A0& a0, const A1& a1, const A2& a2, const A3& a3)
		:SmartPtr<Any>(new T(a0, a1, a2, a3), new_cpp_class<T>(), with_class_t()){}

	template<class A0, class A1, class A2, class A3>
	SmartPtr(SmartPtrSelector<INHERITED_INNOCENCE>, const A0& a0, const A1& a1, const A2& a2, const A3& a3)
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
	SmartPtr(SmartPtrSelector<INHERITED_INNOCENCE>, const A0& a0, const A1& a1, const A2& a2, const A3& a3, const A4& a4)
		:SmartPtr<Any>(T(a0, a1, a2, a3, a4)){}

	template<class A0, class A1, class A2, class A3, class A4>
	SmartPtr(SmartPtrSelector<INHERITED_OTHER>, const A0& a0, const A1& a1, const A2& a2, const A3& a3, const A4& a4)
		:SmartPtr<Any>(new UserTypeHolderSub<T>(), new_cpp_class<T>(), with_class_t()){
		UserTypeHolderSub<T>* p = ((UserTypeHolderSub<T>*)pvalue(*this));
		p->ptr = (T*)&p->fun;
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
	return *(SmartPtr<T>*)(Any*)p;
}

template<class T>
inline const SmartPtr<T>& from_this(SmartPtrSelector<INHERITED_INNOCENCE>, const T* p){
	return *(SmartPtr<T>*)(Any*)p; 
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

inline void inc_ref_count_force(const Any& v){
	if(type(v)==TYPE_BASE){
		pvalue(v)->inc_ref_count();
	}
}

inline void dec_ref_count_force(const Any& v){
	if(type(v)==TYPE_BASE){
		pvalue(v)->dec_ref_count();
	}
}


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

template<class Key, class T, class Pr, class A>
void visit_members(Visitor& m, const std::map<Key, T, Pr, A>& value){
	std::for_each(value.begin(), value.end(), m);
}

template<class T, class A>
void visit_members(Visitor& m, const std::vector<T, A>& value){
	std::for_each(value.begin(), value.end(), m);
}

template<class T, class A>
void visit_members(Visitor& m, const std::deque<T, A>& value){
	std::for_each(value.begin(), value.end(), m);
}

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


class HaveName : public Base{
public:

	HaveName()
		:name_(null), force_(0){}

	virtual StringPtr object_name(int_t depth = -1);

	virtual int_t object_name_force();

	virtual void set_object_name(const StringPtr& name, int_t force, const AnyPtr& parent);

protected:

	StringPtr name_;
	AnyPtr parent_;
	int_t force_;

	virtual void visit_members(Visitor& m){
		Base::visit_members(m);
		m & name_ & parent_;
	}	
};

}
