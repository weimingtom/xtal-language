#include "xtal.h"
#include "xtal_macro.h"

namespace xtal{

/*
void xmemmove(int_t* ss1, const int_t* ss2, size_t n){
	if(n != 0){
		if(ss1 < ss2){
			const int_t *t = ss2 + n;
			do{
				*ss1++ = *ss2++;
			}while(ss2 != t);
		}
		else if(ss1 > ss2){
			const int_t *t = ss2;
			ss1 += n;
			ss2 += n;
			do{
				*--ss1 = *--ss2;
			}while(ss2 != t);
		}
	}
}

void xmemcpy(int_t* ss1, const int_t* ss2, size_t n){
	if(n != 0){
		const int_t* t = ss2 + n;
		do{
			*ss1++ = *ss2++;
		}while(ss2 != t);
	}
}

void xmemset(int_t* s, int_t c, size_t n){
	if(n != 0){
		int_t* addr = s;
		int_t* end = addr + n;
		for(; addr < end; ++addr){
			*addr = c;
		}
	}
}
*/

SmartPtr<Any>::SmartPtr(const char_t* str){
	*this = xnew<String>(str);
}

SmartPtr<Any>::SmartPtr(const char8_t* str){
	*this = xnew<String>(str);
}

SmartPtr<Any>::SmartPtr(const StringLiteral& str){
	*this = xnew<String>(str);
}

SmartPtr<Any>::SmartPtr(const XNewBase<INHERITED_BASE>& m)
	:Any(noinit_t()){
	init_smartptr(m.pvalue, cpp_class(m.klass));
}

SmartPtr<Any>::SmartPtr(const XNewBase<INHERITED_RCBASE>& m)
	:Any(noinit_t()){
	init_smartptr(m.pvalue, m.klass);
}

SmartPtr<Any>::SmartPtr(const XNewBase<INHERITED_ANY>& m)
	:Any(noinit_t()){
	init_smartptr((Any&)m.value);
}

SmartPtr<Any>::SmartPtr(const XNewBase<INHERITED_OTHER>& m)
	:Any(noinit_t()){
	init_smartptr(m.value, cpp_class(m.klass));
}

SmartPtr<Any>& SmartPtr<Any>::operator =(const XNewBase<INHERITED_BASE>& m){
	dec_ref_count_force(*this);
	init_smartptr(m.pvalue, cpp_class(m.klass));
	return *this;
}

SmartPtr<Any>& SmartPtr<Any>::operator =(const XNewBase<INHERITED_RCBASE>& m){
	dec_ref_count_force(*this);
	init_smartptr(m.pvalue, m.klass);
	return *this;
}

SmartPtr<Any>& SmartPtr<Any>::operator =(const XNewBase<INHERITED_ANY>& m){
	dec_ref_count_force(*this);
	init_smartptr((Any&)m.value);
	return *this;
}

SmartPtr<Any>& SmartPtr<Any>::operator =(const XNewBase<INHERITED_OTHER>& m){
	dec_ref_count_force(*this);
	init_smartptr(m.value, cpp_class(m.klass));
	return *this;
}

void SmartPtr<Any>::set_unknown_pointer(const Base* p, const Base*){
	value_.init(p);
	inc_ref_count_force(*this);
}

SmartPtr<Any>& SmartPtr<Any>::operator =(const SmartPtr<Any>& p){
	dec_ref_count_force(*this);
	copy_any(*this, p);
	inc_ref_count_force(*this);
	return *this;
}

void SmartPtr<Any>::init_smartptr(RefCountingBase* p, int_t type){
	value_.init(type, p);
	register_gc(p);
}

void SmartPtr<Any>::init_smartptr(Base* p, const ClassPtr& c){
	value_.init(p);
	p->set_class(c);
	register_gc(p);
}

SmartPtr<Any>::SmartPtr(const SmartPtr<Any>& p)
	:Any(p){
	inc_ref_count_force(*this);
}

SmartPtr<Any>::~SmartPtr(){
#if 0
	if(!environment()){
		XTAL_ASSERT(type(*this)<TYPE_BASE);
	}
#endif

	dec_ref_count_force(*this);
}	

void SmartPtr<Any>::assign_direct(const SmartPtr<Any>& a){
	copy_any(*this, a);
	inc_ref_count_force(*this);
}

void visit_members(Visitor& m, const Any& p){
	if(type(p)>=TYPE_BASE){
		XTAL_ASSERT((int)rcpvalue(p)->ref_count() >= -m.value());
		rcpvalue(p)->add_ref_count(m.value());
	}
}

//////////////////////////////

CppClassSymbolData::CppClassSymbolData(){
	static unsigned int counter = 0;
	static CppClassSymbolData* prev_data = 0;
	value = counter++;
	
	prev = prev_data;
	prebind = 0;
	for(int_t i=0; i<BIND; ++i){
		bind[i] = 0;
	}
	prev_data = this;
}

void BindBase::XTAL_set(BindBase*& dest, StringLiteral& name, const StringLiteral& given){
	dest = this;
	name = given;
}

CppVarSymbolData::CppVarSymbolData(bind_var_fun_t fun){
	static unsigned int counter = 1;
	static CppVarSymbolData* prev_data = 0;
	value = counter++;
	prev = prev_data;
	maker = fun;
	prev_data = this;
}

}
