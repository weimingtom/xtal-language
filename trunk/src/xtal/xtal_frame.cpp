#include "xtal.h"
#include "xtal_macro.h"

namespace xtal{

class MembersIter : public Base{
	FramePtr frame_;
	Frame::map_t::iterator it_;

	virtual void visit_members(Visitor& m){
		Base::visit_members(m);
		m & frame_;
	}

public:

	MembersIter(const FramePtr& a)
		:frame_(a), it_(frame_->map_members_->begin()){
	}

	void block_next(const VMachinePtr& vm){
		if(frame_->map_members_ && it_!=frame_->map_members_->end()){
			vm->return_result(from_this(this), it_->first.primary_key, it_->first.secondary_key, frame_->members_->at(it_->second.num));
			++it_;
		}
		else{
			vm->return_result(null, null, null, null);
		}
	}
};

Frame::Frame(const FramePtr& outer, const CodePtr& code, ScopeInfo* core)
	:outer_(outer), code_(code), core_(core ? core : &empty_class_info), members_(xnew<Array>(core_->variable_size)), map_members_(0){
}

Frame::Frame()
	:outer_(null), code_(null), core_(&empty_class_info), members_(xnew<Array>()), map_members_(0){}
	
Frame::Frame(const Frame& v)
	:HaveName(v), outer_(v.outer_), code_(v.code_), core_(v.core_), members_(v.members_), map_members_(0){

	if(v.map_members_){
		make_map_members();
		*map_members_ = *v.map_members_;
	}

	inc_global_mutate_count();
}

Frame& Frame::operator=(const Frame& v){
	if(this==&v){ return *this; }

	HaveName::operator=(v);

	outer_ = v.outer_;
	code_ = v.code_;
	core_ = v.core_;

	members_ = v.members_;

	if(v.map_members_){
		make_map_members();
		*map_members_ = *v.map_members_;
	}
	else{
		map_members_->~Hashtable();
		user_free(map_members_);
		map_members_ = 0;
	}

	inc_global_mutate_count();

	return *this;
}

Frame::~Frame(){
	if(map_members_){
		map_members_->~Hashtable();
		user_free(map_members_);
	}
}

void Frame::set_class_member(int_t i, const IDPtr& primary_key, const AnyPtr& value, const AnyPtr& secondary_key, int_t accessibility){ 
	members_->set_at(i, value);
	Key key = {primary_key, secondary_key};
	Value val = {i, accessibility};
	map_members_->insert(key, val);
	value->set_object_name(primary_key, object_name_force(), from_this(this));
	inc_global_mutate_count();
}
	
void Frame::set_object_name(const StringPtr& primary_key, int_t force, const AnyPtr& parent){
	if(object_name_force()<force){
		HaveName::set_object_name(primary_key, force, parent);
		if(map_members_){
			for(map_t::iterator it=map_members_->begin(), last=map_members_->end(); it!=last; ++it){
				members_->at(it->second.num)->set_object_name(it->first.primary_key, force, from_this(this));
			}
		}
	}
}

void Frame::make_map_members(){
	if(!map_members_){
		map_members_ = new(user_malloc(sizeof(map_t))) map_t();
	}
}

StringPtr Frame::object_name(int_t depth){
	if(!name_){
		set_object_name(ptr_cast<String>(Xf("<(%s):%s:%d>")->call(get_class()->object_name(depth), code_->source_file_name(), code_->compliant_lineno(code_->data()+core_->pc))), 1, parent_);
	}

	return HaveName::object_name(depth);
}

AnyPtr Frame::members(){
	if(!map_members_ && code_ && core_){
		make_map_members();
		for(int_t i=0; i<core_->variable_size; ++i){
			Key key = {code_->identifier(core_->variable_identifier_offset+i), null};
			Value value = {i, 0};
			map_members_->insert(key, value);
		}
	}

	return xnew<MembersIter>(from_this(this));
}

void initialize_frame(){
	{
		ClassPtr p = new_cpp_class<MembersIter>(Xid(ClassMembersIter));
		p->inherit(Iterator());
		p->def_method(Xid(block_next), &MembersIter::block_next);
	}

	{
		ClassPtr p = new_cpp_class<Frame>(Xid(Frame));
		p->def_method(Xid(members), &Frame::members);
	}

	builtin()->def(Xid(Frame), get_cpp_class<Frame>());
}

}
