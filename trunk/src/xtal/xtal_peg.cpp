#include "xtal.h"

#include "xtal_peg.h"
#include "xtal_macro.h"
	
namespace xtal{ 
	
namespace peg{

enum{
	PARTS_TEST_STRING,
	PARTS_TEST_CH,
	PARTS_TEST_CH_SET,
	PARTS_END,
	PARTS_ANY,
	PARTS_FAIL,
	PARTS_SUCCESS,
	PARTS_SELECT,
	PARTS_FOLLOWED,
	PARTS_MORE,
	PARTS_JOIN,
	PARTS_ARRAY,
	PARTS_IGNORE,
	PARTS_CH_MAP,
	PARTS_NODE,
	PARTS_VAL,
	PARTS_NOT,
	PARTS_AND,
	PARTS_TEST,
	PARTS_SUB,
	PARTS_ASCII,
	PARTS_LINENO,
	PARTS_ACT,
	PARTS_ACTMV,
};

class Parts : public Base{
public:

	Parts(int_t type = 0, const AnyPtr& p1 = null, const AnyPtr& p2 = null)
		:type(type), p1(p1), p2(p2){}

	int_t type;
	AnyPtr p1;
	AnyPtr p2;

	virtual void visit_members(Visitor& m){
		Base::visit_members(m);
		m & p1 & p2;
	}
};

typedef SmartPtr<Parts> PartsPtr;

AnyPtr any;
AnyPtr eof;
AnyPtr success;
AnyPtr fail;
AnyPtr lineno;
AnyPtr cn_any;
AnyPtr ch_alpha;
AnyPtr ch_lalpha;
AnyPtr ch_ualpha;
AnyPtr ch_space;
AnyPtr ch_digit;
AnyPtr ch_ascii;

}

namespace{

void uninitialize_peg(){
	using namespace peg;

	any = null;
	eof = null;
	success = null;
	fail = null;
	lineno = null;

	ch_alpha = null;
	ch_lalpha = null;
	ch_ualpha = null;
	ch_space = null;
	ch_digit = null;
	ch_ascii = null;

}

}

void initialize_peg(){
	using namespace peg;

	register_uninitializer(&uninitialize_peg);

	ClassPtr peg =  xnew<Class>("peg");

	{
		ClassPtr p = new_cpp_class<Parts>("Parts");
		p->def("new", ctor<Parts>());
		p->method("op_shr", &followed);
		p->method("op_mul", &more);
		p->method("op_or", &select);
		p->method("op_neg", &ignore);
		p->method("op_call", &act);

		peg->def("Parts", p);
	}

	{
		ClassPtr p = new_cpp_class<Scanner>("Scanner");
		p->method("results", &Scanner::results);
		peg->def("Scanner", p);
	}

	{
		ClassPtr p = new_cpp_class<CharScanner>("CharScanner");
		p->inherit(get_cpp_class<Scanner>());
		p->def("new", ctor<CharScanner, const StreamPtr&>());

		peg->def("CharScanner", p);
	}

	builtin()->def("peg", peg);


	any = xnew<Parts>(PARTS_ANY);
	eof = xnew<Parts>(PARTS_END);
	success = xnew<Parts>(PARTS_SUCCESS);
	fail = xnew<Parts>(PARTS_FAIL);
	lineno = xnew<Parts>(PARTS_LINENO);
	ch_lalpha = ch_set("abcdefghijklmnopqrstuvwxyz");
	ch_ualpha = ch_set("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
	ch_alpha = select(ch_ualpha, ch_lalpha);
	ch_space = ch_set(" \t\n\r");
	ch_digit = ch_set("0123456789");
	ch_ascii = xnew<Parts>(PARTS_ASCII);

	peg->fun("parse_string", parse_string);

	peg->def("any", any);
	peg->def("eof", eof);
	peg->def("success", success);
	peg->def("fail", fail);
	peg->def("lineno", lineno);
	peg->def("ch_alpha", ch_alpha);
	peg->def("ch_lalpha", ch_lalpha);
	peg->def("ch_ualpha", ch_ualpha);
	peg->def("ch_space", ch_alpha);
	peg->def("ch_digit", ch_digit);
	peg->def("ch_ascii", ch_ascii);
	peg->fun("str", &str);
	peg->fun("ch_set", &ch_set);
	peg->fun("join", &join)->param(null, Named("sep", ""));
	peg->fun("array", &array);
	peg->fun("val", &val);
	peg->fun("and", &and_);
	peg->fun("not", &not_);
}

namespace peg{

MapPtr make_ch_map2(const StringPtr& ch, const PartsPtr& pp){
	MapPtr data = xnew<Map>();
	data->set_at(ch, pp);
	return data;
}

MapPtr make_ch_map2(const MapPtr& map, const PartsPtr& pp){
	MapPtr data = xnew<Map>();
	Xfor2(k, v, map){
		data->set_at(k, pp);
	}
	return data;
}

MapPtr make_ch_map(const PartsPtr& p, const PartsPtr& pp){
	switch(p->type){
		XTAL_DEFAULT{}
		XTAL_CASE(PARTS_TEST_CH){ return make_ch_map2(ptr_cast<String>(p->p1), pp); }
		XTAL_CASE(PARTS_TEST_STRING){ return make_ch_map2(ptr_cast<String>(ptr_cast<Array>(p->p2)->at(0)), pp); }
		XTAL_CASE(PARTS_TEST_CH_SET){ return make_ch_map2(ptr_cast<Map>(p->p1), pp); }
		XTAL_CASE(PARTS_CH_MAP){ return make_ch_map2(ptr_cast<Map>(p->p1), pp); }
		XTAL_CASE(PARTS_FOLLOWED){ return make_ch_map(ptr_cast<Parts>(ptr_cast<Array>(p->p1)->at(0)), pp); }
		XTAL_CASE(PARTS_IGNORE){ return make_ch_map(ptr_cast<Parts>(p->p1), pp); }
		XTAL_CASE(PARTS_ARRAY){ return make_ch_map(ptr_cast<Parts>(p->p1), pp); }
		XTAL_CASE(PARTS_JOIN){ return make_ch_map(ptr_cast<Parts>(p->p1), pp); }
	}

	return null;
}

AnyPtr str(const StringPtr& str){
	StringStreamPtr ss = xnew<StringStream>(str);
	ArrayPtr data = xnew<Array>();
	while(!ss->eof()){
		data->push_back(ss->get_s(1));
	}

	if(data->size()==0){
		return xnew<Parts>(PARTS_SUCCESS);
	}

	if(data->size()==1){
		return xnew<Parts>(PARTS_TEST_CH, data->at(0));
	}

	return xnew<Parts>(PARTS_TEST_STRING, str, data);
}

AnyPtr ch_set(const StringPtr& str){
	MapPtr data = xnew<Map>();
	StringStreamPtr ss = xnew<StringStream>(str);
	while(!ss->eof()){
		StringPtr temp = ss->get_s(1);
		data->set_at(temp, temp);
	}

	return xnew<Parts>(PARTS_TEST_CH_SET, data);
}

AnyPtr more(const AnyPtr& a, int_t n){
	AnyPtr p = P(a);
	if(n==0){
		return xnew<Parts>(PARTS_MORE, test(p));
	}

	if(n<0){
		p = select(p, success);
		AnyPtr pp = p;
		for(int_t i=1; i<-n; ++i){
			pp = followed(pp, p);
		}
		return pp;
	}

	AnyPtr pp = p;
	for(int_t i=1; i<n; ++i){
		pp = followed(pp, p);
	}

	return followed(pp, more(p, 0));
}

AnyPtr ignore(const AnyPtr& a){
	return xnew<Parts>(PARTS_IGNORE, P(a));
}

AnyPtr select(const AnyPtr& a, const AnyPtr& b){
	PartsPtr lhs = static_ptr_cast<Parts>(P(a));
	PartsPtr rhs = static_ptr_cast<Parts>(P(b));

	if(lhs->type==PARTS_TEST_CH_SET && rhs->type==PARTS_TEST_CH_SET){
		return xnew<Parts>(PARTS_TEST_CH_SET, ptr_cast<Map>(lhs->p1)->cat(ptr_cast<Map>(rhs->p1)));
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
	if(lhs->type==PARTS_SELECT){
		data->cat_assign(static_ptr_cast<Array>(lhs->p1));
	}else{
		data->push_back(lhs);
	}

	if(rhs->type==PARTS_SELECT){
		data->cat_assign(static_ptr_cast<Array>(rhs->p1));
	}else{
		data->push_back(rhs);
	}

	for(int_t i=0; i<data->size()-1; ++i){
		data->set_at(i, test(static_ptr_cast<Parts>(data->at(i))));
	}

	return xnew<Parts>(PARTS_SELECT, data);
}

AnyPtr followed(const AnyPtr& a, const AnyPtr& b){
	PartsPtr lhs = static_ptr_cast<Parts>(P(a));
	PartsPtr rhs = static_ptr_cast<Parts>(P(b));

	if(lhs->type==PARTS_TEST_STRING && rhs->type==PARTS_TEST_STRING){
		return str(ptr_cast<String>(lhs->p1)->cat(ptr_cast<String>(rhs->p1)));
	}

	ArrayPtr data = xnew<Array>();
	if(lhs->type==PARTS_FOLLOWED){
		data->cat_assign(static_ptr_cast<Array>(lhs->p1));
	}else{
		data->push_back(lhs);
	}

	if(rhs->type==PARTS_FOLLOWED){
		data->cat_assign(static_ptr_cast<Array>(rhs->p1));
	}else{
		data->push_back(rhs);
	}

	return xnew<Parts>(PARTS_FOLLOWED, data);
}

AnyPtr sub(const AnyPtr& a, const AnyPtr& b){
	return xnew<Parts>(PARTS_SUB, P(a), ignore(b));
}

AnyPtr join(const AnyPtr& a){
	return xnew<Parts>(PARTS_JOIN, P(a));
}

AnyPtr array(const AnyPtr& a){
	return xnew<Parts>(PARTS_ARRAY, P(a));
}

AnyPtr test(const AnyPtr& a){
	PartsPtr p = static_ptr_cast<Parts>(P(a));
	switch(p->type){
		XTAL_DEFAULT{ return xnew<Parts>(PARTS_TEST, p); }
		
		XTAL_CASE(PARTS_TEST){ return p; }
		XTAL_CASE(PARTS_TEST_CH){ return p; }
		XTAL_CASE(PARTS_TEST_STRING){ return p; }
		XTAL_CASE(PARTS_TEST_CH_SET){ return p; }
	}
}

AnyPtr not_(const AnyPtr& v){
	return xnew<Parts>(PARTS_NOT, P(v));
}

AnyPtr and_(const AnyPtr& v){
	return xnew<Parts>(PARTS_AND, P(v));
}

AnyPtr ch_map(const MapPtr& data){
	return xnew<Parts>(PARTS_CH_MAP, data);
}

AnyPtr val(const AnyPtr& v){
	return xnew<Parts>(PARTS_VAL, v);
}

AnyPtr act(const AnyPtr& a, const AnyPtr& b){
	return xnew<Parts>(PARTS_ACT, P(a), b);
}

AnyPtr actmv(const AnyPtr& a, const AnyPtr& b){
	return xnew<Parts>(PARTS_ACTMV, P(a), b);
}

bool parse_inner(const AnyPtr& ps, const ScannerPtr& scanner){

	const PartsPtr& parts = static_ptr_cast<Parts>(ps);

	switch(parts->type){
		XTAL_NODEFAULT;

		XTAL_CASE(PARTS_TEST_STRING){
			const ArrayPtr& data = static_ptr_cast<Array>(parts->p2);
			for(uint_t i=0, sz=data->size(); i<sz; ++i){
				if(rawne(scanner->peek(i), data->at(i))){
					return false;
				}
			}
			scanner->skip(data->size());
			scanner->push_result(parts->p1);
			return true;
		}

		XTAL_CASE(PARTS_TEST_CH){
			if(raweq(scanner->peek(), parts->p1)){
				scanner->push_result(parts->p1);
				scanner->skip(1);
				return true;
			}
			return false;
		}

		XTAL_CASE(PARTS_TEST_CH_SET){
			const MapPtr& data = static_ptr_cast<Map>(parts->p1);
			const AnyPtr& s = scanner->peek();
			if(data->at(s)){
				scanner->push_result(s);
				scanner->skip(1);
				return true;
			}
			return false;
		}

		XTAL_CASE(PARTS_END){
			return scanner->eof();
		}

		XTAL_CASE(PARTS_ANY){
			if(scanner->eof()){
				return false;
			}
			scanner->push_result(scanner->read());
			return true;
		}

		XTAL_CASE(PARTS_MORE){
			const PartsPtr& p = static_ptr_cast<Parts>(parts->p1);
			while(parse_inner(p, scanner)){}
			return true;
		}

		XTAL_CASE(PARTS_SELECT){
			const ArrayPtr& p = static_ptr_cast<Array>(parts->p1);
			for(uint_t i=0, sz=p->size()-1; i<sz; ++i){
				if(parse_inner(p->at(i), scanner)){
					return true;
				}
			}
			return parse_inner(p->back(), scanner);
		}

		XTAL_CASE(PARTS_FOLLOWED){
			const ArrayPtr& p = static_ptr_cast<Array>(parts->p1);
			for(uint_t i=0, sz=p->size()-1; i<sz; ++i){
				if(!parse_inner(p->at(i), scanner)){
					return false;
				}
			}
			return parse_inner(p->back(), scanner);
		}

		XTAL_CASE(PARTS_JOIN){
			const PartsPtr& p = static_ptr_cast<Parts>(parts->p1);
			Scanner::Mark mark = scanner->begin_join();				
			if(parse_inner(p, scanner)){
				scanner->end_join(mark);
				return true;
			}
			scanner->end_join(mark, true);
			return false;
		}

		XTAL_CASE(PARTS_ARRAY){
			const PartsPtr& p = static_ptr_cast<Parts>(parts->p1);
			Scanner::Mark mark = scanner->begin_array();
			if(parse_inner(p, scanner)){
				scanner->end_array(mark);
				return true;
			}
			scanner->end_array(mark, true);
			return false;
		}

		XTAL_CASE(PARTS_TEST){
			const PartsPtr& p = static_ptr_cast<Parts>(parts->p1);
			Scanner::Mark mark = scanner->mark();

			if(parse_inner(p, scanner)){
				scanner->unmark(mark);
				return true;
			}

			scanner->unmark(mark);
			return false;
		}

		XTAL_CASE(PARTS_IGNORE){
			const PartsPtr& p = static_ptr_cast<Parts>(parts->p1);
			scanner->begin_ignore();
			bool ret = parse_inner(p, scanner);
			scanner->end_ignore();
			return ret;
		}

		XTAL_CASE(PARTS_CH_MAP){
			const MapPtr& data = static_ptr_cast<Map>(parts->p1);
			if(const AnyPtr& a = data->at(scanner->peek())){
				return parse_inner(a, scanner);
			}
			return false;
		}

		XTAL_CASE(PARTS_NODE){
			scanner->push_result(parts->p1);
			ArrayPtr results = scanner->results();
			results->push_back(results->splice(results->size()-(parts->p2->to_i()+1), parts->p2->to_i()+1));
			return true;
		}

		XTAL_CASE(PARTS_VAL){
			scanner->push_result(parts->p1);
			return true;
		}

		XTAL_CASE(PARTS_LINENO){
			scanner->push_result(scanner->lineno());
			return true;
		}

		XTAL_CASE(PARTS_FAIL){
			return false;
		}

		XTAL_CASE(PARTS_SUCCESS){
			return true;
		}

		XTAL_CASE(PARTS_NOT){
			const PartsPtr& p = static_ptr_cast<Parts>(parts->p1);
			Scanner::Mark mark = scanner->mark();
			bool ret = parse_inner(p, scanner);
			scanner->unmark(mark, true);
			return !ret;
		}

		XTAL_CASE(PARTS_AND){
			const PartsPtr& p = static_ptr_cast<Parts>(parts->p1);
			Scanner::Mark mark = scanner->mark();
			bool ret = parse_inner(p, scanner);
			scanner->unmark(mark, true);
			return ret;
		}

		XTAL_CASE(PARTS_SUB){
			if(parse_inner(parts->p2, scanner)){
				return false;
			}
			return parse_inner(parts->p1, scanner);
		}

		XTAL_CASE(PARTS_ASCII){
			if(scanner->eof()){
				return false;
			}

			StringPtr str = scanner->peek()->to_s();
			if(str->length()==1 && ((u8)str->c_str()[0])<128){
				scanner->push_result(str);
				return true;
			}
			return false;
		}

		XTAL_CASE(PARTS_ACT){
			Scanner::Mark mark = scanner->begin_act();
			bool ret = parse_inner(parts->p1, scanner);
			scanner->end_act(mark, !ret, parts->p2, false);
			return ret;
		}

		XTAL_CASE(PARTS_ACTMV){
			Scanner::Mark mark = scanner->begin_act();
			bool ret = parse_inner(parts->p1, scanner);
			scanner->end_act(mark, !ret, parts->p2, true);
			return ret;
		}
	}

	return false;
}

ScannerPtr parse_scanner(const AnyPtr& pattern, const ScannerPtr& scanner){
	scanner->set_success(parse_inner(P(pattern), scanner));
	return scanner;
}

AnyPtr P(const AnyPtr& a){
	if(const StringPtr& ret = ptr_as<String>(a)){
		return str(ret);
	}	
	
	if(const PartsPtr& ret = ptr_as<Parts>(a)){
		return ret;
	}

	XTAL_THROW(cast_error(a, "Parts"), return null);
}


}}

