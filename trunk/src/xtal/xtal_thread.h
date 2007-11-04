
#pragma once

#include "xtal_any.h"

namespace xtal{

class Thread : public Base{
public:

	class ID{
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

public:

	Thread(const AnyPtr& callback);

public:

	virtual void join() = 0;
	
	AnyPtr callback(){
		return callback_;
	}

protected:

	void begin_thread();

protected:

	virtual void visit_members(Visitor& m);

	AnyPtr callback_;
};

class Mutex : public Base{
public:

	Mutex();

	virtual void lock() = 0;
	virtual void unlock() = 0;
};


class ThreadLib{
public:
	virtual ~ThreadLib();
	virtual void initialize() = 0;
	virtual ThreadPtr create_thread(const AnyPtr& callback) = 0;
	virtual MutexPtr create_mutex() = 0;
	virtual void yield() = 0;
	virtual void sleep(float_t sec) = 0;
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

void register_thread();
void unregister_thread();

extern bool thread_enabled_;
extern int thread_step_counter_;

int check_yield_thread();

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
