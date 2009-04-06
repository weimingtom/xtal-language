#ifndef XTAL_THREAD_H_INCLUDE_GUARD
#define XTAL_THREAD_H_INCLUDE_GUARD

#pragma once

namespace xtal{

class ThreadSpace;

/**
* \ingroup Xtal Cpp
* \brief スレッド
*/
class Thread : public Base{
public:

	/**
	* \brief スレッドオブジェクトを生成する
	*/
	Thread();

	~Thread();

public:

	/**
	* \brief スレッドを開始する
	*/
	void start(const AnyPtr& callback);

	/**
	* \brief スレッドが終わるまで待機する
	*/
	void join();

public:

	/**
	* \brief スレッドを切り替える
	*/
	static void yield();
	
	/**
	* \brief カレントスレッドをsec秒眠らせる
	*/
	static void sleep(float_t sec);

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
* \ingroup Xtal Cpp
* \brief ミューテックス
*/
class Mutex : public Base{
public:

	/**
	* \brief ミューテックスオブジェクトを生成する
	*/
	Mutex();

	~Mutex();

	/**
	* \brief ロックする
	*/
	void lock();

	/**
	* \brief アンロックする
	*/
	void unlock();

	void rawlock();

private:
	void* impl_;
};

void yield_thread();
void sleep_thread(float_t sec);

}

#endif // XTAL_THREAD_H_INCLUDE_GUARD
