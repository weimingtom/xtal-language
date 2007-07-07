
#include "xtal.h"

#ifndef XTAL_NO_PARSER

#include "xtal_codebuilder.h"
#include "xtal_expr.h"
#include "xtal_parser.h"
#include "xtal_constant.h"
#include "xtal_funimpl.h"
#include "xtal_codeimpl.h"
#include "xtal_streamimpl.h"
#include "xtal_fun.h"
#include "xtal_vmachineimpl.h"


namespace xtal{

CodeBuilder::CodeBuilder(){

}

CodeBuilder::~CodeBuilder(){

}

Fun CodeBuilder::compile(const Stream& stream, const String& source_file_name){

	result_ = Code();
	
	p_ = result_.impl();
	p_->source_file_name_ = source_file_name;

	p_->except_core_table_.push_back(ExceptCore());

	lines_.push(1);
	fun_frame_begin(true, 0, 0, 0, false);
	Stmt* ep = parser_.parse(stream, source_file_name);
	com_ = parser_.common();
	p_->symbol_table_ = com_->ident_table;
	p_->value_table_ = com_->value_table;
	
	if(ep){
		compile(ep);
	}

	parser_.release();
	Code ret = result_;
	result_ = null;

	if(com_->errors.size()==0){
		Fun p(null, null, ret, &ret.impl()->xfun_core_table_[0]);
		p.set_object_name("<TopLevel>", 1, null);
		return p;
	}else{
		return null;
	}
}

void CodeBuilder::interactive_compile(){
	result_ = Code();
	p_ = result_.impl();
	p_->source_file_name_ = "<ix>";

	p_->except_core_table_.push_back(ExceptCore());

	lines_.push(1);
	fun_frame_begin(true, 0, 0, 0, false);
	Fun fun(null, null, result_, &p_->xfun_core_table_[0]);
	fun.set_object_name("<TopLevel>", 1, null);

	Stream stream;
	new(stream) InteractiveStreamImpl();
	parser_.begin_interactive_parsing(stream);

	int_t pc_pos = 0;
	
	while(true){
		Stmt* ep = parser_.interactive_parse();
		((InteractiveStreamImpl*)stream.impl())->set_continue_stmt(false);
		com_ = parser_.common();

		p_->symbol_table_ = com_->ident_table;
		p_->value_table_ = com_->value_table;
		com_->register_ident("toplevel");
		
		if(ep && com_->errors.empty()){
			compile(ep);
		}else{
			if(com_->errors.empty()){
				break;
			}
			com_->error(1, Xt("Xtal Compile Error 1001"));
		}
	
		if(com_->errors.size()==0){
			process_labels();
			put_inst(InstReturn0());
			put_inst(InstThrow());

			fun.set_core(&p_->xfun_core_table_[0]);
			XTAL_TRY{
				vmachine().impl()->execute(fun, &p_->code_[pc_pos]);
			}XTAL_CATCH(e){
				printf("%s\n", e.to_s().c_str());
			}
			
			p_->code_.pop_back();
			pc_pos = p_->code_.size();

		}else{
			printf("Error: %s\n", errors().at(0).to_s().c_str());
			com_->errors.clear();
		}
	}
}


Array CodeBuilder::errors(){
	return com_->errors;
}

ID CodeBuilder::to_id(int_t ident){
	return (ID&)com_->ident_table[ident];
}

int_t CodeBuilder::lookup_variable(int_t key){
	for(size_t i = 0, last = variables_.size(); i<last; ++i){
		if(variables_[i]==key){
			return i;
		}
	}
	return -1;
}

bool CodeBuilder::variable_on_heap(int_t pos){
	if(debug::is_enabled()){
		return true;
	}

	for(size_t i = 0, last = scopes_.size(); i<last; ++i){
		if(pos<scopes_[i].variable_size){
			return scopes_[i].on_heap;
		}
		pos -= scopes_[i].variable_size;
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
		}else if(id>=6){
			if(id<=0xff){
				put_inst(InstSetLocalVariable1ByteDirect(id));
			}else{
				put_inst(InstSetLocalVariable2Byte(id));
			}
		}
		else if(id == 0){ put_inst(InstSetLocalVariable0Direct()); }
		else if(id == 1){ put_inst(InstSetLocalVariable1Direct()); }
		else if(id == 2){ put_inst(InstSetLocalVariable2Direct()); }
		else if(id == 3){ put_inst(InstSetLocalVariable3Direct()); }
		else if(id == 4){ put_inst(InstSetLocalVariable4Direct()); }
		else if(id == 5){ put_inst(InstSetLocalVariable5Direct()); }
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
		}else if(id>=6){
			if(id<=0xff){
				put_inst(InstLocalVariable1ByteDirect(id));
			}else{
				put_inst(InstLocalVariable2Byte(id));
			}
		}
		else if(id == 0){ put_inst(InstLocalVariable0Direct()); }
		else if(id == 1){ put_inst(InstLocalVariable1Direct()); }
		else if(id == 2){ put_inst(InstLocalVariable2Direct()); }
		else if(id == 3){ put_inst(InstLocalVariable3Direct()); }
		else if(id == 4){ put_inst(InstLocalVariable4Direct()); }
		else if(id == 5){ put_inst(InstLocalVariable5Direct()); }
		return true;
	}else{
		put_inst(InstGlobalVariable(var));
		return false;
	}
}

void CodeBuilder::put_send_code(int_t var, Expr* pvar, int_t need_result_count, bool discard, bool tail, bool if_defined){
	if(pvar){
		compile(pvar);
	}	
	
	if(if_defined){
		if(tail){
			put_inst(InstSendIfDefined_T(0, 0, discard ? -need_result_count : need_result_count, pvar ? 0 : var));
		}else{
			put_inst(InstSendIfDefined(0, 0, discard ? -need_result_count : need_result_count, pvar ? 0 : var));
		}
	}else{
		if(tail){
			put_inst(InstSend_T(0, 0, discard ? -need_result_count : need_result_count, pvar ? 0 : var));
		}else{
			put_inst(InstSend(0, 0, discard ? -need_result_count : need_result_count, pvar ? 0 : var)); 
		}
	}
}

void CodeBuilder::put_set_send_code(int_t var, Expr* pvar, bool if_defined){
	if(pvar){
		ExprBuilder& e = *parser_.expr_builder();
		compile(e.bin(InstCat::NUMBER, e.string(com_->register_value("set_")), pvar));
	}	
	
	int_t symbol_number = com_->register_ident(String("set_", 4, to_id(var).c_str(), to_id(var).size()));
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
	com_->error(line(), Xt("Xtal Compile Error 1023")(Named("name", String("_").cat(to_id(key)))));
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
			inst_i16_t& buf = *(inst_i16_t*)&p_->code_[f.set_pos];
			buf = l.pos - f.pos;
		}
	}
	fun_frames_.top().labels.clear();
}

void CodeBuilder::break_off(int_t n){
	for(uint_t scope_count = scopes_.size(); scope_count!=(uint_t)n; scope_count--){
		for(uint_t k = 0; k<fun_frame().finallys.size(); ++k){
			if((uint_t)fun_frame().finallys[k].frame_count==scope_count){
				int_t label = reserve_label();
				set_jump(offsetof(InstPushGoto, address), label);
				put_inst(InstPushGoto());
				put_inst(InstTryEnd());
				set_label(label);
			}
		}
		if(scopes_[scopes_.size()-scope_count].type!=SCOPE){
			put_inst(InstBlockEnd());
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
	loop.frame_count = scopes_.size();
	loop.have_label = have_label;
	fun_frames_.top().loops.push(loop);
}

void CodeBuilder::pop_loop(){
	fun_frames_.top().loops.pop();
}

void CodeBuilder::set_on_heap_flag(){
	for(int_t i = 0; i<(int_t)scopes_.size(); ++i){
		scopes_[i].on_heap = true;	
	}
}

void CodeBuilder::block_begin(int_t type, int_t kind, TList<int_t>& vars, bool on_heap, int_t mixins){
	Scope s;
	s.variable_size = 0;
	s.type = type;
	s.kind = kind;
	s.on_heap = on_heap;
	s.mixins = mixins;
	s.frame_core_num = p_->frame_core_table_.size();

	// もしヒープに乗るスコープが来たなら、今まで積んだ全てのスコープをヒープに乗せるフラグを立てる
	if(on_heap || debug::is_enabled()){
		set_on_heap_flag();
	}
	scopes_.push(s);

	p_->frame_core_table_.push_back(FrameCore());
	p_->frame_core_table_.back().kind = scopes_.top().kind;
	p_->frame_core_table_.back().variable_symbol_offset = p_->symbol_table_.size();
	p_->frame_core_table_.back().line_number = lines_.top();

	for(TList<int_t>::Node* p = vars.head; p; p = p->next){
		variables_.push(p->value);
		scopes_.top().variable_size++;
		p_->frame_core_table_.back().variable_size++;
		p_->symbol_table_.push_back(to_id(p->value));
	}

	if(scopes_.top().type==FRAME){
		put_inst(InstClassBegin(s.frame_core_num, scopes_.top().mixins));
	}else if(scopes_.top().type==FUN){
		
	}else{
		if(scopes_.top().variable_size){
			put_inst(InstBlockBegin(s.frame_core_num));
		}else{
			scopes_.top().type = SCOPE;
		}
	}
}

void CodeBuilder::block_end(){
	if(scopes_.top().type==FRAME){
		variables_.downsize(scopes_.top().variable_size);
		put_inst(InstClassEnd());
	}else if(scopes_.top().type==FUN){
		variables_.downsize(scopes_.top().variable_size);
	}else{
		if(scopes_.top().variable_size){
			variables_.downsize(scopes_.top().variable_size);
			if(scopes_.top().on_heap || debug::is_enabled()){
				put_inst(InstBlockEnd());
			}else{
				put_inst(InstBlockEndDirect());
			}
		}
	}
	scopes_.pop();
}

int_t CodeBuilder::code_size(){
	return p_->code_.size();
}

int_t CodeBuilder::fun_frame_begin(bool have_args, int_t offset, unsigned char min_param_count, unsigned char max_param_count, bool extra_comma){
	FunFrame& f = fun_frames_.push();	
	f.used_args_object = false;
	f.labels.clear();
	f.loops.clear();
	f.finallys.clear();
	f.frame_count = scopes_.size();

	p_->xfun_core_table_.push_back(FunCore());
	p_->xfun_core_table_.back().variable_symbol_offset = p_->symbol_table_.size();
	p_->xfun_core_table_.back().pc = code_size()+offset;
	p_->xfun_core_table_.back().line_number = lines_.top();
	p_->xfun_core_table_.back().min_param_count = min_param_count;
	p_->xfun_core_table_.back().max_param_count = max_param_count;
	p_->xfun_core_table_.back().used_args_object = have_args;
	p_->xfun_core_table_.back().extra_comma = extra_comma;
	fun_frame().used_args_object = have_args;
		
	if(debug::is_enabled()){
		set_on_heap_flag();
	}

	return p_->xfun_core_table_.size()-1;
}

void CodeBuilder::register_param(int_t name){
	p_->xfun_core_table_.back().variable_size++;
	p_->symbol_table_.push_back(to_id(name));
}

void CodeBuilder::fun_frame_end(){
	process_labels();
	fun_frames_.downsize(1);
}

CodeBuilder::FunFrame &CodeBuilder::fun_frame(){
	return fun_frames_.top();
}

#define XTAL_EXPR_CASE(KEY) break; case KEY::TYPE: if(KEY* e = (KEY*)ex)if(e)

void CodeBuilder::compile(Expr* ex, int_t need_result_count, bool discard){

	int_t result_count = 1;

	if(!ex){
		put_inst(InstAdjustResult(0, discard ? -need_result_count : need_result_count));
		return;
	}

	/*
	if(debug::is_enabled() && lines_.top()!=ex->line){
		put_code_u8(CODE_BREAKPOINT);
		put_code_u8(BREAKPOINT_LINE);
	}
	*/
	
	lines_.push(ex->line);
	p_->set_line_number_info(ex->line);

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
			if(e->value==0){ put_inst(InstPushInt0());
			}else if(e->value==1){ put_inst(InstPushInt1());
			}else if(e->value==2){ put_inst(InstPushInt2());
			}else if(e->value==3){ put_inst(InstPushInt3());
			}else if(e->value==4){ put_inst(InstPushInt4());
			}else if(e->value==5){ put_inst(InstPushInt5());
			}else if(e->value==(u8)e->value){ put_inst(InstPushInt1Byte(e->value));
			}else if(e->value==(u16)e->value){ put_inst(InstPushInt2Byte(e->value));
			}else{ put_inst(InstValue(com_->register_value(e->value)));
			}
		}

		XTAL_EXPR_CASE(FloatExpr){
			if(e->value==0){ put_inst(InstPushFloat0());
			}else if(e->value==0.25f){ put_inst(InstPushFloat025());
			}else if(e->value==0.5f){ put_inst(InstPushFloat05());
			}else if(e->value==1){ put_inst(InstPushFloat1());
			}else if(e->value==2){ put_inst(InstPushFloat2());
			}else if(e->value==3){ put_inst(InstPushFloat3());
			}else{ put_inst(InstValue(com_->register_value(e->value)));
			}
		}

		XTAL_EXPR_CASE(StringExpr){
			if(e->kind==KIND_TEXT){
				put_inst(InstValue(com_->register_value(get_text(cast<String>(com_->value_table[e->value]).c_str()))));
			}else if(e->kind==KIND_FORMAT){
				put_inst(InstValue(com_->register_value(format(cast<String>(com_->value_table[e->value]).c_str()))));
			}else{
				put_inst(InstValue(com_->register_value(com_->value_table[e->value])));
			}
		}

		XTAL_EXPR_CASE(ArrayExpr){
			int_t count = 0;
			TList<Expr*>::Node* p;
			for(p = e->values.head; p; p = p->next){
				compile(p->value);
				count++;
				if(count>32){
					break;
				}
			}

			put_inst(InstMakeArray(count));
			for(; p; p = p->next){
				compile(p->value);
				put_inst(InstArrayAppend());				
			}
		}

		XTAL_EXPR_CASE(MapExpr){
			int_t count = 0;
			TPairList<Expr*, Expr*>::Node* p;
			for(p = e->values.head; p; p = p->next){
				compile(p->key);
				compile(p->value);
				count++;
				if(count>16){
					break;
				}
			}

			put_inst(InstMakeMap(count));
			for(; p; p = p->next){
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
			int_t num = com_->append_value(nop);
			put_inst(InstOnce(0, num));
						
			compile(e->expr);
			put_inst(InstDup());
			
			put_inst(InstSetValue(num));
			
			set_label(label_end);	
		}

		XTAL_EXPR_CASE(SendExpr){
			compile(e->lhs);

			int_t iter_first = com_->register_ident(ID("iter_first")); 
			int_t iter_next = com_->register_ident(ID("iter_next")); 
			int_t iter_break = com_->register_ident(ID("iter_break")); 

			if(e->var==iter_first && !e->tail){
				put_inst(InstSendIterFirst(e->discard ? -need_result_count : need_result_count));
			}else if(e->var==iter_next && !e->tail){
				put_inst(InstSendIterNext(e->discard ? -need_result_count : need_result_count));
			}else if(e->var==iter_break && e->if_defined && !e->tail){
				put_inst(InstSendIterBreak(e->discard ? -need_result_count : need_result_count));
			}else{
				put_send_code(e->var, e->pvar, need_result_count, e->discard, e->tail, e->if_defined);
			}
			result_count = need_result_count;
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
					if(e->tail){
						if(have_args){ put_inst(InstSendIfDefined_AT(ordered, named, e->discard ? -need_result_count : need_result_count, e2->pvar ? 0 : e2->var));
						}else{ put_inst(InstSendIfDefined_T(ordered, named, e->discard ? -need_result_count : need_result_count, e2->pvar ? 0 : e2->var)); }
					}else{
						if(have_args){ put_inst(InstSendIfDefined_A(ordered, named, e->discard ? -need_result_count : need_result_count, e2->pvar ? 0 : e2->var));
						}else{ put_inst(InstSendIfDefined(ordered, named, e->discard ? -need_result_count : need_result_count, e2->pvar ? 0 : e2->var)); }
					}
				}else{
					if(e->tail){
						if(have_args){ put_inst(InstSend_AT(ordered, named, e->discard ? -need_result_count : need_result_count, e2->pvar ? 0 : e2->var));
						}else{ put_inst(InstSend_T(ordered, named, e->discard ? -need_result_count : need_result_count, e2->pvar ? 0 : e2->var)); }
					}else{
						if(have_args){ put_inst(InstSend_A(ordered, named, e->discard ? -need_result_count : need_result_count, e2->pvar ? 0 : e2->var));
						}else{ put_inst(InstSend(ordered, named, e->discard ? -need_result_count : need_result_count, e2->pvar ? 0 : e2->var)); }
					}
				}
			}else if(expr_cast<CalleeExpr>(e->expr)){
				if(e->tail){
					if(have_args){ put_inst(InstCallCallee_AT(ordered, named, e->discard ? -need_result_count : need_result_count));
					}else{ put_inst(InstCallCallee_T(ordered, named, e->discard ? -need_result_count : need_result_count)); }
				}else{
					if(have_args){ put_inst(InstCallCallee_A(ordered, named, e->discard ? -need_result_count : need_result_count));
					}else{ put_inst(InstCallCallee(ordered, named, e->discard ? -need_result_count : need_result_count)); }
				}
			}else{
				compile(e->expr);
				if(e->tail){
					if(have_args){ put_inst(InstCall_AT(ordered, named, e->discard ? -need_result_count : need_result_count));
					}else{ put_inst(InstCall_T(ordered, named, e->discard ? -need_result_count : need_result_count)); }
				}else{
					if(have_args){ put_inst(InstCall_A(ordered, named, e->discard ? -need_result_count : need_result_count));
					}else{ put_inst(InstCall(ordered, named, e->discard ? -need_result_count : need_result_count)); }
				}
			}

			result_count = need_result_count;
		}

		XTAL_EXPR_CASE(FunExpr){

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

			int_t n = fun_frame_begin(e->have_args, InstMakeFun::ISIZE, minv, maxv, e->extra_comma);
			
			for(TPairList<int_t, Expr*>::Node* p = e->params.head; p; p = p->next){ 
				register_param(p->key);
			}

			int_t fun_end_label = reserve_label();

			set_jump(offsetof(InstMakeFun, address), fun_end_label);
			put_inst(InstMakeFun(e->kind, n, 0));
			
			if(debug::is_enabled()){
				put_inst(InstBreakPoint(BREAKPOINT_CALL));
			}

			block_begin(FUN, 0, e->vars, e->on_heap);{
				for(TPairList<int_t, Expr*>::Node* p = e->params.head; p; p = p->next){
					// デフォルト値を持つ
					if(p->value){
						
						int_t id = lookup_variable(p->key);
						int_t label = reserve_label();
						
						set_jump(offsetof(InstIfArgIsNull, address), label);
						put_inst(InstIfArgIsNull(id, 0));
					
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
			}block_end();

			set_label(fun_end_label);
			fun_frame_end();
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

		XTAL_EXPR_CASE(FrameExpr){
			block_begin(FRAME, e->kind, e->vars, e->on_heap);{
				for(TList<Stmt*>::Node* p = e->stmts.head; p; p = p->next){
					compile(p->value);
				}
			}block_end();
		}	

		XTAL_EXPR_CASE(ClassExpr){

			for(TList<Expr*>::Node* p = e->mixins.head; p; p = p->next){
				compile(p->value);
			}

			block_begin(FRAME, e->kind, e->vars, e->on_heap, e->mixins.size);{
				class_scopes_.push(e);
				p_->frame_core_table_.back().instance_variable_symbol_offset = 0;
				p_->frame_core_table_.back().instance_variable_size = e->inst_vars.size;
				class_scopes_.top()->frame_number = p_->frame_core_table_.size()-1;

				for(TList<Stmt*>::Node* p = e->stmts.head; p; p = p->next){
					compile(p->value);
				}
				class_scopes_.downsize(1);
			}block_end();
		}
	}
	
	p_->set_line_number_info(ex->line);
	lines_.pop();

	if(need_result_count!=result_count){
		put_inst(InstAdjustResult(result_count, discard ? -need_result_count : need_result_count));
	}
}

void CodeBuilder::compile(Stmt* ex){

	if(!ex)
		return;

	if(debug::is_enabled() && lines_.top()!=ex->line){
		put_inst(InstBreakPoint(BREAKPOINT_LINE));
	}

	lines_.push(ex->line);
	p_->set_line_number_info(ex->line);

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
				put_send_code(p->var, p->pvar, 1, p->discard, p->tail, p->if_defined);
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
				put_send_code(p->var, p->pvar, 1, p->discard, p->tail, p->if_defined);
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
		}

		XTAL_EXPR_CASE(ReturnStmt){

			bool have_finally = false;
			for(uint_t scope_count = scopes_.size(); scope_count!=(uint_t)fun_frame().frame_count+1; scope_count--){
				for(uint_t k = 0; k<(uint_t)fun_frame().finallys.size(); ++k){
					if((uint_t)fun_frame().finallys[k].frame_count==scope_count){
						have_finally = true;
					}
				}
			}

			if(!have_finally && e->exprs.size==1){
				if(CallExpr* ce = expr_cast<CallExpr>(e->exprs.head->value)){
					ce->tail = true;
					compile(ce);
					break;
				}else if(SendExpr* se = expr_cast<SendExpr>(e->exprs.head->value)){
					se->tail = true;
					compile(se);
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

			int_t core = p_->except_core_table_.size();
			put_inst(InstTryBegin(core));
			p_->except_core_table_.push_back(ExceptCore());

			CodeBuilder::FunFrame::Finally exc;
			exc.frame_count = scopes_.size();
			exc.finally_label = finally_label;
			fun_frame().finallys.push(exc);

			compile(e->try_stmt);
			
			set_jump(offsetof(InstPushGoto, address), end_label);
			put_inst(InstPushGoto());
			put_inst(InstTryEnd());

			// catch節のコードを埋め込む
			if(e->catch_stmt){

				p_->except_core_table_[core].catch_pc = code_size();
				
				// catch節の中での例外に備え、例外フレームを構築。
			
				int_t core2 = p_->except_core_table_.size();
				put_inst(InstTryBegin(core2));
				p_->except_core_table_.push_back(ExceptCore());

				CodeBuilder::FunFrame::Finally exc;
				exc.frame_count = scopes_.size();
				exc.finally_label = finally_label;
				fun_frame().finallys.push(exc);

				block_begin(BLOCK, 0, e->catch_vars, e->on_heap);{
					if(e->catch_vars.head){
						put_set_local_code(e->catch_vars.head->value);
					}
					compile(e->catch_stmt);
				}block_end();

				put_inst(InstTryEnd());
				fun_frame().finallys.pop();

				p_->except_core_table_[core2].finally_pc = code_size();
				p_->except_core_table_[core2].end_pc = code_size();
			}
			
			set_label(finally_label);

			p_->except_core_table_[core].finally_pc = code_size();

			// finally節のコードを埋め込む
			compile(e->finally_stmt);
			
			fun_frame().finallys.pop();

			put_inst(InstPopGoto());

			set_label(end_label);
			p_->except_core_table_[core].end_pc = code_size();
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

						if(CallExpr* ce = expr_cast<CallExpr>(rhs->value)){
							ce->discard = e->discard;
						}

						if(SendExpr* ce = expr_cast<SendExpr>(rhs->value)){
							ce->discard = e->discard;
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
						compile(rhs->value, rrc, e->discard);
						pushed_count += rrc;
						break;
					}
				}else{
					compile(rhs->value);
					pushed_count++;
				}
			}

			if(e->lhs.size!=pushed_count){
				put_inst(InstAdjustResult(pushed_count, e->discard ? -e->lhs.size : e->lhs.size));
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
			block_begin(BLOCK, 0, e->vars, e->on_heap);{
				for(TList<Stmt*>::Node* p = e->stmts.head; p; p = p->next){
					compile(p->value);
				}
			}block_end();
		}

		XTAL_EXPR_CASE(SetAccessibilityStmt){
			put_inst(InstSetAccessibility(e->var, e->kind));
		}
		
		XTAL_EXPR_CASE(TopLevelStmt){
			block_begin(BLOCK, 0, e->vars, e->on_heap);{
				for(TList<Stmt*>::Node* p = e->stmts.head; p; p = p->next){
					compile(p->value);
				}
				
				break_off(1);
				if(e->export_expr){
					compile(e->export_expr);
					put_inst(InstReturn1());
				}else{
					put_inst(InstReturn0());
				}
			}block_end();
			
			process_labels();
			put_inst(InstThrow());
		}	
	}

	p_->set_line_number_info(ex->line);
	lines_.pop();
}

}

#endif

