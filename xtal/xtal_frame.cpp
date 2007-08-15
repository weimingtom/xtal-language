
#include "xtal.h"

#include <algorithm>

#include "xtal_frame.h"
#include "xtal_vmachine.h"
#include "xtal_code.h"
#include "xtal_macro.h"

namespace xtal{

class MembersIter : public Base{
	FramePtr frame_;
	IdMap::iterator it_;

	virtual void visit_members(Visitor& m){
		Base::visit_members(m);
		m & frame_;
	}

public:

	MembersIter(const FramePtr& a)
		:frame_(a), it_(frame_->map_members_){
	}

	AnyPtr reset(){
		it_ = frame_->map_members_;
		return SmartPtr<MembersIter>::from_this(this);
	}

	void iter_next(const VMachinePtr& vm){
		if(frame_->map_members_ && !it_.is_done()){
			vm->return_result(SmartPtr<MembersIter>::from_this(this), it_->key, it_->ns, frame_->members_->at(it_->num));
			++it_;
		}else{
			reset();
			vm->return_result(null);
		}
	}
};

void InitClass(){
	{
		ClassPtr p = new_cpp_class<MembersIter>("ClassMembersIter");
		p->inherit(Iterator());
		p->method("reset", &MembersIter::reset);
		p->method("iter_first", &MembersIter::iter_next);
		p->method("iter_next", &MembersIter::iter_next);
	}

	{
		ClassPtr p = new_cpp_class<Frame>("Frame");
	}

	{
		ClassPtr p = new_cpp_class<Class>("Class");
		p->inherit(get_cpp_class<Frame>());
		p->method("inherit", &Class::inherit_strict);
		p->method("is_inherited", &Class::is_inherited);
		p->method("each_member", &Class::each_member);
		p->method("s_new", &Class::s_new);
		p->method("each_inherited_class", &Class::each_inherited_class);
	}

	{
		ClassPtr p = new_cpp_class<Instance>("Instance");
		p->method("instance_serial_save", &Instance::instance_serial_save);
		p->method("instance_serial_load", &Instance::instance_serial_load);
	}

	{
		ClassPtr p = new_cpp_class<CppClass>("CppClass");
		p->inherit(get_cpp_class<Class>());
	}

	{
		ClassPtr p = new_cpp_class<Lib>("Lib");
		p->inherit(get_cpp_class<Class>());
		p->def("new", ctor<Lib>());
		p->method("append_load_path", &Lib::append_load_path);
	}
}

EmptyHaveInstanceVariables empty_have_instance_variables;
uint_t global_mutate_count = 0;


Instance::Instance(const ClassPtr& c){
	Base::set_class(c);
	pvalue(get_class())->inc_ref_count();
}

Instance::~Instance(){
	pvalue(get_class())->dec_ref_count();
}

void Instance::set_class(const ClassPtr& c){
	pvalue(get_class())->dec_ref_count();
	Base::set_class(c);
	pvalue(get_class())->inc_ref_count();	
}

int_t HaveInstanceVariables::find_core_inner(ClassCore* core){
	for(int_t i = 1, size = (int_t)variables_info_.size(); i<size; ++i){
		if(variables_info_[i].core==core){
			std::swap(variables_info_[0], variables_info_[i]);
			return variables_info_[0].pos;
		}	
	}
	XTAL_THROW(builtin()->member("InstanceVariableError")(Xt("Xtal Runtime Error 1003")), return 0);
}

AnyPtr Instance::instance_serial_save(const ClassPtr& cls){
	Class* p = cls.get();
	Code* code = p->code().get();
	ClassCore* core = p->core();
	if(core->instance_variable_size!=0){	
		int_t pos = find_core(core);

		MapPtr insts(xnew<Map>());
		for(int_t i=0; i<(int_t)core->instance_variable_size; ++i){
			insts->set_at(code->symbol(core->instance_variable_symbol_offset+i), variables_->at(pos+i));
		}

		return insts;
	}
	return null;
}

void Instance::instance_serial_load(const ClassPtr& cls, const AnyPtr& v){
	if(!v)
		return;

	MapPtr insts(cast<MapPtr>(v));

	Class* p = cls.get();
	Code* code = p->code().get();
	ClassCore* core = p->core();
	if(core->instance_variable_size!=0){	
		int_t pos = find_core(core);

		for(int_t i=0; i<(int_t)core->instance_variable_size; ++i){
			variables_->set_at(pos+i, insts->at(code->symbol(core->instance_variable_symbol_offset+i)));
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
	
IdMap::Node* IdMap::find(const InternedStringPtr& key, const AnyPtr& ns){
	Node* p = begin_[rawvalue(key) % size_];
	while(p){
		if(raweq(p->key, key)){
			return p;
		}
		p = p->next;
	}
	return 0;
}

IdMap::Node* IdMap::insert(const InternedStringPtr& key, const AnyPtr& ns){
	Node** p = &begin_[rawvalue(key) % size_];
	while(*p){
		if(raweq((*p)->key, key)){
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
	Node** p = &begin_[rawvalue(node->key) % size_];
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

Frame::Frame(const FramePtr& outer, const CodePtr& code, BlockCore* core)
	:outer_(outer), code_(code), core_(core ? core : &empty_class_core), members_(xnew<Array>(core_->variable_size)), map_members_(0){
	if(debug::is_enabled()){
		make_map_members();	
	}
}

Frame::Frame()
	:outer_(null), code_(null), core_(&empty_class_core), members_(xnew<Array>()), map_members_(0){}
	
Frame::~Frame(){
	if(map_members_){
		map_members_->~IdMap();
		user_free(map_members_, sizeof(IdMap));
	}
}

void Frame::set_class_member(int_t i, const InternedStringPtr& name, int_t accessibility, const AnyPtr& ns, const AnyPtr& value){ 
	members_->set_at(i, value);
	//const InternedStringPtr& name = code_.symbol(core_->variable_symbol_offset+(core_->variable_size-1-i));
	IdMap::Node* p = map_members_->insert(name, ns);
	p->flags = accessibility;
	p->num = i;
	value->set_object_name(name, object_name_force(), FramePtr::from_this(this));
	global_mutate_count++;
}
	
void Frame::set_object_name(const StringPtr& name, int_t force, const AnyPtr& parent){
	if(object_name_force()<force){
		HaveName::set_object_name(name, force, parent);
		if(map_members_){
			for(IdMap::iterator it(map_members_); !it.is_done(); ++it){
				members_->at(it->num)->set_object_name(it->key, force, FramePtr::from_this(this));
			}
		}
	}
}

void Frame::make_map_members(){
	if(!map_members_){
		map_members_ = new(user_malloc(sizeof(IdMap))) IdMap();
	}
}


AnyPtr Frame::each_member(){
	return xnew<MembersIter>(FramePtr::from_this(this));
}


Class::Class(const FramePtr& outer, const CodePtr& code, ClassCore* core)
	:Frame(outer, code, core), mixins_(xnew<Array>()){
	is_cpp_class_ = false;
	make_map_members();
	inherit(get_cpp_class<Instance>());
}

Class::Class(const char* name)
	:Frame(null, null, 0), mixins_(xnew<Array>()){
	is_cpp_class_ = false;
	make_map_members();
}

Class::Class(cpp_class_t, const char* name)
	:Frame(null, null, 0), mixins_(xnew<Array>()){
	is_cpp_class_ = true;
	make_map_members();
}


void Class::inherit(const ClassPtr& md){
	if(is_inherited(md))
		return;

	mixins_->push_back(md);
	global_mutate_count++;
}

void Class::inherit_strict(const ClassPtr& md){
	
	if(md->is_cpp_class_){
		XTAL_THROW(builtin()->member("RuntimeError")(Xt("Xtal Runtime Error 1019")), return);
	}

	if(is_inherited(md))
		return;

	mixins_->push_back(md);
	global_mutate_count++;
}

AnyPtr Class::each_inherited_class(){
	ArrayPtr bases = xnew<Array>();
	for(size_t i=0; i<mixins_->size(); ++i){
		bases->push_back(mixins_->at(i));
	}
	return bases;
}

void Class::init_instance(HaveInstanceVariables* inst, const VMachinePtr& vm, const AnyPtr& self){
	for(int_t i = mixins_->size(); i>0; --i){
		static_ptr_cast<Class>(mixins_->at(i-1))->init_instance(inst, vm, self);
	}
	
	if(core()->instance_variable_size){
		inst->init_variables(core());

		vm->setup_call(0);
		vm->set_arg_this(self);
		// 先頭のメソッドはインスタンス変数初期化関数
		members_->at(0)->call(vm);
		vm->cleanup_call();
	}
}

void Class::def(const InternedStringPtr& name, const AnyPtr& value, int_t accessibility, const AnyPtr& ns){
	IdMap::Node* it = map_members_->find(name, ns);
	if(!it){
		it = map_members_->insert(name, ns);
		it->num = members_->size();
		it->flags = accessibility;
		members_->push_back(value);
		value->set_object_name(name, object_name_force(), ClassPtr::from_this(this));
	}else{
		XTAL_THROW(builtin()->member("RedefinedError")(Xt("Xtal Runtime Error 1011")(Named("object", this->object_name()), Named("name", name))), return);
	}
	global_mutate_count++;
}

const AnyPtr& Class::any_member(const InternedStringPtr& name, const AnyPtr& ns){
	IdMap::Node* it = map_members_->find(name, ns);
	if(it){
		return members_->at(it->num);
	}
	return null;
}

const AnyPtr& Class::bases_member(const InternedStringPtr& name){
	for(int_t i = mixins_->size(); i>0; --i){
		if(const AnyPtr& ret = static_ptr_cast<Class>(mixins_->at(i-1))->member(name)){
			return ret;
		}
	}
	return null;
}

const AnyPtr& Class::member(const InternedStringPtr& name, const AnyPtr& self, const AnyPtr& ns){
	IdMap::Node* it = map_members_->find(name, ns);
	if(it){
		// メンバが見つかった

		// しかしprivateが付けられている
		if(it->flags & KIND_PRIVATE){
			if(raweq(self->get_class(), this)){
				return members_->at(it->num);
			}else{
				// アクセスできない
				XTAL_THROW(builtin()->member("AccessibilityError")(Xt("Xtal Runtime Error 1017")(
					Named("object", this->object_name()), Named("name", name), Named("accessibility", "private")))
				, return null);
			}
		}

		// しかしprotectedが付けられている
		if(it->flags & KIND_PROTECTED){
			if(self->is(ClassPtr::from_this(this))){
				
			}else{
				// アクセスできない
				XTAL_THROW(builtin()->member("AccessibilityError")(Xt("Xtal Runtime Error 1017")(
					Named("object", this->object_name()), Named("name", name), Named("accessibility", "protected")))
				, return null);			
			}
		}

		return members_->at(it->num);
	}
	
	for(int_t i = mixins_->size(); i>0; --i){
		if(const AnyPtr& ret = static_ptr_cast<Class>(mixins_->at(i-1))->member(name, self)){
			return ret;
		}
	}

	return get_cpp_class<Any>()->any_member(name, ns);
}

void Class::set_member(const InternedStringPtr& name, const AnyPtr& value, const AnyPtr& ns){
	IdMap::Node* it = map_members_->find(name, ns);
	if(!it){
		XTAL_THROW(builtin()->member("RuntimeError")("undefined"), return);
	}else{
		members_->set_at(it->num, value);
		//value.set_object_name(name, object_name_force(), this);
	}

	global_mutate_count++;
}

bool Class::is_inherited(const ClassPtr& v){
	if(this==pvalue(v)){
		return true;
	}
	for(int_t i = mixins_->size(); i>0; --i){
		if(static_ptr_cast<Class>(mixins_->at(i-1))->is_inherited(v)){
			return true;
		}
	}
	return raweq(v, get_cpp_class<Any>());
}

void Class::call(const VMachinePtr& vm){;
	InstancePtr inst = InstancePtr::nocount(new Instance(ClassPtr::from_this(this)));
	init_instance(inst.get(), vm, inst);
	
	if(inst.get()->empty()){
		if(const AnyPtr& ret = bases_member(Xid(new))){
			ret->call(vm);
			if(type(vm->result())==TYPE_BASE){
				pvalue(vm->result())->set_class(ClassPtr::from_this(this));
			}
			return;
		}
	}
	
	if(const AnyPtr& ret = member(Xid(initialize), vm->ff().self(), null)){
		vm->set_arg_this(inst);
		if(vm->need_result()){
			ret->call(vm);
			vm->replace_result(0, inst);
		}else{
			ret->call(vm);
		}
	}else{
		vm->return_result(inst);
	}
}

void Class::s_new(const VMachinePtr& vm){
	InstancePtr inst = InstancePtr::nocount(new Instance(ClassPtr::from_this(this)));
	init_instance(inst.get(), vm, inst);
	
	if(inst.get()->empty()){
		if(const AnyPtr& ret = bases_member(Xid(serial_new))){
			ret->call(vm);
			if(type(vm->result())==TYPE_BASE){
				pvalue(vm->result())->set_class(ClassPtr::from_this(this));
			}
			return;
		}
	}

	vm->return_result(inst);
}

CppClass::CppClass(const char* name)
	:Class(cpp_class_t(), name){
}

void CppClass::call(const VMachinePtr& vm){
	if(const AnyPtr& ret = member(Xid(new), ClassPtr::from_this(this), null)){
		ret->call(vm);
	}else{
		XTAL_THROW(builtin()->member("RuntimeError")(Xt("Xtal Runtime Error 1013")(object_name())), return);
	}
}

void CppClass::s_new(const VMachinePtr& vm){
	if(const AnyPtr& ret = member(Xid(serial_new), ClassPtr::from_this(this), null)){
		ret->call(vm);
	}else{
		XTAL_THROW(builtin()->member("RuntimeError")(Xt("Xtal Runtime Error 1013")(object_name())), return);
	}
}

Lib:: Lib(){
	set_object_name(Xid(lib), 1000, null);
	load_path_list_ = xnew<Array>();
	path_ = xnew<Array>();
}

Lib::Lib(const ArrayPtr& path)
	:path_(path){
	load_path_list_ = xnew<Array>();
}

const AnyPtr& Lib::member(const InternedStringPtr& name, const AnyPtr& self, const AnyPtr& ns){
	IdMap::Node* it = map_members_->find(name, ns);
	if(it){
		return members_->at(it->num);
	}else{
		Xfor(var, load_path_list_->each()){
			StringPtr file_name = Xf("%s%s%s%s")(var, join_path("/"), name, ".xtal")->to_s();
			if(FILE* fp = fopen(file_name->c_str(), "r")){
				fclose(fp);
				return rawdef(name, load(file_name), ns);
			}
		}
		return null;

		/* 指定した名前をフォルダーとみなす
		ArrayPtr next = path_.clone();
		next.push_back(name);
		AnyPtr lib = xnew<Lib>(next);
		return rawdef(name, lib, ns);
		*/
	}
}

void Lib::def(const InternedStringPtr& name, const AnyPtr& value, int_t accessibility, const AnyPtr& ns){
	rawdef(name, value, ns);
}

const AnyPtr& Lib::rawdef(const InternedStringPtr& name, const AnyPtr& value, const AnyPtr& ns){
	IdMap::Node* it = map_members_->find(name, ns);
	if(!it){
		it = map_members_->insert(name, ns);
		it->num = members_->size();
		it->flags = KIND_PUBLIC;
		members_->push_back(value);
		global_mutate_count++;
		value->set_object_name(name, object_name_force(), LibPtr::from_this(this));
		return members_->back();
	}else{
		XTAL_THROW(builtin()->member("RedefinedError")(Xt("Xtal Runtime Error 1011")(Named("object", this->object_name()), Named("name", name))), return null);
	}
}

StringPtr Lib::join_path(const StringPtr& sep){
	if(path_->empty()){
		return sep;
	}else{
		return sep->cat(path_->join(sep))->cat(sep);
	}
}
	

}
