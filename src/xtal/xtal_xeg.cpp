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


struct XegElem{

	enum Type{
		TYPE_EPSILON,
		TYPE_ANY,

		TYPE_CH,
		TYPE_NOT_CH,
		TYPE_CH_SET,
		TYPE_NOT_CH_SET,

		TYPE_KEEP,

		TYPE_AFTER,
		TYPE_NOT_AFTER,
	};

	Type type;
	AnyPtr param;

	XegElem(Type type, const AnyPtr& param = null)
		:type(type), param(param){}
};

typedef SmartPtr<XegElem> XegElemPtr;

struct XegNode; 
typedef SmartPtr<XegNode> XegNodePtr;

struct XegNode{

	enum Type{
		TYPE_LEAF,     //
		TYPE_CONCAT,   // 連接       (left, right)
		TYPE_OR,       // |          (left, right)

		TYPE_MORE0,    // *          (left)
		TYPE_MORE1,    // +          (left)
		TYPE_01,       // ?          (left)

		TYPE_MORE0_SHORTEST,    // *          (left)
		TYPE_MORE1_SHORTEST,    // +          (left)
		TYPE_01_SHORTEST,       // ?          (left)

		TYPE_EMPTY,    // 空         (--)

		TYPE_CAP, // キャプチャー
		TYPE_NAMED_CAP, // キャプチャー
		TYPE_NODE,
	};

	XegNode(Type type, const AnyPtr& param1 = null, const AnyPtr& param2 = null)
		:type_(type), param1_(param1), param2_(param2){}

	Type type(){ return type_; }
	const AnyPtr& param1(){ return ptr_cast<XegNode>(param1_); }
	const AnyPtr& param2(){ return ptr_cast<XegNode>(param2_); }

	const XegNodePtr& left(){ return ptr_cast<XegNode>(param1_); }
	const XegNodePtr& right(){ return ptr_cast<XegNode>(param2_); }

	const XegElemPtr& ch(){ return ptr_cast<XegElem>(param1_); }
	const InternedStringPtr& name(){ return ptr_cast<InternedString>(param2_); }

private:

	Type type_;
	AnyPtr param1_;
	AnyPtr param2_;
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

XegNodePtr keep(const AnyPtr& left){
	return xnew<XegNode>(XegNode::TYPE_LEAF, xnew<XegElem>(XegElem::TYPE_KEEP, left));
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

XegNodePtr more_Int(const AnyPtr& left, int_t n){
	if(n==0){
		return xnew<XegNode>(XegNode::TYPE_MORE0, P(left));
	}else if(n==1){
		return xnew<XegNode>(XegNode::TYPE_MORE1, P(left));
	}else if(n==-1){
		return xnew<XegNode>(XegNode::TYPE_01, P(left));
	}

	if(n>0){
		return concat(left, more_Int(left, n-1));
	}else{
		return concat(more_Int(left, -1), more_Int(left, n+1));
	}
}

XegNodePtr more_shortest_Int(const AnyPtr& left, int_t n){
	if(n==0){
		return xnew<XegNode>(XegNode::TYPE_MORE0_SHORTEST, P(left));
	}else if(n==1){
		return xnew<XegNode>(XegNode::TYPE_MORE1_SHORTEST, P(left));
	}else if(n==-1){
		return xnew<XegNode>(XegNode::TYPE_01_SHORTEST, P(left));
	}

	if(n>0){
		return concat(left, more_shortest_Int(left, n-1));
	}else{
		return concat(more_shortest_Int(left, -1), more_shortest_Int(left, n+1));
	}
}

XegNodePtr more_greed_Int(const AnyPtr& left, int_t n){
	return keep(more_Int(left, n));
}

XegNodePtr more_IntRange(const AnyPtr& left, const IntRangePtr& range){
	if(range->begin()<=0){
		int n = -(range->end()-1);
		return n < 0 ? more_Int(left, n) : emp();
	}

	return concat(left, more_IntRange(left, xnew<IntRange>(range->begin()-1, range->end()-1, RANGE_LEFT_CLOSED_RIGHT_OPEN)));
}

XegNodePtr more_shortest_IntRange(const AnyPtr& left, const IntRangePtr& range){
	if(range->begin()<=0){
		int n = -(range->end()-1);
		return n < 0 ? more_shortest_Int(left, n) : emp();
	}

	return concat(left, more_shortest_IntRange(left, xnew<IntRange>(range->begin()-1, range->end()-1, RANGE_LEFT_CLOSED_RIGHT_OPEN)));
}

XegNodePtr more_greed_IntRange(const AnyPtr& left, const IntRangePtr& range){
	return keep(more_IntRange(left, range));
}

XegNodePtr inv(const AnyPtr& left){
	AnyPtr a = P(left);
	if(XegNodePtr p = ptr_as<XegNode>(a)){
		if(p->type()==XegNode::TYPE_LEAF && p->ch()->type==XegElem::TYPE_CH){
			return leaf(xnew<XegElem>(XegElem::TYPE_NOT_CH, p->ch()->param)); 
		}

		if(p->type()==XegNode::TYPE_LEAF && p->ch()->type==XegElem::TYPE_CH_SET){
			return leaf(xnew<XegElem>(XegElem::TYPE_NOT_CH_SET, p->ch()->param)); 
		}

		if(p->type()==XegNode::TYPE_LEAF && p->ch()->type==XegElem::TYPE_AFTER){
			return leaf(xnew<XegElem>(XegElem::TYPE_NOT_AFTER, p->ch()->param)); 
		}
	}

	return null;
}

XegNodePtr after(const AnyPtr& left){
	return xnew<XegNode>(XegNode::TYPE_LEAF, xnew<XegElem>(XegElem::TYPE_AFTER, P(left)));
}

/*
XegNodePtr by_ref(const XegNodePtr& left){
	XegNodePtr ret = xnew<XegNode>();
	ret->type = XegNode::TYPE_LEAF;
	ret->ch = xnew<XegElem>(XegElem::TYPE_BY_REF, left);
	return ret;
}

XegNodePtr node(){
	XegNodePtr ret = xnew<XegNode>();
	ret->type = XegNode::TYPE_NODE;
	ret->ch = xnew<XegElem>(XegElem::TYPE_EPSILON);
	return ret;
}
*/

XegNodePtr C(const AnyPtr& left){
	return xnew<XegNode>(XegNode::TYPE_CAP, P(left));
}

XegNodePtr Cn(const InternedStringPtr& name, const AnyPtr& left){
	return xnew<XegNode>(XegNode::TYPE_NAMED_CAP, P(left), name);
}

struct XegTrans;
typedef SmartPtr<XegTrans> XegTransPtr;

struct XegTrans{
	// この文字集合が来たら
	XegElemPtr ch; 

	// 状態番号toの状態へ遷移
	int to; 

	// 連結リスト
	XegTransPtr next;
};

class XegNFA{
public:

	void make(const XegNodePtr& node);
	int match(const ScannerPtr& scanner);

private:

	struct StackInfo{ int st, ps; };
	typedef Stack<StackInfo> stack_t;

	void push(stack_t& stack, int curSt, int pos);

private:

	void add_transition(int from, const XegElemPtr& ch, int to);
	void gen_nfa(int entry, const XegNodePtr& t, int exit);

	void add_e_transition(int from, int to){
		add_transition(from, e_, to);
	}

	struct State{
		XegTransPtr trans;
		int flags;
		int no;
		InternedStringPtr name;
	};

	AC<State>::vector states_;

	int gen_state(){
		State state;
		state.flags = 0;
		states_.push_back(state);
		return states_.size() - 1;
	}

	bool test(const ScannerPtr& scanner, const XegElemPtr& elem);

private:
	XegElemPtr e_;

	struct Cap{
		int begin, end;
	};

	enum{
		FLAG_NONE = 0,
		FLAG_CAP_BEGIN = 1,
		FLAG_CAP_END = 2,
		FLAG_NAMED_CAP_BEGIN = 3,
		FLAG_NAMED_CAP_END = 4,
	};

	int cap_max_;
	int start_, final_;
};

void XegNFA::make(const XegNodePtr& node){
	e_ = xnew<XegElem>(XegElem::TYPE_EPSILON);
	cap_max_ = 0;
	start_ = gen_state();
	final_ = gen_state();
	gen_nfa(start_, node, final_);
}

void XegNFA::add_transition(int from, const XegElemPtr& ch, int to){
	XegTransPtr x = xnew<XegTrans>();
	x->to = to;
	x->ch = ch;
	x->next = states_[from].trans;
	states_[from].trans = x;
}

void XegNFA::gen_nfa(int entry, const XegNodePtr& t, int exit){
	switch(t->type()){
		XTAL_NODEFAULT;

		XTAL_CASE(XegNode::TYPE_LEAF){
			//         ch
			//  entry -----> exit
			add_transition(entry, t->ch(), exit);
		}

		XTAL_CASE(XegNode::TYPE_CONCAT){
			//         left         right
			//  entry ------> step -------> exit
			int step = gen_state();
			gen_nfa(entry, t->left(), step);
			gen_nfa(step, t->right(), exit);
		}

		XTAL_CASE(XegNode::TYPE_OR){
			//           left
			//        ------------->
			//  entry ----->-------> exit
			//          e    right

			int step = gen_state();
			gen_nfa(entry, t->left(), exit);
			add_e_transition(entry, step);
			gen_nfa(step, t->right(), exit);
		}

		XTAL_CASE2(XegNode::TYPE_MORE0, XegNode::TYPE_MORE1){
			//                       e
			//         e          <------        e
			//  entry ---> before ------> after ---> exit
			//    |                left                ^
			//    >------->------------------->------>-|
			//                      e


			//                       e
			//         e          <------        e
			//  entry ---> before ------> after ---> exit
			//                     left
			int before = gen_state();
			int after = gen_state();
			
			// beforeに向かう方が優先
			add_e_transition(entry, before);
			if(t->type() == XegNode::TYPE_MORE0){
				add_e_transition(entry, exit);
			}

			// beforeに向かう方が優先
			add_e_transition(after, before);
			add_e_transition(after, exit);

			gen_nfa(before, t->left(), after);
		}

		XTAL_CASE(XegNode::TYPE_01){
			//           e
			//        ------>
			//  entry ------> exit
			//         left

			// leftを経由する方が優先
			gen_nfa(entry, t->left(), exit);
			add_e_transition(entry, exit);
		}

		XTAL_CASE2(XegNode::TYPE_MORE0_SHORTEST, XegNode::TYPE_MORE1_SHORTEST){
			//                       e
			//         e          <------        e
			//  entry ---> before ------> after ---> exit
			//    |                left                ^
			//    >------->------------------->------>-|
			//                      e


			//                       e
			//         e          <------        e
			//  entry ---> before ------> after ---> exit
			//                     left
			int before = gen_state();
			int after = gen_state();
			
			// exitに向かう方が優先
			if(t->type() == XegNode::TYPE_MORE0_SHORTEST){
				add_e_transition(entry, exit);
			}
			add_e_transition(entry, before);

			// exitに向かう方が優先
			add_e_transition(after, exit);
			add_e_transition(after, before);

			gen_nfa(before, t->left(), after);
		}

		XTAL_CASE(XegNode::TYPE_01_SHORTEST){
			//           e
			//        ------>
			//  entry ------> exit
			//         left

			// eを経由する方が優先
			add_e_transition(entry, exit);
			gen_nfa(entry, t->left(), exit);
		}

		XTAL_CASE(XegNode::TYPE_EMPTY){
			//         e
			//  entry ---> exit
			add_e_transition(entry, exit);
		}

		XTAL_CASE(XegNode::TYPE_NODE){
			gen_nfa(entry, t->left(), exit);
		}

		XTAL_CASE(XegNode::TYPE_CAP){
			int before = gen_state();
			int after = gen_state();

			states_[before].flags = FLAG_CAP_BEGIN;
			states_[before].no = cap_max_;
			states_[after].flags = FLAG_CAP_END;
			states_[after].no = cap_max_;
			cap_max_++;		

			add_e_transition(entry, before);
			gen_nfa(before, t->left(), after);
			add_e_transition(after, exit);
		}

		XTAL_CASE(XegNode::TYPE_NAMED_CAP){
			int before = gen_state();
			int after = gen_state();

			states_[before].flags = FLAG_NAMED_CAP_BEGIN;
			states_[before].name = t->name();
			states_[after].flags = FLAG_NAMED_CAP_END;
			states_[after].name = t->name();

			add_e_transition(entry, before);
			gen_nfa(before, t->left(), after);
			add_e_transition(after, exit);
		}
	}
}


void XegNFA::push(stack_t& stack, int curSt, int pos){
	// ε無限ループ防止策。同じ状態には戻らないように…
	for(uint_t i=0, sz=stack.size(); i<sz; ++i){
		if(stack[i].ps != pos){
			break;
		}else if(stack[i].st == curSt){
			return;
		}
	}

	StackInfo nw = {curSt, pos};
	stack.push(nw);
}

int XegNFA::match(const ScannerPtr& scanner){
	int matchpos = -1;

	stack_t stack;
	push(stack, start_, scanner->pos());
	ArrayPtr cap = xnew<Array>(cap_max_);
	MapPtr named_cap = xnew<Map>();

	while(!stack.empty()){
		StackInfo& se = stack.pop();
		int curSt = se.st;
		int pos = se.ps;
		State& state = states_[curSt];

		if(curSt == final_) // 1==終状態
		if(matchpos < pos){
			matchpos = pos;
		}

		bool fail = true;

		// さらに先の遷移を調べる
		for(const XegTransPtr* tr=&state.trans; *tr; tr=&(*tr)->next){
			scanner->seek(pos);
			if(test(scanner, (*tr)->ch)){
				fail = false;
				push(stack, (*tr)->to, scanner->pos());
			}
		}

		if(fail){
			if(matchpos>=0)
				break;
		}else{
			switch(state.flags){
				XTAL_NODEFAULT;

				XTAL_CASE(FLAG_NONE){

				}

				XTAL_CASE(FLAG_CAP_BEGIN){
					SmartPtr<Cap> temp = xnew<Cap>();
					temp->begin = pos;
					temp->end = -1;
					cap->set_at(state.no, temp);
				}

				XTAL_CASE(FLAG_CAP_END){
					ptr_cast<Cap>(cap->at(state.no))->end = pos;
				}

				XTAL_CASE(FLAG_NAMED_CAP_BEGIN){
					SmartPtr<Cap> temp = xnew<Cap>();
					temp->begin = pos;
					temp->end = -1;
					named_cap->set_at(state.name, temp);
				}

				XTAL_CASE(FLAG_NAMED_CAP_END){
					ptr_cast<Cap>(named_cap->at(state.name))->end = pos;
				}
			}
		}
	}



	if(matchpos>=0){
		Xfor(v, cap){
			if(SmartPtr<Cap> temp = ptr_as<Cap>(v)){
				if(temp->end>=0 && temp->end-temp->begin>0){
					Xf("%d ..< %d => %s\n")(temp->begin, temp->end, scanner->capture(temp->begin, temp->end))->p();
				}
			}
		}

		Xfor2(k, v, named_cap){
			if(SmartPtr<Cap> temp = ptr_as<Cap>(v)){
				if(temp->end>=0 && temp->end-temp->begin>0){
					Xf("%s=%d ..< %d => %s\n")(k, temp->begin, temp->end, scanner->capture(temp->begin, temp->end))->p();
				}
			}
		}

		scanner->seek(matchpos);
	}

	return matchpos;
}

bool XegNFA::test(const ScannerPtr& scanner, const XegElemPtr& elem){
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

		XTAL_CASE(XegElem::TYPE_KEEP){
			XegNFA nfa;
			nfa.make(static_ptr_cast<XegNode>(elem->param));
			int i = nfa.match(scanner);
			return i>=0;
		}

		XTAL_CASE(XegElem::TYPE_AFTER){
			int pos = scanner->pos();
			XegNFA nfa;
			nfa.make(static_ptr_cast<XegNode>(elem->param));
			int i = nfa.match(scanner);
			scanner->seek(pos);
			return i>=0;
		}

		XTAL_CASE(XegElem::TYPE_NOT_AFTER){
			int pos = scanner->pos();
			XegNFA nfa;
			nfa.make(static_ptr_cast<XegNode>(elem->param));
			int i = nfa.match(scanner);
			scanner->seek(pos);
			return i<0;
		}
	}

	return false;
}

}

void initialize_xeg(){
	using namespace xeg;
	register_uninitializer(&uninitialize_xeg);

	ClassPtr xeg = xnew<Class>("xeg");

	{
		ClassPtr p = new_cpp_class<XegNode>("XegNode");
		p->method("op_add", &more_shortest_Int, get_cpp_class<Int>());
		p->method("op_add", &more_shortest_IntRange, get_cpp_class<IntRange>());
		p->method("op_mul", &more_Int, get_cpp_class<Int>());
		p->method("op_mul", &more_IntRange, get_cpp_class<IntRange>());
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
		p->method("op_add", &more_shortest_Int, get_cpp_class<Int>());
		p->method("op_add", &more_shortest_IntRange, get_cpp_class<IntRange>());
		p->method("op_mul", &more_Int, get_cpp_class<Int>());
		p->method("op_mul", &more_IntRange, get_cpp_class<IntRange>());
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
		p->method("op_add", &more_shortest_Int, get_cpp_class<Int>());
		p->method("op_add", &more_shortest_IntRange, get_cpp_class<IntRange>());
		p->method("op_mul", &more_Int, get_cpp_class<Int>());
		p->method("op_mul", &more_IntRange, get_cpp_class<IntRange>());
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

	xeg->fun("after", &after);
	xeg->fun("C", &C);
	xeg->fun("Cn", &Cn);

	builtin()->def("xeg", xeg);
}

void test_xeg(){
	using namespace xeg;
	
	StringStreamPtr sstream = xnew<StringStream>("aaabaaaa");
	StreamScannerPtr ss = xnew<StreamScanner>(sstream);

	// n: XegExpr();
	// n.body = 

	XegNFA nfa;
	//nfa.make(concat(more0(or(ch("a"), ch("b"))), concat(by_ref(more0(ch("c"))), more1(ch("d")))) );
	//XegNodePtr paren = node();
	//paren->left = more0(concat(ch("("), concat(by_ref(paren), ch(")"))));
	// paren.impl = ( "(" >> r(paren) >> ")" )*0;

	//XegNodePtr paren = concat(C(more1(ch("a"))), concat(C(more0(or(ch("a"), ch("b")))), C(more1(ch("a")))));
	//XegNodePtr paren = P(Xsrc(( return xeg::C(("a"|"b")+0) >> ~xeg::after(~"." *0 >> "bbb")))());
	XegNodePtr paren = P(Xsrc((  
		return xeg::Cn("test", "a".."b"*0..5) >> xeg::Cn("foo", "ba");
	))());
	nfa.make(paren);

	int i = nfa.match(ss);

	printf("%d\n", i);
	printf("%d\n", i);
}

}
