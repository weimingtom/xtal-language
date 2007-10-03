#include "xtal.h"


#ifdef XTAL_USE_PEG

#include "xtal_peg.h"
	
namespace xtal{ 
	
namespace peg{

PartsPtr any;
PartsPtr eof;
PartsPtr alpha;
PartsPtr lalpha;
PartsPtr ualpha;
PartsPtr space;
PartsPtr digit;
PartsPtr success;
PartsPtr fail;
PartsPtr lineno;
PartsPtr ascii;

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
		ClassPtr p = new_cpp_class<Parts>("Parts");
		p->def("new", ctor<Parts>());
		p->method("parse_string", &Parts::parse_string);
		p->method("op_shr", &Parts::followed);
		p->method("op_mul", &Parts::repeat);
		p->method("op_or", &Parts::select);
		p->method("op_neg", &Parts::ignore);

		peg->def("Parts", p);
	}

	{
		ClassPtr p = new_cpp_class<Lexer>("Lexer");
		p->method("results", &Lexer::results);
		peg->def("Lexer", p);
	}

	{
		ClassPtr p = new_cpp_class<CharLexer>("CharLexer");
		p->inherit(get_cpp_class<Lexer>());
		p->def("new", ctor<CharLexer, const StreamPtr&>());

		peg->def("CharLexer", p);
	}

	builtin()->def("peg", peg);


	any = xnew<Parts>(Parts::ANY);
	eof = xnew<Parts>(Parts::END);
	lalpha = Parts::ch_set("abcdefghijklmnopqrstuvwxyz");
	ualpha = Parts::ch_set("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
	alpha = Parts::select(ualpha, lalpha);
	space = Parts::ch_set(" \t\n\r");
	digit = Parts::ch_set("0123456789");
	success = xnew<Parts>(Parts::SUCCESS);
	fail = xnew<Parts>(Parts::FAIL);
	lineno = xnew<Parts>(Parts::LINENO);
	ascii = xnew<Parts>(Parts::ASCII);

	peg->fun("parse_string", parse_string);

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
	peg->fun("str", &Parts::str);
	peg->fun("ch_set", &Parts::ch_set);
	peg->fun("join", &Parts::join)->param(null, Named("sep", ""));
	peg->fun("array", &Parts::array);
	peg->fun("val", &Parts::val);
	peg->fun("node", &Parts::node);
}

namespace peg{

Parts::Parts(Type type, const AnyPtr& p1, const AnyPtr& p2)
	:type_(type), param1_(p1), param2_(p2){}

MapPtr Parts::make_ch_map2(const StringPtr& ch, const PartsPtr& pp){
	MapPtr data = xnew<Map>();
	data->set_at(ch, pp);
	return data;
}

MapPtr Parts::make_ch_map2(const MapPtr& ch_map, const PartsPtr& pp){
	MapPtr data = xnew<Map>();
	Xfor2(k, v, ch_map){
		data->set_at(k, pp);
	}
	return data;
}

MapPtr Parts::make_ch_map(const PartsPtr& p, const PartsPtr& pp){
	
	switch(p->type_){
		XTAL_DEFAULT{}
		XTAL_CASE(TRY_CH){ return make_ch_map2(ptr_cast<String>(p->param1_), pp); }
		XTAL_CASE(TRY_STRING){ return make_ch_map2(ptr_cast<String>(ptr_cast<Array>(p->param2_)->at(0)), pp); }
		XTAL_CASE(TRY_CH_SET){ return make_ch_map2(ptr_cast<Map>(p->param1_), pp); }
		XTAL_CASE(CH_MAP){ return make_ch_map2(ptr_cast<Map>(p->param1_), pp); }
		XTAL_CASE(FOLLOWED){ return make_ch_map(ptr_cast<Parts>(ptr_cast<Array>(p->param1_)->at(0)), pp); }
		XTAL_CASE(IGNORE){ return make_ch_map(ptr_cast<Parts>(p->param1_), pp); }
		XTAL_CASE(ARRAY){ return make_ch_map(ptr_cast<Parts>(p->param1_), pp); }
		XTAL_CASE(JOIN){ return make_ch_map(ptr_cast<Parts>(p->param1_), pp); }
	}

	return null;
}

PartsPtr Parts::str(const StringPtr& str){
	StringStreamPtr ss = xnew<StringStream>(str);
	ArrayPtr data = xnew<Array>();
	while(!ss->eof()){
		data->push_back(ss->get_s(1));
	}
	if(data->size()==0){
		return xnew<Parts>(SUCCESS);
	}
	if(data->size()==1){
		return xnew<Parts>(TRY_CH, data->at(0));
	}
	return xnew<Parts>(TRY_STRING, str, data);
}

PartsPtr Parts::ch_set(const StringPtr& str){
	MapPtr data = xnew<Map>();
	StringStreamPtr ss = xnew<StringStream>(str);
	while(!ss->eof()){
		StringPtr temp = ss->get_s(1);
		data->set_at(temp, temp);
	}

	return xnew<Parts>(TRY_CH_SET, data);
}

PartsPtr Parts::repeat(const AnyPtr& a, int_t n){
	PartsPtr p = P(a);
	if(n==0){
		return xnew<Parts>(REPEAT, try_(p));
	}

	if(n<0){
		p = select(p, success);
		PartsPtr pp = p;
		for(int_t i=1; i<-n; ++i){
			pp = followed(pp, p);
		}
		return pp;
	}

	PartsPtr pp = p;
	for(int_t i=1; i<n; ++i){
		pp = followed(pp, p);
	}
	return followed(pp, repeat(p, 0));
}

PartsPtr Parts::ignore(const AnyPtr& a){
	PartsPtr p = P(a);
	return xnew<Parts>(IGNORE, p);
}

PartsPtr Parts::select(const AnyPtr& a, const AnyPtr& b){
	PartsPtr lhs = P(a);
	PartsPtr rhs = P(b);

	if(lhs->type_==TRY_CH_SET && rhs->type_==TRY_CH_SET){
		return xnew<Parts>(TRY_CH_SET, ptr_cast<Map>(lhs->param1_)->cat(ptr_cast<Map>(rhs->param1_)));
	}

	/*
	if(MapPtr ml = make_ch_map(lhs, lhs)){
		if(MapPtr mr = make_ch_map(rhs, rhs)){
			if(ml->size()+mr->size()>2){
				MapPtr mm = ml->cat(mr);
				if(mm->size()==ml->size()+mr->size()){
					return xnew<Parts>(CH_MAP, mm);
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
		data->set_at(i, try_(static_ptr_cast<Parts>(data->at(i))));
	}

	return xnew<Parts>(SELECT, data);
}

PartsPtr Parts::followed(const AnyPtr& a, const AnyPtr& b){
	PartsPtr lhs = P(a);
	PartsPtr rhs = P(b);

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

	return xnew<Parts>(FOLLOWED, data);
}

PartsPtr Parts::join(const AnyPtr& a){
	PartsPtr p = P(a);
	return xnew<Parts>(JOIN, p);
}

PartsPtr Parts::array(const AnyPtr& a){
	PartsPtr p = P(a);
	return xnew<Parts>(ARRAY, p);
}

PartsPtr Parts::try_(const AnyPtr& a){
	PartsPtr p = P(a);
	switch(p->type_){
		XTAL_DEFAULT{ return xnew<Parts>(TRY, p); }
		
		XTAL_CASE(TRY){ return p; }
		XTAL_CASE(TRY_CH){ return p; }
		XTAL_CASE(TRY_STRING){ return p; }
		XTAL_CASE(TRY_CH_SET){ return p; }
	}
}

PartsPtr Parts::ch_map(const MapPtr& data){
	return xnew<Parts>(CH_MAP, data);
}
	
PartsPtr Parts::node(const AnyPtr& tag, int_t n){
	return xnew<Parts>(NODE, tag, n);
}

PartsPtr Parts::val(const AnyPtr& v){
	return xnew<Parts>(VAL, v);
}
	
PartsPtr Parts::not_(const AnyPtr& v){
	return xnew<Parts>(NOT, P(v));
}
	
PartsPtr Parts::test(const AnyPtr& v){
	return xnew<Parts>(TEST, P(v));
}

bool Parts::parse_string(const StringPtr& source, const ArrayPtr& ret){
	SmartPtr<CharLexer> lex = xnew<CharLexer>(xnew<StringStream>(source));
	lex->set_results(ret);
	return parse(lex);
}

void Parts::visit_members(Visitor& m){
	Base::visit_members(m);
	m & param1_ & param2_;
}

#define Parts_RETURN(x) do{ success = x; goto end; }while(0)

bool Parts::parse(const LexerPtr& lex){

	bool success;

	switch(type_){
		XTAL_NODEFAULT;

		XTAL_CASE(TRY_STRING){
			const ArrayPtr& data = static_ptr_cast<Array>(param2_);
			for(uint_t i=0, sz=data->size(); i<sz; ++i){
				if(rawne(lex->peek(i), data->at(i))){
					Parts_RETURN(false);
				}
			}
			lex->skip(data->size());
			lex->push_result(param1_);
			Parts_RETURN(true);
		}

		XTAL_CASE(TRY_CH){
			if(raweq(lex->peek(), param1_)){
				lex->push_result(param1_);
				lex->skip(1);
				Parts_RETURN(true);
			}
			Parts_RETURN(false);
		}

		XTAL_CASE(TRY_CH_SET){
			const MapPtr& data = static_ptr_cast<Map>(param1_);
			const AnyPtr& s = lex->peek();
			if(data->at(s)){
				lex->push_result(s);
				lex->skip(1);
				Parts_RETURN(true);
			}
			Parts_RETURN(false);
		}

		XTAL_CASE(END){
			Parts_RETURN(lex->eof());
		}

		XTAL_CASE(ANY){
			if(lex->eof()){
				Parts_RETURN(false);
			}
			lex->push_result(lex->read());
			Parts_RETURN(true);
		}

		XTAL_CASE(REPEAT){
			const PartsPtr& p = static_ptr_cast<Parts>(param1_);
			while(p->parse(lex)){}
			Parts_RETURN(true);
		}

		XTAL_CASE(SELECT){
			const ArrayPtr& Partss = static_ptr_cast<Array>(param1_);
			for(uint_t i=0, sz=Partss->size()-1; i<sz; ++i){
				if(static_ptr_cast<Parts>(Partss->at(i))->parse(lex)){
					Parts_RETURN(true);
				}
			}
			Parts_RETURN(static_ptr_cast<Parts>(Partss->back())->parse(lex));
		}

		XTAL_CASE(FOLLOWED){
			const ArrayPtr& Partss = static_ptr_cast<Array>(param1_);
			for(uint_t i=0, sz=Partss->size()-1; i<sz; ++i){
				if(!static_ptr_cast<Parts>(Partss->at(i))->parse(lex)){
					Parts_RETURN(false);
				}
			}
			Parts_RETURN(static_ptr_cast<Parts>(Partss->back())->parse(lex));
		}

		XTAL_CASE(JOIN){
			const PartsPtr& p = static_ptr_cast<Parts>(param1_);
			Lexer::Mark mark = lex->begin_join();				
			if(p->parse(lex)){
				lex->end_join(mark);
				Parts_RETURN(true);
			}
			lex->end_join(mark, true);
			Parts_RETURN(false);
		}

		XTAL_CASE(ARRAY){
			const PartsPtr& p = static_ptr_cast<Parts>(param1_);
			Lexer::Mark mark = lex->begin_array();
			if(p->parse(lex)){
				lex->end_array(mark);
				Parts_RETURN(true);
			}
			lex->end_array(mark, true);
			Parts_RETURN(false);
		}

		XTAL_CASE(TRY){
			const PartsPtr& p = static_ptr_cast<Parts>(param1_);
			Lexer::Mark mark = lex->mark();

			if(p->parse(lex)){
				lex->unmark(mark);
				Parts_RETURN(true);
			}

			lex->unmark(mark);
			Parts_RETURN(false);
		}

		XTAL_CASE(IGNORE){
			const PartsPtr& p = static_ptr_cast<Parts>(param1_);
			lex->begin_ignore();
			bool ret = p->parse(lex);
			lex->end_ignore();
			Parts_RETURN(ret);
		}

		XTAL_CASE(CH_MAP){
			const MapPtr& data = static_ptr_cast<Map>(param1_);
			if(const AnyPtr& a = data->at(lex->peek())){
				return static_ptr_cast<Parts>(a)->parse(lex);
			}
			Parts_RETURN(false);
		}

		XTAL_CASE(NODE){
			lex->push_result(param1_);
			ArrayPtr results = lex->results();
			results->push_back(results->splice(results->size()-(param2_->to_i()+1), param2_->to_i()+1));
			Parts_RETURN(true);
		}

		XTAL_CASE(VAL){
			lex->push_result(param1_);
			Parts_RETURN(true);
		}

		XTAL_CASE(LINENO){
			lex->push_result(lex->lineno());
			Parts_RETURN(true);
		}

		XTAL_CASE(FAIL){
			Parts_RETURN(false);
		}

		XTAL_CASE(SUCCESS){
			Parts_RETURN(true);
		}

		XTAL_CASE(NOT){
			const PartsPtr& p = static_ptr_cast<Parts>(param1_);
			Lexer::Mark mark = lex->mark();
			bool ret = p->parse(lex);
			lex->unmark(mark, ret);
			Parts_RETURN(!ret);
		}

		XTAL_CASE(TEST){
			const PartsPtr& p = static_ptr_cast<Parts>(param1_);
			Lexer::Mark mark = lex->mark();
			bool ret = p->parse(lex);
			lex->unmark(mark, !ret);
			Parts_RETURN(ret);
		}

		XTAL_CASE(ASCII){
			if(lex->eof()){
				Parts_RETURN(false);
			}

			StringPtr str = lex->peek()->to_s();
			if(str->length()==1 && ((u8)str->c_str()[0])<128){
				lex->push_result(str);
				Parts_RETURN(true);
			}
			Parts_RETURN(false);
		}
	}

end:

	return success;
}

PartsPtr P(const AnyPtr& a){
	if(const StringPtr& ret = ptr_as<String>(a)){
		return Parts::str(ret);
	}	
	
	if(const PartsPtr& ret = ptr_as<Parts>(a)){
		return ret;
	}

	XTAL_THROW(cast_error(a, "Parts"), return null);
}


}}

#else

namespace xtal{
void initialize_peg(){}
}

#endif
