
#pragma once

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

	PThread(){
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
		new_cpp_class<PThread>()->inherit(cpp_class<Thread>());
		new_cpp_class<PThreadMutex>()->inherit(cpp_class<Mutex>());
	}

	virtual ThreadPtr new_thread(){
		return xnew<PThread>();
	}

	virtual MutexPtr new_mutex(){
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

};