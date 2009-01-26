
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
	* @brief NUL終端のC文字列から構築する
	*
	* @param str NULL終端文字列
	*/
	String(const avoid<char>::type* str);

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

	String(const String& s);

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
	uint_t data_size();

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
	* @brief 一文字づつの文字列を要素とするIteratorを返す。
	*
	*/
	AnyPtr each();

	/**
	* @brief 連結する
	*
	*/
	StringPtr cat(const StringPtr& v);

	uint_t hashcode();

	AnyPtr split(const AnyPtr& pattern);

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
	uint_t data_size(){ return data_size_; }
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

	uint_t data_size_;
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
	* @brief NUL終端のC文字列から構築する
	*
	* @param str NULL終端文字列
	*/
	ID(const avoid<char>::type* str);

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

	ID(const char_t* str, uint_t len, uint_t hashcode, uint_t length);

	/**
	* @brief Stringから構築する
	*
	*/
	ID(const StringPtr& name);
		
};

inline bool operator ==(const IDPtr& a, const IDPtr& b){ return raweq(a, b); }
inline bool operator !=(const IDPtr& a, const IDPtr& b){ return rawne(a, b); }

AnyPtr interned_strings();
int_t edit_distance(const StringPtr& str1, const StringPtr& str2);

class ChRange : public Range{
public:

	ChRange(const StringPtr& left, const StringPtr& right)
		:Range(left, right, CLOSED){}

public:

	const StringPtr& left(){ return static_ptr_cast<String>(left_); }

	const StringPtr& right(){ return static_ptr_cast<String>(right_); }

	AnyPtr each();
};



struct Named2;

/**
* @brief 名前付き引数のトリックのためのクラス
*
*/
struct Named2{
	IDPtr name;
	AnyPtr value;

	Named2(const char_t* name)
		:name(name), value(undefined){}

	Named2(const avoid<char>::type* name)
		:name(name), value(undefined){}

	Named2(const IDPtr& name)
		:name(name), value(undefined){}

	Named2(const IDPtr& name, const AnyPtr& value)
		:name(name), value(value){}

	Named2()
		:name(null), value(undefined){}

	Named2(const Null&)
		:name(null), value(undefined){}
};

struct Param : public Named2{
	template<class T>
	Param(const T& value)
		:Named2(null, value){}
	
	Param(const AnyPtr& value)
		:Named2(null, value){}

	Param(const IDPtr& name, const AnyPtr& value)
		:Named2(name, value){}

	Param(){}
};

/**
* @brief 名前付き引数のトリックのためのクラス
*
*/
struct Named : public Param{

	Named(const AnyPtr& value)
		:Param(null, value){}

	Named(const IDPtr& name, const AnyPtr& value)
		:Param(name, value){}

	Named(){}

	Named(const Null&){}
};

void visit_members(Visitor& m, const Named& p);

const IDPtr& string_literal_to_id(const char_t* str);
	
}