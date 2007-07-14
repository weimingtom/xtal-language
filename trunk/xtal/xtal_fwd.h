
#pragma once

#include "xtal_allocator.h"
#include "xtal_utility.h"

#include <string>
#include <algorithm>
#include <stddef.h>


/**
* 全ての要素はxtal namespaceの中
*/
namespace xtal{

typedef long long int llint;
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


typedef char char_t;
typedef std::basic_string<char_t, std::char_traits<char_t>, Alloc<char_t> > string_t;

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

typedef SelectType<sizeof(void*)>::uint_t thread_id_t;


/**
* プリミティブな型を示す整数値
*/
enum PrimitiveType{
	TYPE_NULL,
	TYPE_FALSE,
	TYPE_TRUE,
	TYPE_BASE,
	TYPE_INT,
	TYPE_FLOAT,
	TYPE_NOP,

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

template<class T>
struct IsFloat{ enum{ value = 0 }; };
template<>
struct IsFloat<float_t>{ enum{ value = 1 }; };

template<class T, class U>
struct NumericCalcResultType{
	typedef typename If<1, T, U>::type type;
};

class Null;
extern Null null;

class Nop;
extern Nop nop;

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
