
#include <algorithm>

#include "xtal.h"
#include "frameimpl.h"

namespace xtal{

static void InitClassIterImpl(){
	TClass<ClassImpl::MembersIterImpl> p("ClassMembersIter");
	p.inherit(Iterator());
	p.method("iter_first", &ClassImpl::MembersIterImpl::iter_first);
	p.method("iter_next", &ClassImpl::MembersIterImpl::iter_next);
}

void InitClass(){
	InitClassIterImpl();
	
	TClass<Class> p("Class");
	p.method("inherit", &Class::inherit);
	p.method("is_inherited", &Class::is_inherited);
	p.method("members", &Class::members);
}



IdMap::IdMap(){
	size_ = 0;
	begin_ = 0;
	used_size_ = 0;
	expand(7);
}

IdMap::~IdMap(){
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
}
	
IdMap::Node* IdMap::find(const ID& key){
	Node* p = begin_[key.rawvalue() % size_];
	while(p){
		if(p->key.impl()==key.impl()){
			return p;
		}
		p = p->next;
	}
	return 0;
}

IdMap::Node* IdMap::insert(const ID& key){
	Node** p = &begin_[key.rawvalue() % size_];
	while(*p){
		if((*p)->key.impl()==key.impl()){
			return *p;
		}
		p = &(*p)->next;
	}
	*p = (Node*)user_malloc(sizeof(Node));
	new(*p) Node(key);
	used_size_++;
	if(rate()>0.8f){
		expand(17);
		return find(key);
	}else{
		return *p;		
	}
}


void IdMap::visit_members(Visitor& m){
	for(uint_t i = 0; i<size_; ++i){
		Node* p = begin_[i];
		while(p){
			Node* next = p->next;
			m & p->key;
			p = next;
		}
	}		
}

void IdMap::set_node(Node* node){
	Node** p = &begin_[node->key.rawvalue() % size_];
	while(*p){
		p = &(*p)->next;
	}
	*p = node;
}

void IdMap::expand(int_t addsize){
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
	user_free(oldbegin, sizeof(Node*)*oldsize);
}


ClassImpl::ClassImpl(const Frame& outer, const Code& code, FrameCore* core)
	:FrameImpl(outer, code, core){
	set_class(TClass<Class>::get());
	make_map_members();
	mutate_count_ = 0;
}

ClassImpl::ClassImpl()
	:FrameImpl(null, null, 0){
	set_class(TClass<Class>::get());
	make_map_members();
	mutate_count_ = 0;
}

void ClassImpl::call(const VMachine& vm){
	if(const Any& ret = member(Xid(new))){
		ret.call(vm);
	}else{
		throw builtin().member("RuntimeError")(Xt("%s :: new 関数が登録されていないため、インスタンスを生成できません。")(object_name()));
	}
}

void ClassImpl::marshal_new(const VMachine& vm){
	if(const Any& ret = member(Xid(new))){
		ret.call(vm);
	}else{
		throw builtin().member("RuntimeError")(Xt("%s :: new 関数が登録されていないため、インスタンスを生成できません。")(object_name()));
	}
}

void ClassImpl::inherit(const Any& md){
	if(is_inherited(md))
		return;
	mixins_.push_back(cast<Class>(md));
	mutate();
}

void ClassImpl::init_instance(HaveInstanceVariables* inst, const VMachine& vm, const Any& self){
	for(int_t i = mixins_.size()-1; i>=0; --i){
		mixins_[i].init_instance(inst, vm, self);
	}
	
	if(core_->instance_variable_size){
		inst->init_variables(core_);

		vm.setup_call(0);
		vm.set_arg_this(self);
		// 一番最初のメソッドがインスタンス変数初期化関数
		members_[0].impl()->call(vm);
		vm.cleanup_call();
	}
}

void ClassImpl::def(const ID& name, const Any& value){
	IdMap::Node* it = map_members_->find(name);
	if(!it){
		it = map_members_->insert(name);
		it->num = members_.size();
		members_.push_back(value);
		value.set_object_name(name, object_name_force(), this);
	}else{
		if((it->flags&NOMEMBER)==0){
			throw builtin().member("BadRedefineError")(Xt("%s :: '%s' は既に定義されています")(this->object_name(), name));
		}else{
			it->flags = 0;
			it->mutate_count = 0;
			it->pmutate_count = 0;
			members_[it->num] = value;
			value.set_object_name(name, object_name_force(), this);
		}
	}
	mutate();
}

void ClassImpl::lay(const ID& name, const Any& value, int_t* pmutate_count){
	IdMap::Node* it = map_members_->find(name);
	if(!it){
		it = map_members_->insert(name);
		it->num = members_.size();
		it->flags = NOMEMBER;
		it->mutate_count = *pmutate_count;
		it->pmutate_count = pmutate_count;
		members_.push_back(value);	
	}else{
		it->mutate_count = *pmutate_count;
		it->pmutate_count = pmutate_count;
		members_[it->num] = value;
	}
}

const Any& ClassImpl::any_member(const ID& name){
	IdMap::Node* it = map_members_->find(name);
	if(it){
		return members_[it->num];
	}
	return null;
}

const Any& ClassImpl::bases_member(const ID& name){
	for(int_t i = mixins_.size()-1; i>=0; --i){
		if(const Any& ret = mixins_[i].member(name)){
			return ret;
		}
	}
	return null;
}

const Any& ClassImpl::member(const ID& name){
	IdMap::Node* it = map_members_->find(name);
	if(it){
		if(it->flags&NOMEMBER){
			if(it->mutate_count==*it->pmutate_count){
				return members_[it->num];
			}
		}else{
			return members_[it->num];
		}
	}
	
	int_t* pmutate_count;
	for(int_t i = mixins_.size()-1; i>=0; --i){
		if(const Any& ret = mixins_[i].member(name, pmutate_count)){
			lay(name, ret, pmutate_count);
			return ret;
		}
	}

	return TClass<Any>::get().impl()->any_member(name);
}

const Any& ClassImpl::member(const ID& name, int_t*& pmutate_count){
	IdMap::Node* it = map_members_->find(name);
	if(it){
		if(it->flags&NOMEMBER){
			if(it->mutate_count==*it->pmutate_count){
				pmutate_count = it->pmutate_count;
				return members_[it->num];
			}
		}else{
			pmutate_count = &mutate_count_;
			return members_[it->num];
		}
	}
	
	for(int_t i = mixins_.size()-1; i>=0; --i){
		if(const Any& ret = mixins_[i].member(name, pmutate_count)){
			lay(name, ret, pmutate_count);
			return ret;
		}
	}

	return null;
}

void ClassImpl::mutate(){
	mutate_count_++;
	for(int_t i = mixins_.size()-1; i>=0; --i){
		mixins_[i].mutate();
	}
}

bool ClassImpl::is_inherited(const Any& v){
	if(this==v.impl()){
		return true;
	}
	for(int_t i = mixins_.size()-1; i>=0; --i){
		if(mixins_[i].is_inherited(v)){
			return true;
		}
	}
	return v.raweq(TClass<Any>::get());
}


void XClassImpl::call(const VMachine& vm){
	Instance inst(Class(this));
	init_instance(inst.impl(), vm, inst);
	
	if(inst.impl()->empty()){
		if(const Any& ret = bases_member(Xid(new))){
			ret.call(vm);
			if(vm.result().type()==TYPE_BASE){
				vm.result().impl()->set_class(Class(this));
			}
			return;
		}
	}
	
	if(const Any& ret = member(Xid(initialize))){
		vm.set_arg_this(inst);
		if(vm.required_result_count()){
			ret.call(vm);
			vm.replace_result(0, inst);
		}else{
			ret.call(vm);
		}
	}else{
		vm.return_result(inst);
	}
}

void XClassImpl::marshal_new(const VMachine& vm){
	Instance inst(Class(this));
	init_instance(inst.impl(), vm, inst);
	
	if(inst.impl()->empty()){
		if(const Any& ret = bases_member(Xid(new))){
			ret.call(vm);
			if(vm.result().type()==TYPE_BASE){
				vm.result().impl()->set_class(Class(this));
			}
			return;
		}
	}
	
	if(const Any& ret = member(Xid(marshal_load))){
		vm.set_arg_this(inst);
		if(vm.required_result_count()){
			ret.call(vm);
			vm.replace_result(0, inst);
		}else{
			ret.call(vm);
		}
	}else{
		vm.return_result(inst);
	}
}

const Any& LibImpl::member(const ID& name){
	IdMap::Node* it = map_members_->find(name);
	if(it){
		return members_[it->num];
	}else{
		int_t* pmutate_count;
		Xfor(var, builtin().member("lib_path").send("each")){
			String file_name = Xf("%s%s%s%s")(var, join_path("/"), name, ".xtal").to_s();
			if(FILE* fp = fopen(file_name.c_str(), "r")){
				fclose(fp);
				return rawdef(name, load(file_name), pmutate_count);
			}
		}
		Array next = path_.clone();
		next.push_back(name);
		Any lib; new(lib) LibImpl(next);
		return rawdef(name, lib, pmutate_count);
	}
}

const Any& LibImpl::member(const ID& name, int_t*& pmutate_count){
	IdMap::Node* it = map_members_->find(name);
	if(it){
		pmutate_count = &mutate_count_;
		return members_[it->num];
	}else{
		Xfor(var, builtin().member("lib_path").send("each")){
			String file_name = Xf("%s%s%s%s")(var, join_path("/"), name, ".xtal").to_s();
			if(FILE* fp = fopen(file_name.c_str(), "r")){
				fclose(fp);
				return rawdef(name, load(file_name), pmutate_count);
			}
		}
		Array next = path_.clone();
		next.push_back(name);
		Any lib; new(lib) LibImpl(next);
		return rawdef(name, lib, pmutate_count);
	}
}

void LibImpl::def(const ID& name, const Any& value){
	int_t* pmutate_count;
	rawdef(name, value, pmutate_count);
}

const Any& LibImpl::rawdef(const ID& name, const Any& value, int_t*& pmutate_count){
	IdMap::Node* it = map_members_->find(name);
	if(!it){
		it = map_members_->insert(name);
		it->num = members_.size();
		members_.push_back(value);
		mutate();
		pmutate_count = &mutate_count_;
		//value.set_object_name(String("lib").cat(join_path("::")).cat(name), 100);
		value.set_object_name(name, object_name_force(), this);
		return members_.back();
	}else{
		throw builtin().member("BadRedefineError")(Xt("%s :: '%s' は既に定義されています")(this->object_name(), name));
	}
}

String LibImpl::join_path(const String& sep){
	if(path_.empty()){
		return sep;
	}else{
		return sep.cat(path_.join(sep)).cat(sep);
	}
}
	

Frame::Frame()
	:Any(null){
	new(*this) FrameImpl();
}
	
Frame::Frame(const Frame& outer, const Code& code, FrameCore* core)
	:Any(null){
	new(*this) FrameImpl(outer, code, core);
}

const Frame& Frame::outer() const{
	return impl()->outer();	
}

int_t Frame::block_size() const{
	return impl()->block_size();
}

const Any& Frame::member_direct(int_t i) const{
	return impl()->member_direct(i);
}

void Frame::set_member_direct(int_t i, const Any& value) const{
	return impl()->set_member_direct(i, value);
}
	
bool Frame::is_defined_by_xtal() const{
	return impl()->is_defined_by_xtal();
}

Class::Class(const char* name, Any*& p, init_tag)
	:Frame(make_impl(p)){
	impl()->set_object_name(name, 1, null);
}

void Class::make_place(Any*& p){
	if(!p){ 
		p = xtal::make_place();
	}
}

Class& Class::make_impl(Any*& p){
	make_place(p);
	if(!*p){ 
		new(*p) ClassImpl();
	}
	return *(Class*)p;
}

Class::Class(const char* name)
	:Frame(null){
	new(*this) ClassImpl();
	impl()->set_object_name(name, 1, null);	
}

Class::Class(const Frame& outer, const Code& code, FrameCore* core)
	:Frame(null){
	new(*this) ClassImpl(outer, code, core);
}

void Class::init_instance(HaveInstanceVariables* inst, const VMachine& vm, const Any& self) const{
	impl()->init_instance(inst, vm, self);
}

void Class::marshal_new(const VMachine& vm){
	impl()->marshal_new(vm);
}

void Class::inherit(const Any& md) const{
	impl()->inherit(md);
}

bool Class::is_inherited(const Any& md) const{
	return impl()->is_inherited(md);
}

const Any& Class::member(const ID& name) const{
	return impl()->member(name);
}
	
const Any& Class::member(const ID& name, int_t*& mutate_count) const{
	return impl()->member(name, mutate_count);
}

void Class::mutate() const{
	return impl()->mutate();
}

Any Class::members() const{
	return impl()->members();
}

Class new_xclass(const Frame& outer, const Code& code, FrameCore* core){
	Class ret(null); new(ret) XClassImpl(outer, code, core);
	return ret; 
}

Instance::Instance(const Class& c)
	:Any(null){
	new(*this) InstanceImpl(c);
}

}
