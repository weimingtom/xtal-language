
#include "xtal.h"

using namespace xtal;

#ifndef XTAL_NO_PARSER

static void print_usage(){
	fprintf(stdout,
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
			fprintf(stdout, "xtal %d.%d.%d.%d\n", VERSION1, VERSION2, VERSION3, VERSION4);
			break;

		default:
			print_usage();
			return;
		}
	
	}

	if(argv[i]!=0){
		ArrayPtr arg_list(xnew<Array>());
		const char *filename = argv[i++];
		for(i=1; argv[i]!=0; i++){
			arg_list->push_back(argv[i]);
		}
		builtin()->def("argv", arg_list);
		load(filename);
	}
}

int main(int argc, char** argv){

	XTAL_TRY{
		set_thread();
		initialize();

		handle_argv(argv);
	}
	XTAL_CATCH(e){
		stderr_stream()->put_s(e->to_s());
		stderr_stream()->put_s("\n");
	}

	uninitialize();

	return 0;
}

#else

int main(int argc, char** argv){
	puts("xtal no parser");
	return 0;
}

#endif