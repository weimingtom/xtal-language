#include "xtal.h"
#include "xtal_macro.h"
	
namespace xtal{ namespace peg{

AnyPtr any;
AnyPtr eof;
AnyPtr success;
AnyPtr fail;
AnyPtr lineno;
AnyPtr cn_any;
AnyPtr ch_alpha;
AnyPtr ch_lalpha;
AnyPtr ch_ualpha;
AnyPtr ch_space;
AnyPtr ch_digit;
AnyPtr ch_ascii;

}

namespace{

void uninitialize_peg(){
	using namespace peg;

	any = null;
	eof = null;
	success = null;
	fail = null;
	lineno = null;

	ch_alpha = null;
	ch_lalpha = null;
	ch_ualpha = null;
	ch_space = null;
	ch_digit = null;
	ch_ascii = null;
}

}

namespace peg{

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

enum PartsType{
	PARTS_TEST_STRING,
	PARTS_TEST_CH,
	PARTS_TEST_CH_SET,
	PARTS_END,
	PARTS_ANY,
	PARTS_FAIL,
	PARTS_SUCCESS,
	PARTS_SELECT,
	PARTS_FOLLOWED,
	PARTS_MORE,
	PARTS_JOIN,
	PARTS_ARRAY,
	PARTS_IGNORE,
	PARTS_CH_MAP,
	PARTS_NODE,
	PARTS_VAL,
	PARTS_NOT,
	PARTS_AND,
	PARTS_TEST,
	PARTS_SUB,
	PARTS_ASCII,
	PARTS_LINENO,
	PARTS_ACT,
	PARTS_ACTMV,
	PARTS_RE,
};

class Parts : public Base{
public:

	Parts(int_t type = 0, const AnyPtr& p1 = null, const AnyPtr& p2 = null)
		:type(type), p1(p1), p2(p2){}

	int_t type;
	AnyPtr p1;
	AnyPtr p2;

	virtual void visit_members(Visitor& m){
		Base::visit_members(m);
		m & p1 & p2;
	}
};

typedef SmartPtr<Parts> PartsPtr;

MapPtr make_ch_map2(const StringPtr& ch, const PartsPtr& pp){
	MapPtr data = xnew<Map>();
	data->set_at(ch, pp);
	return data;
}

MapPtr make_ch_map2(const MapPtr& map, const PartsPtr& pp){
	MapPtr data = xnew<Map>();
	Xfor2(k, v, map){
		data->set_at(k, pp);
	}
	return data;
}

MapPtr make_ch_map(const PartsPtr& p, const PartsPtr& pp){
	switch(p->type){
		XTAL_DEFAULT{}
		XTAL_CASE(PARTS_TEST_CH){ return make_ch_map2(ptr_cast<String>(p->p1), pp); }
		XTAL_CASE(PARTS_TEST_STRING){ return make_ch_map2(ptr_cast<String>(ptr_cast<Array>(p->p2)->at(0)), pp); }
		XTAL_CASE(PARTS_TEST_CH_SET){ return make_ch_map2(ptr_cast<Map>(p->p1), pp); }
		XTAL_CASE(PARTS_CH_MAP){ return make_ch_map2(ptr_cast<Map>(p->p1), pp); }
		XTAL_CASE(PARTS_FOLLOWED){ return make_ch_map(ptr_cast<Parts>(ptr_cast<Array>(p->p1)->at(0)), pp); }
		XTAL_CASE(PARTS_IGNORE){ return make_ch_map(ptr_cast<Parts>(p->p1), pp); }
		XTAL_CASE(PARTS_ARRAY){ return make_ch_map(ptr_cast<Parts>(p->p1), pp); }
		XTAL_CASE(PARTS_JOIN){ return make_ch_map(ptr_cast<Parts>(p->p1), pp); }
	}

	return null;
}

AnyPtr str(const StringPtr& str){
	StringStreamPtr ss = xnew<StringStream>(str);
	ArrayPtr data = xnew<Array>();
	while(!ss->eof()){
		data->push_back(ss->get_s(1));
	}

	if(data->size()==0){
		return xnew<Parts>(PARTS_SUCCESS);
	}

	if(data->size()==1){
		return xnew<Parts>(PARTS_TEST_CH, data->at(0));
	}

	return xnew<Parts>(PARTS_TEST_STRING, str, data);
}

AnyPtr ch_set(const StringPtr& str){
	MapPtr data = xnew<Map>();
	StringStreamPtr ss = xnew<StringStream>(str);
	while(!ss->eof()){
		StringPtr temp = ss->get_s(1);
		data->set_at(temp, temp);
	}

	return xnew<Parts>(PARTS_TEST_CH_SET, data);
}

AnyPtr ch_range(const StringPtr& begin, const StringPtr& end){
	MapPtr data = xnew<Map>();
	Xfor(v, xnew<ChRange>(begin, end)){
		data->set_at(v, true);
	}
	return xnew<Parts>(PARTS_TEST_CH_SET, data);
}

AnyPtr more(const AnyPtr& a, int_t n){
	AnyPtr p = P(a);
	if(n==0){
		return xnew<Parts>(PARTS_MORE, test(p));
	}

	if(n<0){
		p = select(p, success);
		AnyPtr pp = p;
		for(int_t i=1; i<-n; ++i){
			pp = followed(pp, p);
		}
		return pp;
	}

	AnyPtr pp = p;
	for(int_t i=1; i<n; ++i){
		pp = followed(pp, p);
	}

	return followed(pp, more(p, 0));
}

AnyPtr ignore(const AnyPtr& a){
	return xnew<Parts>(PARTS_IGNORE, P(a));
}

AnyPtr select(const AnyPtr& a, const AnyPtr& b){
	PartsPtr lhs = static_ptr_cast<Parts>(P(a));
	PartsPtr rhs = static_ptr_cast<Parts>(P(b));

	if(lhs->type==PARTS_TEST_CH_SET && rhs->type==PARTS_TEST_CH_SET){
		return xnew<Parts>(PARTS_TEST_CH_SET, ptr_cast<Map>(lhs->p1)->cat(ptr_cast<Map>(rhs->p1)));
	}

	/*
	if(MapPtr ml = make_ch_map(lhs, lhs)){
		if(MapPtr mr = make_ch_map(rhs, rhs)){
			if(ml->size()+mr->size()>2){
				MapPtr mm = ml->cat(mr);
				if(mm->size()==ml->size()+mr->size()){
					return xnew<Parts>(CH_MAP, mm);
				}
			}
		}
	}
	*/

	ArrayPtr data = xnew<Array>();
	if(lhs->type==PARTS_SELECT){
		data->cat_assign(static_ptr_cast<Array>(lhs->p1));
	}else{
		data->push_back(lhs);
	}

	if(rhs->type==PARTS_SELECT){
		data->cat_assign(static_ptr_cast<Array>(rhs->p1));
	}else{
		data->push_back(rhs);
	}

	for(uint_t i=0; i<data->size()-1; ++i){
		data->set_at(i, test(static_ptr_cast<Parts>(data->at(i))));
	}

	return xnew<Parts>(PARTS_SELECT, data);
}

AnyPtr followed(const AnyPtr& a, const AnyPtr& b){
	PartsPtr lhs = static_ptr_cast<Parts>(P(a));
	PartsPtr rhs = static_ptr_cast<Parts>(P(b));

	if(lhs->type==PARTS_TEST_STRING && rhs->type==PARTS_TEST_STRING){
		return str(ptr_cast<String>(lhs->p1)->cat(ptr_cast<String>(rhs->p1)));
	}

	ArrayPtr data = xnew<Array>();
	if(lhs->type==PARTS_FOLLOWED){
		data->cat_assign(static_ptr_cast<Array>(lhs->p1));
	}else{
		data->push_back(lhs);
	}

	if(rhs->type==PARTS_FOLLOWED){
		data->cat_assign(static_ptr_cast<Array>(rhs->p1));
	}else{
		data->push_back(rhs);
	}

	return xnew<Parts>(PARTS_FOLLOWED, data);
}

AnyPtr sub(const AnyPtr& a, const AnyPtr& b){
	return xnew<Parts>(PARTS_SUB, P(a), ignore(and_(b)));
}

AnyPtr join(const AnyPtr& a){
	return xnew<Parts>(PARTS_JOIN, P(a));
}

AnyPtr array(const AnyPtr& a){
	return xnew<Parts>(PARTS_ARRAY, P(a));
}

AnyPtr test(const AnyPtr& a){
	PartsPtr p = static_ptr_cast<Parts>(P(a));
	switch(p->type){
		XTAL_DEFAULT{ return xnew<Parts>(PARTS_TEST, p); }
		
		XTAL_CASE(PARTS_TEST){ return p; }
		XTAL_CASE(PARTS_TEST_CH){ return p; }
		XTAL_CASE(PARTS_TEST_STRING){ return p; }
		XTAL_CASE(PARTS_TEST_CH_SET){ return p; }
	}
}

AnyPtr not_(const AnyPtr& v){
	return xnew<Parts>(PARTS_NOT, P(v));
}

AnyPtr and_(const AnyPtr& v){
	return xnew<Parts>(PARTS_AND, P(v));
}

AnyPtr ch_map(const MapPtr& data){
	return xnew<Parts>(PARTS_CH_MAP, data);
}

AnyPtr val(const AnyPtr& v){
	return xnew<Parts>(PARTS_VAL, v);
}

AnyPtr act(const AnyPtr& a, const AnyPtr& b){
	return xnew<Parts>(PARTS_ACT, P(a), b);
}

AnyPtr actmv(const AnyPtr& a, const AnyPtr& b){
	return xnew<Parts>(PARTS_ACTMV, P(a), b);
}

AnyPtr neg(const AnyPtr& p){
	return any - p;
}

AnyPtr P(const AnyPtr& a){
	if(const StringPtr& ret = ptr_as<String>(a)){
		return str(ret);
	}	
	
	if(const PartsPtr& ret = ptr_as<Parts>(a)){
		return ret;
	}

	XTAL_THROW(cast_error(a, "Parts"), return null);
}


/////////////////////////////////////////////////////////////
// re

enum ReCharType{
	P_EPSILON,
	P_ANY,
	P_CH,
	P_CH_SET,
	P_NEG_CH_SET,
};

struct ReChar{
	int_t type;
	AnyPtr param;

	ReChar(int_t type, const AnyPtr& param = null)
		:type(type), param(param){}

	bool check(const AnyPtr& ch, int_t& pos){
		switch(type){
			XTAL_NODEFAULT;

			XTAL_CASE(P_EPSILON){
				return rawne(ch, nop);
			}

			XTAL_CASE(P_ANY){
				if(rawne(ch, nop)){
					pos++;
					return true;
				}else{
					return false;
				}
			}

			XTAL_CASE(P_CH){
				if(raweq(ch, param)){
					pos++;
					return true;
				}
				return false;
			}

			XTAL_CASE(P_CH_SET){
				const MapPtr& data = static_ptr_cast<Map>(param);
				if(data->at(ch)){
					pos++;
					return true;
				}
				return false;
			}

			XTAL_CASE(P_NEG_CH_SET){
				const MapPtr& data = static_ptr_cast<Map>(param);
				if(!data->at(ch)){
					pos++;
					return true;
				}
				return false;
			}
		}

		return false;
	}
};

typedef SmartPtr<ReChar> CharPtr;

enum ReToken{
	R_Char,  // ���ʂ̕���
	R_Any,   // '.'
	R_Lcl,   // '['
	R_Rcl,   // ']'
	R_Ncl,   // '^'
	R_Range, // '-'
	R_Lbr,   // '('
	R_Rbr,   // ')'
	R_Bar,   // '|'
	R_Star,  // '*'
	R_Plus,  // '+'
	R_Quest, // '?'
	R_End    // '\0'
};

class ReLexer{
public:
	void make(const StreamPtr& src);
	
	ReToken GetToken();

	const StringPtr& GetChar(){ return chr_; }

	StringPtr read(){
		if(sub_->eof()){
			if(src_->eof()){
				return null;
			}
			return src_->get_s(1);
		}
		return sub_->get_s(1);
	}

	template<class T, int N>
	void set_sub(T (&str)[N]){
		sub_->clear();
		sub_->put_s(xnew<String>(str, N-1));
		sub_->seek(0);
	}

private:
	StreamPtr src_;
	MemoryStreamPtr sub_;
	StringPtr chr_;
};

void ReLexer::make(const StreamPtr& src){
	src_ = src;
	sub_ = xnew<MemoryStream>();
}

ReToken ReLexer::GetToken(){
	StringPtr x = read();
	if(!x){
		return R_End;
	}

	if(x->buffer_size()>1){
		chr_ = x;
		return R_Char;
	}

	switch(*x->data()){
	case '.': return R_Any;
	case '[': return R_Lcl;
	case ']': return R_Rcl;
	case '^': return R_Ncl;
	case '-': return R_Range;
	case '(': return R_Lbr;
	case ')': return R_Rbr;
	case '|': return R_Bar;
	case '*': return R_Star;
	case '+': return R_Plus;
	case '?': return R_Quest;
	case '\\':

		x = read();
		if(!x){
			return R_End;
		}

		if(x->buffer_size()>1){
			chr_ = x;
			return R_Char;
		}

		switch(*x->data()){
		case 't': chr_ = "\t"; return R_Char;
		case 'n': chr_ = "\n"; return R_Char;
		case 'w': set_sub("[0-9a-zA-Z_]");  return GetToken();
		case 'W': set_sub("[^0-9a-zA-Z_]"); return GetToken();
		case 'd': set_sub("[0-9]"); return GetToken();
		case 'D': set_sub("[^0-9]"); return GetToken();
		case 's': set_sub("[\t ]"); return GetToken();
		case 'S': set_sub("[^\t ]"); return GetToken();
		} // fall through...

	default:
		chr_ = x;
		return R_Char;
	}
}


enum ReType{
	N_Class,    // [...] �Ȃ� (cls)
	N_Concat,   // �A��       (left, right)
	N_Or,       // |          (left, right)
	N_Closure,  // *          (left)
	N_Closure1, // +          (left)
	N_01,       // ?          (left)
	N_Empty     // ��         (--)
};

struct ReNode; 
typedef SmartPtr<ReNode> ReNodePtr;

struct ReNode{
	ReType type; // ���̃m�[�h�̎��
	CharPtr cls; // �����W��	
	ReNodePtr left; // ���̎q
	ReNodePtr right; // �E�̎q
};

class ReParser{
public:
	void make(const StreamPtr& src);
	ReNodePtr root() { return root_; }
	bool err() { return err_; }
	bool isHeadType() const { return isHeadType_; }
	bool isTailType() const { return isTailType_; }

private:
	ReNodePtr make_empty_leaf();
	ReNodePtr make_node(ReType t, const ReNodePtr& lft, const ReNodePtr& rht);
	void eat_token();
	ReNodePtr expr();
	ReNodePtr term();
	ReNodePtr factor();
	ReNodePtr primary();
	ReNodePtr reclass();

private:
	bool err_;
	bool isHeadType_;
	bool isTailType_;
	ReNodePtr root_;

	ReLexer lex_;
	ReToken nextToken_;
};

void ReParser::make(const StreamPtr& src){
	err_ = false;
	isHeadType_ = false;
	isTailType_ = false;
	lex_.make(src);
	eat_token();
	root_ = expr();
}

inline void ReParser::eat_token(){
	nextToken_ = lex_.GetToken();
}

inline ReNodePtr ReParser::make_empty_leaf(){
	ReNodePtr node = xnew<ReNode>();
	node->type = N_Empty;
	return node;
}

ReNodePtr ReParser::make_node(ReType t, const ReNodePtr& lft, const ReNodePtr& rht){
	ReNodePtr node = xnew<ReNode>();
	node->type = t;
	node->left = lft;
	node->right= rht;
	return node;
}

ReNodePtr ReParser::reclass(){
//	CLASS   ::= '^'? CHAR (CHAR | -CHAR)*

	bool neg = false;
	if(nextToken_ == R_Ncl){
		neg = true;
		eat_token();
	}

	SetPtr set = xnew<Set>();
	while(nextToken_ == R_Char){
		StringPtr ch = lex_.GetChar();
		eat_token();

		if(nextToken_ == R_Range){
			eat_token();
			if(nextToken_ != R_Char){
				err_ = true;
			}else{
				StringPtr ch2 = lex_.GetChar();
				eat_token();

				Xfor(v, xnew<ChRange>(ch, ch2)){
					set->set_at(v, true);
				}
			}
		}else{
			set->set_at(ch, true);
		}
	}

	ReNodePtr node = xnew<ReNode>();
	node->type = N_Class;
	node->cls = xnew<ReChar>(neg ? P_NEG_CH_SET : P_CH_SET, set);
	return node;
}

ReNodePtr ReParser::primary(){
//	PRIMARY ::= CHAR
//              '.'
//	            '[' CLASS ']'
//				'(' REGEXP ')'

	ReNodePtr node;
	switch(nextToken_){
		XTAL_DEFAULT{
			node = make_empty_leaf();
			err_ = true;
		}

		XTAL_CASE(R_Any){
			node = xnew<ReNode>();
			node->type = N_Class;
			node->cls = xnew<ReChar>(P_ANY);
			eat_token();
		}

		XTAL_CASE(R_Char){
			node = xnew<ReNode>();
			node->type = N_Class;
			node->cls = xnew<ReChar>(P_CH, lex_.GetChar());
			eat_token();
		}

		XTAL_CASE(R_Lcl){
			eat_token();
			node = reclass();
			if(nextToken_ == R_Rcl){
				eat_token();
			}else{
				err_ = true;
			}
		}

		XTAL_CASE(R_Lbr){
			eat_token();
			node = expr();
			if(nextToken_ == R_Rbr){
				eat_token();
			}else{
				err_ = true;
			}
		}
	}

	return node;
}

ReNodePtr ReParser::factor(){
//	FACTOR  ::= PRIMARY
//	            PRIMARY '*'
//			    PRIMARY '+'
//			    PRIMARY '?'

	ReNodePtr node = primary();
	switch(nextToken_ ){
	case R_Star: node = make_node(N_Closure, node, null); eat_token(); break;
	case R_Plus: node = make_node(N_Closure1, node, null);eat_token(); break;
	case R_Quest:node = make_node(N_01, node, null); eat_token(); break;
	}
	return node;
}

ReNodePtr ReParser::term(){
//	TERM    ::= EMPTY
//	            FACTOR TERM

	if(nextToken_ == R_End){
		return make_empty_leaf();
	}

	ReNodePtr node = factor();
	if(nextToken_==R_Lbr || nextToken_==R_Lcl || nextToken_==R_Char || nextToken_==R_Any){
		node = make_node(N_Concat, node, term());
	}

	return node;
}

ReNodePtr ReParser::expr(){
//	REGEXP  ::= TERM
//	            TERM '|' REGEXP

	ReNodePtr node = term();
	if(nextToken_ == R_Bar){
		eat_token();
		node = make_node(N_Or, node, expr());
	}
	return node;
}

struct ReTrans;
typedef SmartPtr<ReTrans> ReTransPtr;

struct ReTrans{
	// ���̕����W����������
	CharPtr cls; 

	// ��Ԕԍ�to�̏�Ԃ֑J��
	int to; 

	// �A�����X�g
	ReTransPtr next; 
};

class ReNFA{
public:

	void make(const StreamPtr& src);

	int match(const ScannerPtr& scanner);
	bool isHeadType() const { return parser_.isHeadType(); }
	bool isTailType() const { return parser_.isTailType(); }

private:

	int dfa_match(const ScannerPtr& scanner);

	struct st_ele{ int st, ps; };
	typedef PODStack<st_ele> stack_t;

	void push(stack_t& stack, int curSt, int pos);

private:
	void add_transition(int from, const CharPtr& ch, int to);
	void add_e_transition(int from, int to);
	int gen_state();
	void gen_nfa(int entry, const ReNodePtr& t, int exit);

private:
	ReParser parser_;
	ArrayPtr st_;
	int start_, final_;
};

void ReNFA::make(const StreamPtr& src){
	parser_.make(src);
	st_ = xnew<Array>();
	start_ = gen_state();
	final_ = gen_state();
	gen_nfa(start_, parser_.root(), final_);
}

inline void ReNFA::add_transition(int from, const CharPtr& cls, int to){
	ReTransPtr x = xnew<ReTrans>();
	x->next = static_ptr_cast<ReTrans>(st_->at(from));
	x->to = to;
	x->cls = cls;
	st_->set_at(from, x);
}

inline void ReNFA::add_e_transition(int from, int to){
	ReTransPtr x = xnew<ReTrans>();
	x->next = static_ptr_cast<ReTrans>(st_->at(from));
	x->to = to;
	x->cls = xnew<ReChar>(P_EPSILON);
	st_->set_at(from, x);
}

inline int ReNFA::gen_state(){
	st_->push_back(null);
	return st_->size() - 1;
}

void ReNFA::gen_nfa(int entry, const ReNodePtr& t, int exit){
	switch(t->type){
		XTAL_NODEFAULT;

		XTAL_CASE(N_Class){
			//         cls
			//  entry -----> exit
			add_transition(entry, t->cls, exit);
		}

		XTAL_CASE(N_Concat){
			//         left         right
			//  entry ------> step -------> exit
			int step = gen_state();
			gen_nfa(entry, t->left, step);
			gen_nfa(step, t->right, exit);
		}

		XTAL_CASE(N_Or){
			//          left
			//         ------>
			//  entry ------->--> exit
			//          right
			gen_nfa(entry, t->left, exit);
			gen_nfa(entry, t->right, exit);
		}

		XTAL_CASE2(N_Closure, N_Closure1){
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
			add_e_transition(entry, before);
			add_e_transition(after, exit);
			add_e_transition(after, before);
			gen_nfa(before, t->left, after);
			if(t->type != N_Closure1){
				add_e_transition(entry, exit);
			}
		}

		XTAL_CASE(N_01){
			//           e
			//        ------>
			//  entry ------> exit
			//         left
			add_e_transition(entry, exit);
			gen_nfa(entry, t->left, exit);
		}

		XTAL_CASE(N_Empty){
			//         e
			//  entry ---> exit
			add_e_transition(entry, exit);
		}
	}
}


void ReNFA::push(stack_t& stack, int curSt, int pos){
	// �Ö������[�v�h�~��B������Ԃɂ͖߂�Ȃ��悤�Ɂc
	for(uint_t i=0; i<stack.size(); ++i){
		if(stack[i].ps != pos){
			break;
		}else if(stack[i].st == curSt){
			return;
		}
	}

	st_ele nw = {curSt, pos};
	stack.push(nw);
}

int ReNFA::match(const ScannerPtr& scanner){
	if(parser_.err()){
		return -1; // �G���[��ԂȂ̂�match�Ƃ��ł��܂���
	}

	if(st_->size() <= 31){
		return dfa_match(scanner);
	}

	int matchpos = -1;

	stack_t stack;
	push(stack, start_, scanner->pos());
	while(!stack.empty()){
		st_ele se = stack.pop();
		int curSt = se.st;
		int pos = se.ps;

		scanner->seek(pos);

		// �}�b�`�������Ă���L�^
		if(curSt == final_){ // 1==�I���
			if(matchpos < pos){
				matchpos = pos;
			}
		}

		// ����ɐ�̑J�ڂ𒲂ׂ�
		for(ReTransPtr tr=static_ptr_cast<ReTrans>(st_->at(curSt)); tr; tr=tr->next){
			int_t npos = pos;
			if(tr->cls->check(scanner->peek(), npos)){
				push(stack, tr->to, npos);
				scanner->seek(pos);
			}
		}
	}

	return matchpos;
}

int ReNFA::dfa_match(const ScannerPtr& scanner){
	int matchpos = -1;

	unsigned int StateSet = (1<<start_);
	for(int pos=0; StateSet; ++pos){
		// ��-closure
		for(unsigned int DifSS=StateSet; DifSS;){
			unsigned int NewSS = 0;
			for(int s=0; (1u<<s)<=DifSS; ++s){
				if((1u<<s) & DifSS){
					for(ReTransPtr tr=static_ptr_cast<ReTrans>(st_->at(s)); tr; tr=tr->next){
						if(tr->cls->type == P_EPSILON){
							NewSS |= 1u << tr->to;
						}
					}
				}
			}

			DifSS = (NewSS|StateSet) ^ StateSet;
			StateSet |= NewSS;
		}

		// �󗝏�Ԃ��܂�ł邩�ǂ�������
		if(StateSet & (1<<final_)){
			matchpos = pos;
		}

		// ������̏I���ɒB����
		if(scanner->eof()){
			break;
		}

		// �J��
		unsigned int NewSS = 0;
		for(int s=0; (1u<<s)<=StateSet; ++s){
			if((1u<<s) & StateSet){
				for(ReTransPtr tr=static_ptr_cast<ReTrans>(st_->at(s)); tr; tr=tr->next){
					int_t npos = pos;
					if(tr->cls->check(scanner->peek(), npos)){
						if(npos!=pos){
							NewSS |= 1u << tr->to;
						}
					}
				}
			}
		}

		scanner->skip(1);
		StateSet = NewSS;
	}

	return matchpos;
}

class Re : public Parts{
public:

	Re(const StringPtr& str)
		:Parts(PARTS_RE){
		set(str);
	}

	void set(const StringPtr& str){
		original_ = str;
		nfa_.make(xnew<StringStream>(str));
	}
	
	void to_s(const VMachinePtr& vm){
		vm->return_result(original_);
	}

	AnyPtr serial_save(){
		return original_;
	}

	void serial_load(const StringPtr& v){
		set(v);
	}

	bool match(const ScannerPtr& scanner){
		Scanner::Mark mark = scanner->begin_record();

		int_t pos = scanner->pos();
		int mpos = nfa_.match(scanner);
		if(mpos>=0){
			scanner->seek(mpos + pos);
			scanner->end_record(mark, true);
		}else{
			scanner->end_record(mark, false);
		}

		return mpos>=0;

	}

private:

	StringPtr original_;
	ReNFA nfa_;
	
	virtual void visit_members(Visitor& m){
		Base::visit_members(m);
		m & original_;
	}
};


AnyPtr re(const StringPtr& pattern){
	return xnew<Re>(pattern);
}

bool parse_inner(const AnyPtr& ps, const ScannerPtr& scanner){

	const PartsPtr& parts = static_ptr_cast<Parts>(ps);

	switch(parts->type){
		XTAL_NODEFAULT;

		XTAL_CASE(PARTS_TEST_STRING){
			const ArrayPtr& data = static_ptr_cast<Array>(parts->p2);
			for(uint_t i=0, sz=data->size(); i<sz; ++i){
				if(rawne(scanner->peek(i), data->at(i))){
					return false;
				}
			}
			scanner->skip(data->size());
			scanner->push_result(parts->p1);
			return true;
		}

		XTAL_CASE(PARTS_TEST_CH){
			if(raweq(scanner->peek(), parts->p1)){
				scanner->push_result(parts->p1);
				scanner->skip(1);
				return true;
			}
			return false;
		}

		XTAL_CASE(PARTS_TEST_CH_SET){
			const MapPtr& data = static_ptr_cast<Map>(parts->p1);
			const AnyPtr& s = scanner->peek();
			if(data->at(s)){
				scanner->push_result(s);
				scanner->skip(1);
				return true;
			}
			return false;
		}

		XTAL_CASE(PARTS_END){
			return scanner->eof();
		}

		XTAL_CASE(PARTS_ANY){
			if(scanner->eof()){
				return false;
			}
			scanner->push_result(scanner->read());
			return true;
		}

		XTAL_CASE(PARTS_MORE){
			const PartsPtr& p = static_ptr_cast<Parts>(parts->p1);
			while(parse_inner(p, scanner)){}
			return true;
		}

		XTAL_CASE(PARTS_SELECT){
			const ArrayPtr& p = static_ptr_cast<Array>(parts->p1);
			for(uint_t i=0, sz=p->size()-1; i<sz; ++i){
				if(parse_inner(p->at(i), scanner)){
					return true;
				}
			}
			return parse_inner(p->back(), scanner);
		}

		XTAL_CASE(PARTS_FOLLOWED){
			const ArrayPtr& p = static_ptr_cast<Array>(parts->p1);
			for(uint_t i=0, sz=p->size()-1; i<sz; ++i){
				if(!parse_inner(p->at(i), scanner)){
					return false;
				}
			}
			return parse_inner(p->back(), scanner);
		}

		XTAL_CASE(PARTS_JOIN){
			const PartsPtr& p = static_ptr_cast<Parts>(parts->p1);
			Scanner::Mark mark = scanner->begin_join();				
			if(parse_inner(p, scanner)){
				scanner->end_join(mark);
				return true;
			}
			scanner->end_join(mark, true);
			return false;
		}

		XTAL_CASE(PARTS_ARRAY){
			const PartsPtr& p = static_ptr_cast<Parts>(parts->p1);
			Scanner::Mark mark = scanner->begin_array();
			if(parse_inner(p, scanner)){
				scanner->end_array(mark);
				return true;
			}
			scanner->end_array(mark, true);
			return false;
		}

		XTAL_CASE(PARTS_TEST){
			const PartsPtr& p = static_ptr_cast<Parts>(parts->p1);
			Scanner::Mark mark = scanner->mark();

			if(parse_inner(p, scanner)){
				scanner->unmark(mark);
				return true;
			}

			scanner->unmark(mark);
			return false;
		}

		XTAL_CASE(PARTS_IGNORE){
			const PartsPtr& p = static_ptr_cast<Parts>(parts->p1);
			scanner->begin_ignore();
			bool ret = parse_inner(p, scanner);
			scanner->end_ignore();
			return ret;
		}

		XTAL_CASE(PARTS_CH_MAP){
			const MapPtr& data = static_ptr_cast<Map>(parts->p1);
			if(const AnyPtr& a = data->at(scanner->peek())){
				return parse_inner(a, scanner);
			}
			return false;
		}

		XTAL_CASE(PARTS_NODE){
			scanner->push_result(parts->p1);
			ArrayPtr results = scanner->results();
			results->push_back(results->splice(results->size()-(parts->p2->to_i()+1), parts->p2->to_i()+1));
			return true;
		}

		XTAL_CASE(PARTS_VAL){
			scanner->push_result(parts->p1);
			return true;
		}

		XTAL_CASE(PARTS_LINENO){
			scanner->push_result(scanner->lineno());
			return true;
		}

		XTAL_CASE(PARTS_FAIL){
			return false;
		}

		XTAL_CASE(PARTS_SUCCESS){
			return true;
		}

		XTAL_CASE(PARTS_NOT){
			const PartsPtr& p = static_ptr_cast<Parts>(parts->p1);
			Scanner::Mark mark = scanner->mark();
			bool ret = parse_inner(p, scanner);
			scanner->unmark(mark, true);
			return !ret;
		}

		XTAL_CASE(PARTS_AND){
			const PartsPtr& p = static_ptr_cast<Parts>(parts->p1);
			Scanner::Mark mark = scanner->mark();
			bool ret = parse_inner(p, scanner);
			scanner->unmark(mark, true);
			return ret;
		}

		XTAL_CASE(PARTS_SUB){
			if(parse_inner(parts->p2, scanner)){
				return false;
			}
			return parse_inner(parts->p1, scanner);
		}

		XTAL_CASE(PARTS_ASCII){
			if(scanner->eof()){
				return false;
			}

			StringPtr str = scanner->peek()->to_s();
			if(str->buffer_size()==1 && ((u8)str->data()[0])<128){
				scanner->push_result(str);
				return true;
			}
			return false;
		}

		XTAL_CASE(PARTS_ACT){
			Scanner::Mark mark = scanner->begin_act();
			bool ret = parse_inner(parts->p1, scanner);
			scanner->end_act(mark, !ret, parts->p2, false);
			return ret;
		}

		XTAL_CASE(PARTS_ACTMV){
			Scanner::Mark mark = scanner->begin_act();
			bool ret = parse_inner(parts->p1, scanner);
			scanner->end_act(mark, !ret, parts->p2, true);
			return ret;
		}

		XTAL_CASE(PARTS_RE){
			return static_ptr_cast<Re>(parts)->match(scanner);
		}
	}

	return false;
}

ScannerPtr parse_scanner(const AnyPtr& pattern, const ScannerPtr& scanner){
	scanner->set_success(parse_inner(P(pattern), scanner));
	return scanner;
}

////////////////////////////////////////////////////////////////

}

void initialize_peg(){
	using namespace peg;

	register_uninitializer(&uninitialize_peg);

	ClassPtr peg = xnew<Class>("peg");

	new_cpp_class<Parts>("Parts");

	{
		ClassPtr p = get_cpp_class<String>();
		p->method("op_shr", &followed, get_cpp_class<String>());
		p->method("op_shr", &followed, get_cpp_class<Parts>());
		p->method("op_mul", &more, get_cpp_class<Int>());
		p->method("op_sub", &sub, get_cpp_class<String>());
		p->method("op_sub", &sub, get_cpp_class<Parts>());
		p->method("op_or", &select, get_cpp_class<String>());
		p->method("op_or", &select, get_cpp_class<Parts>());
		p->method("op_com", &ignore);
		p->method("op_neg", &neg);
		p->method("op_call", &act);
	}

	{
		ClassPtr p = new_cpp_class<Parts>("Parts");
		p->def("new", ctor<Parts>());
		p->method("op_shr", &followed, get_cpp_class<String>());
		p->method("op_shr", &followed, get_cpp_class<Parts>());
		p->method("op_mul", &more, get_cpp_class<Int>());
		p->method("op_sub", &sub, get_cpp_class<String>());
		p->method("op_sub", &sub, get_cpp_class<Parts>());
		p->method("op_or", &select, get_cpp_class<String>());
		p->method("op_or", &select, get_cpp_class<Parts>());
		p->method("op_com", &ignore);
		p->method("op_neg", &neg);
		p->method("op_call", &act);

		peg->def("Parts", p);

		set_cpp_class<Re>(p);
	}

	{
		ClassPtr p = new_cpp_class<Re>("Re");
		p->inherit(get_cpp_class<Parts>());
	}

	{
		ClassPtr p = new_cpp_class<Scanner>("Scanner");
		p->method("results", &Scanner::results);
		peg->def("Scanner", p);
	}

	{
		ClassPtr p = new_cpp_class<StreamScanner>("StreamScanner");
		p->inherit(get_cpp_class<Scanner>());
		p->def("new", ctor<StreamScanner, const StreamPtr&>());

		peg->def("StreamScanner", p);
	}

	{
		ClassPtr p = new_cpp_class<IteratorScanner>("IteratorScanner");
		p->inherit(get_cpp_class<Scanner>());
		p->def("new", ctor<IteratorScanner, const AnyPtr&>());

		peg->def("IteratorScanner", p);
	}

	builtin()->def("peg", peg);


	any = xnew<Parts>(PARTS_ANY);
	eof = xnew<Parts>(PARTS_END);
	success = xnew<Parts>(PARTS_SUCCESS);
	fail = xnew<Parts>(PARTS_FAIL);
	lineno = xnew<Parts>(PARTS_LINENO);
	ch_lalpha = ch_set("abcdefghijklmnopqrstuvwxyz");
	ch_ualpha = ch_set("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
	ch_alpha = select(ch_ualpha, ch_lalpha);
	ch_space = ch_set(" \t\n\r");
	ch_digit = ch_set("0123456789");
	ch_ascii = xnew<Parts>(PARTS_ASCII);

	peg->fun("parse_string", &parse_string);
	peg->fun("parse_stream", &parse_stream);
	peg->fun("parse_iterator", &parse_iterator);

	peg->def("any", any);
	peg->def("eof", eof);
	peg->def("success", success);
	peg->def("fail", fail);
	peg->def("lineno", lineno);
	peg->def("ch_alpha", ch_alpha);
	peg->def("ch_lalpha", ch_lalpha);
	peg->def("ch_ualpha", ch_ualpha);
	peg->def("ch_space", ch_alpha);
	peg->def("ch_digit", ch_digit);
	peg->def("ch_ascii", ch_ascii);
	peg->fun("str", &str);
	peg->fun("ch_set", &ch_set);
	peg->fun("join", &join)->param(null, Named("sep", ""));
	//peg->fun("array", &array);
	peg->fun("val", &val);
	peg->fun("and", &and_);
	peg->fun("not", &not_);
}

}