
#pragma once

#include "xtal.h"

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

	MemoryStreamPtr mm_;
	int_t join_nest_;
	int_t ignore_nest_;

public:

	Lexer(){
		buf_ = xnew<Array>(64);
		pos_ = 0;
		read_ = 0;

		mm_ = xnew<MemoryStream>();
		join_nest_ = 0;
		ignore_nest_ = 0;
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
		ArrayPtr array = (ignore_nest_ || results_->size()==begin) ? ArrayPtr(null) : results_->slice(begin, results_->size()-begin);

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

	const AnyPtr& peek(){
		if(pos_ == read_){
			return read();
		}else{
			uint_t bufsize = buf_->size();
			uint_t bufmask = bufsize - 1;
			uint_t rpos = read_&bufmask;
			return buf_->at(rpos);
		}
	}

	const AnyPtr& read(){
		uint_t bufsize = buf_->size();
		uint_t bufmask = bufsize - 1;
		uint_t rpos = read_&bufmask;

		// 読み込んでいない領域をreadしようとしているか？
		if(pos_ == read_){
			if(!backtrack_stack_.empty()){
				uint_t mpos = backtrack_stack_.reverse_at(0).read_pos&bufmask;

				if(rpos+1==mpos){
					// マーク中の領域を侵犯しようとしているので、リングバッファを倍に拡大
					buf_->resize(bufsize*2);
					bufsize = buf_->size();
					bufmask = bufsize - 1;
					rpos = read_&bufmask;
					mpos = backtrack_stack_.reverse_at(0).read_pos&bufmask;
				}

				if(mpos>rpos){
					read_ += do_read(buf_->data()+rpos, mpos-rpos-1);
				}else{
					read_ += do_read(buf_->data()+rpos, bufsize-rpos);
				}
			}else{
				read_ += do_read(buf_->data()+rpos, bufsize-rpos);
			}

			if(pos_==read_){
				return nop;
			}
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
		if(!ignore_nest_){
			results_->push_back(v);
		}
	}

	void begin_join(){
		if(ignore_nest_)
			return;

		mark();
		join_nest_++;
	}

	void end_join(bool b){
		if(ignore_nest_)
			return;

		Backtrack& data = backtrack_stack_.top();
		mm_->clear();
		for(int_t i=data.value_pos, sz=results_->size(); i<sz; ++i){
			mm_->put_s(results_->at(i)->to_s());
		}
		results_->erase(data.value_pos, results_->size()-data.value_pos);
		join_nest_--;
		results_->push_back(mm_->to_s());
		unmark();
	}

	void begin_ignore(){
		ignore_nest_++;
	}

	void end_ignore(){
		ignore_nest_--;
	}


	bool fetch_cache(const void* ptr, bool& success){
		if(backtrack_stack_.empty())
			return false;

		uint_t flags = ignore_nest_!=0;
		uint_t hash = (((uint_t)ptr)>>3) ^ pos_ ^ flags;
		CacheUnit& unit = cache_table_[hash & (CACHE_MAX-1)];
		if(ptr==unit.key.ptr && pos_==unit.key.read_pos && flags==unit.key.flags){
			success = unit.value.success;
			pos_ = unit.value.read_pos;
			if(!ignore_nest_){
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
		data.flags = flags;
		return false;
	}

	void store_cache(bool success){
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

	enum{
		STRING,
		CH_SET,
		ALPHA,
		END,
		ANY,
		SELECT,
		FOLLOWED,
		REPEAT,
		JOIN,
		ARRAY,
		TRY,
		IGNORE,
	};

	int_t type_;
	AnyPtr param1_;
	AnyPtr param2_;
public:

	Parser(int_t type = 0, const AnyPtr& p1 = null, const AnyPtr& p2 = null)
		:type_(type), param1_(p1), param2_(p2){}

	static ParserPtr str(const StringPtr& str){
		StringStreamPtr ss = xnew<StringStream>(str);
		ArrayPtr data = xnew<Array>();
		while(!ss->eof()){
			data->push_back(ss->get_s(1));
		}

		return xnew<Parser>(STRING, str, data);
	}

	static ParserPtr end(){
		return xnew<Parser>(END);
	}

	static ParserPtr any(){
		return xnew<Parser>(ANY);
	}

	static ParserPtr alpha(){
		return xnew<Parser>(ALPHA);
	}

	static ParserPtr ch_set(const StringPtr& str){
		MapPtr data = xnew<Map>();
		StringStreamPtr ss = xnew<StringStream>(str);
		while(!ss->eof()){
			data->set_at(ss->get_s(1), true);
		}

		return xnew<Parser>(CH_SET, data);
	}

	static ParserPtr repeat(const ParserPtr& p, int_t n){
		return xnew<Parser>(REPEAT, p, n);
	}

	static ParserPtr ignore(const ParserPtr& p){
		return xnew<Parser>(IGNORE, p);
	}

	static ParserPtr select(const ParserPtr& lhs, const ParserPtr& rhs){
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

		return xnew<Parser>(SELECT, data);
	}

	static ParserPtr followed(const ParserPtr& lhs, const ParserPtr& rhs){
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

	static ParserPtr join(const ParserPtr& p){
		return xnew<Parser>(JOIN, p);
	}

	static ParserPtr array(const ParserPtr& p){
		return xnew<Parser>(ARRAY, p);
	}

	static ParserPtr try_(const ParserPtr& p){
		return xnew<Parser>(TRY, p);
	}

	bool parse_string(const StringPtr& source, const ArrayPtr& ret){
		SmartPtr<CharLexer> lex = xnew<CharLexer>(xnew<StringStream>(source));
		lex->set_results(ret);
		return parse(lex);
	}

#define PARSER_RETURN(x) do{ success = x; goto end; }while(0)

	bool parse(const LexerPtr& lex){

		bool success;

		switch(type_){
			XTAL_NODEFAULT;

			XTAL_CASE(STRING){
				const ArrayPtr& data = static_ptr_cast<Array>(param2_);
				for(uint_t i=0, sz=data->size(); i<sz; ++i){
					if(rawne(lex->read(), data->at(i))){
						PARSER_RETURN(false);
					}
				}
				lex->push_value(param1_);
				PARSER_RETURN(true);
			}

			XTAL_CASE(ALPHA){
				if(const StringPtr& ch = static_ptr_cast<String>(lex->read())){
					if(test_alpha(ch->c_str()[0])){
						lex->push_value(ch);
						PARSER_RETURN(true);
					}
				}
				PARSER_RETURN(false);
			}

			XTAL_CASE(CH_SET){
				const MapPtr& data = static_ptr_cast<Map>(param1_);
				const AnyPtr& s = lex->read();
				if(data->at(s)){
					lex->push_value(s);
					PARSER_RETURN(true);
				}
				PARSER_RETURN(false);
			}

			XTAL_CASE(END){
				PARSER_RETURN(raweq(lex->read(), nop));
			}

			XTAL_CASE(ANY){
				const AnyPtr& ret = lex->read();
				if(raweq(ret, nop)){
					PARSER_RETURN(false);
				}
				lex->push_value(ret);
				PARSER_RETURN(true);
			}

			XTAL_CASE(REPEAT){
				const ParserPtr& p = static_ptr_cast<Parser>(param1_);
				int_t n = ivalue(param2_);
				if(n<0){
					for(int_t i=0; i<-n; ++i){
						if(!p->try_parse(lex)){
							PARSER_RETURN(true);
						}
					}
				}else{
					for(int_t i=0; i<n; ++i){
						if(!p->parse(lex)){
							PARSER_RETURN(false);
						}
					}
					while(p->try_parse(lex)){}
				}
				PARSER_RETURN(true);
			}

			XTAL_CASE(SELECT){
				const ArrayPtr& parsers = static_ptr_cast<Array>(param1_);
				for(uint_t i=0, sz=parsers->size()-1; i<sz; ++i){
					if(static_ptr_cast<Parser>(parsers->at(i))->try_parse(lex)){
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
				lex->mark();

				if(parse(lex)){
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
		}

end:

		return success;
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

	virtual void visit_members(Visitor& m){
		Base::visit_members(m);
		m & param1_ & param2_;
	}
};

ParserPtr to_parser(const AnyPtr& a);

}}


#endif
