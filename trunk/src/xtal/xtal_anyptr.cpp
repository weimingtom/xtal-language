#include "xtal.h"
#include "xtal_macro.h"

namespace xtal{

SmartPtr<Any>::SmartPtr(const char_t* str){
	*this = xnew<String>(str);
}

SmartPtr<Any>::SmartPtr(const char8_t* str){
	*this = xnew<String>(str);
}

SmartPtr<Any>::SmartPtr(const StringLiteral& str){
	*this = xnew<String>(str);
}

SmartPtr<Any>& SmartPtr<Any>::operator =(const SmartPtr<Any>& p){
	dec_ref_count_force(*this);
	*(Any*)this = p;
	inc_ref_count_force(*this);
	return *this;
}

SmartPtr<Any>::SmartPtr(RefCountingBase* p, int_t type, special_ctor_t)
	:Any(noinit_t()){
	set_pvalue(*this, type, p);
	register_gc(p);
}

SmartPtr<Any>::SmartPtr(Base* p, const ClassPtr& c, special_ctor_t)
	:Any(p){
	p->set_class(c);
	register_gc(p);
}

SmartPtr<Any>::SmartPtr(Singleton* p, const ClassPtr& c, special_ctor_t)
	:Any(p){
	register_gc(p);
}

SmartPtr<Any>::SmartPtr(CppSingleton* p, const ClassPtr& c, special_ctor_t)
	:Any(p){
	register_gc(p);
}

SmartPtr<Any>::SmartPtr(const SmartPtr<Any>& p)
	:Any(p){
	inc_ref_count_force(*this);
}

SmartPtr<Any>::~SmartPtr(){
	dec_ref_count_force(*this);
}	
		
}
