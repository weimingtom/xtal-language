#include "xtal.h"
#include "xtal_macro.h"
	
namespace xtal{ namespace xeg{

}

namespace{

	void uninitialize_xeg(){

	}

}

namespace xeg{

int_t StreamScanner::do_read(AnyPtr* buffer, int_t max){
	for(int_t i=0; i<max; ++i){
		if(stream_->eos()){
			return i;
		}

		buffer[i] = stream_->get_s(1);
	}
	return max;
}

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

struct XegElem;
typedef SmartPtr<XegElem> XegElemPtr;

struct XegNode; 
typedef SmartPtr<XegNode> XegNodePtr;

struct XegElem : public Base{

	enum Type{
		TYPE_EPSILON, // 空
		TYPE_ANY, // 終了文字以外
		TYPE_BACKREF, // 後方参照

		// 以下param1はStringである種類
		TYPE_CH,
		TYPE_NOT_CH,

		// 以下param1はSetである種類
		TYPE_CH_SET,
		TYPE_NOT_CH_SET,

		// 以下param1はXegNodeである種類
		TYPE_BEFORE,
		TYPE_NOT_BEFORE,

		TYPE_LEAF,
		TYPE_NODE,

	};

	Type type;
	AnyPtr param1;
	AnyPtr param2;

	XegElem(Type type, const AnyPtr& param1 = null, const AnyPtr& param2 = null)
		:type(type), param1(param1), param2(param2){}

	virtual void visit_members(Visitor& m){
		Base::visit_members(m);
		m & param1 & param2;
	}
};

struct XegNode : public HaveName{

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

	XegNode(int_t type, const AnyPtr& param1 = null, const AnyPtr& param2 = null, int_t param3 = 0)
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

class XegExec{
public:

	struct Trans{
		XegElemPtr ch; // この文字集合が来たら
		int to; // 状態番号toの状態へ遷移
		SmartPtr<Trans> next; // 連結リスト
	};	

	typedef SmartPtr<Trans> TransPtr;

	struct NFA{

		NFA(const XegNodePtr& node);

		void add_transition(int from, const AnyPtr& ch, int to);

		void gen_nfa(int entry, const AnyPtr& t, int exit);

		void add_e_transition(int from, int to){
			add_transition(from, e, to);
		}
		
		struct State{
			TransPtr trans;
			int_t capture_kind;
			int_t capture_index;
			InternedStringPtr capture_name;
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
		XegNodePtr root_node;
		AC<State>::vector states;
		int cap_max, named_cap_max;
		ArrayPtr named_cap_list_;
		int start, final;

		enum{
			CAPTURE_NONE = 0,
			CAPTURE_BEGIN = 1,
			CAPTURE_END = 2,
			NAMED_CAPTURE_BEGIN = 3,
			NAMED_CAPTURE_END = 4,
		};
	};
	typedef SmartPtr<NFA> NFAPtr;

	MatchResultsPtr match(const XegNodePtr& pattern, const ScannerPtr& scanner){
		nfa_map_ = xnew<Map>();
		nodes_ = xnew<Array>();
		NFAPtr nfa = xnew<NFA>(pattern);
		if(match_inner(nfa, scanner)){
			MatchResultsPtr results = xnew<MatchResults>(info_.top().cap_values, info_.top().named_cap_values, nodes_);
			info_.downsize(1);
			return results;
		}
		info_.downsize(1);
		return null;
	}

private:

	bool match_inner(const NFAPtr& nfa, const ScannerPtr& scanner);

	bool test(const ScannerPtr& scanner, const XegElemPtr& elem);

	NFAPtr get_nfa(const XegNodePtr& node){
		const AnyPtr& temp = nfa_map_->at(node);
		NFAPtr nfa;
		if(temp){
			return static_ptr_cast<NFA>(temp);
		}else{
			NFAPtr nfa = xnew<NFA>(node);
			nfa_map_->set_at(node, nfa);
			return nfa;
		}
	}

	void push(int curSt, int pos);

	struct StackInfo{ int st, ps; };
	typedef Stack<StackInfo> stack_t;

	MapPtr nfa_map_;

	struct Info{
		stack_t stack;

		ArrayPtr cap;
		MapPtr named_cap;

		ArrayPtr cap_values;
		MapPtr named_cap_values;
	};

	Stack<MatchTreeNodePtr> tree_;
	ArrayPtr nodes_;

	struct Cap{
		int begin, end;
	};

	Stack<Info> info_;
};


XegNodePtr concat(const AnyPtr& left, const AnyPtr& right);

XegNodePtr term(const XegElemPtr& a){
	return xnew<XegNode>(XegNode::TYPE_TERM, a);
}

XegNodePtr P(const AnyPtr& a){
	if(XegNodePtr p = ptr_as<XegNode>(a)){
		return p;
	}

	if(ChRangePtr p = ptr_as<ChRange>(a)){
		SetPtr chset = xnew<Set>();
		Xfor(v, p){
			chset->set_at(v, true);
		}
		return term(xnew<XegElem>(XegElem::TYPE_CH_SET, chset));
	}

	if(StringPtr p = ptr_as<String>(a)){
		if(p->length()==1){
			return term(xnew<XegElem>(XegElem::TYPE_CH, p));
		}

		AnyPtr str = null;
		Xfor(v, p->each()){
			if(str){
				str = concat(str, v);
			}else{
				str = v;
			}
		}
		
		return P(str);
	}

	return null;
}

XegNodePtr or(const AnyPtr& left, const AnyPtr& right){
	return xnew<XegNode>(XegNode::TYPE_OR, P(left), P(right));
}

XegNodePtr concat(const AnyPtr& left, const AnyPtr& right){
	return xnew<XegNode>(XegNode::TYPE_CONCAT, P(left), P(right));
}

XegNodePtr emp(){
	return xnew<XegNode>(XegNode::TYPE_EMPTY);
}

XegNodePtr more_Int(const AnyPtr& left, int_t n, int_t kind = 0){
	if(n==0){
		return xnew<XegNode>(XegNode::TYPE_MORE0, P(left), null, kind);
	}else if(n==1){
		return xnew<XegNode>(XegNode::TYPE_MORE1, P(left), null, kind);
	}else if(n==-1){
		return xnew<XegNode>(XegNode::TYPE_01, P(left), null, kind);
	}

	if(n>0){
		return concat(left, more_Int(left, n-1, kind));
	}else{
		return concat(more_Int(left, -1, kind), more_Int(left, n+1, kind));
	}
}

XegNodePtr more_IntRange(const AnyPtr& left, const IntRangePtr& range, int_t kind = 0){
	if(range->begin()<=0){
		int n = -(range->end()-1);
		return n < 0 ? more_Int(left, n, kind) : emp();
	}

	return concat(left, more_IntRange(left, xnew<IntRange>(range->begin()-1, range->end()-1, RANGE_LEFT_CLOSED_RIGHT_OPEN), kind));
}

XegNodePtr more_normal_Int(const AnyPtr& left, int_t n){ return more_Int(left, n, 0); }
XegNodePtr more_shortest_Int(const AnyPtr& left, int_t n){ return more_Int(left, n, 1); }
XegNodePtr more_greed_Int(const AnyPtr& left, int_t n){ return more_Int(left, n, 2); }
XegNodePtr more_normal_IntRange(const AnyPtr& left, const IntRangePtr& range){ return more_IntRange(left, range, 0); }
XegNodePtr more_shortest_IntRange(const AnyPtr& left, const IntRangePtr& range){ return more_IntRange(left, range, 1); }
XegNodePtr more_greed_IntRange(const AnyPtr& left, const IntRangePtr& range){ return more_IntRange(left, range, 2); }

XegNodePtr inv(const AnyPtr& left){
	AnyPtr a = P(left);
	if(XegNodePtr p = ptr_as<XegNode>(a)){
		if(p->type==XegNode::TYPE_TERM){
			XegElemPtr elem = ptr_cast<XegElem>(p->param1);
			if(elem->type==XegElem::TYPE_CH){
				return term(xnew<XegElem>(XegElem::TYPE_NOT_CH, elem->param1)); 
			}

			if(elem->type==XegElem::TYPE_CH_SET){
				return term(xnew<XegElem>(XegElem::TYPE_NOT_CH_SET, elem->param1)); 
			}

			if(elem->type==XegElem::TYPE_BEFORE){
				return term(xnew<XegElem>(XegElem::TYPE_NOT_BEFORE, elem->param1)); 
			}
		}
	}

	return null;
}

XegNodePtr before(const AnyPtr& left){ return xnew<XegNode>(XegNode::TYPE_TERM, xnew<XegElem>(XegElem::TYPE_BEFORE, P(left))); }

XegNodePtr cap(const AnyPtr& left){ return xnew<XegNode>(XegNode::TYPE_CAP, P(left)); }
XegNodePtr named_cap(const InternedStringPtr& name, const AnyPtr& left){ return xnew<XegNode>(XegNode::TYPE_CAP, P(left), name, 1); }

void cap_vm(const VMachinePtr& vm){
	if(vm->named_arg_count()!=0){
		vm->return_result(named_cap(vm->arg_name(0), vm->arg(vm->arg_name(0))));
	}else{
		vm->return_result(cap(vm->arg(0)));
	}
}

XegNodePtr node(const AnyPtr& left){ 
	XegElemPtr temp = xnew<XegElem>(XegElem::TYPE_NODE, P(left));
	XegNodePtr ret = term(temp);
	temp->param2 = ret;
	return ret;
}

XegNodePtr leaf(const AnyPtr& left){ 
	XegElemPtr temp = xnew<XegElem>(XegElem::TYPE_LEAF, P(left));
	XegNodePtr ret = term(temp);
	return ret;
}

XegNodePtr backref(const AnyPtr& n){ 
	return term(xnew<XegElem>(XegElem::TYPE_BACKREF, n));
}

XegExec::NFA::NFA(const XegNodePtr& node){
	e = xnew<XegElem>(XegElem::TYPE_EPSILON);

	root_node = node;
	cap_max = 0;
	named_cap_max = 0;
	named_cap_list_ = xnew<Array>();

	start = gen_state();
	final = gen_state();
	gen_nfa(start, node, final);
}

void XegExec::NFA::add_transition(int from, const AnyPtr& ch, int to){
	TransPtr x = xnew<Trans>();
	x->to = to;
	x->ch = static_ptr_cast<XegElem>(ch);
	x->next = states[from].trans;
	states[from].trans = x;
}

void XegExec::NFA::gen_nfa(int entry, const AnyPtr& a, int exit){
	const XegNodePtr& t = static_ptr_cast<XegNode>(a);

	switch(t->type){
		XTAL_NODEFAULT;

		XTAL_CASE(XegNode::TYPE_TERM){
			//         ch
			//  entry -----> exit
			add_transition(entry, t->param1, exit);
		}

		XTAL_CASE(XegNode::TYPE_CONCAT){
			//         left         right
			//  entry ------> step -------> exit
			int step = gen_state();
			gen_nfa(entry, t->param1, step);
			gen_nfa(step, t->param2, exit);
		}

		XTAL_CASE(XegNode::TYPE_OR){
			//           left
			//        ------------->
			//  entry ----->-------> exit
			//          e    right

			int step = gen_state();
			gen_nfa(entry, t->param1, exit);
			add_e_transition(entry, step);
			gen_nfa(step, t->param2, exit);
		}

		XTAL_CASE2(XegNode::TYPE_MORE0, XegNode::TYPE_MORE1){
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
				if(t->type == XegNode::TYPE_MORE0){ add_e_transition(entry, exit); }
				add_e_transition(entry, before);

				add_e_transition(after, exit);
				add_e_transition(after, before);
			}else{
				// beforeに向かう方が優先
				add_e_transition(entry, before);
				if(t->type == XegNode::TYPE_MORE0){ add_e_transition(entry, exit); }

				add_e_transition(after, before);
				add_e_transition(after, exit);
			}

			gen_nfa(before, t->param1, after);

			if(t->param3==2){
				states[entry].greed = true;
				states[after].greed = true;
			}
		}

		XTAL_CASE(XegNode::TYPE_01){
			//           e
			//        ------>
			//  entry ------> exit
			//         left

			if(t->param3==1){
				// eを経由する方が優先
				add_e_transition(entry, exit);
				gen_nfa(entry, t->param1, exit);
			}else{
				// leftを経由する方が優先
				gen_nfa(entry, t->param1, exit);
				add_e_transition(entry, exit);
			}

			if(t->param3==2){
				states[entry].greed = true;
			}
		}

		XTAL_CASE(XegNode::TYPE_EMPTY){
			//         e
			//  entry ---> exit
			add_e_transition(entry, exit);
		}

		XTAL_CASE(XegNode::TYPE_CAP){
			int before = gen_state();
			int after = gen_state();

			if(t->param3==0){
				states[before].capture_kind = CAPTURE_BEGIN;
				states[after].capture_kind = CAPTURE_END;
				states[before].capture_index = cap_max;
				states[after].capture_index = cap_max;
				cap_max++;		
			}else{
				states[before].capture_kind = NAMED_CAPTURE_BEGIN;
				states[after].capture_kind = NAMED_CAPTURE_END;
				states[before].capture_name = static_ptr_cast<InternedString>(t->param2);
				states[after].capture_name = static_ptr_cast<InternedString>(t->param2);
				named_cap_list_->push_back(t->param2);
				named_cap_max++;
			}

			add_e_transition(entry, before);
			gen_nfa(before, t->param1, after);
			add_e_transition(after, exit);
		}
	}
}


void XegExec::push(int curSt, int pos){
	// ε無限ループ防止策。同じ状態には戻らないように…
	Info& info = info_.top();
	for(uint_t i=0, sz=info.stack.size(); i<sz; ++i){
		if(info.stack[i].ps != pos){
			break;
		}else if(info.stack[i].st == curSt){
			return;
		}
	}

	StackInfo nw = {curSt, pos};
	info.stack.push(nw);
}

bool XegExec::match_inner(const NFAPtr& nfa, const ScannerPtr& scanner){

	{
		Info& info = info_.push();
		if(nfa->cap_max!=0){ 
			info.cap = xnew<Array>(nfa->cap_max);
			for(int_t i=0, sz=nfa->cap_max; i<sz; ++i){
				info.cap->set_at(i, xnew<Cap>());
			}
		}

		if(nfa->named_cap_max!=0){ 
			info.named_cap = xnew<Map>(); 
			for(uint_t i=0, sz=nfa->named_cap_list_->size(); i<sz; ++i){
				info.named_cap->set_at(nfa->named_cap_list_->at(i), xnew<Cap>());
			}
		}
	}

	int matchpos = -1;

	int first_pos = scanner->pos();
	push(nfa->start, scanner->pos());

	while(!info_.top().stack.empty()){
		StackInfo& se = info_.top().stack.pop();
		int curSt = se.st;
		int pos = se.ps;
		NFA::State& state = nfa->states[curSt];

		if(curSt == nfa->final) // 1==終状態
		if(matchpos < pos){
			matchpos = pos;
		}

		bool fail = true;

		// さらに先の遷移を調べる
		for(const TransPtr* tr=&state.trans; *tr; tr=&(*tr)->next){
			scanner->seek(pos);
			if(test(scanner, (*tr)->ch)){
				fail = false;
				push((*tr)->to, scanner->pos());

				if(state.greed){
					break;
				}
			}
		}

		if(fail){
			if(matchpos>=0)
				break;
		}else{
			switch(state.capture_kind){
				XTAL_NODEFAULT;

				XTAL_CASE(NFA::CAPTURE_NONE){

				}

				XTAL_CASE(NFA::CAPTURE_BEGIN){
					const SmartPtr<Cap>& temp = static_ptr_cast<Cap>(info_.top().cap->at(state.capture_index));
					temp->begin = pos;
					temp->end = -1;
				}

				XTAL_CASE(NFA::CAPTURE_END){
					const SmartPtr<Cap>& temp = static_ptr_cast<Cap>(info_.top().cap->at(state.capture_index));
					temp->end = pos;
				}

				XTAL_CASE(NFA::NAMED_CAPTURE_BEGIN){
					const SmartPtr<Cap>& temp = static_ptr_cast<Cap>(info_.top().named_cap->at(state.capture_name));
					temp->begin = pos;
					temp->end = -1;
				}

				XTAL_CASE(NFA::NAMED_CAPTURE_END){
					const SmartPtr<Cap>& temp = static_ptr_cast<Cap>(info_.top().named_cap->at(state.capture_name));
					temp->end = pos;
				}
			}
		}
	}

	if(matchpos>=0){
		scanner->seek(matchpos);

		if(nfa->cap_max!=0){
			info_.top().cap_values = xnew<Array>();
			Xfor(v, info_.top().cap){
				const SmartPtr<Cap>& temp = static_ptr_cast<Cap>(v);

				if(temp->end>=0 && temp->end-temp->begin>0){
					info_.top().cap_values->push_back(scanner->capture(temp->begin, temp->end));
				}
			}
		}else{
			info_.top().cap_values = null;
		}

		if(nfa->named_cap_max!=0){
			info_.top().named_cap_values = xnew<Map>();
			Xfor2(k, v, info_.top().named_cap){
				const SmartPtr<Cap>& temp = static_ptr_cast<Cap>(v);

				if(temp->end>=0 && temp->end-temp->begin>0){
					info_.top().named_cap_values->set_at(k, scanner->capture(temp->begin, temp->end));
				}
			}
		}else{
			info_.top().named_cap_values = null;
		}

		return true;
	}

	scanner->seek(first_pos);
	return false;
}

bool XegExec::test(const ScannerPtr& scanner, const XegElemPtr& elem){
	switch(elem->type){
		XTAL_NODEFAULT;

		XTAL_CASE(XegElem::TYPE_EPSILON){
			return !scanner->eos();
		}

		XTAL_CASE(XegElem::TYPE_ANY){
			if(scanner->eos()){ return false; }
			scanner->read();
			return true;
		}

		XTAL_CASE(XegElem::TYPE_CH){
			if(raweq(scanner->read(), elem->param1)){ return true; }
			return false;
		}

		XTAL_CASE(XegElem::TYPE_NOT_CH){
			if(scanner->eos()){ 
				return false; 
			}
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

		XTAL_CASE(XegElem::TYPE_BEFORE){
			NFAPtr nfa = get_nfa(static_ptr_cast<XegNode>(elem->param1));
			if(match_inner(nfa, scanner)){
				info_.downsize(1);
				return true;
			}
			info_.downsize(1);
			return false;
		}

		XTAL_CASE(XegElem::TYPE_NOT_BEFORE){
			NFAPtr nfa = get_nfa(static_ptr_cast<XegNode>(elem->param1));
			if(match_inner(nfa, scanner)){
				info_.downsize(1);
				return false;
			}
			info_.downsize(1);
			return true;
		}

		XTAL_CASE(XegElem::TYPE_LEAF){
			NFAPtr nfa = get_nfa(static_ptr_cast<XegNode>(elem->param1));
			int_t pos = scanner->pos();
			if(match_inner(nfa, scanner)){
				if(tree_.empty()){
					nodes_->push_back(scanner->capture(pos, scanner->pos()));
				}else{
					tree_.top()->children_->push_back(scanner->capture(pos, scanner->pos()));
				}
				info_.downsize(1);
				return true;
			}
			info_.downsize(1);
			return false;
		}

		XTAL_CASE(XegElem::TYPE_NODE){
			NFAPtr nfa = get_nfa(static_ptr_cast<XegNode>(elem->param1));
			int_t pos = scanner->pos();

			MatchTreeNodePtr node = xnew<MatchTreeNode>();
			node->id_ = elem->param2;
			node->lineno_ = scanner->lineno();
			node->children_ = xnew<Array>();
			tree_.push(node);
			if(match_inner(nfa, scanner)){
				AnyPtr temp = tree_.pop();
				if(tree_.empty()){
					nodes_->push_back(temp);
				}else{
					tree_.top()->children_->push_back(temp);
				}
				info_.downsize(1);
				return true;
			}
			tree_.downsize(1);
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
	}

	return false;
}

MatchResultsPtr parse_scanner(const AnyPtr& pattern, const ScannerPtr& scanner){
	XegExec exec;
	return exec.match(P(pattern), scanner);
}

void def_common_method(const ClassPtr& p){
	p->method("op_mod", &more_shortest_Int, get_cpp_class<Int>());
	p->method("op_mod", &more_shortest_IntRange, get_cpp_class<IntRange>());
	p->method("op_mul", &more_normal_Int, get_cpp_class<Int>());
	p->method("op_mul", &more_normal_IntRange, get_cpp_class<IntRange>());
	p->method("op_pow", &more_greed_Int, get_cpp_class<Int>());
	p->method("op_pow", &more_greed_IntRange, get_cpp_class<IntRange>());
	p->method("op_com", &inv);
	
	p->method("op_or", &or, get_cpp_class<XegNode>());
	p->method("op_or", &or, get_cpp_class<String>());
	p->method("op_or", &or, get_cpp_class<ChRange>());
	p->method("op_shr", &concat, get_cpp_class<XegNode>());
	p->method("op_shr", &concat, get_cpp_class<String>());
	p->method("op_shr", &concat, get_cpp_class<ChRange>());
}

}

void initialize_xeg(){
	using namespace xeg;
	register_uninitializer(&uninitialize_xeg);

	ClassPtr xeg = xnew<Class>("xeg");

	{
		ClassPtr p = new_cpp_class<MatchTreeNode>("MatchTreeNode");
		p->method("id", &MatchTreeNode::id);
		p->method("lineno", &MatchTreeNode::lineno);
		p->method("each", &MatchTreeNode::each);
		p->method("op_at", &MatchTreeNode::op_at, get_cpp_class<Int>());
	}

	{
		ClassPtr p = new_cpp_class<MatchResults>("MatchResults");
		p->method("captures", &MatchResults::captures);
		p->method("named_captures", &MatchResults::named_captures);
		p->method("nodes", &MatchResults::nodes);
		p->method("root", &MatchResults::root);
		p->method("op_at", (AnyPtr (MatchResults::*)(int_t))&MatchResults::op_at, get_cpp_class<Int>());
		p->method("op_at", (AnyPtr (MatchResults::*)(const StringPtr&))&MatchResults::op_at, get_cpp_class<String>());
	}

	{
		ClassPtr p = new_cpp_class<XegNode>("XegNode");
		def_common_method(p);
	}

	{
		ClassPtr p = new_cpp_class<ChRange>();
		def_common_method(p);
	}

	{
		ClassPtr p = new_cpp_class<String>();
		def_common_method(p);
	}

	xeg->fun("parse_scanner", &parse_scanner);
	xeg->fun("parse_stream", &parse_stream);
	xeg->fun("parse_string", &parse_string);
	xeg->fun("parse_iterator", &parse_iterator);

	xeg->fun("backref", &backref);
	xeg->fun("before", &before);
	xeg->fun("leaf", &leaf);
	xeg->fun("node", &node);
	xeg->fun("cap", &cap_vm);

	builtin()->def("xeg", xeg);
}

void test_xeg(){
	return;

	using namespace xeg;
	
	XegNodePtr paren = P(Xsrc((  

		filelocal.inherit(xeg);
	builtin::int: node(leaf("0".."9"**1));
	builtin::mul: int >> (leaf("*") >> int)**0;
	builtin::add: mul >> (leaf("+") >> mul)**0;
	builtin::expr: node(add);

	g: class{}
	filelocal.inherit(g);
	g::test3: leaf(cap("a".."z"*2..2) >> backref(0))*0;

	s: MemoryStream();
	100000{
		s.put_s("test:aiu:ewara:aeer:");
	}
	s.put_s("e");
	s.size/1024.0 .p;
	s.seek(0);
	
	t: clock();
	if(result: xeg::parse_stream(leaf("a".."z"*0) >> (":" >> leaf("a".."z"*0))*0, s)){
		(clock()-t).p;
		c: result.nodes{
			//it.p;
		}
	}
	))());
}

}
