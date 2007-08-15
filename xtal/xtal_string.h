
#pragma once

#include "xtal_any.h"
#include "xtal_smartptr.h"
#include "xtal_fwd.h"

namespace xtal{

class LargeString;

class String : public Innocence{
public:

	String(){}

	/**
	* @brief NUL終端のC文字列から構築する
	*
	* @param str NULL終端文字列
	*/
	String(const char* str);

	/**
	* @brief STLの文字列から構築する
	*
	*/
	String(const string_t& str);

	/**
	* @brief C文字列からsize分の長さを取り出し構築する
	*
	*/
	String(const char* str, uint_t size);
	
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
	String(const char* str1, uint_t size1, const char* str2, uint_t size2);

	struct delegate_memory_t{};

	/**
	* @brief user_mallocで獲得したメモリを委譲して構築する。
	*
	* @param str NULL終端文字列
	* @param size 文字列の長さ
	* @param buffer_size 確保したバッファのサイズ
	*/
	String(char* str, uint_t size, uint_t buffer_size, delegate_memory_t);

	String(const char* str, uint_t len, uint_t hashcode);
	
	String(LargeString* left, LargeString* right);

public:

	/**
	* @brief 0終端の文字列先頭のポインタを返す。
	*
	*/
	const char* c_str();

	/**
	* @brief 文字列の長さを返す。
	*
	*/
	uint_t size();

	/**
	* @brief 文字列の長さを返す。
	*
	*/
	uint_t length();

	/**
	* @brief 部分文字列を取り出す。
	*
	*/
	StringPtr slice(int_t i, int_t n);

	/*
	* @brief 浅いコピーを返す。
	*
	*/
	StringPtr clone();

	/**
	* @brief 一意化した文字列を返す。
	*
	*/
	InternedStringPtr intern();

	/**
	* @brief 一意化されているか返す。
	*/
	bool is_interned();

	/**
	* @brief 整数に変換した結果を返す。
	*
	*/ 
	int_t to_i();
	
	/**
	* @brief 浮動小数点数に変換した結果を返す。
	*
	*/ 
	float_t to_f();
	
	/**
	* @brief 自分自身を返す。
	*
	*/
	StringPtr to_s();

	/**
	* @brief sepで区切った文字列を得られるイテレータを返す。
	*
	*/ 
	AnyPtr split(const StringPtr& sep);

	/**
	* @brief 一文字づつ得られるイテレータを返す。
	*
	*/
	AnyPtr each();

	/**
	* @brief 連結する
	*
	*/
	StringPtr cat(const StringPtr& v);

	uint_t hashcode();

public:

	StringPtr op_cat_String(const StringPtr& v);
	StringPtr op_mul_Int(int_t v);

	void op_cat(const VMachinePtr& vm);
	void op_eq(const VMachinePtr& vm);
	void op_lt(const VMachinePtr& vm);

	StringPtr op_cat_r_String(const StringPtr& v);
	bool op_eq_r_String(const StringPtr& v);
	bool op_lt_r_String(const StringPtr& v);

private:

	int_t calc_offset(int_t pos);
	void throw_index_error();

};

class LargeString : public Base{
public:


	LargeString(const char* str = "");
	LargeString(const string_t& str);
	LargeString(const char* str, uint_t size);
	LargeString(const char* begin, const char* last);
	LargeString(const char* str1, uint_t size1, const char* str2, uint_t size2);
	LargeString(char* str, uint_t size, uint_t buffer_size, String::delegate_memory_t);
	LargeString(const char* str, uint_t len, uint_t hashcode);
	LargeString(LargeString* left, LargeString* right);
	~LargeString();

public:

	const char* c_str();
	uint_t size();
	bool is_interned();
	uint_t hashcode();

private:
	
	void common_init(uint_t len);
	void became_unified();
	void write_to_memory(LargeString* p, char_t* memory, uint_t& pos);

	virtual void visit_members(Visitor& m);

	enum{
		ROPE = 1<<0,
		INTERNED = 1<<1,
		NOFREE = 1<<2,
		HASHED = 1<<3
	};
	uint_t flags_;

	struct Str{
		char* p;
		uint_t hashcode;
	};

	struct Rope{
		LargeString* left;
		LargeString* right;
	};

	union{
		Str str_;
		Rope rope_;
	};

	uint_t size_;
};

/**
* @brief 必ずインターン済みの文字列を保持するためのクラス
*
*/
class InternedStringPtr : public StringPtr{
public:
	
	/**
	* @brief NUL終端のC文字列から構築する
	*
	*/	
	InternedStringPtr(const char* name = "");
	
	/**
	* @brief C文字列からsize分の長さを取り出し構築する
	*
	*/
	InternedStringPtr(const char* name, int_t size);

	/**
	* @brief Stringから構築する
	*
	*/
	InternedStringPtr(const StringPtr& name);
		
	/**
	* @brief 文字列を生成せず、nullを入れる
	*
	*/
	InternedStringPtr(const Null&)
		:StringPtr(null){}

private:

	static StringPtr make(const char* name, uint_t size);

};

inline bool operator ==(const InternedStringPtr& a, const InternedStringPtr& b){ return raweq(a, b); }
inline bool operator !=(const InternedStringPtr& a, const InternedStringPtr& b){ return rawne(a, b); }

struct Named2;

/**
* @brief 名前付き引数のトリックのためのクラス
*
*/
struct Named2{
	InternedStringPtr name;
	AnyPtr value;

	Named2(const char* name)
		:name(name){}

	Named2(const InternedStringPtr& name)
		:name(name), value(null){}

	Named2(const InternedStringPtr& name, const AnyPtr& value)
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

	explicit Named(const InternedStringPtr& name)
		:Named2(name){}

	Named(const InternedStringPtr& name, const AnyPtr& value)
		:Named2(name, value){}

	Named(){}

	Named(const Null&){}
};

/*
* @brief 名前付き引数のトリックのためのクラス
*
*/
struct Key : public InternedStringPtr{
	
	/**
	* @brief InternedStringPtrから構築する
	*
	*/
	Key(const InternedStringPtr& name)
		:InternedStringPtr(name){}
	
	/**
	* @brief Key("key")=10という書式のための代入演算子
	*
	*/
	Named operator =(const AnyPtr& value){
		return Named(*this, value);
	}
};

void visit_members(Visitor& m, const Named& p);


template<class Ch, class T>
std::basic_ostream<Ch, T>& operator << (std::basic_ostream<Ch, T>& os, const AnyPtr& a){
	StringPtr str = a->to_s(); 
	os << str->c_str();
	return os;
}
	
#ifdef XTAL_USE_PREDEFINED_ID

//{ID{{
namespace id{
extern InternedStringPtr id__ARGS__;
extern InternedStringPtr idop_or_assign;
extern InternedStringPtr idop_add_assign;
extern InternedStringPtr idop_shr;
extern InternedStringPtr idop_ushr_assign;
extern InternedStringPtr idserial_save;
extern InternedStringPtr idop_call;
extern InternedStringPtr idop_sub_assign;
extern InternedStringPtr idop_lt;
extern InternedStringPtr idop_mul;
extern InternedStringPtr idop_eq;
extern InternedStringPtr idop_neg;
extern InternedStringPtr idserial_load;
extern InternedStringPtr iditer_next;
extern InternedStringPtr iditer_first;
extern InternedStringPtr idtrue;
extern InternedStringPtr idp;
extern InternedStringPtr idset_at;
extern InternedStringPtr idop_and_assign;
extern InternedStringPtr idop_mod_assign;
extern InternedStringPtr idop_div_assign;
extern InternedStringPtr idop_or;
extern InternedStringPtr idop_div;
extern InternedStringPtr idlib;
extern InternedStringPtr idop_cat_assign;
extern InternedStringPtr idop_cat;
extern InternedStringPtr idIOError;
extern InternedStringPtr idop_cat_r_String;
extern InternedStringPtr idfalse;
extern InternedStringPtr idserial_new;
extern InternedStringPtr idto_i;
extern InternedStringPtr idop_add;
extern InternedStringPtr idop_ushr;
extern InternedStringPtr idop_pos;
extern InternedStringPtr idop_dec;
extern InternedStringPtr idop_inc;
extern InternedStringPtr idop_eq_r_String;
extern InternedStringPtr iditer_break;
extern InternedStringPtr idto_f;
extern InternedStringPtr idop_shr_assign;
extern InternedStringPtr idop_mod;
extern InternedStringPtr idstring;
extern InternedStringPtr idop_set_at;
extern InternedStringPtr idop_lt_r_String;
extern InternedStringPtr idinitialize;
extern InternedStringPtr idtest;
extern InternedStringPtr idop_at;
extern InternedStringPtr idvalue;
extern InternedStringPtr idto_s;
extern InternedStringPtr idop_shl_assign;
extern InternedStringPtr idop_sub;
extern InternedStringPtr idop_com;
extern InternedStringPtr idnew;
extern InternedStringPtr idop_shl;
extern InternedStringPtr idop_xor;
extern InternedStringPtr idop_and;
extern InternedStringPtr idat;
extern InternedStringPtr idop_xor_assign;
extern InternedStringPtr idop_mul_assign;
}
//}}ID}








#else

#endif

}
