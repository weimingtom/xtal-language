#include "xtal.h"
#include "xtal_macro.h"

#include "xtal_details.h"

namespace xtal{

InstanceVariables::InstanceVariables()		
	:variables_(0){
	VariablesInfo vi;
	vi.class_info = 0;
	vi.pos = 0;
	variables_info_.push_back(vi);
	variables_.push_back(undefined);
}
		
InstanceVariables::~InstanceVariables(){}
	
void InstanceVariables::init_variables(ClassInfo* class_info){
	if(class_info->instance_variable_size){
		VariablesInfo vi;
		vi.class_info = class_info;
		vi.pos = (int_t)variables_.size();
		variables_.upsize(class_info->instance_variable_size);
		variables_info_.push_back(vi);
	}
}

bool InstanceVariables::is_included(ClassInfo* class_info){
	VariablesInfo& info = variables_info_.back();
	if(info.class_info == class_info){
		return true;
	}
	for(uint_t i = 0, size = variables_info_.size()-1; i<size; ++i){
		if(variables_info_[i].class_info==class_info){
			std::swap(variables_info_[size], variables_info_[i]);
			return true;
		}	
	}
	return false;
}

uint_t InstanceVariables::find_class_info_inner(ClassInfo* class_info, uint_t index){
	for(uint_t i = 0, size = variables_info_.size()-1; i<size; ++i){
		if(variables_info_[i].class_info==class_info){
			std::swap(variables_info_[size], variables_info_[i]);
			return variables_info_[size].pos + index;
		}
	}
	
	XTAL_SET_EXCEPT(cpp_class<InstanceVariableError>()->call(Xt0("XRE1003")));
	return 0;
}

void InstanceVariables::replace(ClassInfo* from, ClassInfo* to){
	for(uint_t i = 0, size = variables_info_.size(); i<size; ++i){
		if(variables_info_[i].class_info==from){
			int_t pos = variables_info_[i].pos;
			variables_info_.erase(i, 1);

			for(uint_t j = 0, jsize = variables_info_.size(); j<jsize; ++j){
				if(variables_info_[j].pos>pos){
					variables_info_[j].pos -= from->instance_variable_size;
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
	variables_.attach(&(AnyPtr&)undefined, 1);
}

EmptyInstanceVariables::~EmptyInstanceVariables(){
	variables_info_.detach();
	variables_.detach();
}

InstanceVariables::VariablesInfo EmptyInstanceVariables::vi;

///////////////////////////////////////

Class::Class()
	:Frame(nul<Frame>(), nul<Code>(), 0){
	init();
}

Class::Class(const FramePtr& outer, const CodePtr& code, ClassInfo* info)
	:Frame(outer, code, info){
	init();
//	instance_variables_layout_.push(InstanceVariablesInfo(info, 0));
}

Class::Class(const IDPtr& name)
	:Frame(nul<Frame>(), nul<Code>(), 0){
	init();
	set_object_temporary_name(name);
}

Class::Class(cpp_class_t)
	:Frame(nul<Frame>(), nul<Code>(), 0){
	init();
	flags_ = FLAG_NATIVE;
}

void Class::init(){
	set_object_temporary_name(empty_id);
	object_force_ = 0;
	flags_ = 0;
	symbol_data_ = 0;
	make_map_members();
	initialized_members_ = true;
}

Class::~Class(){

}

void Class::overwrite(const ClassPtr& p){
	if(!is_native() && !p->is_native()){
		for(int_t i=0, sz=p->inherited_classes_.size(); i<sz; ++i){
			inherit(unchecked_ptr_cast<Class>(p->inherited_classes_.at(i)));
		}
		
		if(p->map_members_){
			for(map_t::iterator it=p->map_members_->begin(), last=p->map_members_->end(); it!=last; ++it){
				overwrite_member(it->first.primary_key, p->member_direct(it->second.num), it->first.secondary_key, it->second.flag);
			}
		}

		for(uint_t i=0; i<alive_object_count(); ++i){
			AnyPtr obj = alive_object(i);
			if(type(obj)==TYPE_BASE){
				if(obj->is(to_smartptr(this))){
					AnyPtr data = obj->save_instance_variables(to_smartptr(this));
					if(pvalue(obj)->instance_variables()){
						pvalue(obj)->instance_variables()->replace(info(), p->info());
					}
					obj->load_instance_variables(p, data);

					{
						map_t::iterator it = find(Xid(reloaded), undefined);
						if(it!=map_members_->end()){
							const VMachinePtr& vm = setup_call(0);
							vm->set_arg_this(obj);
							member_direct(it->second.num)->rawcall(vm);
							vm->cleanup_call();
						}
					}
				}
			}
		}

		set_outer(p->outer());
		set_code(p->code());
		set_info(p->info());
	}
}

void Class::fillup_inherited_classes(){

}

void Class::inherit(const ClassPtr& cls){
	if(is_inherited(cls))
		return;

	XTAL_ASSERT(cls);

	cls->prebind();

	fillup_inherited_classes();
	inherited_classes_.push_back(cls);
	invalidate_cache_is();

	/*
	uint_t offset = 0;
	for(uint_t i=0, size=instance_variables_layout_.size(); i<size; ++i){
		offset += instance_variables_layout_[i].class_info->instance_variable_size;
	}

	for(uint_t i=0, size=cls->instance_variables_layout_.size(); i<size; ++i){
		instance_variables_layout_.push(
			InstanceVariablesInfo(
				cls->instance_variables_layout_[i].class_info,
				cls->instance_variables_layout_[i].pos + offset));
	}
	*/
}

void Class::inherit_first(const ClassPtr& cls){
	if(cls->is_singleton()){
		if(!is_singleton()){
			set_runtime_error(Xt0("XRE1031"));
			return;
		}
	}
	else{
		if(cls->is_native()){
			if(is_inherited_cpp_class()){
				set_runtime_error(Xt0("XRE1019"));
				return;
			}
		}
	}

	if(!symbol_data_){
		symbol_data_ = cls->symbol_data();
	}

	if(cls->is_final()){
		set_runtime_error(Xt1("XRE1028", name, cls->object_name()));
		return;
	}

	inherit(cls);
}

void Class::inherit_strict(const ClassPtr& cls){
	if(cls->is_native()){
		set_runtime_error(Xt1("XRE1029", name, cls->object_name()));
		return;
	}

	inherit_first(cls);
}

AnyPtr Class::inherited_classes(){
	return xnew<ArrayIter>(to_smartptr(this), &inherited_classes_);
}

const NativeFunPtr& Class::def_ctor(int_t type, const NativeFunPtr& ctor_func){
	ctor_[type] = ctor_func;
	flags_ |= (FLAG_LAST_DEFINED_CTOR<<type);
	flags_ |= FLAG_NATIVE;
	invalidate_cache_ctor();
	return ctor_[type];
}

const NativeFunPtr& Class::ctor(int_t type){
	prebind();

	if(cache_ctor(to_smartptr(this), type)){
		return ctor_[type];
	}

	if(ctor_[type]){
		return ctor_[type];
	}

	for(int_t i = inherited_classes_.size(); i>0; --i){
		if(const NativeFunPtr& ret = unchecked_ptr_cast<Class>(inherited_classes_.at(i-1))->ctor(type)){
			return ctor_[type] = ret;
		}
	}

	return nul<NativeMethod>();
}

const NativeFunPtr& Class::def_ctor(const NativeFunPtr& ctor_func){
	return def_ctor(0, ctor_func);
}

const NativeFunPtr& Class::ctor(){
	return ctor(0);
}

const NativeFunPtr& Class::def_serial_ctor(const NativeFunPtr& ctor_func){
	return def_ctor(1, ctor_func);
}

const NativeFunPtr& Class::serial_ctor(){
	return ctor(1);
}

void Class::init_instance(const AnyPtr& self, const VMachinePtr& vm){
	for(int_t i = inherited_classes_.size(); i>0; --i){
		unchecked_ptr_cast<Class>(inherited_classes_.at(i-1))->init_instance(self, vm);
	}
	
	if(info()->instance_variable_size){
		pvalue(self)->make_instance_variables();
		pvalue(self)->instance_variables()->init_variables(info());

		// 先頭のメソッドはインスタンス変数初期化関数
		if(member_direct(0)){
			vm->setup_call(0);
			vm->set_arg_this(self);
			member_direct(0)->rawcall(vm);
			vm->cleanup_call();
		}
	}
}

IDPtr Class::find_near_member2(const IDPtr& primary_key, const AnyPtr& secondary_key, int_t& minv){
	IDPtr minid = null;
	AnyPtr mem = get_class()->member(Xid(members_ancestors_too));

	if(is_undefined(mem)){
		return minid;
	}

	const VMachinePtr& vm = setup_call(1);
	vm->set_arg_this(ap(*this));
	mem->rawcall(vm);

	AnyPtr value = vm->result_and_cleanup_call();

	Xfor_cast(const ValuesPtr& v, value){
		IDPtr id = ptr_cast<ID>(v->at(0));
		int_t dist = edit_distance(primary_key, id);
		if(dist<minv){
			minv = dist;
			minid = id;
		}
	}

	return minid;
}

void Class::def_double_dispatch_method(const IDPtr& primary_key, int_t accessibility){
	on_def(primary_key, xtal::double_dispatch_method(primary_key), undefined, accessibility);
}

void Class::def_double_dispatch_fun(const IDPtr& primary_key, int_t accessibility){
	on_def(primary_key, xtal::double_dispatch_fun(to_smartptr(this), primary_key), undefined, accessibility);
}

const NativeFunPtr& Class::def_and_return(const IDPtr& primary_key, const AnyPtr& secondary_key, int_t accessibility, const param_types_holder_n& pth, const void* val){
	if(is_singleton()){
		return unchecked_ptr_cast<NativeMethod>(def2(primary_key, xnew<NativeFun>(pth, val, to_smartptr(this)), secondary_key, accessibility));
	}
	else{
		return unchecked_ptr_cast<NativeMethod>(def2(primary_key, xnew<NativeMethod>(pth, val), secondary_key, accessibility));
	}
}

void Class::define(const char_t* primary_key, const param_types_holder_n& pth){
	def(intern(primary_key), xnew<StatelessNativeMethod>(pth), undefined, KIND_PUBLIC);
}

void Class::define(const char_t* primary_key, const AnyPtr& secondary_key, const param_types_holder_n& pth){
	def(intern(primary_key), xnew<StatelessNativeMethod>(pth), secondary_key, KIND_PUBLIC);
}

void Class::define_param(const char_t* name, const AnyPtr& default_value){
	AnyPtr member;

	if(flags_&FLAG_LAST_DEFINED_CTOR){
		member = ctor_[0];
	}
	else if(flags_&FLAG_LAST_DEFINED_CTOR2){
		member = ctor_[1];
	}
	else{
		member = members_.back();
	}

	if(type(member)==TYPE_STATELESS_NATIVE_METHOD){
		NativeFunPtr method = xnew<NativeMethod>(*rawvalue(member).pth());
		method->add_param(name, default_value);

		if(flags_&FLAG_LAST_DEFINED_CTOR){
			member = method;
		}
		else if(flags_&FLAG_LAST_DEFINED_CTOR2){
			member = method;
		}
		else{
			members_.set_at(members_.size()-1, method);
		}
	}
	else if(const NativeFunPtr& method = ptr_cast<NativeMethod>(member)){
		method->add_param(name, default_value);
	}
	else{
		XTAL_ASSERT(false);
	}
}

void Class::define(const char_t* primary_key, const AnyPtr& value){
	def(intern(primary_key), value, undefined, KIND_PUBLIC);
}

void Class::define(const char_t* primary_key, const AnyPtr& value, const AnyPtr& secondary_key){
	def(intern(primary_key), value, undefined, KIND_PUBLIC);
}

const AnyPtr& Class::def2(const IDPtr& primary_key, const AnyPtr& value, const AnyPtr& secondary_key, int_t accessibility){
	on_def(primary_key, value, secondary_key, accessibility);
	map_t::iterator it = find(primary_key, secondary_key);
	if(it!=map_members_->end()){
		return member_direct(it->second.num);
	}
	return null;
}

void Class::overwrite_member(const IDPtr& primary_key, const AnyPtr& value, const AnyPtr& secondary_key, int_t accessibility){
	map_t::iterator it = find(primary_key, secondary_key);
	if(it==map_members_->end()){
		on_def(primary_key, value, secondary_key, accessibility);
	}
	else{
		if(const ClassPtr& dest = ptr_cast<Class>(member_direct(it->second.num))){
			if(const ClassPtr& src = ptr_cast<Class>(value)){
				if(!dest->is_native() && !src->is_native()){
					dest->overwrite(src);
					invalidate_cache_member();
					return;
				}
			}
		}

		Frame::set_member_direct(it->second.num, value);
		value->set_object_parent(to_smartptr(this));
		invalidate_cache_member();
	}
}

void Class::on_def(const IDPtr& primary_key, const AnyPtr& value, const AnyPtr& secondary_key, int_t accessibility){
	flags_ &= ~(FLAG_LAST_DEFINED_CTOR | FLAG_LAST_DEFINED_CTOR2);

	map_t::iterator it = find(primary_key, secondary_key);
	if(it==map_members_->end()){
		Key key = {primary_key, secondary_key};
		Value val = {member_size(), accessibility};
		map_members_->insert(key, val);
		push_back_member(value);
		value->set_object_parent(to_smartptr(this));
		invalidate_cache_member();
	}
	else{
		XTAL_SET_EXCEPT(cpp_class<RedefinedError>()->call(Xt2("XRE1011", object, this->object_name(), name, primary_key)));
	}
}

void Class::def(const char_t* primary_key, const AnyPtr& value, const AnyPtr& secondary_key, int_t accessibility){
	on_def(intern(primary_key), value, secondary_key, accessibility);
}

void Class::def(const char8_t* primary_key, const AnyPtr& value, const AnyPtr& secondary_key, int_t accessibility){
	on_def(intern(primary_key), value, secondary_key, accessibility);
}

void Class::def(const IDPtr& primary_key, const AnyPtr& value){
	on_def(primary_key, value, undefined, KIND_PUBLIC);
}

void Class::def(const char_t* primary_key, const AnyPtr& value){
	on_def(intern(primary_key), value, undefined, KIND_PUBLIC);
}

void Class::def(const char8_t* primary_key, const AnyPtr& value){
	on_def(intern(primary_key), value, undefined, KIND_PUBLIC);
}

const AnyPtr& Class::find_member(const IDPtr& primary_key, const AnyPtr& secondary_key, int_t& accessibility, bool& nocache){
	map_t::iterator it = find(primary_key, secondary_key);
	if(it!=map_members_->end()){		
		accessibility = it->second.accessibility();
		nocache = it->second.nocache();
		return member_direct(it->second.num);
	}

	if(const ClassPtr& klass = ptr_cast<Class>(secondary_key)){
		for(int_t i=0, sz=klass->inherited_classes_.size(); i<sz; ++i){
			const AnyPtr& ret = find_member(primary_key, unchecked_ptr_cast<Class>(klass->inherited_classes_.at(i)), accessibility, nocache);
			if(!is_undefined(ret)){
				return ret;
			}
		}

		if(rawne(cpp_class<Any>(), klass)){
			const AnyPtr& ret = find_member(primary_key, cpp_class<Any>(), accessibility, nocache);
			if(!is_undefined(ret)){
				return ret;
			}
		}	
	}

	return undefined;
}
	
const AnyPtr& Class::find_member_from_inherited_classes(const IDPtr& primary_key, const AnyPtr& secondary_key, int_t& accessibility, bool& nocache){
	for(int_t i=inherited_classes_.size(); i>0; --i){
		const AnyPtr& ret = unchecked_ptr_cast<Class>(inherited_classes_.at(i-1))->find_member(primary_key, secondary_key, true, accessibility, nocache);
		if(!is_undefined(ret)){
			return ret;
		}
	}
	return undefined;
}

const AnyPtr& Class::find_member(const IDPtr& primary_key, const AnyPtr& secondary_key, bool inherited_too, int_t& accessibility, bool& nocache){
	const AnyPtr& ret = find_member(primary_key, secondary_key, accessibility, nocache);
	if(!is_undefined(ret)){
		return ret;
	}

	for(int_t i=0; i<CppClassSymbolData::BIND; ++i){
		if(bind(i)){
			const AnyPtr& ret = find_member(primary_key, secondary_key, accessibility, nocache);
			if(!is_undefined(ret)){
				return ret;
			}
		}
	}

	if(inherited_too){
		const AnyPtr& ret = find_member_from_inherited_classes(primary_key, secondary_key, accessibility, nocache);
		if(!is_undefined(ret)){
			return ret;
		}
	}

	return undefined;
}

const AnyPtr& Class::on_rawmember(const IDPtr& primary_key, const AnyPtr& secondary_key, bool inherited_too, int_t& accessibility, bool& nocache){
	const AnyPtr& ret = find_member(primary_key, secondary_key, inherited_too, accessibility, nocache);
	if(!is_undefined(ret)){
		return ret;
	}

	if(inherited_too){
		return cpp_class<Any>()->find_member(primary_key, secondary_key, false, accessibility, nocache);
	}

	return undefined;
}

bool Class::set_member(const IDPtr& primary_key, const AnyPtr& value, const AnyPtr& secondary_key){
	map_t::iterator it = find(primary_key, secondary_key);
	if(it==map_members_->end()){
		set_runtime_error(Xid(undefined));
		return false;
	}

	Frame::set_member_direct(it->second.num, value);
	value->set_object_parent(to_smartptr(this));
	invalidate_cache_member();
	return true;
}

void Class::set_member_direct(int_t i, const IDPtr& primary_key, const AnyPtr& value, const AnyPtr& secondary_key, int_t accessibility){
	Frame::set_member_direct(i, value);
	Key key = {primary_key, secondary_key};
	Value val = {i, accessibility};
	map_members_->insert(key, val);
	value->set_object_parent(to_smartptr(this));
	invalidate_cache_member();
}

void Class::on_set_object_parent(const ClassPtr& parent){
	if(object_force_<parent->object_force()){
		object_force_ = parent->object_force()-1;
		HaveParentBase::on_set_object_parent(parent);
		if(map_members_){
			for(map_t::iterator it=map_members_->begin(), last=map_members_->end(); it!=last; ++it){
				member_direct(it->second.num)->set_object_parent(to_smartptr(this));
			}
		}
	}
}

ValuesPtr Class::child_object_name(const AnyPtr& a){
	if(map_members_){
		for(map_t::iterator it=map_members_->begin(), last=map_members_->end(); it!=last; ++it){
			if(raweq(member_direct(it->second.num), a)){
				return mv(it->first.primary_key, it->first.secondary_key);
			}
		}
	}
	return nul<Values>();
}

const IDPtr& Class::object_temporary_name(){
	if(name_ && name_->data_size()!=0){
		return name_;
	}

	if(symbol_data_ && (symbol_data_->flags&CppClassSymbolData::FLAG_NAME)){
		name_ = *(StringLiteral*)(symbol_data_->name);
		return name_;
	}

	if(code() && info()){
		return code()->identifier(info()->name_number);
	}

	return empty_id;
}

void Class::set_object_temporary_name(const IDPtr& name){
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
		if(unchecked_ptr_cast<Class>(inherited_classes_.at(i))->is_inherited(v)){
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
		if(unchecked_ptr_cast<Class>(inherited_classes_.at(i))->is_inherited_cpp_class()){
			return true;
		}
	}

	return false;
}

void Class::on_rawcall(const VMachinePtr& vm){
	prebind();

	if(is_singleton()){
		return Any::rawsend(vm, Xid(op_call));
	}

	if(is_native()){
		if(const NativeFunPtr& ret = ctor()){
			ret->rawcall(vm);
			if(vm->except()){
				return;
			}
			init_instance(vm->result(), vm);
		}
		else{
			set_runtime_error(Xt1("XRE1013", object, object_name()), vm);
		}
	}
	else{
		AnyPtr instance;
		if(const NativeFunPtr& newfun = ctor()){
			instance = newfun->call();
		}
		else{
			instance = XNew<Base>();
		}

		if(type(instance)==TYPE_BASE){
			pvalue(instance)->set_class(to_smartptr(this));
		}

		init_instance(instance, vm);

		XTAL_CHECK_EXCEPT(e){ return; }
		
		if(const AnyPtr& ret = member(Xid(initialize), undefined)){
			vm->set_arg_this(instance);
			if(vm->need_result()){
				vm->prereturn_result(instance);
			}
			ret->rawcall(vm);
		}
		else{
			vm->return_result(instance);
		}
	}
}

void Class::s_new(const VMachinePtr& vm){
	if(is_singleton()){
		set_runtime_error(Xt1("XRE1013", object, object_name()), vm);
		return;
	}

	if(is_native()){
		if(const AnyPtr& ret = serial_ctor()){
			ret->rawcall(vm);
			init_instance(vm->result(), vm);
		}
		else{
			set_runtime_error(Xt1("XRE1013", object, object_name()), vm);
		}
	}
	else{
		AnyPtr instance;
		if(const AnyPtr& newfun = serial_ctor()){
			instance = newfun->call();
		}
		else{
			instance = XNew<Base>();
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

	for(int_t i=0, sz=inherited_classes_.size(); i<sz; i++){
		const ClassPtr& it = unchecked_ptr_cast<Class>(get_class()->inherited_classes_.at(i));
		ret->push_back(it);

		Xfor(it2, it->ancestors()){
			ret->push_back(it2);
		}
	}

	ret->push_back(cpp_class<Any>());
	return ret;
}

void Class::prebind(){
	if((flags_ & FLAG_PREBINDED)==0){
		flags_ |= FLAG_PREBINDED;
		if(symbol_data_ && (symbol_data_->flags&CppClassSymbolData::FLAG_BIND0)){
			symbol_data_->prebind(this);
		}
	}
}

bool Class::bind(int_t n){
	if(n<0){
		bool ret = false;
		for(int_t i=0; i<CppClassSymbolData::BIND; ++i){
			if(bind(i)){
				ret = true;
			}
		}
		return ret;
	}

	if((flags_ & (FLAG_BINDED<<n))==0){
		flags_ |= (FLAG_BINDED<<n);
		prebind();
		if(symbol_data_ && (symbol_data_->flags&(CppClassSymbolData::FLAG_BIND1<<n))){
			symbol_data_->bind[n](this);
			return true;
		}
	}

	return false;
}

void Class::set_singleton(){
	if((flags_ & FLAG_SINGLETON)==0){
		flags_ |= FLAG_SINGLETON;
		inherit(get_class());
		Base::set_class(to_smartptr(this));
	}
}

void Class::init_singleton(const VMachinePtr& vm){
	init_instance(to_smartptr(this), vm);
	
	if(const AnyPtr& ret = member(Xid(initialize), undefined)){
		vm->setup_call(0);
		vm->set_arg_this(to_smartptr(this));
		ret->rawcall(vm);
		vm->cleanup_call();
	}
}

}
