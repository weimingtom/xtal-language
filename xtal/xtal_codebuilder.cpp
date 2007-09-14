
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

	Stmt* ep = parser_.parse(stream, source_file_name);
	com_ = parser_.common();
	result_->symbol_table_ = com_->ident_table;
	result_->value_table_ = com_->value_table;
	result_->once_table_ = com_->once_table;
	
	if(ep){
		compile_stmt(ep);
	}
	
	fun_end(0);
	var_end();

	parser_.release();
	CodePtr retval = result_;
	result_ = null;

	if(com_->errors->size()==0){
		retval->reset_core();
		return retval;
	}else{
		return null;
	}
}

void CodeBuilder::interactive_compile(){
	result_ = xnew<Code>();
	result_->source_file_name_ = "<ix>";
	result_->except_core_table_.push_back(ExceptCore());

	lines_.push(1);
	fun_begin(KIND_FUN, true, 0, 0, 0);

	StreamPtr stream(xnew<InteractiveStream>());
	parser_.begin_interactive_parsing(stream);

	int_t pc_pos = 0;
	
	while(true){
		Stmt* ep = parser_.interactive_parse();
		cast<SmartPtr<InteractiveStream> >(stream)->set_continue_stmt(false);
		com_ = parser_.common();

		result_->symbol_table_ = com_->ident_table;
		result_->value_table_ = com_->value_table;
		result_->once_table_ = com_->once_table;
		com_->register_ident("filelocal");
		
		if(ep && com_->errors->empty()){
			compile_stmt(ep);
		}else{
			if(com_->errors->empty()){
				break;
			}
			com_->error(1, Xt("Xtal Compile Error 1001"));
		}
	
		if(com_->errors->size()==0){
			process_labels();
			put_inst(InstReturn0());
			put_inst(InstThrow());

			result_->reset_core();
			XTAL_TRY{
				vmachine()->execute(result_->first_fun().get(), &result_->code_[pc_pos]);
			}XTAL_CATCH(e){
				printf("%s\n", e->to_s()->c_str());
			}
			
			result_->code_.pop_back();
			pc_pos = result_->code_.size();

		}else{
			printf("Error: %s\n", errors()->at(0)->to_s()->c_str());
			com_->errors->clear();
		}
	}
}

void CodeBuilder::put_inst2(const Inst& t, uint_t sz){
	if(t.op==255){
		com_->error(line(), Xt("Xtal Compile Error 1027"));
	}

	size_t cur = result_->code_.size();
	result_->code_.resize(cur+sz/sizeof(inst_t));
	memcpy(&result_->code_[cur], &t, sz);
}

ArrayPtr CodeBuilder::errors(){
	return com_->errors;
}

InternedStringPtr CodeBuilder::to_id(int_t ident){
	return (InternedStringPtr&)com_->ident_table->at(ident);
}

bool CodeBuilder::put_set_local_code(int_t var){
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
		put_inst(InstSetGlobalVariable(var));
		return false;
	}
}

void CodeBuilder::put_define_local_code(int_t var){
	LVarInfo info = var_find(var);
	if(info.pos>=0){
		put_set_local_code(var);
	}else{
		put_inst(InstDefineGlobalVariable(var));
	}
}

bool CodeBuilder::put_local_code(int_t var){
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
		put_inst(InstGlobalVariable(var));
		return false;
	}
}

void CodeBuilder::put_send_code(int_t var, Expr* pvar, int_t need_result_count, bool tail, bool if_defined){
	if(pvar){
		compile(pvar);
	}	
	
	if(if_defined){
		if(tail){
			put_inst(InstSendIfDefined_T(0, 0, need_result_count, pvar ? 0 : var));
		}else{
			put_inst(InstSendIfDefined(0, 0, need_result_count, pvar ? 0 : var));
		}
	}else{
		if(tail){
			put_inst(InstSend_T(0, 0, need_result_count, pvar ? 0 : var));
		}else{
			put_inst(InstSend(0, 0, need_result_count, pvar ? 0 : var)); 
		}
	}
}

void CodeBuilder::put_set_send_code(int_t var, Expr* pvar, bool if_defined){
	if(pvar){
		ExprBuilder& e = *parser_.expr_builder();
		compile(e.bin(InstCat::NUMBER, e.string(com_->register_value("set_")), pvar));
	}	
	
	int_t symbol_number = com_->register_ident(xnew<String>("set_", 4)->cat(to_id(var)));
	bool tail = false;

	if(if_defined){
		if(tail){
			put_inst(InstSendIfDefined_T(1, 0, 0, pvar ? 0 : symbol_number));
		}else{
			put_inst(InstSendIfDefined(1, 0, 0, pvar ? 0 : symbol_number));
		}
	}else{
		if(tail){
			put_inst(InstSend_T(1, 0, 0, pvar ? 0 : symbol_number));
		}else{
			put_inst(InstSend(1, 0, 0, pvar ? 0 : symbol_number)); 
		}
	}
}

void CodeBuilder::put_member_code(int_t var, Expr* pvar, bool if_defined){
	if(pvar){
		compile(pvar);
	}
	
	if(if_defined){
		InstMemberIfDefined member;
		member.symbol_number = pvar ? 0 : var;
		put_inst(member);
	}else{
		InstMember member;
		member.symbol_number = pvar ? 0 : var;
		put_inst(member);
	}
}

void CodeBuilder::put_define_member_code(int_t var, Expr* pvar){
	if(pvar){
		compile(pvar);
	}

	InstDefineMember member;
	member.symbol_number = pvar ? 0 : var;
	put_inst(member);
}

int_t CodeBuilder::lookup_instance_variable(int_t key){
	if(!class_frames_.empty()){
		int ret = 0;
		ClassFrame& cf = class_frames_.top();
		for(size_t i = 0, last = cf.entrys.size(); i<last; ++i){
			if(cf.entrys[i].name==key){
				return ret;
			}
			ret++;
		}
	}
	com_->error(line(), Xt("Xtal Compile Error 1023")(Named("name", xnew<String>("_")->cat(to_id(key)))));
	return 0;
}

void CodeBuilder::put_set_instance_variable_code(int_t var){
	put_inst(InstSetInstanceVariable(lookup_instance_variable(var), class_core_num()));
}

void CodeBuilder::put_instance_variable_code(int_t var){
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

void CodeBuilder::put_if_code(Expr* e, int_t label_if, int_t label_if2){
	BinCompExpr* e2 = expr_cast<BinCompExpr>(e);
	if(e2 && InstEq::NUMBER<=e2->code && e2->code<=InstNis::NUMBER){

		if(expr_cast<BinCompExpr>(e2->lhs)){
			com_->error(line(), Xt("Xtal Compile Error 1025"));
		}
		if(expr_cast<BinCompExpr>(e2->rhs)){
			com_->error(line(), Xt("Xtal Compile Error 1025"));
		}
		
		compile(e2->lhs);
		compile(e2->rhs);

		set_jump(offsetof(InstIfEq, address), label_if);
		InstIfEq inst;
		inst.op += e2->code-InstEq::NUMBER;
		put_inst(inst);

		if(e2->code==InstNe::NUMBER || e2->code==InstLe::NUMBER || e2->code==InstGe::NUMBER){
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

void CodeBuilder::push_loop(int break_labelno, int continue_labelno, int_t name, bool have_label){
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
		result_->symbol_table_->push_back(to_id(vf.entrys[i].name));
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
		result_->symbol_table_->push_back(to_id(vf.entrys[i].name));
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

#define XTAL_EXPR_CASE(KEY) break; case KEY::TYPE: if(KEY* e = (KEY*)ex)if(e)

void CodeBuilder::compile(Expr* ex, const CompileInfo& info){

	int_t result_count = 1;

	if(!ex){
		if(info.need_result_count==1){
			put_inst(InstPushNull());
		}else if(info.need_result_count!=0){
			put_inst(InstAdjustResult(0, info.need_result_count));
		}
		return;
	}
	
	lines_.push(ex->line);
	result_->set_lineno_info(ex->line);

	switch(ex->type){

		XTAL_NODEFAULT;

		XTAL_EXPR_CASE(Expr){
			(void)e;
		}

		XTAL_EXPR_CASE(PseudoVariableExpr){
			put_inst(Inst(e->code));
			if(e->code==InstPushCurrentContext::NUMBER){
				var_set_on_heap();
			}
		}

		XTAL_EXPR_CASE(CalleeExpr){
			(void)e;
			put_inst(InstPushCallee());
		}

		XTAL_EXPR_CASE(ArgsExpr){
			(void)e;
			put_local_code(com_->register_ident(Xid(__ARGS__)));
		}

		XTAL_EXPR_CASE(IntExpr){
			if(e->value==(i8)e->value){ put_inst(InstPushInt1Byte(e->value));
			}else if(e->value==(i16)e->value){ put_inst(InstPushInt2Byte(e->value));
			}else{ put_inst(InstValue(com_->register_value(e->value)));
			}
		}

		XTAL_EXPR_CASE(FloatExpr){
			if(e->value==(i8)e->value){ put_inst(InstPushFloat1Byte((u8)e->value));
			}else if(e->value==(i16)e->value){ put_inst(InstPushFloat2Byte((u16)e->value));
			}else{ put_inst(InstValue(com_->register_value(e->value)));
			}
		}

		XTAL_EXPR_CASE(StringExpr){
			if(e->kind==KIND_TEXT){
				put_inst(InstValue(com_->register_value(get_text(cast<StringPtr>(com_->value_table->at(e->value))->c_str()))));
			}else if(e->kind==KIND_FORMAT){
				put_inst(InstValue(com_->register_value(format(cast<StringPtr>(com_->value_table->at(e->value))->c_str()))));
			}else{
				put_inst(InstValue(com_->register_value(com_->value_table->at(e->value))));
			}
		}

		XTAL_EXPR_CASE(ArrayExpr){
			put_inst(InstMakeArray());
			for(TList<Expr*>::Node* p = e->values.head; p; p = p->next){
				compile(p->value);
				put_inst(InstArrayAppend());				
			}
		}

		XTAL_EXPR_CASE(MapExpr){
			put_inst(InstMakeMap());
			for(TPairList<Expr*, Expr*>::Node* p = e->values.head; p; p = p->next){
				compile(p->key);
				compile(p->value);
				put_inst(InstMapInsert());				
			}
		}

		XTAL_EXPR_CASE(BinExpr){
			if(expr_cast<BinCompExpr>(e->lhs)){
				com_->error(line(), Xt("Xtal Compile Error 1013"));
			}
			if(expr_cast<BinCompExpr>(e->rhs)){
				com_->error(line(), Xt("Xtal Compile Error 1013"));
			}
			
			compile(e->lhs);
			compile(e->rhs);

			put_inst(Inst(e->code));
		}

		XTAL_EXPR_CASE(BinCompExpr){
			if(expr_cast<BinCompExpr>(e->lhs)){
				com_->error(line(), Xt("Xtal Compile Error 1025"));
			}
			if(expr_cast<BinCompExpr>(e->rhs)){
				com_->error(line(), Xt("Xtal Compile Error 1025"));
			}

			compile(e->lhs);
			compile(e->rhs);

			put_inst(Inst(e->code));

			if(e->code==InstNe::NUMBER || e->code==InstLe::NUMBER || e->code==InstGe::NUMBER){
				put_inst(InstNot());
			}
		}

		XTAL_EXPR_CASE(PopExpr){
			
		}

		XTAL_EXPR_CASE(TerExpr){
			int_t label_if = reserve_label();
			int_t label_end = reserve_label();

			compile(e->first);

			set_jump(offsetof(InstIf, address), label_if);
			put_inst(InstIf());

			compile(e->second);

			set_jump(offsetof(InstGoto, address), label_end);
			put_inst(InstGoto());

			set_label(label_if);
			
			compile(e->third);
			
			set_label(label_end);
		}
		
		XTAL_EXPR_CASE(AtExpr){
			compile(e->lhs);
			compile(e->index);

			put_inst(InstAt());
		}

		XTAL_EXPR_CASE(AndAndExpr){
			int_t label_if = reserve_label();

			compile(e->lhs);

			put_inst(InstDup());
			
			set_jump(offsetof(InstIf, address), label_if);
			put_inst(InstIf());

			put_inst(InstPop());
			
			compile(e->rhs);
			
			set_label(label_if);
		}

		XTAL_EXPR_CASE(OrOrExpr){
			int_t label_if = reserve_label();
			compile(e->lhs);

			put_inst(InstDup());
			
			set_jump(offsetof(InstUnless, address), label_if);
			put_inst(InstUnless());
			
			put_inst(InstPop());
			
			compile(e->rhs);
			
			set_label(label_if);
		}

		XTAL_EXPR_CASE(UnaExpr){
			compile(e->expr);

			put_inst(Inst(e->code));
		}

		XTAL_EXPR_CASE(OnceExpr){
			int_t label_end = reserve_label();

			set_jump(offsetof(InstOnce, address), label_end);
			int_t num = com_->append_once();
			put_inst(InstOnce(0, num));
						
			compile(e->expr);
			put_inst(InstDup());		
			put_inst(InstSetOnce(num));
			
			set_label(label_end);	
		}

		XTAL_EXPR_CASE(SendExpr){
			compile(e->lhs);

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

			put_send_code(e->var, e->pvar, info.need_result_count, info.tail, e->if_defined);
			result_count = info.need_result_count;
		}

		XTAL_EXPR_CASE(CallExpr){
			
			for(TList<Expr*>::Node* p = e->ordered.head; p; p = p->next){ 
				if(p->next){
					compile(p->value);
				}else{
					if(!expr_cast<ArgsExpr>(p->value)){
						compile(p->value);
					}
				}
			}

			for(TPairList<int_t, Expr*>::Node* p = e->named.head; p; p = p->next){ 
				put_inst(InstValue(com_->register_value(to_id(p->key))));
				compile(p->value);
			}

			bool have_args = !!expr_cast<ArgsExpr>(e->ordered.tail ? (Expr*)e->ordered.tail->value : (Expr*)0);
			int_t ordered, named;

			if(have_args){
				ordered = e->ordered.size-1;
				named = e->named.size;
			}else{
				ordered = e->ordered.size;
				named = e->named.size;
			}

			if(SendExpr* e2 = expr_cast<SendExpr>(e->expr)){ // a.b(); メッセージ送信式
				compile(e2->lhs);

				if(e2->pvar){
					compile(e2->pvar);
				}

				if(e2->if_defined){
					if(info.tail){
						if(have_args){ put_inst(InstSendIfDefined_AT(ordered, named, info.need_result_count, e2->pvar ? 0 : e2->var));
						}else{ put_inst(InstSendIfDefined_T(ordered, named, info.need_result_count, e2->pvar ? 0 : e2->var)); }
					}else{
						if(have_args){ put_inst(InstSendIfDefined_A(ordered, named, info.need_result_count, e2->pvar ? 0 : e2->var));
						}else{ put_inst(InstSendIfDefined(ordered, named, info.need_result_count, e2->pvar ? 0 : e2->var)); }
					}
				}else{
					if(info.tail){
						if(have_args){ put_inst(InstSend_AT(ordered, named, info.need_result_count, e2->pvar ? 0 : e2->var));
						}else{ put_inst(InstSend_T(ordered, named, info.need_result_count, e2->pvar ? 0 : e2->var)); }
					}else{
						if(have_args){ put_inst(InstSend_A(ordered, named, info.need_result_count, e2->pvar ? 0 : e2->var));
						}else{ put_inst(InstSend(ordered, named, info.need_result_count, e2->pvar ? 0 : e2->var)); }
					}
				}
			}else if(expr_cast<CalleeExpr>(e->expr)){
				if(info.tail){
					if(have_args){ put_inst(InstCallCallee_AT(ordered, named, info.need_result_count));
					}else{ put_inst(InstCallCallee_T(ordered, named, info.need_result_count)); }
				}else{
					if(have_args){ put_inst(InstCallCallee_A(ordered, named, info.need_result_count));
					}else{ put_inst(InstCallCallee(ordered, named, info.need_result_count)); }
				}
			}else{
				compile(e->expr);
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

		XTAL_EXPR_CASE(FunExpr){

			var_set_on_heap();

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

			int_t minv = -1, maxv = 0;
			for(TPairList<int_t, Expr*>::Node* p = e->params.head; p; p = p->next){
				if(p->value){
					if(minv!=-1){
						
					}else{
						minv = maxv;
					}
				}else{
					if(minv!=-1){
						com_->error(line(), Xt("Xtal Compile Error 1001"));
					}
				}
				maxv++;
			}
			if(minv==-1){
				minv = maxv;
			}

			var_begin(VarFrame::SCOPE);
			for(TPairList<int_t, Expr*>::Node* p = e->params.head; p; p = p->next){
				var_define(p->key);
			}

			int_t n = result_->xfun_core_table_.size();
			fun_begin(e->kind, e->have_args, InstMakeFun::ISIZE, minv, maxv);{

				int_t fun_end_label = reserve_label();

				set_jump(offsetof(InstMakeFun, address), fun_end_label);
				put_inst(InstMakeFun(n, 0));
				
				if(debug::is_enabled()){
					put_inst(InstBreakPoint(BREAKPOINT_CALL));
				}

				for(TPairList<int_t, Expr*>::Node* p = e->params.head; p; p = p->next){
					// デフォルト値を持つ
					if(p->value){
						
						LVarInfo info = var_find(p->key);
						int_t label = reserve_label();
						
						set_jump(offsetof(InstIfArgIsNull, address), label);
						var_set_direct(*info.var_frame);
						put_inst(InstIfArgIsNull(info.pos, 0));

						compile(p->value);
						put_set_local_code(p->key);
						
						set_label(label);
					}
				}
				compile_stmt(e->stmt);
				break_off(fun_frame().var_frame_count+1);
				if(debug::is_enabled()){
					put_inst(InstBreakPoint(BREAKPOINT_RETURN));
				}
				put_inst(InstReturn0());
				set_label(fun_end_label);
			}fun_end(n);
			var_end();
		}

		XTAL_EXPR_CASE(LocalExpr){
			put_local_code(e->var);
		}

		XTAL_EXPR_CASE(InstanceVariableExpr){
			put_instance_variable_code(e->var);
		}

		XTAL_EXPR_CASE(MemberExpr){
			compile(e->lhs);
			put_member_code(e->var, e->pvar, e->if_defined);
		}

		XTAL_EXPR_CASE(ClassExpr){

			for(TList<Expr*>::Node* p = e->mixins.head; p; p = p->next){
				compile(p->value);
			}

			var_begin(VarFrame::CLASS);
			var_set_on_heap();

			for(TList<Stmt*>::Node* p = e->stmts.head; p; p = p->next){
				if(DefineClassMemberStmt* dcms = stmt_cast<DefineClassMemberStmt>(p->value)){
					var_define(dcms->name);
				}
			}

			ClassFrame& cf = class_frames_.push();
			cf.entrys.clear();
			cf.class_core_num = result_->class_core_table_.size();

			for(TPairList<int_t, Expr*>::Node* p=e->inst_vars.head; p; p=p->next){
				ClassFrame::Entry entry;
				entry.name = p->key;
				cf.entrys.push_back(entry);
			}


			VarFrame& vf = var_frames_.top();

			int_t class_core_num = result_->class_core_table_.size();

			ClassCore core;
			core.lineno = lines_.top();
			core.kind = e->kind;
			core.mixins = e->mixins.size;
			core.variable_size = vf.entrys.size();
			core.instance_variable_size = e->inst_vars.size;
			
			core.variable_symbol_offset = result_->symbol_table_->size();
			for(int_t i=0; i<vf.entrys.size(); ++i){
				result_->symbol_table_->push_back(to_id(vf.entrys[i].name));
			}

			core.instance_variable_symbol_offset = result_->symbol_table_->size();
			for(TPairList<int_t, Expr*>::Node* p=e->inst_vars.head; p; p=p->next){
				result_->symbol_table_->push_back(to_id(p->key));
			}

			put_inst(InstClassBegin(class_core_num));
			result_->class_core_table_.push_back(core);

			{

				for(TList<Stmt*>::Node* p = e->stmts.head; p; p = p->next){
					if(DefineClassMemberStmt* dcms = stmt_cast<DefineClassMemberStmt>(p->value)){
						compile(dcms->expr);
						compile(dcms->ns);
						LVarInfo info = var_find(dcms->name);
						put_inst(InstDefineClassMember(info.pos, dcms->name, dcms->accessibility));
					}
				}
			}

			put_inst(InstClassEnd());
			class_frames_.pop();
			var_end();
		}
	}
	
	result_->set_lineno_info(ex->line);
	lines_.pop();

	if(info.need_result_count!=result_count){
		put_inst(InstAdjustResult(result_count, info.need_result_count));
	}
}

void CodeBuilder::compile_stmt(Stmt* ex){

	if(!ex)
		return;

	if(debug::is_enabled() && lines_.top()!=ex->line){
		put_inst(InstBreakPoint(BREAKPOINT_LINE));
	}

	lines_.push(ex->line);
	result_->set_lineno_info(ex->line);

	switch(ex->type){

		XTAL_NODEFAULT;

		XTAL_EXPR_CASE(ExprStmt){
			compile(e->expr, 0);
		}

		XTAL_EXPR_CASE(PushStmt){
			compile(e->expr);
		}
		
		XTAL_EXPR_CASE(DefineStmt){
			if(LocalExpr* p = expr_cast<LocalExpr>(e->lhs)){
				compile(e->rhs);
				
				if(expr_cast<FunExpr>(e->rhs) || expr_cast<ClassExpr>(e->rhs)){
					put_inst(InstSetName(p->var));
				}

				put_define_local_code(p->var);
			}else if(MemberExpr* p = expr_cast<MemberExpr>(e->lhs)){
				compile(p->lhs);
				compile(e->rhs);

				if(p->var!=0 && (expr_cast<FunExpr>(e->rhs) || expr_cast<ClassExpr>(e->rhs))){
					put_inst(InstSetName(p->var));
				}

				put_define_member_code(p->var, p->pvar);
			}else{
				com_->error(line(), Xt("Xtal Compile Error 1012"));
			}
		}
		
		XTAL_EXPR_CASE(AssignStmt){
			if(LocalExpr* p = expr_cast<LocalExpr>(e->lhs)){
				compile(e->rhs);
				put_set_local_code(p->var);
			}else if(InstanceVariableExpr* p = expr_cast<InstanceVariableExpr>(e->lhs)){
				compile(e->rhs);
				put_set_instance_variable_code(p->var);
			}else if(SendExpr* p = expr_cast<SendExpr>(e->lhs)){
				compile(e->rhs);
				compile(p->lhs);
				put_set_send_code(p->var, p->pvar, p->if_defined);
			}else if(AtExpr* p = expr_cast<AtExpr>(e->lhs)){
				compile(e->rhs);
				compile(p->lhs);
				compile(p->index);
				put_inst(InstSetAt());
			}else{
				com_->error(line(), Xt("Xtal Compile Error 1012"));
			}
		}

		XTAL_EXPR_CASE(OpAssignStmt){
			if(LocalExpr* p = expr_cast<LocalExpr>(e->lhs)){

				/*
				compile(e->rhs);
				InstLocalVariableAddAssign inst(p->var);
				inst.op += e->code-InstAddAssign::NUMBER;
				put_inst(inst);
				put_inst(InstSetLocalVariable2Byte(lookup_variable(p->var)));
				*/

				put_local_code(p->var);
				compile(e->rhs);
				put_inst(Inst(e->code));
				put_set_local_code(p->var);
			}else if(InstanceVariableExpr* p = expr_cast<InstanceVariableExpr>(e->lhs)){

				/*
				compile(e->rhs);
				InstInstanceVariableAddAssign inst(lookup_instance_variable(p->var), frame_number());
				inst.op += e->code-InstAddAssign::NUMBER;
				put_inst(inst);
				put_set_instance_variable_code(p->var);
				*/

				put_instance_variable_code(p->var);
				compile(e->rhs);
				put_inst(Inst(e->code));
				put_set_instance_variable_code(p->var);

			}else if(SendExpr* p = expr_cast<SendExpr>(e->lhs)){
				compile(p->lhs);
				put_inst(InstDup());
				put_send_code(p->var, p->pvar, 1, false, p->if_defined);
				compile(e->rhs);
				put_inst(Inst(e->code));
				put_inst(InstInsert1());
				put_set_send_code(p->var, p->pvar, p->if_defined);
			}else if(AtExpr* p = expr_cast<AtExpr>(e->lhs)){
				compile(p->lhs);
				put_inst(InstDup());
				compile(p->index);
				put_inst(InstDup());
				put_inst(InstInsert2());
				put_inst(InstAt());
				compile(e->rhs);
				put_inst(Inst(e->code));
				put_inst(InstInsert2());
				put_inst(InstSetAt());	
			}
		}

		XTAL_EXPR_CASE(IncStmt){
			if(LocalExpr* p = expr_cast<LocalExpr>(e->lhs)){
				LVarInfo info = var_find(p->var);
				if(info.pos>=0){
					if(info.pos>=256){
						if(e->code == InstInc::NUMBER){
							put_inst(InstLocalVariableInc2Byte(info.pos));
						}else{
							put_inst(InstLocalVariableDec2Byte(info.pos));
						}
						put_inst(InstSetLocalVariable2Byte(info.pos));
					}else{
						if(e->code == InstInc::NUMBER){
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
					put_inst(InstGlobalVariable(p->var));
					put_inst(Inst(e->code));
					put_set_local_code(p->var);
				}

			}else if(InstanceVariableExpr* p = expr_cast<InstanceVariableExpr>(e->lhs)){
				put_instance_variable_code(p->var);
				put_inst(Inst(e->code));
				put_set_instance_variable_code(p->var);
			}else if(SendExpr* p = expr_cast<SendExpr>(e->lhs)){
				compile(p->lhs);
				put_inst(InstDup());
				put_send_code(p->var, p->pvar, 1, false, p->if_defined);
				put_inst(Inst(e->code));
				put_inst(InstInsert1());
				put_set_send_code(p->var, p->pvar, p->if_defined);
			}else if(AtExpr* p = expr_cast<AtExpr>(e->lhs)){
				compile(p->lhs);
				put_inst(InstDup());
				compile(p->index);
				put_inst(InstDup());
				put_inst(InstInsert2());
				put_inst(InstAt());
				put_inst(Inst(e->code));
				put_inst(InstInsert2());
				put_inst(InstSetAt());		
			}
		}

		XTAL_EXPR_CASE(UnaStmt){
			compile(e->expr);
			put_inst(Inst(e->code));
		}
		
		XTAL_EXPR_CASE(YieldStmt){
			for(TList<Expr*>::Node* p = e->exprs.head; p; p = p->next){
				compile(p->value);
			}

			put_inst(InstYield(e->exprs.size));
			if(e->exprs.size>=256){
				com_->error(line(), Xt("Xtal Compile Error 1022"));
			}
		}

		XTAL_EXPR_CASE(ReturnStmt){

			bool have_finally = false;
			for(uint_t scope_count = var_frames_.size(); scope_count!=(uint_t)fun_frame().var_frame_count+1; scope_count--){
				for(uint_t k = 0; k<(uint_t)fun_frame().finallys.size(); ++k){
					if((uint_t)fun_frame().finallys[k].frame_count==scope_count){
						have_finally = true;
					}
				}
			}

			if(!have_finally && e->exprs.size==1){
				if(CallExpr* ce = expr_cast<CallExpr>(e->exprs.head->value)){
					compile(ce, CompileInfo(1, true));
					break;
				}else if(SendExpr* se = expr_cast<SendExpr>(e->exprs.head->value)){
					compile(se, CompileInfo(1, true));
					break;
				}
			}

			for(TList<Expr*>::Node* p = e->exprs.head; p; p = p->next){
				compile(p->value);
			}
			
			{
				
				break_off(fun_frame().var_frame_count+1);

				if(debug::is_enabled()){
					put_inst(InstBreakPoint(BREAKPOINT_RETURN));
				}

				if(e->exprs.size==0){
					put_inst(InstReturn0());
				}else if(e->exprs.size==1){
					put_inst(InstReturn1());
				}else if(e->exprs.size==2){
					put_inst(InstReturn2());
				}else{
					put_inst(InstReturn(e->exprs.size));
					if(e->exprs.size>=256){
						com_->error(line(), Xt("Xtal Compile Error 1022"));
					}
				}	
			}
		}

		XTAL_EXPR_CASE(AssertStmt){
							
			if(e->exprs.size==1){
				compile(e->exprs.head->value);
				put_inst(InstValue(0));
				put_inst(InstValue(0));	
			}else if(e->exprs.size==2){
				compile(e->exprs.head->value);
				compile(e->exprs.head->next->value);
				put_inst(InstValue(0));
			}else if(e->exprs.size==3){
				compile(e->exprs.head->value);
				compile(e->exprs.head->next->value);
				compile(e->exprs.head->next->next->value);
			}else{
				com_->error(line(), Xt("Xtal Compile Error 1016"));
			}
			
			put_inst(InstAssert());
		}

		XTAL_EXPR_CASE(TryStmt){

			int_t finally_label = reserve_label();
			int_t end_label = reserve_label();

			int_t core = result_->except_core_table_.size();
			put_inst(InstTryBegin(core));
			result_->except_core_table_.push_back(ExceptCore());

			CodeBuilder::FunFrame::Finally exc;
			exc.frame_count = var_frames_.size();
			exc.finally_label = finally_label;
			fun_frame().finallys.push(exc);

			compile_stmt(e->try_stmt);
			
			set_jump(offsetof(InstPushGoto, address), end_label);
			put_inst(InstPushGoto());
			put_inst(InstTryEnd());

			// catch節のコードを埋め込む
			if(e->catch_stmt){

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
				var_define(e->catch_var);
				block_begin();{
					put_set_local_code(e->catch_var);
					compile_stmt(e->catch_stmt);
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
			compile_stmt(e->finally_stmt);
			
			fun_frame().finallys.pop();

			put_inst(InstPopGoto());

			set_label(end_label);
			result_->except_core_table_[core].end_pc = code_size();
		}
		
		XTAL_EXPR_CASE(IfStmt){
			int_t label_if = reserve_label();
			int_t label_if2 = reserve_label();
			int_t label_end = reserve_label();

			put_if_code(e->cond_expr, label_if, label_if2);

			compile_stmt(e->body_stmt);
			
			if(e->else_stmt){
				set_jump(offsetof(InstGoto, address), label_end);
				put_inst(InstGoto());
			}
			
			set_label(label_if);
			set_label(label_if2);
			compile_stmt(e->else_stmt);
		
			set_label(label_end);
		}

		XTAL_EXPR_CASE(WhileStmt){
			int_t label_cond = reserve_label();
			int_t label_cond_end = reserve_label();
			int_t label_if = reserve_label();
			int_t label_if2 = reserve_label();
			int_t label_end = reserve_label();
			
			if(e->cond_expr){
				put_if_code(e->cond_expr, label_if, label_if2);
			}
	
			set_jump(offsetof(InstGoto, address), label_cond_end);
			put_inst(InstGoto());

			if(e->next_stmt){
				set_label(label_cond);
				compile_stmt(e->next_stmt);
			}else{
				set_label(label_cond);
			}

			if(e->cond_expr){
				if(e->else_stmt){
					put_if_code(e->cond_expr, label_end, label_end);
				}else{
					put_if_code(e->cond_expr, label_if, label_if2);
				}
			}
			
			set_label(label_cond_end);
			push_loop(label_end, label_cond, e->label);
			compile_stmt(e->body_stmt);
			pop_loop(); 
			
			set_jump(offsetof(InstGoto, address), label_cond);
			put_inst(InstGoto());			

			set_label(label_if);
			set_label(label_if2);
			if(e->nobreak_stmt){
				compile_stmt(e->nobreak_stmt);
			}else{
				compile_stmt(e->else_stmt);
			}

			set_label(label_end);
		}

		XTAL_EXPR_CASE(MultipleAssignStmt){
			int_t pushed_count = 0;
			for(TList<Expr*>::Node* rhs=e->rhs.head; rhs; rhs=rhs->next){	
				if(!rhs->next){
					if(expr_cast<CallExpr>(rhs->value) || expr_cast<SendExpr>(rhs->value)){
						int_t rrc;
						if(pushed_count<e->lhs.size){
							rrc = e->lhs.size - pushed_count;
						}else{
							rrc = 1;
						}

						compile(rhs->value, rrc);
						pushed_count += rrc;

						break;
					}else{
						int_t rrc;
						if(pushed_count<e->lhs.size){
							rrc = e->lhs.size - pushed_count;
						}else{
							rrc = 1;
						}
						compile(rhs->value, rrc);
						pushed_count += rrc;
						break;
					}
				}else{
					compile(rhs->value);
					pushed_count++;
				}
			}

			if(e->lhs.size!=pushed_count){
				put_inst(InstAdjustResult(pushed_count, e->lhs.size));
			}

			if(e->define){
				for(TList<Expr*>::Node* lhs=e->lhs.tail; lhs; lhs=lhs->prev){	
					if(LocalExpr* e2 = expr_cast<LocalExpr>(lhs->value)){
						put_define_local_code(e2->var);
					}else if(MemberExpr* e2 = expr_cast<MemberExpr>(lhs->value)){
						compile(e2->lhs);
						put_define_member_code(e2->var, e2->pvar);
					}else{
						com_->error(line(), Xt("Xtal Compile Error 1008"));
					}
				}
			}else{
				for(TList<Expr*>::Node* lhs=e->lhs.tail; lhs; lhs=lhs->prev){	
					if(LocalExpr* e2 = expr_cast<LocalExpr>(lhs->value)){
						put_set_local_code(e2->var);
					}else if(SendExpr* e2 = expr_cast<SendExpr>(lhs->value)){
						compile(e2->lhs);
						put_set_send_code(e2->var, e2->pvar, e2->if_defined);
					}else if(InstanceVariableExpr* e2 = expr_cast<InstanceVariableExpr>(lhs->value)){
						put_set_instance_variable_code(e2->var);					
					}else if(AtExpr* e2 = expr_cast<AtExpr>(lhs->value)){
						compile(e2->lhs);
						compile(e2->index);
						put_inst(InstSetAt());
					}else{
						com_->error(line(), Xt("Xtal Compile Error 1008"));
					}
				}
			}
		}

		XTAL_EXPR_CASE(BreakStmt){
			if(fun_frame().loops.empty()){
				com_->error(line(), Xt("Xtal Compile Error 1007"));
			}else{
				if(e->var){
					bool found = false;
					int_t name = e->var;
					for(int_t i = 0, last = fun_frame().loops.size(); i<last; ++i){
						if(fun_frame().loops[i].name==name){
							break_off(fun_frame().loops[i].frame_count);
							set_jump(offsetof(InstGoto, address), fun_frame().loops[i].break_label);
							put_inst(InstGoto());
							found = true;
							break;
						}
					}

					if(!found){
						com_->error(line(), Xt("Xtal Compile Error 1005"));
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
						com_->error(line(), Xt("Xtal Compile Error 1005"));
					}
				}
			}
		}	

		XTAL_EXPR_CASE(ContinueStmt){
			if(fun_frame().loops.empty()){
				com_->error(line(), Xt("Xtal Compile Error 1006"));
			}else{
				if(e->var){
					bool found = false;
					int_t name = e->var;
					for(int_t i = 0, last = fun_frame().loops.size(); i<last; ++i){
						if(fun_frame().loops[i].name==name){
							break_off(fun_frame().loops[i].frame_count);
							set_jump(offsetof(InstGoto, address), fun_frame().loops[i].continue_label);
							put_inst(InstGoto());
							found = true;
							break;
						}
					}

					if(!found){
						com_->error(line(), Xt("Xtal Compile Error 1004"));
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
						com_->error(line(), Xt("Xtal Compile Error 1004"));
					}
				}
			}
		}	
		
		XTAL_EXPR_CASE(BlockStmt){
			var_begin(VarFrame::SCOPE);
			var_define(e->stmts);
			block_begin();{
				for(TList<Stmt*>::Node* p = e->stmts.head; p; p = p->next){
					compile_stmt(p->value);
				}
			}block_end();
			var_end();
		}
		
		XTAL_EXPR_CASE(TopLevelStmt){
			var_begin(VarFrame::SCOPE);
			var_define(e->stmts);
			block_begin();{
				for(TList<Stmt*>::Node* p = e->stmts.head; p; p = p->next){
					compile_stmt(p->value);
				}
				
				if(e->export_expr){
					compile(e->export_expr);
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

	result_->set_lineno_info(ex->line);
	lines_.pop();
}

}

#endif

