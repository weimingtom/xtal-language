
#pragma once

#if 0

namespace xtal{ namespace peg{

	
class Lexer{
public:

	Lexer();

	void set_stream(const StreamPtr& stream){
		stream_ = stream;
	}

	/**
	* @brief 読み進める。
	*/
	int_t read();

	/**
	* @brief 次の要素を読む。
	*/
	int_t peek();

	/**
	* @brief 次の要素が引数chと同じだったら読み進める。
	* @param ch この値と次の要素が等しい場合に読み進める。
	* @retval true 次の要素はchと同じで、読み進めた。
	* @retval false 次の要素はchと異なり、読み進めなかった。
	*/
	bool eat(int_t ch);

	/**
	* @brief 要素を一つ戻す。
	*/
	void putback(int_t ch);
	
	/**
	* @brief ポジションの取得。
	*/
	int_t position();
	
	/**
	* @brief ポジションをposの位置に戻す。
	*/
	void set_position(int_t pos);

private:

	enum{ BUF_SIZE = 1024, BUF_MASK = BUF_SIZE-1 };

	StreamPtr stream_;
	StringPtr ws_set_;

	char buf_[BUF_SIZE];

	uint_t pos_;
	uint_t read_;
	uint_t marked_;
};


class Parser;

// 演算子のために強いtypedefとして型定義
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
		for(int_t i=0; i<str_->size(); ++i){
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
	
}}

#endif
