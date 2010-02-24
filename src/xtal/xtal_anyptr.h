/** \file src/xtal/xtal_anyptr.h
* \brief src/xtal/xtal_anyptr.h
*/

#ifndef XTAL_ANYPTR_H_INCLUDE_GUARD
#define XTAL_ANYPTR_H_INCLUDE_GUARD

#pragma once

namespace xtal{

template<class T>
struct UserTypeBuffer{
	AlignBuffer<T> buf;

	void* buffer(){ return buf.buffer; }

	template<class U>
	void destroy(U* p){
		p->~U();
	}
};

struct UserTypeHolder : public Base{
	UserTypeHolder(){}
	UserTypeHolder(void* p):ptr(p){}
	void* ptr;
};

template<class T, class Deleter = UserTypeBuffer<T> >
struct UserTypeHolderSub : public UserTypeHolder, public Deleter{
	UserTypeHolderSub(){}
	UserTypeHolderSub(T* p, const Deleter& f):UserTypeHolder(p), Deleter(f){}
	~UserTypeHolderSub(){ Deleter::destroy((T*)this->ptr); }
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

template<class T>
T* make_object();

template<class T, class Deleter>
UserTypeHolderSub<T, Deleter>* xnew_with_deleter(const T* tp, const Deleter& deleter){
	typedef UserTypeHolderSub<T, Deleter> holder;
	holder* p = new(make_object<holder>()) holder;
	p->ptr = const_cast<T*>(tp);
	register_gc(p);
	return p;
}

////////////////////////////////////

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

template<class T>
struct XNew;

/////////////////////////////////////////

/**
* \brief 何の型のオブジェクトでも保持する特殊化されたスマートポインタ
*/
template<>
class SmartPtr<Any> : public Any{
public:
	
	/**
	* \brief nullで構築するコンストラクタ
	*/
	SmartPtr(){
		value_.init_primitive(TYPE_NULL);
	}

	// デストラクタ
	~SmartPtr();

public:

	// コピーコンストラクタ
	SmartPtr(const SmartPtr<Any>& p);

	// 代入演算子
	SmartPtr<Any>& operator =(const SmartPtr<Any>& p);

public:

	// 任意のポインタ型を受け取ってxtalで参照できるようにするコンストラクタ
	template<class T>
	SmartPtr(const T* p){
		set_unknown_pointer(InheritedCast<T>::cast(p));
	}

	template<class T>
	SmartPtr<Any>& operator =(const T* p){
		unref();
		set_unknown_pointer(InheritedCast<T>::cast(p));	
		return *this;
	}

public:

	/**
	* \brief Tのポインタと、それを破棄するための関数オブジェクトを受け取って構築するコンストラクタ
	*/
	template<class T, class Deleter>
	SmartPtr(const T* p, const Deleter& deleter){
		set_unknown_pointer((const Base*)xnew_with_deleter<T, Deleter>(p, deleter));
	}

	template<class T>
	SmartPtr(const T* p, undeleter_t){
		value_.init_pointer(p,  CppClassSymbol<T>::value.key());
	}

public:

	template<class T>
	SmartPtr(const XNew<T>& x){
		init(x);
	}

	template<class T>
	SmartPtr<Any>& operator =(const XNew<T>& x){
		unref_init(x);
		return *this;
	}
	
protected:

	SmartPtr(noinit_t){}

	void init(const Any& a);
	void unref_init(const Any& a);

	void set_unknown_pointer(const Base* p);
	void set_unknown_pointer(const RefCountingBase* p);
	void set_unknown_pointer(const Any* p);
	void set_unknown_pointer(const AnyPtr* p);

	template<class T>
	void set_unknown_pointer(const T* p){
		value_.init_pointer(p, CppClassSymbol<T>::value.key());
	}

	void ref();
	void unref();

public:

	SmartPtr(const NullPtr&);
	SmartPtr<Any>& operator =(const NullPtr& p);

	SmartPtr(const UndefinedPtr&);
	SmartPtr<Any>& operator =(const UndefinedPtr& p);

	SmartPtr(const IDPtr& p);
	SmartPtr<Any>& operator =(const IDPtr& p);

public:

	/**
	* \brief booleanから構築するコンストラクタ。
	*
	*/
	SmartPtr(bool v){
		value_.init_bool(v);
	}

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
	SmartPtr(char v){ value_.init_int(v); }
	SmartPtr(signed char v){ value_.init_int(v); }
	SmartPtr(unsigned char v){ value_.init_int(v); }
	SmartPtr(short v){ value_.init_int(v); }
	SmartPtr(unsigned short v){ value_.init_int(v); }
	SmartPtr(int v){ value_.init_int(v); }
	SmartPtr(unsigned int v){ value_.init_int(v); }
	SmartPtr(long v){ value_.init_int(v); }
	SmartPtr(unsigned long v){ value_.init_int(v); }
	SmartPtr(long long v){ value_.init_int(v); }
	SmartPtr(unsigned long long v){ value_.init_int(v); }

	SmartPtr(float v){ value_.init_float(v); }
	SmartPtr(double v){ value_.init_float(v); }
	SmartPtr(long double v){ value_.init_float(v); }

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
	
private:

	bool is_true() const{
		return rawtype(*this)>TYPE_FALSE;
	}

public:

#ifdef XTAL_DEBUG

	struct dummy_bool_tag{ void safe_true(){} };
	typedef void (dummy_bool_tag::*safe_bool)();

	operator safe_bool() const{
		return is_true() ? &dummy_bool_tag::safe_true : (safe_bool)0;
	}

#else

	/**
	* \brief booleanへの自動変換
	*/
	operator bool() const{
		return is_true();
	}

#endif

	/**
	* \biref !演算子
	*/
	bool operator !() const{
		return !is_true();
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
	template<class R> SmartPtr(R (*a)());
	template<class R, class A1> SmartPtr(R (*a)(A1));
	template<class R, class A1, class A2> SmartPtr(R (*a)(A1, A2));

};

///////////////////////////////////////////

template<int N, class T>
struct Extract{};

template<class T>
struct Extract<INHERITED_BASE, T>{
	static T* extract(const AnyPtr& a){
		return static_cast<T*>(pvalue(a));
	}
};

template<class T>
struct Extract<INHERITED_RCBASE, T>{
	static T* extract(const AnyPtr& a){
		return static_cast<T*>(rcpvalue(a));
	}
};

template<class T>
struct Extract<INHERITED_ANY, T>{
	static T* extract(const AnyPtr& a){
		return reinterpret_cast<T*>(const_cast<AnyPtr*>(&a));
	}
};

template<class T>
struct Extract<INHERITED_ANYPTR, T>{
	static T* extract(const AnyPtr& a){
		return reinterpret_cast<T*>(const_cast<AnyPtr*>(&a));
	}
};

template<class T>
struct Extract<INHERITED_OTHER, T>{
	static T* extract(const AnyPtr& a){
		if(type(a)==TYPE_BASE){
			return static_cast<T*>((static_cast<UserTypeHolder*>(pvalue(a)))->ptr); 
		}
		else{
			return static_cast<T*>(rawvalue(a).vp());
		}
	}
};

/////////////////////////////////////////////////

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

void ap(const AnyPtr& v);

template<class F, class S>
void visit_members(Visitor& m, const std::pair<F, S>& value){
	m & value.first & value.second;
}

/////////////////////////////////////////////////

typedef void (*XTAL_bind_t)(Class* it);

struct CppClassSymbolData{ 
	void init_bind0(XTAL_bind_t b, const char_t* xtname);
	void init_bind1(XTAL_bind_t b, const char_t* xtname);
	void init_bind2(XTAL_bind_t b, const char_t* xtname);

	enum{
		BIND = 2,

		FLAG_BIND0 = 1<<0,
		FLAG_BIND1 = 1<<1,
		FLAG_BIND2 = 1<<2,
		FLAG_NAME = 1<<3
	};

	uint_t key() const{
		return ((uint_t)this >> 2) & 0xffffff;
	}

	int_t flags;	
	XTAL_bind_t prebind;
	XTAL_bind_t bind[BIND];	
	const char_t* name;
};

template<class T>
struct CppClassSymbolBase{
	static CppClassSymbolData value;
	typedef T type;
};

template<class T>
CppClassSymbolData CppClassSymbolBase<T>::value;


template<class T>
struct CppClassSymbol : CppClassSymbolBase<T>{};

// CppClassSymbolの修飾子をはずすための定義
template<class T> struct CppClassSymbol<T&> : CppClassSymbol<T>{};
template<class T> struct CppClassSymbol<T*> : CppClassSymbol<T>{};
template<class T> struct CppClassSymbol<const T> : CppClassSymbol<T>{};
template<class T> struct CppClassSymbol<volatile T> : CppClassSymbol<T>{};
template<class T> struct CppClassSymbol<SmartPtr<T> > : CppClassSymbol<T>{};
template<class T, class Deleter> struct CppClassSymbol<UserTypeHolderSub<T, Deleter> > : CppClassSymbol<T>{};

template<> struct CppClassSymbol<void> : CppClassSymbol<Any>{};

template<> struct CppClassSymbol<Base> : CppClassSymbol<Any>{};
template<> struct CppClassSymbol<ID> : public CppClassSymbol<String>{};

template<> struct CppClassSymbol<bool> : CppClassSymbol<Bool>{};
template<> struct CppClassSymbol<char> : CppClassSymbol<Int>{};
template<> struct CppClassSymbol<signed char> : CppClassSymbol<Int>{};
template<> struct CppClassSymbol<unsigned char> : CppClassSymbol<Int>{};
template<> struct CppClassSymbol<short> : CppClassSymbol<Int>{};
template<> struct CppClassSymbol<unsigned short> : CppClassSymbol<Int>{};
template<> struct CppClassSymbol<int> : CppClassSymbol<Int>{};
template<> struct CppClassSymbol<unsigned int> : CppClassSymbol<Int>{};
template<> struct CppClassSymbol<long> : CppClassSymbol<Int>{};
template<> struct CppClassSymbol<unsigned long> : CppClassSymbol<Int>{};
template<> struct CppClassSymbol<float> : CppClassSymbol<Float>{};
template<> struct CppClassSymbol<double> : CppClassSymbol<Float>{};
template<> struct CppClassSymbol<long double> : CppClassSymbol<Float>{};

template<> struct CppClassSymbol<const char*> : CppClassSymbol<String>{};
template<> struct CppClassSymbol<const wchar_t*> : CppClassSymbol<String>{};


#define XTAL_BIND_(ClassName, xtname, N) \
	template<class T> struct XTAL_bind_template##N;\
	template<> struct XTAL_bind_template##N<ClassName>{\
		typedef ClassName Self;\
		XTAL_bind_template##N(){\
			::xtal::CppClassSymbol<ClassName>::value.init_bind##N(&XTAL_bind_template##N::on_bind, xtname);\
		}\
		static void on_bind(::xtal::Class* it);\
	};\
	static volatile XTAL_bind_template##N<ClassName> XTAL_UNIQUE(XTAL_bind_variable##N);\
	void XTAL_bind_template##N<ClassName>::on_bind(::xtal::Class* it)


#define XTAL_PREBIND(ClassName) XTAL_BIND_(ClassName, XTAL_L(#ClassName), 0)
#define XTAL_BIND(ClassName) XTAL_BIND_(ClassName, XTAL_L(#ClassName), 1)
#define XTAL_BIND2(ClassName) XTAL_BIND_(ClassName, XTAL_L(""), 2)

#define XTAL_PREBIND_ALIAS(ClassName, Name) XTAL_BIND_(ClassName, XTAL_L(#Name), 0)
#define XTAL_BIND_ALIAS(ClassName, Name) XTAL_BIND_(ClassName, XTAL_L(#Name), 1)

////////////////////////////////////

typedef AnyPtr (*bind_var_fun_t)();

struct CppValueSymbolData{ 
	bind_var_fun_t maker;

	uint_t key() const{
		return ((uint_t)this >> 2) & 0xffffff;
	}
};

template<class T>
struct CppValueSymbol{
	static CppValueSymbolData value;
	static AnyPtr maker(){ return XNew<T>(); }
};

template<class T>
CppValueSymbolData CppValueSymbol<T>::value = {&CppValueSymbol<T>::maker};

}//namespace 

#endif // XTAL_ANYPTR_H_INCLUDE_GUARD
