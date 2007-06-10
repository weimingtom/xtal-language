
#include "xtal.h"

#ifndef XTAL_NO_PARSER

#include "xtal_any.h"
#include "xtal_vmachine.h"
#include "xtal_expr.h"

namespace xtal{

PseudoVariableExpr* ExprBuilder::pseudo(int_t code){
	return new(alloc) PseudoVariableExpr(line(), code);
}

StringExpr* ExprBuilder::string(int_t value, int_t kind){
	return new(alloc) StringExpr(line(), value, kind);
}

UnaExpr* ExprBuilder::una(int_t code, Expr* term){
	return new(alloc) UnaExpr(line(), code, term);
}

BinExpr* ExprBuilder::bin(int_t code, Expr* lhs, Expr* rhs){
	return new(alloc) BinExpr(line(), code, lhs, rhs);
}

BinCompExpr* ExprBuilder::bin_comp(int_t code, Expr* lhs, Expr* rhs){
	return new(alloc) BinCompExpr(line(), code, lhs, rhs);
}

OpAssignStmt* ExprBuilder::op_assign(int_t code, Expr* lhs, Expr* rhs){
	return new(alloc) OpAssignStmt(line(), code, lhs, rhs);
}

AtExpr* ExprBuilder::at(Expr* lhs, Expr* rhs){
	return new(alloc) AtExpr(line(), lhs, rhs);
}

LocalExpr* ExprBuilder::local(int_t var){
	return new(alloc) LocalExpr(line(), var);
}

InstanceVariableExpr* ExprBuilder::instance_variable(int_t var){
	return new(alloc) InstanceVariableExpr(line(), var);
}

DefineStmt* ExprBuilder::define(Expr* lhs, Expr* rhs){
	if(LocalExpr* loc = expr_cast<LocalExpr>(lhs)){
		register_variable(loc->var);
	}
	return new(alloc) DefineStmt(line(), lhs, rhs);
}

AssignStmt* ExprBuilder::assign(Expr* lhs, Expr* rhs){
	return new(alloc) AssignStmt(line(), lhs, rhs);
}

CallExpr* ExprBuilder::call(Expr* lhs, Expr* a1, Expr* a2){
	call_begin(lhs);
	if(a1)call_arg(a1);
	if(a2)call_arg(a1);
	return call_end();
}

MemberExpr* ExprBuilder::member(Expr* lhs, int_t var){
	return new(alloc) MemberExpr(line(), lhs, var);
}

MemberExpr* ExprBuilder::member_q(Expr* lhs, int_t var){
	MemberExpr* ret = new(alloc) MemberExpr(line(), lhs, var);
	ret->if_defined = true;
	return ret;
}

MemberExpr* ExprBuilder::member(Expr* lhs, Expr* var){
	MemberExpr* ret = new(alloc) MemberExpr(line(), lhs);
	ret->pvar = var;
	return ret;
}

MemberExpr* ExprBuilder::member_q(Expr* lhs, Expr* var){
	MemberExpr* ret = new(alloc) MemberExpr(line(), lhs);
	ret->if_defined = true;
	ret->pvar = var;
	return ret;
}

SendExpr* ExprBuilder::send(Expr* lhs, int_t var){
	return new(alloc) SendExpr(line(), lhs, var);
}

SendExpr* ExprBuilder::send_q(Expr* lhs, int_t var){
	SendExpr* ret = new(alloc) SendExpr(line(), lhs, var);
	ret->if_defined = true;
	return ret;
}

SendExpr* ExprBuilder::send(Expr* lhs, Expr* var){
	SendExpr* ret = new(alloc) SendExpr(line(), lhs);
	ret->pvar = var;
	return ret;
}

SendExpr* ExprBuilder::send_q(Expr* lhs, Expr* var){
	SendExpr* ret = new(alloc) SendExpr(line(), lhs);
	ret->if_defined = true;
	ret->pvar = var;
	return ret;
}

ExprStmt* ExprBuilder::e2s(Expr* expr){
	return new(alloc) ExprStmt(line(), expr);
}

ReturnStmt* ExprBuilder::return_(Expr* e1, Expr* e2){
	return_begin();
	ReturnStmt* ret = new(alloc) ReturnStmt(line());
	if(e1)return_add(e1);
	if(e2)return_add(e2);
	return return_end();
}

AssertStmt* ExprBuilder::assert_(Expr* e1, Expr* e2){
	AssertStmt* ret = new(alloc) AssertStmt(line());
	if(e1)ret->exprs.push_back(e1, alloc);
	if(e2)ret->exprs.push_back(e2, alloc);
	return ret;
}

SetAccessibilityStmt* ExprBuilder::set_accessibility(int_t var, int_t kind){
	SetAccessibilityStmt* ret = new(alloc) SetAccessibilityStmt(line());
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
			common->error(line(), Xt("Xtal Compile Error 1026")(
				Named("name", common->ident_table[var])
			));
		}
	}
	var_info_stack.top().variables->push_back(var, alloc);
}

void ExprBuilder::block_begin(){
	block_stack.push(new(alloc) BlockStmt(line()));
	scope_push(&block_stack.top()->vars, &block_stack.top()->on_heap, false);
}	

void ExprBuilder::block_add(Stmt* stmt){
	block_stack.top()->stmts.push_back(stmt, alloc);
}

BlockStmt* ExprBuilder::block_end(){
	scope_pop();
	return block_stack.pop();
}

void ExprBuilder::try_begin(){
	try_stack.push(new(alloc) TryStmt(line()));
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
	while_stack.push(new(alloc) WhileStmt(line()));
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
	if_stack.push(new(alloc) IfStmt(line()));
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
	fun_stack.push(new(alloc) FunExpr(line(), kind));
	scope_push(&fun_stack.top()->vars, &fun_stack.top()->on_heap, false);
	scope_set_on_heap_flag(1);
}

void ExprBuilder::fun_param(int_t name, Expr* def){
	register_variable(name);
	fun_stack.top()->params.push_back(name, def, alloc);
}

FunExpr* ExprBuilder::fun_end(){
	scope_pop();
	return fun_stack.pop();
}

void ExprBuilder::fun_body(Stmt* stmt){
	fun_stack.top()->stmt = stmt;
}

void ExprBuilder::fun_have_args(){
	fun_stack.top()->have_args = true;
}

void ExprBuilder::fun_body_begin(){
	block_begin();

	if(fun_stack.top()->have_args){
		int_t var = register_ident(Xid(__ARGS__));
		block_add(define(local(var), pseudo(CODE_PUSH_ARGS)));
	}
}

void ExprBuilder::fun_body_add(Stmt* stmt){
	block_add(stmt);
}

void ExprBuilder::fun_body_end(){
	fun_body(block_end());
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

