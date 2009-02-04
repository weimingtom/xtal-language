#include "xtal.h"
#include "xtal_macro.h"

namespace xtal{

void Map::visit_members(Visitor& m){
	Base::visit_members(m);
	m & table_ & default_value_;
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

//////////////////////////////////////////////////////////

MapIter::MapIter(const MapPtr& m, int_t type)
	:map_(m), type_(type), node_(m->begin()){
}

void MapIter::block_next(const VMachinePtr& vm){
	if(node_==map_->end()){
		if(type_==0){
			vm->return_result(null, null, null);
		}
		else{
			vm->return_result(null, null);
		}
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

bool MapIter::block_next_direct(AnyPtr& rkey, AnyPtr& rval){
	if(node_==map_->end()){
		rkey = null;
		rval = null;
		return false;
	}
	
	switch(type_){
		case 0:
			rkey = node_->first;
			rval = node_->second;
			break;
		case 1: 
			rkey = node_->first;
			rval = null;
			break;

		case 2: 
			rkey = node_->second;
			rval = null;
			break;

		case 3:
			for(;;){
				if(node_->second){
					rkey = node_->first;
					rval = null;
					++node_;
					return true;
				}

				++node_;

				if(node_==map_->end()){
					rkey = null;
					rval = null;
					return false;
				}
			}
			break;
		default: break;
	}

	++node_;
	return true;
}

void MapIter::visit_members(Visitor& m){
	Base::visit_members(m);
	m & map_;
}

//////////////////////////////////////////////////////////

AnyPtr Set::each(){
	return xnew<MapIter>(from_this(this), 3);
}

//////////////////////////////////////////////////////////

void initialize_map(){
	{
		ClassPtr p = new_cpp_class<MapIter>(Xid(MapIter));
		p->inherit(Iterator());
		p->def_method(Xid(block_next), &MapIter::block_next);
	}

	{
		ClassPtr p = new_cpp_class<Map>(Xid(Map));
		p->inherit(Iterable());
		
		p->def(Xid(new), ctor<Map>());
		p->def_method(Xid(to_m), &Map::to_m);
		p->def_method(Xid(size), &Map::size);
		p->def_method(Xid(length), &Map::length);
		p->def_method(Xid(insert), &Map::insert);
		p->def_method(Xid(each), &Map::pairs);
		p->def_method(Xid(pairs), &Map::pairs);
		p->def_method(Xid(keys), &Map::keys);
		p->def_method(Xid(values), &Map::values);
		p->def_method(Xid(clone), &Map::clone);
		p->def_method(Xid(erase), &Map::erase);
		p->def_method(Xid(empty), &Map::empty);
		p->def_method(Xid(assign), &Map::assign);
		p->def_method(Xid(concat), &Map::concat);

		p->def_method(Xid(op_at), &Map::at, get_cpp_class<Any>());
		p->def_method(Xid(op_set_at), &Map::set_at, get_cpp_class<Any>());
		p->def_method(Xid(op_eq), &Map::op_eq, get_cpp_class<Map>());
		p->def_method(Xid(op_cat), &Map::cat, get_cpp_class<Map>());
		p->def_method(Xid(op_cat_assign), &Map::cat_assign, get_cpp_class<Map>());
	}

	{
		ClassPtr p = new_cpp_class<Set>(Xid(Set));
		p->inherit(get_cpp_class<Map>());
		
		p->def(Xid(new), ctor<Set>());
		p->def_method(Xid(each), &Set::each);
	}

	builtin()->def(Xid(Map), get_cpp_class<Map>());
	builtin()->def(Xid(Set), get_cpp_class<Set>());
}

void initialize_map_script(){
Xemb((
	
Map::block_first: method this.each.block_first;

Map::to_s: method{
	if(this.empty){
		return "[:]";
	}

	ms: MemoryStream();
	ms.put_s("[");
	this.each{ |k, v|
		if(!first_step){
			ms.put_s(",");
		}

		ms.put_s(k.to_s);
		ms.put_s(":");
		ms.put_s(v.to_s);
	}
	ms.put_s("]");
	return ms.to_s;
}

Set::to_s: method{
	return this.each_key[].to_s;
}
	),
"\x78\x74\x61\x6c\x01\x00\x00\x00\x00\x00\x00\x4b\x39\x00\x01\x89\x00\x01\x00\x0f\x0b\x2f\x00\x00\x00\x00\x00\x02\x0b\x25\x01\x25\x00\x37\x00\x03\x39\x00\x01\x89\x00\x02\x00\x0f\x0b\x2f\x00\x00\x00\x00\x00\x04\x01\x25\x01\x25\x00\x37\x00\x05\x39\x00\x01\x89"
"\x00\x03\x00\x0f\x0b\x2f\x00\x00\x00\x00\x00\x04\x01\x25\x01\x25\x00\x37\x00\x06\x25\x00\x8b\x00\x03\x08\x00\x00\x00\x00\x00\x02\x00\x00\x00\x12\x00\x20\x00\x00\x00\x00\x00\x04\x00\x00\x00\x12\x00\x38\x00\x00\x00\x00\x00\x06\x00\x00\x00\x12\x00\x00\x00\x00"
"\x04\x00\x00\x00\x00\x03\x06\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x08\x00\x00\x00\x05\x00\x02\x00\x00\x00\x00\x00\x00\x01\x00\x00\x20\x00\x00\x00\x05\x00\x04\x00\x00\x00\x00\x00\x00\x01\x00\x00\x38\x00\x00\x00\x05\x00\x06\x00\x00\x00\x00\x00\x00\x01\x00"
"\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x14\x00\x00\x00\x00\x11\x00\x00\x00\x00\x00\x00\x00\x06\x00\x00\x00\x00\x00\x00\x00\x03\x00\x00\x00\x03\x00\x00\x00\x06\x00\x00\x00\x08\x00\x00\x00\x04\x00\x00\x00\x10\x00\x00\x00\x05\x00\x00"
"\x00\x13\x00\x00\x00\x06\x00\x00\x00\x18\x00\x00\x00\x0b\x00\x00\x00\x18\x00\x00\x00\x08\x00\x00\x00\x1b\x00\x00\x00\x0b\x00\x00\x00\x20\x00\x00\x00\x09\x00\x00\x00\x28\x00\x00\x00\x0a\x00\x00\x00\x2b\x00\x00\x00\x0b\x00\x00\x00\x30\x00\x00\x00\x10\x00\x00"
"\x00\x30\x00\x00\x00\x0d\x00\x00\x00\x33\x00\x00\x00\x10\x00\x00\x00\x38\x00\x00\x00\x0e\x00\x00\x00\x40\x00\x00\x00\x0f\x00\x00\x00\x43\x00\x00\x00\x10\x00\x00\x00\x48\x00\x00\x00\x11\x00\x00\x00\x00\x01\x0b\x00\x00\x00\x03\x09\x00\x00\x00\x06\x73\x6f\x75"
"\x72\x63\x65\x09\x00\x00\x00\x11\x74\x6f\x6f\x6c\x2f\x74\x65\x6d\x70\x2f\x69\x6e\x2e\x78\x74\x61\x6c\x09\x00\x00\x00\x0b\x69\x64\x65\x6e\x74\x69\x66\x69\x65\x72\x73\x0a\x00\x00\x00\x07\x09\x00\x00\x00\x00\x09\x00\x00\x00\x05\x4d\x75\x74\x65\x78\x09\x00\x00"
"\x00\x04\x6c\x6f\x63\x6b\x09\x00\x00\x00\x0b\x62\x6c\x6f\x63\x6b\x5f\x66\x69\x72\x73\x74\x09\x00\x00\x00\x06\x75\x6e\x6c\x6f\x63\x6b\x09\x00\x00\x00\x0a\x62\x6c\x6f\x63\x6b\x5f\x6e\x65\x78\x74\x09\x00\x00\x00\x0b\x62\x6c\x6f\x63\x6b\x5f\x62\x72\x65\x61\x6b"
"\x09\x00\x00\x00\x06\x76\x61\x6c\x75\x65\x73\x0a\x00\x00\x00\x01\x03"
)->call();
}

}
