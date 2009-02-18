#include "xtal.h"
#include "xtal_macro.h"

namespace xtal{

void Map::visit_members(Visitor& m){
	Base::visit_members(m);
	m & table_ & default_value_;
}	

const AnyPtr& Map::calc_key(const AnyPtr& key){
	if(type(key)==TYPE_STRING){
		if(const StringPtr& str = unchecked_ptr_cast<String>(key)){
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
	:map_(m), node_(m->begin()), type_(type){
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

}
