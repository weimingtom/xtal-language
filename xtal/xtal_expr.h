
#pragma once

#ifndef XTAL_NO_PARSER

#include "xtal_lexer.h"

namespace xtal{

struct RegionAllocObject{
	void* operator new(size_t size, RegionAlloc* r);
	void operator delete(void *, RegionAlloc*);
};


/*

巡回サンプル

for(TList<Any>::Node* p = values.head; p; p = p->next){
	//p->value;
}

*/
template <class T>
struct TList{

	struct Node : public RegionAllocObject{
		T value;

		Node* next;
		Node* prev;

		Node(const T &val = T())
			:value(val), next(0), prev(0){}
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

	struct Node : public RegionAllocObject{
		Key key;
		T value;

		Node* next;
		Node* prev;

		Node(const Key &key = Key(), const T &val = T())
			:key(key), value(val), next(0), prev(0){}
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
struct Expr : public RegionAllocObject{	
	enum{ TYPE = __LINE__ };
	
	int_t type;
	int_t line;
	
	Expr(int_t type, int_t line)
		:type(type), line(line){}

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
struct Stmt : public RegionAllocObject{
	enum{ TYPE = __LINE__ };
	
	int_t type;
	int_t line;

	Stmt(int_t type, int_t line)
		:type(type), line(line){}
	
	operator Stmt*(){ return this; }
};

template<class T>
inline T* stmt_cast(Stmt* p){
	if(p && T::TYPE==p->type){
		return (T*)p;
	}
	return 0;
}


/*

struct XXXExpr{
	int_t expr_type;
	int_t line;
};

typedef SmartPtr<XXXExpr> ExprPtr;

struct XXXExprPushNull : public XXXExpr{

};

struct KeyValuePair{
	AnyPtr key;
	AnyPtr value;
};

struct XXXExprMakeArray : public XXXExpr{
	ArrayPtr values;
};

struct XXXExprMakeMap : public XXXExpr{
	ArrayPtr values;
};

struct XXXExprIf : public XXXExpr{
	ExprPtr cond_expr;
	ExprPtr body_expr;
	ExprPtr else_expr;
};

#define XTAL_DEF_EXPR_0(N, ExprName, SuperExpr) \
	struct Expr##ExprName : public Expr##SuperExpr{\
		enum{ \
			NUMBER = N\
		};\
		Expr##ExprName(){\
			expr_type = N;\
		}\
	}

#define XTAL_DEF_EXPR_1(N, ExprName, SuperExpr, MemberType1, MemberName1) \
	struct Expr##ExprName : public Expr##SuperExpr{\
		enum{ \
			NUMBER = N\
		};\
		MemberType1 MemberName1;\
		Expr##ExprName(){\
			expr_type = N;\
		}\
		Expr##ExprName(const MemberType1& m1)\
			:MemberName1(m1){\
			expr_type = N;\
		}\
	}

#define XTAL_DEF_EXPR_2(N, ExprName, SuperExpr, MemberType1, MemberName1, MemberType2, MemberName2) \
	struct Expr##ExprName : public Expr##SuperExpr{\
		enum{ \
			NUMBER = N\
		};\
		MemberType1 MemberName1;\
		MemberType2 MemberName2;\
		Expr##ExprName(){\
			expr_type = N;\
		}\
		Expr##ExprName(const MemberType1& m1, const MemberType2& m2)\
			:MemberName1(m1), MemberName2(m2){\
			expr_type = N;\
		}\
	}
	
#define XTAL_DEF_EXPR_3(N, ExprName, SuperExpr, MemberType1, MemberName1, MemberType2, MemberName2, MemberType3, MemberName3) \
	struct Expr##ExprName : public Expr##SuperExpr{\
		enum{ \
			NUMBER = N\
		};\
		MemberType1 MemberName1;\
		MemberType2 MemberName2;\
		MemberType3 MemberName3;\
		Expr##ExprName(){\
			expr_type = N;\
		}\
		Expr##ExprName(const MemberType1& m1, const MemberType2& m2, const MemberType3& m3)\
			:MemberName1(m1), MemberName2(m2), MemberName3(m3){\
			expr_type = N;\
		}\
	}
	
#define XTAL_DEF_EXPR_4(N, ExprName, SuperExpr, MemberType1, MemberName1, MemberType2, MemberName2, MemberType3, MemberName3, MemberType4, MemberName4) \
struct Expr##ExprName : public Expr##SuperExpr{\
		enum{ \
			NUMBER = N\
		};\
		MemberType1 MemberName1;\
		MemberType2 MemberName2;\
		MemberType3 MemberName3;\
		MemberType4 MemberName4;\
		Expr##ExprName(){\
			expr_type = N;\
		}\
		Expr##ExprName(const MemberType1& m1, const MemberType2& m2, const MemberType3& m3, const MemberType4& m4)\
			:MemberName1(m1), MemberName2(m2), MemberName3(m3), MemberName4(m4){\
			expr_type = N;\
		}\
	}

XTAL_DEF_EXPR_0(1, PushNull, XXXExpr);
XTAL_DEF_EXPR_0(1, PushNop, XXXExpr);
XTAL_DEF_EXPR_0(1, PushTrue, XXXExpr);
XTAL_DEF_EXPR_0(1, PushFalse, XXXExpr);
XTAL_DEF_EXPR_0(1, PushInt, XXXExpr);
XTAL_DEF_EXPR_0(1, PushFloat, XXXExpr);
XTAL_DEF_EXPR_0(1, PushString, XXXExpr);
XTAL_DEF_EXPR_0(1, PushCallee, XXXExpr);
XTAL_DEF_EXPR_0(1, PushArgs, XXXExpr);

XTAL_DEF_EXPR_1(1, Una, XXXExpr, ExprPtr, expr);
XTAL_DEF_EXPR_2(5, Bin, XXXExpr, ExprPtr, lhs, ExprPtr, rhs);

XTAL_DEF_EXPR_0(1, AndAnd, Bin);
XTAL_DEF_EXPR_0(1, OrOr, Bin);

XTAL_DEF_EXPR_1(5, MakeArray, XXXExpr, ArrayPtr, exprs);
XTAL_DEF_EXPR_1(5, MakeMap, XXXExpr, ArrayPtr, exprs);
XTAL_DEF_EXPR_3(5, If, XXXExpr, ExprPtr, cond, ExprPtr, block, ExprPtr, else_block);
XTAL_DEF_EXPR_3(5, While, XXXExpr, ExprPtr, cond, ExprPtr, block, ExprPtr, nobreak_block, ExprPtr, else_block);

XTAL_DEF_EXPR_1(0, LocalVariable, InternedStringPtr, name);
XTAL_DEF_EXPR_1(0, InstanceVariable, InternedStringPtr, name);
XTAL_DEF_EXPR_1(0, Member, ExprPtr, lhs, InternedStringPtr, name, ExprPtr name_expr);
XTAL_DEF_EXPR_2(5, Define, ExprPtr, lhs, ExprPtr, rhs);

SmartPtr<Block> block = xnew<Block>();
block->block->push_back();
xnew<ExprIf>(xnew<ExprPushNull>(), xnew<Block>(), xnew<Block>());

*/


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
		:PseudoVariableExpr(line, InstPushCallee::NUMBER){ type = TYPE; }
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

struct Var{

	// 変数名
	int_t name;

	// 代入が存在しないか？
	bool constant;

	// 初期値の式
	Expr* init;

	int_t accessibility;
		
	Expr* ns;
};

struct Vars{
	TList<Var> vars;
	bool on_heap;
};

struct TryStmt : public Stmt{ 
	enum{ TYPE = __LINE__ };
	Stmt* try_stmt; 
	Vars catch_vars;
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
		:Stmt(TYPE, line), label(0), cond_expr(cond_expr), body_stmt(body_stmt), next_stmt(0), else_stmt(else_stmt), nobreak_stmt(nobreak_stmt){}
};

struct FunExpr : public Expr{
	enum{ TYPE = __LINE__ };
	int_t kind;
	Stmt* stmt;
	Vars vars;
	bool have_args;
	TPairList<int_t, Expr*> params;
	FunExpr(int_t line, int_t kind, Stmt* stmt = 0)
		:Expr(TYPE, line), kind(kind), stmt(stmt), have_args(false){}
};

struct MultipleAssignStmt : public Stmt{
	enum{ TYPE = __LINE__ };
	bool define;
	TList<Expr*> lhs;
	TList<Expr*> rhs;
	MultipleAssignStmt(int_t line)
		:Stmt(TYPE, line), define(false){}
};

struct IncStmt : public Stmt{ 
	enum{ TYPE = __LINE__ };
	int_t code;
	Expr* lhs;
	IncStmt(int_t line, int_t code, Expr* lhs = 0)
		:Stmt(TYPE, line), code(code), lhs(lhs){}
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
	TList<Expr*> ordered; 
	TPairList<int_t, Expr*> named;
	CallExpr(int_t line, Expr* expr = 0)
		:Expr(TYPE, line), expr(expr){}
};

struct SendExpr : public Expr{
	enum{ TYPE = __LINE__ };
	Expr* lhs;
	int_t var;
	Expr* pvar;
	bool if_defined;
	SendExpr(int_t line, Expr* lhs = 0, int_t var = 0)
		:Expr(TYPE, line), lhs(lhs), var(var), pvar(0), if_defined(false){}
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

struct DefineClassMemberStmt : public Stmt{
	enum{ TYPE = __LINE__ };
	int_t var;
	int_t asscessibility;
	Expr* expr;
	Expr* ns;
	DefineClassMemberStmt(int_t line)
		:Stmt(TYPE, line){}
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
	Vars vars;
	TList<Stmt*> stmts;
	BlockStmt(int_t line)
		:Stmt(TYPE, line){}
};

struct ClassExpr : public Expr{
	enum{ TYPE = __LINE__ };
	TPairList<int_t, Expr*> inst_vars;
	TList<Expr*> mixins;
	int_t frame_number;
	int_t kind;
	Vars vars;

	ClassExpr(int_t line)
		:Expr(TYPE, line), frame_number(0){}
};

struct TopLevelStmt : public Stmt{
	enum{ TYPE = __LINE__ };
	Vars vars;
	TList<Stmt*> stmts;
	Expr* export_expr;
	Stmt* unittest_stmt;
	TopLevelStmt(int_t line)
		:Stmt(TYPE, line), export_expr(0), unittest_stmt(0){}
};

class ExprBuilder{
public:

	PseudoVariableExpr* pseudo(int_t code);
	IntExpr* int_(int_t value);
	FloatExpr* float_(float_t value);
	ArgsExpr* args();
	OnceExpr* once(Expr* expr);
	CalleeExpr* callee();
	AndAndExpr* andand(Expr* lhs, Expr* rhs);
	TerExpr* ter(Expr* cond, Expr* true_expr, Expr* false_expr);
	OrOrExpr* oror(Expr* lhs, Expr* rhs);
	PushStmt* push(Expr* expr);
	PopExpr* pop();
	IncStmt* inc(int_t code, Expr* expr);
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
	ContinueStmt* continue_(int_t name);
	BreakStmt* break_(int_t name);

	void scope_push(Vars* vars);
	void scope_carry_on_heap_flag();
	void scope_set_on_heap_flag(int_t i);
	void scope_pop();
	void register_variable(int_t var);

	void block_begin();
	void block_add(Stmt* stmt);
	BlockStmt* block_end();
	TList<Stmt*>* block_stmts();

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
	void fun_have_args(bool v);

	void toplevel_begin();
	void toplevel_add(Stmt* stmt);
	void toplevel_export(int_t name, Expr* expr);
	TopLevelStmt* toplevel_end();
	void class_begin(int_t kind);
	void class_define_instance_variable(int_t name, Expr* expr);
	void class_define_member(int_t var, int_t accessibility, Expr* ns, Expr* rhs);
	ClassExpr* class_end();
	TList<Expr*>* class_mixins();

	void call_begin(Expr* expr);
	void call_arg(Expr* expr);
	void call_arg(int_t name, Expr* expr);
	CallExpr* call_end();

	void return_begin();
	void return_add(Expr* expr);
	ReturnStmt* return_end();
	TList<Expr*>* return_exprs();
	
	void yield_begin();
	void yield_add(Expr* expr);
	YieldStmt* yield_end();
	TList<Expr*>* yield_exprs();

	void massign_begin();
	void massign_lhs(Expr* expr);
	void massign_rhs(Expr* expr);
	MultipleAssignStmt* massign_end();
	TList<Expr*>* massign_lhs_exprs();
	TList<Expr*>* massign_rhs_exprs();
	void massign_define(bool b);
	void ter_begin(Expr* cond);
	void ter_true(Expr* expr);
	void ter_false(Expr* expr);
	TerExpr* ter_end();
	void array_begin();
	void array_add(Expr* expr);
	ArrayExpr* array_end();
	void map_begin();
	void map_add(Expr* key, Expr* value);
	MapExpr* map_end();

	void init(LPCCommon* com, RegionAlloc* all);

	int_t line(){
		return common->line;
	}

	int_t register_ident(const InternedStringPtr& ident){ return common->register_ident(ident); }
	int_t register_value(const AnyPtr& v){ return common->register_value(v); }
	int_t append_ident(const InternedStringPtr& ident){ return common->append_ident(ident); }
	int_t append_value(const AnyPtr& v){ return common->append_value(v); }

	LPCCommon* common;
	RegionAlloc* alloc;
	
	PStack<Vars*> vars_stack;
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
