
#pragma once

#include "xtal_any.h"

namespace xtal{

class ThreadImpl : public AnyImpl{
public:

	ThreadImpl(void (*fun)(const Any&), const Any& value);

	virtual void join() = 0;
	
	Any value() const{
		return value_;
	}

protected:

	void begin_thread();

protected:

	virtual void visit_members(Visitor& m);

	void (*fun_)(const Any&);
	Any value_;
};

class MutexImpl : public AnyImpl{
public:

	MutexImpl();

	virtual void lock() = 0;
	virtual void unlock() = 0;
};

class Thread : public Any{
public:
	
	class ID{
		//ID
		struct Dummy{ void* dummy[4]; } dummy_;
		bool valid_;
	public:
		ID(){ valid_ = false; }

		template<class T>
		void set(const T& v){
			XTAL_ASSERT(sizeof(v)<=sizeof(dummy_));
			valid_ = true;
			(T&)dummy_ = v;
		}

		template<class T>
		const T& get() const{ return (const T&)dummy_; }

		void invalid(){ valid_ = false; }
		bool is_valid() const{ return valid_; }
	};

	Thread(const Any& fun);

	Thread(const Null&)
		:Any(null){}

	explicit Thread(ThreadImpl* p)
		:Any((AnyImpl*)p){}

	explicit Thread(const ThreadImpl* p)
		:Any((AnyImpl*)p){}

	void join() const;
	
	static void yield();

	ThreadImpl* impl() const{
		return (ThreadImpl*)Any::impl();
	}
};

class Mutex : public Any{
public:

	Mutex();

	Mutex(const Null&)
		:Any(null){}

	explicit Mutex(MutexImpl* p)
		:Any((AnyImpl*)p){}

	explicit Mutex(const MutexImpl* p)
		:Any((AnyImpl*)p){}

	void lock() const;

	void unlock() const;
		
	MutexImpl* impl() const{
		return (MutexImpl*)Any::impl();
	}
};

class ThreadLib{
public:
	virtual ~ThreadLib();
	virtual ThreadImpl* create_thread(void (*fun)(const Any&), const Any& value) = 0;
	virtual MutexImpl* create_mutex() = 0;
	virtual void yield() = 0;
	virtual void current_thread_id(Thread::ID& id) = 0;
	virtual bool equal_thread_id(const Thread::ID& a, const Thread::ID& b) = 0;
};

void InitThread();
void UninitThread();

void set_thread(ThreadLib& lib);
void set_thread();

bool stop_the_world();
void restart_the_world();

void global_interpreter_lock();
void global_interpreter_unlock();

void xlock();
void xunlock();


extern bool thread_enabled_;
extern int thread_counter_;

inline int yield_thread(){
	thread_counter_ = 500;
	Thread::yield();
	return 1;
}

struct GlobalInterpreterLock{
	GlobalInterpreterLock(int){ if(thread_enabled_)global_interpreter_lock(); }
	~GlobalInterpreterLock(){ if(thread_enabled_)global_interpreter_unlock(); }
	operator bool() const{ return true; }
};

struct GlobalInterpreterUnlock{
	GlobalInterpreterUnlock(int){ if(thread_enabled_)global_interpreter_unlock(); }
	~GlobalInterpreterUnlock(){ if(thread_enabled_)global_interpreter_lock(); }
	operator bool() const{ return true; }
};

struct XUnlock{
	XUnlock(int){ if(thread_enabled_)xunlock(); }
	~XUnlock(){ if(thread_enabled_)xlock(); }
	operator bool() const{ return true; }
};

}
