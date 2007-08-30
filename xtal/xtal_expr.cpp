
#include "xtal.h"

#ifndef XTAL_NO_PARSER

#include "xtal_any.h"
#include "xtal_vmachine.h"
#include "xtal_expr.h"
#include "xtal_macro.h"

namespace xtal{

void* RegionAllocObject::operator new(size_t size, RegionAlloc* r){ return r->allocate(size); }
void RegionAllocObject::operator delete(void *, RegionAlloc*){}


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
	if(a2)call_arg(a2);
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

void ExprBuilder::scope_push(Vars* vars){
	vars->on_heap = debug::is_enabled();
	vars_stack.push(vars);
}

void ExprBuilder::scope_carry_on_heap_flag(){
	vars_stack[0]->on_heap = vars_stack[1]->on_heap;
}

void ExprBuilder::scope_set_on_heap_flag(int_t i){
	for(; i<(int_t)vars_stack.size(); ++i){
		vars_stack[i]->on_heap = true;
	}
}

void ExprBuilder::scope_pop(){
	vars_stack.pop();	
}

void ExprBuilder::register_variable(int_t var){
	for(TList<Var>::Node* p = vars_stack[0]->vars.head; p; p = p->next){
		if(p->value.name == var){
			common->error(line(), Xt("Xtal Compile Error 1026")(
				Named("name", common->ident_table->at(var))
			));
		}
	}

	Var v;
	v.name = var;
	v.constant = true;
	v.init = 0;
	v.accessibility = KIND_PUBLIC;
	v.ns = 0;
	vars_stack[0]->vars.push_back(v, alloc);
}

void ExprBuilder::block_begin(){
	block_stack.push(new(alloc) BlockStmt(line()));
	scope_push(&block_stack.top()->vars);
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
	scope_push(&try_stack.top()->catch_vars);
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
	scope_set_on_heap_flag(0);
	scope_push(&fun_stack.top()->vars);
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

void ExprBuilder::fun_have_args(bool v){
	fun_stack.top()->have_args = v;
}

void ExprBuilder::fun_body_begin(){
	block_begin();

	if(fun_stack.top()->have_args){
		int_t var = register_ident(Xid(__ARGS__));
		block_add(define(local(var), pseudo(InstPushArgs::NUMBER)));
	}
}

void ExprBuilder::fun_body_add(Stmt* stmt){
	block_add(stmt);
}

void ExprBuilder::fun_body_end(){
	fun_body(block_end());
}

IntExpr* ExprBuilder::int_(int_t value){
	return new(alloc) IntExpr(line(), value);
}
FloatExpr* ExprBuilder::float_(float_t value){
	return new(alloc) FloatExpr(line(), value);
}
ArgsExpr* ExprBuilder::args(){
	return new(alloc) ArgsExpr(line());
}
OnceExpr* ExprBuilder::once(Expr* expr){
	return new(alloc) OnceExpr(line(), expr);
}
CalleeExpr* ExprBuilder::callee(){
	return new(alloc) CalleeExpr(line());
}
AndAndExpr* ExprBuilder::andand(Expr* lhs, Expr* rhs){
	return new(alloc) AndAndExpr(line(), lhs, rhs);
}
TerExpr* ExprBuilder::ter(Expr* cond, Expr* true_expr, Expr* false_expr){
	TerExpr* ret = new(alloc) TerExpr(line(), cond);
	ret->second = true_expr;
	ret->third = false_expr;
	return ret;
}
OrOrExpr* ExprBuilder::oror(Expr* lhs, Expr* rhs){
	return new(alloc) OrOrExpr(line(), lhs, rhs);
}
PushStmt* ExprBuilder::push(Expr* expr){
	return new(alloc) PushStmt(line(), expr);
}
PopExpr* ExprBuilder::pop(){
	return new(alloc) PopExpr(line());
}
IncStmt* ExprBuilder::inc(int_t code, Expr* expr){
	return new(alloc) IncStmt(line(), code, expr);	
}

ContinueStmt* ExprBuilder::continue_(int_t name){
	return new(alloc) ContinueStmt(line(), name);
}
BreakStmt* ExprBuilder::break_(int_t name){
	return new(alloc) BreakStmt(line(), name);
}

TList<Stmt*>* ExprBuilder::block_stmts(){
	return &block_stack.top()->stmts;
}


void ExprBuilder::toplevel_begin(){
	toplevel_stack.push(new(alloc) TopLevelStmt(line()));
	scope_push(&toplevel_stack.top()->vars);
	scope_set_on_heap_flag(0);
}
void ExprBuilder::toplevel_add(Stmt* stmt){
	toplevel_stack.top()->stmts.push_back(stmt, alloc);
}
void ExprBuilder::toplevel_export(int_t name, Expr* expr){
	if(toplevel_stack.top()->export_expr){
		common->error(line(), Xt("Xtal Compile Error 1019"));
	}else{
		int_t export_id = register_ident(InternedStringPtr("__EXPORT__"));
		
		if(name){
			toplevel_add(define(local(name), expr));
			toplevel_add(define(local(export_id), local(name)));
		}else{
			toplevel_add(define(local(export_id), expr));
		}
		toplevel_stack.top()->export_expr = local(export_id);
	}
}
TopLevelStmt* ExprBuilder::toplevel_end(){
	scope_pop();
	return toplevel_stack.pop();
}

void ExprBuilder::class_begin(int_t kind){
	class_stack.push(new(alloc) ClassExpr(line()));
	class_stack.top()->kind = kind;
	scope_push(&class_stack.top()->vars);
	scope_set_on_heap_flag(0);
}
void ExprBuilder::class_define_instance_variable(int_t name, Expr* expr){
	for(TPairList<int_t, Expr*>::Node* p=class_stack.top()->inst_vars.head; p; p=p->next){
		if(p->key==name){
			common->error(line(), Xt("Xtal Compile Error 1024")(Named("name", name)));
			break;
		}
	}

	class_stack.top()->inst_vars.push_back(name, expr, alloc);
}
void ExprBuilder::class_define_member(int_t var, int_t accessibility, Expr* ns, Expr* rhs){
	register_variable(var);
	Var v;
	v.name = var;
	v.constant = true;
	v.init = rhs;
	v.accessibility = accessibility;
	v.ns = ns;
	class_stack.top()->vars.vars.push_back(v, alloc);
}
ClassExpr* ExprBuilder::class_end(){
	fun_begin(KIND_METHOD);
		block_begin();
			for(TPairList<int_t, Expr*>::Node* p=class_stack.top()->inst_vars.head; p; p=p->next){
				block_add(assign(instance_variable(p->key), p->value));
			}
		fun_body(block_end());
	class_define_member(register_ident("__INITIALIZE__"), KIND_PUBLIC, pseudo(InstPushNull::NUMBER), fun_end());

	scope_pop();
	return class_stack.pop();
}
TList<Expr*>* ExprBuilder::class_mixins(){
	return &class_stack.top()->mixins;
}

void ExprBuilder::call_begin(Expr* expr){
	call_stack.push(new(alloc) CallExpr(line(), expr));
}
void ExprBuilder::call_arg(Expr* expr){
	call_stack.top()->ordered.push_back(expr, alloc);
}
void ExprBuilder::call_arg(int_t name, Expr* expr){
	call_stack.top()->named.push_back(name, expr, alloc);
}
CallExpr* ExprBuilder::call_end(){
	if(call_stack.top()->ordered.tail && !call_stack.top()->ordered.tail->value){
		call_stack.top()->ordered.pop_back();
	}
	return call_stack.pop();
}

void ExprBuilder::return_begin(){
	return_stack.push(new(alloc) ReturnStmt(line()));
}
void ExprBuilder::return_add(Expr* expr){
	return_exprs()->push_back(expr, alloc);
}
ReturnStmt* ExprBuilder::return_end(){
	return return_stack.pop();
}
TList<Expr*>* ExprBuilder::return_exprs(){
	return &return_stack.top()->exprs;
}

void ExprBuilder::yield_begin(){
	yield_stack.push(new(alloc) YieldStmt(line()));
}
void ExprBuilder::yield_add(Expr* expr){
	yield_exprs()->push_back(expr, alloc);
}
YieldStmt* ExprBuilder::yield_end(){
	return yield_stack.pop();
}
TList<Expr*>* ExprBuilder::yield_exprs(){
	return &yield_stack.top()->exprs;
}

void ExprBuilder::massign_begin(){
	massign_stack.push(new(alloc) MultipleAssignStmt(line()));
}
void ExprBuilder::massign_lhs(Expr* expr){
	massign_lhs_exprs()->push_back(expr, alloc);
}
void ExprBuilder::massign_rhs(Expr* expr){
	massign_rhs_exprs()->push_back(expr, alloc);
}
MultipleAssignStmt* ExprBuilder::massign_end(){
	return massign_stack.pop();
}
TList<Expr*>* ExprBuilder::massign_lhs_exprs(){
	return &massign_stack.top()->lhs;
}
TList<Expr*>* ExprBuilder::massign_rhs_exprs(){
	return &massign_stack.top()->rhs;
}
void ExprBuilder::massign_define(bool b){
	massign_stack.top()->define = b;
}
void ExprBuilder::ter_begin(Expr* cond){
	ter_stack.push(new(alloc) TerExpr(line(), cond));
}
void ExprBuilder::ter_true(Expr* expr){
	ter_stack.top()->second = expr;
}
void ExprBuilder::ter_false(Expr* expr){
	ter_stack.top()->third = expr;
}
TerExpr* ExprBuilder::ter_end(){
	return ter_stack.pop();
}

void ExprBuilder::array_begin(){
	array_stack.push(new(alloc) ArrayExpr(line()));
}
void ExprBuilder::array_add(Expr* expr){
	array_stack.top()->values.push_back(expr, alloc);
}
ArrayExpr* ExprBuilder::array_end(){
	return array_stack.pop();
}

void ExprBuilder::map_begin(){
	map_stack.push(new(alloc) MapExpr(line()));
}
void ExprBuilder::map_add(Expr* key, Expr* value){
	map_stack.top()->values.push_back(key, value, alloc);
}
MapExpr* ExprBuilder::map_end(){
	return map_stack.pop();
}

void ExprBuilder::init(LPCCommon* com, RegionAlloc* all){
	common = com;
	alloc = all;
	vars_stack.clear();
	block_stack.clear();
	try_stack.clear();
	while_stack.clear();
	if_stack.clear();
	fun_stack.clear();
}

}

#endif

