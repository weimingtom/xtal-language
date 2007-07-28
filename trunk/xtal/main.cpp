
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

int main(int argc, char** argv){

	try{

		set_thread();
		initialize();
		
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

		handle_argv(argv);

	}catch(Any e){
		fprintf(stderr, "%s\n", e.to_s().c_str());
	}

	uninitialize();

	return 0;
}
