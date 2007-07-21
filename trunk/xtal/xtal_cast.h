
#pragma once

//#include "xtal_frame.h"
#include "xtal_userdata.h"

namespace xtal{
	
template<class T>
struct CastCacheType{
	static CastCacheType<T> instance;
private:
	CastCacheType(){}
	CastCacheType(const CastCacheType&);
	CastCacheType& operator=(const CastCacheType&);
};

template<class T>
CastCacheType<T> CastCacheType<T>::instance;

const void* fetch_cast_cache(const Any& a, const void* type_key);
void store_cast_cache(const Any& a, const void* ret, const void* type_key);

template<class T>
inline const void* fetch_cast_cache(const Any& a){
	return fetch_cast_cache(a, &CastCacheType<T>::instance);
}

template<class T>
inline void store_cast_cache(const Any& a, const void* ret){
	store_cast_cache(a, ret, &CastCacheType<T>::instance);
}

/**
* @brief cast関数、as関数、arg_cast関数の戻り値の型を決定するためのヘルパーテンプレートクラス
*
* ほとんどの場合、CastResult<T>::typeはT自身を返す。
* 異なる場合としてCastResult<const int&>::type はintを返す。
*/
template<class T>
struct CastResult{ typedef T type; };

template<class T> 
inline typename CastResult<T>::type as(const Any& a);
	
template<class T>
inline typename CastResult<T>::type cast(const Any& a);
	
template<class T>
inline typename CastResult<T>::type arg_cast(const Any& a, int param_num, const Any& param_name);

	
// 変換後の型がUserDataの場合
template<class U, class V>
inline UserData<U>* as_helper_helper(const Any& a, const UserData<U>*, const V&){
	if(a.is(TClass<U>::get())){ return (UserData<U>*)&a; }
	return 0;
}

// 変換後の型がAnyImplを継承した型の場合
template<class U>
inline U* as_helper_helper(const Any& a, const AnyImpl*, const U*){
	if(a.is(TClass<U>::get())){ return (U*)a.impl(); }
	return 0;
}
	
// 変換後の型がAnyを継承した型の場合
template<class U>
inline U* as_helper_helper(const Any& a, const Any*, const U*){
	if(a.is(TClass<U>::get())){ return (U*)&a; }
	return 0;
}
	
// 変換後の型がAnyやAnyImplを継承していない型の場合
template<class U>
inline U* as_helper_helper(const Any& a, const void*, const U*){
	if(a.is(TClass<U>::get())){ return ((UserData<U>&)a).get(); }
	return 0;
}


// 変換後の型がAnyImplを継承した型の場合
void* cast_helper_helper_extend_anyimpl(const Any& a, const Class& cls);

// 変換後の型がAnyを継承した型の場合
void* cast_helper_helper_extend_any(const Any& a, const Class& cls);

// 変換後の型がAnyやAnyImplを継承していない型の場合
void* cast_helper_helper_other(const Any& a, const Class& cls);

// 変換後の型がUserDataの場合
template<class U, class V>
inline UserData<U>* cast_helper_helper(const Any& a, const UserData<U>*, const V&){
	return (UserData<U>*)cast_helper_helper_extend_any(a, TClass<U>::get());
}

// 変換後の型がAnyImplを継承した型の場合
template<class U>
inline U* cast_helper_helper(const Any& a, const AnyImpl*, const U*){
	return (U*)cast_helper_helper_extend_anyimpl(a, TClass<U>::get());
}
	
// 変換後の型がAnyを継承した型の場合
template<class U>
inline U* cast_helper_helper(const Any& a, const Any*, const U*){
	return (U*)cast_helper_helper_extend_any(a, TClass<U>::get());
}
	
// 変換後の型がAnyやAnyImplを継承していない型の場合
template<class U>
inline U* cast_helper_helper(const Any& a, const void*, const U*){
	return (U*)cast_helper_helper_other(a, TClass<U>::get());
}


// 変換後の型がAnyImplを継承した型の場合
void* arg_cast_helper_helper_extend_anyimpl(const Any& a, int param_num, const Any& param_name, const Class& cls);

// 変換後の型がAnyを継承した型の場合
void* arg_cast_helper_helper_extend_any(const Any& a, int param_num, const Any& param_name, const Class& cls);

// 変換後の型がAnyやAnyImplを継承していない型の場合
void* arg_cast_helper_helper_other(const Any& a, int param_num, const Any& param_name, const Class& cls);


// 変換後の型がUserDataの場合
template<class U, class V>
inline UserData<U>* arg_cast_helper_helper(const Any& a, int param_num, const Any& param_name, const UserData<U>*, const V&){
	return (UserData<U>*)arg_cast_helper_helper_extend_any(a, param_num, param_name, TClass<U>::get());
}

// 変換後の型がAnyImplを継承した型の場合
template<class U>
inline U* arg_cast_helper_helper(const Any& a, int param_num, const Any& param_name, const AnyImpl*, const U*){
	return (U*)arg_cast_helper_helper_extend_anyimpl(a, param_num, param_name, TClass<U>::get());
}
	
// 変換後の型がAnyを継承した型の場合
template<class U>
inline U* arg_cast_helper_helper(const Any& a, int param_num, const Any& param_name, const Any*, const U*){
	return (U*)arg_cast_helper_helper_extend_any(a, param_num, param_name, TClass<U>::get());
}
	
// 変換後の型がAnyやAnyImplを継承していない型の場合
template<class U>
inline U* arg_cast_helper_helper(const Any& a, int param_num, const Any& param_name, const void*, const U*){
	return (U*)arg_cast_helper_helper_other(a, param_num, param_name, TClass<U>::get());
}

template<class T>
struct TypeType{
	typedef char (&ptr)[1];
	typedef char (&ref)[2];
	typedef char (&other)[3];

	static T makeT();
	
	template<class U> static ptr testT(U* (*)());
	template<class U> static ref testT(U& (*)());
	static other testT(...);

	enum{ value = sizeof(testT((T (*)())0))-1 };
};

template<class T>
struct CastHelper{

	// 変換後の型がポインタの場合
	// このレベルで、キャッシュの機構を取り入れる
	template<class U> static T as_inner(const Any& a, U* (*)()){ 
		if(const void* p = fetch_cast_cache<T>(a)){
			return (T)p;
		}else{
			T ret = (T)as_helper_helper(a, (U*)0, (U*)0);
			store_cast_cache<T>(a, ret);
			return ret;
		}
	}
		
	template<class U> static T cast_inner(const Any& a, U* (*)()){ 
		if(const void* p = fetch_cast_cache<T>(a)){
			return (T)p;
		}else{
			T ret = (T)cast_helper_helper(a, (U*)0, (U*)0);
			store_cast_cache<T>(a, ret);
			return ret; 
		}
	}	
		
	template<class U> static T arg_cast_inner(const Any& a, int param_num, const Any& param_name, U* (*)()){ 
		if(const void* p = fetch_cast_cache<T>(a)){
			return (T)p;
		}else{
			T ret = (T)arg_cast_helper_helper(a, param_num, param_name, (U*)0, (U*)0);
			store_cast_cache<T>(a, ret);
			return ret; 
		}
	}
	
	
	// 変換後の型が参照の場合、ポインタ型としてキャストしたあと参照にする
	template<class U> static T as_inner(const Any& a, U& (*)()){ 
		if(U* ret = xtal::as<U*>(a)){ return *ret; }else{ return *(U*)&null;} }
		
	template<class U> static T cast_inner(const Any& a, U& (*)()){ 
		return *xtal::cast<U*>(a); }
		
	template<class U> static T arg_cast_inner(const Any& a, int param_num, const Any& param_name, U& (*)()){ 
		return *xtal::arg_cast<U*>(a, param_num, param_name); }


	// 変換後の型が参照でもポインタでもない場合、ポインタ型としてキャストしたあと実体にする
	static T as_inner(const Any& a, ...){ 
		if(const T* ret = xtal::as<const T*>(a)){ return *ret; }else{ return *(const T*)&null;} }
		
	static T cast_inner(const Any& a, ...){ 
		return *xtal::cast<const T*>(a); }
		
	static T arg_cast_inner(const Any& a, int param_num, const Any& param_name, ...){ 
		return *xtal::arg_cast<const T*>(a, param_num, param_name); }


	static T as(const Any& a){ 
		return as_inner(a, (T (*)())0); }
		
	static T cast(const Any& a){ 
		return cast_inner(a, (T (*)())0); }
		
	static T arg_cast(const Any& a, int param_num, const Any& param_name){ 
		return arg_cast_inner(a, param_num, param_name, (T (*)())0); }

};


/**
* @brief T型に変換する。
*
* Tに変換できない場合、
* Tがポインタ型ならNULLを返す。
* Tが値か参照で、Anyを継承した型ならxtal::nullを返す。
* それ以外の型の場合は未定義。
*/
template<class T> 
inline typename CastResult<T>::type 
as(const Any& a){
	return CastHelper<T>::as(a);
}

/**
* @brief T型に変換する。
*
* T型に変換できない場合、builtin().member("CastError")が投げられる
*/
template<class T>
inline typename CastResult<T>::type 
cast(const Any& a){
	return CastHelper<T>::cast(a);
}

/**
* @brief T型に変換する。
*
* T型に変換できない場合、builtin().member("ArgumentError")が投げられる
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
	static Any* arg_cast(const Any& a, int, const Any&){ return (Any*)&a; }
};

template<>
struct CastHelper<const Any*>{
	static const Any* as(const Any& a){ return &a; }
	static const Any* cast(const Any& a){ return &a; }
	static const Any* arg_cast(const Any& a, int, const Any&){ return &a; }
};

template<>
struct CastHelper<const Any&>{
	static const Any& as(const Any& a){ return a; }
	static const Any& cast(const Any& a){ return a; }
	static const Any& arg_cast(const Any& a, int, const Any&){ return a; }
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
	static bool arg_cast(const Any& a, int, const Any&){ return a.to_b(); }
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
