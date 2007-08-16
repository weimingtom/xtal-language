
#include "xtal.h"
#include "xtal_macro.h"

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
		ArrayPtr arg_list(xnew<Array>());
		const char *filename = argv[i++];
		for(i=1; argv[i]!=0; i++){
			arg_list->push_back(argv[i]);
		}
		builtin()->def("argv", arg_list);
		load(filename);
	}
}


#include "xtal_peg.h"

void debug_line(const SmartPtr<debug::Info>& info){
	std::cout << Xf("kind=%d, line=%s, file=%s, fun=%s\n")(info->kind(), info->line(), info->file_name(), info->fun_name());

	/*if(info->local_variables()){
		Xfor2(key, value, info->local_variables()->each_member()){
			std::cout << Xf("key=%s, value=%s\n")(key, value);
		}
	}*/
}

//#include <crtdbg.h>

int calc_expr(const AnyPtr& v){
	if(v[0]=="INT") return v[1]->to_i();
	if(v[0]=="ADD") return calc_expr(v[1]) + calc_expr(v[2]);
	if(v[0]=="SUB") return calc_expr(v[1]) - calc_expr(v[2]);
	if(v[0]=="MUL") return calc_expr(v[1]) * calc_expr(v[2]);
	if(v[0]=="DIV") return calc_expr(v[1]) / calc_expr(v[2]);
	return 0;
}

int main(int argc, char** argv){

	//_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF | /*_CRTDBG_CHECK_ALWAYS_DF |*/ _CRTDBG_DELAY_FREE_MEM_DF);

	try{

		initialize();

		//debug::enable();
		//debug::set_line_hook(fun(&debug_line));
		//debug::set_call_hook(fun(&debug_line));
		//debug::set_return_hook(fun(&debug_line));
		
		
		{
			/*
			using namespace peg;
			ParserPtr anych = xnew<AnyChParser>();
			ParserPtr integer = xnew<IntParser>();

			const char* source = "aaaabbbcccd";
			StreamPtr stream = xnew<MemoryStream>(source, strlen(source));
			LexerPtr reader = xnew<Lexer>(stream);
			
			ParserPtr e = join(set("abcd")*0);
			ParserPtr re = ((insert_val("VV") >> e >> integer) | (insert_val("UU") >> e >> str("d")));

			ArrayPtr ret = xnew<Array>();
			if(re->parse(reader, ret)){
				ret->p();
			}

			/*
			using namespace peg;
			ParserPtr anych = xnew<AnyChParser>();
			ParserPtr integer = xnew<IntParser>();

			const char* source = "145+ 1-0 * 500+ (555555555+44444444) - 55/5";
			StreamPtr stream = xnew<MemoryStream>(source, strlen(source));
			LexerPtr reader = xnew<Lexer>(stream);
			
			ParserPtr expr_top = xnew<RefParser>();

			ParserPtr term = (integer >> to_node("INT", 1)) | 
				-str("(") > expr_top > -str(")");

			ParserPtr expr_mul = term > (
				(-str("*") > term >> to_node("MUL", 2)) | 
				(-str("/") > term >> to_node("DIV", 2)) 
				)*0;

			ParserPtr expr_add = expr_mul > (
				(-str("+") > expr_mul >> to_node("ADD", 2)) | 
				(-str("-") > expr_mul >> to_node("SUB", 2)) 
				)*0;

			expr_top << expr_add;

			ArrayPtr ret = xnew<Array>();

			if(expr_add->parse(reader, ret)){
				ret->p();
				AnyPtr(calc_expr(ret[0]))->p();
			}
			*/

			/*
			const char* source = "<test>qqq<o>p</o>ppp<popo/></test>";
			StreamPtr stream = xnew<MemoryStream>(source, strlen(source));
			ReaderPtr reader = xnew<Reader>();
			reader->set_stream(stream);

			ParserPtr element = xnew<RefParser>();

			ParserPtr text = join((anych - str("<") )^1);
			ParserPtr tag_name = join((anych - (str("/") | str(">")) )^1);
			ParserPtr tag = -str("<") >> tag_name >> -str(">");
			ParserPtr end_tag = -str("</") >> tag_name >> -str(">");
			ParserPtr empty_tag = -str("<") >> tag_name >> -str("/>");

			element->set_ref(
				(tag >> array(element^1) >> end_tag >> to_node("TAG", 3)) |
				(text >> to_node("TEXT", 1)) |
				(empty_tag >> to_node("ETAG", 1))
			);

			ParserPtr xml = element;

			ArrayPtr ret = xnew<Array>();

			if(xml->parse(reader, ret)){
				ret->p();
			}
			*/
		}


		{
			StringPtr path(argv[0]);

#ifdef WIN32
			StringPtr sep("\\");
#else
			StringPtr sep("/");
#endif

			ArrayPtr temp = cast<ArrayPtr>(path->split(sep)->send("to_a"));
			temp->pop_back();
#ifdef WIN32
			temp->push_back("message.xtal");
#else
			temp->push_back("message_en.xtal");
#endif
			path = temp->join(sep)->to_s();
			add_get_text_map(cast<MapPtr>(load(path)));
		}

		AnyPtr cd = Xsrc((
			
		))();

		int c;
		c = clock();
		handle_argv(argv);
		printf("%g\n", (clock()-c)/1000.0f);
		
		/*		
		c = clock();
		load("../bench/vec.xtal");
		printf("%g\n", (clock()-c)/1000.0f);		
		
		c = clock();
		load("../bench/inst.xtal");
		printf("%g\n", (clock()-c)/1000.0f);

		c = clock();
		full_gc();
		printf("%g\n", (clock()-c)/1000.0f);

		c = clock();
		load("../bench/loop.xtal");
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

		//*

		load("../test/test_empty.xtal");
		load("../test/test_array.xtal");
		load("../test/test_float.xtal");
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
		load("../test/test_serialize.xtal");
		
		//*/

	}catch(AnyPtr e){
		fprintf(stderr, "%s\n", e->to_s()->c_str());
	}

	vmachine().get()->print_info();

	uninitialize();

	return 0;
}
