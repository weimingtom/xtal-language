/** \file src/xtal/xtal_anyptr.h
* \brief src/xtal/xtal_anyptr.h
*/

#ifndef XTAL_ANYPTR_H_INCLUDE_GUARD
#define XTAL_ANYPTR_H_INCLUDE_GUARD

#pragma once

namespace xtal{

template<uint_t Size>
struct UserTypeBuffer{
	union{
		u8 buf[Size];
		int_t dummy;
	};

	template<class T>
	void destroy(T* p){
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
struct UserTypeHolderSub : public UserTypeHolder<T>, public Deleter{
	UserTypeHolderSub(){}
	UserTypeHolderSub(T* p, const Deleter& f):UserTypeHolder<T>(p), Deleter(f){}
	virtual ~UserTypeHolderSub(){ Deleter::destroy(this->ptr); }
};

struct undeleter_t{
	template<class T>
	void destroy(T* p){}

	template<class T>
	SmartPtr<T> operator()(const T* p){
		return SmartPtr<T>(p, *this);
	}
};

struct deleter_t{
	template<class T>
	void destroy(T* p){
		delete p;
	}

	template<class T>
	SmartPtr<T> operator()(const T* p){
		return SmartPtr<T>(p, *this);
	}
};

extern undeleter_t undeleter;
extern deleter_t deleter;

enum InheritedEnum{
	INHERITED_BASE,
	INHERITED_RCBASE,
	INHERITED_ANY,
	INHERITED_ANYPTR,
	INHERITED_OTHER
};

template<class T>
struct InheritedN{
	enum{
		value = 
			IsInherited<T, Base>::value ? INHERITED_BASE : 
			IsInherited<T, RefCountingBase>::value ? INHERITED_RCBASE : 
			IsInherited<T, AnyPtr>::value ? INHERITED_ANYPTR :
			IsInherited<T, Any>::value ? INHERITED_ANY : INHERITED_OTHER
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

template<class T>
struct SmartPtrCtor4{
	struct type{};
	static int_t call(type){ return 0; };
};


template <int N>
struct SmartPtrSelector{};

template<class T>
inline const ClassPtr& cpp_class();

/**
* \brief 何の型のオブジェクトでも保持する特殊化されたスマートポインタ
*/
template<>
class SmartPtr<Any> : public Any{
public:
	
	/**
	* \brief nullで構築するコンストラクタ
	*/
	SmartPtr(){}

	/**
	* \brief Tのポインタと、それを破棄するための関数オブジェクトを受け取って構築するコンストラクタ
	*/
	template<class T, class Deleter>
	SmartPtr(const T* tp, const Deleter& deleter)
		:Any(noinit_t()){
		init_smartptr(new UserTypeHolderSub<T, Deleter>((T*)tp, deleter), cpp_class<T>());
	}

	template<class T>
	SmartPtr(const T* p, const undeleter_t&)
		:Any(noinit_t()){
		value_.init_pointer(p, CppClassSymbol<T>::value->value);
	}

	// 任意のポインタ型を受け取ってxtalで参照できるようにするコンストラクタ
	template<class T>
	SmartPtr(const T* p)
		:Any(noinit_t()){
		set_unknown_pointer(p, p);
	}

	SmartPtr(const SmartPtr<Any>& p);

	/// nullを受け取るコンストラクタ
	SmartPtr(const NullPtr&){}

public:

	SmartPtr<Any>& operator =(const SmartPtr<Any>& p);

	void assign_direct(const SmartPtr<Any>& a);

	explicit SmartPtr(PrimitiveType type)
		:Any(type){}
	
	~SmartPtr();

protected:
	
	SmartPtr(noinit_t)
		:Any(noinit_t()){}

	template<class T>
	void set_unknown_pointer(const T* p, const Base*){
		value_.init(p);
		inc_ref_count();
	}

	template<class T>
	void set_unknown_pointer(const T* p, const void*){
		value_.init_pointer(p, CppClassSymbol<T>::value->value);
	}

	void init_smartptr(const Any& a){
		*static_cast<Any*>(this) = a;
	}

	void init_smartptr(RefCountingBase* p, int_t type);

	void init_smartptr(Base* p, const ClassPtr& c);

public:

	/**
	* \brief booleanから構築するコンストラクタ。
	*
	*/
	SmartPtr(bool v):Any(v){}

	/**
	* \brief 文字列から構築するコンストラクタ。
	*
	*/
	SmartPtr(const char_t* str);

	/**
	* \brief 文字列から構築するコンストラクタ。
	*
	*/
	SmartPtr(const char8_t* str);

	/**
	* \brief 文字列から構築するコンストラクタ。
	*
	*/
	SmartPtr(const StringLiteral& str);

	// 基本型の整数、浮動小数点数から構築するコンストラクタ
	SmartPtr(char v):Any(v){}
	SmartPtr(signed char v):Any(v){}
	SmartPtr(unsigned char v):Any(v){}
	SmartPtr(short v):Any(v){}
	SmartPtr(unsigned short v):Any(v){}
	SmartPtr(int v):Any(v){}
	SmartPtr(unsigned int v):Any(v){}
	SmartPtr(long v):Any(v){}
	SmartPtr(unsigned long v):Any(v){}
	SmartPtr(long long v):Any(v){}
	SmartPtr(unsigned long long v):Any(v){}

	SmartPtr(float v):Any(v){}
	SmartPtr(double v):Any(v){}
	SmartPtr(long double v):Any(v){}

	// 基本型の整数、浮動小数点数から構築するコンストラクタ
	SmartPtr(const bool* v):Any(*v){}
	SmartPtr(const unsigned char* v):Any(*v){}
	SmartPtr(const short* v):Any(*v){}
	SmartPtr(const unsigned short* v):Any(*v){}
	SmartPtr(const int* v):Any(*v){}
	SmartPtr(const unsigned int* v):Any(*v){}
	SmartPtr(const long* v):Any(*v){}
	SmartPtr(const unsigned long* v):Any(*v){}

	SmartPtr(const float* v):Any(*v){}
	SmartPtr(const double* v):Any(*v){}
	SmartPtr(const long double* v):Any(*v){}
public:

	/**
	* \brief Any型へのポインタを取得する。
	*/
	Any* get() const{ return (Any*)this; }

	/**
	* \brief ->演算子
	* スマートポインタとして扱うためにオーバーロードする。
	*/
	Any* operator ->() const{ return get(); }
	
	/**
	* \brief *演算子
	* スマートポインタとして扱うためにオーバーロードする。
	*/
	Any& operator *() const{ return *get(); }
	
public:

#ifdef XTAL_DEBUG

	struct dummy_bool_tag{ void safe_true(){} };
	typedef void (dummy_bool_tag::*safe_bool)();

	operator safe_bool() const{
		return type(*this)>TYPE_FALSE ? &dummy_bool_tag::safe_true : (safe_bool)0;
	}

#else

	/**
	* \brief booleanへの自動変換
	*/
	operator bool() const{
		return type(*this)>TYPE_FALSE;
	}

#endif

	/**
	* \biref !演算子
	*/
	bool operator !() const{
		return type(*this)<=TYPE_FALSE;
	}

private:

	/**
	* \brief 暗黙の変換を抑えるためのコンストラクタ。
	* 得体の知れないポインタからの構築を拒否するため、このコンストラクタはprivateで実装も存在しない。
	*/
	SmartPtr(void*);

	/**
	* \brief 暗黙の変換を抑えるためのコンストラクタ。
	* 得体の知れない関数ポインタからの構築を拒否するため、このコンストラクタはprivateで実装も存在しない。
	*/
	template<class R> AnyPtr(R (*)());
	template<class R, class A1> AnyPtr(R (*)(A1));
	template<class R, class A1, class A2> AnyPtr(R (*)(A1, A2));

};

void visit_members(Visitor& m, const Any& p);

class Visitor{
	int_t value_;
public:	

	Visitor(int_t value){
		value_ = value;
	}

	int_t value(){
		return value_;
	}

	template<class T> Visitor& operator &(T& value){
		visit_members(*this, value);
		return *this;
	}

	template<class T> Visitor& operator ()(T& value){
		visit_members(*this, value);
		return *this;
	}
};

inline const AnyPtr& ap(const Any& v){
	return (const AnyPtr&)v;
}

template<class F, class S>
void visit_members(Visitor& m, const std::pair<F, S>& value){
	m & value.first & value.second;
}

}//namespace 

#endif // XTAL_ANYPTR_H_INCLUDE_GUARD
