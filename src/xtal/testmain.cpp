
#include "xtal.h"
#include "xtal_macro.h"
#include "xtal_ch.h"

#include <stdio.h>

#include <windows.h>
#include <process.h>

using namespace xtal;

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
		{
			ClassPtr p = new_cpp_class<WinThread>();
			p->inherit(get_cpp_class<Thread>());
		}

		{
			ClassPtr p = new_cpp_class<WinMutex>();
			p->inherit(get_cpp_class<Mutex>());
		}
	}

	virtual ThreadPtr create_thread(){
		return xnew<WinThread>();
	}

	virtual MutexPtr create_mutex(){
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

} win_thread_lib;


void debug_throw(const DebugInfoPtr& info){
//	puts("throw");
}

#ifndef XTAL_NO_PARSER

//}}REPEAT}

char memory_block[1024*1000*5];

int main2(int argc, char** argv){
	//_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF | /*_CRTDBG_CHECK_ALWAYS_DF |*/ _CRTDBG_DELAY_FREE_MEM_DF);
	
	setlocale(LC_ALL, "japanese");
	//set_memory(memory_block, 1024*1000*5);

	using namespace std;

	Core core;
	CoreSetting setting;
	setting.thread_lib = &win_thread_lib;
	core.initialize(&setting);

	try{

		//debug()->enable();
		//debug()->set_throw_hook(fun(&debug_throw));

		ArrayPtr a = xnew<Array>(10);

		Xsrc((/*
			f: fiber{
				yield 9;
				yield 10;
			}

			f().p;*/
		))->call();

#if 1

		int c;

		/*		
		c = clock();
		load("../../bench/vec.xtal");
		printf("vec %g\n\n", (clock()-c)/1000.0f);		
		
		c = clock();
		load("../../bench/inst.xtal");
		printf("inst %g\n\n", (clock()-c)/1000.0f);

		c = clock();
		full_gc();
		printf("full_gc %g\n\n", (clock()-c)/1000.0f);

		c = clock();
		load("../../bench/loop.xtal");
		printf("loop %g\n\n", (clock()-c)/1000.0f);

		c = clock();
		load("../../bench/nested_loops.xtal");
		printf("nested_loops %g\n\n", (clock()-c)/1000.0f);

		c = clock();
		load("../../bench/fib.xtal");
		printf("fib %g\n\n", (clock()-c)/1000.0f);

		c = clock();
		load("../../bench/loop_iter.xtal");
		printf("loop_iter %g\n\n", (clock()-c)/1000.0f);

		c = clock();
		load("../../bench/array_for.xtal");
		printf("array_for %g\n\n", (clock()-c)/1000.0f);

		c = clock();
		load("../../bench/array_each.xtal");
		printf("array_each %g\n\n", (clock()-c)/1000.0f);

		//*/

		//*
		debug()->enable();

#ifdef XTAL_USE_WCHAR
		load("../../test-utf16le/test_empty.xtal");
		load("../../test-utf16le/test_multiple_values.xtal");
		load("../../test-utf16le/test_array.xtal");
		load("../../test-utf16le/test_float.xtal");
		load("../../test-utf16le/test_class.xtal");
		load("../../test-utf16le/test_except.xtal");
		load("../../test-utf16le/test_fiber.xtal");
		load("../../test-utf16le/test_fun.xtal");
		load("../../test-utf16le/test_iter.xtal");
		load("../../test-utf16le/test_fib.xtal");
		load("../../test-utf16le/test_calc.xtal");
		load("../../test-utf16le/test_for.xtal");
		load("../../test-utf16le/test_if.xtal");
		load("../../test-utf16le/test_nested_loops.xtal");
		load("../../test-utf16le/test_assign.xtal");
		load("../../test-utf16le/test_op_assign.xtal");
		load("../../test-utf16le/test_inc.xtal");
		load("../../test-utf16le/test_toplevel.xtal");
		load("../../test-utf16le/test_serialize.xtal");
#else
		load("../../test/test_empty.xtal");
		load("../../test/test_multiple_values.xtal");
		load("../../test/test_array.xtal");
		load("../../test/test_map.xtal");
		load("../../test/test_float.xtal");
		load("../../test/test_class.xtal");
		load("../../test/test_except.xtal");
		load("../../test/test_fiber.xtal");
		load("../../test/test_fun.xtal");
		load("../../test/test_iter.xtal");
		load("../../test/test_fib.xtal");
		load("../../test/test_calc.xtal");
		load("../../test/test_for.xtal");
		load("../../test/test_if.xtal");
		load("../../test/test_nested_loops.xtal");
		load("../../test/test_assign.xtal");
		load("../../test/test_op_assign.xtal");
		load("../../test/test_inc.xtal");
		load("../../test/test_toplevel.xtal");
		load("../../test/test_serialize.xtal");
		load("../../test/test_string.xtal");
#endif
		
		//*/
#endif

	}
	catch(AnyPtr e){
		stderr_stream()->put_s(e->to_s());
		stderr_stream()->put_s("\n");
	}

	vmachine()->print_info();

	return 0;
}

int main(int argc, char** argv){
	int ret = main2(argc, argv);
	return ret;
}

#else

int main(int argc, char** argv){
	try{
		initialize();
	}
	catch(AnyPtr e){
		stderr_stream()->put_s(e->to_s());
		stderr_stream()->put_s("\n");
	}

	vmachine()->print_info();
	print_result_of_cache();

	printf("-------------------\n");
	uninitialize();
	printf("-------------------\n");
	return 0;
}

#endif