#include "xtal.h"
#include "xtal_macro.h"

namespace xtal{

void MembersIter::on_visit_members(Visitor& m){
	Base::on_visit_members(m);
	m & frame_;
}

void MembersIter::block_next(const VMachinePtr& vm){
	while(true){
		if(frame_->map_members_ && it_!=frame_->map_members_->end()){
			if(it_->second.accessibility()==KIND_PUBLIC){
				vm->return_result(to_smartptr(this), it_->first.primary_key, it_->first.secondary_key, frame_->members_.at(it_->second.num));
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

void MembersIter2::on_visit_members(Visitor& m){
	Base::on_visit_members(m);
	m & frame_;
}

void MembersIter2::block_next(const VMachinePtr& vm){
	if(it_ < frame_->scope_info_->variable_size){
		vm->return_result(to_smartptr(this), frame_->code_->identifier(frame_->scope_info_->variable_identifier_offset+it_), undefined, frame_->members_.at(it_));
		++it_;
		return;
	}
	else{
		vm->return_result(null, null, null, null);
		return;
	}
}

Frame::Frame(const FramePtr& outer, const CodePtr& code, ScopeInfo* info)
	:outer_(outer), code_(code), scope_info_(info ? info : &empty_class_info), 
	members_(scope_info_->variable_size), map_members_(0), orphan_(true), initialized_members_(false){
}

Frame::Frame()
	:scope_info_(&empty_class_info), 
	members_(0), map_members_(0), orphan_(true), initialized_members_(false){}

Frame::~Frame(){
	if(!orphan_){
		members_.detach();
	}

	if(map_members_){
		map_members_->~Hashtable();
		xfree(map_members_, sizeof(map_t));
	}
}

void Frame::make_map_members(){
	if(!map_members_){
		map_members_ = new(xmalloc(sizeof(map_t))) map_t();
	}
}

void Frame::make_members(){
	if(!initialized_members_){
		make_members_force(0);
	}
}

void Frame::make_members_force(int_t flags){
	if(code_ && scope_info_){
		make_map_members();

		for(int_t i=0; i<scope_info_->variable_size; ++i){
			Key key = {code_->identifier(scope_info_->variable_identifier_offset+i), undefined};
			Value value = {i, flags};
			map_members_->insert(key, value);
		}

		initialized_members_ = true;
	}
}


AnyPtr Frame::members(){
	if(!orphan_){
		return xnew<MembersIter2>(to_smartptr(this));
	}

	make_members();
	return xnew<MembersIter>(to_smartptr(this));
}

const AnyPtr& Frame::on_rawmember(const IDPtr& primary_key, const AnyPtr& secondary_key, bool inherited_too, int_t& accessibility, bool& nocache){
	nocache = true;

	make_members_force(0);

	Key key = {primary_key, secondary_key};
	map_t::iterator it = map_members_->find(key);
	if(it!=map_members_->end()){		
		return member_direct(it->second.num);
	}

	return undefined;
}

bool Frame::replace_member(const IDPtr& primary_key, const AnyPtr& value){
	make_members();

	Key key = {primary_key, undefined};
	map_t::iterator it = map_members_->find(key);
	if(it!=map_members_->end()){
		if(!it->second.nocache()){
			invalidate_cache_member();
		}
		set_member_direct(it->second.num, value);
		return true;
	}

	return false;
}

void Frame::add_ref_count_members(int_t n){
	for(uint_t i=0, size=members_.size(); i<size; ++i){
		add_ref_count_force(members_.at(i), n);
	}
}

void Frame::push_back_member(const AnyPtr& value){
	if(orphan_){
		members_.push_back(value);
	}
	else{
		members_.upsize(1);
		members_.set_at_unref(members_.size()-1, value);
	}
}

}
