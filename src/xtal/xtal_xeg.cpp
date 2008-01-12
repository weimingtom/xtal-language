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

int_t StreamScanner::do_read(AnyPtr* buffer, int_t max){
	for(int_t i=0; i<max; ++i){
		if(stream_->eos()){
			return i;
		}

		buffer[i] = stream_->get_s(1);
	}
	return max;
}


IteratorScanner::IteratorScanner(const AnyPtr& iter)
	:iter_(iter->send(Xid(each))){}

int_t IteratorScanner::do_read(AnyPtr* buffer, int_t max){
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


////////////////////////////////////////////////////////////////////////

struct XegElem : public Base{

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
		TYPE_NOT_EQL,

		TYPE_EQ,
		TYPE_NOT_EQ,

		// 以下param1はSetである種類
		TYPE_CH_SET,
		TYPE_NOT_CH_SET,

		// 以下param1はXegElemである種類
		TYPE_BEFORE,
		TYPE_NOT_BEFORE,
		TYPE_AFTER,
		TYPE_NOT_AFTER,

		TYPE_LEAF,
		TYPE_NODE,

		TYPE_ERROR,
		TYPE_ERROR_FAIL,

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

	XegElem(Type type, const AnyPtr& param1 = null, const AnyPtr& param2 = null, int_t param3 = 0)
		:type(type), param1(param1), param2(param2), param3(param3){}

	virtual void visit_members(Visitor& m){
		Base::visit_members(m);
		m & param1 & param2;
	}
};

typedef SmartPtr<XegElem> XegElemPtr;

class XegExec{
public:

	struct Trans{
		XegElemPtr ch; // この文字集合が来たら
		int to; // 状態番号toの状態へ遷移
		SmartPtr<Trans> next; // 連結リスト
	};	

	typedef SmartPtr<Trans> TransPtr;

	enum{ STATE_START = 0, STATE_FINAL = 1 };

	struct NFA{

		NFA(const XegElemPtr& node);

		void add_transition(int from, const AnyPtr& ch, int to);

		void gen_nfa(int entry, const AnyPtr& t, int exit);

		void add_e_transition(int from, int to){
			add_transition(from, e, to);
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

		XegElemPtr e;
		XegElemPtr root_node;
		AC<State>::vector states;
		int cap_max, named_cap_max;
		ArrayPtr named_cap_list;

		enum{
			CAPTURE_NONE = 0,
			CAPTURE_BEGIN = 1,
			CAPTURE_END = 2,
			NAMED_CAPTURE_BEGIN = 3,
			NAMED_CAPTURE_END = 4,
		};
	};

	typedef SmartPtr<NFA> NFAPtr;

	MatchResultPtr match(const XegElemPtr& pattern, const ScannerPtr& scanner){
		const NFAPtr& nfa = fetch_nfa(pattern);
		int_t begin = scanner->pos();
		errors_ = xnew<Array>();
		for(;;){
			int_t match_begin = scanner->pos();
			if(match_inner(nfa, scanner)){
				MatchResultPtr result = xnew<MatchResult>();
				result->captures_ = info_.top().cap_values;
				result->named_captures_ = info_.top().named_cap_values;
				result->scanner_ = scanner;
				result->begin_ = begin;
				result->match_begin_ = match_begin;
				result->match_end_ = scanner->pos();
				result->errors_ = errors_;

				info_.downsize(1);
				return result;
			}

			if(scanner->eos()){
				break;
			}

			scanner->read();
		}

		info_.downsize(1);
		return null;
	}

	ParseResultPtr parse(const XegElemPtr& pattern, const ScannerPtr& scanner){
		tree_ = xnew<TreeNode>();
		errors_ = xnew<Array>();
		const NFAPtr& nfa = fetch_nfa(pattern);
		if(match_inner(nfa, scanner)){
			ParseResultPtr result = xnew<ParseResult>();
			result->tree_ = tree_;
			result->errors_ = errors_;
			info_.downsize(1);
			return result;
		}
		info_.downsize(1);
		return null;
	}

private:

	const NFAPtr& fetch_nfa(const XegElemPtr& node){
		const AnyPtr& temp = nfa_map->at(node);
		if(temp){
			return static_ptr_cast<NFA>(temp);
		}
		else{
			nfa_map->set_at(node, xnew<NFA>(node));
			return static_ptr_cast<NFA>(nfa_map->at(node));
		}
	}

	bool match_inner(const NFAPtr& nfa, const ScannerPtr& scanner);

	bool test(const ScannerPtr& scanner, const XegElemPtr& elem);

	void push(uint_t st, uint_t nodes, const Scanner::State& pos);

	struct StackInfo{ 
		uint_t state; 
		uint_t nodes;
		Scanner::State pos; 
	};
	typedef Stack<StackInfo> stack_t;

	struct Info{
		stack_t stack;

		ArrayPtr cap;
		MapPtr named_cap;

		ArrayPtr cap_values;
		MapPtr named_cap_values;
	};

	struct Cap{
		int_t begin, end;
	};

	TreeNodePtr tree_;
	ArrayPtr errors_;
	Stack<Info> info_;

};

AnyPtr concat(const AnyPtr&, const AnyPtr&);

XegElemPtr elem(const AnyPtr& a){
	if(const XegElemPtr& p = ptr_as<XegElem>(a)){
		return p;
	}

	if(const ChRangePtr& p = ptr_as<ChRange>(a)){
		SetPtr chset = xnew<Set>();
		Xfor(v, p){
			chset->set_at(v, true);
		}
		return xnew<XegElem>(XegElem::TYPE_CH_SET, chset);
	}

	if(const StringPtr& p = ptr_as<String>(a)){
		if(p->length()==1){
			return xnew<XegElem>(XegElem::TYPE_EQL, p);
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

	XTAL_THROW(builtin()->member("RuntimeError")(Xt("Xtal Runtime Error 1026")), return null);
	return null;
}

AnyPtr set(const StringPtr& str){
	SetPtr chset = xnew<Set>();
	Xfor(v, str){
		chset->set_at(v, true);
	}
	return xnew<XegElem>(XegElem::TYPE_CH_SET, chset);
}

AnyPtr select(const AnyPtr& left, const AnyPtr& right){
	XegElemPtr eleft = elem(left);
	XegElemPtr eright = elem(right);

	if((eleft->type==XegElem::TYPE_EQL || eleft->type==XegElem::TYPE_CH_SET) && 
		(eright->type==XegElem::TYPE_EQL || eright->type==XegElem::TYPE_CH_SET)){

		SetPtr chset = xnew<Set>();

		if(eleft->type==XegElem::TYPE_EQL){
			chset->set_at(eleft->param1, true);
		}

		if(eleft->type==XegElem::TYPE_CH_SET){
			Xfor(v, eleft->param1){
				chset->set_at(v, true);
			}
		}

		if(eright->type==XegElem::TYPE_EQL){
			chset->set_at(eright->param1, true);
		}

		if(eright->type==XegElem::TYPE_CH_SET){
			Xfor(v, eright->param1){
				chset->set_at(v, true);
			}
		}
		return xnew<XegElem>(XegElem::TYPE_CH_SET, chset);
	}

	return xnew<XegElem>(XegElem::TYPE_OR, eleft, eright); 
}

AnyPtr concat(const AnyPtr& left, const AnyPtr& right){ 
	return xnew<XegElem>(XegElem::TYPE_CONCAT, elem(left), elem(right)); 
}

AnyPtr more_Int(const AnyPtr& left, int_t n, int_t kind = 0){
	if(n==0){ return xnew<XegElem>(XegElem::TYPE_MORE0, elem(left), null, kind); }
	else if(n==1){ return xnew<XegElem>(XegElem::TYPE_MORE1, elem(left), null, kind); }
	else if(n==-1){ return xnew<XegElem>(XegElem::TYPE_01, elem(left), null, kind); }

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
	XegElemPtr e = elem(left);
	switch(e->type){
		XTAL_CASE(XegElem::TYPE_EQ){ return xnew<XegElem>(XegElem::TYPE_NOT_EQ, e->param1); }
		XTAL_CASE(XegElem::TYPE_EQL){ return xnew<XegElem>(XegElem::TYPE_NOT_EQL, e->param1); }
		XTAL_CASE(XegElem::TYPE_CH_SET){ return xnew<XegElem>(XegElem::TYPE_NOT_CH_SET, e->param1); }
		XTAL_CASE(XegElem::TYPE_BEFORE){ return xnew<XegElem>(XegElem::TYPE_NOT_BEFORE, e->param1); }
		XTAL_CASE(XegElem::TYPE_AFTER){ return xnew<XegElem>(XegElem::TYPE_NOT_AFTER, e->param1, e->param2, e->param3); }
	}

	XTAL_THROW(builtin()->member("RuntimeError")(Xt("Xtal Runtime Error 1027")), return null);
	return null;
}

AnyPtr before(const AnyPtr& left){ return xnew<XegElem>(XegElem::TYPE_BEFORE, elem(left)); }
AnyPtr after(const AnyPtr& left, int_t back){ return xnew<XegElem>(XegElem::TYPE_AFTER, elem(left), null, back); }

AnyPtr cap(const AnyPtr& left){ return xnew<XegElem>(XegElem::TYPE_CAP, elem(left)); }
AnyPtr cap(const IDPtr& name, const AnyPtr& left){ return xnew<XegElem>(XegElem::TYPE_CAP, elem(left), name, 1); }

void cap_vm(const VMachinePtr& vm){
	if(vm->named_arg_count()!=0){ vm->return_result(cap(vm->arg_name(0), vm->arg(vm->arg_name(0)))); }
	else{ 
		if(vm->ordered_arg_count()==2){ vm->return_result(cap(ptr_cast<ID>(vm->arg(0)), vm->arg(1))); }
		else{ vm->return_result(cap(vm->arg(0))); }
	}
}

AnyPtr node(const AnyPtr& left){ return xnew<XegElem>(XegElem::TYPE_NODE, elem(left)); }
AnyPtr node(const IDPtr& name, const AnyPtr& left){ return xnew<XegElem>(XegElem::TYPE_NODE, elem(left), name); }

void node_vm(const VMachinePtr& vm){
	if(vm->named_arg_count()!=0){ vm->return_result(node(vm->arg_name(0), vm->arg(vm->arg_name(0)))); }
	else{ 
		if(vm->ordered_arg_count()==2){ vm->return_result(node(ptr_cast<ID>(vm->arg(0)), vm->arg(1))); }
		else{ vm->return_result(node(vm->arg(0))); }
	}
}

AnyPtr splice_node(int_t num, const AnyPtr& left){ return xnew<XegElem>(XegElem::TYPE_NODE, elem(left), null, num); }
AnyPtr splice_node(int_t num, const IDPtr& name, const AnyPtr& left){ return xnew<XegElem>(XegElem::TYPE_NODE, elem(left), name, num); }

void splice_node_vm(const VMachinePtr& vm){
	if(vm->named_arg_count()!=0){ vm->return_result(splice_node(vm->arg(0)->to_i(), vm->arg_name(0), vm->arg(vm->arg_name(0)))); }
	else{ 
		if(vm->ordered_arg_count()==3){ vm->return_result(splice_node(vm->arg(0)->to_i(), ptr_cast<ID>(vm->arg(1)), vm->arg(2))); }
		else{ vm->return_result(splice_node(vm->arg(0)->to_i(), vm->arg(0))); }
	}
}

AnyPtr leaf(const AnyPtr& left){ return xnew<XegElem>(XegElem::TYPE_LEAF, elem(left)); }
AnyPtr back_ref(const AnyPtr& n){ return xnew<XegElem>(XegElem::TYPE_BACKREF, n); }

AnyPtr decl(){ return xnew<XegElem>(XegElem::TYPE_DECL); }
void set_body(const XegElemPtr& x, const AnyPtr& term){ if(x->type==XegElem::TYPE_DECL) x->param1 = elem(term); }

AnyPtr bound(const AnyPtr& body, const AnyPtr& sep){ return after(sep, 1) >> body >> before(sep); }
AnyPtr error(const AnyPtr& fn){ return xnew<XegElem>(XegElem::TYPE_ERROR, fn); }
AnyPtr error_fail(const AnyPtr& fn){ return xnew<XegElem>(XegElem::TYPE_ERROR_FAIL, fn); }
AnyPtr eq(const AnyPtr& e){ return xnew<XegElem>(XegElem::TYPE_EQ, e); }
AnyPtr eql(const AnyPtr& e){ return xnew<XegElem>(XegElem::TYPE_EQL, e); }

XegExec::NFA::NFA(const XegElemPtr& node){
	e = xnew<XegElem>(XegElem::TYPE_INVALID);

	root_node = node;
	cap_max = 0;
	named_cap_max = 0;
	named_cap_list = xnew<Array>();

	gen_state(); // start
	gen_state(); // final
	gen_nfa(STATE_START, node, STATE_FINAL);
}

void XegExec::NFA::add_transition(int from, const AnyPtr& ch, int to){
	TransPtr x = xnew<Trans>();
	x->to = to;
	x->ch = static_ptr_cast<XegElem>(ch);
	x->next = states[from].trans;
	states[from].trans = x;
}

void XegExec::NFA::gen_nfa(int entry, const AnyPtr& a, int exit){
	const XegElemPtr& t = static_ptr_cast<XegElem>(a);

	switch(t->type){
		XTAL_DEFAULT{
			//         ch
			//  entry -----> exit
			add_transition(entry, t, exit);
		}

		XTAL_CASE(XegElem::TYPE_CONCAT){
			//         left         right
			//  entry ------> step -------> exit
			int step = gen_state();
			gen_nfa(entry, t->param1, step);
			gen_nfa(step, t->param2, exit);
		}

		XTAL_CASE(XegElem::TYPE_OR){
			//               left
			//        ----------------->
			//  entry -----> step -----> exit
			//          e         right

			int step = gen_state();
			gen_nfa(entry, t->param1, exit);
			add_e_transition(entry, step);
			gen_nfa(step, t->param2, exit);
		}

		XTAL_CASE2(XegElem::TYPE_MORE0, XegElem::TYPE_MORE1){
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
				if(t->type == XegElem::TYPE_MORE0){ 
					add_e_transition(entry, exit); 
				}
				add_e_transition(entry, before);

				add_e_transition(after, exit);
				add_e_transition(after, before);
			}
			else{
				// beforeに向かう方が優先
				add_e_transition(entry, before);
				if(t->type == XegElem::TYPE_MORE0){ 
					add_e_transition(entry, exit); 
				}

				add_e_transition(after, before);
				add_e_transition(after, exit);
			}

			gen_nfa(before, t->param1, after);

			if(t->param3==2){
				states[entry].greed = true;
				states[after].greed = true;
			}
		}

		XTAL_CASE(XegElem::TYPE_01){
			//           e
			//        ------>
			//  entry ------> exit
			//         left

			if(t->param3==1){
				// eを経由する方が優先
				add_e_transition(entry, exit);
				gen_nfa(entry, t->param1, exit);
			}
			else{
				// leftを経由する方が優先
				gen_nfa(entry, t->param1, exit);
				add_e_transition(entry, exit);
			}

			if(t->param3==2){
				states[entry].greed = true;
			}
		}

		XTAL_CASE(XegElem::TYPE_EMPTY){
			//         e
			//  entry ---> exit
			add_e_transition(entry, exit);
		}

		XTAL_CASE(XegElem::TYPE_CAP){
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
			gen_nfa(before, t->param1, after);
			add_e_transition(after, exit);
		}

		XTAL_CASE(XegElem::TYPE_DECL){
			gen_nfa(entry, t->param1, exit);
		}
	}
}


void XegExec::push(uint_t cur_state, uint_t nodes, const Scanner::State& pos){
	Info& info = info_.top();
	for(uint_t i=0, sz=info.stack.size(); i<sz; ++i){
		if(info.stack[i].pos.pos != pos.pos){
			break;
		}
		else if(info.stack[i].state == cur_state){
			return;
		}
	}

	StackInfo temp = {cur_state, nodes, pos};
	info.stack.push(temp);
}

bool XegExec::match_inner(const NFAPtr& nfa, const ScannerPtr& scanner){

	int_t nodenum = tree_ ? tree_->size() : 0;

	{
		Info& info = info_.push();
		info.stack.clear();
		if(nfa->cap_max!=0){ 
			info.cap = xnew<Array>(nfa->cap_max);
			for(int_t i=0, sz=nfa->cap_max; i<sz; ++i){
				info.cap->set_at(i, xnew<Cap>());
			}
		}

		if(nfa->named_cap_max!=0){ 
			info.named_cap = xnew<Map>(); 
			for(uint_t i=0, sz=nfa->named_cap_list->size(); i<sz; ++i){
				info.named_cap->set_at(nfa->named_cap_list->at(i), xnew<Cap>());
			}
		}
	}

	bool match = false;
	Scanner::State match_pos = {0, 0};
	Scanner::State first_pos = scanner->save();

	push(STATE_START, nodenum, first_pos);

	while(!info_.top().stack.empty()){
		StackInfo& se = info_.top().stack.pop();
		int cur_state = se.state;
		
		if(tree_ && se.nodes<tree_->size()){
			tree_->resize(se.nodes);
		}

		Scanner::State pos = se.pos;
		NFA::State& state = nfa->states[cur_state];

		if(cur_state == STATE_FINAL && match_pos.pos <= pos.pos){
			match_pos = pos;
			match = true;
		}

		bool fail = true;
		for(const TransPtr* tr=&state.trans; *tr; tr=&(*tr)->next){
			scanner->load(pos);
			if(test(scanner, (*tr)->ch)){
				push((*tr)->to, tree_ ? tree_->size() : 0, scanner->save());
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
					const SmartPtr<Cap>& temp = static_ptr_cast<Cap>(info_.top().cap->at(state.capture_index));
					temp->begin = pos.pos;
					temp->end = -1;
				}

				XTAL_CASE(NFA::CAPTURE_END){
					const SmartPtr<Cap>& temp = static_ptr_cast<Cap>(info_.top().cap->at(state.capture_index));
					temp->end = pos.pos;
				}

				XTAL_CASE(NFA::NAMED_CAPTURE_BEGIN){
					const SmartPtr<Cap>& temp = static_ptr_cast<Cap>(info_.top().named_cap->at(state.capture_name));
					temp->begin = pos.pos;
					temp->end = -1;
				}

				XTAL_CASE(NFA::NAMED_CAPTURE_END){
					const SmartPtr<Cap>& temp = static_ptr_cast<Cap>(info_.top().named_cap->at(state.capture_name));
					temp->end = pos.pos;
				}
			}
		}
	}

	if(match){
		scanner->load(match_pos);

		if(nfa->cap_max!=0){
			info_.top().cap_values = xnew<Array>();
			Xfor(v, info_.top().cap){
				const SmartPtr<Cap>& temp = static_ptr_cast<Cap>(v);

				if(temp->end>=0 && temp->end-temp->begin>0){
					info_.top().cap_values->push_back(scanner->capture(temp->begin, temp->end));
				}
				else{
					if(temp->end==temp->begin){
						info_.top().cap_values->push_back("");
					}
					else{
						info_.top().cap_values->push_back(null);
					}
				}
			}
		}
		else{
			info_.top().cap_values = null;
		}

		if(nfa->named_cap_max!=0){
			info_.top().named_cap_values = xnew<Map>();
			Xfor2(k, v, info_.top().named_cap){
				const SmartPtr<Cap>& temp = static_ptr_cast<Cap>(v);

				if(temp->end>=0 && temp->end-temp->begin>0){
					info_.top().named_cap_values->set_at(k, scanner->capture(temp->begin, temp->end));
				}
				else{
					if(temp->end==temp->begin){
						info_.top().cap_values->push_back("");
					}
					else{
						info_.top().cap_values->push_back(null);
					}
				}
			}
		}
		else{
			info_.top().named_cap_values = null;
		}

		return true;
	}

	if(tree_){
		tree_->resize(nodenum);
	}

	scanner->load(first_pos);
	return false;
}

bool XegExec::test(const ScannerPtr& scanner, const XegElemPtr& elem){
	switch(elem->type){
		XTAL_NODEFAULT;

		XTAL_CASE(XegElem::TYPE_INVALID){
			return true;
		}

		XTAL_CASE(XegElem::TYPE_ANY){
			if(scanner->eos()){ return false; }
			scanner->read();
			return true;
		}

		XTAL_CASE(XegElem::TYPE_BOS){
			return scanner->bos();
		}

		XTAL_CASE(XegElem::TYPE_EOS){
			return scanner->eos();
		}

		XTAL_CASE(XegElem::TYPE_BOL){
			return scanner->bol();
		}

		XTAL_CASE(XegElem::TYPE_EOL){
			if(!scanner->eol()){ return false; }
			scanner->skip_eol();
			return true;
		}
	
		XTAL_CASE(XegElem::TYPE_EQ){
			if(scanner->read()==elem->param1){ return true; }
			return false;
		}

		XTAL_CASE(XegElem::TYPE_NOT_EQ){
			if(scanner->eos()){ return false; }
			if(scanner->read()!=elem->param1){ return true; }
			return false;
		}

		XTAL_CASE(XegElem::TYPE_EQL){
			if(raweq(scanner->read(), elem->param1)){ return true; }
			return false;
		}

		XTAL_CASE(XegElem::TYPE_NOT_EQL){
			if(scanner->eos()){ return false; }
			if(rawne(scanner->read(), elem->param1)){ return true; }
			return false;
		}

		XTAL_CASE(XegElem::TYPE_CH_SET){
			const MapPtr& data = static_ptr_cast<Map>(elem->param1);
			if(data->at(scanner->read())){ return true; }
			return false;
		}

		XTAL_CASE(XegElem::TYPE_NOT_CH_SET){
			if(scanner->eos()){ return false; }
			const MapPtr& data = static_ptr_cast<Map>(elem->param1);
			if(!data->at(scanner->read())){ return true; }
			return false;
		}

		XTAL_CASE2(XegElem::TYPE_BEFORE, XegElem::TYPE_NOT_BEFORE){
			const NFAPtr& nfa = fetch_nfa(static_ptr_cast<XegElem>(elem->param1));
			Scanner::State state = scanner->save();
			bool ret = match_inner(nfa, scanner);
			info_.downsize(1);
			scanner->load(state);
			return elem->type==XegElem::TYPE_NOT_BEFORE ? !ret : ret;
		}

		XTAL_CASE2(XegElem::TYPE_AFTER, XegElem::TYPE_NOT_AFTER){
			const NFAPtr& nfa = fetch_nfa(static_ptr_cast<XegElem>(elem->param1));
			Scanner::State state = scanner->save();
			Scanner::State fict_state = state;
			fict_state.pos = fict_state.pos > (uint_t)elem->param3 ? fict_state.pos-elem->param3 : 0;
			scanner->load(fict_state);
			bool ret = match_inner(nfa, scanner);
			info_.downsize(1);
			scanner->load(state);
			return elem->type==XegElem::TYPE_NOT_AFTER ? !ret : ret;
		}

		XTAL_CASE(XegElem::TYPE_LEAF){
			const NFAPtr& nfa = fetch_nfa(static_ptr_cast<XegElem>(elem->param1));
			int_t pos = scanner->pos();
			if(match_inner(nfa, scanner)){
				if(tree_){
					tree_->push_back(scanner->capture(pos, scanner->pos()));
				}
				info_.downsize(1);
				return true;
			}
			info_.downsize(1);
			return false;
		}

		XTAL_CASE(XegElem::TYPE_NODE){
			const NFAPtr& nfa = fetch_nfa(static_ptr_cast<XegElem>(elem->param1));
			int_t pos = scanner->pos();
			if(tree_){
				int_t nodenum = tree_->size() - elem->param3;
				if(nodenum<0){ nodenum = 0; }

				if(match_inner(nfa, scanner)){
					TreeNodePtr node = xnew<TreeNode>();
					node->tag_ = elem->param2->to_s()->intern();
					node->lineno_ = scanner->lineno();
					node->assign(tree_->splice(nodenum, tree_->size()-nodenum));
					tree_->push_back(node);
					info_.downsize(1);
					return true;
				}
			}
			else{
				if(match_inner(nfa, scanner)){
					info_.downsize(1);
					return true;
				}
			}
			info_.downsize(1);
			return false;
		}

		XTAL_CASE(XegElem::TYPE_BACKREF){
			const SmartPtr<Cap>& temp = type(elem->param1)==TYPE_INT
				? static_ptr_cast<Cap>(info_.top().cap->at(ivalue(elem->param1)))
				: static_ptr_cast<Cap>(info_.top().named_cap->at(elem->param1));
			if(temp && temp->end>=0 && temp->end-temp->begin>0){
				return scanner->eat_capture(temp->begin, temp->end);
			}
			return false;
		}

		XTAL_CASE2(XegElem::TYPE_ERROR, XegElem::TYPE_ERROR_FAIL){
			if(errors_){
				errors_->push_back(elem->param1(Named("line", scanner->lineno())));
			}
			return elem->type==XegElem::TYPE_ERROR;
		}
	}

	return false;
}

ScannerPtr create_scanner_Scanner(const ScannerPtr& scanner){ return scanner; }
ScannerPtr create_scanner_Stream(const StreamPtr& stream){ return xnew<StreamScanner>(stream); }
ScannerPtr create_scanner_String(const StringPtr& string){ return xnew<StreamScanner>(xnew<StringStream>(string)); }
ScannerPtr create_scanner_Iterator(const AnyPtr& iter){ return xnew<IteratorScanner>(iter); }

MatchResultPtr Scanner::match(const AnyPtr& pattern){
	return XegExec().match(elem(pattern), from_this(this));
}

ParseResultPtr Scanner::parse(const AnyPtr& pattern){
	return XegExec().parse(elem(pattern), from_this(this));
}

void def_common_method(const ClassPtr& p){
	p->method("op_div", &more_shortest_Int, get_cpp_class<Int>());
	p->method("op_div", &more_shortest_IntRange, get_cpp_class<IntRange>());
	p->method("op_mul", &more_normal_Int, get_cpp_class<Int>());
	p->method("op_mul", &more_normal_IntRange, get_cpp_class<IntRange>());
	p->method("op_mod", &more_greed_Int, get_cpp_class<Int>());
	p->method("op_mod", &more_greed_IntRange, get_cpp_class<IntRange>());
	p->method("op_com", &inv);
	
	p->method("op_or", &select, get_cpp_class<XegElem>());
	p->method("op_or", &select, get_cpp_class<String>());
	p->method("op_or", &select, get_cpp_class<ChRange>());
	p->method("op_or", &select, get_cpp_class<Fun>());
	p->method("op_shr", &concat, get_cpp_class<XegElem>());
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
		ClassPtr p = new_cpp_class<Scanner>("Scanner");
		p->method("parse", &Scanner::parse);
		p->method("match", &Scanner::match);
	}

	{
		ClassPtr p = new_cpp_class<IteratorScanner>("IteratorScanner");
		p->inherit(get_cpp_class<Scanner>());
	}

	{
		ClassPtr p = new_cpp_class<StreamScanner>("StreamScanner");
		p->inherit(get_cpp_class<Scanner>());
	}

	{
		ClassPtr p = new_cpp_class<TreeNode>("TreeNode");
		p->inherit(get_cpp_class<Array>());
		p->method("tag", &TreeNode::tag);
		p->method("lineno", &TreeNode::lineno);
	}

	{
		ClassPtr p = new_cpp_class<MatchResult>("MatchResult");
		p->method("captures", &MatchResult::captures);
		p->method("named_captures", &MatchResult::named_captures);
		p->method("op_at", (AnyPtr (MatchResult::*)(int_t))&MatchResult::at, get_cpp_class<Int>());
		p->method("op_at", (AnyPtr (MatchResult::*)(const StringPtr&))&MatchResult::at, get_cpp_class<String>());
		p->method("size", &MatchResult::size);
		p->method("length", &MatchResult::length);
		p->method("prefix", &MatchResult::prefix);
		p->method("suffix", &MatchResult::suffix);
		p->method("errors", &MatchResult::errors);
	}
	
	{
		ClassPtr p = new_cpp_class<ParseResult>("ParseResult");
		p->method("tree", &ParseResult::tree);
		p->method("errors", &ParseResult::errors);
	}

	new_cpp_class<Scanner>();

	{
		ClassPtr p = new_cpp_class<XegElem>("XegElem");
		def_common_method(p);
		p->method("set_body", &set_body);
	}

	{
		def_common_method(new_cpp_class<ChRange>());
		def_common_method(new_cpp_class<String>());
		def_common_method(new_cpp_class<Fun>());
	}

	any = xnew<XegElem>(XegElem::TYPE_ANY);
	bos = xnew<XegElem>(XegElem::TYPE_BOS);
	eos = xnew<XegElem>(XegElem::TYPE_EOS);
	bol = xnew<XegElem>(XegElem::TYPE_BOL);
	eol = xnew<XegElem>(XegElem::TYPE_EOL);
	empty = xnew<XegElem>(XegElem::TYPE_EMPTY);
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
	xeg->fun("error_fail", &error_fail);

	xeg->fun("decl", &decl);

	xeg->dual_dispatch_fun("create_scanner");
	xeg->fun("create_scanner", &create_scanner_Scanner, get_cpp_class<Scanner>()); 
	xeg->fun("create_scanner", &create_scanner_Stream, get_cpp_class<Stream>()); 
	xeg->fun("create_scanner", &create_scanner_String, get_cpp_class<String>()); 
	xeg->fun("create_scanner", &create_scanner_Iterator, Iterable()); 

	xeg->def("Scanner", new_cpp_class<Scanner>());
	xeg->def("IteratorScanner", new_cpp_class<IteratorScanner>());
	xeg->def("StreamScanner", new_cpp_class<StreamScanner>());
	builtin()->def("xeg", xeg);
}

void test_xeg(){
	using namespace xeg;
	
	XegElemPtr paren = elem(Xsrc((  
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
