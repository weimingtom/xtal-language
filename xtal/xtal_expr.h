
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
	EXPR_TOPLEVEL
};

#define XTAL_DEF_MEMBER(N, Type, Name) \
	Type Name(){ if(size()<=N) resize(N+1); return as<Type>(at(N)); }\
	ExprPtr set_##Name(const Type& v){ if(size()<=N) resize(N+1); set_at(N, v); return ExprPtr::from_this(this); }


class Expr;
typedef SmartPtr<Expr> ExprPtr;

inline ExprPtr ep(const AnyPtr& a){
	return ptr_cast<Expr>(a);
}

class Expr : public Array{
public:
	static ExprPtr make(ExprType type, int_t lineno=0, int_t size=2){
		ExprPtr ret = xnew<Expr>();
		ret->resize(size);
		ret->set_type((int_t)type);
		ret->set_lineno(lineno);
		return ret;
	}

	XTAL_DEF_MEMBER(0, int_t, type);
	XTAL_DEF_MEMBER(1, int_t, lineno);
	
	XTAL_DEF_MEMBER(2, ExprPtr, una_term);

	XTAL_DEF_MEMBER(2, ExprPtr, bin_lhs);
	XTAL_DEF_MEMBER(3, ExprPtr, bin_rhs);

	XTAL_DEF_MEMBER(2, ExprPtr, q_cond);
	XTAL_DEF_MEMBER(3, ExprPtr, q_true);
	XTAL_DEF_MEMBER(4, ExprPtr, q_false);

	XTAL_DEF_MEMBER(2, int_t, int_value);

	XTAL_DEF_MEMBER(2, float_t, float_value);

	XTAL_DEF_MEMBER(2, int_t, string_kind);
	XTAL_DEF_MEMBER(3, InternedStringPtr, string_value);

	XTAL_DEF_MEMBER(2, ArrayPtr, array_values);
	
	XTAL_DEF_MEMBER(2, MapPtr, map_values);

	XTAL_DEF_MEMBER(2, ArrayPtr, return_exprs);
	
	XTAL_DEF_MEMBER(2, ArrayPtr, yield_exprs);

	XTAL_DEF_MEMBER(2, ArrayPtr, assert_exprs);

	XTAL_DEF_MEMBER(2, ExprPtr, throw_expr);

	XTAL_DEF_MEMBER(2, ExprPtr, try_body);
	XTAL_DEF_MEMBER(3, InternedStringPtr, try_catch_var);
	XTAL_DEF_MEMBER(4, ExprPtr, try_catch);
	XTAL_DEF_MEMBER(5, ExprPtr, try_finally);

	XTAL_DEF_MEMBER(2, ExprPtr, if_cond);
	XTAL_DEF_MEMBER(3, ExprPtr, if_body);
	XTAL_DEF_MEMBER(4, ExprPtr, if_else);

	XTAL_DEF_MEMBER(2, InternedStringPtr, for_label);
	XTAL_DEF_MEMBER(3, ExprPtr, for_cond);
	XTAL_DEF_MEMBER(4, ExprPtr, for_next);
	XTAL_DEF_MEMBER(5, ExprPtr, for_body);
	XTAL_DEF_MEMBER(6, ExprPtr, for_else);
	XTAL_DEF_MEMBER(7, ExprPtr, for_nobreak);

	XTAL_DEF_MEMBER(2, int_t, fun_kind);
	XTAL_DEF_MEMBER(3, MapPtr, fun_params);
	XTAL_DEF_MEMBER(4, bool, fun_extendable_param);
	XTAL_DEF_MEMBER(5, ExprPtr, fun_body);

	XTAL_DEF_MEMBER(2, ArrayPtr, massign_lhs_exprs);
	XTAL_DEF_MEMBER(3, ArrayPtr, massign_rhs_exprs);
	XTAL_DEF_MEMBER(4, bool, massign_define);

	XTAL_DEF_MEMBER(2, InternedStringPtr, ivar_name);

	XTAL_DEF_MEMBER(2, ExprPtr, member_term);
	XTAL_DEF_MEMBER(3, InternedStringPtr, member_name);
	XTAL_DEF_MEMBER(4, ExprPtr, member_pname);
	XTAL_DEF_MEMBER(5, bool, member_q);

	XTAL_DEF_MEMBER(2, ExprPtr, send_term);
	XTAL_DEF_MEMBER(3, InternedStringPtr, send_name);
	XTAL_DEF_MEMBER(4, ExprPtr, send_pname);
	XTAL_DEF_MEMBER(5, bool, send_q);
	
	XTAL_DEF_MEMBER(2, ExprPtr, call_term);
	XTAL_DEF_MEMBER(3, ArrayPtr, call_ordered);
	XTAL_DEF_MEMBER(4, MapPtr, call_named);
	XTAL_DEF_MEMBER(5, bool, call_have_args);

	XTAL_DEF_MEMBER(2, InternedStringPtr, lvar_name);

	XTAL_DEF_MEMBER(2, int_t, cdefine_accessibility);
	XTAL_DEF_MEMBER(3, InternedStringPtr, cdefine_name);
	XTAL_DEF_MEMBER(4, ExprPtr, cdefine_ns);
	XTAL_DEF_MEMBER(5, ExprPtr, cdefine_term);

	XTAL_DEF_MEMBER(2, InternedStringPtr, break_label);

	XTAL_DEF_MEMBER(2, InternedStringPtr, continue_label);

	XTAL_DEF_MEMBER(2, ArrayPtr, scope_stmts);

	XTAL_DEF_MEMBER(2, int_t, class_kind);
	XTAL_DEF_MEMBER(3, ArrayPtr, class_mixins);
	XTAL_DEF_MEMBER(4, ArrayPtr, class_stmts);
	XTAL_DEF_MEMBER(5, MapPtr, class_ivars);

	XTAL_DEF_MEMBER(2, ArrayPtr, toplevel_stmts);
};


inline ArrayPtr make_array(){ ArrayPtr ret = xnew<Array>(); return ret; }
inline ArrayPtr make_array(const AnyPtr& v1){ ArrayPtr ret = xnew<Array>(); ret->push_back(v1); return ret; }
inline ArrayPtr make_array(const AnyPtr& v1, const AnyPtr& v2){ ArrayPtr ret = xnew<Array>(); ret->push_back(v1); ret->push_back(v2); return ret; }

inline MapPtr make_map(){ MapPtr ret = xnew<Map>(); return ret; }
inline MapPtr make_map(const AnyPtr& k1, const AnyPtr& v1){ MapPtr ret = xnew<Map>(); ret->set_at(k1, v1); return ret; }
inline MapPtr make_map(const AnyPtr& k1, const AnyPtr& v1, const AnyPtr& k2, const AnyPtr& v2){ MapPtr ret = xnew<Map>();  ret->set_at(k1, v1);  ret->set_at(k2, v2); return ret; }

inline ExprPtr return_(int_t lineno, const ArrayPtr& exprs){ return Expr::make(EXPR_RETURN, lineno, 3)->set_return_exprs(exprs); }
inline ExprPtr continue_(int_t lineno, const InternedStringPtr& label){ return Expr::make(EXPR_CONTINUE, lineno, 3)->set_continue_label(label); }
inline ExprPtr break_(int_t lineno, const InternedStringPtr& label){ return Expr::make(EXPR_BREAK, lineno, 3)->set_break_label(label); }
inline ExprPtr yield(int_t lineno, const ArrayPtr& exprs){ return Expr::make(EXPR_YIELD, lineno, 3)->set_yield_exprs(exprs); }
inline ExprPtr assert_(int_t lineno, const ArrayPtr& exprs){ return Expr::make(EXPR_ASSERT, lineno, 3)->set_assert_exprs(exprs); }
inline ExprPtr scope(int_t lineno, const ArrayPtr& stmts){ return Expr::make(EXPR_SCOPE, lineno, 3)->set_scope_stmts(stmts); }
inline ExprPtr toplevel(int_t lineno, const ArrayPtr& stmts){ return Expr::make(EXPR_TOPLEVEL, lineno, 3)->set_toplevel_stmts(stmts); }
inline ExprPtr class_(int_t lineno, int_t kind, const ArrayPtr& mixins, const ArrayPtr& stmts, const MapPtr& ivars){ return Expr::make(EXPR_CLASS, lineno, 3)->set_class_kind(kind)->set_class_mixins(mixins)->set_class_stmts(stmts)->set_class_ivars(ivars); }
inline ExprPtr cdefine(int_t lineno, int_t accessibility, const InternedStringPtr& name, const ExprPtr& ns, const ExprPtr& term){ return Expr::make(EXPR_CDEFINE, lineno, 6)->set_cdefine_accessibility(accessibility)->set_cdefine_name(name)->set_cdefine_ns(ns)->set_cdefine_term(term); }

inline ExprPtr fun(int_t lineno, int_t kind, const MapPtr& params, bool extendable_param, const ExprPtr& body){ return Expr::make(EXPR_FUN, lineno, 6)->set_fun_kind(kind)->set_fun_params(params)->set_fun_extendable_param(extendable_param)->set_fun_body(body); }

inline ExprPtr bin(ExprType expr_type, int_t lineno, const ExprPtr& lhs, const ExprPtr& rhs){ return Expr::make(expr_type, lineno, 4)->set_bin_lhs(lhs)->set_bin_rhs(rhs); }
inline ExprPtr una(ExprType expr_type, int_t lineno, const ExprPtr& term){ return Expr::make(expr_type, lineno, 3)->set_una_term(term); }

inline ExprPtr lvar(int_t lineno, const InternedStringPtr& name){ return Expr::make(EXPR_LVAR, lineno, 3)->set_lvar_name(name); }
inline ExprPtr ivar(int_t lineno, const InternedStringPtr& name){ return Expr::make(EXPR_IVAR, lineno, 3)->set_ivar_name(name); }

inline ExprPtr string(int_t lineno, int_t kind, const InternedStringPtr& value){ return Expr::make(EXPR_STRING, lineno, 4)->set_string_kind(kind)->set_string_value(value); }
inline ExprPtr int_(int_t lineno, int_t value){ return Expr::make(EXPR_INT, lineno, 3)->set_int_value(value); }
inline ExprPtr float_(int_t lineno, float_t value){ return Expr::make(EXPR_FLOAT, lineno, 3)->set_float_value(value); }

inline ExprPtr null_(int_t lineno){ return Expr::make(EXPR_NULL, lineno); }
inline ExprPtr nop_(int_t lineno){ return Expr::make(EXPR_NOP, lineno); }
inline ExprPtr true_(int_t lineno){ return Expr::make(EXPR_TRUE, lineno); }
inline ExprPtr false_(int_t lineno){ return Expr::make(EXPR_FALSE, lineno); }
inline ExprPtr this_(int_t lineno){ return Expr::make(EXPR_THIS, lineno); }
inline ExprPtr current_context(int_t lineno){ return Expr::make(EXPR_CURRENT_CONTEXT, lineno); }
inline ExprPtr args(int_t lineno){ return Expr::make(EXPR_ARGS, lineno); }
inline ExprPtr callee(int_t lineno){ return Expr::make(EXPR_CALLEE, lineno); }

inline ExprPtr once(int_t lineno, const ExprPtr& term){ return una(EXPR_ONCE, lineno, term); }
inline ExprPtr static_(int_t lineno, const ExprPtr& term){ return una(EXPR_STATIC, lineno, term); }
inline ExprPtr bracket(int_t lineno, const ExprPtr& term){ return una(EXPR_BRACKET, lineno, term); }

inline ExprPtr call(int_t lineno, const ExprPtr& term, const ArrayPtr& ordered, const MapPtr& named, bool have_args){ return Expr::make(EXPR_CALL, lineno, 6)->set_call_term(term)->set_call_ordered(ordered)->set_call_named(named)->set_call_have_args(have_args); }

inline ExprPtr member(int_t lineno, const ExprPtr& term, const InternedStringPtr& name){ return Expr::make(EXPR_MEMBER, lineno, 6)->set_member_term(term)->set_member_name(name); }
inline ExprPtr member_q(int_t lineno, const ExprPtr& term, const InternedStringPtr& name){ return Expr::make(EXPR_MEMBER, lineno, 6)->set_member_term(term)->set_member_name(name)->set_member_q(true); }
inline ExprPtr member_e(int_t lineno, const ExprPtr& term, const ExprPtr& name){ return Expr::make(EXPR_MEMBER, lineno, 6)->set_member_term(term)->set_member_pname(name); }
inline ExprPtr member_eq(int_t lineno, const ExprPtr& term, const ExprPtr& name){ return Expr::make(EXPR_MEMBER, lineno, 6)->set_member_term(term)->set_member_pname(name)->set_member_q(true); }

inline ExprPtr send(int_t lineno, const ExprPtr& term, const InternedStringPtr& name){ return Expr::make(EXPR_SEND, lineno, 6)->set_send_term(term)->set_send_name(name); }
inline ExprPtr send_q(int_t lineno, const ExprPtr& term, const InternedStringPtr& name){ return Expr::make(EXPR_SEND, lineno, 6)->set_send_term(term)->set_send_name(name)->set_send_q(true); }
inline ExprPtr send_e(int_t lineno, const ExprPtr& term, const ExprPtr& name){ return Expr::make(EXPR_SEND, lineno, 6)->set_send_term(term)->set_send_pname(name); }
inline ExprPtr send_eq(int_t lineno, const ExprPtr& term, const ExprPtr& name){ return Expr::make(EXPR_SEND, lineno, 6)->set_send_term(term)->set_send_pname(name)->set_send_q(true); }

inline ExprPtr q(int_t lineno, const ExprPtr& cond, const ExprPtr& true_, const ExprPtr& false_){ return Expr::make(EXPR_Q, lineno, 5)->set_q_cond(cond)->set_q_true(true_)->set_q_false(false_); }

inline ExprPtr try_(int_t lineno, const ExprPtr& body, const InternedStringPtr& catch_var, const ExprPtr& catch_, const ExprPtr& finally_){ return Expr::make(EXPR_TRY, lineno, 6)->set_try_body(body)->set_try_catch_var(catch_var)->set_try_catch(catch_)->set_try_finally(finally_); }
inline ExprPtr for_(int_t lineno, const InternedStringPtr& label, const ExprPtr& cond, const ExprPtr& next, const ExprPtr& body, const ExprPtr& else_, const ExprPtr& nobreak){ return Expr::make(EXPR_FOR, lineno, 8)->set_for_label(label)->set_for_cond(cond)->set_for_next(next)->set_for_body(body)->set_for_else(else_)->set_for_nobreak(nobreak); }
inline ExprPtr if_(int_t lineno, const ExprPtr& cond,const ExprPtr& body, const ExprPtr& else_){ return Expr::make(EXPR_IF, lineno, 5)->set_if_cond(cond)->set_if_body(body)->set_if_else(else_); }
	
inline ExprPtr massign(int_t lineno, const ArrayPtr& lhs, const ArrayPtr& rhs, bool define){ return Expr::make(EXPR_MASSIGN, lineno, 5)->set_massign_lhs_exprs(lhs)->set_massign_rhs_exprs(rhs)->set_massign_define(define); }

inline ExprPtr define(int_t lineno, const ExprPtr& lhs, const ExprPtr& rhs){ return bin(EXPR_DEFINE, lineno, lhs, rhs); }
inline ExprPtr assign(int_t lineno, const ExprPtr& lhs, const ExprPtr& rhs){ return bin(EXPR_ASSIGN, lineno, lhs, rhs); }

inline ExprPtr array(int_t lineno, const ArrayPtr& exprs){ return Expr::make(EXPR_ARRAY, lineno, 3)->set_array_values(exprs); }
inline ExprPtr map(int_t lineno, const MapPtr& exprs){ return Expr::make(EXPR_MAP, lineno, 3)->set_map_values(exprs); }

}

#endif
