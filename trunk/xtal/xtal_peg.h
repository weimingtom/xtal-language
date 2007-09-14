
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
		uint_t lineno;
	};

	PODStack<Cache> cache_stack_;
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
	StringPtr newline_ch_;
	uint_t lineno_;
	ArrayPtr errors_;
	bool eof_;

public:

	Lexer(){
		buf_ = xnew<Array>(64);
		pos_ = 0;
		read_ = 0;

		mm_ = xnew<MemoryStream>();
		join_nest_ = 0;
		ignore_nest_ = 0;

		newline_ch_ = "\n";
		lineno_ = 1;
		eof_ = false;
	}

	/**
	* @brief 現在の位置をマークする
	*
	*/
	void mark(){
		Backtrack& data = backtrack_stack_.push();
		data.read_pos = pos_;
		data.value_pos = results_->size();
		data.lineno = lineno_;
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
		lineno_ = data.lineno;
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

	uint_t lineno(){
		return lineno_;
	}

	const AnyPtr& peek(uint_t n = 0){
		uint_t bufsize = buf_->size();
		uint_t bufmask = bufsize - 1;
		uint_t rpos = read_&bufmask;

		while(pos_+n >= read_){
			uint_t now_read = 0;
			if(!backtrack_stack_.empty()){
				uint_t mpos = backtrack_stack_.reverse_at(0).read_pos&bufmask;

				if(rpos<=mpos && ((rpos+n)&bufmask)>=mpos){
					// マーク中の領域を侵犯しようとしているので、リングバッファを倍に拡大
					buf_->resize(bufsize*2);
					bufsize = buf_->size();
					bufmask = bufsize - 1;
					rpos = read_&bufmask;
					mpos = backtrack_stack_.reverse_at(0).read_pos&bufmask;
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

	bool eat(const AnyPtr& value){
		const AnyPtr& ret = peek();
		if(raweq(ret, value)){
			read();
			return true;
		}
		return false;
	}

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

	void begin_join(){
		if(ignore_nest_)
			return;

		mark();
		join_nest_++;
	}

	void end_join(bool b){
		if(ignore_nest_)
			return;
		join_nest_--;

		if(join_nest_==0){
			Backtrack& data = backtrack_stack_.top();
			mm_->clear();
			for(int_t i=data.value_pos, sz=results_->size(); i<sz; ++i){
				mm_->put_s(results_->at(i)->to_s());
			}
			results_->erase(data.value_pos, results_->size()-data.value_pos);
			results_->push_back(mm_->to_s());
		}
		unmark();
	}

	void begin_ignore(){
		ignore_nest_++;
	}

	void end_ignore(){
		ignore_nest_--;
	}

	struct CacheInfo{
		uint_t read_pos;
		uint_t value_pos;
	};

	bool fetch_cache(const void* ptr, bool& success, CacheInfo& cache_info){
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
		
		cache_info.read_pos = pos_;
		cache_info.value_pos = results_->size();
		return false;
	}

	void store_cache(const void* ptr, bool success, CacheInfo& cache_info){
		if(backtrack_stack_.empty())
			return;

		Cache& data = cache_stack_.push();
		data.ptr = ptr;
		data.success = success;
		data.read_pos = cache_info.read_pos;
		data.read_end = pos_;
		data.value_pos = cache_info.value_pos;
		data.value_end = results_->size();
		cache_stack_.push(data);
	}

	bool judge_cache(const void* ptr){
		uint_t hash = (((uint_t)ptr)>>3) ^ pos_;
		CacheJudgeUnit& unit = cache_judge_table_[hash & (CACHE_JUDGE_MAX-1)];

		// 審判キャッシュに同じデータがあるのなら、キャッシュすべきだろう
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

protected:

	virtual void visit_members(Visitor& m){
		Base::visit_members(m);
		m & buf_;

		for(int_t i=0; i<CACHE_MAX; ++i){
			m & cache_table_[i].value.results;
		}
	}

private:

	ArrayPtr results_;
	ArrayPtr buf_;
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

	virtual void visit_members(Visitor& m){
		Lexer::visit_members(m);
		m & stream_;
	}
};


typedef SmartPtr<Lexer> LexerPtr;

class Parser;

// 演算子のために強いtypedefとして型定義
class ParserPtr : public SmartPtr<Parser>{
public:
	ParserPtr(const SmartPtr<Parser>& p = null)
		:SmartPtr<Parser>(p){}

	ParserPtr(const Null& p)
		:SmartPtr<Parser>(p){}
};

class Parser : public Base{
public:

	enum{
		STRING,
		TRY_STRING,
		CH,
		TRY_CH,
		CH_SET,
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
		LINE_NUMBER,
	};

public:

	Parser(int_t type = 0, const AnyPtr& p1 = null, const AnyPtr& p2 = null);

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

	bool parse_string(const StringPtr& source, const ArrayPtr& ret);

	bool parse(const LexerPtr& lex);

private:

	// 仮想関数ベースはやめた

	int_t type_;
	bool cacheable_;
	AnyPtr param1_;
	AnyPtr param2_;

	virtual void visit_members(Visitor& m);

};

extern ParserPtr success;
extern ParserPtr fail;
extern ParserPtr alpha;
extern ParserPtr ualpha;
extern ParserPtr lalpha;
extern ParserPtr space;
extern ParserPtr digit;
extern ParserPtr any;
extern ParserPtr eos;

ParserPtr to_parser(const AnyPtr& a);

}}


#endif
