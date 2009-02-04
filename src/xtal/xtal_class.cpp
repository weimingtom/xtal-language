#include "xtal.h"
#include "xtal_macro.h"

namespace xtal{

int_t InstanceVariables::find_core_inner(ClassInfo* core){
	for(int_t i = 1, size = (int_t)variables_info_.size(); i<size; ++i){
		if(variables_info_[i].core==core){
			std::swap(variables_info_[0], variables_info_[i]);
			return variables_info_[0].pos;
		}	
	}
	XTAL_THROW(builtin()->member(Xid(InstanceVariableError))->call(Xt("Xtal Runtime Error 1003")), return 0);
}


Class::Class(const FramePtr& outer, const CodePtr& code, ClassInfo* core)
	:Frame(outer, code, core), mixins_(xnew<Array>()){
	is_cpp_class_ = false;
	make_map_members();
}

Class::Class(const StringPtr& name)
	:Frame(null, null, 0), mixins_(xnew<Array>()){
	set_object_name(name, 1, null);
	is_cpp_class_ = false;
	make_map_members();
}

Class::Class(cpp_class_t, const StringPtr& name)
	:Frame(null, null, 0), mixins_(xnew<Array>()){
	set_object_name(name, 1, null);
	is_cpp_class_ = true;
	make_map_members();
}

void Class::overwrite(const ClassPtr& p){
	if(!is_cpp_class_){
		operator=(*p);
		mixins_ = p->mixins_;
		inc_global_mutate_count();
	}
}


void Class::inherit(const ClassPtr& md){
	if(is_inherited(md))
		return;

	mixins_->push_back(md);
	inc_global_mutate_count();
}

void Class::inherit_strict(const ClassPtr& md){
	
	if(md->is_cpp_class() && is_inherited_cpp_class()){
		XTAL_THROW(RuntimeError()->call(Xt("Xtal Runtime Error 1019")), return);
	}

	if(is_inherited(md))
		return;

	mixins_->push_back(md);
	inc_global_mutate_count();
}

AnyPtr Class::inherited_classes(){
	return mixins_->each();
}

void Class::init_instance(const AnyPtr& self, const VMachinePtr& vm){
	for(int_t i = mixins_->size(); i>0; --i){
		unchecked_ptr_cast<Class>(mixins_->at(i-1))->init_instance(self, vm);
	}
	
	if(core()->instance_variable_size){
		pvalue(self)->make_instance_variables();
		pvalue(self)->instance_variables()->init_variables(core());

		vm->setup_call(0);
		vm->set_arg_this(self);
		// 先頭のメソッドはインスタンス変数初期化関数
		members_->at(0)->rawcall(vm);
		vm->cleanup_call();
	}
}

IDPtr Class::find_near_member(const IDPtr& primary_key, const AnyPtr& secondary_key){
	int_t minv = 0xffffff;
	IDPtr minid = null;
	Xfor(v, send(Xid(members))){
		IDPtr id = ptr_cast<ID>(v->send(Xid(op_at), 0));
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

const CFunPtr& Class::def_and_return(const IDPtr& primary_key, const AnyPtr& secondary_key, int_t accessibility, void (*fun)(ParamInfoAndVM& pvm), const void* val, int_t val_size, int_t param_n){
	return unchecked_ptr_cast<CFun>(def2(primary_key, 
		xnew<CFun>(fun, val, val_size, param_n), secondary_key, accessibility));
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
		value->set_object_name(primary_key, object_name_force(), from_this(this));
		inc_global_mutate_count();
	}
	else{
		XTAL_THROW(builtin()->member(Xid(RedefinedError))->call(Xt("Xtal Runtime Error 1011")->call(Named(Xid(object), this->object_name()), Named(Xid(name), primary_key))), return);
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

const AnyPtr& Class::find_member(const IDPtr& primary_key, const AnyPtr& secondary_key, const AnyPtr& self, bool inherited_too){
	Key key = {primary_key, secondary_key};
	map_t::iterator it = map_members_->find(key);

	if(it!=map_members_->end()){
		// メンバが見つかった

		// privateなメンバは見なかったことにする。
		if(it->second.flags & KIND_PRIVATE){

		}
		else{
			// protectedメンバでアクセス権が無いなら例外
			if(it->second.flags & KIND_PROTECTED && !self->is(from_this(this))){
				XTAL_THROW(builtin()->member(Xid(AccessibilityError))->call(Xt("Xtal Runtime Error 1017")->call(
					Named(Xid(object), this->object_name()), Named(Xid(name), primary_key), Named(Xid(accessibility), Xid(protected))))
				, return undefined);			
			}

			return members_->at(it->second.num);
		}
	}
	
	// 継承しているクラスを順次検索
	if(inherited_too){
		for(int_t i=0, sz=mixins_->size(); i<sz; ++i){
			const AnyPtr& ret = unchecked_ptr_cast<Class>(mixins_->at(i))->member(primary_key, secondary_key, self);
			if(rawne(ret, undefined)){
				return ret;
			}
		}
	}

	return undefined;
}

const AnyPtr& Class::do_member(const IDPtr& primary_key, const AnyPtr& secondary_key, const AnyPtr& self, bool inherited_too, bool* nocache){
	{
		const AnyPtr& ret = find_member(primary_key, secondary_key, self, inherited_too);
		if(rawne(ret, undefined)){
			return ret;
		}
	}
		
	{
		const AnyPtr& ret = get_cpp_class<Any>()->any_member(primary_key, secondary_key);
		if(rawne(ret, undefined)){
			return ret;
		}
	}

	// 見つからなかった。

	// もしsecond keyがクラスの場合、スーパークラスをsecond keyに変え、順次試していく
	if(const ClassPtr& klass = ptr_as<Class>(secondary_key)){
		for(int_t i=0, sz=klass->mixins_->size(); i<sz; ++i){
			const AnyPtr& ret = do_member(primary_key, klass->mixins_->at(i), self, inherited_too, nocache);
			if(rawne(ret, undefined)){
				return ret;
			}
		}

		if(rawne(get_cpp_class<Any>(), klass)){
			const AnyPtr& ret = do_member(primary_key, get_cpp_class<Any>(), self, inherited_too, nocache);
			if(rawne(ret, undefined)){
				return ret;
			}
		}	
	}

	// やっぱり見つからなかった。
	return undefined;
}

void Class::set_member(const IDPtr& primary_key, const AnyPtr& value, const AnyPtr& secondary_key){
	Key key = {primary_key, secondary_key};
	map_t::iterator it = map_members_->find(key);
	if(it==map_members_->end()){
		XTAL_THROW(RuntimeError()->call(Xid(undefined)), return);
	}
	else{
		members_->set_at(it->second.num, value);
		//value.set_object_name(name, object_name_force(), this);
	}

	inc_global_mutate_count();
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

	return raweq(v, get_cpp_class<Any>());
}

bool Class::is_inherited_cpp_class(){
	if(is_cpp_class()){
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
	AnyPtr instance;
	if(newfun){
		instance = newfun->call();
	}
	else{
		instance = xnew<Base>();
		pvalue(instance)->set_xtal_instance_flag();
	}

	pvalue(instance)->set_class(from_this(this));
	init_instance(instance, vm);
	
	if(const AnyPtr& ret = member(Xid(initialize), null, vm->ff().self())){
		vm->set_arg_this(instance);
		if(vm->need_result()){
			ret->rawcall(vm);
			vm->replace_result(0, instance);
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
		pvalue(instance)->set_xtal_instance_flag();
	}

	pvalue(instance)->set_class(from_this(this));
	init_instance(instance, vm);

	vm->return_result(instance);
}

AnyPtr Class::ancestors(){
	if(from_this(this)==get_cpp_class<Any>()){
		return null;
	}			
	
	ArrayPtr ret = xnew<Array>();
	Xfor_cast(const ClassPtr& it, inherited_classes()){
		ret->push_back(it);

		Xfor(it2, it->ancestors()){
			ret->push_back(it2);
		}
	}

	ret->push_back(get_cpp_class<Any>());
	return ret;
}

CppClass::CppClass(const StringPtr& name)
	:Class(cpp_class_t(), name){
}

void CppClass::rawcall(const VMachinePtr& vm){
	if(const AnyPtr& ret = member(Xid(new), null, from_this(this))){
		ret->rawcall(vm);
		init_instance(vm->result(), vm);
	}
	else{
		XTAL_THROW(RuntimeError()->call(Xt("Xtal Runtime Error 1013")->call(object_name())), return);
	}
}

void CppClass::s_new(const VMachinePtr& vm){
	if(const AnyPtr& ret = member(Xid(serial_new), null, from_this(this))){
		ret->rawcall(vm);
		init_instance(vm->result(), vm);
	}
	else{
		XTAL_THROW(RuntimeError()->call(Xt("Xtal Runtime Error 1013")->call(object_name())), return);
	}
}

Singleton::Singleton(const StringPtr& name)
	:Class(name){
	Base::set_class(from_this(this));
	inherit(get_cpp_class<Class>());
}

Singleton::Singleton(const FramePtr& outer, const CodePtr& code, ClassInfo* core)
	:Class(outer, code, core){
	Base::set_class(from_this(this));
	inherit(get_cpp_class<Class>());
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
	XTAL_THROW(RuntimeError()->call(Xt("Xtal Runtime Error 1013")->call(object_name())), return);
}

CppSingleton::CppSingleton(const StringPtr& name)
	:Class(name){
	Base::set_class(from_this(this));
	inherit(get_cpp_class<Class>());
}



void initialize_class(){
	{
		ClassPtr p = new_cpp_class<Class>(Xid(Class));
		p->inherit(get_cpp_class<Frame>());
		p->def_method(Xid(inherit), &Class::inherit_strict);
		p->def_method(Xid(overwrite), &Class::overwrite);
		p->def_method(Xid(s_new), &Class::s_new);
		p->def_method(Xid(inherited_classes), &Class::inherited_classes);
		p->def_method(Xid(is_inherited), &Any::is_inherited);
		p->def_method(Xid(find_near_member), &Class::find_near_member)->params(Xid(primary_key), null, Xid(secondary_key), null);
	}

	{
		ClassPtr p = new_cpp_class<CppClass>(Xid(CppClass));
		p->inherit(get_cpp_class<Class>());
	}

	builtin()->def(Xid(Class), get_cpp_class<Class>());
	builtin()->def(Xid(CppClass), get_cpp_class<CppClass>());
}

void initialize_class_script(){
	Xemb((
Class::ancestors: method{
	if(this===Any){
		return null;
	}			
	
	return fiber{
		this.inherited_classes{
			yield it;
			it.ancestors{
				yield it;
			}
		}

		yield Any;
	}
}

Class::members: method(inherited_too: true){
	if(!inherited_too){
		return Frame::members();
	}

	return fiber{
		this.members(false){
			yield it;
		}

		this.ancestors{
			it.members(false){
				yield it;
			}
		}
	}
}
	),
"\x78\x74\x61\x6c\x01\x00\x00\x00\x00\x00\x00\x4b\x39\x00\x01\x89\x00\x01\x00\x0f\x0b\x2f\x00\x00\x00\x00\x00\x02\x0b\x25\x01\x25\x00\x37\x00\x03\x39\x00\x01\x89\x00\x02\x00\x0f\x0b\x2f\x00\x00\x00\x00\x00\x04\x01\x25\x01\x25\x00\x37\x00\x05\x39\x00\x01\x89"
"\x00\x03\x00\x0f\x0b\x2f\x00\x00\x00\x00\x00\x04\x01\x25\x01\x25\x00\x37\x00\x06\x25\x00\x8b\x00\x03\x08\x00\x00\x00\x00\x00\x02\x00\x00\x00\x12\x00\x20\x00\x00\x00\x00\x00\x04\x00\x00\x00\x12\x00\x38\x00\x00\x00\x00\x00\x06\x00\x00\x00\x12\x00\x00\x00\x00"
"\x04\x00\x00\x00\x00\x03\x06\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x08\x00\x00\x00\x05\x00\x02\x00\x00\x00\x00\x00\x00\x01\x00\x00\x20\x00\x00\x00\x05\x00\x04\x00\x00\x00\x00\x00\x00\x01\x00\x00\x38\x00\x00\x00\x05\x00\x06\x00\x00\x00\x00\x00\x00\x01\x00"
"\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x14\x00\x00\x00\x00\x11\x00\x00\x00\x00\x00\x00\x00\x06\x00\x00\x00\x00\x00\x00\x00\x03\x00\x00\x00\x03\x00\x00\x00\x06\x00\x00\x00\x08\x00\x00\x00\x04\x00\x00\x00\x10\x00\x00\x00\x05\x00\x00"
"\x00\x13\x00\x00\x00\x06\x00\x00\x00\x18\x00\x00\x00\x0b\x00\x00\x00\x18\x00\x00\x00\x08\x00\x00\x00\x1b\x00\x00\x00\x0b\x00\x00\x00\x20\x00\x00\x00\x09\x00\x00\x00\x28\x00\x00\x00\x0a\x00\x00\x00\x2b\x00\x00\x00\x0b\x00\x00\x00\x30\x00\x00\x00\x10\x00\x00"
"\x00\x30\x00\x00\x00\x0d\x00\x00\x00\x33\x00\x00\x00\x10\x00\x00\x00\x38\x00\x00\x00\x0e\x00\x00\x00\x40\x00\x00\x00\x0f\x00\x00\x00\x43\x00\x00\x00\x10\x00\x00\x00\x48\x00\x00\x00\x11\x00\x00\x00\x00\x01\x0b\x00\x00\x00\x03\x09\x00\x00\x00\x06\x73\x6f\x75"
"\x72\x63\x65\x09\x00\x00\x00\x11\x74\x6f\x6f\x6c\x2f\x74\x65\x6d\x70\x2f\x69\x6e\x2e\x78\x74\x61\x6c\x09\x00\x00\x00\x0b\x69\x64\x65\x6e\x74\x69\x66\x69\x65\x72\x73\x0a\x00\x00\x00\x07\x09\x00\x00\x00\x00\x09\x00\x00\x00\x05\x4d\x75\x74\x65\x78\x09\x00\x00"
"\x00\x04\x6c\x6f\x63\x6b\x09\x00\x00\x00\x0b\x62\x6c\x6f\x63\x6b\x5f\x66\x69\x72\x73\x74\x09\x00\x00\x00\x06\x75\x6e\x6c\x6f\x63\x6b\x09\x00\x00\x00\x0a\x62\x6c\x6f\x63\x6b\x5f\x6e\x65\x78\x74\x09\x00\x00\x00\x0b\x62\x6c\x6f\x63\x6b\x5f\x62\x72\x65\x61\x6b"
"\x09\x00\x00\x00\x06\x76\x61\x6c\x75\x65\x73\x0a\x00\x00\x00\x01\x03"
)->call();

}

}
