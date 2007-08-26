
#pragma once

#ifdef XTAL_USE_PEG // PEG上手くいかねー

#pragma once

#include "xtal_macro.h"

namespace xtal{ namespace peg{

/**
* @brief peg::Parserの読み取り元
*/
class Lexer : public Base{
private: 
	
	struct Cache{
		const void* ptr;
		bool success;
		uint_t flags;
		uint_t read_pos;
		uint_t read_end;
		uint_t value_pos;
		uint_t value_end;
	};

	struct Backtrack{
		uint_t read_pos;
		uint_t value_pos;
	};

	PODStack<Cache> cache_stack_;
	PODStack<Cache> temp_cache_stack_;
	PODStack<Backtrack> backtrack_stack_;
	PODStack<int_t> flags_stack_;
	
	enum{ CACHE_MAX = 64 };

	struct CacheUnit{

		CacheUnit(){
			key.ptr = 0;
		}

		struct Key{
			const void* ptr;
			uint_t read_pos;
			uint_t flags;
		};

		struct Value{
			ArrayPtr results;
			uint_t value_pos;
			uint_t value_end;
			uint_t read_pos;
			bool success;
		};

		Key key;
		Value value;
	};

	CacheUnit cache_table_[CACHE_MAX];

	enum{ CACHE_JUDGE_MAX = 16 };

	struct CacheJudgeUnit{
		CacheJudgeUnit(){
			ptr = 0;
		}

		const void* ptr;
		uint_t read_pos;
	};

	CacheJudgeUnit cache_judge_table_[CACHE_JUDGE_MAX];

	struct Comp{
		bool operator()(const Cache& a, uint_t b) const{ return a.value_pos < b; }
		bool operator()(uint_t a, const Cache& b) const{ return a < b.value_pos; }
		bool operator()(const Cache& a, const Cache& b) const{ return a.value_pos < b.value_pos; }
	};

public:

	enum{
		FLAG_NOCACHE_BIT = 1<<0,
		FLAG_IGNORE_BIT = 1<<1,
	};

public:

	Lexer(){
		buf_ = xnew<Array>(64);
		pos_ = 0;
		read_ = 0;
		flags_stack_.push(0);
	}

	/**
	* @brief 現在の位置をマークする
	*
	*/
	void mark(){
		Backtrack& data = backtrack_stack_.push();
		data.read_pos = pos_;
		data.value_pos = results_->size();
	}

	/**
	* @brief マークを消す
	*/
	void unmark(){
		Backtrack& data = backtrack_stack_.pop();
		noreflect_cache(data.value_pos);
	}

	/**
	* @brief マークを消してバックトラックする
	*/
	void unmark_and_backtrack(){
		Backtrack& data = backtrack_stack_.pop();
		pos_ = data.read_pos;
		reflect_cache(data.value_pos);
		results_->resize(data.value_pos);
	}

	void noreflect_cache(uint_t n){
		Cache* it = std::lower_bound(cache_stack_.begin(), cache_stack_.end(), n, Comp());
		Cache* end = cache_stack_.end();

		if(it==end)
			return;

		uint_t len = end - it;
		cache_stack_.resize(cache_stack_.size() - len);
	}

	void reflect_cache(uint_t n){
		Cache* beg = cache_stack_.begin();
		Cache* it = std::lower_bound(cache_stack_.begin(), cache_stack_.end(), n, Comp());
		Cache* end = cache_stack_.end();

		if(it==end)
			return;

		uint_t len = end - it;

		uint_t begin = n;
		ArrayPtr array = (ignore_flag() || results_->size()==begin) ? ArrayPtr(null) : results_->slice(begin, results_->size()-begin);

		for(; it!=end; it++){
			uint_t hash = (((uint_t)it->ptr)>>3) ^ it->read_pos ^ it->flags;
			CacheUnit& unit = cache_table_[hash & (CACHE_MAX-1)];

			unit.key.ptr = it->ptr;
			unit.key.read_pos = it->read_pos;
			unit.key.flags = it->flags;
			unit.value.success = it->success;
			unit.value.read_pos = it->read_end;
			unit.value.value_pos = it->value_pos-begin;
			unit.value.value_end = it->value_end-begin;
			unit.value.results = array;
		}

		cache_stack_.resize(cache_stack_.size() - len);
	}

	virtual int_t do_read(AnyPtr* buffer, int_t max) = 0;

	const AnyPtr& read(){
		uint_t bufsize = buf_->size();
		uint_t bufmask = buf_->size() - 1;
		uint_t rpos = read_&bufmask;

		// 読み込んでいない領域をreadしようとしているか？
		if(pos_ == read_){
			if(!backtrack_stack_.empty()){
				uint_t mpos = backtrack_stack_.reverse_at(0).read_pos&bufmask;

				if(rpos==mpos){
					// マーク中の領域を侵犯しようとしているので、リングバッファを倍に拡大
					buf_->resize(bufsize*2);
					bufsize = buf_->size();
					bufmask = buf_->size() - 1;
					rpos = read_&bufmask;
					mpos = backtrack_stack_.reverse_at(0).read_pos&bufmask;
				}

				if(rpos>mpos){
					read_ += do_read(buf_->data()+rpos, bufsize-rpos);
				}else{
					read_ += do_read(buf_->data()+rpos, mpos-rpos);
				}
			}else{
				read_ += do_read(buf_->data()+rpos, bufsize-rpos);
			}
		}

		if(pos_ == read_){
			return nop;
		}

		pos_ += 1;
		return buf_->at((pos_-1)&bufmask);
	}

	void seek(uint_t offset){
		if(read_ < offset){
			pos_ = read_;
		}else{
			pos_ = offset;
		}
	}

	StringPtr white_space(){
		return ws_set_;
	}

	void set_white_space(const StringPtr& ws){
		ws_set_ = ws;
	}

	void push_value(const AnyPtr& v){
		if(!ignore_flag()){
			results_->push_back(v);
		}
	}

	void push_flags(uint_t flags){
		flags_stack_.push(flags);
	}

	void pop_flags(){
		flags_stack_.pop();
	}

	uint_t flags(){
		return flags_stack_.top();
	}

	bool ignore_flag(){
		return (flags()&FLAG_IGNORE_BIT) != 0;
	}

	bool fetch_cache(const void* ptr, bool& success){
		if(flags()&FLAG_NOCACHE_BIT)
			return false;
		if(backtrack_stack_.empty())
			return false;

		uint_t hash = (((uint_t)ptr)>>3) ^ pos_ ^ flags();
		CacheUnit& unit = cache_table_[hash & (CACHE_MAX-1)];
		if(ptr==unit.key.ptr && pos_==unit.key.read_pos && flags()==unit.key.flags){
			success = unit.value.success;
			pos_ = unit.value.read_pos;
			if(!ignore_flag()){
				for(uint_t i=unit.value.value_pos, sz=unit.value.value_end; i<sz; ++i){
					results_->push_back(unit.value.results->at(i));
				}
			}
			return true;
		}
		
		Cache& data = temp_cache_stack_.push();
		data.ptr = ptr;
		data.read_pos = pos_;
		data.value_pos = results_->size();
		data.flags = flags();
		return false;
	}

	void store_cache(bool success){
		if(flags()&FLAG_NOCACHE_BIT)
			return;
		if(backtrack_stack_.empty())
			return;

		Cache& data = temp_cache_stack_.pop();
		data.success = success;
		data.read_end = pos_;
		data.value_end = results_->size();
		cache_stack_.push(data);
	}

	bool judge_cache(const void* ptr){
		uint_t hash = (((uint_t)ptr)>>3) ^ pos_;
		CacheJudgeUnit& unit = cache_judge_table_[hash & (CACHE_JUDGE_MAX-1)];
		if(ptr==unit.ptr && pos_==unit.read_pos){
			return true;
		}else{
			unit.ptr = ptr;
			unit.read_pos = pos_;
			return false;
		}
	}

	const ArrayPtr& results(){
		return results_;
	}

	void set_results(const ArrayPtr& ret){
		results_ = ret;
	}

private:

	virtual void visit_members(Visitor& m){
		Base::visit_members(m);
		m & ws_set_ & buf_;

		for(int_t i=0; i<CACHE_MAX; ++i){
			m & cache_table_[i].value.results;
		}
	}

private:

	ArrayPtr results_;
	ArrayPtr buf_;
	StringPtr ws_set_;
	uint_t pos_;
	uint_t read_;
};

class CharLexer : public Lexer{
public:

	CharLexer(const StreamPtr& stream)
		:stream_(stream){}

	virtual int_t do_read(AnyPtr* buffer, int_t max){
		for(int_t i=0; i<max; ++i){
			if(stream_->eof())
				return i;
			buffer[i] = stream_->get_s(1);
		}
		return max;
	}

private:
	StreamPtr stream_;
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
protected:
	Parser* p_;
	int_t cache_state_;
public:

	Parser(){
		p_ = 0;
		cache_state_ = 0;
	}

	virtual ~Parser(){
		if(p_){
			p_->dec_ref_count();
		}
	}

	bool parse_string(const StringPtr& source, const ArrayPtr& ret){
		SmartPtr<CharLexer> lex = xnew<CharLexer>(xnew<StringStream>(source));
		lex->set_results(ret);
		return parse(lex);
	}

	bool parse(const LexerPtr& lex){
		if(p_){
			return  p_->parse(lex);
		}else{
			if(cache_state_==1){
				bool success;
				if(lex->fetch_cache(this, success)){
					return success;
				}			
				success = do_parse(lex);
				lex->store_cache(success);
				return success;
			}else{
				// このパーサをキャッシュに乗せるべきか判定する
				if(cache_state_!=-1 && lex->judge_cache(this)){
					cache_state_ = 1;
					bool success;
					if(lex->fetch_cache(this, success)){
						return success;
					}			
					success = do_parse(lex);
					lex->store_cache(success);
					return success;
				}else{
					return do_parse(lex);
				}
			}
		}
	}
	
	bool try_parse(const LexerPtr& lex){
		lex->mark();

		if(parse(lex)){
			lex->unmark();
			return true;
		}

		lex->unmark_and_backtrack();
		return false;
	}

	void set(const ParserPtr& p){
		if(!p) return;

		if(p_){
			p_->set(p);
		}else{
			p_ = p.get();
			p_->inc_ref_count();
		}
	}

	virtual bool do_parse(const LexerPtr& lex){
		return true;
	}

	virtual void visit_members(Visitor& m){
		Base::visit_members(m);
		if(p_){
			m & ap(p_);
		}
	}
};

class StringParser : public Parser{
	StringPtr str_;
	ArrayPtr data_;
public:

	StringParser(const StringPtr& str)
		:str_(str){
		StringStreamPtr ss = xnew<StringStream>(str);
		data_ = xnew<Array>();
		while(!ss->eof()){
			data_->push_back(ss->get_s(1));
		}
	}

	virtual bool do_parse(const LexerPtr& lex){
		for(uint_t i=0, sz=data_->size(); i<sz; ++i){
			if(rawne(lex->read(), data_->at(i))){
				return false;
			}
		}

		lex->push_value(str_);
		return true;
	}
};

class EndParser : public Parser{
public:

	EndParser(){
		cache_state_ = -1;
	}

	virtual bool do_parse(const LexerPtr& lex){
		return raweq(lex->read(), nop);
	}
};

class SetParser : public Parser{
	MapPtr data_;
public:

	SetParser(const StringPtr& str){
		data_ = xnew<Map>();
		StringStreamPtr ss = xnew<StringStream>(str);
		while(!ss->eof()){
			data_->set_at(ss->get_s(1), true);
		}
	}

	virtual bool do_parse(const LexerPtr& lex){
		AnyPtr s = lex->read();
		if(raweq(s, nop)){
			return false;
		}

		if(data_->at(s)){
			lex->push_value(s);
			return true;
		}
		return false;
	}
};

class AnyChParser : public Parser{
public:

	AnyChParser(){
		cache_state_ = -1;
	}

	virtual bool do_parse(const LexerPtr& lex){
		AnyPtr ret = lex->read();
		if(raweq(ret, nop))
			return false;
		lex->push_value(ret);
		return true;
	}
};

class RepeatParser : public Parser{
	ParserPtr p_;
	int_t n_;
public:
	RepeatParser(const ParserPtr& p, int_t n)
		:p_(p), n_(n){}

	virtual bool do_parse(const LexerPtr& lex){
		if(n_<0){
			for(int_t i=0; i<-n_; ++i){
				if(!p_->try_parse(lex)){
					return true;
				}
			}
		}else{
			for(int_t i=0; i<n_; ++i){
				if(!p_->parse(lex)){
					return false;
				}
			}
			while(p_->try_parse(lex)){}
		}
		return true;
	}
};

class IgnoreParser : public Parser{
	ParserPtr p_;
public:
	IgnoreParser(const ParserPtr& p)
		:p_(p){}

	virtual bool do_parse(const LexerPtr& lex){
		lex->push_flags(lex->flags() | Lexer::FLAG_IGNORE_BIT);
		bool ret = p_->parse(lex);
		lex->pop_flags();
		return ret;
	}
};

class SubParser : public Parser{
	ParserPtr lhs_, rhs_;
public:
	SubParser(const ParserPtr& l, const ParserPtr& r)
		:lhs_(l), rhs_(r){}

	virtual bool do_parse(const LexerPtr& lex){
		lex->mark();

		if(rhs_->parse(lex)){
			lex->unmark_and_backtrack();
			return false;
		}
		lex->unmark_and_backtrack();

		return lhs_->parse(lex);
	}
};

class OrParser : public Parser{
	ArrayPtr parsers_;
public:
	
	OrParser(const ParserPtr& l, const ParserPtr& r){
		parsers_ = xnew<Array>();
		if(SmartPtr<OrParser> p = ptr_as<OrParser>(l)){
			parsers_->cat_assign(p->parsers_);
		}else{
			parsers_->push_back(l);
		}

		if(SmartPtr<OrParser> p = ptr_as<OrParser>(r)){
			parsers_->cat_assign(p->parsers_);
		}else{
			parsers_->push_back(r);
		}
	}

	virtual bool do_parse(const LexerPtr& lex){
		for(uint_t i=0, sz=parsers_->size()-1; i<sz; ++i){
			if(static_ptr_cast<Parser>(parsers_->at(i))->try_parse(lex)){
				return true;
			}
		}
		return static_ptr_cast<Parser>(parsers_->back())->parse(lex);
	}
};

class FollowedParser : public Parser{
	ArrayPtr parsers_;
public:	
	FollowedParser(const ParserPtr& l, const ParserPtr& r){
		parsers_ = xnew<Array>();
		if(SmartPtr<FollowedParser> p = ptr_as<FollowedParser>(l)){
			parsers_->cat_assign(p->parsers_);
		}else{
			parsers_->push_back(l);
		}

		if(SmartPtr<FollowedParser> p = ptr_as<FollowedParser>(r)){
			parsers_->cat_assign(p->parsers_);
		}else{
			parsers_->push_back(r);
		}
	}

	virtual bool do_parse(const LexerPtr& lex){
		for(uint_t i=0, sz=parsers_->size(); i<sz; ++i){
			if(!static_ptr_cast<Parser>(parsers_->at(i))->parse(lex)){
				return false;
			}
		}
		return true;
	}
};

class JoinParser : public Parser{
	ParserPtr p_;
	StringPtr sep_;
	MemoryStreamPtr mm_;
public:
	JoinParser(const ParserPtr& p, const StringPtr& sep = "")
		:p_(p), sep_(sep), mm_(xnew<MemoryStream>()){}

	virtual bool do_parse(const LexerPtr& lex){
		if(lex->ignore_flag())
			return p_->parse(lex);
		
		ArrayPtr temp = lex->results();
		uint_t size = temp->size();
		if(p_->parse(lex)){
			lex->noreflect_cache(size);
			mm_->clear();
			for(uint_t i=size, sz=temp->size(); i<sz; ++i){
				mm_->put_s(temp->at(i)->to_s());		
			}
			temp->erase(size, temp->size()-size);
			temp->push_back(mm_->to_s());
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

	virtual bool do_parse(const LexerPtr& lex){
		if(lex->ignore_flag())
			return p_->parse(lex);

		ArrayPtr temp = lex->results();
		uint_t size = temp->size();
		if(p_->parse(lex)){
			lex->noreflect_cache(size);
			temp->push_back(temp->splice(size, temp->size()-size));
			return true;
		}
		return false;
	}
};

class ParserParser : public Parser{
	AnyPtr fn_;
public:
	ParserParser(const AnyPtr& fn)
		:fn_(fn){}

	virtual bool do_parse(const LexerPtr& lex){
		return fn_(lex);
	}
};

class ValParser : public Parser{
	AnyPtr val_;
	int_t pos_;
public:
	ValParser(const AnyPtr& val, int_t pos)
		:val_(val), pos_(pos){
		cache_state_ = -1;
	}

	virtual bool do_parse(const LexerPtr& lex){
		lex->push_value(val_);
		return true;
	}
};

class SpliceParser : public Parser{
	int_t n_;
public:
	SpliceParser(int_t n)
		:n_(n){}

	virtual bool do_parse(const LexerPtr& lex){
		return true;
	}
};


ParserPtr to_parser(const AnyPtr& a);

inline ParserPtr parse(const AnyPtr& p){
	return ParserPtr(xnew<ParserParser>(p));
}

inline ParserPtr neg(const ParserPtr& p){
	return ParserPtr(xnew<IgnoreParser>(p));
}

inline ParserPtr operator -(const ParserPtr& p){
	return ParserPtr(xnew<IgnoreParser>(p));
}

inline ParserPtr operator |(const ParserPtr& l, const ParserPtr& r){
	return ParserPtr(xnew<OrParser>(l, r));
}

inline ParserPtr bitor(const ParserPtr& l, const ParserPtr& r){
	return ParserPtr(xnew<OrParser>(l, r));
}

inline ParserPtr str(const StringPtr& p){
	return ParserPtr(xnew<StringParser>(p));
}

inline ParserPtr anych(){
	return ParserPtr(xnew<AnyChParser>());
}

inline ParserPtr end(){
	return ParserPtr(xnew<EndParser>());
}

inline ParserPtr act(const ParserPtr& p, const AnyPtr& act){
	return p;
}

inline ParserPtr followed(const ParserPtr& l, const ParserPtr& r){
	return ParserPtr(xnew<FollowedParser>(l, r));
}

inline ParserPtr operator >>(const ParserPtr& l, const ParserPtr& r){
	return ParserPtr(xnew<FollowedParser>(l, r));
}

inline ParserPtr operator *(const ParserPtr& p, int_t n){
	return ParserPtr(xnew<RepeatParser>(p, n));
}

inline ParserPtr repeat(const ParserPtr& p, int_t n){
	return ParserPtr(xnew<RepeatParser>(p, n));
}

inline ParserPtr operator >(const ParserPtr& l, const ParserPtr& r){
	return l >> -(str(" ")*0) >> r;
}

inline ParserPtr operator ~(const ParserPtr& p){
	return p;
}

inline ParserPtr sub(const ParserPtr& l, const ParserPtr& r){
	return ParserPtr(xnew<SubParser>(l, r));
}

inline ParserPtr operator -(const ParserPtr& l, const ParserPtr& r){
	return ParserPtr(xnew<SubParser>(l, r));
}

inline ParserPtr set(const StringPtr& set){
	return ParserPtr(xnew<SetParser>(set));
}

inline ParserPtr nset(const StringPtr& set){
	return ParserPtr(xnew<AnyChParser>()) - ParserPtr(xnew<SetParser>(set));
}

inline ParserPtr join(const ParserPtr& p, const StringPtr& sep = ""){
	return ParserPtr(xnew<JoinParser>(p, sep));
}

inline ParserPtr array(const ParserPtr& p){
	return ParserPtr(xnew<ArrayParser>(p));
}

inline ParserPtr val(const AnyPtr& v, int_t pos = 0){
	return ParserPtr(xnew<ValParser>(v, pos));
}

inline ParserPtr splice(int_t n){
	return ParserPtr(xnew<SpliceParser>(n));
}

	
}}


#endif
