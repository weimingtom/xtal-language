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
		if(stream_->eof()){
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

struct XegElem{

	enum Type{
		TYPE_EPSILON,
		TYPE_ANY,

		// à»â∫paramÇÕStringÇ≈Ç†ÇÈéÌóﬁ
		TYPE_CH,
		TYPE_NOT_CH,

		// à»â∫paramÇÕSetÇ≈Ç†ÇÈéÌóﬁ
		TYPE_CH_SET,
		TYPE_NOT_CH_SET,

		// à»â∫paramÇÕXegNodeÇ≈Ç†ÇÈéÌóﬁ
		TYPE_AFTER,
		TYPE_NOT_AFTER,

		TYPE_NODE,
	};

	Type type;
	AnyPtr param;

	XegElem(Type type, const AnyPtr& param = null)
		:type(type), param(param){}
};

struct XegNode{

	enum Type{
		TYPE_LEAF, //

		TYPE_CONCAT, // >>
		TYPE_OR, // |

		TYPE_MORE0, // *0
		TYPE_MORE1, // *1
		TYPE_01,  // *-1

		TYPE_EMPTY, // ãÛ

		TYPE_CAP, // ÉLÉÉÉvÉ`ÉÉ
	};

	XegNode(int_t type, const AnyPtr& param1 = null, const AnyPtr& param2 = null, int_t param3 = 0)
		:type(type), param1(param1), param2(param2), param3(param3){}

	int_t type;
	AnyPtr param1;
	AnyPtr param2;
	int_t param3;
};

class XegExec{
public:

	struct Trans{
		// Ç±ÇÃï∂éöèWçáÇ™óàÇΩÇÁ
		XegElemPtr ch; 

		// èÛë‘î‘çÜtoÇÃèÛë‘Ç÷ëJà⁄
		int to; 

		// òAåãÉäÉXÉg
		SmartPtr<Trans> next;
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
		int cap_max;
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
		NFAPtr nfa = xnew<NFA>(pattern);
		if(match_inner(nfa, scanner)){
			return xnew<MatchResults>(info_.top().cap_values, info_.top().named_cap_values, info_.top().root);
		}
		return null;
	}

private:

	bool match_inner(const NFAPtr& nfa, const ScannerPtr& scanner);

	bool test(const NFAPtr& nfa, const ScannerPtr& scanner, const XegElemPtr& elem);

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

		MatchTreeNodePtr root;
	};

	struct Cap{
		int begin, end;
	};

	Stack<Info> info_;
};


XegNodePtr concat(const AnyPtr& left, const AnyPtr& right);

XegNodePtr leaf(const XegElemPtr& a){
	return xnew<XegNode>(XegNode::TYPE_LEAF, a);
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
		return leaf(xnew<XegElem>(XegElem::TYPE_CH_SET, chset));
	}

	if(StringPtr p = ptr_as<String>(a)){
		if(p->length()==1){
			return leaf(xnew<XegElem>(XegElem::TYPE_CH, p));
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
		return xnew<XegNode>(XegNode::TYPE_MORE0, P(left), kind);
	}else if(n==1){
		return xnew<XegNode>(XegNode::TYPE_MORE1, P(left), kind);
	}else if(n==-1){
		return xnew<XegNode>(XegNode::TYPE_01, P(left), kind);
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
		if(p->type==XegNode::TYPE_LEAF){
			XegElemPtr elem = ptr_cast<XegElem>(p->param1);
			if(elem->type==XegElem::TYPE_CH){
				return leaf(xnew<XegElem>(XegElem::TYPE_NOT_CH, elem->param)); 
			}

			if(elem->type==XegElem::TYPE_CH_SET){
				return leaf(xnew<XegElem>(XegElem::TYPE_NOT_CH_SET, elem->param)); 
			}

			if(elem->type==XegElem::TYPE_AFTER){
				return leaf(xnew<XegElem>(XegElem::TYPE_NOT_AFTER, elem->param)); 
			}
		}
	}

	return null;
}

XegNodePtr after(const AnyPtr& left){ return xnew<XegNode>(XegNode::TYPE_LEAF, xnew<XegElem>(XegElem::TYPE_AFTER, P(left))); }
XegNodePtr node(const AnyPtr& left){ return xnew<XegNode>(XegNode::TYPE_LEAF, xnew<XegElem>(XegElem::TYPE_NODE, P(left))); }
XegNodePtr C(const AnyPtr& left){ return xnew<XegNode>(XegNode::TYPE_CAP, P(left)); }
XegNodePtr Cn(const InternedStringPtr& name, const AnyPtr& left){ return xnew<XegNode>(XegNode::TYPE_CAP, P(left), name, 1); }


XegExec::NFA::NFA(const XegNodePtr& node){
	e = xnew<XegElem>(XegElem::TYPE_EPSILON);

	root_node = node;
	cap_max = 0;

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

		XTAL_CASE(XegNode::TYPE_LEAF){
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
				// exitÇ…å¸Ç©Ç§ï˚Ç™óDêÊ
				if(t->type == XegNode::TYPE_MORE0){ add_e_transition(entry, exit); }
				add_e_transition(entry, before);

				add_e_transition(after, exit);
				add_e_transition(after, before);
			}else{
				// beforeÇ…å¸Ç©Ç§ï˚Ç™óDêÊ
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
				// eÇåoóRÇ∑ÇÈï˚Ç™óDêÊ
				add_e_transition(entry, exit);
				gen_nfa(entry, t->param1, exit);
			}else{
				// leftÇåoóRÇ∑ÇÈï˚Ç™óDêÊ
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
			}

			add_e_transition(entry, before);
			gen_nfa(before, t->param1, after);
			add_e_transition(after, exit);
		}
	}
}


void XegExec::push(int curSt, int pos){
	// É√ñ≥å¿ÉãÅ[Évñhé~çÙÅBìØÇ∂èÛë‘Ç…ÇÕñﬂÇÁÇ»Ç¢ÇÊÇ§Ç…Åc
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
		info.cap = xnew<Array>(nfa->cap_max);
		info.named_cap = xnew<Map>();
	}

	int matchpos = -1;

	int first_pos = scanner->pos();
	push(nfa->start, scanner->pos());

	while(!info_.top().stack.empty()){
		StackInfo& se = info_.top().stack.pop();
		int curSt = se.st;
		int pos = se.ps;
		NFA::State& state = nfa->states[curSt];

		if(curSt == nfa->final) // 1==èIèÛë‘
		if(matchpos < pos){
			matchpos = pos;
		}

		bool fail = true;

		// Ç≥ÇÁÇ…êÊÇÃëJà⁄Çí≤Ç◊ÇÈ
		for(const TransPtr* tr=&state.trans; *tr; tr=&(*tr)->next){
			scanner->seek(pos);
			if(test(nfa, scanner, (*tr)->ch)){
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

		if(!info_.top().cap->empty()){
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

		if(!info_.top().named_cap->empty()){
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

bool XegExec::test(const NFAPtr& pnfa, const ScannerPtr& scanner, const XegElemPtr& elem){
	switch(elem->type){
		XTAL_NODEFAULT;

		XTAL_CASE(XegElem::TYPE_EPSILON){
			return !scanner->eof();
		}

		XTAL_CASE(XegElem::TYPE_ANY){
			if(scanner->eof()){ return false; }
			scanner->read();
			return true;
		}

		XTAL_CASE(XegElem::TYPE_CH){
			if(raweq(scanner->read(), elem->param)){ return true; }
			return false;
		}

		XTAL_CASE(XegElem::TYPE_NOT_CH){
			if(scanner->eof()){ 
				return false; 
			}
			if(rawne(scanner->read(), elem->param)){ return true; }
			return false;
		}

		XTAL_CASE(XegElem::TYPE_CH_SET){
			const MapPtr& data = static_ptr_cast<Map>(elem->param);
			if(data->at(scanner->read())){ return true; }
			return false;
		}

		XTAL_CASE(XegElem::TYPE_NOT_CH_SET){
			if(scanner->eof()){ return false; }
			const MapPtr& data = static_ptr_cast<Map>(elem->param);
			if(!data->at(scanner->read())){ return true; }
			return false;
		}

		XTAL_CASE(XegElem::TYPE_AFTER){
			NFAPtr nfa = get_nfa(static_ptr_cast<XegNode>(elem->param));
			if(match_inner(nfa, scanner)){
				info_.downsize(1);
				return true;
			}
			info_.downsize(1);
			return false;
		}

		XTAL_CASE(XegElem::TYPE_NOT_AFTER){
			NFAPtr nfa = get_nfa(static_ptr_cast<XegNode>(elem->param));
			if(match_inner(nfa, scanner)){
				info_.downsize(1);
				return false;
			}
			info_.downsize(1);
			return true;
		}

		XTAL_CASE(XegElem::TYPE_NODE){
			NFAPtr nfa = get_nfa(static_ptr_cast<XegNode>(elem->param));
			int_t pos = scanner->pos();
			if(match_inner(nfa, scanner)){
				Info& first = info_[0];
				Info& second = info_[1];
				if(!second.root){
					second.root = xnew<MatchTreeNode>();
					second.root->id_ = pnfa->root_node;
					second.root->lineno_ = 0;
					second.root->children_ = xnew<Array>();
				}

				if(first.root){
					second.root->children_->push_back(first.root);
				}else{
					second.root->children_->push_back(scanner->capture(pos, scanner->pos()));
				}
				info_.downsize(1);
				return true;
			}
			info_.downsize(1);
			return false;
		}
	}

	return false;
}

MatchResultsPtr parse_scanner(const AnyPtr& pattern, const ScannerPtr& scanner){
	XegExec exec;
	return exec.match(P(pattern), scanner);
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
		p->method("children", &MatchTreeNode::children);
	}

	{
		ClassPtr p = new_cpp_class<MatchResults>("MatchResults");
		p->method("captures", &MatchResults::captures);
		p->method("named_captures", &MatchResults::named_captures);
		p->method("root", &MatchResults::root);
	}

	{
		ClassPtr p = new_cpp_class<XegNode>("XegNode");
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

	{
		ClassPtr p = new_cpp_class<ChRange>();
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

	{
		ClassPtr p = new_cpp_class<String>();
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

	xeg->fun("parse_scanner", &parse_scanner);
	xeg->fun("parse_stream", &parse_stream);
	xeg->fun("parse_string", &parse_string);
	xeg->fun("parse_iterator", &parse_iterator);

	xeg->fun("after", &after);
	xeg->fun("node", &node);
	xeg->fun("C", &C);
	xeg->fun("Cn", &Cn);

	builtin()->def("xeg", xeg);
}

void test_xeg(){
	using namespace xeg;
	
	XegNodePtr paren = P(Xsrc((  

		filelocal.inherit(xeg);
int: node("0".."9"**1);
mul: int >> (node("*") >> int)**0;
add: mul >> (node("+") >> mul)**0;
expr: node(add);

results: xeg::parse_string(expr, "10+10");
	if(results){
		c: results.root.children[];
		c.size.p;
		c[0].children[][0].p;;
	}
	))());
}

}
