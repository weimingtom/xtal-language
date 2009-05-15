#include "xtal.h"
#include "xtal_macro.h"

#include "xtal_codebuilder.h"

#ifndef XTAL_NO_PARSER

namespace xtal{
	
CodeBuilder::CodeBuilder(){
	implicit_ref_map_ = xnew<Map>();
}

CodeBuilder::~CodeBuilder(){}

CodePtr CodeBuilder::compile(const StreamPtr& stream, const StringPtr& source_file_name){
	error_= &errorimpl_;
	error_->init(source_file_name);
	ExprPtr e = parser_.parse(stream, error_);
	if(!e){
		return null;
	}
	prev_inst_op_ = -1;
	return compile_toplevel(e, source_file_name);
}

void CodeBuilder::interactive_compile(const StreamPtr& stream){
	error_= &errorimpl_;
	error_->init("<ix>");

	{
		StreamPtr ss = xnew<StringStream>(";");
		ExprPtr e = parser_.parse_stmt(ss, error_);
		compile_toplevel(e, "<ix>");
		result_->code_.clear();
	}

	result_->xfun_info_table_.reserve(10000);
	result_->class_info_table_.reserve(10000);
	result_->scope_info_table_.reserve(10000);
	result_->except_info_table_.reserve(10000);

	result_->first_fun()->set_info(&result_->xfun_info_table_[0]);

	int_t pc_pos = 0;
	while(!stream->eos()){

		ExprPtr e = parser_.parse_stmt(stream, error_);
		stream->flush();

		if(!e){
			stderr_stream()->put_s(error_->errors->join("\n"));
			stderr_stream()->put_s("\n");
			error_->errors->clear();
		}
		else{

			if(e->itag()==EXPR_RETURN){
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
			}
			else{

				process_labels();

				fun_frames_.downsize(1);
				
				if(code_size()==0)
					continue;

				result_->inspect_range(last_code_size, code_size())->p();

				put_inst(InstReturn(0));
				put_inst(InstThrow());

				vmachine()->execute(result_->first_fun().get(), &result_->code_[pc_pos]);
				
				XTAL_CATCH_EXCEPT(e){
					stderr_stream()->put_s(e->to_s());
					stderr_stream()->put_s("\n");
				}

				for(uint_t i=0; i<(sizeof(InstThrow)+sizeof(InstReturn))/sizeof(inst_t); ++i){
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
	result_->except_info_table_.push_back(ExceptInfo());

	result_->identifier_table_ = xnew<Array>();
	identifier_map_ = xnew<Map>();
	register_identifier(empty_id);

	result_->value_table_ = xnew<Array>();
	value_map_ = xnew<Map>();
	register_value(null);
	
	result_->once_table_ = xnew<Array>();
	result_->once_table_->push_back(undefined);

	linenos_.push(1);

	// 関数フレームを作成する
	fun_frames_.push();	
	ff().labels.clear();
	ff().loops.clear();
	ff().finallies.clear();
	ff().var_frame_count = var_frames_.size();
	ff().stack_count = 0;
	ff().max_stack_count = 0;
	ff().variable_count = 1;
	ff().max_variable_count = 1;
	ff().extendable_param = true;

	// 変数フレームを作成して、コマンドライン引数を登録する
	var_begin(VarFrame::FRAME);

	// コマンドライン引数
	var_define(Xid(arg));

	// 関数コアを作成
	FunInfo info;
	info.pc = 0;
	info.kind = KIND_FUN;
	info.min_param_count = 0;
	info.max_param_count = 0;
	info.name_number = register_identifier(Xid(toplevel));
	info.flags = FunInfo::FLAG_EXTENDABLE_PARAM;

	// 引数の名前を識別子テーブルに順番に乗せる
	info.variable_size = vf().entries.size();
	info.variable_identifier_offset = result_->identifier_table_->size();
	for(uint_t i=0; i<vf().entries.size(); ++i){
		vf().entries[i].initialized = true;
		result_->identifier_table_->push_back(vf().entries[i].name);
	}

	int_t fun_info_table_number = 0;
	result_->xfun_info_table_.push_back(info);

	var_begin(VarFrame::DEFAULT);
	scope_begin();	

	// 関数本体を処理する
	compile_stmt(e);

	scope_end();
	var_end();
	
	break_off(ff().var_frame_count+1);

	put_inst(InstReturn(0));
	put_inst(InstThrow());

	process_labels();
	
	if(vf().scope_chain){
		result_->xfun_info_table_[fun_info_table_number].flags |= FunInfo::FLAG_SCOPE_CHAIN;
	}

	// 関数フレームをポップする
	fun_frames_.downsize(1);

	// 変数フレームをポップする
	var_end();

	Xfor2(k, v, implicit_ref_map_){
		if(v){
			Code::ImplcitInfo info = {k->to_i(), v->to_i()};
			result_->implicit_table_.push_back(info);
		}
	}

	if(error_->errors->size()==0){
		result_->first_fun()->set_info(&result_->xfun_info_table_[0]);
		return result_;
	}
	else{
		result_ = null;
		return null;
	}
}

AnyPtr CodeBuilder::errors(){
	return error_->errors->each();
}

void CodeBuilder::put_inst2(const Inst& t, uint_t sz){
	if(t.op==255){
		error_->error(lineno(), Xt("Xtal Compile Error 1027"));
	}

///*
	if(t.op==InstLocalVariable1Byte::NUMBER){
		if(prev_inst_op_==InstLocalVariable1Byte::NUMBER){
			InstLocalVariable1Byte prev_inst;
			uint_t prev_op_size = sizeof(InstLocalVariable1Byte);
			uint_t prev_op_isize = sizeof(InstLocalVariable1Byte)/sizeof(inst_t);
			std::memcpy(&prev_inst, &result_->code_[result_->code_.size()-prev_op_isize], prev_op_size);
			result_->code_.downsize(prev_op_isize);

			prev_inst_op_ = -1;
			put_inst(InstLocalVariable1ByteX2(prev_inst.number, ((InstLocalVariable1Byte&)t).number));
			return;
		}
	}

	if(t.op==InstSetLocalVariable1Byte::NUMBER){
		if(prev_inst_op_==InstSetLocalVariable1Byte::NUMBER){
			InstLocalVariable1Byte prev_inst;
			uint_t prev_op_size = sizeof(InstSetLocalVariable1Byte);
			uint_t prev_op_isize = sizeof(InstSetLocalVariable1Byte)/sizeof(inst_t);
			std::memcpy(&prev_inst, &result_->code_[result_->code_.size()-prev_op_isize], prev_op_size);
			result_->code_.downsize(prev_op_isize);

			prev_inst_op_ = -1;
			put_inst(InstSetLocalVariable1ByteX2(prev_inst.number, ((InstLocalVariable1Byte&)t).number));
			return;
		}
	}
//*/

	prev_inst_op_ = t.op;

	size_t cur = result_->code_.size();
	result_->code_.resize(cur+sz/sizeof(inst_t));
	std::memcpy(&result_->code_[cur], &t, sz);
}

bool CodeBuilder::put_set_local_code(const IDPtr& var){
	LVarInfo info = var_find(var);
	if(info.pos>=0){
		if(entry(info).constant){
			error_->error(lineno(), Xt("Xtal Compile Error 1019")->call(Named(Xid(name), var)));
		}

		if(info.pos<=0xff){
			put_inst(InstSetLocalVariable1Byte(info.pos));
		}
		else{
			put_inst(InstSetLocalVariable2Byte(info.pos));
		}

		entry(info).value = undefined;

		return true;
	}
	else{
		error_->error(lineno(), Xt("Xtal Compile Error 1009")->call(Named(Xid(name), var)));
		return false;
	}
}

void CodeBuilder::put_define_local_code(const IDPtr& var, const ExprPtr& rhs){
	LVarInfo info = var_find(var, true);

	if(info.pos>=0){
		if(rhs){
			AnyPtr val = entry(info).initialized ? entry(info).value : do_expr(rhs);

			if(raweq(val, undefined) || entry(info).assigned){
				if(raweq(val, undefined)){

					if(rhs->itag()==EXPR_CLASS){
						compile_class(rhs, var);
					}
					else if(rhs->itag()==EXPR_FUN){
						compile_fun(rhs, var);
					}
					else{
						compile_expr(rhs);
					}
				}
				else{
					put_val_code(val);
				}

				if(info.pos<=0xff){
					put_inst(InstSetLocalVariable1Byte(info.pos));
				}
				else{
					put_inst(InstSetLocalVariable2Byte(info.pos));
				}
			}

			entry(info).value = val;

		}
		else{
			if(info.pos<=0xff){
				put_inst(InstSetLocalVariable1Byte(info.pos));
			}
			else{
				put_inst(InstSetLocalVariable2Byte(info.pos));
			}
		}
	}
	else{
		XTAL_ASSERT(false);
	}
}

bool CodeBuilder::put_local_code(const IDPtr& var){
	LVarInfo info = var_find(var);
	if(info.pos>=0){
		if(info.pos<=0xff){
			put_inst(InstLocalVariable1Byte(info.pos));
		}
		else{
			put_inst(InstLocalVariable2Byte(info.pos));
		}		

		entry(info).referenced = true;

		return true;
	}
	else{
		int_t id = register_identifier(var);
		put_inst(InstFilelocalVariable(id));
		implicit_ref_map_->set_at(id, lineno());
		return false;
	}
}

int_t CodeBuilder::register_identifier_or_compile_expr(const AnyPtr& var){
	if(const IDPtr& id = ptr_cast<ID>(var)){ 
		return register_identifier(id);
	}
	compile_expr(ep(var)); 
	return 0;
}

int_t CodeBuilder::register_identifier(const IDPtr& v){
	if(const AnyPtr& pos = identifier_map_->at(v)){ return pos->to_i(); }
	result_->identifier_table_->push_back(v);
	identifier_map_->set_at(v, result_->identifier_table_->size()-1);
	return result_->identifier_table_->size()-1;
}

int_t CodeBuilder::register_value(const AnyPtr& v){
	if(const AnyPtr& pos = value_map_->at(v)){ return pos->to_i(); }
	result_->value_table_->push_back(v);
	value_map_->set_at(v, result_->value_table_->size()-1);
	return result_->value_table_->size()-1;
}

int_t CodeBuilder::append_identifier(const IDPtr& identifier){
	result_->identifier_table_->push_back(identifier);
	return result_->identifier_table_->size()-1;
}

int_t CodeBuilder::append_value(const AnyPtr& v){
	result_->value_table_->push_back(v);
	return result_->value_table_->size()-1;
}

void CodeBuilder::put_send_code(const AnyPtr& var, const ExprPtr& secondary_key, 
	int_t need_result_count, int_t ordered, int_t named, int_t flags){

	int_t key = register_identifier_or_compile_expr(var);

	if(secondary_key){
		compile_expr(secondary_key);
		flags |= CALL_FLAG_NS;
		put_inst(InstSend(ordered, named, need_result_count, flags, key));
	}
	else{
		if(flags==0 && key!=0 && need_result_count==1 && ordered==0 && named==0){
			put_inst(InstProperty(key));
		}
		else{
			put_inst(InstSend(ordered, named, need_result_count, flags, key));
		}
	}
}

void CodeBuilder::put_set_send_code(const AnyPtr& var, const ExprPtr& secondary_key, int_t flags){
	int_t key = 0;
	if(ptr_cast<Expr>(var)){ 
		eb_.push(KIND_STRING);
		eb_.push(Xid(set_));
		eb_.splice(EXPR_STRING, 2);
		eb_.push(ep(var));
		eb_.splice(EXPR_CAT, 2);
		compile_expr(eb_.pop()); 
	}
	else{
		key = register_identifier(Xid(set_)->cat(ptr_cast<ID>(var)));
	}
	
	if(secondary_key){
		compile_expr(secondary_key);
		flags |= CALL_FLAG_NS;
		put_inst(InstSend(1, 0, 0, flags, key));
	}
	else{
		if(flags==0 && key!=0){
			put_inst(InstSetProperty(key));
		}
		else{
			put_inst(InstSend(1, 0, 0, flags, key));
		}
	}
}

void CodeBuilder::put_define_member_code(const AnyPtr& var, const ExprPtr& secondary_key){
	int_t key = register_identifier_or_compile_expr(var);

	if(secondary_key){
		compile_expr(secondary_key);
		put_inst(InstDefineMember(key, CALL_FLAG_NS));
	}
	else{
		put_inst(InstDefineMember(key, CALL_FLAG_NONE));
	}
}

int_t CodeBuilder::lookup_instance_variable(const IDPtr& key){
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

	error_->error(lineno(), Xt("Xtal Compile Error 1023")->call(Named(Xid(name), Xid(_)->cat(key))));
	return 0;
}

void CodeBuilder::put_set_instance_variable_code(const IDPtr& var){
	put_inst(InstSetInstanceVariable(lookup_instance_variable(var), class_info_num()));
}

void CodeBuilder::put_instance_variable_code(const IDPtr& var){
	put_inst(InstInstanceVariable(lookup_instance_variable(var), class_info_num()));
}

int_t CodeBuilder::reserve_label(){
	ff().labels.resize(fun_frames_.top().labels.size()+1);
	ff().labels[ff().labels.size()-1].pos = -1;
	return ff().labels.size()-1;
}

void CodeBuilder::set_label(int_t labelno){
	prev_inst_op_ = -1;
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
		XTAL_ASSERT(l.pos!=-1);
		for(size_t j = 0; j<l.froms.size(); ++j){
			FunFrame::Label::From &f = l.froms[j];
			inst_address_t& buf = *(inst_address_t*)&result_->code_[f.set_pos];
			buf = l.pos - f.set_pos; //l.pos - f.pos;

			Code::AddressJump address_jump = {f.set_pos};
			result_->address_jump_table_.push_back(address_jump);
		}
	}
	ff().labels.clear();
}

void CodeBuilder::break_off(int_t n){
	for(uint_t scope_count = var_frames_.size(); scope_count!=(uint_t)n; scope_count--){
		for(uint_t k = 0; k<ff().finallies.size(); ++k){
			if((uint_t)ff().finallies[k].frame_count==scope_count){
				int_t label = reserve_label();
				set_jump(InstPushGoto::OFFSET_address, label);
				put_inst(InstPushGoto());
				put_inst(InstTryEnd());
				set_label(label);
			}
		}

		VarFrame& vf = var_frames_[var_frames_.size()-scope_count];
		if(vf.real_entry_num!=0 && vf.kind==VarFrame::FRAME){
			put_inst(InstScopeEnd());
		}
	}
}

static bool is_comp_bin(const ExprPtr& e){
	if(e && EXPR_EQ<=e->itag() && e->itag()<=EXPR_NIS){
		return true;
	}	
	return false;
}

void CodeBuilder::put_val_code(const AnyPtr& val){
	switch(type(val)){
		XTAL_DEFAULT{
			put_inst(InstValue(register_value(val)));
		}

		XTAL_CASE(TYPE_NULL){ put_inst(InstPushNull()); }
		XTAL_CASE(TYPE_TRUE){ put_inst(InstPushTrue()); }
		XTAL_CASE(TYPE_FALSE){ put_inst(InstPushFalse()); }

		XTAL_CASE(TYPE_INT){
			int_t value = ivalue(val);
			if(value==(i8)value){ 
				put_inst(InstPushInt1Byte(value));
			}
			else{ 
				put_inst(InstValue(register_value(value)));
			}
		}

		XTAL_CASE(TYPE_FLOAT){
			float_t value = fvalue(val);
			if(value==(i8)value){ 
				put_inst(InstPushFloat1Byte((i8)value));
			}
			else{ 
				put_inst(InstValue(register_value(value)));
			}
		}
	}
}

void CodeBuilder::put_if_code(const ExprPtr& e, int_t label_false){
	AnyPtr value = do_expr(e);
	if(value){
		return;
	}

	int_t label_true = reserve_label();

	if(is_comp_bin(e)){
		if(EXPR_EQ==e->itag()){
			compile_expr(e->bin_lhs());
			compile_expr(e->bin_rhs());

			put_inst(InstIfEq());

			set_jump(InstIf::OFFSET_address_true, label_true);
			set_jump(InstIf::OFFSET_address_false, label_false);
			put_inst(InstIf());
		}
		else if(EXPR_NE==e->itag()){
			compile_expr(e->bin_lhs());
			compile_expr(e->bin_rhs());

			put_inst(InstIfEq());

			set_jump(InstIf::OFFSET_address_false, label_true);
			set_jump(InstIf::OFFSET_address_true, label_false);
			put_inst(InstIf());
		}
		else if(EXPR_LT==e->itag()){
			compile_expr(e->bin_lhs());
			compile_expr(e->bin_rhs());

			put_inst(InstIfLt());

			set_jump(InstIf::OFFSET_address_true, label_true);
			set_jump(InstIf::OFFSET_address_false, label_false);
			put_inst(InstIf());
		}
		else if(EXPR_GT==e->itag()){
			compile_expr(e->bin_rhs());
			compile_expr(e->bin_lhs());

			put_inst(InstIfLt());

			set_jump(InstIf::OFFSET_address_true, label_true);
			set_jump(InstIf::OFFSET_address_false, label_false);
			put_inst(InstIf());
		}
		else if(EXPR_GE==e->itag()){
			compile_expr(e->bin_lhs());
			compile_expr(e->bin_rhs());

			put_inst(InstIfLt());

			set_jump(InstIf::OFFSET_address_false, label_true);
			set_jump(InstIf::OFFSET_address_true, label_false);
			put_inst(InstIf());
		}
		else if(EXPR_LE==e->itag()){
			compile_expr(e->bin_rhs());
			compile_expr(e->bin_lhs());

			put_inst(InstIfLt());

			set_jump(InstIf::OFFSET_address_false, label_true);
			set_jump(InstIf::OFFSET_address_true, label_false);
			put_inst(InstIf());
		}
		else if(EXPR_RAWEQ==e->itag()){
			compile_expr(e->bin_lhs());
			compile_expr(e->bin_rhs());

			put_inst(InstIfRawEq());

			set_jump(InstIf::OFFSET_address_true, label_true);
			set_jump(InstIf::OFFSET_address_false, label_false);
			put_inst(InstIf());
		}
		else if(EXPR_RAWNE==e->itag()){
			compile_expr(e->bin_lhs());
			compile_expr(e->bin_rhs());

			put_inst(InstIfRawEq());

			set_jump(InstIf::OFFSET_address_false, label_true);
			set_jump(InstIf::OFFSET_address_true, label_false);
			put_inst(InstIf());
		}
		else if(EXPR_IN==e->itag()){
			compile_expr(e->bin_lhs());
			compile_expr(e->bin_rhs());

			put_inst(InstIfIn());

			set_jump(InstIf::OFFSET_address_true, label_true);
			set_jump(InstIf::OFFSET_address_false, label_false);
			put_inst(InstIf());
		}
		else if(EXPR_NIN==e->itag()){
			compile_expr(e->bin_lhs());
			compile_expr(e->bin_rhs());

			put_inst(InstIfIn());

			set_jump(InstIf::OFFSET_address_false, label_true);
			set_jump(InstIf::OFFSET_address_true, label_false);
			put_inst(InstIf());
		}
		else if(EXPR_IS==e->itag()){
			compile_expr(e->bin_lhs());
			compile_expr(e->bin_rhs());

			put_inst(InstIfIs());

			set_jump(InstIf::OFFSET_address_true, label_true);
			set_jump(InstIf::OFFSET_address_false, label_false);
			put_inst(InstIf());
		}
		else if(EXPR_NIS==e->itag()){
			compile_expr(e->bin_lhs());
			compile_expr(e->bin_rhs());

			put_inst(InstIfIs());

			set_jump(InstIf::OFFSET_address_false, label_true);
			set_jump(InstIf::OFFSET_address_true, label_false);
			put_inst(InstIf());
		}
		else{
			XTAL_ASSERT(false);
		}
	}
	else{
		if(e->itag()==EXPR_NOT){
			compile_expr(e->una_term());
			set_jump(InstIf::OFFSET_address_false, label_true);
			set_jump(InstIf::OFFSET_address_true, label_false);
			put_inst(InstIf());
		}
		else{
			compile_expr(e);
			set_jump(InstIf::OFFSET_address_true, label_true);
			set_jump(InstIf::OFFSET_address_false, label_false);
			put_inst(InstIf());
		}
	}

	set_label(label_true);
}

void CodeBuilder::scope_chain(int_t var_frame_size){
	for(int_t i=0; i<var_frame_size; ++i){
		var_frames_[i].scope_chain = true;
	}
}

CodeBuilder::LVarInfo CodeBuilder::var_find(const IDPtr& key, bool define, bool traceless, int_t number){
	LVarInfo ret = {0, 0, 0};
	for(size_t i = 0, last = var_frames_.size(); i<last; ++i){
		VarFrame& vf = var_frames_[i];
		for(size_t j = 0, jlast = vf.entries.size(); j<jlast; ++j){
			VarFrame::Entry& entry = vf.entries[vf.entries.size()-1-j];
			if(raweq(entry.name, key) && (number<0 || entry.number<0 || number==entry.number)){
				if((uint_t)vf.fun_frames_size!=fun_frames_.size() || entry.initialized || define){
					ret.var_frame = var_frames_.size()-1-i;
					ret.entry = vf.entries.size()-1-j;
					if(!traceless){
						if(define){ entry.initialized = true; }
						scope_chain(i);
					}
					return ret;
				}
			}

			if(!entry.removed){
				ret.pos++;
			}
		}
	}
	ret.pos = -1;
	return ret;
}

void CodeBuilder::var_begin(int_t kind){
	var_frames_.push();
	vf().entries.clear();
	vf().scope_info_num = 0;
	vf().kind = kind;
	vf().fun_frames_size = fun_frames_.size();
	vf().scope_chain = false;
}

void CodeBuilder::var_define_stmts(const ExprPtr& stmts){
	Xfor(v, stmts){
		var_define_stmt(v);
	}
}

void CodeBuilder::var_define_stmt(const AnyPtr& stmt){
	if(ExprPtr v = ptr_cast<Expr>(stmt)){
		if(v->itag()==EXPR_DEFINE){
			if(v->bin_lhs()->itag()==EXPR_LVAR){
				var_define(v->bin_lhs()->lvar_name(), v->bin_rhs());
			}
		}
		else if(v->itag()==EXPR_MDEFINE){
			Xfor(v1, v->mdefine_lhs_exprs()){
				ExprPtr vv = ep(v1);
				if(vv->itag()==EXPR_LVAR){
					var_define(vv->lvar_name());
				}			
			}
		}
	}
}

void CodeBuilder::var_define(const IDPtr& name, const ExprPtr& expr, int_t accessibility, bool define, bool constant, bool assign, int_t number){
	if(number<0){
		for(size_t j = 0, jlast = vf().entries.size(); j<jlast; ++j){
			if(raweq(vf().entries[vf().entries.size()-1-j].name, name)){
				error_->error(lineno(), Xt("Xtal Compile Error 1026")->call(Named(Xid(name), name)));
				return;
			}
		}
	}

	VarFrame::Entry entry;
	entry.name = name;
	entry.expr = expr;
	entry.value = undefined;
	entry.constant = constant;
	entry.initialized = define;
	entry.accessibility = accessibility;
	entry.assigned = assign;
	entry.referenced = false;
	entry.removed = false;
	entry.number = number;
	vf().entries.push_back(entry);
}

void CodeBuilder::var_end(){
	var_frames_.downsize(1);
}

void CodeBuilder::scope_begin(){
	int_t scope_info_num = result_->scope_info_table_.size();

	ScopeInfo info;
	info.pc = code_size();

	ff().variable_count += vf().entries.size();
	if(ff().max_variable_count<ff().variable_count){
		ff().max_variable_count = ff().variable_count;
	}

	int_t real_entry_num = vf().entries.size();

	for(uint_t i=0; i<vf().entries.size(); ++i){
		VarFrame::Entry& entry = vf().entries[i];

		// 変数を消せるか調べる
		if(entry.expr && !entry.assigned){
			entry.value = do_expr(entry.expr);
			if(rawne(entry.value, undefined)){
				entry.initialized = true;
				entry.removed = true;
				real_entry_num--;

				result_->identifier_table_->push_back(entry.name);
			}
		}
	}

	vf().real_entry_num = real_entry_num;
	vf().scope_info_num = scope_info_num;

	info.variable_size = real_entry_num;
	info.variable_identifier_offset = result_->identifier_table_->size();
	for(uint_t i=0; i<vf().entries.size(); ++i){
		VarFrame::Entry& entry = vf().entries[i];
		if(!entry.removed){	
			result_->identifier_table_->push_back(entry.name);
		}
	}

	if(vf().real_entry_num!=0){
		put_inst(InstScopeBegin(scope_info_num));
	}

	ff().variable_count += vf().real_entry_num;

	result_->scope_info_table_.push_back(info);
}

void CodeBuilder::scope_end(){
	
	ff().variable_count -= vf().real_entry_num;

	if(vf().real_entry_num!=0){
		put_inst(InstScopeEnd());
	}
		
	if(vf().scope_chain){
		result_->scope_info_table_[vf().scope_info_num].flags |= FunInfo::FLAG_SCOPE_CHAIN;
	}
}

int_t CodeBuilder::code_size(){
	return result_->code_.size();
}

void CodeBuilder::compile_bin(const ExprPtr& e){
	if(is_comp_bin(e->bin_lhs()) || is_comp_bin(e->bin_rhs())){
		error_->error(lineno(), Xt("Xtal Compile Error 1013"));
	}
	
	compile_expr(e->bin_lhs());
	compile_expr(e->bin_rhs());

	InstAdd inst;
	inst.op += e->itag() - EXPR_ADD;
	put_inst(inst);
}

void CodeBuilder::compile_comp_bin(const ExprPtr& e){
	if(is_comp_bin(e->bin_lhs()) || is_comp_bin(e->bin_rhs())){
		error_->error(lineno(), Xt("Xtal Compile Error 1025"));
	}

	int_t label_false = reserve_label();
	put_if_code(e, label_false);

	put_inst(InstPushTrueAndSkip());

	set_label(label_false);

	put_inst(InstPushFalse());
}

void CodeBuilder::compile_comp_bin_assert(const AnyPtr& f, const ExprPtr& e, const ExprPtr& str, const ExprPtr& mes, int_t label){
	if(is_comp_bin(e->bin_lhs()) || is_comp_bin(e->bin_rhs())){
		error_->error(lineno(), Xt("Xtal Compile Error 1025"));
	}
	
	{
		int_t label_false = reserve_label();
		put_if_code(e, label_false);
		put_inst(InstPushTrueAndSkip());
		set_label(label_false);
		put_inst(InstPushFalse());
	}

	int_t label_true = reserve_label();
	set_jump(InstIf::OFFSET_address_false, label_true);
	set_jump(InstIf::OFFSET_address_true, label);
	put_inst(InstIf());
	set_label(label_true);

	if(str){ compile_expr(str); }
	else{ put_inst(InstValue(register_value(empty_string))); }

	compile_expr(e->bin_lhs());
	compile_expr(e->bin_rhs());

	if(mes){ compile_expr(mes); }
	else{ put_inst(InstValue(register_value(empty_string))); }

	put_inst(InstValue(register_value(f)));
	put_inst(InstCall(4, 0, 1, 0));
	put_inst(InstAssert());
}

void CodeBuilder::compile_op_assign(const ExprPtr& e){
	ExprPtr lhs = e->bin_lhs();
	ExprPtr rhs = e->bin_rhs();

	InstAddAssign inst;
	inst.op += e->itag() - EXPR_ADD_ASSIGN;

	if(lhs->itag()==EXPR_LVAR){
		put_local_code(lhs->lvar_name());
		compile_expr(rhs);
		put_inst(inst);
		put_set_local_code(lhs->lvar_name());
	}
	else if(lhs->itag()==EXPR_IVAR){
		put_instance_variable_code(lhs->ivar_name());
		compile_expr(rhs);
		put_inst(inst);
		put_set_instance_variable_code(lhs->ivar_name());
	}
	else if(lhs->itag()==EXPR_PROPERTY){
		compile_expr(lhs->property_term());
		put_inst(InstDup());
		put_send_code(lhs->property_name(), lhs->property_ns(), 1, 0, 0, CALL_FLAG_NONE);
		compile_expr(rhs);
		put_inst(inst);
		put_inst(InstInsert1());
		put_set_send_code(lhs->property_name(), lhs->property_ns(), CALL_FLAG_NONE);
	}
	else if(lhs->itag()==EXPR_PROPERTY_Q){
		compile_expr(lhs->property_term());
		put_inst(InstDup());
		put_send_code(lhs->property_name(), lhs->property_ns(), 1, 0, 0, CALL_FLAG_Q);
		compile_expr(rhs);
		put_inst(inst);
		put_inst(InstInsert1());
		put_set_send_code(lhs->property_name(), lhs->property_ns(), CALL_FLAG_Q);
	}
	else if(lhs->itag()==EXPR_AT){
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
	inst.op += e->itag() - EXPR_INC;

	if(term->itag()==EXPR_LVAR){
		LVarInfo info = var_find(term->lvar_name());
		if(info.pos>=0){
			if(info.pos>=256){
				put_local_code(term->lvar_name());
				put_inst(inst);
				put_set_local_code(term->lvar_name());
			}
			else{
				if(e->itag() == EXPR_INC){
					put_inst(InstLocalVariableInc1Byte(info.pos));
				}
				else{
					put_inst(InstLocalVariableDec1Byte(info.pos));
				}
				put_inst(InstSetLocalVariable1Byte(info.pos));
			}

			entry(info).value = undefined;

		}
		else{
			error_->error(lineno(), Xt("Xtal Compile Error 1009")->call(Named(Xid(name), term->lvar_name())));
		}

	}
	else if(term->itag()==EXPR_IVAR){
		put_instance_variable_code(term->ivar_name());
		put_inst(inst);
		put_set_instance_variable_code(term->ivar_name());
	}
	else if(term->itag()==EXPR_PROPERTY){
		compile_expr(term->property_term());
		put_inst(InstDup());
		put_send_code(term->property_name(), term->property_ns(), 1, 0, 0, CALL_FLAG_NONE);
		put_inst(inst);
		put_inst(InstInsert1());
		put_set_send_code(term->property_name(), term->property_ns(), CALL_FLAG_NONE);
	}
	else if(term->itag()==EXPR_PROPERTY_Q){
		compile_expr(term->property_term());
		put_inst(InstDup());
		put_send_code(term->property_name(), term->property_ns(), 1, 0, 0, CALL_FLAG_Q);
		put_inst(inst);
		put_inst(InstInsert1());
		put_set_send_code(term->property_name(), term->property_ns(), CALL_FLAG_Q);
	}
	else if(term->itag()==EXPR_AT){
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
	IDPtr label;
	int_t label_kind = 0;

	if(e->itag()==EXPR_BREAK){
		label = e->break_label();
		label_kind = 0;
	}
	else if(e->itag()==EXPR_CONTINUE){
		label = e->continue_label();
		label_kind = 1;
	}

	if(ff().loops.empty()){
		error_->error(lineno(), Xt("Xtal Compile Error 1006"));
	}
	else{
		if(label){
			bool found = false;
			for(int_t i = 0, last = ff().loops.size(); i<last; ++i){
				if(raweq(ff().loops[i].label, label)){
					break_off(ff().loops[i].frame_count);
					set_jump(InstGoto::OFFSET_address, ff().loops[i].control_statement_label[label_kind]);
					put_inst(InstGoto());
					found = true;
					break;
				}
			}

			if(!found){
				error_->error(lineno(), Xt("Xtal Compile Error 1006"));
			}
		}
		else{
			bool found = false;
			for(size_t i = 0, last = ff().loops.size(); i<last; ++i){
				if(!ff().loops[i].have_label){
					break_off(ff().loops[i].frame_count);
					set_jump(InstGoto::OFFSET_address, ff().loops[i].control_statement_label[label_kind]);
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

void CodeBuilder::compile_class(const ExprPtr& e, const IDPtr& id){
	// インスタンス変数を暗黙的初期化するメソッドを定義する

	{
		int_t method_kind = e->class_kind()==KIND_SINGLETON ? KIND_FUN : KIND_METHOD;
		ExprPtr stmts = xnew<Expr>();
		MapPtr ivar_map = xnew<Map>();
		bool auto_initialize = false;
		Xfor_cast(const ExprPtr& v, e->class_stmts()->clone()){
			if(v->itag()==EXPR_CDEFINE_IVAR){
				if(v->cdefine_ivar_term()){
					eb_.push(v->cdefine_ivar_name());
					eb_.splice(EXPR_IVAR, 1);
					eb_.push(v->cdefine_ivar_term());
					eb_.splice(EXPR_ASSIGN, 2);
					stmts->push_back(eb_.pop());
					auto_initialize = true;
				}

				// 可触性が付いているので、アクセッサを定義する
				if(v->cdefine_ivar_accessibility()){ 
					IDPtr var = v->cdefine_ivar_name();
					eb_.push(v->cdefine_ivar_accessibility());
					eb_.push(var);
					eb_.push(null);
					
					eb_.push(method_kind);
					eb_.push(null);
					eb_.push(null);
					eb_.push(var);
					eb_.splice(EXPR_IVAR, 1);
					eb_.splice(0, 1);
					eb_.splice(EXPR_RETURN, 1);
					eb_.splice(EXPR_FUN, 4);

					eb_.splice(EXPR_CDEFINE_MEMBER, 4);

					e->class_stmts()->push_back(eb_.pop());

					IDPtr var2 = Xid(set_)->cat(var);
					eb_.push(v->cdefine_ivar_accessibility());
					eb_.push(var2);
					eb_.push(null);

					eb_.push(method_kind);
					eb_.push(Xid(value));
					eb_.splice(EXPR_LVAR, 1);
					eb_.push(null);
					eb_.splice(0, 2);
					eb_.splice(0, 1);
					eb_.push(null);
					eb_.push(var);
					eb_.splice(EXPR_IVAR, 1);
					eb_.push(Xid(value));
					eb_.splice(EXPR_LVAR, 1);
					eb_.splice(EXPR_ASSIGN, 2);
					eb_.splice(EXPR_FUN, 4);

					eb_.splice(EXPR_CDEFINE_MEMBER, 4);

					e->class_stmts()->push_back(eb_.pop());
				}
			}
		}

		eb_.push(KIND_PUBLIC);
		eb_.push(Xid(auto_initialize));
		eb_.push(null);

		if(auto_initialize){
			eb_.push(method_kind);
			eb_.push(null);
			eb_.push(null);
			eb_.push(stmts);
			eb_.splice(EXPR_SCOPE, 1);
			eb_.splice(EXPR_FUN, 4);
		}
		else{
			eb_.splice(EXPR_NULL, 0);
		}

		eb_.splice(EXPR_CDEFINE_MEMBER, 4);

		e->class_stmts()->push_front(eb_.pop());
	}

	// 継承
	int_t mixins = compile_exprs(e->class_mixins());

	// 変数フレームを作成
	var_begin(VarFrame::CLASS);

	// 変数を定義
	{
		int_t number = 0;
		Xfor(v, e->class_stmts()){
			ExprPtr v1 = ep(v);
			if(v1->itag()==EXPR_CDEFINE_MEMBER){
				if(v1->cdefine_member_ns()){
					var_define(v1->cdefine_member_name(), null, v1->cdefine_member_accessibility()->to_i(), false, true, false, number++);
				}
				else{
					var_define(v1->cdefine_member_name(), null, v1->cdefine_member_accessibility()->to_i(), false, true, false);
				}
			}
		}
	}

	class_frames_.push();
	cf().entries.clear();
	cf().class_info_num = result_->class_info_table_.size();

	int_t ivar_num = 0;
	int_t instance_variable_identifier_offset = result_->identifier_table_->size();
	Xfor_cast(const ExprPtr& v, e->class_stmts()){
		if(v->itag()==EXPR_CDEFINE_IVAR){
			ClassFrame::Entry entry;
			entry.name = v->cdefine_ivar_name();
			cf().entries.push_back(entry);

			result_->identifier_table_->push_back(entry.name);

			ivar_num++;
		}
	}

	int_t class_info_num = result_->class_info_table_.size();

	ClassInfo info;
	info.pc = code_size();
	info.kind = e->class_kind();
	info.mixins = mixins;
	info.variable_size = vf().entries.size();
	info.instance_variable_size = ivar_num;
	info.instance_variable_identifier_offset = instance_variable_identifier_offset;
	info.name_number = register_identifier(id);
	
	info.variable_identifier_offset = result_->identifier_table_->size();
	for(uint_t i=0; i<vf().entries.size(); ++i){
		result_->identifier_table_->push_back(vf().entries[i].name);
	}


	put_inst(InstClassBegin(class_info_num));
	result_->class_info_table_.push_back(info);

	{
		int_t number = 0;
		Xfor(v, e->class_stmts()){
			ExprPtr v1 = ep(v);
			if(v1->itag()==EXPR_CDEFINE_MEMBER){					
				AnyPtr val;
				if(!compile_expr(v1->cdefine_member_term(), CompileInfo(), val)){
					compile_expr(v1->cdefine_member_term());
				}


				if(v1->cdefine_member_ns()){
					compile_expr(v1->cdefine_member_ns());
					LVarInfo info = var_find(v1->cdefine_member_name(), true, false, number++);
					entry(info).value = val;
					put_inst(InstDefineClassMember(info.pos, register_identifier(v1->cdefine_member_name()), v1->cdefine_member_accessibility()->to_i()));
				}
				else{
					put_inst(InstPushUndefined());
					LVarInfo info = var_find(v1->cdefine_member_name(), true, false);
					entry(info).value = val;
					put_inst(InstDefineClassMember(info.pos, register_identifier(v1->cdefine_member_name()), v1->cdefine_member_accessibility()->to_i()));
				}
			}
		}
	}

	if(var_frames_.top().scope_chain){
		result_->class_info_table_[class_info_num].flags |= FunInfo::FLAG_SCOPE_CHAIN;
	}

	put_inst(InstClassEnd());
	class_frames_.pop();

	var_end();
}

void CodeBuilder::compile_fun(const ExprPtr& e, const IDPtr& id){

	int_t ordered = 0;
	int_t named = 0;

	Xfor_cast(const ExprPtr& v, e->fun_params()){
		if(!v->at(0) || (ep(v->at(0))->itag()!=EXPR_LVAR && ep(v->at(0))->itag()!=EXPR_IVAR)){
			error_->error(lineno(), Xt("Xtal Compile Error 1004"));
			return;
		}

		if(v->at(1)){
			named++;
		}
		else{
			if(named!=0){
				error_->error(lineno(), Xt("Xtal Compile Error 1005"));
			}

			ordered++;
		}
	}

	int_t minv = ordered;
	int_t maxv = ordered + named;

	// ゲッター、またはセッター用の最適化を行う
	if(e->fun_kind()==KIND_METHOD){

		// ゲッタか？
		if(ordered==0 && named==0){
			ExprPtr body = e->fun_body();
			if(body->itag()==EXPR_SCOPE && body->scope_stmts() && body->scope_stmts()->size()==1){
				body = ep(body->scope_stmts()->front());
			}

			if(body->itag()==EXPR_RETURN){
				if(body->return_exprs() && body->return_exprs()->size()==1){
					body = ep(body->return_exprs()->front());
					if(body->itag()==EXPR_IVAR){
						put_inst(InstMakeInstanceVariableAccessor(0, lookup_instance_variable(body->ivar_name()), class_info_num()));
						return;
					}
				}
			}
		}

		// セッタか？
		if(ordered==1 && named==0){
			ExprPtr body = e->fun_body();
			if(body->itag()==EXPR_SCOPE && body->scope_stmts() && body->scope_stmts()->size()==1){
				body = ep(body->scope_stmts()->front());
			}

			if(body->itag()==EXPR_ASSIGN){
				ExprPtr lhs = body->bin_lhs();
				ExprPtr rhs = body->bin_rhs();

				IDPtr key;
				ExprPtr arg = ep(e->fun_params()->at(0));
				if(arg){
					arg = ep(arg->at(0));
				}

				if(arg->itag()==EXPR_LVAR){
					key = arg->lvar_name();
				}
				else if(arg->itag()==EXPR_IVAR){
					key = arg->ivar_name();
				}

				if(key && lhs->itag()==EXPR_IVAR && rhs->itag()==EXPR_LVAR && raweq(rhs->lvar_name(), key)){
					put_inst(InstMakeInstanceVariableAccessor(1, lookup_instance_variable(lhs->ivar_name()), class_info_num()));
					return;
				}
			}
		}
	}

	// 関数フレームを作成する
	fun_frames_.push();	
	ff().labels.clear();
	ff().loops.clear();
	ff().finallies.clear();
	ff().var_frame_count = var_frames_.size();
	ff().stack_count = 0;
	ff().max_stack_count = 0;
	ff().variable_count = 0;
	ff().max_variable_count = 0;
	ff().extendable_param = e->fun_extendable_param();

	// 変数フレームを作成して、引数を登録する
	var_begin(VarFrame::DEFAULT);
	Xfor_cast(const ExprPtr& v1, e->fun_params()){
		if(const ExprPtr& v = ep(v1->at(0))){
			if(v->itag()==EXPR_LVAR){
				var_define(v->lvar_name());
			}
			else if(v->itag()==EXPR_IVAR){
				var_define(v->ivar_name());
			}
		}
	}

	// 可変長引数があるか？
	if(e->fun_extendable_param()){
		var_define(e->fun_extendable_param());
	}

	// 関数コアを作成
	FunInfo info;
	info.pc = code_size() + InstMakeFun::ISIZE;
	info.kind = e->fun_kind();
	info.min_param_count = minv;
	info.max_param_count = maxv;
	info.name_number = register_identifier(id);
	info.flags = e->fun_extendable_param() ? FunInfo::FLAG_EXTENDABLE_PARAM : 0;

	// 引数の名前を識別子テーブルに順番に乗せる
	info.variable_size = vf().entries.size();
	info.variable_identifier_offset = result_->identifier_table_->size();
	for(uint_t i=0; i<vf().entries.size(); ++i){
		vf().entries[i].initialized = true;
		result_->identifier_table_->push_back(vf().entries[i].name);
	}

	int_t fun_info_table_number = result_->xfun_info_table_.size();
	result_->xfun_info_table_.push_back(info);

	int_t fun_end_label = reserve_label();
	set_jump(InstMakeFun::OFFSET_address, fun_end_label);
	put_inst(InstMakeFun(fun_info_table_number, 0));

	ff().variable_count += vf().entries.size();
	if(ff().max_variable_count<ff().variable_count){
		ff().max_variable_count = ff().variable_count;
	}

	// デフォルト値を持つ引数を処理する
	{
		int_t i = 0;
		Xfor_cast(const ExprPtr& v1, e->fun_params()){
			if(const ExprPtr& v = ptr_cast<Expr>(v1->at(1))){
				int_t label = reserve_label();
				
				set_jump(InstIfArgIsUndefined::OFFSET_address, label);
				put_inst(InstIfArgIsUndefined(maxv-1-i, 0));

				compile_expr(v);
				
				var_find(vf().entries[i].name, true);
				put_set_local_code(vf().entries[i].name);
				
				set_label(label);
			}
			else{
				var_find(vf().entries[i].name, true);
			}	

			++i;
		}
	}

	// 引数にインスタンス変数がある場合に、特別な処理を入れる
	Xfor_cast(const ExprPtr& v1, e->fun_params()){
		const ExprPtr& v = ep(v1->at(0));
		if(v->itag()==EXPR_IVAR){
			eb_.push(v);
			eb_.push(v->ivar_name());
			eb_.splice(EXPR_LVAR, 1);
			eb_.splice(EXPR_ASSIGN, 2);
			compile_stmt(eb_.pop());
		}
	}

	if(e->fun_params()){
		add_stack_count(e->fun_params()->size());
		add_stack_count(-(int_t)e->fun_params()->size());
	}

	// 関数本体を処理する
	compile_stmt(e->fun_body());

	XTAL_ASSERT(ff().stack_count==0);
	
	break_off(ff().var_frame_count+1);

	put_inst(InstReturn(0));
	set_label(fun_end_label);

	process_labels();

	if(vf().scope_chain){
		result_->xfun_info_table_[fun_info_table_number].flags |= FunInfo::FLAG_SCOPE_CHAIN;
	}

	// 変数フレームをポップする
	var_end();

	result_->xfun_info_table_[fun_info_table_number].max_variable = ff().max_variable_count;

	// 関数フレームをポップする
	fun_frames_.downsize(1);

}

int_t CodeBuilder::compile_exprs(const ExprPtr& e){
	int_t count = 0;
	Xfor(v, e){
		compile_expr(v);
		count++;
	}
	return count;
}

bool CodeBuilder::compile_expr(const AnyPtr& p, const CompileInfo& info, AnyPtr& ret){
	if(!p){
		if(info.need_result_count==1){
			put_inst(InstPushNull());
			ret = null;
			return true;
		}
		else if(info.need_result_count!=0){
			put_inst(InstAdjustResult(0, info.need_result_count));
		}
		ret = undefined;
		return true;
	}

	ExprPtr e = ep(p);

	AnyPtr val = do_expr(e);
	if(rawne(val, undefined)){
		if(info.need_result_count!=0){
			put_val_code(val);

			if(info.need_result_count!=1){
				put_inst(InstAdjustResult(1, info.need_result_count));
			}
		}
		ret = val;
		return true;
	}

	ret = undefined;
	return false;
}



AnyPtr CodeBuilder::do_expr(const AnyPtr& p){

	if(!p){
		return null;
	}

	ExprPtr e = ep(p);

	switch(e->itag()){

		XTAL_DEFAULT;

		XTAL_CASE(EXPR_NULL){ return null; }
		XTAL_CASE(EXPR_TRUE){ return true; }
		XTAL_CASE(EXPR_FALSE){ return false; }
		XTAL_CASE(EXPR_NUMBER){ return e->number_value(); }
		XTAL_CASE(EXPR_STRING){
			if(e->string_kind()==KIND_STRING){
				return e->string_value();
			}
		}

		XTAL_CASE(EXPR_LVAR){
			LVarInfo info = var_find(e->lvar_name(), false, true);
			if(info.pos>=0){
				if((var_frame(info).fun_frames_size==fun_frames_.size() || !entry(info).assigned)){
					entry(info).referenced = true;
					return entry(info).value;
				}
			}
		}
	}

	return undefined;
}

void CodeBuilder::check_lvar_assign(const ExprPtr& e){
	if(e && e->itag()==EXPR_LVAR){
		LVarInfo info = var_find(e->lvar_name(), true, true);
		if(info.pos>=0){
			entry(info).assigned = true;
			entry(info).value = undefined;
		}
	}
}

// 代入演算子があるか調べる
void CodeBuilder::check_lvar_assign_stmt(const AnyPtr& p){
	if(ExprPtr e = ep(p)){
		switch(e->itag()){
		case EXPR_ASSIGN:
		case EXPR_ADD_ASSIGN:
		case EXPR_SUB_ASSIGN:
		case EXPR_CAT_ASSIGN:
		case EXPR_MUL_ASSIGN:
		case EXPR_DIV_ASSIGN:
		case EXPR_MOD_ASSIGN:
		case EXPR_OR_ASSIGN:
		case EXPR_AND_ASSIGN:
		case EXPR_XOR_ASSIGN:
		case EXPR_SHR_ASSIGN:
		case EXPR_SHL_ASSIGN:
		case EXPR_USHR_ASSIGN:
			check_lvar_assign(e->bin_lhs());
			break;

		case EXPR_INC:
		case EXPR_DEC:
			check_lvar_assign(e->una_term());
			break;

		case EXPR_MASSIGN:
			Xfor(v, e->massign_lhs_exprs()){
				check_lvar_assign(ep(v));
			}
			break;
		}

		Xfor(v, e){
			check_lvar_assign_stmt(v);
		}
	}
}

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
	case EXPR_VALUES:

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

