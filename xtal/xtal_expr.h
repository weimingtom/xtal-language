
#pragma once

#ifndef XTAL_NO_PARSER

#include "xtal_codeimpl.h"
#include "xtal_constant.h"
#include "xtal_lexer.h"

namespace xtal{


/*

巡回サンプル

for(TList<Any>::Node* p = values.head; p; p = p->next){
	//p->value;
}

*/
template <class T>
struct TList{

	struct Node{
		T value;

		Node* next;
		Node* prev;

		Node(const T &val = T())
			:value(val), next(0), prev(0){}

		void *operator new(size_t size, RegionAlloc* r){ return r->allocate(size); }
		void operator delete(void *, RegionAlloc*){}
	};

	Node* head;
	Node* tail;
	int size;

	TList()
		:head(0), tail(0), size(0){}

	void push_front(const T& v, RegionAlloc* alloc);

	void push_back(const T &v, RegionAlloc* alloc);

	void pop_back();
};

template <class T>
void TList<T>::push_front(const T &v, RegionAlloc* alloc){
	if(head){
		Node* p = new(alloc) Node(v);
		p->next = head;
		head->prev = p;
		head = p;
	}else{
		head = tail = new(alloc) Node(v);
	}
	size++;
}

template <class T>
void TList<T>::push_back(const T &v, RegionAlloc* alloc){
	if(head){
		Node* p = new(alloc) Node(v);
		p->prev = tail;
		tail->next = p;
		tail = p;
	}else{
		head = tail = new(alloc) Node(v);
	}
	size++;
}

template <class T>
void TList<T>::pop_back(){
	if(head){
		if(head==tail){
			head = tail = 0;
		}else{
			tail = tail->prev;
			tail->next = 0;
		}
		size--;
	}
}

template <class Key, class T>
struct TPairList{

	struct Node{
		Key key;
		T value;

		Node* next;
		Node* prev;

		Node(const Key &key = Key(), const T &val = T())
			:key(key), value(val), next(0), prev(0){}

		void *operator new(size_t size, RegionAlloc* r){ return r->allocate(size); }
		void operator delete(void *, RegionAlloc*){}
	};

	Node* head;
	Node* tail;
	int size;

	TPairList()
		:head(0), tail(0), size(0){}

	void push_back(const Key& k, const T &v, RegionAlloc* alloc);

	void pop_back();
};

template <class Key, class T>
void TPairList<Key, T>::push_back(const Key& k, const T &v, RegionAlloc* alloc){
	if(head){
		Node* p = new(alloc) Node(k, v);
		p->prev = tail;
		tail->next = p;
		tail = p;
	}else{
		head = tail = new(alloc) Node(k, v);
	}
	size++;
}

template <class Key, class T>
void TPairList<Key, T>::pop_back(){
	if(head){
		if(head==tail){
			head = tail = 0;
		}else{
			tail = tail->prev;
			tail->next = 0;
		}
		size--;
	}
}

// 値を返す文法要素
struct Expr{	
	enum{ TYPE = __LINE__ };
	
	int_t type;
	int_t line;
	
	Expr(int_t type, int_t line)
		:type(type), line(line){}

	void *operator new(size_t size, RegionAlloc* a){ return a->allocate(size); }
	void operator delete(void *, RegionAlloc*){}

	operator Expr*(){ return this; }
};

template<class T>
inline T* expr_cast(Expr* p){
	if(p && T::TYPE==p->type){
		return (T*)p;
	}
	return 0;
}

// 値を返さない文法要素
struct Stmt{
	enum{ TYPE = __LINE__ };
	
	int_t type;
	int_t line;

	Stmt(int_t type, int_t line)
		:type(type), line(line){}

	void *operator new(size_t size, RegionAlloc* a){ return  a->allocate(size); }
	void operator delete(void *, RegionAlloc*){}
	
	operator Stmt*(){ return this; }
};

template<class T>
inline T* stmt_cast(Stmt* p){
	if(p && T::TYPE==p->type){
		return (T*)p;
	}
	return 0;
}


struct ExprStmt : public Stmt{
	enum{ TYPE = __LINE__ };
	Expr* expr;
	ExprStmt(int_t line, Expr* expr)
		:Stmt(TYPE, line), expr(expr){}
};

struct PseudoVariableExpr : public Expr{
	enum{ TYPE = __LINE__ };
	int_t code;
	PseudoVariableExpr(int_t line, int_t code)
		:Expr(TYPE, line), code(code){}
};

struct CalleeExpr : public PseudoVariableExpr{
	enum{ TYPE = __LINE__ };
	CalleeExpr(int_t line)
		:PseudoVariableExpr(line, CODE_PUSH_CALLEE){ type = TYPE; }
};

struct ArgsExpr : public Expr{
	enum{ TYPE = __LINE__ };
	ArgsExpr(int_t line):Expr(TYPE, line){}
};

struct IntExpr : public Expr{
	enum{ TYPE = __LINE__ };
	int_t value;	
	IntExpr(int_t line, int_t value)
		:Expr(TYPE, line), value(value){}
};

struct FloatExpr : public Expr{
	enum{ TYPE = __LINE__ };
	float_t value;	
	FloatExpr(int_t line, float_t value)
		:Expr(TYPE, line), value(value){}
};

struct StringExpr : public Expr{ 
	enum{ TYPE = __LINE__ };
	int_t value;
	int_t kind;
	StringExpr(int_t line, int_t value, int_t kind = KIND_STRING)
		:Expr(TYPE, line), value(value), kind(kind){}
}; 

struct ArrayExpr : public Expr{
	enum{ TYPE = __LINE__ };
	TList<Expr*> values;
	ArrayExpr(int_t line):Expr(TYPE, line){}
};

struct MapExpr : public Expr{
	enum{ TYPE = __LINE__ };
	TPairList<Expr*, Expr*> values;
	MapExpr(int_t line):Expr(TYPE, line){}
};

struct BinExpr : public Expr{ 
	enum{ TYPE = __LINE__ };
	int_t code; 
	Expr* lhs; 
	Expr* rhs;	
	BinExpr(int_t line, int_t code, Expr* lhs = 0, Expr* rhs = 0)
		:Expr(TYPE, line), code(code), lhs(lhs), rhs(rhs){}
};

struct BinCompExpr : public BinExpr{ 
	enum{ TYPE = __LINE__ };
	BinCompExpr(int_t line, int_t code, Expr* lhs = 0, Expr* rhs = 0)
		:BinExpr(line, code, lhs, rhs){ type = TYPE; }
};

struct AndAndExpr : public Expr{
	enum{ TYPE = __LINE__ };
	Expr* lhs; 
	Expr* rhs;
	AndAndExpr(int_t line, Expr* lhs = 0, Expr* rhs = 0)
		:Expr(TYPE, line), lhs(lhs), rhs(rhs){}
};

struct OrOrExpr : public Expr{
	enum{ TYPE = __LINE__ };
	Expr* lhs; 
	Expr* rhs;
	OrOrExpr(int_t line, Expr* lhs = 0, Expr* rhs = 0)
		:Expr(TYPE, line), lhs(lhs), rhs(rhs){}
};

struct UnaExpr : public Expr{ 
	enum{ TYPE = __LINE__ };
	int_t code;
	Expr* expr;
	UnaExpr(int_t line, int_t code = 0, Expr* expr = 0)
		:Expr(TYPE, line), code(code), expr(expr){}
};

struct UnaStmt : public Stmt{ 
	enum{ TYPE = __LINE__ };
	int_t code;
	Expr* expr;
	UnaStmt(int_t line, int_t code = 0, Expr* expr = 0)
		:Stmt(TYPE, line), code(code), expr(expr){}
};

struct TerExpr : public Expr{ 
	enum{ TYPE = __LINE__ };
	Expr* first;
	Expr* second;
	Expr* third;
	TerExpr(int_t line, Expr* first = 0)
		:Expr(TYPE, line), first(first), second(0), third(0){}
};

struct ReturnStmt : public Stmt{
	enum{ TYPE = __LINE__ };
	TList<Expr*> exprs;
	ReturnStmt(int_t line)
		:Stmt(TYPE, line){}
};

struct YieldStmt : public Stmt{
	enum{ TYPE = __LINE__ };
	TList<Expr*> exprs;
	YieldStmt(int_t line)
		:Stmt(TYPE, line){}
};

struct AssertStmt : public Stmt{
	enum{ TYPE = __LINE__ };
	TList<Expr*> exprs;
	AssertStmt(int_t line)
		:Stmt(TYPE, line){}
};

struct OnceExpr : public Expr{
	enum{ TYPE = __LINE__ };
	Expr* expr;
	OnceExpr(int_t line, Expr* expr = 0)
		:Expr(TYPE, line), expr(expr){}
};

struct TryStmt : public Stmt{ 
	enum{ TYPE = __LINE__ };
	Stmt* try_stmt; 
	TList<int_t> catch_vars;
	bool on_heap;
	Stmt* catch_stmt; 
	Stmt* finally_stmt;	
	TryStmt(int_t line, Stmt* try_stmt = 0, Stmt* catch_stmt = 0, Stmt* finally_stmt = 0)
		:Stmt(TYPE, line), try_stmt(try_stmt), catch_stmt(catch_stmt), finally_stmt(finally_stmt){}
};

struct IfStmt : public Stmt{
	enum{ TYPE = __LINE__ };
	Expr* cond_expr;
	Stmt* body_stmt;
	Stmt* else_stmt;
	IfStmt(int_t line, Expr* cond_expr = 0, Stmt* body_stmt = 0, Stmt* else_stmt = 0)
		:Stmt(TYPE, line), cond_expr(cond_expr), body_stmt(body_stmt), else_stmt(else_stmt){}
};

struct WhileStmt : public Stmt{
	enum{ TYPE = __LINE__ };
	int_t label;
	Expr* cond_expr; 
	Stmt* body_stmt; 
	Stmt* next_stmt;
	Stmt* else_stmt;
	Stmt* nobreak_stmt;
	WhileStmt(int_t line, Expr* cond_expr = 0, Stmt* body_stmt = 0, Stmt* else_stmt = 0, Stmt* nobreak_stmt = 0)
		:Stmt(TYPE, line), label(0), cond_expr(cond_expr), body_stmt(body_stmt), else_stmt(else_stmt), nobreak_stmt(nobreak_stmt), next_stmt(0){}
};

struct FunExpr : public Expr{
	enum{ TYPE = __LINE__ };
	int_t kind;
	bool have_args;
	Stmt* stmt;
	TList<int_t> vars;
	bool on_heap;
	TPairList<int_t, Expr*> params;
	FunExpr(int_t line, int_t kind, Stmt* stmt = 0)
		:Expr(TYPE, line), kind(kind), stmt(stmt), have_args(false){}
};

struct MultipleAssignStmt : public Stmt{
	enum{ TYPE = __LINE__ };
	bool define;
	bool discard;
	TList<Expr*> lhs;
	TList<Expr*> rhs;
	MultipleAssignStmt(int_t line)
		:Stmt(TYPE, line), define(false), discard(false){}
};

struct IncStmt : public Stmt{ 
	enum{ TYPE = __LINE__ };
	int_t code;
	Expr* lhs;
	IncStmt(int_t line, int_t code, Expr* lhs = 0)
		:Stmt(TYPE, line), lhs(lhs), code(code){}
};
	
struct InstanceVariableExpr : public Expr{
	enum{ TYPE = __LINE__ };
	int_t var; 
	InstanceVariableExpr(int_t line, int_t var = 0)
		:Expr(TYPE, line), var(var){}
};

struct MemberExpr : public Expr{
	enum{ TYPE = __LINE__ };
	Expr* lhs;
	int_t var;
	Expr* pvar;
	bool if_defined;
	MemberExpr(int_t line, Expr* lhs = 0, int_t var = 0)
		:Expr(TYPE, line), lhs(lhs), var(var), pvar(0), if_defined(false){}
};

struct CallExpr : public Expr{ 
	enum{ TYPE = __LINE__ };
	Expr* expr; 
	bool tail;
	bool discard;
	TList<Expr*> ordered; 
	TPairList<int_t, Expr*> named;
	CallExpr(int_t line, Expr* expr = 0)
		:Expr(TYPE, line), expr(expr), tail(false), discard(false){}
};

struct SendExpr : public Expr{
	enum{ TYPE = __LINE__ };
	Expr* lhs;
	int_t var;
	Expr* pvar;
	bool tail;
	bool if_defined;
	bool discard;
	SendExpr(int_t line, Expr* lhs = 0, int_t var = 0)
		:Expr(TYPE, line), lhs(lhs), var(var), pvar(0), tail(false), discard(false), if_defined(false){}
};

struct AssignStmt : public Stmt{
	enum{ TYPE = __LINE__ };
	Expr* lhs;
	Expr* rhs;
	AssignStmt(int_t line, Expr* lhs = 0, Expr* rhs = 0)
		:Stmt(TYPE, line), lhs(lhs), rhs(rhs){}
};
	
struct OpAssignStmt : public Stmt{
	enum{ TYPE = __LINE__ };
	int_t code;
	Expr* lhs;
	Expr* rhs;
	OpAssignStmt(int_t line, int_t code, Expr* lhs = 0, Expr* rhs = 0)
		:Stmt(TYPE, line), code(code), lhs(lhs), rhs(rhs){}
};

struct LocalExpr : public Expr{
	enum{ TYPE = __LINE__ };
	int_t var; 
	LocalExpr(int_t line, int_t var = 0)
		:Expr(TYPE, line), var(var){}
};

struct PushStmt : public Stmt{
	enum{ TYPE = __LINE__ };
	Expr* expr;
	PushStmt(int_t line, Expr* expr = 0)
		:Stmt(TYPE, line), expr(expr){}
};

struct PopExpr : public Expr{
	enum{ TYPE = __LINE__ }; 
	PopExpr(int_t line)
		:Expr(TYPE, line){}
};

struct DefineStmt : public Stmt{
	enum{ TYPE = __LINE__ };
	Expr* lhs; 
	Expr* rhs;
	DefineStmt(int_t line, Expr* lhs, Expr* rhs)
		:Stmt(TYPE, line), lhs(lhs), rhs(rhs){}
};

struct AtExpr : public Expr{
	enum{ TYPE = __LINE__ };
	Expr* lhs;
	Expr* index;
	AtExpr(int_t line, Expr* lhs = 0, Expr* index = 0)
		:Expr(TYPE, line), lhs(lhs), index(index){}
};
	
struct BreakStmt : public Stmt{
	enum{ TYPE = __LINE__ };
	int_t var; 
	BreakStmt(int_t line, int_t var = 0)
		:Stmt(TYPE, line), var(var){}
};

struct ContinueStmt : public Stmt{
	enum{ TYPE = __LINE__ };
	int_t var; 
	ContinueStmt(int_t line, int_t var = 0)
		:Stmt(TYPE, line), var(var){}
};

struct BlockStmt : public Stmt{
	enum{ TYPE = __LINE__ };
	TList<int_t> vars;
	bool on_heap;
	TList<Stmt*> stmts;
	BlockStmt(int_t line)
		:Stmt(TYPE, line){}
};

struct FrameExpr : public Expr{
	enum{ TYPE = __LINE__ };
	TList<int_t> vars;
	bool on_heap;
	TList<Stmt*> stmts;
	int_t kind;
	FrameExpr(int_t line, int_t kind)
		:Expr(TYPE, line), kind(kind){}
};

struct ClassExpr : public FrameExpr{
	enum{ TYPE = __LINE__ };
	TPairList<int_t, Expr*> inst_vars;
	TList<Expr*> mixins;
//-
	int_t frame_number;
//-
	ClassExpr(int_t line)
		:FrameExpr(line, KIND_CLASS), frame_number(0){ type = TYPE; }
};

struct TopLevelStmt : public Stmt{
	enum{ TYPE = __LINE__ };
	TList<int_t> vars;
	bool on_heap;
	TList<Stmt*> stmts;
	Expr* export_expr;
	Stmt* unittest_stmt;
	TopLevelStmt(int_t line)
		:Stmt(TYPE, line), export_expr(0), unittest_stmt(0){}
};

struct SetAccessibilityStmt : public Stmt{
	enum{ TYPE = __LINE__ };
	int_t var;
	int_t kind;
	SetAccessibilityStmt(int_t line)
		:Stmt(TYPE, line), var(0), kind(0){}
};


class ExprBuilder{
public:

	PseudoVariableExpr* pseudo(int_t code);
	IntExpr* int_(int_t value){
		return new(alloc) IntExpr(line(), value);
	}
	FloatExpr* float_(float_t value){
		return new(alloc) FloatExpr(line(), value);
	}
	ArgsExpr* args(){
		return new(alloc) ArgsExpr(line());
	}
	OnceExpr* once(Expr* expr){
		return new(alloc) OnceExpr(line(), expr);
	}
	CalleeExpr* callee(){
		return new(alloc) CalleeExpr(line());
	}
	AndAndExpr* andand(Expr* lhs, Expr* rhs){
		return new(alloc) AndAndExpr(line(), lhs, rhs);
	}
	TerExpr* ter(Expr* cond, Expr* true_expr, Expr* false_expr){
		TerExpr* ret = new(alloc) TerExpr(line(), cond);
		ret->second = true_expr;
		ret->third = false_expr;
		return ret;
	}
	OrOrExpr* oror(Expr* lhs, Expr* rhs){
		return new(alloc) OrOrExpr(line(), lhs, rhs);
	}
	PushStmt* push(Expr* expr){
		return new(alloc) PushStmt(line(), expr);
	}
	PopExpr* pop(){
		return new(alloc) PopExpr(line());
	}
	IncStmt* inc(int_t code, Expr* expr){
		return new(alloc) IncStmt(line(), code, expr);	
	}
	StringExpr* string(int_t n, int_t kind = KIND_STRING);
	UnaExpr* una(int_t code, Expr* term);
	BinExpr* bin(int_t code, Expr* lhs, Expr* rhs);
	BinCompExpr* bin_comp(int_t code, Expr* lhs, Expr* rhs);
	OpAssignStmt* op_assign(int_t code, Expr* lhs, Expr* rhs);
	AtExpr* at(Expr* lhs, Expr* rhs);
	LocalExpr* local(int_t var);
	InstanceVariableExpr* instance_variable(int_t var);
	DefineStmt* define(Expr* lhs, Expr* rhs);
	AssignStmt* assign(Expr* lhs, Expr* rhs);
	CallExpr* call(Expr* lhs, Expr* a1 = 0, Expr* a2 = 0);
	MemberExpr* member(Expr* lhs, int_t var);
	MemberExpr* member_q(Expr* lhs, int_t var);
	MemberExpr* member(Expr* lhs, Expr* var);
	MemberExpr* member_q(Expr* lhs, Expr* var);
	SendExpr* send(Expr* lhs, int_t var);
	SendExpr* send_q(Expr* lhs, int_t var);
	SendExpr* send(Expr* lhs, Expr* var);
	SendExpr* send_q(Expr* lhs, Expr* var);
	ExprStmt* e2s(Expr* expr);
	ReturnStmt* return_(Expr* e1 = 0, Expr* e2 = 0);
	AssertStmt* assert_(Expr* e1 = 0, Expr* e2 = 0);
	SetAccessibilityStmt* set_accessibility(int_t var, int_t kind);
	ContinueStmt* continue_(int_t name){
		return new(alloc) ContinueStmt(line(), name);
	}
	BreakStmt* break_(int_t name){
		return new(alloc) BreakStmt(line(), name);
	}

	void scope_push(TList<int_t>* list, bool* on_heap, bool set_name_flag);
	void scope_carry_on_heap_flag();
	void scope_set_on_heap_flag(int_t i);
	void scope_pop();
	void register_variable(int_t var);

	void block_begin();
	void block_add(Stmt* stmt);
	BlockStmt* block_end();
	TList<Stmt*>* block_stmts(){
		return &block_stack.top()->stmts;
	}

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
	void while_nobreak(Stmt* stmt);
	Stmt* while_end();

	void if_begin(int_t var, Expr* expr);
	void if_body(Stmt* stmt);
	void if_else(Stmt* stmt);
	Stmt* if_end();

	void fun_begin(int_t kind);
	void fun_param(int_t name, Expr* default_value = 0);
	void fun_body(Stmt* stmt);
	void fun_body_begin();
	void fun_body_add(Stmt* stmt);
	void fun_body_end();
	FunExpr* fun_end();
	void fun_have_args();

	void toplevel_begin(){
		toplevel_stack.push(new(alloc) TopLevelStmt(line()));
		scope_push(&toplevel_stack.top()->vars, &toplevel_stack.top()->on_heap, true);
		scope_set_on_heap_flag(0);
	}
	void toplevel_add(Stmt* stmt){
		toplevel_stack.top()->stmts.push_back(stmt, alloc);
	}
	void toplevel_export(int_t name, Expr* expr){
			if(toplevel_stack.top()->export_expr){
				common->error(line(), Xt("Xtal Compile Error 1019"));
			}else{
				int_t export_id = register_ident(ID("__EXPORT__"));
				
				if(name){
					toplevel_add(define(local(name), expr));
					toplevel_add(define(local(export_id), local(name)));
				}else{
					toplevel_add(define(local(export_id), expr));
				}
				toplevel_stack.top()->export_expr = local(export_id);
			}
	}
	TopLevelStmt* toplevel_end(){
		scope_pop();
		return toplevel_stack.pop();
	}

	void class_begin(){
		class_stack.push(new(alloc) ClassExpr(line()));
		class_stack.top()->kind = KIND_CLASS;

		scope_push(&class_stack.top()->vars, &class_stack.top()->on_heap, true);
		scope_set_on_heap_flag(0);
	}
	void class_define_instance_variable(int_t name, Expr* expr){
		for(TPairList<int_t, Expr*>::Node* p=class_stack.top()->inst_vars.head; p; p=p->next){
			if(p->key==name){
				common->error(line(), Xt("Xtal Compile Error 1024")(Named("name", name)));
				break;
			}
		}

		class_stack.top()->inst_vars.push_back(name, expr, alloc);
	}
	void class_add(Stmt* stmt){
		class_stack.top()->stmts.push_back(stmt, alloc);
	}
	ClassExpr* class_end(){
		Stmt* s;
		fun_begin(KIND_METHOD);
			block_begin();
				for(TPairList<int_t, Expr*>::Node* p=class_stack.top()->inst_vars.head; p; p=p->next){
					block_add(assign(instance_variable(p->key), p->value));
				}
			s = block_end();
			fun_body(s);
		class_stack.top()->stmts.push_front(define(local(register_ident("__INITIALIZE__")), fun_end()), alloc);

		scope_pop();
		return class_stack.pop();
	}
	TList<Expr*>* class_mixins(){
		return &class_stack.top()->mixins;
	}

	void call_begin(Expr* expr){
		call_stack.push(new(alloc) CallExpr(line(), expr));
	}
	void call_arg(Expr* expr){
		call_stack.top()->ordered.push_back(expr, alloc);
	}
	void call_arg(int_t name, Expr* expr){
		call_stack.top()->named.push_back(name, expr, alloc);
	}
	CallExpr* call_end(){
		if(call_stack.top()->ordered.tail && !call_stack.top()->ordered.tail->value){
			call_stack.top()->ordered.pop_back();
		}
		return call_stack.pop();
	}

	void return_begin(){
		return_stack.push(new(alloc) ReturnStmt(line()));
	}
	void return_add(Expr* expr){
		return_exprs()->push_back(expr, alloc);
	}
	ReturnStmt* return_end(){
		return return_stack.pop();
	}
	TList<Expr*>* return_exprs(){
		return &return_stack.top()->exprs;
	}
	
	void yield_begin(){
		yield_stack.push(new(alloc) YieldStmt(line()));
	}
	void yield_add(Expr* expr){
		yield_exprs()->push_back(expr, alloc);
	}
	YieldStmt* yield_end(){
		return yield_stack.pop();
	}
	TList<Expr*>* yield_exprs(){
		return &yield_stack.top()->exprs;
	}

	void massign_begin(){
		massign_stack.push(new(alloc) MultipleAssignStmt(line()));
	}
	void massign_lhs(Expr* expr){
		massign_lhs_exprs()->push_back(expr, alloc);
	}
	void massign_rhs(Expr* expr){
		massign_rhs_exprs()->push_back(expr, alloc);
	}
	MultipleAssignStmt* massign_end(){
		return massign_stack.pop();
	}
	TList<Expr*>* massign_lhs_exprs(){
		return &massign_stack.top()->lhs;
	}
	TList<Expr*>* massign_rhs_exprs(){
		return &massign_stack.top()->rhs;
	}
	void massign_define(bool b){
		massign_stack.top()->define = b;
	}
	void massign_discard(bool b){
		massign_stack.top()->discard = b;
	}

	void ter_begin(Expr* cond){
		ter_stack.push(new(alloc) TerExpr(line(), cond));
	}
	void ter_true(Expr* expr){
		ter_stack.top()->second = expr;
	}
	void ter_false(Expr* expr){
		ter_stack.top()->third = expr;
	}
	TerExpr* ter_end(){
		return ter_stack.pop();
	}

	void array_begin(){
		array_stack.push(new(alloc) ArrayExpr(line()));
	}
	void array_add(Expr* expr){
		array_stack.top()->values.push_back(expr, alloc);
	}
	ArrayExpr* array_end(){
		return array_stack.pop();
	}

	void map_begin(){
		map_stack.push(new(alloc) MapExpr(line()));
	}
	void map_add(Expr* key, Expr* value){
		map_stack.top()->values.push_back(key, value, alloc);
	}
	MapExpr* map_end(){
		return map_stack.pop();
	}

	void init(LPCCommon* com, RegionAlloc* all);

	int_t line(){
		return common->line;
	}

	int_t register_ident(const ID& ident){ return common->register_ident(ident); }
	int_t register_value(const Any& v){ return common->register_value(v); }
	int_t append_ident(const ID& ident){ return common->append_ident(ident); }
	int_t append_value(const Any& v){ return common->append_value(v); }

	struct VarInfo{
		TList<int_t>* variables;
		bool* on_heap_flag;
		bool set_name_flag;
	};
	
	PODStack<VarInfo> var_info_stack;
	
	LPCCommon* common;
	RegionAlloc* alloc;
	
	PStack<BlockStmt*> block_stack;
	PStack<TryStmt*> try_stack;
	PStack<WhileStmt*> while_stack;
	PStack<IfStmt*> if_stack;
	PStack<FunExpr*> fun_stack;
	PStack<TopLevelStmt*> toplevel_stack;
	PStack<CallExpr*> call_stack;
	PStack<MultipleAssignStmt*> massign_stack;
	PStack<ClassExpr*> class_stack;
	PStack<ReturnStmt*> return_stack;
	PStack<YieldStmt*> yield_stack;
	PStack<TerExpr*> ter_stack;
	PStack<ArrayExpr*> array_stack;
	PStack<MapExpr*> map_stack;
};

}

#endif
