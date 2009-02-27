#include "xtal.h"
#include "xtal_macro.h"

namespace xtal{

/// @brief primary_keyメソッドを呼び出す
AnyPtr Any::send(const IDPtr& primary_key) const{
	const VMachinePtr& vm = vmachine();
	vm->setup_call();
	rawsend(vm, primary_key);
	return vm->result_and_cleanup_call();
}

/// @brief primary_key#secondary_keyメソッドを呼び出す
AnyPtr Any::send2(const IDPtr& primary_key, const AnyPtr& secondary_key) const{
	const VMachinePtr& vm = vmachine();
	vm->setup_call();
	rawsend(vm, primary_key, secondary_key);
	return vm->result_and_cleanup_call();
}

/// @brief 関数を呼び出す
AnyPtr Any::call() const{
	const VMachinePtr& vm = vmachine();
	vm->setup_call();
	rawcall(vm);
	return vm->result_and_cleanup_call();
}

const AnyPtr& Any::member(const IDPtr& primary_key, const AnyPtr& secondary_key, const AnyPtr& self, bool inherited_too) const{
	return core()->cache_member(*this, primary_key, secondary_key, self, inherited_too);
}

void Any::def(const IDPtr& primary_key, const AnyPtr& value, const AnyPtr& secondary_key, int_t accessibility) const{
	switch(type(*this)){
		XTAL_DEFAULT;
		XTAL_CASE(TYPE_BASE){
			pvalue(*this)->def(primary_key, value, secondary_key, accessibility);
		}
	}
}

void Any::rawsend(const VMachinePtr& vm, const IDPtr& primary_key, const AnyPtr& secondary_key, const AnyPtr& self, bool inherited_too) const{
	const ClassPtr& cls = get_class();
	const AnyPtr& ret = ap(cls)->member(primary_key, secondary_key, self, inherited_too);
	if(rawne(ret, undefined)){
		vm->set_arg_this(ap(*this));
		ret->rawcall(vm);
	}
	else{
		const AnyPtr& ret = ap(cls)->member(Xid(send_missing), null, self, inherited_too);
		if(rawne(ret, undefined)){
			vm->set_arg_this(ap(*this));
			ArgumentsPtr args = vm->make_arguments();
			vm->recycle_call();
			vm->push_arg(primary_key);
			vm->push_arg(secondary_key);
			vm->push_arg(args);
			ret->rawcall(vm);
		}
		else{
			vm->set_unsuported_error_info(*this, primary_key, secondary_key);
		}
	}
}

void Any::rawsend(const VMachinePtr& vm, const IDPtr& primary_key) const{
	const ClassPtr& cls = get_class();
	const AnyPtr& ret = ap(cls)->member(primary_key, null, null, true);
	if(rawne(ret, undefined)){
		vm->set_arg_this(ap(*this));
		ret->rawcall(vm);
	}
	else{
		const AnyPtr& ret = ap(cls)->member(Xid(send_missing), null, null, true);
		if(rawne(ret, undefined)){
			vm->set_arg_this(ap(*this));
			ArgumentsPtr args = vm->make_arguments();
			vm->recycle_call();
			vm->push_arg(primary_key);
			vm->push_arg(null);
			vm->push_arg(args);
			ret->rawcall(vm);
		}
		else{
			vm->set_unsuported_error_info(*this, primary_key, null);
		}
	}	
}

void Any::rawcall(const VMachinePtr& vm) const{
	vm->set_hint(ap(*this));
	switch(type(*this)){
		XTAL_DEFAULT{}
		XTAL_CASE(TYPE_BASE){ pvalue(*this)->rawcall(vm); }
		XTAL_CASE(TYPE_NATIVE_FUN){ unchecked_ptr_cast<NativeFun>(ap(*this))->rawcall(vm); }
		XTAL_CASE(TYPE_NATIVE_FUN_BINDED_THIS){ unchecked_ptr_cast<NativeFunBindedThis>(ap(*this))->rawcall(vm); }
	}
}

int_t Any::to_i() const{
	switch(type(*this)){
		XTAL_DEFAULT{ return cast<int_t>((*this).send(Xid(to_i))); }
		XTAL_CASE(TYPE_NULL){ return 0; }
		XTAL_CASE(TYPE_INT){ return ivalue(*this); }
		XTAL_CASE(TYPE_FLOAT){ return (int_t)fvalue(*this); }
	}
	return 0;
}

float_t Any::to_f() const{
	switch(type(*this)){
		XTAL_DEFAULT{ return cast<float_t>((*this).send(Xid(to_f))); }
		XTAL_CASE(TYPE_NULL){ return 0; }
		XTAL_CASE(TYPE_INT){ return (float_t)ivalue(*this); }
		XTAL_CASE(TYPE_FLOAT){ return fvalue(*this); }
	}
	return 0;
}

StringPtr Any::to_s() const{
	switch(type(*this)){
		XTAL_DEFAULT{
			return ptr_cast<String>((*this).send(Xid(to_s)));
		}

		XTAL_CASE(TYPE_NULL){ return Xid(null); }
		XTAL_CASE(TYPE_UNDEFINED){ return Xid(undefined); }

		XTAL_CASE(TYPE_BASE){ 
			return ptr_cast<String>((*this).send(Xid(to_s)));
		}

		XTAL_CASE(TYPE_INT){ return Xf("%d")->call(ap(*this))->to_s(); }
		XTAL_CASE(TYPE_FLOAT){ return Xf("%g")->call(ap(*this))->to_s(); }
		XTAL_CASE(TYPE_FALSE){ return Xid(true); }
		XTAL_CASE(TYPE_TRUE){ return Xid(false); }
		XTAL_CASE(TYPE_SMALL_STRING){ return unchecked_ptr_cast<String>(ap(*this)); }
		XTAL_CASE(TYPE_STRING){ return unchecked_ptr_cast<String>(ap(*this)); }
	}
	return empty_string;
}

ArrayPtr Any::to_a() const{
	return ptr_cast<Array>((*this).send(Xid(op_to_a)));
}

MapPtr Any::to_m() const{
	return ptr_cast<Map>((*this).send(Xid(op_to_m)));
}

const ClassPtr& Any::object_parent() const{
	switch(type(*this)){
		XTAL_DEFAULT{}
		XTAL_CASE(TYPE_BASE){ return pvalue(*this)->object_parent(); }
		XTAL_CASE(TYPE_NATIVE_FUN){ return unchecked_ptr_cast<NativeFun>(ap(*this))->object_parent();  }
		XTAL_CASE(TYPE_NATIVE_FUN_BINDED_THIS){ return unchecked_ptr_cast<NativeFunBindedThis>(ap(*this))->object_parent();  }
	}
	return null;	
}

void Any::set_object_parent(const ClassPtr& parent) const{
	switch(type(*this)){
		XTAL_DEFAULT{}
		XTAL_CASE(TYPE_BASE){ return pvalue(*this)->set_object_parent(parent);  }
		XTAL_CASE(TYPE_NATIVE_FUN){ return unchecked_ptr_cast<NativeFun>(ap(*this))->set_object_parent(parent);  }
		XTAL_CASE(TYPE_NATIVE_FUN_BINDED_THIS){ return unchecked_ptr_cast<NativeFunBindedThis>(ap(*this))->set_object_parent(parent);  }
	}
}

ArrayPtr Any::object_name_list() const{
	if(const ClassPtr& parent = object_parent()){
		ArrayPtr ret = parent->object_name_list();
		if(AnyPtr name = parent->child_object_name(ap(*this))){
			ret->push_back(parent->child_object_name(ap(*this)));
		}
		return ret;
	}
	else if(const ClassPtr& cls = ptr_as<Class>(ap(*this))){
		ArrayPtr ret = xnew<Array>();
		ret->push_back(mv(cls->object_name(), null));
		return ret;
	}
	return xnew<Array>();
}

StringPtr Any::object_name() const{
	switch(type(*this)){
		XTAL_DEFAULT;
		XTAL_CASE(TYPE_NULL){ return Xid(null); }
		XTAL_CASE(TYPE_UNDEFINED){ return Xid(undefined); }
		XTAL_CASE(TYPE_INT){ return Xf("%d")->call(ap(*this))->to_s(); }
		XTAL_CASE(TYPE_FLOAT){ return Xf("%g")->call(ap(*this))->to_s(); }
		XTAL_CASE(TYPE_FALSE){ return Xid(true); }
		XTAL_CASE(TYPE_TRUE){ return Xid(false); }
		XTAL_CASE(TYPE_SMALL_STRING){ return unchecked_ptr_cast<String>(ap(*this)); }
		XTAL_CASE(TYPE_STRING){ return unchecked_ptr_cast<String>(ap(*this)); }
	}

	// 親がいるなら、親が名前を知っている
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

	// クラスの場合、自身が名前を保持してるかも
	if(const ClassPtr& cls = ptr_as<Class>(ap(*this))){
		if(StringPtr name = cls->object_name()){
			if(rawne(name, empty_string)){
				return name;
			}
		}
		
		if(cls->code()){
			// 保持していないなら、その定義位置を表示しとこう
			return Xf("(instance of %s) %s(%d)")->call(get_class()->object_name(), 
				cls->code()->source_file_name(), 
				cls->code()->compliant_lineno(cls->code()->data()+cls->info()->pc))->to_s();
		}
		else{
			return Xf("(instance of %s)")->call(get_class()->object_name())->to_s();
		}
	}

	// メソッドの場合、その定義位置を表示しとこう
	if(const MethodPtr& mtd = ptr_as<Method>(ap(*this))){
		if(mtd->code()){
			return Xf("(instance of %s) %s(%d)")->call(get_class()->object_name(), 
				mtd->code()->source_file_name(), 
				mtd->code()->compliant_lineno(mtd->code()->data()+mtd->info()->pc))->to_s();
		}
		else{
			return Xf("(instance of %s)")->call(get_class()->object_name())->to_s();
		}
	}

	return Xf("(instance of %s)")->call(get_class()->object_name())->to_s();
}

const ClassPtr& Any::get_class() const{
	switch(type(*this)){
		XTAL_NODEFAULT;
		XTAL_CASE(TYPE_NULL){ return get_cpp_class<Null>(); }
		XTAL_CASE(TYPE_UNDEFINED){ return get_cpp_class<Undefined>(); }
		XTAL_CASE(TYPE_BASE){ return pvalue(*this)->get_class(); }
		XTAL_CASE(TYPE_INT){ return get_cpp_class<Int>(); }
		XTAL_CASE(TYPE_FLOAT){ return get_cpp_class<Float>(); }
		XTAL_CASE(TYPE_FALSE){ return get_cpp_class<Bool>(); }
		XTAL_CASE(TYPE_TRUE){ return get_cpp_class<Bool>(); }
		XTAL_CASE(TYPE_SMALL_STRING){ return get_cpp_class<String>(); }
		XTAL_CASE(TYPE_STRING){ return get_cpp_class<String>(); }
		XTAL_CASE(TYPE_ARRAY){ return get_cpp_class<Array>(); }
		XTAL_CASE(TYPE_MULTI_VALUE){ return get_cpp_class<MultiValue>(); }
		XTAL_CASE(TYPE_TREE_NODE){ return get_cpp_class<xpeg::TreeNode>(); }
		XTAL_CASE(TYPE_NATIVE_FUN){ return get_cpp_class<NativeFun>(); }
		XTAL_CASE(TYPE_NATIVE_FUN_BINDED_THIS){ return get_cpp_class<NativeFunBindedThis>(); }
	}
	return get_cpp_class<Any>();
}

bool Any::is(const AnyPtr& klass) const{
	const ClassPtr& my_class = get_class();
	if(raweq(my_class, klass)) return true;
	return core()->cache_is(my_class, klass);
}

bool Any::is_inherited(const AnyPtr& klass) const{
	if(raweq(*this, klass)) return true;
	return core()->cache_is_inherited(*this, klass);
}

AnyPtr Any::p() const{
	ap(*this)->send(Xid(p));
	return ap(*this);
}

AnyPtr Any::s_save() const{
	MapPtr ret = xnew<Map>();
	ClassPtr klass = get_class();

	ArrayPtr ary = klass->send(Xid(ancestors))->to_a();
	ary->push_back(klass);

	Xfor(it, ary){
		if(const AnyPtr& member = it->member(Xid(serial_save), null, null, false)){
			const VMachinePtr& vm = vmachine();
			vm->setup_call(1);
			vm->set_arg_this(ap(*this));
			member->rawcall(vm);
			ret->set_at(it, vm->result_and_cleanup_call());
		}
		else{
			ret->set_at(it, serial_save(unchecked_ptr_cast<Class>(it)));
		}
	}

	return ret;
}

void Any::s_load(const AnyPtr& v) const{
	MapPtr ret = ptr_cast<Map>(v);
	ClassPtr klass = get_class();

	ArrayPtr ary = klass->ancestors()->to_a();
	ary->push_back(klass);

	Xfor(it, ary){
		if(const AnyPtr& member = it->member(Xid(serial_load), null, null, false)){
			const VMachinePtr& vm = vmachine();
			vm->setup_call(1, ret->at(it));
			vm->set_arg_this(ap(*this));
			member->rawcall(vm);
			vm->cleanup_call();
		}
		else{
			serial_load(unchecked_ptr_cast<Class>(it), ret->at(it));
		}
	}
}

AnyPtr Any::serial_save(const ClassPtr& p) const{
	if(type(*this)!=TYPE_BASE){
		return null;
	}

	if(InstanceVariables* iv = pvalue(*this)->instance_variables()){
		if(const CodePtr& code = p->code()){
			ClassInfo* info = p->info();
			if(info->instance_variable_size!=0){	
				MapPtr insts = xnew<Map>();
				for(int_t i=0; i<(int_t)info->instance_variable_size; ++i){
					insts->set_at(code->identifier(info->instance_variable_identifier_offset+i), iv->variable(i, info));
				}

				return insts;
			}
		}
	}
	return null;
}

void Any::serial_load(const ClassPtr& p, const AnyPtr& v) const{
	if(type(*this)!=TYPE_BASE){
		return;
	}

	if(InstanceVariables* iv = pvalue(*this)->instance_variables()){
		if(const MapPtr& insts = ptr_as<Map>(v)){
			if(const CodePtr& code = p->code()){
				ClassInfo* info = p->info();
				if(info->instance_variable_size!=0){	
					for(int_t i=0; i<(int_t)info->instance_variable_size; ++i){
						StringPtr str = code->identifier(info->instance_variable_identifier_offset+i);
						iv->set_variable(i, info, insts->at(code->identifier(info->instance_variable_identifier_offset+i)));
					}
				}
			}
		}
	}
}

void Any::visit_members(Visitor& m) const{
	switch(type(*this)){
		XTAL_DEFAULT;
		XTAL_CASE(TYPE_BASE){ 
			pvalue(*this)->visit_members(m); 
		}

		XTAL_CASE(TYPE_ARRAY){ 
			((Array*)rcpvalue(*this))->visit_members(m); 
		}

		XTAL_CASE(TYPE_MULTI_VALUE){ 
			((MultiValue*)rcpvalue(*this))->visit_members(m); 
		}

		XTAL_CASE(TYPE_TREE_NODE){ 
			using namespace xpeg;
			((TreeNode*)rcpvalue(*this))->visit_members(m); 
		}

		XTAL_CASE(TYPE_NATIVE_FUN){ 
			unchecked_ptr_cast<NativeFun>(ap(*this))->visit_members(m); 
		}

		XTAL_CASE(TYPE_NATIVE_FUN_BINDED_THIS){ 
			unchecked_ptr_cast<NativeFunBindedThis>(ap(*this))->visit_members(m); 
		}
	}
}

void Any::destroy() const{
	switch(type(*this)){
		XTAL_NODEFAULT;

		XTAL_CASE(TYPE_BASE){
			delete pvalue(*this);
		}

		XTAL_CASE(TYPE_STRING){ 
			uint_t size = StringData::calc_size(((StringData*)rcpvalue(*this))->data_size());
			((StringData*)rcpvalue(*this))->~StringData(); 
			rcpvalue(*this)->value_ = size; 
		}

		XTAL_CASE(TYPE_ARRAY){ 
			((Array*)rcpvalue(*this))->~Array(); 
			rcpvalue(*this)->value_ = sizeof(Array); 
		}

		XTAL_CASE(TYPE_MULTI_VALUE){ 
			((MultiValue*)rcpvalue(*this))->~MultiValue(); 
			rcpvalue(*this)->value_ = sizeof(MultiValue); 
		}

		XTAL_CASE(TYPE_TREE_NODE){ 
			using namespace xpeg;
			((TreeNode*)rcpvalue(*this))->~TreeNode(); 
			rcpvalue(*this)->value_ = sizeof(TreeNode); 
		}

		XTAL_CASE(TYPE_NATIVE_FUN){ 
			unchecked_ptr_cast<NativeFun>(ap(*this))->~NativeFun(); 
			rcpvalue(*this)->value_ = sizeof(NativeFun); 
		}

		XTAL_CASE(TYPE_NATIVE_FUN_BINDED_THIS){ 
			unchecked_ptr_cast<NativeFunBindedThis>(ap(*this))->~NativeFunBindedThis(); 
			rcpvalue(*this)->value_ = sizeof(NativeFunBindedThis); 
		}
	}
}

void Any::object_free(){
	if(type(*this)==TYPE_BASE){
		so_free(static_cast<Base*>(this), value_);
	}
	else if(type(*this)>TYPE_BASE){
		so_free(this, value_);
	}
}

void visit_members(Visitor& m, const AnyPtr& p){
	if(type(p)==TYPE_BASE){
		XTAL_ASSERT((int)pvalue(p)->ref_count() >= -m.value());
		pvalue(p)->add_ref_count(m.value());
	}
	else if(type(p)>TYPE_BASE){
		XTAL_ASSERT((int)rcpvalue(p)->ref_count() >= -m.value());
		rcpvalue(p)->add_ref_count(m.value());
	}
}


void inc_ref_count_force(const Any& v){
	if(type(v)==TYPE_BASE){
		pvalue(v)->inc_ref_count();
	}
	else if(type(v)>TYPE_BASE){
		rcpvalue(v)->inc_ref_count();		
	}
}

void dec_ref_count_force(const Any& v){
	if(type(v)==TYPE_BASE){
		pvalue(v)->dec_ref_count();
	}
	else if(type(v)>TYPE_BASE){
		rcpvalue(v)->dec_ref_count();		
	}
}

}
