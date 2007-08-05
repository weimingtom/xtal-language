
#pragma once

#include <stddef.h>

#include "xtal_base.h"

namespace xtal{

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

	struct pt{};
	SmartPtr(pt, Base* p)
		:Innocence(p){
		p->inc_ref_count();
	}

	struct nc{};
	SmartPtr(nc, Base* p)
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
	* @brief thisからSmartPtr<T>を取り出す。
	*/
	static SmartPtr<T> from_this(T* p){
		if(IsInherited<T, Base>::value){
			return SmartPtr<T>(pt(), (Base*)p);
		}else{
			return SmartPtr<T>(pt(), ((Base*)p - 1));
		}
	}

	/**
	* @brief T*をカウントせずにスマートポインタに格納する
	*/
	static SmartPtr<T> nocount(Base* p){
		return SmartPtr<T>(nc(), p);
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
	T* get() const{
		if(IsInherited<T, Base>::value){
			return (T*)pvalue(*this);
		}else{
			return (T*)((Base*)pvalue(*this) + 1);
		}
	}

private:

	SmartPtr(pt, Base* p)
		:SmartPtr<Any>(pt(), p){}

	SmartPtr(nc, Base* p)
		:SmartPtr<Any>(nc(), p){}
};


template<class T>
class TBase : public Base{
	virtual ~TBase(){ ((T*)(this+1))->~T(); }
};

template<class T>
SmartPtr<T> xnew(){
	/*return SmartPtr<T>();*/
	if(IsInherited<T, Base>::value){
		Base* p = (Base*)(new T());
		p->set_class(new_cpp_class<T>());
		return SmartPtr<T>::nocount(p);
	}else{
		Base* p = (Base*)Base::operator new(sizeof(T) + sizeof(Base));
		new(p) TBase<T>();
		new(p+1) T();
		p->set_class(new_cpp_class<T>());
		return SmartPtr<T>::nocount(p);
	}
}

template<class T, class A0>
SmartPtr<T> xnew(const A0& a0){
	/*return SmartPtr<T>();*/
	if(IsInherited<T, Base>::value){
		Base* p = (Base*)(new T(a0));
		p->set_class(new_cpp_class<T>());
		return SmartPtr<T>::nocount(p);
	}else{
		Base* p = (Base*)Base::operator new(sizeof(T) + sizeof(Base));
		new(p) TBase<T>();
		new(p+1) T(a0);
		p->set_class(new_cpp_class<T>());
		return SmartPtr<T>::nocount(p);
	}
}

template<class T, class A0, class A1>
SmartPtr<T> xnew(const A0& a0, const A1& a1){
	/*return SmartPtr<T>();*/
	if(IsInherited<T, Base>::value){
		Base* p = (Base*)(new T(a0, a1));
		p->set_class(new_cpp_class<T>());
		return SmartPtr<T>::nocount(p);
	}else{
		Base* p = (Base*)Base::operator new(sizeof(T) + sizeof(Base));
		new(p) TBase<T>();
		new(p+1) T(a0, a1);
		p->set_class(new_cpp_class<T>());
		return SmartPtr<T>::nocount(p);
	}
}

template<class T, class A0, class A1, class A2>
SmartPtr<T> xnew(const A0& a0, const A1& a1, const A2& a2){
	if(IsInherited<T, Base>::value){
		Base* p = (Base*)(new T(a0, a1, a2));
		p->set_class(new_cpp_class<T>());
		return SmartPtr<T>::nocount(p);
	}else{
		Base* p = (Base*)Base::operator new(sizeof(T) + sizeof(Base));
		new(p) TBase<T>();
		new(p+1) T(a0, a1, a2);
		p->set_class(new_cpp_class<T>());
		return SmartPtr<T>::nocount(p);
	}
}

template<class T, class A0, class A1, class A2, class A3>
SmartPtr<T> xnew(const A0& a0, const A1& a1, const A2& a2, const A3& a3){
	if(IsInherited<T, Base>::value){
		Base* p = (Base*)(new T(a0, a1, a2, a3));
		p->set_class(new_cpp_class<T>());
		return SmartPtr<T>::nocount(p);
	}else{
		Base* p = (Base*)Base::operator new(sizeof(T) + sizeof(Base));
		new(p) TBase<T>();
		new(p+1) T(a0, a1, a2, a3);
		p->set_class(new_cpp_class<T>());
		return SmartPtr<T>::nocount(p);
	}
}

template<class T, class A0, class A1, class A2, class A3, class A4>
SmartPtr<T> xnew(const A0& a0, const A1& a1, const A2& a2, const A3& a3, const A4& a4){
	if(IsInherited<T, Base>::value){
		Base* p = (Base*)(new T(a0, a1, a2, a3, a4));
		p->set_class(new_cpp_class<T>());
		return SmartPtr<T>::nocount(p);
	}else{
		Base* p = (Base*)Base::operator new(sizeof(T) + sizeof(Base));
		new(p) TBase<T>();
		new(p+1) T(a0, a1, a2, a3, a4);
		p->set_class(new_cpp_class<T>());
		return SmartPtr<T>::nocount(p);
	}
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
