#include "xtal.h"
#include "xtal_macro.h"
#include "xtal_stringspace.h"

namespace xtal{

CppClassSymbolData** classdata[] = {
	&CppClassSymbol<Null>::value,
	&CppClassSymbol<Undefined>::value,
	&CppClassSymbol<Bool>::value,
	&CppClassSymbol<Bool>::value,
	&CppClassSymbol<Int>::value,
	&CppClassSymbol<Float>::value,
	&CppClassSymbol<String>::value,
	&CppClassSymbol<Any>::value,
	&CppClassSymbol<String>::value,
	&CppClassSymbol<Array>::value,
	&CppClassSymbol<Values>::value,
	&CppClassSymbol<xpeg::TreeNode>::value,
	&CppClassSymbol<NativeMethod>::value,
	&CppClassSymbol<NativeFun>::value,
	&CppClassSymbol<InstanceVariableGetter>::value,
	&CppClassSymbol<InstanceVariableSetter>::value,
};


/// \brief primary_keyメソッドを呼び出す
AnyPtr Any::send(const IDPtr& primary_key) const{
	const VMachinePtr& vm = vmachine();
	vm->setup_call();
	rawsend(vm, primary_key);
	return vm->result_and_cleanup_call();
}

/// \brief primary_key#secondary_keyメソッドを呼び出す
AnyPtr Any::send2(const IDPtr& primary_key, const AnyPtr& secondary_key) const{
	XTAL_ASSERT(!raweq(secondary_key, null)); // セカンダリキーが無いときはnullでなくundefinedを指定するようになったので、検出用assert
	const VMachinePtr& vm = vmachine();
	vm->setup_call();
	rawsend(vm, primary_key, secondary_key);
	return vm->result_and_cleanup_call();
}

/// \brief 関数を呼び出す
AnyPtr Any::call() const{
	const VMachinePtr& vm = vmachine();
	vm->setup_call();
	rawcall(vm);
	return vm->result_and_cleanup_call();
}

//{REPEAT{{
/*
/// \brief primary_keyメソッドを呼び出す
AnyPtr Any::send(const IDPtr& primary_key, const Param& a0 #COMMA_REPEAT#const Param& a`i+1`#) const{
	const VMachinePtr& vm = vmachine();
	vm->setup_call(1, a0 #COMMA_REPEAT#a`i+1`#);
	rawsend(vm, primary_key);
	return vm->result_and_cleanup_call();
}

/// \brief primary_key#secondary_keyメソッドを呼び出す
AnyPtr Any::send2(const IDPtr& primary_key, const AnyPtr& secondary_key, const Param& a0 #COMMA_REPEAT#const Param& a`i+1`#) const{
	const VMachinePtr& vm = vmachine();
	vm->setup_call(1, a0 #COMMA_REPEAT#a`i+1`#);
	rawsend(vm, primary_key, secondary_key);
	return vm->result_and_cleanup_call();
}

/// \brief 関数を呼び出す
AnyPtr Any::call(const Param& a0 #COMMA_REPEAT#const Param& a`i+1`#) const{
	const VMachinePtr& vm = vmachine();
	vm->setup_call(1, a0 #COMMA_REPEAT#a`i+1`#);
	rawcall(vm);
	return vm->result_and_cleanup_call();
}
*/

/// \brief primary_keyメソッドを呼び出す
AnyPtr Any::send(const IDPtr& primary_key, const Param& a0 ) const{
	const VMachinePtr& vm = vmachine();
	vm->setup_call(1, a0 );
	rawsend(vm, primary_key);
	return vm->result_and_cleanup_call();
}

/// \brief primary_key#secondary_keyメソッドを呼び出す
AnyPtr Any::send2(const IDPtr& primary_key, const AnyPtr& secondary_key, const Param& a0 ) const{
	const VMachinePtr& vm = vmachine();
	vm->setup_call(1, a0 );
	rawsend(vm, primary_key, secondary_key);
	return vm->result_and_cleanup_call();
}

/// \brief 関数を呼び出す
AnyPtr Any::call(const Param& a0 ) const{
	const VMachinePtr& vm = vmachine();
	vm->setup_call(1, a0 );
	rawcall(vm);
	return vm->result_and_cleanup_call();
}

/// \brief primary_keyメソッドを呼び出す
AnyPtr Any::send(const IDPtr& primary_key, const Param& a0 , const Param& a1) const{
	const VMachinePtr& vm = vmachine();
	vm->setup_call(1, a0 , a1);
	rawsend(vm, primary_key);
	return vm->result_and_cleanup_call();
}

/// \brief primary_key#secondary_keyメソッドを呼び出す
AnyPtr Any::send2(const IDPtr& primary_key, const AnyPtr& secondary_key, const Param& a0 , const Param& a1) const{
	const VMachinePtr& vm = vmachine();
	vm->setup_call(1, a0 , a1);
	rawsend(vm, primary_key, secondary_key);
	return vm->result_and_cleanup_call();
}

/// \brief 関数を呼び出す
AnyPtr Any::call(const Param& a0 , const Param& a1) const{
	const VMachinePtr& vm = vmachine();
	vm->setup_call(1, a0 , a1);
	rawcall(vm);
	return vm->result_and_cleanup_call();
}

/// \brief primary_keyメソッドを呼び出す
AnyPtr Any::send(const IDPtr& primary_key, const Param& a0 , const Param& a1, const Param& a2) const{
	const VMachinePtr& vm = vmachine();
	vm->setup_call(1, a0 , a1, a2);
	rawsend(vm, primary_key);
	return vm->result_and_cleanup_call();
}

/// \brief primary_key#secondary_keyメソッドを呼び出す
AnyPtr Any::send2(const IDPtr& primary_key, const AnyPtr& secondary_key, const Param& a0 , const Param& a1, const Param& a2) const{
	const VMachinePtr& vm = vmachine();
	vm->setup_call(1, a0 , a1, a2);
	rawsend(vm, primary_key, secondary_key);
	return vm->result_and_cleanup_call();
}

/// \brief 関数を呼び出す
AnyPtr Any::call(const Param& a0 , const Param& a1, const Param& a2) const{
	const VMachinePtr& vm = vmachine();
	vm->setup_call(1, a0 , a1, a2);
	rawcall(vm);
	return vm->result_and_cleanup_call();
}

/// \brief primary_keyメソッドを呼び出す
AnyPtr Any::send(const IDPtr& primary_key, const Param& a0 , const Param& a1, const Param& a2, const Param& a3) const{
	const VMachinePtr& vm = vmachine();
	vm->setup_call(1, a0 , a1, a2, a3);
	rawsend(vm, primary_key);
	return vm->result_and_cleanup_call();
}

/// \brief primary_key#secondary_keyメソッドを呼び出す
AnyPtr Any::send2(const IDPtr& primary_key, const AnyPtr& secondary_key, const Param& a0 , const Param& a1, const Param& a2, const Param& a3) const{
	const VMachinePtr& vm = vmachine();
	vm->setup_call(1, a0 , a1, a2, a3);
	rawsend(vm, primary_key, secondary_key);
	return vm->result_and_cleanup_call();
}

/// \brief 関数を呼び出す
AnyPtr Any::call(const Param& a0 , const Param& a1, const Param& a2, const Param& a3) const{
	const VMachinePtr& vm = vmachine();
	vm->setup_call(1, a0 , a1, a2, a3);
	rawcall(vm);
	return vm->result_and_cleanup_call();
}

/// \brief primary_keyメソッドを呼び出す
AnyPtr Any::send(const IDPtr& primary_key, const Param& a0 , const Param& a1, const Param& a2, const Param& a3, const Param& a4) const{
	const VMachinePtr& vm = vmachine();
	vm->setup_call(1, a0 , a1, a2, a3, a4);
	rawsend(vm, primary_key);
	return vm->result_and_cleanup_call();
}

/// \brief primary_key#secondary_keyメソッドを呼び出す
AnyPtr Any::send2(const IDPtr& primary_key, const AnyPtr& secondary_key, const Param& a0 , const Param& a1, const Param& a2, const Param& a3, const Param& a4) const{
	const VMachinePtr& vm = vmachine();
	vm->setup_call(1, a0 , a1, a2, a3, a4);
	rawsend(vm, primary_key, secondary_key);
	return vm->result_and_cleanup_call();
}

/// \brief 関数を呼び出す
AnyPtr Any::call(const Param& a0 , const Param& a1, const Param& a2, const Param& a3, const Param& a4) const{
	const VMachinePtr& vm = vmachine();
	vm->setup_call(1, a0 , a1, a2, a3, a4);
	rawcall(vm);
	return vm->result_and_cleanup_call();
}

/// \brief primary_keyメソッドを呼び出す
AnyPtr Any::send(const IDPtr& primary_key, const Param& a0 , const Param& a1, const Param& a2, const Param& a3, const Param& a4, const Param& a5) const{
	const VMachinePtr& vm = vmachine();
	vm->setup_call(1, a0 , a1, a2, a3, a4, a5);
	rawsend(vm, primary_key);
	return vm->result_and_cleanup_call();
}

/// \brief primary_key#secondary_keyメソッドを呼び出す
AnyPtr Any::send2(const IDPtr& primary_key, const AnyPtr& secondary_key, const Param& a0 , const Param& a1, const Param& a2, const Param& a3, const Param& a4, const Param& a5) const{
	const VMachinePtr& vm = vmachine();
	vm->setup_call(1, a0 , a1, a2, a3, a4, a5);
	rawsend(vm, primary_key, secondary_key);
	return vm->result_and_cleanup_call();
}

/// \brief 関数を呼び出す
AnyPtr Any::call(const Param& a0 , const Param& a1, const Param& a2, const Param& a3, const Param& a4, const Param& a5) const{
	const VMachinePtr& vm = vmachine();
	vm->setup_call(1, a0 , a1, a2, a3, a4, a5);
	rawcall(vm);
	return vm->result_and_cleanup_call();
}

//}}REPEAT}

const AnyPtr& Any::member(const IDPtr& primary_key, const AnyPtr& secondary_key, bool inherited_too, int_t& accessibility) const{
	accessibility = 0;
	const AnyPtr& ret = inherited_too ?
		cache_member(ap(*this), primary_key, secondary_key, accessibility) :
		type(*this)==TYPE_BASE ? pvalue(*this)->rawmember(primary_key, secondary_key, false, accessibility, Temp()) : undefined;
	return ret;
}

void Any::def(const IDPtr& primary_key, const AnyPtr& value, const AnyPtr& secondary_key, int_t accessibility) const{
	switch(type(*this)){
		XTAL_DEFAULT;
		XTAL_CASE(TYPE_BASE){
			pvalue(*this)->def(primary_key, value, secondary_key, accessibility);
		}
	}
}

void Any::rawsend(const VMachinePtr& vm, const IDPtr& primary_key, const AnyPtr& secondary_key, bool inherited_too) const{
	const ClassPtr& cls = get_class();
	const AnyPtr& ret = ap(cls)->member(primary_key, secondary_key, inherited_too);
	vm->set_arg_this(ap(*this));

	switch(type(ret)){
		XTAL_DEFAULT{
			ret->rawsend(vm, id_op_list()[IDOp::id_op_call]);
		}

		XTAL_CASE(TYPE_UNDEFINED){
			const AnyPtr& ret = ap(cls)->member(Xid(send_missing), undefined, inherited_too);
			if(rawne(ret, undefined)){
				vm->set_arg_this(ap(*this));
				ArgumentsPtr args = vm->make_arguments();
				vm->recycle_call();
				vm->push_arg(primary_key);
				vm->push_arg(secondary_key);
				vm->push_arg(args);
				ret->rawcall(vm);
			}
		}

		XTAL_CASE(TYPE_BASE){ 
			pvalue(ret)->rawcall(vm); 
		}

		XTAL_CASE(TYPE_NATIVE_METHOD){ 
			unchecked_ptr_cast<NativeMethod>(ret)->rawcall(vm); 
		}

		XTAL_CASE(TYPE_NATIVE_FUN){ 
			unchecked_ptr_cast<NativeFun>(ret)->rawcall(vm); 
		}

		XTAL_CASE(TYPE_IVAR_GETTER){ 
			unchecked_ptr_cast<InstanceVariableGetter>(ret)->rawcall(vm); 
		}

		XTAL_CASE(TYPE_IVAR_SETTER){ 
			unchecked_ptr_cast<InstanceVariableSetter>(ret)->rawcall(vm); 
		}
	}

	if(!vm->processed()){
		vm->set_unsuported_error_info(ap(*this), primary_key, secondary_key);
	}
}

void Any::rawcall(const VMachinePtr& vm) const{
	switch(type(*this)){
		XTAL_DEFAULT{}
		XTAL_CASE(TYPE_BASE){ pvalue(*this)->rawcall(vm); }
		XTAL_CASE(TYPE_NATIVE_METHOD){ unchecked_ptr_cast<NativeMethod>(ap(*this))->rawcall(vm); }
		XTAL_CASE(TYPE_NATIVE_FUN){ unchecked_ptr_cast<NativeFun>(ap(*this))->rawcall(vm); }
		XTAL_CASE(TYPE_IVAR_GETTER){ 
			unchecked_ptr_cast<InstanceVariableGetter>(ap(*this))->rawcall(vm); 
		}
		XTAL_CASE(TYPE_IVAR_SETTER){ 
			unchecked_ptr_cast<InstanceVariableSetter>(ap(*this))->rawcall(vm);
		}
	}

	if(!vm->processed()){
		vm->set_unsuported_error_info(ap(*this), id_op_list()[IDOp::id_op_call], undefined);
	}
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

		XTAL_CASE(TYPE_INT){ return Xf("%d")->call(ap(*this))->to_s(); }
		XTAL_CASE(TYPE_FLOAT){ return Xf("%g")->call(ap(*this))->to_s(); }
		XTAL_CASE(TYPE_FALSE){ return Xid(true); }
		XTAL_CASE(TYPE_TRUE){ return Xid(false); }
		XTAL_CASE(TYPE_SMALL_STRING){ return unchecked_ptr_cast<String>(ap(*this)); }
		XTAL_CASE(TYPE_STRING){ return unchecked_ptr_cast<String>(ap(*this)); }
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
		XTAL_DEFAULT{ return unchecked_ptr_cast<Class>(null); }
		XTAL_CASE(TYPE_BASE){ return pvalue(*this)->object_parent(); }
		XTAL_CASE(TYPE_NATIVE_METHOD){ return unchecked_ptr_cast<NativeMethod>(ap(*this))->object_parent();  }
		XTAL_CASE(TYPE_NATIVE_FUN){ return unchecked_ptr_cast<NativeFun>(ap(*this))->object_parent();  }
		XTAL_CASE(TYPE_IVAR_GETTER){ return unchecked_ptr_cast<InstanceVariableGetter>(ap(*this))->object_parent(); }
		XTAL_CASE(TYPE_IVAR_SETTER){ return unchecked_ptr_cast<InstanceVariableSetter>(ap(*this))->object_parent(); }
	}
	//return unchecked_ptr_cast<Class>(null);	
}

void Any::set_object_parent(const ClassPtr& parent) const{
	if(type(*this)>=TYPE_BASE){
		rcpvalue(*this)->set_object_parent(parent);
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
	else if(const ClassPtr& cls = ptr_cast<Class>(ap(*this))){
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
		if(ValuesPtr myname = parent->child_object_name(ap(*this))){
			if(raweq(myname->at(1), undefined)){
				return Xf("%s::%s")->call(parent->object_name(), myname->at(0))->to_s();
			}
			else{
				return Xf("%s::%s#%s")->call(parent->object_name(), myname->at(0), myname->at(1))->to_s();
			}
		}
	}

	// クラスの場合、自身が名前を保持してるかも
	if(const ClassPtr& cls = ptr_cast<Class>(ap(*this))){
		if(StringPtr name = cls->object_name()){
			if(rawne(name, empty_string)){
				return name;
			}
		}
		
		if(cls->code()){
			if(cls->info()->name_number!=0){
				return cls->code()->identifier(cls->info()->name_number);
			}

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
	if(const MethodPtr& mtd = ptr_cast<Method>(ap(*this))){
		if(mtd->code()){
			if(mtd->info()->name_number!=0){
				return mtd->code()->identifier(mtd->info()->name_number);
			}

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

bool Any::is_inherited(const AnyPtr& klass) const{
	if(raweq(*this, klass)) return true;

	if(const ClassPtr& cls = ptr_cast<Class>(ap(*this))){
		return cls->is_inherited(klass);
	}
	else{
		return false;
	}
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
		if(const AnyPtr& member = it->member(Xid(serial_save), undefined, false)){
			const VMachinePtr& vm = vmachine();
			vm->setup_call(1);
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

void Any::s_load(const AnyPtr& v) const{
	MapPtr ret = ptr_cast<Map>(v);
	ClassPtr klass = get_class();

	ArrayPtr ary = klass->ancestors()->to_a();
	ary->push_back(klass);

	Xfor(it, ary){
		if(const AnyPtr& member = it->member(Xid(serial_load), undefined, false)){
			const VMachinePtr& vm = vmachine();
			vm->setup_call(1, ret->at(it));
			vm->set_arg_this(ap(*this));
			member->rawcall(vm);
			vm->cleanup_call();
		}
		else{
			load_instance_variables(unchecked_ptr_cast<Class>(it), ret->at(it));
		}
	}
}

AnyPtr Any::save_instance_variables(const ClassPtr& p) const{
	if(type(*this)!=TYPE_BASE){
		return null;
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
	return null;
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
						StringPtr str = code->identifier(info->instance_variable_identifier_offset+i);
						iv->set_variable(i, info, insts->at(code->identifier(info->instance_variable_identifier_offset+i)));
					}
				}
			}
		}
	}
}

void visit_members(Visitor& m, const AnyPtr& p){
	if(type(p)>=TYPE_BASE){
		XTAL_ASSERT((int)rcpvalue(p)->ref_count() >= -m.value());
		rcpvalue(p)->add_ref_count(m.value());
	}
}

}
