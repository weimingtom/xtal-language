#include "xtal.h"
#include "xtal_macro.h"

namespace xtal{


AnyPtr operator +(const AnyPtr& a){ return a->send(Xid(op_pos)); }
AnyPtr operator -(const AnyPtr& a){ return a->send(Xid(op_neg)); }
AnyPtr operator ~(const AnyPtr& a){ return a->send(Xid(op_com)); }

AnyPtr operator +(const AnyPtr& a, const AnyPtr& b){ return a->send2(Xid(op_add), b->get_class(), b); }
AnyPtr operator -(const AnyPtr& a, const AnyPtr& b){ return a->send2(Xid(op_sub), b->get_class(), b); }
AnyPtr operator *(const AnyPtr& a, const AnyPtr& b){ return a->send2(Xid(op_mul), b->get_class(), b); }
AnyPtr operator /(const AnyPtr& a, const AnyPtr& b){ return a->send2(Xid(op_div), b->get_class(), b); }
AnyPtr operator %(const AnyPtr& a, const AnyPtr& b){ return a->send2(Xid(op_mod), b->get_class(), b); }
AnyPtr operator |(const AnyPtr& a, const AnyPtr& b){ return a->send2(Xid(op_or), b->get_class(), b); }
AnyPtr operator &(const AnyPtr& a, const AnyPtr& b){ return a->send2(Xid(op_and), b->get_class(), b); }
AnyPtr operator ^(const AnyPtr& a, const AnyPtr& b){ return a->send2(Xid(op_xor), b->get_class(), b); }
AnyPtr operator >>(const AnyPtr& a, const AnyPtr& b){ return a->send2(Xid(op_shr), b->get_class(), b); }
AnyPtr operator <<(const AnyPtr& a, const AnyPtr& b){ return a->send2(Xid(op_shl), b->get_class(), b); }

bool operator ==(const AnyPtr& a, const AnyPtr& b){ 
	if(raweq(a, b))
		return true;

	const VMachinePtr& vm = vmachine();
	vm->setup_call(1, b);
	a->rawsend(vm, Xid(op_eq), b->get_class());
	if(vm->processed() && vm->result()){
		vm->cleanup_call();
		return true;
	}
	vm->return_result();
	vm->cleanup_call();
	return false;
}

bool operator !=(const AnyPtr& a, const AnyPtr& b){ return !(a==b); }
bool operator <(const AnyPtr& a, const AnyPtr& b){ return a->send2(Xid(op_lt), b->get_class(), b); }
bool operator >(const AnyPtr& a, const AnyPtr& b){ return b<a; }
bool operator <=(const AnyPtr& a, const AnyPtr& b){ return !(b<a); }
bool operator >=(const AnyPtr& a, const AnyPtr& b){ return !(a<b); }

AnyPtr& operator +=(AnyPtr& a, const AnyPtr& b){ a = a->send2(Xid(op_add_assign), b->get_class(), b); return a; }
AnyPtr& operator -=(AnyPtr& a, const AnyPtr& b){ a = a->send2(Xid(op_sub_assign), b->get_class(), b); return a; }
AnyPtr& operator *=(AnyPtr& a, const AnyPtr& b){ a = a->send2(Xid(op_mul_assign), b->get_class(), b); return a; }
AnyPtr& operator /=(AnyPtr& a, const AnyPtr& b){ a = a->send2(Xid(op_div_assign), b->get_class(), b); return a; }
AnyPtr& operator %=(AnyPtr& a, const AnyPtr& b){ a = a->send2(Xid(op_mod_assign), b->get_class(), b); return a; }
AnyPtr& operator |=(AnyPtr& a, const AnyPtr& b){ a = a->send2(Xid(op_or_assign), b->get_class(), b); return a; }
AnyPtr& operator &=(AnyPtr& a, const AnyPtr& b){ a = a->send2(Xid(op_and_assign), b->get_class(), b); return a; }
AnyPtr& operator ^=(AnyPtr& a, const AnyPtr& b){ a = a->send2(Xid(op_xor_assign), b->get_class(), b); return a; }
AnyPtr& operator >>=(AnyPtr& a, const AnyPtr& b){ a = a->send2(Xid(op_shr_assign), b->get_class(), b); return a; }
AnyPtr& operator <<=(AnyPtr& a, const AnyPtr& b){ a = a->send2(Xid(op_shl_assign), b->get_class(), b); return a; }

Innocence::Innocence(const char_t* str){
	*this = xnew<String>(str);
}

Innocence::Innocence(const avoid<char>::type* str){
	*this = xnew<String>(str);
}

SmartPtr<Any>& SmartPtr<Any>::operator =(const SmartPtr<Any>& p){
	return ap_copy(*this, p);
}

void SmartPtr<Any>::set_p_with_class(Base* p, const ClassPtr& c){
	Innocence::set_p(p);
	p->set_class(c);
	register_gc(p);
}

SmartPtr<Any>::SmartPtr(Base* p, const ClassPtr& c, with_class_t)
	:Innocence(p){
	p->set_class(c);
	register_gc(p);
}

SmartPtr<Any>::SmartPtr(Singleton* p, const ClassPtr& c, with_class_t)
	:Innocence(p){
	register_gc(p);
}

SmartPtr<Any>::SmartPtr(CppSingleton* p, const ClassPtr& c, with_class_t)
	:Innocence(p){
	register_gc(p);
}

AnyPtr Any::send(const IDPtr& primary_key) const{
	const VMachinePtr& vm = vmachine();
	vm->setup_call(1);
	rawsend(vm, primary_key);
	return vm->result_and_cleanup_call();
}

AnyPtr Any::send(const IDPtr& primary_key, const Param& a0) const{
	const VMachinePtr& vm = vmachine();
	vm->setup_call(1, a0);
	rawsend(vm, primary_key);
	return vm->result_and_cleanup_call();
}

AnyPtr Any::send(const IDPtr& primary_key, const Param& a0, const Param& a1) const{
	const VMachinePtr& vm = vmachine();
	vm->setup_call(1, a0, a1);
	rawsend(vm, primary_key);
	return vm->result_and_cleanup_call();
}

AnyPtr Any::send(const IDPtr& primary_key, const Param& a0, const Param& a1, const Param& a2) const{
	const VMachinePtr& vm = vmachine();
	vm->setup_call(1, a0, a1, a2);
	rawsend(vm, primary_key);
	return vm->result_and_cleanup_call();
}

AnyPtr Any::send(const IDPtr& primary_key, const Param& a0, const Param& a1, const Param& a2, const Param& a3) const{
	const VMachinePtr& vm = vmachine();
	vm->setup_call(1, a0, a1, a2, a3);
	rawsend(vm, primary_key);
	return vm->result_and_cleanup_call();
}

AnyPtr Any::send(const IDPtr& primary_key, const Param& a0, const Param& a1, const Param& a2, const Param& a3, const Param& a4) const{
	const VMachinePtr& vm = vmachine();
	vm->setup_call(1, a0, a1, a2, a3, a4);
	rawsend(vm, primary_key);
	return vm->result_and_cleanup_call();
}


AnyPtr Any::send2(const IDPtr& primary_key, const AnyPtr& secondary_key) const{
	const VMachinePtr& vm = vmachine();
	vm->setup_call(1);
	rawsend(vm, primary_key, secondary_key);
	return vm->result_and_cleanup_call();
}

AnyPtr Any::send2(const IDPtr& primary_key, const AnyPtr& secondary_key, const Param& a0) const{
	const VMachinePtr& vm = vmachine();
	vm->setup_call(1, a0);
	rawsend(vm, primary_key, secondary_key);
	return vm->result_and_cleanup_call();
}

AnyPtr Any::send2(const IDPtr& primary_key, const AnyPtr& secondary_key, const Param& a0, const Param& a1) const{
	const VMachinePtr& vm = vmachine();
	vm->setup_call(1, a0, a1);
	rawsend(vm, primary_key, secondary_key);
	return vm->result_and_cleanup_call();
}

AnyPtr Any::send2(const IDPtr& primary_key, const AnyPtr& secondary_key, const Param& a0, const Param& a1, const Param& a2) const{
	const VMachinePtr& vm = vmachine();
	vm->setup_call(1, a0, a1, a2);
	rawsend(vm, primary_key);
	return vm->result_and_cleanup_call();
}

AnyPtr Any::send2(const IDPtr& primary_key, const AnyPtr& secondary_key, const Param& a0, const Param& a1, const Param& a2, const Param& a3) const{
	const VMachinePtr& vm = vmachine();
	vm->setup_call(1, a0, a1, a2, a3);
	rawsend(vm, primary_key, secondary_key);
	return vm->result_and_cleanup_call();
}

AnyPtr Any::send2(const IDPtr& primary_key, const AnyPtr& secondary_key, const Param& a0, const Param& a1, const Param& a2, const Param& a3, const Param& a4) const{
	const VMachinePtr& vm = vmachine();
	vm->setup_call(1, a0, a1, a2, a3, a4);
	rawsend(vm, primary_key, secondary_key);
	return vm->result_and_cleanup_call();
}

AnyPtr Any::call() const{
	const VMachinePtr& vm = vmachine();
	vm->setup_call(1);
	ap(*this)->rawcall(vm);
	return vm->result_and_cleanup_call();
}

AnyPtr Any::call(const Param& a0) const{
	const VMachinePtr& vm = vmachine();
	vm->setup_call(1, a0);
	ap(*this)->rawcall(vm);
	return vm->result_and_cleanup_call();
}

AnyPtr Any::call(const Param& a0, const Param& a1) const{
	const VMachinePtr& vm = vmachine();
	vm->setup_call(1, a0, a1);
	ap(*this)->rawcall(vm);
	return vm->result_and_cleanup_call();
}

AnyPtr Any::call(const Param& a0, const Param& a1, const Param& a2) const{
	const VMachinePtr& vm = vmachine();
	vm->setup_call(1, a0, a1, a2);
	ap(*this)->rawcall(vm);
	return vm->result_and_cleanup_call();
}

AnyPtr Any::call(const Param& a0, const Param& a1, const Param& a2, const Param& a3) const{
	const VMachinePtr& vm = vmachine();
	vm->setup_call(1, a0, a1, a2, a3);
	ap(*this)->rawcall(vm);
	return vm->result_and_cleanup_call();
}

AnyPtr Any::call(const Param& a0, const Param& a1, const Param& a2, const Param& a3, const Param& a4) const{
	const VMachinePtr& vm = vmachine();
	vm->setup_call(1, a0, a1, a2, a3, a4);
	ap(*this)->rawcall(vm);
	return vm->result_and_cleanup_call();
}

const AnyPtr& Any::member(const IDPtr& primary_key, const AnyPtr& secondary_key, const AnyPtr& self, bool inherited_too) const{
	return environment()->cache_member(*this, primary_key, secondary_key, self, inherited_too);
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
		vm->set_unsuported_error_info(*this, primary_key, secondary_key);
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
		XTAL_DEFAULT{}
		XTAL_CASE(TYPE_BASE){ if(const StringPtr& ret = ptr_as<String>(ap(*this)))return ret; }
		XTAL_CASE(TYPE_SMALL_STRING){ static_ptr_cast<String>(ap(*this)); }
	}
	return ptr_cast<String>((*this).send(Xid(to_s)));
}

ArrayPtr Any::to_a() const{
	return ptr_cast<Array>((*this).send(Xid(to_a)));
}

MapPtr Any::to_m() const{
	return ptr_cast<Map>((*this).send(Xid(to_m)));
}

StringPtr Any::object_name(int_t depth) const{
	switch(type(*this)){
		XTAL_DEFAULT{ return StringPtr("instance of ")->cat(get_class()->object_name(depth)); }
		XTAL_CASE(TYPE_BASE){ return pvalue(*this)->object_name(depth); }
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
		XTAL_CASE(TYPE_FALSE){ return get_cpp_class<False>(); }
		XTAL_CASE(TYPE_TRUE){ return get_cpp_class<True>(); }
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
	return environment()->cache_is(my_class, klass);
}

bool Any::is_inherited(const AnyPtr& klass) const{
	if(raweq(*this, klass)) return true;
	return environment()->cache_is_inherited(*this, klass);
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
			ret->set_at(it->object_name(), serial_save(static_ptr_cast<Class>(it)));
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
			serial_load(static_ptr_cast<Class>(it), ret->at(it->object_name()));
		}
	}
}

AnyPtr Any::serial_save(const ClassPtr& p) const{
	if(type(*this)!=TYPE_BASE){
		return null;
	}

	if(InstanceVariables* iv = pvalue(*this)->instance_variables()){
		if(CodePtr code = p->code()){
			ClassCore* core = p->core();
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
		if(MapPtr insts = ptr_as<Map>(v)){
			if(CodePtr code = p->code()){
				ClassCore* core = p->core();
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


//////////////////////////////////

namespace{

	bool op_in_Any_Array(const AnyPtr& v, const ArrayPtr& values){
		Xfor(v2, values){
			if(v == v2){
				return true;
			}
		}
		return false;
	}

	bool op_in_Any_Set(const AnyPtr& v, const SetPtr& values){
		return values->at(v);
	}
}

void initialize_any(){
	
	{
		ClassPtr p = new_cpp_class<Any>(Xid(Any));
		p->method(Xid(class), &Any::get_class);
		p->method(Xid(get_class), &Any::get_class);
		p->method(Xid(self), &Any::self);
		p->method(Xid(object_name), &Any::object_name)->param(Named(Xid(depth), -1));
		p->method(Xid(s_save), &Any::s_save);
		p->method(Xid(s_load), &Any::s_load);
		p->method(Xid(to_mv), &Any::to_mv);
		p->method(Xid(flatten_mv), &Any::flatten_mv);
		p->method(Xid(flatten_all_mv), &Any::flatten_all_mv);

		p->dual_dispatch_method(Xid(op_add));
		p->dual_dispatch_method(Xid(op_sub));
		p->dual_dispatch_method(Xid(op_cat));
		p->dual_dispatch_method(Xid(op_mul));
		p->dual_dispatch_method(Xid(op_div));
		p->dual_dispatch_method(Xid(op_mod));
		p->dual_dispatch_method(Xid(op_and));
		p->dual_dispatch_method(Xid(op_or));
		p->dual_dispatch_method(Xid(op_xor));
		p->dual_dispatch_method(Xid(op_shr));
		p->dual_dispatch_method(Xid(op_shl));
		p->dual_dispatch_method(Xid(op_ushr));

		p->dual_dispatch_method(Xid(op_add_assign));
		p->dual_dispatch_method(Xid(op_sub_assign));
		p->dual_dispatch_method(Xid(op_cat_assign));
		p->dual_dispatch_method(Xid(op_mul_assign));
		p->dual_dispatch_method(Xid(op_div_assign));
		p->dual_dispatch_method(Xid(op_mod_assign));
		p->dual_dispatch_method(Xid(op_and_assign));
		p->dual_dispatch_method(Xid(op_or_assign));
		p->dual_dispatch_method(Xid(op_xor_assign));
		p->dual_dispatch_method(Xid(op_shr_assign));
		p->dual_dispatch_method(Xid(op_shl_assign));
		p->dual_dispatch_method(Xid(op_ushr_assign));

		p->dual_dispatch_method(Xid(op_eq));
		p->dual_dispatch_method(Xid(op_lt));

		p->dual_dispatch_method(Xid(op_at));
		p->dual_dispatch_method(Xid(op_set_at));

		p->dual_dispatch_method(Xid(op_call));

		p->dual_dispatch_method(Xid(op_range));
		p->method(Xid(op_in), &op_in_Any_Array, new_cpp_class<Array>());
		p->method(Xid(op_in), &op_in_Any_Set, new_cpp_class<Set>());
	}

	builtin()->def(Xid(Any), get_cpp_class<Any>());
}

}
