
#pragma once

namespace xtal{

template<class T>
class TBase : public Base{
	virtual ~TBase(){ ((T*)(this+1))->~T(); }
};

enum{
	INHERITED_BASE,
	INHERITED_INNOCENCE,
	INHERITED_HAVE_ORIGINAL_CLASS,
	INHERITED_OTHER,
};

template<class T>
struct InheritedN{
	enum{
		value = IsInherited<T, Base>::value ? INHERITED_BASE : 
			IsInherited<T, Innocence>::value ? INHERITED_INNOCENCE : INHERITED_OTHER
	};
};


/**
* @brief 何の型のオブジェクトでも保持する特殊化されたスマートポインタ
*/
template<>
class SmartPtr<Any> : public Innocence{
public:
	
	SmartPtr(){}

	SmartPtr(const SmartPtr<Any>& p)
		:Innocence(p){
		inc_ref_count();
	}

	SmartPtr<Any>& operator =(const SmartPtr<Any>& p){
		dec_ref_count();
		*(Innocence*)this = p;
		inc_ref_count();
		return *this;
	}

	SmartPtr(const Null&){}

	explicit SmartPtr(PrimitiveType type)
		:Innocence(type){}

	explicit SmartPtr(Base* p)
		:Innocence(p){
		inc_ref_count();
	}

	~SmartPtr(){
		dec_ref_count();
		*(Innocence*)this = Innocence();
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
	SmartPtr(const char* str)
		:Innocence(str){
		inc_ref_count();
	}


	// 基本型の整数、浮動小数点数から構築するコンストラクタ
	SmartPtr(check_xtype<int>::type v){ set_i((int_t)v); }
	SmartPtr(check_xtype<long>::type v){ set_i((int_t)v); }
	SmartPtr(check_xtype<short>::type v){ set_i((int_t)v); }
	SmartPtr(check_xtype<char>::type v){ set_i((int_t)v); }
	SmartPtr(check_xtype<unsigned int>::type v){ set_i((int_t)v); }
	SmartPtr(check_xtype<unsigned long>::type v){ set_i((int_t)v); }
	SmartPtr(check_xtype<unsigned short>::type v){ set_i((int_t)v); }
	SmartPtr(check_xtype<unsigned char>::type v){ set_i((int_t)v); }
	SmartPtr(check_xtype<signed char>::type v){ set_i((int_t)v); }
	SmartPtr(check_xtype<float>::type v){ set_f((float_t)v); }
	SmartPtr(check_xtype<double>::type v){ set_f((float_t)v); }
	SmartPtr(check_xtype<long double>::type v){ set_f((float_t)v); }

private:

	/**
	* @brief bool値を'true'または'false'に強制的に設定します、を拒否するコンストラクタ
	*
	* このコンストラクタはprivateである。
	*/
	SmartPtr(void*);

protected:

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

private:

	struct dummy_bool_tag{ void safe_true(dummy_bool_tag){} };
	typedef void (dummy_bool_tag::*safe_bool)(dummy_bool_tag);

public:
	
	/**
	* @brief bool型に自動変換する。
	*
	*/
	operator safe_bool() const{
		return type(*this)>TYPE_FALSE ? &dummy_bool_tag::safe_true : (safe_bool)0;
	}

	bool operator !() const{
		return type(*this)<=TYPE_FALSE;
	}

};

inline const AnyPtr& ap(const Innocence& v){
	return (const AnyPtr&)v;
}

class Null : public AnyPtr{};
class Nop : public AnyPtr{public: Nop():AnyPtr(TYPE_NOP){} };

inline bool operator ==(const AnyPtr& a, const Null&){ return raweq(a, null); }
inline bool operator !=(const AnyPtr& a, const Null&){ return rawne(a, null); }
inline bool operator ==(const Null&, const AnyPtr& a){ return raweq(a, null); }
inline bool operator !=(const Null&, const AnyPtr& a){ return rawne(a, null); }

inline bool operator ==(const AnyPtr& a, const Nop&){ return raweq(a, nop); }
inline bool operator !=(const AnyPtr& a, const Nop&){ return rawne(a, nop); }
inline bool operator ==(const Nop&, const AnyPtr& a){ return raweq(a, nop); }
inline bool operator !=(const Nop&, const AnyPtr& a){ return rawne(a, nop); }

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

template <int N>
struct SmartPtrSelector{};

/**
* @brief T型へのポインタを保持するためのスマートポインタ
*/
template<class T>
class SmartPtr : public SmartPtr<Any>{
public:
	
	SmartPtr(){}

	explicit SmartPtr(T* p){
		set_p2(SmartPtrSelector<InheritedN<T>::value>(), p);
		inc_ref_count();
	}

	template<class U>
	SmartPtr(const SmartPtr<U>& p)
		:SmartPtr<Any>(p){
		T* n = (U*)0; // extends test
	}

	/// nullを受け取るコンストラクタ
	SmartPtr(const Null&){}

	/// 特別なコンストラクタ1
	SmartPtr(typename SmartPtrCtor1<T>::type v)
		:SmartPtr<Any>(SmartPtrCtor1<T>::call(v)){}

	/// 特別なコンストラクタ2
	SmartPtr(typename SmartPtrCtor2<T>::type v)
		:SmartPtr<Any>(SmartPtrCtor2<T>::call(v)){}

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
	*/
	SmartPtr(void*);

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

	void set_p2(SmartPtrSelector<INHERITED_BASE>, T* p){ set_p((Base*)p); }
	void set_p2(SmartPtrSelector<INHERITED_INNOCENCE>, T* p){ *(Innocence*)this = *(Innocence*)p; }
	void set_p2(SmartPtrSelector<INHERITED_OTHER>, T* p){ set_p(((Base*)p - 1)); }

	T* get2(SmartPtrSelector<INHERITED_BASE>) const{ 
		XTAL_ASSERT(type(*this)!=TYPE_NULL); // このアサーションで止まる場合、nullポインタが格納されている
		return (T*)pvalue(*this); 
	}

	T* get2(SmartPtrSelector<INHERITED_INNOCENCE>) const{ return (T*)this; }
	T* get2(SmartPtrSelector<INHERITED_OTHER>) const{ return (T*)((Base*)pvalue(*this) + 1); }

public:

	SmartPtr(SmartPtrSelector<INHERITED_BASE>){
		T* p = new T();
		set_p((Base*)p);
		inc_ref_count();
		p->set_class(new_cpp_class<T>());
		register_gc(p);
	}

	SmartPtr(SmartPtrSelector<INHERITED_INNOCENCE>)
		:SmartPtr<Any>((SmartPtr<Any>&)T()){}

	SmartPtr(SmartPtrSelector<INHERITED_OTHER>){
		TBase<T>* p = new(sizeof(T)) TBase<T>();
		set_p((Base*)p);
		inc_ref_count();
		new(p+1) T;
		p->set_class(new_cpp_class<T>());
		register_gc(p);
	}

/////////////////////

	template<class A0>
	SmartPtr(SmartPtrSelector<INHERITED_BASE>, const A0& a0){
		T* p = new T(a0);
		set_p((Base*)p);
		inc_ref_count();
		p->set_class(new_cpp_class<T>());
		register_gc(p);
	}

	template<class A0>
	SmartPtr(SmartPtrSelector<INHERITED_INNOCENCE>, const A0& a0)
		:SmartPtr<Any>(ap(T(a0))){}

	template<class A0>
	SmartPtr(SmartPtrSelector<INHERITED_OTHER>, const A0& a0){
		TBase<T>* p = new(sizeof(T)) TBase<T>();
		set_p((Base*)p);
		inc_ref_count();
		new(p+1) T(a0);
		p->set_class(new_cpp_class<T>());
		register_gc(p);
	}

/////////////////////

	template<class A0, class A1>
	SmartPtr(SmartPtrSelector<INHERITED_BASE>, const A0& a0, const A1& a1){
		T* p = new T(a0, a1);
		set_p((Base*)p);
		inc_ref_count();
		p->set_class(new_cpp_class<T>());
		register_gc(p);
	}

	template<class A0, class A1>
	SmartPtr(SmartPtrSelector<INHERITED_INNOCENCE>, const A0& a0, const A1& a1)
		:SmartPtr<Any>(ap(T(a0, a1))){}

	template<class A0, class A1>
	SmartPtr(SmartPtrSelector<INHERITED_OTHER>, const A0& a0, const A1& a1){
		TBase<T>* p = new(sizeof(T)) TBase<T>();
		set_p((Base*)p);
		inc_ref_count();
		new(p+1) T(a0, a1);
		p->set_class(new_cpp_class<T>());
		register_gc(p);
	}

/////////////////////

	template<class A0, class A1, class A2>
	SmartPtr(SmartPtrSelector<INHERITED_BASE>, const A0& a0, const A1& a1, const A2& a2){
		T* p = new T(a0, a1, a2);
		set_p((Base*)p);
		inc_ref_count();
		p->set_class(new_cpp_class<T>());
		register_gc(p);
	}

	template<class A0, class A1, class A2>
	SmartPtr(SmartPtrSelector<INHERITED_INNOCENCE>, const A0& a0, const A1& a1, const A2& a2)
		:SmartPtr<Any>(ap(T(a0, a1, a2))){}

	template<class A0, class A1, class A2>
	SmartPtr(SmartPtrSelector<INHERITED_OTHER>, const A0& a0, const A1& a1, const A2& a2){
		TBase<T>* p = new(sizeof(T)) TBase<T>();
		set_p((Base*)p);
		inc_ref_count();
		new(p+1) T(a0, a1, a2);
		p->set_class(new_cpp_class<T>());
		register_gc(p);
	}

/////////////////////

	template<class A0, class A1, class A2, class A3>
	SmartPtr(SmartPtrSelector<INHERITED_BASE>, const A0& a0, const A1& a1, const A2& a2, const A3& a3){
		T* p = new T(a0, a1, a2, a3);
		set_p((Base*)p);
		inc_ref_count();
		p->set_class(new_cpp_class<T>());
		register_gc(p);
	}

	template<class A0, class A1, class A2, class A3>
	SmartPtr(SmartPtrSelector<INHERITED_INNOCENCE>, const A0& a0, const A1& a1, const A2& a2, const A3& a3)
		:SmartPtr<Any>(ap(T(a0, a1, a2, a3))){}

	template<class A0, class A1, class A2, class A3>
	SmartPtr(SmartPtrSelector<INHERITED_OTHER>, const A0& a0, const A1& a1, const A2& a2, const A3& a3){
		TBase<T>* p = new(sizeof(T)) TBase<T>();
		set_p((Base*)p);
		inc_ref_count();
		new(p+1) T(a0, a1, a2, a3);
		p->set_class(new_cpp_class<T>());
		register_gc(p);
	}

/////////////////////

	template<class A0, class A1, class A2, class A3, class A4>
	SmartPtr(SmartPtrSelector<INHERITED_BASE>, const A0& a0, const A1& a1, const A2& a2, const A3& a3, const A4& a4){
		T* p = new T(a0, a1, a2, a3, a4);
		set_p((Base*)p);
		inc_ref_count();
		p->set_class(new_cpp_class<T>());
		register_gc(p);
	}

	template<class A0, class A1, class A2, class A3, class A4>
	SmartPtr(SmartPtrSelector<INHERITED_INNOCENCE>, const A0& a0, const A1& a1, const A2& a2, const A3& a3, const A4& a4)
		:SmartPtr<Any>(ap(T(a0, a1, a2, a3, a4))){}

	template<class A0, class A1, class A2, class A3, class A4>
	SmartPtr(SmartPtrSelector<INHERITED_OTHER>, const A0& a0, const A1& a1, const A2& a2, const A3& a3, const A4& a4){
		TBase<T>* p = new(sizeof(T)) TBase<T>();
		set_p((Base*)p);
		inc_ref_count();
		new(p+1) T(a0, a1, a2, a3, a4);
		p->set_class(new_cpp_class<T>());
		register_gc(p);
	}
};

/**
* @brief Tオブジェクトを生成する
*/
template<class T>
SmartPtr<T> xnew(){
	return SmartPtr<T>(SmartPtrSelector<InheritedN<T>::value>());
}

/**
* @brief Tオブジェクトを生成する
*/
template<class T, class A0>
SmartPtr<T> xnew(const A0& a0){
	return SmartPtr<T>(SmartPtrSelector<InheritedN<T>::value>(), a0);
}

/**
* @brief Tオブジェクトを生成する
*/
template<class T, class A0, class A1>
SmartPtr<T> xnew(const A0& a0, const A1& a1){
	return SmartPtr<T>(SmartPtrSelector<InheritedN<T>::value>(), a0, a1);
}

/**
* @brief Tオブジェクトを生成する
*/
template<class T, class A0, class A1, class A2>
SmartPtr<T> xnew(const A0& a0, const A1& a1, const A2& a2){
	return SmartPtr<T>(SmartPtrSelector<InheritedN<T>::value>(), a0, a1, a2);
}

/**
* @brief Tオブジェクトを生成する
*/
template<class T, class A0, class A1, class A2, class A3>
SmartPtr<T> xnew(const A0& a0, const A1& a1, const A2& a2, const A3& a3){
	return SmartPtr<T>(SmartPtrSelector<InheritedN<T>::value>(), a0, a1, a2, a3);
}

/**
* @brief Tオブジェクトを生成する
*/
template<class T, class A0, class A1, class A2, class A3, class A4>
SmartPtr<T> xnew(const A0& a0, const A1& a1, const A2& a2, const A3& a3, const A4& a4){
	return SmartPtr<T>(SmartPtrSelector<InheritedN<T>::value>(), a0, a1, a2, a3, a4);
}


template<>
struct SmartPtrCtor1<String>{
	typedef const char* type;
	static AnyPtr call(type v){
		return xnew<String>(v);
	}
};

template<>
struct SmartPtrCtor1<InternedString>{
	typedef const char* type;
	static AnyPtr call(type v);
};

template<>
struct SmartPtrCtor2<InternedString>{
	typedef const StringPtr& type;
	static AnyPtr call(type v);
};


inline void inc_ref_count_force(const Innocence& v){
	if(type(v)==TYPE_BASE){
		pvalue(v)->inc_ref_count();
	}
}

inline void dec_ref_count_force(const Innocence& v){
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
	m & ap(Innocence(p));
}

inline void visit_members(Visitor& m, const Innocence& p){
	m & ap(p);
}


class HaveName : public Base{
public:

	HaveName()
		:name_(null), force_(0){}

	virtual StringPtr object_name();

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
