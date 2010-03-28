#include "xtal.h"
#include "xtal_macro.h"

namespace xtal{

SmartPtr<Any>::SmartPtr(const char_t* str){
	value_.init_primitive(TYPE_NULL);
	*this = XNew<String>(str);
}

SmartPtr<Any>::SmartPtr(const LongLivedString& str){
	value_.init_primitive(TYPE_NULL);
	*this = XNew<String>(str);
}

void SmartPtr<Any>::set_unknown_pointer(const Base* p){
	copy_any(*this, *p);
	inc_ref_count_force(*this);
}

void SmartPtr<Any>::set_unknown_pointer(const RefCountingBase* p){
	copy_any(*this, *p);
	inc_ref_count_force(*this);
}

void SmartPtr<Any>::set_unknown_pointer(const Any* p){
	copy_any(*this, *p);
	inc_ref_count_force(*this);
}

void SmartPtr<Any>::set_unknown_pointer(const AnyPtr* p){
	copy_any(*this, *p);
	inc_ref_count_force(*this);
}

void SmartPtr<Any>::init(const Any& a){
	copy_any(*this, a);
	inc_ref_count_force(*this);
}

void SmartPtr<Any>::unref_init(const Any& a){
	inc_ref_count_force(a);
	dec_ref_count_force(*this);
	copy_any(*this, a);
}

void SmartPtr<Any>::ref(){
	inc_ref_count_force(*this);
}

void SmartPtr<Any>::unref(){
	dec_ref_count_force(*this);
}

SmartPtr<Any>::SmartPtr(const SmartPtr<Any>& p)
	:Any(p){
	inc_ref_count_force(*this);
}

SmartPtr<Any>::~SmartPtr(){
#ifdef XTAL_CHECK_REF_COUNT
	if(!environment()){
		XTAL_ASSERT(type(*this)<TYPE_BASE);
	}
#endif

	dec_ref_count_force(*this);
}	

SmartPtr<Any>::SmartPtr(const NullPtr&){
	value_.init_primitive(TYPE_NULL);
}

SmartPtr<Any>::SmartPtr(const UndefinedPtr&){
	value_.init_primitive(TYPE_UNDEFINED);
}

SmartPtr<Any>::SmartPtr(const IDPtr& p){
	Any::operator=(p);
	XTAL_ASSERT(type(*this)<TYPE_BASE);
}

SmartPtr<Any>& SmartPtr<Any>::operator =(const IDPtr& p){
	dec_ref_count_force(*this);
	Any::operator=(p);
	XTAL_ASSERT(type(*this)<TYPE_BASE);
	return *this;
}

void visit_members(Visitor& m, RefCountingBase* p){
	XTAL_ASSERT((int)p->ref_count() >= -m.value());
	p->add_ref_count(m.value());
}

void visit_members(Visitor& m, const Any& p){
	if(type(p)>=TYPE_BASE){
		XTAL_ASSERT((int)rcpvalue(p)->ref_count() >= -m.value());
		rcpvalue(p)->add_ref_count(m.value());
	}
}

//////////////////////////////

void CppClassSymbolData::init_bind(int n, const char_t* xtname, XTAL_bind_t b){
	if(n==0){
		flags |= FLAG_NAME | FLAG_BIND0;
		prebind = b;
		name = xtname;
	}
	else if(n==1){
		flags |= FLAG_NAME | FLAG_BIND1;
		bind[0] = b;
		name = xtname;
	}
	else{
		flags |= FLAG_BIND2;
		bind[1] = b;
	}
}

}
