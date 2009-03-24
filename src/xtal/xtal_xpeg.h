#ifndef XTAL_XPEG_H_INCLUDE_GUARD
#define XTAL_XPEG_H_INCLUDE_GUARD

#pragma once

namespace xtal{ namespace xpeg{

class MatchResult;
typedef SmartPtr<MatchResult> MatchResultPtr;

class TreeNode;
typedef SmartPtr<TreeNode> TreeNodePtr;

class Executor;
typedef SmartPtr<Executor> ExecutorPtr;

struct Element;
typedef SmartPtr<Element> ElementPtr;

struct Trans;
typedef SmartPtr<Trans> TransPtr;

struct NFA;
typedef SmartPtr<NFA> NFAPtr;

class Scanner;
typedef SmartPtr<Scanner> ScannerPtr;

/**
* @brief 構文木のノード
*/
class TreeNode : public Array{
public:
	enum{ TYPE = TYPE_TREE_NODE };

	TreeNode(const AnyPtr& tag=null, int_t lineno=0){
		set_pvalue(*this, TYPE_TREE_NODE, this);

		tag_ = tag;
		lineno_ = lineno;
	}

	const AnyPtr& tag(){
		return tag_;
	}

	int_t itag(){
		return ivalue(tag_);
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
};

/**
* @brief PEGを実行する
*/
class Executor : public Base{
public:

	Executor(const AnyPtr& stream_or_iterator = null){
		reset(stream_or_iterator);
	}

	/**
	* @brief 設定をリセットする。
	*/
	void reset(const AnyPtr& stream_or_iterator);

	/**
	* @brief 渡されたパターンがマッチするか調べる。
	*/
	bool match(const AnyPtr& pattern);

	/**
	* @brief 渡されたパターンでパースする。
	*/
	bool parse(const AnyPtr& pattern);

public:

	/**
	* @brief キャプチャされた文字列を列挙するイテレータを取得する。
	*/
	AnyPtr captures();

	/**
	* @brief キャプチャされた文字列を得る。
	*/
	StringPtr at(const StringPtr& key);
	
	/**
	* @brief マッチした部分から前の文字列を取得する。
	*/
	StringPtr prefix();

	/**
	* @brief マッチした部分から後の文字列を取得する。
	*/
	StringPtr suffix();
	
public:

	/**
	* @brief キャプチャされた値の配列のイテレータを取得する。
	*/
	AnyPtr captures_values();

	/**
	* @brief キャプチャされた値の配列を得る。
	*/
	AnyPtr call(const StringPtr& key);

	/**
	* @brief マッチした部分から前の値の配列のイテレータを取得する。
	*/
	AnyPtr prefix_values();

	/**
	* @brief マッチした部分から後の値の配列のイテレータを取得する。
	*/
	AnyPtr suffix_values();

public:

	/**
	* @brief 発生したエラーのイテレータを取得する。
	*/
	AnyPtr errors(){
		if(!errors_) return null;
		return errors_->each();
	}

	void error(const AnyPtr& message, int_t lineno = 0);

	/**
	* @brief 生成した構文木を取得する。
	*/
	TreeNodePtr tree(){
		return tree_;
	}

	/**
	* @brief 構文木に要素を追加する
	*/
	void push_result(const AnyPtr& a){
		tree_->push_back(a);
	}

	AnyPtr pop_result(){
		AnyPtr ret = tree_->back();
		tree_->pop_back();
		return ret;
	}

	const AnyPtr& back_result(){
		return tree_->back();
	}

public:

	/**
	* @brief 一文字取得する。
	*/
	const AnyPtr& read();

	const StringPtr& read_s();

	int_t read_ascii();

	/**
	* @brief n文字先をのぞき見る。
	*/
	const AnyPtr& peek(uint_t n = 0);

	const StringPtr& peek_s(uint_t n = 0);

	int_t peek_ascii(uint_t n = 0);

	struct State{
		uint_t lineno;
		uint_t pos;
	};

	/**
	* @brief マークをつける
	*/
	State save();

	/**
	* @brief マークを付けた位置に戻る
	*/
	void load(const State& state);

	/**
	* @brief 現在の位置を返す
	*/
	uint_t pos();

	/**
	* @brief 現在の行数を返す
	*/
	uint_t lineno();

	/**
	* @brief 一番最初の位置にあるか調べる
	*/
	bool bos();

	/**
	* @brief 終了しているか調べる
	*/
	bool eos();

	/**
	* @brief 行頭か調べる
	*/
	bool bol();

	/**
	* @brief 行末か調べる
	*/
	bool eol();

	/**
	* @brief n要素読み飛ばす
	*/
	void skip(uint_t n);

	void skip();

	/**
	* @brief 行末を読み飛ばす
	*/
	void skip_eol();

	bool eat(const AnyPtr& v);

	bool eat_ascii(int_t ch);

private:
	
	const NFAPtr& fetch_nfa(const ElementPtr& node);

	bool match_inner(const AnyPtr& nfa);

	bool test(const AnyPtr& elem);

	void push(uint_t mins, uint_t st, uint_t nodes, const State& pos);

	struct StackInfo{ 
		uint_t state;
		uint_t nodes;
		State pos; 
	};

	typedef PODStack<StackInfo> stack_t;

	struct Cap{
		int_t begin, end;
	};

	stack_t stack_;
	MapPtr nfa_map_;
	MapPtr cap_;

	TreeNodePtr tree_;
	ArrayPtr errors_;
	ScannerPtr scanner_;

	int_t begin_;
	int_t match_begin_;
	int_t match_end_;

	virtual void visit_members(Visitor& m){
		Base::visit_members(m);
		m & nfa_map_ & tree_ & errors_ & cap_;
	}
};


AnyPtr lookahead(const AnyPtr& elem);

AnyPtr lookbehind(const AnyPtr& elem, int_t back);

AnyPtr bound(const AnyPtr& body, const AnyPtr& sep);

AnyPtr pred(const AnyPtr& elem);

/**
* @brief 後方参照
*/
AnyPtr back_ref(const AnyPtr& no);

/**
* @brief 名前つきキャプチャ
*/
AnyPtr cap(const IDPtr& name, const AnyPtr& elem);

AnyPtr error(const AnyPtr& fn);

AnyPtr node(const AnyPtr& elem);
AnyPtr node(const IDPtr& name, const AnyPtr& elem);
AnyPtr splice_node(int_t num, const AnyPtr& elem);
AnyPtr splice_node(int_t num, const IDPtr& name, const AnyPtr& elem);
AnyPtr leaf(const AnyPtr& left);
AnyPtr leafs(const AnyPtr& left);

}}


#endif // XTAL_XPEG_H_INCLUDE_GUARD
