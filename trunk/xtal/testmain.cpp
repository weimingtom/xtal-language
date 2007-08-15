
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


#include "xtal_parser.h"

using namespace xtal;
typedef SmartPtr<Reader> ReaderPtr;


namespace grammer{

class Parser;

class ParserPtr : public SmartPtr<Parser>{
public:

	ParserPtr(const SmartPtr<Parser>& p = null)
		:SmartPtr<Parser>(p){}

};


class Parser : public Base{
public:
	
	virtual bool parse(const ReaderPtr& r, const ArrayPtr& out) = 0;
	
	bool try_parse(const ReaderPtr& r, const ArrayPtr& out){
		if(out){
			int_t len = out->length();
			int_t pos = r->position();
			if(parse(r, out)){
				return true;
			}
			r->set_position(pos);
			out->resize(len);
		}else{
			int_t pos = r->position();
			if(parse(r, out)){
				return true;
			}
			r->set_position(pos);
		}
		return false;
	}

	virtual void set_ref(const ParserPtr& p){}
};

class RefParser : public Parser{
	ParserPtr p_;
public:

	virtual bool parse(const ReaderPtr& r, const ArrayPtr& out){
		return p_->parse(r, out);
	}

	virtual void set_ref(const ParserPtr& p){
		p_ = p;
	}
};

class ChParser : public Parser{
	int ch_;
public:
	virtual bool parse(const ReaderPtr& r, const ArrayPtr& out){
		if(r->read()==ch_){
			if(out){
				out->push_back(ch_);
			}
			return true;
		}
		return false;
	}
};

class StringParser : public Parser{
	StringPtr str_;
public:
	StringParser(const StringPtr& str)
		:str_(str){}

	virtual bool parse(const ReaderPtr& r, const ArrayPtr& out){
		const char_t* str = str_->c_str();
		for(int_t i=0; i<str_->byte_size(); ++i){
			if(r->read()!=str[i]){
				return false;
			}
		}
		if(out){
			out->push_back(str_);
		}
		return true;
	}
};

class AnyChParser : public Parser{
public:
	virtual bool parse(const ReaderPtr& r, const ArrayPtr& out){
		int_t ret = r->read();
		if(ret<0)
			return false;
		char buf[2] = {ret, 0};
		if(out) out->push_back(xnew<String>(buf));
		return true;
	}
};

class RepeatParser : public Parser{
	ParserPtr p_;
	int_t n_;
public:
	RepeatParser(const ParserPtr& p, int_t n)
		:p_(p), n_(n){}

	virtual bool parse(const ReaderPtr& r, const ArrayPtr& out){
		if(n_<0){
			for(int_t i=0; i<-n_; ++i){
				if(!p_->try_parse(r, out)){
					return true;
				}
			}
		}else{
			for(int_t i=0; i<n_; ++i){
				if(!p_->parse(r, out)){
					return false;
				}
			}
			while(p_->try_parse(r, out)){}
		}
		return true;
	}
};

class SkipParser : public Parser{
	ParserPtr p_;
public:
	SkipParser(const ParserPtr& p)
		:p_(p){}

	virtual bool parse(const ReaderPtr& r, const ArrayPtr& out){
		return p_->parse(r, null);
	}
};

class SubParser : public Parser{
	ParserPtr lhs_, rhs_;
public:
	SubParser(const ParserPtr& l, const ParserPtr& r)
		:lhs_(l), rhs_(r){}

	virtual bool parse(const ReaderPtr& r, const ArrayPtr& out){
		int_t pos = r->position();
		if(rhs_->try_parse(r, null)){
			r->set_position(pos);
			return false;
		}

		return lhs_->parse(r, out);
	}
};

class OrParser : public Parser{
	ParserPtr lhs_, rhs_;
public:
	OrParser(const ParserPtr& l, const ParserPtr& r)
		:lhs_(l), rhs_(r){}

	virtual bool parse(const ReaderPtr& r, const ArrayPtr& out){
		return lhs_->try_parse(r, out) || rhs_->parse(r, out);
	}
};

class AndParser : public Parser{
	ParserPtr lhs_, rhs_;
public:	
	AndParser(const ParserPtr& l, const ParserPtr& r)
		:lhs_(l), rhs_(r){}

	virtual bool parse(const ReaderPtr& r, const ArrayPtr& out){
		return lhs_->parse(r, out) && rhs_->parse(r, out);
	}
};

class InvertParser : public Parser{
	ParserPtr p_;
public:
	InvertParser(const ParserPtr& p)
		:p_(p){}

	virtual bool parse(const ReaderPtr& r, const ArrayPtr& out){
		int_t len = out->length();
		int_t pos = r->position();
		bool ret = !parse(r, out);
		r->set_position(pos);
		out->resize(len);
		return ret;
	}
};

class EmptyParser : public Parser{
public:
	virtual bool parse(const ReaderPtr& r, const ArrayPtr& out){
		return true;
	}
};

class JoinParser : public Parser{
	ParserPtr p_;
	StringPtr sep_;
public:
	JoinParser(const ParserPtr& p, const StringPtr& sep = "")
		:p_(p), sep_(sep){}

	virtual bool parse(const ReaderPtr& r, const ArrayPtr& out){
		ArrayPtr ret = xnew<Array>();
		if(p_->parse(r, ret)){
			out->push_back(ret->join(sep_));
			return true;
		}
		return false;
	}
};

class ArrayParser : public Parser{
	ParserPtr p_;
public:
	ArrayParser(const ParserPtr& p)
		:p_(p){}

	virtual bool parse(const ReaderPtr& r, const ArrayPtr& out){
		if(out){
			int_t pos = out->size();
			if(p_->parse(r, out)){
				out->push_back(out->splice(pos, out->size()-pos));
				return true;
			}
			return false;
		}

		return p_->parse(r, out);
	}
};

class InsertValParser : public Parser{
	AnyPtr val_;
	int_t pos_;
public:
	InsertValParser(const AnyPtr& val, int_t pos)
		:val_(val), pos_(pos){}

	virtual bool parse(const ReaderPtr& r, const ArrayPtr& out){
		if(out) out->insert(out->size() - pos_, val_);
		return true;
	}
};

class SpliceParser : public Parser{
	int_t n_;
public:
	SpliceParser(int_t n)
		:n_(n){}

	virtual bool parse(const ReaderPtr& r, const ArrayPtr& out){
		if(out) out->push_back(out->splice(out->size() - n_, n_));
		return true;
	}
};


inline ParserPtr operator -(const ParserPtr& p){
	return ParserPtr(xnew<SkipParser>(p));
}

inline ParserPtr operator |(const ParserPtr& l, const ParserPtr& r){
	return ParserPtr(xnew<OrParser>(l, r));
}

inline ParserPtr str(const StringPtr& p){
	return ParserPtr(xnew<StringParser>(p));
}

inline ParserPtr operator >>(const ParserPtr& l, const ParserPtr& r){
	return ParserPtr(xnew<AndParser>(l, r));
}

inline ParserPtr operator ^(const ParserPtr& p, int_t n){
	return ParserPtr(xnew<RepeatParser>(p, n));
}

inline ParserPtr operator <<(const ParserPtr& l, const ParserPtr& r){
	return l >> -(str(" ")^0) >> r;
}

inline ParserPtr operator ~(const ParserPtr& p){
	return ParserPtr(xnew<InvertParser>(p));
}

inline ParserPtr operator -(const ParserPtr& l, const ParserPtr& r){
	return ParserPtr(xnew<SubParser>(l, r));
}


inline ParserPtr join(const ParserPtr& p, const StringPtr& sep = ""){
	return ParserPtr(xnew<JoinParser>(p, sep));
}

inline ParserPtr array(const ParserPtr& p){
	return ParserPtr(xnew<ArrayParser>(p));
}

inline ParserPtr insert_val(const AnyPtr& p, int_t pos = 0){
	return ParserPtr(xnew<InsertValParser>(p, pos));
}

inline ParserPtr splice(int_t n){
	return ParserPtr(xnew<SpliceParser>(n));
}

inline ParserPtr to_node(const AnyPtr& tag, int_t n){
	return insert_val(tag, n) >> splice(n+1);
}

class IntParser : public Parser{
public:
	virtual bool parse(const ReaderPtr& r, const ArrayPtr& out){
		if(!test_digit(r->peek())){
			return false;
		}
		int_t ret = 0;
		while(1){
			if(test_digit(r->peek())){
				ret *= 10;
				ret += r->read()-'0';
			}else if(r->peek()=='_'){
				r->read();
			}else{
				break;
			}
		}
		out->push_back(ret);
		return true;
	}
};
			
}


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
			using namespace grammer;
			ParserPtr anych = xnew<AnyChParser>();
			ParserPtr integer = xnew<IntParser>();

			const char* source = "145+ 1-0 * 500+  55/5";
			StreamPtr stream = xnew<MemoryStream>(source, strlen(source));
			ReaderPtr reader = xnew<Reader>();
			reader->set_stream(stream);
			
			ParserPtr term = integer >> to_node("INT", 1);

			ParserPtr expr_mul = term << ((
				(-str("*") << term >> to_node("MUL", 2)) | 
				(-str("/") << term >> to_node("DIV", 2)) 
				)^0);

			ParserPtr expr_add = expr_mul << ((
				(-str("+") << expr_mul >> to_node("ADD", 2)) | 
				(-str("-") << expr_mul >> to_node("SUB", 2)) 
				)^0);


			ArrayPtr ret = xnew<Array>();

			if(expr_add->parse(reader, ret)){
				ret->p();
				AnyPtr(calc_expr(ret[0]))->p();
			}

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
			ms: MemoryStream();
			ms.put_s("‚ a‚¢i‚¤u‚¦e‚¨o");
			ms.seek(0);
			ms.get_s(1).p;
			ms.get_s.each.to_a.p;
			ms.get_s.p;
			ms.get_s.p;
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
