
#pragma once

namespace xtal{

class Thread : public Base{
public:

	class ID{
		struct Dummy{ int_t dummy[4]; } dummy_;
		bool valid_;
	public:
		ID(){ valid_ = false; }

		template<class T>
		void set(const T& v){
			XTAL_ASSERT(sizeof(v)<=sizeof(dummy_)-sizeof(int_t));
			valid_ = true;
			dummy_.dummy[0] = dummy_.dummy[1] = dummy_.dummy[2] = dummy_.dummy[3] = 0;
			(T&)dummy_ = v;
		}

		template<class T>
		const T& get() const{ return (const T&)dummy_; }

		void invalid(){ valid_ = false; }

		bool is_valid() const{ return valid_; }

		const IDPtr& intern() const;
	};

public:

	Thread();

public:
	
	virtual void start() = 0;

	virtual void join() = 0;
	
	void set_callback(const AnyPtr& a){
		callback_ = a;
	}

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

	virtual void lock(){}
	virtual void unlock(){}
};


class ThreadLib{
public:
	virtual ~ThreadLib(){}
	virtual void initialize(){}
	virtual ThreadPtr new_thread(){ return ThreadPtr(); }
	virtual MutexPtr new_mutex(){ return xnew<Mutex>(); }
	virtual void yield(){}
	virtual void sleep(float_t sec){}
	virtual void current_thread_id(Thread::ID& id){ id.set(0); }
	virtual bool equal_thread_id(const Thread::ID& a, const Thread::ID& b){ return true; }
};

class ThreadMgr{
public:

	void initialize(ThreadLib* lib = 0);

	void uninitialize();

public:

	void change_vmachine(const Thread::ID& id);

	void register_vmachine();

	void remove_vmachine();

	void xlock();

	void xunlock();

	void thread_entry(const ThreadPtr& thread);

	void register_thread();

	void unregister_thread();

	void global_interpreter_lock();

	void global_interpreter_unlock();

	void yield_thread();

	void sleep_thread(float_t sec);

	bool thread_enabled(){
		return thread_enabled_;
	}

	const VMachinePtr& vmachine();

	ThreadPtr new_thread(const AnyPtr& fun);

	MutexPtr new_mutex();

	void lock_mutex(const MutexPtr& self);

	void visit_members(Visitor& m);

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
	MutexPtr mutex2_;
	VMachinePtr vmachine_;

	bool thread_enabled_;

	ThreadLib* thread_lib_;
};

bool stop_the_world();
void restart_the_world();

void xlock();
void xunlock();

void register_thread();
void unregister_thread();

struct XUnlock{
	XUnlock(int){ xunlock(); }
	~XUnlock(){ xlock(); }
	operator bool() const{ return true; }
};

}
