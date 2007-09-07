
#include "xtal.h"

#include "xtal_map.h"
#include "xtal_macro.h"

namespace xtal{

class MapIter : public Base{
	MapPtr map_;
	Map::Node* node_;
	int_t type_;

	virtual void visit_members(Visitor& m){
		Base::visit_members(m);
		m & map_;
	}

public:

	MapIter(const MapPtr& m, int_t type)
		:map_(m), type_(type), node_(0){
		reset();
	}

	AnyPtr reset(){
		node_ = map_->ordered_head_;
		return SmartPtr<MapIter>::from_this(this);
	}
	
	void block_next(const VMachinePtr& vm){
		if(!node_){
			vm->return_result(null);
			return;
		}
		
		switch(type_){
			case 0: vm->return_result(SmartPtr<MapIter>::from_this(this), node_->key, node_->value); break;
			case 1: vm->return_result(SmartPtr<MapIter>::from_this(this), node_->key); break;
			case 2: vm->return_result(SmartPtr<MapIter>::from_this(this), node_->value); break;
			default: vm->return_result(null); break;
		}

		node_ = node_->ordered_next;
	}
};


void initialize_map(){
	{
		ClassPtr p = new_cpp_class<MapIter>("MapIter");
		p->inherit(PseudoArray());
		p->method("reset", &MapIter::reset);
		p->method("block_first", &MapIter::block_next);
		p->method("block_next", &MapIter::block_next);
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
		p->method("each", &Map::pairs);
		p->method("pairs", &Map::pairs);
		p->method("keys", &Map::keys);
		p->method("values", &Map::values);
		p->method("clone", &Map::clone);
		p->method("op_cat", &Map::cat);
		p->method("op_cat_assign", &Map::cat_assign);
		p->method("cat", &Map::cat);
		p->method("cat_assign", &Map::cat_assign);
		p->method("erase", &Map::erase);
		p->method("empty", &Map::empty);
		p->method("op_eq", &Map::op_eq);
	}

	builtin()->def("Map", get_cpp_class<Map>());
}

void Map::visit_members(Visitor& m){
	Base::visit_members(m);
	for(Node* p = ordered_head_; p; p=p->ordered_next){
		m & p->key & p->value;
	}
}	

Map::Map(){
	size_ = 0;
	begin_ = 0;
	used_size_ = 0;
	ordered_head_ = 0;
	ordered_tail_ = 0;
	expand(7);
}

Map::~Map(){
	clear();
}
	
void Map::clear(){
	for(Node* p = ordered_head_; p;){
		Node* next = p->ordered_next;
		p->~Node();
		user_free(p);
		p = next;
	}

	begin_ = 0;
	used_size_ = 0;
	ordered_head_ = 0;
	ordered_tail_ = 0;
}

const AnyPtr& Map::calc_key(const AnyPtr& key){
	if(type(key)==TYPE_BASE){
		if(const StringPtr& str = as<const StringPtr&>(key)){
			return str->intern();
		}
	}
	return key;
}
	
const AnyPtr& Map::at(const AnyPtr& akey){
	const AnyPtr& key = calc_key(akey);
	uint_t hash = calc_offset(key);
	Node* p = begin_[calc_offset(key)];
	while(p){
		if(raweq(p->key, key)){
			return p->value;
		}
		p = p->next;
	}
	return nop;
}

void Map::set_at(const AnyPtr& akey, const AnyPtr& value){
	const AnyPtr& key = calc_key(akey);
	Node** p = &begin_[calc_offset(key)];
	while(*p){
		if(raweq((*p)->key, key)){
			(*p)->value = value;
			return;
		}
		p = &(*p)->next;
	}

	*p = (Node*)user_malloc(sizeof(Node));
	new(*p) Node(key, value);

	if(ordered_tail_){
		ordered_tail_->ordered_next = *p;
		(*p)->ordered_prev = ordered_tail_;
		ordered_tail_ = *p;
	}else{
		ordered_head_ = *p;
		ordered_tail_ = *p;
	}

	used_size_++;
	if(rate()>0.8f){
		expand(17);
	}
}

void Map::erase(const AnyPtr& akey){
	const AnyPtr& key = calc_key(akey);
	uint_t hash = calc_offset(key);
	Node* p = begin_[hash];
	Node* prev = 0;
	while(p){
		if(raweq(p->key, key)){
			if(prev){
				prev->next = p->next;
			}else{
				begin_[hash] = p->next;
			}

			if(p->ordered_next) p->ordered_next->ordered_prev = p->ordered_prev;
			if(p->ordered_prev) p->ordered_prev->ordered_next = p->ordered_next;

			p->~Node();
			user_free(p);
			break;
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
	for(Node* p = a->ordered_head_; p; p=p->ordered_next){
		set_at(p->key, p->value);
	}
	return MapPtr::from_this(this);
}

void Map::expand(int_t addsize){
	Node** oldbegin = begin_;
	size_ = size_ + size_ + addsize;
	begin_ = (Node**)user_malloc(sizeof(Node*)*size_);
	
	for(uint_t i = 0; i<size_; ++i){
		begin_[i] = 0;
	}

	for(Node* p = ordered_head_; p; p=p->ordered_next){
		p->next = 0;
	}

	for(Node* p = ordered_head_; p; p=p->ordered_next){
		Node** temp = &begin_[calc_offset(p->key)];
		while(*temp){
			temp = &(*temp)->next;
		}
		*temp = p;
	}

	user_free(oldbegin);
}

StringPtr Map::to_s(){
	if(empty())
		return xnew<String>("[:]");

	MemoryStreamPtr ms = xnew<MemoryStream>();
	ms->put_s("[");
	for(Node* p = ordered_head_; p; p=p->ordered_next){
		if(p!=ordered_head_){
			ms->put_s(",");
		}

		ms->put_s(p->key->to_s());
		ms->put_s(":");
		ms->put_s(p->value->to_s());
	}
	ms->put_s("]");
	return ms->to_s();
}

bool Map::op_eq(const MapPtr& other){
	if(size()!=other->size())
		return false;
	
	Xfor2(key, value, other){
		if(at(key)!=value){
			return false;
		}
	}
	return true;
}
	
AnyPtr Map::pairs(){
	return xnew<MapIter>(MapPtr::from_this(this), 0);
}

AnyPtr Map::keys(){
	return xnew<MapIter>(MapPtr::from_this(this), 1);
}

AnyPtr Map::values(){
	return xnew<MapIter>(MapPtr::from_this(this), 2);
}

MapPtr Map::clone(){
	MapPtr ret(xnew<Map>());
	for(Node* p = ordered_head_; p; p=p->ordered_next){
		ret->set_at(p->key, p->value);
	}	
	return ret;
}

void Map::push_all(const VMachinePtr& vm){
	for(Node* p = ordered_head_; p; p=p->ordered_next){
		vm->push(p->key);
		vm->push(p->value);
	}	
}

}
