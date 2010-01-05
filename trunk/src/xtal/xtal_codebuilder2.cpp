#include "xtal.h"
#include "xtal_macro.h"

#include "xtal_codebuilder.h"

#ifndef XTAL_NO_PARSER

namespace xtal{

int_t CodeBuilder::compile_bin2(const ExprPtr& e, int_t stack_top, int_t result){
	int_t lhs = compile_expr(e->bin_lhs(), stack_top);
	int_t rhs = compile_expr(e->bin_rhs(), stack_top);
	put_bin(result, e, lhs, rhs, stack_top);
	return 1;
}
	
int_t CodeBuilder::compile_expr(const AnyPtr& p, int_t& stack_top){
	if(type(p)==TYPE_INT){
		return ivalue(p);
	}

	if(ep(p)->itag()==EXPR_LVAR){
		LVarInfo var = var_find(ep(p)->lvar_name());
		if(var.pos>=0){
			if(var.pos<=0xff && !var.out_of_fun){
				return var.vpos;
			}
		}
	}
	
	compile_expr(p, stack_top+1, stack_top);

	return stack_top++;
}

int_t CodeBuilder::compile_expr(const AnyPtr& p, int_t stack_top, int_t result, int_t result_count){
	//AnyPtr val;
	//if(compile_expr(p, info, val)){
	//	return 0;
	//}	

	if(type(p)==TYPE_INT){
		put_inst(InstCopy(result, ivalue(p)));
		return 1;
	}

	ExprPtr e = ep(p);

	int_t temp = stack_top;
	int_t ret = compile_e(e, temp, result, result_count);

	if(result_count!=0 && ret!=result_count){
		put_inst(InstAdjustValues(result, ret, result_count));
	}

	return ret;
}
	
void CodeBuilder::compile_stmt(const AnyPtr& p){
	if(!p){
		return;
	}

	ExprPtr e = ep(p);

	if(e->lineno()!=0){
		linenos_.push(e->lineno());
		if(result_->set_lineno_info(e->lineno())){
			put_inst(InstLine());
		}
	}

	compile_e(e, 0, 0, 0);

	if(e->lineno()!=0){
		result_->set_lineno_info(e->lineno());
		linenos_.pop();
	}
}

int_t CodeBuilder::compile_e(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count){
	if(!e){
		error_->error(lineno(), Xt("XCE1001"));
		return 0;
	}

	typedef int_t (CodeBuilder::*expr_compile_fun_t)(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count);
	static expr_compile_fun_t expr_compile_fun_[EXPR_MAX] ={
//{STMT_TABLE{{
		&CodeBuilder::compile_expr_LIST,
		&CodeBuilder::compile_expr_NULL,
		&CodeBuilder::compile_expr_UNDEFINED,
		&CodeBuilder::compile_expr_TRUE,
		&CodeBuilder::compile_expr_FALSE,
		&CodeBuilder::compile_expr_CALLEE,
		&CodeBuilder::compile_expr_THIS,
		&CodeBuilder::compile_expr_NUMBER,
		&CodeBuilder::compile_expr_STRING,
		&CodeBuilder::compile_expr_ARRAY,
		&CodeBuilder::compile_expr_MAP,
		&CodeBuilder::compile_expr_VALUES,
		&CodeBuilder::compile_expr_ADD,
		&CodeBuilder::compile_expr_SUB,
		&CodeBuilder::compile_expr_CAT,
		&CodeBuilder::compile_expr_MUL,
		&CodeBuilder::compile_expr_DIV,
		&CodeBuilder::compile_expr_MOD,
		&CodeBuilder::compile_expr_AND,
		&CodeBuilder::compile_expr_OR,
		&CodeBuilder::compile_expr_XOR,
		&CodeBuilder::compile_expr_SHL,
		&CodeBuilder::compile_expr_SHR,
		&CodeBuilder::compile_expr_USHR,
		&CodeBuilder::compile_expr_EQ,
		&CodeBuilder::compile_expr_NE,
		&CodeBuilder::compile_expr_LT,
		&CodeBuilder::compile_expr_LE,
		&CodeBuilder::compile_expr_GT,
		&CodeBuilder::compile_expr_GE,
		&CodeBuilder::compile_expr_RAWEQ,
		&CodeBuilder::compile_expr_RAWNE,
		&CodeBuilder::compile_expr_IN,
		&CodeBuilder::compile_expr_NIN,
		&CodeBuilder::compile_expr_IS,
		&CodeBuilder::compile_expr_NIS,
		&CodeBuilder::compile_expr_ANDAND,
		&CodeBuilder::compile_expr_OROR,
		&CodeBuilder::compile_expr_CATCH,
		&CodeBuilder::compile_expr_POS,
		&CodeBuilder::compile_expr_NEG,
		&CodeBuilder::compile_expr_COM,
		&CodeBuilder::compile_expr_NOT,
		&CodeBuilder::compile_expr_RANGE,
		&CodeBuilder::compile_expr_FUN,
		&CodeBuilder::compile_expr_CLASS,
		&CodeBuilder::compile_expr_ONCE,
		&CodeBuilder::compile_expr_IVAR,
		&CodeBuilder::compile_expr_LVAR,
		&CodeBuilder::compile_expr_AT,
		&CodeBuilder::compile_expr_Q,
		&CodeBuilder::compile_expr_MEMBER,
		&CodeBuilder::compile_expr_MEMBER_Q,
		&CodeBuilder::compile_expr_PROPERTY,
		&CodeBuilder::compile_expr_PROPERTY_Q,
		&CodeBuilder::compile_expr_CALL,
		&CodeBuilder::compile_expr_YIELD,
		&CodeBuilder::compile_expr_INC,
		&CodeBuilder::compile_expr_DEC,
		&CodeBuilder::compile_expr_ADD_ASSIGN,
		&CodeBuilder::compile_expr_SUB_ASSIGN,
		&CodeBuilder::compile_expr_CAT_ASSIGN,
		&CodeBuilder::compile_expr_MUL_ASSIGN,
		&CodeBuilder::compile_expr_DIV_ASSIGN,
		&CodeBuilder::compile_expr_MOD_ASSIGN,
		&CodeBuilder::compile_expr_AND_ASSIGN,
		&CodeBuilder::compile_expr_OR_ASSIGN,
		&CodeBuilder::compile_expr_XOR_ASSIGN,
		&CodeBuilder::compile_expr_SHL_ASSIGN,
		&CodeBuilder::compile_expr_SHR_ASSIGN,
		&CodeBuilder::compile_expr_USHR_ASSIGN,
		&CodeBuilder::compile_expr_RETURN,
		&CodeBuilder::compile_expr_ASSERT,
		&CodeBuilder::compile_expr_THROW,
		&CodeBuilder::compile_expr_TRY,
		&CodeBuilder::compile_expr_IF,
		&CodeBuilder::compile_expr_FOR,
		&CodeBuilder::compile_expr_MASSIGN,
		&CodeBuilder::compile_expr_MDEFINE,
		&CodeBuilder::compile_expr_ASSIGN,
		&CodeBuilder::compile_expr_DEFINE,
		&CodeBuilder::compile_expr_CDEFINE_MEMBER,
		&CodeBuilder::compile_expr_CDEFINE_IVAR,
		&CodeBuilder::compile_expr_BREAK,
		&CodeBuilder::compile_expr_CONTINUE,
		&CodeBuilder::compile_expr_BRACKET,
		&CodeBuilder::compile_expr_SCOPE,
		&CodeBuilder::compile_expr_SWITCH,
		&CodeBuilder::compile_expr_TOPLEVEL,
//}}STMT_TABLE}
	};

	return (this->*expr_compile_fun_[e->itag()])(e, stack_top, result, result_count);
}


//{STMT_IMPLS{{
int_t CodeBuilder::compile_expr_LIST(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count){
	//compile_expr(e, 0);
	return int_t(0);
}

int_t CodeBuilder::compile_expr_NULL(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count){
	put_inst(InstLoadValue(result, LOAD_NULL));
	return 1;
}

int_t CodeBuilder::compile_expr_UNDEFINED(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count){
	put_inst(InstLoadValue(result, LOAD_UNDEFINED));
	return 1;
}

int_t CodeBuilder::compile_expr_TRUE(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count){
	put_inst(InstLoadValue(result, LOAD_TRUE));
	return 1;
}

int_t CodeBuilder::compile_expr_FALSE(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count){
	put_inst(InstLoadValue(result, LOAD_FALSE));
	return 1;
}

int_t CodeBuilder::compile_expr_CALLEE(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count){
	put_inst(InstLoadCallee(result));
	return 1;
}

int_t CodeBuilder::compile_expr_THIS(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count){
	put_inst(InstLoadThis(result));
	return 1;
}

int_t CodeBuilder::compile_expr_NUMBER(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count){
	AnyPtr nvalue = e->number_value();
	if(type(nvalue)==TYPE_INT){
		int_t value = ivalue(nvalue);
		if(value==(i8)value){ 
			put_inst(InstLoadInt1Byte(result, value));
		}
		else{ 
			put_inst(InstLoadConstant(result, register_value(value)));
		}
	}
	else{
		float_t value = fvalue(nvalue);
		if(value==(i8)value){ 
			put_inst(InstLoadFloat1Byte(result, (i8)value));
		}
		else{ 
			put_inst(InstLoadConstant(result, register_value(value)));
		}
	}

	return 1;
}

int_t CodeBuilder::compile_expr_STRING(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count){
	if(e->string_kind()==KIND_TEXT){
		put_inst(InstLoadConstant(result, register_value(text(e->string_value()->to_s()))));
	}
	else if(e->string_kind()==KIND_FORMAT){
		put_inst(InstLoadConstant(result, register_value(format(e->string_value()->to_s()))));
	}
	else{
		put_inst(InstLoadConstant(result, register_value(e->string_value())));
	}

	return 1;
}

int_t CodeBuilder::compile_expr_ARRAY(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count){
	if(result_count!=0){
		put_inst(InstMakeArray(result));
		Xfor(v, e->array_values()){
			compile_expr(v, stack_top+1, stack_top);
			put_inst(InstArrayAppend(result, stack_top));
		}
		return 1;
	}
	else{
		Xfor(v, e->array_values()){
			compile_expr(v, stack_top+1, stack_top);
		}
		return 0;
	}
}

int_t CodeBuilder::compile_expr_MAP(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count){
	if(result_count!=0){
		put_inst(InstMakeMap(result));
		Xfor_cast(const ArrayPtr& v, e->map_values()){
			compile_expr(v->at(0), stack_top+1, stack_top);
			compile_expr(v->at(1), stack_top+2, stack_top+1);
			put_inst(InstMapInsert(result, stack_top, stack_top+1));
		}
		return 1;
	}
	else{
		Xfor_cast(const ArrayPtr& v, e->map_values()){
			compile_expr(v->at(0), stack_top+1, stack_top);
			compile_expr(v->at(1), stack_top+2, stack_top+1);
		}
		return 0;
	}
}

int_t CodeBuilder::compile_expr_VALUES(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count){
	if(e->multi_value_exprs()){
		int_t stack_base = stack_top;

		Xfor(v, e->multi_value_exprs()){
			compile_expr(v, stack_top);	
		}

		if(e->multi_value_exprs()->size()!=(uint_t)result_count){
			put_inst(InstAdjustValues(stack_base, e->multi_value_exprs()->size(), 1));
		}

		put_inst(InstCopy(result, stack_base));
		return 1;
	}
	put_inst(InstLoadValue(result, LOAD_UNDEFINED));
	return 1;
}

int_t CodeBuilder::compile_expr_ADD(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count){
	return compile_bin2(e, stack_top, result);
}

int_t CodeBuilder::compile_expr_SUB(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count){
	return compile_bin2(e, stack_top, result);
}

int_t CodeBuilder::compile_expr_CAT(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count){
	return compile_bin2(e, stack_top, result);
}

int_t CodeBuilder::compile_expr_MUL(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count){
	return compile_bin2(e, stack_top, result);
}

int_t CodeBuilder::compile_expr_DIV(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count){
	return compile_bin2(e, stack_top, result);
}

int_t CodeBuilder::compile_expr_MOD(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count){
	return compile_bin2(e, stack_top, result);
}

int_t CodeBuilder::compile_expr_AND(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count){
	return compile_bin2(e, stack_top, result);
}

int_t CodeBuilder::compile_expr_OR(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count){
	return compile_bin2(e, stack_top, result);
}

int_t CodeBuilder::compile_expr_XOR(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count){
	return compile_bin2(e, stack_top, result);
}

int_t CodeBuilder::compile_expr_SHL(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count){
	return compile_bin2(e, stack_top, result);
}

int_t CodeBuilder::compile_expr_SHR(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count){
	return compile_bin2(e, stack_top, result);
}

int_t CodeBuilder::compile_expr_USHR(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count){
	return compile_bin2(e, stack_top, result);
}

int_t CodeBuilder::compile_comp_bin2(const ExprPtr& e, int_t stack_top, int_t result){
	if(is_comp_bin(e->bin_lhs()) || is_comp_bin(e->bin_rhs())){
		error_->error(lineno(), Xt("XCE1025"));
	}

	int_t label_false = reserve_label();
	put_if_code(e, label_false, stack_top);

	int_t label_true = reserve_label();
	put_inst(InstLoadValue(result, LOAD_TRUE));

	set_jump(InstGoto::OFFSET_address, label_true);
	put_inst(InstGoto());

	set_label(label_false);

	put_inst(InstLoadValue(result, LOAD_FALSE));

	set_label(label_true);

	return 1;
}

void CodeBuilder::put_if_code(int_t tag, int_t target, int_t lhs, int_t rhs, inst_t label_true, int_t label_false, int_t stack_top){
	if(EXPR_EQ==tag){
		put_inst(InstIfComp(IF_COMP_EQ, lhs, rhs, stack_top));

		set_jump(InstIf::OFFSET_address_true, label_true);
		set_jump(InstIf::OFFSET_address_false, label_false);
		put_inst(InstIf(target, 0, 0));
	}
	else if(EXPR_NE==tag){
		put_inst(InstIfComp(IF_COMP_EQ, lhs, rhs, stack_top));

		set_jump(InstIf::OFFSET_address_false, label_true);
		set_jump(InstIf::OFFSET_address_true, label_false);
		put_inst(InstIf(target, 0, 0));
	}
	else if(EXPR_LT==tag){
		put_inst(InstIfComp(IF_COMP_LT, lhs, rhs, stack_top));

		set_jump(InstIf::OFFSET_address_true, label_true);
		set_jump(InstIf::OFFSET_address_false, label_false);
		put_inst(InstIf(target, 0, 0));
	}
	else if(EXPR_GT==tag){
		put_inst(InstIfComp(IF_COMP_LT, rhs, lhs, stack_top));

		set_jump(InstIf::OFFSET_address_true, label_true);
		set_jump(InstIf::OFFSET_address_false, label_false);
		put_inst(InstIf(target, 0, 0));
	}
	else if(EXPR_GE==tag){
		put_inst(InstIfComp(IF_COMP_LT, lhs, rhs, stack_top));

		set_jump(InstIf::OFFSET_address_false, label_true);
		set_jump(InstIf::OFFSET_address_true, label_false);
		put_inst(InstIf(target, 0, 0));
	}
	else if(EXPR_LE==tag){
		put_inst(InstIfComp(IF_COMP_LT, rhs, lhs, stack_top));

		set_jump(InstIf::OFFSET_address_false, label_true);
		set_jump(InstIf::OFFSET_address_true, label_false);
		put_inst(InstIf(target, 0, 0));
	}
	else if(EXPR_RAWEQ==tag){
		put_inst(InstIfRawEq(lhs, rhs, stack_top));

		set_jump(InstIf::OFFSET_address_true, label_true);
		set_jump(InstIf::OFFSET_address_false, label_false);
		put_inst(InstIf(target, 0, 0));
	}
	else if(EXPR_RAWNE==tag){
		put_inst(InstIfRawEq(lhs, rhs, stack_top));

		set_jump(InstIf::OFFSET_address_false, label_true);
		set_jump(InstIf::OFFSET_address_true, label_false);
		put_inst(InstIf(target, 0, 0));
	}
	else if(EXPR_IN==tag){
		put_inst(InstIfComp(IF_COMP_IN, lhs, rhs, stack_top));

		set_jump(InstIf::OFFSET_address_true, label_true);
		set_jump(InstIf::OFFSET_address_false, label_false);
		put_inst(InstIf(target, 0, 0));
	}
	else if(EXPR_NIN==tag){
		put_inst(InstIfComp(IF_COMP_IN, lhs, rhs, stack_top));

		set_jump(InstIf::OFFSET_address_false, label_true);
		set_jump(InstIf::OFFSET_address_true, label_false);
		put_inst(InstIf(target, 0, 0));
	}
	else if(EXPR_IS==tag){
		put_inst(InstIfIs(lhs, rhs, stack_top));

		set_jump(InstIf::OFFSET_address_true, label_true);
		set_jump(InstIf::OFFSET_address_false, label_false);
		put_inst(InstIf(target, 0, 0));
	}
	else if(EXPR_NIS==tag){
		put_inst(InstIfIs(lhs, rhs, stack_top));

		set_jump(InstIf::OFFSET_address_false, label_true);
		set_jump(InstIf::OFFSET_address_true, label_false);
		put_inst(InstIf(target, 0, 0));
	}
	else{
		XTAL_ASSERT(false);
	}
}

void CodeBuilder::put_if_code(const ExprPtr& e, int_t label_false, int_t stack_top){
	AnyPtr value = do_expr(e);
	if(value){
		return;
	}

	int_t label_true = reserve_label();

	if(is_comp_bin(e)){
		int_t target = stack_top++;
		int_t lhs = compile_expr(e->bin_lhs(), stack_top);
		int_t rhs = compile_expr(e->bin_rhs(), stack_top);
		put_if_code(e->itag(), target, lhs, rhs, label_true, label_false, stack_top);
	}
	else{
		if(e->itag()==EXPR_NOT){
			int_t lhs = compile_expr(e->una_term(), stack_top);
			set_jump(InstIf::OFFSET_address_false, label_true);
			set_jump(InstIf::OFFSET_address_true, label_false);
			put_inst(InstIf(lhs, 0, 0));
		}
		else{
			int_t lhs = compile_expr(e, stack_top);
			set_jump(InstIf::OFFSET_address_true, label_true);
			set_jump(InstIf::OFFSET_address_false, label_false);
			put_inst(InstIf(lhs, 0, 0));
		}
	}

	set_label(label_true);
}

int_t CodeBuilder::compile_expr_EQ(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count){
	int_t ret = compile_comp_bin2(e, stack_top, result);
	return ret;
}

int_t CodeBuilder::compile_expr_NE(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count){
	int_t ret = compile_comp_bin2(e, stack_top, result);
	return ret;
}

int_t CodeBuilder::compile_expr_LT(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count){
	int_t ret = compile_comp_bin2(e, stack_top, result);
	return ret;
}

int_t CodeBuilder::compile_expr_LE(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count){
	int_t ret = compile_comp_bin2(e, stack_top, result);
	return ret;
}

int_t CodeBuilder::compile_expr_GT(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count){
	int_t ret = compile_comp_bin2(e, stack_top, result);
	return ret;
}

int_t CodeBuilder::compile_expr_GE(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count){
	int_t ret = compile_comp_bin2(e, stack_top, result);
	return ret;
}

int_t CodeBuilder::compile_expr_RAWEQ(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count){
	int_t ret = compile_comp_bin2(e, stack_top, result);
	return ret;
}

int_t CodeBuilder::compile_expr_RAWNE(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count){
	int_t ret = compile_comp_bin2(e, stack_top, result);
	return ret;
}

int_t CodeBuilder::compile_expr_IN(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count){
	int_t ret = compile_comp_bin2(e, stack_top, result);
	return ret;
}

int_t CodeBuilder::compile_expr_NIN(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count){
	int_t ret = compile_comp_bin2(e, stack_top, result);
	return ret;
}

int_t CodeBuilder::compile_expr_IS(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count){
	int_t ret = compile_comp_bin2(e, stack_top, result);
	return ret;
}

int_t CodeBuilder::compile_expr_NIS(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count){
	int_t ret = compile_comp_bin2(e, stack_top, result);
	return ret;
}

int_t CodeBuilder::compile_expr_ANDAND(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count){
	int_t label_true = reserve_label();
	int_t label_false = reserve_label();

	compile_expr(e->bin_lhs(), stack_top, result);
	
	set_jump(InstIf::OFFSET_address_true, label_true);
	set_jump(InstIf::OFFSET_address_false, label_false);
	put_inst(InstIf(result, 0, 0));
	set_label(label_true);
	
	compile_expr(e->bin_rhs(), stack_top, result);
	
	set_label(label_false);
	return 1;
}

int_t CodeBuilder::compile_expr_OROR(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count){
	int_t label_true = reserve_label();
	int_t label_false = reserve_label();

	compile_expr(e->bin_lhs(), stack_top, result);
	
	set_jump(InstIf::OFFSET_address_true, label_true);
	set_jump(InstIf::OFFSET_address_false, label_false);
	put_inst(InstIf(result, 0, 0));
	set_label(label_false);
	
	compile_expr(e->bin_rhs(), stack_top, result);
		
	set_label(label_true);
	return 1;
}

int_t CodeBuilder::compile_expr_CATCH(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count){
	int_t end_label = reserve_label();

	int_t n = result_->except_info_table_.size();
	result_->except_info_table_.push_back(ExceptInfo());
	put_inst(InstTryBegin(n));

	compile_expr(e->catch_body(), stack_top, result);

	put_inst(InstTryEnd());

	// catch節のコードを埋め込む
	{
		result_->except_info_table_[n].catch_pc = code_size();

		// 例外を受け取るために変数スコープを構築
		var_begin(VarFrame::FRAME);
		var_define(e->catch_catch_var(), null, 0, true, false, true);
		scope_begin();

		put_inst(InstPop(stack_top));
		compile_lassign(stack_top, e->catch_catch_var());

		put_inst(InstPop(stack_top));
		compile_expr(e->catch_catch(), stack_top+1, stack_top);
		put_inst(InstPush(stack_top));
		
		scope_end();
		var_end();

		put_inst(InstPop(result));
	}
		
	set_label(end_label);
	result_->except_info_table_[n].finally_pc = code_size();
	result_->except_info_table_[n].end_pc = code_size();
	return 1;
}

int_t CodeBuilder::compile_expr_POS(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count){
	compile_expr(e->una_term(), stack_top, result);
	put_inst(InstUna(result, UNA_POS, result, stack_top));
	return 1;
}

int_t CodeBuilder::compile_expr_NEG(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count){
	compile_expr(e->una_term(), stack_top, result);
	put_inst(InstUna(result, UNA_NEG, result, stack_top));
	return 1;
}

int_t CodeBuilder::compile_expr_COM(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count){
	compile_expr(e->una_term(), stack_top, result);
	put_inst(InstUna(result, UNA_COM, result, stack_top));
	return 1;
}

int_t CodeBuilder::compile_expr_NOT(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count){
	compile_expr(e->una_term(), stack_top, result);
	put_inst(InstNot(result, result));
	return 1;
}

int_t CodeBuilder::compile_expr_RANGE(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count){
	int_t lhs = stack_top++; compile_expr(e->range_lhs(), stack_top, lhs);
	int_t rhs = stack_top++; compile_expr(e->range_rhs(), stack_top, rhs);
	put_inst(InstRange(result, (int_t)e->range_kind(), lhs, rhs, stack_top));
	return 1;
}

int_t CodeBuilder::compile_expr_FUN(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count){
	return compile_fun(e, stack_top, result);
}

int_t CodeBuilder::compile_expr_CLASS(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count){
	compile_class(e, stack_top, result);
	return 1;
}

int_t CodeBuilder::compile_expr_ONCE(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count){
	int_t label_end = reserve_label();
	
	set_jump(InstOnce::OFFSET_address, label_end);
	int_t num = result_->once_table_->size();
	result_->once_table_->push_back(undefined);
	put_inst(InstOnce(result, 0, num));
				
	compile_expr(e->una_term(), stack_top, result);
	put_inst(InstSetOnce(result, num));
	
	set_label(label_end);

	return 1;
}

int_t CodeBuilder::compile_expr_IVAR(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count){
	put_instance_variable_code(result, e->ivar_name());
	return 1;
}

int_t CodeBuilder::compile_expr_LVAR(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count){
	LVarInfo var = var_find(e->lvar_name());
	if(var.pos>=0){
		entry(var).referenced = true;

		if(var.pos<=0xff && !var.out_of_fun){
			put_inst(InstCopy(result, var.vpos));
		}
		else{
			put_inst(InstLocalVariable(result, var.pos, var.depth));
		}
	}
	else{
		int_t id = register_identifier(e->lvar_name());
		put_inst(InstFilelocalVariable(result, id));
		implicit_ref_map_->set_at(id, lineno());
	}

	return 1;
}

int_t CodeBuilder::compile_expr_AT(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count){
	int_t lhs = compile_expr(e->bin_lhs(), stack_top);
	int_t rhs = compile_expr(e->bin_rhs(), stack_top);
	put_inst(InstAt(result, lhs, rhs, stack_top));

	return 1;
}

int_t CodeBuilder::compile_expr_Q(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count){
	int_t label_true = reserve_label();
	int_t label_false = reserve_label();
	int_t label_end = reserve_label();

	compile_expr(e->q_cond(), stack_top, result);

	set_jump(InstIf::OFFSET_address_true, label_true);
	set_jump(InstIf::OFFSET_address_false, label_false);
	put_inst(InstIf(result, 0, 0));

	set_label(label_true);	

	compile_expr(e->q_true(), stack_top, result);

	set_jump(InstGoto::OFFSET_address, label_end);
	put_inst(InstGoto());

	set_label(label_false);
	
	compile_expr(e->q_false(), stack_top, result);
	
	set_label(label_end);
	return 1;
}


int_t CodeBuilder::compile_member(const AnyPtr& eterm, const AnyPtr& eprimary, const AnyPtr& esecondary, int_t flags, int_t stack_top, int_t result){
	int_t target = compile_expr(eterm, stack_top);
	int_t primary = 0;
	int_t secondary = 0;

	if(const IDPtr& id = ptr_cast<ID>(eprimary)){
		primary = register_identifier(id);
	}
	else{
		primary = stack_top++; compile_expr(eprimary, stack_top, primary);
		compile_property(primary, Xid(to_s), null, 0, stack_top, primary, 1);
		compile_property(primary, Xid(intern), null, 0, stack_top, primary, 1);
		flags |= MEMBER_FLAG_P_BIT;
	}

	if(esecondary){
		secondary = compile_expr(esecondary, stack_top);
		flags |= MEMBER_FLAG_S_BIT;
	}

	put_inst(InstMember(result, target, primary, secondary, flags));

	return 1;
}

int_t CodeBuilder::compile_property(const AnyPtr& eterm, const AnyPtr& eprimary, const AnyPtr& esecondary, int_t flags, int_t stack_top, int_t result, int_t result_count){
	return compile_send(eterm, eprimary, esecondary, null, null, flags, stack_top, result, result_count);
}	

int_t CodeBuilder::compile_set_property(const AnyPtr& eterm, const AnyPtr& eprimary, const AnyPtr& esecondary, const AnyPtr& set, int_t flags, int_t stack_top, int_t result, int_t result_count){
	eb_.push(null);
	eb_.push(set);
	eb_.splice(EXPR_LIST, 2);
	eb_.splice(EXPR_LIST, 1);
	ExprPtr args = ep(eb_.pop());

	if(ptr_cast<Expr>(eprimary)){ 
		eb_.push(KIND_STRING);
		eb_.push(Xid(set_));
		eb_.splice(EXPR_STRING, 2);
		eb_.push(ep(eprimary));
		eb_.splice(EXPR_CAT, 2);
		return compile_send(eterm, eb_.pop(), esecondary, args, null, flags, stack_top, result, result_count);
	}
	else{
		return compile_send(eterm, Xid(set_)->cat(ptr_cast<ID>(eprimary))->intern(), esecondary, args, null, flags, stack_top, result, result_count);
	}
}

int_t CodeBuilder::compile_send(const AnyPtr& eterm, const AnyPtr& eprimary, const AnyPtr& esecondary, const ExprPtr& args, const ExprPtr& eargs, int_t flags, int_t stack_top, int_t result, int_t result_count){
	int_t target = compile_expr(eterm, stack_top);
	int_t primary = 0;
	int_t secondary = 0;

	if(const IDPtr& id = ptr_cast<ID>(eprimary)){
		primary = register_identifier(id);
	}
	else{
		primary = stack_top++; compile_expr(eprimary, stack_top, primary);
		compile_property(primary, Xid(to_s), null, 0, stack_top, primary, 1);
		compile_property(primary, Xid(intern), null, 0, stack_top, primary, 1);
		flags |= MEMBER_FLAG_P_BIT;
	}

	if(esecondary){
		secondary = compile_expr(esecondary, stack_top);
		flags |= MEMBER_FLAG_S_BIT;
	}

	int_t stack_base = stack_top;
	int_t ordered = 0;
	int_t named = 0;

	if(args){
		Xfor_cast(const ExprPtr& v, args){
			if(v->at(0)){
				named++;
			}
			else{
				if(named!=0){
					error_->error(lineno(), Xt("XCE1005"));
				}

				ordered++;

				compile_expr(v->at(1), stack_top+1, stack_top);
				stack_top++;
			}
		}
		
		Xfor_cast(const ExprPtr& v, args){
			if(v->at(0)){
				const ExprPtr& k = ptr_cast<Expr>(v->at(0));

				put_inst(InstLoadConstant(stack_top++, register_value(k->lvar_name())));
				compile_expr(v->at(1), stack_top+1, stack_top);
				stack_top++;
			}
		}
	}

	if(eargs){
		compile_expr(eargs, stack_top+1, stack_top);
		flags |= CALL_FLAG_ARGS_BIT;
	}

	if(flags==0 && named==0){
		if(result_count==1 && ordered==0){
			put_inst(InstProperty(result, target, primary, stack_base));
			return 1;
		}

		if(result_count==0 && ordered==1){
			put_inst(InstSetProperty(target, primary, stack_base));
			return 0;
		}
	}
		
	put_inst(InstSend(result, result_count, target, primary, secondary, stack_base, ordered, named, flags));
	return result_count;
}

int_t CodeBuilder::compile_call(int_t target, int_t self, const ExprPtr& args, const ExprPtr& eargs, int_t flags, int_t stack_top, int_t result, int_t result_count){
	int_t stack_base = stack_top;
	int_t ordered = 0;
	int_t named = 0;

	if(args){
		Xfor_cast(const ExprPtr& v, args){
			if(v->at(0)){
				named++;
			}
			else{
				if(named!=0){
					error_->error(lineno(), Xt("XCE1005"));
				}

				ordered++;

				compile_expr(ep(v->at(1)), stack_top+1, stack_top);
				stack_top++;
			}
		}
		
		Xfor_cast(const ExprPtr& v,args){
			if(v->at(0)){
				const ExprPtr& k = ptr_cast<Expr>(v->at(0));

				put_inst(InstLoadConstant(stack_top++, register_value(k->lvar_name())));
				compile_expr(ep(v->at(1)), stack_top+1, stack_top);
				stack_top++;
			}
		}
	}

	if(eargs){
		compile_expr(eargs, stack_top+1, stack_top);
		flags |= CALL_FLAG_ARGS_BIT;
	}
		
	put_inst(InstCall(result, result_count, target, self, stack_base, ordered, named, flags));

	return result_count;
}

int_t CodeBuilder::compile_expr_MEMBER(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count){
	return compile_member(e->member_term(), e->member_name(), e->member_ns(), 0, stack_top, result);
}

int_t CodeBuilder::compile_expr_MEMBER_Q(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count){
	return compile_member(e->member_term(), e->member_name(), e->member_ns(), MEMBER_FLAG_Q_BIT, stack_top, result);
}

int_t CodeBuilder::compile_expr_PROPERTY(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count){
	return compile_property(e->property_term(), e->property_name(), e->property_ns(), 0, stack_top, result, result_count);
}

int_t CodeBuilder::compile_expr_PROPERTY_Q(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count){
	return compile_property(e->property_term(), e->property_name(), e->property_ns(), MEMBER_FLAG_Q_BIT, stack_top, result, result_count);
}

int_t CodeBuilder::compile_expr_CALL(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count){
	ExprPtr cterm = e->call_term();

	if(cterm->itag()==EXPR_PROPERTY){
		return compile_send(cterm->property_term(), cterm->property_name(), cterm->property_ns(), e->call_args(), e->call_extendable_arg(), 0, stack_top, result, result_count);
	}
	else if(cterm->itag()==EXPR_PROPERTY_Q){
		return compile_send(cterm->property_term(), cterm->property_name(), cterm->property_ns(), e->call_args(), e->call_extendable_arg(), MEMBER_FLAG_Q_BIT, stack_top, result, result_count);
	}
	else{
		int_t target = compile_expr(e->call_term(), stack_top);
		return compile_call(target, 0, e->call_args(), e->call_extendable_arg(), 0, stack_top, result, result_count);
	}
}

int_t CodeBuilder::compile_expr_YIELD(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count){
	int_t count = 0;
	Xfor(v, e->yield_exprs()){
		int_t temp = stack_top+1;
		compile_expr(v, temp, stack_top);
		put_inst(InstPush(stack_top));
		count++;
	}
		
	put_inst(InstYield(result, result_count, count));
	return result_count;
}

int_t CodeBuilder::compile_expr_RETURN(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count){
	int_t count = 0;
	Xfor(v, e->return_exprs()){
		int_t temp = stack_top+1;
		compile_expr(v, temp, stack_top);
		put_inst(InstPush(stack_top));
		count++;
	}

	break_off(ff().var_frame_count+1);
	put_inst(InstReturn(count));
	if(count>=256){
		error_->error(lineno(), Xt("XCE1022"));
	}	
	
	return 0;
}

int_t CodeBuilder::compile_expr_INC(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count){
	compile_incdec(e, stack_top);
	return 0;
}

int_t CodeBuilder::compile_expr_DEC(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count){
	compile_incdec(e, stack_top);
	return 0;
}

int_t CodeBuilder::compile_expr_ADD_ASSIGN(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count){
	compile_op_assign(e, stack_top);
	return 0;
}

int_t CodeBuilder::compile_expr_SUB_ASSIGN(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count){
	compile_op_assign(e, stack_top);
	return 0;
}

int_t CodeBuilder::compile_expr_CAT_ASSIGN(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count){
	compile_op_assign(e, stack_top);
	return 0;
}

int_t CodeBuilder::compile_expr_MUL_ASSIGN(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count){
	compile_op_assign(e, stack_top);
	return 0;
}

int_t CodeBuilder::compile_expr_DIV_ASSIGN(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count){
	compile_op_assign(e, stack_top);
	return 0;
}

int_t CodeBuilder::compile_expr_MOD_ASSIGN(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count){
	compile_op_assign(e, stack_top);
	return 0;
}

int_t CodeBuilder::compile_expr_AND_ASSIGN(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count){
	compile_op_assign(e, stack_top);
	return 0;
}

int_t CodeBuilder::compile_expr_OR_ASSIGN(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count){
	compile_op_assign(e, stack_top);
	return 0;
}

int_t CodeBuilder::compile_expr_XOR_ASSIGN(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count){
	compile_op_assign(e, stack_top);
	return 0;
}

int_t CodeBuilder::compile_expr_SHL_ASSIGN(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count){
	compile_op_assign(e, stack_top);
	return 0;
}

int_t CodeBuilder::compile_expr_SHR_ASSIGN(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count){
	compile_op_assign(e, stack_top);
	return 0;
}

int_t CodeBuilder::compile_expr_USHR_ASSIGN(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count){
	compile_op_assign(e, stack_top);
	return 0;
}

int_t CodeBuilder::compile_expr_ASSERT(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count){
	int_t label_end = reserve_label();

	set_jump(InstIfDebug::OFFSET_address, label_end);
	put_inst(InstIfDebug());

	if(ExprPtr e2 = e->assert_cond()){
		switch(e2->itag()){
		XTAL_DEFAULT{
			int_t target = compile_expr(e2, stack_top);

			int_t label_true = reserve_label();
			set_jump(InstIf::OFFSET_address_false, label_true);
			set_jump(InstIf::OFFSET_address_true, label_end);
			put_inst(InstIf(target, 0, 0));

			set_label(label_true);

			int_t vart = stack_top++;
			put_inst(InstLoadConstant(vart, register_value(Xf("%s : %s"))));

			int_t strt = stack_top++;
			if(e->assert_string()){ compile_expr(e->assert_string(), stack_top, strt); }
			else{ put_inst(InstLoadConstant(strt, register_value(empty_string))); }
			
			int_t mest = stack_top++;
			if(e->assert_message()){ compile_expr(e->assert_message(), stack_top, mest); }
			else{ put_inst(InstLoadConstant(mest, register_value(empty_string))); }

			put_inst(InstCall(target, 1, vart, 0, vart+1, 2, 0, 0));
			put_inst(InstAssert(target));
		}

		XTAL_CASE(EXPR_EQ){ compile_comp_bin_assert(Xf("%s : ![%s == %s] : %s"), e2, e->assert_string(), e->assert_message(), label_end, stack_top); }
		XTAL_CASE(EXPR_NE){ compile_comp_bin_assert(Xf("%s : ![%s !=  %s] : %s"), e2, e->assert_string(), e->assert_message(), label_end, stack_top); }
		XTAL_CASE(EXPR_LT){ compile_comp_bin_assert(Xf("%s : ![%s <  %s] : %s"), e2, e->assert_string(), e->assert_message(), label_end, stack_top); }
		XTAL_CASE(EXPR_GT){ compile_comp_bin_assert(Xf("%s : ![%s >  %s] : %s"), e2, e->assert_string(), e->assert_message(), label_end, stack_top); }
		XTAL_CASE(EXPR_LE){ compile_comp_bin_assert(Xf("%s : ![%s <=  %s] : %s"), e2, e->assert_string(), e->assert_message(), label_end, stack_top); }
		XTAL_CASE(EXPR_GE){ compile_comp_bin_assert(Xf("%s : ![%s >=  %s] : %s"), e2, e->assert_string(), e->assert_message(), label_end, stack_top); }
		XTAL_CASE(EXPR_RAWEQ){ compile_comp_bin_assert(Xf("%s : ![%s ===  %s] : %s"), e2, e->assert_string(), e->assert_message(), label_end, stack_top); }
		XTAL_CASE(EXPR_RAWNE){ compile_comp_bin_assert(Xf("%s : ![%s !==  %s] : %s"), e2, e->assert_string(), e->assert_message(), label_end, stack_top); }
		XTAL_CASE(EXPR_IN){ compile_comp_bin_assert(Xf("%s : ![%s in  %s] : %s"), e2, e->assert_string(), e->assert_message(), label_end, stack_top); }
		XTAL_CASE(EXPR_NIN){ compile_comp_bin_assert(Xf("%s : ![%s !in  %s] : %s"), e2, e->assert_string(), e->assert_message(), label_end, stack_top); }
		XTAL_CASE(EXPR_IS){ compile_comp_bin_assert(Xf("%s : ![%s is  %s] : %s"), e2, e->assert_string(), e->assert_message(), label_end, stack_top); }
		XTAL_CASE(EXPR_NIS){ compile_comp_bin_assert(Xf("%s : ![%s !is  %s] : %s"), e2, e->assert_string(), e->assert_message(), label_end, stack_top); }
		}
	}
	else{		
		int_t target = stack_top++;

		int_t vart = stack_top++;
		put_inst(InstLoadConstant(vart, register_value(Xf("%s : %s"))));

		int_t strt = stack_top++;
		if(e->assert_string()){ compile_expr(e->assert_string(), stack_top, strt); }
		else{ put_inst(InstLoadConstant(strt, register_value(empty_string))); }
		
		int_t mest = stack_top++;
		if(e->assert_message()){ compile_expr(e->assert_message(), stack_top, mest); }
		else{ put_inst(InstLoadConstant(mest, register_value(empty_string))); }

		put_inst(InstCall(target, 1, vart, 0, vart+1, 2, 0, 0));
		put_inst(InstAssert(target));
	}

	set_label(label_end);
	
	return 0;
}

int_t CodeBuilder::compile_expr_THROW(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count){
	compile_expr(e->una_term(), stack_top, result);
	put_inst(InstPush(result));
	put_inst(InstThrow());
	return 0;
}

int_t CodeBuilder::compile_expr_TRY(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count){
	int_t finally_label = reserve_label();
	int_t end_label = reserve_label();

	int_t n = result_->except_info_table_.size();
	result_->except_info_table_.push_back(ExceptInfo());
	put_inst(InstTryBegin(n));

	CodeBuilder::FunFrame::Finally exc;
	exc.frame_count = var_frames_.size();
	exc.finally_label = finally_label;
	ff().finallies.push(exc);

	compile_stmt(e->try_body());
	
	set_jump(InstPushGoto::OFFSET_address, end_label);
	put_inst(InstPushGoto());
	put_inst(InstTryEnd());

	// catch節のコードを埋め込む
	if(e->try_catch()){

		result_->except_info_table_[n].catch_pc = code_size();
		
		// catch節の中での例外に備え、例外フレームを構築。

		int_t n2 = result_->except_info_table_.size();
		result_->except_info_table_.push_back(ExceptInfo());
		put_inst(InstTryBegin(n2));

		CodeBuilder::FunFrame::Finally exc;
		exc.frame_count = var_frames_.size();
		exc.finally_label = finally_label;
		ff().finallies.push(exc);

		// 例外を受け取るために変数スコープを構築
		var_begin(VarFrame::FRAME);
		var_define(e->try_catch_var(), null, 0, true, false, true);
		check_lvar_assign_stmt(e->try_catch());
		scope_begin();
		scope_chain(1);

		put_inst(InstPop(stack_top));
		compile_lassign(stack_top, e->try_catch_var());
		compile_stmt(e->try_catch());
		
		scope_end();
		var_end();

		put_inst(InstTryEnd());
		ff().finallies.pop();

		result_->except_info_table_[n2].finally_pc = code_size();
		result_->except_info_table_[n2].end_pc = code_size();
	}
	
	set_label(finally_label);

	result_->except_info_table_[n].finally_pc = code_size();

	// finally節のコードを埋め込む
	compile_stmt(e->try_finally());
	
	ff().finallies.pop();

	put_inst(InstPopGoto());

	set_label(end_label);
	result_->except_info_table_[n].end_pc = code_size();
	return 0;
}

int_t CodeBuilder::compile_expr_IF(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count){
	ExprPtr cond = e->if_cond();

	// 条件式の部分が変数定義式である場合
	if(cond->itag()==EXPR_DEFINE && cond->bin_lhs()->itag()==EXPR_LVAR){
		// スコープを形成する
		var_begin(VarFrame::FRAME);
		var_define(cond->bin_lhs()->lvar_name());
		check_lvar_assign_stmt(e);
		scope_begin();

		LVarInfo var = var_find(cond->bin_lhs()->lvar_name(), true);
		entry(var).initialized = false;
		if(var.pos>=0){
			compile_expr(cond->bin_rhs(), stack_top, var.vpos);
			entry(var).initialized = true;
		}
		else{
			XTAL_ASSERT(false);
		}

		// 変数参照を条件式とする
		cond = cond->bin_lhs();
	}


	AnyPtr val = do_expr(cond);
	if(rawne(val, undefined)){
		if(val){
			compile_stmt(e->if_body());
		}
		else{
			compile_stmt(e->if_else());
		}
	}
	else{
		int_t label_false = reserve_label();
		int_t label_end = reserve_label();

		put_if_code(cond, label_false, stack_top);

		compile_stmt(e->if_body());
		
		if(e->if_else()){
			set_jump(InstGoto::OFFSET_address, label_end);
			put_inst(InstGoto());
		}
		
		set_label(label_false);
		compile_stmt(e->if_else());
	
		set_label(label_end);
	}

	if(rawne(cond, e->if_cond())){
		scope_end();
		var_end();
	}
	
	return 0;
}

int_t CodeBuilder::compile_expr_FOR(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count){
	var_begin(VarFrame::FRAME);
	var_define_stmt(e->for_init());
	var_define(Xid(first_step), null, 0, true, true);
	check_lvar_assign_stmt(e);
	scope_begin();
	
	LVarInfo first_step_info = var_find(Xid(first_step));

	int_t label_cond = reserve_label();
	int_t label_false_q = reserve_label();
	int_t label_false = reserve_label();
	int_t label_break = reserve_label();
	int_t label_continue = reserve_label();
	int_t label_body = reserve_label();

	FunFrame::Loop loop;
	loop.control_statement_label[0] = label_break;
	loop.control_statement_label[1] = label_continue;
	loop.label = e->for_label();
	loop.frame_count = var_frames_.size();
	loop.have_label = false;
	ff().loops.push(loop);

	compile_stmt(e->for_init());

	// 条件式をコンパイル
	if(e->for_cond()){
		put_inst(InstLoadValue(first_step_info.vpos, LOAD_TRUE));
		put_if_code(e->for_cond(), label_false, stack_top);
	}

	set_label(label_body);

	// ループ本体をコンパイル
	compile_stmt(e->for_body());

	set_label(label_continue);

	// next部をコンパイル
	if(e->for_next()){
		compile_stmt(e->for_next());
	}

	set_label(label_cond);

	// 条件式をコンパイル 2回目
	if(e->for_cond()){
		put_inst(InstLoadValue(first_step_info.vpos, LOAD_FALSE));
		put_if_code(e->for_cond(), label_false_q, stack_top);
	}

	/*if(referenced_first_step){
		// ループ本体をコンパイル 2回目
		compile_stmt(e->for_body());

		// label_continue部分にジャンプ
		set_jump(InstGoto::OFFSET_address, label_continue);
		put_inst(InstGoto());
	}
	else*/{
		// label_body部分にジャンプ
		set_jump(InstGoto::OFFSET_address, label_body);
		put_inst(InstGoto());
	}
	
	ff().loops.pop();
	
	set_label(label_false);
	if(e->for_else()){
		compile_stmt(e->for_else());
	}

	set_label(label_false_q);
	if(e->for_nobreak()){
		compile_stmt(e->for_nobreak());
	}

	set_label(label_break);

	scope_end();
	var_end();
	return 0;
}

int_t CodeBuilder::compile_expr_MASSIGN(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count){
	ExprPtr lhs = e->massign_lhs_exprs();
	ExprPtr rhs = e->massign_rhs_exprs();

	int_t lhs_stack_base = stack_top;

	// 左辺をすべて評価する
	for(uint_t i=0; i<lhs->size(); ++i){
		ExprPtr term = ep(lhs->at(i));

		if(term->itag()==EXPR_LVAR){

		}
		else if(term->itag()==EXPR_IVAR){

		}
		else if(term->itag()==EXPR_PROPERTY){
			int_t nterm = stack_top++; compile_expr(term->property_term(), stack_top, nterm);

			if(term->property_ns()){
				int_t sec = stack_top++; compile_expr(term->property_ns(), stack_top, sec);
			}
			else{
				int_t sec = stack_top++; put_inst(InstLoadValue(sec, LOAD_UNDEFINED));
			}
		}
		else if(term->itag()==EXPR_PROPERTY_Q){
			int_t nterm = stack_top++; compile_expr(term->property_term(), stack_top, nterm);

			if(term->property_ns()){
				int_t sec = stack_top++; compile_expr(term->property_ns(), stack_top, sec);
			}
			else{
				int_t sec = stack_top++; put_inst(InstLoadValue(sec, LOAD_UNDEFINED));
			}
		}
		else if(term->itag()==EXPR_AT){
			int_t var = stack_top++; compile_expr(term->bin_lhs(), stack_top, var); 
			int_t key = stack_top++; compile_expr(term->bin_rhs(), stack_top, key); 
		}
	}

	// スタックトップを保存
	int_t rhs_stack_base = stack_top;

	// 右辺をすべて評価する
	for(uint_t i=0; i<lhs->size(); ++i){
		// 右辺最後の要素
		if(i==rhs->size()-1){
			int_t rrc = lhs->size() - i;
			compile_expr(rhs->at(i), stack_top+1, stack_top, rrc);
			stack_top += rrc;
			break;
		}
		// 左辺最後の要素
		else if(i==lhs->size()-1){
			int_t stack_base2 = stack_top;
			for(; i<rhs->size(); ++i){
				compile_expr(rhs->at(i), stack_top+1, stack_top);
				stack_top++;
			}

			if(rhs->size()!=lhs->size()){
				put_inst(InstAdjustValues(stack_base2, rhs->size()-(lhs->size()-1), 1));
			}
			break;
		}
		else{
			compile_expr(rhs->at(i), stack_top+1, stack_top);
			stack_top++;
		}
	}


	// 左辺に代入する
	for(uint_t i=0; i<lhs->size(); ++i){
		ExprPtr term = ep(lhs->at(i));

		if(term->itag()==EXPR_LVAR){
			int_t target = rhs_stack_base++;
			compile_lassign(target, term->ivar_name());
		}
		else if(term->itag()==EXPR_IVAR){
			int_t target = rhs_stack_base++;
			put_set_instance_variable_code(target, term->ivar_name());
		}
		else if(term->itag()==EXPR_PROPERTY){
			int_t nterm = lhs_stack_base++;
			int_t sec = lhs_stack_base++;
			int_t target = rhs_stack_base++;
			compile_set_property(nterm, term->property_name(), sec, target, 0, stack_top, target);
		}
		else if(term->itag()==EXPR_PROPERTY_Q){
			int_t nterm = lhs_stack_base++;
			int_t sec = lhs_stack_base++;
			int_t target = rhs_stack_base++;
			compile_set_property(nterm, term->property_name(), sec, target, MEMBER_FLAG_Q_BIT, stack_top, target);
		}
		else if(term->itag()==EXPR_AT){
			int_t var = lhs_stack_base++;
			int_t key = lhs_stack_base++;
			int_t target = rhs_stack_base++;
			put_inst(InstSetAt(var, key, target, stack_top));
		}
		else{
			error_->error(lineno(), Xt("XCE1012"));
		}
	}

	return 0;
}

int_t CodeBuilder::compile_expr_MDEFINE(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count){
	ExprPtr lhs = e->massign_lhs_exprs();
	ExprPtr rhs = e->massign_rhs_exprs();

	int_t lhs_stack_base = stack_top;

	// 左辺をすべて評価する
	for(uint_t i=0; i<lhs->size(); ++i){
		ExprPtr term = ep(lhs->at(i));

		if(term->itag()==EXPR_LVAR){

		}
		else if(term->itag()==EXPR_MEMBER){
			int_t nterm = stack_top++; compile_expr(term->member_term(), stack_top, nterm);

			if(ptr_cast<ID>(term->member_name())){
			
			}
			else{
				int_t primary = stack_top++; compile_expr(term->member_name(), stack_top, primary);
				compile_property(primary, Xid(to_s), null, 0, stack_top, primary, 1);
				compile_property(primary, Xid(intern), null, 0, stack_top, primary, 1);
			}

			if(term->member_ns()){
				int_t sec = stack_top++; compile_expr(term->member_ns(), stack_top, sec);
			}
			else{
				int_t sec = stack_top++; put_inst(InstLoadValue(sec, LOAD_UNDEFINED));
			}
		}
	}

	// スタックトップを保存
	int_t rhs_stack_base = stack_top;

	// 右辺をすべて評価する
	for(uint_t i=0; i<lhs->size(); ++i){
		// 右辺最後の要素
		if(i==rhs->size()-1){
			int_t rrc = lhs->size() - i;
			compile_expr(rhs->at(i), stack_top+1, stack_top, rrc);
			stack_top += rrc;
			break;
		}
		// 左辺最後の要素
		else if(i==lhs->size()-1){
			int_t stack_base2 = stack_top;
			for(; i<rhs->size(); ++i){
				compile_expr(rhs->at(i), stack_top+1, stack_top);
				stack_top++;
			}

			if(rhs->size()!=lhs->size()){
				put_inst(InstAdjustValues(stack_base2, rhs->size()-(lhs->size()-1), 1));
			}
			break;
		}
		else{
			compile_expr(rhs->at(i), stack_top+1, stack_top);
			stack_top++;
		}
	}

	// 左辺に代入する
	for(uint_t i=0; i<lhs->size(); ++i){
		ExprPtr term = ep(lhs->at(i));

		if(term->itag()==EXPR_LVAR){
			int_t target = rhs_stack_base++;
			LVarInfo var = var_find(term->lvar_name(), true);

			if(var.pos>=0){
				put_inst(InstCopy(var.vpos, target));
			}
			else{
				XTAL_ASSERT(false);
			}
		}
		else if(term->itag()==EXPR_MEMBER){
			int_t flags = 0;
			int_t nterm = lhs_stack_base++;

			int_t primary;
			if(const IDPtr& id = ptr_cast<ID>(term->member_name())){
				primary = register_identifier(id);
			}
			else{
				primary = lhs_stack_base++;
				flags |= MEMBER_FLAG_P_BIT;		
			}

			int_t sec = lhs_stack_base++;
			if(term->member_ns()){
				flags |= MEMBER_FLAG_S_BIT;		
			}
			
			int_t target = rhs_stack_base++;
			put_inst(InstDefineMember(nterm, primary, sec, flags, target));
		}
		else{
			error_->error(lineno(), Xt("XCE1012"));
		}
	}

	return 0;
}

void CodeBuilder::compile_lassign(int_t target, const IDPtr& var){
	LVarInfo varinfo = var_find(var);
	if(varinfo.pos>=0){
		if(entry(varinfo).constant){
			error_->error(lineno(), Xt("XCE1019")->call(Named(Xid(name), var)));
		}

		if(varinfo.pos<=0xff && !varinfo.out_of_fun){
			put_inst(InstCopy(varinfo.vpos, target));
		}
		else{
			put_inst(InstSetLocalVariable(target, varinfo.pos, varinfo.depth));
		}

		entry(varinfo).value = undefined;
	}
	else{
		error_->error(lineno(), Xt("XCE1009")->call(Named(Xid(name), var)));
	}	
}

int_t CodeBuilder::compile_expr_ASSIGN(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count){
	if(e->bin_lhs()->itag()==EXPR_LVAR){
		LVarInfo var = var_find(e->bin_lhs()->lvar_name());
		if(var.pos>=0 && var.pos<=0xff && !var.out_of_fun){
			compile_expr(e->bin_rhs(), stack_top, var.vpos);
		}
		else{
			int_t target = compile_expr(e->bin_rhs(), stack_top);
			compile_lassign(target, e->bin_lhs()->lvar_name());
		}
	}
	else if(e->bin_lhs()->itag()==EXPR_IVAR){
		int_t target = compile_expr(e->bin_rhs(), stack_top);
		put_set_instance_variable_code(target, e->bin_lhs()->ivar_name());
	}
	else if(e->bin_lhs()->itag()==EXPR_PROPERTY){
		compile_set_property(e->bin_lhs()->property_term(), e->bin_lhs()->property_name(), e->bin_lhs()->property_ns(),
			e->bin_rhs(), 0, stack_top, result, result_count);
	}
	else if(e->bin_lhs()->itag()==EXPR_PROPERTY_Q){
		compile_set_property(e->bin_lhs()->property_term(), e->bin_lhs()->property_name(), e->bin_lhs()->property_ns(),
			e->bin_rhs(), MEMBER_FLAG_Q_BIT, stack_top, result, result_count);
	}
	else if(e->bin_lhs()->itag()==EXPR_AT){
		int_t target = compile_expr(e->bin_lhs()->bin_lhs(), stack_top);
		int_t key = compile_expr(e->bin_lhs()->bin_rhs(), stack_top);
		int_t value = compile_expr(e->bin_rhs(), stack_top);
		put_inst(InstSetAt(target, key, value, stack_top));
	}
	else{
		error_->error(lineno(), Xt("XCE1012"));
	}

	return 0;
}

int_t CodeBuilder::compile_expr_DEFINE(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count){
	if(e->bin_lhs()->itag()==EXPR_LVAR){
		LVarInfo var = var_find(e->bin_lhs()->lvar_name(), true);
		entry(var).initialized = false;
		if(var.pos>=0){
			compile_expr(e->bin_rhs(), stack_top, var.vpos);
			entry(var).initialized = true;
		}
		else{
			XTAL_ASSERT(false);
		}
	}
	else if(e->bin_lhs()->itag()==EXPR_MEMBER){
		int_t lhs = compile_expr(e->bin_lhs()->member_term(), stack_top);
		int_t rhs = compile_expr(e->bin_rhs(), stack_top);

		int_t flags = 0;
		int_t primary = 0;
		int_t secondary = 0;

		if(const IDPtr& id = ptr_cast<ID>(e->bin_lhs()->member_name())){
			primary = register_identifier(id);
		}
		else{
			primary = stack_top++; compile_expr(e->bin_lhs()->member_name(), stack_top, primary);
			compile_property(primary, Xid(to_s), null, 0, stack_top, primary, 1);
			compile_property(primary, Xid(intern), null, 0, stack_top, primary, 1);
			flags |= MEMBER_FLAG_P_BIT;		
		}

		if(e->bin_lhs()->member_ns()){
			secondary = compile_expr(e->bin_lhs()->member_ns(), stack_top);
			flags |= MEMBER_FLAG_S_BIT;		
		}
		
		put_inst(InstDefineMember(lhs, primary, secondary, flags, rhs));
	}
	else{
		error_->error(lineno(), Xt("XCE1012"));
	}

	return 0;
}

int_t CodeBuilder::compile_expr_CDEFINE_MEMBER(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count){
	return 0;
}

int_t CodeBuilder::compile_expr_CDEFINE_IVAR(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count){
	return 0;
}

int_t CodeBuilder::compile_expr_BREAK(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count){
	compile_loop_control_statement(e);
	return 0;
}

int_t CodeBuilder::compile_expr_CONTINUE(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count){
	compile_loop_control_statement(e);
	return 0;
}

int_t CodeBuilder::compile_expr_BRACKET(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count){
	return compile_expr(e->una_term(), stack_top, result, result_count);
}

int_t CodeBuilder::compile_expr_SCOPE(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count){
	var_begin(VarFrame::FRAME);
	var_define_stmts(e->scope_stmts());
	check_lvar_assign_stmt(e);
	scope_begin();{
		Xfor(v, e->scope_stmts()){
			compile_stmt(v);
		}
	}scope_end();
	var_end();
	return 0;
}

int_t CodeBuilder::compile_expr_SWITCH(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count){
	ExprPtr cond = e->switch_cond();

	// 条件式の部分が変数定義式である場合
	if(cond->itag()==EXPR_DEFINE && cond->bin_lhs()->itag()==EXPR_LVAR){
		// スコープを形成する
		var_begin(VarFrame::FRAME);
		var_define(cond->bin_lhs()->lvar_name());
		check_lvar_assign_stmt(e);
		scope_begin();
		LVarInfo var = var_find(cond->bin_lhs()->lvar_name(), true);
		entry(var).initialized = false;
		if(var.pos>=0){
			compile_expr(cond->bin_rhs(), stack_top, var.vpos);
			entry(var).initialized = true;
		}
		else{
			XTAL_ASSERT(false);
		}

		// 変数参照を条件式とする
		cond = cond->bin_lhs();
	}

	int_t label_jump = reserve_label();
	int_t label_end = reserve_label();
	int_t label_default = reserve_label();

	int_t base = stack_top++;

	set_jump(InstOnce::OFFSET_address, label_jump);
	int_t num = result_->once_table_->size();
	result_->once_table_->push_back(undefined);
	put_inst(InstOnce(base, 0, num));

	MapPtr case_map = xnew<Map>();
	ExprPtr default_case = e->switch_default();
	Xfor_cast(const ExprPtr& v, e->switch_cases()){
		Xfor_cast(const ExprPtr& k, v->at(0)){
			case_map->set_at(k, v->at(1));
		}
	}

	MapPtr jump_map = xnew<Map>();
	Xfor2(k, v, case_map){
		XTAL_UNUSED_VAR(v);
		int_t jump_to = reserve_label();
		jump_map->set_at(k, jump_to);
	}

	put_inst(InstMakeMap(result));
	Xfor2(k, v, case_map){
		XTAL_UNUSED_VAR(v);
		compile_expr(k, stack_top+1, stack_top);
		set_jump(InstPushGoto::OFFSET_address, jump_map->at(k)->to_i());
		put_inst(InstPushGoto());
		put_inst(InstPop(stack_top+1));
		put_inst(InstMapInsert(base, stack_top, stack_top+1));
	}

	set_jump(InstPushGoto::OFFSET_address, label_default);
	put_inst(InstPushGoto());
	put_inst(InstPop(stack_top+1));
	put_inst(InstMapSetDefault(base, stack_top+1));
				
	put_inst(InstSetOnce(base, num));
	
	set_label(label_jump);

	compile_expr(cond, stack_top+1, stack_top);
	put_inst(InstAt(base, base, stack_top, stack_top+1));
	put_inst(InstPush(base));
	put_inst(InstPopGoto());

	Xfor2(k, v, case_map){
		set_label(jump_map->at(k)->to_i());
		compile_stmt(v);
		set_jump(InstGoto::OFFSET_address, label_end);
		put_inst(InstGoto());
	}

	set_label(label_default);

	compile_stmt(default_case);
	set_jump(InstGoto::OFFSET_address, label_end);
	put_inst(InstGoto());

	set_label(label_end);	

	if(rawne(cond, e->switch_cond())){
		scope_end();
		var_end();
	}

	return 0;
}

int_t CodeBuilder::compile_expr_TOPLEVEL(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count){
	var_begin(VarFrame::FRAME);
	var_define_stmts(e->toplevel_stmts());
	check_lvar_assign_stmt(e);
	scope_begin();{
		Xfor(v, e->toplevel_stmts()){
			if(ExprPtr e = ep(v)){
				if(e->itag()==EXPR_DEFINE){
					if(e->bin_lhs()->itag()==EXPR_LVAR && (e->bin_rhs()->itag()==EXPR_CLASS || e->bin_rhs()->itag()==EXPR_FUN)){
						ExprPtr lhs = e->bin_lhs();
						eb_.push(Xid(filelocal));
						eb_.splice(EXPR_LVAR, 1);
						eb_.push(lhs->lvar_name());
						eb_.push(null);
						eb_.splice(EXPR_MEMBER, 3);
						e->set_bin_lhs(ep(eb_.pop()));
						compile_stmt(e);

						e->set_bin_lhs(lhs);
						eb_.push(Xid(filelocal));
						eb_.splice(EXPR_LVAR, 1);
						eb_.push(lhs->lvar_name());
						eb_.push(null);
						eb_.splice(EXPR_MEMBER, 3);
						e->set_bin_rhs(ep(eb_.pop()));
						compile_stmt(e);

						continue;
					}
				}
			}

			compile_stmt(v);
		}
	}scope_end();
	var_end();

	return 0;
}

//}}STMT_IMPLS}

}

#endif

