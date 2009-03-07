#include "xtal.h"
#include "xtal_macro.h"

namespace xtal{

const IDPtr& Thread::ID::intern() const{
	return xtal::intern((char_t*)&dummy_);
}

Thread::Thread(){
	inc_ref_count();
}

void Thread::visit_members(Visitor& m){
	Base::visit_members(m);
	m & callback_;
}

void Thread::begin_thread(){
	thread_entry(from_this(this));
	callback_ = null;	
	dec_ref_count();
}

Mutex::Mutex(){

}


}
