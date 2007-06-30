
#pragma once

#include "xtal_any.h"
#include "xtal_string.h"

namespace xtal{

void InitArray();

class ArrayImpl;

/**
* @brief 配列
*
*/
class Array : public Any{
public:
	
	/**
	* @brief sizeの長さの配列を生成する 
	*
	*/
	explicit Array(int_t size = 0);

	explicit Array(check_xtype<int>::type size);
	
	/**
	* @brief 配列を生成せず、nullを入れる
	*
	*/
	Array(const Null&)
		:Any(null){}

	explicit Array(ArrayImpl* p)
		:Any((AnyImpl*)p){}

	explicit Array(const ArrayImpl* p)
		:Any((AnyImpl*)p){}
	
	/**
	* @brief 配列の長さを返す
	*
	*/
	int_t size() const;

	/**
	* @brief 配列の長さを変更する
	*/
	void resize(int_t sz) const;
	
	/**
	* @brief 配列の長さを返す
	*
	*/
	int_t length() const;
	
	/**
	* @brief i番目の要素を返す
	*
	*/
	const Any& at(int_t i) const;
	
	/**
	* @brief i番目の要素を設定する
	*
	*/
	void set_at(int_t i, const Any& v) const;

	/**
	* @brief 先頭に要素を追加する
	*
	*/
	void push_front(const Any& v) const;

	/**
	* @brief 先頭の要素を削除する
	*
	*/
	void pop_front() const;

	/**
	* @brief 末尾に要素を追加する
	*
	*/
	void push_back(const Any& v) const;

	/**
	* @brief 末尾の要素を削除する
	*
	*/
	void pop_back() const;

	/**
	* @brief firstからlastまでの部分配列を返す
	*
	*/
	Array slice(int_t first, int_t last) const;

	/**
	* @brief i番目の要素を削除する
	*
	*/
	void erase(int_t i) const;

	/**
	* @brief i番目に要素を追加する
	*
	*/
	void insert(int_t i, const Any& v) const;

	/**
	* @brief 要素を文字列として連結した結果を返す
	*
	* @param sep 要素と要素の区切り文字列
	*/
	String join(const String& sep) const;

	/**
	* @brief この配列の文字列表現を返す
	*
	*/
	String to_s() const;

	/**
	* @brief 自身を返す
	*
	*/
	Array to_a() const;
	
	/**
	* @brief 浅いコピーを返す
	*
	*/
	Array clone() const;

	/**
	* @brief 連結した配列を返す
	*
	*/
	Array cat(const Array& a) const;

	/**
	* @brief 連結した配列を返す
	*
	*/
	Array cat_assign(const Array& a) const;

	/**
	* @brief 要素を巡回するIteratorを返す
	*
	*/
	Any each() const;
	
	/**
	* @brief 要素を全て削除する
	*
	*/
	void clear() const;

	/**
	* @brief 空か調べる
	*
	*/
	bool empty() const;

	/**
	* @brief 最初の要素を取得する
	*
	*/
	const Any& front() const;

	/**
	* @brief 最後の要素を取得する
	*
	*/
	const Any& back() const;

	/**
	* @brief 配列の要素を全て多値として返す
	*
	*/
	void values(const VMachine& vm) const;

	bool op_eq(const Array& other) const;

	ArrayImpl* impl() const{ return (ArrayImpl*)Any::impl(); }

	const Any& operator [] (int_t i){
		return at(i);
	}

};

template<class T>
class TArray : public Array{
public:

	TArray(int_t size = 0)
		:Array(size){}
	
	TArray(const Null&)
		:Array(null){}

	explicit TArray(ArrayImpl* p)
		:Array((AnyImpl*)p){}

	explicit TArray(const ArrayImpl* p)
		:Array((AnyImpl*)p){}

	const T& at(int_t i) const{
		return (const T&)Array::at(i);
	}
	
	void set_at(int_t i, const T& v) const{
		Array::set_at(i, v);
	}

	void push_front(const T& v) const{
		Array::push_front(v);
	}

	void push_back(const T& v) const{
		Array::push_back(v);
	}

	TArray slice(int_t first, int_t last) const{
		return Array::slice(first, last).impl();
	}

	void insert(int_t i, const T& v) const{
		Array::insert(i, v);
	}

	TArray cat(const TArray<T>& a) const{ 
		Array::cat(a); 
		return *this;
	}

	const T& operator [] (int_t i){ 
		return (const T&)at(i); 
	}
};

}//namespace

