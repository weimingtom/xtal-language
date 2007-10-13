#include "xtal.h"

#include "xtal_peg.h"
#include "xtal_macro.h"

namespace xtal{ namespace peg{

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
	R_Char,  // 普通の文字
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
	ReLexer(const StreamPtr& src);
	
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

inline ReLexer::ReLexer(const StreamPtr& src)
	:src_(src), sub_(xnew<MemoryStream>()){}

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
	N_Class,    // [...] など (cls)
	N_Concat,   // 連接       (left, right)
	N_Or,       // |          (left, right)
	N_Closure,  // *          (left)
	N_Closure1, // +          (left)
	N_01,       // ?          (left)
	N_Empty     // 空         (--)
};

struct ReNode; 
typedef SmartPtr<ReNode> ReNodePtr;

struct ReNode{
	ReType type; // このノードの種類
	CharPtr cls; // 文字集合	
	ReNodePtr left; // 左の子
	ReNodePtr right; // 右の子
};

class ReParser{
public:
	ReParser(const StreamPtr& src);
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

inline ReParser::ReParser(const StreamPtr& src)
	:err_(false), isHeadType_(false), isTailType_(false), lex_(src){
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

				Xfor(v, make_range(ch->data(), ch->buffer_size(), ch2->data(), ch2->buffer_size())){
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
	// この文字集合が来たら
	CharPtr cls; 

	// 状態番号toの状態へ遷移
	int to; 

	// 連結リスト
	ReTransPtr next; 
};

class ReNFA{
public:
	ReNFA(const StreamPtr& src);

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

ReNFA::ReNFA(const StreamPtr& src)
	:parser_(src){
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
	// ε無限ループ防止策。同じ状態には戻らないように…
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
		return -1; // エラー状態なのでmatchとかできません
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

		// マッチ成功してたら記録
		if(curSt == final_){ // 1==終状態
			if(matchpos < pos){
				matchpos = pos;
			}
		}

		// さらに先の遷移を調べる
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
		// ε-closure
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

		// 受理状態を含んでるかどうか判定
		if(StateSet & (1<<final_)){
			matchpos = pos;
		}

		// 文字列の終わりに達した
		if(scanner->eof()){
			break;
		}

		// 遷移
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


bool reg_match(const StreamPtr& src, const ScannerPtr& scanner){
	ReNFA re(src);
	Scanner::Mark mark = scanner->mark();
	Scanner::Mark mark2 = scanner->begin_record();

	int mpos = re.match(scanner);
	if(mpos>=0){
		scanner->seek(mpos);

		scanner->end_record(mark2)->p();
	}

	scanner->unmark(mark, mpos<0);

	return mpos>=0;
}

}}
