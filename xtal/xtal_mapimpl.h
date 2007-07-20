
#pragma once

#include "xtal.h"

#include "xtal_map.h"
#include "xtal_utilimpl.h"
#include "xtal_macro.h"

namespace xtal{

class MapImpl : public AnyImpl{
public:

	struct Node{
		Any key;
		Any value;
		Node* next;
		
		Node()
			:next(0){}

		Node(const Any& key, const Any& value)
			:key(key), value(value), next(0){}
	};


	MapImpl(){
		set_class(TClass<Map>::get());
		size_ = 0;
		begin_ = 0;
		used_size_ = 0;
		expand(7);
	}

	virtual ~MapImpl(){
		for(uint_t i = 0; i<size_; ++i){
			Node* p = begin_[i];
			while(p){
				Node* next = p->next;
				p->~Node();
				user_free(p, sizeof(Node));
				p = next;
			}
		}
		user_free(begin_, sizeof(Node*)*size_);
	}
		
	const Any& at(const Any& key){
		Node* p = begin_[key.hashcode() % size_];
		while(p){
			if(p->key==key){
				return p->value;
			}
			p = p->next;
		}
		return null;
	}
	
	void set_at(const Any& key, const Any& value){
		Node** p = &begin_[key.hashcode() % size_];
		while(*p){
			if((*p)->key==key){
				(*p)->value = value;
				return;
			}
			p = &(*p)->next;
		}
		*p = (Node*)user_malloc(sizeof(Node));
		new(*p) Node(key, value);
		used_size_++;
		if(rate()>0.8f){
			expand(17);
		}
	}

	void erase(const Any& key){
		Node* p = begin_[key.hashcode() % size_];
		Node* prev = 0;
		while(p){
			if(p->key==key){
				if(prev){
					prev->next = p->next;
				}else{
					begin_[key.hashcode() % size_] = p->next;
				}
				p->~Node();
				user_free(p, sizeof(Node));
				return;
			}
			prev = p;
			p = p->next;
		}
	}

	int_t size(){
		return used_size_;
	}
	
	bool empty(){
		return used_size_==0;
	}

	Map cat(const Map& a){
		Map ret(clone());
		ret.cat_assign(a);
		return ret;
	}

	Map cat_assign(const Map& a){
		Xfor2(k, v, a.each_pair()){
			set_at(k, v);
		}		
		return Map(this);
	}

	String to_s(){
		if(empty())
			return String("[:]");
		String ret("[");
		bool first = true;
		for(uint_t i = 0; i<size_; ++i){
			Node* p = begin_[i];
			while(p){
				Node* next = p->next;
				if(!first){
					ret = ret.cat(",");
				}else{
					first = false;
				}
				ret = ret.cat(p->key.to_s());
				ret = ret.cat(":");
				ret = ret.cat(p->value.to_s());
				p = next;
			}
		}
		ret = ret.cat("]");
		return ret;
	}

	bool op_eq(const Map& other){
		if(size()!=other.size())
			return false;

		Xfor2(key, value, other){
			if(at(key)!=other.at(key)){
				return false;
			}
		}
		return true;
	}
	
	friend class MapIterImpl;
	friend class MapKeyIterImpl;
	friend class MapValueIterImpl;

	class MapIterImpl : public AnyImpl{
		Any map_;
		uint_t pos_;
		Node* node_;

		virtual void visit_members(Visitor& m){
			AnyImpl::visit_members(m);
			m & map_;
		}

	public:

		MapIterImpl(MapImpl* m)
			:map_(m), pos_(0), node_(0){
			set_class(TClass<MapIterImpl>::get());
			restart();
		}

		Any restart(){
			MapImpl* impl = (MapImpl*)map_.impl();
			pos_ = 0;
			node_ = impl->begin_[pos_];
			return this;
		}
		
		void iter_next(const VMachine& vm){
			MapImpl* impl = (MapImpl*)map_.impl();
			while(!node_){
				if(pos_!= impl->size_-1){
					pos_++;
					node_ = impl->begin_[pos_];
				}else{
					restart();
					vm.return_result(null, null, null);
					return;
				}
			}
			
			vm.return_result(this, node_->key, node_->value);
			node_ = node_->next;
		}
	};

	class MapKeyIterImpl : public AnyImpl{
		Any map_;
		uint_t pos_;
		Node* node_;

		virtual void visit_members(Visitor& m){
			AnyImpl::visit_members(m);
			m & map_;
		}

	public:

		MapKeyIterImpl(MapImpl* m)
			:map_(m), pos_(0), node_(0){
			set_class(TClass<MapKeyIterImpl>::get());
			restart();
		}

		Any restart(){
			MapImpl* impl = (MapImpl*)map_.impl();
			pos_ = 0;
			node_ = impl->begin_[pos_];
			return this;
		}

		void iter_next(const VMachine& vm){
			MapImpl* impl = (MapImpl*)map_.impl();
			while(!node_){
				if(pos_!= impl->size_-1){
					pos_++;
					node_ = impl->begin_[pos_];
				}else{
					restart();
					vm.return_result(null, null);
					return;
				}
			}
			
			vm.return_result(this, node_->key);
			node_ = node_->next;
		}
	};

	class MapValueIterImpl : public AnyImpl{
		Any map_;
		uint_t pos_;
		Node* node_;

		virtual void visit_members(Visitor& m){
			AnyImpl::visit_members(m);
			m & map_;
		}

	public:

		MapValueIterImpl(MapImpl* m)
			:map_(m), pos_(0), node_(0){
			set_class(TClass<MapValueIterImpl>::get());
			restart();
		}
		
		Any restart(){
			MapImpl* impl = (MapImpl*)map_.impl();
			pos_ = 0;
			node_ = impl->begin_[pos_];
			return this;
		}

		void iter_next(const VMachine& vm){
			MapImpl* impl = (MapImpl*)map_.impl();
			while(!node_){
				if(pos_!= impl->size_-1){
					pos_++;
					node_ = impl->begin_[pos_];
				}else{
					restart();
					vm.return_result(null, null);
					return;
				}
			}
			
			vm.return_result(this, node_->value);
			node_ = node_->next;
		}
	};
	
	Any each_pair(){
		Any ret;
		new(ret) MapIterImpl(this);
		return ret;
	}
	
	Any each_key(){
		Any ret;
		new(ret) MapKeyIterImpl(this);
		return ret;
	}
	
	Any each_value(){
		Any ret;
		new(ret) MapValueIterImpl(this);
		return ret;
	}
	
	Map clone(){
		Map ret;
		Xfor2(k, v, each_pair()){
			ret.set_at(k, v);
		}
		return ret;
	}
	
	void push_all(const VMachine& vm){
		for(uint_t i = 0; i<size_; ++i){
			Node* p = begin_[i];
			while(p){
				Node* next = p->next;
				vm.push(p->key);
				vm.push(p->value);
				p = next;
			}
		}		
	}
	
protected:

	float_t rate(){
		return used_size_/(float_t)size_;
	}
	
	void set_node(Node* node){
		Node** p = &begin_[node->key.hashcode() % size_];
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
		user_free(oldbegin, sizeof(Node*)*oldsize);
	}
	
protected:

	Node** begin_;
	uint_t size_;
	uint_t used_size_;

	virtual void visit_members(Visitor& m){
		AnyImpl::visit_members(m);
		for(uint_t i = 0; i<size_; ++i){
			Node* p = begin_[i];
			while(p){
				Node* next = p->next;
				m & p->key & p->value;
				p = next;
			}
		}		
	}	
};

class StrictMap{
public:

	struct Node{
		Any key;
		Any value;
		Node* next;
		
		Node()
			:next(0){}

		Node(const Any& key, const Any& value)
			:key(key), value(value), next(0){}
	};


	StrictMap();

	~StrictMap();
		
	const Any& at(const Any& key);
	
	void set_at(const Any& key, const Any& value);

	int_t size(){
		return used_size_;
	}
	
	bool empty(){
		return used_size_==0;
	}

	void destroy();
	
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


}
