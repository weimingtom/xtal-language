/** \file src/xtal/xtal_xpeg.h
* rief src/xtal/xtal_xpeg.h
*/

#ifndef XTAL_XPEG_H_INCLUDE_GUARD
#define XTAL_XPEG_H_INCLUDE_GUARD

#pragma once

namespace xtal{ 

/**
* @brief PEGライブラリ
*/
namespace xpeg{

/**
* \xbind lib::builtin::xpeg
* \xinherit lib::builtin::Any
* \brief PEGを実行する
*/
class Executor : public Base{
public:

	/**
	* \xbind
	* \brief ストリームかイテレータを受け取り構築する
	*/
	Executor(const AnyPtr& source = null, const StringPtr& source_name = empty_string){
		reset(source, source_name);
	}

	~Executor();

	/**
	* \brief 設定をリセットする。
	*/
	void reset(const AnyPtr& source, const StringPtr& source_name = empty_string);

	/**
	* \brief 渡されたパターンがマッチするか調べる。
	*/
	bool match(const AnyPtr& pattern);

	/**
	* \brief 渡されたパターンでパースする。
	*/
	bool parse(const AnyPtr& pattern);

public:

	/**
	* \brief キャプチャされた文字列を列挙するイテレータを取得する。
	*/
	AnyPtr captures();

	/**
	* \brief キャプチャされた文字列を得る。
	*/
	StringPtr at(const StringPtr& key);
	
	/**
	* \brief マッチした部分から前の文字列を取得する。
	*/
	StringPtr prefix();

	/**
	* \brief マッチした部分から後の文字列を取得する。
	*/
	StringPtr suffix();
	
public:

	/**
	* \brief キャプチャされた値の配列のイテレータを取得する。
	*/
	AnyPtr captures_values();

	/**
	* \brief キャプチャされた値の配列を得る。
	*/
	AnyPtr call(const StringPtr& key);

	/**
	* \brief マッチした部分から前の値の配列のイテレータを取得する。
	*/
	AnyPtr prefix_values();

	/**
	* \brief マッチした部分から後の値の配列のイテレータを取得する。
	*/
	AnyPtr suffix_values();

public:

	/**
	* \brief 発生したエラーのイテレータを取得する。
	*/
	AnyPtr errors(){
		if(!errors_ || errors_->empty()) return null;
		return errors_->each();
	}

	void error(const AnyPtr& message, int_t lineno = 0);

	void clear_errors(){
		if(errors_) return errors_->clear();
	}

public:

	/**
	* \brief 生成した構文木を取得する。
	*/
	const TreeNodePtr& tree(){
		return tree_;
	}

	/**
	* \brief 構文木に要素を一つ追加する
	*/
	void tree_add(const AnyPtr& a){
		tree_->push_back(a);
	}

	/**
	* \brief 構文木から要素を一つ取り除く
	*/
	AnyPtr tree_remove(){
		AnyPtr ret = tree_->back();
		tree_->pop_back();
		return ret;
	}

	/**
	* \brief 構文木の最後の要素を取得する
	*/
	const AnyPtr& tree_back(){
		return tree_->back();
	}

	struct TreeNodeState{
		int_t lineno;
		int_t pos;
	};

	TreeNodeState tree_node_begin(){
		TreeNodeState state = {lineno_, tree_->size()};
		return state;
	}

	void tree_node_end(const AnyPtr& tag, const TreeNodeState& state){
		tree_splice(tag, tree_->size()-state.pos, state.lineno);
	}

	void tree_splice(const AnyPtr& tag, int_t num);

	void tree_splice(const AnyPtr& tag, int_t num, int_t lineno);

public:

	/**
	* \brief 一文字取得する。
	*/
	const AnyPtr& read();

	const StringPtr& read_s();

	int_t read_ascii();

	/**
	* \brief n文字先をのぞき見る。
	*/
	const AnyPtr& peek(int_t n = 0);

	const StringPtr& peek_s(int_t n = 0);

	int_t peek_ascii(int_t n = 0);
	
	struct State{
		uint_t lineno;
		uint_t pos;
	};

	/**
	* \brief 現在の位置状態を取得する
	*/
	State save(){
		State state;
		state.lineno = lineno_;
		state.pos = pos_;
		return state;
	}

	/**
	* \brief 指定した位置状態に戻る
	*/
	void load(const State& state){
		pos_ = state.pos;
		lineno_ = state.lineno;
	}

	/**
	* \brief 現在の位置を返す
	*/
	uint_t pos(){
		return pos_;
	}

	/**
	* \brief 現在の行数を返す
	*/
	uint_t lineno(){
		return lineno_;
	}

	/**
	* \brief 一番最初の位置にあるか調べる
	*/
	bool bos(){
		return pos()==0;
	}

	/**
	* \brief 終了しているか調べる
	*/
	bool eos(){
		return raweq(peek(), undefined);
	}

	/**
	* \brief 行頭か調べる
	*/
	bool bol();

	/**
	* \brief 行末か調べる
	*/
	bool eol();

	/**
	* \brief n要素読み飛ばす
	*/
	void skip(uint_t n);

	void skip();

	/**
	* \brief 行末を読み飛ばす
	*/
	void skip_eol();

	bool eat(const AnyPtr& v);

	bool eat_ascii(int_t ch);

private:
	
	bool match_inner(const ElementPtr& nfa);

	bool test(const ElementPtr& elem);

	struct StackInfo{ 
		uint_t state;
		uint_t nodes;
		State pos; 
	};

	typedef PODStack<StackInfo> stack_t;
	stack_t stack_;

	void push(uint_t mins, uint_t cur_state, uint_t nodes, const State& pos){
		StackInfo temp = {cur_state, nodes, pos};
		stack_.push(temp);
	}

	struct Cap : public Base{
		int_t begin, end;
	};

	MapPtr cap_;

	TreeNodePtr tree_;
	ArrayPtr errors_;

	int_t pos_begin_;
	int_t match_begin_;
	int_t match_end_;

public:

	struct Key{
		uint_t pos;
		void* ptr;
	};

	struct Fun{
		static uint_t hash(const Key& key){
			return key.pos ^ (uint_t)key.ptr;
		}

		static bool eq(const Key& a, const Key& b){
			return a.pos==b.pos && a.ptr==b.ptr;
		}
	};

	struct Value{
		State state;
		ArrayPtr tree;
	};

	void on_visit_members(Visitor& m){
		Base::on_visit_members(m);
		m & tree_ & errors_ & cap_;
		m & mm_;
		for(uint_t i=base_, sz=num_; i<sz; ++i){
			for(int j=0; j<ONE_BLOCK_SIZE; ++j){
				m & begin_[i-base_][j];
			}
		}
	}

private:

	typedef Hashtable<Key, Value, Fun> memotable_t;
	memotable_t memotable_;

private:
	enum{
		ONE_BLOCK_SHIFT = 8,
		ONE_BLOCK_SIZE = 1<<ONE_BLOCK_SHIFT,
		ONE_BLOCK_MASK = ONE_BLOCK_SIZE-1
	};

public:

	void bin();

public:

	ArrayPtr capture_values(int_t begin, int_t end);
	
	ArrayPtr capture_values(int_t begin);
	
	StringPtr capture(int_t begin, int_t end);

	StringPtr capture(int_t begin);

	bool eat_capture(int_t begin, int_t end);

public:

	/**
	* \brief 文字列の記録を開始する
	*/
	void begin_record();

	/**
	* \brief 文字列の記録を終了して、それを返す。
	*/
	StringPtr end_record();

protected:

	int_t do_read(AnyPtr* buffer, int_t max);

	void expand();

	AnyPtr& access(int_t pos){
		if(pos<base_*ONE_BLOCK_SIZE){ 
			return undefined; 
		}
		return begin_[(pos>>ONE_BLOCK_SHIFT)-base_][pos&ONE_BLOCK_MASK];
	}

protected:
	StreamPtr stream_;
	StringPtr source_name_;

	IDPtr n_ch_;
	IDPtr r_ch_;
	
	MemoryStreamPtr mm_;
	AnyPtr** begin_;

	uint_t num_;
	uint_t max_;

	int_t pos_;
	int_t read_;
	int_t base_;

	uint_t lineno_;
	uint_t record_pos_;
};

struct Element : public Base{

	enum Type{
		TYPE_INVALID, // 空
		TYPE_ANY, // 終了文字以外
		TYPE_BOS, // beginning of stream
		TYPE_EOS, // end of stream
		TYPE_BOL, // beginning of line
		TYPE_EOL, // end of line
		TYPE_BACKREF, // 後方参照

		// 以下param1はAny
		TYPE_EQL,
		TYPE_PRED,
		TYPE_CALL,
		TYPE_INT_RANGE,
		TYPE_FLOAT_RANGE,
		TYPE_CH_RANGE,

		// 以下param1はSetである種類
		TYPE_CH_SET,

		// 以下param1はElementである種類
		TYPE_GREED,
		TYPE_LOOKAHEAD,
		TYPE_LOOKBEHIND,
		TYPE_LEAF,
		TYPE_NODE,
		TYPE_ERROR,

		////////////////////////////////

		TYPE_CONCAT, // >>
		TYPE_OR, // |
		TYPE_MORE0, // *0
		TYPE_MORE1, // *1
		TYPE_01,  // *-1
		TYPE_EMPTY, // 空
		TYPE_CAP, // キャプチャ
		TYPE_DECL // 宣言
	};

	u8 type;
	bool inv;
	AnyPtr param1;
	AnyPtr param2;
	int_t param3;

	NFAPtr nfa;

	Element(Type type);
	Element(Type type, const AnyPtr& param1);
	Element(Type type, const AnyPtr& param1, const AnyPtr& param2);
	Element(Type type, const AnyPtr& param1, const AnyPtr& param2, int_t param3);

	~Element();

	bool is_e_transition() const;

	void on_visit_members(Visitor& m){
		Base::on_visit_members(m);
		m & param1 & param2 & nfa;
	}
};

////////////////////////////////////////////////////////////////////////

struct Trans : public Base{
	ElementPtr ch;
	int to;
	SmartPtr<Trans> next;

	void on_visit_members(Visitor& m){
		Base::on_visit_members(m);
		m & ch & next;
	}
};	

////////////////////////////////////////////////////////////////////////

struct NFA : public Base{

	NFA(const ElementPtr& node);

	void add_transition(int from, const AnyPtr& ch, int to);

	void gen_nfa(int entry, const AnyPtr& t, int exit, int depth);
	
	struct State{
		TransPtr trans;
		int_t capture_kind;
		IDPtr capture_name;
	};

	int gen_state();

	bool check_infinity_loop();

	ElementPtr e_;
	ArrayList<State> states_;
	uint_t cap_max_;
	ArrayPtr cap_list_;

	void on_visit_members(Visitor& m){
		Base::on_visit_members(m);
		for(uint_t i=0; i<states_.size(); ++i){
			m & states_[i].trans;
		}
	}

	enum{
		CAPTURE_NONE = 0,
		CAPTURE_BEGIN = 1,
		CAPTURE_END = 2
	};
};

ElementPtr elem(const AnyPtr& a);
ElementPtr set(const StringPtr& str);
ElementPtr call(const AnyPtr& fun);
ElementPtr select(const AnyPtr& left, const AnyPtr& right);
ElementPtr concat(const AnyPtr& left, const AnyPtr& right);
ElementPtr more_Int(const AnyPtr& left, int_t n, int_t kind = 0);
ElementPtr more_IntRange(const AnyPtr& left, const IntRangePtr& range, int_t kind = 0);
ElementPtr more_normal_Int(const AnyPtr& left, int_t n);
ElementPtr more_shortest_Int(const AnyPtr& left, int_t n);
ElementPtr more_greed_Int(const AnyPtr& left, int_t n);
ElementPtr more_normal_IntRange(const AnyPtr& left, const IntRangePtr& range);
ElementPtr more_shortest_IntRange(const AnyPtr& left, const IntRangePtr& range);
ElementPtr more_greed_IntRange(const AnyPtr& left, const IntRangePtr& range);
ElementPtr inv(const AnyPtr& left);
ElementPtr lookahead(const AnyPtr& left);
ElementPtr lookbehind(const AnyPtr& left, int_t back);
ElementPtr cap(const IDPtr& name, const AnyPtr& left);
void cap_vm(const VMachinePtr& vm);
ElementPtr node(const AnyPtr& left);
ElementPtr node(const IDPtr& name, const AnyPtr& left);
void node_vm(const VMachinePtr& vm);
ElementPtr splice_node(int_t num, const AnyPtr& left);
ElementPtr splice_node(int_t num, const IDPtr& name, const AnyPtr& left);
void splice_node_vm(const VMachinePtr& vm);
ElementPtr leaf(const AnyPtr& left);
ElementPtr leafs(const AnyPtr& left);
ElementPtr back_ref(const AnyPtr& n);
ElementPtr decl();
void set_body(const ElementPtr& x, const AnyPtr& term);
ElementPtr bound(const AnyPtr& body, const AnyPtr& sep);
ElementPtr error(const AnyPtr& fn);
ElementPtr pred(const AnyPtr& e);

}

}


#endif // XTAL_XPEG_H_INCLUDE_GUARD
