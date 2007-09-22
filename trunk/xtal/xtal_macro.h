
#pragma once

#include "xtal_lib.h"
#include "xtal_iterator.h"

/** @addtogroup xmacro */
/*@{*/

/**
* @brief foreachを簡単に記述するためのマクロ
*
* @code
* Xfor(value, array){
*   // use value
* }
* @endcode
*/
#define Xfor(var, tar) \
	if(::xtal::BlockValueHolder<1> block_value_holder = tar)\
	for(::xtal::AnyPtr &var = block_value_holder.values[0]; block_value_holder;)\
	for(::xtal::block_next(block_value_holder.target, var, true); block_value_holder; ::xtal::block_next(block_value_holder.target, var, false))

/**
* @brief foreachを簡単に記述するためのマクロ
*
* @code
* Xfor(key, value, map.send("pairs")){
*   // use key and value
* }
* @endcode
*/
#define Xfor2(var1, var2, tar) \
	if(::xtal::BlockValueHolder<2> block_value_holder = tar)\
	for(::xtal::AnyPtr &var1 = block_value_holder.values[0], &var2 = block_value_holder.values[1]; block_value_holder;)\
	for(::xtal::block_next(block_value_holder.target, var1, var2, true); block_value_holder.target; ::xtal::block_next(block_value_holder.target, var1, var2, false))

/**
* @brief foreachを簡単に記述するためのマクロ
*
* @code
* Xfor(v1, v2, v3, hoge.send("each3")){
*   // use v1, v2 and v3
* }
* @endcode
*/
#define Xfor3(var1, var2, var3, tar) \
	if(::xtal::BlockValueHolder<3> block_value_holder = tar)\
	for(::xtal::AnyPtr &var1 = block_value_holder.values[0], &var2 = block_value_holder.values[1], &var3 = block_value_holder.values[2]; block_value_holder;)\
	for(::xtal::block_next(block_value_holder.target, var1, var2, var3, true); block_value_holder.target; ::xtal::block_next(block_value_holder.target, var1, var2, var3, false))


/**
* @brief get_textを簡単に記述するためのマクロ
*
* @code
* AnyPtr text = Xt("Text %d %s")(10, "test");
* @endcode
*/
#define Xt(text) ::xtal::get_text(text) 

/**
* @brief formatを簡単に記述するためのマクロ
*
* @code
* AnyPtr fmt = Xf("Text %d %s")(10, "test");
* @endcode
*/
#define Xf(text) ::xtal::format(text) 

/**
* @brief Xtalのソースを簡単に記述するためのマクロ
*
* @code
* AnyPtr src = Xsrc((
*   return [0, 2, 3, 4];
* ));
* @endcode
*/
#define Xsrc(text) ::xtal::source(#text+1, sizeof(#text)-3, __FILE__)

/*@}*/


#ifndef XTAL_USE_PREDEFINED_ID

/*
* @brief インターンされた文字列を簡単に記述するためのマクロ
* XTAL_USE_PREDEFINED_ID が定義されている場合、::xtal::id::id##string という名前の変数の直接アクセスとなる。
* ある事情により、これはユーザーが使ってはならないマクロとなった。
* 将来はまた使えるようになる日がくるかもしれない。
* 
* @code
* InternedStringPtr id = Xid(test);
* @endcode
*/
#define Xid(string) ::xtal::InternedStringPtr(#string, sizeof(#string)-1)

#else

#define Xid(string) ((const ::xtal::InternedStringPtr&) ::xtal::id::id##string)

#endif
