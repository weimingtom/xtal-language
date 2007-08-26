
#pragma once

#include <stddef.h>

#include "xtal_base.h"
#include "xtal_allocator.h"

namespace xtal{

struct PT{};
struct NC{};

template<class T>
class TBase : public Base{
	virtual ~TBase(){ ((T*)(this+1))->~T(); }
};

enum{
	INHERITED_BASE,
	INHERITED_INNOCENCE,
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

protected:

	SmartPtr(PT, Base* p)
		:Innocence(p){
		p->inc_ref_count();
	}

	SmartPtr(NC, Base* p)
		:Innocence(p){
	}

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
class Nop : public AnyPtr{ public: Nop():AnyPtr(TYPE_NOP){} };

inline bool operator ==(const AnyPtr& a, const Null&){ return !a; }
inline bool operator !=(const AnyPtr& a, const Null&){ return a; }
inline bool operator ==(const Null&, const AnyPtr& a){ return !a; }
inline bool operator !=(const Null&, const AnyPtr& a){ return a; }


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

/**
* @brief T型へのポインタを保持するためのスマートポインタ
*/
template<class T>
class SmartPtr : public SmartPtr<Any>{
public:
	
	SmartPtr(){}

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

public:

	/**
	* @brief T*からSmartPtr<T>を取り出す。
	*/
	static SmartPtr<T> from_this(T* p){ return from_this2(I2T<InheritedN<T>::value>(), p); }

	/**
	* @brief T*をカウントせずにスマートポインタに格納する
	*/
	static SmartPtr<T> nocount(T* p){ return nocount2(I2T<InheritedN<T>::value>(), p); }

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
	T* get() const{ return get2(I2T<InheritedN<T>::value>()); }

private:

	SmartPtr(PT pt, Base* p)
		:SmartPtr<Any>(pt, p){}

	SmartPtr(NC nc, Base* p)
		:SmartPtr<Any>(nc, p){}

	static SmartPtr<T> from_this2(I2T<INHERITED_BASE>, T* p){ return SmartPtr<T>(PT(), (Base*)p); }
	static SmartPtr<T> from_this2(I2T<INHERITED_INNOCENCE>, T* p){ return *(SmartPtr<T>*)p; }
	static SmartPtr<T> from_this2(I2T<INHERITED_OTHER>, T* p){ return SmartPtr<T>(PT(), ((Base*)p - 1)); }

	static SmartPtr<T> nocount2(I2T<INHERITED_BASE>, T* p){ return SmartPtr<T>(NC(), p); }
	static SmartPtr<T> nocount2(I2T<INHERITED_INNOCENCE>, T* p){ return *(SmartPtr<T>*)p; }
	static SmartPtr<T> nocount2(I2T<INHERITED_OTHER>, T* p){ return SmartPtr<T>(NC(), ((Base*)p - 1)); }

	T* get2(I2T<INHERITED_BASE>) const{ return (T*)pvalue(*this); }
	T* get2(I2T<INHERITED_INNOCENCE>) const{ return (T*)this; }
	T* get2(I2T<INHERITED_OTHER>) const{ return (T*)((Base*)pvalue(*this) + 1); }

};


template<class T, class A0 = Void, class A1 = Void, class A2 = Void, class A3 = Void, class A4 = Void>
struct XNewT{
	static SmartPtr<T> make(I2T<INHERITED_BASE>);
	static SmartPtr<T> make(I2T<INHERITED_INNOCENCE>);
	static SmartPtr<T> make(I2T<INHERITED_OTHER>);

	static SmartPtr<T> make(I2T<INHERITED_BASE>, const A0& a0);
	static SmartPtr<T> make(I2T<INHERITED_INNOCENCE>, const A0& a0);
	static SmartPtr<T> make(I2T<INHERITED_OTHER>, const A0& a0);

	static SmartPtr<T> make(I2T<INHERITED_BASE>, const A0& a0, const A1& a1);
	static SmartPtr<T> make(I2T<INHERITED_INNOCENCE>, const A0& a0, const A1& a1);
	static SmartPtr<T> make(I2T<INHERITED_OTHER>, const A0& a0, const A1& a1);

	static SmartPtr<T> make(I2T<INHERITED_BASE>, const A0& a0, const A1& a1, const A2& a2);
	static SmartPtr<T> make(I2T<INHERITED_INNOCENCE>, const A0& a0, const A1& a1, const A2& a2);
	static SmartPtr<T> make(I2T<INHERITED_OTHER>, const A0& a0, const A1& a1, const A2& a2);

	static SmartPtr<T> make(I2T<INHERITED_BASE>, const A0& a0, const A1& a1, const A2& a2, const A3& a3);
	static SmartPtr<T> make(I2T<INHERITED_INNOCENCE>, const A0& a0, const A1& a1, const A2& a2, const A3& a3);
	static SmartPtr<T> make(I2T<INHERITED_OTHER>, const A0& a0, const A1& a1, const A2& a2, const A3& a3);

	static SmartPtr<T> make(I2T<INHERITED_BASE>, const A0& a0, const A1& a1, const A2& a2, const A3& a3, const A4& a4);
	static SmartPtr<T> make(I2T<INHERITED_INNOCENCE>, const A0& a0, const A1& a1, const A2& a2, const A3& a3, const A4& a4);
	static SmartPtr<T> make(I2T<INHERITED_OTHER>, const A0& a0, const A1& a1, const A2& a2, const A3& a3, const A4& a4);
};

template<class T, class A0, class A1, class A2, class A3, class A4>
SmartPtr<T> XNewT<T, A0, A1, A2, A3, A4>::make(I2T<INHERITED_BASE>){
	Base* p = new T();
	p->set_class(new_cpp_class<T>());
	return SmartPtr<T>::nocount((T*)p);
}

template<class T, class A0, class A1, class A2, class A3, class A4>
SmartPtr<T> XNewT<T, A0, A1, A2, A3, A4>::make(I2T<INHERITED_INNOCENCE>){
	T temp;
	return SmartPtr<T>::nocount(&temp);
}

template<class T, class A0, class A1, class A2, class A3, class A4>
SmartPtr<T> XNewT<T, A0, A1, A2, A3, A4>::make(I2T<INHERITED_OTHER>){
	Base* p = (Base*)Base::operator new(sizeof(T) + sizeof(Base));
	new(p) TBase<T>();
	new(p+1) T();
	p->set_class(new_cpp_class<T>());
	return SmartPtr<T>::nocount((T*)(p+1));		
}

////

template<class T, class A0, class A1, class A2, class A3, class A4>
SmartPtr<T> XNewT<T, A0, A1, A2, A3, A4>::make(I2T<INHERITED_BASE>, const A0& a0){
	Base* p = new T(a0);
	p->set_class(new_cpp_class<T>());
	return SmartPtr<T>::nocount((T*)p);
}

template<class T, class A0, class A1, class A2, class A3, class A4>
SmartPtr<T> XNewT<T, A0, A1, A2, A3, A4>::make(I2T<INHERITED_INNOCENCE>, const A0& a0){
	T temp(a0);
	return SmartPtr<T>::nocount(&temp);
}

template<class T, class A0, class A1, class A2, class A3, class A4>
SmartPtr<T> XNewT<T, A0, A1, A2, A3, A4>::make(I2T<INHERITED_OTHER>, const A0& a0){
	Base* p = (Base*)Base::operator new(sizeof(T) + sizeof(Base));
	new(p) TBase<T>();
	new(p+1) T(a0);
	p->set_class(new_cpp_class<T>());
	return SmartPtr<T>::nocount((T*)(p+1));		
}

////

template<class T, class A0, class A1, class A2, class A3, class A4>
SmartPtr<T> XNewT<T, A0, A1, A2, A3, A4>::make(I2T<INHERITED_BASE>, const A0& a0, const A1& a1){
	Base* p = new T(a0, a1);
	p->set_class(new_cpp_class<T>());
	return SmartPtr<T>::nocount((T*)p);
}

template<class T, class A0, class A1, class A2, class A3, class A4>
SmartPtr<T> XNewT<T, A0, A1, A2, A3, A4>::make(I2T<INHERITED_INNOCENCE>, const A0& a0, const A1& a1){
	T temp(a0, a1);
	return SmartPtr<T>::nocount(&temp);
}

template<class T, class A0, class A1, class A2, class A3, class A4>
SmartPtr<T> XNewT<T, A0, A1, A2, A3, A4>::make(I2T<INHERITED_OTHER>, const A0& a0, const A1& a1){
	Base* p = (Base*)Base::operator new(sizeof(T) + sizeof(Base));
	new(p) TBase<T>();
	new(p+1) T(a0, a1);
	p->set_class(new_cpp_class<T>());
	return SmartPtr<T>::nocount((T*)(p+1));		
}

////

template<class T, class A0, class A1, class A2, class A3, class A4>
SmartPtr<T> XNewT<T, A0, A1, A2, A3, A4>::make(I2T<INHERITED_BASE>, const A0& a0, const A1& a1, const A2& a2){
	Base* p = new T(a0, a1, a2);
	p->set_class(new_cpp_class<T>());
	return SmartPtr<T>::nocount((T*)p);
}

template<class T, class A0, class A1, class A2, class A3, class A4>
SmartPtr<T> XNewT<T, A0, A1, A2, A3, A4>::make(I2T<INHERITED_INNOCENCE>, const A0& a0, const A1& a1, const A2& a2){
	T temp(a0, a1, a2);
	return SmartPtr<T>::nocount(&temp);
}

template<class T, class A0, class A1, class A2, class A3, class A4>
SmartPtr<T> XNewT<T, A0, A1, A2, A3, A4>::make(I2T<INHERITED_OTHER>, const A0& a0, const A1& a1, const A2& a2){
	Base* p = (Base*)Base::operator new(sizeof(T) + sizeof(Base));
	new(p) TBase<T>();
	new(p+1) T(a0, a1, a2);
	p->set_class(new_cpp_class<T>());
	return SmartPtr<T>::nocount((T*)(p+1));		
}

////

template<class T, class A0, class A1, class A2, class A3, class A4>
SmartPtr<T> XNewT<T, A0, A1, A2, A3, A4>::make(I2T<INHERITED_BASE>, const A0& a0, const A1& a1, const A2& a2, const A3& a3){
	Base* p = new T(a0, a1, a2, a3);
	p->set_class(new_cpp_class<T>());
	return SmartPtr<T>::nocount((T*)p);
}

template<class T, class A0, class A1, class A2, class A3, class A4>
SmartPtr<T> XNewT<T, A0, A1, A2, A3, A4>::make(I2T<INHERITED_INNOCENCE>, const A0& a0, const A1& a1, const A2& a2, const A3& a3){
	T temp(a0, a1, a2, a3);
	return SmartPtr<T>::nocount(&temp);
}

template<class T, class A0, class A1, class A2, class A3, class A4>
SmartPtr<T> XNewT<T, A0, A1, A2, A3, A4>::make(I2T<INHERITED_OTHER>, const A0& a0, const A1& a1, const A2& a2, const A3& a3){
	Base* p = (Base*)Base::operator new(sizeof(T) + sizeof(Base));
	new(p) TBase<T>();
	new(p+1) T(a0, a1, a2, a3);
	p->set_class(new_cpp_class<T>());
	return SmartPtr<T>::nocount((T*)(p+1));		
}

////

template<class T, class A0, class A1, class A2, class A3, class A4>
SmartPtr<T> XNewT<T, A0, A1, A2, A3, A4>::make(I2T<INHERITED_BASE>, const A0& a0, const A1& a1, const A2& a2, const A3& a3, const A4& a4){
	Base* p = new T(a0, a1, a2, a3, a4);
	p->set_class(new_cpp_class<T>());
	return SmartPtr<T>::nocount((T*)p);
}

template<class T, class A0, class A1, class A2, class A3, class A4>
SmartPtr<T> XNewT<T, A0, A1, A2, A3, A4>::make(I2T<INHERITED_INNOCENCE>, const A0& a0, const A1& a1, const A2& a2, const A3& a3, const A4& a4){
	T temp(a0, a1, a2, a3, a4);
	return SmartPtr<T>::nocount(&temp);
}

template<class T, class A0, class A1, class A2, class A3, class A4>
SmartPtr<T> XNewT<T, A0, A1, A2, A3, A4>::make(I2T<INHERITED_OTHER>, const A0& a0, const A1& a1, const A2& a2, const A3& a3, const A4& a4){
	Base* p = (Base*)Base::operator new(sizeof(T) + sizeof(Base));
	new(p) TBase<T>();
	new(p+1) T(a0, a1, a2, a3, a4);
	p->set_class(new_cpp_class<T>());
	return SmartPtr<T>::nocount((T*)(p+1));		
}




template<class T>
inline SmartPtr<T> xnew(){
	return XNewT<T>::make(I2T<InheritedN<T>::value>());
}

template<class T, class A0>
inline SmartPtr<T> xnew(const A0& a0){
	return XNewT<T, A0>::make(I2T<InheritedN<T>::value>(), a0);
}

template<class T, class A0, class A1>
inline SmartPtr<T> xnew(const A0& a0, const A1& a1){
	return XNewT<T, A0, A1>::make(I2T<InheritedN<T>::value>(), a0, a1);
}

template<class T, class A0, class A1, class A2>
inline SmartPtr<T> xnew(const A0& a0, const A1& a1, const A2& a2){
	return XNewT<T, A0, A1, A2>::make(I2T<InheritedN<T>::value>(), a0, a1, a2);
}

template<class T, class A0, class A1, class A2, class A3>
inline SmartPtr<T> xnew(const A0& a0, const A1& a1, const A2& a2, const A3& a3){
	return XNewT<T, A0, A1, A2, A3>::make(I2T<InheritedN<T>::value>(), a0, a1, a2, a3);
}

template<class T, class A0, class A1, class A2, class A3, class A4>
inline SmartPtr<T> xnew(const A0& a0, const A1& a1, const A2& a2, const A3& a3, const A4& a4){
	return XNewT<T, A0, A1, A2, A3, A4>::make(I2T<InheritedN<T>::value>(), a0, a1, a2, a3, a4);
}


template<>
struct SmartPtrCtor1<String>{
	typedef const char* type;
	static AnyPtr call(type v){
		return xnew<String>(v);
	}
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

template<class T>
class LLVar : public T{
public:
	LLVar(){ add_long_life_var(this); }
	LLVar(const LLVar<T>& v):T(v){ add_long_life_var(this); }
	template<class U> LLVar(const U& v):T(v){ add_long_life_var(this); }
	~LLVar(){ remove_long_life_var(this); }
	template<class U> LLVar<T>& operator =(const U& v){ this->T::operator=(v); return *this; }
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
