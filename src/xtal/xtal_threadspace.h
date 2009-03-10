
#pragma once

namespace xtal{

void set_vmachine(const VMachinePtr& vm);

class ThreadSpace{
public:

	void initialize(ThreadLib* lib){
		thread_lib_ = lib;
		environment_ = environment();

		thread_enabled_ = false;
		registered_thread_ = false;
		thread_count_ = 1;

		mutex_ = new_mutex(); 

		thread_enabled_ = true;

		register_vmachine();
		xlock();
	}

	void uninitialize(){
		unregister_vmachine();
		xunlock();
		
		thread_enabled_ = false;
		thread_lib_ = 0;
		mutex_ = null;
	}

	void join_all_threads(){
		while(thread_count_!=1){
			yield_thread();
		}
	}

	void register_vmachine(){
		set_vmachine(xnew<VMachine>());
	}

	void unregister_vmachine(){
		set_vmachine(null);
	}

	void xlock(){
		if(!thread_enabled_){
			return;
		}

		mutex_->lock();
	}

	void xunlock(){
		if(!thread_enabled_){
			return;
		}

		mutex_->unlock();
	}

	void thread_entry(const ThreadPtr& thread){
		register_thread();

		VMachinePtr vm = vmachine();
		vm->setup_call(0);
		thread->callback()->rawcall(vm);
		vm->cleanup_call();
		vm->reset();
		vm = null;

		unregister_thread();
	}

	void register_thread(){
		XTAL_ASSERT(environment()==0);
		set_environment(environment_);

		registered_thread_ = true;
		xlock();
		registered_thread_ = false;
		thread_count_++;
		register_vmachine();
	}

	void unregister_thread(){
		unregister_vmachine();
		thread_count_--;
		xunlock();
	}

	void yield_thread(){
		if(thread_count_==1 && !registered_thread_){
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
		ret->set_thread_space(this);
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

	bool thread_enabled(){
		return thread_enabled_;
	}

private:
	int thread_count_;

	MutexPtr mutex_;

	bool thread_enabled_;
	bool registered_thread_;

	ThreadLib* thread_lib_;
	Environment* environment_;
};

}
