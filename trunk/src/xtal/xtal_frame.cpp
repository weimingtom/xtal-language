
#include "xtal.h"

#include <algorithm>

#include "xtal_frame.h"
#include "xtal_vmachine.h"
#include "xtal_code.h"
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
			vm->return_result(SmartPtr<MembersIter>::from_this(this), it_->first.key, it_->first.ns, frame_->members_->at(it_->second.num));
			++it_;
		}else{
			vm->return_result(null);
		}
	}
};

EmptyInstanceVariables empty_instance_variables;
uint_t global_mutate_count = 0;

void initialize_frame(){
	{
		ClassPtr p = new_cpp_class<MembersIter>("ClassMembersIter");
		p->inherit(Iterator());
		p->method("block_next", &MembersIter::block_next);
	}

	{
		ClassPtr p = new_cpp_class<Frame>("Frame");
	}

	{
		ClassPtr p = new_cpp_class<Class>("Class");
		p->inherit(get_cpp_class<Frame>());
		p->method("inherit", &Class::inherit_strict);
		p->method("is_inherited", &Class::is_inherited);
		p->method("members", &Class::members);
		p->method("s_new", &Class::s_new);
		p->method("inherited_classes", &Class::inherited_classes);
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

	builtin()->def("Class", get_cpp_class<Class>());
	builtin()->def("CppClass", get_cpp_class<CppClass>());
	builtin()->def("lib", lib());
	builtin()->def("Lib", get_cpp_class<Lib>());
}

int_t InstanceVariables::find_core_inner(ClassCore* core){
	for(int_t i = 1, size = (int_t)variables_info_.size(); i<size; ++i){
		if(variables_info_[i].core==core){
			std::swap(variables_info_[0], variables_info_[i]);
			return variables_info_[0].pos;
		}	
	}
	XTAL_THROW(builtin()->member("InstanceVariableError")(Xt("Xtal Runtime Error 1003")), return 0);
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
		map_members_->~Hashtable();
		user_free(map_members_);
	}
}

void Frame::set_class_member(int_t i, const InternedStringPtr& name, const AnyPtr& value, const AnyPtr& ns, int_t accessibility){ 
	members_->set_at(i, value);
	Key key = {name, ns};
	Value val = {i, accessibility};
	map_members_->insert(key, val);
	value->set_object_name(name, object_name_force(), FramePtr::from_this(this));
	global_mutate_count++;
}
	
void Frame::set_object_name(const StringPtr& name, int_t force, const AnyPtr& parent){
	if(object_name_force()<force){
		HaveName::set_object_name(name, force, parent);
		if(map_members_){
			for(map_t::iterator it=map_members_->begin(), last=map_members_->end(); it!=last; ++it){
				members_->at(it->second.num)->set_object_name(it->first.key, force, FramePtr::from_this(this));
			}
		}
	}
}

void Frame::make_map_members(){
	if(!map_members_){
		map_members_ = new(user_malloc(sizeof(map_t))) map_t();
	}
}

StringPtr Frame::object_name(){
	if(!name_){
		set_object_name(ptr_cast<String>(Xf("<(%s):%s:%d>")(get_class()->object_name(), code_->source_file_name(), code_->compliant_lineno(code_->data()+core_->pc))), 1, parent_);
	}

	return HaveName::object_name();
}

AnyPtr Frame::members(){
	return xnew<MembersIter>(FramePtr::from_this(this));
}


Class::Class(const FramePtr& outer, const CodePtr& code, ClassCore* core)
	:Frame(outer, code, core), mixins_(xnew<Array>()){
	is_cpp_class_ = false;
	make_map_members();
}

Class::Class(const char* name)
	:Frame(null, null, 0), mixins_(xnew<Array>()){
	set_object_name(name, 1, null);
	is_cpp_class_ = false;
	make_map_members();
}

Class::Class(cpp_class_t, const char* name)
	:Frame(null, null, 0), mixins_(xnew<Array>()){
	set_object_name(name, 1, null);
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
	
	if(md->is_cpp_class() && is_inherited_cpp_class()){
		XTAL_THROW(builtin()->member("RuntimeError")(Xt("Xtal Runtime Error 1019")), return);
	}

	if(is_inherited(md))
		return;

	mixins_->push_back(md);
	global_mutate_count++;
}

AnyPtr Class::inherited_classes(){
	return mixins_->each();
}

void Class::init_instance(const AnyPtr& self, const VMachinePtr& vm){
	for(int_t i = mixins_->size(); i>0; --i){
		static_ptr_cast<Class>(mixins_->at(i-1))->init_instance(self, vm);
	}
	
	if(core()->instance_variable_size){
		pvalue(self)->make_instance_variables();
		pvalue(self)->instance_variables()->init_variables(core());

		vm->setup_call(0);
		vm->set_arg_this(self);
		// 先頭のメソッドはインスタンス変数初期化関数
		members_->at(0)->call(vm);
		vm->cleanup_call();
	}
}

void Class::def(const InternedStringPtr& name, const AnyPtr& value, const AnyPtr& ns, int_t accessibility){
	Key key = {name, ns};
	map_t::iterator it = map_members_->find(key);
	if(it==map_members_->end()){
		Value val = {members_->size(), accessibility};
		map_members_->insert(key, val);
		members_->push_back(value);
		value->set_object_name(name, object_name_force(), ClassPtr::from_this(this));
	}else{
		XTAL_THROW(builtin()->member("RedefinedError")(Xt("Xtal Runtime Error 1011")(Named("object", this->object_name()), Named("name", name))), return);
	}
	global_mutate_count++;
}

const AnyPtr& Class::any_member(const InternedStringPtr& name, const AnyPtr& ns){
	Key key = {name, ns};
	map_t::iterator it = map_members_->find(key);
	if(it!=map_members_->end()){
		return members_->at(it->second.num);
	}
	return nop;
}

const AnyPtr& Class::bases_member(const InternedStringPtr& name){
	for(int_t i = mixins_->size(); i>0; --i){
		if(const AnyPtr& ret = static_ptr_cast<Class>(mixins_->at(i-1))->member(name)){
			return ret;
		}
	}
	return nop;
}

const AnyPtr& Class::do_member(const InternedStringPtr& name, const AnyPtr& ns, const AnyPtr& self, bool inherited_too){
	Key key = {name, ns};
	map_t::iterator it = map_members_->find(key);
	if(it!=map_members_->end()){
		// メンバが見つかった

		// しかしprivateが付けられている
		if(it->second.flags & KIND_PRIVATE){
			if(raweq(self->get_class(), this)){
				return members_->at(it->second.num);
			}else{
				// アクセスできない
				XTAL_THROW(builtin()->member("AccessibilityError")(Xt("Xtal Runtime Error 1017")(
					Named("object", this->object_name()), Named("name", name), Named("accessibility", "private")))
				, return nop);
			}
		}

		// しかしprotectedが付けられている
		if(it->second.flags & KIND_PROTECTED){
			if(self->is(ClassPtr::from_this(this))){
				
			}else{
				// アクセスできない
				XTAL_THROW(builtin()->member("AccessibilityError")(Xt("Xtal Runtime Error 1017")(
					Named("object", this->object_name()), Named("name", name), Named("accessibility", "protected")))
				, return nop);			
			}
		}

		return members_->at(it->second.num);
	}
	
	// 継承しているクラスを順次検索
	if(inherited_too){
		for(int_t i = mixins_->size(); i>0; --i){
			if(const AnyPtr& ret = static_ptr_cast<Class>(mixins_->at(i-1))->member(name, ns, self)){
				return ret;
			}
		}

		return get_cpp_class<Any>()->any_member(name, ns);
	}
	return nop;
}

void Class::set_member(const InternedStringPtr& name, const AnyPtr& value, const AnyPtr& ns){
	Key key = {name, ns};
	map_t::iterator it = map_members_->find(key);
	if(it==map_members_->end()){
		XTAL_THROW(builtin()->member("RuntimeError")("undefined"), return);
	}else{
		members_->set_at(it->second.num, value);
		//value.set_object_name(name, object_name_force(), this);
	}

	global_mutate_count++;
}

bool Class::is_inherited(const AnyPtr& v){
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

bool Class::is_inherited_cpp_class(){
	if(is_cpp_class()){
		return true;
	}

	for(int_t i = mixins_->size(); i>0; --i){
		if(static_ptr_cast<Class>(mixins_->at(i-1))->is_inherited_cpp_class()){
			return true;
		}
	}

	return false;
}

void Class::call(const VMachinePtr& vm){
	const AnyPtr& newfun = bases_member(Xid(new));
	AnyPtr instance;
	if(newfun){
		instance = newfun();
	}else{
		instance = xnew<Base>();
		pvalue(instance)->set_xtal_instance_flag();
	}

	pvalue(instance)->set_class(ClassPtr::from_this(this));
	init_instance(instance, vm);
	
	if(const AnyPtr& ret = member(Xid(initialize), null, vm->ff().self())){
		vm->set_arg_this(instance);
		if(vm->need_result()){
			ret->call(vm);
			vm->replace_result(0, instance);
		}else{
			ret->call(vm);
		}
	}else{
		vm->return_result(instance);
	}
}

void Class::s_new(const VMachinePtr& vm){
	const AnyPtr& newfun = bases_member(Xid(serial_new));
	AnyPtr instance;
	if(newfun){
		instance = newfun();
	}else{
		instance = xnew<Base>();
		pvalue(instance)->set_xtal_instance_flag();
	}

	pvalue(instance)->set_class(ClassPtr::from_this(this));
	init_instance(instance, vm);

	vm->return_result(instance);
}

CppClass::CppClass(const char* name)
	:Class(cpp_class_t(), name){
}

void CppClass::call(const VMachinePtr& vm){
	if(const AnyPtr& ret = member(Xid(new), null, ClassPtr::from_this(this))){
		ret->call(vm);
		init_instance(vm->result(), vm);
	}else{
		XTAL_THROW(builtin()->member("RuntimeError")(Xt("Xtal Runtime Error 1013")(object_name())), return);
	}
}

void CppClass::s_new(const VMachinePtr& vm){
	if(const AnyPtr& ret = member(Xid(serial_new), null, ClassPtr::from_this(this))){
		ret->call(vm);
		init_instance(vm->result(), vm);
	}else{
		XTAL_THROW(builtin()->member("RuntimeError")(Xt("Xtal Runtime Error 1013")(object_name())), return);
	}
}

Lib::Lib(){
	set_object_name("lib", 1000, null);
	load_path_list_ = xnew<Array>();
	path_ = xnew<Array>();
}

Lib::Lib(const ArrayPtr& path)
	:path_(path){
	load_path_list_ = xnew<Array>();
}

const AnyPtr& Lib::do_member(const InternedStringPtr& name, const AnyPtr& ns, const AnyPtr& self, bool inherited_too){
	Key key = {name, ns};
	map_t::iterator it = map_members_->find(key);
	if(it!=map_members_->end()){
		return members_->at(it->second.num);
	}else{
		Xfor(var, load_path_list_){
			StringPtr file_name = Xf("%s%s%s%s")(var, join_path("/"), name, ".xtal")->to_s();
			if(FILE* fp = fopen(file_name->c_str(), "r")){
				fclose(fp);
				return rawdef(name, load(file_name), ns);
			}
		}
		return nop;

		/* 指定した名前をフォルダーとみなす
		ArrayPtr next = path_.clone();
		next.push_back(name);
		AnyPtr lib = xnew<Lib>(next);
		return rawdef(name, lib, ns);
		*/
	}
}

void Lib::def(const InternedStringPtr& name, const AnyPtr& value, const AnyPtr& ns, int_t accessibility){
	rawdef(name, value, ns);
}

const AnyPtr& Lib::rawdef(const InternedStringPtr& name, const AnyPtr& value, const AnyPtr& ns){
	Key key = {name, ns};
	map_t::iterator it = map_members_->find(key);
	if(it==map_members_->end()){
		Value val = {members_->size(), KIND_PUBLIC};
		map_members_->insert(key, val);
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

Singleton::Singleton(const char* name)
	:Class(name){
	Base::set_class(ClassPtr::from_this(this));
	inherit(get_cpp_class<Class>());
}

Singleton::Singleton(const FramePtr& outer, const CodePtr& code, ClassCore* core)
	:Class(outer, code, core){
	Base::set_class(ClassPtr::from_this(this));
	inherit(get_cpp_class<Class>());
}

void Singleton::init_singleton(const VMachinePtr& vm){;
	SingletonPtr instance = SingletonPtr::from_this(this);
	init_instance(instance, vm);
	
	if(const AnyPtr& ret = member(Xid(initialize), null, vm->ff().self())){
		vm->setup_call(0);
		vm->set_arg_this(instance);
		ret->call(vm);
		vm->cleanup_call();
	}
}

void Singleton::call(const VMachinePtr& vm){
	ap(Innocence(this))->rawsend(vm, Xid(op_call));
}

void Singleton::s_new(const VMachinePtr& vm){
	XTAL_THROW(builtin()->member("RuntimeError")(Xt("Xtal Runtime Error 1013")(object_name())), return);
}

}
