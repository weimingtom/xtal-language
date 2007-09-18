
#include "xtal.h"

#ifndef XTAL_NO_PARSER

#include "xtal_codebuilder.h"
#include "xtal_expr.h"
#include "xtal_parser.h"
#include "xtal_code.h"
#include "xtal_stream.h"
#include "xtal_fun.h"
#include "xtal_vmachine.h"
#include "xtal_code.h"

namespace xtal{

extern AnyPtr once_value_none_;

CodeBuilder::CodeBuilder(){

}

CodeBuilder::~CodeBuilder(){

}

CodePtr CodeBuilder::compile(const StreamPtr& stream, const StringPtr& source_file_name){
	result_ = xnew<Code>();
	result_->source_file_name_ = source_file_name;
	result_->except_core_table_.push_back(ExceptCore());

	lines_.push(1);
	var_begin(VarFrame::BLOCK);
	fun_begin(KIND_FUN, true, 0, 0, 0);

	ExprPtr ep = parser_.parse(stream, source_file_name);

	result_->symbol_table_ = xnew<Array>();
	symbol_map_ = xnew<Map>();
	register_symbol("");

	result_->value_table_ = xnew<Array>();
	value_map_ = xnew<Map>();
	
	result_->once_table_ = xnew<Array>();
	result_->once_table_->push_back(once_value_none_);

	if(ep){
		compile_stmt(ep);
	}
	
	fun_end(0);
	var_end();

	parser_.release();
	CodePtr retval = result_;
	result_ = null;

	//if(errors->size()==0){
		retval->reset_core();
		return retval;
	//}else{
	//	return null;
	//}
}

void CodeBuilder::interactive_compile(){

}

void CodeBuilder::put_inst2(const Inst& t, uint_t sz){
	if(t.op==255){
		error(line(), Xt("Xtal Compile Error 1027"));
	}

	size_t cur = result_->code_.size();
	result_->code_.resize(cur+sz/sizeof(inst_t));
	memcpy(&result_->code_[cur], &t, sz);
}

ArrayPtr CodeBuilder::errors(){
	return xnew<Array>();
}

bool CodeBuilder::put_set_local_code(const InternedStringPtr& var){
	LVarInfo info = var_find(var);
	if(info.pos>=0){
		if(info.pos<=0xff){
			var_set_direct(*info.var_frame);
			put_inst(InstSetLocalVariable1Byte(info.pos));
		}else{
			put_inst(InstSetLocalVariable2Byte(info.pos));
		}		

		return true;
	}else{
		put_inst(InstSetGlobalVariable(register_symbol(var)));
		return false;
	}
}

void CodeBuilder::put_define_local_code(const InternedStringPtr& var){
	LVarInfo info = var_find(var, true);
	if(info.pos>=0){
		put_set_local_code(var);
	}else{
		put_inst(InstDefineGlobalVariable(register_symbol(var)));
	}
}

bool CodeBuilder::put_local_code(const InternedStringPtr& var){
	LVarInfo info = var_find(var);
	if(info.pos>=0){
		if(info.pos<=0xff){
			var_set_direct(*info.var_frame);
			put_inst(InstLocalVariable1Byte(info.pos));
		}else{
			put_inst(InstLocalVariable2Byte(info.pos));
		}		

		return true;
	}else{
		put_inst(InstGlobalVariable(register_symbol(var)));
		return false;
	}
}

void CodeBuilder::put_send_code(const InternedStringPtr& var, ExprPtr pvar, int_t need_result_count, bool tail, bool if_defined){
	if(pvar){
		compile(pvar);
	}	
	
	if(if_defined){
		if(tail){
			put_inst(InstSendIfDefined_T(0, 0, need_result_count, pvar ? 0 : register_symbol(var)));
		}else{
			put_inst(InstSendIfDefined(0, 0, need_result_count, pvar ? 0 : register_symbol(var)));
		}
	}else{
		if(tail){
			put_inst(InstSend_T(0, 0, need_result_count, pvar ? 0 : register_symbol(var)));
		}else{
			put_inst(InstSend(0, 0, need_result_count, pvar ? 0 : register_symbol(var))); 
		}
	}
}

void CodeBuilder::put_set_send_code(const InternedStringPtr& var, ExprPtr pvar, bool if_defined){
	if(pvar){
		compile(bin(0, EXPR_CAT, string(0, KIND_STRING, "set_"), pvar));
	}	
	
	InternedStringPtr symbol_number = xnew<String>("set_", 4)->cat(var);
	bool tail = false;

	if(if_defined){
		if(tail){
			put_inst(InstSendIfDefined_T(1, 0, 0, pvar ? 0 : register_symbol(symbol_number)));
		}else{
			put_inst(InstSendIfDefined(1, 0, 0, pvar ? 0 : register_symbol(symbol_number)));
		}
	}else{
		if(tail){
			put_inst(InstSend_T(1, 0, 0, pvar ? 0 : register_symbol(symbol_number)));
		}else{
			put_inst(InstSend(1, 0, 0, pvar ? 0 : register_symbol(symbol_number))); 
		}
	}
}

void CodeBuilder::put_member_code(const InternedStringPtr& var, ExprPtr pvar, bool if_defined){
	if(pvar){
		compile(pvar);
	}
	
	if(if_defined){
		InstMemberIfDefined member;
		member.symbol_number = pvar ? 0 : register_symbol(var);
		put_inst(member);
	}else{
		InstMember member;
		member.symbol_number = pvar ? 0 : register_symbol(var);
		put_inst(member);
	}
}

void CodeBuilder::put_define_member_code(const InternedStringPtr& var, ExprPtr pvar){
	if(pvar){
		compile(pvar);
	}

	InstDefineMember member;
	member.symbol_number = pvar ? 0 : register_symbol(var);
	put_inst(member);
}

int_t CodeBuilder::lookup_instance_variable(const InternedStringPtr& key){
	if(!class_frames_.empty()){
		int ret = 0;
		ClassFrame& cf = class_frames_.top();
		for(size_t i = 0, last = cf.entrys.size(); i<last; ++i){
			if(raweq(cf.entrys[i].name, key)){
				return ret;
			}
			ret++;
		}
	}
	error(line(), Xt("Xtal Compile Error 1023")(Named("name", xnew<String>("_")->cat(key))));
	return 0;
}

void CodeBuilder::put_set_instance_variable_code(const InternedStringPtr& var){
	put_inst(InstSetInstanceVariable(lookup_instance_variable(var), class_core_num()));
}

void CodeBuilder::put_instance_variable_code(const InternedStringPtr& var){
	put_inst(InstInstanceVariable(lookup_instance_variable(var), class_core_num()));
}

int_t CodeBuilder::reserve_label(){
	fun_frames_.top().labels.resize(fun_frames_.top().labels.size()+1);
	return fun_frames_.top().labels.size()-1;
}

void CodeBuilder::set_label(int_t labelno){
	fun_frames_.top().labels[labelno].pos = code_size();
}

void CodeBuilder::set_jump(int_t offset, int_t labelno){
	FunFrame::Label::From f;
	f.line = lines_.top();
	f.pos = code_size();
	f.set_pos = f.pos + offset/sizeof(inst_t);
	fun_frames_.top().labels[labelno].froms.push_back(f);
}

void CodeBuilder::process_labels(){
	for(size_t i = 0; i<fun_frames_.top().labels.size(); ++i){
		FunFrame::Label &l = fun_frames_.top().labels[i];
		for(size_t j = 0; j<l.froms.size(); ++j){
			FunFrame::Label::From &f = l.froms[j];
			inst_i16_t& buf = *(inst_i16_t*)&result_->code_[f.set_pos];
			buf = l.pos - f.pos;
		}
	}
	fun_frames_.top().labels.clear();
}

void CodeBuilder::break_off(int_t n){
	for(uint_t scope_count = var_frames_.size(); scope_count!=(uint_t)n; scope_count--){
		for(uint_t k = 0; k<fun_frame().finallys.size(); ++k){
			if((uint_t)fun_frame().finallys[k].frame_count==scope_count){
				int_t label = reserve_label();
				set_jump(offsetof(InstPushGoto, address), label);
				put_inst(InstPushGoto());
				put_inst(InstTryEnd());
				set_label(label);
			}
		}

		VarFrame& vf = var_frames_[var_frames_.size()-scope_count];
		if(vf.entrys.size() && (vf.kind==VarFrame::SCOPE || vf.kind==VarFrame::BLOCK)){
			var_set_direct(vf);
			put_inst(InstBlockEnd(vf.block_core_num));
		}
	}
}

static bool is_comp_bin(ExprPtr e){
	if(EXPR_EQ<=e->type() && e->type()<=EXPR_NIS){
		return true;
	}	
	return false;
}

void CodeBuilder::put_if_code(ExprPtr e, int_t label_if, int_t label_if2){
	if(is_comp_bin(e)){

		if(is_comp_bin(e->bin_lhs())){
			error(line(), Xt("Xtal Compile Error 1025"));
		}
		if(is_comp_bin(e->bin_rhs())){
			error(line(), Xt("Xtal Compile Error 1025"));
		}
		
		compile(e->bin_lhs());
		compile(e->bin_rhs());

		set_jump(offsetof(InstIfEq, address), label_if);
		InstIfEq inst;
		inst.op += e->type()-EXPR_EQ;
		put_inst(inst);

		if(e->type()==EXPR_NE || e->type()==EXPR_LE || e->type()==EXPR_GE){
			put_inst(InstNot());
		}

		set_jump(offsetof(InstIf, address), label_if2);
		put_inst(InstIf());
	}else{
		compile(e);

		set_jump(offsetof(InstIf, address), label_if);
		put_inst(InstIf());
	}
}

void CodeBuilder::push_loop(int break_labelno, int continue_labelno, const InternedStringPtr& name, bool have_label){
	FunFrame::Loop loop;
	loop.break_label = break_labelno;
	loop.continue_label = continue_labelno;
	loop.name = name;
	loop.frame_count = var_frames_.size();
	loop.have_label = have_label;
	fun_frames_.top().loops.push(loop);
}

void CodeBuilder::pop_loop(){
	fun_frames_.top().loops.pop();
}

void CodeBuilder::block_begin(){
	VarFrame& vf = var_frames_.top();

	int_t block_core_num = result_->block_core_table_.size();

	BlockCore core;
	core.variable_size = vf.entrys.size();
	core.variable_symbol_offset = result_->symbol_table_->size();
	core.lineno = lines_.top();

	vf.block_core_num = block_core_num;

	for(int_t i=0; i<vf.entrys.size(); ++i){
		result_->symbol_table_->push_back(vf.entrys[i].name);
	}

	if(vf.entrys.size()){
		var_set_direct(var_frames_.top());
		put_inst(InstBlockBegin(block_core_num));
	}

	result_->block_core_table_.push_back(core);
}

void CodeBuilder::block_end(){
	VarFrame& vf = var_frames_.top();
	if(vf.entrys.size()){
		var_set_direct(var_frames_.top());
		put_inst(InstBlockEnd(vf.block_core_num));
	}
}

void CodeBuilder::fun_begin(int_t kind, bool have_args, int_t offset, u8 min_param_count, u8 max_param_count){
	FunFrame& f = fun_frames_.push();	
	f.used_args_object = false;
	f.labels.clear();
	f.loops.clear();
	f.finallys.clear();
	f.var_frame_count = var_frames_.size()-1;
	f.used_args_object = have_args;

	VarFrame& vf = var_frames_.top();

	FunCore core;
	core.variable_symbol_offset = result_->symbol_table_->size();
	core.pc = code_size()+offset;
	core.lineno = lines_.top();
	core.kind = kind;
	core.min_param_count = min_param_count;
	core.max_param_count = max_param_count;
	core.used_args_object = have_args;
	core.on_heap = 0;
	core.variable_size = vf.entrys.size();

	for(int_t i=0; i<vf.entrys.size(); ++i){
		result_->symbol_table_->push_back(vf.entrys[i].name);
	}

	result_->xfun_core_table_.push_back(core);
}

void CodeBuilder::fun_end(int_t n){
	process_labels();
	fun_frames_.downsize(1);

	result_->xfun_core_table_[n].on_heap = var_frames_.top().kind!=VarFrame::SCOPE;
}

int_t CodeBuilder::code_size(){
	return result_->code_.size();
}

CodeBuilder::FunFrame &CodeBuilder::fun_frame(){
	return fun_frames_.top();
}


void CodeBuilder::compile_bin(ExprPtr e){
	if(is_comp_bin(e->bin_lhs())){
		error(line(), Xt("Xtal Compile Error 1013"));
	}
	if(is_comp_bin(e->bin_rhs())){
		error(line(), Xt("Xtal Compile Error 1013"));
	}
	
	compile(e->bin_lhs());
	compile(e->bin_rhs());

	InstAdd inst;
	inst.op += e->type() - EXPR_ADD;
	put_inst(inst);
}

void CodeBuilder::compile_comp_bin(ExprPtr e){
	if(is_comp_bin(e->bin_lhs())){
		error(line(), Xt("Xtal Compile Error 1025"));
	}
	if(is_comp_bin(e->bin_rhs())){
		error(line(), Xt("Xtal Compile Error 1025"));
	}
	
	compile(e->bin_lhs());
	compile(e->bin_rhs());

	InstEq inst;
	inst.op += e->type() - EXPR_EQ;
	put_inst(inst);

	if(e->type()==EXPR_NE || e->type()==EXPR_LE || e->type()==EXPR_GE){
		put_inst(InstNot());
	}
}

void CodeBuilder::compile_op_assign(const ExprPtr& e){
	ExprPtr lhs = e->bin_lhs();
	ExprPtr rhs = e->bin_rhs();

	InstAddAssign inst;
	inst.op += e->type() - EXPR_ADD_ASSIGN;

	if(lhs->type()==EXPR_LVAR){
		put_local_code(lhs->lvar_name());
		compile(rhs);
		put_inst(inst);
		put_set_local_code(lhs->lvar_name());
	}else if(lhs->type()==EXPR_IVAR){
		put_instance_variable_code(lhs->ivar_name());
		compile(rhs);
		put_inst(inst);
		put_set_instance_variable_code(lhs->ivar_name());
	}else if(lhs->type()==EXPR_SEND){
		compile(lhs->send_term());
		put_inst(InstDup());
		put_send_code(lhs->send_name(), lhs->send_pname(), 1, false, lhs->send_q());
		compile(rhs);
		put_inst(inst);
		put_inst(InstInsert1());
		put_set_send_code(lhs->send_name(), lhs->send_pname(), lhs->send_q());
	}else if(lhs->type()==EXPR_AT){
		compile(lhs->bin_lhs());
		put_inst(InstDup());
		compile(lhs->bin_rhs());
		put_inst(InstDup());
		put_inst(InstInsert2());
		put_inst(InstAt());
		compile(rhs);
		put_inst(inst);
		put_inst(InstInsert2());
		put_inst(InstSetAt());	
	}
}

void CodeBuilder::compile_incdec(const ExprPtr& e){
	ExprPtr term = e->una_term();

	InstInc inst;
	inst.op += e->type() - EXPR_INC;

	if(term->type()==EXPR_LVAR){
		LVarInfo info = var_find(term->lvar_name());
		if(info.pos>=0){
			if(info.pos>=256){
				if(e->type() == EXPR_INC){
					put_inst(InstLocalVariableInc2Byte(info.pos));
				}else{
					put_inst(InstLocalVariableDec2Byte(info.pos));
				}
				put_inst(InstSetLocalVariable2Byte(info.pos));
			}else{
				if(e->type() == EXPR_INC){
					var_set_direct(*info.var_frame);
					put_inst(InstLocalVariableInc(info.pos));
				}else{
					var_set_direct(*info.var_frame);
					put_inst(InstLocalVariableDec(info.pos));
				}
				var_set_direct(*info.var_frame);
				put_inst(InstSetLocalVariable1Byte(info.pos));
			}
		}else{
			put_inst(InstGlobalVariable(register_symbol(term->lvar_name())));
			put_inst(inst);
			put_set_local_code(term->lvar_name());
		}

	}else if(term->type()==EXPR_IVAR){
		put_instance_variable_code(term->ivar_name());
		put_inst(inst);
		put_set_instance_variable_code(term->ivar_name());
	}else if(term->type()==EXPR_SEND){
		compile(term->send_term());
		put_inst(InstDup());
		put_send_code(term->send_name(), term->send_pname(), 1, false, term->send_q());
		put_inst(inst);
		put_inst(InstInsert1());
		put_set_send_code(term->send_name(), term->send_pname(), term->send_q());
	}else if(term->type()==EXPR_AT){
		compile(term->bin_lhs());
		put_inst(InstDup());
		compile(term->bin_rhs());
		put_inst(InstDup());
		put_inst(InstInsert2());
		put_inst(InstAt());
		put_inst(inst);
		put_inst(InstInsert2());
		put_inst(InstSetAt());		
	}
}

void CodeBuilder::compile(const AnyPtr& p, const CompileInfo& info){

	int_t result_count = 1;

	if(!p){
		if(info.need_result_count==1){
			put_inst(InstPushNull());
		}else if(info.need_result_count!=0){
			put_inst(InstAdjustResult(0, info.need_result_count));
		}
		return;
	}

	ExprPtr e = ep(p);
	
	
	lines_.push(e->lineno());
	result_->set_lineno_info(e->lineno());

	switch(e->type()){

		XTAL_NODEFAULT;

		XTAL_CASE(EXPR_NULL){ put_inst(InstPushNull()); }
		XTAL_CASE(EXPR_TRUE){ put_inst(InstPushTrue()); }
		XTAL_CASE(EXPR_FALSE){ put_inst(InstPushFalse()); }
		XTAL_CASE(EXPR_THIS){ put_inst(InstPushThis()); }
		XTAL_CASE(EXPR_CURRENT_CONTEXT){ put_inst(InstPushCurrentContext()); var_set_on_heap(); }
		XTAL_CASE(EXPR_CALLEE){ put_inst(InstPushCallee()); }
		XTAL_CASE(EXPR_ARGS){ put_inst(InstPushArgs()); }

		XTAL_CASE(EXPR_INT){
			int_t value = e->int_value();
			if(value==(i8)value){ 
				put_inst(InstPushInt1Byte(value));
			}else if(value==(i16)value){ 
				put_inst(InstPushInt2Byte(value));
			}else{ 
				put_inst(InstValue(register_value(value)));
			}
		}

		XTAL_CASE(EXPR_FLOAT){
			float_t value = e->float_value();
			if(value==(i8)value){ 
				put_inst(InstPushFloat1Byte((u8)value));
			}else if(value==(i16)value){ 
				put_inst(InstPushFloat2Byte((u16)value));
			}else{ 
				put_inst(InstValue(register_value(value)));
			}
		}

		XTAL_CASE(EXPR_STRING){
			if(e->string_kind()==KIND_TEXT){
				put_inst(InstValue(register_value(get_text(e->string_value()->to_s()->c_str()))));
			}else if(e->string_kind()==KIND_FORMAT){
				put_inst(InstValue(register_value(format(e->string_value()->to_s()->c_str()))));
			}else{
				put_inst(InstValue(register_value(e->string_value())));
			}
		}

		XTAL_CASE(EXPR_ARRAY){
			put_inst(InstMakeArray());
			Xfor(v, e->array_values()){
				compile(v);
				put_inst(InstArrayAppend());				
			}
		}

		XTAL_CASE(EXPR_MAP){
			put_inst(InstMakeMap());
			Xfor2(k, v, e->map_values()){
				compile(k);
				compile(v);
				put_inst(InstMapInsert());				
			}
		}

		XTAL_CASE(EXPR_ADD){ compile_bin(e); }
		XTAL_CASE(EXPR_SUB){ compile_bin(e); }
		XTAL_CASE(EXPR_CAT){ compile_bin(e); }
		XTAL_CASE(EXPR_MUL){ compile_bin(e); }
		XTAL_CASE(EXPR_DIV){ compile_bin(e); }
		XTAL_CASE(EXPR_MOD){ compile_bin(e); }
		XTAL_CASE(EXPR_OR){ compile_bin(e); }
		XTAL_CASE(EXPR_AND){ compile_bin(e); }
		XTAL_CASE(EXPR_XOR){ compile_bin(e); }
		XTAL_CASE(EXPR_SHR){ compile_bin(e); }
		XTAL_CASE(EXPR_SHL){ compile_bin(e); }
		XTAL_CASE(EXPR_USHR){ compile_bin(e); }

		XTAL_CASE(EXPR_EQ){ compile_comp_bin(e); }
		XTAL_CASE(EXPR_NE){ compile_comp_bin(e); }
		XTAL_CASE(EXPR_LT){ compile_comp_bin(e); }
		XTAL_CASE(EXPR_GT){ compile_comp_bin(e); }
		XTAL_CASE(EXPR_LE){ compile_comp_bin(e); }
		XTAL_CASE(EXPR_GE){ compile_comp_bin(e); }
		XTAL_CASE(EXPR_RAWEQ){ compile_comp_bin(e); }
		XTAL_CASE(EXPR_RAWNE){ compile_comp_bin(e); }
		XTAL_CASE(EXPR_IS){ compile_comp_bin(e); }
		XTAL_CASE(EXPR_NIS){ compile_comp_bin(e); }

		XTAL_CASE(EXPR_POP){
			
		}

		XTAL_CASE(EXPR_Q){
			int_t label_if = reserve_label();
			int_t label_end = reserve_label();

			compile(e->q_cond());

			set_jump(offsetof(InstIf, address), label_if);
			put_inst(InstIf());

			compile(e->q_true());

			set_jump(offsetof(InstGoto, address), label_end);
			put_inst(InstGoto());

			set_label(label_if);
			
			compile(e->q_false());
			
			set_label(label_end);
		}
		
		XTAL_CASE(EXPR_AT){
			compile(e->bin_lhs());
			compile(e->bin_rhs());
			put_inst(InstAt());
		}

		XTAL_CASE(EXPR_ANDAND){
			int_t label_if = reserve_label();

			compile(e->bin_lhs());

			put_inst(InstDup());
			
			set_jump(offsetof(InstIf, address), label_if);
			put_inst(InstIf());

			put_inst(InstPop());
			
			compile(e->bin_rhs());
			
			set_label(label_if);
		}

		XTAL_CASE(EXPR_OROR){
			int_t label_if = reserve_label();
			compile(e->bin_lhs());

			put_inst(InstDup());
			
			set_jump(offsetof(InstUnless, address), label_if);
			put_inst(InstUnless());
			
			put_inst(InstPop());
			
			compile(e->bin_rhs());
			
			set_label(label_if);
		}

		XTAL_CASE(EXPR_POS){ compile(e->una_term()); put_inst(InstPos()); }
		XTAL_CASE(EXPR_NEG){ compile(e->una_term()); put_inst(InstNeg()); }
		XTAL_CASE(EXPR_COM){ compile(e->una_term()); put_inst(InstCom()); }
		XTAL_CASE(EXPR_NOT){ compile(e->una_term()); put_inst(InstNot()); }

		XTAL_CASE(EXPR_ONCE){
			int_t label_end = reserve_label();
			
			set_jump(offsetof(InstOnce, address), label_end);
			int_t num = result_->once_table_->size();
			result_->once_table_->push_back(once_value_none_);
			put_inst(InstOnce(0, num));
						
			compile(e->una_term());
			put_inst(InstDup());		
			put_inst(InstSetOnce(num));
			
			set_label(label_end);	
		}

		XTAL_CASE(EXPR_SEND){
			compile(e->send_term());

			/*
			int_t block_first = com_->register_ident(InternedStringPtr("block_first")); 
			int_t block_next = com_->register_ident(InternedStringPtr("block_next")); 
			int_t block_break = com_->register_ident(InternedStringPtr("block_break")); 

			if(e->var==block_first && !info.tail){
				put_inst(InstSendIterFirst(info.need_result_count));
			}else if(e->var==block_next && !info.tail){
				put_inst(InstSendIterNext(info.need_result_count));
			}else if(e->var==block_break && e->if_defined && !info.tail){
				put_inst(InstSendIterBreak(info.need_result_count));
			}else{
				put_send_code(e->var, e->pvar, info.need_result_count, info.tail, e->if_defined);
			}
			*/

			put_send_code(e->send_name(), e->send_pname(), info.need_result_count, info.tail, e->send_q());
			result_count = info.need_result_count;
		}

		XTAL_CASE(EXPR_CALL){
			
			Xfor(v, e->call_ordered()->each()){
				compile(ptr_cast<Expr>(v));
			}

			Xfor2(k, v, e->call_named()){
				put_inst(InstValue(register_value(k->to_s()->intern())));
				compile(ptr_cast<Expr>(v));
			}

			bool have_args = e->call_have_args();
			int_t ordered = e->call_ordered()->size();
			int_t named = e->call_named()->size();

			if(e->call_term()->type()==EXPR_SEND){ // a.b(); メッセージ送信式

				ExprPtr e2 = e->call_term();
				compile(e2->send_term());

				if(e2->send_pname()){
					compile(e2->send_pname());
				}

				int_t send_name = 0;
				if(!e2->send_pname()){
					send_name = register_symbol(e2->send_name());
				}

				if(e2->send_q()){
					if(info.tail){
						if(have_args){ put_inst(InstSendIfDefined_AT(ordered, named, info.need_result_count, send_name));
						}else{ put_inst(InstSendIfDefined_T(ordered, named, info.need_result_count, send_name)); }
					}else{
						if(have_args){ put_inst(InstSendIfDefined_A(ordered, named, info.need_result_count, send_name));
						}else{ put_inst(InstSendIfDefined(ordered, named, info.need_result_count, send_name)); }
					}
				}else{
					if(info.tail){
						if(have_args){ put_inst(InstSend_AT(ordered, named, info.need_result_count, send_name));
						}else{ put_inst(InstSend_T(ordered, named, info.need_result_count, send_name)); }
					}else{
						if(have_args){ put_inst(InstSend_A(ordered, named, info.need_result_count, send_name));
						}else{ put_inst(InstSend(ordered, named, info.need_result_count, send_name)); }
					}
				}
			}else if(e->call_term()->type()==EXPR_CALLEE){
				if(info.tail){
					if(have_args){ put_inst(InstCallCallee_AT(ordered, named, info.need_result_count));
					}else{ put_inst(InstCallCallee_T(ordered, named, info.need_result_count)); }
				}else{
					if(have_args){ put_inst(InstCallCallee_A(ordered, named, info.need_result_count));
					}else{ put_inst(InstCallCallee(ordered, named, info.need_result_count)); }
				}
			}else{
				compile(e->call_term());
				if(info.tail){
					if(have_args){ put_inst(InstCall_AT(ordered, named, info.need_result_count));
					}else{ put_inst(InstCall_T(ordered, named, info.need_result_count)); }
				}else{
					if(have_args){ put_inst(InstCall_A(ordered, named, info.need_result_count));
					}else{ put_inst(InstCall(ordered, named, info.need_result_count)); }
				}
			}

			result_count = info.need_result_count;
		}

		XTAL_CASE(EXPR_FUN){

			var_set_on_heap();

			/*
			if(e->kind==KIND_METHOD){

				// ゲッタか？
				if(e->params.size==0){
					if(BlockStmt* p = stmt_cast<BlockStmt>(e->stmt)){
						if(p->stmts.size==1){
							if(ReturnStmt* p2 = stmt_cast<ReturnStmt>(p->stmts.head->value)){
								if(p2->exprs.size==1){
									if(InstanceVariableExpr* p3 = expr_cast<InstanceVariableExpr>(p2->exprs.head->value)){
										put_inst(InstMakeInstanceVariableAccessor(0, lookup_instance_variable(p3->var), class_core_num()));
										break;
									}
								}
							}
						}
					}else if(ReturnStmt* p2 = stmt_cast<ReturnStmt>(e->stmt)){
						if(p2->exprs.size==1){
							if(InstanceVariableExpr* p3 = expr_cast<InstanceVariableExpr>(p2->exprs.head->value)){
								put_inst(InstMakeInstanceVariableAccessor(0, lookup_instance_variable(p3->var), class_core_num()));
								break;
							}
						}
					}
				}

				// セッタか？
				if(e->params.size==1){
					if(!e->params.head->value){
						if(BlockStmt* p = stmt_cast<BlockStmt>(e->stmt)){
							if(p->stmts.size==1){
								if(AssignStmt* p2 = stmt_cast<AssignStmt>(p->stmts.head->value)){
									if(InstanceVariableExpr* p3 = expr_cast<InstanceVariableExpr>(p2->lhs)){
										put_inst(InstMakeInstanceVariableAccessor(1, lookup_instance_variable(p3->var), class_core_num()));
										break;
									}
								}
							}
						}else if(AssignStmt* p2 = stmt_cast<AssignStmt>(e->stmt)){
							if(InstanceVariableExpr* p3 = expr_cast<InstanceVariableExpr>(p2->lhs)){
								put_inst(InstMakeInstanceVariableAccessor(1, lookup_instance_variable(p3->var), class_core_num()));
								break;
							}
						}
					}
				}
			}
			*/

			int_t minv = -1, maxv = 0;
			Xfor2(k, v, e->fun_params()){
				if(v){
					if(minv!=-1){
						
					}else{
						minv = maxv;
					}
				}else{
					if(minv!=-1){
						error(line(), Xt("Xtal Compile Error 1001"));
					}
				}
				maxv++;
			}
			if(minv==-1){
				minv = maxv;
			}

			var_begin(VarFrame::SCOPE);
			Xfor2(k, v, e->fun_params()){
				var_define(k->to_s()->intern());
			}

			int_t n = result_->xfun_core_table_.size();
			fun_begin(e->fun_kind(), e->fun_have_args(), InstMakeFun::ISIZE, minv, maxv);{

				int_t fun_end_label = reserve_label();

				set_jump(offsetof(InstMakeFun, address), fun_end_label);
				put_inst(InstMakeFun(n, 0));
				
				if(debug::is_enabled()){
					put_inst(InstBreakPoint(BREAKPOINT_CALL));
				}

				Xfor2(k, v, e->fun_params()){
					// デフォルト値を持つ
					if(v){
						LVarInfo info = var_find(k->to_s()->intern());
						int_t label = reserve_label();
						
						set_jump(offsetof(InstIfArgIsNull, address), label);
						var_set_direct(*info.var_frame);
						put_inst(InstIfArgIsNull(info.pos, 0));

						compile(v);
						put_set_local_code(k->to_s()->intern());
						
						set_label(label);
					}

					var_find(k->to_s()->intern(), true);
				}

				compile_stmt(e->fun_body());
				break_off(fun_frame().var_frame_count+1);
				if(debug::is_enabled()){
					put_inst(InstBreakPoint(BREAKPOINT_RETURN));
				}
				put_inst(InstReturn0());
				set_label(fun_end_label);
			}fun_end(n);
			var_end();
		}

		XTAL_CASE(EXPR_LVAR){
			put_local_code(e->lvar_name());
		}

		XTAL_CASE(EXPR_IVAR){
			put_instance_variable_code(e->ivar_name());
		}

		XTAL_CASE(EXPR_MEMBER){
			compile(e->member_term());
			put_member_code(e->member_name(), e->member_pname(), e->member_q());
		}

		XTAL_CASE(EXPR_CLASS){

			ExprPtr fun = Expr::make(EXPR_FUN);
			fun->set_fun_kind(KIND_METHOD);
			ExprPtr block = Expr::make(EXPR_SCOPE);
			ArrayPtr stmts = xnew<Array>();
			Xfor2(k, v, e->class_ivars()){
				if(v){
					stmts->push_back(Expr::make(EXPR_ASSIGN)->set_bin_lhs(Expr::make(EXPR_IVAR)->set_ivar_name(k->to_s()->intern()))->set_bin_rhs(ptr_cast<Expr>(v)));
				}
			}
			block->set_scope_stmts(stmts);
			fun->set_fun_body(block);

			e->class_stmts()->push_front(cdefine(0, KIND_PUBLIC, Xid(__INITIALIZE__), null_(0), fun));


			Xfor(v, e->class_mixins()){
				compile(v);
			}

			var_begin(VarFrame::CLASS);
			var_set_on_heap();

			Xfor(v, e->class_stmts()){
				ExprPtr v1 = ep(v);
				if(v1->type()==EXPR_CDEFINE){
					var_define(v1->cdefine_name());
				}
			}

			ClassFrame& cf = class_frames_.push();
			cf.entrys.clear();
			cf.class_core_num = result_->class_core_table_.size();

			Xfor2(k, v, e->class_ivars()){
				ClassFrame::Entry entry;
				entry.name = k->to_s()->intern();
				cf.entrys.push_back(entry);
			}


			VarFrame& vf = var_frames_.top();

			int_t class_core_num = result_->class_core_table_.size();

			ClassCore core;
			core.lineno = lines_.top();
			core.kind = e->class_kind();
			core.mixins = e->class_mixins() ? e->class_mixins()->size() : 0;
			core.variable_size = vf.entrys.size();
			core.instance_variable_size = e->class_ivars() ? e->class_ivars()->size() : 0;
			
			core.variable_symbol_offset = result_->symbol_table_->size();
			for(int_t i=0; i<vf.entrys.size(); ++i){
				result_->symbol_table_->push_back(vf.entrys[i].name);
			}

			core.instance_variable_symbol_offset = result_->symbol_table_->size();
			Xfor2(k, v, e->class_ivars()){
				result_->symbol_table_->push_back(k->to_s()->intern());
			}

			put_inst(InstClassBegin(class_core_num));
			result_->class_core_table_.push_back(core);

			{

				Xfor(v, e->class_stmts()){
					ExprPtr v1 = ep(v);
					if(v1->type()==EXPR_CDEFINE){					
						compile(v1->cdefine_term());
						compile(v1->cdefine_ns());
						LVarInfo info = var_find(v1->cdefine_name(), true);
						put_inst(InstDefineClassMember(info.pos, register_symbol(v1->cdefine_name()), v1->cdefine_accessibility()));
					}
				}
			}

			put_inst(InstClassEnd());
			class_frames_.pop();
			var_end();
		}
	}
	
	result_->set_lineno_info(e->lineno());
	lines_.pop();

	if(info.need_result_count!=result_count){
		put_inst(InstAdjustResult(result_count, info.need_result_count));
	}
}

void CodeBuilder::compile_stmt(const AnyPtr& p){

	if(!p)
		return;

	ExprPtr e = ep(p);

	if(debug::is_enabled() && lines_.top()!=e->lineno()){
		put_inst(InstBreakPoint(BREAKPOINT_LINE));
	}

	lines_.push(e->lineno());
	result_->set_lineno_info(e->lineno());

	switch(e->type()){

		XTAL_DEFAULT{
			compile(e, 0);
		}

		XTAL_CASE(EXPR_PUSH){
			compile(e->una_term());
		}
		
		XTAL_CASE(EXPR_DEFINE){
			if(e->bin_lhs()->type()==EXPR_LVAR){
				compile(e->bin_rhs());
				
				if(e->bin_rhs()->type()==EXPR_FUN || e->bin_rhs()->type()==EXPR_CLASS){
					put_inst(InstSetName(register_symbol(e->bin_lhs()->lvar_name())));
				}

				put_define_local_code(e->bin_lhs()->lvar_name());
			}else if(e->bin_lhs()->type()==EXPR_MEMBER){
				compile(e->bin_lhs()->member_term());
				compile(e->bin_rhs());

				if(e->bin_lhs()->member_name() && (e->bin_rhs()->type()==EXPR_FUN || e->bin_rhs()->type()==EXPR_CLASS)){
					put_inst(InstSetName(register_symbol(e->bin_lhs()->member_name())));
				}

				put_define_member_code(e->bin_lhs()->member_name(), e->bin_lhs()->member_pname());
			}else{
				error(line(), Xt("Xtal Compile Error 1012"));
			}
		}
		
		XTAL_CASE(EXPR_ASSIGN){
			if(e->bin_lhs()->type()==EXPR_LVAR){
				compile(e->bin_rhs());
				put_set_local_code(e->bin_lhs()->lvar_name());
			}else if(e->bin_lhs()->type()==EXPR_IVAR){
				compile(e->bin_rhs());
				put_set_instance_variable_code(e->bin_lhs()->ivar_name());
			}else if(e->bin_lhs()->type()==EXPR_SEND){
				compile(e->bin_rhs());
				compile(e->bin_lhs()->send_term());
				put_set_send_code(e->bin_lhs()->send_name(), e->bin_lhs()->send_pname(), e->bin_lhs()->send_q());
			}else if(e->bin_lhs()->type()==EXPR_AT){
				compile(e->bin_rhs());
				compile(e->bin_lhs()->bin_lhs());
				compile(e->bin_lhs()->bin_rhs());
				put_inst(InstSetAt());
			}else{
				error(line(), Xt("Xtal Compile Error 1012"));
			}
		}

		XTAL_CASE(EXPR_ADD_ASSIGN){ compile_op_assign(e); }
		XTAL_CASE(EXPR_SUB_ASSIGN){ compile_op_assign(e); }
		XTAL_CASE(EXPR_CAT_ASSIGN){ compile_op_assign(e); }
		XTAL_CASE(EXPR_MUL_ASSIGN){ compile_op_assign(e); }
		XTAL_CASE(EXPR_DIV_ASSIGN){ compile_op_assign(e); }
		XTAL_CASE(EXPR_MOD_ASSIGN){ compile_op_assign(e); }
		XTAL_CASE(EXPR_OR_ASSIGN){ compile_op_assign(e); }
		XTAL_CASE(EXPR_AND_ASSIGN){ compile_op_assign(e); }
		XTAL_CASE(EXPR_XOR_ASSIGN){ compile_op_assign(e); }
		XTAL_CASE(EXPR_SHR_ASSIGN){ compile_op_assign(e); }
		XTAL_CASE(EXPR_SHL_ASSIGN){ compile_op_assign(e); }
		XTAL_CASE(EXPR_USHR_ASSIGN){ compile_op_assign(e); }

		XTAL_CASE(EXPR_INC){ compile_incdec(e); }
		XTAL_CASE(EXPR_DEC){ compile_incdec(e); }

		/*
		XTAL_CASE(UnaStmt){
			compile(e->expr);
			put_inst(Inst(e->code));
		}
		*/
		
		XTAL_CASE(EXPR_YIELD){
			Xfor(v, e->yield_exprs()->each()){
				compile(v);
			}

			int_t exprs_size = e->yield_exprs()->size();
			put_inst(InstYield(exprs_size));
			if(exprs_size>=256){
				error(line(), Xt("Xtal Compile Error 1022"));
			}
		}

		XTAL_CASE(EXPR_RETURN){

			bool have_finally = false;
			for(uint_t scope_count = var_frames_.size(); scope_count!=(uint_t)fun_frame().var_frame_count+1; scope_count--){
				for(uint_t k = 0; k<(uint_t)fun_frame().finallys.size(); ++k){
					if((uint_t)fun_frame().finallys[k].frame_count==scope_count){
						have_finally = true;
					}
				}
			}

			int_t exprs_size = e->return_exprs() ? e->return_exprs()->size() : 0;
			if(!have_finally && exprs_size==1){
				ExprPtr front = ep(e->return_exprs()->front());
				if(front->type()==EXPR_CALL){
					compile(front, CompileInfo(1, true));
					break;
				}else if(front->type()==EXPR_SEND){
					compile(front, CompileInfo(1, true));
					break;
				}
			}

			Xfor(v, e->return_exprs()){
				compile(v);
			}
			
			{
				
				break_off(fun_frame().var_frame_count+1);

				if(debug::is_enabled()){
					put_inst(InstBreakPoint(BREAKPOINT_RETURN));
				}

				if(exprs_size==0){
					put_inst(InstReturn0());
				}else if(exprs_size==1){
					put_inst(InstReturn1());
				}else if(exprs_size==2){
					put_inst(InstReturn2());
				}else{
					put_inst(InstReturn(exprs_size));
					if(exprs_size>=256){
						error(line(), Xt("Xtal Compile Error 1022"));
					}
				}	
			}
		}

		XTAL_CASE(EXPR_ASSERT){		
			int_t exprs_size = e->assert_exprs()->size();
			if(exprs_size==1){
				compile(e->assert_exprs()->at(0));
				put_inst(InstValue(0));
				put_inst(InstValue(0));	
			}else if(exprs_size==2){
				compile(e->assert_exprs()->at(0));
				compile(e->assert_exprs()->at(1));
				put_inst(InstValue(0));
			}else if(exprs_size==3){
				compile(e->assert_exprs()->at(0));
				compile(e->assert_exprs()->at(1));
				compile(e->assert_exprs()->at(2));
			}else{
				error(line(), Xt("Xtal Compile Error 1016"));
			}
			
			put_inst(InstAssert());
		}

		XTAL_CASE(EXPR_TRY){

			int_t finally_label = reserve_label();
			int_t end_label = reserve_label();

			int_t core = result_->except_core_table_.size();
			put_inst(InstTryBegin(core));
			result_->except_core_table_.push_back(ExceptCore());

			CodeBuilder::FunFrame::Finally exc;
			exc.frame_count = var_frames_.size();
			exc.finally_label = finally_label;
			fun_frame().finallys.push(exc);

			compile_stmt(e->try_body());
			
			set_jump(offsetof(InstPushGoto, address), end_label);
			put_inst(InstPushGoto());
			put_inst(InstTryEnd());

			// catch節のコードを埋め込む
			if(e->try_catch()){

				result_->except_core_table_[core].catch_pc = code_size();
				
				// catch節の中での例外に備え、例外フレームを構築。
			
				int_t core2 = result_->except_core_table_.size();
				put_inst(InstTryBegin(core2));
				result_->except_core_table_.push_back(ExceptCore());

				CodeBuilder::FunFrame::Finally exc;
				exc.frame_count = var_frames_.size();
				exc.finally_label = finally_label;
				fun_frame().finallys.push(exc);

				var_begin(VarFrame::SCOPE);
				var_define(e->try_catch_var(), 0, true);
				block_begin();{
					put_set_local_code(e->try_catch_var());
					compile_stmt(e->try_catch());
				}block_end();
				var_end();

				put_inst(InstTryEnd());
				fun_frame().finallys.pop();

				result_->except_core_table_[core2].finally_pc = code_size();
				result_->except_core_table_[core2].end_pc = code_size();
			}
			
			set_label(finally_label);

			result_->except_core_table_[core].finally_pc = code_size();

			// finally節のコードを埋め込む
			compile_stmt(e->try_finally());
			
			fun_frame().finallys.pop();

			put_inst(InstPopGoto());

			set_label(end_label);
			result_->except_core_table_[core].end_pc = code_size();
		}

		XTAL_CASE(EXPR_THROW){
			compile(e->una_term());
			put_inst(InstThrow());
		}
		
		XTAL_CASE(EXPR_IF){
			int_t label_if = reserve_label();
			int_t label_if2 = reserve_label();
			int_t label_end = reserve_label();

			put_if_code(e->if_cond(), label_if, label_if2);

			compile_stmt(e->if_body());
			
			if(e->if_else()){
				set_jump(offsetof(InstGoto, address), label_end);
				put_inst(InstGoto());
			}
			
			set_label(label_if);
			set_label(label_if2);
			compile_stmt(e->if_else());
		
			set_label(label_end);
		}

		XTAL_CASE(EXPR_FOR){
			int_t label_cond = reserve_label();
			int_t label_cond_end = reserve_label();
			int_t label_if = reserve_label();
			int_t label_if2 = reserve_label();
			int_t label_end = reserve_label();
			
			if(e->for_cond()){
				put_if_code(e->for_cond(), label_if, label_if2);
			}
	
			set_jump(offsetof(InstGoto, address), label_cond_end);
			put_inst(InstGoto());

			if(e->for_next()){
				set_label(label_cond);
				compile_stmt(e->for_next());
			}else{
				set_label(label_cond);
			}

			if(e->for_cond()){
				if(e->for_else()){
					put_if_code(e->for_cond(), label_end, label_end);
				}else{
					put_if_code(e->for_cond(), label_if, label_if2);
				}
			}
			
			set_label(label_cond_end);
			push_loop(label_end, label_cond, e->for_label());
			compile_stmt(e->for_body());
			pop_loop(); 
			
			set_jump(offsetof(InstGoto, address), label_cond);
			put_inst(InstGoto());			

			set_label(label_if);
			set_label(label_if2);
			if(e->for_nobreak()){
				compile_stmt(e->for_nobreak());
			}else{
				compile_stmt(e->for_else());
			}

			set_label(label_end);
		}

		XTAL_CASE(EXPR_MASSIGN){
			int_t pushed_count = 0;

			ArrayPtr lhs = e->massign_lhs_exprs();
			ArrayPtr rhs = e->massign_rhs_exprs();

			for(int_t r=0; r<rhs->size(); ++r){	
				if(r==rhs->size()-1){
					int_t rrc;
					if(pushed_count<lhs->size()){
						rrc = lhs->size() - pushed_count;
					}else{
						rrc = 1;
					}

					compile(rhs->at(r), rrc);
					pushed_count += rrc;
					break;
				}else{
					compile(rhs->at(r));
					pushed_count++;
				}
			}

			if(lhs->size()!=pushed_count){
				put_inst(InstAdjustResult(pushed_count, lhs->size()));
			}

			if(e->massign_define()){
				Xfor(v1, lhs->r_each()){
					ExprPtr v = ep(v1);
					if(v->type()==EXPR_LVAR){
						put_define_local_code(v->lvar_name());
					}else if(v->type()==EXPR_MEMBER){
						compile(v->member_term());
						put_define_member_code(v->member_name(), v->member_pname());
					}else{
						error(line(), Xt("Xtal Compile Error 1008"));
					}
				}
			}else{
				Xfor(v1, lhs->r_each()){
					ExprPtr v = ep(v1);
					if(v->type()==EXPR_LVAR){
						put_set_local_code(v->lvar_name());
					}else if(v->type()==EXPR_SEND){
						compile(v->send_term());
						put_set_send_code(v->send_name(), v->send_pname(), v->send_q());
					}else if(v->type()==EXPR_IVAR){
						put_set_instance_variable_code(v->ivar_name());					
					}else if(v->type()==EXPR_AT){
						compile(v->bin_lhs());
						compile(v->bin_rhs());
						put_inst(InstSetAt());
					}else{
						error(line(), Xt("Xtal Compile Error 1008"));
					}
				}
			}
		}

		XTAL_CASE(EXPR_BREAK){
			if(fun_frame().loops.empty()){
				error(line(), Xt("Xtal Compile Error 1007"));
			}else{
				InternedStringPtr name = e->break_label();
				if(name){
					bool found = false;
					for(int_t i = 0, last = fun_frame().loops.size(); i<last; ++i){
						if(raweq(fun_frame().loops[i].name, name)){
							break_off(fun_frame().loops[i].frame_count);
							set_jump(offsetof(InstGoto, address), fun_frame().loops[i].break_label);
							put_inst(InstGoto());
							found = true;
							break;
						}
					}

					if(!found){
						error(line(), Xt("Xtal Compile Error 1005"));
					}
				}else{
					bool found = false;
					for(int_t i = 0, last = fun_frame().loops.size(); i<last; ++i){
						if(!fun_frame().loops[i].have_label){
							break_off(fun_frame().loops[i].frame_count);
							set_jump(offsetof(InstGoto, address), fun_frame().loops[i].break_label);
							put_inst(InstGoto());
							found = true;
							break;
						}
					}

					if(!found){
						error(line(), Xt("Xtal Compile Error 1005"));
					}
				}
			}
		}	

		XTAL_CASE(EXPR_CONTINUE){
			if(fun_frame().loops.empty()){
				error(line(), Xt("Xtal Compile Error 1006"));
			}else{
				InternedStringPtr name = e->continue_label();
				if(name){
					bool found = false;
					for(int_t i = 0, last = fun_frame().loops.size(); i<last; ++i){
						if(raweq(fun_frame().loops[i].name, name)){
							break_off(fun_frame().loops[i].frame_count);
							set_jump(offsetof(InstGoto, address), fun_frame().loops[i].continue_label);
							put_inst(InstGoto());
							found = true;
							break;
						}
					}

					if(!found){
						error(line(), Xt("Xtal Compile Error 1004"));
					}
				}else{
					bool found = false;
					for(size_t i = 0, last = fun_frame().loops.size(); i<last; ++i){
						if(!fun_frame().loops[i].have_label){
							break_off(fun_frame().loops[i].frame_count);
							set_jump(offsetof(InstGoto, address), fun_frame().loops[i].continue_label);
							put_inst(InstGoto());		
							found = true;
							break;
						}
					}

					if(!found){
						error(line(), Xt("Xtal Compile Error 1004"));
					}
				}
			}
		}	
		
		XTAL_CASE(EXPR_SCOPE){
			var_begin(VarFrame::SCOPE);
			var_define(e->scope_stmts());
			block_begin();{
				Xfor(v, e->scope_stmts()){
					compile_stmt(v);
				}
			}block_end();
			var_end();
		}
		
		XTAL_CASE(EXPR_TOPLEVEL){
			var_begin(VarFrame::SCOPE);
			var_define(e->toplevel_stmts());
			block_begin();{
				Xfor(v, e->toplevel_stmts()->each()){
					compile_stmt(v);
				}
				
				if(e->toplevel_export()){
					compile(e->toplevel_export());
					break_off(1);
					put_inst(InstReturn1());
				}else{
					break_off(1);
					put_inst(InstReturn0());
				}
			}block_end();
			var_end();
			
			process_labels();
			put_inst(InstThrow());
		}	
	}

	result_->set_lineno_info(e->lineno());
	lines_.pop();
}

}

#endif

