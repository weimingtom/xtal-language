
#include "xtal.h"

#include "mapimpl.h"

namespace xtal{

static void InitMapIterImpl(){
	TClass<MapImpl::MapIterImpl> p("MapIter");
	p.inherit(Iterator());
	p.method("restart", &MapImpl::MapIterImpl::restart);
	p.method("iter_first", &MapImpl::MapIterImpl::iter_next);
	p.method("iter_next", &MapImpl::MapIterImpl::iter_next);
}

static void InitMapKeyIterImpl(){
	TClass<MapImpl::MapKeyIterImpl> p("MapKeyIter");
	p.inherit(Iterator());
	p.method("restart", &MapImpl::MapKeyIterImpl::restart);
	p.method("iter_first", &MapImpl::MapKeyIterImpl::iter_next);
	p.method("iter_next", &MapImpl::MapKeyIterImpl::iter_next);
}

static void InitMapValueIterImpl(){
	TClass<MapImpl::MapValueIterImpl> p("MapValueIter");
	p.inherit(Iterator());
	p.method("restart", &MapImpl::MapValueIterImpl::restart);
	p.method("iter_first", &MapImpl::MapValueIterImpl::iter_next);
	p.method("iter_next", &MapImpl::MapValueIterImpl::iter_next);
}

void InitMap(){

	InitMapIterImpl();
	InitMapKeyIterImpl();
	InitMapValueIterImpl();

	TClass<Map> p("Map");
	p.inherit(Enumerable());
	
	p.def("new", New<Map>());
	p.method("to_s", &Map::to_s);
	p.method("size", &Map::size);
	p.method("length", &Map::length);
	p.method("at", &Map::at);
	p.method("set_at", &Map::set_at);
	p.method("op_at", &Map::at);
	p.method("op_set_at", &Map::set_at);
	p.method("each", &Map::each_pair);
	p.method("each_pair", &Map::each_pair);
	p.method("each_key", &Map::each_key);
	p.method("each_value", &Map::each_value);
	p.method("clone", &Map::clone);
	p.method("op_cat", &Map::cat);
	p.method("op_cat_assign", &Map::cat_assign);
	p.method("cat", &Map::cat);
	p.method("cat_assign", &Map::cat_assign);
	p.method("erase", &Map::erase);
	p.method("empty", &Map::empty);
}

Map::Map(){
	new(*this) MapImpl();
}

int_t Map::size() const{ 
	return impl()->size();
}

const Any& Map::at(const Any& i) const{ 
	return impl()->at(i);
}

void Map::set_at(const Any& i, const Any& v) const{ 
	impl()->set_at(i, v);
}

String Map::to_s() const{
	return impl()->to_s();
}

Any Map::each_pair() const{ 
	return impl()->each_pair();
}

Any Map::each_key() const{ 
	return impl()->each_key();
}

Any Map::each_value() const{ 
	return impl()->each_value();
}

Map Map::clone() const{ 
	return impl()->clone();
}

void Map::erase(const Any& key) const{
	impl()->erase(key);
}

Map Map::cat(const Map& a) const{
	return impl()->cat(a);
}

Map Map::cat_assign(const Map& a) const{
	return impl()->cat_assign(a);
}

int_t Map::length() const{
	return impl()->size();
}

bool Map::empty() const{
	return impl()->size()==0;
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
	
const Any& StrictMap::at(const Any& key){
	Node* p = begin_[key.rawvalue() % size_];
	while(p){
		if(p->key.raweq(key)){
			return p->value;
		}
		p = p->next;
	}
	return null;
}

void StrictMap::set_at(const Any& key, const Any& value){
	Node** p = &begin_[key.rawvalue() % size_];
	while(*p){
		if((*p)->key.raweq(key)){
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
	Node** p = &begin_[node->key.hashcode() % size_];
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
			set_node(p);
			p->next = 0;
			p = next;
		}
	}
	user_free(oldbegin, sizeof(Node*)*oldsize);
}
	
void StrictMap::destroy(){
	if(!begin_)
		return;

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
	size_ = 0;
	begin_ = 0;
	used_size_ = 0;
}

}
