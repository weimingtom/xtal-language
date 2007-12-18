
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
	Type Name(){ return as<Type>(at(N)); }\
	ExprPtr set_##Name(const Type& v){ set_at(N, v);return from_this(this); }


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

	const AnyPtr& at(uint_t i){
		if(size()<=i)
			resize(i+1);
		return Array::at(i);
	}

	void set_at(uint_t i, const AnyPtr& v){
		if(size()<=i) 
			resize(i+1); 
		Array::set_at(i, v); 
	}

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

	XTAL_DEF_MEMBER(0, ExprPtr, member_term);
	XTAL_DEF_MEMBER(1, AnyPtr, member_name);
	XTAL_DEF_MEMBER(2, ExprPtr, member_ns);

	XTAL_DEF_MEMBER(0, ExprPtr, send_term);
	XTAL_DEF_MEMBER(1, AnyPtr, send_name);
	XTAL_DEF_MEMBER(2, ExprPtr, send_ns);

	XTAL_DEF_MEMBER(0, ExprPtr, call_term);
	XTAL_DEF_MEMBER(1, ExprPtr, call_args);
	XTAL_DEF_MEMBER(2, ExprPtr, call_extendable_arg);

	XTAL_DEF_MEMBER(0, IDPtr, lvar_name);

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


inline ExprPtr make_exprs(){ ExprPtr ret = xnew<Expr>(); return ret; }
inline ExprPtr make_exprs(const AnyPtr& v1){ ExprPtr ret = xnew<Expr>(); ret->push_back(v1); return ret; }
inline ExprPtr make_exprs(const AnyPtr& v1, const AnyPtr& v2){ ExprPtr ret = xnew<Expr>(); ret->push_back(v1); ret->push_back(v2); return ret; }

class ExprMaker{
public:

	ExprMaker(int_t lineno = 0)
		:lineno_(lineno){}

	int_t lineno(){
		return lineno_;
	}

	void set_lineno(int_t ln){
		lineno_ = ln;
	}

	ExprPtr return_(const ExprPtr& exprs){ return xnew<Expr>(EXPR_RETURN, lineno_)->set_return_exprs(exprs); }
	ExprPtr continue_(const IDPtr& label){ return xnew<Expr>(EXPR_CONTINUE, lineno_)->set_continue_label(label); }
	ExprPtr break_(const IDPtr& label){ return xnew<Expr>(EXPR_BREAK, lineno_)->set_break_label(label); }
	ExprPtr yield(const ExprPtr& exprs){ return xnew<Expr>(EXPR_YIELD, lineno_)->set_yield_exprs(exprs); }
	ExprPtr scope(const ExprPtr& stmts){ return xnew<Expr>(EXPR_SCOPE, lineno_)->set_scope_stmts(stmts); }
	ExprPtr toplevel(const ExprPtr& stmts){ return xnew<Expr>(EXPR_TOPLEVEL, lineno_)->set_toplevel_stmts(stmts); }

	ExprPtr cdefine_member(const AnyPtr& accessibility, const IDPtr& name, const ExprPtr& secondary_key, const ExprPtr& term){ return xnew<Expr>(EXPR_CDEFINE_MEMBER, lineno_)->set_cdefine_member_accessibility(accessibility)->set_cdefine_member_name(name)->set_cdefine_member_ns(secondary_key)->set_cdefine_member_term(term); }
	ExprPtr cdefine_ivar(const AnyPtr& accessibility, const IDPtr& name, const ExprPtr& term){ return xnew<Expr>(EXPR_CDEFINE_IVAR, lineno_)->set_cdefine_ivar_accessibility(accessibility)->set_cdefine_ivar_name(name)->set_cdefine_ivar_term(term); }

	ExprPtr fun(int_t kind, const ExprPtr& params, const ExprPtr& extendable_param, const ExprPtr& body){ return xnew<Expr>(EXPR_FUN, lineno_)->set_fun_kind(kind)->set_fun_params(params)->set_fun_extendable_param(extendable_param)->set_fun_body(body); }

	ExprPtr bin(ExprType expr_type, const ExprPtr& lhs, const ExprPtr& rhs){ return xnew<Expr>(expr_type, lineno_)->set_bin_lhs(lhs)->set_bin_rhs(rhs); }
	ExprPtr una(ExprType expr_type, const ExprPtr& term){ return xnew<Expr>(expr_type, lineno_)->set_una_term(term); }

	ExprPtr lvar(const IDPtr& name){ return xnew<Expr>(EXPR_LVAR, lineno_)->set_lvar_name(name); }
	ExprPtr ivar(const IDPtr& name){ return xnew<Expr>(EXPR_IVAR, lineno_)->set_ivar_name(name); }

	ExprPtr string(int_t kind, const IDPtr& value){ return xnew<Expr>(EXPR_STRING, lineno_)->set_string_kind(kind)->set_string_value(value); }
	ExprPtr int_(int_t value){ return xnew<Expr>(EXPR_INT, lineno_)->set_int_value(value); }
	ExprPtr float_(float_t value){ return xnew<Expr>(EXPR_FLOAT, lineno_)->set_float_value(value); }

	ExprPtr null_(){ return xnew<Expr>(EXPR_NULL, lineno_); }
	ExprPtr undefined_(){ return xnew<Expr>(EXPR_UNDEFINED, lineno_); }
	ExprPtr true_(){ return xnew<Expr>(EXPR_TRUE, lineno_); }
	ExprPtr false_(){ return xnew<Expr>(EXPR_FALSE, lineno_); }
	ExprPtr this_(){ return xnew<Expr>(EXPR_THIS, lineno_); }
	ExprPtr current_context(){ return xnew<Expr>(EXPR_CURRENT_CONTEXT, lineno_); }
	ExprPtr args(){ return xnew<Expr>(EXPR_ARGS, lineno_); }
	ExprPtr callee(){ return xnew<Expr>(EXPR_CALLEE, lineno_); }

	ExprPtr multi_value(const ExprPtr& exprs){ return xnew<Expr>(EXPR_MULTI_VALUE, lineno_)->set_multi_value_exprs(exprs); }

	ExprPtr once(const ExprPtr& term){ return una(EXPR_ONCE, term); }
	ExprPtr bracket(const ExprPtr& term){ return una(EXPR_BRACKET, term); }

	ExprPtr call(const ExprPtr& term, const ExprPtr& args, const ExprPtr& extendable_arg){ 
		return xnew<Expr>(EXPR_CALL, lineno_)->set_call_term(term)->set_call_args(args)->set_call_extendable_arg(extendable_arg); }

	ExprPtr member(const ExprPtr& term, const AnyPtr& name){ return xnew<Expr>(EXPR_MEMBER, lineno_)->set_member_term(term)->set_member_name(name); }
	ExprPtr member_q(const ExprPtr& term, const AnyPtr& name){ return xnew<Expr>(EXPR_MEMBER_Q, lineno_)->set_member_term(term)->set_member_name(name); }

	ExprPtr send(const ExprPtr& term, const AnyPtr& name){ return xnew<Expr>(EXPR_SEND, lineno_)->set_send_term(term)->set_send_name(name); }
	ExprPtr send_q(const ExprPtr& term, const AnyPtr& name){ return xnew<Expr>(EXPR_SEND_Q, lineno_)->set_send_term(term)->set_send_name(name); }

	ExprPtr q(const ExprPtr& cond, const ExprPtr& true_, const ExprPtr& false_){ return xnew<Expr>(EXPR_Q, lineno_)->set_q_cond(cond)->set_q_true(true_)->set_q_false(false_); }

	ExprPtr try_(const ExprPtr& body, const IDPtr& catch_var, const ExprPtr& catch_, const ExprPtr& finally_){ return xnew<Expr>(EXPR_TRY, lineno_)->set_try_body(body)->set_try_catch_var(catch_var)->set_try_catch(catch_)->set_try_finally(finally_); }
	ExprPtr for_(const IDPtr& label, const ExprPtr& cond, const ExprPtr& next, const ExprPtr& body, const ExprPtr& else_, const ExprPtr& nobreak){ return xnew<Expr>(EXPR_FOR, lineno_)->set_for_label(label)->set_for_cond(cond)->set_for_next(next)->set_for_body(body)->set_for_else(else_)->set_for_nobreak(nobreak); }
	ExprPtr if_(const ExprPtr& cond,const ExprPtr& body, const ExprPtr& else_){ return xnew<Expr>(EXPR_IF, lineno_)->set_if_cond(cond)->set_if_body(body)->set_if_else(else_); }
		
	ExprPtr massign(const ExprPtr& lhs, const ExprPtr& rhs){ return xnew<Expr>(EXPR_MASSIGN, lineno_)->set_massign_lhs_exprs(lhs)->set_massign_rhs_exprs(rhs); }
	ExprPtr mdefine(const ExprPtr& lhs, const ExprPtr& rhs){ return xnew<Expr>(EXPR_MDEFINE, lineno_)->set_massign_lhs_exprs(lhs)->set_massign_rhs_exprs(rhs); }

	ExprPtr define(const ExprPtr& lhs, const ExprPtr& rhs){ return bin(EXPR_DEFINE, lhs, rhs); }
	ExprPtr assign(const ExprPtr& lhs, const ExprPtr& rhs){ return bin(EXPR_ASSIGN, lhs, rhs); }

	ExprPtr range_(const ExprPtr& lhs, const ExprPtr& rhs, int_t kind){ return xnew<Expr>(EXPR_RANGE, lineno_)->set_range_lhs(lhs)->set_range_rhs(rhs)->set_range_kind(kind); }

private:

	int_t lineno_;
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

	void splice(int_t tag, int_t num){
		ExprPtr ret = xnew<Expr>(tag);
		for(uint_t i=root_->size()-num; i<root_->size(); ++i){
			ret->push_back(root_->at(i));
		}
		root_->resize(root_->size()-num);
		root_->push_back(ret);
	}

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
