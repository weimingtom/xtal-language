
#include "xtal.h"

#ifndef XTAL_NO_PARSER

#include <math.h>

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
		PRI_NIS = PRI_EQ,

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
	com_->error(line(), Xt("Xtal Compile Error 1003"));
}

Stmt* Parser::parse(const Stream& stream, const String& source_file_name){
	lexer_.init(stream, source_file_name);
	com_ = lexer_.common();
	e.init(com_, &alloc_);

	Stmt* p = parse_top_level();

	if(com_->errors.size()!=0){
		return 0;
	}
	return p;
}

void Parser::begin_interactive_parsing(const Stream& stream){
	lexer_.init(stream, "<ix>");
	com_ = lexer_.common();
	e.init(com_, &alloc_);

	e.toplevel_begin();
}

Stmt* Parser::interactive_parse(){
	if(eat_end())
		return e.e2s(e.pseudo(CODE_PUSH_NULL));
	
	if(Stmt* p = parse_stmt())
		return p;

	Token tok = lexer_.read();
	if(tok.type()==Token::TYPE_TOKEN && tok.ivalue()==-1)
		return 0;

	return e.e2s(e.pseudo(CODE_PUSH_NULL));
}

LPCCommon* Parser::common(){
	return com_;
}

ExprBuilder* Parser::expr_builder(){
	return &e;
}

void Parser::release(){
	alloc_.release();
}
	
void Parser::expect(int_t ch){
	if(eat(ch)){
		return;
	}		
	com_->error(line(), Xt("Xtal Compile Error 1002")(Named("char", lexer_.token2str(lexer_.peek()))));
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
			com_->error(line(), Xt("Xtal Compile Error 1011"));
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
				str+=(char_t)ch;
				for(int_t i=1, size = ch_len(ch); i<size; ++i){
					str+=(char_t)lexer_.read().ivalue();
				}
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
				
				XTAL_CASE('|'){
					ret = parse_fun(KIND_FUN, true);
				}

				XTAL_CASE(c2('|', '|')){
					e.fun_begin(KIND_FUN);
					e.fun_body(e.return_(parse_expr_must()));
					ret = e.fun_end();
				}

				XTAL_CASE('_'){
					ret = e.instance_variable(parse_ident());
				}

				XTAL_CASE('"'){ //"
					lexer_.set_string_mode();
					String str(parse_string('"', '"'));
					ret = e.string(com_->register_value(str));
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
								com_->error(line(), Xt("Xtal Compile Error 1017"));
							}
							kind = KIND_TEXT;
							open = lexer_.read().ivalue();
							continue;

						case 'f':
							if(kind!=KIND_STRING){
								com_->error(line(), Xt("Xtal Compile Error 1017"));
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
							com_->error(line(), Xt("Xtal Compile Error 1017"));
							break;
						}

						break;
					}
				
					String str(parse_string(open, close));
					ret = e.string(com_->register_value(str), kind);
				}
				
				XTAL_CASE(c3('.','.','.')){ 
					ret = e.args();
				}

////////////////////////////////////////////////////////////////////////////////////////

				XTAL_CASE('+'){
					if(Expr* rhs = parse_expr(PRI_POS - r_space)){
						ret = e.una(CODE_POS, rhs);
					}
				}

				XTAL_CASE('-'){ 
					if(Expr* rhs = parse_expr(PRI_NEG - r_space)){
						ret = e.una(CODE_NEG, rhs);
					}
				}

				XTAL_CASE('~'){ 
					if(Expr* rhs = parse_expr(PRI_COM - r_space)){
						ret = e.una(CODE_COM, rhs);
					}
				}

				XTAL_CASE('!'){ 
					if(Expr* rhs = parse_expr(PRI_NOT - r_space)){
						ret = e.una(CODE_NOT, rhs);
					}
				}
			}
		}

		XTAL_CASE(Token::TYPE_KEYWORD){
			switch(ch.ivalue()){

				XTAL_DEFAULT{}
				
				XTAL_CASE(Token::KEYWORD_ONCE){ ret = e.once(parse_expr_must(PRI_ONCE - r_space*2)); }
				XTAL_CASE(Token::KEYWORD_CLASS){ ret = parse_class(); }
				XTAL_CASE(Token::KEYWORD_FUN){ ret = parse_fun(KIND_FUN); }
				XTAL_CASE(Token::KEYWORD_METHOD){ ret = parse_fun(KIND_METHOD); }
				XTAL_CASE(Token::KEYWORD_FIBER){ ret = parse_fun(KIND_FIBER); }
				XTAL_CASE(Token::KEYWORD_DOFUN){ ret = e.call(parse_fun(KIND_FUN)); }
				XTAL_CASE(Token::KEYWORD_CALLEE){ ret = e.callee(); }
				XTAL_CASE(Token::KEYWORD_NULL){ ret = e.pseudo(CODE_PUSH_NULL); }
				XTAL_CASE(Token::KEYWORD_TRUE){ ret = e.pseudo(CODE_PUSH_TRUE); }
				XTAL_CASE(Token::KEYWORD_FALSE){ ret = e.pseudo(CODE_PUSH_FALSE); }
				XTAL_CASE(Token::KEYWORD_NOP){ ret = e.pseudo(CODE_PUSH_NOP); }
				XTAL_CASE(Token::KEYWORD_THIS){ ret = e.pseudo(CODE_PUSH_THIS); }
				XTAL_CASE(Token::KEYWORD_NEED_RESULT){ ret = e.pseudo(CODE_PUSH_NEED_RESULT); }
				
				XTAL_CASE(Token::KEYWORD_CURRENT_CONTEXT){
					e.scope_set_on_heap_flag(0);
					ret = e.pseudo(CODE_PUSH_CURRENT_CONTEXT); 
				}
				
				XTAL_CASE(Token::KEYWORD_CURRENT_CONTINUATION){
					e.scope_set_on_heap_flag(0);
					ret = e.pseudo(CODE_PUSH_CURRENT_CONTINUATION); 
				}
			}
		}
		
		XTAL_CASE(Token::TYPE_INT){
			ret = e.int_(ch.ivalue());
		}

		XTAL_CASE(Token::TYPE_FLOAT){
			ret = e.float_(ch.fvalue());
		}

		XTAL_CASE(Token::TYPE_IDENT){
			ret = e.local(ch.ivalue());
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
						ret = e.bin_comp(CODE_IS, lhs, parse_expr(PRI_IS - r_space)); 
					}
				}
			}
		}
		
		XTAL_CASE(Token::TYPE_TOKEN){
			switch(ch.ivalue()){

				XTAL_DEFAULT{
					//com_->error(line(), "構文エラーです");
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
						ret = e.bin(CODE_ADD, lhs, parse_expr(PRI_ADD - r_space)); 
					}
				}

				XTAL_CASE('-'){
					if(pri < PRI_SUB - l_space){
						ret = e.bin(CODE_SUB, lhs, parse_expr(PRI_SUB - r_space)); 
					}
				}

				XTAL_CASE('~'){
					if(pri < PRI_CAT - l_space){
						ret = e.bin(CODE_CAT, lhs, parse_expr(PRI_CAT - r_space)); 
					}
				}

				XTAL_CASE('*'){
					if(pri < PRI_MUL - l_space){
						ret = e.bin(CODE_MUL, lhs, parse_expr(PRI_MUL - r_space)); 
					}
				}

				XTAL_CASE('/'){
					if(pri < PRI_DIV - l_space){
						ret = e.bin(CODE_DIV, lhs, parse_expr(PRI_DIV - r_space)); 
					}
				}

				XTAL_CASE('%'){
					if(pri < PRI_MOD - l_space){
						ret = e.bin(CODE_MOD, lhs, parse_expr(PRI_MOD - r_space)); 
					}
				}

				XTAL_CASE('^'){
					if(pri < PRI_XOR - l_space){
						ret = e.bin(CODE_XOR, lhs, parse_expr(PRI_XOR - r_space)); 
					}
				}

				XTAL_CASE(c2('&','&')){ 
					if(pri < PRI_ANDAND - l_space){
						ret = e.andand(lhs, parse_expr_must(PRI_ANDAND - r_space));
					}
				}

				XTAL_CASE('&'){
					if(pri < PRI_AND - l_space){
						ret = e.bin(CODE_AND, lhs, parse_expr(PRI_AND - r_space)); 
					}
				}

				XTAL_CASE(c2('|','|')){ 
					if(pri < PRI_OROR - l_space){
						ret = e.oror(lhs, parse_expr(PRI_OROR - r_space));
					}
				}

				XTAL_CASE('|'){
					if(pri < PRI_OR - l_space){
						ret = e.bin(CODE_OR, lhs, parse_expr(PRI_OR - r_space)); 
					}
				}

				XTAL_CASE(c2('<','<')){ 
					if(pri < PRI_SHL - l_space){
						ret = e.bin(CODE_SHL, lhs, parse_expr(PRI_SHL - r_space)); 
					}
				}
				
				XTAL_CASE(c2('>','>')){ 
					if(pri < PRI_SHR - l_space){
						ret = e.bin(CODE_SHR, lhs, parse_expr(PRI_SHR - r_space)); 
					}
				}
				
				XTAL_CASE(c3('>','>','>')){ 
					if(pri < PRI_USHR - l_space){
						ret = e.bin(CODE_USHR, lhs, parse_expr(PRI_USHR - r_space)); 
					}
				}

				XTAL_CASE(c2('<','=')){ 
					if(pri < PRI_LE - l_space){
						ret = e.bin_comp(CODE_LE, lhs, parse_expr(PRI_LE - r_space)); 
					}
				}

				XTAL_CASE('<'){ 
					if(pri < PRI_LT - l_space){
						ret = e.bin_comp(CODE_LT, lhs, parse_expr(PRI_LT - r_space)); 
					}
				}

				XTAL_CASE(c2('>','=')){ 
					if(pri < PRI_GE - l_space){
						ret = e.bin_comp(CODE_GE, lhs, parse_expr(PRI_GE - r_space)); 
					}
				}

				XTAL_CASE('>'){ 
					if(pri < PRI_GT - l_space){
						ret = e.bin_comp(CODE_GT, lhs, parse_expr(PRI_GT - r_space)); 
					}
				}

				XTAL_CASE(c2('=','=')){ 
					if(pri < PRI_EQ - l_space){
						ret = e.bin_comp(CODE_EQ, lhs, parse_expr(PRI_EQ - r_space)); 
					}
				}

				XTAL_CASE(c2('!','=')){ 
					if(pri < PRI_NE - l_space){
						ret = e.bin_comp(CODE_NE, lhs, parse_expr(PRI_NE - r_space)); 
					}
				}

				XTAL_CASE(c3('=','=','=')){ 
					if(pri < PRI_RAW_EQ - l_space){
						ret = e.bin_comp(CODE_RAW_EQ, lhs, parse_expr(PRI_RAW_EQ - r_space)); 
					}
				}

				XTAL_CASE(c3('!','=','=')){ 
					if(pri < PRI_RAW_NE - l_space){
						ret = e.bin_comp(CODE_RAW_NE, lhs, parse_expr(PRI_RAW_NE - r_space)); 
					}
				}

				XTAL_CASE(c3('!','i','s')){ 
					if(pri < PRI_NIS - l_space){
						ret = e.bin_comp(CODE_NIS, lhs, parse_expr(PRI_NIS - r_space)); 
					}
				}

				XTAL_CASE(c2(':',':')){
					if(pri < PRI_MEMBER - l_space){
						if(eat('(')){
							ret = e.member(lhs, parse_expr_must());
							expect_a(')');
						}else{
							ret = e.member(lhs, parse_ident_or_keyword());
						}
					}
				}

				XTAL_CASE(c3(':',':','?')){
					if(pri < PRI_MEMBER - l_space){
						if(eat('(')){
							ret = e.member_q(lhs, parse_expr_must());
							expect_a(')');
						}else{
							ret = e.member_q(lhs, parse_ident_or_keyword());
						}
					}
				}

				XTAL_CASE('.'){
					if(pri < PRI_SEND - l_space){
						if(eat('(')){
							ret = e.send(lhs, parse_expr_must());
							expect_a(')');
						}else{
							ret = e.send(lhs, parse_ident_or_keyword());
						}
					}
				}

				XTAL_CASE(c2('.', '?')){ 
					if(pri < PRI_SEND - l_space){
						if(eat('(')){
							ret = e.send_q(lhs, parse_expr_must());
							expect_a(')');
						}else{
							ret = e.send_q(lhs, parse_ident_or_keyword());
						}
					}
				}

				XTAL_CASE('?'){
					if(pri < PRI_Q - l_space){
						e.ter_begin(lhs);
						e.ter_true(parse_expr_must());
						expect_a(':');
						e.ter_false(parse_expr_must());
						ret = e.ter_end();
					}
				}

				XTAL_CASE('('){
					if(pri < PRI_CALL - l_space){
						ret = parse_call(lhs);
					}
				}

				XTAL_CASE('['){
					if(pri < PRI_AT - l_space){
						ret = e.at(lhs, parse_expr_must());
						expect_a(']');
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

	int_t iter_first = com_->register_ident(ID("iter_first")); 
	int_t iter_next = com_->register_ident(ID("iter_next")); 
	int_t iter_break = com_->register_ident(ID("iter_break")); 
	int_t it = com_->register_ident(ID("__IT__"));
	int_t itv = com_->register_ident(ID("it"));

	Stmt* s;
	int_t ln = lexer_.line();
	
	e.block_begin();
		e.scope_carry_on_heap_flag();
		e.block_add(e.push(lhs));

		e.block_begin();
			e.scope_carry_on_heap_flag();

			TList<Expr*> param;
			e.register_variable(it);
			param.push_back(e.local(it), &alloc_);
			bool discard = false;
			if(eat('|')){ // ブロックパラメータ
				while(true){
					Token ch = lexer_.peek();
					if(ch.type()==ch.TYPE_IDENT){
						discard = false;
						lexer_.read();
						e.register_variable(ch.ivalue());
						param.push_back(e.local(ch.ivalue()), &alloc_);
						if(!eat(',')){
							expect('|');
							break;
						}else{
							discard = true;
						}
					}else{
						expect('|');
						break;
					}
				}
			}else{
				e.register_variable(itv);
				param.push_back(e.local(itv), &alloc_);
			}

			e.massign_begin();
			*e.massign_lhs_exprs() = param;
			e.massign_define(true);
			e.massign_rhs(e.call(e.send(e.pop(), iter_first)));
			e.block_add(e.massign_end());
			
			e.try_begin();

				e.while_begin(0, e.local(it));
					Stmt* body = parse_block();
					e.while_label(label);
					e.while_body(body);

					e.massign_begin();
					*e.massign_lhs_exprs() = param;
					e.massign_define(false);
					e.massign_rhs(e.call(e.send(e.local(it), iter_next)));
					e.while_next(e.massign_end());

					if(eat_a(Token::KEYWORD_ELSE)){
						e.while_else(parse_stmt_must());
						lexer_.putback();
					}else if(eat_a(Token::KEYWORD_NOBREAK)){
						e.while_nobreak(parse_stmt_must());
						lexer_.putback();
					}

				s = e.while_end();
				e.try_body(s);
				
				s = e.e2s(set_line(ln, e.call(e.send_q(e.local(it), iter_break))));
				e.try_finally(s);

			s = e.try_end();
			e.block_add(s);	
		s = e.block_end();
		e.block_add(s);	
	return e.block_end();
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
				return e.define(e.local(ident), lhs);
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
			XTAL_CASE(c2('+','+')){ return e.inc(CODE_INC, parse_expr_must()); }
			XTAL_CASE(c2('-','-')){ return e.inc(CODE_DEC, parse_expr_must()); }
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
						return e.e2s(lhs); 
					}

					XTAL_CASE(','){
						MultipleAssignStmt* mas = XTAL_NEW MultipleAssignStmt(ln);
						mas->lhs.push_back(lhs, &alloc_);
						mas->discard = true;
						parse_multiple_expr(&mas->lhs, &mas->discard);
						
						if(eat('=')){
							mas->define = false;
							parse_multiple_expr(&mas->rhs);
						}else if(eat(':')){
							mas->define = true;
							parse_multiple_expr(&mas->rhs);
							for(TList<Expr*>::Node* p = mas->lhs.head; p; p = p->next){
								if(LocalExpr* le = expr_cast<LocalExpr>(p->value)){
									e.register_variable(le->var);
								}else if(SendExpr* le = expr_cast<SendExpr>(lhs)){
								
								}else{
									com_->error(line(), Xt("Xtal Compile Error 1001"));
								}
							}
						}else{
							com_->error(line(), Xt("Xtal Compile Error 1001"));
						}
						
						return mas;
					}

					XTAL_CASE('='){  return e.assign(lhs, parse_expr_must()); }
					XTAL_CASE(':'){ return e.define(lhs, parse_expr_must()); }

					XTAL_CASE(c2('+','+')){ return e.inc(CODE_INC, lhs); }
					XTAL_CASE(c2('-','-')){ return e.inc(CODE_DEC, lhs); }
					
					XTAL_CASE(c2('+','=')){ return e.op_assign(CODE_ADD_ASSIGN, lhs, parse_expr_must()); }
					XTAL_CASE(c2('-','=')){ return e.op_assign(CODE_SUB_ASSIGN, lhs, parse_expr_must()); }
					XTAL_CASE(c2('~','=')){ return e.op_assign(CODE_CAT_ASSIGN, lhs, parse_expr_must()); }
					XTAL_CASE(c2('*','=')){ return e.op_assign(CODE_MUL_ASSIGN, lhs, parse_expr_must()); }
					XTAL_CASE(c2('/','=')){ return e.op_assign(CODE_DIV_ASSIGN, lhs, parse_expr_must()); }
					XTAL_CASE(c2('%','=')){ return e.op_assign(CODE_MOD_ASSIGN, lhs, parse_expr_must()); }
					XTAL_CASE(c2('^','=')){ return e.op_assign(CODE_XOR_ASSIGN, lhs, parse_expr_must()); }
					XTAL_CASE(c2('|','=')){ return e.op_assign(CODE_OR_ASSIGN, lhs, parse_expr_must()); }
					XTAL_CASE(c2('&','=')){ return e.op_assign(CODE_AND_ASSIGN, lhs, parse_expr_must()); }
					XTAL_CASE(c3('<','<','=')){ return e.op_assign(CODE_SHL_ASSIGN, lhs, parse_expr_must()); }
					XTAL_CASE(c3('>','>','=')){ return e.op_assign(CODE_SHR_ASSIGN, lhs, parse_expr_must()); }
					XTAL_CASE(c4('>','>','>','=')){ return e.op_assign(CODE_USHR_ASSIGN, lhs, parse_expr_must()); }

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
				XTAL_CASE(Token::KEYWORD_SWITCH){ return parse_switch(); }
				XTAL_CASE(Token::KEYWORD_IF){ return parse_if(); }
				XTAL_CASE(Token::KEYWORD_THROW){ return parse_throw(); }
				XTAL_CASE(Token::KEYWORD_TRY){ return parse_try(); }
				XTAL_CASE(Token::KEYWORD_ASSERT){ return parse_assert(); }
				
				XTAL_CASE(Token::KEYWORD_YIELD){ 
					e.yield_begin();
					parse_multiple_expr(e.yield_exprs());
					expect_end();
					return e.yield_end();
				}
			}
		}
		
		XTAL_CASE(Token::TYPE_TOKEN){
			switch(ch.ivalue()){
				XTAL_CASE('{'){ Stmt* p = parse_block(); eat_end(); return p; }
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
		//eat_end();
	}
	return ret;
}
	
Stmt* Parser::parse_stmt_must(){
	Stmt* ret = parse_stmt();
	if(!ret){
		com_->error(line(), Xt("Xtal Compile Error 1001"));
	}
	return ret;
}

Stmt* Parser::parse_assert(){
	AssertStmt* p = XTAL_NEW AssertStmt(lexer_.line());

	lexer_.begin_record();
	if(Expr* e = parse_expr()){
		String ref_str(lexer_.end_record());
		p->exprs.push_back(e, &alloc_);
		p->exprs.push_back(XTAL_NEW StringExpr(lexer_.line(), com_->register_value(ref_str)), &alloc_);

		if(eat_a(',')){
			if(Expr* e = parse_expr()){
				p->exprs.push_back(e, &alloc_);
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
	Expr* ret = e.string(com_->register_value(String(str)));
	str = "";
	return ret;
}
	
void Parser::parse_multiple_expr(TList<Expr*>* exprs, bool* discard){
	while(1){
		if(Expr* p = parse_expr()){
			if(discard) *discard = false;
			exprs->push_back(p, &alloc_);
			if(eat_a(',')){
				if(discard) *discard = true;
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
			stmts->push_back(p, &alloc_);
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
		return com_->register_ident(lexer_.keyword2id(lexer_.read().ivalue()));
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

	e.block_begin();
		expect('(');
		e.block_add(parse_assign_stmt());
		expect(';');
	
		e.while_begin(0, parse_expr());
			e.while_label(label);
			expect(';');
			
			e.while_next(parse_assign_stmt());
			expect_a(')');

			e.while_body(parse_stmt_must());

			if(eat(Token::KEYWORD_ELSE)){
				e.while_else(parse_stmt_must());
			}else if(eat(Token::KEYWORD_NOBREAK)){
				e.while_nobreak(parse_stmt_must());
			}

		s = e.while_end();
		e.block_add(s);
	
	return e.block_end();
}

Stmt* Parser::parse_top_level(){
	e.toplevel_begin();
	
	while(1){
		while(eat_end()){}
		if(Stmt* p = parse_stmt()){
			e.toplevel_add(p);
		}else if(eat(Token::KEYWORD_EXPORT)){
			int_t name = parse_var();
			e.toplevel_export(name, parse_expr_must());
			expect_end();
		}else{
			break;
		}
	}
	
	expect(-1);
	return e.toplevel_end();
}

Stmt* Parser::parse_block(){
	e.block_begin();
	parse_multiple_stmt(e.block_stmts());
	expect('}');
	return e.block_end();
}

Expr* Parser::parse_class(){
	e.class_begin();

	if(eat('(')){
		parse_multiple_expr(e.class_mixins());
		expect_a(')');
	}

	//AC<std::pair<int_t, Expr*> >::vector insts;

	expect('{');
	while(true){

		int_t accessibility = -1;
		
		if(eat('#')){// 可触性 protected 指定
			accessibility = KIND_PROTECTED;
		}else if(eat('-')){// 可触性 private 指定
			accessibility = KIND_PRIVATE;
		}else if(eat('+')){// 可触性 public 指定
			accessibility = KIND_PUBLIC;
		}

		if(int_t var = parse_var()){ // メンバ定義
			e.class_add(e.define(e.local(var), parse_expr_must()));
			expect_end();
			
			if(accessibility==KIND_PRIVATE || accessibility==KIND_PROTECTED)
				e.class_add(e.set_accessibility(var, accessibility));

		}else if(eat('_')){// インスタンス変数定義
			if(int_t var = parse_ident()){
				
				if(eat(':')){ // 初期値込み
					e.class_define_instance_variable(var, parse_expr_must());
				}else{
					e.class_define_instance_variable(var, 0);
				}
				expect_end();

				if(accessibility!=-1){ // 可触性が付いているので、アクセッサを定義する
					e.fun_begin(KIND_METHOD);
					e.fun_body(e.return_(e.instance_variable(var)));
					e.class_add(e.define(e.local(var), e.fun_end()));
					if(accessibility==KIND_PRIVATE || accessibility==KIND_PROTECTED)
						e.class_add(e.set_accessibility(var, accessibility));
					
					int_t var2 = com_->register_ident(String("set_").cat(com_->ident_table[var].to_s()).intern());
					e.fun_begin(KIND_METHOD);
					e.fun_param(com_->register_ident(Xid(value)));
					e.fun_body(e.assign(e.instance_variable(var), e.local(com_->register_ident(Xid(value)))));
					e.class_add(e.define(e.local(var2), e.fun_end()));
					if(accessibility==KIND_PRIVATE || accessibility==KIND_PROTECTED)
						e.class_add(e.set_accessibility(var2, accessibility));
				}
			}else{
				com_->error(line(), Xt("Xtal Compile Error 1001"));
			}
		}else{
			break;
		}
	}

	expect('}');
	return e.class_end();
}

Stmt* Parser::parse_define_local_stmt(){
	if(int_t var = parse_var()){
		Stmt* p = e.define(e.local(var), parse_expr());
		expect_end();
		return p;
	}	
	return 0;
}

Expr* Parser::parse_define_local_or_expr(){
	if(int_t var = parse_var()){
		e.register_variable(var);
	}
	return parse_expr();
}

Stmt* Parser::parse_try(){
	e.try_begin();

	e.try_body(parse_stmt_must());
	
		if(eat(Token::KEYWORD_CATCH)){
			expect('(');
			e.register_variable(parse_ident());
			expect(')');
			e.try_catch(parse_stmt_must());
		}
	
		if(eat(Token::KEYWORD_FINALLY)){
			e.try_finally(parse_stmt_must());
		}

	return e.try_end();
}

Expr* Parser::parse_fun(int_t kind, bool lambda){
	e.fun_begin(kind);

	int_t inst_assign_list_count = 0;
	int_t inst_assign_list[255];

	if(lambda || eat('(')){

		while(true){
			
			if(eat_a(lambda ? '|' : ')')){
				break;
			}
				
			if(eat(c3('.','.','.'))){
				e.fun_have_args();
				expect(')');
				break;
			}
			
			if(eat('_')){
				if(int_t var = parse_ident()){
					if(!lambda && eat(':')){
						e.fun_param(var, parse_expr_must());
					}else{
						e.fun_param(var);
					}
					if(inst_assign_list_count<255)
						inst_assign_list[inst_assign_list_count++] = var;
				}else{
					com_->error(line(), Xt("Xtal Compile Error 1001"));
				}
			}else if(int_t var = parse_ident()){
				if(!lambda && eat(':')){
					e.fun_param(var, parse_expr_must());
				}else{
					e.fun_param(var);
				}
			}
			
			if(eat_a(',')){
				if(eat_a(lambda ? '|' : ')')){
					break;
				}
			}else{
				expect_a(lambda ? '|' : ')');
				break;
			}
		}
	}

	e.fun_body_begin();

	if(inst_assign_list_count==0){
		if(eat('{')){
			e.fun_body_add(parse_block());
		}else{
			e.fun_body_add(e.return_(parse_expr_must()));
		}
	}else{
		for(int_t i=0; i<inst_assign_list_count; ++i){
			int_t var = inst_assign_list[i];
			e.fun_body_add(e.assign(e.instance_variable(var), e.local(var)));
		}
		if(eat('{')){
			e.fun_body_add(parse_block());
		}else{
			e.fun_body_add(e.return_(parse_expr_must()));
		}
	}

	e.fun_body_end();
	return e.fun_end();
}

Expr* Parser::parse_call(Expr* lhs){
	e.call_begin(lhs);
	while(true){

		if(int_t var = parse_var()){
			e.call_arg(var, parse_expr_must());
		}else{
			e.call_arg(parse_expr());
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

	return e.call_end();
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
		com_->error(line(), Xt("Xtal Compile Error 1001"));
	}
	return ret;
}

Expr* Parser::parse_expr_must(){
	Expr* ret = parse_expr();
	if(!ret){
		com_->error(line(), Xt("Xtal Compile Error 1001"));
	}
	return ret;
}

Stmt* Parser::parse_return(){
	e.return_begin();
	parse_multiple_expr(e.return_exprs());
	expect_end();
	return e.return_end();
}

Stmt* Parser::parse_continue(){
	ContinueStmt* ret = e.continue_(parse_ident());
	expect_end();
	return ret;
}

Stmt* Parser::parse_break(){
	BreakStmt* ret = e.break_(parse_ident());
	expect_end();
	return ret;
}

Stmt* Parser::parse_if(){
	expect('(');
	int_t var = parse_var();
	e.if_begin(var, parse_expr_must());
		expect_a(')');
		e.if_body(parse_stmt_must());
		if(eat(Token::KEYWORD_ELSE)){
			e.if_else(parse_stmt_must());
		}
	return e.if_end();
}

Stmt* Parser::parse_while(int_t label){
	expect('(');
	int_t var = parse_var();
	e.while_begin(var, parse_expr_must());
		e.while_label(label);
		expect_a(')');
		e.while_body(parse_stmt_must());
		if(eat(Token::KEYWORD_ELSE)){
			e.while_else(parse_stmt_must());
		}else if(eat(Token::KEYWORD_NOBREAK)){
			e.while_nobreak(parse_stmt_must());
		}
	return e.while_end();
}

Stmt* Parser::parse_switch(){
	expect('(');
	e.block_begin();
		int_t var = parse_var();
		if(!var){
			var = com_->register_ident(ID("__SWITCH__"));
		}
		
		e.block_add(e.define(e.local(var), parse_expr_must()));
		expect_a(')');
		expect('{');
		
		IfStmt* if_stmt = 0;
		IfStmt* first = 0;
		Stmt* default_stmt = 0;
		while(true){
			if(eat(Token::KEYWORD_CASE)){
			
				IfStmt* temp = XTAL_NEW IfStmt(lexer_.line());
				
				if(if_stmt){
					if_stmt->else_stmt = temp;
					if_stmt = temp;
				}else{
					if_stmt = temp;
					first = temp;
				}
				
				expect('(');
				
				while(true){
					if(temp->cond_expr){
						temp->cond_expr = e.oror(temp->cond_expr,
							e.bin_comp(CODE_EQ, e.local(var), parse_expr_must()));
					}else{
						temp->cond_expr = e.bin_comp(CODE_EQ, e.local(var), parse_expr_must());
					}
					
					if(eat_a(',')){
						if(eat(')'))
							break;
					}else{
						if(eat(')'))
							break;
					}
				}
							
				temp->body_stmt = parse_stmt();

			}else if(eat(Token::KEYWORD_DEFAULT)){
				if(default_stmt){
					com_->error(line(), Xt("Xtal Compile Error 1018")());					
				}
				default_stmt = parse_stmt();
			}else{
				expect('}');
				break;
			}
		}
		
		if(if_stmt){
			if_stmt->else_stmt = default_stmt;
			e.block_add(first);
		}else{
			if(default_stmt){
				e.block_add(default_stmt);
			}
		}
	return e.block_end();
}


Stmt* Parser::parse_throw(){
	UnaStmt* p = XTAL_NEW UnaStmt(lexer_.line(), CODE_THROW);
	p->expr = parse_expr_must();
	expect_end();
	return p;
}

Expr* Parser::parse_array(){
	
	int_t ln = lexer_.line();

	if(eat(']')){//empty array
		e.array_begin();
		return e.array_end();
	}
	
	if(eat(':')){//empty map
		expect(']');
		e.map_begin();
		return e.map_end();
	}
		
	Expr* key = parse_expr_must();
	if(eat_a(':')){//map
		e.map_begin();
		e.map_add(key, parse_expr_must());	
		
		if(eat_a(',')){
			while(true){
				if((key = parse_expr())){
					expect_a(':');
					e.map_add(key, parse_expr_must());
					
					if(!eat_a(',')){
						break;
					}
				}else{
					break;
				}
			}
		}

		expect_a(']');
		return e.map_end();
	}else{//array
		e.array_begin();
		e.array_add(key);
		if(eat_a(',')){
			while(true){
				if((key = parse_expr())){
					e.array_add(key);
					if(!eat_a(',')){
						break;
					}
				}else{
					break;
				}
			}
		}
		expect_a(']');
		return e.array_end();
	}
}

}

#endif
