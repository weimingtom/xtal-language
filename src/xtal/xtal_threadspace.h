
#pragma once

namespace xtal{

class ThreadSpace{
public:

	void initialize(ThreadLib* lib){
		thread_enabled_ = false;

		thread_count_ = 1;
		thread_locked_count_ = 0;
		thread_unlocked_count_ = 0;
		stop_the_world_ = false;
		current_thread_recursive_ = 0;
		thread_lib_ = lib;
		vm_map_ = xnew<Map>();
		mutex_ = new_mutex(); 
		thread_enabled_ = true;
	//	global_interpreter_lock();
		register_vmachine();
		thread_enabled_ = false;
	}

	void uninitialize(){
	//	global_interpreter_unlock();
		thread_enabled_ = false;
		thread_lib_ = 0;
		vm_map_ = null;
		mutex_ = null;
		vmachine_ = null;
	}

	void change_vmachine(const Thread::ID& id){
		if(!current_vmachine_id_.is_valid() || !thread_lib_->equal_thread_id(current_vmachine_id_, id)){
			vmachine_ = unchecked_ptr_cast<VMachine>(vm_map_->at(id.intern()));
			current_vmachine_id_ = id;
		}
		current_thread_recursive_ = 1;
		current_thread_id_ = id;
	}

	void register_vmachine(){
		Thread::ID id;
		thread_lib_->current_thread_id(id);

		vmachine_ = xnew<VMachine>();
		vm_map_->set_at(id.intern(), vmachine_);
	}

	void remove_vmachine(){
		Thread::ID id;
		thread_lib_->current_thread_id(id);

		vm_map_->erase(id.intern());
	}

	void xlock(){
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

	void xunlock(){
		if(!thread_enabled_){
			return;
		}

		current_thread_recursive_--;
		if(current_thread_recursive_==0){
			current_thread_id_.invalid();
			mutex_->unlock();
		}
	}

	void thread_entry(const ThreadPtr& thread){
		register_thread();

		register_vmachine();
		const VMachinePtr& vm(vmachine_);

		vm->setup_call(0);
		thread->callback()->rawcall(vm);
		vm->cleanup_call();

		vm->reset();	
		unregister_thread();
	}

	void register_thread(){
		if(!thread_enabled_)
			return;

		mutex_->lock();
		thread_count_++;
		mutex_->unlock();
		
		register_vmachine();

		global_interpreter_lock();
	}

	void unregister_thread(){
		if(!thread_enabled_)
			return;

		remove_vmachine();
		thread_count_--;

		global_interpreter_unlock();
	}

	void global_interpreter_lock(){
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

	void global_interpreter_unlock(){
		current_thread_recursive_--;
		if(current_thread_recursive_==0){
			current_thread_id_.invalid();
			mutex_->unlock();
		}
	}

	void yield_thread(){
		if(thread_count_==1){
			return;
		}

		XTAL_UNLOCK{
			thread_lib_->yield();
		}
	}

	void sleep_thread(float_t sec){
		XTAL_UNLOCK{
			thread_lib_->sleep(sec);
		}
	}

	ThreadPtr new_thread(const AnyPtr& fun){
		ThreadPtr ret = thread_lib_->new_thread();
		ret->set_callback(fun);
		ret->start();
		return ret;
	}

	MutexPtr new_mutex(){
		return thread_lib_->new_mutex();
	}

	void lock_mutex(const MutexPtr& self){
		XTAL_UNLOCK{
			self->lock();
		}
	}

	const VMachinePtr& ThreadSpace::vmachine(){
		return vmachine_;
	}

	bool thread_enabled(){
		return thread_enabled_;
	}

private:

	MapPtr vm_map_;

	int thread_count_;
	int thread_locked_count_;
	int thread_unlocked_count_;
	bool stop_the_world_;

	Thread::ID current_thread_id_;
	Thread::ID current_vmachine_id_;
	Thread::ID stop_the_world_thread_id_;
	int current_thread_recursive_;

	MutexPtr mutex_;
	VMachinePtr vmachine_;

	bool thread_enabled_;

	ThreadLib* thread_lib_;
};

}
