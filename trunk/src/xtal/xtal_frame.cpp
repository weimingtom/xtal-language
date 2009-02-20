#include "xtal.h"
#include "xtal_macro.h"

namespace xtal{

void MembersIter::visit_members(Visitor& m){
	Base::visit_members(m);
	m & frame_;
}

void MembersIter::block_next(const VMachinePtr& vm){
	while(true){
		if(frame_->map_members_ && it_!=frame_->map_members_->end()){
			if(it_->second.flags==0){
				vm->return_result(from_this(this), it_->first.primary_key, it_->first.secondary_key, frame_->members_->at(it_->second.num));
				++it_;
				return;
			}
			else{
				++it_;
			}
		}
		else{
			vm->return_result(null, null, null, null);
			return;
		}
	}
}


Frame::Frame(const FramePtr& outer, const CodePtr& code, ScopeInfo* info)
	:outer_(outer), code_(code), scope_info_(info ? info : &empty_class_info), members_(xnew<Array>(scope_info_->variable_size)), map_members_(0){
}

Frame::Frame()
	:outer_(null), code_(null), scope_info_(&empty_class_info), members_(xnew<Array>()), map_members_(0){}
	
Frame::Frame(const Frame& v)
:HaveParent(v), outer_(v.outer_), code_(v.code_), scope_info_(v.scope_info_), members_(v.members_->clone()), map_members_(0){

	if(v.map_members_){
		make_map_members();
		*map_members_ = *v.map_members_;
	}

	inc_global_mutate_count();
}

Frame& Frame::operator=(const Frame& v){
	if(this==&v){ return *this; }

	HaveParent::operator=(v);

	outer_ = v.outer_;
	code_ = v.code_;
	scope_info_ = v.scope_info_;

	members_ = v.members_;

	if(v.map_members_){
		make_map_members();
		*map_members_ = *v.map_members_;
	}
	else{
		map_members_->~Hashtable();
		so_free(map_members_, sizeof(map_t));
		map_members_ = 0;
	}

	inc_global_mutate_count();

	return *this;
}

Frame::~Frame(){
	if(map_members_){
		map_members_->~Hashtable();
		so_free(map_members_, sizeof(map_t));
	}
}
	
void Frame::make_map_members(){
	if(!map_members_){
		map_members_ = new(so_malloc(sizeof(map_t))) map_t();
	}
}

AnyPtr Frame::members(){
	if(!map_members_ && code_ && scope_info_){
		make_map_members();
		for(int_t i=0; i<scope_info_->variable_size; ++i){
			Key key = {code_->identifier(scope_info_->variable_identifier_offset+i), null};
			Value value = {i, 0};
			map_members_->insert(key, value);
		}
	}

	return xnew<MembersIter>(from_this(this));
}

}
