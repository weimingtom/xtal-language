
#pragma once

#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>

namespace xtal{

class PThreadMutex{
	pthread_mutex_t mutex_;
public:
	PThreadMutex(){
		pthread_mutex_init(&mutex_, 0);
	}
	
	~PThreadMutex(){
		pthread_mutex_destroy(&mutex_);
	}
	
	void lock(){
		pthread_mutex_lock(&mutex_);
	}
	
	void unlock(){
		pthread_mutex_unlock(&mutex_);
	}
};

class PThread{
	bool start_;
	pthread_t id_;
	void (*callback_)(void*);
	void* data_;

	static void* entry(void* self){
		((PThread*)self)->begin_thread();
		return 0;
	}
	
public:

	PThread(){
          start_ = false;
	}

	~PThread(){
		if(start_){
			pthread_detach(id_);
		}
	}

	void start(void (*callback)(void*), void* data){
		callback_ = callback;
		data_ = data;
		start_ = true;
		pthread_create(&id_, 0, entry, this);
	}

	void join(){
		if(start_){
			void* p = 0;
			pthread_join(id_, &p);
		}
	}

	void begin_thread(){
		if(start_){
			callback_(data_);
		}
	}
};

class PThreadLib : public ThreadLib{
public:

	virtual void* new_thread(){
		void* p = xmalloc(sizeof(PThread));
		return new(p) PThread();
	}

	virtual void delete_thread(void* thread_object){
		((PThread*)thread_object)->~PThread();
		xfree(thread_object, sizeof(PThread));
	}

	virtual void start_thread(void* thread_object, void (*callback)(void*), void* data){
		((PThread*)thread_object)->start(callback, data);
	}

	virtual void join_thread(void* thread_object){
		((PThread*)thread_object)->join();
	}

	virtual void* new_mutex(){
		void* p = xmalloc(sizeof(PThreadMutex));
		return new(p) PThreadMutex();
	}

	virtual void delete_mutex(void* mutex_object){
		((PThreadMutex*)mutex_object)->~PThreadMutex();
		xfree(mutex_object, sizeof(PThreadMutex));
	}

	virtual void lock_mutex(void* mutex_object){
		((PThreadMutex*)mutex_object)->lock();
	}

	virtual void unlock_mutex(void* mutex_object){
		((PThreadMutex*)mutex_object)->unlock();
	}

	virtual void yield(){
		sched_yield();
	}

	virtual void sleep(float_t sec){
		usleep((useconds_t)(sec*1000*1000));
	}
};

}
