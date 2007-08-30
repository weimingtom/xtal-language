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
		p->method("op_shr", &Parser::followed);
		p->method("op_mul", &Parser::repeat);
		p->method("op_or", &Parser::select);
		p->method("op_neg", &Parser::ignore);

		peg->def("Parser", p);
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

	peg->def("any", Parser::any());
	peg->def("end", Parser::end());
	peg->def("alpha", Parser::alpha());
	peg->fun("str", &Parser::str);
	peg->fun("ch_set", &Parser::ch_set);
	peg->fun("join", &Parser::join)->param(null, Named("sep", ""));
	peg->fun("array", &Parser::array);
	//peg->fun("val", &val)->param(null, Named("pos", 0));
}


ParserPtr to_parser(const AnyPtr& a){
	if(const StringPtr& ret = ptr_as<String>(a)){
		return Parser::str(ret);
	}	
	
	if(const ParserPtr& ret = ptr_as<Parser>(a)){
		return ret;
	}

	XTAL_THROW(cast_error(a, "Parser"), return null);
}


}}

#endif
