
#pragma once

namespace xtal{

class ThreadSpace;

class Thread : public Base{
public:

	Thread();

public:
	
	virtual void start() = 0;

	virtual void join() = 0;
	
	void set_callback(const AnyPtr& a){
		callback_ = a;
	}

	void set_thread_space(ThreadSpace* a){
		thread_space_ = a;
	}

	AnyPtr callback(){
		return callback_;
	}

protected:

	void begin_thread();

protected:

	virtual void visit_members(Visitor& m);

	AnyPtr callback_;
	ThreadSpace* thread_space_;
};

class Mutex : public Base{
public:

	Mutex();

	virtual void lock(){}
	virtual void unlock(){}
};

}
