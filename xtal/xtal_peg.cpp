#include "xtal.h"

#ifdef XTAL_USE_PEG

#include "xtal_peg.h"

namespace xtal{ namespace peg{

void InitPEG(){
	
	ClassPtr peg =  xnew<Class>("peg");

	{
		ClassPtr p = new_cpp_class<Parser>("Parser");
		p->def("new", ctor<Parser>());
		p->method("parse_string", &Parser::parse_string);
		p->method("set", &Parser::set);
		p->method("op_shr", &followed);
		p->method("op_mul", &repeat);
		p->method("op_or", &bitor);
		p->method("op_neg", &neg);
		p->method("op_sub", &sub);
		p->method("op_at", &act);

		peg->def("Parser", p);

		{
			ClassPtr pp = new_cpp_class<OrParser>("OrParser");
			pp->inherit(p);
		}

		{
			ClassPtr pp = new_cpp_class<FollowedParser>("FollowedParser");
			pp->inherit(p);
		}

		set_cpp_class<AnyChParser>(p);
		set_cpp_class<StringParser>(p);
		set_cpp_class<RepeatParser>(p);
		set_cpp_class<JoinParser>(p);
		set_cpp_class<IgnoreParser>(p);
		set_cpp_class<SubParser>(p);
		set_cpp_class<SetParser>(p);
		set_cpp_class<ValParser>(p);
//		set_cpp_class<ActParser>(p);
		set_cpp_class<EndParser>(p);
		set_cpp_class<ArrayParser>(p);
	}

	{
		ClassPtr p = new_cpp_class<Lexer>("Lexer");

		peg->def("Lexer", p);
	}

	{
		ClassPtr p = new_cpp_class<CharLexer>("CharLexer");
		p->inherit(get_cpp_class<Lexer>());
		p->def("new", ctor<CharLexer, const StreamPtr&>());

		peg->def("CharLexer", p);
	}

	builtin()->def("peg", peg);

	peg->def("anych", anych());
	peg->def("end", end());
	peg->fun("str", &str);
	peg->fun("val", &val)->param(null, Named("pos", 0));
	peg->fun("set", &set);
	peg->fun("join", &join)->param(null, Named("sep", ""));
	peg->fun("array", &array);
}


ParserPtr to_parser(const AnyPtr& a){
	if(const StringPtr& ret = ptr_as<String>(a)){
		return str(ret);
	}	
	
	if(const ParserPtr& ret = ptr_as<Parser>(a)){
		return ret;
	}

	XTAL_THROW(cast_error(a, "Parser"), return null);
}


}}

#endif
