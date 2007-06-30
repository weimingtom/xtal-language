
#include "xtal.h"

namespace xtal{

namespace{
	int thread_count_ = 1;
	int thread_locked_count_ = 0;
	int thread_unlocked_count_ = 0;
	bool stop_the_world_ = false;
	uint_t current_thread_id_ = (uint_t)-1;
	uint_t current_vmachine_id_ = (uint_t)-1;
	int current_thread_recursive_ = 0;
	ThreadLib* thread_lib_temp_ = 0;
	ThreadLib* thread_lib_ = 0;

	Mutex mutex_(null);
	Mutex mutex2_(null);
	VMachine vmachine_(null);

	StrictMap vmachine_table_;
	
	inline void change_vmachine(uint_t id){
		if(current_vmachine_id_!=id){
			if(const Any& ret = vmachine_table_.at(UncountedAny((int_t)id).cref())){
				vmachine_ = (const VMachine&)ret;
			}else{
				vmachine_ = VMachine();
				vmachine_table_.set_at(UncountedAny((int_t)id).cref(), vmachine_);
			}
			current_vmachine_id_ = id;
		}
	}

}
	
bool thread_enabled_ = false;

const VMachine& vmachine(){
	return vmachine_;
}

void InitThread(){
	{
		TClass<Thread> p("Thread");
		p.def("new", New<Thread, const Any&>());
		p.method("join", &Thread::join);
		p.fun("sleep", &Thread::sleep);
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
		vmachine_table_.destroy();
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

bool stop_the_world(){
	if(!thread_lib_){
		return true;
	}

	if(stop_the_world_){
		return false;
	}

	stop_the_world_ = true;
	mutex2_.impl()->lock();	
	mutex_.impl()->unlock();
	thread_lib_->sleep(0);
	
	while(true){
		mutex_.impl()->lock();

		if(thread_locked_count_+thread_unlocked_count_==thread_count_-1){
			break;
		}/*else{
			printf("locked=%d, unlocked=%d, count%d\n", thread_locked_count_, thread_unlocked_count_, thread_count_);
			thread_lib_->sleep(1000);
		}*/

		mutex_.impl()->unlock();
		thread_lib_->sleep(0);
	}
	return true;
}

void restart_the_world(){
	if(!thread_lib_){
		return;
	}
	
	if(!stop_the_world_){
		return;
	}

	stop_the_world_ = false;
	mutex2_.impl()->unlock();
}

void global_interpreter_lock(){
	uint_t id = thread_lib_->current_thread_id();
	if(current_thread_id_!=id){
		mutex_.impl()->lock();

		if(stop_the_world_){ // lockを獲得できたのが、世界が止まった所為なら
			thread_locked_count_++;
			mutex_.impl()->unlock();
			
			mutex2_.impl()->lock();
			mutex2_.impl()->unlock();
			
			mutex_.impl()->lock();
			thread_locked_count_--;
		}
		
		// locked状態
		current_thread_id_ = id;
		current_thread_recursive_ = 1;
		change_vmachine(id);
	}else{
		current_thread_recursive_++;
	}
}

void global_interpreter_unlock(){
	current_thread_recursive_--;
	if(current_thread_recursive_==0){
		current_thread_id_ = (uint_t)-1;
		mutex_.impl()->unlock();
	}
}

void xlock(){
	uint_t id = thread_lib_->current_thread_id();
	if(current_thread_id_!=id){
		mutex_.impl()->lock();
		thread_unlocked_count_--;

		if(stop_the_world_){ // lockを獲得できたのが、世界が止まった所為なら
			thread_locked_count_++;
			mutex_.impl()->unlock();
			
			mutex2_.impl()->lock();
			mutex2_.impl()->unlock();
			
			mutex_.impl()->lock();
			thread_locked_count_--;
		}
		
		// locked状態
		current_thread_id_ = id;
		current_thread_recursive_ = 1;
		change_vmachine(id);
	}else{
		current_thread_recursive_++;
	}
}

void xunlock(){
	current_thread_recursive_--;
	if(current_thread_recursive_==0){
		thread_unlocked_count_++;
		mutex_.impl()->unlock();
		current_thread_id_ = (uint_t)-1;
	}
}

void thread_entry(const Any& thread){
	mutex_.impl()->lock();
	thread_count_++;
	mutex_.impl()->unlock();

	XTAL_GLOBAL_INTERPRETER_LOCK{
		VMachine vm;
		XTAL_TRY{
			vm.setup_call(0);
			((ThreadImpl*)thread.impl())->value().call(vm);
			vm.cleanup_call();
		}XTAL_CATCH(e){
			std::cout << e << std::endl;
		}
		vm.reset();
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

void Thread::sleep(uint_t millisec){
	if(!thread_lib_){
		return;
	}
	XTAL_UNLOCK{
		thread_lib_->sleep(millisec);
	}
}

uint_t Thread::current_thread_id(){
	if(!thread_lib_){
		return 0;
	}
	return thread_lib_->current_thread_id();
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


#if defined(_WIN32) && defined(_MT)

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

	virtual void sleep(uint_t millisec){
		Sleep(millisec);
	}

	virtual uint_t current_thread_id(){
		return GetCurrentThreadId();
	}

} win_thread_lib;

void set_thread(){
	set_thread(win_thread_lib);
}

}

#else

namespace xtal{

void set_thread(){
	
}

}

#endif
