#include "xtal.h"
#include "xtal_macro.h"

namespace xtal{

class MapIter : public Base{
	MapPtr map_;
	Map::iterator node_;
	int_t type_;

	virtual void visit_members(Visitor& m){
		Base::visit_members(m);
		m & map_;
	}

public:

	MapIter(const MapPtr& m, int_t type)
		:map_(m), type_(type), node_(m->begin()){
	}
	
	void block_next(const VMachinePtr& vm){
		if(node_==map_->end()){
			vm->return_result(null, null);
			return;
		}
		
		switch(type_){
			case 0: vm->return_result(from_this(this), node_->first, node_->second); break;
			case 1: vm->return_result(from_this(this), node_->first); break;
			case 2: vm->return_result(from_this(this), node_->second); break;
			case 3:
				for(;;){
					if(node_->second){
						vm->return_result(from_this(this), node_->first);
						++node_;
						return;
					}

					++node_;

					if(node_==map_->end()){
						vm->return_result(null, null);
						return;
					}
				}
				break;
			default: vm->return_result(null, null); break;
		}

		++node_;
	}
};

void Map::visit_members(Visitor& m){
	Base::visit_members(m);
	m & table_;
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
	iterator it = table_.find(key);
	if(it!=end()){
		return it->second;
	}
	else{
		return default_value_;
	}
}

MapPtr Map::cat(const MapPtr& a){
	MapPtr ret(clone());
	ret->cat_assign(a);
	return ret;
}

MapPtr Map::cat_assign(const MapPtr& a){
	for(iterator p = a->begin(); p!=a->end(); ++p){
		set_at(p->first, p->second);
	}
	return from_this(this);
}

StringPtr Map::to_s(){
	if(empty())
		return xnew<String>("[:]");

	MemoryStreamPtr ms = xnew<MemoryStream>();
	ms->put_s("[");
	for(iterator p = begin(); p!=end(); ++p){
		if(p!=begin()){
			ms->put_s(",");
		}

		ms->put_s(p->first->to_s());
		ms->put_s(":");
		ms->put_s(p->second->to_s());
	}
	ms->put_s("]");
	return ms->to_s();
}

bool Map::op_eq(const MapPtr& a){
	if(size()!=a->size())
		return false;
	
	for(iterator p = a->begin(); p!=a->end(); ++p){
		if(at(p->first)!=p->second){
			return false;
		}
	}
	return true;
}
	
AnyPtr Map::pairs(){
	return xnew<MapIter>(from_this(this), 0);
}

AnyPtr Map::keys(){
	return xnew<MapIter>(from_this(this), 1);
}

AnyPtr Map::values(){
	return xnew<MapIter>(from_this(this), 2);
}

MapPtr Map::clone(){
	MapPtr ret(xnew<Map>());
	for(iterator p = begin(); p!=end(); ++p){
		ret->set_at(p->first, p->second);
	}	
	return ret;
}
	
void Map::assign(const AnyPtr& iterator){
	clear();

	Xfor2(k, v, iterator){
		set_at(k, v);
	}
}
	
void Map::concat(const AnyPtr& iterator){
	Xfor2(k, v, iterator){
		set_at(k, v);
	}
}

void Map::push_all(const VMachinePtr& vm){
	for(iterator p = begin(); p!=end(); ++p){
		vm->push(p->first);
		vm->push(p->second);
	}	
}


AnyPtr Set::each(){
	return xnew<MapIter>(from_this(this), 3);
}

void initialize_map(){
	{
		ClassPtr p = new_cpp_class<MapIter>(Xid(MapIter));
		p->inherit(Iterator());
		p->method(Xid(block_next), &MapIter::block_next);
	}

	{
		ClassPtr p = new_cpp_class<Map>(Xid(Map));
		p->inherit(Iterable());
		
		p->def(Xid(new), ctor<Map>());
		p->method(Xid(to_s), &Map::to_s);
		p->method(Xid(to_m), &Map::to_m);
		p->method(Xid(size), &Map::size);
		p->method(Xid(length), &Map::length);
		p->method(Xid(insert), &Map::insert);
		p->method(Xid(each), &Map::pairs);
		p->method(Xid(pairs), &Map::pairs);
		p->method(Xid(keys), &Map::keys);
		p->method(Xid(values), &Map::values);
		p->method(Xid(clone), &Map::clone);
		p->method(Xid(erase), &Map::erase);
		p->method(Xid(empty), &Map::empty);
		p->method(Xid(assign), &Map::assign);
		p->method(Xid(concat), &Map::concat);

		p->method(Xid(op_at), &Map::at, get_cpp_class<Any>());
		p->method(Xid(op_set_at), &Map::set_at, get_cpp_class<Any>());
		p->method(Xid(op_eq), &Map::op_eq, get_cpp_class<Map>());
		p->method(Xid(op_cat), &Map::cat, get_cpp_class<Map>());
		p->method(Xid(op_cat_assign), &Map::cat_assign, get_cpp_class<Map>());
	}

	{
		ClassPtr p = new_cpp_class<Set>(Xid(Set));
		p->inherit(get_cpp_class<Map>());
		
		p->def(Xid(new), ctor<Set>());
		p->method(Xid(each), &Set::each);
	}

	builtin()->def(Xid(Map), get_cpp_class<Map>());
	builtin()->def(Xid(Set), get_cpp_class<Set>());
}
}
