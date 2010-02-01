/** \file src/xtal/xtal_string.h
* \brief src/xtal/xtal_string.h
*/

#ifndef XTAL_STRING_H_INCLUDE_GUARD
#define XTAL_STRING_H_INCLUDE_GUARD

#pragma once

namespace xtal{

class StringData;

/**
* \xbind lib::builtin
* \xinherit lib::builtin::Any lib::builtin::Iterable
* \brief 文字列のクラス
*/
class String : public Any{
public:

	/**
	* \xbind
	* \brief 空文字列を構築する
	*/
	String();

	/**
	* \brief NUL終端のC文字列から構築する
	* \param str NUL終端文字列
	*/
	String(const char_t* str);

	/**
	* \brief NUL終端のC文字列から構築する
	* \param str NUL終端文字列
	*/
	String(const char8_t* str);

	/**
	* \brief C文字列からsize分の長さを取り出し構築する
	*/
	String(const char_t* str, uint_t size);

	/**
	* \brief C文字列リテラルから構築する
	*/
	String(const StringLiteral& str);

	/**
	* \brief beginからlastまでの文字列で構築する
	* [begin, last)
	*/
	String(const char_t* begin, const char_t* last);

	/**
	* \brief 二つのC文字列から構築する
	*/
	String(const char_t* str1, uint_t size1, const char_t* str2, uint_t size2);

public:

	struct intern_t{};
	String(const char_t* str, uint_t size, intern_t);
	String(const StringLiteral& str, intern_t);
	
	String& operator= (const String& s);

	String(const String& s);

protected:

	String(noinit_t){}

public:

	/**
	* \brief 0終端の文字列先頭のポインタを返す。
	* data()とdata_size()で代用可能ならそちらを使ってください。
	*/
	const char_t* c_str() const;

	/**
	* \brief 文字列先頭のポインタを返す。
	* これは0終端文字列が返されるとは限らない。
	*/
	const char_t* data() const;

	/**
	* \xbind
	* \brief データサイズを返す。
	* マルチバイト文字を考慮しない。
	*/
	uint_t data_size() const;

	/**
	* \xbind
	* \brief 文字列の長さを返す。
	* マルチバイト文字を考慮する。
	*/
	uint_t length() const;

	/*
	* \brief 浅いコピーを返す。
	*/
	StringPtr clone() const;

	/**
	* \xbind
	* \brief 一意化した文字列を返す。
	*/
	const IDPtr& intern() const;

	/**
	* \xbind
	* \brief 一意化されているか返す。
	*/
	bool is_interned() const;

	/**
	* \xbind
	* \brief 整数に変換した結果を返す。
	*/ 
	int_t to_i() const;
	
	/**
	* \xbind
	* \brief 浮動小数点数に変換した結果を返す。
	*/ 
	float_t to_f() const;
	
	/**
	* \xbind
	* \brief 自分自身を返す。
	*/
	StringPtr to_s() const;

	/**
	* \xbind
	* \brief 一文字づつの文字列を要素とするIteratorを返す。
	* \return 一文字づつの文字列を要素とするIterator
	*/
	AnyPtr each() const;

	/**
	* \xbind
	* \brief 連結する
	* \return 連結された新しいオブジェクト
	*/
	StringPtr cat(const StringPtr& v) const;

public:

	/**
	* \xbind
	* \brief 一文字の文字列かどうか
	*/
	bool is_ch() const;

	/**
	* \xbind
	* \brief 空の文字列かどうか
	*/
	bool is_empty() const;
	bool empty() const{ return is_empty(); }

	/**
	* \xbind
	* \brief 一文字の文字列の場合、そのasciiコードを返す
	*/
	int_t ascii() const;

	/**
	* \xbind
	* \brief 文字の範囲オブジェクトの範囲内かどうか
	*/
	bool op_in(const ChRangePtr& range) const;

public:

	/**
	* \xbind
	* \brief 範囲オブジェクトを生成する
	*/
	ChRangePtr op_range(const StringPtr& right, int_t kind) const;
	
	/**
	* \xbind
	* \brief 連結する
	*/
	StringPtr op_cat(const StringPtr& v) const;
	
	/**
	* \xbind
	* \brief 値が等しいか比べる
	*/
	bool op_eq(const StringPtr& v) const;

	/**
	* \xbind
	* \brief より小さいか比べる
	*/
	bool op_lt(const StringPtr& v) const;

public:

	/**
	* \xbind
	* \brief 文字列をスキャンする
	*/
	AnyPtr scan(const AnyPtr& pattern) const;

	/**
	* \xbind
	* \brief 文字列を分割し、その部分文字列を要素とするIteratorを返す
	*/
	AnyPtr split(const AnyPtr& pattern) const;

	/**
	* \xbind
	* \brief 文字列を分割し、その部分文字列を要素とするIteratorを返す
	*/	
	bool match(const AnyPtr& pattern) const;

	/**
	* \xbind
	* \brief マッチする部分を置き換えた新しい文字列を返す
	*/	
	StringPtr gsub(const AnyPtr& pattern, const AnyPtr& fn) const;

	/**
	* \xbind
	* \brief 最初にマッチする部分を置き換えた新しい文字列を返す
	*/	
	StringPtr sub(const AnyPtr& pattern, const AnyPtr& fn) const;

private:
	void init_string(const char_t* str, uint_t size);
	StringData* new_string_data(uint_t size);

public:

	class iterator{
	public:
		
		iterator(const char_t* p)
			:p_(p){}
		
		char_t operator *() const{
			return *p_;
		}

		iterator& operator ++(){
			++p_;
			return *this;
		}

		iterator operator ++(int){
			iterator temp(*this);
			++p_;
			return temp;
		}

		friend bool operator ==(iterator a, iterator b){
			return a.p_ == b.p_;
		}

		friend bool operator !=(iterator a, iterator b){
			return a.p_ != b.p_;
		}

	private:
		const char_t* p_;
	};

	iterator begin() const;

	iterator end() const;
};

class StringData : public RefCountingBase{
	enum{
		INTERNED = 1<<0,
		SIZE_SHIFT = 1
	};

	char_t* buf_;
	uint_t data_size_;
public:

	enum{
		TYPE = TYPE_STRING
	};

	StringData(uint_t size);

	~StringData();

	uint_t data_size(){
		return data_size_>>SIZE_SHIFT;
	}

	char_t* buf(){ 
		return buf_; 
	}

	bool is_interned(){
		return (data_size_&INTERNED)!=0;
	}

	void set_interned(){
		data_size_ |= INTERNED;
	}

private:
	
	XTAL_DISALLOW_COPY_AND_ASSIGN(StringData);
};


/**
* \brief Intern済みのString
*
* これはC++の型システムのために存在する。
* XtalにおいてIntern済みStringはString型である。
*/
class ID : public String{
public:

	/**
	* \brief NUL終端のC文字列から構築する
	*
	* \param str NULL終端文字列
	*/
	ID(const char_t* str);

	/**
	* \brief NUL終端のC文字列から構築する
	*
	* \param str NULL終端文字列
	*/
	ID(const char8_t* str);

	/**
	* \brief C文字列からsize分の長さを取り出し構築する
	*
	*/
	ID(const char_t* str, uint_t size);
	
	/**
	* \brief C文字列リテラルから構築する
	*
	*/
	ID(const StringLiteral& str);

	/**
	* \brief beginからlastまでの文字列で構築する
	*
	* [begin, last)
	*/
	ID(const char_t* begin, const char_t* last);

	/**
	* \brief Stringから構築する
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
		:Range(left, right, RANGE_CLOSED){}

public:

	const StringPtr& left(){ return unchecked_ptr_cast<String>(left_); }

	const StringPtr& right(){ return unchecked_ptr_cast<String>(right_); }

	AnyPtr each();
};

class StringEachIter : public Base{
public:

	StringEachIter(const StringPtr& str);

	void block_next(const VMachinePtr& vm);

	void on_visit_members(Visitor& m);


private:
	StringStreamPtr ss_;
};

class ChRangeIter : public Base{
public:

	ChRangeIter(const ChRangePtr& range)
		:it_(range->left()), end_(range->right()){}

	void block_next(const VMachinePtr& vm);

private:
	StringPtr it_, end_;
};

}

#endif // XTAL_STRING_H_INCLUDE_GUARD
