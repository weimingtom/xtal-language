/** \file src/xtal/xtal_cast.h
* \brief src/xtal/xtal_cast.h
*/

#ifndef XTAL_CAST_H_INCLUDE_GUARD
#define XTAL_CAST_H_INCLUDE_GUARD

#pragma once

namespace xtal{
	
struct ParamInfo;

typedef AnyPtr (*bind_var_fun_t)();

struct CppVarSymbolData{ 
	CppVarSymbolData(bind_var_fun_t fun);

	CppVarSymbolData* prev;
	bind_var_fun_t maker;
	unsigned int value;
};

template<class T>
struct CppVarSymbol{
	static CppVarSymbolData value;
	static AnyPtr maker(){ return xnew<T>(); }
};

template<class T>
CppVarSymbolData CppVarSymbol<T>::value(&CppVarSymbol<T>::maker);


/**
* \internal
* brief cast関数、as関数の戻り値の型を決定するためのヘルパーテンプレートクラス
*
* ほとんどの場合、CastResult<T>::typeはT自身を返す。
* 異なる場合としては、例えばCastResult<const int&>::type はintを返す。
*/
template<class T>
struct CastResult{ typedef T type; };

/////////////////////////////////////////////////////////////////////////////

template<int N, class T>
struct UncheckedCastHelper{};

template<class T>
struct UncheckedCastHelper<INHERITED_BASE, T>{
	static const void* cast(const AnyPtr& a){
		return pvalue(a);
	}
};

template<class T>
struct UncheckedCastHelper<INHERITED_RCBASE, T>{
	static const void* cast(const AnyPtr& a){
		return rcpvalue(a);
	}
};

template<class T>
struct UncheckedCastHelper<INHERITED_ANY, T>{
	static const void* cast(const AnyPtr& a){
		return &a;
	}
};

template<class T>
struct UncheckedCastHelper<INHERITED_ANYPTR, T>{
	static const void* cast(const AnyPtr& a){
		return &a;
	}
};

template<class T>
struct UncheckedCastHelper<INHERITED_OTHER, T>{
	static const void* cast(const AnyPtr& a){
		return ((SmartPtr<T>&)a).get();
	}
};


/////////////////////////////////////////////////////////////////////////////

template<int N, class T>
struct GetNullHelper{};

template<class T>
struct GetNullHelper<INHERITED_BASE, T>{
	static const void* get(){
		return 0;
	}
};

template<class T>
struct GetNullHelper<INHERITED_RCBASE, T>{
	static const void* get(){
		return 0;
	}
};

template<class T>
struct GetNullHelper<INHERITED_ANY, T>{
	static const void* get(){
		return &null;
	}
};

template<class T>
struct GetNullHelper<INHERITED_ANYPTR, T>{
	static const void* get(){
		return &null;
	}
};

template<class T>
struct GetNullHelper<INHERITED_OTHER, T>{
	static const void* get(){
		return 0;
	}
};

/////////////////////////////////////////////////////////////////////////////

template<class T>
struct CastHelper{
	// 変換後の型が参照でもポインタでもない場合、ポインタ型としてキャストしたあと実体にする
	
	static bool can_cast(const AnyPtr& a){ 
		return a->is(cpp_class<T>());
	}

	static T unchecked_cast(const AnyPtr& a){ 
		return *CastHelper<const T*>::unchecked_cast(a); 
	}
	
	static T get_null(){ 
		return *CastHelper<const T*>::get_null();
	}
};

template<class T>
struct CastHelper<T*>{
	// 変換後の型がポインタの場合

	static bool can_cast(const AnyPtr& a){ 
		return a->is(cpp_class<T>());
	}

	static T* unchecked_cast(const AnyPtr& a){ 
		return (T*)UncheckedCastHelper<InheritedN<T>::value, T>::cast(a); 
	}	

	static T* get_null(){ 
		return (T*)GetNullHelper<InheritedN<T>::value, T>::get(); 
	}
};

template<class T>
struct CastHelper<T&>{
	// 変換後の型が参照の場合、ポインタ型としてキャストしたあと参照にする
	
	static bool can_cast(const AnyPtr& a){ 
		return a->is(cpp_class<T>());
	}
	
	static T& unchecked_cast(const AnyPtr& a){ 
		return *CastHelper<const T*>::unchecked_cast(a); 
	}

	static T& get_null(){ 
		return *CastHelper<const T*>::get_null();
	}
};

template<class T>
struct CastHelperHelper{
	static T cast(const AnyPtr& a){ 
		if(can_cast(a)){
			return unchecked_cast(a);
		}
		else{
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
	static bool can_cast(const AnyPtr& a){ return a->is(cpp_class<String>()); }
	static const char_t* unchecked_cast(const AnyPtr& a);
	static const Any* get_null(){ return 0; }
};
template<> struct CppClassSymbol<const char_t*> : public CppClassSymbol<String>{};

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
struct CastHelper<bool>{
	static bool can_cast(const AnyPtr&){ return true; }
	static bool unchecked_cast(const AnyPtr& a){ return a; }
	static bool get_null(){ return false; }
};
template<> struct CppClassSymbol<bool> : public CppClassSymbol<Bool>{};

bool Int_can_cast(const AnyPtr& a);
int_t Int_unchecked_cast(const AnyPtr& a);

bool Float_can_cast(const AnyPtr& a);
float_t Float_unchecked_cast(const AnyPtr& a);


#undef XTAL_CAST_HELPER

#define XTAL_CAST_HELPER(Type, XType) \
template<>\
struct CastHelper<Type>{\
	static bool can_cast(const AnyPtr& a){ return XType##_can_cast(a); }\
	static Type unchecked_cast(const AnyPtr& a){ return XType##_unchecked_cast(a); }\
	static Type get_null(){ return 0; }\
};\
template<> struct CppClassSymbol<Type> : public CppClassSymbol<XType>{}


XTAL_CAST_HELPER(char, Int);
XTAL_CAST_HELPER(signed char, Int);
XTAL_CAST_HELPER(unsigned char, Int);
XTAL_CAST_HELPER(short, Int);
XTAL_CAST_HELPER(unsigned short, Int);
XTAL_CAST_HELPER(int, Int);
XTAL_CAST_HELPER(unsigned int, Int);
XTAL_CAST_HELPER(long, Int);
XTAL_CAST_HELPER(unsigned long, Int);

XTAL_CAST_HELPER(float, Float);
XTAL_CAST_HELPER(double, Float);
XTAL_CAST_HELPER(long double, Float);

#undef XTAL_CAST_HELPER

//////////////////////////////////////////////////////////////////////////////

/// \name 型変換
//@{

/**
* \brief SmartPtr<T>型に変換する。
*
* Tに変換できない場合xtal::nullを返す。
*/
template<class T>
inline const SmartPtr<T>&
ptr_cast(const AnyPtr& a){
	return CastHelperHelper<const SmartPtr<T>&>::cast(a);
}

/**
* \brief SmartPtr<T>型に、実際の型がどうであるかを無視して強制変換する。
*/
template<class T>
inline const SmartPtr<T>&
unchecked_ptr_cast(const AnyPtr& a){
	return *(const SmartPtr<T>*)&a;
}

/**
* \brief T型に変換する。
*
* Tに変換できない場合、
* Tがポインタ型ならNULLを返す。
* Tが値か参照で、AnyPtrを継承した型ならxtal::nullを返す。
* それ以外の型の場合は未定義。
*/
template<class T>
inline typename CastResult<T>::type 
cast(const AnyPtr& a){
	return CastHelperHelper<T>::cast(a);
}

/**
* \brief T型に変換できるか調べる。
*
*/
template<class T>
inline bool can_cast(const AnyPtr& a){
	return CastHelperHelper<T>::can_cast(a);
}

/**
* \brief T型にチェック無しで変換する。
*
*/
template<class T>
inline typename CastResult<T>::type 
unchecked_cast(const AnyPtr& a){
	return CastHelperHelper<T>::unchecked_cast(a);
}

//@}

//////////////////////////////////////////

template<class T>
inline typename CastResult<T>::type 
tricky_cast(const AnyPtr& a, void (*f)(T)){
	return CastHelperHelper<T>::cast(a);
}

}

#endif // XTAL_CAST_H_INCLUDE_GUARD
