
#include "xtal.h"

#ifndef XTAL_NO_PARSER

#include <math.h>

#include "xtal_parser.h"
#include "xtal_any.h"
#include "xtal_macro.h"

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

	PRI_AT,
		PRI_SEND = PRI_AT,
		PRI_MEMBER = PRI_AT,
		PRI_CALL = PRI_AT,
		PRI_NS = PRI_AT,
		PRI_TO_A = PRI_AT,
		PRI_TO_M = PRI_AT,

	PRI_ONCE,

	PRI_END_,

	PRI_MAX = PRI_END_-PRI_BEGIN_
};


#define c2(C1, C2) ((C2)<<8 | (C1))
#define c3(C1, C2, C3) ((C3)<<16 | (C2)<<8 | (C1))
#define c4(C1, C2, C3, C4) ((C4)<<24 | (C3)<<16 | (C2)<<8 | (C1))

Parser::Parser(){
	expr_end_flag_ = false;
}

ExprPtr Parser::parse(const StreamPtr& stream, const StringPtr& source_file_name){
	lexer_.init(stream, source_file_name);
	com_ = lexer_.common();

	ExprPtr p = parse_top_level();

	if(com_->errors->size()!=0){
		return null;
	}
	return p;
}

void Parser::begin_interactive_parsing(const StreamPtr& stream){
	/*
	lexer_.init(stream, "<ix>");
	com_ = lexer_.common();

	e.toplevel_begin();
	*/
}

ExprPtr Parser::interactive_parse(){
	/*
	if(eat(';'))
		return e.null_();
	
	if(ExprPtr p = parse_stmt())
		return p;

	Token tok = lexer_.read();
	if(tok.type()==Token::TYPE_TOKEN && tok.ivalue()==-1)
		return null;

	return e.null_();
	*/
	return null;
}

LPCCommon* Parser::common(){
	return com_;
}

void Parser::release(){

}
	
void Parser::expect(int_t ch){
	if(eat(ch)){
		return;
	}		
	printf("\n\n '%c' '%s'\n", ch, lexer_.token2str(lexer_.peek())->to_s()->c_str());
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
	
StringPtr Parser::parse_string(int_t open, int_t close){
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
					str+=(char_t)lexer_.peek().ivalue(); 
				}
				
				XTAL_CASE('n'){ str+='\n'; }
				XTAL_CASE('r'){ str+='\r'; }
				XTAL_CASE('t'){ str+='\t'; }
				XTAL_CASE('f'){ str+='\f'; }
				XTAL_CASE('b'){ str+='\b'; }
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
				for(int_t i=1, size = ch_len((char_t)ch); i<size; ++i){
					str+=(char_t)lexer_.read().ivalue();
				}
			}
		}	
	}
	lexer_.set_normal_mode();
	return xnew<String>(str);
}

ExprPtr Parser::parse_term(){
	
	Token ch = lexer_.read();

	ExprPtr ret = null;
	int_t r_space = ch.right_space() ? PRI_MAX : 0;
	int_t ln = line();

	switch(ch.type()){
		XTAL_NODEFAULT;

		XTAL_CASE(Token::TYPE_TOKEN){
			switch(ch.ivalue()){

				XTAL_DEFAULT{}

				XTAL_CASE('('){ ret = parse_expr(); expect(')'); }
				XTAL_CASE('['){ ret = parse_array(); }
				XTAL_CASE('|'){ ret = parse_fun(KIND_LAMBDA); }

				XTAL_CASE(c2('|', '|')){
					ret = fun(ln, KIND_LAMBDA, make_map(Xid(it), null), false, null); 
					if(eat('{')){
						ret->set_fun_body(parse_scope());
					}else{
						ret->set_fun_body(return_(ln, make_array(parse_expr_must())));
					}
				}

				XTAL_CASE('_'){ ret = ivar(line(), parse_ident()); }

				XTAL_CASE('"'){ lexer_.set_string_mode(); ret = string(ln, KIND_STRING, parse_string('"', '"')); }
				
				XTAL_CASE('%'){
					lexer_.set_string_mode();
					int_t open = lexer_.read().ivalue();
					int_t close = 0;
					int_t kind = KIND_STRING;
					for(;;){
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
				
					ret = string(ln, kind, parse_string(open, close));
				}
				
				XTAL_CASE(c3('.','.','.')){ ret = args(ln); }

////////////////////////////////////////////////////////////////////////////////////////

				XTAL_CASE('+'){ if(ExprPtr rhs = parse_expr(PRI_POS - r_space)){ ret = una(EXPR_POS, ln, rhs); } }
				XTAL_CASE('-'){ if(ExprPtr rhs = parse_expr(PRI_NEG - r_space)){ ret = una(EXPR_NEG, ln, rhs); } }
				XTAL_CASE('~'){ if(ExprPtr rhs = parse_expr(PRI_COM - r_space)){ ret = una(EXPR_COM, ln, rhs); } }
				XTAL_CASE('!'){ if(ExprPtr rhs = parse_expr(PRI_NOT - r_space)){ ret = una(EXPR_NOT, ln, rhs); } }
			}
		}

		XTAL_CASE(Token::TYPE_KEYWORD){
			switch(ch.ivalue()){

				XTAL_DEFAULT{}
				
				XTAL_CASE(Token::KEYWORD_ONCE){ ret = once(ln, parse_expr_must(PRI_ONCE - r_space*2)); }
				XTAL_CASE(Token::KEYWORD_CLASS){ ret = parse_class(KIND_CLASS); }
				XTAL_CASE(Token::KEYWORD_SINGLETON){ ret = parse_class(KIND_SINGLETON); }
				XTAL_CASE(Token::KEYWORD_FUN){ ret = parse_fun(KIND_FUN); }
				XTAL_CASE(Token::KEYWORD_METHOD){ ret = parse_fun(KIND_METHOD); }
				XTAL_CASE(Token::KEYWORD_FIBER){ ret = parse_fun(KIND_FIBER); }
				XTAL_CASE(Token::KEYWORD_DOFUN){ ret = call(ln, parse_fun(KIND_FUN), null, null, false); }
				XTAL_CASE(Token::KEYWORD_CALLEE){ ret = callee(ln); }
				XTAL_CASE(Token::KEYWORD_NULL){ ret = null_(ln); }
				XTAL_CASE(Token::KEYWORD_TRUE){ ret = true_(ln); }
				XTAL_CASE(Token::KEYWORD_FALSE){ ret = false_(ln); }
				XTAL_CASE(Token::KEYWORD_THIS){ ret = this_(ln); }
				XTAL_CASE(Token::KEYWORD_CURRENT_CONTEXT){ ret = current_context(ln); }
			}
		}
		
		XTAL_CASE(Token::TYPE_INT){ ret = int_(ln, ch.ivalue()); }
		XTAL_CASE(Token::TYPE_FLOAT){ ret = float_(ln, ch.fvalue()); }
		XTAL_CASE(Token::TYPE_IDENT){ ret = lvar(ln, (InternedStringPtr&)com_->ident_table->at(ch.ivalue())); }
	}

	if(!ret){
		lexer_.putback(ch);
	}

	return ret;
}

ExprPtr Parser::parse_post(ExprPtr lhs, int_t pri){
	if(expr_end_flag_)
		return null;

	Token ch = lexer_.read();

	ExprPtr ret;

	int_t ln = line();
	int_t r_space = (ch.right_space()) ? PRI_MAX : 0;
	int_t l_space = (ch.left_space()) ? PRI_MAX : 0;

	switch(ch.type()){
	
		XTAL_DEFAULT{}
		
		XTAL_CASE(Token::TYPE_KEYWORD){
			switch(ch.ivalue()){
				XTAL_DEFAULT{}
				
				XTAL_CASE(Token::KEYWORD_IS){ if(pri < PRI_IS - l_space){ ret = bin(EXPR_IS, ln, lhs, parse_expr(PRI_IS - r_space)); } }
			}
		}
		
		XTAL_CASE(Token::TYPE_TOKEN){
			switch(ch.ivalue()){

				XTAL_DEFAULT{ ret = null; }
			
				XTAL_CASE('+'){ if(pri < PRI_ADD - l_space){ ret = bin(EXPR_ADD, ln, lhs, parse_expr(PRI_ADD - r_space)); } }
				XTAL_CASE('-'){ if(pri < PRI_SUB - l_space){ ret = bin(EXPR_SUB, ln, lhs, parse_expr(PRI_SUB - r_space)); } }
				XTAL_CASE('~'){ if(pri < PRI_CAT - l_space){ ret = bin(EXPR_CAT, ln, lhs, parse_expr(PRI_CAT - r_space)); } } 
				XTAL_CASE('*'){ if(pri < PRI_MUL - l_space){ ret = bin(EXPR_MUL, ln, lhs, parse_expr(PRI_MUL - r_space)); } } 
				XTAL_CASE('/'){ if(pri < PRI_DIV - l_space){ ret = bin(EXPR_DIV, ln, lhs, parse_expr(PRI_DIV - r_space)); } } 
				XTAL_CASE('%'){ if(pri < PRI_MOD - l_space){ ret = bin(EXPR_MOD, ln, lhs, parse_expr(PRI_MOD - r_space)); } } 
				XTAL_CASE('^'){ if(pri < PRI_XOR - l_space){ ret = bin(EXPR_XOR, ln, lhs, parse_expr(PRI_XOR - r_space)); } } 
				XTAL_CASE(c2('&','&')){ if(pri < PRI_ANDAND - l_space){ ret = bin(EXPR_ANDAND, ln, lhs, parse_expr_must(PRI_ANDAND - r_space));} } 
				XTAL_CASE('&'){ if(pri < PRI_AND - l_space){ ret = bin(EXPR_AND, ln, lhs, parse_expr(PRI_AND - r_space)); } } 
				XTAL_CASE(c2('|','|')){ if(pri < PRI_OROR - l_space){ ret = bin(EXPR_OROR, ln, lhs, parse_expr(PRI_OROR - r_space));} } 
				XTAL_CASE('|'){ if(pri < PRI_OR - l_space){ ret = bin(EXPR_OR, ln, lhs, parse_expr(PRI_OR - r_space)); } } 
				XTAL_CASE(c2('<','<')){ if(pri < PRI_SHL - l_space){ ret = bin(EXPR_SHL, ln, lhs, parse_expr(PRI_SHL - r_space)); } } 
				XTAL_CASE(c2('>','>')){ if(pri < PRI_SHR - l_space){ ret = bin(EXPR_SHR, ln, lhs, parse_expr(PRI_SHR - r_space)); } } 
				XTAL_CASE(c3('>','>','>')){ if(pri < PRI_USHR - l_space){ ret = bin(EXPR_USHR, ln, lhs, parse_expr(PRI_USHR - r_space)); } } 
				XTAL_CASE(c2('<','=')){ if(pri < PRI_LE - l_space){ ret = bin(EXPR_LE, ln, lhs, parse_expr(PRI_LE - r_space)); } } 
				XTAL_CASE('<'){ if(pri < PRI_LT - l_space){ ret = bin(EXPR_LT, ln, lhs, parse_expr(PRI_LT - r_space)); } } 
				XTAL_CASE(c2('>','=')){ if(pri < PRI_GE - l_space){ ret = bin(EXPR_GE, ln, lhs, parse_expr(PRI_GE - r_space)); } } 
				XTAL_CASE('>'){ if(pri < PRI_GT - l_space){ ret = bin(EXPR_GT, ln, lhs, parse_expr(PRI_GT - r_space)); } } 
				XTAL_CASE(c2('=','=')){ if(pri < PRI_EQ - l_space){ ret = bin(EXPR_EQ, ln, lhs, parse_expr(PRI_EQ - r_space)); } } 
				XTAL_CASE(c2('!','=')){ if(pri < PRI_NE - l_space){ ret = bin(EXPR_NE, ln, lhs, parse_expr(PRI_NE - r_space)); } } 
				XTAL_CASE(c3('=','=','=')){ if(pri < PRI_RAW_EQ - l_space){ ret = bin(EXPR_RAWEQ, ln, lhs, parse_expr(PRI_RAW_EQ - r_space)); } } 
				XTAL_CASE(c3('!','=','=')){ if(pri < PRI_RAW_NE - l_space){ ret = bin(EXPR_RAWNE, ln, lhs, parse_expr(PRI_RAW_NE - r_space)); } } 
				XTAL_CASE(c3('!','i','s')){ if(pri < PRI_NIS - l_space){ ret = bin(EXPR_NIS, ln, lhs, parse_expr(PRI_NIS - r_space)); } }

				XTAL_CASE(c2(':',':')){
					if(pri < PRI_MEMBER - l_space){
						if(eat('(')){
							ret = member_e(ln, lhs, parse_expr_must());
							expect(')');
						}else{
							ret = member(ln, lhs, parse_ident_or_keyword());
						}
					}
				}

				XTAL_CASE(c3(':',':','?')){
					if(pri < PRI_MEMBER - l_space){
						if(eat('(')){
							ret = member_eq(ln, lhs, parse_expr_must());
							expect(')');
						}else{
							ret = member_q(ln, lhs, parse_ident_or_keyword());
						}
					}
				}

				XTAL_CASE('.'){
					if(pri < PRI_SEND - l_space){
						if(eat('(')){
							ret = send_e(ln, lhs, parse_expr_must());
							expect(')');
						}else{
							ret = send(ln, lhs, parse_ident_or_keyword());
						}
					}
				}

				XTAL_CASE(c2('.', '?')){ 
					if(pri < PRI_SEND - l_space){
						if(eat('(')){
							ret = send_eq(ln, lhs, parse_expr_must());
							expect(')');
						}else{
							ret = send_q(ln, lhs, parse_ident_or_keyword());
						}
					}
				}

				XTAL_CASE('?'){
					if(pri < PRI_Q - l_space){
						ret = q(ln, lhs, null, null);
						ret->set_q_true(parse_expr_must());
						expect(':');
						ret->set_q_false(parse_expr_must());
					}
				}

				XTAL_CASE('('){
					if(pri < PRI_CALL - l_space){
						ret = parse_call(lhs);
					}
				}

				XTAL_CASE('['){
					if(pri < PRI_AT - l_space){
						ret = bin(EXPR_AT, ln, lhs, parse_expr_must());
						expect(']');
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


InternedStringPtr Parser::to_id(int_t ident){
	return (InternedStringPtr&)com_->ident_table->at(ident);
}

ExprPtr Parser::parse_each(const InternedStringPtr& label, ExprPtr lhs){
	int_t ln = lexer_.line();	

	ArrayPtr params = xnew<Array>();
	params->push_back(lvar(0, Xid(iterator)));
	bool discard = false;
	if(eat('|')){ // ブロックパラメータ
		while(true){
			Token ch = lexer_.peek();
			if(ch.type()==ch.TYPE_IDENT){
				discard = false;
				lexer_.read();
				params->push_back(lvar(0, to_id(ch.ivalue())));
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
		params->push_back(lvar(0, Xid(it)));
	}

	if(discard){
		params->push_back(lvar(0, Xid(dummy_block_parameter)));
	}

	ArrayPtr scope_stmts = xnew<Array>();
	scope_stmts->push_back(massign(0, params, make_array(send(0, lhs, Xid(block_first))), true));
			
	ExprPtr efor = for_(0, label, lvar(0, Xid(iterator)), massign(0, params, make_array(send(0, lvar(0, Xid(iterator)), Xid(block_next))), false), parse_scope(), null, null);
	if(eat(Token::KEYWORD_ELSE)){
		efor->set_for_else(parse_stmt_must());
	}else if(eat(Token::KEYWORD_NOBREAK)){
		efor->set_for_nobreak(parse_stmt_must());
	}

	scope_stmts->push_back(try_(0, efor, null, null, send_q(0, lvar(0, Xid(iterator)), Xid(block_break))));

	return scope(0, scope_stmts);
}

ExprPtr Parser::parse_loop(){
	if(InternedStringPtr ident = parse_var()){
		Token ch = lexer_.read(); // :の次を読み取る
		if(ch.type()==Token::TYPE_KEYWORD){
			switch(ch.ivalue()){
				XTAL_DEFAULT{}
				XTAL_CASE(Token::KEYWORD_FOR){ return parse_for(ident); }
				XTAL_CASE(Token::KEYWORD_WHILE){ return parse_while(ident); }
			}
		}

		lexer_.putback(ch);
		if(ExprPtr lhs = parse_expr()){
			if(!expr_end_flag_ && eat('{')){
				return parse_each(ident, lhs);
			}else{
				return define(0, lvar(0, ident), lhs);
			}
		}

		lexer_.putback();
		lexer_.putback();
	}
	return null;
}

ExprPtr Parser::parse_assign_stmt(){

	int_t ln = line();
	Token ch = lexer_.read();

	if(ch.type()==Token::TYPE_TOKEN){
		switch(ch.ivalue()){
			XTAL_DEFAULT{}
			XTAL_CASE(c2('+','+')){ return una(EXPR_INC, ln, parse_expr_must()); }
			XTAL_CASE(c2('-','-')){ return una(EXPR_DEC, ln, parse_expr_must()); }
		}
	}
	lexer_.putback();

	if(ExprPtr lhs = parse_expr()){
		if(expr_end_flag_)
			return lhs;
		
		ch = lexer_.read();

		switch(ch.type()){
			XTAL_DEFAULT{}
			
			XTAL_CASE(Token::TYPE_TOKEN){
				switch(ch.ivalue()){

					XTAL_DEFAULT{
						lexer_.putback();
						return lhs; 
					}

					XTAL_CASE(','){
						bool discard = true;
						ArrayPtr left_exprs = parse_exprs(&discard);
						left_exprs->push_front(lhs);
						if(discard){
							left_exprs->push_back(lvar(0, Xid(dummy_lhs_parameter)));
						}
						
						if(eat('=')){
							return massign(ln, left_exprs, parse_exprs(), false);
						}else if(eat(':')){
							return massign(ln, left_exprs, parse_exprs(), true);
						}else{
							com_->error(line(), Xt("Xtal Compile Error 1001"));
						}
						
						return null;
					}

					XTAL_CASE('='){  return assign(ln, lhs, parse_expr_must()); }
					XTAL_CASE(':'){ 
						return define(ln, lhs, parse_expr_must()); 
					}

					XTAL_CASE(c2('+','+')){ return una(EXPR_INC, ln, lhs); }
					XTAL_CASE(c2('-','-')){ return una(EXPR_DEC, ln, lhs); }
					
					XTAL_CASE(c2('+','=')){ return bin(EXPR_ADD_ASSIGN, ln, lhs, parse_expr_must()); }
					XTAL_CASE(c2('-','=')){ return bin(EXPR_SUB_ASSIGN, ln, lhs, parse_expr_must()); }
					XTAL_CASE(c2('~','=')){ return bin(EXPR_CAT_ASSIGN, ln, lhs, parse_expr_must()); }
					XTAL_CASE(c2('*','=')){ return bin(EXPR_MUL_ASSIGN, ln, lhs, parse_expr_must()); }
					XTAL_CASE(c2('/','=')){ return bin(EXPR_DIV_ASSIGN, ln, lhs, parse_expr_must()); }
					XTAL_CASE(c2('%','=')){ return bin(EXPR_MOD_ASSIGN, ln, lhs, parse_expr_must()); }
					XTAL_CASE(c2('^','=')){ return bin(EXPR_XOR_ASSIGN, ln, lhs, parse_expr_must()); }
					XTAL_CASE(c2('|','=')){ return bin(EXPR_OR_ASSIGN, ln, lhs, parse_expr_must()); }
					XTAL_CASE(c2('&','=')){ return bin(EXPR_AND_ASSIGN, ln, lhs, parse_expr_must()); }
					XTAL_CASE(c3('<','<','=')){ return bin(EXPR_SHL_ASSIGN, ln, lhs, parse_expr_must()); }
					XTAL_CASE(c3('>','>','=')){ return bin(EXPR_SHR_ASSIGN, ln, lhs, parse_expr_must()); }
					XTAL_CASE(c3('>','3','=')){ return bin(EXPR_USHR_ASSIGN, ln, lhs, parse_expr_must()); }

					XTAL_CASE('{'){
						return parse_each(0, lhs);
					}
				}
			}
		}
	}

	return null;
}

ExprPtr Parser::parse_stmt(){

	ExprPtr ret;

	if(ExprPtr p = parse_loop()){
		ret = p;
	}else{

		Token ch = lexer_.read();

		switch(ch.type()){
			XTAL_DEFAULT{}

			XTAL_CASE(Token::TYPE_KEYWORD){
				switch(ch.ivalue()){
					XTAL_CASE(Token::KEYWORD_THROW){	
						ExprPtr temp = xnew<Expr>();
						temp->set_type(EXPR_THROW);
						temp->set_throw_expr(parse_expr_must());
						ret = temp;
					}				
					XTAL_CASE(Token::KEYWORD_RETURN){ ret = return_(line(), parse_exprs()); }
					XTAL_CASE(Token::KEYWORD_CONTINUE){ ret = continue_(line(), parse_ident()); }
					XTAL_CASE(Token::KEYWORD_BREAK){ ret = break_(line(), parse_ident()); }
					XTAL_CASE(Token::KEYWORD_FOR){ ret = parse_for(); }
					XTAL_CASE(Token::KEYWORD_WHILE){ ret = parse_while(); }
					XTAL_CASE(Token::KEYWORD_SWITCH){ ret = parse_switch(); }
					XTAL_CASE(Token::KEYWORD_IF){ ret = parse_if(); }

					XTAL_CASE(Token::KEYWORD_TRY){ ret = parse_try(); }
					XTAL_CASE(Token::KEYWORD_ASSERT){ ret = parse_assert(); }
					XTAL_CASE(Token::KEYWORD_YIELD){ ret = yield(line(), parse_exprs()); }
				}
			}
			
			XTAL_CASE(Token::TYPE_TOKEN){
				switch(ch.ivalue()){
					XTAL_CASE('{'){ ret = parse_scope(); }
					XTAL_CASE(';'){ return null_(line()); }
				}
			}
		}
		
		if(!ret){
			lexer_.putback();

			if(ExprPtr stmt = parse_assign_stmt()){
				return stmt;
			}
		}
	}

	eat(';');

	return ret;
}
	
ExprPtr Parser::parse_stmt_must(){
	ExprPtr ret = parse_stmt();
	if(!ret){
		com_->error(line(), Xt("Xtal Compile Error 1001"));
	}
	return ret;
}

ExprPtr Parser::parse_assert(){
	int_t ln = line();
	ArrayPtr exprs = xnew<Array>();

	lexer_.begin_record();
	if(ExprPtr ep = parse_expr()){
		StringPtr ref_str(xnew<String>(lexer_.end_record()));
		exprs->push_back(ep);
		exprs->push_back(string(ln, KIND_STRING, ref_str));
		if(eat(',')){
			if(ExprPtr ep = parse_expr()){
				exprs->push_back(ep);
			}
		}
	}else{
		lexer_.end_record();
	}

	return assert_(ln, exprs);
}
	
ArrayPtr Parser::parse_exprs(bool* discard){
	ArrayPtr ret = xnew<Array>();
	while(1){
		if(ExprPtr p = parse_expr()){
			if(discard) *discard = false;
			ret->push_back(p);
			if(eat(',')){
				if(discard) *discard = true;
			}else{
				break;
			}
		}else{
			break;
		}
	}
	return ret;
}

ArrayPtr Parser::parse_stmts(){
	ArrayPtr ret = xnew<Array>();
	while(1){
		while(eat(';')){}
		if(ExprPtr p = parse_stmt()){
			ret->push_back(p);
		}else{
			break;
		}
	}
	return ret;
}

InternedStringPtr Parser::parse_ident(){
	if(lexer_.peek().type()==Token::TYPE_IDENT){
		return to_id(lexer_.read().ivalue());
	}
	return null;
}

InternedStringPtr Parser::parse_ident_or_keyword(){
	if(lexer_.peek().type()==Token::TYPE_IDENT){
		return to_id(lexer_.read().ivalue());
	}else if(lexer_.peek().type()==Token::TYPE_KEYWORD){
		return lexer_.keyword2id(lexer_.read().ivalue());
	}
	return null;
}

InternedStringPtr Parser::parse_var(){
	if(InternedStringPtr ident = parse_ident()){
		if(eat(':')){ 
			return ident; 
		}else{
			lexer_.putback();
		}
	}
	return null;
}
	
ExprPtr Parser::parse_for(const InternedStringPtr& label){
	int_t ln = line();

	ArrayPtr scope_stmts = xnew<Array>();
	expect('(');
	scope_stmts->push_back(parse_assign_stmt());
	expect(';');

	ExprPtr efor = for_(ln, label, null, null, null, null, null);
	efor->set_for_cond(parse_expr());
	expect(';');

	efor->set_for_next(parse_assign_stmt());
	expect(')');

	efor->set_for_body(parse_stmt_must());

	if(eat(Token::KEYWORD_ELSE)){
		efor->set_for_else(parse_stmt_must());
	}else if(eat(Token::KEYWORD_NOBREAK)){
		efor->set_for_nobreak(parse_stmt_must());
	}

	scope_stmts->push_back(efor);

	return scope(ln, scope_stmts);
}

ExprPtr Parser::parse_top_level(){
	ExprPtr etoplevel = toplevel(line(), xnew<Array>(), null);

	while(1){
		while(eat(';')){}
		if(ExprPtr p = parse_stmt()){
			etoplevel->toplevel_stmts()->push_back(p);
		}else if(eat(Token::KEYWORD_EXPORT)){
			if(InternedStringPtr name = parse_var()){
				int_t ln = line();
				etoplevel->toplevel_stmts()->push_back(define(ln, lvar(ln, Xid(export)), parse_expr_must()));
				etoplevel->toplevel_stmts()->push_back(lvar(ln, Xid(export)));
			}else{
				int_t ln = line();
				etoplevel->toplevel_stmts()->push_back(define(ln, lvar(ln, Xid(export)), parse_expr_must()));
			}
			eat(';');
			etoplevel->set_toplevel_export(lvar(line(), Xid(export)));
		}else{
			break;
		}
	}
	
	expect(-1);
	return etoplevel;
}

ExprPtr Parser::parse_scope(){
	int_t ln = line();
	ExprPtr ret = scope(ln, parse_stmts());
	expect('}');
	expr_end_flag_ = true;
	return ret;
}

ExprPtr Parser::parse_class(int_t kind){
	ExprPtr eclass = class_(line(), kind, null, xnew<Array>(), xnew<Map>());

	if(eat('(')){
		eclass->set_class_mixins(parse_exprs());
		expect(')');
	}

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

		if(InternedStringPtr var = parse_ident()){ // メンバ定義
			if(eat('@')){
				ExprPtr ns = parse_expr();
				expect(':');
				int_t ln = line();
				eclass->class_stmts()->push_back(cdefine(ln, accessibility<0 ? KIND_PUBLIC : accessibility, var, ns, parse_expr_must()));
			}else{
				expect(':');
				int_t ln = line();
				eclass->class_stmts()->push_back(cdefine(ln, accessibility<0 ? KIND_PUBLIC : accessibility, var, null_(line()), parse_expr_must()));
			}
			eat(';');
			
		}else if(eat('_')){// インスタンス変数定義
			if(InternedStringPtr var = parse_ident()){
				
				if(eat(':')){ // 初期値込み
					eclass->class_ivars()->set_at(var, parse_expr_must());
				}else{
					eclass->class_ivars()->set_at(var, null);
				}
				eat(';');

				if(accessibility!=-1){ // 可触性が付いているので、アクセッサを定義する
					eclass->class_stmts()->push_back(
						cdefine(line(), accessibility, var, null_(line()), 
							fun(line(), KIND_METHOD, null, false, 
								return_(line(), make_array(ivar(line(), var))))));
			
					InternedStringPtr var2 = xnew<String>("set_")->cat(var);
					eclass->class_stmts()->push_back(
						cdefine(line(), accessibility, var2, null_(line()), 
							fun(line(), KIND_METHOD, make_map(Xid(value), null), false, 
								assign(line(), ivar(line(), var), lvar(line(), Xid(value))))));
				}
			}else{
				com_->error(line(), Xt("Xtal Compile Error 1001"));
			}
		}else{
			break;
		}
	}

	expect('}');
	expr_end_flag_ = true;
	return eclass;
}

ExprPtr Parser::parse_try(){
	ExprPtr etry = try_(line(), null, null, null, null);

	etry->set_try_body(parse_stmt_must());
	
	if(eat(Token::KEYWORD_CATCH)){
		expect('(');
		etry->set_try_catch_var(parse_ident());
		expect(')');
		etry->set_try_catch(parse_stmt_must());
	}

	if(eat(Token::KEYWORD_FINALLY)){
		etry->set_try_finally(parse_stmt_must());
	}

	return etry;
}

ExprPtr Parser::parse_fun(int_t kind){
	bool lambda = kind==KIND_LAMBDA;
	
	MapPtr params = xnew<Map>();
	ExprPtr efun = fun(line(), kind, params, false, null);

	int_t inst_assign_list_count = 0;
	InternedStringPtr inst_assign_list[255];

	if(lambda || eat('(')){

		while(true){
			
			if(eat(lambda ? '|' : ')')){
				break;
			}
				
			if(!lambda && eat(c3('.','.','.'))){
				efun->set_fun_have_args(true);
				expect(')');
				break;
			}
			
			if(eat('_')){
				if(InternedStringPtr var = parse_ident()){
					if(!lambda && eat(':')){
						params->set_at(var, parse_expr_must());
					}else{
						params->set_at(var, null);
					}
					if(inst_assign_list_count<255)
						inst_assign_list[inst_assign_list_count++] = var;
				}else{
					com_->error(line(), Xt("Xtal Compile Error 1001"));
				}
			}else if(InternedStringPtr var = parse_ident()){
				if(!lambda && eat(':')){
					params->set_at(var, parse_expr_must());
				}else{
					params->set_at(var, null);
				}
			}
			
			if(eat(',')){
				if(lambda && eat('|')){
					params->set_at(Xid(dummy_fun_parameter), null);
					break;
				}
			}
		}
	}

	if(inst_assign_list_count==0){
		if(eat('{')){
			efun->set_fun_body(parse_scope());
		}else{
			int_t ln = line();
			efun->set_fun_body(return_(ln, make_array(parse_expr_must())));
		}
	}else{
		
		ArrayPtr scope_stmts = xnew<Array>();
		for(int_t i=0; i<inst_assign_list_count; ++i){
			InternedStringPtr var = inst_assign_list[i];
			scope_stmts->push_back(assign(line(), ivar(line(), var), lvar(line(), var)));
		}

		if(eat('{')){
			int_t ln = line();
			scope_stmts->push_back(parse_scope());
			efun->set_fun_body(scope(ln, scope_stmts));
		}else{
			int_t ln = line();
			scope_stmts->push_back(return_(ln, make_array(parse_expr_must())));
			efun->set_fun_body(scope(ln, scope_stmts));
		}
	}

	return efun;
}

ExprPtr Parser::parse_call(ExprPtr lhs){
	ArrayPtr ordered = xnew<Array>();
	MapPtr named = xnew<Map>();
	ExprPtr ecall = call(line(), lhs, ordered, named, false);
	while(true){

		if(InternedStringPtr var = parse_var()){
			named->set_at(var, parse_expr_must());
		}else{
			ordered->push_back(parse_expr());
		}
		
		if(eat(',')){
			if(eat(')')){
				break;
			}
		}else{
			expect(')');
			break;
		}
	}

	if(!ordered->empty() && !ordered->back()){
		ordered->pop_back();
	}

	if(!ordered->empty() && ep(ordered->back())->type()==EXPR_ARGS){
		ordered->pop_back();
		ecall->set_call_have_args(true);
	}

	return ecall;
}

ExprPtr Parser::parse_expr(){
	expr_end_flag_ = false;
	ExprPtr ret = parse_expr(0);
	return ret;
}

ExprPtr Parser::parse_expr(int_t pri){
	
	ExprPtr ret = parse_term();
	if(!ret){
		return null;
	}
	
	while(true){
		if(ExprPtr ret2 = parse_post(ret, pri)){
			ret = ret2;
		}else{
			break;
		}
	}
	
	return ret;
}

ExprPtr Parser::parse_expr_must(int_t pri){
	ExprPtr ret = parse_expr(pri);
	if(!ret){
		com_->error(line(), Xt("Xtal Compile Error 1001"));
	}
	return ret;
}

ExprPtr Parser::parse_expr_must(){
	expr_end_flag_ = false;
	ExprPtr ret = parse_expr_must(0);
	return ret;

}

ExprPtr Parser::parse_if(){
	expect('(');
	ExprPtr eif = if_(line(), null, null, null);
	if(InternedStringPtr var = parse_var()){
		ExprPtr escope = scope(line(), xnew<Array>());
		escope->push_back(define(line(), lvar(line(), var), parse_expr_must()));
		expect(')');
		eif->set_if_cond(lvar(line(), var));
		eif->set_if_body(parse_stmt_must());
		if(eat(Token::KEYWORD_ELSE)){
			eif->set_if_else(parse_stmt_must());
		}
		escope->push_back(eif);
		return escope;
	}else{
		eif->set_if_cond(parse_expr_must());
		expect(')');
		eif->set_if_body(parse_stmt_must());
		if(eat(Token::KEYWORD_ELSE)){
			eif->set_if_else(parse_stmt_must());
		}
		return eif;		
	}
}

ExprPtr Parser::parse_while(const InternedStringPtr& label){
	expect('(');
	ExprPtr efor = for_(line(), label, null, null, null, null, null);
	if(InternedStringPtr var = parse_var()){
		ExprPtr escope = scope(line(), xnew<Array>());
		escope->push_back(define(line(), lvar(line(), var), parse_expr_must()));
		expect(')');
		efor->set_for_cond(lvar(line(), var));
		efor->set_for_body(parse_stmt_must());
		if(eat(Token::KEYWORD_ELSE)){
			efor->set_for_else(parse_stmt_must());
		}else if(eat(Token::KEYWORD_NOBREAK)){
			efor->set_for_nobreak(parse_stmt_must());
		}		
		escope->push_back(efor);
		return escope;
	}else{
		efor->set_for_cond(parse_expr_must());
		expect(')');
		efor->set_for_body(parse_stmt_must());
		if(eat(Token::KEYWORD_ELSE)){
			efor->set_for_else(parse_stmt_must());
		}else if(eat(Token::KEYWORD_NOBREAK)){
			efor->set_for_nobreak(parse_stmt_must());
		}
		return efor;		
	}
}

ExprPtr Parser::parse_switch(){
	/*
	expect('(');
	e.block_begin();
		int_t var = parse_var();
		if(!var){
			var = com_->register_ident(InternedStringPtr("__SWITCH__"));
		}
		
		e.block_add(e.define(e.lvar(var), parse_expr_must()));
		expect(')');
		expect('{');
		
		ExprPtr if_stmt = 0;
		ExprPtr first = 0;
		ExprPtr default_stmt = 0;
		while(true){
			if(eat(Token::KEYWORD_CASE)){
			
				IfExprPtr temp = XTAL_NEW IfStmt(lexer_.line());
				
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
							bin_comp(InstEq::NUMBER, e.lvar(var), parse_expr_must()));
					}else{
						temp->cond_expr = bin_comp(InstEq::NUMBER, e.lvar(var), parse_expr_must());
					}
					
					if(eat(',')){
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
		*/
	return null;
}

ExprPtr Parser::parse_array(){
	int_t ln = line();
	
	if(eat(']')){//empty array
		return array(ln, null);
	}
	
	if(eat(':')){//empty map
		expect(']');
		return map(ln, null);
	}
		
	ExprPtr key = parse_expr_must();
	if(eat(':')){//map
		ExprPtr emap = map(ln, xnew<Map>());
		emap->map_values()->set_at(key, parse_expr_must());	
		
		if(eat(',')){
			for(;;){
				key = parse_expr();
				if(key){
					expect(':');
					emap->map_values()->set_at(key, parse_expr_must());
					
					if(!eat(',')){
						break;
					}
				}else{
					break;
				}
			}
		}

		expect(']');
		return emap;
	}else{//array
		ExprPtr earray = array(ln, xnew<Array>());
		earray->array_values()->push_back(key);
		if(eat(',')){
			while(true){
				key = parse_expr();
				if(key){
					earray->array_values()->push_back(key);
					if(!eat(',')){
						break;
					}
				}else{
					break;
				}
			}
		}
		expect(']');
		return earray;
	}
}

}

#endif
