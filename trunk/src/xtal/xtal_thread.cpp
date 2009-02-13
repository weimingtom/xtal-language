#include "xtal.h"
#include "xtal_macro.h"

namespace xtal{

const IDPtr& Thread::ID::intern() const{
	return xtal::intern((char_t*)&dummy_);
}

ThreadMgr::ThreadMgr(ThreadLib* lib){
	thread_enabled_ = false;

	thread_count_ = 1;
	thread_locked_count_ = 0;
	thread_unlocked_count_ = 0;
	stop_the_world_ = false;
	current_thread_recursive_ = 0;

	thread_lib_ = lib;

	vm_map_ = xnew<Map>();

	mutex_ = new_mutex(); 
	mutex2_ = new_mutex(); 

	thread_enabled_ = true;
//	global_interpreter_lock();

	register_vmachine();
	
	thread_enabled_ = false;
}

ThreadMgr::~ThreadMgr(){
	vmachine_ = null;
}

void ThreadMgr::destroy(){
//	global_interpreter_unlock();
	thread_enabled_ = false;
	thread_lib_ = 0;
	vm_map_ = null;

	mutex_ = null;
	mutex2_ = null;
}

void ThreadMgr::change_vmachine(const Thread::ID& id){
	if(!current_vmachine_id_.is_valid() || !thread_lib_->equal_thread_id(current_vmachine_id_, id)){
		vmachine_ = unchecked_ptr_cast<VMachine>(vm_map_->at(id.intern()));
		current_vmachine_id_ = id;
	}
	current_thread_recursive_ = 1;
	current_thread_id_ = id;
}

void ThreadMgr::visit_members(Visitor& m){
	Base::visit_members(m);

	m & mutex_ & mutex2_ & vmachine_ & vm_map_;
}

void ThreadMgr::register_vmachine(){
	Thread::ID id;
	thread_lib_->current_thread_id(id);

	vmachine_ = xnew<VMachine>();
	vm_map_->set_at(id.intern(), vmachine_);
}

void ThreadMgr::remove_vmachine(){
	Thread::ID id;
	thread_lib_->current_thread_id(id);

	vm_map_->erase(id.intern());
}

void ThreadMgr::xlock(){
	if(!thread_enabled_){
		return;
	}

	Thread::ID id;
	thread_lib_->current_thread_id(id);
	if(current_thread_recursive_==0){
		mutex_->lock();
		change_vmachine(id);
	}
	else{
		current_thread_recursive_++;
	}
}

void ThreadMgr::xunlock(){
	if(!thread_enabled_){
		return;
	}

	current_thread_recursive_--;
	if(current_thread_recursive_==0){
		current_thread_id_.invalid();
		mutex_->unlock();
	}
}

void ThreadMgr::thread_entry(const ThreadPtr& thread){
	register_thread();

	register_vmachine();
	const VMachinePtr& vm(vmachine_);

	vm->setup_call(0);
	thread->callback()->rawcall(vm);
	vm->cleanup_call();

	vm->reset();	
	unregister_thread();
}

void ThreadMgr::register_thread(){
	if(!thread_enabled_)
		return;

	mutex_->lock();
	thread_count_++;
	mutex_->unlock();
	
	register_vmachine();

	global_interpreter_lock();
}

void ThreadMgr::unregister_thread(){
	if(!thread_enabled_)
		return;

	remove_vmachine();
	thread_count_--;

	global_interpreter_unlock();
}

void ThreadMgr::global_interpreter_lock(){
	Thread::ID id;
	thread_lib_->current_thread_id(id);
	if(current_thread_recursive_==0){
		mutex_->lock();
		change_vmachine(id);
	}
	else{
		current_thread_recursive_++;
	}
}

void ThreadMgr::global_interpreter_unlock(){
	current_thread_recursive_--;
	if(current_thread_recursive_==0){
		current_thread_id_.invalid();
		mutex_->unlock();
	}
}

void ThreadMgr::yield_thread(){
	if(thread_count_==1){
		return;
	}

	XTAL_UNLOCK{
		thread_lib_->yield();
	}
}

void ThreadMgr::sleep_thread(float_t sec){
	XTAL_UNLOCK{
		thread_lib_->sleep(sec);
	}
}

ThreadPtr ThreadMgr::new_thread(const AnyPtr& fun){
	ThreadPtr ret = thread_lib_->new_thread();
	ret->set_callback(fun);
	ret->start();
	return ret;
}

MutexPtr ThreadMgr::new_mutex(){
	return thread_lib_->new_mutex();
}

void ThreadMgr::lock_mutex(const MutexPtr& self){
	XTAL_UNLOCK{
		self->lock();
	}
}

bool stop_the_world(){
	return true;
}

void restart_the_world(){

}

void xlock(){
	core()->thread_mgr()->xlock();
}

void xunlock(){
	core()->thread_mgr()->xunlock();
}

void thread_entry(const ThreadPtr& thread){
	core()->thread_mgr()->thread_entry(thread);
}

void register_thread(){
	core()->thread_mgr()->register_thread();
}

void unregister_thread(){
	core()->thread_mgr()->unregister_thread();
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
