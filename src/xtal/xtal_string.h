
#pragma once

namespace xtal{

class LargeString;

class String : public Any{
public:

	String(){}

	/**
	* @brief NUL終端のC文字列から構築する
	*
	* @param str NULL終端文字列
	*/
	String(const char_t* str);

	/**
	* @brief STLの文字列から構築する
	*
	*/
	String(const string_t& str);

	/**
	* @brief C文字列からsize分の長さを取り出し構築する
	*
	*/
	String(const char_t* str, uint_t size);
	
	/**
	* @brief beginからlastまでの文字列で構築する
	*
	* [begin, last)
	*/
	String(const char_t* begin, const char_t* last);

	/**
	* @brief 二つのC文字列から構築する
	*
	*/
	String(const char_t* str1, uint_t size1, const char_t* str2, uint_t size2);

	/**
	* @brief 1つの文字から構築する
	*/
	String(char_t a);

	/**
	* @brief 2つの文字から構築する
	*/
	String(char_t a, char_t b);

	/**
	* @brief 3つの文字から構築する
	*/
	String(char_t a, char_t b, char_t c);

public:

	String(const char_t* str, uint_t len, uint_t hashcode, uint_t length, bool intern_flag = false);
	
	String(LargeString* left, LargeString* right);

protected:

	String(noinit_t n)
		:Any(n){}

public:

	/**
	* @brief 0終端の文字列先頭のポインタを返す。
	*
	*/
	const char_t* c_str();

	/**
	* @brief 文字列先頭のポインタを返す。
	*
	* これは0終端文字列が返されるとは限らない。
	*/
	const char_t* data();

	/**
	* @brief データサイズを返す。
	*
	* 文字列の長さではなく、バイナリとして見た場合のサイズを返す。
	*/
	uint_t buffer_size();

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

	/*
	* @brief 浅いコピーを返す。
	*
	*/
	StringPtr clone();

	/**
	* @brief 一意化した文字列を返す。
	*
	*/
	const InternedStringPtr& intern();

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
	* @brief sepで区切った文字列を要素とするIteratorを返す。
	*
	* @param sep 文字列かPEGのParser
	*/ 
	AnyPtr split(const AnyPtr& sep);

	/**
	* @brief 一文字づつの文字列を要素とするIteratorを返す。
	*
	*/
	AnyPtr each();

	/**
	*
	*
	*/
	AnyPtr scan(const AnyPtr& p);

	/**
	* @brief patternと一致する部分を置き換えた新しい文字列を返す
	*
	* @param  pattern 置き換える文字列かPEG
	*/ 
	AnyPtr replace(const AnyPtr& pattern, const StringPtr& str);

	/**
	* @brief 連結する
	*
	*/
	StringPtr cat(const StringPtr& v);

	uint_t hashcode();

public:

	ChRangePtr op_range(const StringPtr& right, int_t kind);
	StringPtr op_cat(const StringPtr& v);
	bool op_eq(const StringPtr& v);
	bool op_lt(const StringPtr& v);

private:
	void init_string(const char_t* str, uint_t size);
	int_t calc_offset(int_t pos);
	void throw_index_error();

};

class LargeString : public Base{
public:

	LargeString(const char_t* str1, uint_t size1, const char_t* str2, uint_t size2);
	LargeString(const char_t* str, uint_t len, uint_t hashcode, uint_t length, bool intern_flag = false);
	LargeString(LargeString* left, LargeString* right);
	virtual ~LargeString();

public:

	const char_t* c_str();
	uint_t buffer_size(){ return buffer_size_; }
	uint_t length(){ return length_; }
	bool is_interned(){ return (flags_ & INTERNED)!=0; }
	uint_t hashcode();

private:
	
	void common_init(uint_t len);
	void became_unified();
	void write_to_memory(LargeString* p, char_t* memory, uint_t& pos);

	virtual void visit_members(Visitor& m);

	enum{
		ROPE = 1<<0,
		INTERNED = 1<<1,

	};
	uint_t flags_;

	struct Str{
		char_t* p;
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

	uint_t buffer_size_;
	uint_t length_;
};

/**
* @brief Intern済みのString
*
* これはC++の型システムのために存在する。
*/
class InternedString : public String{
public:

	/**
	* @brief NUL終端のC文字列から構築する
	*
	* @param str NULL終端文字列
	*/
	InternedString(const char_t* str);

	/**
	* @brief STLの文字列から構築する
	*
	*/
	InternedString(const string_t& str);

	/**
	* @brief C文字列からsize分の長さを取り出し構築する
	*
	*/
	InternedString(const char_t* str, uint_t size);
	
	/**
	* @brief beginからlastまでの文字列で構築する
	*
	* [begin, last)
	*/
	InternedString(const char_t* begin, const char_t* last);

	/**
	* @brief 1つの文字から構築する
	*/
	InternedString(char_t a);

	/**
	* @brief 2つの文字から構築する
	*/
	InternedString(char_t a, char_t b);

	/**
	* @brief 3つの文字から構築する
	*/
	InternedString(char_t a, char_t b, char_t c);

	/**
	* @brief Stringから構築する
	*
	*/
	InternedString(const StringPtr& name);
		
};

inline bool operator ==(const InternedStringPtr& a, const InternedStringPtr& b){ return raweq(a, b); }
inline bool operator !=(const InternedStringPtr& a, const InternedStringPtr& b){ return rawne(a, b); }


class ChRange : public Base{
public:

	ChRange(const StringPtr& left, const StringPtr& right)
		:left_(left), right_(right){}

public:

	const StringPtr& left(){ return left_; }

	const StringPtr& right(){ return right_; }

	AnyPtr each();

private:

	virtual void visit_members(Visitor& m){
		Base::visit_members(m);
		m & left_ & right_;
	}

	StringPtr left_;
	StringPtr right_;
};



struct Named2;

/**
* @brief 名前付き引数のトリックのためのクラス
*
*/
struct Named2{
	InternedStringPtr name;
	AnyPtr value;

	Named2(const char* name)
		:name(name), value(nop){}

	Named2(const InternedStringPtr& name)
		:name(name), value(nop){}

	Named2(const InternedStringPtr& name, const AnyPtr& value)
		:name(name), value(value){}

	Named2()
		:name(null), value(nop){}

	Named2(const Null&)
		:name(null), value(nop){}
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
	
#ifdef XTAL_USE_PREDEFINED_ID

//{ID{{
namespace id{
extern InternedStringPtr idblock_catch;
extern InternedStringPtr idcallee;
extern InternedStringPtr idnew;
extern InternedStringPtr idop_shl_assign;
extern InternedStringPtr idop_at;
extern InternedStringPtr idtest;
extern InternedStringPtr idfor;
extern InternedStringPtr idop_inc;
extern InternedStringPtr idserial_new;
extern InternedStringPtr idop_div_assign;
extern InternedStringPtr idop_mul;
extern InternedStringPtr idop_xor_assign;
extern InternedStringPtr idto_a;
extern InternedStringPtr idinitialize;
extern InternedStringPtr idonce;
extern InternedStringPtr idfalse;
extern InternedStringPtr iddo;
extern InternedStringPtr idstring;
extern InternedStringPtr idancestors;
extern InternedStringPtr idop_and_assign;
extern InternedStringPtr idop_add_assign;
extern InternedStringPtr idop_cat_assign;
extern InternedStringPtr idsingleton;
extern InternedStringPtr idop_shl;
extern InternedStringPtr idblock_next;
extern InternedStringPtr idyield;
extern InternedStringPtr idop_shr_assign;
extern InternedStringPtr idop_cat;
extern InternedStringPtr idop_neg;
extern InternedStringPtr idvalue;
extern InternedStringPtr iddefault;
extern InternedStringPtr idcase;
extern InternedStringPtr idto_s;
extern InternedStringPtr idop_dec;
extern InternedStringPtr idop_shr;
extern InternedStringPtr idpure;
extern InternedStringPtr idfinally;
extern InternedStringPtr idthis;
extern InternedStringPtr idnull;
extern InternedStringPtr idop_div;
extern InternedStringPtr idserial_load;
extern InternedStringPtr idIOError;
extern InternedStringPtr id_dummy_lhs_parameter_;
extern InternedStringPtr idcatch;
extern InternedStringPtr idop_mul_assign;
extern InternedStringPtr idmethod;
extern InternedStringPtr idop_lt;
extern InternedStringPtr idset_at;
extern InternedStringPtr id_switch_;
extern InternedStringPtr idop_mod_assign;
extern InternedStringPtr idbreak;
extern InternedStringPtr idtry;
extern InternedStringPtr idop_mod;
extern InternedStringPtr idnop;
extern InternedStringPtr idto_i;
extern InternedStringPtr idop_or;
extern InternedStringPtr idcontinue;
extern InternedStringPtr ide;
extern InternedStringPtr iditerator;
extern InternedStringPtr idthrow;
extern InternedStringPtr idop_and;
extern InternedStringPtr idelse;
extern InternedStringPtr idfun;
extern InternedStringPtr idto_f;
extern InternedStringPtr idop_sub_assign;
extern InternedStringPtr idlib;
extern InternedStringPtr iddofun;
extern InternedStringPtr ideach;
extern InternedStringPtr idop_set_at;
extern InternedStringPtr ids_load;
extern InternedStringPtr idclass;
extern InternedStringPtr idop_com;
extern InternedStringPtr idop_pos;
extern InternedStringPtr idop_add;
extern InternedStringPtr idop_ushr_assign;
extern InternedStringPtr idnobreak;
extern InternedStringPtr idcurrent_context;
extern InternedStringPtr idto_m;
extern InternedStringPtr idreturn;
extern InternedStringPtr idop_eq;
extern InternedStringPtr idfiber;
extern InternedStringPtr idop_or_assign;
extern InternedStringPtr ids_save;
extern InternedStringPtr idstatic;
extern InternedStringPtr idswitch;
extern InternedStringPtr idop_sub;
extern InternedStringPtr idop_ushr;
extern InternedStringPtr idfirst_step;
extern InternedStringPtr idblock_break;
extern InternedStringPtr idserial_save;
extern InternedStringPtr idop_range;
extern InternedStringPtr id_dummy_fun_parameter_;
extern InternedStringPtr id_dummy_block_parameter_;
extern InternedStringPtr idunittest;
extern InternedStringPtr idtrue;
extern InternedStringPtr idop_xor;
extern InternedStringPtr idblock_first;
extern InternedStringPtr idop_call;
extern InternedStringPtr id_initialize_;
extern InternedStringPtr idis;
extern InternedStringPtr idwhile;
extern InternedStringPtr idit;
extern InternedStringPtr idassert;
extern InternedStringPtr idxtal;
extern InternedStringPtr idif;
extern InternedStringPtr idp;
}
//}}ID}





























#else

#endif

}
