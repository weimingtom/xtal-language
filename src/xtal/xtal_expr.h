
#pragma once

#ifndef XTAL_NO_PARSER

namespace xtal{

enum ExprType{

	EXPR_LIST,

	EXPR_NULL,
	EXPR_UNDEFINED,
	EXPR_TRUE,
	EXPR_FALSE,
	EXPR_CALLEE,
	EXPR_ARGS,
	EXPR_THIS,
	EXPR_CURRENT_CONTEXT,
	EXPR_INT,
	EXPR_FLOAT,
	EXPR_STRING,
	EXPR_ARRAY,
	EXPR_MAP,
	EXPR_MULTI_VALUE,

	EXPR_ADD,
	EXPR_SUB,
	EXPR_CAT,
	EXPR_MUL,
	EXPR_DIV,
	EXPR_MOD,
	EXPR_AND,
	EXPR_OR,
	EXPR_XOR,
	EXPR_SHL,
	EXPR_SHR,
	EXPR_USHR,

	EXPR_ADD_ASSIGN,
	EXPR_SUB_ASSIGN,
	EXPR_CAT_ASSIGN,
	EXPR_MUL_ASSIGN,
	EXPR_DIV_ASSIGN,
	EXPR_MOD_ASSIGN,
	EXPR_AND_ASSIGN,
	EXPR_OR_ASSIGN,
	EXPR_XOR_ASSIGN,
	EXPR_SHL_ASSIGN,
	EXPR_SHR_ASSIGN,
	EXPR_USHR_ASSIGN,

	EXPR_EQ,
	EXPR_NE,
	EXPR_LT,
	EXPR_LE,
	EXPR_GT,
	EXPR_GE,
	EXPR_RAWEQ,
	EXPR_RAWNE,
	EXPR_IN,
	EXPR_NIN,
	EXPR_IS,
	EXPR_NIS,

	EXPR_ANDAND,
	EXPR_OROR,

	EXPR_INC,
	EXPR_DEC,
	EXPR_POS,
	EXPR_NEG,
	EXPR_COM,
	EXPR_NOT,

	EXPR_RANGE,
	EXPR_RETURN,
	EXPR_YIELD,
	EXPR_ASSERT,
	EXPR_ONCE,
	EXPR_THROW,
	EXPR_Q,
	EXPR_TRY,
	EXPR_IF,
	EXPR_FOR,
	EXPR_FUN,
	EXPR_MASSIGN,
	EXPR_MDEFINE,
	EXPR_IVAR,
	EXPR_LVAR,
	EXPR_MEMBER,
	EXPR_MEMBER_Q,
	EXPR_SEND,
	EXPR_SEND_Q,
	EXPR_CALL,
	EXPR_ASSIGN,
	EXPR_DEFINE,
	EXPR_CDEFINE_MEMBER,
	EXPR_CDEFINE_IVAR,
	EXPR_AT,
	EXPR_BREAK,
	EXPR_CONTINUE,
	EXPR_BRACKET,
	EXPR_SCOPE,
	EXPR_CLASS,
	EXPR_SWITCH,
	EXPR_SWITCH_CASE,
	EXPR_SWITCH_DEFAULT,
	EXPR_TOPLEVEL,

	EXPR_MAX
};

#define XTAL_DEF_MEMBER(N, Type, Name) \
	CastResult<Type>::type Name(){ return as<Type>(at(N)); }\
	void set_##Name(const Type& v){ set_at(N, v); }


class Expr;
typedef SmartPtr<Expr> ExprPtr;

inline const ExprPtr& ep(const AnyPtr& a){
	return ptr_cast<Expr>(a);
}

class Expr : public Array{
	AnyPtr tag_;
	int_t lineno_;

public:

	Expr(const AnyPtr& tag=0, int_t lineno=0){
		tag_ = tag;
		lineno_ = lineno;
	}

	int_t tag(){
		return ivalue(tag_);
	}

	void set_tag(int_t tag){
		tag_ = tag;
	}
	
	int_t lineno(){
		return lineno_;
	}

	void set_lineno(int_t lineno){
		lineno_ = lineno;
	}

	const AnyPtr& at(uint_t i);

	void set_at(uint_t i, const AnyPtr& v);

	XTAL_DEF_MEMBER(0, ExprPtr, una_term);

	XTAL_DEF_MEMBER(0, ExprPtr, bin_lhs);
	XTAL_DEF_MEMBER(1, ExprPtr, bin_rhs);

	XTAL_DEF_MEMBER(0, ExprPtr, q_cond);
	XTAL_DEF_MEMBER(1, ExprPtr, q_true);
	XTAL_DEF_MEMBER(2, ExprPtr, q_false);

	XTAL_DEF_MEMBER(0, int_t, int_value);

	XTAL_DEF_MEMBER(0, float_t, float_value);

	XTAL_DEF_MEMBER(0, int_t, string_kind);
	XTAL_DEF_MEMBER(1, IDPtr, string_value);

	XTAL_DEF_MEMBER(0, int_t, range_kind);
	XTAL_DEF_MEMBER(1, ExprPtr, range_lhs);
	XTAL_DEF_MEMBER(2, ExprPtr, range_rhs);

	XTAL_DEF_MEMBER(0, ExprPtr, array_values);
	
	XTAL_DEF_MEMBER(0, ExprPtr, map_values);

	XTAL_DEF_MEMBER(0, ExprPtr, multi_value_exprs);

	XTAL_DEF_MEMBER(0, ExprPtr, return_exprs);
	
	XTAL_DEF_MEMBER(0, ExprPtr, yield_exprs);

	XTAL_DEF_MEMBER(0, ExprPtr, assert_exprs);

	XTAL_DEF_MEMBER(0, ExprPtr, throw_expr);

	XTAL_DEF_MEMBER(0, ExprPtr, try_body);
	XTAL_DEF_MEMBER(1, IDPtr, try_catch_var);
	XTAL_DEF_MEMBER(2, ExprPtr, try_catch);
	XTAL_DEF_MEMBER(3, ExprPtr, try_finally);

	XTAL_DEF_MEMBER(0, ExprPtr, if_cond);
	XTAL_DEF_MEMBER(1, ExprPtr, if_body);
	XTAL_DEF_MEMBER(2, ExprPtr, if_else);

	XTAL_DEF_MEMBER(0, IDPtr, for_label);
	XTAL_DEF_MEMBER(1, ExprPtr, for_init);
	XTAL_DEF_MEMBER(2, ExprPtr, for_cond);
	XTAL_DEF_MEMBER(3, ExprPtr, for_next);
	XTAL_DEF_MEMBER(4, ExprPtr, for_body);
	XTAL_DEF_MEMBER(5, ExprPtr, for_else);
	XTAL_DEF_MEMBER(6, ExprPtr, for_nobreak);

	XTAL_DEF_MEMBER(0, int_t, fun_kind);
	XTAL_DEF_MEMBER(1, ExprPtr, fun_params);
	XTAL_DEF_MEMBER(2, ExprPtr, fun_extendable_param);
	XTAL_DEF_MEMBER(3, ExprPtr, fun_body);

	XTAL_DEF_MEMBER(0, ExprPtr, massign_lhs_exprs);
	XTAL_DEF_MEMBER(1, ExprPtr, massign_rhs_exprs);

	XTAL_DEF_MEMBER(0, ExprPtr, mdefine_lhs_exprs);
	XTAL_DEF_MEMBER(1, ExprPtr, mdefine_rhs_exprs);

	XTAL_DEF_MEMBER(0, IDPtr, ivar_name);
	XTAL_DEF_MEMBER(0, IDPtr, lvar_name);

	XTAL_DEF_MEMBER(0, ExprPtr, member_term);
	XTAL_DEF_MEMBER(1, AnyPtr, member_name);
	XTAL_DEF_MEMBER(2, ExprPtr, member_ns);

	XTAL_DEF_MEMBER(0, ExprPtr, send_term);
	XTAL_DEF_MEMBER(1, AnyPtr, send_name);
	XTAL_DEF_MEMBER(2, ExprPtr, send_ns);

	XTAL_DEF_MEMBER(0, ExprPtr, call_term);
	XTAL_DEF_MEMBER(1, ExprPtr, call_args);
	XTAL_DEF_MEMBER(2, ExprPtr, call_extendable_arg);

	XTAL_DEF_MEMBER(0, AnyPtr, cdefine_member_accessibility);
	XTAL_DEF_MEMBER(1, IDPtr, cdefine_member_name);
	XTAL_DEF_MEMBER(2, ExprPtr, cdefine_member_ns);
	XTAL_DEF_MEMBER(3, ExprPtr, cdefine_member_term);

	XTAL_DEF_MEMBER(0, AnyPtr, cdefine_ivar_accessibility);
	XTAL_DEF_MEMBER(1, IDPtr, cdefine_ivar_name);
	XTAL_DEF_MEMBER(2, ExprPtr, cdefine_ivar_term);

	XTAL_DEF_MEMBER(0, IDPtr, break_label);

	XTAL_DEF_MEMBER(0, IDPtr, continue_label);

	XTAL_DEF_MEMBER(0, int_t, class_kind);
	XTAL_DEF_MEMBER(1, ExprPtr, class_mixins);
	XTAL_DEF_MEMBER(2, ExprPtr, class_stmts);

	XTAL_DEF_MEMBER(0, ExprPtr, switch_cond);
	XTAL_DEF_MEMBER(1, ExprPtr, switch_cases);

	XTAL_DEF_MEMBER(0, ExprPtr, scope_stmts);
	XTAL_DEF_MEMBER(0, ExprPtr, toplevel_stmts);
};

class ExprBuilder{
public:

	ExprBuilder(int_t lineno = 0)
		:lineno_(lineno), root_(xnew<Expr>()){}

	int_t lineno(){
		return lineno_;
	}

	void set_lineno(int_t ln){
		lineno_ = ln;
	}

	void splice(int_t tag, int_t num);

	struct State{
		int_t lineno;
		int_t pos;
	};

	State begin(){
		State state = {lineno_, root_->size()};
		return state;
	}

	void end(int_t tag, const State& state){
		splice(tag, root_->size()-state.pos);
	}

	const AnyPtr& back(){
		return root_->back();
	}

	void push(const AnyPtr& v){
		root_->push_back(v);
	}

	void insert(int_t n, const AnyPtr& v){
		root_->insert(root_->size()-n, v);
	}

	AnyPtr pop(){
		AnyPtr ret = root_->back();
		root_->pop_back();
		return ret;
	}

private:
	ExprPtr root_;
	ArrayPtr errors_;
	int_t lineno_;
};

}

#endif
