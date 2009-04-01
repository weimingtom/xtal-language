#ifndef XTAL_THREAD_H_INCLUDE_GUARD
#define XTAL_THREAD_H_INCLUDE_GUARD

#pragma once

namespace xtal{

class ThreadSpace;

/**
* \brief スレッド
*/
class Thread : public Base{
public:

	Thread();

	~Thread();

public:
	
	void start(const AnyPtr& a);

	void join();

protected:

	static void trampoline(void* data);

	void begin_thread();

protected:

	virtual void visit_members(Visitor& m);

	AnyPtr callback_;
	void* impl_;
	Environment* env_;
};

/**
* \brief ミューテックス
*/
class Mutex : public Base{
public:

	Mutex();

	~Mutex();

	void lock();

	void unlock();

	void rawlock();

private:
	void* impl_;
};

void yield_thread();
void sleep_thread(float_t sec);


}

#endif // XTAL_THREAD_H_INCLUDE_GUARD
