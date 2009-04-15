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
	for(uint_t i = 1, size = variables_info_.size(); i<size; ++i){
		if(variables_info_[i].class_info==class_info){
			std::swap(variables_info_[0], variables_info_[i]);
			return true;
		}	
	}
	return false;
}

int_t InstanceVariables::find_class_info_inner(ClassInfo* class_info){
	for(uint_t i = 1, size = variables_info_.size(); i<size; ++i){
		if(variables_info_[i].class_info==class_info){
			std::swap(variables_info_[0], variables_info_[i]);
			return variables_info_[0].pos;
		}	
	}
	XTAL_SET_EXCEPT(cpp_class<InstanceVariableError>()->call(Xt("Xtal Runtime Error 1003")));
	return -1;
}

void InstanceVariables::replace(ClassInfo* from, ClassInfo* to){
	for(uint_t i = 0, size = variables_info_.size(); i<size; ++i){
		if(variables_info_[i].class_info==from){
			int_t pos = variables_info_[i].pos;
			variables_info_.erase(i, 1);
			for(uint_t j = 0, jsize = variables_info_.size(); j<jsize; ++j){
				if(variables_info_[i].pos>pos){
					variables_info_[i].pos -= from->instance_variable_size;
				}
			}
			variables_.erase(pos, from->instance_variable_size);
			init_variables(to);
			return;
		}	
	}
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
	:Frame(outer, code, info){
	set_object_name(empty_string);
	object_force_ = 0;
	prebinder_ = 0;
	binder_ = 0;
	flags_ = 0;
	make_map_members();
}

Class::Class(const StringPtr& name)
	:Frame(null, null, 0){
	set_object_name(name);
	object_force_ = 0;
	prebinder_ = 0;
	binder_ = 0;
	flags_ = 0;
	make_map_members();
}

Class::Class(cpp_class_t)
	:Frame(null, null, 0){
	set_object_name(empty_string);
	object_force_ = 0;
	prebinder_ = 0;
	binder_ = 0;
	flags_ = FLAG_NATIVE;
	make_map_members();
}

Class::~Class(){
	for(int_t i=0, sz=inherited_classes_.size(); i<sz; ++i){
		inherited_classes_[i]->dec_ref_count();
	}
}

void Class::overwrite(const ClassPtr& p){
	if(!is_native() && !p->is_native()){
		for(int_t i=0, sz=p->inherited_classes_.size(); i<sz; ++i){
			inherit(to_smartptr(p->inherited_classes_[i]));
		}		
		
		if(p->map_members_){
			for(map_t::iterator it=p->map_members_->begin(), last=p->map_members_->end(); it!=last; ++it){
				overwrite_member(it->first.primary_key, p->members_.at(it->second.num), it->first.secondary_key, it->second.flags);
			}
		}

		for(uint_t i=0; i<alive_object_count(); ++i){
			AnyPtr obj = alive_object(i);
			if(type(obj)==TYPE_BASE){
				if(obj->is(to_smartptr(this))){
					AnyPtr data = obj->serial_save(to_smartptr(this));
					if(pvalue(obj)->instance_variables()){
						pvalue(obj)->instance_variables()->replace((ClassInfo*)scope_info_, (ClassInfo*)p->scope_info_);
					}
					obj->serial_load(p, data);

					{
						Key key = {Xid(reloaded), undefined};
						map_t::iterator it = map_members_->find(key);
						if(it!=map_members_->end()){
							const VMachinePtr& vm = vmachine();
							vm->setup_call(0);
							vm->set_arg_this(obj);
							members_.at(it->second.num)->rawcall(vm);
							vm->cleanup_call();
						}
					}
				}
			}
		}

		outer_ = p->outer_;
		code_ = p->code_;
		scope_info_ = p->scope_info_;
	}
}

void Class::inherit(const ClassPtr& cls){
	if(is_inherited(cls))
		return;

	XTAL_ASSERT(cls);

	cls->prebind();

	inherited_classes_.push_back(cls.get());
	cls.get()->inc_ref_count();
	invalidate_cache_is();
}

void Class::inherit_first(const ClassPtr& cls){
	if(cls->is_singleton()){
		if(!is_singleton()){
			XTAL_SET_EXCEPT(cpp_class<RuntimeError>()->call(Xt("Xtal Runtime Error 1031")));
			return;
		}
	}
	else{
		if(cls->is_native()){
			if(is_inherited_cpp_class()){
				XTAL_SET_EXCEPT(cpp_class<RuntimeError>()->call(Xt("Xtal Runtime Error 1019")));
				return;
			}
		}
	}

	if(cls->is_final()){
		XTAL_SET_EXCEPT(cpp_class<RuntimeError>()->call(Xt("Xtal Runtime Error 1028")->call(Named(Xid(name), cls->object_name()))));
		return;
	}

	if(is_inherited(cls)){
		return;
	}

	XTAL_ASSERT(cls);

	cls->prebind();

	inherited_classes_.push_back(cls.get());
	cls.get()->inc_ref_count();
	invalidate_cache_is();
}

void Class::inherit_strict(const ClassPtr& cls){
	if(cls->is_native()){
		XTAL_SET_EXCEPT(cpp_class<RuntimeError>()->call(Xt("Xtal Runtime Error 1029")->call(Named(Xid(name), cls->object_name()))));
		return;
	}

	inherit_first(cls);
}

AnyPtr Class::inherited_classes(){
	return xnew<ClassInheritedClassesIter>(to_smartptr(this));
}

const NativeFunPtr& Class::def_ctor(const NativeFunPtr& ctor_fun){
	ctor_ = ctor_fun;
	flags_ |= FLAG_NATIVE;
	invalidate_cache_ctor();
	return ctor_;
}

const NativeFunPtr& Class::ctor(){
	prebind();

	if(cache_ctor(to_smartptr(this), 0)){
		return ctor_;
	}

	if(ctor_){
		return ctor_;
	}

	for(int_t i = inherited_classes_.size(); i>0; --i){
		if(const NativeFunPtr& ret = inherited_classes_[i-1]->ctor()){
			return ctor_ = ret;
		}
	}

	return unchecked_ptr_cast<NativeFun>(null);
}

const NativeFunPtr& Class::def_serial_ctor(const NativeFunPtr& ctor_fun){
	serial_ctor_ = ctor_fun;
	flags_ |= FLAG_NATIVE;
	invalidate_cache_ctor();
	return serial_ctor_;
}

const NativeFunPtr& Class::serial_ctor(){
	prebind();

	if(cache_ctor(to_smartptr(this), 1)){
		return serial_ctor_;
	}

	if(serial_ctor_){
		return serial_ctor_;
	}

	for(int_t i = inherited_classes_.size(); i>0; --i){
		if(const NativeFunPtr& ret = inherited_classes_[i-1]->serial_ctor()){
			return serial_ctor_ = ret;
		}
	}

	return unchecked_ptr_cast<NativeFun>(null);
}

void Class::init_instance(const AnyPtr& self, const VMachinePtr& vm){
	for(int_t i = inherited_classes_.size(); i>0; --i){
		inherited_classes_[i-1]->init_instance(self, vm);
	}
	
	if(info()->instance_variable_size){
		pvalue(self)->make_instance_variables();
		pvalue(self)->instance_variables()->init_variables(info());

		// 先頭のメソッドはインスタンス変数初期化関数
		if(members_.at(0)){
			vm->setup_call(0);
			vm->set_arg_this(self);
			members_.at(0)->rawcall(vm);
			vm->cleanup_call();
		}
	}
}

IDPtr Class::find_near_member(const IDPtr& primary_key, const AnyPtr& secondary_key){
	XTAL_ASSERT(!raweq(secondary_key, null)); // セカンダリキーが無いときはnullでなくundefinedを指定するようになったので、検出用assert
	int_t minv = 0xffffff;
	IDPtr minid = null;
	Xfor_cast(const ValuesPtr& v, send(Xid(members_ancestors_too))){
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
	def(primary_key, xtal::double_dispatch_method(primary_key), undefined, accessibility);
}

void Class::def_double_dispatch_fun(const IDPtr& primary_key, int_t accessibility){
	def(primary_key, xtal::double_dispatch_fun(to_smartptr(this), primary_key), undefined, accessibility);
}

const NativeFunPtr& Class::def_and_return(const IDPtr& primary_key, const AnyPtr& secondary_key, int_t accessibility, const param_types_holder_n& pth, const void* val, int_t val_size){
	XTAL_ASSERT(!raweq(secondary_key, null)); // セカンダリキーが無いときはnullでなくundefinedを指定するようになったので、検出用assert
	return unchecked_ptr_cast<NativeFun>(def2(primary_key, xnew<NativeFun>(pth, val, val_size), secondary_key, accessibility));
}

const NativeFunPtr& Class::def_and_return_bind_this(const IDPtr& primary_key, const AnyPtr& secondary_key, int_t accessibility, const param_types_holder_n& pth, const void* val, int_t val_size){
	XTAL_ASSERT(!raweq(secondary_key, null)); // セカンダリキーが無いときはnullでなくundefinedを指定するようになったので、検出用assert
	return unchecked_ptr_cast<NativeFun>(def2(primary_key, xnew<NativeFunBindedThis>(pth, val, val_size, to_smartptr(this)), secondary_key, accessibility));
}

const AnyPtr& Class::def2(const IDPtr& primary_key, const AnyPtr& value, const AnyPtr& secondary_key, int_t accessibility){
	XTAL_ASSERT(!raweq(secondary_key, null)); // セカンダリキーが無いときはnullでなくundefinedを指定するようになったので、検出用assert
	def(primary_key, value, secondary_key, accessibility);
	Key key = {primary_key, secondary_key};
	map_t::iterator it = map_members_->find(key);
	if(it!=map_members_->end()){
		return members_.at(it->second.num);
	}
	return null;
}

void Class::overwrite_member(const IDPtr& primary_key, const AnyPtr& value, const AnyPtr& secondary_key, int_t accessibility){
	XTAL_ASSERT(!raweq(secondary_key, null)); // セカンダリキーが無いときはnullでなくundefinedを指定するようになったので、検出用assert
	Key key = {primary_key, secondary_key};
	map_t::iterator it = map_members_->find(key);
	if(it==map_members_->end()){
		Value val = {members_.size(), accessibility};
		map_members_->insert(key, val);
		members_.push_back(value);
		value->set_object_parent(to_smartptr(this));
		invalidate_cache_member();
	}
	else{
		if(const ClassPtr& dest = ptr_cast<Class>(members_.at(it->second.num))){
			if(const ClassPtr& src = ptr_cast<Class>(value)){
				if(!dest->is_native() && !src->is_native()){
					dest->overwrite(src);
					invalidate_cache_member();
					return;
				}
			}
		}

		members_.set_at(it->second.num, value);
		value->set_object_parent(to_smartptr(this));
		invalidate_cache_member();
	}
}

void Class::def(const IDPtr& primary_key, const AnyPtr& value, const AnyPtr& secondary_key, int_t accessibility){
	XTAL_ASSERT(!raweq(secondary_key, null)); // セカンダリキーが無いときはnullでなくundefinedを指定するようになったので、検出用assert
	Key key = {primary_key, secondary_key};
	map_t::iterator it = map_members_->find(key);
	if(it==map_members_->end()){
		Value val = {members_.size(), accessibility};
		map_members_->insert(key, val);
		members_.push_back(value);
		value->set_object_parent(to_smartptr(this));
		invalidate_cache_member();
	}
	else{
		XTAL_SET_EXCEPT(cpp_class<RedefinedError>()->call(Xt("Xtal Runtime Error 1011")->call(Named(Xid(object), this->object_name()), Named(Xid(name), primary_key))));
	}
}

const AnyPtr& Class::any_member(const IDPtr& primary_key, const AnyPtr& secondary_key){
	XTAL_ASSERT(!raweq(secondary_key, null)); // セカンダリキーが無いときはnullでなくundefinedを指定するようになったので、検出用assert
	cpp_class<Any>()->bind();

	Key key = {primary_key, secondary_key};
	map_t::iterator it = map_members_->find(key);
	if(it!=map_members_->end()){
		return members_.at(it->second.num);
	}
	return undefined;
}

const AnyPtr& Class::bases_member(const IDPtr& name){
	for(int_t i = inherited_classes_.size(); i>0; --i){
		if(const AnyPtr& ret = inherited_classes_[i-1]->member(name)){
			return ret;
		}
	}
	return undefined;
}

const AnyPtr& Class::find_member(const IDPtr& primary_key, const AnyPtr& secondary_key, bool inherited_too, int_t& accessibility, bool& nocache){
	XTAL_ASSERT(!raweq(secondary_key, null)); // セカンダリキーが無いときはnullでなくundefinedを指定するようになったので、検出用assert
	Key key = {primary_key, secondary_key};
	map_t::iterator it = map_members_->find(key);

	if(it!=map_members_->end()){
		// メンバが見つかった
		
		accessibility = it->second.flags & 0x3;
		return members_.at(it->second.num);
	}
	
	// 継承しているクラスを順次検索
	if(inherited_too){
		for(int_t i=inherited_classes_.size(); i>0; --i){
			const AnyPtr& ret = inherited_classes_[i-1]->do_member(primary_key, secondary_key, true, accessibility, nocache);
			if(rawne(ret, undefined)){
				return ret;
			}
		}
	}

	return undefined;
}

const AnyPtr& Class::do_member(const IDPtr& primary_key, const AnyPtr& secondary_key, bool inherited_too, int_t& accessibility, bool& nocache){
	XTAL_ASSERT(!raweq(secondary_key, null)); // セカンダリキーが無いときはnullでなくundefinedを指定するようになったので、検出用assert
	bind();

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
		for(int_t i=0, sz=klass->inherited_classes_.size(); i<sz; ++i){
			const AnyPtr& ret = do_member(primary_key, to_smartptr(klass->inherited_classes_[i]), inherited_too, accessibility, nocache);
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

bool Class::set_member(const IDPtr& primary_key, const AnyPtr& value, const AnyPtr& secondary_key){
	XTAL_ASSERT(!raweq(secondary_key, null)); // セカンダリキーが無いときはnullでなくundefinedを指定するようになったので、検出用assert
	Key key = {primary_key, secondary_key};
	map_t::iterator it = map_members_->find(key);
	if(it==map_members_->end()){
		XTAL_SET_EXCEPT(cpp_class<RuntimeError>()->call(Xid(undefined)));
		return false;
	}

	members_.set_at(it->second.num, value);
	value->set_object_parent(to_smartptr(this));
	invalidate_cache_member();
	return true;
}

void Class::set_member_direct(int_t i, const IDPtr& primary_key, const AnyPtr& value, const AnyPtr& secondary_key, int_t accessibility){
	XTAL_ASSERT(!raweq(secondary_key, null)); // セカンダリキーが無いときはnullでなくundefinedを指定するようになったので、検出用assert 
	members_.set_at(i, value);
	Key key = {primary_key, secondary_key};
	Value val = {i, accessibility};
	map_members_->insert(key, val);
	value->set_object_parent(to_smartptr(this));
	invalidate_cache_member();
}

void Class::set_object_parent(const ClassPtr& parent){
	if(object_force_<parent->object_force()){
		object_force_ = parent->object_force()-1;
		HaveParent::set_object_parent(parent);
		if(map_members_){
			for(map_t::iterator it=map_members_->begin(), last=map_members_->end(); it!=last; ++it){
				members_.at(it->second.num)->set_object_parent(to_smartptr(this));
			}
		}
	}
}

ValuesPtr Class::child_object_name(const AnyPtr& a){
	if(map_members_){
		for(map_t::iterator it=map_members_->begin(), last=map_members_->end(); it!=last; ++it){
			if(raweq(members_.at(it->second.num), a)){
				return mv(it->first.primary_key, it->first.secondary_key);
			}
		}
	}
	return null;
}

StringPtr Class::object_name(){
	if(const ClassPtr& parent = object_parent()){
		if(ValuesPtr myname = parent->child_object_name(ap(*this))){
			if(raweq(myname->at(1), undefined)){
				return Xf("%s::%s")->call(parent->object_name(), myname->at(0))->to_s();
			}
			else{
				return Xf("%s::%s#%s")->call(parent->object_name(), myname->at(0), myname->at(1))->to_s();
			}
		}
	}

	if(name_ && name_->data_size()!=0){
		return name_;
	}

	return Xid(AnonymousClass);
}

void Class::set_object_name(const StringPtr& name){
	name_ = name;
}

bool Class::is_inherited(const AnyPtr& v){
	if(this==pvalue(v)){
		return true;
	}

	if(raweq(v, cpp_class<Any>())){
		return true;
	}

	prebind();

	for(int_t i=0, sz=inherited_classes_.size(); i<sz; ++i){
		if(inherited_classes_[i]->is_inherited(v)){
			return true;
		}
	}

	return false;
}

bool Class::is_inherited_cpp_class(){
	if(is_native()){
		return true;
	}

	prebind();

	for(int_t i=0, sz=inherited_classes_.size(); i<sz; ++i){
		if(inherited_classes_[i]->is_inherited_cpp_class()){
			return true;
		}
	}

	return false;
}

void Class::rawcall(const VMachinePtr& vm){
	prebind();

	if(is_native()){
		if(const NativeFunPtr& ret = ctor()){
			ret->rawcall(vm);
			if(vm->except()){
				return;
			}
			init_instance(vm->result(), vm);
		}
		else{
			vm->set_except(cpp_class<RuntimeError>()->call(Xt("Xtal Runtime Error 1013")->call(object_name())));
		}
	}
	else{
		AnyPtr instance;
		if(const NativeFunPtr& newfun = ctor()){
			instance = newfun->call();
		}
		else{
			instance = xnew<Base>();
		}

		if(type(instance)==TYPE_BASE){
			pvalue(instance)->set_class(to_smartptr(this));
		}

		init_instance(instance, vm);

		XTAL_CHECK_EXCEPT(e){ return; }
		
		if(const AnyPtr& ret = member(Xid(initialize), undefined, vm->ff().self())){
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
}

void Class::s_new(const VMachinePtr& vm){
	if(is_native()){
		if(const AnyPtr& ret = serial_ctor()){
			ret->rawcall(vm);
			init_instance(vm->result(), vm);
		}
		else{
			vm->set_except(cpp_class<RuntimeError>()->call(Xt("Xtal Runtime Error 1013")->call(object_name())));
		}
	}
	else{
		AnyPtr instance;
		if(const AnyPtr& newfun = serial_ctor()){
			instance = newfun->call();
		}
		else{
			instance = xnew<Base>();
		}

		if(type(instance)==TYPE_BASE){
			pvalue(instance)->set_class(to_smartptr(this));
			init_instance(instance, vm);
		}

		vm->return_result(instance);
	}
}

AnyPtr Class::ancestors(){
	if(raweq(to_smartptr(this), cpp_class<Any>())){
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

void Class::prebind(){
	if(prebinder_){
		void (*temp)(const ClassPtr&) = prebinder_;
		prebinder_ = 0;
		temp(to_smartptr(this));
	}
}

void Class::bind(){
	if(binder_){
		prebind();
		void (*temp)(const ClassPtr&) = binder_;
		binder_ = 0;
		temp(to_smartptr(this));
	}
}

Singleton::Singleton(const StringPtr& name)
	:Class(name){
	flags_ |= FLAG_SINGLETON;
	Base::set_class(to_smartptr(this));
	inherit(cpp_class<Class>());
}

Singleton::Singleton(const FramePtr& outer, const CodePtr& code, ClassInfo* info)
	:Class(outer, code, info){
	flags_ |= FLAG_SINGLETON;
	Base::set_class(to_smartptr(this));
	inherit(cpp_class<Class>());
}

void Singleton::init_singleton(const VMachinePtr& vm){
	SingletonPtr instance = to_smartptr(this);
	init_instance(instance, vm);
	
	if(const AnyPtr& ret = member(Xid(initialize), undefined, vm->ff().self())){
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
	XTAL_SET_EXCEPT(cpp_class<RuntimeError>()->call(Xt("Xtal Runtime Error 1013")->call(object_name())));
}

CppSingleton::CppSingleton()
:Class(cpp_class_t()){
	flags_ |= FLAG_SINGLETON;
	Base::set_class(to_smartptr(this));
	inherit(cpp_class<Class>());
}


}
