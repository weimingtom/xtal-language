#include "xtal.h"
#include "xtal_macro.h"
	
namespace xtal{ namespace xeg{

AnyPtr any;
AnyPtr bos;
AnyPtr eos;
AnyPtr bol;
AnyPtr eol;
AnyPtr empty;
AnyPtr alpha;
AnyPtr degit;
AnyPtr lower;
AnyPtr upper;
AnyPtr word;
AnyPtr ascii;

MapPtr nfa_map;

void uninitialize_xeg(){
	any = null;
	bos = null;
	eos = null;
	bol = null;
	eol = null;
	empty = null;
	alpha = null;
	degit = null;
	lower = null;
	upper = null;
	word = null;
	ascii = null;

	nfa_map = null;
}

class Scanner;
typedef SmartPtr<Scanner> ScannerPtr;

class StreamScanner;
typedef SmartPtr<StreamScanner> StreamScannerPtr;

class IteratorScanner;
typedef SmartPtr<IteratorScanner> IteratorScannerPtr;

struct Element;
typedef SmartPtr<Element> ElementPtr;

struct Trans;
typedef SmartPtr<Trans> TransPtr;

struct NFA;
typedef SmartPtr<NFA> NFAPtr;

AnyPtr concat(const AnyPtr&, const AnyPtr&);
ElementPtr elem(const AnyPtr& a);

/**
* @brief 
*/
class Scanner : public Base{

	enum{
		ONE_BLOCK_SHIFT = 12,
		ONE_BLOCK_SIZE = 1<<ONE_BLOCK_SHIFT,
		ONE_BLOCK_MASK = ONE_BLOCK_SIZE-1
	};

public:

	Scanner();

	typedef Executor::SState State;

	/**
	* @brief マークをつける
	*/
	State save(){
		State state;
		state.lineno = lineno_;
		state.pos = pos_;
		return state;
	}

	/**
	* @brief マークを付けた位置に戻る
	*/
	void load(const State& state){
		pos_ = state.pos;
		lineno_ = state.lineno;
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
	const AnyPtr& peek(uint_t n = 0);

	/**
	* @brief ひとつ読み取る
	*/
	const AnyPtr& read();

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
		return raweq(peek(), undefined);
	}

	/**
	* @brief 行頭か調べる
	*/
	bool bol();

	/**
	* @brief 行末か調べる
	*/
	bool eol(){
		const AnyPtr& ch = peek();
		return raweq(ch, r_ch_) || raweq(ch, n_ch_);
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
	void skip_eol();

public:

	StringPtr capture(int_t begin, int_t end);

	StringPtr capture(int_t begin);

	bool eat_capture(int_t begin, int_t end);

protected:

	virtual int_t do_read(AnyPtr* buffer, int_t max) = 0;

	virtual void visit_members(Visitor& m){
		Base::visit_members(m);
		m & mm_;
	}

	void expand();

	AnyPtr& access(uint_t pos){
		return begin_[pos>>ONE_BLOCK_SHIFT][pos&ONE_BLOCK_MASK];
	}

private:

	IDPtr n_ch_;
	IDPtr r_ch_;
	
	MemoryStreamPtr mm_;
	AnyPtr** begin_;
	uint_t num_;
	uint_t pos_;
	uint_t read_;
	uint_t lineno_;
};

Scanner::Scanner(){
	num_ = 0;
	begin_ = 0;

	mm_ = xnew<MemoryStream>();
	pos_ = 0;
	read_ = 0;

	n_ch_ = XTAL_STRING("\n");
	r_ch_ = XTAL_STRING("\r");

	lineno_ = 1;

	expand();
}

const AnyPtr& Scanner::peek(uint_t n){
	while(pos_+n >= read_){
		uint_t now_read = 0;

		if((read_>>ONE_BLOCK_SHIFT)==num_){
			expand();
		}

		now_read = do_read(&access(read_), ONE_BLOCK_SIZE-(read_&ONE_BLOCK_MASK));

		if(now_read==0){
			return undefined;
		}

		read_ += now_read;
	}
	
	return access(pos_+n);
}

const AnyPtr& Scanner::read(){
	const AnyPtr& ret = peek();
	if(raweq(ret, r_ch_)){
		if(rawne(peek(1), n_ch_)){
			lineno_++;
		}
	}
	else if(raweq(ret, n_ch_)){
		lineno_++;
	}

	pos_ += 1;
	return  ret;
}

bool Scanner::bol(){
	if(pos_==0){
		return true;
	}

	const AnyPtr& ch = access(pos_-1);
	return raweq(ch, n_ch_) || raweq(ch, r_ch_);
}

void Scanner::skip_eol(){
	const AnyPtr& ch = peek();
	if(raweq(ch, r_ch_)){
		if(raweq(peek(1), n_ch_)){
			skip(2);
		}
		else{
			skip(1);
		}
	}
	else if(raweq(ch, n_ch_)){
		skip(1);
	}
}

StringPtr Scanner::capture(int_t begin, int_t end){
	mm_->clear();
	for(int_t i=begin; i<end; ++i){
		mm_->put_s(access(i)->to_s());
	}
	return mm_->to_s();
}

StringPtr Scanner::capture(int_t begin){
	mm_->clear();
	int_t saved = pos_;
	pos_ = begin;
	while(!eos()){
		mm_->put_s(read()->to_s());
	}
	pos_ = saved;
	return mm_->to_s();
}

bool Scanner::eat_capture(int_t begin, int_t end){
	for(int_t i=begin; i<end; ++i){
		if(rawne(peek(i-begin), access(i))){
			return false;
		}
	}
	skip(end-begin);
	return true;
}

void Scanner::expand(){
	uint_t newnum = num_ + 1;
	AnyPtr** newp = (AnyPtr**)user_malloc(sizeof(AnyPtr*)*newnum);

	if(begin_){
		std::memcpy(newp, begin_, sizeof(AnyPtr*)*num_);
	}

	newp[num_] = (AnyPtr*)user_malloc(sizeof(AnyPtr)*ONE_BLOCK_SIZE);
	std::memset(newp[num_], 0, sizeof(AnyPtr)*ONE_BLOCK_SIZE);

	user_free(begin_);
	begin_ = newp;
	num_ = newnum;
}

class StreamScanner : public Scanner{
public:

	StreamScanner(const StreamPtr& stream)
		:stream_(stream){}

	virtual int_t do_read(AnyPtr* buffer, int_t max){
		for(int_t i=0; i<max; ++i){
			if(stream_->eos()){
				return i;
			}

			buffer[i] = stream_->get_s(1);
		}
		return max;
	}

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
		:iter_(iter->send(Xid(each))){}

	virtual int_t do_read(AnyPtr* buffer, int_t max){
		if(!iter_){
			return 0;
		}

		const VMachinePtr& vm = vmachine();
		for(int_t i=0; i<max; ++i){
			vm->setup_call(2);
			iter_->rawsend(vm, Xid(block_next));
			iter_ = vm->result(0);
			if(!iter_){
				vm->cleanup_call();
				return i;
			}
			buffer[i] = vm->result(1);
			vm->cleanup_call();
		}
		return max;
	}

private:
	AnyPtr iter_;

	virtual void visit_members(Visitor& m){
		Scanner::visit_members(m);
		m & iter_;
	}
};

////////////////////////////////////////////////////////////////////////

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

		TYPE_EQ,

		TYPE_CALL,

		// 以下param1はSetである種類
		TYPE_CH_SET,

		// 以下param1はElementである種類
		TYPE_BEFORE,
		TYPE_AFTER,

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
		TYPE_DECL, // 宣言
	};

	Type type;
	AnyPtr param1;
	AnyPtr param2;
	int_t param3;
	bool inv;

	Element(Type type, const AnyPtr& param1 = null, const AnyPtr& param2 = null, int_t param3 = 0)
		:type(type), param1(param1), param2(param2), param3(param3), inv(false){}

	virtual void visit_members(Visitor& m){
		Base::visit_members(m);
		m & param1 & param2;
	}
};

////////////////////////////////////////////////////////////////////////

struct Trans : public Base{
	ElementPtr ch;
	int to;
	SmartPtr<Trans> next;

	virtual void visit_members(Visitor& m){
		Base::visit_members(m);
		m & ch & next;
	}
};	

////////////////////////////////////////////////////////////////////////

struct NFA : public Base{

	NFA(const ElementPtr& node);

	void add_transition(int from, const AnyPtr& ch, int to);

	void gen_nfa(int entry, const AnyPtr& t, int exit, int depth);

	void add_e_transition(int from, int to){
		add_transition(from, e, to);
	}

	void printn(const char* str, int depth){
		//for(int i=0; i<depth; ++i){
		//	printf("%s", " ");
		//}
		//printf("%s\n", str);
	}
	
	struct State{
		TransPtr trans;
		int_t capture_kind;
		int_t capture_index;
		IDPtr capture_name;
		bool greed;
	};

	int gen_state(){
		State state;
		state.capture_kind = 0;
		state.capture_index = 0;
		state.greed = false;
		states.push_back(state);
		return states.size() - 1;
	}

	ElementPtr e;
	ElementPtr root_node;
	AC<State>::vector states;
	uint_t cap_max, named_cap_max;
	ArrayPtr named_cap_list;

	enum{
		CAPTURE_NONE = 0,
		CAPTURE_BEGIN = 1,
		CAPTURE_END = 2,
		NAMED_CAPTURE_BEGIN = 3,
		NAMED_CAPTURE_END = 4,
	};
};
	
enum{ NFA_STATE_START = 0, NFA_STATE_FINAL = 1 };

NFA::NFA(const ElementPtr& node){
	e = xnew<Element>(Element::TYPE_INVALID);

	root_node = node;
	cap_max = 0;
	named_cap_max = 0;
	named_cap_list = xnew<Array>();

	gen_state(); // NFA_STATE_START
	gen_state(); // NFA_STATE_FINAL
	gen_nfa(NFA_STATE_START, node, NFA_STATE_FINAL, 0);
}

void NFA::add_transition(int from, const AnyPtr& ch, int to){
	TransPtr x = xnew<Trans>();
	x->to = to;
	x->ch = ptr_cast<Element>(ch);
	x->next = states[from].trans;
	states[from].trans = x;
}

void NFA::gen_nfa(int entry, const AnyPtr& a, int exit, int depth){
	const ElementPtr& t = static_ptr_cast<Element>(a);

	switch(t->type){
		XTAL_DEFAULT{
			printn("DEFAULT", depth);
			//         ch
			//  entry -----> exit
			add_transition(entry, t, exit);
		}

		XTAL_CASE(Element::TYPE_CONCAT){
			printn("CONCAT", depth);
			//         left         right
			//  entry ------> step -------> exit

			int step = gen_state();
			gen_nfa(entry, t->param1, step, depth+1);
			gen_nfa(step, t->param2, exit, depth+1);
		}

		XTAL_CASE(Element::TYPE_OR){
			printn("OR", depth);
			//               left
			//        ----------------->
			//  entry -----> step -----> exit
			//          e         right

			int step = gen_state();
			gen_nfa(entry, t->param1, exit, depth+1);
			add_e_transition(entry, step);
			gen_nfa(step, t->param2, exit, depth+1);
		}

		XTAL_CASE2(Element::TYPE_MORE0, Element::TYPE_MORE1){
			printn("MORE", depth);
			//                       e
			//         e          <------        e
			//  entry ---> before ------> after ---> exit
			//    |                left                ^
			//    >------->------------------->------>-|
			//                      e
			//----------------------------------------------
			//                       e
			//         e          <------        e
			//  entry ---> before ------> after ---> exit
			//                     left
			int before = gen_state();
			int after = gen_state();
			
			if(t->param3==1){
				// exitに向かう方が優先
				if(t->type == Element::TYPE_MORE0){ add_e_transition(entry, exit); }
				add_e_transition(entry, before);

				// exitに向かう方が優先
				add_e_transition(after, exit);
				add_e_transition(after, before);
			}
			else{
				// beforeに向かう方が優先
				add_e_transition(entry, before);
				if(t->type == Element::TYPE_MORE0){ add_e_transition(entry, exit); }

				// beforeに向かう方が優先
				add_e_transition(after, before);
				add_e_transition(after, exit);
			}

			// beforeからafterへの遷移 
			gen_nfa(before, t->param1, after, depth+1);

			if(t->param3==2){
				states[entry].greed = true;
				states[after].greed = true;
			}
		}

		XTAL_CASE(Element::TYPE_01){
			printn("01", depth);
			//           e
			//        ------>
			//  entry ------> exit
			//         left

			if(t->param3==1){
				// eを経由する方が優先
				add_e_transition(entry, exit);
				gen_nfa(entry, t->param1, exit, depth+1);
			}
			else{
				// leftを経由する方が優先
				gen_nfa(entry, t->param1, exit, depth+1);
				add_e_transition(entry, exit);
			}

			if(t->param3==2){
				states[entry].greed = true;
			}
		}

		XTAL_CASE(Element::TYPE_EMPTY){
			printn("EMPTY", depth);
			//         e
			//  entry ---> exit
			add_e_transition(entry, exit);
		}

		XTAL_CASE(Element::TYPE_CAP){
			printn("CAP", depth);
			int before = gen_state();
			int after = gen_state();

			if(t->param3==0){
				states[before].capture_kind = CAPTURE_BEGIN;
				states[after].capture_kind = CAPTURE_END;
				states[before].capture_index = cap_max;
				states[after].capture_index = cap_max;
				cap_max++;		
			}
			else{
				states[before].capture_kind = NAMED_CAPTURE_BEGIN;
				states[after].capture_kind = NAMED_CAPTURE_END;
				states[before].capture_name = static_ptr_cast<ID>(t->param2);
				states[after].capture_name = static_ptr_cast<ID>(t->param2);
				named_cap_list->push_back(t->param2);
				named_cap_max++;
			}

			add_e_transition(entry, before);
			gen_nfa(before, t->param1, after, depth+1);
			add_e_transition(after, exit);
		}

		XTAL_CASE(Element::TYPE_DECL){
			printn("DECL", depth);
			gen_nfa(entry, t->param1, exit, depth+1);
		}
	}
}

////////////////////////////////////////////////////////////////////////

const NFAPtr& fetch_nfa(const ElementPtr& node){
	const AnyPtr& temp = nfa_map->at(node);
	if(temp){
		return static_ptr_cast<NFA>(temp);
	}
	else{
		nfa_map->set_at(node, xnew<NFA>(node));
		return static_ptr_cast<NFA>(nfa_map->at(node));
	}
}

void Executor::reset(const AnyPtr& stream_or_iterator){

	if(stream_or_iterator->is(get_cpp_class<Stream>())){
		scanner_ = xnew<StreamScanner>(ptr_cast<Stream>(stream_or_iterator));
	}
	else if(stream_or_iterator->is(get_cpp_class<String>())){
		scanner_ = xnew<StreamScanner>(xnew<StringStream>(ptr_cast<String>(stream_or_iterator)));
	}
	else{
		scanner_ = xnew<IteratorScanner>(stream_or_iterator);
	}

	cap_ = xnew<Array>();
	named_cap_ = xnew<Map>();
	cap_values_ = xnew<Array>();
	named_cap_values_ = xnew<Map>();
	tree_ = xnew<TreeNode>();
	errors_ = xnew<Array>();
	begin_ = 0;
	match_begin_ = 0;
	match_end_ = 0;
}

bool Executor::match(const AnyPtr& pattern){
	const NFAPtr& nfa = fetch_nfa(elem(pattern));
	begin_ = scanner_->pos();
	for(;;){
		match_begin_ = scanner_->pos();
		if(match_inner(nfa)){
			match_end_ = scanner_->pos();
			return true;
		}

		if(scanner_->eos()){
			return false;
		}

		scanner_->read();
	}
}

bool Executor::parse(const AnyPtr& pattern){
	return match_inner(fetch_nfa(elem(pattern)));
}

AnyPtr Executor::at(int_t key){
	if(key==0){
		return static_ptr_cast<Scanner>(scanner_)->capture(match_begin_, match_end_);
	}
	else{
		return cap_values_ ? cap_values_->op_at(key-1) : null;
	}
}

StringPtr Executor::prefix(){
	return static_ptr_cast<Scanner>(scanner_)->capture(begin_, match_begin_);
}

StringPtr Executor::suffix(){
	return static_ptr_cast<Scanner>(scanner_)->capture(match_end_);
}

const AnyPtr& Executor::read(){
	return scanner_->read();
}

const AnyPtr& Executor::peek(uint_t n){
	return scanner_->peek(n);
}

void Executor::push(uint_t mins, uint_t cur_state, uint_t nodes, const SState& pos){
	for(uint_t i=mins, sz=stack_.size(); i<sz; ++i){
		if(stack_[i].pos.pos != pos.pos){
			break;
		}
		else if(stack_[i].state == cur_state){
			return;
		}
	}

	StackInfo temp = {cur_state, nodes, pos};
	stack_.push(temp);
}

bool Executor::match_inner(const AnyPtr& anfa){
	const NFAPtr& nfa = static_ptr_cast<NFA>(anfa);

	int_t nodenum = tree_->size();
	uint_t mins = stack_.size();
	uint_t minc = cap_->size();

	if(minc<nfa->cap_max){
		cap_->resize(nfa->cap_max);
		for(int_t i=minc, sz=nfa->cap_max; i<sz; ++i){
			cap_->set_at(i, xnew<Cap>());
		}
	}

	for(uint_t i=0, sz=nfa->named_cap_list->size(); i<sz; ++i){
		named_cap_->set_at(nfa->named_cap_list->at(i), xnew<Cap>());
	}

	bool match = false;
	SState match_pos = {0, 0};
	SState first_pos = scanner_->save();

	push(mins, NFA_STATE_START, nodenum, first_pos);

	while(stack_.size()>mins){
		StackInfo& se = stack_.pop();
		int cur_state = se.state;
		
		SState pos = se.pos;
		NFA::State& state = nfa->states[cur_state];

		if(cur_state == NFA_STATE_FINAL && match_pos.pos <= pos.pos){
			match_pos = pos;
			match = true;
		}

		bool fail = true;
		for(const TransPtr* tr=&state.trans; *tr; tr=&(*tr)->next){
			scanner_->load(pos);

			if(se.nodes<tree_->size()){
				tree_->resize(se.nodes);
			}

			if(test((*tr)->ch)){
				push(mins, (*tr)->to, tree_->size(), scanner_->save());
				fail = false;

				if(state.greed){
					break;
				}
			}
		}

		if(fail){
			if(match){
				break;
			}
		}
		else{
			switch(state.capture_kind){
				XTAL_NODEFAULT;
				XTAL_CASE(NFA::CAPTURE_NONE){}

				XTAL_CASE(NFA::CAPTURE_BEGIN){
					const SmartPtr<Cap>& temp = static_ptr_cast<Cap>(cap_->at(state.capture_index));
					temp->begin = pos.pos;
					temp->end = -1;
				}

				XTAL_CASE(NFA::CAPTURE_END){
					const SmartPtr<Cap>& temp = static_ptr_cast<Cap>(cap_->at(state.capture_index));
					temp->end = pos.pos;
				}

				XTAL_CASE(NFA::NAMED_CAPTURE_BEGIN){
					const SmartPtr<Cap>& temp = static_ptr_cast<Cap>(named_cap_->at(state.capture_name));
					temp->begin = pos.pos;
					temp->end = -1;
				}

				XTAL_CASE(NFA::NAMED_CAPTURE_END){
					const SmartPtr<Cap>& temp = static_ptr_cast<Cap>(named_cap_->at(state.capture_name));
					temp->end = pos.pos;
				}
			}
		}
	}

	stack_.downsize_n(mins);

	if(match){
		scanner_->load(match_pos);

		if(nfa->cap_max!=0){
			Xfor(v, cap_){
				const SmartPtr<Cap>& temp = static_ptr_cast<Cap>(v);

				if(temp->end>=0 && temp->end-temp->begin>0){
					cap_values_->push_back(scanner_->capture(temp->begin, temp->end));
				}
				else{
					if(temp->end==temp->begin){
						cap_values_->push_back("");
					}
					else{
						cap_values_->push_back(null);
					}
				}
			}
		}

		if(nfa->named_cap_max!=0){
			Xfor2(k, v, named_cap_){
				const SmartPtr<Cap>& temp = static_ptr_cast<Cap>(v);

				if(temp->end>=0 && temp->end-temp->begin>0){
					named_cap_values_->set_at(k, scanner_->capture(temp->begin, temp->end));
				}
				else{
					if(temp->end==temp->begin){
						cap_values_->push_back("");
					}
					else{
						cap_values_->push_back(null);
					}
				}
			}
		}

		return true;
	}

	tree_->resize(nodenum);

	scanner_->load(first_pos);
	return false;
}

bool Executor::test(const AnyPtr& ae){
	const ElementPtr& e = static_ptr_cast<Element>(ae);

	switch(e->type){
		XTAL_NODEFAULT;

		XTAL_CASE(Element::TYPE_INVALID){
			return !e->inv;
		}

		XTAL_CASE(Element::TYPE_ANY){
			if(scanner_->eos()){ return e->inv; }
			scanner_->read();
			return !e->inv;
		}

		XTAL_CASE(Element::TYPE_BOS){
			return scanner_->bos()!=e->inv;
		}

		XTAL_CASE(Element::TYPE_EOS){
			return scanner_->eos()!=e->inv;
		}

		XTAL_CASE(Element::TYPE_BOL){
			return scanner_->bol()!=e->inv;
		}

		XTAL_CASE(Element::TYPE_EOL){
			if(!scanner_->eol()){ return e->inv; }
			scanner_->skip_eol();
			return !e->inv;
		}
	
		XTAL_CASE(Element::TYPE_EQ){
			if(scanner_->read()==e->param1){ return !e->inv; }
			return e->inv;
		}

		XTAL_CASE(Element::TYPE_EQL){
			if(raweq(scanner_->read(), e->param1)){ return !e->inv; }
			return e->inv;
		}

		XTAL_CASE(Element::TYPE_CH_SET){
			const MapPtr& data = static_ptr_cast<Map>(e->param1);
			if(data->at(scanner_->read())){ return !e->inv; }
			return e->inv;
		}

		XTAL_CASE(Element::TYPE_CALL){
			if(scanner_->eos()){ return e->inv; }
			AnyPtr ret = e->param1(from_this(this));
			return (ret || raweq(ret, undefined))!=e->inv;
		}

		XTAL_CASE(Element::TYPE_BEFORE){
			const NFAPtr& nfa = fetch_nfa(static_ptr_cast<Element>(e->param1));
			Scanner::State state = scanner_->save();
			bool ret = match_inner(nfa);
			scanner_->load(state);
			return ret!=e->inv;
		}

		XTAL_CASE(Element::TYPE_AFTER){
			const NFAPtr& nfa = fetch_nfa(static_ptr_cast<Element>(e->param1));
			Scanner::State state = scanner_->save();
			Scanner::State fict_state = state;
			fict_state.pos = fict_state.pos > (uint_t)e->param3 ? fict_state.pos-e->param3 : 0;
			scanner_->load(fict_state);
			bool ret = match_inner(nfa);
			scanner_->load(state);
			return ret!=e->inv;
		}

		XTAL_CASE(Element::TYPE_LEAF){
			const NFAPtr& nfa = fetch_nfa(static_ptr_cast<Element>(e->param1));
			int_t pos = scanner_->pos();
			if(match_inner(nfa)){
				if(tree_){
					tree_->push_back(scanner_->capture(pos, scanner_->pos()));
				}
				return !e->inv;
			}
			return e->inv;
		}

		XTAL_CASE(Element::TYPE_NODE){
			const NFAPtr& nfa = fetch_nfa(static_ptr_cast<Element>(e->param1));
			int_t pos = scanner_->pos();
			if(tree_){
				int_t nodenum = tree_->size() - e->param3;
				if(nodenum<0){ nodenum = 0; }

				if(match_inner(nfa)){
					TreeNodePtr node = xnew<TreeNode>();
					node->set_tag(e->param2->to_s()->intern());
					node->set_lineno(scanner_->lineno());
					node->assign(tree_->splice(nodenum, tree_->size()-nodenum));
					tree_->push_back(node);
					return !e->inv;
				}
			}
			else{
				if(match_inner(nfa)){
					return !e->inv;
				}
			}
			return e->inv;
		}

		XTAL_CASE(Element::TYPE_BACKREF){
			const SmartPtr<Cap>& temp = type(e->param1)==TYPE_INT
				? static_ptr_cast<Cap>(cap_->at(ivalue(e->param1)))
				: static_ptr_cast<Cap>(named_cap_->at(e->param1));
			if(temp && temp->end>=0 && temp->end-temp->begin>0){
				return (scanner_->eat_capture(temp->begin, temp->end))!=e->inv;
			}
			return e->inv;
		}

		XTAL_CASE(Element::TYPE_ERROR){
			if(errors_){
				errors_->push_back(e->param1(Named("line", scanner_->lineno())));
			}
			return !e->inv;
		}
	}

	return false;
}

////////////////////////////////////////////////////////////////////////

ElementPtr elem(const AnyPtr& a){
	if(const ElementPtr& p = ptr_as<Element>(a)){
		return p;
	}

	if(const ChRangePtr& p = ptr_as<ChRange>(a)){
		SetPtr chset = xnew<Set>();
		Xfor(v, p){
			chset->set_at(v, true);
		}
		return xnew<Element>(Element::TYPE_CH_SET, chset);
	}

	if(const StringPtr& p = ptr_as<String>(a)){
		if(p->length()==1){
			return xnew<Element>(Element::TYPE_EQL, p);
		}

		AnyPtr str = null;
		Xfor(v, p->each()){
			if(str){
				str = concat(str, v);
			}
			else{
				str = v;
			}
		}
		
		return elem(str);
	}

	if(const FunPtr& p = ptr_as<Fun>(a)){
		return xnew<Element>(Element::TYPE_CALL, p);
	}

	XTAL_THROW(builtin()->member("RuntimeError")(Xt("Xtal Runtime Error 1026")), return null);
	return null;
}

AnyPtr set(const StringPtr& str){
	SetPtr chset = xnew<Set>();
	Xfor(v, str){
		chset->set_at(v, true);
	}
	return xnew<Element>(Element::TYPE_CH_SET, chset);
}

AnyPtr call(const AnyPtr& fun){
	return xnew<Element>(Element::TYPE_CALL, fun);
}

AnyPtr select(const AnyPtr& left, const AnyPtr& right){
	ElementPtr eleft = elem(left);
	ElementPtr eright = elem(right);

	if((eleft->type==Element::TYPE_EQL || eleft->type==Element::TYPE_CH_SET) && 
		(eright->type==Element::TYPE_EQL || eright->type==Element::TYPE_CH_SET)){

		SetPtr chset = xnew<Set>();

		if(eleft->type==Element::TYPE_EQL){
			chset->set_at(eleft->param1, true);
		}

		if(eleft->type==Element::TYPE_CH_SET){
			Xfor(v, eleft->param1){
				chset->set_at(v, true);
			}
		}

		if(eright->type==Element::TYPE_EQL){
			chset->set_at(eright->param1, true);
		}

		if(eright->type==Element::TYPE_CH_SET){
			Xfor(v, eright->param1){
				chset->set_at(v, true);
			}
		}
		return xnew<Element>(Element::TYPE_CH_SET, chset);
	}

	return xnew<Element>(Element::TYPE_OR, eleft, eright); 
}

AnyPtr concat(const AnyPtr& left, const AnyPtr& right){ 
	return xnew<Element>(Element::TYPE_CONCAT, elem(left), elem(right)); 
}

AnyPtr more_Int(const AnyPtr& left, int_t n, int_t kind = 0){
	if(n==0){ return xnew<Element>(Element::TYPE_MORE0, elem(left), null, kind); }
	else if(n==1){ return xnew<Element>(Element::TYPE_MORE1, elem(left), null, kind); }
	else if(n==-1){ return xnew<Element>(Element::TYPE_01, elem(left), null, kind); }

	if(n>0){ return concat(left, more_Int(left, n-1, kind)); }
	else{ return concat(more_Int(left, -1, kind), more_Int(left, n+1, kind)); }
}

AnyPtr more_IntRange(const AnyPtr& left, const IntRangePtr& range, int_t kind = 0){
	if(range->begin()<=0){
		int n = -(range->end()-1);
		return n < 0 ? more_Int(left, n, kind) : empty;
	}

	return concat(left, more_IntRange(left, xnew<IntRange>(range->begin()-1, range->end()-1, RANGE_LEFT_CLOSED_RIGHT_OPEN), kind));
}

AnyPtr more_normal_Int(const AnyPtr& left, int_t n){ return more_Int(left, n, 0); }
AnyPtr more_shortest_Int(const AnyPtr& left, int_t n){ return more_Int(left, n, 1); }
AnyPtr more_greed_Int(const AnyPtr& left, int_t n){ return more_Int(left, n, 2); }
AnyPtr more_normal_IntRange(const AnyPtr& left, const IntRangePtr& range){ return more_IntRange(left, range, 0); }
AnyPtr more_shortest_IntRange(const AnyPtr& left, const IntRangePtr& range){ return more_IntRange(left, range, 1); }
AnyPtr more_greed_IntRange(const AnyPtr& left, const IntRangePtr& range){ return more_IntRange(left, range, 2); }

AnyPtr inv(const AnyPtr& left){
	ElementPtr e = elem(left);
	ElementPtr ret = xnew<Element>(e->type, e->param1, e->param2, e->param3);
	ret->inv = !e->inv;
	return ret;
}

AnyPtr before(const AnyPtr& left){ return xnew<Element>(Element::TYPE_BEFORE, elem(left)); }
AnyPtr after(const AnyPtr& left, int_t back){ return xnew<Element>(Element::TYPE_AFTER, elem(left), null, back); }

AnyPtr cap(const AnyPtr& left){ return xnew<Element>(Element::TYPE_CAP, elem(left)); }
AnyPtr cap(const IDPtr& name, const AnyPtr& left){ return xnew<Element>(Element::TYPE_CAP, elem(left), name, 1); }

void cap_vm(const VMachinePtr& vm){
	if(vm->named_arg_count()!=0){ vm->return_result(cap(vm->arg_name(0), vm->arg(vm->arg_name(0)))); }
	else{ 
		if(vm->ordered_arg_count()==2){ vm->return_result(cap(ptr_cast<ID>(vm->arg(0)), vm->arg(1))); }
		else{ vm->return_result(cap(vm->arg(0))); }
	}
}

AnyPtr node(const AnyPtr& left){ return xnew<Element>(Element::TYPE_NODE, elem(left)); }
AnyPtr node(const IDPtr& name, const AnyPtr& left){ return xnew<Element>(Element::TYPE_NODE, elem(left), name); }

void node_vm(const VMachinePtr& vm){
	if(vm->named_arg_count()!=0){ vm->return_result(node(vm->arg_name(0), vm->arg(vm->arg_name(0)))); }
	else{ 
		if(vm->ordered_arg_count()==2){ vm->return_result(node(ptr_cast<ID>(vm->arg(0)), vm->arg(1))); }
		else{ vm->return_result(node(vm->arg(0))); }
	}
}

AnyPtr splice_node(int_t num, const AnyPtr& left){ return xnew<Element>(Element::TYPE_NODE, elem(left), null, num); }
AnyPtr splice_node(int_t num, const IDPtr& name, const AnyPtr& left){ return xnew<Element>(Element::TYPE_NODE, elem(left), name, num); }

void splice_node_vm(const VMachinePtr& vm){
	if(vm->named_arg_count()!=0){ vm->return_result(splice_node(vm->arg(0)->to_i(), vm->arg_name(0), vm->arg(vm->arg_name(0)))); }
	else{ 
		if(vm->ordered_arg_count()==3){ vm->return_result(splice_node(vm->arg(0)->to_i(), ptr_cast<ID>(vm->arg(1)), vm->arg(2))); }
		else{ vm->return_result(splice_node(vm->arg(0)->to_i(), vm->arg(0))); }
	}
}

AnyPtr leaf(const AnyPtr& left){ return xnew<Element>(Element::TYPE_LEAF, elem(left)); }
AnyPtr back_ref(const AnyPtr& n){ return xnew<Element>(Element::TYPE_BACKREF, n); }

AnyPtr decl(){ return xnew<Element>(Element::TYPE_DECL); }
void set_body(const ElementPtr& x, const AnyPtr& term){ if(x->type==Element::TYPE_DECL) x->param1 = elem(term); }

AnyPtr bound(const AnyPtr& body, const AnyPtr& sep){ return after(sep, 1) >> body >> before(sep); }
AnyPtr error(const AnyPtr& fn){ return xnew<Element>(Element::TYPE_ERROR, fn); }
AnyPtr eq(const AnyPtr& e){ return xnew<Element>(Element::TYPE_EQ, e); }
AnyPtr eql(const AnyPtr& e){ return xnew<Element>(Element::TYPE_EQL, e); }

////////////////////////////////////////////////////////////////////////

void def_common_methods(const ClassPtr& p){
	p->method("op_div", &more_shortest_Int, get_cpp_class<Int>());
	p->method("op_div", &more_shortest_IntRange, get_cpp_class<IntRange>());
	p->method("op_mul", &more_normal_Int, get_cpp_class<Int>());
	p->method("op_mul", &more_normal_IntRange, get_cpp_class<IntRange>());
	p->method("op_mod", &more_greed_Int, get_cpp_class<Int>());
	p->method("op_mod", &more_greed_IntRange, get_cpp_class<IntRange>());
	p->method("op_com", &inv);
	
	p->method("op_or", &select, get_cpp_class<Element>());
	p->method("op_or", &select, get_cpp_class<String>());
	p->method("op_or", &select, get_cpp_class<ChRange>());
	p->method("op_or", &select, get_cpp_class<Fun>());
	p->method("op_shr", &concat, get_cpp_class<Element>());
	p->method("op_shr", &concat, get_cpp_class<String>());
	p->method("op_shr", &concat, get_cpp_class<ChRange>());
	p->method("op_shr", &concat, get_cpp_class<Fun>());
}

}

void initialize_xeg(){
	using namespace xeg;
	register_uninitializer(&uninitialize_xeg);

	ClassPtr xeg = xnew<Singleton>("xeg");

	{
		ClassPtr p = new_cpp_class<Executor>("Executor");
		p->def("new", ctor<Executor, const AnyPtr&>()->param(Named("stream_or_iterator", "")));
		p->method("reset", &Executor::reset);
		p->method("parse", &Executor::parse);
		p->method("match", &Executor::match);

		p->method("captures", &Executor::captures);
		p->method("named_captures", &Executor::named_captures);
		p->method("op_at", (AnyPtr (Executor::*)(int_t))&Executor::at, get_cpp_class<Int>());
		p->method("op_at", (AnyPtr (Executor::*)(const StringPtr&))&Executor::at, get_cpp_class<String>());
		p->method("prefix", &Executor::prefix);
		p->method("suffix", &Executor::suffix);
		p->method("errors", &Executor::errors);
		p->method("read", &Executor::read);
		p->method("peek", &Executor::peek)->param(Named("n", 0));
		p->method("tree", &Executor::tree);
	}

	{
		ClassPtr p = new_cpp_class<TreeNode>("TreeNode");
		p->inherit(get_cpp_class<Array>());
		p->method("tag", &TreeNode::tag);
		p->method("lineno", &TreeNode::lineno);
	}

	{
		ClassPtr p = new_cpp_class<Element>("Element");
		def_common_methods(p);
		p->method("set_body", &set_body);
	}

	{
		def_common_methods(new_cpp_class<ChRange>());
		def_common_methods(new_cpp_class<String>());
		def_common_methods(new_cpp_class<Fun>());
	}

	any = xnew<Element>(Element::TYPE_ANY);
	bos = xnew<Element>(Element::TYPE_BOS);
	eos = xnew<Element>(Element::TYPE_EOS);
	bol = xnew<Element>(Element::TYPE_BOL);
	eol = xnew<Element>(Element::TYPE_EOL);
	empty = xnew<Element>(Element::TYPE_EMPTY);
	degit = elem(AnyPtr("0")->send(Xid(op_range))("9", RANGE_CLOSED));
	lower = elem(AnyPtr("a")->send(Xid(op_range))("z", RANGE_CLOSED));
	upper = elem(AnyPtr("A")->send(Xid(op_range))("Z", RANGE_CLOSED));
	alpha = lower | upper;
	word = alpha | degit | "_";
	ascii = elem(xnew<String>((char_t)1)->send(Xid(op_range))(xnew<String>((char_t)127), RANGE_CLOSED));

	nfa_map = xnew<Map>();

	xeg->def("any", any);
	xeg->def("bos", bos);
	xeg->def("eos", eos);
	xeg->def("bol", bol);
	xeg->def("eol", eol);
	xeg->def("empty", empty);
	xeg->def("degit", degit);
	xeg->def("lower", lower);
	xeg->def("upper", upper);
	xeg->def("alpha", alpha);
	xeg->def("word", word);
	xeg->def("ascii", ascii);
	
	xeg->fun("set", &set);
	xeg->fun("back_ref", &back_ref);
	xeg->fun("before", &before);
	xeg->fun("after", &after);
	xeg->fun("leaf", &leaf);
	xeg->fun("node", &node_vm);
	xeg->fun("splice_node", &splice_node_vm);
	xeg->fun("cap", &cap_vm);
	xeg->fun("bound", &bound);
	xeg->fun("eq", &eq);
	xeg->fun("eql", &eql);
	xeg->fun("error", &error);

	xeg->fun("decl", &decl);

	xeg->def("Executor", new_cpp_class<Executor>());
	builtin()->def("xeg", xeg);
}

void test_xeg(){
	using namespace xeg;
	
	ElementPtr paren = elem(Xsrc((  
	filelocal.inherit(xeg);

myexpect: fun(pattern) pattern | error(%f(line=%(line)s expect error));
//myexpect: fun(pattern) pattern | error(fun(line:0) line.p);

	pattern: eql("a") >> myexpect("b");

	"erwedrgadrtreabew\naer".scan(pattern){
		it[0].p;
		it.errors[].p;
	}

	"ete:rwer,a-er::ere,".split("::" | set(":,") | "-")[].p;

	))());
}

}
