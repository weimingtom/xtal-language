
#pragma once

#ifndef XTAL_NO_PARSER

#include "xtal_lexer.h"
#include "xtal_allocator.h"
#include "xtal_expr.h"

namespace xtal{

class Parser{
public:

	Parser();

	Stmt* parse(const Stream& stream, const String& source_file_name);

	void begin_interactive_parsing(const Stream& stream);
	Stmt* interactive_parse();

	/**
	* Lexer, Parser, CodeBuilderが共通して持つLPCCommonオブジェクトを返す
	*/
	LPCCommon* common();
	
	ExprBuilder* expr_builder();
	
	/**
	* 内部で使用しているメモリなどのリソースを解放する
	*/
	void release();

private:

	int_t line(){ return lexer_.line(); }
	
	void replace();
	
	void expect(int_t ch);
	void expect_a(int_t ch);
	void expect_end();
	
	bool eat(int_t ch);
	bool eat(Token::Keyword kw);
	bool eat_a(Token::Keyword kw);
	bool eat_end();
	bool eat_a(int_t ch);

	string_t parse_string(int_t open, int_t close);
	Expr* string2expr(string_t &str);
	Expr* parse_pre_expr(int_t code, int_t expr_priority);
	Expr* parse_post_expr(int_t code, int_t pri, int_t space, Expr* lhs, bool left, int_t expr_priority);
	Expr* parse_bin_expr(int_t code, int_t pri, int_t space, Expr* lhs, bool left, int_t expr_priority);
	Expr* parse_bin_comp_expr(int_t code, int_t pri, int_t space, Expr* lhs, bool left, int_t expr_priority);
	Expr* parse_post(Expr* lhs, int_t pri);
	Stmt* parse_assign_stmt();
	Stmt* parse_stmt2();
	Stmt* parse_stmt();
	Stmt* parse_stmt_must();
	Stmt* parse_assert();
	Stmt* parse_each(int_t label, Expr* lhs);
	void parse_multiple_stmt(TList<Stmt*>* stmts);
	bool parse_multiple_expr(TList<Expr*>* exprs, bool discard=false);
	Stmt* parse_define_local_stmt();
	Expr* parse_define_local_or_expr();
	int_t parse_var();
	int_t parse_ident();
	int_t parse_ident_or_keyword();
	Stmt* parse_if();
	Expr* parse_term();
	Stmt* parse_top_level();
	Stmt* parse_block();
	Stmt* parse_switch();
	Expr* parse_frame(int_t kind);
	Expr* parse_class();
	Expr* parse_prop();
	Stmt* parse_loop();
	Expr* parse_fun(int_t kind);
	Expr* parse_call(Expr* lhs);
	Expr* parse_expr(int_t pri);
	Expr* parse_expr();
	Expr* parse_expr_must(int_t pri);
	Expr* parse_expr_must();
	Expr* parse_array();
	Stmt* parse_for(int_t label = 0);
	Stmt* parse_try();
	int_t parse_number_suffix();
	Expr* parse_number();
	Stmt* parse_return();
	Stmt* parse_continue();
	Stmt* parse_break();
	Stmt* parse_while(int_t label = 0);
	Stmt* parse_using();
	Stmt* parse_throw();
		
public:

	Lexer lexer_;
	LPCCommon* com_;
	RegionAlloc alloc_;
	ExprBuilder e;
};


}

#endif

