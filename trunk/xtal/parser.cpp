
#include <math.h>

#include "xtal.h"
#include "parser.h"
#include "any.h"
#include "xmacro.h"

namespace xtal{


enum{//Expressions priority
	
	PRI_BEGIN_ = 0x1000,

	PRI_Q,

	PRI_OROR,
	PRI_ANDAND,

	PRI_OR,
	PRI_XOR,
	PRI_AND,

	PRI_EQ,
		PRI_NE = PRI_EQ,
		PRI_LT = PRI_EQ,
		PRI_GT = PRI_EQ,
		PRI_LE = PRI_EQ,
		PRI_GE = PRI_EQ,
		PRI_RAW_EQ = PRI_EQ,
		PRI_RAW_NE = PRI_EQ,
		PRI_IS = PRI_EQ,

	PRI_SHL,
		PRI_SHR = PRI_SHL,
		PRI_USHR = PRI_SHL,

	PRI_ADD, 
		PRI_SUB = PRI_ADD, 
		PRI_CAT = PRI_ADD,
	
	PRI_MUL, 
		PRI_DIV = PRI_MUL, 
		PRI_MOD = PRI_MUL,

	PRI_NEG,
		PRI_POS = PRI_NEG,
		PRI_COM = PRI_NEG,
		PRI_NOT = PRI_NEG,
		PRI_REF = PRI_NEG,
		PRI_DEREF = PRI_NEG,

	PRI_AT,
		PRI_SEND = PRI_AT,
		PRI_MEMBER = PRI_AT,
		PRI_CALL = PRI_AT,
		PRI_NS = PRI_AT,

	PRI_ONCE,

	PRI_END_,

	PRI_MAX = PRI_END_-PRI_BEGIN_
};

template<class T>
T* set_line(int_t line, T* t){
	t->line = line;
	return t;
}

#define XTAL_NEW new(&alloc_)
#define c2(C1, C2) ((C2)<<8 | (C1))
#define c3(C1, C2, C3) ((C3)<<16 | (C2)<<8 | (C1))
#define c4(C1, C2, C3, C4) ((C4)<<24 | (C3)<<16 | (C2)<<8 | (C1))

Parser::Parser(){}

bool Parser::eat_end(){
	if(eat('\a') || eat(';'))
		return true;
	return false;
}

bool Parser::eat_a(int_t ach){
	Token ch = lexer_.peek();
	if(ch.type()==Token::TYPE_TOKEN && ch.ivalue()=='\a'){
		lexer_.read();
		if(eat(ach)){
			return true;
		}
		lexer_.putback(ch);
	}else{
		if(eat(ach)){
			return true;
		}
	}
	return false;
}
	
void Parser::expect_a(int_t ch){
	eat('\a');
	expect(ch);
}

void Parser::expect_end(){
	if(eat_end()){
		return;
	}
	com_.error(line(), Xt("';' がありません"));
}

Stmt* Parser::parse(const Stream& stream, const String& source_file_name){
	lexer_.init(stream, source_file_name);
	com_ = lexer_.common();

	Stmt* p = parse_top_level();

	if(com_.errors.size()!=0){
		return 0;
	}
	return p;
}

LPCCommon Parser::common(){
	return com_;
}


void Parser::release(){
	alloc_.release();
}
	
void Parser::expect(int_t ch){
	if(eat(ch)){
		return;
	}		
	com_.error(line(), Xt("予期せぬ '%(char)s' が検出されました")(
		Xid(char)=lexer_.token2str(lexer_.peek())));
}

bool Parser::eat(int_t ch){
	Token n = lexer_.peek();
	if(n.type() == Token::TYPE_TOKEN){
		if(n.ivalue()==ch){
			lexer_.read();
			return true;
		}
	}
	return false;
}

bool Parser::eat(Token::Keyword kw){
	Token n = lexer_.peek();
	if(n.type() == Token::TYPE_KEYWORD){
		if(n.ivalue()==kw){
			lexer_.read();
			return true;
		}
	}
	return false;
}
	
bool Parser::eat_a(Token::Keyword kw){
	Token ch = lexer_.peek();
	if(ch.type()==Token::TYPE_TOKEN && ch.ivalue()=='\a'){
		lexer_.read();
		if(eat(kw)){
			return true;
		}
		lexer_.putback(ch);
	}else{
		if(eat(kw)){
			return true;
		}
	}
	return false;
}

string_t Parser::parse_string(int_t open, int_t close){
	string_t str;
	int_t depth = 1;
	while(1){
		int_t ch = lexer_.read().ivalue();
		if(ch==close){
			--depth;
			if(depth==0){
				break;
			}
		}
		if(ch==open){
			++depth;
		}
		if(ch==-1){
			com_.error(line(), Xt("文字列リテラルが終わる前にEOFが検出されました"));
			break;
		}
		if(ch=='\\'){
			switch(lexer_.peek().ivalue()){
				XTAL_DEFAULT{ 
					str+='\\';
					str+=lexer_.peek().ivalue(); 
				}
				
				XTAL_CASE('n'){ str+='\n'; }
				XTAL_CASE('r'){ str+='\r'; }
				XTAL_CASE('t'){ str+='\t'; }
				XTAL_CASE('f'){ str+='\f'; }
				XTAL_CASE('b'){ str+='\b'; }
				XTAL_CASE('0'){ str+='\0'; }
				XTAL_CASE('\\'){ str+='\\'; }
				XTAL_CASE('"'){ str+='"'; } 
				
				XTAL_CASE('\r'){ 
					if(lexer_.peek().ivalue()=='\n')
						lexer_.read().ivalue();
					str+='\r';
					str+='\n';
				}
				
				XTAL_CASE('\n'){ 
					str+='\n';
				}
			}
			lexer_.read();
		}else{
			if(ch=='\r'){
				if(lexer_.peek().ivalue()=='\n')
					lexer_.read();
				str+='\r';
				str+='\n';
			}else if(ch=='\n'){
				str+='\n';
			}else{
				str+=ch;
			}
		}	
	}
	lexer_.set_normal_mode();
	return str;
}

Expr* Parser::parse_term(){
	
	int_t ln = lexer_.line();
	Token ch = lexer_.read();

	Expr* ret = 0;
	int_t r_space = ch.right_space() ? PRI_MAX : 0;

	switch(ch.type()){
		XTAL_NODEFAULT;

		XTAL_CASE(Token::TYPE_TOKEN){
			switch(ch.ivalue()){

				XTAL_DEFAULT{}

				XTAL_CASE('('){ 
					ret = parse_expr(); 
					expect_a(')');
				}

				XTAL_CASE('['){ 
					ret = parse_array();
				}

				XTAL_CASE('@'){
					ret = XTAL_NEW InstanceVariableExpr(ln, parse_ident_or_keyword());
				}

				XTAL_CASE('"'){ //"
					lexer_.set_string_mode();
					String str(parse_string('"', '"'));
					ret = XTAL_NEW StringExpr(ln, com_.register_value(str));
				}
				
				XTAL_CASE('%'){
					lexer_.set_string_mode();
					int_t open = lexer_.read().ivalue();
					int_t close = 0;
					int_t kind = KIND_STRING;
					while(true){
						switch(open){
						case 't':
							if(kind!=KIND_STRING){
								com_.error(line(), Xt("不正な%%記法リテラルです"));
							}
							kind = KIND_TEXT;
							open = lexer_.read().ivalue();
							continue;

						case 'f':
							if(kind!=KIND_STRING){
								com_.error(line(), Xt("不正な%%記法リテラルです"));
							}
							kind = KIND_FORMAT;
							open = lexer_.read().ivalue();
							continue;

						case '!': case '?': case '"': case '&': //"
						case '#': case '\'':case '|': case ':':
						case '^': case '+': case '-': case '*':
						case '/': case '@': case '$': case '.':
						case '=': case '~': case '`': case ';':
							close = open; break;

						case '(': close = ')'; break;
						case '<': close = '>'; break;
						case '{': close = '}'; break;
						case '[': close = ']'; break;

						default:
							close = open;
							com_.error(line(), Xt("不正な%%記法リテラルです"));
							break;
						}

						break;
					}
				
					String str(parse_string(open, close));
					ret = XTAL_NEW StringExpr(ln, com_.register_value(str), kind);
				}
				
				XTAL_CASE(c3('.','.','.')){ 
					ret = XTAL_NEW ArgsExpr(ln);
				}

////////////////////////////////////////////////////////////////////////////////////////

				XTAL_CASE('+'){
					if(Expr* rhs = parse_expr(PRI_POS - r_space)){
						ret = XTAL_NEW UnaExpr(ln, CODE_POS, rhs);
					}
				}

				XTAL_CASE('-'){ 
					if(Expr* rhs = parse_expr(PRI_NEG - r_space)){
						ret = XTAL_NEW UnaExpr(ln, CODE_NEG, rhs);
					}
				}

				XTAL_CASE('~'){ 
					if(Expr* rhs = parse_expr(PRI_COM - r_space)){
						ret = XTAL_NEW UnaExpr(ln, CODE_COM, rhs);
					}
				}

				XTAL_CASE('!'){ 
					if(Expr* rhs = parse_expr(PRI_NOT - r_space)){
						ret = XTAL_NEW UnaExpr(ln, CODE_NOT, rhs);
					}
				}
			}
		}

		XTAL_CASE(Token::TYPE_KEYWORD){
			switch(ch.ivalue()){

				XTAL_DEFAULT{}
				
				XTAL_CASE(Token::KEYWORD_ONCE){ ret = XTAL_NEW OnceExpr(ln, parse_expr_must(PRI_ONCE - r_space*2)); }
				XTAL_CASE(Token::KEYWORD_CLASS){ ret = parse_class_or_module(KIND_CLASS); }
				XTAL_CASE(Token::KEYWORD_FUN){ ret = parse_fun(KIND_FUN); }
				XTAL_CASE(Token::KEYWORD_METHOD){ ret = parse_fun(KIND_METHOD); }
				XTAL_CASE(Token::KEYWORD_FIBER){ ret = parse_fun(KIND_FIBER); }
				XTAL_CASE(Token::KEYWORD_DOFUN){ ret = XTAL_NEW CallExpr(ln, parse_fun(KIND_FUN)); }
				XTAL_CASE(Token::KEYWORD_CALLEE){ ret = XTAL_NEW CalleeExpr(ln); }
				XTAL_CASE(Token::KEYWORD_NULL){ ret = XTAL_NEW PseudoVariableExpr(ln, CODE_PUSH_NULL); }
				XTAL_CASE(Token::KEYWORD_TRUE){ ret = XTAL_NEW PseudoVariableExpr(ln, CODE_PUSH_TRUE); }
				XTAL_CASE(Token::KEYWORD_FALSE){ ret = XTAL_NEW PseudoVariableExpr(ln, CODE_PUSH_NULL); }
				XTAL_CASE(Token::KEYWORD_NOP){ ret = XTAL_NEW PseudoVariableExpr(ln, CODE_PUSH_NOP); }
				XTAL_CASE(Token::KEYWORD_THIS){ ret = XTAL_NEW PseudoVariableExpr(ln, CODE_PUSH_THIS); }
			}
		}
		
		XTAL_CASE(Token::TYPE_INT){
			ret = XTAL_NEW IntExpr(ln, ch.ivalue());
		}

		XTAL_CASE(Token::TYPE_FLOAT){
			ret = XTAL_NEW FloatExpr(ln, ch.fvalue());
		}

		XTAL_CASE(Token::TYPE_IDENT){
			ret = XTAL_NEW LocalExpr(ln, ch.ivalue());
		}
	}

	if(!ret){
		lexer_.putback(ch);
	}

	return ret;
}

Expr* Parser::parse_post(Expr* lhs, int_t pri){

	int_t ln = lexer_.line();
	Token ch = lexer_.read();

	Expr* ret = 0;

	int_t r_space = (ch.right_space()) ? PRI_MAX : 0;
	int_t l_space = (ch.left_space()) ? PRI_MAX : 0;

	switch(ch.type()){
	
		XTAL_DEFAULT{}
		
		XTAL_CASE(Token::TYPE_KEYWORD){
			switch(ch.ivalue()){
				XTAL_DEFAULT{}
				
				XTAL_CASE(Token::KEYWORD_IS){ 
					if(pri < PRI_IS - l_space){
						ret = XTAL_NEW BinCompExpr(ln, CODE_IS, lhs, parse_expr(PRI_IS - r_space)); 
					}
				}
			}
		}
		
		XTAL_CASE(Token::TYPE_TOKEN){
			switch(ch.ivalue()){

				XTAL_DEFAULT{
					//com_.error(line(), "構文エラーです");
					ret = 0;
				}

				XTAL_CASE('{'){ ret = 0; }
				XTAL_CASE('}'){ ret = 0; }
				XTAL_CASE(')'){ ret = 0; }
				XTAL_CASE(']'){ ret = 0; }
				XTAL_CASE(','){ ret = 0; }
				XTAL_CASE(';'){ ret = 0; }
				XTAL_CASE(':'){ ret = 0; }
				XTAL_CASE('='){ ret = 0; }
				XTAL_CASE(c2('+','+')){ ret = 0; }
				XTAL_CASE(c2('-','-')){ ret = 0; }
				XTAL_CASE(c2('+','=')){ ret = 0; }
				XTAL_CASE(c2('-','=')){ ret = 0; }
				XTAL_CASE(c2('*','=')){ ret = 0; }
				XTAL_CASE(c2('/','=')){ ret = 0; }
				XTAL_CASE(c2('%','=')){ ret = 0; }
				XTAL_CASE(c2('^','=')){ ret = 0; }
				XTAL_CASE(c2('|','=')){ ret = 0; }
				XTAL_CASE(c2('&','=')){ ret = 0; }
				XTAL_CASE(c3('<','<','=')){ ret = 0; }
				XTAL_CASE(c3('>','>','=')){ ret = 0; }
				XTAL_CASE(c3('>','3','=')){ ret = 0; }
				XTAL_CASE('\a'){ ret = 0; }
				XTAL_CASE(-1){ ret = 0; }
			
				XTAL_CASE('+'){
					if(pri < PRI_ADD - l_space){
						ret = XTAL_NEW BinExpr(ln, CODE_ADD, lhs, parse_expr(PRI_ADD - r_space)); 
					}
				}

				XTAL_CASE('-'){
					if(pri < PRI_SUB - l_space){
						ret = XTAL_NEW BinExpr(ln, CODE_SUB, lhs, parse_expr(PRI_SUB - r_space)); 
					}
				}

				XTAL_CASE('~'){
					if(pri < PRI_CAT - l_space){
						ret = XTAL_NEW BinExpr(ln, CODE_CAT, lhs, parse_expr(PRI_CAT - r_space)); 
					}
				}

				XTAL_CASE('*'){
					if(pri < PRI_MUL - l_space){
						ret = XTAL_NEW BinExpr(ln, CODE_MUL, lhs, parse_expr(PRI_MUL - r_space)); 
					}
				}

				XTAL_CASE('/'){
					if(pri < PRI_DIV - l_space){
						ret = XTAL_NEW BinExpr(ln, CODE_DIV, lhs, parse_expr(PRI_DIV - r_space)); 
					}
				}

				XTAL_CASE('%'){
					if(pri < PRI_MOD - l_space){
						ret = XTAL_NEW BinExpr(ln, CODE_MOD, lhs, parse_expr(PRI_MOD - r_space)); 
					}
				}

				XTAL_CASE('^'){
					if(pri < PRI_XOR - l_space){
						ret = XTAL_NEW BinExpr(ln, CODE_XOR, lhs, parse_expr(PRI_XOR - r_space)); 
					}
				}

				XTAL_CASE(c2('&','&')){ 
					if(pri < PRI_ANDAND - l_space){
						ret =XTAL_NEW AndAndExpr(ln, lhs, parse_expr_must(PRI_ANDAND - r_space));
					}
				}

				XTAL_CASE('&'){
					if(pri < PRI_AND - l_space){
						ret = XTAL_NEW BinExpr(ln, CODE_AND, lhs, parse_expr(PRI_AND - r_space)); 
					}
				}

				XTAL_CASE(c2('|','|')){ 
					if(pri < PRI_OROR - l_space){
						ret = XTAL_NEW OrOrExpr(ln, lhs, parse_expr(PRI_OROR - r_space));
					}
				}

				XTAL_CASE('|'){
					if(pri < PRI_OR - l_space){
						ret = XTAL_NEW BinExpr(ln, CODE_OR, lhs, parse_expr(PRI_OR - r_space)); 
					}
				}

				XTAL_CASE(c2('<','<')){ 
					if(pri < PRI_SHL - l_space){
						ret = XTAL_NEW BinExpr(ln, CODE_SHL, lhs, parse_expr(PRI_SHL - r_space)); 
					}
				}

				XTAL_CASE(c2('<','=')){ 
					if(pri < PRI_LE - l_space){
						ret = XTAL_NEW BinCompExpr(ln, CODE_LE, lhs, parse_expr(PRI_LE - r_space)); 
					}
				}

				XTAL_CASE('<'){ 
					if(pri < PRI_LT - l_space){
						ret = XTAL_NEW BinCompExpr(ln, CODE_LT, lhs, parse_expr(PRI_LT - r_space)); 
					}
				}

				XTAL_CASE(c2('>','>')){ 
					if(pri < PRI_SHR - l_space){
						ret = XTAL_NEW BinExpr(ln, CODE_SHR, lhs, parse_expr(PRI_SHR - r_space)); 
					}
				}
				
				XTAL_CASE(c3('>','>','>')){ 
					if(pri < PRI_USHR - l_space){
						ret = XTAL_NEW BinExpr(ln, CODE_USHR, lhs, parse_expr(PRI_USHR - r_space)); 
					}
				}

				XTAL_CASE(c2('>','=')){ 
					if(pri < PRI_GE - l_space){
						ret = XTAL_NEW BinCompExpr(ln, CODE_GE, lhs, parse_expr(PRI_GE - r_space)); 
					}
				}

				XTAL_CASE('>'){ 
					if(pri < PRI_GT - l_space){
						ret = XTAL_NEW BinCompExpr(ln, CODE_GT, lhs, parse_expr(PRI_GT - r_space)); 
					}
				}

				XTAL_CASE(c2('=','=')){ 
					if(pri < PRI_EQ - l_space){
						ret = XTAL_NEW BinCompExpr(ln, CODE_EQ, lhs, parse_expr(PRI_EQ - r_space)); 
					}
				}

				XTAL_CASE(c2('!','=')){ 
					if(pri < PRI_NE - l_space){
						ret = XTAL_NEW BinCompExpr(ln, CODE_NE, lhs, parse_expr(PRI_NE - r_space)); 
					}
				}

				XTAL_CASE(c3('=','=','=')){ 
					if(pri < PRI_RAW_EQ - l_space){
						ret = XTAL_NEW BinCompExpr(ln, CODE_RAW_EQ, lhs, parse_expr(PRI_RAW_EQ - r_space)); 
					}
				}

				XTAL_CASE(c3('!','=','=')){ 
					if(pri < PRI_RAW_NE - l_space){
						ret = XTAL_NEW BinCompExpr(ln, CODE_RAW_NE, lhs, parse_expr(PRI_RAW_NE - r_space)); 
					}
				}

				XTAL_CASE(c3('!','i','s')){ 
					if(pri < PRI_IS - l_space){
						ret = XTAL_NEW BinCompExpr(ln, CODE_NIS, lhs, parse_expr(PRI_IS - r_space)); 
					}
				}

				XTAL_CASE(c2(':',':')){
					if(pri < PRI_MEMBER - l_space){
						MemberExpr* p = XTAL_NEW MemberExpr(ln, lhs);
						p->var = parse_ident_or_keyword();
						ret = p;
					}
				}

				XTAL_CASE('.'){
					if(pri < PRI_SEND - l_space){
						if(eat('<')){
						//	lexer_.set_string_mode();
						//	send(lhs, false, com_.register_ident("child"));
						}else{
							SendExpr* p = XTAL_NEW SendExpr(ln, lhs);
							p->var = parse_ident_or_keyword();
							ret = p;
						}
					}
				}

				XTAL_CASE(c2('.', '?')){ 
					if(pri < PRI_SEND - l_space){
						SendExpr* p = XTAL_NEW SendExpr(ln, lhs);
						p->var = parse_ident_or_keyword();
						p->if_defined = true;
						ret = p;
					}
				}

				XTAL_CASE('?'){
					if(pri < PRI_Q - l_space){
						TerExpr* p = XTAL_NEW TerExpr(ln, lhs);
						p->second = parse_expr_must();
						expect_a(':');
						p->third = parse_expr_must();
						ret = p;
					}
				}

				XTAL_CASE('('){
					if(pri < PRI_CALL - l_space){
						ret = parse_call(lhs);
					}
				}

				XTAL_CASE('['){
					if(pri < PRI_AT - l_space){
						AtExpr* p = XTAL_NEW AtExpr(ln, lhs);
						p->index = parse_expr_must();
						expect_a(']');
						ret = p;
					}
				}
			}
		}
	}

	if(!ret){
		lexer_.putback(ch);
	}

	return ret;
}

Stmt* Parser::parse_each(int_t label, Expr* lhs){

	int_t iter_first = com_.register_ident(ID("iter_first")); 
	int_t iter_next = com_.register_ident(ID("iter_next")); 
	int_t iter_break = com_.register_ident(ID("iter_break")); 
	int_t it = com_.register_ident(ID("$it"));
	int_t itv = com_.register_ident(ID("it"));

	Stmt* s;
	CallExpr* send_expr;
	MultipleAssignStmt* mas;
	int_t ln = lexer_.line();
	
	block_begin();
		*var_info_stack_[0].on_heap_flag = *var_info_stack_[1].on_heap_flag;
		
		block_add(XTAL_NEW PushStmt(ln, lhs));

	block_begin();
		*var_info_stack_[0].on_heap_flag = *var_info_stack_[1].on_heap_flag;

		TList<Expr*> param(&alloc_);
		register_variable(it);
		param.push_back(local(it));
		if(eat('|')){ // ブロックパラメータ
			while(true){
				Token ch = lexer_.peek();
				if(ch.type()==ch.TYPE_IDENT){
					lexer_.read();
					register_variable(ch.ivalue());
					param.push_back(local(ch.ivalue()));
					if(!eat(',')){
						expect('|');
						break;
					}
				}else{
					expect('|');
					break;
				}
			}
		}else{
			register_variable(itv);
			param.push_back(local(itv));
		}

		mas = XTAL_NEW MultipleAssignStmt(ln);
		mas->lhs = param;
		mas->define = true;
		send_expr = send(XTAL_NEW PopExpr(ln), false, iter_first);
		if(param.size==2){
			send_expr->result_flag = RESULT_TO_ARRAY;
		}else if(param.size>2){
			send_expr->result_flag = RESULT_COVER_FROM_ARRAY;
		}
		mas->rhs.push_back(set_line(ln, send_expr));
		block_add(mas);
		
		try_begin();

			while_begin(0, local(it));
				Stmt* body = parse_block();
				while_label(label);
				while_body(body);
				mas = XTAL_NEW MultipleAssignStmt(ln);
				mas->lhs = param;
				mas->define = false;

				send_expr = send(local(it), false, iter_next);
				if(param.size==2){
					send_expr->result_flag = RESULT_TO_ARRAY;
				}else if(param.size>2){
					send_expr->result_flag = RESULT_COVER_FROM_ARRAY;
				}
				mas->rhs.push_back(set_line(ln, send_expr));
				while_next(mas);

				if(eat_a(Token::KEYWORD_ELSE)){
					while_else(parse_stmt_must());
					lexer_.putback();
				}else if(eat_a(Token::KEYWORD_NOBREAK)){
					while_nobreak(parse_stmt_must());
					lexer_.putback();
				}

			s = while_end();
			try_body(s);
			
			s = e2s(set_line(ln, send(local(it), true, iter_break)));
			try_finally(s);

		s = try_end();
		block_add(s);	
		s = block_end();
		block_add(s);	
	return block_end();
}

Stmt* Parser::parse_loop(){
	if(int_t ident = parse_var()){
		Token ch = lexer_.read(); // :の次を読み取る
		if(ch.type()==Token::TYPE_KEYWORD){
			switch(ch.ivalue()){
				XTAL_DEFAULT{}
				XTAL_CASE(Token::KEYWORD_FOR){ return parse_for(ident); }
				XTAL_CASE(Token::KEYWORD_WHILE){ return parse_while(ident); }
			}
		}
		
		lexer_.putback(ch);
		if(Expr* lhs = parse_expr()){
			if(eat('{')){
				return parse_each(ident, lhs);
			}else{
				expect_end();
				return def_local(ident, lhs);
			}
		}

		lexer_.putback();
		lexer_.putback();
	}
	return 0;
}

Stmt* Parser::parse_assign_stmt(){

	int_t ln = lexer_.line();
	Token ch = lexer_.read();

	if(ch.type()==Token::TYPE_TOKEN){
		switch(ch.ivalue()){
			XTAL_DEFAULT{}
			XTAL_CASE(c2('+','+')){ return XTAL_NEW IncStmt(ln, CODE_INC, parse_expr_must()); }
			XTAL_CASE(c2('-','-')){ return XTAL_NEW IncStmt(ln, CODE_DEC, parse_expr_must()); }
		}
	}
	lexer_.putback();

	if(Expr* lhs = parse_expr()){
		
		ch = lexer_.read();

		switch(ch.type()){
			XTAL_DEFAULT{}
			
			XTAL_CASE(Token::TYPE_TOKEN){
				switch(ch.ivalue()){

					XTAL_DEFAULT{
						lexer_.putback();
						return XTAL_NEW ExprStmt(ln, lhs); 
					}

					XTAL_CASE(','){
						MultipleAssignStmt* mas = XTAL_NEW MultipleAssignStmt(ln);
						mas->lhs.push_back(lhs);
						parse_multiple_expr(&mas->lhs);
						
						if(eat('=')){
							mas->define = false;
							parse_multiple_expr(&mas->rhs);
						}else if(eat(':')){
							mas->define = true;
							parse_multiple_expr(&mas->rhs);
							for(TList<Expr*>::Node* p = mas->lhs.head; p; p = p->next){
								if(LocalExpr* le = expr_cast<LocalExpr>(p->value)){
									register_variable(le->var);
								}else if(SendExpr* le = expr_cast<SendExpr>(lhs)){
								
								}else{
									com_.error(line(), Xt("構文エラー"));
								}
							}
						}else{
							com_.error(line(), Xt("構文エラー"));
						}
						
						return mas;
					}

					XTAL_CASE('='){  return XTAL_NEW AssignStmt(ln, lhs, parse_expr_must()); }

					XTAL_CASE(':'){ 
						if(LocalExpr* le = expr_cast<LocalExpr>(lhs)){
							return def_local(le->var, parse_expr_must());
						}else if(MemberExpr* le = expr_cast<MemberExpr>(lhs)){
							DefineMemberStmt* ret = XTAL_NEW DefineMemberStmt(ln, le->lhs, le->var, parse_expr_must());
							return ret;
						}else{
							com_.error(line(), Xt("構文エラー"));
						}
					}

					XTAL_CASE(c2('+','=')){ return XTAL_NEW OpAssignStmt(ln, CODE_ADD_ASSIGN, lhs, parse_expr_must()); }
					XTAL_CASE(c2('+','+')){ return XTAL_NEW IncStmt(ln, CODE_INC, lhs); }
					XTAL_CASE(c2('-','=')){ return XTAL_NEW OpAssignStmt(ln, CODE_SUB_ASSIGN, lhs, parse_expr_must()); }
					XTAL_CASE(c2('-','-')){ return XTAL_NEW IncStmt(ln, CODE_DEC, lhs); }
					XTAL_CASE(c2('~','=')){ return XTAL_NEW OpAssignStmt(ln, CODE_CAT_ASSIGN, lhs, parse_expr_must()); }
					XTAL_CASE(c2('*','=')){ return XTAL_NEW OpAssignStmt(ln, CODE_MUL_ASSIGN, lhs, parse_expr_must()); }
					XTAL_CASE(c2('/','=')){ return XTAL_NEW OpAssignStmt(ln, CODE_DIV_ASSIGN, lhs, parse_expr_must()); }
					XTAL_CASE(c2('%','=')){ return XTAL_NEW OpAssignStmt(ln, CODE_MOD_ASSIGN, lhs, parse_expr_must()); }
					XTAL_CASE(c2('^','=')){ return XTAL_NEW OpAssignStmt(ln, CODE_XOR_ASSIGN, lhs, parse_expr_must()); }
					XTAL_CASE(c2('|','=')){ return XTAL_NEW OpAssignStmt(ln, CODE_OR_ASSIGN, lhs, parse_expr_must()); }
					XTAL_CASE(c2('&','=')){ return XTAL_NEW OpAssignStmt(ln, CODE_AND_ASSIGN, lhs, parse_expr_must()); }
					XTAL_CASE(c3('<','<','=')){ return XTAL_NEW OpAssignStmt(ln, CODE_SHL_ASSIGN, lhs, parse_expr_must()); }
					XTAL_CASE(c3('>','>','=')){ return XTAL_NEW OpAssignStmt(ln, CODE_SHR_ASSIGN, lhs, parse_expr_must()); }
					XTAL_CASE(c4('>','>','>','=')){ return XTAL_NEW OpAssignStmt(ln, CODE_USHR_ASSIGN, lhs, parse_expr_must()); }

					XTAL_CASE('{'){
						return parse_each(0, lhs);
					}
				}
			}
		}
	}

	return 0;
}

Stmt* Parser::parse_stmt2(){

	if(Stmt* p = parse_loop()){
		return p;
	}

	Token ch = lexer_.read();

	switch(ch.type()){
		XTAL_DEFAULT{}
		
		XTAL_CASE(Token::TYPE_KEYWORD){
			switch(ch.ivalue()){
				XTAL_CASE(Token::KEYWORD_RETURN){ return parse_return(); }
				XTAL_CASE(Token::KEYWORD_CONTINUE){ return parse_continue(); }
				XTAL_CASE(Token::KEYWORD_BREAK){ return parse_break(); }
				XTAL_CASE(Token::KEYWORD_FOR){ return parse_for(); }
				XTAL_CASE(Token::KEYWORD_WHILE){ return parse_while(); }
				XTAL_CASE(Token::KEYWORD_IF){ return parse_if(); }
				XTAL_CASE(Token::KEYWORD_THROW){ return parse_throw(); }
				XTAL_CASE(Token::KEYWORD_TRY){ return parse_try(); }
				XTAL_CASE(Token::KEYWORD_ASSERT){ return parse_assert(); }
				
				XTAL_CASE(Token::KEYWORD_YIELD){ 
					YieldStmt* p = XTAL_NEW YieldStmt(line()); 
					parse_multiple_expr(&p->exprs);
					expect_end();
					return p;
				}
			}
		}
		
		XTAL_CASE(Token::TYPE_TOKEN){
			switch(ch.ivalue()){
				XTAL_CASE('{'){ Stmt* p = parse_block(); return p; }
				XTAL_CASE(';'){ return 0; }
			}
		}
	}
	
	lexer_.putback();

	if(Stmt* stmt = parse_assign_stmt()){
		expect_end();
		return stmt;
	}
	return 0;
}

Stmt* Parser::parse_stmt(){
	Stmt* ret = parse_stmt2();
	if(ret){
		eat_end();
	}
	return ret;
}
	
Stmt* Parser::parse_stmt_must(){
	Stmt* ret = parse_stmt();
	if(!ret){
		com_.error(line(), Xt("構文エラー"));
	}
	return ret;
}

Stmt* Parser::parse_assert(){
	AssertStmt* p = XTAL_NEW AssertStmt(lexer_.line());

	lexer_.begin_record();
	if(Expr* e = parse_expr()){
		String ref_str(lexer_.end_record());
		p->exprs.push_back(e);
		p->exprs.push_back(XTAL_NEW StringExpr(lexer_.line(), com_.register_value(ref_str)));

		if(eat_a(',')){
			if(Expr* e = parse_expr()){
				p->exprs.push_back(e);
			}
		}
	}else{
		lexer_.end_record();
	}

	parse_multiple_expr(&p->exprs);
	expect_end();
	return p;
}

Expr* Parser::string2expr(string_t& str){
	String ref_str(str);
	StringExpr* e = XTAL_NEW StringExpr(lexer_.line(), com_.register_value(ref_str));
	str="";
	return e;
}
	
void Parser::parse_multiple_expr(TList<Expr*>* exprs){
	while(1){
		if(Expr* p = parse_expr()){
			exprs->push_back(p);
			if(eat_a(',')){

			}else{
				break;
			}
		}else{
			break;
		}
	}
}

void Parser::parse_multiple_stmt(TList<Stmt*>* stmts){
	while(1){
		while(eat_end()){}
		if(Stmt* p = parse_stmt()){
			stmts->push_back(p);
		}else{
			break;
		}
	}
}

int_t Parser::parse_ident(){
	if(lexer_.peek().type()==Token::TYPE_IDENT){
		return lexer_.read().ivalue();
	}
	return 0;
}

int_t Parser::parse_ident_or_keyword(){
	if(lexer_.peek().type()==Token::TYPE_IDENT){
		return lexer_.read().ivalue();
	}else if(lexer_.peek().type()==Token::TYPE_KEYWORD){
		return com_.register_ident(lexer_.keyword2id(lexer_.read().ivalue()));
	}
	return 0;
}

int_t Parser::parse_var(){
	if(int_t ident = parse_ident()){
		if(eat(':')){ 
			return ident; 
		}else{
			lexer_.putback();
		}
	}
	return 0;
}
	
Stmt* Parser::parse_for(int_t label){
	Stmt* s;

	block_begin();
		expect('(');
		block_add(parse_assign_stmt());
		expect(';');
	
		while_begin(0, parse_expr());
			while_label(label);
			expect(';');
			
			while_next(parse_assign_stmt());
			expect_a(')');

			while_body(parse_stmt_must());

			if(eat(Token::KEYWORD_ELSE)){
				while_else(parse_stmt_must());
			}else if(eat(Token::KEYWORD_NOBREAK)){
				while_nobreak(parse_stmt_must());
			}

		s = while_end();
		block_add(s);
	
	return block_end();
}

Stmt* Parser::parse_top_level(){
	TopLevelStmt* top = XTAL_NEW TopLevelStmt(lexer_.line());
	push_scope(&top->vars, &top->on_heap, true);
	set_scope_on_heap_flag(0);
	
	while(1){
		while(eat_end()){}
		if(Stmt* p = parse_stmt()){
			top->stmts.push_back(p);
		}else if(eat(Token::KEYWORD_EXPORT)){
			if(top->export_expr){
				com_.error(line(), Xt("export出来るのは一つのファイルで一つだけです"));
			}else{
				int_t export_id = com_.register_ident(ID("$export"));
				int_t ident = parse_var();
				
				Expr* expr = parse_expr_must();
				expect_end();
				
				if(ident){
					top->stmts.push_back(def_local(ident, expr));
					top->stmts.push_back(def_local(export_id, local(ident)));
				}else{
					top->stmts.push_back(def_local(export_id, expr));
				}
				top->export_expr = local(export_id);
			}
		}else if(eat(Token::KEYWORD_UNITTEST)){
			top->unittest_stmt = def_local(Token::KEYWORD_UNITTEST, parse_fun(KIND_FUN));
		}else{
			break;
		}
	}
	
	expect(-1);
	pop_scope();
	return top;
}

Stmt* Parser::parse_block(){
	BlockStmt* p = XTAL_NEW BlockStmt(lexer_.line());
	push_scope(&p->vars, &p->on_heap, false);
	parse_multiple_stmt(&p->stmts);
	expect('}');
	pop_scope();
	return p;
}

Expr* Parser::parse_class_or_module(int_t kind){
	ClassExpr* p = XTAL_NEW ClassExpr(lexer_.line());
	p->kind = kind;

	push_scope(&p->vars, &p->on_heap, true);
	set_scope_on_heap_flag(0);

	if(eat('(')){
		parse_multiple_expr(&p->mixins);
		expect_a(')');
	}

	AC<std::pair<int_t, Expr*> >::vector insts;

	expect('{');
	while(true){
		if(int_t var = parse_var()){
			if(eat('@')){
				if(int_t var2 = parse_ident()){
					bool found = false;
					for(TList<int_t>::Node* np = p->inst_vars.head; np; np = np->next){
						if(np->value==var2){
							found = true;
							break;
						}
					}
					if(!found){
						p->inst_vars.push_back(var2);
					}

					if(eat('=')){
						fun_begin(KIND_METHOD);
						fun_param(com_.register_ident(Xid(value)));
						AssignStmt *as = XTAL_NEW AssignStmt(lexer_.line());
						as->lhs = XTAL_NEW InstanceVariableExpr(lexer_.line(), var2);
						as->rhs = local(com_.register_ident(Xid(value)));
						fun_body(as);
						p->stmts.push_back(def_local(var, fun_end()));
					}else{
						fun_begin(KIND_METHOD);
						fun_body(return_(XTAL_NEW InstanceVariableExpr(lexer_.line(), var2)));
						p->stmts.push_back(def_local(var, fun_end()));
					}

					expect_end();
				}
			}else{
				p->stmts.push_back(def_local(var, parse_expr_must()));
				expect_end();
			}
		}else if(eat('@')){
			if(int_t var2 = parse_ident()){
				var = var2;
				if(eat(':')){
					insts.push_back(std::make_pair(var, parse_expr_must()));
				}
				expect_end();

				for(TList<int_t>::Node* np = p->inst_vars.head; np; np = np->next){
					if(np->value==var){
						com_.error(line(), Xt("同じインスタンス変数名 '%(name)s' が重複定義されました")(Xid(name)=var));
						break;
					}
				}
				p->inst_vars.push_back(var);
			}				
		}else if(int_t var = parse_ident()){
			DefineMemberStmt* dp = XTAL_NEW DefineMemberStmt(line(), 0);
			dp->var = var;

			if(eat(':')){
				dp->lhs = XTAL_NEW PseudoVariableExpr(line(), CODE_PUSH_CURRENT_CONTEXT);
				dp->rhs = parse_expr_must();
			}else{
				com_.error(line(), Xt("構文エラー"));
			}
		
			p->stmts.push_back(dp);
			expect_end();

		}else{
			break;
		}
	}

	Stmt* s;
	Expr* e;
	fun_begin(KIND_METHOD);
		block_begin();
			for(int_t i = 0; i<(int_t)insts.size(); ++i){
				block_add(set_inst(insts[i].first, insts[i].second));
			}
		s = block_end();
		fun_body(s);
	e = fun_end();
	p->stmts.push_front(def_local(com_.register_ident("$special_initialize"), e));

	expect('}');
	pop_scope();
	return p;
}

Stmt* Parser::parse_define_local_stmt(){
	if(int_t var = parse_var()){
		Stmt* p = def_local(var, parse_expr());
		expect_end();
		return p;
	}	
	return 0;
}

Expr* Parser::parse_define_local_or_expr(){
	if(int_t var = parse_var()){
		register_variable(var);
	}
	return parse_expr();
}

Stmt* Parser::parse_try(){
	try_begin();

		try_body(parse_stmt_must());
	
		if(eat(Token::KEYWORD_CATCH)){
			expect('(');
			register_variable(parse_ident());
			expect(')');
			try_catch(parse_stmt_must());
		}
	
		if(eat(Token::KEYWORD_FINALLY)){
			try_finally(parse_stmt_must());
		}

	return try_end();
}

Expr* Parser::parse_fun(int_t kind){

	FunExpr* e = XTAL_NEW FunExpr(lexer_.line(), kind);
	push_scope(&e->vars, &e->on_heap, false);
	set_scope_on_heap_flag(1);

	if(eat('(')){

		while(true){
			
			if(eat_a(')')){
				break;
			}
				
			if(eat(c3('.','.','.'))){
				e->have_args = true;
				expect(')');
				break;
			}
			
			if(int_t var = parse_ident()){
				register_variable(var);
				if(eat(':')){
					e->params.push_back(var, parse_expr_must());
				}else{
					e->params.push_back(var, 0);
				}
			}
			
			if(eat_a(',')){
				if(eat_a(')')){
					break;
				}
			}else{
				expect_a(')');
				break;
			}
		}
	}

	if(eat('{')){
		e->stmt = parse_block();
	}else{
		e->stmt = return_(parse_expr_must());
	}
	pop_scope();
	return e;
}

Expr* Parser::parse_call(Expr* lhs){
	CallExpr* e = XTAL_NEW CallExpr(lexer_.line(), lhs);
	while(true){

		if(int_t var = parse_var()){
			e->named.push_back(var, parse_expr());
		}else{
			e->ordered.push_back(parse_expr());
		}
		
		if(eat_a(',')){
			if(eat_a(')')){
				if(e->ordered.tail && !e->ordered.tail->value){
					e->ordered.pop_back();
				}
				break;
			}
		}else{
			expect_a(')');
			if(e->ordered.tail && !e->ordered.tail->value){
				e->ordered.pop_back();
			}
			break;
		}
	}

	return e;
}

Expr* Parser::parse_expr(){
	Expr* ret = parse_expr(0);
	return ret;
}

Expr* Parser::parse_expr(int_t pri){
	
	Expr* ret = parse_term();
	if(!ret){
		return 0;
	}
	
	while(true){
		if(Expr* ret2 = parse_post(ret, pri)){
			ret = ret2;
		}else{
			break;
		}
	}
	
	return ret;
}

Expr* Parser::parse_expr_must(int_t pri){
	Expr* ret = parse_expr(pri);
	if(!ret){
		com_.error(line(), Xt("構文エラー"));
	}
	return ret;
}

Expr* Parser::parse_expr_must(){
	Expr* ret = parse_expr();
	if(!ret){
		com_.error(line(), Xt("構文エラー"));
	}
	return ret;
}

Stmt* Parser::parse_return(){
	ReturnStmt* p = XTAL_NEW ReturnStmt(lexer_.line());
	parse_multiple_expr(&p->exprs);
	expect_end();
	return p;
}

Stmt* Parser::parse_continue(){
	ContinueStmt* p = XTAL_NEW ContinueStmt(lexer_.line());
	p->var = parse_ident();
	expect_end();
	return p;
}

Stmt* Parser::parse_break(){
	BreakStmt* p = XTAL_NEW BreakStmt(lexer_.line());
	p->var = parse_ident();
	expect_end();
	return p;
}

Stmt* Parser::parse_if(){
	expect('(');
	int_t var = parse_var();
	if_begin(var, parse_expr_must());
		expect_a(')');
		if_body(parse_stmt_must());
		if(eat(Token::KEYWORD_ELSE)){
			if_else(parse_stmt_must());
		}
	return if_end();
}

Stmt* Parser::parse_while(int_t label){
	expect('(');
	int_t var = parse_var();
	while_begin(var, parse_expr_must());
		while_label(label);
		expect_a(')');
		while_body(parse_stmt_must());
		if(eat(Token::KEYWORD_ELSE)){
			while_else(parse_stmt_must());
		}else if(eat(Token::KEYWORD_NOBREAK)){
			while_nobreak(parse_stmt_must());
		}
	return while_end();
}

Stmt* Parser::parse_throw(){
	UnaStmt* p = XTAL_NEW UnaStmt(lexer_.line(), CODE_THROW);
	p->expr = parse_expr();
	expect_end();
	return p;
}

Expr* Parser::parse_array(){
	
	int_t ln = lexer_.line();

	if(eat(']')){//empty array
		return XTAL_NEW ArrayExpr(ln);
	}
	
	if(eat(':')){//empty map
		expect(']');
		return XTAL_NEW MapExpr(ln);
	}
	
	Expr* ret = 0;
	
	Expr* key = parse_expr_must();
	if(eat_a(':')){//map
		MapExpr* e = XTAL_NEW MapExpr(ln);
		ret = e;
		e->values.push_back(key, parse_expr_must());	
		
		if(eat_a(',')){
			while(true){
				if((key = parse_expr())){
					expect_a(':');
					e->values.push_back(key, parse_expr_must());
					
					if(!eat_a(',')){
						break;
					}
				}else{
					break;
				}
			}
		}
	}else{//array
		ArrayExpr* e = XTAL_NEW ArrayExpr(ln);
		ret = e;
		e->values.push_back(key);
		if(eat_a(',')){
			while(true){
				if((key = parse_expr())){
					e->values.push_back(key);
					if(!eat_a(',')){
						break;
					}
				}else{
					break;
				}
			}
		}
	}
	
	expect_a(']');
	
	return ret;
}

void Parser::push_scope(TList<int_t>* list, bool* on_heap, bool set_name_flag){
	VarInfo vi = {list, on_heap, set_name_flag};
	var_info_stack_.push(vi);
	*on_heap = false;
}

void Parser::set_scope_on_heap_flag(int_t i){
	*var_info_stack_[i].on_heap_flag = true;
}

void Parser::set_scope_set_name_flag(bool b){

}

void Parser::pop_scope(){
	var_info_stack_.pop();	
}

void Parser::register_variable(int_t var){
	for(TList<int_t>::Node* p = var_info_stack_.top().variables->head; p; p = p->next){
		if(p->value == var){
			com_.error(line(), Xt("同じスコープ内で、同じ変数名 '%(name)s' が重複定義されました")(
				Xid(name)=com_.ident_table[var]
			));
		}
	}
	var_info_stack_.top().variables->push_back(var);
}



void Parser::block_begin(){
	block_stack_.push(new(&alloc_) BlockStmt(lexer_.line()));
	push_scope(&block_stack_.top()->vars, &block_stack_.top()->on_heap, false);
}

void Parser::block_add(Stmt* stmt){
	block_stack_.top()->stmts.push_back(stmt);
}

BlockStmt* Parser::block_end(){
	pop_scope();
	return block_stack_.pop();
}
	
Expr* Parser::inst(int_t var){
	return new(&alloc_) InstanceVariableExpr(lexer_.line(), var);
}
	
Stmt* Parser::set_inst(int_t var, Expr* rhs){
	return new(&alloc_) AssignStmt(lexer_.line(), inst(var), rhs);
}

Expr* Parser::local(int_t var){
	return new(&alloc_) LocalExpr(lexer_.line(), var);
}

DefineLocalStmt* Parser::def_local(int_t var, Expr* expr){
	register_variable(var);
	return new(&alloc_) DefineLocalStmt(lexer_.line(), var, expr, var_info_stack_.top().set_name_flag);
}

Stmt* Parser::set_local(int_t var, Expr* expr){
	return new(&alloc_) AssignStmt(lexer_.line(), local(var), expr);
}

CallExpr* Parser::call(Expr* lhs, Expr* a1, Expr* a2){
	CallExpr* ret = new(&alloc_) CallExpr(lexer_.line(), lhs);
	if(a1)ret->ordered.push_back(a1);
	if(a2)ret->ordered.push_back(a2);
	return ret;
}

Expr* Parser::member(Expr* lhs, bool if_defined, int_t var){
	MemberExpr* ret = new(&alloc_) MemberExpr(lexer_.line(), lhs, var);
	ret->if_defined = if_defined;
	return ret;
}

CallExpr* Parser::send(Expr* lhs, bool if_defined, int_t var, Expr* a1, Expr* a2){
	SendExpr* ret = new(&alloc_) SendExpr(lexer_.line(), lhs, var);
	ret->if_defined = if_defined;
	return call(ret, a1, a2);
}

Stmt* Parser::e2s(Expr* expr){
	return new(&alloc_) ExprStmt(lexer_.line(), expr);
}

void Parser::try_begin(){
	try_stack_.push(new(&alloc_) TryStmt(lexer_.line()));
	push_scope(&try_stack_.top()->catch_vars, &try_stack_.top()->on_heap, false);
}

void Parser::try_body(Stmt* stmt){
	try_stack_.top()->try_stmt = stmt;
}

void Parser::try_catch(Stmt* stmt){
	try_stack_.top()->catch_stmt = stmt;
}

void Parser::try_finally(Stmt* stmt){
	try_stack_.top()->finally_stmt = stmt;
}

TryStmt* Parser::try_end(){
	pop_scope();
	return try_stack_.pop();
}

void Parser::while_begin(int_t var, Expr* expr){
	while_stack_.push(new(&alloc_) WhileStmt(lexer_.line()));
	block_begin();
	if(var){
		block_add(def_local(var, expr));
		while_stack_.top()->cond_expr = local(var);
	}else{
		while_stack_.top()->cond_expr = expr;
	}
}

void Parser::while_label(int_t label){
	while_stack_.top()->label = label;
}

void Parser::while_body(Stmt* stmt){
	while_stack_.top()->body_stmt = stmt;
}

void Parser::while_next(Stmt* stmt){
	while_stack_.top()->next_stmt = stmt;
}

void Parser::while_else(Stmt* stmt){
	while_stack_.top()->else_stmt = stmt;
}

void Parser::while_nobreak(Stmt* stmt){
	while_stack_.top()->nobreak_stmt = stmt;
}

Stmt* Parser::while_end(){
	block_add(while_stack_.pop());
	return block_end();
}

void Parser::if_begin(int_t var, Expr* expr){
	if_stack_.push(new(&alloc_) IfStmt(lexer_.line()));
	block_begin();
	if(var){
		block_add(def_local(var, expr));
		if_stack_.top()->cond_expr = local(var);
	}else{
		if_stack_.top()->cond_expr = expr;
	}
}

void Parser::if_body(Stmt* stmt){
	if_stack_.top()->body_stmt = stmt;
}

void Parser::if_else(Stmt* stmt){
	if_stack_.top()->else_stmt = stmt;
}

Stmt* Parser::if_end(){
	block_add(if_stack_.pop());
	return block_end();
}

void Parser::fun_begin(int_t kind){
	fun_stack_.push(new(&alloc_) FunExpr(lexer_.line(), kind));
	push_scope(&fun_stack_.top()->vars, &fun_stack_.top()->on_heap, false);
	set_scope_on_heap_flag(1);
}

void Parser::fun_param(int_t name, Expr* def){
	register_variable(name);
	fun_stack_.top()->params.push_back(name, def);
}

void Parser::fun_body(Stmt* stmt){
	fun_stack_.top()->stmt = stmt;
}

FunExpr* Parser::fun_end(){
	pop_scope();
	return fun_stack_.pop();
}

Stmt* Parser::return_(Expr* e1, Expr* e2){
	ReturnStmt* ret = new(&alloc_) ReturnStmt(lexer_.line());
	if(e1)ret->exprs.push_back(e1);
	if(e2)ret->exprs.push_back(e2);
	return ret;
}

Stmt* Parser::assert_(Expr* e1, Expr* e2){
	AssertStmt* ret = new(&alloc_) AssertStmt(lexer_.line());
	if(e1)ret->exprs.push_back(e1);
	if(e2)ret->exprs.push_back(e2);
	return ret;
}

}
