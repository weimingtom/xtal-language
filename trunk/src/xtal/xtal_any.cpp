#include "xtal.h"
#include "xtal_macro.h"

namespace xtal{

/// \brief primary_key���\�b�h���Ăяo��
AnyPtr Any::send(const IDPtr& primary_key) const{
	const VMachinePtr& vm = vmachine();
	vm->setup_call();
	rawsend(vm, primary_key);
	return vm->result_and_cleanup_call();
}

/// \brief primary_key#secondary_key���\�b�h���Ăяo��
AnyPtr Any::send2(const IDPtr& primary_key, const AnyPtr& secondary_key) const{
	XTAL_ASSERT(!raweq(secondary_key, null)); // �Z�J���_���L�[�������Ƃ���null�łȂ�undefined���w�肷��悤�ɂȂ����̂ŁA���o�passert
	const VMachinePtr& vm = vmachine();
	vm->setup_call();
	rawsend(vm, primary_key, secondary_key);
	return vm->result_and_cleanup_call();
}

/// \brief �֐����Ăяo��
AnyPtr Any::call() const{
	const VMachinePtr& vm = vmachine();
	vm->setup_call();
	rawcall(vm);
	return vm->result_and_cleanup_call();
}

const AnyPtr& Any::member(const IDPtr& primary_key, const AnyPtr& secondary_key, const AnyPtr& self, bool inherited_too) const{
	XTAL_ASSERT(!raweq(secondary_key, null)); // �Z�J���_���L�[�������Ƃ���null�łȂ�undefined���w�肷��悤�ɂȂ����̂ŁA���o�passert
	int_t accessibility = 0;
	const AnyPtr& ret = inherited_too ?
		cache_member(ap(*this), primary_key, secondary_key, accessibility) :
		type(*this)==TYPE_BASE ? pvalue(*this)->do_member(primary_key, secondary_key, false, accessibility, Temp()) : undefined;

	if(accessibility==0){
		return ret;
	}

	if(accessibility & KIND_PRIVATE){
		if(rawne(self->get_class(), *this)){
			XTAL_SET_EXCEPT(cpp_class<AccessibilityError>()->call(Xt("Xtal Runtime Error 1017")->call(
				Named(Xid(object), object_name()), 
				Named(Xid(name), primary_key), 
				Named(Xid(secondary_key), secondary_key), 
				Named(Xid(accessibility), Xid(private))))
			);
			return undefined;
		}
	}
	else if(accessibility & KIND_PROTECTED){
		if(!self->is(ap(*this))){
			XTAL_SET_EXCEPT(cpp_class<AccessibilityError>()->call(Xt("Xtal Runtime Error 1017")->call(
				Named(Xid(object), object_name()), 
				Named(Xid(primary_key), primary_key), 
				Named(Xid(secondary_key), secondary_key), 
				Named(Xid(accessibility), Xid(protected))))
			);
			return undefined;
		}
	}

	return ret;
}

void Any::def(const IDPtr& primary_key, const AnyPtr& value, const AnyPtr& secondary_key, int_t accessibility) const{
	XTAL_ASSERT(!raweq(secondary_key, null)); // �Z�J���_���L�[�������Ƃ���null�łȂ�undefined���w�肷��悤�ɂȂ����̂ŁA���o�passert
	XTAL_ASSERT(!raweq(secondary_key, null)); // �Z�J���_���L�[�������Ƃ���null�łȂ�undefined���w�肷��悤�ɂȂ����̂ŁA���o�passert
	switch(type(*this)){
		XTAL_DEFAULT;
		XTAL_CASE(TYPE_BASE){
			pvalue(*this)->def(primary_key, value, secondary_key, accessibility);
		}
	}
}

void Any::rawsend(const VMachinePtr& vm, const IDPtr& primary_key, const AnyPtr& secondary_key, const AnyPtr& self, bool inherited_too) const{
	XTAL_ASSERT(!raweq(secondary_key, null)); // �Z�J���_���L�[�������Ƃ���null�łȂ�undefined���w�肷��悤�ɂȂ����̂ŁA���o�passert
	const ClassPtr& cls = get_class();
	const AnyPtr& ret = ap(cls)->member(primary_key, secondary_key, self, inherited_too);
	if(rawne(ret, undefined)){
		vm->set_arg_this(ap(*this));
		ret->rawcall(vm);
	}
	else{
		XTAL_CHECK_EXCEPT(e){ return; }
		const AnyPtr& ret = ap(cls)->member(Xid(send_missing), undefined, self, inherited_too);
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
	const AnyPtr& ret = ap(cls)->member(primary_key, undefined, null, true);

	switch(type(ret)){
		XTAL_DEFAULT{}

		XTAL_CASE(TYPE_BASE){ 
			vm->set_arg_this(ap(*this));
			pvalue(ret)->rawcall(vm); 
			return;
		}

		XTAL_CASE(TYPE_NATIVE_FUN){ 
			vm->set_arg_this(ap(*this));
			unchecked_ptr_cast<NativeFun>(ret)->rawcall(vm); 
			return;
		}

		XTAL_CASE(TYPE_NATIVE_FUN_BINDED_THIS){ 
			vm->set_arg_this(ap(*this));
			unchecked_ptr_cast<NativeFunBindedThis>(ret)->rawcall(vm); 
			return;
		}
	}

	{
		XTAL_CHECK_EXCEPT(e){ return; }
		const AnyPtr& ret = ap(cls)->member(Xid(send_missing), undefined, null, true);
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
			vm->set_unsuported_error_info(*this, primary_key, undefined);
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
	return ptr_cast<Array>((*this).send(Xid(op_to_array)));
}

MapPtr Any::to_m() const{
	return ptr_cast<Map>((*this).send(Xid(op_to_map)));
}

const ClassPtr& Any::object_parent() const{
	switch(type(*this)){
		XTAL_DEFAULT{ return unchecked_ptr_cast<Class>(null); }
		XTAL_CASE(TYPE_BASE){ return pvalue(*this)->object_parent(); }
		XTAL_CASE(TYPE_NATIVE_FUN){ return unchecked_ptr_cast<NativeFun>(ap(*this))->object_parent();  }
		XTAL_CASE(TYPE_NATIVE_FUN_BINDED_THIS){ return unchecked_ptr_cast<NativeFunBindedThis>(ap(*this))->object_parent();  }
	}
	//return unchecked_ptr_cast<Class>(null);	
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

	// �e������Ȃ�A�e�����O��m���Ă���
	if(const ClassPtr& parent = object_parent()){
		if(MultiValuePtr myname = parent->child_object_name(ap(*this))){
			if(raweq(myname->at(1), undefined)){
				return Xf("%s::%s")->call(parent->object_name(), myname->at(0))->to_s();
			}
			else{
				return Xf("%s::%s#%s")->call(parent->object_name(), myname->at(0), myname->at(1))->to_s();
			}
		}
	}

	// �N���X�̏ꍇ�A���g�����O��ێ����Ă邩��
	if(const ClassPtr& cls = ptr_cast<Class>(ap(*this))){
		if(StringPtr name = cls->object_name()){
			if(rawne(name, empty_string)){
				return name;
			}
		}
		
		if(cls->code()){
			// �ێ����Ă��Ȃ��Ȃ�A���̒�`�ʒu��\�����Ƃ���
			return Xf("(instance of %s) %s(%d)")->call(get_class()->object_name(), 
				cls->code()->source_file_name(), 
				cls->code()->compliant_lineno(cls->code()->data()+cls->info()->pc))->to_s();
		}
		else{
			return Xf("(instance of %s)")->call(get_class()->object_name())->to_s();
		}
	}

	// ���\�b�h�̏ꍇ�A���̒�`�ʒu��\�����Ƃ���
	if(const MethodPtr& mtd = ptr_cast<Method>(ap(*this))){
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
		XTAL_CASE(TYPE_NULL){ return cpp_class<Null>(); }
		XTAL_CASE(TYPE_UNDEFINED){ return cpp_class<Undefined>(); }
		XTAL_CASE(TYPE_BASE){ return pvalue(*this)->get_class(); }
		XTAL_CASE(TYPE_INT){ return cpp_class<Int>(); }
		XTAL_CASE(TYPE_FLOAT){ return cpp_class<Float>(); }
		XTAL_CASE(TYPE_FALSE){ return cpp_class<Bool>(); }
		XTAL_CASE(TYPE_TRUE){ return cpp_class<Bool>(); }
		XTAL_CASE(TYPE_SMALL_STRING){ return cpp_class<String>(); }
		XTAL_CASE(TYPE_STRING){ return cpp_class<String>(); }
		XTAL_CASE(TYPE_ARRAY){ return cpp_class<Array>(); }
		XTAL_CASE(TYPE_MULTI_VALUE){ return cpp_class<MultiValue>(); }
		XTAL_CASE(TYPE_TREE_NODE){ return cpp_class<xpeg::TreeNode>(); }
		XTAL_CASE(TYPE_NATIVE_FUN){ return cpp_class<NativeFun>(); }
		XTAL_CASE(TYPE_NATIVE_FUN_BINDED_THIS){ return cpp_class<NativeFunBindedThis>(); }
	}
	return cpp_class<Any>();
}

bool Any::is(const AnyPtr& klass) const{
	const ClassPtr& my_class = get_class();
	if(raweq(my_class, klass)) return true;
	return cache_is(my_class, klass);
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
		if(const AnyPtr& member = it->member(Xid(serial_save), undefined, null, false)){
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
		if(const AnyPtr& member = it->member(Xid(serial_load), undefined, null, false)){
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
				for(uint_t i=0; i<info->instance_variable_size; ++i){
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

void Any::destroy(){
	switch(type(*this)){
		XTAL_NODEFAULT;

		XTAL_CASE(TYPE_BASE){
			delete pvalue(*this);
		}

		XTAL_CASE(TYPE_STRING){ 
			uint_t size = StringData::calc_size(((StringData*)rcpvalue(*this))->data_size());
			unchecked_ptr_cast<StringData>(ap(*this))->~StringData(); 
			value_.uvalue = size; 
		}

		XTAL_CASE(TYPE_ARRAY){ 
			unchecked_ptr_cast<Array>(ap(*this))->~Array(); 
			value_.uvalue = sizeof(Array); 
		}

		XTAL_CASE(TYPE_MULTI_VALUE){ 
			unchecked_ptr_cast<MultiValue>(ap(*this))->~MultiValue(); 
			value_.uvalue = sizeof(MultiValue); 
		}

		XTAL_CASE(TYPE_TREE_NODE){ 
			using namespace xpeg;
			unchecked_ptr_cast<TreeNode>(ap(*this))->~TreeNode(); 
			value_.uvalue = sizeof(TreeNode); 
		}

		XTAL_CASE(TYPE_NATIVE_FUN){ 
			unchecked_ptr_cast<NativeFun>(ap(*this))->~NativeFun(); 
			value_.uvalue = sizeof(NativeFun); 
		}

		XTAL_CASE(TYPE_NATIVE_FUN_BINDED_THIS){ 
			unchecked_ptr_cast<NativeFunBindedThis>(ap(*this))->~NativeFunBindedThis(); 
			value_.uvalue = sizeof(NativeFunBindedThis); 
		}
	}
}

void Any::object_free(){
	if(type(*this)==TYPE_BASE){
		xfree(static_cast<Base*>(this), value_.uvalue);
	}
	else if(type(*this)>TYPE_BASE){
		xfree(static_cast<RefCountingBase*>(this), value_.uvalue);
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

}