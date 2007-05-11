
#pragma once

#include "lexer.h"
#include "allocator.h"
#include "expr.h"

namespace xtal{

struct ExprBuilder{

	PseudoVariableExpr* pseudo(int_t code){
		return new(alloc) PseudoVariableExpr(common->line, code);
	}
	
	BinExpr* bin(int_t code, Expr* lhs, Expr* rhs){
		return new(alloc) BinExpr(common->line, code, lhs, rhs);
	}
	
	BinCompExpr* bin_comp(int_t code, Expr* lhs, Expr* rhs){
		return new(alloc) BinCompExpr(common->line, code, lhs, rhs);
	}
	
	OpAssignStmt* op_assign(int_t code, Expr* lhs, Expr* rhs){
		return new(alloc) OpAssignStmt(common->line, code, lhs, rhs);
	}

	AtExpr* at(Expr* lhs, Expr* rhs){
		return new(alloc) AtExpr(common->line, lhs, rhs);
	}
	
	LocalExpr* local(int_t var){
		return new(alloc) LocalExpr(common->line, var);
	}
	
	InstanceVariableExpr* instance_variable(int_t var){
		return new(alloc) InstanceVariableExpr(common->line, var);
	}
	
	DefineStmt* define(Expr* lhs, Expr* rhs){
		if(LocalExpr* loc = expr_cast<LocalExpr>(lhs)){
			register_variable(loc->var);
		}
		return new(alloc) DefineStmt(common->line, lhs, rhs);
	}
	
	AssignStmt* assign(Expr* lhs, Expr* rhs){
		return new(alloc) AssignStmt(common->line, lhs, rhs);
	}
	
	CallExpr* call(Expr* lhs, Expr* a1 = 0, Expr* a2 = 0){
		CallExpr* ret = new(alloc) CallExpr(common->line, lhs);
		if(a1)ret->ordered.push_back(a1);
		if(a2)ret->ordered.push_back(a2);
		return ret;
	}

	MemberExpr* member(Expr* lhs, int_t var){
		return new(alloc) MemberExpr(common->line, lhs, var);
	}
	
	MemberExpr* member_q(Expr* lhs, int_t var){
		MemberExpr* ret = new(alloc) MemberExpr(common->line, lhs, var);
		ret->if_defined = true;
		return ret;
	}

	SendExpr* send(Expr* lhs, int_t var){
		return new(alloc) SendExpr(common->line, lhs, var);
	}
	
	SendExpr* send_q(Expr* lhs, int_t var){
		SendExpr* ret = new(alloc) SendExpr(common->line, lhs, var);
		ret->if_defined = true;
		return ret;
	}
	
	ExprStmt* e2s(Expr* expr){
		return new(alloc) ExprStmt(common->line, expr);
	}

	ReturnStmt* return_(Expr* e1 = 0, Expr* e2 = 0){
		ReturnStmt* ret = new(alloc) ReturnStmt(common->line);
		if(e1)ret->exprs.push_back(e1);
		if(e2)ret->exprs.push_back(e2);
		return ret;
	}

	AssertStmt* assert_(Expr* e1 = 0, Expr* e2 = 0){
		AssertStmt* ret = new(alloc) AssertStmt(common->line);
		if(e1)ret->exprs.push_back(e1);
		if(e2)ret->exprs.push_back(e2);
		return ret;
	}
	
	
/////////////

	struct VarInfo{
		TList<int_t>* variables;
		bool* on_heap_flag;
		bool set_name_flag;
	};
	
	PODStack<VarInfo> var_info_stack;
	
	void scope_push(TList<int_t>* list, bool* on_heap, bool set_name_flag){
		VarInfo vi = {list, on_heap, set_name_flag};
		var_info_stack.push(vi);
		*on_heap = false;
	}
	
	void scope_carry_on_heap_flag(){
		*var_info_stack[0].on_heap_flag = *var_info_stack[1].on_heap_flag;
	}
	
	void scope_set_on_heap_flag(int_t i){
		*var_info_stack[i].on_heap_flag = true;
	}

	void scope_pop(){
		var_info_stack.pop();	
	}

	void register_variable(int_t var){
		for(TList<int_t>::Node* p = var_info_stack.top().variables->head; p; p = p->next){
			if(p->value == var){
				common->error(common->line, Xt("同じスコープ内で、同じ変数名 '%(name)s' が重複定義されました")(
					Xid(name)=common->ident_table[var]
				));
			}
		}
		var_info_stack.top().variables->push_back(var);
	}
	
/////////////

	void block_begin(){
		block_stack.push(new(alloc) BlockStmt(common->line));
		scope_push(&block_stack.top()->vars, &block_stack.top()->on_heap, false);
	}	

	void block_add(Stmt* stmt){
		block_stack.top()->stmts.push_back(stmt);
	}

	BlockStmt* block_end(){
		scope_pop();
		return block_stack.pop();
	}

	void try_begin(){
		try_stack.push(new(alloc) TryStmt(common->line));
		scope_push(&try_stack.top()->catch_vars, &try_stack.top()->on_heap, false);
	}

	void try_body(Stmt* stmt){
		try_stack.top()->try_stmt = stmt;
	}

	void try_catch(Stmt* stmt){
		try_stack.top()->catch_stmt = stmt;
	}

	void try_finally(Stmt* stmt){
		try_stack.top()->finally_stmt = stmt;
	}

	TryStmt* try_end(){
		scope_pop();
		return try_stack.pop();
	}

	void while_begin(int_t var, Expr* expr){
		while_stack.push(new(alloc) WhileStmt(common->line));
		block_begin();
		if(var){
			block_add(define(local(var), expr));
			while_stack.top()->cond_expr = local(var);
		}else{
			while_stack.top()->cond_expr = expr;
		}
	}

	void while_label(int_t label){
		while_stack.top()->label = label;
	}

	void while_body(Stmt* stmt){
		while_stack.top()->body_stmt = stmt;
	}

	void while_next(Stmt* stmt){
		while_stack.top()->next_stmt = stmt;
	}

	void while_else(Stmt* stmt){
		while_stack.top()->else_stmt = stmt;
	}

	void while_nobreak(Stmt* stmt){
		while_stack.top()->nobreak_stmt = stmt;
	}

	Stmt* while_end(){
		block_add(while_stack.pop());
		return block_end();
	}

	void if_begin(int_t var, Expr* expr){
		if_stack.push(new(alloc) IfStmt(common->line));
		block_begin();
		if(var){
			block_add(define(local(var), expr));
			if_stack.top()->cond_expr = local(var);
		}else{
			if_stack.top()->cond_expr = expr;
		}
	}

	void if_body(Stmt* stmt){
		if_stack.top()->body_stmt = stmt;
	}

	void if_else(Stmt* stmt){
		if_stack.top()->else_stmt = stmt;
	}

	Stmt* if_end(){
		block_add(if_stack.pop());
		return block_end();
	}

	void fun_begin(int_t kind){
		fun_stack.push(new(alloc) FunExpr(common->line, kind));
		scope_push(&fun_stack.top()->vars, &fun_stack.top()->on_heap, false);
		scope_set_on_heap_flag(1);
	}

	void fun_param(int_t name, Expr* def = 0){
		register_variable(name);
		fun_stack.top()->params.push_back(name, def);
	}

	void fun_body(Stmt* stmt){
		fun_stack.top()->stmt = stmt;
	}

	FunExpr* fun_end(){
		scope_pop();
		return fun_stack.pop();
	}
	
	void init(LPCCommon* com, RegionAlloc* all){
		common = com;
		alloc = all;
		var_info_stack.clear();
		block_stack.clear();
		try_stack.clear();
		while_stack.clear();
		if_stack.clear();
		fun_stack.clear();
	}

	LPCCommon* common;
	RegionAlloc* alloc;
	
	PStack<BlockStmt*> block_stack;
	PStack<TryStmt*> try_stack;
	PStack<WhileStmt*> while_stack;
	PStack<IfStmt*> if_stack;
	PStack<FunExpr*> fun_stack;

};


class Parser{
public:

	Parser();

	Stmt* parse(const Stream& stream, const String& source_file_name);

	void begin_interactive_parsing(const Stream& stream);
	void end_interactive_parsing();
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
	
	Lexer& lexer(){
		return lexer_;
	}

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
	Stmt* parse_switch();
	Expr* parse_frame(int_t kind);
	Expr* parse_class_or_module(int_t kind);
	Expr* parse_prop();
	Stmt* parse_loop();
	Expr* parse_fun(int_t kind);
	Expr* parse_lambda();
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
