
#pragma once

#include "xtal.h"

namespace xtal{ namespace peg{

/**
* @brief 
*/
class Scanner : public Base{
public:

	struct Mark{
		uint_t read_pos;
		uint_t value_pos;
		uint_t lineno;
	};

	Scanner(){
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
		success_ = true;
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

		if(~0 == marked_){
			marked_ = pos_;
		}

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

	void seek(uint_t pos){
		if(pos_ < pos){
			peek(pos - pos_);
		}
		pos_ = pos;
	}

	uint_t pos(){
		return pos_;
	}

	uint_t lineno(){
		return lineno_;
	}

	void error(const AnyPtr& text){

	}

	AnyPtr errors(){
		return errors_->each();
	}

	/**
	* @brief n個先の要素を覗き見る
	*/
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
			rpos = read_&bufmask;
		}
		
		return buf_->at((pos_+n)&bufmask);
	}

	/**
	* @brief ひとつ読み取る
	*/
	const AnyPtr& read(){
		const AnyPtr& ret = peek();
		if(raweq(ret, newline_ch_)){
			lineno_++;
		}
		pos_ += 1;
		return  ret;
	}

	/**
	* @brief valueと等しい場合に読み飛ばす
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

	/**
	* @brief n文字読み飛ばす
	*/
	void skip(uint_t n){
		for(uint_t i=0; i<n; ++i){
			read();
		}
	}

public:

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

public:

	Mark begin_record(){
		return mark();
	}

	StringPtr end_record(const Mark& mark){
		uint_t mask = buf_->size()-1;
		mm_->clear();
		for(uint_t i=mark.read_pos; i<pos_; ++i){
			mm_->put_s(buf_->at(i & mask)->to_s());
		}
		unmark(mark);
		return mm_->to_s();
	}

public:

	Mark begin_join(){
		join_nest_++;
		return mark();
	}

	void end_join(const Mark& mark, bool fail = false){
		join_nest_--;

		if(fail){
			results_->resize(mark.value_pos);
			unmark(mark);
			return;
		}

		int_t diff = results_->size() - mark.value_pos;
		if(join_nest_==0 && ignore_nest_==0 && diff>0){
			mm_->clear();
			if(int_t size = results_->size()){
				for(int_t i=mark.value_pos; i<size; ++i){
					mm_->put_s(results_->at(i)->to_s());
				}
				results_->resize(mark.value_pos);
			}
			results_->push_back(mm_->to_s());
		}
		unmark(mark);
	}

	Mark begin_array(){
		return mark();
	}

	void end_array(const Mark& mark, bool fail = false){
		if(fail){
			results_->resize(mark.value_pos);
			unmark(mark);
			return;
		}

		int_t diff = results_->size() - mark.value_pos;
		if(diff>=0){
			results_->push_back(results_->splice(mark.value_pos, diff));
		}
		unmark(mark, fail);
	}

	Mark begin_act(){
		return mark();
	}

	void end_act(const Mark& mark, bool fail, const AnyPtr& act, bool mv){

		if(fail){
			results_->resize(mark.value_pos);
			unmark(mark);
			return;
		}

		const VMachinePtr& vm = vmachine();
		vm->setup_call(1);
		for(uint_t i=mark.value_pos; i<results_->size(); ++i){
			vm->push_arg(results_->at(i));
		}

		results_->resize(mark.value_pos);
		unmark(mark);
		act->call(vm);

		if(mv){
			results_->cat_assign(ptr_cast<Array>(vm->result_and_cleanup_call(0)));
		}else{
			results_->push_back(vm->result_and_cleanup_call(0));
		}
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

	bool success(){
		return success_;
	}

	void set_success(bool success){
		success_ = success;
	}

	bool full(){
		return eof();
	}

protected:

	virtual int_t do_read(AnyPtr* buffer, int_t max) = 0;

	virtual void visit_members(Visitor& m){
		Base::visit_members(m);
		m & buf_ & mm_ & errors_ & results_ & newline_ch_;
	}

private:
	
	MemoryStreamPtr mm_;
	StringPtr newline_ch_;
	ArrayPtr errors_;
	ArrayPtr results_;
	ArrayPtr buf_;

	int_t join_nest_;
	int_t ignore_nest_;
	uint_t pos_;
	uint_t read_;
	uint_t lineno_;
	uint_t marked_;
	bool success_;
};

class StreamScanner : public Scanner{
public:

	StreamScanner(const StreamPtr& stream)
		:stream_(stream){}

	virtual int_t do_read(AnyPtr* buffer, int_t max);

private:
	StreamPtr stream_;

	virtual void visit_members(Visitor& m){
		Scanner::visit_members(m);
		m & stream_;
	}
};

class IteratorScanner : public Scanner{
public:

	IteratorScanner(const AnyPtr& iter)
		:iter_(iter){}

	virtual int_t do_read(AnyPtr* buffer, int_t max);

private:
	AnyPtr iter_;

	virtual void visit_members(Visitor& m){
		Scanner::visit_members(m);
		m & iter_;
	}
};

class Scanner;
typedef SmartPtr<Scanner> ScannerPtr;

MapPtr make_ch_map2(const StringPtr& ch, const AnyPtr& pp);
MapPtr make_ch_map2(const MapPtr& ch_map, const AnyPtr& pp);
MapPtr make_ch_map(const AnyPtr& p, const AnyPtr& pp);

AnyPtr str(const StringPtr& str);
AnyPtr ch_set(const StringPtr& str);
AnyPtr ch_map(const MapPtr& data);
AnyPtr more(const AnyPtr& p, int_t n);
AnyPtr ignore(const AnyPtr& p);
AnyPtr select(const AnyPtr& lhs, const AnyPtr& rhs);
AnyPtr followed(const AnyPtr& lhs, const AnyPtr& rhs);
AnyPtr join(const AnyPtr& p);
AnyPtr array(const AnyPtr& p);
AnyPtr test(const AnyPtr& p);
AnyPtr val(const AnyPtr& p);
AnyPtr and_(const AnyPtr& p);
AnyPtr not_(const AnyPtr& p);
AnyPtr sub(const AnyPtr& a, const AnyPtr& b);
AnyPtr act(const AnyPtr& a, const AnyPtr& b);
AnyPtr actmv(const AnyPtr& a, const AnyPtr& b);
AnyPtr neg(const AnyPtr& p);

AnyPtr re(const AnyPtr& pattern);


extern AnyPtr success;
extern AnyPtr fail;
extern AnyPtr eof;
extern AnyPtr any;
extern AnyPtr ch_alpha;
extern AnyPtr ch_ualpha;
extern AnyPtr ch_lalpha;
extern AnyPtr ch_space;
extern AnyPtr ch_digit;
extern AnyPtr ch_ascii;

AnyPtr P(const AnyPtr& a);

ScannerPtr parse_scanner(const AnyPtr& pattern, const ScannerPtr& scanner);

inline ScannerPtr parse_stream(const AnyPtr& pattern, const StreamPtr& stream){
	return parse_scanner(pattern, xnew<StreamScanner>(stream));
}

inline ScannerPtr parse_string(const AnyPtr& pattern, const AnyPtr& string){
	return parse_stream(pattern, xnew<StringStream>(string->to_s()));
}

inline ScannerPtr parse_iterator(const AnyPtr& pattern, const AnyPtr& iter){
	return parse_scanner(pattern, xnew<IteratorScanner>(iter));
}

}}

