
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
	* @brief [key, value]を要素とする、PseudoArrayを返す
	*
	*/
	AnyPtr pairs();
	
	/**
	* @brief keyを要素とする、PseudoArrayを返す
	*
	*/
	AnyPtr keys();
	
	/**
	* @brief valueを要素とする、PseudoArrayを返す
	*
	*/
	AnyPtr values();

	/**
	* @brief [key, value]を要素とする、PseudoArrayを返す
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

	void clear();
	
	void push_all(const VMachinePtr& vm);

protected:

	struct Node{
		std::pair<AnyPtr, AnyPtr> pair;
		Node* next;
		Node* ordered_next;
		Node* ordered_prev;
		
		Node()
			:next(0), ordered_next(0), ordered_prev(0){}

		Node(const AnyPtr& key, const AnyPtr& value)
			:pair(key, value), next(0), ordered_next(0), ordered_prev(0){}
	};

public:

	class iterator{
	public:
		
		iterator(Node* node = 0)
			:node_(node){}
		
		std::pair<AnyPtr, AnyPtr>& operator *() const{
			return node_->pair;
		}

		std::pair<AnyPtr, AnyPtr>* operator ->() const{
			return &node_->pair;
		}

		iterator& operator ++(){
			node_ = node_->ordered_next;
			return *this;
		}

		iterator operator ++(int){
			iterator temp(*this);
			node_ = node_->ordered_next;
			return *this;
		}

		friend bool operator ==(iterator a, iterator b){
			return a.node_ == b.node_;
		}

		friend bool operator !=(iterator a, iterator b){
			return a.node_ != b.node_;
		}

	private:
		Node* node_;
	};

	iterator begin(){
		return iterator(ordered_head_);
	}

	iterator end(){
		return iterator(0);
	}

protected:

	friend class MapIter;
	friend class MapKeyIter;
	friend class MapValueIter;

	float_t rate(){
		return used_size_/(float_t)size_;
	}
	
	void expand(int_t addsize);

	const AnyPtr& calc_key(const AnyPtr& key);
	
	uint_t calc_offset(const AnyPtr& key){
		return ((rawvalue(key) ^ type(key) ^ (rawvalue(key)>>3))) & (size_-1);
	}

protected:

	Node** begin_;
	Node* ordered_head_;
	Node* ordered_tail_;
	uint_t size_;
	uint_t used_size_;

	virtual void visit_members(Visitor& m);
};

}//namespace

