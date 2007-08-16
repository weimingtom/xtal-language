
#pragma once

#include "xtal_macro.h"

namespace xtal{ namespace peg{

	
class Lexer : public Stream{
public:

	Lexer(const StreamPtr& stream){
		stream_ = stream;
		buf_.resize(1024);

		pos_ = 0;
		read_ = 0;
		marked_pos_ = 0;
		marked_count_ = 0;
	}

	/**
	* @brief 現在の位置をマークする
	*
	* この位置まではseek可能なことが保障される
	*/
	uint_t mark(){
		if(marked_count_==0 || pos_<marked_pos_){
			marked_count_ = 1;
			marked_pos_ = pos_;
		}else if(marked_count_!=0 && pos_==marked_pos_){
			marked_count_++;
		}
		return pos_;
	}

	/**
	* @brief マークを消す
	*/
	void unmark(uint_t marked){
		if(marked_count_!=0){
			if(marked==marked_pos_){
				marked_count_--;
			}
		}
	}

	void seek_and_unmark(uint_t marked){
		seek(marked);
		unmark(marked);
	}

	virtual uint_t tell(){
		return pos_;
	}

	virtual uint_t write(const void* p, uint_t size){
		return 0;
	}

	virtual uint_t read(void* p, uint_t size){

		uint_t bufsize = buf_.size();
		uint_t bufmask = buf_.size() - 1;

		// 読み込もうとしているサイズがバッファのサイズより小さい場合は
		// 簡単のためにバッファを拡大する
		while(bufsize<size){
			buf_.resize(bufsize*2);
			bufsize = buf_.size();
			bufmask = buf_.size() - 1;		
		}

		// 読み込んでいない領域をreadしようとしている
		if(pos_ + size > read_){
			uint_t newreadsize = pos_ + size - read_;

			if(marked_count_!=0){
				// マーク中の領域を侵犯しようとしているので、リングバッファを倍倍で拡大
				while((read_&bufmask) < (marked_pos_&bufmask) && ((read_+newreadsize)&bufmask) > (marked_pos_&bufmask)){
					buf_.resize(bufsize*2);
					bufsize = buf_.size();
					bufmask = buf_.size() - 1;
				}
			}

			// 新しく読もうとしているところは、リングバッファの境目を跨いでいる
			if((read_&bufmask) + newreadsize > bufsize){
				uint_t nsize = (read_&bufmask) + newreadsize - bufsize;
				uint_t readsize = stream_->read(&buf_[read_&bufmask], nsize);
				readsize += stream_->read(&buf_[0], bufsize - nsize);
				read_ += readsize;
			}else{
				uint_t readsize = stream_->read(&buf_[read_&bufmask], newreadsize);
				read_ += readsize;
			}
		}
		
		if((pos_&bufmask) + size > bufsize){
			uint_t nsize = (pos_&bufmask) + size - bufsize;
			memcpy(p, &buf_[pos_&bufmask], nsize);
			memcpy((u8*)p + nsize, &buf_[0], bufsize - nsize);
		}else{
			memcpy(p, &buf_[pos_&bufmask], size);
		}
		pos_ += size;
		return size;
	}

	virtual void seek(int_t offset, int_t whence = XSEEK_SET){
		pos_ = offset;
	}

	virtual void close(){
		stream_->close();
	}

private:

	StreamPtr stream_;
	StringPtr ws_set_;

	AC<u8>::vector buf_;

	uint_t pos_;
	uint_t read_;
	uint_t marked_pos_;
	uint_t marked_count_;
};


typedef SmartPtr<Lexer> LexerPtr;

class Parser;

// 演算子のために強いtypedefとして型定義
class ParserPtr : public SmartPtr<Parser>{
public:
	ParserPtr(const SmartPtr<Parser>& p = null)
		:SmartPtr<Parser>(p){}
};


class Parser : public Base{
public:
	
	virtual bool parse(const LexerPtr& r, const ArrayPtr& out) = 0;
	
	bool try_parse(const LexerPtr& r, const ArrayPtr& out){
		if(out){
			uint_t len = out->length();
			uint_t pos = r->mark();
			if(parse(r, out)){
				r->unmark(pos);
				return true;
			}
			r->seek_and_unmark(pos);
			out->resize(len);
		}else{
			uint_t pos = r->mark();
			if(parse(r, out)){
				r->unmark(pos);
				return true;
			}
			r->seek_and_unmark(pos);
		}
		return false;
	}

	virtual void set_ref(const ParserPtr& p){}
};

class RefParser : public Parser{
	ParserPtr p_;
public:

	virtual bool parse(const LexerPtr& r, const ArrayPtr& out){
		return p_->parse(r, out);
	}

	virtual void set_ref(const ParserPtr& p){
		p_ = p;
	}
};

class StringParser : public Parser{
	StringPtr str_;
public:
	StringParser(const StringPtr& str)
		:str_(str){}

	virtual bool parse(const LexerPtr& r, const ArrayPtr& out){
		const char_t* str = str_->c_str();
		uint_t size = str_->buffer_size();

		for(uint_t i=0; i<size; ++i){
			if(r->get_u8()!=(u8)str[i]){
				return false;
			}
		}
		if(out){
			out->push_back(str_);
		}
		return true;
	}
};

class SetParser : public Parser{
	MapPtr set_;
public:

	SetParser(const StringPtr& str){
		set_ = xnew<Map>();
		Xfor(v, str->split("")){
			set_->set_at(v, true);
		}
	}

	virtual bool parse(const LexerPtr& r, const ArrayPtr& out){
		StringPtr s = r->get_s(1);
		if(set_->at(s)){
			if(out) out->push_back(s);
			return true;
		}
		return false;
	}
};

class AnyChParser : public Parser{
public:
	virtual bool parse(const LexerPtr& r, const ArrayPtr& out){
		StringPtr ret = r->get_s(1);
		if(ret->buffer_size()==0)
			return false;
		if(out) out->push_back(ret);
		return true;
	}
};

class RepeatParser : public Parser{
	ParserPtr p_;
	int_t n_;
public:
	RepeatParser(const ParserPtr& p, int_t n)
		:p_(p), n_(n){}

	virtual bool parse(const LexerPtr& r, const ArrayPtr& out){
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

	virtual bool parse(const LexerPtr& r, const ArrayPtr& out){
		return p_->parse(r, null);
	}
};

class SubParser : public Parser{
	ParserPtr lhs_, rhs_;
public:
	SubParser(const ParserPtr& l, const ParserPtr& r)
		:lhs_(l), rhs_(r){}

	virtual bool parse(const LexerPtr& r, const ArrayPtr& out){
		uint_t pos = r->mark();
		if(rhs_->try_parse(r, null)){
			r->seek_and_unmark(pos);
			return false;
		}
		r->unmark(pos);
		return lhs_->parse(r, out);
	}
};

class OrParser : public Parser{
	ParserPtr lhs_, rhs_;
public:
	OrParser(const ParserPtr& l, const ParserPtr& r)
		:lhs_(l), rhs_(r){}

	virtual bool parse(const LexerPtr& r, const ArrayPtr& out){
		return lhs_->try_parse(r, out) || rhs_->parse(r, out);
	}
};

class AndParser : public Parser{
	ParserPtr lhs_, rhs_;
public:	
	AndParser(const ParserPtr& l, const ParserPtr& r)
		:lhs_(l), rhs_(r){}

	virtual bool parse(const LexerPtr& r, const ArrayPtr& out){
		return lhs_->parse(r, out) && rhs_->parse(r, out);
	}
};

class InvertParser : public Parser{
	ParserPtr p_;
public:
	InvertParser(const ParserPtr& p)
		:p_(p){}

	virtual bool parse(const LexerPtr& r, const ArrayPtr& out){
		uint_t len = out->length();
		uint_t pos = r->mark();
		bool ret = !parse(r, out);
		r->seek_and_unmark(pos);
		out->resize(len);
		return ret;
	}
};

class EmptyParser : public Parser{
public:
	virtual bool parse(const LexerPtr& r, const ArrayPtr& out){
		return true;
	}
};

class JoinParser : public Parser{
	ParserPtr p_;
	StringPtr sep_;
public:
	JoinParser(const ParserPtr& p, const StringPtr& sep = "")
		:p_(p), sep_(sep){}

	virtual bool parse(const LexerPtr& r, const ArrayPtr& out){
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

	virtual bool parse(const LexerPtr& r, const ArrayPtr& out){
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

	virtual bool parse(const LexerPtr& r, const ArrayPtr& out){
		if(out) out->insert(out->size() - pos_, val_);
		return true;
	}
};

class SpliceParser : public Parser{
	int_t n_;
public:
	SpliceParser(int_t n)
		:n_(n){}

	virtual bool parse(const LexerPtr& r, const ArrayPtr& out){
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

inline ParserPtr operator *(const ParserPtr& p, int_t n){
	return ParserPtr(xnew<RepeatParser>(p, n));
}

inline ParserPtr operator <<(const ParserPtr& l, const ParserPtr& r){
	return l >> -(str(" ")*0) >> r;
}

inline ParserPtr operator ~(const ParserPtr& p){
	return ParserPtr(xnew<InvertParser>(p));
}

inline ParserPtr operator -(const ParserPtr& l, const ParserPtr& r){
	return ParserPtr(xnew<SubParser>(l, r));
}

inline ParserPtr set(const StringPtr& set){
	return ParserPtr(xnew<SetParser>(set));
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
	virtual bool parse(const LexerPtr& r, const ArrayPtr& out){
		uint_t pos = r->mark();
		if(!test_digit(r->get_u8())){
			r->seek_and_unmark(pos);
			return false;
		}
		r->seek_and_unmark(pos);

		int_t ret = 0;
		while(1){
			pos = r->mark();
			u8 ch = r->get_u8();
			if(test_digit(ch)){
				ret *= 10;
				ret += ch-'0';
			}else if(ch=='_'){
				
			}else{
				r->seek_and_unmark(pos);
				break;
			}
			r->unmark(pos);
		}
		out->push_back(ret);
		return true;
	}
};
	
}}

