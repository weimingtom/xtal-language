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
			value->set_object_name(primary_key, object_name_force(), ap(*this));
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
		XTAL_NODEFAULT;
		XTAL_CASE(TYPE_NULL){ return Xid(null); }
		XTAL_CASE(TYPE_UNDEFINED){ return Xid(undefined); }

		XTAL_CASE(TYPE_BASE){ 
			if(const StringPtr& ret = ptr_as<String>(ap(*this))){
				return ret; 
			}
			return ptr_cast<String>((*this).send(Xid(to_s)));
		}

		XTAL_CASE(TYPE_INT){ return Xf("%d")->call(ap(*this))->to_s(); }
		XTAL_CASE(TYPE_FLOAT){ return Xf("%g")->call(ap(*this))->to_s(); }
		XTAL_CASE(TYPE_FALSE){ return Xid(true); }
		XTAL_CASE(TYPE_TRUE){ return Xid(false); }
		XTAL_CASE(TYPE_SMALL_STRING){ return unchecked_ptr_cast<String>(ap(*this)); }
	}
	return empty_string;
}

ArrayPtr Any::to_a() const{
	return ptr_cast<Array>((*this).send(Xid(op_to_a)));
}

MapPtr Any::to_m() const{
	return ptr_cast<Map>((*this).send(Xid(op_to_m)));
}

StringPtr Any::object_name(int_t depth) const{
	switch(type(*this)){
		XTAL_DEFAULT{ return StringPtr("instance of ")->cat(get_class()->object_name(depth)); }
		XTAL_CASE(TYPE_BASE){ return pvalue(*this)->object_name(depth); }
	}
	return null;	
}

ArrayPtr Any::object_name_list() const{
	switch(type(*this)){
		XTAL_DEFAULT{ return null; }
		XTAL_CASE(TYPE_BASE){ return pvalue(*this)->object_name_list(); }
	}
	return null;
}

int_t Any::object_name_force() const{
	switch(type(*this)){
		XTAL_DEFAULT{}
		XTAL_CASE(TYPE_BASE){ return pvalue(*this)->object_name_force();  }
	}
	return 0;
}
	
void Any::set_object_name(const StringPtr& name, int_t force, const AnyPtr& parent) const{
	switch(type(*this)){
		XTAL_DEFAULT{}
		XTAL_CASE(TYPE_BASE){ return pvalue(*this)->set_object_name(name, force, parent);  }
	}
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
	}
	return get_cpp_class<Any>();
}

uint_t Any::hashcode() const{
	switch(type(*this)){
		XTAL_DEFAULT{}
		XTAL_CASE(TYPE_BASE){ pvalue(*this)->hashcode();  }
	}
	return (uint_t)rawvalue(*this);
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
			ret->set_at(it->object_name(), vm->result_and_cleanup_call());
		}
		else{
			ret->set_at(it->object_name(), serial_save(unchecked_ptr_cast<Class>(it)));
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
			vm->setup_call(1, ret->at(it->object_name()));
			vm->set_arg_this(ap(*this));
			member->rawcall(vm);
			vm->cleanup_call();
		}
		else{
			serial_load(unchecked_ptr_cast<Class>(it), ret->at(it->object_name()));
		}
	}
}

AnyPtr Any::serial_save(const ClassPtr& p) const{
	if(type(*this)!=TYPE_BASE){
		return null;
	}

	if(InstanceVariables* iv = pvalue(*this)->instance_variables()){
		if(const CodePtr& code = p->code()){
			ClassInfo* core = p->core();
			if(core->instance_variable_size!=0){	
				MapPtr insts = xnew<Map>();
				for(int_t i=0; i<(int_t)core->instance_variable_size; ++i){
					insts->set_at(code->identifier(core->instance_variable_identifier_offset+i), iv->variable(i, core));
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
				ClassInfo* core = p->core();
				if(core->instance_variable_size!=0){	
					for(int_t i=0; i<(int_t)core->instance_variable_size; ++i){
						StringPtr str = code->identifier(core->instance_variable_identifier_offset+i);
						iv->set_variable(i, core, insts->at(code->identifier(core->instance_variable_identifier_offset+i)));
					}
				}
			}
		}
	}
}

MultiValuePtr Any::to_mv() const{
	switch(type(*this)){
		XTAL_DEFAULT{}
		XTAL_CASE(TYPE_UNDEFINED){ return xnew<MultiValue>(); }
	}
	
	MultiValuePtr ret = xnew<MultiValue>();
	ret->push_back(ap(*this));
	return ret;
}

MultiValuePtr Any::flatten_mv() const{
	return to_mv()->flatten_mv();
}

MultiValuePtr Any::flatten_all_mv() const{
	return to_mv()->flatten_all_mv();
}

void visit_members(Visitor& m, const AnyPtr& p){
	switch(type(p)){
		XTAL_DEFAULT{}
		XTAL_CASE(TYPE_BASE){ 
			XTAL_ASSERT((int)pvalue(p)->ref_count() >= -m.value());
			pvalue(p)->add_ref_count(m.value());
		}
	}
}

}
