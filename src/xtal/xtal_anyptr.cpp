#include "xtal.h"
#include "xtal_macro.h"

namespace xtal{

SmartPtr<Any>::SmartPtr(const char_t* str){
	value_.init_primitive(TYPE_NULL);
	*this = XNew<String>(str);
}

SmartPtr<Any>::SmartPtr(const char8_t* str){
	value_.init_primitive(TYPE_NULL);
	*this = XNew<String>(str);
}

SmartPtr<Any>::SmartPtr(const StringLiteral& str){
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
	dec_ref_count_force(*this);
	copy_any(*this, a);
	inc_ref_count_force(*this);
}

void SmartPtr<Any>::ref(){
	inc_ref_count_force(*this);
}

void SmartPtr<Any>::unref(){
	dec_ref_count_force(*this);
}

SmartPtr<Any>& SmartPtr<Any>::operator =(const SmartPtr<Any>& p){
	dec_ref_count_force(*this);
	copy_any(*this, p);
	inc_ref_count_force(*this);
	return *this;
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

SmartPtr<Any>& SmartPtr<Any>::operator =(const NullPtr& p){
	dec_ref_count_force(*this);
	value_.init_primitive(TYPE_NULL);
	return *this;
}

SmartPtr<Any>::SmartPtr(const UndefinedPtr&){
	value_.init_primitive(TYPE_UNDEFINED);
}

SmartPtr<Any>& SmartPtr<Any>::operator =(const UndefinedPtr& p){
	dec_ref_count_force(*this);
	value_.init_primitive(TYPE_UNDEFINED);
	return *this;
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

void visit_members(Visitor& m, const Any& p){
	if(type(p)>=TYPE_BASE){
		XTAL_ASSERT((int)rcpvalue(p)->ref_count() >= -m.value());
		rcpvalue(p)->add_ref_count(m.value());
	}
}

//////////////////////////////

void CppClassSymbolData::init_bind0(XTAL_bind_t b, const char_t* xtname){
	flags |= FLAG_NAME | FLAG_BIND0;
	prebind = b;
	name = xtname;
}

void CppClassSymbolData::init_bind1(XTAL_bind_t b, const char_t* xtname){
	flags |= FLAG_NAME | FLAG_BIND1;
	bind[0] = b;
	name = xtname;
}

void CppClassSymbolData::init_bind2(XTAL_bind_t b, const char_t* xtname){
	flags |= FLAG_BIND2;
	bind[1] = b;
}

}
