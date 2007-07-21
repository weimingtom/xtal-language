
#include "xtal.h"
#include "xtal_vmachineimpl.h"

using namespace xtal;

static void print_usage(){
	fprintf(stderr,
		"usage: xtal [options] [script [args]].\n"
		"Available options are:\n"
		"  -v       show version information\n"
	);
}

static void handle_argv(char** argv){
	int i;
	for(i=1; argv[i]!=0; i++){
		if(argv[i][0]!='-')
			break;

		switch(argv[i][1]){
		case 'v':
			fprintf(stderr, "xtal %d.%d.%d.%d\n", VERSION1, VERSION2, VERSION3, VERSION4);
			break;

		default:
			print_usage();
			return;
		}
	
	}

	if(argv[i]!=0){
		Array arg_list;
		const char *filename = argv[i++];
		for(i=1; argv[i]!=0; i++){
			arg_list.push_back(argv[i]);
		}
		builtin().def("argv", arg_list);
		load(filename);
	}
}

void debug_line(const debug::Info& info){
	std::cout << Xf("kind=%d, line=%s, file=%s, fun=%s\n")(info.kind(), info.line(), info.file_name(), info.fun_name());

	if(info.local_variables()){
		Xfor2(key, value, info.local_variables().each_member()){
			//std::cout << Xf("key=%s, value=%s\n")(key, value);
		}
	}
}

//#include <crtdbg.h>

int main(int argc, char** argv){

	//_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF | /*_CRTDBG_CHECK_ALWAYS_DF |*/ _CRTDBG_DELAY_FREE_MEM_DF);

	try{

		set_thread();
		initialize();

		//debug::enable();
		//debug::set_line_hook(fun(&debug_line));
		//debug::set_call_hook(fun(&debug_line));
		//debug::set_return_hook(fun(&debug_line));
		
		compile_file("../test/test_iter.xtal").inspect().p();
		load("../test/test_iter.xtal");

		{
			String path(argv[0]);


#ifdef WIN32
			String sep("\\");
#else
			String sep("/");
#endif

			Array temp = cast<Array>(path.split(sep).send("to_a"));
			temp.pop_back();
#ifdef WIN32
			temp.push_back("message.xtal");
#else
			temp.push_back("message_en.xtal");
#endif
			path = temp.join(sep).to_s();
			add_get_text_map(load(path));
		}
	
		int c;
		
		/*		
		c = clock();
		handle_argv(argv);
		printf("%g\n", (clock()-c)/1000.0f);


		c = clock();
		full_gc();
		printf("%g\n", (clock()-c)/1000.0f);

		c = clock();
		load("../bench/loop.xtal");
		printf("%g\n", (clock()-c)/1000.0f);

		c = clock();
		load("../bench/inst.xtal");
		printf("%g\n", (clock()-c)/1000.0f);

		c = clock();
		load("../bench/vec.xtal");
		printf("%g\n", (clock()-c)/1000.0f);

		c = clock();
		load("../bench/nested_loops.xtal");
		printf("%g\n", (clock()-c)/1000.0f);

		c = clock();
		load("../bench/fib.xtal");
		printf("%g\n", (clock()-c)/1000.0f);

		c = clock();
		load("../bench/loop_iter.xtal");
		printf("%g\n", (clock()-c)/1000.0f);

		c = clock();
		load("../bench/array_for.xtal");
		printf("%g\n", (clock()-c)/1000.0f);

		c = clock();
		load("../bench/array_each.xtal");
		printf("%g\n", (clock()-c)/1000.0f);

		//*/

		load("../test/test_empty.xtal");
		load("../test/test_class.xtal");
		load("../test/test_multiple_values.xtal");
		load("../test/test_except.xtal");
		load("../test/test_fiber.xtal");
		load("../test/test_fun.xtal");
		load("../test/test_iter.xtal");
		load("../test/test_fib.xtal");
		load("../test/test_calc.xtal");
		load("../test/test_for.xtal");
		load("../test/test_if.xtal");
		load("../test/test_nested_loops.xtal");
		load("../test/test_assign.xtal");
		load("../test/test_op_assign.xtal");
		load("../test/test_inc.xtal");
		load("../test/test_toplevel.xtal");
		

		/*
		MemoryStream out;
		object_dump(load("../message.xtal"), out);
		unsigned char* data = (unsigned char*)out.data();
		int size = out.tell();

		for(int i=0; i<size; ++i){
			printf("0x%02x, ", data[i]);
			if((i%8)==7){
				printf("\n");
			}
		}
		printf("\n");
		//*/

		//ix();

	}catch(Any e){
		fprintf(stderr, "%s\n", e.to_s().c_str());
	}

	vmachine().impl()->print_info();

	uninitialize();

	return 0;
}
