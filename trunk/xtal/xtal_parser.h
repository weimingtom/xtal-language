
#pragma once

#ifndef XTAL_NO_PARSER

#include "xtal_lexer.h"
#include "xtal_allocator.h"
#include "xtal_expr.h"

namespace xtal{

class Parser{
public:

	Parser();

	ExprPtr parse(const StreamPtr& stream, const StringPtr& source_file_name);

	void begin_interactive_parsing(const StreamPtr& stream);

	ExprPtr interactive_parse();

	/**
	* Lexer, Parser, CodeBuilderが共通して持つLPCCommonオブジェクトを返す
	*/
	LPCCommon* common();
		
	/**
	* 内部で使用しているメモリなどのリソースを解放する
	*/
	void release();

private:

	int_t line(){ return lexer_.line(); }
	
	void expect(int_t ch);
	
	bool eat(int_t ch);
	bool eat(Token::Keyword kw);

	InternedStringPtr to_id(int_t ident);

	StringPtr parse_string(int_t open, int_t close);
	ExprPtr string2expr(string_t &str);
	ExprPtr parse_pre_expr(int_t code, int_t expr_priority);
	ExprPtr parse_post_expr(int_t code, int_t pri, int_t space, ExprPtr lhs, bool left, int_t expr_priority);
	ExprPtr parse_bin_expr(int_t code, int_t pri, int_t space, ExprPtr lhs, bool left, int_t expr_priority);
	ExprPtr parse_bin_comp_expr(int_t code, int_t pri, int_t space, ExprPtr lhs, bool left, int_t expr_priority);
	ExprPtr parse_post(ExprPtr lhs, int_t pri);
	ExprPtr parse_assign_stmt();
	ExprPtr parse_stmt();
	ExprPtr parse_stmt_must();
	ExprPtr parse_assert();
	ExprPtr parse_each(const InternedStringPtr& label, ExprPtr lhs);
	ArrayPtr parse_stmts();
	ArrayPtr parse_exprs(bool* discard = 0);
	ExprPtr parse_define_local_stmt();
	ExprPtr parse_define_local_or_expr();
	InternedStringPtr parse_var();
	InternedStringPtr parse_ident();
	InternedStringPtr parse_ident_or_keyword();
	ExprPtr parse_if();
	ExprPtr parse_term();
	ExprPtr parse_top_level();
	ExprPtr parse_scope();
	ExprPtr parse_switch();
	ExprPtr parse_frame(int_t kind);
	ExprPtr parse_class(int_t kind);
	ExprPtr parse_prop();
	ExprPtr parse_loop();
	ExprPtr parse_fun(int_t kind);
	ExprPtr parse_call(ExprPtr lhs);
	ExprPtr parse_expr(int_t pri);
	ExprPtr parse_expr();
	ExprPtr parse_expr_must(int_t pri);
	ExprPtr parse_expr_must();
	ExprPtr parse_array();
	ExprPtr parse_for(const InternedStringPtr& label = 0);
	ExprPtr parse_try();
	int_t parse_number_suffix();
	ExprPtr parse_number();
	ExprPtr parse_return();
	ExprPtr parse_continue();
	ExprPtr parse_break();
	ExprPtr parse_while(const InternedStringPtr& label = 0);
	ExprPtr parse_using();
	ExprPtr parse_throw();
		
public:

	bool expr_end_flag_;
	Lexer lexer_;
	LPCCommon* com_;
};


}

#endif

