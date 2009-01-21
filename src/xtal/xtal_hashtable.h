
#pragma once

namespace xtal{

template<class Key, class Val, class Fun>
class Hashtable{
public:

	typedef std::pair<Key, Val> pair_t;

	struct Node{
		pair_t pair;
		Node* next;
		Node* ordered_next;
		Node* ordered_prev;
		
		Node()
			:next(0), ordered_next(0), ordered_prev(0){}

		Node(const Key& key, const Val& value)
			:pair(key, value), next(0), ordered_next(0), ordered_prev(0){}
	};

	typedef Key key_type;
	typedef Val value_type;
	typedef Fun fun_type;

public:

	class iterator{
	public:
		
		iterator(Node* node = 0)
			:node_(node){}
		
		pair_t& operator *() const{
			return node_->pair;
		}

		pair_t* operator ->() const{
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

		Node* to_node() const{
			return node_;
		}

	private:
		Node* node_;
	};

	typedef iterator const_iterator;

	iterator begin(){
		return iterator(ordered_head_);
	}

	iterator end(){
		return iterator(0);
	}

	const_iterator begin() const{
		return iterator(ordered_head_);
	}

	const_iterator end() const{
		return iterator(0);
	}

public:

	/**
	* @brief 空のハッシュテーブルを生成する 
	*
	*/
	Hashtable();

	Hashtable(const Hashtable<Key, Val, Fun>& v);

	Hashtable<Key, Val, Fun>& operator=(const Hashtable<Key, Val, Fun>& v);

	~Hashtable();
		
	/**
	* @brief iに対応する要素を返す
	*
	*/
	iterator find(const Key& key){
		return find(key, Fun::hash(key));
	}

	/**
	* @brief iに対応する要素を返す
	*
	*/
	iterator find(const Key& key, uint_t hash);

	/**
	* @brief iに対応する要素を設定する
	*
	*/	
	std::pair<iterator, bool> insert(const Key& key, const Val& value){
		return insert(key, value, Fun::hash(key));
	}

	/**
	* @brief iに対応する要素を設定する
	*
	*/	
	std::pair<iterator, bool> insert(const Key& key, const Val& value, uint_t hash);

	/**
	* @brief keyに対応する値を削除する
	*
	*/
	void erase(const Key& key);

	void erase(iterator it){
		erase(it->first);
	}

	/**
	* @brief 連想配列の容量を返す
	*
	*/
	uint_t size(){
		return used_size_;
	}

	/**
	* @brief 空か調べる
	*
	*/
	bool empty(){
		return used_size_==0;
	}

	void clear();


protected:

	float_t rate(){
		return used_size_/(float_t)size_;
	}
	
	void expand(int_t addsize);

protected:

	Node** begin_;
	Node* ordered_head_;
	Node* ordered_tail_;
	uint_t size_;
	uint_t used_size_;
};

template<class Key, class Val, class Fun>
Hashtable<Key, Val, Fun>::Hashtable(){
	size_ = 0;
	begin_ = 0;
	used_size_ = 0;
	ordered_head_ = 0;
	ordered_tail_ = 0;
	expand(4);
}

template<class Key, class Val, class Fun>
Hashtable<Key, Val, Fun>::Hashtable(const Hashtable<Key, Val, Fun>& v){
	size_ = 0;
	begin_ = 0;
	used_size_ = 0;
	ordered_head_ = 0;
	ordered_tail_ = 0;
	expand(4);

	for(const_iterator it=v.begin(); it!=v.end(); ++it){
		insert(it->first, it->second);
	}
}

template<class Key, class Val, class Fun>
Hashtable<Key, Val, Fun>& Hashtable<Key, Val, Fun>::operator=(const Hashtable<Key, Val, Fun>& v){
	if(this==&v){ return *this; }
	clear();
	for(const_iterator it=v.begin(); it!=v.end(); ++it){
		insert(it->first, it->second);
	}
	return *this;
}

template<class Key, class Val, class Fun>
Hashtable<Key, Val, Fun>::~Hashtable(){
	clear();
	so_free(begin_, sizeof(Node*)*size_);
}

template<class Key, class Val, class Fun>
typename Hashtable<Key, Val, Fun>::iterator Hashtable<Key, Val, Fun>::find(const Key& key, uint_t hash){
	Node* p = begin_[hash & (size_-1)];
	while(p){
		if(Fun::eq(p->pair.first, key)){
			return iterator(p);
		}
		p = p->next;
	}
	return end();
}

template<class Key, class Val, class Fun>
std::pair<typename Hashtable<Key, Val, Fun>::iterator, bool> Hashtable<Key, Val, Fun>::insert(const Key& key, const Val& value, uint_t hash){
	Node** p = &begin_[hash & (size_-1)];
	while(*p){
		if(Fun::eq((*p)->pair.first, key)){
			(*p)->pair.second = value;
			return std::pair<iterator, bool>(*p, false);
		}
		p = &(*p)->next;
	}

	*p = (Node*)so_malloc(sizeof(Node));
	Node* ret = *p;
	new(ret) Node(key, value);

	if(ordered_tail_){
		ordered_tail_->ordered_next = ret;
		(ret)->ordered_prev = ordered_tail_;
		ordered_tail_ = ret;
	}
	else{
		ordered_head_ = ret;
		ordered_tail_ = ret;
	}

	used_size_++;
	if(rate()>0.8f){
		expand(0);
	}

	return std::pair<iterator, bool>(ret, true);
}

template<class Key, class Val, class Fun>
void Hashtable<Key, Val, Fun>::erase(const Key& key){
	uint_t hash = Fun::hash(key);
	uint_t pos = hash  & (size_-1);
	Node* p = begin_[pos];
	Node* prev = 0;
	while(p){
		if(Fun::eq(p->pair.first, key)){
			if(prev){
				prev->next = p->next;
			}
			else{
				begin_[pos] = p->next;
			}

			if(p==ordered_head_){
				ordered_head_ = p->ordered_next;
			}

			if(p==ordered_tail_){
				ordered_tail_ = p->ordered_prev;
			}

			if(p->ordered_next) p->ordered_next->ordered_prev = p->ordered_prev;
			if(p->ordered_prev) p->ordered_prev->ordered_next = p->ordered_next;

			p->~Node();
			so_free(p, sizeof(Node));

			used_size_--;
			break;
		}
		prev = p;
		p = p->next;
	}
}

template<class Key, class Val, class Fun>
void Hashtable<Key, Val, Fun>::clear(){
	for(Node* p = ordered_head_; p;){
		Node* next = p->ordered_next;
		p->~Node();
		so_free(p, sizeof(Node));
		p = next;
	}

	for(uint_t i = 0; i<size_; ++i){
		begin_[i] = 0;
	}

	used_size_ = 0;
	ordered_head_ = 0;
	ordered_tail_ = 0;
}

template<class Key, class Val, class Fun>
void Hashtable<Key, Val, Fun>::expand(int_t addsize){
	Node** oldbegin = begin_;
	uint_t oldsize = size_;

	size_ = size_*2 + addsize;
	begin_ = (Node**)so_malloc(sizeof(Node*)*size_);
	
	for(uint_t i = 0; i<size_; ++i){
		begin_[i] = 0;
	}

	for(Node* p = ordered_head_; p; p=p->ordered_next){
		p->next = 0;
	}

	for(Node* p = ordered_head_; p; p=p->ordered_next){
		Node** temp = &begin_[Fun::hash(p->pair.first) & (size_-1)];
		while(*temp){
			temp = &(*temp)->next;
		}
		*temp = p;
	}

	so_free(oldbegin, sizeof(Node*)*oldsize);
}

template<class Key, class Val, class Fun>
void visit_members(Visitor& m, const Hashtable<Key, Val, Fun>& values){
	for(typename Hashtable<Key, Val, Fun>::const_iterator it = values.begin(); it!=values.end(); ++it){
		m & it->first & it->second;
	}
}

}
