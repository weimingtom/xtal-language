
#pragma once

namespace xtal{

class Map : public Base{
public:

	Map()
		:default_value_(undefined){}
		
	/**
	* @brief iに対応する要素を返す
	*
	*/
	const AnyPtr& at(const AnyPtr& key);
	
	/**
	* @brief iに対応する要素を設定する
	*
	*/	
	void set_at(const AnyPtr& key, const AnyPtr& value){
		insert(key, value);
	}

	/**
	* @brief iに対応する要素を設定する
	*
	*/	
	void insert(const AnyPtr& akey, const AnyPtr& value){
		const AnyPtr& key = calc_key(akey);
		table_.insert(key, value);
	}

	/**
	* @brief keyに対応する値を削除する
	*
	*/
	void erase(const AnyPtr& akey){
		const AnyPtr& key = calc_key(akey);
		table_.erase(key);
	}

	/**
	* @brief 連想配列の容量を返す
	*
	*/
	int_t size(){
		return table_.size();
	}
	
	/**
	* @brief 連想配列の容量を返す
	*
	*/
	int_t length(){
		return table_.size();
	}

	/**
	* @brief 空か調べる
	*
	*/
	bool empty(){
		return table_.empty();
	}

	/**
	* @brief 連結した連想配列を返す
	*
	*/
	MapPtr cat(const MapPtr& a);

	/**
	* @brief 自身と連結し、自身を返す
	*
	*/
	MapPtr cat_assign(const MapPtr& a);

	/**
	* @brief == の再定義
	*/
	bool op_eq(const MapPtr& other);
		
	/**
	* @brief [key, value]を要素とする、Iteratorを返す
	*
	*/
	AnyPtr pairs();
	
	/**
	* @brief keyを要素とする、Iteratorを返す
	*
	*/
	AnyPtr keys();
	
	/**
	* @brief valueを要素とする、Iteratorを返す
	*
	*/
	AnyPtr values();

	/**
	* @brief [key, value]を要素とする、Iteratorを返す
	*
	* pairsと同じ
	*/
	AnyPtr each(){
		return pairs();
	}
	
	/**
	* @brief 浅いコピーを返す
	*
	*/
	MapPtr clone();

	/**
	* @brief 要素を全て削除する
	*
	*/
	void clear(){
		table_.clear();
	}
	
	MapPtr to_m(){
		return from_this(this);
	}

	/**
	* @brief 中身をiteratorで取得できる要素に置き換える
	*/
	void assign(const AnyPtr& iterator);

	void concat(const AnyPtr& iterator);

	void push_all(const VMachinePtr& vm);

	const AnyPtr& default_value(){
		return default_value_;
	}

	void set_default_value(const AnyPtr& default_value){
		default_value_ = default_value;
	}

protected:

	struct Fun{
		static uint_t hash(const AnyPtr& key){
			return (rawvalue(key) ^ type(key) ^ (rawvalue(key)>>3));
		}

		static bool eq(const AnyPtr& a, const AnyPtr& b){
			return raweq(a, b);
		}
	};

	typedef Hashtable<AnyPtr, AnyPtr, Fun> table_t;

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
	* @brief valueが真なkeyを要素とするIteratorを返す
	*
	*/
	AnyPtr each();
};

}//namespace

