
#pragma once

#include "xtal_utility.h"

#include <string>
#include <algorithm>
#include <stddef.h>


/**
* 全ての要素はxtal namespaceの中
*/
namespace xtal{

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


template<class T>
struct TypeValue{ 
	T val; 
	TypeValue(T val):val(val){} 
	operator T() const{ return val; }
};

template<class T>
struct check_xtype{ typedef T type; };

template<>
struct check_xtype<int_t>{ typedef TypeValue<int_t> type; };

template<>
struct check_xtype<float_t>{ typedef TypeValue<float_t> type; };


template<class T>
struct IsFloat{ enum{ value = 0 }; };
template<>
struct IsFloat<float_t>{ enum{ value = 1 }; };


template<class T>
struct Alloc;

typedef char char_t;
typedef std::basic_string<char_t, std::char_traits<char_t>, Alloc<char_t> > string_t;


/**
* @brief プリミティブな型の種類
*/
enum PrimitiveType{
	TYPE_NULL = 0,

	TYPE_FALSE = 1,
	TYPE_TRUE = 2,
	
	TYPE_BASE = 3,
	
	TYPE_INT = 4,
	TYPE_FLOAT = 5,
	
	TYPE_SMALL_STRING = 6,
	

	TYPE_MASK = (1<<0) | (1<<1) | (1<<2),
	TYPE_SHIFT = 3
};


/**
* @brief ブロックの種類
*/
enum{
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
enum{
	KIND_STRING,
	KIND_TEXT,
	KIND_FORMAT
};

/**
* @brief 可触性の種類
*/
enum{
	KIND_PUBLIC = 0,
	KIND_PROTECTED = 1<<0,
	KIND_PRIVATE = 1<<1,
};

/**
* @brief ブレークポイントの種類
*/
enum{
	BREAKPOINT_LINE,
	BREAKPOINT_CALL,
	BREAKPOINT_RETURN
};


template<class T>
class SmartPtr;

class Any;
template<> class SmartPtr<Any>;
typedef SmartPtr<Any> AnyPtr;


class Array;
class Map;
class Stream;
class MemoryStream;
class FileStream;
class StringStream;
class Fun;
class Method;
class Fiber;
class InstanceVariableGetter;
class InstanceVariableSetter;
class Lambda;
class String;
class Code;
class Arguments;
class VMachine;
class CFun;
class Frame;
class Class;
class Lib;
class CppClass;
class Thread;
class Mutex;
class Singleton;

typedef SmartPtr<Array> ArrayPtr;
typedef SmartPtr<Map> MapPtr;
typedef SmartPtr<Stream> StreamPtr;
typedef SmartPtr<MemoryStream> MemoryStreamPtr;
typedef SmartPtr<FileStream> FileStreamPtr;
typedef SmartPtr<StringStream> StringStreamPtr;
typedef SmartPtr<Fun> FunPtr;
typedef SmartPtr<Fiber> FiberPtr;
typedef SmartPtr<InstanceVariableGetter> InstanceVariableGetterPtr;
typedef SmartPtr<InstanceVariableSetter> InstanceVariableSetterPtr;
typedef SmartPtr<String> StringPtr;
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

class Base;
class InternedStringPtr;
class AtProxy;
class Visitor;
class InstanceVariables;
class Int;
class Float;

struct BlockCore{
	BlockCore()
		:lineno(0), kind(0), variable_symbol_offset(0), variable_size(0){}

	u16 lineno;
	u16 kind;
	u16 variable_symbol_offset;
	u16 variable_size;
};

struct ClassCore : public BlockCore{
	ClassCore(u16 offset = 0, u16 size = 0)
		:instance_variable_symbol_offset(offset), instance_variable_size(size){}

	u16 instance_variable_symbol_offset;
	u16 instance_variable_size;
	u8 mixins;
};

struct FunCore : public BlockCore{
	FunCore()
		:pc(0), max_stack(256), min_param_count(0), max_param_count(0), used_args_object(0), on_heap(0){}

	u16 pc;
	u16 max_stack;
	u8 min_param_count;
	u8 max_param_count;
	u8 used_args_object;
	u8 on_heap;
};

struct ExceptCore{
	ExceptCore(u16 catch_pc = 0, u16 finally_pc = 0, u16 end_pc = 0)
		:catch_pc(catch_pc), finally_pc(finally_pc), end_pc(end_pc){}

	u16 catch_pc;
	u16 finally_pc;
	u16 end_pc;
};

extern BlockCore empty_block_core;
extern ClassCore empty_class_core;
extern FunCore empty_fun_core;
extern ExceptCore empty_except_core;

class Null;
extern Null null;

class True;
class False;

struct Result;
struct ReturnThis;
struct ReturnVoid;

extern Result result;
extern ReturnThis return_this;
extern ReturnVoid return_void;

template<class T>
const ClassPtr& new_cpp_class(const char* name = "");

template<class T>
inline bool exists_cpp_class();

template<class T>
inline const ClassPtr& get_cpp_class();

}
