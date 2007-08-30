
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
	Vars vars;
	vars.on_heap = true;
	fun_begin(KIND_FUN, &vars, true, 0, 0, 0);

	Stmt* ep = parser_.parse(stream, source_file_name);
	com_ = parser_.common();
	result_->symbol_table_ = com_->ident_table;
	result_->value_table_ = com_->value_table;
	result_->once_table_ = com_->once_table;
	
	if(ep){
		compile(ep);
	}

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
	Vars vars;
	vars.on_heap = true;
	fun_begin(KIND_FUN, &vars, true, 0, 0, 0);

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
			compile(ep);
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


ArrayPtr CodeBuilder::errors(){
	return com_->errors;
}

InternedStringPtr CodeBuilder::to_id(int_t ident){
	return (InternedStringPtr&)com_->ident_table->at(ident);
}

int_t CodeBuilder::lookup_variable(int_t key){
	int ret = 0;
	for(size_t i = 0, last = vars_stack_.size(); i<last; ++i){
		for(TList<Var>::Node* p = vars_stack_[i]->vars.tail; p; p = p->prev){
			if(p->value.name==key){
				return ret;
			}
			ret++;
		}
	}
	return -1;
}

bool CodeBuilder::variable_on_heap(int_t pos){
	for(size_t i = 0, last = vars_stack_.size(); i<last; ++i){
		if(pos<vars_stack_[i]->vars.size){
			return vars_stack_[i]->on_heap;
		}
		pos -= vars_stack_[i]->vars.size;
	}
	return false;
}

bool CodeBuilder::put_set_local_code(int_t var){
	int_t id = lookup_variable(var);
	if(id>=0){
		bool on_heap = variable_on_heap(id);
 
		if(on_heap){
			if(id<=0xff){
				put_inst(InstSetLocalVariable1Byte(id));
			}else{
				put_inst(InstSetLocalVariable2Byte(id));
			}		
		}else{
			if(id<=0xff){
				put_inst(InstSetLocalVariable1ByteDirect(id));
			}else{
				put_inst(InstSetLocalVariable2Byte(id));
			}
		}
		return true;
	}else{
		//com_->error(line(), Xt("定義されていない変数%sに代入しようとしました")(to_id(var)));
		put_inst(InstSetGlobalVariable(var));
		return false;
	}
}

void CodeBuilder::put_define_local_code(int_t var){
	int_t id = lookup_variable(var);
	if(id>=0){
		put_set_local_code(var);
	}else{
		put_inst(InstDefineGlobalVariable(var));
	}
}

bool CodeBuilder::put_local_code(int_t var){
	int_t id = lookup_variable(var);
	if(id>=0){
		bool on_heap = variable_on_heap(id);

		if(on_heap){
			if(id<=0xff){
				put_inst(InstLocalVariable1Byte(id));
			}else{
				put_inst(InstLocalVariable2Byte(id));
			}		
		}else{
			if(id<=0xff){
				put_inst(InstLocalVariable1ByteDirect(id));
			}else{
				put_inst(InstLocalVariable2Byte(id));
			}
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
	if(!class_scopes_.empty()){
		int_t i = 0;
		for(TPairList<int_t, Expr*>::Node* p=class_scopes_.top()->inst_vars.head; p; p=p->next){
			if(p->key==key){
				return i; 
			}
			i++;
		}
	}
	com_->error(line(), Xt("Xtal Compile Error 1023")(Named("name", xnew<String>("_")->cat(to_id(key)))));
	return 0;
}

void CodeBuilder::put_set_instance_variable_code(int_t var){
	put_inst(InstSetInstanceVariable(lookup_instance_variable(var), class_scopes_.empty() ? 0 : class_scopes_.top()->frame_number));
}

void CodeBuilder::put_instance_variable_code(int_t var){
	put_inst(InstInstanceVariable(lookup_instance_variable(var), class_scopes_.empty() ? 0 : class_scopes_.top()->frame_number));
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
	for(uint_t scope_count = vars_stack_.size(); scope_count!=(uint_t)n; scope_count--){
		for(uint_t k = 0; k<fun_frame().finallys.size(); ++k){
			if((uint_t)fun_frame().finallys[k].frame_count==scope_count){
				int_t label = reserve_label();
				set_jump(offsetof(InstPushGoto, address), label);
				put_inst(InstPushGoto());
				put_inst(InstTryEnd());
				set_label(label);
			}
		}
		if(vars_stack_[vars_stack_.size()-scope_count]->vars.size){
			if(vars_stack_[vars_stack_.size()-scope_count]->on_heap){
				put_inst(InstBlockEnd());
			}else{
				put_inst(InstBlockEndDirect(vars_stack_[vars_stack_.size()-scope_count]->vars.size));
			}
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
	loop.frame_count = vars_stack_.size();
	loop.have_label = have_label;
	fun_frames_.top().loops.push(loop);
}

void CodeBuilder::pop_loop(){
	fun_frames_.top().loops.pop();
}

void CodeBuilder::block_begin(Vars* vars){
	vars_stack_.push(vars);

	int_t block_core_num = result_->block_core_table_.size();
	result_->block_core_table_.push_back(BlockCore());
	result_->block_core_table_.back().variable_symbol_offset = result_->symbol_table_->size();
	result_->block_core_table_.back().line_number = lines_.top();

	for(TList<Var>::Node* p = vars->vars.head; p; p = p->next){
		result_->block_core_table_.back().variable_size++;
		result_->symbol_table_->push_back(to_id(p->value.name));
	}

	if(vars->vars.size){
		if(vars->on_heap){
			put_inst(InstBlockBegin(block_core_num));
		}else{
			put_inst(InstBlockBeginDirect(vars->vars.size));
		}
	}
}

void CodeBuilder::block_end(){
	Vars* vars = vars_stack_.top();

	if(vars->vars.size){
		if(vars->on_heap){
			put_inst(InstBlockEnd());
		}else{
			put_inst(InstBlockEndDirect(vars->vars.size));
		}
	}
	vars_stack_.pop();
}

void CodeBuilder::class_begin(int_t kind, Vars* vars, int_t mixins){
	vars_stack_.push(vars);

	int_t class_core_num = result_->class_core_table_.size();
	result_->class_core_table_.push_back(ClassCore());
	result_->class_core_table_.back().variable_symbol_offset = result_->symbol_table_->size();
	result_->class_core_table_.back().line_number = lines_.top();
	result_->class_core_table_.back().kind = kind;

	for(TList<Var>::Node* p = vars->vars.head; p; p = p->next){
		result_->class_core_table_.back().variable_size++;
		result_->symbol_table_->push_back(to_id(p->value.name));
	}

	put_inst(InstClassBegin(class_core_num, mixins));
}

void CodeBuilder::class_end(){
	put_inst(InstClassEnd());
	vars_stack_.pop();
}

void CodeBuilder::fun_begin(int_t kind, Vars* vars, bool have_args, int_t offset, u8 min_param_count, u8 max_param_count){

	FunFrame& f = fun_frames_.push();	
	f.used_args_object = false;
	f.labels.clear();
	f.loops.clear();
	f.finallys.clear();
	f.frame_count = vars_stack_.size();
	f.used_args_object = have_args;

	vars_stack_.push(vars);

	FunCore core;
	core.variable_symbol_offset = result_->symbol_table_->size();
	core.pc = code_size()+offset;
	core.line_number = lines_.top();
	core.kind = kind;
	core.min_param_count = min_param_count;
	core.max_param_count = max_param_count;
	core.used_args_object = have_args;
	core.on_heap = vars->on_heap;

	for(TList<Var>::Node* p = vars->vars.head; p; p = p->next){
		core.variable_size++;
		result_->symbol_table_->push_back(to_id(p->value.name));
	}
	result_->xfun_core_table_.push_back(core);
}

void CodeBuilder::fun_end(){
	vars_stack_.pop();
	process_labels();
	fun_frames_.downsize(1);
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
	result_->set_line_number_info(ex->line);

	switch(ex->type){

		XTAL_NODEFAULT;

		XTAL_EXPR_CASE(Expr){
			(void)e;
		}

		XTAL_EXPR_CASE(PseudoVariableExpr){
			put_inst(Inst(e->code));
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
			int_t iter_first = com_->register_ident(InternedStringPtr("iter_first")); 
			int_t iter_next = com_->register_ident(InternedStringPtr("iter_next")); 
			int_t iter_break = com_->register_ident(InternedStringPtr("iter_break")); 

			if(e->var==iter_first && !info.tail){
				put_inst(InstSendIterFirst(info.need_result_count));
			}else if(e->var==iter_next && !info.tail){
				put_inst(InstSendIterNext(info.need_result_count));
			}else if(e->var==iter_break && e->if_defined && !info.tail){
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

			if(e->kind==KIND_METHOD){

				// ゲッタか？
				if(e->params.size==0){
					if(BlockStmt* p = stmt_cast<BlockStmt>(e->stmt)){
						if(p->stmts.size==1){
							if(ReturnStmt* p2 = stmt_cast<ReturnStmt>(p->stmts.head->value)){
								if(p2->exprs.size==1){
									if(InstanceVariableExpr* p3 = expr_cast<InstanceVariableExpr>(p2->exprs.head->value)){
										put_inst(InstMakeInstanceVariableAccessor(0, lookup_instance_variable(p3->var), class_scopes_.empty() ? 0 : class_scopes_.top()->frame_number));
										break;
									}
								}
							}
						}
					}else if(ReturnStmt* p2 = stmt_cast<ReturnStmt>(e->stmt)){
						if(p2->exprs.size==1){
							if(InstanceVariableExpr* p3 = expr_cast<InstanceVariableExpr>(p2->exprs.head->value)){
								put_inst(InstMakeInstanceVariableAccessor(0, lookup_instance_variable(p3->var), class_scopes_.empty() ? 0 : class_scopes_.top()->frame_number));
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
										put_inst(InstMakeInstanceVariableAccessor(1, lookup_instance_variable(p3->var), class_scopes_.empty() ? 0 : class_scopes_.top()->frame_number));
										break;
									}
								}
							}
						}else if(AssignStmt* p2 = stmt_cast<AssignStmt>(e->stmt)){
							if(InstanceVariableExpr* p3 = expr_cast<InstanceVariableExpr>(p2->lhs)){
								put_inst(InstMakeInstanceVariableAccessor(1, lookup_instance_variable(p3->var), class_scopes_.empty() ? 0 : class_scopes_.top()->frame_number));
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

			int_t n = result_->xfun_core_table_.size();
			fun_begin(e->kind, &e->vars, e->have_args, InstMakeFun::ISIZE, minv, maxv);{

				int_t fun_end_label = reserve_label();

				set_jump(offsetof(InstMakeFun, address), fun_end_label);
				put_inst(InstMakeFun(n, 0));
				
				if(debug::is_enabled()){
					put_inst(InstBreakPoint(BREAKPOINT_CALL));
				}

				for(TPairList<int_t, Expr*>::Node* p = e->params.head; p; p = p->next){
					// デフォルト値を持つ
					if(p->value){
						
						int_t id = lookup_variable(p->key);
						bool on_heap = variable_on_heap(id);
						int_t label = reserve_label();
						
						if(on_heap){
							set_jump(offsetof(InstIfArgIsNull, address), label);
							put_inst(InstIfArgIsNull(id, 0));
						}else{
							set_jump(offsetof(InstIfArgIsNullDirect, address), label);
							put_inst(InstIfArgIsNullDirect(id, 0));
						}
						compile(p->value);
						put_set_local_code(p->key);
						
						set_label(label);
					}
				}
				compile(e->stmt);
				break_off(fun_frame().frame_count+1);
				if(debug::is_enabled()){
					put_inst(InstBreakPoint(BREAKPOINT_RETURN));
				}
				put_inst(InstReturn0());
				set_label(fun_end_label);
			}fun_end();
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

			class_begin(e->kind, &e->vars, e->mixins.size);{
				class_scopes_.push(e);
				result_->class_core_table_.back().instance_variable_symbol_offset = result_->symbol_table_->size();
				result_->class_core_table_.back().instance_variable_size = e->inst_vars.size;
				class_scopes_.top()->frame_number = result_->class_core_table_.size()-1;

				for(TPairList<int_t, Expr*>::Node* p=e->inst_vars.head; p; p=p->next){
					result_->symbol_table_->push_back(to_id(p->key));
				}

				for(TList<Var>::Node* p = e->vars.vars.head; p; p = p->next){
					compile(p->value.init);
					compile(p->value.ns);
					put_inst(InstDefineClassMember(lookup_variable(p->value.name), p->value.name, p->value.accessibility));
				}
				class_scopes_.downsize(1);
			}class_end();
		}
	}
	
	result_->set_line_number_info(ex->line);
	lines_.pop();

	if(info.need_result_count!=result_count){
		put_inst(InstAdjustResult(result_count, info.need_result_count));
	}
}

void CodeBuilder::compile(Stmt* ex){

	if(!ex)
		return;

	if(debug::is_enabled() && lines_.top()!=ex->line){
		put_inst(InstBreakPoint(BREAKPOINT_LINE));
	}

	lines_.push(ex->line);
	result_->set_line_number_info(ex->line);

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
				int_t id = lookup_variable(p->var);
				if(id>=0){
					if(variable_on_heap(id) || id>=256){
						if(e->code == InstInc::NUMBER){
							put_inst(InstLocalVariableInc(id));
						}else{
							put_inst(InstLocalVariableDec(id));
						}
						put_inst(InstSetLocalVariable2Byte(id));
					}else{
						if(e->code == InstInc::NUMBER){
							put_inst(InstLocalVariableIncDirect(id));
						}else{
							put_inst(InstLocalVariableDecDirect(id));
						}
						put_inst(InstSetLocalVariable1ByteDirect(id));
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
			for(uint_t scope_count = vars_stack_.size(); scope_count!=(uint_t)fun_frame().frame_count+1; scope_count--){
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
				
				break_off(fun_frame().frame_count+1);

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
			exc.frame_count = vars_stack_.size();
			exc.finally_label = finally_label;
			fun_frame().finallys.push(exc);

			compile(e->try_stmt);
			
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
				exc.frame_count = vars_stack_.size();
				exc.finally_label = finally_label;
				fun_frame().finallys.push(exc);

				block_begin(&e->catch_vars);{
					if(e->catch_vars.vars.head){
						put_set_local_code(e->catch_vars.vars.head->value.name);
					}
					compile(e->catch_stmt);
				}block_end();

				put_inst(InstTryEnd());
				fun_frame().finallys.pop();

				result_->except_core_table_[core2].finally_pc = code_size();
				result_->except_core_table_[core2].end_pc = code_size();
			}
			
			set_label(finally_label);

			result_->except_core_table_[core].finally_pc = code_size();

			// finally節のコードを埋め込む
			compile(e->finally_stmt);
			
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

			compile(e->body_stmt);
			
			if(e->else_stmt){
				set_jump(offsetof(InstGoto, address), label_end);
				put_inst(InstGoto());
			}
			
			set_label(label_if);
			set_label(label_if2);
			compile(e->else_stmt);
		
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
				compile(e->next_stmt);
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
			compile(e->body_stmt);
			pop_loop(); 
			
			set_jump(offsetof(InstGoto, address), label_cond);
			put_inst(InstGoto());			

			set_label(label_if);
			set_label(label_if2);
			if(e->nobreak_stmt){
				compile(e->nobreak_stmt);
			}else{
				compile(e->else_stmt);
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

			if(2<=e->lhs.size && e->lhs.size<=4){
				bool bad = false;
				int_t list[4];
				int_t i = 0;
				for(TList<Expr*>::Node* lhs=e->lhs.head; lhs; lhs=lhs->next){
					if(LocalExpr* e2 = expr_cast<LocalExpr>(lhs->value)){
						int_t id = lookup_variable(e2->var);
						if(id>=256 || id<0 || variable_on_heap(id)){
							bad = true;
							break;
						}
						list[i] = id;
					}else{
						bad = true;
						break;
					}
					++i;
				}

				if(!bad){
					switch(i){
					case 2: put_inst(InstSetMultipleLocalVariable2Direct(list[0], list[1])); break;
					case 3: put_inst(InstSetMultipleLocalVariable3Direct(list[0], list[1], list[2])); break;
					case 4: put_inst(InstSetMultipleLocalVariable4Direct(list[0], list[1], list[2], list[3])); break;
					}
					break;
				}
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
			block_begin(&e->vars);{
				for(TList<Stmt*>::Node* p = e->stmts.head; p; p = p->next){
					compile(p->value);
				}
			}block_end();
		}
		
		XTAL_EXPR_CASE(TopLevelStmt){
			block_begin(&e->vars);{
				for(TList<Stmt*>::Node* p = e->stmts.head; p; p = p->next){
					compile(p->value);
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
			
			process_labels();
			put_inst(InstThrow());
		}	
	}

	result_->set_line_number_info(ex->line);
	lines_.pop();
}

}

#endif

