
#include "xtal.h"

namespace xtal{

namespace{
	int thread_count_ = 1;
	int thread_locked_count_ = 0;
	int thread_unlocked_count_ = 0;
	bool stop_the_world_ = false;
	Thread::ID current_thread_id_;
	Thread::ID current_vmachine_id_;
	Thread::ID stop_the_world_thread_id_;
	int current_thread_recursive_ = 0;
	ThreadLib* thread_lib_temp_ = 0;
	ThreadLib* thread_lib_ = 0;

	Mutex mutex_(null);
	Mutex mutex2_(null);
	VMachine vmachine_(null);

	struct VMachineTableUnit{
		Thread::ID id;
		Any vm;
	};

	AC<VMachineTableUnit>::vector vmachine_table_;

	inline void change_vmachine(const Thread::ID& id){
		if(!current_vmachine_id_.is_valid() || !thread_lib_->equal_thread_id(current_vmachine_id_, id)){
			for(uint_t i=0; i<vmachine_table_.size(); ++i){
				VMachineTableUnit& unit = vmachine_table_[i];
				if(thread_lib_->equal_thread_id(unit.id, id)){
					vmachine_ = (VMachine&)unit.vm;
					current_vmachine_id_ = id;
					break;
				}
			}
		}
		current_thread_recursive_ = 1;
		current_thread_id_ = id;
	}

	void register_vmachine(){
		Thread::ID id;
		thread_lib_->current_thread_id(id);

		VMachineTableUnit unit;
		unit.vm = vmachine_ = VMachine();
		unit.id = id;
		vmachine_table_.push_back(unit);	
	}

	void remove_vmachine(){
		Thread::ID id;
		thread_lib_->current_thread_id(id);

		for(uint_t i=0; i<vmachine_table_.size(); ++i){
			VMachineTableUnit& unit = vmachine_table_[i];
			if(thread_lib_->equal_thread_id(unit.id, id)){
				unit = vmachine_table_[vmachine_table_.size()-1];
				vmachine_table_.pop_back();
				break;
			}
		}
	}

}
	
bool thread_enabled_ = false;
int thread_counter_ = 500;

const VMachine& vmachine(){
	return vmachine_;
}

void InitThread(){
	{
		TClass<Thread> p("Thread");
		p.def("new", New<Thread, const Any&>());
		p.method("join", &Thread::join);
		p.fun("yield", &Thread::yield);
	}

	{
		TClass<Mutex> p("Mutex");
		p.def("new", New<Mutex>());
		p.method("lock", &Mutex::lock);
		p.method("unlock", &Mutex::unlock);
	}

	if(thread_lib_temp_){
		add_long_life_var(&mutex_);
		add_long_life_var(&mutex2_);
		add_long_life_var(&vmachine_);

		mutex_ = Mutex(); 
		mutex2_ = Mutex(); 

		/*
		change_vmachine(thread_lib_temp_->current_thread_id());
		current_thread_id_ = thread_lib_temp_->current_thread_id();
		current_thread_recursive_ = 1;
		*/

		thread_lib_ = thread_lib_temp_;
		thread_enabled_ = true;
		global_interpreter_lock();

		register_vmachine();
	}else{
		add_long_life_var(&vmachine_);
		vmachine_ = VMachine();
	}
}

void UninitThread(){
	if(thread_lib_){
		global_interpreter_unlock();
		thread_enabled_ = false;
		thread_lib_ = 0;
		vmachine_table_.clear();
	}
}

ThreadImpl::ThreadImpl(void (*fun)(const Any&), const Any& value)
	:fun_(fun), value_(value){
	set_class(TClass<Thread>::get());
}

MutexImpl::MutexImpl(){
	set_class(TClass<Mutex>::get());
}

void ThreadImpl::visit_members(Visitor& m){
	AnyImpl::visit_members(m);
	m & value_;
}

void ThreadImpl::begin_thread(){
	fun_(this);
	value_ = null;	
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
	mutex2_.impl()->lock();	
	mutex_.impl()->unlock();
	thread_lib_->yield();
	
	int count = 0;
	while(true){
		mutex_.impl()->lock();

		if(thread_locked_count_+thread_unlocked_count_==thread_count_-1){
			if(count!=0){
				printf("ok locked=%d, unlocked=%d, count%d\n", thread_locked_count_, thread_unlocked_count_, thread_count_);
			}
			break;
		}else{
			printf("locked=%d, unlocked=%d, count%d\n", thread_locked_count_, thread_unlocked_count_, thread_count_);
			thread_lib_->yield();
			count++;
		}

		mutex_.impl()->unlock();
		thread_lib_->yield();
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
	mutex2_.impl()->unlock();
}

void global_interpreter_lock(){
	Thread::ID id;
	thread_lib_->current_thread_id(id);
	if(!current_thread_id_.is_valid() || !thread_lib_->equal_thread_id(current_thread_id_, id)){

		mutex_.impl()->lock();
		for(;;){
			if(stop_the_world_){ // lockを獲得できたのが、世界が止まった所為なら
				thread_locked_count_++;

				mutex_.impl()->unlock(); // lockを開放し
				mutex2_.impl()->lock(); // 世界が動き出すまで待つ
				mutex2_.impl()->unlock();

				thread_locked_count_--;
				mutex_.impl()->lock();
			}else{
				break;
			}
		}

		// locked状態
		change_vmachine(id);
	}else{
		current_thread_recursive_++;
	}
}

void global_interpreter_unlock(){
	current_thread_recursive_--;
	if(current_thread_recursive_==0){
		current_thread_id_.invalid();
		mutex_.impl()->unlock();
	}
}

void xlock(){
	Thread::ID id;
	thread_lib_->current_thread_id(id);
	if(!current_thread_id_.is_valid() || !thread_lib_->equal_thread_id(current_thread_id_, id)){

		mutex_.impl()->lock();
		for(;;){
			if(stop_the_world_){ // lockを獲得できたのが、世界が止まった所為なら
				thread_locked_count_++;

				mutex_.impl()->unlock(); // lockを開放し
				mutex2_.impl()->lock(); // 世界が動き出すまで待つ
				mutex2_.impl()->unlock();

				mutex_.impl()->lock();
				thread_locked_count_--;
			}else{
				thread_unlocked_count_--;
				break;
			}
		}
		
		// locked状態
		change_vmachine(id);
	}else{
		current_thread_recursive_++;
	}
}

void xunlock(){
	current_thread_recursive_--;
	if(current_thread_recursive_==0){
		thread_unlocked_count_++;
		current_thread_id_.invalid();
		mutex_.impl()->unlock();
	}
}

#else

bool stop_the_world(){
	return true;
}

void restart_the_world(){

}

void global_interpreter_lock(){
	Thread::ID id;
	thread_lib_->current_thread_id(id);
	if(!current_thread_id_.is_valid() || !thread_lib_->equal_thread_id(current_thread_id_, id)){
		mutex_.impl()->lock();
		// locked状態
		change_vmachine(id);
	}else{
		current_thread_recursive_++;
	}
}

void global_interpreter_unlock(){
	current_thread_recursive_--;
	if(current_thread_recursive_==0){
		current_thread_id_.invalid();
		mutex_.impl()->unlock();
	}
}

void xlock(){
	Thread::ID id;
	thread_lib_->current_thread_id(id);
	if(!current_thread_id_.is_valid() || !thread_lib_->equal_thread_id(current_thread_id_, id)){
		mutex_.impl()->lock();
		// locked状態
		change_vmachine(id);
	}else{
		current_thread_recursive_++;
	}
}

void xunlock(){
	current_thread_recursive_--;
	if(current_thread_recursive_==0){
		current_thread_id_.invalid();
		mutex_.impl()->unlock();
	}
}

#endif

void thread_entry(const Any& thread){
	mutex_.impl()->lock();
	thread_count_++;
	mutex_.impl()->unlock();

	{
		GlobalInterpreterLock guard(0);

		register_vmachine();
		const VMachine& vm(vmachine_);
		XTAL_TRY{
			vm.setup_call(0);
			((ThreadImpl*)thread.impl())->value().rawcall(vm);
			vm.cleanup_call();
		}XTAL_CATCH(e){
			std::cout << e << std::endl;
		}
		vm.reset();	
		remove_vmachine();

		thread_count_--;
		thread.impl()->dec_ref_count();
	}
}

ThreadLib::~ThreadLib(){}

Thread::Thread(const Any& fun){
	if(!thread_lib_){
		XTAL_THROW(unsupported_error("Thread", "new"));
		return;
	}
	*this = Thread(thread_lib_->create_thread(&thread_entry, fun));
}

void Thread::yield(){
	if(!thread_lib_){
		return;
	}
	XTAL_UNLOCK{
		thread_lib_->yield();
	}
}

void Thread::join() const{
	XTAL_UNLOCK{
		impl()->join();
	}
}

Mutex::Mutex(){
	if(!thread_lib_temp_){
		XTAL_THROW(unsupported_error("Mutex", "new"));
		return;
	}
	*this = Mutex(thread_lib_temp_->create_mutex()); 
	impl()->dec_ref_count();
}

void Mutex::lock() const{
	XTAL_UNLOCK{
		impl()->lock();
	}
}

void Mutex::unlock() const{
	impl()->unlock();
}

}


#if defined(_WIN32) && (defined(_MT) || defined(__GNUC__))

#ifndef _MT
#	define _MT
#endif

#include <windows.h>
#include <process.h>

namespace xtal{

class WinMutex : public MutexImpl{
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

class WinThread : public ThreadImpl{
	HANDLE id_;
	
	static unsigned int WINAPI entry(void* self){
		((WinThread*)self)->begin_thread();
		return 0;
	}
	
public:

	WinThread(void (*fun)(const Any&), const Any& value)
		:ThreadImpl(fun, value){
		id_ = (HANDLE)_beginthreadex(0, 0, &entry, this, 0, 0);
	}

	~WinThread(){
		CloseHandle(id_);
	}

	virtual void join(){
		WaitForSingleObject(id_, INFINITE);
	}
};


class WinThreadLib : public ThreadLib{
public:

	virtual ThreadImpl* create_thread(void (*fun)(const Any&), const Any& value){
		return new WinThread(fun, value);
	}

	virtual MutexImpl* create_mutex(){
		return new WinMutex();
	}

	virtual void yield(){
		Sleep(0);
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

#elif defined(XTAL_USE_PTHREAD)

#include <pthread.h>

namespace xtal{

class PThreadMutex : public MutexImpl{
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

class PThreadThread : public ThreadImpl{
	pthread_t id_;
	
	static void* entry(void* self){
		((PThreadThread*)self)->begin_thread();
		return 0;
	}
	
public:

	PThreadThread(void (*fun)(const Any&), const Any& value)
		:ThreadImpl(fun, value){
		pthread_create(&id_, 0, entry, this);
	}

	~PThreadThread(){
		pthread_detach(&id_);
	}

	virtual void join(){
		void* p = 0;
		pthread_join(&id_, &p);
	}
};


class PThreadThreadLib : public ThreadLib{
public:

	virtual ThreadImpl* create_thread(void (*fun)(const Any&), const Any& value){
		return new PThreadThread(fun, value);
	}

	virtual MutexImpl* create_mutex(){
		return new PThreadMutex();
	}

	virtual void yield(){
		sched_yield();
	}

	virtual void current_thread_id(Thread::ID& id){
		id.set(pthread_self());
	}

	virtual bool equal_thread_id(const Thread:ID& a, const Thread::ID& b){
		return pthread_equal(a.get<pthread_t>(), b.get<pthread_t>())!=0;
	}

} pthread_thread_lib;

void set_thread(){
	set_thread(pthread_thread_lib);
}

}

#else

namespace xtal{

void set_thread(){
	
}

}

#endif
