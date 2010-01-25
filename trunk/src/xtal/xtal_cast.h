/** \file src/xtal/xtal_cast.h
* \brief src/xtal/xtal_cast.h
*/

#ifndef XTAL_CAST_H_INCLUDE_GUARD
#define XTAL_CAST_H_INCLUDE_GUARD

#pragma once

namespace xtal{

const char_t* cast_const_char_t_ptr(const AnyPtr& a);
const IDPtr& cast_IDPtr(const AnyPtr& a);
int_t cast_Int(const AnyPtr& a);
float_t cast_Float(const AnyPtr& a);

/////////////////////////////////////////////////////////////////////////////

template<class T> struct NullGetter{ static T get(){ return 0; } };
template<class T> struct NullGetter<const SmartPtr<T>&>{ static const SmartPtr<T>& get(){ return nul<T>(); } };
template<class T> struct NullGetter<SmartPtr<T> >{ static const SmartPtr<T>& get(){ return nul<T>(); } };

/////////////////////////////////////////////////////////////////////////////


template<class T>
struct Caster{
	typedef typename Caster<const T&>::type type;
	static type cast(const AnyPtr& a){ return Caster<const T&>::cast(a); }
};

template<class T>
struct Caster<const T*>{
	typedef const T* type;
	static type cast(const AnyPtr& a){ return &Caster<const T&>::cast(a); }
};

template<class T>
struct Caster<T*>{
	typedef T* type;
	static type cast(const AnyPtr& a){ return (type)&Caster<const T&>::cast(a); }
};

template<class T>
struct Caster<const T&>{
	typedef const T& type;
	static type cast(const AnyPtr& a){ return *((const SmartPtr<T>*)&a)->get(); }
};

template<class T>
struct Caster<const SmartPtr<T>&>{
	typedef const SmartPtr<T>& type;
	static type cast(const AnyPtr& a){ return *(const SmartPtr<T>*)&a; }
};

template<>
struct Caster<const char_t*>{
	typedef const char* type;
	static type cast(const AnyPtr& a){ return cast_const_char_t_ptr(a); }
};

template<>
struct Caster<const IDPtr&>{
	typedef const IDPtr& type;
	static type cast(const AnyPtr& a){ return *(const IDPtr*)&a; }
};

template<>
struct Caster<const bool&>{
	typedef bool type;
	static type cast(const AnyPtr& a){ return a; }
};

#undef XTAL_CAST_HELPER

#define XTAL_CAST_HELPER(Type, XType) \
template<>\
struct Caster<const Type&>{\
	typedef Type type;\
	static type cast(const AnyPtr& a){ return (type)cast_##XType(a); }\
}

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
* \brief T型に変換できるか調べる。
*
*/
template<class T>
inline bool can_cast(const AnyPtr& a){
	return a->is(cpp_class<typename CppClassSymbol<T>::type>());
}

/**
* \brief T型にチェック無しで変換する。
*
*/
template<class T>
inline typename Caster<T>::type 
unchecked_cast(const AnyPtr& a){
	return Caster<T>::cast(a);
}

/**
* \brief T型に変換する。
*
* Tに変換できない場合、
* Tがポインタ型なら(T*)NULLを返す。
* Tが値か参照で、AnyPtrを継承した型ならxtal::nullを返す。
* それ以外の型の場合はコンパイルエラーとなる。
*/
template<class T>
inline typename Caster<T>::type 
cast(const AnyPtr& a){
	if(can_cast<T>(a)){
		return unchecked_cast<T>(a);
	}
	else{
		return NullGetter<typename Caster<T>::type>::get();
	}
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
* \brief SmartPtr<T>型に変換する。
*
* Tに変換できない場合xtal::nullを返す。
*/
template<class T>
inline const SmartPtr<T>&
ptr_cast(const AnyPtr& a){
	if(can_cast<T>(a)){
		return unchecked_ptr_cast<T>(a);
	}
	else{
		return nul<T>();
	}
}

//@}

//////////////////////////////////////////

template<class T>
inline typename Caster<T>::type 
tricky_cast(const AnyPtr& a, void (*f)(T)){
	return cast<T>(a);
}

}

#endif // XTAL_CAST_H_INCLUDE_GUARD
