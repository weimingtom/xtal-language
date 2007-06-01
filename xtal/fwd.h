
#pragma once

#include "allocator.h"
#include "utility.h"

#include <string>
#include <algorithm>
#include <stddef.h>


/**
* 全ての要素はxtal namespaceの中
*/
namespace xtal{

/// int
typedef int int_t;
	
/// float
typedef float float_t;
	
/// uint
typedef unsigned int uint_t;

/// byte
typedef unsigned char byte_t;

template<int N>
struct SelectInt{
	typedef typename If<sizeof(signed char)==N,
		signed char,
		typename If<sizeof(signed short)==N, 
			signed short,
			typename If<sizeof(signed int)==N,
				signed int,
				typename If<sizeof(signed long)==N,
					signed long,
					void
				>::type
			>::type
		>::type
	>::type signed_t;

	typedef typename If<sizeof(unsigned char)==N,
		unsigned char,
		typename If<sizeof(unsigned short)==N, 
			unsigned short,
			typename If<sizeof(unsigned int)==N,
				unsigned int,
				typename If<sizeof(unsigned long)==N,
					unsigned long,
					void
				>::type
			>::type
		>::type
	>::type unsigned_t;
};

/// 1-byte uint
typedef SelectInt<1>::unsigned_t u8;

/// 2-byte uint
typedef SelectInt<2>::unsigned_t u16;
	
/// 4-byte uint
typedef SelectInt<4>::unsigned_t u32;


/// 1-byte int
typedef SelectInt<1>::signed_t s8;

/// 2-byte int
typedef SelectInt<2>::signed_t s16;
	
/// 4-byte int
typedef SelectInt<4>::signed_t s32;

typedef char char_t;

typedef std::basic_string<char_t, std::char_traits<char_t>, Alloc<char_t> > string_t;

/**
* プリミティブな型を示す整数値
*/
enum PrimitiveType{
	TYPE_NULL = 0,
	TYPE_BASE = 1,
	TYPE_INT = 2,
	TYPE_FLOAT = 3,
	
	TYPE_FALSE = 4,
	TYPE_TRUE = 5,

	TYPE_MASK = (1<<0) | (1<<1) | (1<<2),
	TYPE_SHIFT = 3
};

class Any;

class Int;
class Float;

class Array;
class Code;
class Map;
class Fun;
class Method;
class Fiber;
class Prop;
class VMachine;
class Frame;
class ClassImpl;
class Object;
class Instance;
class HaveInstanceVariables;
class Arguments;
class Lib;
struct Named;
class GCObserverImpl;
class Class;
class String;
class ID;
class Visitor;
class FrameImpl;
class ClassImpl;
class StringImpl;
class AnyImpl;
class Stream;

template<class T> class UserData;
template<class T> class TClass;

const VMachine& vmachine();

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


class Null;
extern Null null;

void add_long_life_var(Any* a, int_t n = 1);
void remove_long_life_var(Any* a, int_t n = 1);

Any* make_place();

void initialize();
void uninitialize();
void initialize_lib();
bool initialized();

Any cast_error(const Any& from, const Any& to);
Any argument_error(const Any& from, const Any& to, int param_num, const Any& param_name);
Any unsupported_error(const Any& name, const Any& member);

template<class T>
UserData<T> new_userdata();

template<class T, class A0>
UserData<T> new_userdata(const A0& a0);

template<class T, class A0, class A1>
UserData<T> new_userdata(const A0& a0, const A1& a1);

template<class T, class A0, class A1, class A2>
UserData<T> new_userdata(const A0& a0, const A1& a1, const A2& a2);

template<class T, class A0, class A1, class A2, class A3>
UserData<T> new_userdata(const A0& a0, const A1& a1, const A2& a2, const A3& a3);

template<class T, class A0, class A1, class A2, class A3, class A4>
UserData<T> new_userdata(const A0& a0, const A1& a1, const A2& a2, const A3& a3, const A4& a4);


void visit_members(Visitor& m, const Any& value);

}
