

#pragma once

#include "xtal_any.h"
#include "xtal_string.h"

namespace xtal{

void InitMap();

class MapImpl;

/**
* @brief 連想配列
*
*/
class Map : public Any{
public:

	/**
	* @brief 空の連想配列を生成する 
	*
	*/
	Map();

	/**
	* @brief 連想配列を生成せず、nullを入れる
	*
	*/
	Map(const Null&)
		:Any(null){}

	explicit Map(MapImpl* p)
		:Any((AnyImpl*)p){}

	explicit Map(const MapImpl* p)
		:Any((AnyImpl*)p){}
		
	/**
	* @brief 連想配列の容量を返す
	*
	*/
	int_t size() const;

	/**
	* @brief 連想配列の容量を返す
	*
	*/
	int_t length() const;

	/**
	* @brief iに対応する要素を返す
	*
	*/
	const Any& at(const Any& i) const;
	
	/**
	* @brief iに対応する要素を設定する
	*
	*/
	void set_at(const Any& i, const Any& v) const;
		
	/**
	* @brief 浅いコピーを返す
	*
	*/
	Map clone() const;
	
	/**
	* @brief この配列の文字列表現を返す
	*
	*/
	String to_s() const;
	
	/**
	* @brief keyとvalueをブロックパラメータとするIteratorを返す
	*
	*/
	Any each_pair() const;

	/**
	* @brief keyをブロックパラメータとするIteratorを返す
	*
	*/
	Any each_key() const;

	/**
	* @brief valueをブロックパラメータとするIteratorを返す
	*
	*/
	Any each_value() const;

	/**
	* @brief keyとvalueをブロックパラメータとするIteratorを返す
	*
	* each_pairと同じ
	*/
	Any each() const{ return each_pair(); }

	/**
	* @brief keyに対応する値を削除する
	*
	*/
	void erase(const Any& key) const;
	
	/**
	* @brief 連結した連想配列を返す
	*
	*/
	Map cat(const Map& a) const;

	/**
	* @brief 連結した連想配列を返す
	*
	*/
	Map cat_assign(const Map& a) const;

	/**
	* @brief 空か調べる
	*
	*/
	bool empty() const;


	MapImpl* impl() const{ return (MapImpl*)Any::impl(); }

	const Any operator [](const Any& key) const{
		return at(key);
	}
};

}//namespace

