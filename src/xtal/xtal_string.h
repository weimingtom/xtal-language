#ifndef XTAL_STRING_H_INCLUDE_GUARD
#define XTAL_STRING_H_INCLUDE_GUARD

#pragma once

namespace xtal{

class String : public Any{
public:

	String();

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
	* @brief C文字列からsize分の長さを取り出し構築する
	*
	*/
	String(const char_t* str, uint_t size);

	/**
	* @brief C文字列リテラルから構築する
	*
	*/
	String(const StringLiteral& str);

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

	struct make_t{};
	String(const char_t* str, uint_t size, make_t);
	
	String& operator= (const String& s);

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

	bool is_ch();

	int_t ascii();

public:

	ChRangePtr op_range(const StringPtr& right, int_t kind);
	
	StringPtr op_cat(const StringPtr& v);
	
	bool op_eq(const StringPtr& v);

	bool op_lt(const StringPtr& v);

private:
	void init_string(const char_t* str, uint_t size);
};

class StringData : public RefCountingBase{
	enum{
		INTERNED = 1<<0,
		SIZE_SHIFT = 1
	};

	XTAL_DEBUG_ONLY(char_t* buf_);

	uint_t data_size_;
public:

	StringData(uint_t size){
		set_pvalue(*this, TYPE_STRING, this);
		data_size_ = size<<SIZE_SHIFT;
		buf()[size] = 0;
		XTAL_DEBUG_ONLY(buf_ = buf());
	}

	~StringData(){}

	uint_t data_size(){
		return data_size_>>SIZE_SHIFT;
	}

	char_t* buf(){ 
		return (char_t*)(this+1); 
	}

	bool is_interned(){
		return (data_size_&INTERNED)!=0;
	}

	void set_interned(){
		data_size_ |= INTERNED;
	}

	static uint_t calc_size(uint_t sz){
		return sizeof(StringData)+(sz+1)*sizeof(char_t);
	}

private:
	
	XTAL_DISALLOW_COPY_AND_ASSIGN(StringData);
};


/**
* @brief Intern済みのString
*
* これはC++の型システムのために存在する。
* XtalにおいてIntern済みStringはString型である。
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
	* @brief C文字列からsize分の長さを取り出し構築する
	*
	*/
	ID(const char_t* str, uint_t size);
	
	/**
	* @brief C文字列リテラルから構築する
	*
	*/
	ID(const StringLiteral& str);

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

AnyPtr interned_strings();
int_t edit_distance(const StringPtr& str1, const StringPtr& str2);
uint_t string_hashcode(const char_t* str, uint_t size);
void string_data_size_and_hashcode(const char_t* str, uint_t& size, uint_t& hash);
uint_t string_length(const char_t* str);
uint_t string_data_size(const char_t* str);
int_t string_compare(const char_t* a, uint_t asize, const char_t* b, uint_t bsize);
void string_copy(char_t* a, const char_t* b, uint_t size);
bool string_is_ch(const char_t* str, uint_t size);

class ChRange : public Range{
public:

	ChRange(const StringPtr& left, const StringPtr& right)
		:Range(left, right, CLOSED){}

public:

	const StringPtr& left(){ return unchecked_ptr_cast<String>(left_); }

	const StringPtr& right(){ return unchecked_ptr_cast<String>(right_); }

	AnyPtr each();
};

class StringEachIter : public Base{
	StringStreamPtr ss_;

	virtual void visit_members(Visitor& m);

public:

	StringEachIter(const StringPtr& str);

	void block_next(const VMachinePtr& vm);
};

class ChRangeIter : public Base{
public:

	ChRangeIter(const ChRangePtr& range)
		:it_(range->left()), end_(range->right()){}

	void block_next(const VMachinePtr& vm);
private:

	virtual void visit_members(Visitor& m);

	StringPtr it_, end_;
};


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
		:value(undefined){}

	Named2(const Null&)
		:value(undefined){}
};

/**
* @brief 名前付き引数のトリックのためのクラス
*
*/
struct Named : public Named2{

	Named(const IDPtr& name, const AnyPtr& value)
		:Named2(name, value){}

	Named(){}
};

inline void visit_members(Visitor& m, const Named& p){
	m & p.name & p.value;
}

}

#endif // XTAL_STRING_H_INCLUDE_GUARD
