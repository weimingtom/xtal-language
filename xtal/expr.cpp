
#include "any.h"
#include "vmachine.h"
#include "frame.h"

namespace xtal{

/*
namespace par{

typedef UserData<xtal::Reader> Reader;

class Parser;

class ParserImpl : public AnyImpl{
public:
	
	virtual bool parse(const Reader& r, const Array& out) = 0;
	
	bool try_parse(const Reader& r, const Array& out){
		int_t len = out.length();
		int_t pos = r->position();
		if(parse(r, out)){
			return true;
		}
		r->set_position(pos);
		out.resize(len);
		return false;
	}
	
	virtual void set(const Parser& p){}
};

class Parser : public Any{
public:

	Parser();

	Parser(ParserImpl* p)
		:Any((AnyImpl*)p){}
		
	Parser(const Null&)
		:Any(null){}
		
	bool parse(const Reader& r, const Array& out);
	
	bool try_parse(const Reader& r, const Array& out);
	
	void set(const Parser& p);
	
	ParserImpl* impl() const{
		return (ParserImpl*)Any::impl();
	}
};

class RefParserImpl : public ParserImpl{
	Parser p_;
public:
	RefParserImpl(const Parser& p = null):p_(p){}

	virtual bool parse(const Reader& r, const Array& out){
		return p_.parse(r, out);
	}
			
	virtual void set(const Parser& p){
		p_ = p;
	}
};

Parser::Parser()
	:Any(new RefParserImpl()){}
		
void Parser::set(const Parser& p){
	return impl()->set(p);
}

class ChParserImpl : public ParserImpl{
	int ch_;
public:
	virtual bool parse(const Reader& r, const Array& out){
		if(r->read()==ch_){
			if(out){
				out.push_back(ch_);
			}
			return true;
		}
		return false;
	}
};

class StringParserImpl : public ParserImpl{
	String str_;
public:
	StringParserImpl(const String& str):str_(str){}
	virtual bool parse(const Reader& r, const Array& out){
		for(int_t i=0; i<str_.size(); ++i){
			if(r->read()!=str_.c_str()[i]){
				return false;
			}
		}
		if(out){
			out.push_back(str_);
		}
		return true;
	}
};

class ManyParserImpl : public ParserImpl{
	Parser p_;
public:
	ManyParserImpl(const Parser& p):p_(p){}
	virtual bool parse(const Reader& r, const Array& out){
		while(p_.try_parse(r, out)){}
		return true;
	}
};

class Many1ParserImpl : public ParserImpl{
	Parser p_;
public:
	Many1ParserImpl(const Parser& p):p_(p){}
	virtual bool parse(const Reader& r, const Array& out){
		if(!p_.parse(r, out)){
			while(p_.try_parse(r, out)){}
			return true;
		}
		return false;
	}
};

class SkipParserImpl : public ParserImpl{
	Parser p_;
public:
	SkipParserImpl(const Parser& p):p_(p){}
	virtual bool parse(const Reader& r, const Array& out){
		return p_.parse(r, null);
	}
};

class OrParserImpl : public ParserImpl{
	Parser lhs_, rhs_;
public:
	OrParserImpl(const Parser& l, const Parser& r):lhs_(l), rhs_(r){}
	virtual bool parse(const Reader& r, const Array& out){
		return lhs_.try_parse(r, out) || rhs_.parse(r, out);
	}
};

class AndParserImpl : public ParserImpl{
	Parser lhs_, rhs_;
public:	
	AndParserImpl(const Parser& l, const Parser& r):lhs_(l), rhs_(r){}
	virtual bool parse(const Reader& r, const Array& out){
		return lhs_.parse(r, out) && rhs_.parse(r, out);
	}
};

class InvertParserImpl : public ParserImpl{
	Parser p_;
public:
	InvertParserImpl(const Parser& p):p_(p){}
	virtual bool parse(const Reader& r, const Array& out){
		int_t len = out.length();
		int_t pos = r->position();
		bool ret = !parse(r, out);
		r->set_position(pos);
		out.resize(len);
		return ret;
	}
};

class EmptyParserImpl : public ParserImpl{
public:
	virtual bool parse(const Reader& r, const Array& out){
		return true;
	}
};

const Parser e(new EmptyParserImpl());


class JoinParserImpl : public ParserImpl{
	Parser p_;
	String sep_;
public:
	JoinParserImpl(const Parser& p, const String& sep = ""):p_(p), sep_(sep){}
	virtual bool parse(const Reader& r, const Array& out){
		Array ret;
		if(p_.parse(r, ret)){
			out.push_back(ret.join(sep_));
			return true;
		}
		return false;
	}
};

class ArrayParserImpl : public ParserImpl{
	Parser p_;
public:
	ArrayParserImpl(const Parser& p):p_(p){}
	virtual bool parse(const Reader& r, const Array& out){
		Array ret;
		if(p_.parse(r, ret)){
			out.push_back(ret);
			return true;
		}
		return false;
	}
};


class ValParserImpl : public ParserImpl{
	Any val_;
public:
	ValParserImpl(const Any& val):val_(val){}
	virtual bool parse(const Reader& r, const Array& out){
		out.push_back(val_);
		return true;
	}
};

bool Parser::parse(const Reader& r, const Array& out){
	return impl()->parse(r, out);
}

bool Parser::try_parse(const Reader& r, const Array& out){
	return impl()->try_parse(r, out);
}

inline Parser operator *(const Parser& p){
	return Parser(new ManyParserImpl(p));
}

inline Parser operator +(const Parser& p){
	return Parser(new Many1ParserImpl(p));
}

inline Parser operator -(const Parser& p){
	return Parser(new SkipParserImpl(p));
}

inline Parser operator |(const Parser& l, const Parser& r){
	return Parser(new OrParserImpl(l, r));
}

inline Parser operator >>(const Parser& l, const Parser& r){
	return Parser(new AndParserImpl(l, r));
}

inline Parser operator ~(const Parser& p){
	return Parser(new InvertParserImpl(p));
}

inline Parser operator !(const Parser& p){
	return p | e;
}

inline Parser join(const Parser& p, const String& sep = ""){
	return Parser(new JoinParserImpl(p, sep));
}

inline Parser str(const String& p){
	return Parser(new StringParserImpl(p));
}

inline Parser array(const Parser& p){
	return Parser(new ArrayParserImpl(p));
}

inline Parser val(const Any& p){
	return Parser(new ValParserImpl(p));
}

class IntParserImpl : public ParserImpl{
public:
	virtual bool parse(const Reader& r, const Array& out){
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
		out.push_back(ret);
	}
};

const Parser int_(new IntParserImpl());


Parser end = str(";") | str("\a");
Parser if_ = array(-ident("if") >> val(IF_STMT) >> line >> -str("(") >> expr >> -str(")") >> stmt);
Parser multi_expr = array(*(expr >> -!str(",")));
Parser return_ = array(-ident("return") >> val(RETURN_STMT) >> line >> multi_expr >> -end);
Parser fun_expr = array(-ident(fun) >> str("(") >> args >> str(")") >> str("{") >> multi_stmt >> str("}"));
Parser multi_stmt = array(*stmt);
Parser args = *(expr >> -!str(",")) >> *(key >> str(":") >> expr >> -!str(","))

}
*/

}
