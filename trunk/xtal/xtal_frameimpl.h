
#pragma once

#include "xtal_frame.h"
#include "xtal_code.h"
#include "xtal_string.h"
#include "xtal_utilimpl.h"
#include "xtal_macro.h"
#include "xtal_constant.h"

namespace xtal{

class HaveInstanceVariables{
public:

	HaveInstanceVariables(){
		VariablesInfo vi;
		vi.core = 0;
		vi.pos = 0;
		variables_info_.push_back(vi);
	}
			
	~HaveInstanceVariables(){}
		
	void init_variables(FrameCore* core){
		VariablesInfo vi;
		vi.core = core;
		vi.pos = (int_t)variables_.size();
		variables_.resize(vi.pos+core->instance_variable_size);
		variables_info_.push_back(vi);
	}

	const Any& variable(int_t index, FrameCore* core){
		return variables_[find_core(core) + index];
	}

	void set_variable(int_t index, FrameCore* core, const Any& value){
		variables_[find_core(core) + index] = value;
	}

	int_t find_core(FrameCore* core){
		if(variables_info_[0].core == core)
			return variables_info_[0].pos;

		for(int_t i = 1, size = (int_t)variables_info_.size(); i<size; ++i){
			if(variables_info_[i].core==core){
				std::swap(variables_info_[0], variables_info_[i]);
				return variables_info_[0].pos;
			}	
		}
		//XTAL_THROW(builtin().member("InstanceVariableError")(Xt("Xtal Runtime Error 1003")));
		return 0;
	}
	
	bool empty(){
		return variables_.size()==1;
	}

protected:
	
	struct VariablesInfo{
		FrameCore* core;
		int_t pos;
	};

	AC<VariablesInfo>::vector variables_info_;
	AC<Any>::vector variables_;
	
	void visit_members(Visitor& m){
		m & variables_;
	}

};

class InstanceImpl : public AnyImpl, public HaveInstanceVariables{
public:

	InstanceImpl(const Class& c){
		AnyImpl::set_class(c);
		((AnyImpl*)get_class().impl())->inc_ref_count();
	}
	
	~InstanceImpl(){
		((AnyImpl*)get_class().impl())->dec_ref_count();
	}

public:

	void set_class(const Class& c){
		((AnyImpl*)get_class().impl())->dec_ref_count();
		AnyImpl::set_class(c);
		((AnyImpl*)get_class().impl())->inc_ref_count();	
	}

	virtual HaveInstanceVariables* have_instance_variables(){
		return this;
	}

	virtual void visit_members(Visitor& m){
		HaveInstanceVariables::visit_members(m);
		AnyImpl::visit_members(m);
		m & get_class();
	}
};

class IdMap{
public:

	struct Node{
		ID key;
		u16 num;
		u16 flags;
		int_t mutate_count;
		int_t* pmutate_count;
		Node* next;
		
		Node(const ID& key = null, u16 num = 0, u16 flags = 0, int_t mutate_count = 0, int_t* pmutate_count = 0)
			:key(key), num(num), flags(flags), mutate_count(mutate_count), pmutate_count(pmutate_count), next(0){}
	};
	
	friend class iterator;
	
	class iterator{
		IdMap* map_;
		uint_t pos_;
		Node* node_;
	public:
	
		iterator(IdMap* map){
			map_ = map;
			pos_ = 0;
			node_ = map_->begin_[pos_];
			
			while(!node_){
				if(pos_<map_->size_-1){
					++pos_;
					node_ = map_->begin_[pos_];
				}else{
					break;
				}
			}
		}
		
		bool is_done() const{
			return node_==0;
		}
		
		Node* operator->() const{
			return node_;
		}
		
		Node& operator*() const{
			return *node_;
		}
		
		iterator& operator++(){
			if(node_){
				node_ = node_->next;
			}
			while(!node_){
				if(pos_<map_->size_-1){
					++pos_;
					node_ = map_->begin_[pos_];
				}else{
					return *this;
				}
			}
			return *this;
		}
	};


	IdMap();

	~IdMap();
		
	Node* find(const ID& key);

	Node* insert(const ID& key);

	int_t size(){
		return used_size_;
	}
	
	bool empty(){
		return used_size_==0;
	}

	void visit_members(Visitor& m);
	
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

	IdMap(const IdMap&);
	IdMap& operator = (const IdMap&);
};

	
class FrameImpl : public HaveNameImpl{
public:
	
	FrameImpl(const Frame& outer, const Code& code, FrameCore* core)
		:outer_(outer), code_(code), core_(core ? core : &empty_frame_core), members_(core_->variable_size), map_members_(0){
		if(debug::is_enabled()){
			make_map_members();	
		}
	}
	
	FrameImpl()
		:outer_(null), code_(null), core_(&empty_frame_core), map_members_(0){}
		
	~FrameImpl(){
		if(map_members_){
			map_members_->~IdMap();
			user_free(map_members_, sizeof(IdMap));
		}
	}

public:
	
	const Frame& outer(){ 
		return outer_; 
	}

	int_t block_size(){ 
		return core_->variable_size; 
	}

	const Any& member_direct(int_t i){ 
		return members_[i];
	}

	void set_member_direct(int_t i, const Any& value){ 
		members_[i] = value;
	}
		
	void set_object_name(const String& name, int_t force, const Any& parent){
		if(object_name_force()<force){
			HaveNameImpl::set_object_name(name, force, parent);
			if(map_members_){
				for(IdMap::iterator it(map_members_); !it.is_done(); ++it){
					members_[it->num].set_object_name(it->key, force, this);
				}
			}
		}
	}
	
	bool is_defined_by_xtal(){
		return code_;
	}

public:

	void make_map_members(){
		if(!map_members_){
			map_members_ = new(user_malloc(sizeof(IdMap))) IdMap();
			for(int_t i = 0; i<core_->variable_size; ++i){
				IdMap::Node* p = map_members_->insert(code_.get_symbol(core_->variable_symbol_offset+i));
				p->num = (u16)(core_->variable_size-1-i);
			}
		}
	}

	class MembersIterImpl : public AnyImpl{
		Frame frame_;
		IdMap::iterator it_;

		virtual void visit_members(Visitor& m){
			AnyImpl::visit_members(m);
			m & frame_;
		}

	public:

		MembersIterImpl(const Frame& a)
			:frame_(a), it_(frame_.impl()->map_members_){
			set_class(TClass<MembersIterImpl>::get());
		}

		Any restart(){
			it_ = frame_.impl()->map_members_;
			return this;
		}

		void iter_next(const VMachine& vm){
			if(frame_.impl()->map_members_ && !it_.is_done()){
				vm.return_result(this, it_->key, frame_.impl()->members_[it_->num]);
				++it_;
			}else{
				restart();
				vm.return_result(null);
			}
		}
	};

	Any each_member(){
		Any ret;
		new(ret) MembersIterImpl(Frame(this));
		return ret;
	}
	
protected:

	Frame outer_;
	Code code_;
	FrameCore* core_;
	
	typedef AC<Any>::vector vector_t;
	vector_t members_;
	
	enum{
		NOMEMBER = 1 << 0,
		PROTECTED = 1 << 1,
		PRIVATE = 1 << 2
	};
	
	IdMap* map_members_;
	
	virtual void visit_members(Visitor& m){
		HaveNameImpl::visit_members(m);
		m & outer_ & code_ & members_;
		if(map_members_){
			map_members_->visit_members(m);
		}
	}
};

class ClassImpl : public FrameImpl{
public:

	ClassImpl(const Frame& outer, const Code& code, FrameCore* core);
	
	ClassImpl();

public:
	
	virtual void call(const VMachine& vm);
	
	virtual int_t arity();

	virtual void marshal_new(const VMachine& vm);

	virtual void inherit(const Any& md);

	void init_instance(HaveInstanceVariables* inst, const VMachine& vm, const Any& self);

	virtual void def(const ID& name, const Any& value);
	
	void lay(const ID& name, const Any& value, int_t* pmutate_count, unsigned short flags);

	const Any& any_member(const ID& name);
	
	const Any& bases_member(const ID& name);
	
	virtual void set_member(const ID& name, const Any& value);

	virtual const Any& member(const ID& name, const Any& self);
	
	virtual const Any& member(const ID& name, const Any& self, int_t*& pmutate_count, unsigned short& flags);
	
	virtual const Any& member(const ID& name);
	
	virtual const Any& member(const ID& name, int_t*& pmutate_count, unsigned short& flags);

	void mutate();

	bool is_inherited(const Any& v);

	void set_accessibility(const ID& name, int_t kind){
		if(IdMap::Node* node = map_members_->find(name)){
			node->flags |= kind==KIND_PROTECTED ? PROTECTED : (kind==KIND_PRIVATE ? PRIVATE : 0);
		}
	}
	
protected:

	AC<Class>::vector mixins_;
	int_t mutate_count_;

	virtual void visit_members(Visitor& m){
		FrameImpl::visit_members(m);
		m & mixins_;
	}

};

class XClassImpl : public ClassImpl{
public:
		
	XClassImpl(const Frame& outer, const Code& code, FrameCore* core)
		:ClassImpl(outer, code, core){
	}

public:

	virtual void call(const VMachine& vm);

	virtual void marshal_new(const VMachine& vm);
};

class LibImpl : public ClassImpl{
public:

	LibImpl(){
		set_object_name(Xid(lib), 1000, null);
		set_class(TClass<LibImpl>::get());
	}

	LibImpl(const Array& path)
		:path_(path){
		set_class(TClass<LibImpl>::get());
	}

	virtual const Any& member(const ID& name);

	virtual const Any& member(const ID& name, int_t*& pmutate_count, unsigned short& flags);
	
	virtual const Any& member(const ID& name, const Any& self){
		return member(name);
	}

	virtual const Any& member(const ID& name, const Any& self, int_t*& pmutate_count, unsigned short& flags){
		return member(name, pmutate_count, flags);
	}

	virtual void def(const ID& name, const Any& value);

	void append_load_path(const String& path){
		load_path_list_.push_back(path);
	}

private:

	const Any& rawdef(const ID& name, const Any& value, int_t*& pmutate_count);

	String join_path(const String& sep);

private:

	Array load_path_list_;
	Array path_;

	virtual void visit_members(Visitor& m){
		ClassImpl::visit_members(m);
		m & path_ & load_path_list_;
	}
};

class NopImpl : public ClassImpl{
public:

	NopImpl(){
	}

	virtual const Any& member(const ID& name){
		return nop;
	}

	virtual const Any& member(const ID& name, int_t*& pmutate_count, unsigned short& flags){
		return nop;
	}
	
	virtual const Any& member(const ID& name, const Any& self){
		return nop;
	}

	virtual const Any& member(const ID& name, const Any& self, int_t*& pmutate_count, unsigned short& flags){
		return nop;
	}

	virtual void def(const ID& name, const Any& value){

	}
};

}
