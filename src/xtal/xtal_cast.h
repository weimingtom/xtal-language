
#pragma once

namespace xtal{
	
/**
* @brief cast関数、as関数、arg_cast関数の戻り値の型を決定するためのヘルパーテンプレートクラス
*
* ほとんどの場合、CastResult<T>::typeはT自身を返す。
* 異なる場合としては、例えばCastResult<const int&>::type はintを返す。
*/
template<class T>
struct CastResult{ typedef T type; };

template<class T> 
inline typename CastResult<T>::type as(const AnyPtr& a);
	
template<class T>
inline typename CastResult<T>::type cast(const AnyPtr& a);
	
template<class T>
inline typename CastResult<T>::type arg_cast(const AnyPtr& a, int_t param_num, const AnyPtr& param_name);


/////////////////////////////////////////////////////////////////////////////

// 変換後の型がSmartPtrの場合
template<class U, class V>
inline const void* as_helper_helper(const AnyPtr& a, const SmartPtr<U>*, const V&){
	if(a->is(get_cpp_class<U>())){ return &a; }
	return 0;
}

// 変換後の型がBaseを継承した型の場合
template<class U>
inline const void* as_helper_helper(const AnyPtr& a, const Base*, const U*){
	if(a->is(get_cpp_class<U>())){ return pvalue(a); }
	return 0;
}

// 変換後の型がInnocenceを継承した型の場合
template<class U>
inline const void* as_helper_helper(const AnyPtr& a, const Innocence*, const U*){
	if(a->is(get_cpp_class<U>())){ return &a; }
	return 0;
}

// 変換後の型がInnocenceやBaseを継承していない型の場合
template<class U>
inline const void* as_helper_helper(const AnyPtr& a, const void*, const U*){
	if(a->is(get_cpp_class<U>())){ return ((SmartPtr<U>&)a).get(); }
	return 0;
}

/////////////////////////////////////////////////////////////////////////////

const void* cast_helper_helper_smartptr(const AnyPtr& a, const ClassPtr& cls);
const void* cast_helper_helper_base(const AnyPtr& a, const ClassPtr& cls);
const void* cast_helper_helper_innocence(const AnyPtr& a, const ClassPtr& cls);
const void* cast_helper_helper_other(const AnyPtr& a, const ClassPtr& cls);

// 変換後の型がSmartPtrの場合
template<class U, class V>
inline const void* cast_helper_helper(const AnyPtr& a, const SmartPtr<U>*, const V&){
	return cast_helper_helper_smartptr(a, get_cpp_class<U>()); 
}

// 変換後の型がBaseを継承した型の場合
template<class U>
inline const void* cast_helper_helper(const AnyPtr& a, const Base*, const U*){
	return cast_helper_helper_base(a, get_cpp_class<U>()); 
}
	
// 変換後の型がInnocenceを継承した型の場合
template<class U>
inline const void* cast_helper_helper(const AnyPtr& a, const Innocence*, const U*){
	return cast_helper_helper_innocence(a, get_cpp_class<U>()); 
}
	
// 変換後の型がAnyPtrやBaseを継承していない型の場合
template<class U>
inline const void* cast_helper_helper(const AnyPtr& a, const void*, const U*){
	return cast_helper_helper_other(a, get_cpp_class<U>()); 
}

/////////////////////////////////////////////////////////////////////////////

const void* arg_cast_helper_helper_smartptr(const AnyPtr& a, int_t param_num, const AnyPtr& param_name, const ClassPtr& cls);
const void* arg_cast_helper_helper_base(const AnyPtr& a, int_t param_num, const AnyPtr& param_name, const ClassPtr& cls);
const void* arg_cast_helper_helper_innocence(const AnyPtr& a, int_t param_num, const AnyPtr& param_name, const ClassPtr& cls);
const void* arg_cast_helper_helper_other(const AnyPtr& a, int_t param_num, const AnyPtr& param_name, const ClassPtr& cls);

// 変換後の型がSmartPtrの場合
template<class U, class V>
inline const void* arg_cast_helper_helper(const AnyPtr& a, int_t param_num, const AnyPtr& param_name, const SmartPtr<U>*, const V&){
	return arg_cast_helper_helper_smartptr(a, param_num, param_name, get_cpp_class<U>());
}

// 変換後の型がBaseを継承した型の場合
template<class U>
inline const void* arg_cast_helper_helper(const AnyPtr& a, int_t param_num, const AnyPtr& param_name, const Base*, const U*){
	return arg_cast_helper_helper_base(a, param_num, param_name, get_cpp_class<U>());
}
	
// 変換後の型がInnocenceを継承した型の場合
template<class U>
inline const void* arg_cast_helper_helper(const AnyPtr& a, int_t param_num, const AnyPtr& param_name, const Innocence*, const U*){
	return arg_cast_helper_helper_innocence(a, param_num, param_name, get_cpp_class<U>());
}
	
// 変換後の型がInnocenceやBaseを継承していない型の場合
template<class U>
inline const void* arg_cast_helper_helper(const AnyPtr& a, int_t param_num, const AnyPtr& param_name, const void*, const U*){
	return arg_cast_helper_helper_other(a, param_num, param_name, get_cpp_class<U>());
}

#ifdef XTAL_NO_TEMPLATE_PARTIAL_SPECIALIZATION

template<class T>
struct CastHelper{

	// 変換後の型がポインタの場合
	template<class U> static T as_inner(const AnyPtr& a, U* (*)()){ 
		return (T)as_helper_helper(a, (U*)0, (U*)0); }
		
	template<class U> static T cast_inner(const AnyPtr& a, U* (*)()){ 
		return (T)cast_helper_helper(a, (U*)0, (U*)0); }	
		
	template<class U> static T arg_cast_inner(const AnyPtr& a, int_t param_num, const AnyPtr& param_name, U* (*)()){ 
		return (T)arg_cast_helper_helper(a, param_num, param_name, (U*)0, (U*)0); }
	
	
	// 変換後の型が参照の場合、ポインタ型としてキャストしたあと参照にする
	template<class U> static T as_inner(const AnyPtr& a, U& (*)()){ 
		if(U* ret = xtal::as<U*>(a)){ return *ret; }else{ return *(U*)&null;} }
		
	template<class U> static T cast_inner(const AnyPtr& a, U& (*)()){ 
		return *xtal::cast<U*>(a); }
		
	template<class U> static T arg_cast_inner(const AnyPtr& a, int_t param_num, const AnyPtr& param_name, U& (*)()){ 
		return *xtal::arg_cast<U*>(a, param_num, param_name); }


	// 変換後の型が参照でもポインタでもない場合、ポインタ型としてキャストしたあと実体にする
	static T as_inner(const AnyPtr& a, ...){ 
		if(const T* ret = xtal::as<const T*>(a)){ return *ret; }else{ return *(const T*)&null;} }
		
	static T cast_inner(const AnyPtr& a, ...){ 
		return *xtal::cast<const T*>(a); }
		
	static T arg_cast_inner(const AnyPtr& a, int_t param_num, const AnyPtr& param_name, ...){ 
		return *xtal::arg_cast<const T*>(a, param_num, param_name); }


	static T as(const AnyPtr& a){ 
		return as_inner(a, (T (*)())0); }
		
	static T cast(const AnyPtr& a){ 
		return cast_inner(a, (T (*)())0); }
		
	static T arg_cast(const AnyPtr& a, int_t param_num, const AnyPtr& param_name){ 
		return arg_cast_inner(a, param_num, param_name, (T (*)())0); }

};

#else

template<class T>
struct CastHelper{

	// 変換後の型が参照でもポインタでもない場合、ポインタ型としてキャストしたあと実体にする
	static T as(const AnyPtr& a){ 
		if(const T* ret = xtal::as<const T*>(a)){ return *ret; }else{ return *(const T*)&null;} }
		
	static T cast(const AnyPtr& a){ 
		return *xtal::cast<const T*>(a); }
		
	static T arg_cast(const AnyPtr& a, int_t param_num, const AnyPtr& param_name){ 
		return *xtal::arg_cast<const T*>(a, param_num, param_name); }
};

template<class T>
struct CastHelper<T*>{

	// 変換後の型がポインタの場合
	static T* as(const AnyPtr& a){ 
		return (T*)as_helper_helper(a, (T*)0, (T*)0); }
		
	static T* cast(const AnyPtr& a){ 
		return (T*)cast_helper_helper(a, (T*)0, (T*)0); }	
		
	static T* arg_cast(const AnyPtr& a, int_t param_num, const AnyPtr& param_name){ 
		return (T*)arg_cast_helper_helper(a, param_num, param_name, (T*)0, (T*)0); }
};

template<class T>
struct CastHelper<T&>{
	
	// 変換後の型が参照の場合、ポインタ型としてキャストしたあと参照にする
	static T& as(const AnyPtr& a){ 
		if(T* ret = xtal::as<T*>(a)){ return *ret; }else{ return *(T*)&null;} }
		
	static T& cast(const AnyPtr& a){ 
		return *xtal::cast<T*>(a); }
		
	static T& arg_cast(const AnyPtr& a, int_t param_num, const AnyPtr& param_name){ 
		return *xtal::arg_cast<T*>(a, param_num, param_name); }
};


#endif

//////////////////////////////////////////////////////////////////////////////

/**
* @brief T型に変換する。
*
* Tに変換できない場合、
* Tがポインタ型ならNULLを返す。
* Tが値か参照で、AnyPtrを継承した型ならxtal::nullを返す。
* それ以外の型の場合は未定義。
*/
template<class T> 
inline typename CastResult<T>::type 
as(const AnyPtr& a){
	return CastHelper<T>::as(a);
}

/**
* @brief T型に変換する。
*
* T型に変換できない場合、builtin()->member("CastError")が投げられる
*/
template<class T>
inline typename CastResult<T>::type 
cast(const AnyPtr& a){
	return CastHelper<T>::cast(a);
}

/**
* @brief T型に変換する。
*
* T型に変換できない場合、builtin()->member("ArgumentError")が投げられる
*/
template<class T>
inline typename CastResult<T>::type 
arg_cast(const AnyPtr& a, int_t param_num, const AnyPtr& param_name){
	return CastHelper<T>::arg_cast(a, param_num, param_name);
}

/**
* @brief SmartPtr<T>型に変換する。
*
* Tに変換できない場合xtal::nullを返す。
*/
template<class T> 
inline const SmartPtr<T>&
ptr_as(const AnyPtr& a){
	return CastHelper<const SmartPtr<T>&>::as(a);
}

/**
* @brief SmartPtr<T>型に変換する。
*
* T型に変換できない場合、builtin()->member("CastError")が投げられる
*/
template<class T>
inline const SmartPtr<T>&
ptr_cast(const AnyPtr& a){
	return CastHelper<const SmartPtr<T>&>::cast(a);
}

/**
* @brief SmartPtr<T>型に、実際の型がどうであるかを無視して強制変換する。
*/
template<class T>
inline const SmartPtr<T>&
static_ptr_cast(const AnyPtr& a){
	return *(const SmartPtr<T>*)&a;
}


/////////////////////////////////////////////////////////////////////////////

template<>
struct CastHelper<AnyPtr*>{
	static AnyPtr* as(const AnyPtr& a){ return (AnyPtr*)&a; }
	static AnyPtr* cast(const AnyPtr& a){ return (AnyPtr*)&a; }
	static AnyPtr* arg_cast(const AnyPtr& a, int, const AnyPtr&){ return (AnyPtr*)&a; }
};

template<>
struct CastHelper<const AnyPtr*>{
	static const AnyPtr* as(const AnyPtr& a){ return (AnyPtr*)&a; }
	static const AnyPtr* cast(const AnyPtr& a){ return (AnyPtr*)&a; }
	static const AnyPtr* arg_cast(const AnyPtr& a, int, const AnyPtr&){ return (AnyPtr*)&a; }
};

template<>
struct CastHelper<const AnyPtr&>{
	static const AnyPtr& as(const AnyPtr& a){ return (AnyPtr&)a; }
	static const AnyPtr& cast(const AnyPtr& a){ return (AnyPtr&)a; }
	static const AnyPtr& arg_cast(const AnyPtr& a, int, const AnyPtr&){ return (AnyPtr&)a; }
};

template<>
struct CastHelper<Any*>{
	static Any* as(const AnyPtr& a){ return (Any*)&a; }
	static Any* cast(const AnyPtr& a){ return (Any*)&a; }
	static Any* arg_cast(const AnyPtr& a, int, const AnyPtr&){ return (Any*)&a; }
};

template<>
struct CastHelper<const Any*>{
	static const Any* as(const AnyPtr& a){ return (Any*)&a; }
	static const Any* cast(const AnyPtr& a){ return (Any*)&a; }
	static const Any* arg_cast(const AnyPtr& a, int, const AnyPtr&){ return (Any*)&a; }
};


template<>
struct CastHelper<const Any&>{
	static Any& as(const AnyPtr& a){ return *(Any*)&a; }
	static Any& cast(const AnyPtr& a){ return *(Any*)&a; }
	static Any& arg_cast(const AnyPtr& a, int, const AnyPtr&){ return *(Any*)&a; }
};


template<>
struct CastHelper<const IDPtr*>{
	static const IDPtr* as(const AnyPtr& a);
	static const IDPtr* cast(const AnyPtr& a);
	static const IDPtr* arg_cast(const AnyPtr& a, int_t param_num, const AnyPtr& param_name);
};

template<>
struct CastHelper<const char*>{
	static const char* as(const AnyPtr& a);
	static const char* cast(const AnyPtr& a);
	static const char* arg_cast(const AnyPtr& a, int_t param_num, const AnyPtr& param_name);
};


template<>
struct CastHelper<int_t>{
	static int_t as(const AnyPtr& a);
	static int_t cast(const AnyPtr& a);
	static int_t arg_cast(const AnyPtr& a, int_t param_num, const AnyPtr& param_name);
};

template<>
struct CastHelper<float_t>{
	static float_t as(const AnyPtr& a);
	static float_t cast(const AnyPtr& a);
	static float_t arg_cast(const AnyPtr& a, int_t param_num, const AnyPtr& param_name);
};

template<>
struct CastHelper<bool>{
	static bool as(const AnyPtr& a){ return a; }
	static bool cast(const AnyPtr& a){ return a; }
	static bool arg_cast(const AnyPtr& a, int, const AnyPtr&){ return a; }
};

#define XTAL_CAST_HELPER(Type) \
template<> struct CastResult<const Type&>{ typedef Type type; };\
template<>\
struct CastHelper<const Type&>{\
	static Type as(const AnyPtr& a){ return xtal::as<Type>(a); }\
	static Type cast(const AnyPtr& a){ return xtal::cast<Type>(a); }\
	static Type arg_cast(const AnyPtr& a, int_t param_num, const AnyPtr& param_name){ return xtal::arg_cast<Type>(a, param_num, param_name); }\
}

XTAL_CAST_HELPER(int_t);
XTAL_CAST_HELPER(float_t);
XTAL_CAST_HELPER(bool);

#undef XTAL_CAST_HELPER

template<>
struct CastHelper<check_xtype<int>::type>{
	static int as(const AnyPtr& a){ return CastHelper<int_t>::as(a); }
	static int cast(const AnyPtr& a){ return CastHelper<int_t>::cast(a); }
	static int arg_cast(const AnyPtr& a, int_t param_num, const AnyPtr& param_name){ return CastHelper<int_t>::arg_cast(a, param_num, param_name); }
};

template<>
struct CastHelper<check_xtype<long>::type>{
	static long as(const AnyPtr& a){ return CastHelper<int_t>::as(a); }
	static long cast(const AnyPtr& a){ return CastHelper<int_t>::cast(a); }
	static long arg_cast(const AnyPtr& a, int_t param_num, const AnyPtr& param_name){ return CastHelper<int_t>::arg_cast(a, param_num, param_name); }
};

template<>
struct CastHelper<check_xtype<short>::type>{
	static short as(const AnyPtr& a){ return (short)CastHelper<int_t>::as(a); }
	static short cast(const AnyPtr& a){ return (short)CastHelper<int_t>::cast(a); }
	static short arg_cast(const AnyPtr& a, int_t param_num, const AnyPtr& param_name){ return (short)CastHelper<int_t>::arg_cast(a, param_num, param_name); }
};

template<>
struct CastHelper<check_xtype<char>::type>{
	static char as(const AnyPtr& a){ return (char)CastHelper<int_t>::as(a); }
	static char cast(const AnyPtr& a){ return (char)CastHelper<int_t>::cast(a); }
	static char arg_cast(const AnyPtr& a, int_t param_num, const AnyPtr& param_name){ return (char)CastHelper<int_t>::arg_cast(a, param_num, param_name); }
};

template<>
struct CastHelper<check_xtype<unsigned int>::type>{
	static unsigned int as(const AnyPtr& a){ return CastHelper<int_t>::as(a); }
	static unsigned int cast(const AnyPtr& a){ return CastHelper<int_t>::cast(a); }
	static unsigned int arg_cast(const AnyPtr& a, int_t param_num, const AnyPtr& param_name){ return CastHelper<int_t>::arg_cast(a, param_num, param_name); }
};

template<>
struct CastHelper<check_xtype<unsigned long>::type>{
	static unsigned long as(const AnyPtr& a){ return CastHelper<int_t>::as(a); }
	static unsigned long cast(const AnyPtr& a){ return CastHelper<int_t>::cast(a); }
	static unsigned long arg_cast(const AnyPtr& a, int_t param_num, const AnyPtr& param_name){ return CastHelper<int_t>::arg_cast(a, param_num, param_name); }
};

template<>
struct CastHelper<check_xtype<unsigned short>::type>{
	static unsigned short as(const AnyPtr& a){ return (unsigned short)CastHelper<int_t>::as(a); }
	static unsigned short cast(const AnyPtr& a){ return (unsigned short)CastHelper<int_t>::cast(a); }
	static unsigned short arg_cast(const AnyPtr& a, int_t param_num, const AnyPtr& param_name){ return (unsigned short)CastHelper<int_t>::arg_cast(a, param_num, param_name); }
};

template<>
struct CastHelper<check_xtype<unsigned char>::type>{
	static unsigned char as(const AnyPtr& a){ return (unsigned char)CastHelper<int_t>::as(a); }
	static unsigned char cast(const AnyPtr& a){ return (unsigned char)CastHelper<int_t>::cast(a); }
	static unsigned char arg_cast(const AnyPtr& a, int_t param_num, const AnyPtr& param_name){ return (unsigned char)CastHelper<int_t>::arg_cast(a, param_num, param_name); }
};

template<>
struct CastHelper<check_xtype<signed char>::type>{
	static signed char as(const AnyPtr& a){ return (signed char)CastHelper<int_t>::as(a); }
	static signed char cast(const AnyPtr& a){ return (signed char)CastHelper<int_t>::cast(a); }
	static signed char arg_cast(const AnyPtr& a, int_t param_num, const AnyPtr& param_name){ return (signed char)CastHelper<int_t>::arg_cast(a, param_num, param_name); }
};

template<>
struct CastHelper<check_xtype<float>::type>{
	static float as(const AnyPtr& a){ return (float)CastHelper<float_t>::as(a); }
	static float cast(const AnyPtr& a){ return (float)CastHelper<float_t>::cast(a); }
	static float arg_cast(const AnyPtr& a, int_t param_num, const AnyPtr& param_name){ return (float)CastHelper<float_t>::arg_cast(a, param_num, param_name); }
};

template<>
struct CastHelper<check_xtype<double>::type>{
	static double as(const AnyPtr& a){ return CastHelper<float_t>::as(a); }
	static double cast(const AnyPtr& a){ return CastHelper<float_t>::cast(a); }
	static double arg_cast(const AnyPtr& a, int_t param_num, const AnyPtr& param_name){ return CastHelper<float_t>::arg_cast(a, param_num, param_name); }
};

template<>
struct CastHelper<check_xtype<long double>::type>{
	static long double as(const AnyPtr& a){ return CastHelper<float_t>::as(a); }
	static long double cast(const AnyPtr& a){ return CastHelper<float_t>::cast(a); }
	static long double arg_cast(const AnyPtr& a, int_t param_num, const AnyPtr& param_name){ return CastHelper<float_t>::arg_cast(a, param_num, param_name); }
};

#define XTAL_CAST_HELPER(Type) \
template<> struct CastResult<check_xtype<const Type&> >{ typedef Type type; };\
template<>\
struct CastHelper<check_xtype<const Type&> >{\
	static Type as(const AnyPtr& a){ return xtal::as<Type>(a); }\
	static Type cast(const AnyPtr& a){ return xtal::cast<Type>(a); }\
	static Type arg_cast(const AnyPtr& a, int_t param_num, const AnyPtr& param_name){ return xtal::arg_cast<Type>(a, param_num, param_name); }\
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
