#include "xtal.h"
#include "xtal_macro.h"

#include "xtal_codebuilder.h"

#ifndef XTAL_NO_PARSER

namespace xtal{
	
ExprPtr CodeBuilder::setup_expr(const ExprPtr& e){
/*
	switch(e->itag()){
	case EXPR_NULL:
	case EXPR_UNDEFINED:
	case EXPR_TRUE:
	case EXPR_FALSE:
	case EXPR_CALLEE:
	case EXPR_ARGS:
	case EXPR_THIS:
	case EXPR_DEBUG:
	case EXPR_CURRENT_CONTEXT:
	case EXPR_INT:
	case EXPR_FLOAT:
	case EXPR_STRING:
	case EXPR_ARRAY:
	case EXPR_MAP:
	case EXPR_MULTI_VALUE:

	case EXPR_ADD:
	case EXPR_SUB:
	case EXPR_CAT:
	case EXPR_MUL:
	case EXPR_DIV:
	case EXPR_MOD:
	case EXPR_AND:
	case EXPR_OR:
	case EXPR_XOR:
	case EXPR_SHL:
	case EXPR_SHR:
	case EXPR_USHR:

	case EXPR_ADD_ASSIGN:
	case EXPR_SUB_ASSIGN:
	case EXPR_CAT_ASSIGN:
	case EXPR_MUL_ASSIGN:
	case EXPR_DIV_ASSIGN:
	case EXPR_MOD_ASSIGN:
	case EXPR_AND_ASSIGN:
	case EXPR_OR_ASSIGN:
	case EXPR_XOR_ASSIGN:
	case EXPR_SHL_ASSIGN:
	case EXPR_SHR_ASSIGN:
	case EXPR_USHR_ASSIGN:

	case EXPR_EQ:
	case EXPR_NE:
	case EXPR_LT:
	case EXPR_LE:
	case EXPR_GT:
	case EXPR_GE:
	case EXPR_RAWEQ:
	case EXPR_RAWNE:
	case EXPR_IN:
	case EXPR_NIN:
	case EXPR_IS:
	case EXPR_NIS:

	case EXPR_ANDAND:
	case EXPR_OROR:
		setup_expr(e->bin_lhs, e->bin_rhs());
		break;

	case EXPR_CATCH:
		setup_expr(e->at(0), e->at(1));
		break;

	case EXPR_INC:
	case EXPR_DEC:
	case EXPR_POS:
	case EXPR_NEG:
	case EXPR_COM:
	case EXPR_NOT:

	case EXPR_RANGE:
	case EXPR_RETURN:
	case EXPR_YIELD:
	case EXPR_ASSERT:
	case EXPR_ONCE:
	case EXPR_THROW:
	case EXPR_Q:
	case EXPR_TRY:
	case EXPR_IF:
	case EXPR_FOR:
	case EXPR_FUN:
	case EXPR_MASSIGN:
	case EXPR_MDEFINE:
	case EXPR_IVAR:
	case EXPR_LVAR:
	case EXPR_MEMBER:
	case EXPR_MEMBER_Q:
	case EXPR_PROPERTY:
	case EXPR_PROPERTY_Q:
	case EXPR_CALL:
	case EXPR_ASSIGN:
	case EXPR_DEFINE:
	case EXPR_CDEFINE_MEMBER:
	case EXPR_CDEFINE_IVAR:
	case EXPR_AT:
	case EXPR_BREAK:
	case EXPR_CONTINUE:
	case EXPR_BRACKET:
	case EXPR_SCOPE:
	case EXPR_CLASS:
	case EXPR_SWITCH:
	case EXPR_SWITCH_CASE:
	case EXPR_SWITCH_DEFAULT:
	case EXPR_TOPLEVEL:
*/
	return null;
}

}

#endif

