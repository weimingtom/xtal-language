
#pragma once

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

	WinThread(){
		id_ = (HANDLE)-1;
	}

	~WinThread(){
		if(id_!=(HANDLE)-1){
			CloseHandle(id_);
		}
	}

	virtual void start(){
		id_ = (HANDLE)_beginthreadex(0, 0, &entry, this, 0, 0);
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
		new_cpp_class<WinThread>()->inherit(get_cpp_class<Thread>());
		new_cpp_class<WinMutex>()->inherit(get_cpp_class<Mutex>());
	}

	virtual ThreadPtr new_thread(){
		return xnew<WinThread>();
	}

	virtual MutexPtr new_mutex(){
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

};

}
