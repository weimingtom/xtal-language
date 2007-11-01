
#pragma once

#include "xtal.h"

namespace xtal{ namespace xeg{

class Parts;

/**
* @brief 
*/
class Scanner : public Base{
public:

	Scanner(){
		buf_ = xnew<Array>(256);
		pos_ = 0;
		read_ = 0;
		marked_ = 0; //~0;

		mm_ = xnew<MemoryStream>();

		n_ch_ = "\n";
		r_ch_ = "\r";

		lineno_ = 1;
	}

	struct Mark{
		uint_t lineno;
		uint_t pos;
	};

	/**
	* @brief マークをつける
	*/
	Mark mark(){
		Mark mark;
		mark.lineno = lineno_;
		mark.pos = pos_;
		return mark;
	}

	/**
	* @brief マークをはずす
	*/
	void unmark(const Mark& mark){
		if(marked_==mark.pos){
			marked_ = 0;
		}
	}

	/**
	* @brief マークを付けた位置に戻る
	*/
	void backtrack(const Mark& mark){
		unmark(mark);
		pos_ = mark.pos;
		lineno_ = mark.lineno;
	}

	/**
	* @brief 現在の位置を返す
	*/
	uint_t pos(){
		return pos_;
	}

	/**
	* @brief 現在の行数を返す
	*/
	uint_t lineno(){
		return lineno_;
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
				return nop;
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
		if(raweq(ret, r_ch_)){
			if(rawne(peek(1), n_ch_)){
				lineno_++;
			}
		}else if(raweq(ret, n_ch_)){
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
	* @brief 一番最初の位置にあるか調べる
	*/
	bool bos(){
		return pos_==0;
	}

	/**
	* @brief 終了しているか調べる
	*/
	bool eos(){
		return raweq(peek(), nop);
	}

	/**
	* @brief 行頭か調べる
	*/
	bool bol(){
		if(pos_==0){
			return true;
		}

		uint_t bufsize = buf_->size();
		uint_t bufmask = bufsize - 1;
		const AnyPtr& ch = buf_->at((pos_-1)&bufmask);
		return raweq(ch, n_ch_) || raweq(ch, r_ch_);
	}

	/**
	* @brief 行末か調べる
	*/
	bool eol(){
		const AnyPtr& ch = peek();
		return raweq(ch, r_ch_) || raweq(ch, n_ch_) || raweq(ch, nop);
	}

	/**
	* @brief n文字読み飛ばす
	*/
	void skip(uint_t n){
		for(uint_t i=0; i<n; ++i){
			read();
		}
	}

	/**
	* @brief 行末を読み飛ばす
	*/
	void skip_eol(){
		const AnyPtr& ch = peek();
		if(raweq(ch, r_ch_)){
			if(rawne(peek(1), n_ch_)){
				skip(2);
			}else{
				skip(1);
			}
		}else if(raweq(ch, n_ch_)){
			skip(1);
		}
	}

public:

	StringPtr capture(int_t begin, int_t end){
		uint_t mask = buf_->size()-1;
		mm_->clear();
		for(int_t i=begin; i<end; ++i){
			mm_->put_s(buf_->at(i & mask)->to_s());
		}
		return mm_->to_s();
	}

	StringPtr capture(int_t begin){
		uint_t mask = buf_->size()-1;
		mm_->clear();
		int_t saved = pos_;
		pos_ = begin;
		while(!eos()){
			mm_->put_s(read()->to_s());
		}
		pos_ = saved;
		return mm_->to_s();
	}

	bool eat_capture(int_t begin, int_t end){
		uint_t mask = buf_->size()-1;
		for(int_t i=begin; i<end; ++i){
			if(rawne(peek(i-begin), buf_->at(i & mask))){
				return false;
			}
		}
		skip(end-begin);
		return true;
	}

protected:

	virtual int_t do_read(AnyPtr* buffer, int_t max) = 0;

	virtual void visit_members(Visitor& m){
		Base::visit_members(m);
		m & buf_ & mm_;
	}

private:
	
	MemoryStreamPtr mm_;
	ArrayPtr buf_;
	IDPtr n_ch_;
	IDPtr r_ch_;

	uint_t pos_;
	uint_t read_;
	uint_t marked_;
	uint_t lineno_;
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

typedef SmartPtr<Scanner> ScannerPtr;
typedef SmartPtr<StreamScanner> StreamScannerPtr;
typedef SmartPtr<IteratorScanner> IteratorScannerPtr;

class MatchResult;
typedef SmartPtr<MatchResult> MatchResultPtr;

class ParseResult;
typedef SmartPtr<ParseResult> ParseResultPtr;

class TreeNode;
typedef SmartPtr<TreeNode> TreeNodePtr;


class TreeNode : public Array{
public:

	TreeNode(const AnyPtr& tag=null, int_t lineno=0){
		tag_ = tag;
		lineno_ = lineno;
	}

	AnyPtr tag(){
		return tag_;
	}

	void set_tag(const AnyPtr& tag){
		tag_ = tag;
	}
	
	int_t lineno(){
		return lineno_;
	}

	void set_lineno(int_t lineno){
		lineno_ = lineno;
	}

private:
	AnyPtr tag_;
	int_t lineno_;

	friend class XegExec;
};


class MatchResult{
public:

	AnyPtr captures(){
		return captures_ ? captures_->each() : null;
	}

	AnyPtr named_captures(){
		return named_captures_ ? named_captures_->each() : null;
	}

	AnyPtr at(int_t key){
		if(key==0){
			return scanner_->capture(match_begin_, match_end_);
		}else{
			return captures_ ? captures_->op_at(key-1) : null;
		}
	}
	
	AnyPtr at(const StringPtr& key){
		return  named_captures_ ? named_captures_->at(key) : null;
	}

	int_t size(){
		return captures_ ? captures_->size() : 0;
	}

	int_t length(){
		return captures_ ? captures_->size() : 0;
	}

	StringPtr match(){
		return scanner_->capture(match_begin_, match_end_);
	}

	StringPtr prefix(){
		return scanner_->capture(begin_, match_begin_);
	}

	StringPtr suffix(){
		return scanner_->capture(match_end_);
	}

private:
	ScannerPtr scanner_;
	ArrayPtr captures_;
	MapPtr named_captures_;
	int_t begin_, match_begin_, match_end_;
	friend class XegExec;
};


class ParseResult{
public:

	ParseResult(const TreeNodePtr& root){
		root_ = root;
	}

	AnyPtr root(){
		return root_;
	}

private:
	TreeNodePtr root_;
	friend class XegExec;
};

MatchResultPtr match_scanner(const AnyPtr& pattern, const ScannerPtr& scanner);
MatchResultPtr match_stream(const AnyPtr& pattern, const StreamPtr& stream);
MatchResultPtr match_string(const AnyPtr& pattern, const AnyPtr& string);
MatchResultPtr match_iterator(const AnyPtr& pattern, const AnyPtr& iter);

ParseResultPtr parse_scanner(const AnyPtr& pattern, const ScannerPtr& scanner);
ParseResultPtr parse_stream(const AnyPtr& pattern, const StreamPtr& stream);
ParseResultPtr parse_string(const AnyPtr& pattern, const AnyPtr& string);
ParseResultPtr parse_iterator(const AnyPtr& pattern, const AnyPtr& iter);

struct XegExpr; 
typedef SmartPtr<XegExpr> XegExprPtr;

XegExprPtr expr(const AnyPtr& a);
XegExprPtr before(const AnyPtr& left);
XegExprPtr cap(const AnyPtr& left);
XegExprPtr cap(const IDPtr& name, const AnyPtr& left);
XegExprPtr node(const AnyPtr& left);
XegExprPtr node(const IDPtr& name, const AnyPtr& left);
XegExprPtr splice_node(int_t num, const AnyPtr& left);
XegExprPtr splice_node(int_t num, const IDPtr& name, const AnyPtr& left);
XegExprPtr leaf(const AnyPtr& left);
XegExprPtr backref(const AnyPtr& n);


struct XegExpr : public HaveName{

	enum Type{
		TYPE_TERM, //

		TYPE_CONCAT, // >>
		TYPE_OR, // |

		TYPE_MORE0, // *0
		TYPE_MORE1, // *1
		TYPE_01,  // *-1

		TYPE_EMPTY, // 空

		TYPE_CAP, // キャプチャ
	};

	XegExpr(int_t type, const AnyPtr& param1 = null, const AnyPtr& param2 = null, int_t param3 = 0)
		:type(type), param1(param1), param2(param2), param3(param3){}

	int_t type;
	AnyPtr param1;
	AnyPtr param2;
	int_t param3;

	virtual void visit_members(Visitor& m){
		HaveName::visit_members(m);
		m & param1 & param2;
	}
};

extern AnyPtr any;
extern AnyPtr bos;
extern AnyPtr eos;
extern AnyPtr bol;
extern AnyPtr eol;

}}

