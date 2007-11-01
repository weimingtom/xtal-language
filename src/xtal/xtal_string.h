
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
	const IDPtr& intern();

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
	* @brief patternに順番にマッチした結果を要素とするIteratorを返す。
	*
	*/
	AnyPtr scan(const AnyPtr& pattern);

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
class ID : public String{
public:

	/**
	* @brief NUL終端のC文字列から構築する
	*
	* @param str NULL終端文字列
	*/
	ID(const char_t* str);

	/**
	* @brief STLの文字列から構築する
	*
	*/
	ID(const string_t& str);

	/**
	* @brief C文字列からsize分の長さを取り出し構築する
	*
	*/
	ID(const char_t* str, uint_t size);
	
	/**
	* @brief beginからlastまでの文字列で構築する
	*
	* [begin, last)
	*/
	ID(const char_t* begin, const char_t* last);

	/**
	* @brief 1つの文字から構築する
	*/
	ID(char_t a);

	/**
	* @brief 2つの文字から構築する
	*/
	ID(char_t a, char_t b);

	/**
	* @brief 3つの文字から構築する
	*/
	ID(char_t a, char_t b, char_t c);

	/**
	* @brief Stringから構築する
	*
	*/
	ID(const StringPtr& name);
		
};

inline bool operator ==(const IDPtr& a, const IDPtr& b){ return raweq(a, b); }
inline bool operator !=(const IDPtr& a, const IDPtr& b){ return rawne(a, b); }


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
	IDPtr name;
	AnyPtr value;

	Named2(const char* name)
		:name(name), value(nop){}

	Named2(const IDPtr& name)
		:name(name), value(nop){}

	Named2(const IDPtr& name, const AnyPtr& value)
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

	explicit Named(const IDPtr& name)
		:Named2(name){}

	Named(const IDPtr& name, const AnyPtr& value)
		:Named2(name, value){}

	Named(){}

	Named(const Null&){}
};

/*
* @brief 名前付き引数のトリックのためのクラス
*
*/
struct Key : public IDPtr{
	
	/**
	* @brief IDPtrから構築する
	*
	*/
	Key(const IDPtr& name)
		:IDPtr(name){}
	
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
extern IDPtr idop_inc;
extern IDPtr idblock_catch;
extern IDPtr idcallee;
extern IDPtr idnew;
extern IDPtr idop_shl_assign;
extern IDPtr idop_at;
extern IDPtr idtest;
extern IDPtr idfor;
extern IDPtr idserial_new;
extern IDPtr idop_div_assign;
extern IDPtr idop_mul;
extern IDPtr idop_xor_assign;
extern IDPtr idto_a;
extern IDPtr idinitialize;
extern IDPtr idonce;
extern IDPtr iddo;
extern IDPtr idstring;
extern IDPtr idfalse;
extern IDPtr idancestors;
extern IDPtr idop_and_assign;
extern IDPtr idop_add_assign;
extern IDPtr idop_cat_assign;
extern IDPtr idsingleton;
extern IDPtr idop_shl;
extern IDPtr idblock_next;
extern IDPtr idyield;
extern IDPtr idop_shr_assign;
extern IDPtr idop_cat;
extern IDPtr idop_neg;
extern IDPtr idop_dec;
extern IDPtr idvalue;
extern IDPtr iddefault;
extern IDPtr idcase;
extern IDPtr idto_s;
extern IDPtr idop_shr;
extern IDPtr idpure;
extern IDPtr idfinally;
extern IDPtr idthis;
extern IDPtr idnull;
extern IDPtr idop_div;
extern IDPtr idserial_load;
extern IDPtr idIOError;
extern IDPtr id_dummy_lhs_parameter_;
extern IDPtr idin;
extern IDPtr idcatch;
extern IDPtr idop_mul_assign;
extern IDPtr idmethod;
extern IDPtr idop_lt;
extern IDPtr idset_at;
extern IDPtr id_switch_;
extern IDPtr idop_mod_assign;
extern IDPtr idbreak;
extern IDPtr idtry;
extern IDPtr idop_mod;
extern IDPtr idnop;
extern IDPtr idto_i;
extern IDPtr idop_or;
extern IDPtr idcontinue;
extern IDPtr ide;
extern IDPtr iditerator;
extern IDPtr idthrow;
extern IDPtr idop_and;
extern IDPtr idelse;
extern IDPtr idfun;
extern IDPtr idto_f;
extern IDPtr idop_sub_assign;
extern IDPtr idlib;
extern IDPtr iddofun;
extern IDPtr ideach;
extern IDPtr idop_set_at;
extern IDPtr idop_in;
extern IDPtr ids_load;
extern IDPtr idclass;
extern IDPtr idop_com;
extern IDPtr idop_pos;
extern IDPtr idop_add;
extern IDPtr idop_ushr_assign;
extern IDPtr idnobreak;
extern IDPtr idcurrent_context;
extern IDPtr idto_m;
extern IDPtr idreturn;
extern IDPtr idop_eq;
extern IDPtr idfiber;
extern IDPtr idop_or_assign;
extern IDPtr ids_save;
extern IDPtr idstatic;
extern IDPtr idswitch;
extern IDPtr idop_sub;
extern IDPtr idop_ushr;
extern IDPtr idfirst_step;
extern IDPtr idblock_break;
extern IDPtr idserial_save;
extern IDPtr idop_range;
extern IDPtr id_dummy_fun_parameter_;
extern IDPtr id_dummy_block_parameter_;
extern IDPtr idunittest;
extern IDPtr idop_xor;
extern IDPtr idblock_first;
extern IDPtr idtrue;
extern IDPtr idop_call;
extern IDPtr id_initialize_;
extern IDPtr idis;
extern IDPtr idwhile;
extern IDPtr idit;
extern IDPtr idassert;
extern IDPtr idxtal;
extern IDPtr idif;
extern IDPtr idp;
}
//}}ID}

































#else

#endif

}
