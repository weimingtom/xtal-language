#include "xtal.h"
#include "xtal_macro.h"

namespace xtal{

bool thread_enabled_ = false;
int thread_step_counter_ = 500;

namespace{
	ThreadLib* thread_lib_temp_ = 0;
	ThreadLib* thread_lib_ = 0;

	ThreadPtr create_thread(const AnyPtr& fun){
		return thread_lib_->create_thread(fun);
	}

	MutexPtr create_mutex(){
		return thread_lib_->create_mutex();
	}

	void lock_mutex(const MutexPtr& self){
		XTAL_UNLOCK{
			self->lock();
		}
	}

	struct ThreadMgr : public Base{
		struct VMachineTableUnit{
			Thread::ID id;
			AnyPtr vm;
		};		
		
		AC<VMachineTableUnit>::vector table;

		int thread_count_;
		int thread_locked_count_;
		int thread_unlocked_count_;
		bool stop_the_world_;
		Thread::ID current_thread_id_;
		Thread::ID current_vmachine_id_;
		Thread::ID stop_the_world_thread_id_;
		int current_thread_recursive_;

		MutexPtr mutex_;
		MutexPtr mutex2_;
		VMachinePtr vmachine_;

		ThreadMgr(bool use_thread){
			thread_count_ = 1;
			thread_locked_count_ = 0;
			thread_unlocked_count_ = 0;
			stop_the_world_ = false;
			current_thread_recursive_ = 0;

			if(use_thread){
				mutex_ = create_mutex(); 
				mutex2_ = create_mutex(); 

				thread_enabled_ = true;
				global_interpreter_lock();

				register_vmachine();
			}
			else{
				vmachine_ = xnew<VMachine>();
			}
		}

		~ThreadMgr(){
			global_interpreter_unlock();
			thread_enabled_ = false;
			thread_lib_ = 0;
			table.clear();

			mutex_ = null;
			mutex2_ = null;
			vmachine_ = null;
		}

		void change_vmachine(const Thread::ID& id){
			if(!current_vmachine_id_.is_valid() || !thread_lib_->equal_thread_id(current_vmachine_id_, id)){
				for(uint_t i=0; i<table.size(); ++i){
					VMachineTableUnit& unit = table[i];
					if(thread_lib_->equal_thread_id(unit.id, id)){
						vmachine_ = (VMachinePtr&)unit.vm;
						current_vmachine_id_ = id;
						break;
					}
				}
			}
			current_thread_recursive_ = 1;
			current_thread_id_ = id;
		}

		virtual void visit_members(Visitor& m){
			Base::visit_members(m);
			for(uint_t i=0; i<table.size(); ++i){
				m & table[i].vm;
			}
		}

		void register_vmachine(){
			Thread::ID id;
			thread_lib_->current_thread_id(id);

			VMachineTableUnit unit;
			unit.vm = vmachine_ = xnew<VMachine>();
			unit.id = id;
			table.push_back(unit);	
		}

		void remove_vmachine(){
			Thread::ID id;
			thread_lib_->current_thread_id(id);

			for(uint_t i=0; i<table.size(); ++i){
				VMachineTableUnit& unit = table[i];
				if(thread_lib_->equal_thread_id(unit.id, id)){
					unit = table[table.size()-1];
					table.pop_back();
					break;
				}
			}
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

		void xlock(){
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

			XTAL_TRY{
				vm->setup_call(0);
				thread->callback()->rawcall(vm);
				vm->cleanup_call();
			}
			XTAL_CATCH(e){
				fprintf(stderr, "%s\n", e->to_s()->c_str());
			}

			vm->reset();	

			unregister_thread();
		}

		void register_thread(){
			if(!thread_enabled_)
				return;

			mutex_->lock();
			thread_count_++;
			if(thread_step_counter_>500){
				thread_step_counter_ = 500;
			}
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

	};

	SmartPtr<ThreadMgr> thread_mgr_;

	void uninitialize_thread(){
		thread_mgr_ = null;
	}
}
	
const VMachinePtr& vmachine(){
	return thread_mgr_->vmachine_;
}

int check_yield_thread(){

	if(thread_mgr_->thread_count_==1){
		thread_step_counter_ = 0x7fffffff;
		return 1;
	}

	thread_step_counter_ = 500;

	XTAL_UNLOCK{
		thread_lib_->yield();
	}

	return 1;
}

void yield_thread(){
	XTAL_UNLOCK{
		thread_lib_->yield();
	}
}

void sleep_thread(float_t sec){
	if(!thread_lib_)
		return;

	XTAL_UNLOCK{
		thread_lib_->sleep(sec);
	}
}

void initialize_thread(){
	register_uninitializer(&uninitialize_thread);

	if(thread_lib_temp_){
		thread_lib_ = thread_lib_temp_;
	}

	{
		ClassPtr p = new_cpp_class<Thread>();
		if(thread_lib_){
			p->def(Xid(new), fun(&create_thread));
		}

		p->method(Xid(join), &Thread::join);
		p->fun(Xid(yield), &yield_thread);
		p->fun(Xid(sleep), &sleep_thread);
	}

	{
		ClassPtr p = new_cpp_class<Mutex>();
		if(thread_lib_){
			p->def(Xid(new), fun(&create_mutex));
		}

		p->method(Xid(lock), &lock_mutex);
		p->method(Xid(unlock), &Mutex::unlock);
	}

	if(thread_lib_){
		thread_lib_->initialize();
		thread_mgr_ = xnew<ThreadMgr>(true);
	}
	else{
		thread_mgr_ = xnew<ThreadMgr>(false);
	}

	builtin()->def(Xid(Thread), get_cpp_class<Thread>());
	builtin()->def(Xid(Mutex), get_cpp_class<Mutex>());

}

void initialize_thread_script(){
	Xemb((

Mutex::block_first: method{
	this.lock;
	return this;
}

Mutex::block_next: method{
	this.unlock;
	return null;
}

Mutex::block_break: method{
	this.unlock;
	return null;
}
	),
"\x78\x74\x61\x6c\x01\x00\x00\x00\x00\x00\x00\x4b\x39\x00\x01\x89\x00\x01\x00\x0f\x0b\x2f\x00\x00\x00\x00\x00\x02\x0b\x25\x01\x25\x00\x37\x00\x03\x39\x00\x01\x89\x00\x02\x00\x0f\x0b\x2f\x00\x00\x00\x00\x00\x04\x01\x25\x01\x25\x00\x37\x00\x05\x39\x00\x01\x89"
"\x00\x03\x00\x0f\x0b\x2f\x00\x00\x00\x00\x00\x04\x01\x25\x01\x25\x00\x37\x00\x06\x25\x00\x8b\x00\x03\x08\x00\x00\x00\x00\x00\x02\x00\x00\x00\x12\x00\x20\x00\x00\x00\x00\x00\x04\x00\x00\x00\x12\x00\x38\x00\x00\x00\x00\x00\x06\x00\x00\x00\x12\x00\x00\x00\x00"
"\x04\x00\x00\x00\x00\x03\x06\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x08\x00\x00\x00\x05\x00\x02\x00\x00\x00\x00\x00\x00\x01\x00\x00\x20\x00\x00\x00\x05\x00\x04\x00\x00\x00\x00\x00\x00\x01\x00\x00\x38\x00\x00\x00\x05\x00\x06\x00\x00\x00\x00\x00\x00\x01\x00"
"\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x14\x00\x00\x00\x00\x11\x00\x00\x00\x00\x00\x00\x00\x06\x00\x00\x00\x00\x00\x00\x00\x03\x00\x00\x00\x03\x00\x00\x00\x06\x00\x00\x00\x08\x00\x00\x00\x04\x00\x00\x00\x10\x00\x00\x00\x05\x00\x00"
"\x00\x13\x00\x00\x00\x06\x00\x00\x00\x18\x00\x00\x00\x0b\x00\x00\x00\x18\x00\x00\x00\x08\x00\x00\x00\x1b\x00\x00\x00\x0b\x00\x00\x00\x20\x00\x00\x00\x09\x00\x00\x00\x28\x00\x00\x00\x0a\x00\x00\x00\x2b\x00\x00\x00\x0b\x00\x00\x00\x30\x00\x00\x00\x10\x00\x00"
"\x00\x30\x00\x00\x00\x0d\x00\x00\x00\x33\x00\x00\x00\x10\x00\x00\x00\x38\x00\x00\x00\x0e\x00\x00\x00\x40\x00\x00\x00\x0f\x00\x00\x00\x43\x00\x00\x00\x10\x00\x00\x00\x48\x00\x00\x00\x11\x00\x00\x00\x00\x01\x0b\x00\x00\x00\x03\x09\x00\x00\x00\x06\x73\x6f\x75"
"\x72\x63\x65\x09\x00\x00\x00\x11\x74\x6f\x6f\x6c\x2f\x74\x65\x6d\x70\x2f\x69\x6e\x2e\x78\x74\x61\x6c\x09\x00\x00\x00\x0b\x69\x64\x65\x6e\x74\x69\x66\x69\x65\x72\x73\x0a\x00\x00\x00\x07\x09\x00\x00\x00\x00\x09\x00\x00\x00\x05\x4d\x75\x74\x65\x78\x09\x00\x00"
"\x00\x04\x6c\x6f\x63\x6b\x09\x00\x00\x00\x0b\x62\x6c\x6f\x63\x6b\x5f\x66\x69\x72\x73\x74\x09\x00\x00\x00\x06\x75\x6e\x6c\x6f\x63\x6b\x09\x00\x00\x00\x0a\x62\x6c\x6f\x63\x6b\x5f\x6e\x65\x78\x74\x09\x00\x00\x00\x0b\x62\x6c\x6f\x63\x6b\x5f\x62\x72\x65\x61\x6b"
"\x09\x00\x00\x00\x06\x76\x61\x6c\x75\x65\x73\x0a\x00\x00\x00\x01\x03"
)->call();
}

void set_thread(ThreadLib& lib){
	thread_lib_temp_ = &lib;
}

#ifdef XTAL_USE_THREAD_MODEL_2

bool stop_the_world(){
	if(!thread_lib_){
		return true;
	}

	if(stop_the_world_){
		return false;
	}

	stop_the_world_ = true;
	thread_lib_->current_thread_id(stop_the_world_thread_id_);
	mutex2_.pvalue()->lock();	
	mutex_->unlock();
	yield_thread();
	
	int count = 0;
	while(true){
		mutex_->lock();

		if(thread_locked_count_+thread_unlocked_count_==thread_count_-1){
			if(count!=0){
				printf("ok locked=%d, unlocked=%d, count%d\n", thread_locked_count_, thread_unlocked_count_, thread_count_);
			}
			break;
		}
		else{
			printf("locked=%d, unlocked=%d, count%d\n", thread_locked_count_, thread_unlocked_count_, thread_count_);
			yield_thread();
			count++;
		}

		mutex_->unlock();
		yield_thread();
	}
	printf("start locked=%d, unlocked=%d, count%d\n", thread_locked_count_, thread_unlocked_count_, thread_count_);
	return true;
}

void restart_the_world(){
	if(!thread_lib_){
		return;
	}
	printf("restart the world\n");

	XTAL_ASSERT(stop_the_world_);

	stop_the_world_ = false;
	mutex2_.pvalue()->unlock();
}

void global_interpreter_lock(){
	Thread::ID id;
	thread_lib_->current_thread_id(id);
	if(!current_thread_id_.is_valid() || !thread_lib_->equal_thread_id(current_thread_id_, id)){

		mutex_->lock();
		for(;;){
			if(stop_the_world_){ // lockを獲得できたのが、世界が止まった所為なら
				thread_locked_count_++;

				mutex_->unlock(); // lockを開放し
				mutex2_.pvalue()->lock(); // 世界が動き出すまで待つ
				mutex2_.pvalue()->unlock();

				thread_locked_count_--;
				mutex_->lock();
			}
			else{
				break;
			}
		}

		// locked状態
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

void xlock(){
	Thread::ID id;
	thread_lib_->current_thread_id(id);
	if(!current_thread_id_.is_valid() || !thread_lib_->equal_thread_id(current_thread_id_, id)){

		mutex_->lock();
		for(;;){
			if(stop_the_world_){ // lockを獲得できたのが、世界が止まった所為なら
				thread_locked_count_++;

				mutex_->unlock(); // lockを開放し
				mutex2_.pvalue()->lock(); // 世界が動き出すまで待つ
				mutex2_.pvalue()->unlock();

				mutex_->lock();
				thread_locked_count_--;
			}
			else{
				thread_unlocked_count_--;
				break;
			}
		}
		
		// locked状態
		change_vmachine(id);
	}
	else{
		current_thread_recursive_++;
	}
}

void xunlock(){
	current_thread_recursive_--;
	if(current_thread_recursive_==0){
		thread_unlocked_count_++;
		current_thread_id_.invalid();
		mutex_->unlock();
	}
}

#else

bool stop_the_world(){
	return true;
}

void restart_the_world(){

}

void global_interpreter_lock(){
	thread_mgr_->global_interpreter_lock();
}

void global_interpreter_unlock(){
	thread_mgr_->global_interpreter_unlock();
}

void xlock(){
	thread_mgr_->xlock();
}

void xunlock(){
	thread_mgr_->xunlock();
}

#endif

void thread_entry(const ThreadPtr& thread){
	thread_mgr_->thread_entry(thread);
}

void register_thread(){
	thread_mgr_->register_thread();
}

void unregister_thread(){
	thread_mgr_->unregister_thread();
}


Thread::Thread(const AnyPtr& callback)
	:callback_(callback){
	inc_ref_count();
}

Mutex::Mutex(){
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

ThreadLib::~ThreadLib(){
}

}

//#define XTAL_USE_PTHREAD

#if defined(_WIN32) && (defined(_MT) || defined(__GNUC__)) && !defined(XTAL_USE_PTHREAD)

#ifndef _MT
#	define _MT
#endif

#include <windows.h>
#include <process.h>

namespace xtal{

class WinMutex : public Mutex{
	CRITICAL_SECTION sect_;
public:
	WinMutex(){
		InitializeCriticalSection(&sect_);
	}
	
	~WinMutex(){
		DeleteCriticalSection(&sect_);
	}
	
	virtual void lock(){
		EnterCriticalSection(&sect_);
	}
	
	virtual void unlock(){
		LeaveCriticalSection(&sect_);
	}
};

class WinThread : public Thread{
	HANDLE id_;
	
	static unsigned int WINAPI entry(void* self){
		((WinThread*)self)->begin_thread();
		return 0;
	}
	
public:

	WinThread(const AnyPtr& callback)
		:Thread(callback){
		id_ = (HANDLE)_beginthreadex(0, 0, &entry, this, 0, 0);
	}

	~WinThread(){
		CloseHandle(id_);
	}

	virtual void join(){
		XTAL_UNLOCK{
			WaitForSingleObject(id_, INFINITE);
		}
	}
};


class WinThreadLib : public ThreadLib{
public:

	virtual void initialize(){
		{
			ClassPtr p = new_cpp_class<WinThread>();
			p->inherit(get_cpp_class<Thread>());
		}

		{
			ClassPtr p = new_cpp_class<WinMutex>();
			p->inherit(get_cpp_class<Mutex>());
		}
	}

	virtual ThreadPtr create_thread(const AnyPtr& callback){
		return xnew<WinThread>(callback);
	}

	virtual MutexPtr create_mutex(){
		return xnew<WinMutex>();
	}

	virtual void yield(){
		Sleep(0);
	}

	virtual void sleep(float_t sec){
		Sleep((DWORD)(1000*sec));
	}

	virtual void current_thread_id(Thread::ID& id){
		id.set(GetCurrentThreadId());
	}

	virtual bool equal_thread_id(const Thread::ID& a, const Thread::ID& b){
		return a.get<uint_t>() == b.get<uint_t>();
	}

} win_thread_lib;

void set_thread(){
	set_thread(win_thread_lib);
}

}

#elif defined(XTAL_USE_PTHREAD) || defined(__CYGWIN__) 

#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>

namespace xtal{

class PThreadMutex : public Mutex{
	pthread_mutex_t mutex_;
public:
	PThreadMutex(){
		pthread_mutex_init(&mutex_, 0);
	}
	
	~PThreadMutex(){
		pthread_mutex_destroy(&mutex_);
	}
	
	virtual void lock(){
		pthread_mutex_lock(&mutex_);
	}
	
	virtual void unlock(){
		pthread_mutex_unlock(&mutex_);
	}
};

class PThread : public Thread{
	pthread_t id_;
	
	static void* entry(void* self){
		((PThread*)self)->begin_thread();
		return 0;
	}
	
public:

	PThread(const AnyPtr& callback)
		:Thread(callback){
		pthread_create(&id_, 0, entry, this);
	}

	~PThread(){
		pthread_detach(id_);
	}

	virtual void join(){
		void* p = 0;
		XTAL_UNLOCK{
			pthread_join(id_, &p);
		}
	}
};


class PThreadLib : public ThreadLib{
public:

	virtual void initialize(){
		{
			ClassPtr p = new_cpp_class<PThread>();
			p->inherit(get_cpp_class<Thread>());
		}

		{
			ClassPtr p = new_cpp_class<PThreadMutex>();
			p->inherit(get_cpp_class<Mutex>());
		}
	}

	virtual ThreadPtr create_thread(const AnyPtr& callback){
		return xnew<PThread>(callback);
	}

	virtual MutexPtr create_mutex(){
		return xnew<PThreadMutex>();
	}

	virtual void yield(){
		sched_yield();
	}

	virtual void sleep(float_t sec){
		usleep((useconds_t)(sec*1000*1000));
	}

	virtual void current_thread_id(Thread::ID& id){
		id.set(pthread_self());
	}

	virtual bool equal_thread_id(const Thread::ID& a, const Thread::ID& b){
		return pthread_equal(a.get<pthread_t>(), b.get<pthread_t>())!=0;
	}

} pthread_lib;

void set_thread(){
	set_thread(pthread_lib);
}

}

#else

namespace xtal{

void set_thread(){
	
}

}

#endif
