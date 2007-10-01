
#pragma once

#include "xtal.h"

#ifdef XTAL_USE_PEG // PEG上手くいかねー

#pragma once

#include "xtal_macro.h"

namespace xtal{ namespace peg{


class Match : public Base{
public:
	AnyPtr results();
	bool success();
	bool full();
};

/**
* @brief peg::Parserの読み取り元
*/
class Lexer : public Base{
public:

	struct Mark{
		uint_t read_pos;
		uint_t value_pos;
		uint_t lineno;
	};

	Lexer(){
		buf_ = xnew<Array>(64);
		pos_ = 0;
		read_ = 0;
		marked_ = ~0;

		mm_ = xnew<MemoryStream>();
		join_nest_ = 0;
		ignore_nest_ = 0;

		newline_ch_ = "\n";
		lineno_ = 1;

		results_ = xnew<Array>();
	}

	/**
	* @brief 現在の位置をマークする
	*
	*/
	Mark mark(){
		Mark mark;
		mark.read_pos = pos_;
		mark.value_pos = results_->size();
		mark.lineno = lineno_;
		return mark;
	}

	/**
	* @brief マークを消す
	*/
	void unmark(const Mark& mark, bool fail = false){
		if(mark.read_pos==marked_){
			marked_ = ~0;
		}

		if(fail){
			pos_ = mark.read_pos;
			results_->resize(mark.value_pos);
			lineno_ = mark.lineno;
		}
	}

	virtual int_t do_read(AnyPtr* buffer, int_t max) = 0;

	uint_t lineno(){
		return lineno_;
	}

	void error(const AnyPtr& text){

	}

	AnyPtr errors(){
		return errors_->each();
	}

	const AnyPtr& peek(uint_t n = 0){
		uint_t bufsize = buf_->size();
		uint_t bufmask = bufsize - 1;
		uint_t rpos = read_&bufmask;

		while(pos_+n >= read_){
			uint_t now_read = 0;
			if(marked_ != ~0){
				uint_t mpos = marked_&bufmask;

				if(rpos<=mpos && ((rpos+n)&bufmask)>mpos){
					// マーク中の領域を侵犯しようとしているので、リングバッファを倍に拡大
					buf_->resize(bufsize*2);
					bufsize = buf_->size();
					bufmask = bufsize - 1;
					rpos = read_&bufmask;
					mpos = marked_&bufmask;
				}

				if(mpos>rpos){
					now_read = do_read(buf_->data()+rpos, mpos-rpos);
				}else{
					now_read = do_read(buf_->data()+rpos, bufsize-rpos);
				}
			}else{
				now_read = do_read(buf_->data()+rpos, bufsize-rpos);
			}

			if(now_read==0){
				return null;
			}

			read_ += now_read;
		}
		
		return buf_->at((pos_+n)&bufmask);
	}

	const AnyPtr& read(){
		const AnyPtr& ret = peek();
		if(raweq(ret, newline_ch_)){
			lineno_++;
		}
		pos_ += 1;
		return  ret;
	}

	/*
	void putback(const AnyPtr& v){
		uint_t bufsize = buf_->size();
		uint_t bufmask = bufsize - 1;
		pos_--;
		buf_[pos_ & bufmask] = v;
	}
	*/

	bool eat(const AnyPtr& value){
		const AnyPtr& ret = peek();
		if(raweq(ret, value)){
			read();
			return true;
		}
		return false;
	}

	/**
	* @brief 一つ先読みし、終了しているか調べる
	*/
	bool eof(){
		peek(1);
		return pos_==read_;
	}

	void skip(uint_t n){
		for(uint_t i=0; i<n; ++i){
			read();
		}
	}

	void seek(uint_t offset){
		if(read_ < offset){
			pos_ = read_;
		}else{
			pos_ = offset;
		}
	}

	void push_result(const AnyPtr& v){
		if(!ignore_nest_){
			results_->push_back(v);
		}
	}

	AnyPtr pop_result(){
		AnyPtr temp = results_->back();
		results_->pop_back();
		return temp;
	}

	Mark begin_join(){
		join_nest_++;
		return mark();
	}

	void end_join(const Mark& mark, bool fail = false){
		join_nest_--;
		int_t diff = results_->size() - mark.value_pos;
		if(join_nest_==0 && ignore_nest_==0 && diff>0){
			mm_->clear();
			if(int_t size = results_->size()){
				for(int_t i=mark.value_pos; i<size; ++i){
					mm_->put_s(results_->at(i)->to_s());
				}
				results_->erase(mark.value_pos, diff);
			}
			results_->push_back(mm_->to_s());
		}
		unmark(mark, fail);
	}

	Mark begin_array(){
		return mark();
	}

	void end_array(const Mark& mark, bool fail = false){
		int_t diff = results_->size() - mark.value_pos;
		if(diff>=0){
			results_->push_back(results_->splice(mark.value_pos, diff));
		}
		unmark(mark, fail);
	}

	void begin_ignore(){
		ignore_nest_++;
	}

	void end_ignore(){
		ignore_nest_--;
	}

	const ArrayPtr& results(){
		return results_;
	}

	void set_results(const ArrayPtr& ret){
		results_ = ret;
	}

protected:

	virtual void visit_members(Visitor& m){
		Base::visit_members(m);
		m & buf_;
	}

private:
	
	MemoryStreamPtr mm_;
	int_t join_nest_;
	int_t ignore_nest_;
	StringPtr newline_ch_;
	ArrayPtr errors_;
	ArrayPtr results_;
	ArrayPtr buf_;
	uint_t pos_;
	uint_t read_;
	uint_t lineno_;
	uint_t marked_;
};

class CharLexer : public Lexer{
public:

	CharLexer(const StreamPtr& stream)
		:stream_(stream){}

	virtual int_t do_read(AnyPtr* buffer, int_t max){
		max = 1;
		for(int_t i=0; i<max; ++i){
			if(stream_->eof())
				return i;
			buffer[i] = stream_->get_s(1);
		}
		return max;
	}

private:
	StreamPtr stream_;

	virtual void visit_members(Visitor& m){
		Lexer::visit_members(m);
		m & stream_;
	}
};


typedef SmartPtr<Lexer> LexerPtr;

class Parser;
typedef SmartPtr<Parser> ParserPtr;

class Parser : public Base{
public:

	enum Type{
		TRY_STRING,
		TRY_CH,
		TRY_CH_SET,
		END,
		ANY,
		FAIL,
		SUCCESS,
		SELECT,
		FOLLOWED,
		REPEAT,
		JOIN,
		ARRAY,
		IGNORE,
		CH_MAP,
		NODE,
		TRY,
		VAL,
		NOT,
		TEST,
		ASCII,
		LINENO,
		ACT,
	};

public:

	Parser(Type type = ANY, const AnyPtr& p1 = null, const AnyPtr& p2 = null);

	static MapPtr make_ch_map2(const StringPtr& ch, const ParserPtr& pp);

	static MapPtr make_ch_map2(const MapPtr& ch_map, const ParserPtr& pp);

	static MapPtr make_ch_map(const ParserPtr& p, const ParserPtr& pp);

	static ParserPtr str(const StringPtr& str);

	static ParserPtr ch_set(const StringPtr& str);

	static ParserPtr repeat(const AnyPtr& p, int_t n);

	static ParserPtr ignore(const AnyPtr& p);

	static ParserPtr select(const AnyPtr& lhs, const AnyPtr& rhs);

	static ParserPtr followed(const AnyPtr& lhs, const AnyPtr& rhs);

	static ParserPtr join(const AnyPtr& p);

	static ParserPtr array(const AnyPtr& p);

	static ParserPtr try_(const AnyPtr& p);
	
	static ParserPtr ch_map(const MapPtr& data);

	static ParserPtr node(const AnyPtr& tag, int_t n);

	static ParserPtr error(const AnyPtr& message);

	static ParserPtr val(const AnyPtr& v);

	static ParserPtr not_(const AnyPtr& v);
	
	static ParserPtr test(const AnyPtr& v);

	bool parse_string(const StringPtr& source, const ArrayPtr& ret);

	bool parse(const LexerPtr& lex);

	static ParserPtr act(const AnyPtr& fn);

private:

	// 仮想関数ベースはやめた

	int_t type_;
	AnyPtr param1_;
	AnyPtr param2_;

	virtual void visit_members(Visitor& m);

};

extern ParserPtr success;
extern ParserPtr fail;
extern ParserPtr eof;
extern ParserPtr ch_alpha;
extern ParserPtr ch_ualpha;
extern ParserPtr ch_lalpha;
extern ParserPtr ch_space;
extern ParserPtr ch_digit;
extern ParserPtr ch_any;
extern ParserPtr ch_ascii;

inline ParserPtr operator |(const AnyPtr& a, const AnyPtr& b){ return Parser::select(a, b); }
inline ParserPtr operator >>(const AnyPtr& a, const AnyPtr& b){ return Parser::followed(a, b); }
inline ParserPtr operator -(const AnyPtr& a){ return Parser::ignore(a); }
inline ParserPtr operator *(const AnyPtr& a, int_t n){ return Parser::repeat(a, n); }
inline ParserPtr join(const AnyPtr& a){ return Parser::join(a); }
inline ParserPtr val(const AnyPtr& a){ return Parser::val(a); }
inline ParserPtr ch_set(const StringPtr& a){ return Parser::ch_set(a); }
inline ParserPtr not_(const AnyPtr& a){ return Parser::not_(a); }
inline ParserPtr test(const AnyPtr& a){ return Parser::test(a); }

ParserPtr P(const AnyPtr& a);

inline LexerPtr parse_lexer(const AnyPtr& pattern, const LexerPtr& lex){
	P(pattern)->parse(lex);
	return lex;
}

inline LexerPtr parse_string(const AnyPtr& pattern, const AnyPtr& string){
	return parse_lexer(pattern, xnew<CharLexer>(xnew<StringStream>(string->to_s())));
}


}}


#endif
