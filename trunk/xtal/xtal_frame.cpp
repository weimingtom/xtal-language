
#include "xtal.h"

#include <algorithm>

#include "xtal_frameimpl.h"
#include "xtal_vmachineimpl.h"

namespace xtal{

void InitClass(){
	{
		TClass<ClassImpl::MembersIterImpl> p("ClassMembersIter");
		p.inherit(Iterator());
		p.method("reset", &ClassImpl::MembersIterImpl::reset);
		p.method("iter_first", &ClassImpl::MembersIterImpl::iter_next);
		p.method("iter_next", &ClassImpl::MembersIterImpl::iter_next);
	}

	{
		TClass<Class> p("Class");
		p.method("inherit", &Class::inherit_strict);
		p.method("is_inherited", &Class::is_inherited);
		p.method("each_member", &Class::each_member);
		p.method("serial_new", &Class::serial_new);
		p.method("each_inherited_class", &Class::each_inherited_class);
	}

	{
		TClass<Instance> p("Instance");
		p.method("instance_serial_save", &Instance::instance_serial_save);
		p.method("instance_serial_load", &Instance::instance_serial_load);
	}

	{
		TClass<LibImpl> p("Lib");
		p.inherit(TClass<Class>::get());
		p.def("new", New<LibImpl>());
		p.method("append_load_path", &LibImpl::append_load_path);
	}

	{
		TClass<Nop> p(new NopImpl());
		p.impl()->dec_ref_count();
	}
}

EmptyHaveInstanceVariables empty_have_instance_variables;
uint_t global_mutate_count = 0;

int_t HaveInstanceVariables::find_core_inner(ClassCore* core){
	for(int_t i = 1, size = (int_t)variables_info_.size(); i<size; ++i){
		if(variables_info_[i].core==core){
			std::swap(variables_info_[0], variables_info_[i]);
			return variables_info_[0].pos;
		}	
	}
	XTAL_THROW(builtin().member("InstanceVariableError")(Xt("Xtal Runtime Error 1003")));
}

Any InstanceImpl::instance_serial_save(const Class& cls){
	ClassImpl* p = cls.impl();
	CodeImpl* code = p->code().impl();
	ClassCore* core = p->core();
	if(core->instance_variable_size!=0){	
		int_t pos = find_core(core);

		Map insts;
		for(int_t i=0; i<(int_t)core->instance_variable_size; ++i){
			insts.set_at(code->symbol(core->instance_variable_symbol_offset+i), variables_[pos+i]);
		}

		return insts;
	}
	return null;
}

void InstanceImpl::instance_serial_load(const Class& cls, const Any& v){
	if(!v)
		return;

	Map insts = cast<Map>(v);

	ClassImpl* p = cls.impl();
	CodeImpl* code = p->code().impl();
	ClassCore* core = p->core();
	if(core->instance_variable_size!=0){	
		int_t pos = find_core(core);

		for(int_t i=0; i<(int_t)core->instance_variable_size; ++i){
			variables_[pos+i] = insts.at(code->symbol(core->instance_variable_symbol_offset+i));
		}
	}
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
	
IdMap::Node* IdMap::find(const ID& key, const Any& ns){
	Node* p = begin_[key.rawvalue() % size_];
	while(p){
		if(p->key.impl()==key.impl()){
			return p;
		}
		p = p->next;
	}
	return 0;
}

IdMap::Node* IdMap::insert(const ID& key, const Any& ns){
	Node** p = &begin_[key.rawvalue() % size_];
	while(*p){
		if((*p)->key.impl()==key.impl()){
			return *p;
		}
		p = &(*p)->next;
	}
	*p = (Node*)user_malloc(sizeof(Node));
	new(*p) Node(key, ns);
	used_size_++;
	if(rate()>0.8f){
		expand(17);
		return find(key, ns);
	}else{
		return *p;		
	}
}


void IdMap::visit_members(Visitor& m){
	for(uint_t i = 0; i<size_; ++i){
		Node* p = begin_[i];
		while(p){
			Node* next = p->next;
			m & p->key & p->ns;
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


ClassImpl::ClassImpl(const Frame& outer, const Code& code, ClassCore* core)
	:FrameImpl(outer, code, core){
	set_class(TClass<Class>::get());
	is_defined_by_xtal_ = true;
	make_map_members();
}

ClassImpl::ClassImpl()
	:FrameImpl(null, null, 0){
	set_class(TClass<Class>::get());
	is_defined_by_xtal_ = false;
	make_map_members();
}

void ClassImpl::call(const VMachine& vm){
	if(const Any& ret = member(Xid(new), this, null)){
		ret.call(vm);
	}else{
		XTAL_THROW(builtin().member("RuntimeError")(Xt("Xtal Runtime Error 1013")(object_name())));
	}
}

int_t ClassImpl::arity(){
	return member(Xid(initialize), this, null).arity();
}

void ClassImpl::serial_new(const VMachine& vm){
	if(const Any& ret = member(Xid(serial_new), this, null)){
		ret.call(vm);
	}else{
		XTAL_THROW(builtin().member("RuntimeError")(Xt("Xtal Runtime Error 1013")(object_name())));
	}
}

void ClassImpl::inherit(const Class& md){
	if(is_inherited(md))
		return;
	mixins_.push_back(md);
	global_mutate_count++;
}

void ClassImpl::inherit_strict(const Class& md){
	if(is_inherited(md))
		return;
	if(!md.impl()->is_defined_by_xtal_)
		return;
	mixins_.push_back(md);
	global_mutate_count++;
}


void ClassImpl::init_instance(HaveInstanceVariables* inst, const VMachine& vm, const Any& self){
	for(int_t i = mixins_.size()-1; i>=0; --i){
		mixins_[i].init_instance(inst, vm, self);
	}
	
	if(core()->instance_variable_size){
		inst->init_variables(core());

		vm.setup_call(0);
		vm.set_arg_this(self);
		// 先頭のメソッドはインスタンス変数初期化関数
		members_[0].impl()->call(vm);
		vm.cleanup_call();
	}
}

void ClassImpl::def(const ID& name, const Any& value, int_t accessibility, const Any& ns){
	IdMap::Node* it = map_members_->find(name, ns);
	if(!it){
		it = map_members_->insert(name, ns);
		it->num = members_.size();
		it->flags = accessibility;
		members_.push_back(value);
		value.set_object_name(name, object_name_force(), this);
	}else{
		XTAL_THROW(builtin().member("RedefinedError")(Xt("Xtal Runtime Error 1011")(this->object_name(), name)));
	}
	global_mutate_count++;
}

const Any& ClassImpl::any_member(const ID& name, const Any& ns){
	IdMap::Node* it = map_members_->find(name, ns);
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

const Any& ClassImpl::member(const ID& name, const Any& self, const Any& ns){
	IdMap::Node* it = map_members_->find(name, ns);
	if(it){
		// メンバが見つかった

		// しかしprivateが付けられている
		if(it->flags & KIND_PRIVATE){
			if(self.get_class().raweq(this)){
				return members_[it->num];
			}else{
				// アクセスできない
				XTAL_THROW(builtin().member("AccessibilityError")(Xt("Xtal Runtime Error 1017")(
					Named("object", this->object_name()), Named("name", name), Named("accessibility", "private")))
				);
			}
		}

		// しかしprotectedが付けられている
		if(it->flags & KIND_PROTECTED){
			if(self.is(Class(this))){
				
			}else{
				// アクセスできない
				XTAL_THROW(builtin().member("AccessibilityError")(Xt("Xtal Runtime Error 1017")(
					Named("object", this->object_name()), Named("name", name), Named("accessibility", "protected")))
				);			
			}
		}

		return members_[it->num];
	}
	
	for(int_t i = mixins_.size()-1; i>=0; --i){
		if(const Any& ret = mixins_[i].member(name, self)){
			return ret;
		}
	}

	return TClass<Any>::get().impl()->any_member(name, ns);
}

void ClassImpl::set_member(const ID& name, const Any& value, const Any& ns){
	IdMap::Node* it = map_members_->find(name, ns);
	if(!it){
		XTAL_THROW(builtin().member("RuntimeError")("undefined"));
	}else{
		members_.push_back(value);
		value.set_object_name(name, object_name_force(), this);
	}

	global_mutate_count++;
}

bool ClassImpl::is_inherited(const Class& v){
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


void XClassImpl::call(const VMachine& vm){;
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
	
	if(const Any& ret = member(Xid(initialize), vm.impl()->ff().self(), null)){
		vm.set_arg_this(inst);
		if(vm.need_result()){
			ret.call(vm);
			vm.replace_result(0, inst);
		}else{
			ret.call(vm);
		}
	}else{
		vm.return_result(inst);
	}
}

void XClassImpl::serial_new(const VMachine& vm){
	Instance inst(Class(this));
	init_instance(inst.impl(), vm, inst);
	
	if(inst.impl()->empty()){
		if(const Any& ret = bases_member(Xid(serial_new))){
			ret.call(vm);
			if(vm.result().type()==TYPE_BASE){
				vm.result().impl()->set_class(Class(this));
			}
			return;
		}
	}

	inst.send(Xid(serial_load), vm.arg(0));
	vm.return_result(inst);
}

const Any& LibImpl::member(const ID& name, const Any& self, const Any& ns){
	IdMap::Node* it = map_members_->find(name, ns);
	if(it){
		return members_[it->num];
	}else{
		Xfor(var, load_path_list_.each()){
			String file_name = Xf("%s%s%s%s")(var, join_path("/"), name, ".xtal").to_s();
			if(FILE* fp = fopen(file_name.c_str(), "r")){
				fclose(fp);
				return rawdef(name, load(file_name), ns);
			}
		}
		return null;

		/* 指定した名前をフォルダーとみなす
		Array next = path_.clone();
		next.push_back(name);
		Any lib; new(lib) LibImpl(next);
		return rawdef(name, lib, ns);
		*/
	}
}

void LibImpl::def(const ID& name, const Any& value, int_t accessibility, const Any& ns){
	rawdef(name, value, ns);
}

const Any& LibImpl::rawdef(const ID& name, const Any& value, const Any& ns){
	IdMap::Node* it = map_members_->find(name, ns);
	if(!it){
		it = map_members_->insert(name, ns);
		it->num = members_.size();
		it->flags = KIND_PUBLIC;
		members_.push_back(value);
		global_mutate_count++;
		value.set_object_name(name, object_name_force(), this);
		return members_.back();
	}else{
		XTAL_THROW(builtin().member("RedefinedError")(Xt("Xtal Runtime Error 1011")(this->object_name(), name)));
		return null;
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
	
Frame::Frame(const Frame& outer, const Code& code, BlockCore* core)
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

Any Frame::each_member() const{
	return impl()->each_member();
}

Class::Class(const ID& name, Any*& p, init_tag)
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

Class::Class(const ID& name)
	:Frame(null){
	new(*this) ClassImpl(null, null, 0);
	impl()->set_object_name(name, 1, null);	
}

Class::Class(const Frame& outer, const Code& code, ClassCore* core)
	:Frame(null){
	new(*this) ClassImpl(outer, code, core);
}

void Class::init_instance(HaveInstanceVariables* inst, const VMachine& vm, const Any& self) const{
	impl()->init_instance(inst, vm, self);
}

void Class::serial_new(const VMachine& vm){
	impl()->serial_new(vm);
}

void Class::inherit(const Class& md) const{
	impl()->inherit(md);
}

void Class::inherit_strict(const Class& md) const{
	impl()->inherit_strict(md);
}

bool Class::is_inherited(const Class& md) const{
	return impl()->is_inherited(md);
}

Any Class::each_inherited_class() const{
	return impl()->each_inherited_class();
}

const Any& Class::member(const ID& name) const{
	return impl()->member(name, *this, null);
}

const Any& Class::member(const ID& name, const Any& self) const{
	return impl()->member(name, self, null);
}

const Any& Class::member(const ID& name, const Any& self, const Any& ns) const{
	return impl()->member(name, self, ns);
}

void Class::set_member(const ID& name, const Any& value, const Any& ns) const{
	return impl()->set_member(name, value, ns);
}

Class new_xclass(const Frame& outer, const Code& code, ClassCore* core){
	Class ret(null); new(ret) XClassImpl(outer, code, core);
	return ret; 
}

Instance::Instance(const Class& c)
	:Any(null){
	new(*this) InstanceImpl(c);
}

Any Instance::instance_serial_save(const Class& cls){
	return impl()->instance_serial_save(cls);
}
	
void Instance::instance_serial_load(const Class& cls, const Any& v){
	impl()->instance_serial_load(cls, v);
}


}