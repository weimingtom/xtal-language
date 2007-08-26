
#include "xtal.h"

#include "xtal_map.h"
#include "xtal_macro.h"

namespace xtal{

class MapIter : public Base{
	MapPtr map_;
	uint_t pos_;
	Map::Node* node_;

	virtual void visit_members(Visitor& m){
		Base::visit_members(m);
		m & map_;
	}

public:

	MapIter(const MapPtr& m)
		:map_(m), pos_(0), node_(0){
		reset();
	}

	AnyPtr reset(){
		pos_ = 0;
		node_ = map_->begin_[pos_];
		return SmartPtr<MapIter>::from_this(this);
	}
	
	void iter_next(const VMachinePtr& vm){
		while(!node_){
			if(pos_!= map_->size_-1){
				pos_++;
				node_ = map_->begin_[pos_];
			}else{
				reset();
				vm->return_result(null, null, null);
				return;
			}
		}
		
		vm->return_result(SmartPtr<MapIter>::from_this(this), node_->key, node_->value);
		node_ = node_->next;
	}
};

class MapKeyIter : public Base{
	MapPtr map_;
	uint_t pos_;
	Map::Node* node_;

	virtual void visit_members(Visitor& m){
		Base::visit_members(m);
		m & map_;
	}

public:

	MapKeyIter(const MapPtr& m)
		:map_(m), pos_(0), node_(0){
		reset();
	}

	AnyPtr reset(){
		pos_ = 0;
		node_ = map_->begin_[pos_];
		return SmartPtr<MapKeyIter>::from_this(this);
	}

	void iter_next(const VMachinePtr& vm){
		while(!node_){
			if(pos_!= map_->size_-1){
				pos_++;
				node_ = map_->begin_[pos_];
			}else{
				reset();
				vm->return_result(null, null);
				return;
			}
		}
		
		vm->return_result(SmartPtr<MapKeyIter>::from_this(this), node_->key);
		node_ = node_->next;
	}
};

class MapValueIter : public Base{
	MapPtr map_;
	uint_t pos_;
	Map::Node* node_;

	virtual void visit_members(Visitor& m){
		Base::visit_members(m);
		m & map_;
	}

public:

	MapValueIter(const MapPtr& m)
		:map_(m), pos_(0), node_(0){
		reset();
	}
	
	AnyPtr reset(){
		pos_ = 0;
		node_ = map_->begin_[pos_];
		return SmartPtr<MapValueIter>::from_this(this);
	}

	void iter_next(const VMachinePtr& vm){
		while(!node_){
			if(pos_!= map_->size_-1){
				pos_++;
				node_ = map_->begin_[pos_];
			}else{
				reset();
				vm->return_result(null, null);
				return;
			}
		}
		
		vm->return_result(SmartPtr<MapValueIter>::from_this(this), node_->value);
		node_ = node_->next;
	}
};

void InitMap(){

	{
		ClassPtr p = new_cpp_class<MapKeyIter>("MapKeyIter");
		p->inherit(Iterator());
		p->method("reset", &MapKeyIter::reset);
		p->method("iter_first", &MapKeyIter::iter_next);
		p->method("iter_next", &MapKeyIter::iter_next);
	}

	{
		ClassPtr p = new_cpp_class<MapIter>("MapIter");
		p->inherit(Iterator());
		p->method("reset", &MapIter::reset);
		p->method("iter_first", &MapIter::iter_next);
		p->method("iter_next", &MapIter::iter_next);
	}

	{
		ClassPtr p = new_cpp_class<MapValueIter>("MapValueIter");
		p->inherit(Iterator());
		p->method("reset", &MapValueIter::reset);
		p->method("iter_first", &MapValueIter::iter_next);
		p->method("iter_next", &MapValueIter::iter_next);
	}

	{
		ClassPtr p = new_cpp_class<Map>("Map");
		p->inherit(Enumerator());
		
		p->def("new", ctor<Map>());
		p->method("to_s", &Map::to_s);
		p->method("size", &Map::size);
		p->method("length", &Map::length);
		p->method("at", &Map::at);
		p->method("set_at", &Map::set_at);
		p->method("op_at", &Map::at);
		p->method("op_set_at", &Map::set_at);
		p->method("each", &Map::each_pair);
		p->method("each_pair", &Map::each_pair);
		p->method("each_key", &Map::each_key);
		p->method("each_value", &Map::each_value);
		p->method("clone", &Map::clone);
		p->method("op_cat", &Map::cat);
		p->method("op_cat_assign", &Map::cat_assign);
		p->method("cat", &Map::cat);
		p->method("cat_assign", &Map::cat_assign);
		p->method("erase", &Map::erase);
		p->method("empty", &Map::empty);
		p->method("op_eq", &Map::op_eq);
	}
}


void Map::visit_members(Visitor& m){
	Base::visit_members(m);
	for(uint_t i = 0; i<size_; ++i){
		Node* p = begin_[i];
		while(p){
			Node* next = p->next;
			m & p->key & p->value;
			p = next;
		}
	}		
}	

Map::Map(){
	size_ = 0;
	begin_ = 0;
	used_size_ = 0;
	expand(7);
}

Map::~Map(){
	for(uint_t i = 0; i<size_; ++i){
		Node* p = begin_[i];
		while(p){
			Node* next = p->next;
			p->~Node();
			user_free(p);
			p = next;
		}
	}
	user_free(begin_);
}
	
const AnyPtr& Map::at(const AnyPtr& key){
	Node* p = begin_[key->hashcode() % size_];
	while(p){
		if(p->key==key){
			return p->value;
		}
		p = p->next;
	}
	return null;
}

void Map::set_at(const AnyPtr& key, const AnyPtr& value){
	Node** p = &begin_[key->hashcode() % size_];
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

void Map::erase(const AnyPtr& key){
	Node* p = begin_[key->hashcode() % size_];
	Node* prev = 0;
	while(p){
		if(p->key==key){
			if(prev){
				prev->next = p->next;
			}else{
				begin_[key->hashcode() % size_] = p->next;
			}
			p->~Node();
			user_free(p);
		}
		prev = p;
		p = p->next;
	}
}

MapPtr Map::cat(const MapPtr& a){
	MapPtr ret(clone());
	ret->cat_assign(a);
	return ret;
}

MapPtr Map::cat_assign(const MapPtr& a){
	Xfor2(k, v, a->each_pair()){
		set_at(k, v);
	}		
	return MapPtr::from_this(this);
}

StringPtr Map::to_s(){
	if(empty())
		return xnew<String>("[:]");
	StringPtr ret(xnew<String>("["));
	bool first = true;
	for(uint_t i = 0; i<size_; ++i){
		Node* p = begin_[i];
		while(p){
			Node* next = p->next;
			if(!first){
				ret = ret->cat(",");
			}else{
				first = false;
			}
			ret = ret->cat(p->key->to_s());
			ret = ret->cat(":");
			ret = ret->cat(p->value->to_s());
			p = next;
		}
	}
	ret = ret->cat("]");
	return ret;
}

bool Map::op_eq(const MapPtr& other){
	if(size()!=other->size())
		return false;
	
	const VMachinePtr& vm = vmachine();
	Xfor2(key, value, other){
		vm->setup_call(1, value);
		at(key)->rawsend(vm, Xid(op_eq));
		if(!vm->processed() || !vm->result()){
			vm->cleanup_call();
			return false;
		}
		vm->cleanup_call();
	}
	return true;
}
	
AnyPtr Map::each_pair(){
	return xnew<MapIter>(MapPtr::from_this(this));
}

AnyPtr Map::each_key(){
	return xnew<MapKeyIter>(MapPtr::from_this(this));
}

AnyPtr Map::each_value(){
	return xnew<MapValueIter>(MapPtr::from_this(this));
}

MapPtr Map::clone(){
	MapPtr ret(xnew<Map>());
	Xfor2(k, v, each_pair()){
		ret->set_at(k, v);
	}
	return ret;
}

void Map::push_all(const VMachinePtr& vm){
	for(uint_t i = 0; i<size_; ++i){
		Node* p = begin_[i];
		while(p){
			Node* next = p->next;
			vm->push(p->key);
			vm->push(p->value);
			p = next;
		}
	}		
}


StrictMap::StrictMap(){
	size_ = 0;
	begin_ = 0;
	used_size_ = 0;
	expand(7);
}

StrictMap::~StrictMap(){
	destroy();
}
	
const AnyPtr& StrictMap::at(const AnyPtr& key){
	Node* p = begin_[rawvalue(key) % size_];
	while(p){
		if(raweq(p->key, key)){
			return p->value;
		}
		p = p->next;
	}
	return null;
}

void StrictMap::set_at(const AnyPtr& key, const AnyPtr& value){
	Node** p = &begin_[rawvalue(key) % size_];
	while(*p){
		if(raweq((*p)->key, key)){
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

void StrictMap::set_node(Node* node){
	Node** p = &begin_[rawvalue(node->key) % size_];
	while(*p){
		p = &(*p)->next;
	}
	*p = node;
}

void StrictMap::expand(int_t addsize){
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
			p->next = 0;
			set_node(p);
			p = next;
		}
	}
	user_free(oldbegin);
}
	
void StrictMap::destroy(){
	if(!begin_)
		return;

	for(uint_t i = 0; i<size_; ++i){
		Node* p = begin_[i];
		while(p){
			Node* next = p->next;
			p->~Node();
			user_free(p);
			p = next;
		}
	}
	user_free(begin_);
	size_ = 0;
	begin_ = 0;
	used_size_ = 0;
}

}
