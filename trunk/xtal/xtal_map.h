
#pragma once

#include "xtal_any.h"
#include "xtal_string.h"

namespace xtal{

class Map : public Base{
public:

	/**
	* @brief 空の連想配列を生成する 
	*
	*/
	Map();

	virtual ~Map();
		
	/**
	* @brief iに対応する要素を返す
	*
	*/
	const AnyPtr& at(const AnyPtr& key);
	
	/**
	* @brief iに対応する要素を設定する
	*
	*/	
	void set_at(const AnyPtr& key, const AnyPtr& value);

	/**
	* @brief keyに対応する値を削除する
	*
	*/
	void erase(const AnyPtr& key);

	/**
	* @brief 連想配列の容量を返す
	*
	*/
	int_t size(){
		return used_size_;
	}
	
	/**
	* @brief 連想配列の容量を返す
	*
	*/
	int_t length(){
		return used_size_;
	}

	/**
	* @brief 空か調べる
	*
	*/
	bool empty(){
		return used_size_==0;
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
	* @brief この配列の文字列表現を返す
	*
	*/
	StringPtr to_s();

	/**
	* @brief == の再定義
	*/
	bool op_eq(const MapPtr& other);
		
	/**
	* @brief keyとvalueをブロックパラメータとするIteratorを返す
	*
	*/
	AnyPtr each_pair();
	
	/**
	* @brief keyをブロックパラメータとするIteratorを返す
	*
	*/
	AnyPtr each_key();
	
	/**
	* @brief valueをブロックパラメータとするIteratorを返す
	*
	*/
	AnyPtr each_value();

	/**
	* @brief keyとvalueをブロックパラメータとするIteratorを返す
	*
	* each_pairと同じ
	*/
	AnyPtr each(){
		return each_pair();
	}
	
	/**
	* @brief 浅いコピーを返す
	*
	*/
	MapPtr clone();
	
	void push_all(const VMachinePtr& vm);

protected:

	struct Node{
		AnyPtr key;
		AnyPtr value;
		Node* next;
		
		Node()
			:next(0){}

		Node(const AnyPtr& key, const AnyPtr& value)
			:key(key), value(value), next(0){}
	};

	friend class MapIter;
	friend class MapKeyIter;
	friend class MapValueIter;

	float_t rate(){
		return used_size_/(float_t)size_;
	}
	
	void set_node(Node* node){
		Node** p = &begin_[node->key->hashcode() % size_];
		while(*p){
			p = &(*p)->next;
		}
		*p = node;
	}

	void expand(int_t addsize){
		Node** oldbegin = begin_;
		uint_t oldsize = size_;

		size_ = size_ + size_/2 + addsize;
		begin_ = (Node**)user_malloc(sizeof(Node*)*size_);
		for(uint_t i = 0; i<size_; ++i){
			begin_[i] = 0;
		}

		for(uint_t i = 0; i<oldsize; ++i){
			Node* p = oldbegin[i];
			while(p){
				Node* next = p->next;
				set_node(p);
				p->next = 0;
				p = next;
			}
		}
		user_free(oldbegin);
	}
	
protected:

	Node** begin_;
	uint_t size_;
	uint_t used_size_;

	virtual void visit_members(Visitor& m);
};

class StrictMap{
public:

	struct Node{
		AnyPtr key;
		AnyPtr value;
		Node* next;
		
		Node()
			:next(0){}

		Node(const AnyPtr& key, const AnyPtr& value)
			:key(key), value(value), next(0){}
	};


	StrictMap();

	~StrictMap();
		
	const AnyPtr& at(const AnyPtr& key);
	
	void set_at(const AnyPtr& key, const AnyPtr& value);

	int_t size(){
		return used_size_;
	}
	
	bool empty(){
		return used_size_==0;
	}

	void destroy();

	void clear(){
		destroy();
		expand(7);
	}
	
private:

	float_t rate(){
		return used_size_/(float_t)size_;
	}
	
	void set_node(Node* node);

	void expand(int_t addsize);
		
private:

	Node** begin_;
	uint_t size_;
	uint_t used_size_;

private:
	
	StrictMap(const StrictMap&);
	StrictMap& operator = (const StrictMap&);
};

}//namespace

