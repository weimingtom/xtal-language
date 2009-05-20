#include "xtal.h"
#include "xtal_macro.h"

#include "xtal_codebuilder.h"

#ifndef XTAL_NO_PARSER

namespace xtal{

void CodeBuilder::compile_expr(const AnyPtr& p, const CompileInfo& info){
	AnyPtr val;
	if(compile_expr(p, info, val)){
		return;
	}	

	ExprPtr e = ep(p);
	
	if(e->lineno()!=0){
		linenos_.push(e->lineno());
		result_->set_lineno_info(e->lineno());
	}

	int_t result_count = compile_e(e, info);
	if(info.need_result_count!=result_count){
		put_inst(InstAdjustResult(result_count, info.need_result_count));
	}

	if(e->lineno()!=0){
		result_->set_lineno_info(e->lineno());
		linenos_.pop();
	}
}
	
void CodeBuilder::compile_stmt(const AnyPtr& p){
	if(!p){
		return;
	}

	ExprPtr e = ep(p);

	if(e->lineno()!=0){
		linenos_.push(e->lineno());
		result_->set_lineno_info(e->lineno());
	}

	int_t result_count = compile_e(e, 0);
	if(result_count!=0){
		put_inst(InstAdjustResult(result_count, 0));
	}

	if(e->lineno()!=0){
		result_->set_lineno_info(e->lineno());
		linenos_.pop();
	}
}

int_t CodeBuilder::compile_e(const ExprPtr& e, const CompileInfo& info){
	typedef int_t (CodeBuilder::*expr_compile_fun_t)(const ExprPtr& e, const CompileInfo& info);
	static expr_compile_fun_t expr_compile_fun_[EXPR_MAX] ={
//{STMT_TABLE{{
		&CodeBuilder::compile_expr_LIST,
		&CodeBuilder::compile_expr_NULL,
		&CodeBuilder::compile_expr_UNDEFINED,
		&CodeBuilder::compile_expr_TRUE,
		&CodeBuilder::compile_expr_FALSE,
		&CodeBuilder::compile_expr_CALLEE,
		&CodeBuilder::compile_expr_THIS,
		&CodeBuilder::compile_expr_CURRENT_CONTEXT,
		&CodeBuilder::compile_expr_NUMBER,
		&CodeBuilder::compile_expr_STRING,
		&CodeBuilder::compile_expr_ARRAY,
		&CodeBuilder::compile_expr_MAP,
		&CodeBuilder::compile_EXPR_VALUES,
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

	return (this->*expr_compile_fun_[e->itag()])(e, info);
}

//{STMT_IMPLS{{
int_t CodeBuilder::compile_expr_LIST(const ExprPtr& e, const CompileInfo& info){
	//compile_expr(e, 0);
	return 0;
}

int_t CodeBuilder::compile_expr_NULL(const ExprPtr& e, const CompileInfo& info){
	put_inst(InstPushNull());
	return 1;
}

int_t CodeBuilder::compile_expr_UNDEFINED(const ExprPtr& e, const CompileInfo& info){
	put_inst(InstPushUndefined());
	return 1;
}

int_t CodeBuilder::compile_expr_TRUE(const ExprPtr& e, const CompileInfo& info){
	put_inst(InstPushTrue());
	return 1;
}

int_t CodeBuilder::compile_expr_FALSE(const ExprPtr& e, const CompileInfo& info){
	put_inst(InstPushFalse());
	return 1;
}

int_t CodeBuilder::compile_expr_CALLEE(const ExprPtr& e, const CompileInfo& info){
	put_inst(InstPushCallee());
	return 1;
}

int_t CodeBuilder::compile_expr_THIS(const ExprPtr& e, const CompileInfo& info){
	put_inst(InstPushThis());
	return 1;
}

int_t CodeBuilder::compile_expr_CURRENT_CONTEXT(const ExprPtr& e, const CompileInfo& info){
	put_inst(InstPushCurrentContext());
	return 1;
}

int_t CodeBuilder::compile_expr_NUMBER(const ExprPtr& e, const CompileInfo& info){
	AnyPtr nvalue = e->number_value();
	if(type(nvalue)==TYPE_INT){
		int_t value = ivalue(nvalue);
		if(value==(i8)value){ 
			put_inst(InstPushInt1Byte(value));
		}
		else{ 
			put_inst(InstValue(register_value(value)));
		}
	}
	else{
		float_t value = fvalue(nvalue);
		if(value==(i8)value){ 
			put_inst(InstPushFloat1Byte((i8)value));
		}
		else{ 
			put_inst(InstValue(register_value(value)));
		}
	}
	return 1;
}

int_t CodeBuilder::compile_expr_STRING(const ExprPtr& e, const CompileInfo& info){
	if(e->string_kind()==KIND_TEXT){
		put_inst(InstValue(register_value(text(e->string_value()->to_s()))));
	}
	else if(e->string_kind()==KIND_FORMAT){
		put_inst(InstValue(register_value(format(e->string_value()->to_s()))));
	}
	else{
		put_inst(InstValue(register_value(e->string_value())));
	}
	return 1;
}

int_t CodeBuilder::compile_expr_ARRAY(const ExprPtr& e, const CompileInfo& info){
	put_inst(InstMakeArray());
	Xfor(v, e->array_values()){
		compile_expr(v);
		put_inst(InstArrayAppend());				
	}
	return 1;
}

int_t CodeBuilder::compile_expr_MAP(const ExprPtr& e, const CompileInfo& info){
	put_inst(InstMakeMap());
	Xfor_cast(const ArrayPtr& v, e->map_values()){
		compile_expr(v->at(0));
		compile_expr(v->at(1));
		put_inst(InstMapInsert());				
	}
	return 1;
}

int_t CodeBuilder::compile_EXPR_VALUES(const ExprPtr& e, const CompileInfo& info){
	if(e->multi_value_exprs()){
		Xfor(v, e->multi_value_exprs()){
			compile_expr(v);		
		}	
		return e->multi_value_exprs()->size();
	}
	return 0;
}

int_t CodeBuilder::compile_expr_ADD(const ExprPtr& e, const CompileInfo& info){
	compile_bin(e);
	return 1;
}

int_t CodeBuilder::compile_expr_SUB(const ExprPtr& e, const CompileInfo& info){
	compile_bin(e);
	return 1;
}

int_t CodeBuilder::compile_expr_CAT(const ExprPtr& e, const CompileInfo& info){
	compile_bin(e);
	return 1;
}

int_t CodeBuilder::compile_expr_MUL(const ExprPtr& e, const CompileInfo& info){
	compile_bin(e);
	return 1;
}

int_t CodeBuilder::compile_expr_DIV(const ExprPtr& e, const CompileInfo& info){
	compile_bin(e);
	return 1;
}

int_t CodeBuilder::compile_expr_MOD(const ExprPtr& e, const CompileInfo& info){
	compile_bin(e);
	return 1;
}

int_t CodeBuilder::compile_expr_AND(const ExprPtr& e, const CompileInfo& info){
	compile_bin(e);
	return 1;
}

int_t CodeBuilder::compile_expr_OR(const ExprPtr& e, const CompileInfo& info){
	compile_bin(e);
	return 1;
}

int_t CodeBuilder::compile_expr_XOR(const ExprPtr& e, const CompileInfo& info){
	compile_bin(e);
	return 1;
}

int_t CodeBuilder::compile_expr_SHL(const ExprPtr& e, const CompileInfo& info){
	compile_bin(e);
	return 1;
}

int_t CodeBuilder::compile_expr_SHR(const ExprPtr& e, const CompileInfo& info){
	compile_bin(e);
	return 1;
}

int_t CodeBuilder::compile_expr_USHR(const ExprPtr& e, const CompileInfo& info){
	compile_bin(e);
	return 1;
}

int_t CodeBuilder::compile_expr_EQ(const ExprPtr& e, const CompileInfo& info){
	compile_comp_bin(e);
	return 1;
}

int_t CodeBuilder::compile_expr_NE(const ExprPtr& e, const CompileInfo& info){
	compile_comp_bin(e);
	return 1;
}

int_t CodeBuilder::compile_expr_LT(const ExprPtr& e, const CompileInfo& info){
	compile_comp_bin(e);
	return 1;
}

int_t CodeBuilder::compile_expr_LE(const ExprPtr& e, const CompileInfo& info){
	compile_comp_bin(e);
	return 1;
}

int_t CodeBuilder::compile_expr_GT(const ExprPtr& e, const CompileInfo& info){
	compile_comp_bin(e);
	return 1;
}

int_t CodeBuilder::compile_expr_GE(const ExprPtr& e, const CompileInfo& info){
	compile_comp_bin(e);
	return 1;
}

int_t CodeBuilder::compile_expr_RAWEQ(const ExprPtr& e, const CompileInfo& info){
	compile_comp_bin(e);
	return 1;
}

int_t CodeBuilder::compile_expr_RAWNE(const ExprPtr& e, const CompileInfo& info){
	compile_comp_bin(e);
	return 1;
}

int_t CodeBuilder::compile_expr_IN(const ExprPtr& e, const CompileInfo& info){
	compile_comp_bin(e);
	return 1;
}

int_t CodeBuilder::compile_expr_NIN(const ExprPtr& e, const CompileInfo& info){
	compile_comp_bin(e);
	return 1;
}

int_t CodeBuilder::compile_expr_IS(const ExprPtr& e, const CompileInfo& info){
	compile_comp_bin(e);
	return 1;
}

int_t CodeBuilder::compile_expr_NIS(const ExprPtr& e, const CompileInfo& info){
	compile_comp_bin(e);
	return 1;
}

int_t CodeBuilder::compile_expr_ANDAND(const ExprPtr& e, const CompileInfo& info){
	int_t label_true = reserve_label();
	int_t label_false = reserve_label();

	compile_expr(e->bin_lhs());

	put_inst(InstDup());
	
	set_jump(InstIf::OFFSET_address_true, label_true);
	set_jump(InstIf::OFFSET_address_false, label_false);
	put_inst(InstIf());
	set_label(label_true);

	put_inst(InstPop());
	
	compile_expr(e->bin_rhs());
	
	set_label(label_false);
	return 1;
}

int_t CodeBuilder::compile_expr_OROR(const ExprPtr& e, const CompileInfo& info){
	int_t label_true = reserve_label();
	int_t label_false = reserve_label();

	compile_expr(e->bin_lhs());

	put_inst(InstDup());
	
	set_jump(InstIf::OFFSET_address_true, label_true);
	set_jump(InstIf::OFFSET_address_false, label_false);
	put_inst(InstIf());

	set_label(label_false);
	
	put_inst(InstPop());
	
	compile_expr(e->bin_rhs());
	
	set_label(label_true);
	return 1;
}

int_t CodeBuilder::compile_expr_CATCH(const ExprPtr& e, const CompileInfo& cinfo){
	int_t end_label = reserve_label();

	int_t info = result_->except_info_table_.size();
	result_->except_info_table_.push_back(ExceptInfo());
	put_inst(InstTryBegin(info));

	compile_expr(e->catch_body());

	put_inst(InstTryEnd());

	// catch節のコードを埋め込む
	{
		result_->except_info_table_[info].catch_pc = code_size();

		// 例外を受け取るために変数スコープを構築
		var_begin(VarFrame::FRAME);
		var_define(e->catch_catch_var(), null, 0, true, false, true);
		scope_begin();

		put_set_local_code(e->catch_catch_var());
		put_inst(InstPop());
		compile_expr(e->catch_catch());
		
		scope_end();
		var_end();
	}
		
	set_label(end_label);
	result_->except_info_table_[info].finally_pc = code_size();
	result_->except_info_table_[info].end_pc = code_size();
	return 1;
}

int_t CodeBuilder::compile_expr_POS(const ExprPtr& e, const CompileInfo& info){
	compile_expr(e->una_term()); 
	put_inst(InstPos());
	return 1;
}

int_t CodeBuilder::compile_expr_NEG(const ExprPtr& e, const CompileInfo& info){
	compile_expr(e->una_term()); 
	put_inst(InstNeg());
	return 1;
}

int_t CodeBuilder::compile_expr_COM(const ExprPtr& e, const CompileInfo& info){
	compile_expr(e->una_term()); 
	put_inst(InstCom());
	return 1;
}

int_t CodeBuilder::compile_expr_NOT(const ExprPtr& e, const CompileInfo& info){
	compile_expr(e->una_term()); 
	put_inst(InstNot());
	return 1;
}

int_t CodeBuilder::compile_expr_RANGE(const ExprPtr& e, const CompileInfo& info){
	compile_expr(e->range_lhs());
	compile_expr(e->range_rhs());
	put_inst(InstRange((int_t)e->range_kind()));
	return 1;
}

int_t CodeBuilder::compile_expr_FUN(const ExprPtr& e, const CompileInfo& info){
	compile_fun(e);
	return 1;
}

int_t CodeBuilder::compile_expr_CLASS(const ExprPtr& e, const CompileInfo& info){
	compile_class(e);
	return 1;
}

int_t CodeBuilder::compile_expr_ONCE(const ExprPtr& e, const CompileInfo& info){
	int_t label_end = reserve_label();
	
	set_jump(InstOnce::OFFSET_address, label_end);
	int_t num = result_->once_table_->size();
	result_->once_table_->push_back(undefined);
	put_inst(InstOnce(0, num));
				
	compile_expr(e->una_term());
	put_inst(InstDup());		
	put_inst(InstSetOnce(num));
	
	set_label(label_end);
	return 1;
}

int_t CodeBuilder::compile_expr_IVAR(const ExprPtr& e, const CompileInfo& info){
	put_instance_variable_code(e->ivar_name());
	return 1;
}

int_t CodeBuilder::compile_expr_LVAR(const ExprPtr& e, const CompileInfo& info){
	put_local_code(e->lvar_name());
	return 1;
}

int_t CodeBuilder::compile_expr_AT(const ExprPtr& e, const CompileInfo& info){
	compile_expr(e->bin_lhs());
	compile_expr(e->bin_rhs());
	put_inst(InstAt());
	return 1;
}

int_t CodeBuilder::compile_expr_Q(const ExprPtr& e, const CompileInfo& info){
	int_t label_true = reserve_label();
	int_t label_false = reserve_label();
	int_t label_end = reserve_label();

	compile_expr(e->q_cond());

	set_jump(InstIf::OFFSET_address_true, label_true);
	set_jump(InstIf::OFFSET_address_false, label_false);
	put_inst(InstIf());

	set_label(label_true);	

	compile_expr(e->q_true());

	set_jump(InstGoto::OFFSET_address, label_end);
	put_inst(InstGoto());

	set_label(label_false);
	
	compile_expr(e->q_false());
	
	set_label(label_end);
	return 1;
}

int_t CodeBuilder::compile_expr_MEMBER(const ExprPtr& e, const CompileInfo& info){
	compile_expr(e->member_term());
	int_t key = register_identifier_or_compile_expr(e->member_name());

	if(e->member_ns()){
		compile_expr(e->member_ns());
		put_inst(InstMember(key, CALL_FLAG_NS));
	}
	else{
		put_inst(InstMember(key, CALL_FLAG_NONE));
	}	
	return 1;
}

int_t CodeBuilder::compile_expr_MEMBER_Q(const ExprPtr& e, const CompileInfo& info){
	compile_expr(e->member_term());
	int_t key = register_identifier_or_compile_expr(e->member_name());
	
	if(e->member_ns()){
		compile_expr(e->member_ns());
		put_inst(InstMember(key, CALL_FLAG_NS | CALL_FLAG_Q));
	}
	else{
		put_inst(InstMember(key, CALL_FLAG_Q));
	}	
	return 1;
}

int_t CodeBuilder::compile_expr_PROPERTY(const ExprPtr& e, const CompileInfo& info){
	compile_expr(e->property_term());
	put_send_code(e->property_name(), e->property_ns(), info.need_result_count, 0, 0, CALL_FLAG_NONE);
	return info.need_result_count;
}

int_t CodeBuilder::compile_expr_PROPERTY_Q(const ExprPtr& e, const CompileInfo& info){
	compile_expr(e->property_term());
	put_send_code(e->property_name(), e->property_ns(), info.need_result_count, 0, 0, CALL_FLAG_Q);
	return info.need_result_count;
}

int_t CodeBuilder::compile_expr_CALL(const ExprPtr& e, const CompileInfo& info){
	int_t ordered = 0;
	int_t named = 0;

	Xfor_cast(const ExprPtr& v, e->call_args()){
		if(v->at(0)){
			named++;
		}
		else{
			if(named!=0){
				error_->error(lineno(), Xt("Xtal Compile Error 1005"));
			}

			ordered++;
			compile_expr(ep(v->at(1)));
		}
	}
	
	Xfor_cast(const ExprPtr& v, e->call_args()){
		if(v->at(0)){
			const ExprPtr& k = ptr_cast<Expr>(v->at(0));
			put_inst(InstValue(register_value(k->lvar_name())));
			compile_expr(ep(v->at(1)));
		}
	}

	int_t flags = (info.tail ? CALL_FLAG_TAIL : 0) | (e->call_extendable_arg() ? CALL_FLAG_ARGS : 0);

	if(e->call_extendable_arg()){
		compile_expr(e->call_extendable_arg());
	}

	if(e->call_term()->itag()==EXPR_PROPERTY){ // a.b(); メッセージ送信式
		ExprPtr e2 = e->call_term();
		compile_expr(e2->property_term());

		put_send_code(e2->property_name(), e2->property_ns(), info.need_result_count, ordered, named, flags);
	}
	else if(e->call_term()->itag()==EXPR_PROPERTY_Q){ // a.?b(); メッセージ送信式
		ExprPtr e2 = e->call_term();
		compile_expr(e2->property_term());

		put_send_code(e2->property_name(), e2->property_ns(), info.need_result_count, ordered, named, flags | CALL_FLAG_Q);
	}
	else{
		compile_expr(e->call_term());
		put_inst(InstCall(ordered, named, info.need_result_count, flags));
	}

	return info.need_result_count;
}

int_t CodeBuilder::compile_expr_YIELD(const ExprPtr& e, const CompileInfo& info){
	int_t exprs_size = compile_exprs(e->yield_exprs());
	put_inst(InstYield(exprs_size));
	if(exprs_size>=256){
		error_->error(lineno(), Xt("Xtal Compile Error 1022"));
	}
	return info.need_result_count;
}

int_t CodeBuilder::compile_expr_INC(const ExprPtr& e, const CompileInfo& info){
	compile_incdec(e);
	return 0;
}

int_t CodeBuilder::compile_expr_DEC(const ExprPtr& e, const CompileInfo& info){
	compile_incdec(e);
	return 0;
}

int_t CodeBuilder::compile_expr_ADD_ASSIGN(const ExprPtr& e, const CompileInfo& info){
	compile_op_assign(e);
	return 0;
}

int_t CodeBuilder::compile_expr_SUB_ASSIGN(const ExprPtr& e, const CompileInfo& info){
	compile_op_assign(e);
	return 0;
}

int_t CodeBuilder::compile_expr_CAT_ASSIGN(const ExprPtr& e, const CompileInfo& info){
	compile_op_assign(e);
	return 0;
}

int_t CodeBuilder::compile_expr_MUL_ASSIGN(const ExprPtr& e, const CompileInfo& info){
	compile_op_assign(e);
	return 0;
}

int_t CodeBuilder::compile_expr_DIV_ASSIGN(const ExprPtr& e, const CompileInfo& info){
	compile_op_assign(e);
	return 0;
}

int_t CodeBuilder::compile_expr_MOD_ASSIGN(const ExprPtr& e, const CompileInfo& info){
	compile_op_assign(e);
	return 0;
}

int_t CodeBuilder::compile_expr_AND_ASSIGN(const ExprPtr& e, const CompileInfo& info){
	compile_op_assign(e);
	return 0;
}

int_t CodeBuilder::compile_expr_OR_ASSIGN(const ExprPtr& e, const CompileInfo& info){
	compile_op_assign(e);
	return 0;
}

int_t CodeBuilder::compile_expr_XOR_ASSIGN(const ExprPtr& e, const CompileInfo& info){
	compile_op_assign(e);
	return 0;
}

int_t CodeBuilder::compile_expr_SHL_ASSIGN(const ExprPtr& e, const CompileInfo& info){
	compile_op_assign(e);
	return 0;
}

int_t CodeBuilder::compile_expr_SHR_ASSIGN(const ExprPtr& e, const CompileInfo& info){
	compile_op_assign(e);
	return 0;
}

int_t CodeBuilder::compile_expr_USHR_ASSIGN(const ExprPtr& e, const CompileInfo& info){
	compile_op_assign(e);
	return 0;
}

int_t CodeBuilder::compile_expr_RETURN(const ExprPtr& e, const CompileInfo& info){
	bool have_finally = false;
	for(uint_t scope_count = var_frames_.size(); scope_count!=(uint_t)ff().var_frame_count+1; scope_count--){
		for(uint_t k = 0; k<(uint_t)ff().finallies.size(); ++k){
			if((uint_t)ff().finallies[k].frame_count==scope_count){
				have_finally = true;
			}
		}
	}

	int_t exprs_size = e->return_exprs() ? e->return_exprs()->size() : 0;

	/*
	if(!have_finally && exprs_size==1){
		ExprPtr front = ep(e->return_exprs()->front());
		if(front->itag()==EXPR_CALL || front->itag()==EXPR_PROPERTY || front->itag()==EXPR_PROPERTY_Q){
			compile_expr(front, CompileInfo(1, true));

			break_off(ff().var_frame_count+1);
			put_inst(InstReturn(exprs_size));
			if(exprs_size>=256){
				error_->error(lineno(), Xt("Xtal Compile Error 1022"));
			}	

			return 0;
		}
	}
	*/

	compile_exprs(e->return_exprs());

	break_off(ff().var_frame_count+1);
	put_inst(InstReturn(exprs_size));
	if(exprs_size>=256){
		error_->error(lineno(), Xt("Xtal Compile Error 1022"));
	}	
	return 0;
}

int_t CodeBuilder::compile_expr_ASSERT(const ExprPtr& e, const CompileInfo& info){
	int_t label_end = reserve_label();

	set_jump(InstIfDebug::OFFSET_address, label_end);
	put_inst(InstIfDebug());

	if(ExprPtr e2 = e->assert_cond()){
		switch(e2->itag()){
		XTAL_DEFAULT{
			compile_expr(e2);

			int_t label_true = reserve_label();
			set_jump(InstIf::OFFSET_address_false, label_true);
			set_jump(InstIf::OFFSET_address_true, label_end);
			put_inst(InstIf());

			set_label(label_true);

			if(e->assert_string()){ compile_expr(e->assert_string()); }
			else{ put_inst(InstValue(register_value(empty_string))); }
			if(e->assert_message()){ compile_expr(e->assert_message()); }
			else{ put_inst(InstValue(register_value(empty_string))); }
			
			put_inst(InstValue(register_value(Xf("%s : %s"))));
			put_inst(InstCall(2, 0, 1, 0));
			put_inst(InstAssert());
		}

		XTAL_CASE(EXPR_EQ){ compile_comp_bin_assert(Xf("%s : ![%s == %s] : %s"), e2, e->assert_string(), e->assert_message(), label_end); }
		XTAL_CASE(EXPR_NE){ compile_comp_bin_assert(Xf("%s : ![%s !=  %s] : %s"), e2, e->assert_string(), e->assert_message(), label_end); }
		XTAL_CASE(EXPR_LT){ compile_comp_bin_assert(Xf("%s : ![%s <  %s] : %s"), e2, e->assert_string(), e->assert_message(), label_end); }
		XTAL_CASE(EXPR_GT){ compile_comp_bin_assert(Xf("%s : ![%s >  %s] : %s"), e2, e->assert_string(), e->assert_message(), label_end); }
		XTAL_CASE(EXPR_LE){ compile_comp_bin_assert(Xf("%s : ![%s <=  %s] : %s"), e2, e->assert_string(), e->assert_message(), label_end); }
		XTAL_CASE(EXPR_GE){ compile_comp_bin_assert(Xf("%s : ![%s >=  %s] : %s"), e2, e->assert_string(), e->assert_message(), label_end); }
		XTAL_CASE(EXPR_RAWEQ){ compile_comp_bin_assert(Xf("%s : ![%s ===  %s] : %s"), e2, e->assert_string(), e->assert_message(), label_end); }
		XTAL_CASE(EXPR_RAWNE){ compile_comp_bin_assert(Xf("%s : ![%s !==  %s] : %s"), e2, e->assert_string(), e->assert_message(), label_end); }
		XTAL_CASE(EXPR_IN){ compile_comp_bin_assert(Xf("%s : ![%s in  %s] : %s"), e2, e->assert_string(), e->assert_message(), label_end); }
		XTAL_CASE(EXPR_NIN){ compile_comp_bin_assert(Xf("%s : ![%s !in  %s] : %s"), e2, e->assert_string(), e->assert_message(), label_end); }
		XTAL_CASE(EXPR_IS){ compile_comp_bin_assert(Xf("%s : ![%s is  %s] : %s"), e2, e->assert_string(), e->assert_message(), label_end); }
		XTAL_CASE(EXPR_NIS){ compile_comp_bin_assert(Xf("%s : ![%s !is  %s] : %s"), e2, e->assert_string(), e->assert_message(), label_end); }
		}
	}
	else{
		if(e->assert_string()){ compile_expr(e->assert_string()); }
		else{ put_inst(InstValue(register_value(empty_string))); }
		if(e->assert_message()){ compile_expr(e->assert_message()); }
		else{ put_inst(InstValue(register_value(empty_string))); }
		
		put_inst(InstValue(register_value(Xf("%s : %s"))));
		put_inst(InstCall(2, 0, 1, 0));
		put_inst(InstAssert());
	}

	set_label(label_end);
	return 0;
}

int_t CodeBuilder::compile_expr_THROW(const ExprPtr& e, const CompileInfo& info){
	compile_expr(e->una_term());
	put_inst(InstThrow());
	return 0;
}

int_t CodeBuilder::compile_expr_TRY(const ExprPtr& e, const CompileInfo& cinfo){
	int_t finally_label = reserve_label();
	int_t end_label = reserve_label();

	int_t info = result_->except_info_table_.size();
	result_->except_info_table_.push_back(ExceptInfo());
	put_inst(InstTryBegin(info));

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

		result_->except_info_table_[info].catch_pc = code_size();
		
		// catch節の中での例外に備え、例外フレームを構築。

		int_t info2 = result_->except_info_table_.size();
		result_->except_info_table_.push_back(ExceptInfo());
		put_inst(InstTryBegin(info2));

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

		put_set_local_code(e->try_catch_var());
		compile_stmt(e->try_catch());
		
		scope_end();
		var_end();

		put_inst(InstTryEnd());
		ff().finallies.pop();

		result_->except_info_table_[info2].finally_pc = code_size();
		result_->except_info_table_[info2].end_pc = code_size();
	}
	
	set_label(finally_label);

	result_->except_info_table_[info].finally_pc = code_size();

	// finally節のコードを埋め込む
	compile_stmt(e->try_finally());
	
	ff().finallies.pop();

	put_inst(InstPopGoto());

	set_label(end_label);
	result_->except_info_table_[info].end_pc = code_size();
	return 0;
}

int_t CodeBuilder::compile_expr_IF(const ExprPtr& e, const CompileInfo& info){
	ExprPtr cond = e->if_cond();

	// 条件式の部分が変数定義式である場合
	if(cond->itag()==EXPR_DEFINE && cond->bin_lhs()->itag()==EXPR_LVAR){
		// スコープを形成する
		var_begin(VarFrame::FRAME);
		var_define(cond->bin_lhs()->lvar_name());
		check_lvar_assign_stmt(e);
		scope_begin();
		put_define_local_code(cond->bin_lhs()->lvar_name(), cond->bin_rhs());

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

		put_if_code(cond, label_false);

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

int_t CodeBuilder::compile_expr_FOR(const ExprPtr& e, const CompileInfo& info){
	var_begin(VarFrame::FRAME);
	var_define_stmt(e->for_init());
	var_define(Xid(first_step), xnew<Expr>(EXPR_TRUE));
	check_lvar_assign_stmt(e);
	scope_begin();
	
	{
		LVarInfo info = var_find(Xid(first_step));
		entry(info).removed = true;
		entry(info).constant = true;
	}

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
		put_if_code(e->for_cond(), label_false);
	}

	set_label(label_body);

	// ループ本体をコンパイル
	compile_stmt(e->for_body());

	bool referenced_first_step;
	{
		LVarInfo info = var_find(Xid(first_step));
		entry(info).value = false;
		referenced_first_step = entry(info).referenced;
	}

	set_label(label_continue);

	// next部をコンパイル
	if(e->for_next()){
		compile_stmt(e->for_next());
	}

	set_label(label_cond);

	// 条件式をコンパイル 2回目
	if(e->for_cond()){
		put_if_code(e->for_cond(), label_false_q);
	}

	if(referenced_first_step){
		// ループ本体をコンパイル 2回目
		compile_stmt(e->for_body());

		// label_continue部分にジャンプ
		set_jump(InstGoto::OFFSET_address, label_continue);
		put_inst(InstGoto());
	}
	else{
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

int_t CodeBuilder::compile_expr_MASSIGN(const ExprPtr& e, const CompileInfo& info){
	uint_t pushed_count = 0;

	ExprPtr lhs = e->massign_lhs_exprs();
	ExprPtr rhs = e->massign_rhs_exprs();

	for(uint_t r=0; r<rhs->size(); ++r){	
		if(r==rhs->size()-1){
			int_t rrc;
			if(pushed_count<lhs->size()){
				rrc = lhs->size() - pushed_count;
			}
			else{
				rrc = 1;
			}

			compile_expr(rhs->at(r), rrc);
			pushed_count += rrc;
			break;
		}
		else{
			compile_expr(rhs->at(r));
			pushed_count++;
		}
	}

	if(lhs->size()!=pushed_count){
		put_inst(InstAdjustResult(pushed_count, lhs->size()));
	}

	Xfor(v1, lhs->reverse()){
		ExprPtr v = ep(v1);
		if(v->itag()==EXPR_LVAR){
			put_set_local_code(v->lvar_name());
		}
		else if(v->itag()==EXPR_PROPERTY){
			compile_expr(v->property_term());
			put_set_send_code(v->property_name(), v->property_ns(), CALL_FLAG_NONE);
		}
		else if(v->itag()==EXPR_PROPERTY_Q){
			compile_expr(v->property_term());
			put_set_send_code(v->property_name(), v->property_ns(), CALL_FLAG_Q);
		}
		else if(v->itag()==EXPR_IVAR){
			put_set_instance_variable_code(v->ivar_name());					
		}
		else if(v->itag()==EXPR_AT){
			compile_expr(v->bin_lhs());
			compile_expr(v->bin_rhs());
			put_inst(InstSetAt());
		}
		else{
			error_->error(lineno(), Xt("Xtal Compile Error 1008"));
		}
	}
	return 0;
}

int_t CodeBuilder::compile_expr_MDEFINE(const ExprPtr& e, const CompileInfo& info){
	uint_t pushed_count = 0;

	ExprPtr lhs = e->massign_lhs_exprs();
	ExprPtr rhs = e->massign_rhs_exprs();

	for(uint_t r=0; r<rhs->size(); ++r){	
		if(r==rhs->size()-1){
			int_t rrc;
			if(pushed_count<lhs->size()){
				rrc = lhs->size() - pushed_count;
			}
			else{
				rrc = 1;
			}

			compile_expr(rhs->at(r), rrc);
			pushed_count += rrc;
			break;
		}
		else{
			compile_expr(rhs->at(r));
			pushed_count++;
		}
	}

	if(lhs->size()!=pushed_count){
		put_inst(InstAdjustResult(pushed_count, lhs->size()));
	}

	Xfor(v1, lhs->reverse()){
		ExprPtr v = ep(v1);
		if(v->itag()==EXPR_LVAR){
			put_define_local_code(v->lvar_name());
		}
		else if(v->itag()==EXPR_MEMBER){
			compile_expr(v->member_term());
			put_define_member_code(v->member_name(), v->member_ns());
		}
		else{
			error_->error(lineno(), Xt("Xtal Compile Error 1008"));
		}
	}
	return 0;
}

int_t CodeBuilder::compile_expr_ASSIGN(const ExprPtr& e, const CompileInfo& info){
	if(e->bin_lhs()->itag()==EXPR_LVAR){
		compile_expr(e->bin_rhs());
		put_set_local_code(e->bin_lhs()->lvar_name());
	}
	else if(e->bin_lhs()->itag()==EXPR_IVAR){
		compile_expr(e->bin_rhs());
		put_set_instance_variable_code(e->bin_lhs()->ivar_name());
	}
	else if(e->bin_lhs()->itag()==EXPR_PROPERTY){
		compile_expr(e->bin_rhs());
		compile_expr(e->bin_lhs()->property_term());
		put_set_send_code(e->bin_lhs()->property_name(), e->bin_lhs()->property_ns(), CALL_FLAG_NONE);
	}
	else if(e->bin_lhs()->itag()==EXPR_PROPERTY_Q){
		compile_expr(e->bin_rhs());
		compile_expr(e->bin_lhs()->property_term());
		put_set_send_code(e->bin_lhs()->property_name(), e->bin_lhs()->property_ns(), CALL_FLAG_Q);
	}
	else if(e->bin_lhs()->itag()==EXPR_AT){
		compile_expr(e->bin_rhs());
		compile_expr(e->bin_lhs()->bin_lhs());
		compile_expr(e->bin_lhs()->bin_rhs());
		put_inst(InstSetAt());
	}
	else{
		error_->error(lineno(), Xt("Xtal Compile Error 1012"));
	}
	return 0;
}

int_t CodeBuilder::compile_expr_DEFINE(const ExprPtr& e, const CompileInfo& info){
	if(e->bin_lhs()->itag()==EXPR_LVAR){
		put_define_local_code(e->bin_lhs()->lvar_name(), e->bin_rhs());

	}
	else if(e->bin_lhs()->itag()==EXPR_MEMBER){
		compile_expr(e->bin_lhs()->member_term());
		compile_expr(e->bin_rhs());

		put_define_member_code(e->bin_lhs()->member_name(), e->bin_lhs()->member_ns());
	}
	else{
		error_->error(lineno(), Xt("Xtal Compile Error 1012"));
	}
	return 0;
}

int_t CodeBuilder::compile_expr_CDEFINE_MEMBER(const ExprPtr& e, const CompileInfo& info){

	return 0;
}

int_t CodeBuilder::compile_expr_CDEFINE_IVAR(const ExprPtr& e, const CompileInfo& info){

	return 0;
}

int_t CodeBuilder::compile_expr_BREAK(const ExprPtr& e, const CompileInfo& info){
	compile_loop_control_statement(e);
	return 0;
}

int_t CodeBuilder::compile_expr_CONTINUE(const ExprPtr& e, const CompileInfo& info){
	compile_loop_control_statement(e);
	return 0;
}

int_t CodeBuilder::compile_expr_BRACKET(const ExprPtr& e, const CompileInfo& info){
	compile_expr(e->una_term(), info.need_result_count);
	return info.need_result_count;
}

int_t CodeBuilder::compile_expr_SCOPE(const ExprPtr& e, const CompileInfo& info){
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

int_t CodeBuilder::compile_expr_SWITCH(const ExprPtr& e, const CompileInfo& info){
	ExprPtr cond = e->switch_cond();

	// 条件式の部分が変数定義式である場合
	if(cond->itag()==EXPR_DEFINE && cond->bin_lhs()->itag()==EXPR_LVAR){
		// スコープを形成する
		var_begin(VarFrame::FRAME);
		var_define(cond->bin_lhs()->lvar_name());
		check_lvar_assign_stmt(e);
		scope_begin();
		put_define_local_code(cond->bin_lhs()->lvar_name(), cond->bin_rhs());

		// 変数参照を条件式とする
		cond = cond->bin_lhs();
	}

	int_t label_jump = reserve_label();
	int_t label_end = reserve_label();
	int_t label_default = reserve_label();

	set_jump(InstOnce::OFFSET_address, label_jump);
	int_t num = result_->once_table_->size();
	result_->once_table_->push_back(undefined);
	put_inst(InstOnce(0, num));

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

	put_inst(InstMakeMap());
	Xfor2(k, v, case_map){
		XTAL_UNUSED_VAR(v);
		compile_expr(k);
		set_jump(InstPushGoto::OFFSET_address, jump_map->at(k)->to_i());
		put_inst(InstPushGoto());
		put_inst(InstMapInsert());
	}

	set_jump(InstPushGoto::OFFSET_address, label_default);
	put_inst(InstPushGoto());
	put_inst(InstMapSetDefault());
				
	put_inst(InstDup());
	put_inst(InstSetOnce(num));
	
	set_label(label_jump);

	compile_expr(cond);
	put_inst(InstAt());
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

int_t CodeBuilder::compile_expr_TOPLEVEL(const ExprPtr& e, const CompileInfo& info){
	var_begin(VarFrame::FRAME);
	var_define_stmts(e->toplevel_stmts());
	check_lvar_assign_stmt(e);
	scope_begin();{
		Xfor(v, e->toplevel_stmts()){
			compile_stmt(v);
		}
	}scope_end();
	var_end();

	return 0;
}

//}}STMT_IMPLS}

}

#endif

