
#pragma once

namespace xtal{
	
/**
* @brief castŠÖ”AasŠÖ”‚Ì–ß‚è’l‚ÌŒ^‚ğŒˆ’è‚·‚é‚½‚ß‚Ìƒwƒ‹ƒp[ƒeƒ“ƒvƒŒ[ƒgƒNƒ‰ƒX
*
* ‚Ù‚Æ‚ñ‚Ç‚Ìê‡ACastResult<T>::type‚ÍT©g‚ğ•Ô‚·B
* ˆÙ‚È‚éê‡‚Æ‚µ‚Ä‚ÍA—á‚¦‚ÎCastResult<const int&>::type ‚Íint‚ğ•Ô‚·B
*/
template<class T>
struct CastResult{ typedef T type; };

template<class T> 
inline typename CastResult<T>::type as(const AnyPtr& a);
	
template<class T>
inline typename CastResult<T>::type cast(const AnyPtr& a);
	
template<class T>
inline bool can_cast(const AnyPtr& a);

template<class T>
inline typename CastResult<T>::type nocheck_cast(const AnyPtr& a);

/////////////////////////////////////////////////////////////////////////////

// •ÏŠ·Œã‚ÌŒ^‚ªSmartPtr‚Ìê‡
template<class U, class V>
inline const void* as_helper_helper(const AnyPtr& a, const SmartPtr<U>*, const V&){
	if(a->is(get_cpp_class<U>())){ return &a; }
	return 0;
}

// •ÏŠ·Œã‚ÌŒ^‚ªBase‚ğŒp³‚µ‚½Œ^‚Ìê‡
template<class U>
inline const void* as_helper_helper(const AnyPtr& a, const Base*, const U*){
	if(a->is(get_cpp_class<U>())){ return pvalue(a); }
	return 0;
}

// •ÏŠ·Œã‚ÌŒ^‚ªInnocence‚ğŒp³‚µ‚½Œ^‚Ìê‡
template<class U>
inline const void* as_helper_helper(const AnyPtr& a, const Innocence*, const U*){
	if(a->is(get_cpp_class<U>())){ return &a; }
	return 0;
}

// •ÏŠ·Œã‚ÌŒ^‚ªInnocence‚âBase‚ğŒp³‚µ‚Ä‚¢‚È‚¢Œ^‚Ìê‡
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

// •ÏŠ·Œã‚ÌŒ^‚ªSmartPtr‚Ìê‡
template<class U, class V>
inline const void* cast_helper_helper(const AnyPtr& a, const SmartPtr<U>*, const V&){
	return cast_helper_helper_smartptr(a, get_cpp_class<U>()); 
}

// •ÏŠ·Œã‚ÌŒ^‚ªBase‚ğŒp³‚µ‚½Œ^‚Ìê‡
template<class U>
inline const void* cast_helper_helper(const AnyPtr& a, const Base*, const U*){
	return cast_helper_helper_base(a, get_cpp_class<U>()); 
}
	
// •ÏŠ·Œã‚ÌŒ^‚ªInnocence‚ğŒp³‚µ‚½Œ^‚Ìê‡
template<class U>
inline const void* cast_helper_helper(const AnyPtr& a, const Innocence*, const U*){
	return cast_helper_helper_innocence(a, get_cpp_class<U>()); 
}
	
// •ÏŠ·Œã‚ÌŒ^‚ªAnyPtr‚âBase‚ğŒp³‚µ‚Ä‚¢‚È‚¢Œ^‚Ìê‡
template<class U>
inline const void* cast_helper_helper(const AnyPtr& a, const void*, const U*){
	return cast_helper_helper_other(a, get_cpp_class<U>()); 
}

/////////////////////////////////////////////////////////////////////////////

// •ÏŠ·Œã‚ÌŒ^‚ªSmartPtr‚Ìê‡
template<class U, class V>
inline bool can_cast_helper_helper(const AnyPtr& a, const SmartPtr<U>*, const V&){
	return a->is(get_cpp_class<U>());
}

// •ÏŠ·Œã‚ÌŒ^‚ªBase‚ğŒp³‚µ‚½Œ^‚Ìê‡
template<class U>
inline bool can_cast_helper_helper(const AnyPtr& a, const Base*, const U*){
	return a->is(get_cpp_class<U>());
}

// •ÏŠ·Œã‚ÌŒ^‚ªInnocence‚ğŒp³‚µ‚½Œ^‚Ìê‡
template<class U>
inline bool can_cast_helper_helper(const AnyPtr& a, const Innocence*, const U*){
	return a->is(get_cpp_class<U>());
}

// •ÏŠ·Œã‚ÌŒ^‚ªInnocence‚âBase‚ğŒp³‚µ‚Ä‚¢‚È‚¢Œ^‚Ìê‡
template<class U>
inline bool can_cast_helper_helper(const AnyPtr& a, const void*, const U*){
	return a->is(get_cpp_class<U>());
}

/////////////////////////////////////////////////////////////////////////////

// •ÏŠ·Œã‚ÌŒ^‚ªSmartPtr‚Ìê‡
template<class U, class V>
inline const void* nocheck_cast_helper_helper(const AnyPtr& a, const SmartPtr<U>*, const V&){
	return &a;
}

// •ÏŠ·Œã‚ÌŒ^‚ªBase‚ğŒp³‚µ‚½Œ^‚Ìê‡
template<class U>
inline const void* nocheck_cast_helper_helper(const AnyPtr& a, const Base*, const U*){
	return pvalue(a);
}

// •ÏŠ·Œã‚ÌŒ^‚ªInnocence‚ğŒp³‚µ‚½Œ^‚Ìê‡
template<class U>
inline const void* nocheck_cast_helper_helper(const AnyPtr& a, const Innocence*, const U*){
	return &a;
}

// •ÏŠ·Œã‚ÌŒ^‚ªInnocence‚âBase‚ğŒp³‚µ‚Ä‚¢‚È‚¢Œ^‚Ìê‡
template<class U>
inline const void* nocheck_cast_helper_helper(const AnyPtr& a, const void*, const U*){
	return ((SmartPtr<U>&)a).get();
}

/////////////////////////////////////////////////////////////////////////////

#ifdef XTAL_NO_TEMPLATE_PARTIAL_SPECIALIZATION

template<class T>
struct CastHelper{

	// •ÏŠ·Œã‚ÌŒ^‚ªƒ|ƒCƒ“ƒ^‚Ìê‡
	template<class U> static T as_inner(const AnyPtr& a, U* (*)()){ 
		return (T)as_helper_helper(a, (U*)0, (U*)0); }
		
	template<class U> static T cast_inner(const AnyPtr& a, U* (*)()){ 
		return (T)cast_helper_helper(a, (U*)0, (U*)0); }	
		
	
	// •ÏŠ·Œã‚ÌŒ^‚ªQÆ‚Ìê‡Aƒ|ƒCƒ“ƒ^Œ^‚Æ‚µ‚ÄƒLƒƒƒXƒg‚µ‚½‚ ‚ÆQÆ‚É‚·‚é
	template<class U> static T as_inner(const AnyPtr& a, U& (*)()){ 
		if(U* ret = xtal::as<U*>(a)){ return *ret; }else{ return *(U*)&null;} }
		
	template<class U> static T cast_inner(const AnyPtr& a, U& (*)()){ 
		return *xtal::cast<U*>(a); }
		

	// •ÏŠ·Œã‚ÌŒ^‚ªQÆ‚Å‚àƒ|ƒCƒ“ƒ^‚Å‚à‚È‚¢ê‡Aƒ|ƒCƒ“ƒ^Œ^‚Æ‚µ‚ÄƒLƒƒƒXƒg‚µ‚½‚ ‚ÆÀ‘Ì‚É‚·‚é
	static T as_inner(const AnyPtr& a, ...){ 
		if(const T* ret = xtal::as<const T*>(a)){ return *ret; }else{ return *(const T*)&null;} }
		
	static T cast_inner(const AnyPtr& a, ...){ 
		return *xtal::cast<const T*>(a); }
		


	static T as(const AnyPtr& a){ 
		return as_inner(a, (T (*)())0); }
		
	static T cast(const AnyPtr& a){ 
		return cast_inner(a, (T (*)())0); }
		
};

#else

template<class T>
struct CastHelper{

	// •ÏŠ·Œã‚ÌŒ^‚ªQÆ‚Å‚àƒ|ƒCƒ“ƒ^‚Å‚à‚È‚¢ê‡Aƒ|ƒCƒ“ƒ^Œ^‚Æ‚µ‚ÄƒLƒƒƒXƒg‚µ‚½‚ ‚ÆÀ‘Ì‚É‚·‚é
	static T as(const AnyPtr& a){ 
		if(const T* ret = xtal::as<const T*>(a)){ return *ret; }else{ return *(const T*)&null;} }
		
	static T cast(const AnyPtr& a){ 
		return *xtal::cast<const T*>(a); }
		
	static bool can_cast(const AnyPtr& a){ 
		return xtal::can_cast<const T*>(a); }

	static T nocheck_cast(const AnyPtr& a){ 
		return *xtal::nocheck_cast<const T*>(a); }
};

template<class T>
struct CastHelper<T*>{

	// •ÏŠ·Œã‚ÌŒ^‚ªƒ|ƒCƒ“ƒ^‚Ìê‡
	static T* as(const AnyPtr& a){ 
		return (T*)as_helper_helper(a, (T*)0, (T*)0); }
		
	static T* cast(const AnyPtr& a){ 
		return (T*)cast_helper_helper(a, (T*)0, (T*)0); }	
		
	static bool can_cast(const AnyPtr& a){ 
		return can_cast_helper_helper(a, (T*)0, (T*)0); }	

	static T* nocheck_cast(const AnyPtr& a){ 
		return (T*)nocheck_cast_helper_helper(a, (T*)0, (T*)0); }	
};

template<class T>
struct CastHelper<T&>{
	
	// •ÏŠ·Œã‚ÌŒ^‚ªQÆ‚Ìê‡Aƒ|ƒCƒ“ƒ^Œ^‚Æ‚µ‚ÄƒLƒƒƒXƒg‚µ‚½‚ ‚ÆQÆ‚É‚·‚é
	static T& as(const AnyPtr& a){ 
		if(T* ret = xtal::as<T*>(a)){ return *ret; }else{ return *(T*)&null;} }
		
	static T& cast(const AnyPtr& a){ 
		return *xtal::cast<T*>(a); }
		
	static bool can_cast(const AnyPtr& a){ 
		return xtal::can_cast<T*>(a); }

	static T& nocheck_cast(const AnyPtr& a){ 
		return *xtal::nocheck_cast<T*>(a); }
};


#endif

//////////////////////////////////////////////////////////////////////////////

/**
* @brief TŒ^‚É•ÏŠ·‚·‚éB
*
* T‚É•ÏŠ·‚Å‚«‚È‚¢ê‡A
* T‚ªƒ|ƒCƒ“ƒ^Œ^‚È‚çNULL‚ğ•Ô‚·B
* T‚ª’l‚©QÆ‚ÅAAnyPtr‚ğŒp³‚µ‚½Œ^‚È‚çxtal::null‚ğ•Ô‚·B
* ‚»‚êˆÈŠO‚ÌŒ^‚Ìê‡‚Í–¢’è‹`B
*/
template<class T> 
inline typename CastResult<T>::type 
as(const AnyPtr& a){
	return CastHelper<T>::as(a);
}

/**
* @brief TŒ^‚É•ÏŠ·‚·‚éB
*
* TŒ^‚É•ÏŠ·‚Å‚«‚È‚¢ê‡Abuiltin()->member("CastError")‚ª“Š‚°‚ç‚ê‚é
*/
template<class T>
inline typename CastResult<T>::type 
cast(const AnyPtr& a){
	return CastHelper<T>::cast(a);
}

/**
* @brief TŒ^‚É•ÏŠ·‚Å‚«‚é‚©’²‚×‚éB
*
*/
template<class T>
inline bool 
can_cast(const AnyPtr& a){
	return CastHelper<T>::can_cast(a);
}

/**
* @brief TŒ^‚Éƒ`ƒFƒbƒN–³‚µ‚Å•ÏŠ·‚·‚éB
*
*/
template<class T>
inline typename CastResult<T>::type 
nocheck_cast(const AnyPtr& a){
	return CastHelper<T>::nocheck_cast(a);
}

/**
* @brief SmartPtr<T>Œ^‚É•ÏŠ·‚·‚éB
*
* T‚É•ÏŠ·‚Å‚«‚È‚¢ê‡xtal::null‚ğ•Ô‚·B
*/
template<class T> 
inline const SmartPtr<T>&
ptr_as(const AnyPtr& a){
	return CastHelper<const SmartPtr<T>&>::as(a);
}

/**
* @brief SmartPtr<T>Œ^‚É•ÏŠ·‚·‚éB
*
* TŒ^‚É•ÏŠ·‚Å‚«‚È‚¢ê‡Abuiltin()->member("CastError")‚ª“Š‚°‚ç‚ê‚é
*/
template<class T>
inline const SmartPtr<T>&
ptr_cast(const AnyPtr& a){
	return CastHelper<const SmartPtr<T>&>::cast(a);
}

/**
* @brief SmartPtr<T>Œ^‚ÉAÀÛ‚ÌŒ^‚ª‚Ç‚¤‚Å‚ ‚é‚©‚ğ–³‹‚µ‚Ä‹­§•ÏŠ·‚·‚éB
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
	static bool can_cast(const AnyPtr& a){ return true; }
	static AnyPtr* nocheck_cast(const AnyPtr& a){ return (AnyPtr*)&a; }
};

template<>
struct CastHelper<const AnyPtr*>{
	static const AnyPtr* as(const AnyPtr& a){ return (AnyPtr*)&a; }
	static const AnyPtr* cast(const AnyPtr& a){ return (AnyPtr*)&a; }
	static bool can_cast(const AnyPtr& a){ return true; }
	static AnyPtr* nocheck_cast(const AnyPtr& a){ return (AnyPtr*)&a; }
};

template<>
struct CastHelper<const AnyPtr&>{
	static const AnyPtr& as(const AnyPtr& a){ return (AnyPtr&)a; }
	static const AnyPtr& cast(const AnyPtr& a){ return (AnyPtr&)a; }
	static bool can_cast(const AnyPtr& a){ return true; }
	static const AnyPtr& nocheck_cast(const AnyPtr& a){ return (AnyPtr&)a; }
};

template<>
struct CastHelper<Any*>{
	static Any* as(const AnyPtr& a){ return (Any*)&a; }
	static Any* cast(const AnyPtr& a){ return (Any*)&a; }
	static bool can_cast(const AnyPtr& a){ return true; }
	static Any* nocheck_cast(const AnyPtr& a){ return (Any*)&a; }
};

template<>
struct CastHelper<const Any*>{
	static const Any* as(const AnyPtr& a){ return (Any*)&a; }
	static const Any* cast(const AnyPtr& a){ return (Any*)&a; }
	static bool can_cast(const AnyPtr& a){ return true; }
	static const Any* nocheck_cast(const AnyPtr& a){ return (Any*)&a; }
};


template<>
struct CastHelper<const Any&>{
	static Any& as(const AnyPtr& a){ return *(Any*)&a; }
	static Any& cast(const AnyPtr& a){ return *(Any*)&a; }
	static bool can_cast(const AnyPtr& a){ return true; }
	static Any& nocheck_cast(const AnyPtr& a){ return *(Any*)&a; }
};


template<>
struct CastHelper<const IDPtr*>{
	static const IDPtr* as(const AnyPtr& a);
	static const IDPtr* cast(const AnyPtr& a);
	static bool can_cast(const AnyPtr& a);
	static const IDPtr* nocheck_cast(const AnyPtr& a);
};

template<>
struct CastHelper<const char*>{
	static const char* as(const AnyPtr& a);
	static const char* cast(const AnyPtr& a);
	static bool can_cast(const AnyPtr& a);
	static const char* nocheck_cast(const AnyPtr& a);
};


template<>
struct CastHelper<int_t>{
	static int_t as(const AnyPtr& a);
	static int_t cast(const AnyPtr& a);
	static bool can_cast(const AnyPtr& a);
	static int_t nocheck_cast(const AnyPtr& a);
};

template<>
struct CastHelper<float_t>{
	static float_t as(const AnyPtr& a);
	static float_t cast(const AnyPtr& a);
	static bool can_cast(const AnyPtr& a);
	static float_t nocheck_cast(const AnyPtr& a);
};

template<>
struct CastHelper<bool>{
	static bool as(const AnyPtr& a){ return a; }
	static bool cast(const AnyPtr& a){ return a; }
	static bool can_cast(const AnyPtr& a){ return true; }
	static bool nocheck_cast(const AnyPtr& a){ return a; }
};

#define XTAL_CAST_HELPER(Type) \
template<> struct CastResult<const Type&>{ typedef Type type; };\
template<>\
struct CastHelper<const Type&>{\
	static Type as(const AnyPtr& a){ return xtal::as<Type>(a); }\
	static Type cast(const AnyPtr& a){ return xtal::cast<Type>(a); }\
	static bool can_cast(const AnyPtr& a){ return xtal::can_cast<Type>(a); }\
	static Type nocheck_cast(const AnyPtr& a){ return xtal::nocheck_cast<Type>(a); }\
}

XTAL_CAST_HELPER(int_t);
XTAL_CAST_HELPER(float_t);
XTAL_CAST_HELPER(bool);

#undef XTAL_CAST_HELPER

#define XTAL_CAST_HELPER(Type, Type2) \
template<>\
struct CastHelper<check_xtype<Type>::type>{\
	static Type as(const AnyPtr& a){ return (Type)xtal::CastHelper<Type2>::as(a); }\
	static Type cast(const AnyPtr& a){ return (Type)xtal::CastHelper<Type2>::cast(a); }\
	static bool can_cast(const AnyPtr& a){ return xtal::CastHelper<Type2>::can_cast(a); }\
	static Type nocheck_cast(const AnyPtr& a){ return (Type)xtal::CastHelper<Type2>::nocheck_cast(a); }\
};\
template<> struct CastResult<check_xtype<const Type&> >{ typedef Type type; };\
template<>\
struct CastHelper<check_xtype<const Type&> >{\
	static Type as(const AnyPtr& a){ return xtal::as<Type>(a); }\
	static Type cast(const AnyPtr& a){ return xtal::cast<Type>(a); }\
	static bool can_cast(const AnyPtr& a){ return xtal::can_cast<Type>(a); }\
	static Type nocheck_cast(const AnyPtr& a){ return (Type)xtal::nocheck_cast<Type2>(a); }\
}

XTAL_CAST_HELPER(int, int_t);
XTAL_CAST_HELPER(unsigned int, int_t);
XTAL_CAST_HELPER(long, int_t);
XTAL_CAST_HELPER(unsigned long, int_t);
XTAL_CAST_HELPER(short, int_t);
XTAL_CAST_HELPER(unsigned short, int_t);
XTAL_CAST_HELPER(char, int_t);
XTAL_CAST_HELPER(signed char, int_t);
XTAL_CAST_HELPER(unsigned char, int_t);
XTAL_CAST_HELPER(float, float_t);
XTAL_CAST_HELPER(double, float_t);
XTAL_CAST_HELPER(long double, float_t);

#undef XTAL_CAST_HELPER

}
