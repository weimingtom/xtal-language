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
		p->method("members", &Frame::members);
	}

	{
		ClassPtr p = new_cpp_class<Class>("Class");
		p->inherit(get_cpp_class<Frame>());
		p->method("inherit", &Class::inherit_strict);
		p->method("overwrite", &Class::overwrite);
		p->method("s_new", &Class::s_new);
		p->method("inherited_classes", &Class::inherited_classes);
		p->method("is_inherited", &Any::is_inherited);
		p->method("find_near_member", &Class::find_near_member)->param("primary_key", Named("secondary_key", null));
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

	builtin()->def("Frame", get_cpp_class<Frame>());
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

Frame::Frame(const FramePtr& outer, const CodePtr& code, ScopeCore* core)
	:outer_(outer), code_(code), core_(core ? core : &empty_class_core), members_(xnew<Array>(core_->variable_size)), map_members_(0){
}

Frame::Frame()
	:outer_(null), code_(null), core_(&empty_class_core), members_(xnew<Array>()), map_members_(0){}
	
Frame::Frame(const Frame& v)
	:HaveName(v), outer_(v.outer_), code_(v.code_), core_(v.core_), members_(v.members_), map_members_(0){

	if(v.map_members_){
		make_map_members();
		*map_members_ = *v.map_members_;
	}

	global_mutate_count++;
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

	global_mutate_count++;

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
	global_mutate_count++;
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
		set_object_name(ptr_cast<String>(Xf("<(%s):%s:%d>")(get_class()->object_name(depth), code_->source_file_name(), code_->compliant_lineno(code_->data()+core_->pc))), 1, parent_);
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


Class::Class(const FramePtr& outer, const CodePtr& code, ClassCore* core)
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
		global_mutate_count++;
	}
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

IDPtr Class::find_near_member(const IDPtr& primary_key, const AnyPtr& secondary_key){
	int_t minv = 0xffffff;
	IDPtr minid = null;
	Xfor(v, send(Xid(members))){
		IDPtr id = ptr_cast<ID>(v[0]);
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

const CFunPtr& Class::def_and_return(const IDPtr& primary_key, const CFunEssence& cfun, const AnyPtr& secondary_key, int_t accessibility){
	return static_ptr_cast<CFun>(def2(primary_key, new_cfun(cfun), secondary_key, accessibility));
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
		global_mutate_count++;
	}
	else{
		XTAL_THROW(builtin()->member("RedefinedError")(Xt("Xtal Runtime Error 1011")(Named("object", this->object_name()), Named("name", primary_key))), return);
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
		if(const AnyPtr& ret = static_ptr_cast<Class>(mixins_->at(i-1))->member(name)){
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
				XTAL_THROW(builtin()->member("AccessibilityError")(Xt("Xtal Runtime Error 1017")(
					Named("object", this->object_name()), Named("name", primary_key), Named("accessibility", "protected")))
				, return undefined);			
			}

			return members_->at(it->second.num);
		}
	}
	
	// 継承しているクラスを順次検索
	if(inherited_too){
		for(int_t i=0, sz=mixins_->size(); i<sz; ++i){
			const AnyPtr& ret = static_ptr_cast<Class>(mixins_->at(i))->member(primary_key, secondary_key, self);
			if(rawne(ret, undefined)){
				return ret;
			}
		}
	}

	return undefined;
}

const AnyPtr& Class::do_member(const IDPtr& primary_key, const AnyPtr& secondary_key, const AnyPtr& self, bool inherited_too){
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
			const AnyPtr& ret = do_member(primary_key, klass->mixins_->at(i), self, inherited_too);
			if(rawne(ret, undefined)){
				return ret;
			}
		}

		if(rawne(get_cpp_class<Any>(), klass)){
			const AnyPtr& ret = do_member(primary_key, get_cpp_class<Any>(), self, inherited_too);
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
		XTAL_THROW(builtin()->member("RuntimeError")("undefined"), return);
	}
	else{
		members_->set_at(it->second.num, value);
		//value.set_object_name(name, object_name_force(), this);
	}

	global_mutate_count++;
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
		if(static_ptr_cast<Class>(mixins_->at(i))->is_inherited_cpp_class()){
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
			ret->call(vm);
			vm->replace_result(0, instance);
		}
		else{
			ret->call(vm);
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
		instance = newfun();
	}
	else{
		instance = xnew<Base>();
		pvalue(instance)->set_xtal_instance_flag();
	}

	pvalue(instance)->set_class(from_this(this));
	init_instance(instance, vm);

	vm->return_result(instance);
}

CppClass::CppClass(const StringPtr& name)
	:Class(cpp_class_t(), name){
}

void CppClass::call(const VMachinePtr& vm){
	if(const AnyPtr& ret = member(Xid(new), null, from_this(this))){
		ret->call(vm);
		init_instance(vm->result(), vm);
	}
	else{
		XTAL_THROW(builtin()->member("RuntimeError")(Xt("Xtal Runtime Error 1013")(object_name())), return);
	}
}

void CppClass::s_new(const VMachinePtr& vm){
	if(const AnyPtr& ret = member(Xid(serial_new), null, from_this(this))){
		ret->call(vm);
		init_instance(vm->result(), vm);
	}
	else{
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

const AnyPtr& Lib::do_member(const IDPtr& primary_key, const AnyPtr& secondary_key, const AnyPtr& self, bool inherited_too){
	Key key = {primary_key, secondary_key};
	map_t::iterator it = map_members_->find(key);
	if(it!=map_members_->end()){
		return members_->at(it->second.num);
	}
	else{
		Xfor(var, load_path_list_){
			StringPtr file_name = Xf("%s%s%s%s")(var, join_path("/"), primary_key, ".xtal")->to_s();
			return rawdef(primary_key, load(file_name), secondary_key);
		}
		return undefined;

		/* 指定した名前をフォルダーとみなす
		ArrayPtr next = path_.clone();
		next.push_back(name);
		AnyPtr lib = xnew<Lib>(next);
		return rawdef(name, lib, secondary_key);
		*/
	}
}

void Lib::def(const IDPtr& primary_key, const AnyPtr& value, const AnyPtr& secondary_key, int_t accessibility){
	rawdef(primary_key, value, secondary_key);
}

const AnyPtr& Lib::rawdef(const IDPtr& primary_key, const AnyPtr& value, const AnyPtr& secondary_key){
	Key key = {primary_key, secondary_key};
	map_t::iterator it = map_members_->find(key);
	if(it==map_members_->end()){
		Value val = {members_->size(), KIND_PUBLIC};
		map_members_->insert(key, val);
		members_->push_back(value);
		global_mutate_count++;
		value->set_object_name(primary_key, object_name_force(), from_this(this));
		return members_->back();
	}
	else{
		XTAL_THROW(builtin()->member("RedefinedError")(Xt("Xtal Runtime Error 1011")(Named("object", this->object_name()))), return undefined);
	}
}

StringPtr Lib::join_path(const StringPtr& sep){
	if(path_->empty()){
		return sep;
	}
	else{
		return sep->cat(path_->join(sep))->cat(sep);
	}
}

Singleton::Singleton(const StringPtr& name)
	:Class(name){
	Base::set_class(from_this(this));
	inherit(get_cpp_class<Class>());
}

Singleton::Singleton(const FramePtr& outer, const CodePtr& code, ClassCore* core)
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

const ClassPtr& new_cpp_class_impl(CppClassHolderList& list, const StringPtr& name){
	if(!list.value){
		chain_cpp_class(list);
		list.value = xnew<CppClass>(name);
	}
	return list.value;
}


}
