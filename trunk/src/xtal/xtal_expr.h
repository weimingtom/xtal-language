
#pragma once

#ifndef XTAL_NO_PARSER

namespace xtal{

enum ExprType{
	EXPR_NULL,
	EXPR_NOP,
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
	EXPR_STATIC,
	EXPR_THROW,
	EXPR_Q,
	EXPR_TRY,
	EXPR_IF,
	EXPR_FOR,
	EXPR_FUN,
	EXPR_MASSIGN,
	EXPR_IVAR,
	EXPR_LVAR,
	EXPR_MEMBER,
	EXPR_CALL,
	EXPR_SEND,
	EXPR_ASSIGN,
	EXPR_DEFINE,
	EXPR_CDEFINE,
	EXPR_AT,
	EXPR_BREAK,
	EXPR_CONTINUE,
	EXPR_BRACKET,
	EXPR_SCOPE,
	EXPR_CLASS,
	EXPR_SWITCH,
	EXPR_TOPLEVEL
};

#define XTAL_DEF_MEMBER(N, Type, Name) \
	Type Name(){ if(size()<=N) resize(N+1); return as<Type>(at(N)); }\
	ExprPtr set_##Name(const Type& v){ if(size()<=N) resize(N+1); set_at(N, v); return ExprPtr(this); }


class Expr;
typedef SmartPtr<Expr> ExprPtr;

inline ExprPtr ep(const AnyPtr& a){
	return ptr_cast<Expr>(a);
}

class Expr : public Array{
	AnyPtr tag_;
	int_t lineno_;

public:

	Expr(const AnyPtr& tag=null, int_t lineno=0){
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

	static ExprPtr make(ExprType type, int_t lineno=0, int_t size=2){
		ExprPtr ret = xnew<Expr>((int_t)type, lineno);
		ret->resize(size);
		return ret;
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

	XTAL_DEF_MEMBER(0, ExprPtr, range_lhs);
	XTAL_DEF_MEMBER(1, ExprPtr, range_rhs);
	XTAL_DEF_MEMBER(2, int_t, range_kind);

	XTAL_DEF_MEMBER(0, ArrayPtr, array_values);
	
	XTAL_DEF_MEMBER(0, MapPtr, map_values);

	XTAL_DEF_MEMBER(0, ArrayPtr, return_exprs);
	
	XTAL_DEF_MEMBER(0, ArrayPtr, yield_exprs);

	XTAL_DEF_MEMBER(0, ArrayPtr, assert_exprs);

	XTAL_DEF_MEMBER(0, ExprPtr, throw_expr);

	XTAL_DEF_MEMBER(0, ExprPtr, try_body);
	XTAL_DEF_MEMBER(1, IDPtr, try_catch_var);
	XTAL_DEF_MEMBER(2, ExprPtr, try_catch);
	XTAL_DEF_MEMBER(3, ExprPtr, try_finally);

	XTAL_DEF_MEMBER(0, ExprPtr, if_cond);
	XTAL_DEF_MEMBER(1, ExprPtr, if_body);
	XTAL_DEF_MEMBER(2, ExprPtr, if_else);

	XTAL_DEF_MEMBER(0, IDPtr, for_label);
	XTAL_DEF_MEMBER(1, ExprPtr, for_cond);
	XTAL_DEF_MEMBER(2, ExprPtr, for_next);
	XTAL_DEF_MEMBER(3, ExprPtr, for_body);
	XTAL_DEF_MEMBER(4, ExprPtr, for_else);
	XTAL_DEF_MEMBER(5, ExprPtr, for_nobreak);

	XTAL_DEF_MEMBER(0, int_t, fun_kind);
	XTAL_DEF_MEMBER(1, MapPtr, fun_params);
	XTAL_DEF_MEMBER(2, bool, fun_extendable_param);
	XTAL_DEF_MEMBER(3, ExprPtr, fun_body);

	XTAL_DEF_MEMBER(0, ArrayPtr, massign_lhs_exprs);
	XTAL_DEF_MEMBER(1, ArrayPtr, massign_rhs_exprs);
	XTAL_DEF_MEMBER(2, bool, massign_define);

	XTAL_DEF_MEMBER(0, IDPtr, ivar_name);

	XTAL_DEF_MEMBER(0, ExprPtr, member_term);
	XTAL_DEF_MEMBER(1, IDPtr, member_name);
	XTAL_DEF_MEMBER(2, ExprPtr, member_pname);
	XTAL_DEF_MEMBER(3, bool, member_q);
	XTAL_DEF_MEMBER(4, ExprPtr, member_ns);

	XTAL_DEF_MEMBER(0, ExprPtr, send_term);
	XTAL_DEF_MEMBER(1, IDPtr, send_name);
	XTAL_DEF_MEMBER(2, ExprPtr, send_pname);
	XTAL_DEF_MEMBER(3, bool, send_q);
	XTAL_DEF_MEMBER(4, ExprPtr, send_ns);
	
	XTAL_DEF_MEMBER(0, ExprPtr, call_term);
	XTAL_DEF_MEMBER(1, ArrayPtr, call_ordered);
	XTAL_DEF_MEMBER(2, MapPtr, call_named);
	XTAL_DEF_MEMBER(3, ExprPtr, call_args);

	XTAL_DEF_MEMBER(0, IDPtr, lvar_name);

	XTAL_DEF_MEMBER(0, int_t, cdefine_accessibility);
	XTAL_DEF_MEMBER(1, IDPtr, cdefine_name);
	XTAL_DEF_MEMBER(2, ExprPtr, cdefine_ns);
	XTAL_DEF_MEMBER(3, ExprPtr, cdefine_term);

	XTAL_DEF_MEMBER(0, IDPtr, break_label);

	XTAL_DEF_MEMBER(0, IDPtr, continue_label);

	XTAL_DEF_MEMBER(0, ArrayPtr, scope_stmts);

	XTAL_DEF_MEMBER(0, int_t, class_kind);
	XTAL_DEF_MEMBER(1, ArrayPtr, class_mixins);
	XTAL_DEF_MEMBER(2, ArrayPtr, class_stmts);
	XTAL_DEF_MEMBER(3, MapPtr, class_ivars);

	XTAL_DEF_MEMBER(0, ExprPtr, switch_cond);
	XTAL_DEF_MEMBER(1, MapPtr, switch_cases);
	XTAL_DEF_MEMBER(2, ExprPtr, switch_default);

	XTAL_DEF_MEMBER(0, ArrayPtr, toplevel_stmts);
};


inline ArrayPtr make_array(){ ArrayPtr ret = xnew<Array>(); return ret; }
inline ArrayPtr make_array(const AnyPtr& v1){ ArrayPtr ret = xnew<Array>(); ret->push_back(v1); return ret; }
inline ArrayPtr make_array(const AnyPtr& v1, const AnyPtr& v2){ ArrayPtr ret = xnew<Array>(); ret->push_back(v1); ret->push_back(v2); return ret; }

inline MapPtr make_map(){ MapPtr ret = xnew<Map>(); return ret; }
inline MapPtr make_map(const AnyPtr& k1, const AnyPtr& v1){ MapPtr ret = xnew<Map>(); ret->set_at(k1, v1); return ret; }
inline MapPtr make_map(const AnyPtr& k1, const AnyPtr& v1, const AnyPtr& k2, const AnyPtr& v2){ MapPtr ret = xnew<Map>();  ret->set_at(k1, v1);  ret->set_at(k2, v2); return ret; }


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

	ExprPtr return_(const ArrayPtr& exprs){ return Expr::make(EXPR_RETURN, lineno_)->set_return_exprs(exprs); }
	ExprPtr continue_(const IDPtr& label){ return Expr::make(EXPR_CONTINUE, lineno_)->set_continue_label(label); }
	ExprPtr break_(const IDPtr& label){ return Expr::make(EXPR_BREAK, lineno_)->set_break_label(label); }
	ExprPtr yield(const ArrayPtr& exprs){ return Expr::make(EXPR_YIELD, lineno_)->set_yield_exprs(exprs); }
	ExprPtr assert_(const ArrayPtr& exprs){ return Expr::make(EXPR_ASSERT, lineno_)->set_assert_exprs(exprs); }
	ExprPtr scope(const ArrayPtr& stmts){ return Expr::make(EXPR_SCOPE, lineno_)->set_scope_stmts(stmts); }
	ExprPtr toplevel(const ArrayPtr& stmts){ return Expr::make(EXPR_TOPLEVEL, lineno_)->set_toplevel_stmts(stmts); }
	ExprPtr class_(int_t kind, const ArrayPtr& mixins, const ArrayPtr& stmts, const MapPtr& ivars){ return Expr::make(EXPR_CLASS, lineno_)->set_class_kind(kind)->set_class_mixins(mixins)->set_class_stmts(stmts)->set_class_ivars(ivars); }
	ExprPtr cdefine(int_t accessibility, const IDPtr& name, const ExprPtr& secondary_key, const ExprPtr& term){ return Expr::make(EXPR_CDEFINE, lineno_)->set_cdefine_accessibility(accessibility)->set_cdefine_name(name)->set_cdefine_ns(secondary_key)->set_cdefine_term(term); }

	ExprPtr fun(int_t kind, const MapPtr& params, bool extendable_param, const ExprPtr& body){ return Expr::make(EXPR_FUN, lineno_)->set_fun_kind(kind)->set_fun_params(params)->set_fun_extendable_param(extendable_param)->set_fun_body(body); }

	ExprPtr bin(ExprType expr_type, const ExprPtr& lhs, const ExprPtr& rhs){ return Expr::make(expr_type, lineno_)->set_bin_lhs(lhs)->set_bin_rhs(rhs); }
	ExprPtr una(ExprType expr_type, const ExprPtr& term){ return Expr::make(expr_type, lineno_)->set_una_term(term); }

	ExprPtr lvar(const IDPtr& name){ return Expr::make(EXPR_LVAR, lineno_)->set_lvar_name(name); }
	ExprPtr ivar(const IDPtr& name){ return Expr::make(EXPR_IVAR, lineno_)->set_ivar_name(name); }

	ExprPtr string(int_t kind, const IDPtr& value){ return Expr::make(EXPR_STRING, lineno_)->set_string_kind(kind)->set_string_value(value); }
	ExprPtr int_(int_t value){ return Expr::make(EXPR_INT, lineno_)->set_int_value(value); }
	ExprPtr float_(float_t value){ return Expr::make(EXPR_FLOAT, lineno_)->set_float_value(value); }

	ExprPtr null_(){ return Expr::make(EXPR_NULL, lineno_); }
	ExprPtr nop_(){ return Expr::make(EXPR_NOP, lineno_); }
	ExprPtr true_(){ return Expr::make(EXPR_TRUE, lineno_); }
	ExprPtr false_(){ return Expr::make(EXPR_FALSE, lineno_); }
	ExprPtr this_(){ return Expr::make(EXPR_THIS, lineno_); }
	ExprPtr current_context(){ return Expr::make(EXPR_CURRENT_CONTEXT, lineno_); }
	ExprPtr args(){ return Expr::make(EXPR_ARGS, lineno_); }
	ExprPtr callee(){ return Expr::make(EXPR_CALLEE, lineno_); }

	ExprPtr once(const ExprPtr& term){ return una(EXPR_ONCE, term); }
	ExprPtr static_(const ExprPtr& term){ return una(EXPR_STATIC, term); }
	ExprPtr bracket(const ExprPtr& term){ return una(EXPR_BRACKET, term); }

	ExprPtr call(const ExprPtr& term, const ArrayPtr& ordered, const MapPtr& named, const ExprPtr& args){ return Expr::make(EXPR_CALL, lineno_)->set_call_term(term)->set_call_ordered(ordered)->set_call_named(named)->set_call_args(args); }

	ExprPtr member(const ExprPtr& term, const IDPtr& name){ return Expr::make(EXPR_MEMBER, lineno_)->set_member_term(term)->set_member_name(name); }
	ExprPtr member_q(const ExprPtr& term, const IDPtr& name){ return Expr::make(EXPR_MEMBER, lineno_)->set_member_term(term)->set_member_name(name)->set_member_q(true); }
	ExprPtr member_e(const ExprPtr& term, const ExprPtr& name){ return Expr::make(EXPR_MEMBER, lineno_)->set_member_term(term)->set_member_pname(name); }
	ExprPtr member_eq(const ExprPtr& term, const ExprPtr& name){ return Expr::make(EXPR_MEMBER, lineno_)->set_member_term(term)->set_member_pname(name)->set_member_q(true); }

	ExprPtr send(const ExprPtr& term, const IDPtr& name){ return Expr::make(EXPR_SEND, lineno_)->set_send_term(term)->set_send_name(name); }
	ExprPtr send_q(const ExprPtr& term, const IDPtr& name){ return Expr::make(EXPR_SEND, lineno_)->set_send_term(term)->set_send_name(name)->set_send_q(true); }
	ExprPtr send_e(const ExprPtr& term, const ExprPtr& name){ return Expr::make(EXPR_SEND, lineno_)->set_send_term(term)->set_send_pname(name); }
	ExprPtr send_eq(const ExprPtr& term, const ExprPtr& name){ return Expr::make(EXPR_SEND, lineno_)->set_send_term(term)->set_send_pname(name)->set_send_q(true); }

	ExprPtr q(const ExprPtr& cond, const ExprPtr& true_, const ExprPtr& false_){ return Expr::make(EXPR_Q, lineno_)->set_q_cond(cond)->set_q_true(true_)->set_q_false(false_); }

	ExprPtr try_(const ExprPtr& body, const IDPtr& catch_var, const ExprPtr& catch_, const ExprPtr& finally_){ return Expr::make(EXPR_TRY, lineno_)->set_try_body(body)->set_try_catch_var(catch_var)->set_try_catch(catch_)->set_try_finally(finally_); }
	ExprPtr for_(const IDPtr& label, const ExprPtr& cond, const ExprPtr& next, const ExprPtr& body, const ExprPtr& else_, const ExprPtr& nobreak){ return Expr::make(EXPR_FOR, lineno_, 8)->set_for_label(label)->set_for_cond(cond)->set_for_next(next)->set_for_body(body)->set_for_else(else_)->set_for_nobreak(nobreak); }
	ExprPtr if_(const ExprPtr& cond,const ExprPtr& body, const ExprPtr& else_){ return Expr::make(EXPR_IF, lineno_)->set_if_cond(cond)->set_if_body(body)->set_if_else(else_); }
		
	ExprPtr massign(const ArrayPtr& lhs, const ArrayPtr& rhs, bool define){ return Expr::make(EXPR_MASSIGN, lineno_)->set_massign_lhs_exprs(lhs)->set_massign_rhs_exprs(rhs)->set_massign_define(define); }

	ExprPtr define(const ExprPtr& lhs, const ExprPtr& rhs){ return bin(EXPR_DEFINE, lhs, rhs); }
	ExprPtr assign(const ExprPtr& lhs, const ExprPtr& rhs){ return bin(EXPR_ASSIGN, lhs, rhs); }

	ExprPtr array(const ArrayPtr& exprs){ return Expr::make(EXPR_ARRAY, lineno_)->set_array_values(exprs); }
	ExprPtr map(const MapPtr& exprs){ return Expr::make(EXPR_MAP, lineno_)->set_map_values(exprs); }

	ExprPtr switch_(const ExprPtr& cond, const MapPtr& cases, const ExprPtr& default_){ return Expr::make(EXPR_SWITCH, lineno_)->set_switch_cond(cond)->set_switch_cases(cases)->set_switch_default(default_); }

	ExprPtr range_(const ExprPtr& lhs, const ExprPtr& rhs, int_t kind){ return Expr::make(EXPR_RANGE, lineno_)->set_range_lhs(lhs)->set_range_rhs(rhs)->set_range_kind(kind); }

private:

	int_t lineno_;
};

}

#endif
