#include "xtal.h"

#ifdef XTAL_USE_PEG

#include "xtal_peg.h"
	
namespace xtal{ 
	
namespace peg{

ParserPtr any;
ParserPtr eof;
ParserPtr alpha;
ParserPtr lalpha;
ParserPtr ualpha;
ParserPtr space;
ParserPtr digit;
ParserPtr success;
ParserPtr fail;
ParserPtr lineno;
ParserPtr ascii;

}

namespace{

void uninitialize_peg(){
	using namespace peg;

	any = null;
	eof = null;
	alpha = null;
	lalpha = null;
	ualpha = null;
	space = null;
	digit = null;
	success = null;
	fail = null;
	lineno = null;
	ascii = null;
}

}

void initialize_peg(){
	using namespace peg;

	register_uninitializer(&uninitialize_peg);

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


	any = xnew<Parser>(Parser::ANY);
	eof = xnew<Parser>(Parser::END);
	lalpha = Parser::ch_set("abcdefghijklmnopqrstuvwxyz");
	ualpha = Parser::ch_set("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
	alpha = Parser::select(ualpha, lalpha);
	space = Parser::ch_set(" \t\n\r");
	digit = Parser::ch_set("0123456789");
	success = xnew<Parser>(Parser::SUCCESS);
	fail = xnew<Parser>(Parser::FAIL);
	lineno = xnew<Parser>(Parser::LINENO);
	ascii = xnew<Parser>(Parser::ASCII);

	peg->def("any", any);
	peg->def("eof", eof);
	peg->def("alpha", alpha);
	peg->def("lalpha", lalpha);
	peg->def("ualpha", ualpha);
	peg->def("space", alpha);
	peg->def("digit", digit);
	peg->def("success", success);
	peg->def("fail", fail);
	peg->def("lineno", lineno);
	peg->def("ascii", ascii);
	peg->fun("str", &Parser::str);
	peg->fun("ch_set", &Parser::ch_set);
	peg->fun("join", &Parser::join)->param(null, Named("sep", ""));
	peg->fun("array", &Parser::array);
	peg->fun("val", &Parser::val);
	peg->fun("node", &Parser::node);
}

namespace peg{

Parser::Parser(int_t type, const AnyPtr& p1, const AnyPtr& p2)
	:type_(type), cacheable_(false), param1_(p1), param2_(p2){}

MapPtr Parser::make_ch_map2(const StringPtr& ch, const ParserPtr& pp){
	MapPtr data = xnew<Map>();
	data->set_at(ch, pp);
	return data;
}

MapPtr Parser::make_ch_map2(const MapPtr& ch_map, const ParserPtr& pp){
	MapPtr data = xnew<Map>();
	Xfor2(k, v, ch_map){
		data->set_at(k, pp);
	}
	return data;
}

MapPtr Parser::make_ch_map(const ParserPtr& p, const ParserPtr& pp){
	
	switch(p->type_){
		XTAL_DEFAULT{}
		XTAL_CASE(TRY_CH){ return make_ch_map2(ptr_cast<String>(p->param1_), pp); }
		XTAL_CASE(TRY_STRING){ return make_ch_map2(ptr_cast<String>(ptr_cast<Array>(p->param2_)->at(0)), pp); }
		XTAL_CASE(TRY_CH_SET){ return make_ch_map2(ptr_cast<Map>(p->param1_), pp); }
		XTAL_CASE(CH_MAP){ return make_ch_map2(ptr_cast<Map>(p->param1_), pp); }
		XTAL_CASE(FOLLOWED){ return make_ch_map(ptr_cast<Parser>(ptr_cast<Array>(p->param1_)->at(0)), pp); }
		XTAL_CASE(IGNORE){ return make_ch_map(ptr_cast<Parser>(p->param1_), pp); }
		XTAL_CASE(ARRAY){ return make_ch_map(ptr_cast<Parser>(p->param1_), pp); }
		XTAL_CASE(JOIN){ return make_ch_map(ptr_cast<Parser>(p->param1_), pp); }
	}

	return null;
}

ParserPtr Parser::str(const StringPtr& str){
	StringStreamPtr ss = xnew<StringStream>(str);
	ArrayPtr data = xnew<Array>();
	while(!ss->eof()){
		data->push_back(ss->get_s(1));
	}
	if(data->size()==0){
		return xnew<Parser>(SUCCESS);
	}
	if(data->size()==1){
		return xnew<Parser>(TRY_CH, data->at(0));
	}
	return xnew<Parser>(TRY_STRING, str, data);
}

ParserPtr Parser::ch_set(const StringPtr& str){
	MapPtr data = xnew<Map>();
	StringStreamPtr ss = xnew<StringStream>(str);
	while(!ss->eof()){
		StringPtr temp = ss->get_s(1);
		data->set_at(temp, temp);
	}

	return xnew<Parser>(TRY_CH_SET, data);
}

ParserPtr Parser::repeat(const AnyPtr& a, int_t n){
	ParserPtr p = P(a);
	if(n==0){
		return xnew<Parser>(REPEAT, try_(p));
	}

	if(n<0){
		p = select(p, success());
		ParserPtr pp = p;
		for(int_t i=1; i<-n; ++i){
			pp = followed(pp, p);
		}
		return pp;
	}

	ParserPtr pp = p;
	for(int_t i=1; i<n; ++i){
		pp = followed(pp, p);
	}
	return followed(pp, repeat(p, 0));
}

ParserPtr Parser::ignore(const AnyPtr& a){
	ParserPtr p = P(a);
	return xnew<Parser>(IGNORE, p);
}

ParserPtr Parser::select(const AnyPtr& a, const AnyPtr& b){
	ParserPtr lhs = P(a);
	ParserPtr rhs = P(b);

	if(lhs->type_==TRY_CH_SET && rhs->type_==TRY_CH_SET){
		return xnew<Parser>(TRY_CH_SET, ptr_cast<Map>(lhs->param1_)->cat(ptr_cast<Map>(rhs->param1_)));
	}

	/*
	if(MapPtr ml = make_ch_map(lhs, lhs)){
		if(MapPtr mr = make_ch_map(rhs, rhs)){
			if(ml->size()+mr->size()>2){
				MapPtr mm = ml->cat(mr);
				if(mm->size()==ml->size()+mr->size()){
					return xnew<Parser>(CH_MAP, mm);
				}
			}
		}
	}
	*/

	ArrayPtr data = xnew<Array>();
	if(lhs->type_==SELECT){
		data->cat_assign(static_ptr_cast<Array>(lhs->param1_));
	}else{
		data->push_back(lhs);
	}

	if(rhs->type_==SELECT){
		data->cat_assign(static_ptr_cast<Array>(rhs->param1_));
	}else{
		data->push_back(rhs);
	}

	for(int_t i=0; i<data->size()-1; ++i){
		data->set_at(i, try_(static_ptr_cast<Parser>(data->at(i))));
	}

	return xnew<Parser>(SELECT, data);
}

ParserPtr Parser::followed(const AnyPtr& a, const AnyPtr& b){
	ParserPtr lhs = P(a);
	ParserPtr rhs = P(b);

	if(lhs->type_==TRY_STRING && rhs->type_==TRY_STRING){
		return str(ptr_cast<String>(lhs->param1_)->cat(ptr_cast<String>(rhs->param1_)));
	}

	ArrayPtr data = xnew<Array>();
	if(lhs->type_==FOLLOWED){
		data->cat_assign(static_ptr_cast<Array>(lhs->param1_));
	}else{
		data->push_back(lhs);
	}

	if(rhs->type_==FOLLOWED){
		data->cat_assign(static_ptr_cast<Array>(rhs->param1_));
	}else{
		data->push_back(rhs);
	}

	return xnew<Parser>(FOLLOWED, data);
}

ParserPtr Parser::join(const AnyPtr& a){
	ParserPtr p = P(a);
	return xnew<Parser>(JOIN, p);
}

ParserPtr Parser::array(const AnyPtr& a){
	ParserPtr p = P(a);
	return xnew<Parser>(ARRAY, p);
}

ParserPtr Parser::try_(const AnyPtr& a){
	ParserPtr p = P(a);
	switch(p->type_){
		XTAL_DEFAULT{ return xnew<Parser>(TRY, p); }
		
		XTAL_CASE(TRY){ return p; }
		XTAL_CASE(TRY_CH){ return p; }
		XTAL_CASE(TRY_STRING){ return p; }
		XTAL_CASE(TRY_CH_SET){ return p; }
	}
}

ParserPtr Parser::ch_map(const MapPtr& data){
	return xnew<Parser>(CH_MAP, data);
}
	
ParserPtr Parser::node(const AnyPtr& tag, int_t n){
	return xnew<Parser>(NODE, tag, n);
}

ParserPtr Parser::val(const AnyPtr& v){
	return xnew<Parser>(VAL, v);
}
	
ParserPtr Parser::not(const AnyPtr& v){
	return xnew<Parser>(NOT, P(v));
}
	
ParserPtr Parser::test(const AnyPtr& v){
	return xnew<Parser>(TEST, P(v));
}

bool Parser::parse_string(const StringPtr& source, const ArrayPtr& ret){
	SmartPtr<CharLexer> lex = xnew<CharLexer>(xnew<StringStream>(source));
	lex->set_results(ret);
	return parse(lex);
}

void Parser::visit_members(Visitor& m){
	Base::visit_members(m);
	m & param1_ & param2_;
}

#define PARSER_RETURN(x) do{ success = x; goto end; }while(0)

bool Parser::parse(const LexerPtr& lex){

	bool success;
	Lexer::CacheInfo cache_info;

	if(cacheable_){
		if(lex->fetch_cache(this, success, cache_info)){
			return success;
		}
	}else{
		if(lex->judge_cache(this)){
			cacheable_ = true;
			if(lex->fetch_cache(this, success, cache_info)){
				return success;
			}
		}
	}

	switch(type_){
		XTAL_NODEFAULT;

		XTAL_CASE(TRY_STRING){
			const ArrayPtr& data = static_ptr_cast<Array>(param2_);
			for(uint_t i=0, sz=data->size(); i<sz; ++i){
				if(rawne(lex->peek(i), data->at(i))){
					PARSER_RETURN(false);
				}
			}
			lex->skip(data->size());
			lex->push_result(param1_);
			PARSER_RETURN(true);
		}

		XTAL_CASE(TRY_CH){
			if(raweq(lex->peek(), param1_)){
				lex->push_result(param1_);
				lex->skip(1);
				PARSER_RETURN(true);
			}
			PARSER_RETURN(false);
		}

		XTAL_CASE(TRY_CH_SET){
			const MapPtr& data = static_ptr_cast<Map>(param1_);
			const AnyPtr& s = lex->peek();
			if(data->at(s)){
				lex->push_result(s);
				lex->skip(1);
				PARSER_RETURN(true);
			}
			PARSER_RETURN(false);
		}

		XTAL_CASE(END){
			PARSER_RETURN(lex->eof());
		}

		XTAL_CASE(ANY){
			if(lex->eof()){
				PARSER_RETURN(false);
			}
			lex->push_result(lex->read());
			PARSER_RETURN(true);
		}

		XTAL_CASE(REPEAT){
			const ParserPtr& p = static_ptr_cast<Parser>(param1_);
			while(p->parse(lex)){}
			PARSER_RETURN(true);
		}

		XTAL_CASE(SELECT){
			const ArrayPtr& parsers = static_ptr_cast<Array>(param1_);
			for(uint_t i=0, sz=parsers->size()-1; i<sz; ++i){
				if(static_ptr_cast<Parser>(parsers->at(i))->parse(lex)){
					PARSER_RETURN(true);
				}
			}
			PARSER_RETURN(static_ptr_cast<Parser>(parsers->back())->parse(lex));
		}

		XTAL_CASE(FOLLOWED){
			const ArrayPtr& parsers = static_ptr_cast<Array>(param1_);
			for(uint_t i=0, sz=parsers->size()-1; i<sz; ++i){
				if(!static_ptr_cast<Parser>(parsers->at(i))->parse(lex)){
					PARSER_RETURN(false);
				}
			}
			PARSER_RETURN(static_ptr_cast<Parser>(parsers->back())->parse(lex));
		}

		XTAL_CASE(JOIN){
			const ParserPtr& p = static_ptr_cast<Parser>(param1_);
			lex->begin_join();				
			if(p->parse(lex)){
				lex->end_join(true);
				PARSER_RETURN(true);
			}
			lex->end_join(false);
			PARSER_RETURN(false);
		}

		XTAL_CASE(ARRAY){
			const ParserPtr& p = static_ptr_cast<Parser>(param1_);

			const ArrayPtr& temp = lex->results();
			uint_t size = temp->size();
			if(p->parse(lex)){
				lex->noreflect_cache(size);
				temp->push_back(temp->splice(size, temp->size()-size));
				PARSER_RETURN(true);
			}
			PARSER_RETURN(false);
		}

		XTAL_CASE(TRY){
			const ParserPtr& p = static_ptr_cast<Parser>(param1_);
			lex->mark();

			if(p->parse(lex)){
				lex->unmark();
				PARSER_RETURN(true);
			}

			lex->unmark_and_backtrack();
			PARSER_RETURN(false);
		}

		XTAL_CASE(IGNORE){
			const ParserPtr& p = static_ptr_cast<Parser>(param1_);
			lex->begin_ignore();
			bool ret = p->parse(lex);
			lex->end_ignore();
			PARSER_RETURN(ret);
		}

		XTAL_CASE(CH_MAP){
			const MapPtr& data = static_ptr_cast<Map>(param1_);
			if(const AnyPtr& a = data->at(lex->peek())){
				return static_ptr_cast<Parser>(a)->parse(lex);
			}
			PARSER_RETURN(false);
		}

		XTAL_CASE(NODE){
			lex->push_result(param1_);
			ArrayPtr results = lex->results();
			results->push_back(results->splice(results->size()-(param2_->to_i()+1), param2_->to_i()+1));
			PARSER_RETURN(true);
		}

		XTAL_CASE(VAL){
			lex->push_result(param1_);
			PARSER_RETURN(true);
		}

		XTAL_CASE(LINENO){
			lex->push_result(lex->lineno());
			PARSER_RETURN(true);
		}

		XTAL_CASE(FAIL){
			PARSER_RETURN(false);
		}

		XTAL_CASE(SUCCESS){
			PARSER_RETURN(true);
		}

		XTAL_CASE(NOT){
			const ParserPtr& p = static_ptr_cast<Parser>(param1_);
			lex->mark();
			bool ret = p->parse(lex);
			lex->unmark_and_backtrack();
			PARSER_RETURN(!ret);
		}

		XTAL_CASE(TEST){
			const ParserPtr& p = static_ptr_cast<Parser>(param1_);
			lex->mark();
			bool ret = p->parse(lex);
			lex->unmark_and_backtrack();
			PARSER_RETURN(ret);
		}

		XTAL_CASE(ASCII){
			if(lex->eof()){
				PARSER_RETURN(false);
			}

			StringPtr str = lex->peek()->to_s();
			if(str->length()==1 && str->c_str()[0]<128){
				lex->push_result(str);
				PARSER_RETURN(true);
			}
			PARSER_RETURN(false);
		}
	}

end:

	if(cacheable_){
		lex->store_cache(this, success, cache_info);
	}

	return success;
}

ParserPtr P(const AnyPtr& a){
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
