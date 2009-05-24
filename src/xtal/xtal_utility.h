/** \file src/xtal/xtal_utility.h
* \brief src/xtal/xtal_utility.h
*/

#ifndef XTAL_UTILITY_H_INCLUDE_GUARD
#define XTAL_UTILITY_H_INCLUDE_GUARD

#pragma once

/**
* @brief xtal namespace
*/
namespace xtal{
enum{
	VERSION1 = 0,
	VERSION2 = 9,
	VERSION3 = 9,
	VERSION4 = 9
};
}

//#define XTAL_USE_COMPILED_EMB
//#define XTAL_ENFORCE_64_BIT
//#define XTAL_USE_THREAD_MODEL_2
//#define XTAL_NO_XPEG

//#define XTAL_NO_ASSERT

//#define XTAL_DEBUG_PRINT

#ifdef _UNICODE
#	ifndef XTAL_USE_WCHAR
#		define XTAL_USE_WCHAR
#	endif
#endif 

#if !defined(XTAL_NO_THREAD) && !defined(XTAL_TLS_PTR) && defined(XTAL_USE_PTHREAD_TLS)
#include <pthread.h>
template<class T>
struct TLSPtr{
	TLSPtr(){ pthread_key_create(&key, 0); }
	~TLSPtr(){ pthread_key_delete(key); }
	void operator =(T* p){ pthread_setspecific(key, p); }
	operator T*(){ return static_cast<T*>(pthread_getspecific(key)); }
	T* operator ->(){ return *this; }
private:
	pthread_key_t key;
	TLSPtr(const TLSPtr&);
	void operator=(const TLSPtr&);
};
#define XTAL_TLS_PTR(x) TLSPtr<x>
#endif

#if !defined(XTAL_NO_THREAD) && !defined(XTAL_TLS_PTR) && defined(_WIN32) && !defined(_MSC_VER)
#include <windows.h>
template<class T>
struct TLSPtr{
	TLSPtr(){ key = TlsAlloc(); }
	~TLSPtr(){ TlsFree(key); }
	void operator =(T* p){ TlsSetValue(key, p); }
	operator T*(){ return static_cast<T*>(TlsGetValue(key)); }
	T* operator ->(){ return *this; }
private:
	DWORD key;
	TLSPtr(const TLSPtr&);
	void operator=(const TLSPtr&);
};
#define XTAL_TLS_PTR(x) TLSPtr<x>
#endif

#ifdef XTAL_NO_PARSER
#	define XTAL_USE_COMPILED_EMB
#endif

#if !defined(NDEBUG) && (defined(_DEBUG) || defined(DEBUG))
#	define XTAL_DEBUG
#endif

#if defined(XTAL_DEBUG) && !defined(XTAL_NO_ASSERT)
#	define XTAL_ASSERT(expr) assert(expr)
#else
#	define XTAL_ASSERT(expr)
#endif

#ifdef XTAL_DEBUG
#	define XTAL_NODEFAULT default: XTAL_ASSERT(false); break
#else
#	ifdef _MSC_VER
#		define XTAL_NODEFAULT default: __assume(0)
#	else
#		define XTAL_NODEFAULT
#	endif
#endif

#ifdef XTAL_DEBUG
#	define XTAL_DEBUG_ONLY(x) x
#else
#	define XTAL_DEBUG_ONLY(x) 
#endif

#define XTAL_UNUSED_VAR(x) ((void)x)

#define XTAL_DISALLOW_COPY_AND_ASSIGN(ClassName) ClassName(const ClassName&); void operator=(const ClassName&)

#define XTAL_DEFAULT default:
#define XTAL_CASE(key) break; case key:
#define XTAL_CASE1(key) break; case key:
#define XTAL_CASE2(key, key2) break; case key:case key2:
#define XTAL_CASE3(key, key2, key3) break; case key:case key2:case key3:
#define XTAL_CASE4(key, key2, key3, key4) break; case key:case key2:case key3:case key4:

#if defined(__GNUC__) && (__GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 96))
#	define XTAL_LIKELY(cond) __builtin_expect(!!(int)(cond), 1)
#	define XTAL_UNLIKELY(cond) __builtin_expect((int)(cond), 0)
#else
#	define XTAL_LIKELY(cond) (cond)
#	define XTAL_UNLIKELY(cond) (cond)
#endif

#ifdef XTAL_NO_THREAD
#	define XTAL_UNLOCK 
#else
#	define XTAL_UNLOCK if(const ::xtal::XUnlock& xunlock = (XTAL_UNUSED_VAR(xunlock), 0))
#endif

#ifdef __GNUC__
#	define XTAL_NOINLINE __attribute__((noinline)) 
#else
#	define XTAL_NOINLINE
#endif

#if defined(_MSC_VER) && _MSC_VER>=1400
#	pragma warning(disable: 4819)
#	pragma warning(disable: 4127)
#	pragma warning(disable: 4100)
#endif

#ifdef XTAL_USE_WCHAR
#	define XTAL_STRING(x) ::xtal::StringLiteral(L##x, sizeof(L##x)/sizeof(wchar_t)-1)
#else 
#	define XTAL_STRING(x) ::xtal::StringLiteral(x, sizeof(x)-1)
#endif

#if defined(_MSC_VER) || defined(__MINGW__) || defined(__MINGW32__)
#	define XTAL_INT_FMT (sizeof(int_t)==8 ? XTAL_STRING("I64") : XTAL_STRING(""))
#else
#	define XTAL_INT_FMT (sizeof(int_t)==8 ? XTAL_STRING("ll") : XTAL_STRING(""))
#endif

#ifdef XTAL_USE_WCHAR
#	if defined(_MSC_VER) && _MSC_VER>=1400
#		define XTAL_SPRINTF(buffer, data_size, format, value) swprintf_s(buffer, data_size, format, value)
#	else
#		define XTAL_SPRINTF(buffer, data_size, format, value) std::swprintf(buffer, format, value)
#	endif
#else
#	if defined(_MSC_VER) && _MSC_VER>=1400
#		define XTAL_SPRINTF(buffer, data_size, format, value) sprintf_s(buffer, data_size, format, value)
#	else
#		define XTAL_SPRINTF(buffer, data_size, format, value) std::sprintf(buffer, format, value)
#	endif
#endif

#ifdef XTAL_NO_THREAD
#	define XTAL_TLS_PTR(x) x*
#else
#	ifndef XTAL_TLS_PTR
#		if defined(_MSC_VER)
#			define XTAL_TLS_PTR(x) __declspec(thread) x*
#		elif defined(__GNUC__) && !defined(__CYGWIN__)
#			define XTAL_TLS_PTR(x) __thread x*
#		else 
#			error // XTAL_NO_THREADを定義しない場合、TLSを実現するためのXTAL_TLS_PTR(x)の実装が必要
#		endif
#	endif
#endif

namespace xtal{

template<class T>
inline T align(T v, int N){
	return (v+(N-1)) & ~(N-1);
}

template<class T>
inline T* align_p(T* v, int N){
	return (T*)align((unsigned long)v, N);
}

template<class T>
inline T align_2(T v){
	v-=1;
	v|=v>>1;
	v|=v>>2;
	v|=v>>4;
	v|=v>>8;
	v|=v>>16;
	v|=v>>32;
	return v+1;
}

template<int N>
class static_count_bits{
	enum{
		N1 = (N & 0x55555555) + (N >> 1 & 0x55555555),
		N2 = (N1 & 0x33333333) + (N1 >> 2 & 0x33333333),
		N3 = (N2 & 0x0f0f0f0f) + (N2 >> 4 & 0x0f0f0f0f),
		N4 = (N3 & 0x00ff00ff) + (N3 >> 8 & 0x00ff00ff)
	};
public:	
	enum{
		value = (N4 & 0x0000ffff) + (N4>>16 & 0x0000ffff)	
	};
};

template<int N>
struct static_ntz{
	enum{
		value = static_count_bits<((N&(-N))-1)>::value
	};
};

// 最低限のメタプログラミング下地

template<bool>
struct IfHelper{
	template<class T, class U>
	struct inner{
		typedef T type;
	};
};

template<>
struct IfHelper<false>{
	template<class T, class U>
	struct inner{
		typedef U type;
	};
};

template<bool N, class T, class U>
struct If{
	typedef typename IfHelper<N>::template inner<T, U>::type type;
};


template<int>
struct TypeIntType{};

template<class T, class U>
struct Convertible{
	typedef char (&yes)[2];
	typedef char (&no)[1];
	static yes test(U);
	static no test(...);
	static T makeT();

	enum{ value = sizeof(test(makeT()))==sizeof(yes) };
};

template<class T, class U>
struct IsInherited{
	typedef char (&yes)[2];
	typedef char (&no)[1];
	static yes test(const U*);
	static no test(...);
	static T* makeT();

	enum{ value = sizeof(test(makeT()))==sizeof(yes) };
};

template<int N>
struct I2T{
	enum{ value = N };
};

template<class T>
struct T2T{
	typedef T type;
};

struct Other{
	template<class T>
	Other(const T&){}
};

template<class T>
struct IsConst{
	typedef char (&yes)[2];
	typedef char (&no)[1];
	template<class U>
	static yes test(const U (*)());
	static no test(...);

	enum{ value = sizeof(test((T (*)())0))==sizeof(yes) };
};

template<class T>
struct IsReference{
	typedef char (&yes)[2];
	typedef char (&no)[1];
	template<class U>
	static yes test(U& (*)());
	static no test(...);

	enum{ value = sizeof(test((T (*)())0))==sizeof(yes) };
};

template<class T>
struct IsPointer{
	typedef char (&yes)[2];
	typedef char (&no)[1];
	template<class U>
	static yes test(U* (*)());
	static no test(...);

	enum{ value = sizeof(test((T (*)())0))==sizeof(yes) };
};

template<class T, class U>
struct IsSame{
	typedef char (&yes)[2];
	typedef char (&no)[1];
	static yes test(U (*)());
	static no test(...);

	enum{ value = sizeof(test((T (*)())0))==sizeof(yes) };
};

template<class T>
struct IsVoid{ enum{ value = 0 }; };
template<>
struct IsVoid<void>{ enum{ value = 1 }; };

template<class T>
struct IsNotVoid{ enum{ value = 1 }; };
template<>
struct IsNotVoid<void>{ enum{ value = 0 }; };

template<class T>
struct TempHolder{
	static T temp;
};

template<class T>
T TempHolder<T>::temp;

struct Temp{
	template<class T>
	operator T&() const{
		return TempHolder<T>::temp;
	}
};

template<class T>
struct Ref{
	T& ref;
	Ref(T& ref):ref(ref){};
	operator T&() const{ return ref; }
};

template<class T>
inline Ref<T> ref(T& r){
	return Ref<T>(r);
}

template<class T, class U>
struct NumericCalcResultType{
	typedef typename If<1, T, U>::type type;
};

template<int N>
struct SelectType{
	typedef typename If<sizeof(signed char)==N,
		signed char,
		typename If<sizeof(signed short)==N, 
			signed short int,
			typename If<sizeof(signed int)==N,
				signed int,
				typename If<sizeof(signed long)==N,
					signed long int,
					typename If<sizeof(signed long long)==N,
						signed long long int,
						void
					>::type
				>::type
			>::type
		>::type
	>::type int_t;

	typedef typename If<sizeof(unsigned char)==N,
		unsigned char,
		typename If<sizeof(unsigned short)==N, 
			unsigned short int,
			typename If<sizeof(unsigned int)==N,
				unsigned int,
				typename If<sizeof(unsigned long)==N,
					unsigned long int,
					typename If<sizeof(unsigned long long)==N,
						unsigned long long int,
						void
					>::type
				>::type
			>::type
		>::type
	>::type uint_t;

	typedef typename If<sizeof(float)==N,
		float,
		typename If<sizeof(double)==N, 
			double,
			void
		>::type
	>::type float_t;
};

/// 1-byte uint
typedef SelectType<1>::uint_t u8;

/// 2-byte uint
typedef SelectType<2>::uint_t u16;
	
/// 4-byte uint
typedef SelectType<4>::uint_t u32;

/// 8-byte uint
typedef SelectType<8>::uint_t u64;

/// 1-byte int
typedef SelectType<1>::int_t i8;

/// 2-byte int
typedef SelectType<2>::int_t i16;
	
/// 4-byte int
typedef SelectType<4>::int_t i32;

/// 8-byte int
typedef SelectType<8>::int_t i64;

/// 4-byte float
typedef SelectType<4>::float_t f32;

/// 8-byte float
typedef SelectType<8>::float_t f64;


#ifdef XTAL_ENFORCE_64_BIT

/// int
typedef SelectType<8>::int_t int_t;
	
/// float
typedef SelectType<8>::float_t float_t;
	
/// uint
typedef SelectType<8>::uint_t uint_t;

#else

/// int
typedef SelectType<sizeof(void*)>::int_t int_t;
	
/// float
typedef SelectType<sizeof(void*)>::float_t float_t;
	
/// uint
typedef SelectType<sizeof(void*)>::uint_t uint_t;

#endif

/// byte
typedef SelectType<1>::uint_t byte_t;

//typedef std::size_t size_t;
//typedef std::ptrdiff_t ptrdiff_t;

#ifdef XTAL_USE_WCHAR

typedef wchar_t char_t;
typedef char char8_t;

#else

typedef char char_t;
typedef signed char char8_t;

#endif

/// unsigned char_t
typedef SelectType<sizeof(char_t)>::uint_t uchar_t;

template<class T>
struct IsFloat{ enum{ value = 0 }; };

template<>
struct IsFloat<float_t>{ enum{ value = 1 }; };

class StringLiteral{
public:

	StringLiteral(const char_t* str, uint_t size)
		:str_(str), size_(size){
	}

	operator const char_t*() const{
		return str_;
	}

	uint_t size() const{
		return size_;
	}

private:
	const char_t* str_;
	uint_t size_;
};


/**
* \brief プリミティブな型の種類
*/
enum PrimitiveType{
	TYPE_NULL = 0,
	TYPE_UNDEFINED = 1,

	TYPE_FALSE = 2,
	// ここから上は、ifなどで偽と評価される

	// ここから下は、値によらず真と評価される
	TYPE_TRUE = 3,
	
	TYPE_INT = 4,
	TYPE_FLOAT = 5,
	
	TYPE_SMALL_STRING = 6,
	// ここから上はimmutableな値型である

	// ここから下は参照型である
	TYPE_BASE = 7,

	TYPE_STRING = 8,
	
	TYPE_ARRAY = 9,
	TYPE_VALUES = 10,
	TYPE_TREE_NODE = 11,

	TYPE_NATIVE_METHOD = 12,
	TYPE_NATIVE_FUN = 13,

	TYPE_IVAR_GETTER = 14,
	TYPE_IVAR_SETTER = 15,

	/*
	TYPE_FRAME,
	TYPE_CLASS,

	TYPE_METHOD,
	TYPE_FUN,
	TYPE_LAMBDA,
	TYPE_FIBER,
	*/

	TYPE_MAX,

	TYPE_SHIFT = 4,
	TYPE_MASK = (1<<TYPE_SHIFT)-1
};


/**
* \brief ブロックの種類
*/
enum BlockKind{
	KIND_BLOCK,
	KIND_CLASS,
	KIND_SINGLETON,

	KIND_FUN,
	KIND_LAMBDA,
	KIND_METHOD,
	KIND_FIBER
};

/**
* \brief 文字列の種類
*/
enum StringKind{
	KIND_STRING,
	KIND_TEXT,
	KIND_FORMAT
};

/**
* \brief 可触性の種類
*/
enum AccessibilityKind{
	/**
	* \biref public
	*/
	KIND_PUBLIC = 0,

	/**
	* \brief protected
	*/
	KIND_PROTECTED = 1<<0,

	/**
	* \brief private
	*/
	KIND_PRIVATE = 1<<1
};

/**
* \brief ブレークポイントの種類
*/
enum BreakPointKind{
	/**
	* \brief ブレークポイント
	*/
	BREAKPOINT,

	/**
	* \brief 関数呼び出し時ブレークポイント
	*/
	BREAKPOINT_CALL,

	/**
	* \brief 関数リターン時ブレークポイント
	*/
	BREAKPOINT_RETURN,

	/**
	* \brief 例外創出時ブレークポイント
	*/
	BREAKPOINT_THROW,

	/**
	* \brief 表明ブレークポイント
	*/
	BREAKPOINT_ASSERT
};

/**
* \brief 区間の種類
*/
enum RangeKind{
	/**
	* \brief 閉区間[left, right]
	*/
	RANGE_CLOSED = (0<<1) | (0<<0),

	/**
	* \brief 左開右閉区間 [left, right)
	*/
	RANGE_LEFT_CLOSED_RIGHT_OPEN = (0<<1) | (1<<0),

	/**
	* \brief 左開右閉区間 (left, right]
	*/
	RANGE_LEFT_OPEN_RIGHT_CLOSED = (1<<1) | (0<<0),

	/**
	* \brief 開区間 (left, right)
	*/
	RANGE_OPEN = (1<<1) | (1<<0)
};



template<class T>
class SmartPtr;

class Any;
template<> class SmartPtr<Any>;
typedef SmartPtr<Any> AnyPtr;

class Environment;
struct Param;

class Null;
class Undefined;
class Array;
class Map;
class Set;
class Stream;
class MemoryStream;
class StringStream;
class InteractiveStream;
class Fun;
class Method;
class Fiber;
class InstanceVariableGetter;
class InstanceVariableSetter;
class Lambda;
class String;
class ID;
class Code;
class Arguments;
class VMachine;
class NativeMethod;
class Frame;
class Class;
class Lib;
class Thread;
class Mutex;
class IntRange;
class FloatRange;
class ChRange;
class DoubleDispatchMethod;
class DoubleDispatchFun;
class Values;
class Exception;

typedef SmartPtr<Null> NullPtr;
typedef SmartPtr<Undefined> UndefinedPtr;
typedef SmartPtr<Array> ArrayPtr;
typedef SmartPtr<Map> MapPtr;
typedef SmartPtr<Set> SetPtr;
typedef SmartPtr<Stream> StreamPtr;
typedef SmartPtr<MemoryStream> MemoryStreamPtr;
typedef SmartPtr<StringStream> StringStreamPtr;
typedef SmartPtr<InteractiveStream> InteractiveStreamPtr;
typedef SmartPtr<Fun> FunPtr;
typedef SmartPtr<Method> MethodPtr;
typedef SmartPtr<Fiber> FiberPtr;
typedef SmartPtr<InstanceVariableGetter> InstanceVariableGetterPtr;
typedef SmartPtr<InstanceVariableSetter> InstanceVariableSetterPtr;
typedef SmartPtr<String> StringPtr;
typedef SmartPtr<ID> IDPtr;
typedef SmartPtr<Code> CodePtr;
typedef SmartPtr<Arguments> ArgumentsPtr;
typedef SmartPtr<VMachine> VMachinePtr;
typedef SmartPtr<NativeMethod> NativeFunPtr;
typedef SmartPtr<Frame> FramePtr;
typedef SmartPtr<Class> ClassPtr;
typedef SmartPtr<Lib> LibPtr;
typedef SmartPtr<Thread> ThreadPtr;
typedef SmartPtr<Mutex> MutexPtr;
typedef SmartPtr<IntRange> IntRangePtr;
typedef SmartPtr<FloatRange> FloatRangePtr;
typedef SmartPtr<ChRange> ChRangePtr;
typedef SmartPtr<DoubleDispatchMethod> DoubleDispatchMethodPtr;
typedef SmartPtr<DoubleDispatchFun> DoubleDispatchFunPtr;
typedef SmartPtr<Values> ValuesPtr;
typedef SmartPtr<Exception> ExceptionPtr;

class Base;
class RefCountingBase;
class GCObserver;

class Int;
class Float;
class Undefined;
class Bool;

class Visitor;
class InstanceVariables;

/**
* \brief スコープ情報
*/
struct ScopeInfo{
	ScopeInfo()
		:pc(0), kind(0), flags(0), variable_identifier_offset(0), variable_size(0){}

	u32 pc;
	u8 kind;
	u8 flags;
	u16 variable_identifier_offset;
	u16 variable_size;

	enum{
		FLAG_SCOPE_CHAIN = 1<<0,

		FLAG_USED_BIT = 1
	};
};

/**
* \brief クラススコープ情報
*/
struct ClassInfo : public ScopeInfo{
	ClassInfo(u16 size = 0, u16 offset = 0)
		:instance_variable_size(size), instance_variable_identifier_offset(offset), name_number(0), mixins(0){}

	u16 instance_variable_size;
	u16 instance_variable_identifier_offset;
	u16 name_number;
	u8 mixins;
};

/**
* \brief 関数スコープ情報
*/
struct FunInfo : public ScopeInfo{
	FunInfo()
		:max_stack(256), max_variable(0), name_number(0), min_param_count(0), max_param_count(0){}

	u16 max_stack;
	u16 max_variable;
	u16 name_number;
	u8 min_param_count;
	u8 max_param_count;

	enum{
		FLAG_EXTENDABLE_PARAM = 1<<(ScopeInfo::FLAG_USED_BIT+0),

		FLAG_USED_BIT = ScopeInfo::FLAG_USED_BIT+1
	};
};

/**
* \brief 例外スコープ情報
*/
struct ExceptInfo{
	ExceptInfo()
		:catch_pc(0), finally_pc(0), end_pc(0){}

	u32 catch_pc;
	u32 finally_pc;
	u32 end_pc;
};

class EmptyInstanceVariables;

extern ScopeInfo empty_scope_info;
extern ClassInfo empty_class_info;
extern FunInfo empty_xfun_info;
extern ExceptInfo empty_except_info;
extern EmptyInstanceVariables empty_instance_variables;

/**
* \brief 空文字列
*/
extern IDPtr empty_id;

/**
* \brief 空文字列
*/
extern StringPtr empty_string;

/**
* \brief nullオブジェクトのインスタンス
*/
extern NullPtr null;

/**
* \brief 未定義値
*/
extern UndefinedPtr undefined;

struct CppClassSymbolData;
extern CppClassSymbolData** classdata[];

}

#endif // XTAL_UTILITY_H_INCLUDE_GUARD
