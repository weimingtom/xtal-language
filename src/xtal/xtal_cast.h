
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
inline typename CastResult<T>::type unchecked_cast(const AnyPtr& a);

template<class T>
const ClassPtr& get_cpp_class();

/////////////////////////////////////////////////////////////////////////////

void cast_failed(const AnyPtr& a, const ClassPtr& cls);

/////////////////////////////////////////////////////////////////////////////

// •ÏŠ·Œã‚ÌŒ^‚ªSmartPtr‚Ìê‡
template<class U, class V>
inline const void* unchecked_cast_helper(const AnyPtr& a, const SmartPtr<U>*, const V&){
	return &a;
}

// •ÏŠ·Œã‚ÌŒ^‚ªBase‚ğŒp³‚µ‚½Œ^‚Ìê‡
template<class U>
inline const void* unchecked_cast_helper(const AnyPtr& a, const Base*, const U*){
	return pvalue(a);
}

// •ÏŠ·Œã‚ÌŒ^‚ªRefCountingBaseBase‚ğŒp³‚µ‚½Œ^‚Ìê‡
template<class U>
inline const void* unchecked_cast_helper(const AnyPtr& a, const RefCountingBase*, const U*){
	return rcpvalue(a);
}

// •ÏŠ·Œã‚ÌŒ^‚ªAny‚ğŒp³‚µ‚½Œ^‚Ìê‡
template<class U>
inline const void* unchecked_cast_helper(const AnyPtr& a, const Any*, const U*){
	return &a;
}

// •ÏŠ·Œã‚ÌŒ^‚ªAny‚âBase‚ğŒp³‚µ‚Ä‚¢‚È‚¢Œ^‚Ìê‡
template<class U>
inline const void* unchecked_cast_helper(const AnyPtr& a, const void*, const U*){
	return ((SmartPtr<U>&)a).get();
}

/////////////////////////////////////////////////////////////////////////////

// •ÏŠ·Œã‚ÌŒ^‚ªSmartPtr‚Ìê‡
template<class U, class V>
inline const void* get_null_helper(const SmartPtr<U>*, const V&){
	return &null;
}

// •ÏŠ·Œã‚ÌŒ^‚ªBase‚ğŒp³‚µ‚½Œ^‚Ìê‡
template<class U>
inline const void* get_null_helper(const Base*, const U*){
	return 0;
}

// •ÏŠ·Œã‚ÌŒ^‚ªRefCountingBase‚ğŒp³‚µ‚½Œ^‚Ìê‡
template<class U>
inline const void* get_null_helper(const RefCountingBase*, const U*){
	return 0;
}

// •ÏŠ·Œã‚ÌŒ^‚ªAny‚ğŒp³‚µ‚½Œ^‚Ìê‡
template<class U>
inline const void* get_null_helper(const Any*, const U*){
	return &null;
}

// •ÏŠ·Œã‚ÌŒ^‚ªAny‚âBase‚ğŒp³‚µ‚Ä‚¢‚È‚¢Œ^‚Ìê‡
template<class U>
inline const void* get_null_helper(const void*, const U*){
	return U();
}

/////////////////////////////////////////////////////////////////////////////

template<class T>
struct CastHelper{
	// •ÏŠ·Œã‚ÌŒ^‚ªQÆ‚Å‚àƒ|ƒCƒ“ƒ^‚Å‚à‚È‚¢ê‡Aƒ|ƒCƒ“ƒ^Œ^‚Æ‚µ‚ÄƒLƒƒƒXƒg‚µ‚½‚ ‚ÆÀ‘Ì‚É‚·‚é
	
	static bool can_cast(const AnyPtr& a){ 
		return a->is(get_cpp_class<T>());
	}

	static T unchecked_cast(const AnyPtr& a){ 
		return *CastHelper<const T*>::unchecked_cast(a); 
	}
	
	static T get_null(){ 
		return *CastHelper<const T*>::get_null();
	}
};

template<class T>
struct CastHelper<T&>{
	// •ÏŠ·Œã‚ÌŒ^‚ªQÆ‚Ìê‡Aƒ|ƒCƒ“ƒ^Œ^‚Æ‚µ‚ÄƒLƒƒƒXƒg‚µ‚½‚ ‚ÆQÆ‚É‚·‚é
	
	static bool can_cast(const AnyPtr& a){ 
		return a->is(get_cpp_class<T>());
	}
	
	static T& unchecked_cast(const AnyPtr& a){ 
		return *CastHelper<const T*>::unchecked_cast(a); 
	}

	static T& get_null(){ 
		return *CastHelper<const T*>::get_null();
	}
};

template<class T>
struct CastHelper<T*>{
	// •ÏŠ·Œã‚ÌŒ^‚ªƒ|ƒCƒ“ƒ^‚Ìê‡

	static bool can_cast(const AnyPtr& a){ 
		return a->is(get_cpp_class<T>());
	}

	static T* unchecked_cast(const AnyPtr& a){ 
		return (T*)unchecked_cast_helper(a, (T*)0, (T*)0); 
	}	

	static T* get_null(){ 
		return (T*)get_null_helper((T*)0, (T*)0);
	}
};

template<class T>
struct CastHelperHelper{
	static T as(const AnyPtr& a){
		if(can_cast(a)){
			return unchecked_cast(a);
		}
		else{
			return get_null();
		}
	}
		
	static T cast(const AnyPtr& a){ 
		if(can_cast(a)){
			return unchecked_cast(a);
		}
		else{
			cast_failed(a, get_cpp_class<T>());
			return get_null();
		}
	}
		
	static bool can_cast(const AnyPtr& a){ 
		return CastHelper<T>::can_cast(a); 
	}

	static T unchecked_cast(const AnyPtr& a){ 
		return CastHelper<T>::unchecked_cast(a); 
	}
	
	static T get_null(){ 
		return CastHelper<T>::get_null();
	}
};

/////////////////////////////////////////////////////////////////////////////

template<>
struct CastHelper<const AnyPtr*>{
	static bool can_cast(const AnyPtr&){ return true; }
	static const AnyPtr* unchecked_cast(const AnyPtr& a){ return (AnyPtr*)&a; }
	static const AnyPtr* get_null(){ return (AnyPtr*)&null; }
};

template<>
struct CastHelper<const Any*>{
	static bool can_cast(const AnyPtr&){ return true; }
	static const Any* unchecked_cast(const AnyPtr& a){ return (Any*)&a; }
	static const Any* get_null(){ return (Any*)&null; }
};

template<>
struct CastHelper<const char_t*>{
	static bool can_cast(const AnyPtr& a){ return a->is(get_cpp_class<String>()); }
	static const char_t* unchecked_cast(const AnyPtr& a);
	static const Any* get_null(){ return 0; }
};

template<>
struct CastHelper<const IDPtr*>{
	static bool can_cast(const AnyPtr& a);
	static const IDPtr* unchecked_cast(const AnyPtr& a);
	static const IDPtr* get_null(){ return (IDPtr*)&null; }
};


template<>
struct CastHelper<const Int*>{
	static bool can_cast(const AnyPtr& a){ return type(a)==TYPE_INT; }
	static const Int* unchecked_cast(const AnyPtr& a){ return (const Int*)&a; }
	static const Int* get_null(){ return (Int*)&null; }
};

template<>
struct CastHelper<const Float*>{
	static bool can_cast(const AnyPtr& a){ return type(a)==TYPE_FLOAT; }
	static const Float* unchecked_cast(const AnyPtr& a){ return (const Float*)&a; }
	static const Float* get_null(){ return (Float*)&null; }
};

template<>
struct CastHelper<const Bool*>{
	static bool can_cast(const AnyPtr& a){ return type(a)==TYPE_TRUE || type(a)==TYPE_FALSE; }
	static const Bool* unchecked_cast(const AnyPtr& a){ return (const Bool*)&a; }
	static const Bool* get_null(){ return (Bool*)&null; }
};

template<>
struct CastHelper<int_t>{
	static bool can_cast(const AnyPtr& a);
	static int_t unchecked_cast(const AnyPtr& a);
	static int_t get_null(){ return 0; }
};

template<>
struct CastHelper<float_t>{
	static bool can_cast(const AnyPtr& a);
	static float_t unchecked_cast(const AnyPtr& a);
	static float_t get_null(){ return 0; }
};

template<>
struct CastHelper<bool>{
	static bool can_cast(const AnyPtr&){ return true; }
	static bool unchecked_cast(const AnyPtr& a){ return a; }
	static bool get_null(){ return false; }
};

#define XTAL_CAST_HELPER(Type, XType) \
template<> struct CastResult<const Type&>{ typedef Type type; };\
template<>\
struct CastHelper<const Type&>{\
	static bool can_cast(const AnyPtr& a){ return CastHelper<Type>::can_cast(a); }\
	static Type unchecked_cast(const AnyPtr& a){ return CastHelper<Type>::unchecked_cast(a); }\
	static Type get_null(){ return 0; }\
}

XTAL_CAST_HELPER(int_t, Int);
XTAL_CAST_HELPER(float_t, Float);
XTAL_CAST_HELPER(bool, Bool);

template<> struct CppClassSymbol<int_t> : public CppClassSymbol<Int>{};
template<> struct CppClassSymbol<float_t> : public CppClassSymbol<Float>{};
template<> struct CppClassSymbol<bool> : public CppClassSymbol<Bool>{};


#undef XTAL_CAST_HELPER

#define XTAL_CAST_HELPER(Type, Type2, XType) \
template<>\
struct CastHelper<avoid<Type>::type>{\
	static bool can_cast(const AnyPtr& a){ return CastHelper<Type2>::can_cast(a); }\
	static Type unchecked_cast(const AnyPtr& a){ return (Type)CastHelper<Type2>::unchecked_cast(a); }\
	static Type get_null(){ return 0; }\
};\
template<> struct CastResult<avoid<const Type&> >{ typedef Type type; };\
template<>\
struct CastHelper<const avoid<Type>::type&>{\
	static bool can_cast(const AnyPtr& a){ return CastHelper<Type>::can_cast(a); }\
	static Type unchecked_cast(const AnyPtr& a){ return CastHelper<Type>::unchecked_cast(a); }\
	static Type get_null(){ return 0; }\
};\
template<> struct CppClassSymbol<avoid<Type>::type> : public CppClassSymbol<XType>{}

XTAL_CAST_HELPER(int, int_t, Int);
XTAL_CAST_HELPER(unsigned int, int_t, Int);
XTAL_CAST_HELPER(long, int_t, Int);
XTAL_CAST_HELPER(unsigned long, int_t, Int);
XTAL_CAST_HELPER(short, int_t, Int);
XTAL_CAST_HELPER(unsigned short, int_t, Int);
XTAL_CAST_HELPER(char, int_t, Int);
XTAL_CAST_HELPER(signed char, int_t, Int);
XTAL_CAST_HELPER(unsigned char, int_t, Int);
XTAL_CAST_HELPER(float, float_t, Float);
XTAL_CAST_HELPER(double, float_t, Float);
XTAL_CAST_HELPER(long double, float_t, Float);

#undef XTAL_CAST_HELPER

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
	return CastHelperHelper<T>::as(a);
}

/**
* @brief TŒ^‚É•ÏŠ·‚·‚éB
*
* TŒ^‚É•ÏŠ·‚Å‚«‚È‚¢ê‡Abuiltin()->member("CastError")‚ª“Š‚°‚ç‚ê‚é
*/
template<class T>
inline typename CastResult<T>::type 
cast(const AnyPtr& a){
	return CastHelperHelper<T>::cast(a);
}

/**
* @brief TŒ^‚É•ÏŠ·‚Å‚«‚é‚©’²‚×‚éB
*
*/
template<class T>
inline bool can_cast(const AnyPtr& a){
	return CastHelperHelper<T>::can_cast(a);
}

/**
* @brief TŒ^‚Éƒ`ƒFƒbƒN–³‚µ‚Å•ÏŠ·‚·‚éB
*
*/
template<class T>
inline typename CastResult<T>::type 
unchecked_cast(const AnyPtr& a){
	return CastHelperHelper<T>::unchecked_cast(a);
}

/**
* @brief SmartPtr<T>Œ^‚É•ÏŠ·‚·‚éB
*
* T‚É•ÏŠ·‚Å‚«‚È‚¢ê‡xtal::null‚ğ•Ô‚·B
*/
template<class T> 
inline const SmartPtr<T>&
ptr_as(const AnyPtr& a){
	return CastHelperHelper<const SmartPtr<T>&>::as(a);
}

/**
* @brief SmartPtr<T>Œ^‚É•ÏŠ·‚·‚éB
*
* TŒ^‚É•ÏŠ·‚Å‚«‚È‚¢ê‡Abuiltin()->member("CastError")‚ª“Š‚°‚ç‚ê‚é
*/
template<class T>
inline const SmartPtr<T>&
ptr_cast(const AnyPtr& a){
	return CastHelperHelper<const SmartPtr<T>&>::cast(a);
}

/**
* @brief SmartPtr<T>Œ^‚ÉAÀÛ‚ÌŒ^‚ª‚Ç‚¤‚Å‚ ‚é‚©‚ğ–³‹‚µ‚Ä‹­§•ÏŠ·‚·‚éB
*/
template<class T>
inline const SmartPtr<T>&
unchecked_ptr_cast(const AnyPtr& a){
	return *(const SmartPtr<T>*)&a;
}
	
//////////////////////////////////////////

template<class T>
inline typename CastResult<T>::type 
tricky_cast(const AnyPtr& a, void (*f)(T)){
	return CastHelperHelper<T>::cast(a);
}

template<class T>
inline typename CastResult<T>::type 
tricky_as(const AnyPtr& a, void (*f)(T)){
	return CastHelperHelper<T>::as(a);
}

}
