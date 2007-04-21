
#pragma once

#include "lexer.h"
#include "allocator.h"
#include "expr.h"

namespace xtal{

class Parser{
public:

	Parser();

	Stmt* parse(const Stream& stream, const String& source_file_name);

	/**
	* Lexer, Parser, CodeBuilderが共通して持つLPCCommonオブジェクトを返す
	*/
	LPCCommon common();
	
	/**
	* 内部で使用しているメモリなどのリソースを解放する
	*/
	void release();
	
private:
	
	PStack<BlockStmt*> block_stack_;
	PStack<TryStmt*> try_stack_;
	PStack<WhileStmt*> while_stack_;
	PStack<IfStmt*> if_stack_;
	PStack<FunExpr*> fun_stack_;

	void block_begin();
	void block_add(Stmt* stmt);
	BlockStmt* block_end();

	Expr* inst(int_t var);
	Stmt* set_inst(int_t var, Expr* rhs);

	Expr* local(int_t var);
	DefineLocalStmt* def_local(int_t var, Expr* expr);
	Stmt* set_local(int_t var, Expr* expr);
	
	CallExpr* call(Expr* lhs, Expr* a1 = 0, Expr* a2 = 0);
	Expr* member(Expr* lhs, bool if_defined, int_t var);
	CallExpr* send(Expr* lhs, bool if_defined, int_t var, Expr* a1 = 0, Expr* a2 = 0);
	Stmt* e2s(Expr* expr);

	void try_begin();
	void try_body(Stmt* stmt);
	void try_catch(Stmt* stmt);
	void try_finally(Stmt* stmt);
	TryStmt* try_end();

	void while_begin(int_t var, Expr* expr);
	void while_label(int_t label);
	void while_body(Stmt* stmt);
	void while_next(Stmt* stmt);
	void while_else(Stmt* stmt);
	Stmt* while_end();

	void if_begin(int_t var, Expr* expr);
	void if_body(Stmt* stmt);
	void if_else(Stmt* stmt);
	Stmt* if_end();

	void fun_begin(int_t kind);
	void fun_param(int_t name, Expr* def = 0);
	void fun_body(Stmt* stmt);
	FunExpr* fun_end();

	Stmt* return_(Expr* e1 = 0, Expr* e2 = 0);

	void multi_begin();
	void multi_add(Expr* expr);
	TList<Expr*> multi_end();

	Stmt* assert_(Expr* e1, Expr* e2);

private:

	int_t line(){ return lexer_.line(); }
	
	void push_scope(TList<int_t>* list, bool* on_heap, bool set_object_name);

	void set_scope_on_heap_flag(int_t i = 0);
	
	void set_scope_set_name_flag(bool b);
	
	void pop_scope();
	
	void register_variable(int_t var);

	void replace();

	void expect(int_t ch);
	
	void expect_a(int_t ch);

	bool eat(int_t ch);
	
	bool eat(Token::Keyword kw);

	bool eat_a(Token::Keyword kw);

	bool eat_end();
	
	bool eat_a(int_t ch);

	void expect_end();

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

	void parse_multiple_expr(TList<Expr*>* exprs);

	Stmt* parse_define_local_stmt();

	Expr* parse_define_local_or_expr();

	int_t parse_var();

	int_t parse_ident();

	int_t parse_ident_or_keyword();

	Stmt* parse_if();

	Expr* parse_term();

	Stmt* parse_top_level();

	Stmt* parse_block();

	Expr* parse_frame(int_t kind);

	Expr* parse_class_or_module(int_t kind);

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
		
private:

	Lexer lexer_;
	LPCCommon com_;
	RegionAlloc alloc_;
	
	struct VarInfo{
		TList<int_t>* variables;
		bool* on_heap_flag;
		bool set_name_flag;
	};
	
	PODStack<VarInfo> var_info_stack_;
	
};


}
