
#include "xtal.h"
#include "xtal_macro.h"
#include "xtal_ch.h"

#include <stdio.h>

using namespace xtal;

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

	try{
		initialize();
		
		debug()->enable();
		debug()->set_throw_hook(fun(&debug_throw));

		Xsrc((
			filelocal.inherit(xpeg);

			assert 0 + 9 == 10;
			(9, 0, 7).p;

		[0,32,45,53,23,23,1123,43,45,3].each.scan(pred(|x|x==45) >> cap(n: any)){
			it("n")[0].p;
		}

		//"ak,byy,cser,dy,oioieeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee".scan(cap(n:alpha/1) >> alpha >> "eee"){
		//	it["n"].p;
		//}
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

	printf("-------------------\n");
	uninitialize();
	printf("-------------------\n");

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