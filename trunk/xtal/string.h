
#pragma once

#include "any.h"

namespace xtal{

void InitString(); 
	
// fwd decl
class StringImpl;

/**
* @brief 文字列
*
*/
class String : public Any{
public:

	/**
	* @brief NUL終端のC文字列から構築する
	*
	* @param str NULL終端文字列
	*/
	String(const char* str = "");

	/**
	* @brief STLの文字列から構築する
	*
	*/
	String(const string_t& str);

	/**
	* @brief C文字列からsize分の長さを取り出し構築する
	*
	*/
	String(const char* str, int_t size);
	
	/**
	* @brief beginからlastまでの文字列で構築する
	*
	* [begin, last)
	*/
	String(const char* begin, const char* last);

	/**
	* @brief 二つのC文字列から構築する
	*
	*/
	String(const char* str1, int_t size1, const char* str2, int_t size2);


	struct delegate_memory_t{};

	/**
	* @brief user_mallocで獲得したメモリを委譲して構築する。
	*
	* @param str NULL終端文字列
	* @param size 文字列の長さ
	* @param buffer_size 確保したバッファのサイズ
	*/
	String(char* str, int_t size, int_t buffer_size, delegate_memory_t);

	explicit String(StringImpl* p)
		:Any((AnyImpl*)p){}

	explicit String(const StringImpl* p)
		:Any((AnyImpl*)p){}

	/**
	* @brief 文字列を生成せず、nullを入れる
	*
	*/
	String(const Null&)
		:Any(null){}

public:

	/**
	* @brief 0終端の文字列先頭のポインタを返す。
	*
	*/
	const char* c_str() const;

	/**
	* @brief 文字列の長さを返す。
	*
	*/
	int_t size() const;

	/**
	* @brief 文字列の長さを返す。
	*
	*/
	int_t length() const;

	/**
	* @brief 部分文字列を取り出す。
	*
	*/
	String slice(int_t first, int_t last) const;

	/*
	* @brief 浅いコピーを返す。
	*
	*/
	String clone() const;

	/**
	* @brief 一意化した文字列を返す。
	*
	*/
	ID intern() const;

	/**
	* @brief 一意化されているか返す。
	*/
	bool is_interned() const;

	/**
	* @brief 整数に変換した結果を返す。
	*
	*/ 
	int_t to_i() const;
	
	/**
	* @brief 浮動小数点数に変換した結果を返す。
	*
	*/ 
	float_t to_f() const;
	
	/**
	* @brief 自分自身を返す。
	*
	*/
	String to_s() const;

	/**
	* @brief sepで区切った文字列を得られるイテレータを返す。
	*
	*/ 
	Any split(const String& sep) const;

	/**
	* @brief 連結する
	*
	*/
	String cat(const String& v) const;

public:

	String op_cat_String(const String& v) const;
	String op_mul_Int(int_t v) const;

	void op_cat(const VMachine& vm) const;
	void op_eq(const VMachine& vm) const;
	void op_lt(const VMachine& vm) const;

	String op_cat_r_String(const String& v) const;
	bool op_eq_r_String(const String& v) const;
	bool op_lt_r_String(const String& v) const;

	StringImpl* impl() const{ return (StringImpl*)Any::impl(); }
};

/**
* @brief 必ずインターン済みの文字列を保持するためのクラス
*
*/
class ID : public String{
public:
	
	/**
	* @brief NUL終端のC文字列から構築する
	*
	*/	
	ID(const char* name = "");
	
	/**
	* @brief C文字列からsize分の長さを取り出し構築する
	*
	*/
	ID(const char* name, int_t size);

	/**
	* @brief Stringから構築する
	*
	*/
	ID(const String& name);
		
	/**
	* @brief 文字列を生成せず、nullを入れる
	*
	*/
	ID(const Null&)
		:String(null){}
};

inline bool operator ==(const ID& a, const ID& b){ return a.raweq(b); }
inline bool operator !=(const ID& a, const ID& b){ return !a.raweq(b); }

struct Named2;

/**
* @brief 名前付き引数のトリックのためのクラス
*
*/
struct Named2{
	ID name;
	Any value;

	Named2(const char* name)
		:name(name){}

	Named2(const ID& name)
		:name(name), value(null){}

	Named2(const ID& name, const Any& value)
		:name(name), value(value){}

	Named2()
		:name(null), value(null){}

	Named2(const Null&)
		:name(null), value(null){}
};

/**
* @brief 名前付き引数のトリックのためのクラス
*
*/
struct Named : public Named2{

	Named(const ID& name)
		:Named2(name){}

	Named(const ID& name, const Any& value)
		:Named2(name, value){}

	Named(){}

	Named(const Null&){}
};

/*
* @brief 名前付き引数のトリックのためのクラス
*
*/
struct Key : public ID{
	
	/**
	* @brief IDから構築する
	*
	*/
	Key(const ID& name)
		:ID(name){}
	
	/**
	* @brief Key("key")=10という書式のための代入演算子
	*
	*/
	Named operator =(const Any& value){
		return Named(*this, value);
	}
};


template<class Ch, class T>
std::basic_ostream<Ch, T>& operator << (std::basic_ostream<Ch, T>& os, const Any& a){
	String str = a.to_s(); 
	os << str.c_str();
	return os;
}
	
#ifdef XTAL_USE_PREDEFINED_ID

//{ID{{
namespace id{
extern ID id__ARGS__;
extern ID idop_or_assign;
extern ID idop_add_assign;
extern ID idop_shr;
extern ID idop_ushr_assign;
extern ID idop_call;
extern ID idop_sub_assign;
extern ID idop_lt;
extern ID idop_eq;
extern ID idop_mul;
extern ID idop_neg;
extern ID idmarshal_new;
extern ID iditer_next;
extern ID iditer_first;
extern ID idtrue;
extern ID idmarshal_dump;
extern ID idop_and_assign;
extern ID idop_mod_assign;
extern ID idop_div_assign;
extern ID idop_or;
extern ID idop_div;
extern ID idlib;
extern ID idop_cat_assign;
extern ID idop_cat;
extern ID idIOError;
extern ID idmarshal_load;
extern ID idfalse;
extern ID idop_add;
extern ID idop_cat_r_String;
extern ID idop_dec;
extern ID idop_inc;
extern ID idop_ushr;
extern ID idop_pos;
extern ID idop_shr_assign;
extern ID idop_mod;
extern ID iditer_break;
extern ID idop_eq_r_String;
extern ID idstring;
extern ID idinitialize;
extern ID idop_set_at;
extern ID idop_lt_r_String;
extern ID idtest;
extern ID idop_at;
extern ID idop_shl_assign;
extern ID idop_sub;
extern ID idvalue;
extern ID idop_clone;
extern ID idop_com;
extern ID idnew;
extern ID idop_shl;
extern ID idop_xor;
extern ID idop_and;
extern ID idsize;
extern ID idop_xor_assign;
extern ID idop_mul_assign;
}
//}}ID}



#else

#endif

}
