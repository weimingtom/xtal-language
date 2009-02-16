
#pragma once

namespace xtal{
	enum{
		VERSION1 = 0,
		VERSION2 = 9,
		VERSION3 = 9,
		VERSION4 = 1
	};
}

/**
* 全ての要素はxtal namespaceの中
*/
namespace xtal{

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

typedef std::size_t size_t;
typedef std::ptrdiff_t ptrdiff_t;

template<class T>
struct Alloc;

#ifdef XTAL_USE_WCHAR
typedef wchar_t char_t;
#else
typedef char char_t;
#endif

/// unsigned char_t
typedef SelectType<sizeof(char_t)>::uint_t uchar_t;

template<class T>
struct TypeValue{ 
	T val; 
	TypeValue(T val):val(val){} 
	operator T() const{ return val; }
};

template<class T>
struct avoid{ typedef T type; };

template<>
struct avoid<int_t>{ typedef TypeValue<int_t> type; };

template<>
struct avoid<float_t>{ typedef TypeValue<float_t> type; };

template<>
struct avoid<char_t>{ typedef TypeValue<char_t> type; };

template<class T>
struct IsFloat{ enum{ value = 0 }; };
template<>
struct IsFloat<float_t>{ enum{ value = 1 }; };


/**
* @brief プリミティブな型の種類
*/
enum PrimitiveType{
	TYPE_NULL = 0,
	TYPE_UNDEFINED = 1,

	TYPE_FALSE = 2, // ここから上は、ifなどで偽と評価される
	TYPE_TRUE = 3,
	
	TYPE_BASE = 4,
	
	TYPE_INT = 5,
	TYPE_FLOAT = 6,
	
	TYPE_SMALL_STRING = 7,

	TYPE_STRING = 8,

	TYPE_SHIFT = 4,
	TYPE_MASK = (1<<TYPE_SHIFT)-1
};


/**
* @brief ブロックの種類
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
* @brief 文字列の種類
*/
enum StringKind{
	KIND_STRING,
	KIND_TEXT,
	KIND_FORMAT
};

/**
* @brief 可触性の種類
*/
enum AccessibilityKind{
	KIND_PUBLIC = 0,
	KIND_PROTECTED = 1<<0,
	KIND_PRIVATE = 1<<1
};

/**
* @brief ブレークポイントの種類
*/
enum BreakPointKind{
	BREAKPOINT,
	BREAKPOINT_CALL,
	BREAKPOINT_RETURN,
	BREAKPOINT_THROW,
	BREAKPOINT_ASSERT
};

/**
* @brief 範囲区間の種類
*/
enum RangeKind{
	RANGE_CLOSED = (0<<1) | (0<<0),
	RANGE_LEFT_CLOSED_RIGHT_OPEN = (0<<1) | (1<<0),
	RANGE_LEFT_OPEN_RIGHT_CLOSED = (1<<1) | (0<<0),
	RANGE_OPEN = (1<<1) | (1<<0)
};



template<class T>
class SmartPtr;

class Any;
template<> class SmartPtr<Any>;
typedef SmartPtr<Any> AnyPtr;


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
class CFun;
class Frame;
class Class;
class Lib;
class CppClass;
class CppSingleton;
class Thread;
class Mutex;
class Singleton;
class IntRange;
class FloatRange;
class ChRange;
class DualDispatchMethod;
class DualDispatchFun;
class MultiValue;
class Debug;
class DebugInfo;
class Exception;

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
typedef SmartPtr<CFun> CFunPtr;
typedef SmartPtr<Frame> FramePtr;
typedef SmartPtr<Class> ClassPtr;
typedef SmartPtr<Lib> LibPtr;
typedef SmartPtr<Thread> ThreadPtr;
typedef SmartPtr<Mutex> MutexPtr;
typedef SmartPtr<Singleton> SingletonPtr;
typedef SmartPtr<IntRange> IntRangePtr;
typedef SmartPtr<FloatRange> FloatRangePtr;
typedef SmartPtr<ChRange> ChRangePtr;
typedef SmartPtr<DualDispatchMethod> DualDispatchMethodPtr;
typedef SmartPtr<DualDispatchFun> DualDispatchFunPtr;
typedef SmartPtr<MultiValue> MultiValuePtr;
typedef SmartPtr<Exception> ExceptionPtr;

class Base;
class RefCountingBase;

class Int;
class Float;
class Undefined;
class Bool;

class Visitor;
class InstanceVariables;

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

struct ClassInfo : public ScopeInfo{
	ClassInfo(u16 offset = 0, u16 size = 0)
		:instance_variable_identifier_offset(offset), instance_variable_size(size){}

	u16 instance_variable_identifier_offset;
	u16 instance_variable_size;
	u8 mixins;
};

struct FunInfo : public ScopeInfo{
	FunInfo()
		:max_stack(256), min_param_count(0), max_param_count(0){}

	u16 max_stack;
	u8 min_param_count;
	u8 max_param_count;

	enum{
		FLAG_EXTENDABLE_PARAM = 1<<(ScopeInfo::FLAG_USED_BIT+0),
		FLAG_ON_HEAP = 1<<(ScopeInfo::FLAG_USED_BIT+1),

		FLAG_USED_BIT = ScopeInfo::FLAG_USED_BIT+2
	};
};

struct ExceptInfo{
	ExceptInfo()
		:catch_pc(0), finally_pc(0), end_pc(0){}

	u32 catch_pc;
	u32 finally_pc;
	u32 end_pc;
};

class EmptyInstanceVariables;
class Null;
class Undefined;
struct Named;

extern ScopeInfo empty_scope_info;
extern ClassInfo empty_class_info;
extern FunInfo empty_xfun_info;
extern ExceptInfo empty_except_info;
extern EmptyInstanceVariables empty_instance_variables;
extern IDPtr empty_id;
extern StringPtr empty_string;
extern Null null;
extern Undefined undefined;
extern Named null_named;

struct ParamInfo;
struct VMAndData;

struct CppClassSymbolData{ int value; };

template<class T>
struct CppClassSymbol{
	static CppClassSymbolData value;
};

template<class T>
CppClassSymbolData CppClassSymbol<T>::value = {0xc1a55};

template<class T> struct CppClassSymbol<T&> : public CppClassSymbol<T>{};
template<class T> struct CppClassSymbol<T*> : public CppClassSymbol<T>{};
template<class T> struct CppClassSymbol<const T> : public CppClassSymbol<T>{};
template<class T> struct CppClassSymbol<volatile T> : public CppClassSymbol<T>{};
template<class T> struct CppClassSymbol<SmartPtr<T> > : public CppClassSymbol<T>{};

template<class T>
const ClassPtr& new_cpp_class(const StringPtr& name=empty_string);

}
