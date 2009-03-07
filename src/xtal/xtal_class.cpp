#include "xtal.h"
#include "xtal_macro.h"

namespace xtal{

InstanceVariables::InstanceVariables()		
	:variables_(0){
	VariablesInfo vi;
	vi.class_info = 0;
	vi.pos = 0;
	variables_info_.push(vi);
}
		
InstanceVariables::~InstanceVariables(){}
	
void InstanceVariables::init_variables(ClassInfo* class_info){
	if(class_info->instance_variable_size){
		VariablesInfo vi;
		vi.class_info = class_info;
		vi.pos = (int_t)variables_.size();
		variables_.upsize(class_info->instance_variable_size);
		variables_info_.push(vi);
	}
}

bool InstanceVariables::is_included(ClassInfo* class_info){
	VariablesInfo& info = variables_info_.top();
	if(info.class_info == class_info){
		return true;
	}
	for(int_t i = 1, size = (int_t)variables_info_.size(); i<size; ++i){
		if(variables_info_[i].class_info==class_info){
			std::swap(variables_info_[0], variables_info_[i]);
			return true;
		}	
	}
	return false;
}

int_t InstanceVariables::find_class_info_inner(ClassInfo* class_info){
	for(int_t i = 1, size = (int_t)variables_info_.size(); i<size; ++i){
		if(variables_info_[i].class_info==class_info){
			std::swap(variables_info_[0], variables_info_[i]);
			return variables_info_[0].pos;
		}	
	}
	XTAL_SET_EXCEPT(builtin()->member(Xid(InstanceVariableError))->call(Xt("Xtal Runtime Error 1003")));
	return -1;
}

EmptyInstanceVariables::EmptyInstanceVariables()
	:InstanceVariables(uninit_t()){
	vi.class_info = 0;
	vi.pos = 0;
	variables_info_.attach(&vi);
}

EmptyInstanceVariables::~EmptyInstanceVariables(){
	variables_info_.detach();
}

InstanceVariables::VariablesInfo EmptyInstanceVariables::vi;

///////////////////////////////////////

Class::Class(const FramePtr& outer, const CodePtr& code, ClassInfo* info)
	:Frame(outer, code, info), mixins_(xnew<Array>()){
	object_force_ = 0;
	is_native_ = false;
	is_final_ = false;
	make_map_members();
}

Class::Class(const StringPtr& name)
	:Frame(null, null, 0), mixins_(xnew<Array>()){
	set_object_name(name);
	object_force_ = 0;
	is_native_ = false;
	is_final_ = false;
	make_map_members();
}

Class::Class(cpp_class_t, const StringPtr& name)
	:Frame(null, null, 0), mixins_(xnew<Array>()){
	set_object_name(name);
	object_force_ = 0;
	is_native_ = true;
	is_final_ = false;
	make_map_members();
}

void Class::overwrite(const ClassPtr& p){
	if(!is_native_){
		operator=(*p);
		mixins_ = p->mixins_;
	}
}

void Class::inherit(const ClassPtr& cls){
	if(is_inherited(cls))
		return;

	XTAL_ASSERT(cls);
	
	mixins_->push_back(cls);
	invalidate_cache_is();
}

void Class::inherit_first(const ClassPtr& cls){
	if(cls->is_native()){
		if(is_inherited_cpp_class()){
			XTAL_SET_EXCEPT(RuntimeError()->call(Xt("Xtal Runtime Error 1019")));
			return;
		}
	}

	if(cls->is_final()){
		XTAL_SET_EXCEPT(RuntimeError()->call(Xt("Xtal Runtime Error 1028")->call(Named(Xid(name), cls->object_name()))));
		return;
	}

	if(is_inherited(cls)){
		return;
	}

	XTAL_ASSERT(cls);

	mixins_->push_back(cls);
	invalidate_cache_is();
}

void Class::inherit_strict(const ClassPtr& cls){
	if(cls->is_native()){
		XTAL_SET_EXCEPT(RuntimeError()->call(Xt("Xtal Runtime Error 1029")->call(Named(Xid(name), cls->object_name()))));
	}

	inherit_first(cls);
}

AnyPtr Class::inherited_classes(){
	return mixins_->each();
}

void Class::init_instance(const AnyPtr& self, const VMachinePtr& vm){
	for(int_t i = mixins_->size(); i>0; --i){
		unchecked_ptr_cast<Class>(mixins_->at(i-1))->init_instance(self, vm);
	}
	
	if(info()->instance_variable_size){
		pvalue(self)->make_instance_variables();
		pvalue(self)->instance_variables()->init_variables(info());

		// 先頭のメソッドはインスタンス変数初期化関数
		if(members_->at(0)){
			vm->setup_call(0);
			vm->set_arg_this(self);
			members_->at(0)->rawcall(vm);
			vm->cleanup_call();
		}
	}
}

IDPtr Class::find_near_member(const IDPtr& primary_key, const AnyPtr& secondary_key){
	int_t minv = 0xffffff;
	IDPtr minid = null;
	Xfor_cast(const MultiValuePtr& v, send(Xid(members))){
		IDPtr id = ptr_cast<ID>(v->at(0));
		if(raweq(primary_key, id)){
			return id;
		}

		int_t dist = edit_distance(primary_key, id);
		if(dist<minv){
			minv = dist;
			minid = id;
		}
	}

	return minid;
}

void Class::def_double_dispatch_method(const IDPtr& primary_key, int_t accessibility){
	def(primary_key, xtal::double_dispatch_method(primary_key), null, accessibility);
}

void Class::def_double_dispatch_fun(const IDPtr& primary_key, int_t accessibility){
	def(primary_key, xtal::double_dispatch_fun(from_this(this), primary_key), null, accessibility);
}

const NativeFunPtr& Class::def_and_return(const IDPtr& primary_key, const AnyPtr& secondary_key, int_t accessibility, const param_types_holder_n& pth, const void* val, int_t val_size){
	return unchecked_ptr_cast<NativeFun>(def2(primary_key, xnew<NativeFun>(pth, val, val_size), secondary_key, accessibility));
}

const NativeFunPtr& Class::def_and_return_bind_this(const IDPtr& primary_key, const AnyPtr& secondary_key, int_t accessibility, const param_types_holder_n& pth, const void* val, int_t val_size){
	return unchecked_ptr_cast<NativeFun>(def2(primary_key, xnew<NativeFunBindedThis>(pth, val, val_size, from_this(this)), secondary_key, accessibility));
}

const AnyPtr& Class::def2(const IDPtr& primary_key, const AnyPtr& value, const AnyPtr& secondary_key, int_t accessibility){
	def(primary_key, value, secondary_key, accessibility);
	Key key = {primary_key, secondary_key};
	map_t::iterator it = map_members_->find(key);
	if(it!=map_members_->end()){
		return members_->at(it->second.num);
	}
	return null;
}

void Class::def(const IDPtr& primary_key, const AnyPtr& value, const AnyPtr& secondary_key, int_t accessibility){
	Key key = {primary_key, secondary_key};
	map_t::iterator it = map_members_->find(key);
	if(it==map_members_->end()){
		Value val = {members_->size(), accessibility};
		map_members_->insert(key, val);
		members_->push_back(value);
		value->set_object_parent(from_this(this));
		invalidate_cache_member();
	}
	else{
		XTAL_SET_EXCEPT(builtin()->member(Xid(RedefinedError))->call(Xt("Xtal Runtime Error 1011")->call(Named(Xid(object), this->object_name()), Named(Xid(name), primary_key))));
	}
}

const AnyPtr& Class::any_member(const IDPtr& primary_key, const AnyPtr& secondary_key){
	Key key = {primary_key, secondary_key};
	map_t::iterator it = map_members_->find(key);
	if(it!=map_members_->end()){
		return members_->at(it->second.num);
	}
	return undefined;
}

const AnyPtr& Class::bases_member(const IDPtr& name){
	for(int_t i = mixins_->size(); i>0; --i){
		if(const AnyPtr& ret = unchecked_ptr_cast<Class>(mixins_->at(i-1))->member(name)){
			return ret;
		}
	}
	return undefined;
}

const AnyPtr& Class::find_member(const IDPtr& primary_key, const AnyPtr& secondary_key, bool inherited_too, int_t& accessibility, bool& nocache){
	Key key = {primary_key, secondary_key};
	map_t::iterator it = map_members_->find(key);

	if(it!=map_members_->end()){
		// メンバが見つかった
		
		accessibility = it->second.flags & 0x3;
		return members_->at(it->second.num);
	}
	
	// 継承しているクラスを順次検索
	if(inherited_too){
		for(int_t i=0, sz=mixins_->size(); i<sz; ++i){
			const AnyPtr& ret = unchecked_ptr_cast<Class>(mixins_->at(i))->do_member(primary_key, secondary_key, true, accessibility, nocache);
			if(rawne(ret, undefined)){
				return ret;
			}
		}
	}

	return undefined;
}

const AnyPtr& Class::do_member(const IDPtr& primary_key, const AnyPtr& secondary_key, bool inherited_too, int_t& accessibility, bool& nocache){
	{
		const AnyPtr& ret = find_member(primary_key, secondary_key, inherited_too, accessibility, nocache);
		if(rawne(ret, undefined)){
			return ret;
		}

		//XTAL_CHECK_EXCEPT(e){ return undefined; }
	}
		
	{
		const AnyPtr& ret = cpp_class<Any>()->any_member(primary_key, secondary_key);
		if(rawne(ret, undefined)){
			return ret;
		}

		//XTAL_CHECK_EXCEPT(e){ return undefined; }
	}

	// 見つからなかった。

	// もしsecond keyがクラスの場合、スーパークラスをsecond keyに変え、順次試していく
	if(const ClassPtr& klass = ptr_cast<Class>(secondary_key)){
		for(int_t i=0, sz=klass->mixins_->size(); i<sz; ++i){
			const AnyPtr& ret = do_member(primary_key, klass->mixins_->at(i), inherited_too, accessibility, nocache);
			if(rawne(ret, undefined)){
				return ret;
			}
		}

		//XTAL_CHECK_EXCEPT(e){ return undefined; }

		if(rawne(cpp_class<Any>(), klass)){
			const AnyPtr& ret = do_member(primary_key, cpp_class<Any>(), inherited_too, accessibility, nocache);
			if(rawne(ret, undefined)){
				return ret;
			}

			//XTAL_CHECK_EXCEPT(e){ return undefined; }
		}	
	}

	// やっぱり見つからなかった。
	return undefined;
}

void Class::set_member(const IDPtr& primary_key, const AnyPtr& value, const AnyPtr& secondary_key){
	Key key = {primary_key, secondary_key};
	map_t::iterator it = map_members_->find(key);
	if(it==map_members_->end()){
		XTAL_SET_EXCEPT(RuntimeError()->call(Xid(undefined)));
		return;
	}
	else{
		members_->set_at(it->second.num, value);
		//value.set_object_name(name, object_name_force(), this);
	}

	invalidate_cache_member();
}

void Class::set_member_direct(int_t i, const IDPtr& primary_key, const AnyPtr& value, const AnyPtr& secondary_key, int_t accessibility){ 
	members_->set_at(i, value);
	Key key = {primary_key, secondary_key};
	Value val = {i, accessibility};
	map_members_->insert(key, val);
	value->set_object_parent(from_this(this));
	invalidate_cache_member();
}

void Class::set_object_parent(const ClassPtr& parent){
	if(object_force_<parent->object_force()){
		object_force_ = parent->object_force()-1;
		HaveParent::set_object_parent(parent);
		if(map_members_){
			for(map_t::iterator it=map_members_->begin(), last=map_members_->end(); it!=last; ++it){
				members_->at(it->second.num)->set_object_parent(from_this(this));
			}
		}
	}
}

MultiValuePtr Class::child_object_name(const AnyPtr& a){
	if(map_members_){
		for(map_t::iterator it=map_members_->begin(), last=map_members_->end(); it!=last; ++it){
			if(raweq(members_->at(it->second.num), a)){
				return mv(it->first.primary_key, it->first.secondary_key);
			}
		}
	}
	return null;
}

StringPtr Class::object_name(){
	if(const ClassPtr& parent = object_parent()){
		if(MultiValuePtr myname = parent->child_object_name(ap(*this))){
			if(raweq(myname->at(1), null)){
				return Xf("%s::%s")->call(parent->object_name(), myname->at(0))->to_s();
			}
			else{
				return Xf("%s::%s#%s")->call(parent->object_name(), myname->at(0), myname->at(1))->to_s();
			}
		}
	}

	return name_;
}

void Class::set_object_name(const StringPtr& name){
	name_ = name;
}

bool Class::is_inherited(const AnyPtr& v){
	if(this==pvalue(v)){
		return true;
	}

	for(int_t i=0, sz=mixins_->size(); i<sz; ++i){
		if(mixins_->at(i)->is_inherited(v)){
			return true;
		}
	}

	return raweq(v, cpp_class<Any>());
}

bool Class::is_inherited_cpp_class(){
	if(is_native()){
		return true;
	}

	for(int_t i=0, sz=mixins_->size(); i<sz; ++i){
		if(unchecked_ptr_cast<Class>(mixins_->at(i))->is_inherited_cpp_class()){
			return true;
		}
	}

	return false;
}

void Class::rawcall(const VMachinePtr& vm){
	const AnyPtr& newfun = bases_member(Xid(new));
	
	XTAL_CHECK_EXCEPT(e){ return; }

	AnyPtr instance;
	if(newfun){
		instance = newfun->call();
	}
	else{
		instance = xnew<Base>();
	}

	XTAL_CHECK_EXCEPT(e){ return; }

	if(type(instance)==TYPE_BASE){
		pvalue(instance)->set_class(from_this(this));
	}

	init_instance(instance, vm);

	XTAL_CHECK_EXCEPT(e){ return; }
	
	if(const AnyPtr& ret = member(Xid(initialize), null, vm->ff().self())){
		vm->set_arg_this(instance);
		if(vm->need_result()){
			vm->prereturn_result(instance);
			ret->rawcall(vm);
		}
		else{
			ret->rawcall(vm);
		}
	}
	else{
		vm->return_result(instance);
	}
}

void Class::s_new(const VMachinePtr& vm){
	const AnyPtr& newfun = bases_member(Xid(serial_new));
	AnyPtr instance;
	if(newfun){
		instance = newfun->call();
	}
	else{
		instance = xnew<Base>();
	}

	if(type(instance)==TYPE_BASE){
		pvalue(instance)->set_class(from_this(this));
		init_instance(instance, vm);
	}

	vm->return_result(instance);
}

AnyPtr Class::ancestors(){
	if(raweq(from_this(this), cpp_class<Any>())){
		return null;
	}			
	
	ArrayPtr ret = xnew<Array>();
	Xfor_cast(const ClassPtr& it, inherited_classes()){
		ret->push_back(it);

		Xfor(it2, it->ancestors()){
			ret->push_back(it2);
		}
	}

	ret->push_back(cpp_class<Any>());
	return ret;
}

CppClass::CppClass(const StringPtr& name)
	:Class(cpp_class_t(), name){
}

void CppClass::rawcall(const VMachinePtr& vm){
	if(const AnyPtr& ret = member(Xid(new), null, from_this(this))){
		ret->rawcall(vm);
		if(vm->except()){
			return;
		}
		init_instance(vm->result(), vm);
	}
	else{
		vm->set_except(RuntimeError()->call(Xt("Xtal Runtime Error 1013")->call(object_name())));
	}
}

void CppClass::s_new(const VMachinePtr& vm){
	if(const AnyPtr& ret = member(Xid(serial_new), null, from_this(this))){
		ret->rawcall(vm);
		init_instance(vm->result(), vm);
	}
	else{
		vm->set_except(RuntimeError()->call(Xt("Xtal Runtime Error 1013")->call(object_name())));
	}
}

Singleton::Singleton(const StringPtr& name)
	:Class(name){
	Base::set_class(from_this(this));
	inherit(cpp_class<Class>());
}

Singleton::Singleton(const FramePtr& outer, const CodePtr& code, ClassInfo* info)
	:Class(outer, code, info){
	Base::set_class(from_this(this));
	inherit(cpp_class<Class>());
}

void Singleton::init_singleton(const VMachinePtr& vm){;
	SingletonPtr instance = from_this(this);
	init_instance(instance, vm);
	
	if(const AnyPtr& ret = member(Xid(initialize), null, vm->ff().self())){
		vm->setup_call(0);
		vm->set_arg_this(instance);
		ret->rawcall(vm);
		vm->cleanup_call();
	}
}

void Singleton::rawcall(const VMachinePtr& vm){
	ap(Any(this))->rawsend(vm, Xid(op_call));
}

void Singleton::s_new(const VMachinePtr& vm){
	XTAL_SET_EXCEPT(RuntimeError()->call(Xt("Xtal Runtime Error 1013")->call(object_name())));
}

CppSingleton::CppSingleton(const StringPtr& name)
	:Class(name){
	Base::set_class(from_this(this));
	inherit(cpp_class<Class>());
}


}
