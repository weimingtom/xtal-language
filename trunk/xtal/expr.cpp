
#include "xtal.h"

#ifndef XTAL_NO_PARSER

#include "any.h"
#include "vmachine.h"
#include "expr.h"

namespace xtal{

PseudoVariableExpr* ExprBuilder::pseudo(int_t code){
	return new(alloc) PseudoVariableExpr(common->line, code);
}

UnaExpr* ExprBuilder::una(int_t code, Expr* term){
	return new(alloc) UnaExpr(common->line, code, term);
}

BinExpr* ExprBuilder::bin(int_t code, Expr* lhs, Expr* rhs){
	return new(alloc) BinExpr(common->line, code, lhs, rhs);
}

BinCompExpr* ExprBuilder::bin_comp(int_t code, Expr* lhs, Expr* rhs){
	return new(alloc) BinCompExpr(common->line, code, lhs, rhs);
}

OpAssignStmt* ExprBuilder::op_assign(int_t code, Expr* lhs, Expr* rhs){
	return new(alloc) OpAssignStmt(common->line, code, lhs, rhs);
}

AtExpr* ExprBuilder::at(Expr* lhs, Expr* rhs){
	return new(alloc) AtExpr(common->line, lhs, rhs);
}

LocalExpr* ExprBuilder::local(int_t var){
	return new(alloc) LocalExpr(common->line, var);
}

InstanceVariableExpr* ExprBuilder::instance_variable(int_t var){
	return new(alloc) InstanceVariableExpr(common->line, var);
}

DefineStmt* ExprBuilder::define(Expr* lhs, Expr* rhs){
	if(LocalExpr* loc = expr_cast<LocalExpr>(lhs)){
		register_variable(loc->var);
	}
	return new(alloc) DefineStmt(common->line, lhs, rhs);
}

AssignStmt* ExprBuilder::assign(Expr* lhs, Expr* rhs){
	return new(alloc) AssignStmt(common->line, lhs, rhs);
}

CallExpr* ExprBuilder::call(Expr* lhs, Expr* a1, Expr* a2){
	CallExpr* ret = new(alloc) CallExpr(common->line, lhs);
	if(a1)ret->ordered.push_back(a1);
	if(a2)ret->ordered.push_back(a2);
	return ret;
}

MemberExpr* ExprBuilder::member(Expr* lhs, int_t var){
	return new(alloc) MemberExpr(common->line, lhs, var);
}

MemberExpr* ExprBuilder::member_q(Expr* lhs, int_t var){
	MemberExpr* ret = new(alloc) MemberExpr(common->line, lhs, var);
	ret->if_defined = true;
	return ret;
}

SendExpr* ExprBuilder::send(Expr* lhs, int_t var){
	return new(alloc) SendExpr(common->line, lhs, var);
}

SendExpr* ExprBuilder::send_q(Expr* lhs, int_t var){
	SendExpr* ret = new(alloc) SendExpr(common->line, lhs, var);
	ret->if_defined = true;
	return ret;
}

ExprStmt* ExprBuilder::e2s(Expr* expr){
	return new(alloc) ExprStmt(common->line, expr);
}

ReturnStmt* ExprBuilder::return_(Expr* e1, Expr* e2){
	ReturnStmt* ret = new(alloc) ReturnStmt(common->line);
	if(e1)ret->exprs.push_back(e1);
	if(e2)ret->exprs.push_back(e2);
	return ret;
}

AssertStmt* ExprBuilder::assert_(Expr* e1, Expr* e2){
	AssertStmt* ret = new(alloc) AssertStmt(common->line);
	if(e1)ret->exprs.push_back(e1);
	if(e2)ret->exprs.push_back(e2);
	return ret;
}

SetAccessibilityStmt* ExprBuilder::set_accessibility(int_t var, int_t kind){
	SetAccessibilityStmt* ret = new(alloc) SetAccessibilityStmt(common->line);
	ret->var = var;
	ret->kind = kind;
	return ret;
}

void ExprBuilder::scope_push(TList<int_t>* list, bool* on_heap, bool set_name_flag){
	VarInfo vi = {list, on_heap, set_name_flag};
	var_info_stack.push(vi);
	*on_heap = false;
}

void ExprBuilder::scope_carry_on_heap_flag(){
	*var_info_stack[0].on_heap_flag = *var_info_stack[1].on_heap_flag;
}

void ExprBuilder::scope_set_on_heap_flag(int_t i){
	*var_info_stack[i].on_heap_flag = true;
}

void ExprBuilder::scope_pop(){
	var_info_stack.pop();	
}

void ExprBuilder::register_variable(int_t var){
	for(TList<int_t>::Node* p = var_info_stack.top().variables->head; p; p = p->next){
		if(p->value == var){
			common->error(common->line, Xt("Xtal Compile Error 1026")(
				Named("name", common->ident_table[var])
			));
		}
	}
	var_info_stack.top().variables->push_back(var);
}

void ExprBuilder::block_begin(){
	block_stack.push(new(alloc) BlockStmt(common->line));
	scope_push(&block_stack.top()->vars, &block_stack.top()->on_heap, false);
}	

void ExprBuilder::block_add(Stmt* stmt){
	block_stack.top()->stmts.push_back(stmt);
}

BlockStmt* ExprBuilder::block_end(){
	scope_pop();
	return block_stack.pop();
}

void ExprBuilder::try_begin(){
	try_stack.push(new(alloc) TryStmt(common->line));
	scope_push(&try_stack.top()->catch_vars, &try_stack.top()->on_heap, false);
}

void ExprBuilder::try_body(Stmt* stmt){
	try_stack.top()->try_stmt = stmt;
}

void ExprBuilder::try_catch(Stmt* stmt){
	try_stack.top()->catch_stmt = stmt;
}

void ExprBuilder::try_finally(Stmt* stmt){
	try_stack.top()->finally_stmt = stmt;
}

TryStmt* ExprBuilder::try_end(){
	scope_pop();
	return try_stack.pop();
}

void ExprBuilder::while_begin(int_t var, Expr* expr){
	while_stack.push(new(alloc) WhileStmt(common->line));
	block_begin();
	if(var){
		block_add(define(local(var), expr));
		while_stack.top()->cond_expr = local(var);
	}else{
		while_stack.top()->cond_expr = expr;
	}
}

void ExprBuilder::while_label(int_t label){
	while_stack.top()->label = label;
}

void ExprBuilder::while_body(Stmt* stmt){
	while_stack.top()->body_stmt = stmt;
}

void ExprBuilder::while_next(Stmt* stmt){
	while_stack.top()->next_stmt = stmt;
}

void ExprBuilder::while_else(Stmt* stmt){
	while_stack.top()->else_stmt = stmt;
}

void ExprBuilder::while_nobreak(Stmt* stmt){
	while_stack.top()->nobreak_stmt = stmt;
}

Stmt* ExprBuilder::while_end(){
	block_add(while_stack.pop());
	return block_end();
}

void ExprBuilder::if_begin(int_t var, Expr* expr){
	if_stack.push(new(alloc) IfStmt(common->line));
	block_begin();
	if(var){
		block_add(define(local(var), expr));
		if_stack.top()->cond_expr = local(var);
	}else{
		if_stack.top()->cond_expr = expr;
	}
}

void ExprBuilder::if_body(Stmt* stmt){
	if_stack.top()->body_stmt = stmt;
}

void ExprBuilder::if_else(Stmt* stmt){
	if_stack.top()->else_stmt = stmt;
}

Stmt* ExprBuilder::if_end(){
	block_add(if_stack.pop());
	return block_end();
}

void ExprBuilder::fun_begin(int_t kind){
	fun_stack.push(new(alloc) FunExpr(common->line, kind));
	scope_push(&fun_stack.top()->vars, &fun_stack.top()->on_heap, false);
	scope_set_on_heap_flag(1);
}

void ExprBuilder::fun_param(int_t name, Expr* def){
	register_variable(name);
	fun_stack.top()->params.push_back(name, def);
}

void ExprBuilder::fun_body(Stmt* stmt){
	fun_stack.top()->stmt = stmt;
}

FunExpr* ExprBuilder::fun_end(){
	scope_pop();
	return fun_stack.pop();
}

void ExprBuilder::init(LPCCommon* com, RegionAlloc* all){
	common = com;
	alloc = all;
	var_info_stack.clear();
	block_stack.clear();
	try_stack.clear();
	while_stack.clear();
	if_stack.clear();
	fun_stack.clear();
}

}

#endif

