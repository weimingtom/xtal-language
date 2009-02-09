
#include "xtal.h"
#include "xtal_macro.h"

#include "xtal_lib/xtal_winthread.h"
#include "xtal_lib/xtal_cstdiostream.h"
#include "xtal_lib/xtal_winfilesystem.h"

using namespace xtal;

void debug_throw(const DebugInfoPtr& info){
//	puts("throw");
}

#ifndef XTAL_NO_PARSER

int main2(int argc, char** argv){
	//_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF | /*_CRTDBG_CHECK_ALWAYS_DF |*/ _CRTDBG_DELAY_FREE_MEM_DF);
	
	//setlocale(LC_ALL, "japanese");
	//set_memory(memory_block, 1024*500*2);

	using namespace std;

	CStdioStreamLib stream_lib;
	WinThreadLib win_thread_lib;
	WinFilesystemLib win_filesystem_lib;
	CoreSetting setting;
	//setting.thread_lib = &win_thread_lib;
	setting.stream_lib = &stream_lib;
	setting.filesystem_lib = &win_filesystem_lib;

	Core core;
	core.initialize(setting);

	try{
		//debug()->enable();
		//debug()->set_throw_hook(fun(&debug_throw));

		Xsrc((
10.times.ip.filter(|x|(x%2)==0).ip.map(|x|x*2).ip.with_index.ip.map(|i, v| i+v).ip{
}

		))->call(); //->inspect()->p();

#if 1

		int c;

		/*		
		c = clock();
		load("../bench/vec.xtal");
		printf("vec %g\n\n", (clock()-c)/1000.0f);		
		
		c = clock();
		load("../bench/inst.xtal");
		printf("inst %g\n\n", (clock()-c)/1000.0f);

		c = clock();
		load("../bench/gc.xtal");
		printf("gc %g\n\n", (clock()-c)/1000.0f);

		c = clock();
		load("../bench/loop.xtal");
		printf("loop %g\n\n", (clock()-c)/1000.0f);

		c = clock();
		load("../bench/nested_loops.xtal");
		printf("nested_loops %g\n\n", (clock()-c)/1000.0f);

		c = clock();
		load("../bench/fib.xtal");
		printf("fib %g\n\n", (clock()-c)/1000.0f);

		c = clock();
		load("../bench/loop_iter.xtal");
		printf("loop_iter %g\n\n", (clock()-c)/1000.0f);

		c = clock();
		load("../bench/array_for.xtal");
		printf("array_for %g\n\n", (clock()-c)/1000.0f);

		c = clock();
		load("../bench/array_each.xtal");
		printf("array_each %g\n\n", (clock()-c)/1000.0f);

		//*/

		//*
		load("../test/test.xtal_");
		lib()->member("test")->send("run_dir", "../test/");
		
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