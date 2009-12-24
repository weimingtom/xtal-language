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

SmartPtr<Any>& SmartPtr<Any>::operator =(const SmartPtr<Any>& p){
	dec_ref_count_force(*this);
	*(Any*)this = p;
	inc_ref_count_force(*this);
	return *this;
}

SmartPtr<Any>::SmartPtr(RefCountingBase* p, int_t type, special_ctor_t)
	:Any(noinit_t()){
	value_.init(type, p);
	register_gc(p);
}

SmartPtr<Any>::SmartPtr(Base* p, const ClassPtr& c, special_ctor_t)
	:Any(p){
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

}
