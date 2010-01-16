#include "xtal.h"
#include "xtal_macro.h"

#include "xtal_threadspace.h"

namespace xtal{

Thread::Thread(){
	impl_ = thread_lib()->new_thread();
}

Thread::~Thread(){
	thread_lib()->delete_thread(impl_);
}

void Thread::on_visit_members(Visitor& m){
	Base::on_visit_members(m);
	m & callback_;
}

void Thread::start(const AnyPtr& callback){
	callback_ = callback;
	env_ = environment();
	thread_lib()->start_thread(impl_, &trampoline, this);
}

void Thread::join(){
	XTAL_UNLOCK{
		thread_lib()->join_thread(impl_);
	}
}

void Thread::trampoline(void* data){
	((Thread*)data)->begin_thread();
}

void Thread::begin_thread(){
	register_thread(env_);

	VMachinePtr vm = vmachine();
	vm->setup_call(0);
	callback_->rawcall(vm);
	vm->cleanup_call();
	callback_ = null;
	vm->reset();
	vm = null;

	unregister_thread(env_);
}

void Thread::yield(){
	yield_thread();
}
	
void Thread::sleep(float_t sec){
	sleep_thread(sec);
}

Mutex::Mutex(){
	impl_ = thread_lib()->new_mutex();
}

Mutex::~Mutex(){
	thread_lib()->delete_mutex(impl_);
}

void Mutex::lock(){
	XTAL_UNLOCK{
		thread_lib()->lock_mutex(impl_);
	}
}

void Mutex::unlock(){
	thread_lib()->unlock_mutex(impl_);
}

void Mutex::rawlock(){
	thread_lib()->lock_mutex(impl_);
}

}
