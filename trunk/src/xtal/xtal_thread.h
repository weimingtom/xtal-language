
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

bool stop_the_world();
void restart_the_world();

void thread_entry(const ThreadPtr& thread);

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
