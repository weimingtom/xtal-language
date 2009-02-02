#include "xtal.h"
#include "xtal_macro.h"

namespace xtal{

SmartPtr<Any>& SmartPtr<Any>::operator =(const SmartPtr<Any>& p){
	return ap_copy(*this, p);
}

void SmartPtr<Any>::set_p_with_class(Base* p, const ClassPtr& c){
	Any::set_p(p);
	p->set_class(c);
	register_gc(p);
}

SmartPtr<Any>::SmartPtr(Base* p, const ClassPtr& c, with_class_t)
	:Any(p){
	p->set_class(c);
	register_gc(p);
}

SmartPtr<Any>::SmartPtr(Singleton* p, const ClassPtr& c, with_class_t)
	:Any(p){
	register_gc(p);
}

SmartPtr<Any>::SmartPtr(CppSingleton* p, const ClassPtr& c, with_class_t)
	:Any(p){
	register_gc(p);
}

}