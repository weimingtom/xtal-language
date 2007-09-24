
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

CodePtr CodeBuilder::compile(const StreamPtr& stream, const StringPtr& source_file_name){
	error_= &errorimpl_;
	error_->init(source_file_name);
	ExprPtr e = parser_.parse(stream, error_);
	if(!e){
		return null;
	}
	return compile_toplevel(e, source_file_name);
}

void CodeBuilder::interactive_compile(){
	error_= &errorimpl_;
	error_->init("<ix>");

	{
		StreamPtr ss = xnew<StringStream>(";");
		ExprPtr e = parser_.parse_stmt(ss, error_);
		compile_toplevel(e, "<ix>");
		result_->code_.clear();
	}

	result_->xfun_core_table_.reserve(10000);
	result_->class_core_table_.reserve(10000);
	result_->block_core_table_.reserve(10000);
	result_->except_core_table_.reserve(10000);

	result_->first_fun()->set_core(&result_->xfun_core_table_[0]);

	int_t pc_pos = 0;
	InteractiveStreamPtr stream = xnew<InteractiveStream>();
	while(!stream->eof()){

		ExprPtr e = parser_.parse_stmt(stream, error_);
		stream->terminate_statement();

		if(!e){
			stderr_stream()->put_s(error_->errors->join("\n"));
			stderr_stream()->put_s("\n");
			error_->errors->clear();
		}else{

			if(e->type()==EXPR_RETURN){
				break;
			}

			fun_frames_.push();	
			ff().labels.clear();
			ff().loops.clear();
			ff().finallies.clear();
			ff().var_frame_count = var_frames_.size();
			ff().extendable_param = true;

			int_t last_code_size = code_size();
			compile_stmt(e);

			if(error_->errors->size()!=0){
				stderr_stream()->put_s(error_->errors->join("\n"));
				stderr_stream()->put_s("\n");
				error_->errors->clear();				
				result_->code_.resize(last_code_size);
				fun_frames_.downsize(1);
			}else{

				process_labels();

				fun_frames_.downsize(1);

				result_->inspect_range(last_code_size, code_size())->p();

				put_inst(InstReturn0());
				put_inst(InstThrow());

				XTAL_TRY{
					vmachine()->execute(result_->first_fun().get(), &result_->code_[pc_pos]);
				}XTAL_CATCH(e){
					printf("%s\n", e->to_s()->c_str());
				}

				for(int_t i=0; i<(sizeof(InstThrow)+sizeof(InstReturn0))/sizeof(inst_t); ++i){
					result_->code_.pop_back();
				}

				pc_pos = result_->code_.size();
			}
		}
	}
}

CodePtr CodeBuilder::compile_toplevel(const ExprPtr& e, const StringPtr& source_file_name){
	result_ = xnew<Code>();
	result_->source_file_name_ = source_file_name;
	result_->except_core_table_.push_back(ExceptCore());

	result_->identifier_table_ = xnew<Array>();
	identifier_map_ = xnew<Map>();
	register_identifier("");

	result_->value_table_ = xnew<Array>();
	value_map_ = xnew<Map>();
	register_value(null);
	
	result_->once_table_ = xnew<Array>();
	result_->once_table_->push_back(nop);

	linenos_.push(1);

	// 関数フレームを作成する
	fun_frames_.push();	
	ff().labels.clear();
	ff().loops.clear();
	ff().finallies.clear();
	ff().var_frame_count = var_frames_.size();
	ff().extendable_param = true;

	// 変数フレームを作成して、引数を登録する
	var_begin(VarFrame::BLOCK);

	// 関数コアを作成
	FunCore core;
	core.pc = 0;
	core.lineno = linenos_.top();
	core.kind = KIND_FUN;
	core.min_param_count = 0;
	core.max_param_count = 0;
	core.flags = FunCore::FLAG_EXTENDABLE_PARAM;
	core.variable_size = 0;
	core.variable_identifier_offset = 0;

	int_t fun_core_table_number = 0;
	result_->xfun_core_table_.push_back(core);

	// 関数本体を処理する
	compile_stmt(e);
	
	break_off(ff().var_frame_count+1);

	put_inst(InstReturn0());
	put_inst(InstThrow());

	process_labels();
	
	if(vf().kind!=VarFrame::SCOPE){
		result_->xfun_core_table_[fun_core_table_number].flags |= FunCore::FLAG_ON_HEAP;
	}

	if(vf().scope_chain){
		result_->xfun_core_table_[fun_core_table_number].flags |= FunCore::FLAG_SCOPE_CHAIN;
		var_set_on_heap(1);
	}

	// 関数フレームをポップする
	fun_frames_.downsize(1);

	// 変数フレームをポップする
	var_end();

	if(error_->errors->size()==0){
		result_->first_fun()->set_core(&result_->xfun_core_table_[0]);
		return result_;
	}else{
		result_ = null;
		return null;
	}
}

void CodeBuilder::put_inst2(const Inst& t, uint_t sz){
	if(t.op==255){
		error_->error(lineno(), Xt("Xtal Compile Error 1027"));
	}

	size_t cur = result_->code_.size();
	result_->code_.resize(cur+sz/sizeof(inst_t));
	memcpy(&result_->code_[cur], &t, sz);
}

AnyPtr CodeBuilder::errors(){
	return error_->errors->each();
}

bool CodeBuilder::put_set_local_code(const InternedStringPtr& var){
	LVarInfo info = var_find(var);
	if(info.pos>=0){
		if(info.entry->constant){
			error_->error(lineno(), Xt("Xtal Compile Error 1019")(Named("name", var)));
		}

		if(info.pos<=0xff){
			var_set_direct(*info.var_frame);
			put_inst(InstSetLocalVariable1Byte(info.pos));
		}else{
			put_inst(InstSetLocalVariable2Byte(info.pos));
		}		

		return true;
	}else{
		put_inst(InstSetGlobalVariable(register_identifier(var)));
		return false;
	}
}

void CodeBuilder::put_define_local_code(const InternedStringPtr& var){
	LVarInfo info = var_find(var, true);

	if(info.pos>=0){
		if(info.pos<=0xff){
			var_set_direct(*info.var_frame);
			put_inst(InstSetLocalVariable1Byte(info.pos));
		}else{
			put_inst(InstSetLocalVariable2Byte(info.pos));
		}
	}else{
		put_inst(InstDefineGlobalVariable(register_identifier(var)));
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
		put_inst(InstGlobalVariable(register_identifier(var)));
		return false;
	}
}

void CodeBuilder::put_send_code(const InternedStringPtr& var, ExprPtr pvar, int_t need_result_count, bool tail, bool if_defined){
	if(pvar){
		compile_expr(pvar);
	}	
	
	if(if_defined){
		if(tail){
			put_inst(InstSendIfDefined_T(0, 0, need_result_count, pvar ? 0 : register_identifier(var)));
		}else{
			put_inst(InstSendIfDefined(0, 0, need_result_count, pvar ? 0 : register_identifier(var)));
		}
	}else{
		if(tail){
			put_inst(InstSend_T(0, 0, need_result_count, pvar ? 0 : register_identifier(var)));
		}else{
			put_inst(InstSend(0, 0, need_result_count, pvar ? 0 : register_identifier(var))); 
		}
	}
}

void CodeBuilder::put_set_send_code(const InternedStringPtr& var, ExprPtr pvar, bool if_defined){
	if(pvar){
		compile_expr(bin(0, EXPR_CAT, string(0, KIND_STRING, "set_"), pvar));
	}	
	
	InternedStringPtr identifier_number = xnew<String>("set_", 4)->cat(var);
	bool tail = false;

	if(if_defined){
		if(tail){
			put_inst(InstSendIfDefined_T(1, 0, 0, pvar ? 0 : register_identifier(identifier_number)));
		}else{
			put_inst(InstSendIfDefined(1, 0, 0, pvar ? 0 : register_identifier(identifier_number)));
		}
	}else{
		if(tail){
			put_inst(InstSend_T(1, 0, 0, pvar ? 0 : register_identifier(identifier_number)));
		}else{
			put_inst(InstSend(1, 0, 0, pvar ? 0 : register_identifier(identifier_number))); 
		}
	}
}

void CodeBuilder::put_member_code(const InternedStringPtr& var, ExprPtr pvar, bool if_defined){
	if(pvar){
		compile_expr(pvar);
	}
	
	if(if_defined){
		InstMemberIfDefined member;
		member.identifier_number = pvar ? 0 : register_identifier(var);
		put_inst(member);
	}else{
		InstMember member;
		member.identifier_number = pvar ? 0 : register_identifier(var);
		put_inst(member);
	}
}

void CodeBuilder::put_define_member_code(const InternedStringPtr& var, ExprPtr pvar){
	if(pvar){
		compile_expr(pvar);
	}

	InstDefineMember member;
	member.identifier_number = pvar ? 0 : register_identifier(var);
	put_inst(member);
}

int_t CodeBuilder::lookup_instance_variable(const InternedStringPtr& key){
	if(!class_frames_.empty()){
		int ret = 0;
		ClassFrame& cf = class_frames_.top();
		for(size_t i = 0, last = cf.entries.size(); i<last; ++i){
			if(raweq(cf.entries[i].name, key)){
				return ret;
			}
			ret++;
		}
	}

	error_->error(lineno(), Xt("Xtal Compile Error 1023")(Named("name", xnew<String>("_")->cat(key))));
	return 0;
}

void CodeBuilder::put_set_instance_variable_code(const InternedStringPtr& var){
	put_inst(InstSetInstanceVariable(lookup_instance_variable(var), class_core_num()));
}

void CodeBuilder::put_instance_variable_code(const InternedStringPtr& var){
	put_inst(InstInstanceVariable(lookup_instance_variable(var), class_core_num()));
}

int_t CodeBuilder::reserve_label(){
	ff().labels.resize(fun_frames_.top().labels.size()+1);
	return ff().labels.size()-1;
}

void CodeBuilder::set_label(int_t labelno){
	ff().labels[labelno].pos = code_size();
}

void CodeBuilder::set_jump(int_t offset, int_t labelno){
	FunFrame::Label::From f;
	f.lineno = linenos_.top();
	f.pos = code_size();
	f.set_pos = f.pos + offset/sizeof(inst_t);
	ff().labels[labelno].froms.push_back(f);
}

void CodeBuilder::process_labels(){
	for(size_t i = 0; i<ff().labels.size(); ++i){
		FunFrame::Label &l = ff().labels[i];
		for(size_t j = 0; j<l.froms.size(); ++j){
			FunFrame::Label::From &f = l.froms[j];
			inst_i16_t& buf = *(inst_i16_t*)&result_->code_[f.set_pos];
			buf = l.pos - f.pos;
		}
	}
	ff().labels.clear();
}

void CodeBuilder::break_off(int_t n){
	for(uint_t scope_count = var_frames_.size(); scope_count!=(uint_t)n; scope_count--){
		for(uint_t k = 0; k<ff().finallies.size(); ++k){
			if((uint_t)ff().finallies[k].frame_count==scope_count){
				int_t label = reserve_label();
				set_jump(offsetof(InstPushGoto, address), label);
				put_inst(InstPushGoto());
				put_inst(InstTryEnd());
				set_label(label);
			}
		}

		VarFrame& vf = var_frames_[var_frames_.size()-scope_count];
		if(vf.entries.size() && (vf.kind==VarFrame::SCOPE || vf.kind==VarFrame::BLOCK)){
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
			error_->error(lineno(), Xt("Xtal Compile Error 1025"));
		}
		if(is_comp_bin(e->bin_rhs())){
			error_->error(lineno(), Xt("Xtal Compile Error 1025"));
		}
		
		compile_expr(e->bin_lhs());
		compile_expr(e->bin_rhs());

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
		compile_expr(e);

		set_jump(offsetof(InstIf, address), label_if);
		put_inst(InstIf());
	}
}

void CodeBuilder::scope_chain(int_t var_frame_size){
	for(int_t i=0; i<var_frame_size; ++i){
		var_frames_[i].scope_chain = true;
	}
}

CodeBuilder::LVarInfo CodeBuilder::var_find(const InternedStringPtr& key, bool define){
	LVarInfo ret = {0, 0, 0};
	for(size_t i = 0, last = var_frames_.size(); i<last; ++i){
		VarFrame& vf = var_frames_[i];
		for(size_t j = 0, jlast = vf.entries.size(); j<jlast; ++j){
			VarFrame::Entry& entry = vf.entries[vf.entries.size()-1-j];
			if(raweq(entry.name, key)){
				if(define){ entry.initialized = true; }
				if(vf.fun_frames_size!=fun_frames_.size() || entry.initialized){
					ret.var_frame = &vf;
					ret.entry = &entry;
					scope_chain(i);
					return ret;
				}
			}
			ret.pos++;
		}
	}
	ret.pos = -1;
	return ret;
}

void CodeBuilder::var_begin(int_t kind){
	var_frames_.push();
	vf().entries.clear();
	vf().directs.clear();
	vf().block_core_num = 0;
	vf().kind = kind;
	vf().fun_frames_size = fun_frames_.size();
	vf().scope_chain = false;
}

void CodeBuilder::var_define(const ArrayPtr& stmts){
	Xfor(v0, stmts->each()){
		if(!v0){
			continue;
		}

		ExprPtr v = ep(v0);
		if(ep(v)->type()==EXPR_DEFINE){
			if(v->bin_lhs()->type()==EXPR_LVAR){
				var_define(v->bin_lhs()->lvar_name());
			}
		}else if(v->type()==EXPR_MASSIGN){
			if(v->massign_define()){
				Xfor(v1, v->massign_lhs_exprs()){
					ExprPtr vv = ep(v1);
					if(vv->type()==EXPR_LVAR){
						var_define(vv->lvar_name());
					}			
				}
			}
		}
	}
}

void CodeBuilder::var_define(const InternedStringPtr& name, int_t accessibility, bool define, bool constant){
	for(size_t j = 0, jlast = vf().entries.size(); j<jlast; ++j){
		if(raweq(vf().entries[vf().entries.size()-1-j].name, name)){
			error_->error(lineno(), Xt("Xtal Compile Error 1026")(name));
			return;
		}
	}

	VarFrame::Entry entry;
	entry.name = name;
	entry.constant = constant;
	entry.initialized = define;
	entry.accessibility = accessibility;
	vf().entries.push_back(entry);
}

void CodeBuilder::var_set_direct(VarFrame& vf){
	VarFrame::Direct d;
	d.pos = code_size();
	vf.directs.push_back(d);
}

void CodeBuilder::var_set_on_heap(int_t i){
	if(i<var_frames_.size()){
		if(var_frames_[i].kind==VarFrame::SCOPE){
			var_frames_[i].kind = VarFrame::BLOCK;
		}
	}
}

void CodeBuilder::var_end(){
	if(vf().kind==VarFrame::SCOPE){
		for(uint_t i=0; i<vf().directs.size(); ++i){
			Inst* p = (Inst*)&result_->code_[vf().directs[i].pos];
			p->op += 1;
		}
	}
	var_frames_.downsize(1);
}

void CodeBuilder::block_begin(){
	int_t block_core_num = result_->block_core_table_.size();

	BlockCore core;
	core.variable_size = vf().entries.size();
	core.variable_identifier_offset = result_->identifier_table_->size();
	core.lineno = linenos_.top();

	vf().block_core_num = block_core_num;

	for(int_t i=0; i<vf().entries.size(); ++i){
		result_->identifier_table_->push_back(vf().entries[i].name);
	}

	if(vf().entries.size()){
		var_set_direct(vf());
		put_inst(InstBlockBegin(block_core_num));
	}

	result_->block_core_table_.push_back(core);
}

void CodeBuilder::block_end(){
	if(vf().entries.size()){
		var_set_direct(vf());
		put_inst(InstBlockEnd(vf().block_core_num));
	}

	if(vf().kind!=VarFrame::SCOPE){
		result_->block_core_table_[vf().block_core_num].flags |= FunCore::FLAG_ON_HEAP;
	}

	if(vf().scope_chain){
		result_->block_core_table_[vf().block_core_num].flags |= FunCore::FLAG_SCOPE_CHAIN;
		if(vf().kind!=VarFrame::SCOPE){
			var_set_on_heap(1);
		}
	}
}

int_t CodeBuilder::code_size(){
	return result_->code_.size();
}

void CodeBuilder::compile_bin(const ExprPtr& e){
	if(is_comp_bin(e->bin_lhs())){
		error_->error(lineno(), Xt("Xtal Compile Error 1013"));
	}
	if(is_comp_bin(e->bin_rhs())){
		error_->error(lineno(), Xt("Xtal Compile Error 1013"));
	}
	
	compile_expr(e->bin_lhs());
	compile_expr(e->bin_rhs());

	InstAdd inst;
	inst.op += e->type() - EXPR_ADD;
	put_inst(inst);
}

void CodeBuilder::compile_comp_bin(const ExprPtr& e){
	if(is_comp_bin(e->bin_lhs())){
		error_->error(lineno(), Xt("Xtal Compile Error 1025"));
	}
	if(is_comp_bin(e->bin_rhs())){
		error_->error(lineno(), Xt("Xtal Compile Error 1025"));
	}
	
	compile_expr(e->bin_lhs());
	compile_expr(e->bin_rhs());

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
		compile_expr(rhs);
		put_inst(inst);
		put_set_local_code(lhs->lvar_name());
	}else if(lhs->type()==EXPR_IVAR){
		put_instance_variable_code(lhs->ivar_name());
		compile_expr(rhs);
		put_inst(inst);
		put_set_instance_variable_code(lhs->ivar_name());
	}else if(lhs->type()==EXPR_SEND){
		compile_expr(lhs->send_term());
		put_inst(InstDup());
		put_send_code(lhs->send_name(), lhs->send_pname(), 1, false, lhs->send_q());
		compile_expr(rhs);
		put_inst(inst);
		put_inst(InstInsert1());
		put_set_send_code(lhs->send_name(), lhs->send_pname(), lhs->send_q());
	}else if(lhs->type()==EXPR_AT){
		compile_expr(lhs->bin_lhs());
		put_inst(InstDup());
		compile_expr(lhs->bin_rhs());
		put_inst(InstDup());
		put_inst(InstInsert2());
		put_inst(InstAt());
		compile_expr(rhs);
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
			put_inst(InstGlobalVariable(register_identifier(term->lvar_name())));
			put_inst(inst);
			put_set_local_code(term->lvar_name());
		}

	}else if(term->type()==EXPR_IVAR){
		put_instance_variable_code(term->ivar_name());
		put_inst(inst);
		put_set_instance_variable_code(term->ivar_name());
	}else if(term->type()==EXPR_SEND){
		compile_expr(term->send_term());
		put_inst(InstDup());
		put_send_code(term->send_name(), term->send_pname(), 1, false, term->send_q());
		put_inst(inst);
		put_inst(InstInsert1());
		put_set_send_code(term->send_name(), term->send_pname(), term->send_q());
	}else if(term->type()==EXPR_AT){
		compile_expr(term->bin_lhs());
		put_inst(InstDup());
		compile_expr(term->bin_rhs());
		put_inst(InstDup());
		put_inst(InstInsert2());
		put_inst(InstAt());
		put_inst(inst);
		put_inst(InstInsert2());
		put_inst(InstSetAt());		
	}
}

void CodeBuilder::compile_loop_control_statement(const ExprPtr& e){
	InternedStringPtr label;
	int_t label_kind;

	if(e->type()==EXPR_BREAK){
		label = e->break_label();
		label_kind = 0;
	}else if(e->type()==EXPR_CONTINUE){
		label = e->continue_label();
		label_kind = 1;
	}

	if(ff().loops.empty()){
		error_->error(lineno(), Xt("Xtal Compile Error 1006"));
	}else{
		if(label){
			bool found = false;
			for(int_t i = 0, last = ff().loops.size(); i<last; ++i){
				if(raweq(ff().loops[i].label, label)){
					break_off(ff().loops[i].frame_count);
					set_jump(offsetof(InstGoto, address), ff().loops[i].control_statement_label[label_kind]);
					put_inst(InstGoto());
					found = true;
					break;
				}
			}

			if(!found){
				error_->error(lineno(), Xt("Xtal Compile Error 1006"));
			}
		}else{
			bool found = false;
			for(size_t i = 0, last = ff().loops.size(); i<last; ++i){
				if(!ff().loops[i].have_label){
					break_off(ff().loops[i].frame_count);
					set_jump(offsetof(InstGoto, address), ff().loops[i].control_statement_label[label_kind]);
					put_inst(InstGoto());		
					found = true;
					break;
				}
			}

			if(!found){
				error_->error(lineno(), Xt("Xtal Compile Error 1006"));
			}
		}
	}
}

void CodeBuilder::compile_class(const ExprPtr& e){

	// インスタンス変数を暗黙的初期化するメソッドを定義する
	{
		ExprPtr init_method = fun(e->lineno(), KIND_METHOD, null, false, null);
		ArrayPtr stmts = xnew<Array>();
		ExprPtr block = scope(e->lineno(), stmts);
		Xfor2(k, v, e->class_ivars()){
			if(v){
				stmts->push_back(Expr::make(EXPR_ASSIGN)->set_bin_lhs(Expr::make(EXPR_IVAR)->set_ivar_name(k->to_s()->intern()))->set_bin_rhs(ptr_cast<Expr>(v)));
			}
		}
		init_method->set_fun_body(block);
		e->class_stmts()->push_front(cdefine(0, KIND_PUBLIC, Xid(_initialize_), null_(0), init_method));
	}

	// 継承
	Xfor(v, e->class_mixins()){
		compile_expr(v);
	}

	// 変数フレームを作成
	var_begin(VarFrame::CLASS);

	// 変数を定義
	Xfor(v, e->class_stmts()){
		ExprPtr v1 = ep(v);
		if(v1->type()==EXPR_CDEFINE){
			var_define(v1->cdefine_name(), v1->cdefine_accessibility(), true, true);
		}
	}

	class_frames_.push();
	cf().entries.clear();
	cf().class_core_num = result_->class_core_table_.size();

	Xfor2(k, v, e->class_ivars()){
		ClassFrame::Entry entry;
		entry.name = k->to_s()->intern();
		cf().entries.push_back(entry);
	}

	int_t class_core_num = result_->class_core_table_.size();

	ClassCore core;
	core.lineno = linenos_.top();
	core.kind = e->class_kind();
	core.mixins = e->class_mixins() ? e->class_mixins()->size() : 0;
	core.variable_size = vf().entries.size();
	core.instance_variable_size = e->class_ivars() ? e->class_ivars()->size() : 0;
	
	core.variable_identifier_offset = result_->identifier_table_->size();
	for(int_t i=0; i<vf().entries.size(); ++i){
		result_->identifier_table_->push_back(vf().entries[i].name);
	}

	core.instance_variable_identifier_offset = result_->identifier_table_->size();
	Xfor2(k, v, e->class_ivars()){
		result_->identifier_table_->push_back(k->to_s()->intern());
	}

	put_inst(InstClassBegin(class_core_num));
	result_->class_core_table_.push_back(core);

	{
		Xfor(v, e->class_stmts()){
			ExprPtr v1 = ep(v);
			if(v1->type()==EXPR_CDEFINE){					
				compile_expr(v1->cdefine_term());
				compile_expr(v1->cdefine_ns());
				LVarInfo info = var_find(v1->cdefine_name(), true);
				put_inst(InstDefineClassMember(info.pos, register_identifier(v1->cdefine_name()), v1->cdefine_accessibility()));
			}
		}
	}

	if(var_frames_.top().scope_chain){
		result_->class_core_table_[class_core_num].flags |= FunCore::FLAG_SCOPE_CHAIN;
		var_set_on_heap(1);
	}

	put_inst(InstClassEnd());
	class_frames_.pop();

	var_end();
}

void CodeBuilder::compile_fun(const ExprPtr& e){

	// ゲッター、またはセッター用の最適化を行う
	if(e->fun_kind()==KIND_METHOD){

		// ゲッタか？
		if(!e->fun_params() || e->fun_params()->size()==0){
			ExprPtr body = e->fun_body();
			if(body->type()==EXPR_SCOPE && body->scope_stmts() && body->scope_stmts()->size()==1){
				body = ep(body->scope_stmts()->front());
			}

			if(body->type()==EXPR_RETURN){
				if(body->return_exprs() && body->return_exprs()->size()==1){
					body = ep(body->return_exprs()->front());
					if(body->type()==EXPR_IVAR){
						put_inst(InstMakeInstanceVariableAccessor(0, lookup_instance_variable(body->ivar_name()), class_core_num()));
						return;
					}
				}
			}
		}

		// セッタか？
		if(e->fun_params() && e->fun_params()->size()==1){
			ExprPtr body = e->fun_body();
			if(body->type()==EXPR_SCOPE && body->scope_stmts() && body->scope_stmts()->size()==1){
				body = ep(body->scope_stmts()->front());
			}

			if(body->type()==EXPR_ASSIGN){
				ExprPtr lhs = body->bin_lhs();
				ExprPtr rhs = body->bin_rhs();

				InternedStringPtr key;
				Xfor2(k, v, e->fun_params()){ key = ptr_cast<String>(k); }

				if(lhs->type()==EXPR_IVAR && rhs->type()==EXPR_LVAR && raweq(rhs->lvar_name(), key)){
					put_inst(InstMakeInstanceVariableAccessor(1, lookup_instance_variable(lhs->ivar_name()), class_core_num()));
					return;
				}
			}
		}
	}

	// 引数の最大と最低を取得する
	int_t minv = -1, maxv = 0;
	{
		Xfor2(k, v, e->fun_params()){
			if(v){
				if(minv==-1){ minv = maxv; }
			}else{
				if(minv!=-1){ error_->error(lineno(), Xt("Xtal Compile Error 1001")); }
			}
			maxv++;
		}

		if(minv==-1){
			minv = maxv;
		}
	}

	// 関数フレームを作成する
	fun_frames_.push();	
	ff().labels.clear();
	ff().loops.clear();
	ff().finallies.clear();
	ff().var_frame_count = var_frames_.size();
	ff().extendable_param = e->fun_extendable_param();

	// 変数フレームを作成して、引数を登録する
	var_begin(VarFrame::SCOPE);
	Xfor2(k, v, e->fun_params()){
		var_define(k->to_s()->intern());
	}

	// 関数コアを作成
	FunCore core;
	core.pc = code_size() + InstMakeFun::ISIZE;
	core.lineno = linenos_.top();
	core.kind = e->fun_kind();
	core.min_param_count = minv;
	core.max_param_count = maxv;
	core.flags = e->fun_extendable_param() ? FunCore::FLAG_EXTENDABLE_PARAM : 0;

	// 引数の名前を識別子テーブルに順番に乗せる
	core.variable_size = vf().entries.size();
	core.variable_identifier_offset = result_->identifier_table_->size();
	for(int_t i=0; i<vf().entries.size(); ++i){
		result_->identifier_table_->push_back(vf().entries[i].name);
	}

	int_t fun_core_table_number = result_->xfun_core_table_.size();
	result_->xfun_core_table_.push_back(core);

	int_t fun_end_label = reserve_label();
	set_jump(offsetof(InstMakeFun, address), fun_end_label);
	put_inst(InstMakeFun(fun_core_table_number, 0));
	
	if(debug::is_enabled()){
		put_inst(InstBreakPoint(BREAKPOINT_CALL));
	}

	// デフォルト値を持つ引数を処理する
	{
		int_t i = 0;
		Xfor2(k, v, e->fun_params()){
			if(v){
				int_t label = reserve_label();
				
				set_jump(offsetof(InstIfArgIsNop, address), label);
				var_set_direct(vf());
				put_inst(InstIfArgIsNop(e->fun_params()->size()-1-i, 0));

				compile_expr(v);
				
				var_find(k->to_s()->intern(), true);
				put_set_local_code(k->to_s()->intern());
				
				set_label(label);
			}else{
				var_find(k->to_s()->intern(), true);
			}

			++i;
		}
	}

	// 関数本体を処理する
	compile_stmt(e->fun_body());
	
	break_off(ff().var_frame_count+1);
	if(debug::is_enabled()){
		put_inst(InstBreakPoint(BREAKPOINT_RETURN));
	}
	put_inst(InstReturn0());
	set_label(fun_end_label);

	process_labels();
	
	if(vf().kind!=VarFrame::SCOPE){
		result_->xfun_core_table_[fun_core_table_number].flags |= FunCore::FLAG_ON_HEAP;
	}

	if(vf().scope_chain){
		result_->xfun_core_table_[fun_core_table_number].flags |= FunCore::FLAG_SCOPE_CHAIN;
		var_set_on_heap(1);
	}

	// 関数フレームをポップする
	fun_frames_.downsize(1);

	// 変数フレームをポップする
	var_end();
}

void CodeBuilder::compile_for(const ExprPtr& e){
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

	FunFrame::Loop loop;
	loop.control_statement_label[0] = label_end;
	loop.control_statement_label[1] = label_cond;
	loop.label = e->for_label();
	loop.frame_count = var_frames_.size();
	loop.have_label = false;
	ff().loops.push(loop);

	compile_stmt(e->for_body());
	
	ff().loops.pop();
	
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

void CodeBuilder::compile_expr(const AnyPtr& p, const CompileInfo& info){

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
	
	
	if(e->lineno()!=0){
		linenos_.push(e->lineno());
		result_->set_lineno_info(e->lineno());
	}

	switch(e->type()){

		XTAL_NODEFAULT;

		XTAL_CASE(EXPR_NULL){ put_inst(InstPushNull()); }
		XTAL_CASE(EXPR_NOP){ put_inst(InstPushNop()); }
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
				compile_expr(v);
				put_inst(InstArrayAppend());				
			}
		}

		XTAL_CASE(EXPR_MAP){
			put_inst(InstMakeMap());
			Xfor2(k, v, e->map_values()){
				compile_expr(k);
				compile_expr(v);
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

		XTAL_CASE(EXPR_Q){
			int_t label_if = reserve_label();
			int_t label_end = reserve_label();

			compile_expr(e->q_cond());

			set_jump(offsetof(InstIf, address), label_if);
			put_inst(InstIf());

			compile_expr(e->q_true());

			set_jump(offsetof(InstGoto, address), label_end);
			put_inst(InstGoto());

			set_label(label_if);
			
			compile_expr(e->q_false());
			
			set_label(label_end);
		}
		
		XTAL_CASE(EXPR_AT){
			compile_expr(e->bin_lhs());
			compile_expr(e->bin_rhs());
			put_inst(InstAt());
		}

		XTAL_CASE(EXPR_ANDAND){
			int_t label_if = reserve_label();

			compile_expr(e->bin_lhs());

			put_inst(InstDup());
			
			set_jump(offsetof(InstIf, address), label_if);
			put_inst(InstIf());

			put_inst(InstPop());
			
			compile_expr(e->bin_rhs());
			
			set_label(label_if);
		}

		XTAL_CASE(EXPR_OROR){
			int_t label_if = reserve_label();
			compile_expr(e->bin_lhs());

			put_inst(InstDup());
			
			set_jump(offsetof(InstUnless, address), label_if);
			put_inst(InstUnless());
			
			put_inst(InstPop());
			
			compile_expr(e->bin_rhs());
			
			set_label(label_if);
		}

		XTAL_CASE(EXPR_POS){ compile_expr(e->una_term()); put_inst(InstPos()); }
		XTAL_CASE(EXPR_NEG){ compile_expr(e->una_term()); put_inst(InstNeg()); }
		XTAL_CASE(EXPR_COM){ compile_expr(e->una_term()); put_inst(InstCom()); }
		XTAL_CASE(EXPR_NOT){ compile_expr(e->una_term()); put_inst(InstNot()); }

		XTAL_CASE(EXPR_ONCE){
			int_t label_end = reserve_label();
			
			set_jump(offsetof(InstOnce, address), label_end);
			int_t num = result_->once_table_->size();
			result_->once_table_->push_back(nop);
			put_inst(InstOnce(0, num));
						
			compile_expr(e->una_term());
			put_inst(InstDup());		
			put_inst(InstSetOnce(num));
			
			set_label(label_end);	
		}

		XTAL_CASE(EXPR_SEND){
			compile_expr(e->send_term());

			/*
			int_t block_first = com_->register_identifier(InternedStringPtr("block_first")); 
			int_t block_next = com_->register_identifier(InternedStringPtr("block_next")); 
			int_t block_break = com_->register_identifier(InternedStringPtr("block_break")); 

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
			
			Xfor(v, e->call_ordered()){
				compile_expr(ptr_cast<Expr>(v));
			}

			Xfor2(k, v, e->call_named()){
				put_inst(InstValue(register_value(k->to_s()->intern())));
				compile_expr(ptr_cast<Expr>(v));
			}

			bool have_args = e->call_have_args();
			int_t ordered = e->call_ordered() ? e->call_ordered()->size() : 0;
			int_t named = e->call_named() ? e->call_named()->size() : 0;

			if(e->call_term()->type()==EXPR_SEND){ // a.b(); メッセージ送信式

				ExprPtr e2 = e->call_term();
				compile_expr(e2->send_term());

				if(e2->send_pname()){
					compile_expr(e2->send_pname());
				}

				int_t send_name = 0;
				if(!e2->send_pname()){
					send_name = register_identifier(e2->send_name());
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
				compile_expr(e->call_term());
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
			compile_fun(e);
		}

		XTAL_CASE(EXPR_LVAR){
			put_local_code(e->lvar_name());
		}

		XTAL_CASE(EXPR_IVAR){
			put_instance_variable_code(e->ivar_name());
		}

		XTAL_CASE(EXPR_MEMBER){
			compile_expr(e->member_term());
			put_member_code(e->member_name(), e->member_pname(), e->member_q());
		}

		XTAL_CASE(EXPR_CLASS){
			compile_class(e);
		}
	}
	
	if(e->lineno()!=0){
		result_->set_lineno_info(e->lineno());
		linenos_.pop();
	}

	if(info.need_result_count!=result_count){
		put_inst(InstAdjustResult(result_count, info.need_result_count));
	}
}

void CodeBuilder::compile_stmt(const AnyPtr& p){

	if(!p)
		return;

	ExprPtr e = ep(p);

	if(debug::is_enabled() && linenos_.top()!=e->lineno()){
		put_inst(InstBreakPoint(BREAKPOINT_LINE));
	}

	if(e->lineno()!=0){
		linenos_.push(e->lineno());
		result_->set_lineno_info(e->lineno());
	}

	switch(e->type()){

		XTAL_DEFAULT{
			compile_expr(e, 0);
		}
		
		XTAL_CASE(EXPR_DEFINE){
			if(e->bin_lhs()->type()==EXPR_LVAR){
				compile_expr(e->bin_rhs());
				
				if(e->bin_rhs()->type()==EXPR_FUN || e->bin_rhs()->type()==EXPR_CLASS){
					put_inst(InstSetName(register_identifier(e->bin_lhs()->lvar_name())));
				}

				put_define_local_code(e->bin_lhs()->lvar_name());
			}else if(e->bin_lhs()->type()==EXPR_MEMBER){
				compile_expr(e->bin_lhs()->member_term());
				compile_expr(e->bin_rhs());

				if(e->bin_lhs()->member_name() && (e->bin_rhs()->type()==EXPR_FUN || e->bin_rhs()->type()==EXPR_CLASS)){
					put_inst(InstSetName(register_identifier(e->bin_lhs()->member_name())));
				}

				put_define_member_code(e->bin_lhs()->member_name(), e->bin_lhs()->member_pname());
			}else{
				error_->error(lineno(), Xt("Xtal Compile Error 1012"));
			}
		}
		
		XTAL_CASE(EXPR_ASSIGN){
			if(e->bin_lhs()->type()==EXPR_LVAR){
				compile_expr(e->bin_rhs());
				put_set_local_code(e->bin_lhs()->lvar_name());
			}else if(e->bin_lhs()->type()==EXPR_IVAR){
				compile_expr(e->bin_rhs());
				put_set_instance_variable_code(e->bin_lhs()->ivar_name());
			}else if(e->bin_lhs()->type()==EXPR_SEND){
				compile_expr(e->bin_rhs());
				compile_expr(e->bin_lhs()->send_term());
				put_set_send_code(e->bin_lhs()->send_name(), e->bin_lhs()->send_pname(), e->bin_lhs()->send_q());
			}else if(e->bin_lhs()->type()==EXPR_AT){
				compile_expr(e->bin_rhs());
				compile_expr(e->bin_lhs()->bin_lhs());
				compile_expr(e->bin_lhs()->bin_rhs());
				put_inst(InstSetAt());
			}else{
				error_->error(lineno(), Xt("Xtal Compile Error 1012"));
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

		XTAL_CASE(EXPR_YIELD){
			Xfor(v, e->yield_exprs()){
				compile_expr(v);
			}

			int_t exprs_size = e->yield_exprs()->size();
			put_inst(InstYield(exprs_size));
			if(exprs_size>=256){
				error_->error(lineno(), Xt("Xtal Compile Error 1022"));
			}
		}

		XTAL_CASE(EXPR_RETURN){	
			bool have_finally = false;
			for(uint_t scope_count = var_frames_.size(); scope_count!=(uint_t)ff().var_frame_count+1; scope_count--){
				for(uint_t k = 0; k<(uint_t)ff().finallies.size(); ++k){
					if((uint_t)ff().finallies[k].frame_count==scope_count){
						have_finally = true;
					}
				}
			}

			int_t exprs_size = e->return_exprs() ? e->return_exprs()->size() : 0;
			if(!have_finally && exprs_size==1){
				ExprPtr front = ep(e->return_exprs()->front());
				if(front->type()==EXPR_CALL){
					compile_expr(front, CompileInfo(1, true));
					break;
				}else if(front->type()==EXPR_SEND){
					compile_expr(front, CompileInfo(1, true));
					break;
				}
			}

			Xfor(v, e->return_exprs()){
				compile_expr(v);
			}
			
			{
				
				break_off(ff().var_frame_count+1);

				if(debug::is_enabled()){
					put_inst(InstBreakPoint(BREAKPOINT_RETURN));
				}

				if(exprs_size==0){ put_inst(InstReturn0()); }
				else if(exprs_size==1){ put_inst(InstReturn1()); }
				else if(exprs_size==2){ put_inst(InstReturn2()); }
				else{
					put_inst(InstReturn(exprs_size));
					if(exprs_size>=256){
						error_->error(lineno(), Xt("Xtal Compile Error 1022"));
					}
				}	
			}
		}

		XTAL_CASE(EXPR_ASSERT){		
			int_t exprs_size = e->assert_exprs()->size();
			if(exprs_size==1){
				compile_expr(e->assert_exprs()->at(0));
				put_inst(InstValue(0));
				put_inst(InstValue(0));	
			}else if(exprs_size==2){
				compile_expr(e->assert_exprs()->at(0));
				compile_expr(e->assert_exprs()->at(1));
				put_inst(InstValue(0));
			}else if(exprs_size==3){
				compile_expr(e->assert_exprs()->at(0));
				compile_expr(e->assert_exprs()->at(1));
				compile_expr(e->assert_exprs()->at(2));
			}else{
				error_->error(lineno(), Xt("Xtal Compile Error 1016"));
			}
			
			put_inst(InstAssert());
		}

		XTAL_CASE(EXPR_TRY){
			int_t finally_label = reserve_label();
			int_t end_label = reserve_label();

			int_t core = result_->except_core_table_.size();
			result_->except_core_table_.push_back(ExceptCore());
			put_inst(InstTryBegin(core));

			CodeBuilder::FunFrame::Finally exc;
			exc.frame_count = var_frames_.size();
			exc.finally_label = finally_label;
			ff().finallies.push(exc);

			compile_stmt(e->try_body());
			
			set_jump(offsetof(InstPushGoto, address), end_label);
			put_inst(InstPushGoto());
			put_inst(InstTryEnd());

			// catch節のコードを埋め込む
			if(e->try_catch()){

				result_->except_core_table_[core].catch_pc = code_size();
				
				// catch節の中での例外に備え、例外フレームを構築。

				int_t core2 = result_->except_core_table_.size();
				result_->except_core_table_.push_back(ExceptCore());
				put_inst(InstTryBegin(core2));

				CodeBuilder::FunFrame::Finally exc;
				exc.frame_count = var_frames_.size();
				exc.finally_label = finally_label;
				ff().finallies.push(exc);

				var_begin(VarFrame::SCOPE);
				var_define(e->try_catch_var(), 0, true);
				block_begin();

				put_set_local_code(e->try_catch_var());
				compile_stmt(e->try_catch());
				
				block_end();
				var_end();

				put_inst(InstTryEnd());
				ff().finallies.pop();

				result_->except_core_table_[core2].finally_pc = code_size();
				result_->except_core_table_[core2].end_pc = code_size();
			}
			
			set_label(finally_label);

			result_->except_core_table_[core].finally_pc = code_size();

			// finally節のコードを埋め込む
			compile_stmt(e->try_finally());
			
			ff().finallies.pop();

			put_inst(InstPopGoto());

			set_label(end_label);
			result_->except_core_table_[core].end_pc = code_size();
		}

		XTAL_CASE(EXPR_THROW){
			compile_expr(e->una_term());
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
			compile_for(e);
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

					compile_expr(rhs->at(r), rrc);
					pushed_count += rrc;
					break;
				}else{
					compile_expr(rhs->at(r));
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
						compile_expr(v->member_term());
						put_define_member_code(v->member_name(), v->member_pname());
					}else{
						error_->error(lineno(), Xt("Xtal Compile Error 1008"));
					}
				}
			}else{
				Xfor(v1, lhs->r_each()){
					ExprPtr v = ep(v1);
					if(v->type()==EXPR_LVAR){
						put_set_local_code(v->lvar_name());
					}else if(v->type()==EXPR_SEND){
						compile_expr(v->send_term());
						put_set_send_code(v->send_name(), v->send_pname(), v->send_q());
					}else if(v->type()==EXPR_IVAR){
						put_set_instance_variable_code(v->ivar_name());					
					}else if(v->type()==EXPR_AT){
						compile_expr(v->bin_lhs());
						compile_expr(v->bin_rhs());
						put_inst(InstSetAt());
					}else{
						error_->error(lineno(), Xt("Xtal Compile Error 1008"));
					}
				}
			}
		}

		XTAL_CASE(EXPR_BREAK){
			compile_loop_control_statement(e);
		}	

		XTAL_CASE(EXPR_CONTINUE){
			compile_loop_control_statement(e);
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
			Xfor(v, e->toplevel_stmts()){
				compile_stmt(v);
			}
		}
	}

	if(e->lineno()!=0){
		result_->set_lineno_info(e->lineno());
		linenos_.pop();
	}
}

}

#endif

