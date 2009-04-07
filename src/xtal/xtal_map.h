/** \file src/xtal/xtal_map.h
* \brief src/xtal/xtal_map.h
*/

#ifndef XTAL_MAP_H_INCLUDE_GUARD
#define XTAL_MAP_H_INCLUDE_GUARD

#pragma once

namespace xtal{

/**
* \xbind
* \brief 連想配列
*/
class Map : public Base{
public:

	/**
	* \xbind
	* \brief 空の連想配列オブジェクトを生成
	*/
	Map()
		:default_value_(undefined){}
		
	/**
	* \brief keyに対応する要素を返す
	*
	* \param key 検索のためのキー 
	*/
	const AnyPtr& at(const AnyPtr& key);
	
	/**
	* \brief keyに対応する要素を設定する
	*
	* \param key 検索のためのキー 
	* \param value keyに対応する値
	*/	
	void set_at(const AnyPtr& key, const AnyPtr& value){
		insert(key, value);
	}

	/**
	* \xbind
	* \brief keyに対応する要素を設定する
	*
	* \param key 検索のためのキー 
	* \param value keyに対応する値
	*/	
	void insert(const AnyPtr& key, const AnyPtr& value){
		const AnyPtr& akey = calc_key(key);
		table_.insert(akey, value);
	}

	/**
	* \xbind
	* \brief keyに対応する値を削除する
	*
	* \param key 検索のためのキー 
	*/
	void erase(const AnyPtr& key){
		const AnyPtr& akey = calc_key(key);
		table_.erase(akey);
	}

	/**
	* \xbind
	* \brief 連想配列に登録された要素の数を返す
	*
	* \return 連想配列に登録された要素の数
	*/
	uint_t size(){
		return table_.size();
	}
	
	/**
	* \xbind
	* \brief 連想配列に登録された要素の数を返す
	*
	* \return 連想配列に登録された要素の数
	*/
	uint_t length(){
		return table_.size();
	}

	/**
	* \xbind
	* \brief 空か調べる
	*
	* \retval true 空
	* \retval false 非空
	*/
	bool empty(){
		return table_.empty();
	}

	/**
	* \xbind
	* \brief 空か調べる
	*
	* \retval true 空
	* \retval false 非空
	*/
	bool is_empty(){
		return table_.empty();
	}

	/**
	* \brief 連結した連想配列を返す
	*
	*/
	MapPtr cat(const MapPtr& a);

	/**
	* \brief 自身と連結し、自身を返す
	*
	*/
	MapPtr cat_assign(const MapPtr& a);
		
	/**
	* \xbind
	* \brief (key, value)を要素とする、Iteratorを返す
	*
	*/
	AnyPtr pairs();
	
	/**
	* \xbind
	* \brief keyを要素とする、Iteratorを返す
	*
	*/
	AnyPtr keys();
	
	/**
	* \xbind
	* \brief valueを要素とする、Iteratorを返す
	*
	*/
	AnyPtr values();

	/**
	* \xbind
	* \brief (key, value)を要素とする、Iteratorを返す
	*
	* pairsと同じ
	*/
	AnyPtr each(){
		return pairs();
	}
	
	/**
	* \xbind
	* \brief 浅いコピーを返す
	*
	*/
	MapPtr clone();

	/**
	* \xbind
	* \brief 要素を全て削除する
	*
	*/
	void clear(){
		table_.clear();
	}
	
	MapPtr to_m(){
		return to_smartptr(this);
	}

	/**
	* \xbind
	* \brief 中身をiteratorで取得できる要素に置き換える
	*/
	void assign(const AnyPtr& iterator);

	void concat(const AnyPtr& iterator);

	void push_all(const VMachinePtr& vm);

	/**
	* \xbind
	* \brief デフォルト値として設定されているオブジェクトを取り出す
	*
	* \return デフォルト値
	*/
	const AnyPtr& default_value(){
		return default_value_;
	}

	/**
	* \xbind
	* \brief デフォルト値を設定する
	*
	* \param default_value デフォルト値
	*/
	void set_default_value(const AnyPtr& default_value){
		default_value_ = default_value;
	}

protected:

	struct Fun{
		static uint_t hash(const AnyPtr& key){
			return (rawvalue(key).uvalue ^ type(key) ^ (rawvalue(key).uvalue>>3));
		}

		static bool eq(const AnyPtr& a, const AnyPtr& b){
			return raweq(a, b);
		}
	};

	typedef OrderedHashtable<AnyPtr, AnyPtr, Fun> table_t;

	const AnyPtr& calc_key(const AnyPtr& key);

public:

	typedef table_t::iterator iterator;

	iterator begin(){
		return table_.begin();
	}

	iterator end(){
		return table_.end();
	}
	
protected:

	table_t table_;
	AnyPtr default_value_;

	virtual void visit_members(Visitor& m);
};

class MapIter : public Base{
public:

	MapIter(const MapPtr& m, int_t type);
	
	void block_next(const VMachinePtr& vm);

public:

	bool block_next_direct(AnyPtr& rkey, AnyPtr& rval);

private:
	MapPtr map_;
	Map::iterator node_;
	int_t type_;

	virtual void visit_members(Visitor& m);
};

class Set : public Map{
public:

	/**
	* \brief valueが真なkeyを要素とするIteratorを返す
	*
	*/
	AnyPtr each();
};

}//namespace


#endif // XTAL_MAP_H_INCLUDE_GUARD
