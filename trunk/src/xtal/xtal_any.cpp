#include "xtal.h"
#include "xtal_macro.h"
#include "xtal_stringspace.h"
#include "xtal_details.h"

namespace xtal{

/// \brief primary_keyメソッドを呼び出す
AnyPtr Any::send(const IDPtr& primary_key) const{
	const VMachinePtr& vm = setup_call();
	rawsend(vm, primary_key);
	return vm->result_and_cleanup_call();
}

/// \brief primary_key#secondary_keyメソッドを呼び出す
AnyPtr Any::send2(const IDPtr& primary_key, const AnyPtr& secondary_key) const{
	const VMachinePtr& vm = setup_call();
	rawsend(vm, primary_key, secondary_key);
	return vm->result_and_cleanup_call();
}

/// \brief 関数を呼び出す
AnyPtr Any::call() const{
	const VMachinePtr& vm = setup_call();
	rawcall(vm);
	return vm->result_and_cleanup_call();
}

const AnyPtr& Any::member(const IDPtr& primary_key, const AnyPtr& secondary_key, bool inherited_too, int_t& accessibility) const{
	accessibility = 0;

	if(type(*this)==TYPE_BASE){
		Base* p = pvalue(*this);
		const AnyPtr& ret = inherited_too ?
			environment_->member_cache_table_.cache(p, primary_key, secondary_key, accessibility) :
			p->rawmember(primary_key, secondary_key, false, accessibility, Temp());
		return ret;
	}
	return undefined;
}

void Any::def(const IDPtr& primary_key, const AnyPtr& value, const AnyPtr& secondary_key, int_t accessibility) const{
	if(type(*this)>=TYPE_BASE){
		rcpvalue(*this)->def(primary_key, value, secondary_key, accessibility); 
	}
}

void Any::rawsend(const VMachinePtr& vm, const IDPtr& primary_key, const AnyPtr& secondary_key, bool inherited_too, bool q) const{
	const ClassPtr& cls = get_class();
	Any mem = cls->member(primary_key, secondary_key, inherited_too);
	vm->set_arg_this(ap(*this));

	if(is_undefined(mem)){
		if(!q){
			set_unsupported_error(cls, primary_key, secondary_key, vm);
		}
		return;
	}

	ap(mem)->rawcall(vm);
}

void Any::rawsend(const VMachinePtr& vm, const IDPtr& primary_key) const{
	rawsend(vm, primary_key, undefined, true, false);
}

void Any::rawsend(const VMachinePtr& vm, const IDPtr& primary_key, const AnyPtr& secondary_key) const{
	rawsend(vm, primary_key, secondary_key, true, false);
}

void Any::rawsend(const VMachinePtr& vm, const IDPtr& primary_key, const AnyPtr& secondary_key, bool inherited_too) const{
	rawsend(vm, primary_key, secondary_key, inherited_too, false);
}

void Any::rawcall(const VMachinePtr& vm) const{
	switch(type(*this)){
		XTAL_DEFAULT{
			rawsend(vm, id_op_list()[IDOp::id_op_call]);
		}

		XTAL_CASE(TYPE_BASE){ 
			pvalue(*this)->rawcall(vm);
		}

		XTAL_CASE(TYPE_IVAR_GETTER){ unchecked_ptr_cast<InstanceVariableGetter>(ap(*this))->on_rawcall(vm); }
		XTAL_CASE(TYPE_IVAR_SETTER){ unchecked_ptr_cast<InstanceVariableSetter>(ap(*this))->on_rawcall(vm); }
		XTAL_CASE(TYPE_STATELESS_NATIVE_METHOD){ unchecked_ptr_cast<StatelessNativeMethod>(ap(*this))->on_rawcall(vm); }
		XTAL_CASE(TYPE_NATIVE_METHOD){ unchecked_ptr_cast<NativeMethod>(ap(*this))->on_rawcall(vm); }
		XTAL_CASE(TYPE_NATIVE_FUN){ unchecked_ptr_cast<NativeFun>(ap(*this))->on_rawcall(vm); }
		XTAL_CASE(TYPE_METHOD){ unchecked_ptr_cast<Method>(ap(*this))->on_rawcall(vm); }
		XTAL_CASE(TYPE_FUN){ unchecked_ptr_cast<Fun>(ap(*this))->on_rawcall(vm); }
		XTAL_CASE(TYPE_LAMBDA){ unchecked_ptr_cast<Lambda>(ap(*this))->on_rawcall(vm); }
		XTAL_CASE(TYPE_FIBER){ unchecked_ptr_cast<Fiber>(ap(*this))->on_rawcall(vm); }
	}

	if(vm->processed()==0){
		if(vm->except()){ 
			return;
		}

		set_unsupported_error(ap(*this)->get_class(), id_op_list()[IDOp::id_op_call], undefined, vm);
		return;
	}
}


void Any::init(RefCountingBase* p){ 
	value_ = p->value_;
	register_gc(p);
}

int_t Any::to_i() const{
	switch(type(*this)){
		XTAL_DEFAULT{ return cast<int_t>((*this).send(Xid(to_i))); }
		XTAL_CASE(TYPE_NULL){ return 0; }
		XTAL_CASE(TYPE_INT){ return ivalue(*this); }
		XTAL_CASE(TYPE_FLOAT){ return (int_t)fvalue(*this); }
	}
//	return 0;
}

float_t Any::to_f() const{
	switch(type(*this)){
		XTAL_DEFAULT{ return cast<float_t>((*this).send(Xid(to_f))); }
		XTAL_CASE(TYPE_NULL){ return 0; }
		XTAL_CASE(TYPE_INT){ return (float_t)ivalue(*this); }
		XTAL_CASE(TYPE_FLOAT){ return fvalue(*this); }
	}
//	return 0;
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

		XTAL_CASE(TYPE_INT){ return Xf1("%d", 0, ap(*this)); }
		XTAL_CASE(TYPE_FLOAT){ return Xf1("%g", 0, ap(*this)); }
		XTAL_CASE(TYPE_FALSE){ return Xid(false); }
		XTAL_CASE(TYPE_TRUE){ return Xid(true); }

		XTAL_CASE4(TYPE_SMALL_STRING, TYPE_LITERAL_STRING, TYPE_INTERNED_STRING, TYPE_STRING){
			return unchecked_ptr_cast<String>(ap(*this));
		}
	}
//	return empty_string;
}

ArrayPtr Any::to_a() const{
	if(const ArrayPtr& ret = ptr_cast<Array>(ap(*this))){
		return ret;	
	}
	
	return ptr_cast<Array>((*this).send(Xid(op_to_array)));
}

MapPtr Any::to_m() const{
	if(const MapPtr& ret = ptr_cast<Map>(ap(*this))){
		return ret;	
	}
	
	return ptr_cast<Map>((*this).send(Xid(op_to_map)));
}

const ClassPtr& Any::object_parent() const{
	switch(type(*this)){
		XTAL_DEFAULT{ return nul<Class>(); }

		XTAL_CASE(TYPE_BASE){ 
			return pvalue(*this)->object_parent(); 
		}

		XTAL_CASE(TYPE_NATIVE_METHOD){ return unchecked_ptr_cast<NativeMethod>(ap(*this))->object_parent();  }
		XTAL_CASE(TYPE_NATIVE_FUN){ return unchecked_ptr_cast<NativeFun>(ap(*this))->object_parent();  }
		XTAL_CASE(TYPE_IVAR_GETTER){ return unchecked_ptr_cast<InstanceVariableGetter>(ap(*this))->object_parent(); }
		XTAL_CASE(TYPE_IVAR_SETTER){ return unchecked_ptr_cast<InstanceVariableSetter>(ap(*this))->object_parent(); }
	}
//	return nul<Class>();	
}

void Any::set_object_parent(const ClassPtr& parent) const{
	if(type(*this)>=TYPE_BASE){
		rcpvalue(*this)->set_object_parent(parent);
	}
}

ArrayPtr Any::object_name_list() const{
	ArrayPtr ret;

	if(const ClassPtr& parent = object_parent()){
		ret = parent->object_name_list();
		if(AnyPtr name = parent->child_object_name(ap(*this))){
			ret->push_back(name);
		}
	}
	else{
		ret = XNew<Array>();
		if(const ClassPtr& cls = ptr_cast<Class>(ap(*this))){
			ret->push_back(mv(cls->object_name(), null));
		}
	}
	
	return ret;
}

StringPtr Any::defined_place_name(const CodePtr& code, int_t pc, int_t name_number) const{
	if(code){
		if(name_number!=0){
			return code->identifier(name_number);
		}

		return Xf2("%s(%d)", 0, code->source_file_name(), 1, code->compliant_lineno(code->data()+pc));
	}
	else{
		return XTAL_STRING("?");
	}
}

StringPtr Any::ask_object_name_to_parent() const{
	if(const ClassPtr& parent = object_parent()){
		if(ValuesPtr myname = parent->child_object_name(ap(*this))){
			if(is_undefined(myname->at(1))){
				return Xf2("%s::%s", 0, parent->object_name(), 1, myname->at(0));
			}
			else{
				return Xf3("%s::%s#%s", 0, parent->object_name(), 1, myname->at(0), 2, myname->at(1));
			}
		}
	}

	return empty_string;
}

StringPtr Any::object_name() const{
	StringPtr ret;

	switch(type(*this)){
		XTAL_DEFAULT;
		XTAL_CASE(TYPE_NULL){ ret = Xid(null); }
		XTAL_CASE(TYPE_UNDEFINED){ ret = Xid(undefined); }
		XTAL_CASE(TYPE_INT){ ret = Xf1("%d", 0, ap(*this)); }
		XTAL_CASE(TYPE_FLOAT){ ret = Xf1("%g", 0, ap(*this)); }
		XTAL_CASE(TYPE_FALSE){ ret = Xid(true); }
		XTAL_CASE(TYPE_TRUE){ ret = Xid(false); }

		XTAL_CASE4(TYPE_SMALL_STRING, TYPE_LITERAL_STRING, TYPE_INTERNED_STRING, TYPE_STRING){ 
			ret = unchecked_ptr_cast<String>(ap(*this)); 
		}
	}

	do{
		if(ret){ break; }

		ret = ask_object_name_to_parent();
		if(ret->data_size()!=0){ break; }

		if(const ClassPtr& cls = ptr_cast<Class>(ap(*this))){
			ret = cls->object_temporary_name();
			if(ret && ret->data_size()!=0){ break; }
			ret = defined_place_name(cls->code(), cls->info()->pc, cls->info()->name_number);
			break;
		}

		if(const MethodPtr& mtd = ptr_cast<Method>(ap(*this))){
			ret = mtd->object_temporary_name();
			if(ret && ret->data_size()!=0){ break; }
			ret = defined_place_name(mtd->code(), mtd->info()->pc, mtd->info()->name_number);
			break;
		}

		ret = Xf1("(instance of %s)", 0, get_class()->object_name());

	}while(0);

	return ret;
}

bool Any::is_inherited(const AnyPtr& klass) const{
	if(raweq(*this, klass)) return true;

	if(const ClassPtr& cls = ptr_cast<Class>(ap(*this))){
		return cls->is_inherited(klass);
	}
	else{
		return false;
	}
}

bool Any::op_eq( const AnyPtr& v) const{
  return raweq(*this, *v); 
} 

const AnyPtr& Any::p() const{
	ap(*this)->send(Xid(p));
	return ap(*this);
}

MapPtr Any::s_save() const{
	MapPtr ret = xnew<Map>();
	const ClassPtr& klass = get_class();

	ArrayPtr ary = klass->ancestors()->to_a();
	ary->push_back(klass);

	Xfor(it, ary){
		if(const AnyPtr& member = it->member(Xid(serial_save), undefined, false)){
			const VMachinePtr& vm = setup_call(1);
			vm->set_arg_this(ap(*this));
			member->rawcall(vm);
			ret->set_at(it, vm->result_and_cleanup_call());
		}
		else{
			ret->set_at(it, save_instance_variables(unchecked_ptr_cast<Class>(it)));
		}
	}

	return ret;
}

void Any::s_load(const MapPtr& ret) const{
	const ClassPtr& klass = get_class();

	ArrayPtr ary = klass->ancestors()->to_a();
	ary->push_back(klass);

	Xfor(it, ary){
		if(rawne(it, cpp_class<Any>())){
			if(const AnyPtr& member = it->member(Xid(serial_load), undefined, false)){
				const VMachinePtr& vm = setup_call(1);
				vm->push_arg(ret->at(it));
				vm->set_arg_this(ap(*this));
				member->rawcall(vm);
				vm->cleanup_call();

				continue;
			}
			load_instance_variables(unchecked_ptr_cast<Class>(it), ret->at(it));
		}		
	}
}

AnyPtr Any::save_instance_variables(const ClassPtr& p) const{
	if(type(*this)!=TYPE_BASE){
		return undefined;
	}

	if(InstanceVariables* iv = pvalue(*this)->instance_variables()){
		if(const CodePtr& code = p->code()){
			ClassInfo* info = p->info();
			if(info->instance_variable_size!=0){	
				MapPtr insts = xnew<Map>();
				for(uint_t i=0; i<info->instance_variable_size; ++i){
					insts->set_at(code->identifier(info->instance_variable_identifier_offset+i), iv->variable(i, info));
				}
				return insts;
			}
		}
	}
	return undefined;
}

void Any::load_instance_variables(const ClassPtr& p, const AnyPtr& v) const{
	if(type(*this)!=TYPE_BASE){
		return;
	}

	if(InstanceVariables* iv = pvalue(*this)->instance_variables()){
		if(const MapPtr& insts = ptr_cast<Map>(v)){
			if(const CodePtr& code = p->code()){
				ClassInfo* info = p->info();
				if(info->instance_variable_size!=0){	
					for(uint_t i=0; i<info->instance_variable_size; ++i){
						iv->set_variable(i, info, insts->at(code->identifier(info->instance_variable_identifier_offset+i)));
					}
				}
			}
		}
	}
}

}
