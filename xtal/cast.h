
#pragma once

//#include "frame.h"
#include "userdata.h"

namespace xtal{
	
/**
* @brief castŠÖ”AasŠÖ”Aarg_castŠÖ”‚Ì–ß‚è’l‚ÌŒ^‚ğŒˆ’è‚·‚é‚½‚ß‚Ìƒwƒ‹ƒp[ƒeƒ“ƒvƒŒ[ƒgƒNƒ‰ƒX
*
* ‚Ù‚Æ‚ñ‚Ç‚Ìê‡ACastResult<T>::type‚ÍT©g‚ğ•Ô‚·B
* ˆÙ‚È‚éê‡‚Æ‚µ‚ÄCastResult<const int&>::type ‚Íint‚ğ•Ô‚·B
*/
template<class T>
struct CastResult{ typedef T type; };

template<class T> 
inline typename CastResult<T>::type as(const Any& a);
	
template<class T>
inline typename CastResult<T>::type cast(const Any& a);
	
template<class T>
inline typename CastResult<T>::type arg_cast(const Any& a, int param_num, const Any& param_name);

	
// •ÏŠ·Œã‚ÌŒ^‚ªUserData‚Ìê‡
template<class U, class V>
inline UserData<U>* as_helper_helper(const Any& a, const UserData<U>*, const V&){
	if(a.is(TClass<U>::get())){ return (UserData<U>*)&a; }
	return 0;
}

// •ÏŠ·Œã‚ÌŒ^‚ªAnyImpl‚ğŒp³‚µ‚½Œ^‚Ìê‡
template<class U>
inline U* as_helper_helper(const Any& a, const AnyImpl*, const U*){
	if(a.is(TClass<U>::get())){ return (U*)a.impl(); }
	return 0;
}
	
// •ÏŠ·Œã‚ÌŒ^‚ªAny‚ğŒp³‚µ‚½Œ^‚Ìê‡
template<class U>
inline U* as_helper_helper(const Any& a, const Any*, const U*){
	if(a.is(TClass<U>::get())){ return (U*)&a; }
	return 0;
}
	
// •ÏŠ·Œã‚ÌŒ^‚ªAny‚âAnyImpl‚ğŒp³‚µ‚Ä‚¢‚È‚¢Œ^‚Ìê‡
template<class U>
inline U* as_helper_helper(const Any& a, const void*, const U*){
	if(a.is(TClass<U>::get())){ return ((UserData<U>&)a).get(); }
	return 0;
}


// •ÏŠ·Œã‚ÌŒ^‚ªAnyImpl‚ğŒp³‚µ‚½Œ^‚Ìê‡
void* cast_helper_helper_extend_anyimpl(const Any& a, const Class& cls);

// •ÏŠ·Œã‚ÌŒ^‚ªAny‚ğŒp³‚µ‚½Œ^‚Ìê‡
void* cast_helper_helper_extend_any(const Any& a, const Class& cls);

// •ÏŠ·Œã‚ÌŒ^‚ªAny‚âAnyImpl‚ğŒp³‚µ‚Ä‚¢‚È‚¢Œ^‚Ìê‡
void* cast_helper_helper_other(const Any& a, const Class& cls);

// •ÏŠ·Œã‚ÌŒ^‚ªUserData‚Ìê‡
template<class U, class V>
inline UserData<U>* cast_helper_helper(const Any& a, const UserData<U>*, const V&){
	return (UserData<U>*)cast_helper_helper_extend_any(a, TClass<U>::get());
}

// •ÏŠ·Œã‚ÌŒ^‚ªAnyImpl‚ğŒp³‚µ‚½Œ^‚Ìê‡
template<class U>
inline U* cast_helper_helper(const Any& a, const AnyImpl*, const U*){
	return (U*)cast_helper_helper_extend_anyimpl(a, TClass<U>::get());
}
	
// •ÏŠ·Œã‚ÌŒ^‚ªAny‚ğŒp³‚µ‚½Œ^‚Ìê‡
template<class U>
inline U* cast_helper_helper(const Any& a, const Any*, const U*){
	return (U*)cast_helper_helper_extend_any(a, TClass<U>::get());
}
	
// •ÏŠ·Œã‚ÌŒ^‚ªAny‚âAnyImpl‚ğŒp³‚µ‚Ä‚¢‚È‚¢Œ^‚Ìê‡
template<class U>
inline U* cast_helper_helper(const Any& a, const void*, const U*){
	return (U*)cast_helper_helper_other(a, TClass<U>::get());
}


// •ÏŠ·Œã‚ÌŒ^‚ªAnyImpl‚ğŒp³‚µ‚½Œ^‚Ìê‡
void* arg_cast_helper_helper_extend_anyimpl(const Any& a, int param_num, const Any& param_name, const Class& cls);

// •ÏŠ·Œã‚ÌŒ^‚ªAny‚ğŒp³‚µ‚½Œ^‚Ìê‡
void* arg_cast_helper_helper_extend_any(const Any& a, int param_num, const Any& param_name, const Class& cls);

// •ÏŠ·Œã‚ÌŒ^‚ªAny‚âAnyImpl‚ğŒp³‚µ‚Ä‚¢‚È‚¢Œ^‚Ìê‡
void* arg_cast_helper_helper_other(const Any& a, int param_num, const Any& param_name, const Class& cls);


// •ÏŠ·Œã‚ÌŒ^‚ªUserData‚Ìê‡
template<class U, class V>
inline UserData<U>* arg_cast_helper_helper(const Any& a, int param_num, const Any& param_name, const UserData<U>*, const V&){
	return (UserData<U>*)arg_cast_helper_helper_extend_any(a, param_num, param_name, TClass<U>::get());
}

// •ÏŠ·Œã‚ÌŒ^‚ªAnyImpl‚ğŒp³‚µ‚½Œ^‚Ìê‡
template<class U>
inline U* arg_cast_helper_helper(const Any& a, int param_num, const Any& param_name, const AnyImpl*, const U*){
	return (U*)arg_cast_helper_helper_extend_anyimpl(a, param_num, param_name, TClass<U>::get());
}
	
// •ÏŠ·Œã‚ÌŒ^‚ªAny‚ğŒp³‚µ‚½Œ^‚Ìê‡
template<class U>
inline U* arg_cast_helper_helper(const Any& a, int param_num, const Any& param_name, const Any*, const U*){
	return (U*)arg_cast_helper_helper_extend_any(a, param_num, param_name, TClass<U>::get());
}
	
// •ÏŠ·Œã‚ÌŒ^‚ªAny‚âAnyImpl‚ğŒp³‚µ‚Ä‚¢‚È‚¢Œ^‚Ìê‡
template<class U>
inline U* arg_cast_helper_helper(const Any& a, int param_num, const Any& param_name, const void*, const U*){
	return (U*)arg_cast_helper_helper_other(a, param_num, param_name, TClass<U>::get());
}


template<class T>
struct CastHelper{

	// •ÏŠ·Œã‚ÌŒ^‚ªƒ|ƒCƒ“ƒ^‚Ìê‡
	template<class U> static T as_inner(const Any& a, U* (*)()){ return as_helper_helper(a, (U*)0, (U*)0); }
	template<class U> static T cast_inner(const Any& a, U* (*)()){ return cast_helper_helper(a, (U*)0, (U*)0); }
	template<class U> static T arg_cast_inner(const Any& a, int param_num, const Any& param_name, U* (*)()){ return arg_cast_helper_helper(a, param_num, param_name, (U*)0, (U*)0); }
	
	// •ÏŠ·Œã‚ÌŒ^‚ªQÆ‚Ìê‡
	template<class U> static T as_inner(const Any& a, U& (*)()){ if(U* ret = xtal::as<U*>(a)){ return *ret; }else{ return *(U*)&null;} }
	template<class U> static T cast_inner(const Any& a, U& (*)()){ return *xtal::cast<U*>(a); }
	template<class U> static T arg_cast_inner(const Any& a, int param_num, const Any& param_name, U& (*)()){ return *xtal::arg_cast<U*>(a, param_num, param_name); }

	// •ÏŠ·Œã‚ÌŒ^‚ªQÆ‚Å‚àƒ|ƒCƒ“ƒ^‚Å‚à‚È‚¢ê‡
	static T as_inner(const Any& a, ...){ if(const T* ret = xtal::as<const T*>(a)){ return *ret; }else{ return *(const T*)&null;} }
	static T cast_inner(const Any& a, ...){ return *xtal::cast<const T*>(a); }
	static T arg_cast_inner(const Any& a, int param_num, const Any& param_name, ...){ return *xtal::arg_cast<const T*>(a, param_num, param_name); }

	static T as(const Any& a){ return as_inner(a, (T (*)())0); }
	static T cast(const Any& a){ return cast_inner(a, (T (*)())0); }
	static T arg_cast(const Any& a, int param_num, const Any& param_name){ return arg_cast_inner(a, param_num, param_name, (T (*)())0); }

};


/**
* @brief TŒ^‚É•ÏŠ·‚·‚éB
*
* T‚É•ÏŠ·‚Å‚«‚È‚¢ê‡A
* T‚ªƒ|ƒCƒ“ƒ^Œ^‚È‚çNULL‚ğ•Ô‚·B
* T‚ª’l‚©QÆ‚ÅAAny‚ğŒp³‚µ‚½Œ^‚È‚çxtal::null‚ğ•Ô‚·B
* ‚»‚êˆÈŠO‚ÌŒ^‚Ìê‡‚Í–¢’è‹`B
*/
template<class T> 
inline typename CastResult<T>::type 
as(const Any& a){
	return CastHelper<T>::as(a);
}

/**
* @brief TŒ^‚É•ÏŠ·‚·‚éB
*
* TŒ^‚É•ÏŠ·‚Å‚«‚È‚¢ê‡Abuiltin().member("BadCastError")‚ª“Š‚°‚ç‚ê‚é
*/
template<class T>
inline typename CastResult<T>::type 
cast(const Any& a){
	return CastHelper<T>::cast(a);
}

/**
* @brief TŒ^‚É•ÏŠ·‚·‚éB
*
* TŒ^‚É•ÏŠ·‚Å‚«‚È‚¢ê‡Abuiltin().member("InvalidArgumentError")‚ª“Š‚°‚ç‚ê‚é
*/
template<class T>
inline typename CastResult<T>::type 
arg_cast(const Any& a, int param_num, const Any& param_name){
	return CastHelper<T>::arg_cast(a, param_num, param_name);
}

template<>
struct CastHelper<Any*>{
	static Any* as(const Any& a){ return (Any*)&a; }
	static Any* cast(const Any& a){ return (Any*)&a; }
	static Any* arg_cast(const Any& a, int param_num, const Any& param_name){ return (Any*)&a; }
};

template<>
struct CastHelper<const Any*>{
	static const Any* as(const Any& a){ return &a; }
	static const Any* cast(const Any& a){ return &a; }
	static const Any* arg_cast(const Any& a, int param_num, const Any& param_name){ return &a; }
};

template<>
struct CastHelper<const ID*>{
	static const ID* as(const Any& a);
	static const ID* cast(const Any& a);
	static const ID* arg_cast(const Any& a, int param_num, const Any& param_name);
};

template<>
struct CastHelper<const char*>{
	static const char* as(const Any& a);
	static const char* cast(const Any& a);
	static const char* arg_cast(const Any& a, int param_num, const Any& param_name);
};


template<>
struct CastHelper<int_t>{
	static int_t as(const Any& a);
	static int_t cast(const Any& a);
	static int_t arg_cast(const Any& a, int param_num, const Any& param_name);
};

template<>
struct CastHelper<float_t>{
	static float_t as(const Any& a);
	static float_t cast(const Any& a);
	static float_t arg_cast(const Any& a, int param_num, const Any& param_name);
};

template<>
struct CastHelper<bool>{
	static bool as(const Any& a){ return a.to_b(); }
	static bool cast(const Any& a){ return a.to_b(); }
	static bool arg_cast(const Any& a, int param_num, const Any& param_name){ return a.to_b(); }
};

#define XTAL_CAST_HELPER(Type) \
template<> struct CastResult<const Type&>{ typedef Type type; };\
template<>\
struct CastHelper<const Type&>{\
	static Type as(const Any& a){ return xtal::as<Type>(a); }\
	static Type cast(const Any& a){ return xtal::cast<Type>(a); }\
	static Type arg_cast(const Any& a, int param_num, const Any& param_name){ return xtal::arg_cast<Type>(a, param_num, param_name); }\
}

XTAL_CAST_HELPER(int_t);
XTAL_CAST_HELPER(float_t);
XTAL_CAST_HELPER(bool);

#undef XTAL_CAST_HELPER

template<>
struct CastHelper<check_xtype<int>::type>{
	static int as(const Any& a){ return CastHelper<int_t>::as(a); }
	static int cast(const Any& a){ return CastHelper<int_t>::cast(a); }
	static int arg_cast(const Any& a, int param_num, const Any& param_name){ return CastHelper<int_t>::arg_cast(a, param_num, param_name); }
};

template<>
struct CastHelper<check_xtype<long>::type>{
	static long as(const Any& a){ return CastHelper<int_t>::as(a); }
	static long cast(const Any& a){ return CastHelper<int_t>::cast(a); }
	static long arg_cast(const Any& a, int param_num, const Any& param_name){ return CastHelper<int_t>::arg_cast(a, param_num, param_name); }
};

template<>
struct CastHelper<check_xtype<short>::type>{
	static short as(const Any& a){ return (short)CastHelper<int_t>::as(a); }
	static short cast(const Any& a){ return (short)CastHelper<int_t>::cast(a); }
	static short arg_cast(const Any& a, int param_num, const Any& param_name){ return (short)CastHelper<int_t>::arg_cast(a, param_num, param_name); }
};

template<>
struct CastHelper<check_xtype<char>::type>{
	static char as(const Any& a){ return (char)CastHelper<int_t>::as(a); }
	static char cast(const Any& a){ return (char)CastHelper<int_t>::cast(a); }
	static char arg_cast(const Any& a, int param_num, const Any& param_name){ return (char)CastHelper<int_t>::arg_cast(a, param_num, param_name); }
};

template<>
struct CastHelper<check_xtype<unsigned int>::type>{
	static unsigned int as(const Any& a){ return CastHelper<int_t>::as(a); }
	static unsigned int cast(const Any& a){ return CastHelper<int_t>::cast(a); }
	static unsigned int arg_cast(const Any& a, int param_num, const Any& param_name){ return CastHelper<int_t>::arg_cast(a, param_num, param_name); }
};

template<>
struct CastHelper<check_xtype<unsigned long>::type>{
	static unsigned long as(const Any& a){ return CastHelper<int_t>::as(a); }
	static unsigned long cast(const Any& a){ return CastHelper<int_t>::cast(a); }
	static unsigned long arg_cast(const Any& a, int param_num, const Any& param_name){ return CastHelper<int_t>::arg_cast(a, param_num, param_name); }
};

template<>
struct CastHelper<check_xtype<unsigned short>::type>{
	static unsigned short as(const Any& a){ return (unsigned short)CastHelper<int_t>::as(a); }
	static unsigned short cast(const Any& a){ return (unsigned short)CastHelper<int_t>::cast(a); }
	static unsigned short arg_cast(const Any& a, int param_num, const Any& param_name){ return (unsigned short)CastHelper<int_t>::arg_cast(a, param_num, param_name); }
};

template<>
struct CastHelper<check_xtype<unsigned char>::type>{
	static unsigned char as(const Any& a){ return (unsigned char)CastHelper<int_t>::as(a); }
	static unsigned char cast(const Any& a){ return (unsigned char)CastHelper<int_t>::cast(a); }
	static unsigned char arg_cast(const Any& a, int param_num, const Any& param_name){ return (unsigned char)CastHelper<int_t>::arg_cast(a, param_num, param_name); }
};

template<>
struct CastHelper<check_xtype<signed char>::type>{
	static signed char as(const Any& a){ return (signed char)CastHelper<int_t>::as(a); }
	static signed char cast(const Any& a){ return (signed char)CastHelper<int_t>::cast(a); }
	static signed char arg_cast(const Any& a, int param_num, const Any& param_name){ return (signed char)CastHelper<int_t>::arg_cast(a, param_num, param_name); }
};

template<>
struct CastHelper<check_xtype<float>::type>{
	static float as(const Any& a){ return (float)CastHelper<float_t>::as(a); }
	static float cast(const Any& a){ return (float)CastHelper<float_t>::cast(a); }
	static float arg_cast(const Any& a, int param_num, const Any& param_name){ return (float)CastHelper<float_t>::arg_cast(a, param_num, param_name); }
};

template<>
struct CastHelper<check_xtype<double>::type>{
	static double as(const Any& a){ return CastHelper<float_t>::as(a); }
	static double cast(const Any& a){ return CastHelper<float_t>::cast(a); }
	static double arg_cast(const Any& a, int param_num, const Any& param_name){ return CastHelper<float_t>::arg_cast(a, param_num, param_name); }
};

template<>
struct CastHelper<check_xtype<long double>::type>{
	static long double as(const Any& a){ return CastHelper<float_t>::as(a); }
	static long double cast(const Any& a){ return CastHelper<float_t>::cast(a); }
	static long double arg_cast(const Any& a, int param_num, const Any& param_name){ return CastHelper<float_t>::arg_cast(a, param_num, param_name); }
};

#define XTAL_CAST_HELPER(Type) \
template<> struct CastResult<check_xtype<const Type&> >{ typedef Type type; };\
template<>\
struct CastHelper<check_xtype<const Type&> >{\
	static Type as(const Any& a){ return xtal::as<Type>(a); }\
	static Type cast(const Any& a){ return xtal::cast<Type>(a); }\
	static Type arg_cast(const Any& a, int param_num, const Any& param_name){ return xtal::arg_cast<Type>(a, param_num, param_name); }\
}

XTAL_CAST_HELPER(int);
XTAL_CAST_HELPER(unsigned int);
XTAL_CAST_HELPER(long);
XTAL_CAST_HELPER(unsigned long);
XTAL_CAST_HELPER(short);
XTAL_CAST_HELPER(unsigned short);
XTAL_CAST_HELPER(char);
XTAL_CAST_HELPER(signed char);
XTAL_CAST_HELPER(unsigned char);
XTAL_CAST_HELPER(float);
XTAL_CAST_HELPER(double);
XTAL_CAST_HELPER(long double);

#undef XTAL_CAST_HELPER

}
