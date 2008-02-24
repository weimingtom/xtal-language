#include "xtal.h"
#include "xtal_macro.h"

#ifndef XTAL_NO_PARSER

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
	result_->scope_core_table_.reserve(10000);
	result_->except_core_table_.reserve(10000);

	result_->first_fun()->set_core(&result_->xfun_core_table_[0]);

	int_t pc_pos = 0;
	InteractiveStreamPtr stream = xnew<InteractiveStream>();
	while(!stream->eos()){

		ExprPtr e = parser_.parse_stmt(stream, error_);
		stream->terminate_statement();

		if(!e){
			stderr_stream()->put_s(error_->errors->join("\n"));
			stderr_stream()->put_s("\n");
			error_->errors->clear();
		}
		else{

			if(e->tag()==EXPR_RETURN){
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

				XTAL_TRY{
					vmachine()->execute(result_->first_fun().get(), &result_->code_[pc_pos]);
				}
				XTAL_CATCH(e){
					std::printf("%s\n", e->to_s()->c_str());
				}

				for(int_t i=0; i<(sizeof(InstThrow)+sizeof(InstReturn))/sizeof(inst_t); ++i){
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
	ff().extendable_param = true;

	// 変数フレームを作成して、引数を登録する
	var_begin(VarFrame::FRAME);

	// 関数コアを作成
	FunCore core;
	core.pc = 0;
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

	put_inst(InstReturn(0));
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

	size_t cur = result_->code_.size();
	result_->code_.resize(cur+sz/sizeof(inst_t));
	std::memcpy(&result_->code_[cur], &t, sz);
}

bool CodeBuilder::put_set_local_code(const IDPtr& var){
	LVarInfo info = var_find(var);
	if(info.pos>=0){
		if(info.entry->constant){
			error_->error(lineno(), Xt("Xtal Compile Error 1019")(Named("name", var)));
		}

		if(info.pos<=0xff){
			var_set_direct(*info.var_frame);
			put_inst(InstSetLocalVariable1Byte(info.pos));
		}
		else{
			put_inst(InstSetLocalVariable2Byte(info.pos));
		}

		info.entry->value = undefined;

		return true;
	}
	else{
		put_inst(InstSetGlobalVariable(register_identifier(var)));
		return false;
	}
}

void CodeBuilder::put_define_local_code(const IDPtr& var, const ExprPtr& rhs){
	LVarInfo info = var_find(var, true);

	if(info.pos>=0){

		if(rhs){
			AnyPtr val = info.entry->initialized ? info.entry->value : do_expr(rhs);

			if(raweq(val, undefined) || info.entry->assigned){

				if(raweq(val, undefined)){
					compile_expr(rhs);
				}
				else{
					put_val_code(val);
				}

				if(info.pos<=0xff){
					var_set_direct(*info.var_frame);
					put_inst(InstSetLocalVariable1Byte(info.pos));
				}
				else{
					put_inst(InstSetLocalVariable2Byte(info.pos));
				}
			}

			info.entry->value = val;

		}
		else{
			if(info.pos<=0xff){
				var_set_direct(*info.var_frame);
				put_inst(InstSetLocalVariable1Byte(info.pos));
			}
			else{
				put_inst(InstSetLocalVariable2Byte(info.pos));
			}
		}
		
	}
	else{
		if(rhs) compile_expr(rhs);
		put_inst(InstDefineGlobalVariable(register_identifier(var)));
	}
}

bool CodeBuilder::put_local_code(const IDPtr& var){
	LVarInfo info = var_find(var);
	if(info.pos>=0){
		if(info.pos<=0xff){
			var_set_direct(*info.var_frame);
			put_inst(InstLocalVariable1Byte(info.pos));
		}
		else{
			put_inst(InstLocalVariable2Byte(info.pos));
		}		

		info.entry->referenced = true;

		return true;
	}
	else{
		put_inst(InstGlobalVariable(register_identifier(var)));
		return false;
	}
}

int_t CodeBuilder::register_identifier_or_compile_expr(const AnyPtr& var){
	if(const IDPtr& id = ptr_as<ID>(var)){ 
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

void CodeBuilder::put_send_code(const AnyPtr& var,int_t need_result_count, bool tail, bool q, const ExprPtr& secondary_key){
	int_t key = register_identifier_or_compile_expr(var);
	int_t flags = (tail ? CALL_FLAG_TAIL : 0);

	if(secondary_key){
		compile_expr(secondary_key);
		if(q){
			put_inst(InstSendQNS(0, 0, need_result_count, flags, key));
		}
		else{
			if(need_result_count==1 && flags==0){
				put_inst(InstPropertyNS(key));
			}
			else{
				put_inst(InstSendNS(0, 0, need_result_count, flags, key));
			}
		}
	}
	else{
		if(q){
			put_inst(InstSendQ(0, 0, need_result_count, flags, key));
		}
		else{
			if(need_result_count==1 && flags==0){
				put_inst(InstProperty(key));
			}
			else{
				put_inst(InstSend(0, 0, need_result_count, flags, key));
			}
		}
	}
}

void CodeBuilder::put_set_send_code(const AnyPtr& var, bool q, const ExprPtr& secondary_key){
	int_t key = 0;
	if(ptr_as<Expr>(var)){ 
		eb_.push(KIND_STRING);
		eb_.push("set_");
		eb_.splice(EXPR_STRING, 2);
		eb_.push(ep(var));
		eb_.splice(EXPR_CAT, 2);
		compile_expr(eb_.pop()); 
	}
	else{
		key = register_identifier(xnew<String>("set_")->cat(ptr_as<ID>(var)));
	}
	
	if(secondary_key){
		compile_expr(secondary_key);

		if(q){
			put_inst(InstSendQNS(1, 0, 0, 0, key));
		}
		else{
			put_inst(InstSetPropertyNS(key));
		}
	}
	else{
		if(q){
			put_inst(InstSendQ(1, 0, 0, 0, key));
		}
		else{
			put_inst(InstSetProperty(key));
		}
	}
}

void CodeBuilder::put_define_member_code(const AnyPtr& var, const ExprPtr& secondary_key){
	int_t key = register_identifier_or_compile_expr(var);

	if(secondary_key){
		compile_expr(secondary_key);
		put_inst(InstDefineMemberNS(key));
	}
	else{
		put_inst(InstDefineMember(key));
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

	error_->error(lineno(), Xt("Xtal Compile Error 1023")(Named("name", xnew<String>("_")->cat(key))));
	return 0;
}

void CodeBuilder::put_set_instance_variable_code(const IDPtr& var){
	put_inst(InstSetInstanceVariable(lookup_instance_variable(var), class_core_num()));
}

void CodeBuilder::put_instance_variable_code(const IDPtr& var){
	put_inst(InstInstanceVariable(lookup_instance_variable(var), class_core_num()));
}

int_t CodeBuilder::reserve_label(){
	ff().labels.resize(fun_frames_.top().labels.size()+1);
	ff().labels[ff().labels.size()-1].pos = -1;
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
		XTAL_ASSERT(l.pos!=-1);
		for(size_t j = 0; j<l.froms.size(); ++j){
			FunFrame::Label::From &f = l.froms[j];
			inst_address_t& buf = *(inst_address_t*)&result_->code_[f.set_pos];
			buf = l.pos - f.pos;
			//XTAL_ASSERT(l.pos - f.pos > -1000);
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
		if(vf.real_entry_num!=0 && (vf.kind==VarFrame::SCOPE || vf.kind==VarFrame::FRAME)){
			var_set_direct(vf);
			put_inst(InstBlockEnd(vf.scope_core_num));
		}
	}
}

static bool is_comp_bin(ExprPtr e){
	if(e && EXPR_EQ<=e->tag() && e->tag()<=EXPR_NIS){
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
			else if(value==(i16)value){ 
				put_inst(InstPushInt2Byte(value));
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
			else if(value==(i16)value){ 
				put_inst(InstPushFloat2Byte((i16)value));
			}
			else{ 
				put_inst(InstValue(register_value(value)));
			}
		}
	}
}

void CodeBuilder::put_if_code(const ExprPtr& e, int_t label_if, int_t label_if2){
	AnyPtr val = do_expr(e);

	if(rawne(val, undefined)){
		if(!val){
			set_jump(InstGoto::OFFSET_address, label_if);
			put_inst(InstGoto());
		}
	}
	else if(is_comp_bin(e)){
		
		compile_expr(e->bin_lhs());
		compile_expr(e->bin_rhs());

		set_jump(InstIfEq::OFFSET_address, label_if);
		InstIfEq inst;
		inst.op += e->tag()-EXPR_EQ;
		put_inst(inst);

		if(e->tag()==EXPR_NE || e->tag()==EXPR_LE || e->tag()==EXPR_GE || e->tag()==EXPR_NIN){
			set_jump(InstUnless::OFFSET_address, label_if2);
			put_inst(InstUnless());
		}
		else{
			set_jump(InstIf::OFFSET_address, label_if2);
			put_inst(InstIf());
		}
	}
	else{
		if(e->tag()==EXPR_NOT){
			compile_expr(e->una_term());
			set_jump(InstUnless::OFFSET_address, label_if);
			put_inst(InstUnless());
		}
		else{
			compile_expr(e);
			set_jump(InstIf::OFFSET_address, label_if);
			put_inst(InstIf());
		}
	}
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
				if(vf.fun_frames_size!=fun_frames_.size() || entry.initialized || define){
					ret.var_frame = &vf;
					ret.entry = &entry;
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
	vf().directs.clear();
	vf().scope_core_num = 0;
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
	if(ExprPtr v = ptr_as<Expr>(stmt)){
		if(v->tag()==EXPR_DEFINE){
			if(v->bin_lhs()->tag()==EXPR_LVAR){
				var_define(v->bin_lhs()->lvar_name(), v->bin_rhs());
			}
		}
		else if(v->tag()==EXPR_MDEFINE){
			Xfor(v1, v->mdefine_lhs_exprs()){
				ExprPtr vv = ep(v1);
				if(vv->tag()==EXPR_LVAR){
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
				error_->error(lineno(), Xt("Xtal Compile Error 1026")(name));
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

void CodeBuilder::var_set_direct(VarFrame& vf){
	if(!vf.directs.empty() && vf.directs.back().pos==code_size()){
		return;
	}

	VarFrame::Direct d;
	d.pos = code_size();
	vf.directs.push_back(d);
}

void CodeBuilder::var_set_on_heap(int_t i){
	if(i<(int_t)var_frames_.size()){
		if(var_frames_[i].kind==VarFrame::SCOPE){
			var_frames_[i].kind = VarFrame::FRAME;
		}
	}
}

void CodeBuilder::var_end(){
	// ローカル変数の命令を、ダイレクト系へ変更する
	if(vf().kind==VarFrame::SCOPE){
		for(uint_t i=0; i<vf().directs.size(); ++i){
			Inst* p = (Inst*)&result_->code_[vf().directs[i].pos];
			
			switch(p->op){
				default:
					break;

				case InstLocalVariableInc/*Direct*/::NUMBER:
				case InstLocalVariableDec/*Direct*/::NUMBER:
				case InstLocalVariable1Byte/*Direct*/::NUMBER:
				case InstSetLocalVariable1Byte/*Direct*/::NUMBER:
				case InstBlockBegin/*Direct*/::NUMBER:
				case InstBlockEnd/*Direct*/::NUMBER:
				case InstIfArgIsUndefined/*Direct*/::NUMBER:
					p->op += 1;
					break;
			}
		}
		vf().directs.clear();
	}
	var_frames_.downsize(1);
}

void CodeBuilder::scope_begin(){
	int_t scope_core_num = result_->scope_core_table_.size();

	ScopeCore core;
	core.pc = code_size();

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
			}
		}
	}

	vf().real_entry_num = real_entry_num;
	vf().scope_core_num = scope_core_num;

	core.variable_size = real_entry_num;
	core.variable_identifier_offset = result_->identifier_table_->size();
	for(uint_t i=0; i<vf().entries.size(); ++i){
		VarFrame::Entry& entry = vf().entries[i];
		if(!entry.removed){	
			result_->identifier_table_->push_back(entry.name);
		}
	}

	if(vf().real_entry_num!=0){
		var_set_direct(vf());
		put_inst(InstBlockBegin(scope_core_num));
	}

	result_->scope_core_table_.push_back(core);
}

void CodeBuilder::scope_end(){
	if(vf().real_entry_num!=0){
		var_set_direct(vf());
		put_inst(InstBlockEnd(vf().scope_core_num));
	}

	if(vf().kind!=VarFrame::SCOPE){
		result_->scope_core_table_[vf().scope_core_num].flags |= FunCore::FLAG_ON_HEAP;
	}

	if(vf().scope_chain){
		result_->scope_core_table_[vf().scope_core_num].flags |= FunCore::FLAG_SCOPE_CHAIN;
		if(vf().kind!=VarFrame::SCOPE){
			var_set_on_heap(1);
		}
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
	inst.op += e->tag() - EXPR_ADD;
	put_inst(inst);
}

void CodeBuilder::compile_comp_bin(const ExprPtr& e){
	if(is_comp_bin(e->bin_lhs()) || is_comp_bin(e->bin_rhs())){
		error_->error(lineno(), Xt("Xtal Compile Error 1025"));
	}
	
	compile_expr(e->bin_lhs());
	compile_expr(e->bin_rhs());

	InstEq inst;
	inst.op += e->tag() - EXPR_EQ;
	put_inst(inst);

	if(e->tag()==EXPR_NE || e->tag()==EXPR_LE || e->tag()==EXPR_GE || e->tag()==EXPR_NIN){
		put_inst(InstNot());
	}
}

void CodeBuilder::compile_op_assign(const ExprPtr& e){
	ExprPtr lhs = e->bin_lhs();
	ExprPtr rhs = e->bin_rhs();

	InstAddAssign inst;
	inst.op += e->tag() - EXPR_ADD_ASSIGN;

	if(lhs->tag()==EXPR_LVAR){
		put_local_code(lhs->lvar_name());
		compile_expr(rhs);
		put_inst(inst);
		put_set_local_code(lhs->lvar_name());
	}
	else if(lhs->tag()==EXPR_IVAR){
		put_instance_variable_code(lhs->ivar_name());
		compile_expr(rhs);
		put_inst(inst);
		put_set_instance_variable_code(lhs->ivar_name());
	}
	else if(lhs->tag()==EXPR_SEND){
		compile_expr(lhs->send_term());
		put_inst(InstDup());
		put_send_code(lhs->send_name(), 1, false, false, lhs->send_ns());
		compile_expr(rhs);
		put_inst(inst);
		put_inst(InstInsert1());
		put_set_send_code(lhs->send_name(), false, lhs->send_ns());
	}
	else if(lhs->tag()==EXPR_SEND_Q){
		compile_expr(lhs->send_term());
		put_inst(InstDup());
		put_send_code(lhs->send_name(), 1, false, true, lhs->send_ns());
		compile_expr(rhs);
		put_inst(inst);
		put_inst(InstInsert1());
		put_set_send_code(lhs->send_name(), true, lhs->send_ns());
	}
	else if(lhs->tag()==EXPR_AT){
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
	inst.op += e->tag() - EXPR_INC;

	if(term->tag()==EXPR_LVAR){
		LVarInfo info = var_find(term->lvar_name());
		if(info.pos>=0){
			if(info.pos>=256){
				if(e->tag() == EXPR_INC){
					put_inst(InstLocalVariableInc2Byte(info.pos));
				}
				else{
					put_inst(InstLocalVariableDec2Byte(info.pos));
				}
				put_inst(InstSetLocalVariable2Byte(info.pos));
			}
			else{
				if(e->tag() == EXPR_INC){
					var_set_direct(*info.var_frame);
					put_inst(InstLocalVariableInc(info.pos));
				}
				else{
					var_set_direct(*info.var_frame);
					put_inst(InstLocalVariableDec(info.pos));
				}
				var_set_direct(*info.var_frame);
				put_inst(InstSetLocalVariable1Byte(info.pos));
			}

			info.entry->value = undefined;

		}
		else{
			put_inst(InstGlobalVariable(register_identifier(term->lvar_name())));
			put_inst(inst);
			put_set_local_code(term->lvar_name());
		}

	}
	else if(term->tag()==EXPR_IVAR){
		put_instance_variable_code(term->ivar_name());
		put_inst(inst);
		put_set_instance_variable_code(term->ivar_name());
	}
	else if(term->tag()==EXPR_SEND){
		compile_expr(term->send_term());
		put_inst(InstDup());
		put_send_code(term->send_name(), 1, false, false, term->send_ns());
		put_inst(inst);
		put_inst(InstInsert1());
		put_set_send_code(term->send_name(), false, term->send_ns());
	}
	else if(term->tag()==EXPR_SEND_Q){
		compile_expr(term->send_term());
		put_inst(InstDup());
		put_send_code(term->send_name(), 1, false, true, term->send_ns());
		put_inst(inst);
		put_inst(InstInsert1());
		put_set_send_code(term->send_name(), true, term->send_ns());
	}
	else if(term->tag()==EXPR_AT){
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
	int_t label_kind;

	if(e->tag()==EXPR_BREAK){
		label = e->break_label();
		label_kind = 0;
	}
	else if(e->tag()==EXPR_CONTINUE){
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

void CodeBuilder::compile_class(const ExprPtr& e){
	// インスタンス変数を暗黙的初期化するメソッドを定義する
	{
		ExprPtr stmts = xnew<Expr>();
		MapPtr ivar_map = xnew<Map>();
		Xfor_as(const ExprPtr& v, e->class_stmts()->clone()){
			if(v->tag()==EXPR_CDEFINE_IVAR){
				if(v->cdefine_ivar_term()){
					eb_.push(v->cdefine_ivar_name());
					eb_.splice(EXPR_IVAR, 1);
					eb_.push(v->cdefine_ivar_term());
					eb_.splice(EXPR_ASSIGN, 2);
					stmts->push_back(eb_.pop());
				}

				// 可触性が付いているので、アクセッサを定義する
				if(v->cdefine_ivar_accessibility()){ 
					IDPtr var = v->cdefine_ivar_name();
					eb_.push(v->cdefine_ivar_accessibility());
					eb_.push(var);
					eb_.push(null);
					
					eb_.push(KIND_METHOD);
					eb_.push(null);
					eb_.push(null);
					eb_.push(var);
					eb_.splice(EXPR_IVAR, 1);
					eb_.splice(0, 1);
					eb_.splice(EXPR_RETURN, 1);
					eb_.splice(EXPR_FUN, 4);

					eb_.splice(EXPR_CDEFINE_MEMBER, 4);

					e->class_stmts()->push_back(eb_.pop());

					IDPtr var2 = xnew<String>("set_")->cat(var);
					eb_.push(v->cdefine_ivar_accessibility());
					eb_.push(var2);
					eb_.push(null);

					eb_.push(KIND_METHOD);
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
		eb_.push(Xid(_initialize_));
		eb_.push(null);

		eb_.push(KIND_METHOD);
		eb_.push(null);
		eb_.push(null);
		eb_.push(stmts);
		eb_.splice(EXPR_SCOPE, 1);
		eb_.splice(EXPR_FUN, 4);

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
			if(v1->tag()==EXPR_CDEFINE_MEMBER){
				if(v1->cdefine_member_ns()){
					var_define(v1->cdefine_member_name(), null, v1->cdefine_member_accessibility()->to_i(), false, true, false);
				}
				else{
					var_define(v1->cdefine_member_name(), null, v1->cdefine_member_accessibility()->to_i(), false, true, false, number++);
				}
			}
		}
	}

	class_frames_.push();
	cf().entries.clear();
	cf().class_core_num = result_->class_core_table_.size();

	int_t ivar_num = 0;
	Xfor_as(const ExprPtr& v, e->class_stmts()){
		if(v->tag()==EXPR_CDEFINE_IVAR){
			ClassFrame::Entry entry;
			entry.name = v->cdefine_ivar_name();
			cf().entries.push_back(entry);
			ivar_num++;
		}
	}

	int_t class_core_num = result_->class_core_table_.size();

	ClassCore core;
	core.pc = code_size();
	core.kind = e->class_kind();
	core.mixins = mixins;
	core.variable_size = vf().entries.size();
	core.instance_variable_size = ivar_num;
	
	core.variable_identifier_offset = result_->identifier_table_->size();
	for(uint_t i=0; i<vf().entries.size(); ++i){
		result_->identifier_table_->push_back(vf().entries[i].name);
	}

	core.instance_variable_identifier_offset = result_->identifier_table_->size();
	Xfor_as(const ExprPtr& v, e->class_stmts()){
		if(v->tag()==EXPR_CDEFINE_IVAR){
			if(v->cdefine_ivar_term()){
				result_->identifier_table_->push_back(v->cdefine_ivar_name());
			}			
		}
	}

	put_inst(InstClassBegin(class_core_num));
	result_->class_core_table_.push_back(core);

	{
		int_t number = 0;
		Xfor(v, e->class_stmts()){
			ExprPtr v1 = ep(v);
			if(v1->tag()==EXPR_CDEFINE_MEMBER){					
				AnyPtr val = compile_expr(v1->cdefine_member_term());
				compile_expr(v1->cdefine_member_ns());

				if(v1->cdefine_member_ns()){
					LVarInfo info = var_find(v1->cdefine_member_name(), true, false);
					info.entry->value = val;
					put_inst(InstDefineClassMember(info.pos, register_identifier(v1->cdefine_member_name()), v1->cdefine_member_accessibility()->to_i()));
				}
				else{
					LVarInfo info = var_find(v1->cdefine_member_name(), true, false, number++);
					info.entry->value = val;
					put_inst(InstDefineClassMember(info.pos, register_identifier(v1->cdefine_member_name()), v1->cdefine_member_accessibility()->to_i()));
				}
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

	int_t ordered = 0;
	int_t named = 0;

	Xfor_as(const ExprPtr& v, e->fun_params()){

		if(!v->at(0) || (ep(v->at(0))->tag()!=EXPR_LVAR && ep(v->at(0))->tag()!=EXPR_IVAR)){
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
			if(body->tag()==EXPR_SCOPE && body->scope_stmts() && body->scope_stmts()->size()==1){
				body = ep(body->scope_stmts()->front());
			}

			if(body->tag()==EXPR_RETURN){
				if(body->return_exprs() && body->return_exprs()->size()==1){
					body = ep(body->return_exprs()->front());
					if(body->tag()==EXPR_IVAR){
						put_inst(InstMakeInstanceVariableAccessor(0, lookup_instance_variable(body->ivar_name()), class_core_num()));
						return;
					}
				}
			}
		}

		// セッタか？
		if(ordered==1 && named==0){
			ExprPtr body = e->fun_body();
			if(body->tag()==EXPR_SCOPE && body->scope_stmts() && body->scope_stmts()->size()==1){
				body = ep(body->scope_stmts()->front());
			}

			if(body->tag()==EXPR_ASSIGN){
				ExprPtr lhs = body->bin_lhs();
				ExprPtr rhs = body->bin_rhs();

				IDPtr key;
				ExprPtr arg = ep(e->fun_params()->at(0));
				if(arg->tag()==EXPR_LVAR){
					key = arg->lvar_name();
				}
				else if(arg->tag()==EXPR_IVAR){
					key = arg->ivar_name();
				}

				if(key && lhs->tag()==EXPR_IVAR && rhs->tag()==EXPR_LVAR && raweq(rhs->lvar_name(), key)){
					put_inst(InstMakeInstanceVariableAccessor(1, lookup_instance_variable(lhs->ivar_name()), class_core_num()));
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
	ff().extendable_param = e->fun_extendable_param();


	// 変数フレームを作成して、引数を登録する
	var_begin(VarFrame::SCOPE);
	Xfor_as(const ExprPtr& v1, e->fun_params()){
		const ExprPtr& v = ep(v1->at(0));
		if(v->tag()==EXPR_LVAR){
			var_define(v->lvar_name());
		}
		else if(v->tag()==EXPR_IVAR){
			var_define(v->ivar_name());
		}
	}

	// 関数コアを作成
	FunCore core;
	core.pc = code_size() + InstMakeFun::ISIZE;
	core.kind = e->fun_kind();
	core.min_param_count = minv;
	core.max_param_count = maxv;
	core.flags = e->fun_extendable_param() ? FunCore::FLAG_EXTENDABLE_PARAM : 0;

	// 引数の名前を識別子テーブルに順番に乗せる
	core.variable_size = vf().entries.size();
	core.variable_identifier_offset = result_->identifier_table_->size();
	for(uint_t i=0; i<vf().entries.size(); ++i){
		result_->identifier_table_->push_back(vf().entries[i].name);
	}

	int_t fun_core_table_number = result_->xfun_core_table_.size();
	result_->xfun_core_table_.push_back(core);

	int_t fun_end_label = reserve_label();
	set_jump(InstMakeFun::OFFSET_address, fun_end_label);
	put_inst(InstMakeFun(fun_core_table_number, 0));


	// デフォルト値を持つ引数を処理する
	{
		int_t i = 0;
		Xfor_as(const ExprPtr& v1, e->fun_params()){
			if(const ExprPtr& v = ptr_as<Expr>(v1->at(1))){
				int_t label = reserve_label();
				
				set_jump(InstIfArgIsUndefined::OFFSET_address, label);
				var_set_direct(vf());
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
	Xfor_as(const ExprPtr& v1, e->fun_params()){
		const ExprPtr& v = ep(v1->at(0));
		if(v->tag()==EXPR_IVAR){
			eb_.push(v);
			eb_.push(v->ivar_name());
			eb_.splice(EXPR_LVAR, 1);
			eb_.splice(EXPR_ASSIGN, 2);
			compile_stmt(eb_.pop());
		}
	}

	add_stack_count(e->fun_params() ? e->fun_params()->size() : 0);
	add_stack_count(e->fun_params() ? -(int_t)e->fun_params()->size() : 0);

	// 関数本体を処理する
	compile_stmt(e->fun_body());

	XTAL_ASSERT(ff().stack_count==0);
	
	break_off(ff().var_frame_count+1);

	put_inst(InstReturn(0));
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
	var_begin(VarFrame::SCOPE);
	var_define_stmt(e->for_init());
	var_define(Xid(first_step), xnew<Expr>(EXPR_TRUE));
	check_lvar_assign_stmt(e);
	scope_begin();
	
	{
		LVarInfo info = var_find(Xid(first_step));
		info.entry->removed = true;
		info.entry->constant = true;
	}

	int_t label_cond = reserve_label();
	int_t label_if_q = reserve_label();
	int_t label_if2_q = reserve_label();
	int_t label_if = reserve_label();
	int_t label_if2 = reserve_label();
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
		put_if_code(e->for_cond(), label_if, label_if2);
	}

	set_label(label_body);

	// ループ本体をコンパイル
	compile_stmt(e->for_body());

	bool referenced_first_step;
	{
		LVarInfo info = var_find(Xid(first_step));
		info.entry->value = false;
		referenced_first_step = info.entry->referenced;
	}

	set_label(label_continue);

	// next部をコンパイル
	if(e->for_next()){
		compile_stmt(e->for_next());
	}

	set_label(label_cond);

	// 条件式をコンパイル 2回目
	if(e->for_cond()){
		put_if_code(e->for_cond(), label_if_q, label_if2_q);
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
	
	set_label(label_if);
	set_label(label_if2);
	if(e->for_else()){
		compile_stmt(e->for_else());
	}

	set_label(label_if_q);
	set_label(label_if2_q);
	if(e->for_nobreak()){
		compile_stmt(e->for_nobreak());
	}

	set_label(label_break);

	scope_end();
	var_end();
}

int_t CodeBuilder::compile_exprs(const ExprPtr& e){
	int_t count = 0;
	Xfor(v, e){
		compile_expr(v);
		count++;
	}
	return count;
}

AnyPtr CodeBuilder::compile_expr(const AnyPtr& p, const CompileInfo& info){
	if(!p){
		if(info.need_result_count==1){
			put_inst(InstPushNull());
			return null;
		}
		else if(info.need_result_count!=0){
			put_inst(InstAdjustResult(0, info.need_result_count));
		}

		return undefined;
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

		return val;
	}
	
	if(e->lineno()!=0){
		linenos_.push(e->lineno());
		result_->set_lineno_info(e->lineno());
	}

	int_t result_count = 1;
	switch(e->tag()){

		XTAL_NODEFAULT;

		XTAL_CASE(EXPR_UNDEFINED){ put_inst(InstPushUndefined()); }
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
			}
			else if(value==(i16)value){ 
				put_inst(InstPushInt2Byte(value));
			}
			else{ 
				put_inst(InstValue(register_value(value)));
			}
		}

		XTAL_CASE(EXPR_FLOAT){
			float_t value = e->float_value();
			if(value==(i8)value){ 
				put_inst(InstPushFloat1Byte((i8)value));
			}
			else if(value==(i16)value){ 
				put_inst(InstPushFloat2Byte((i16)value));
			}
			else{ 
				put_inst(InstValue(register_value(value)));
			}
		} 

		XTAL_CASE(EXPR_STRING){
			if(e->string_kind()==KIND_TEXT){
				put_inst(InstValue(register_value(text(e->string_value()->to_s()))));
			}
			else if(e->string_kind()==KIND_FORMAT){
				put_inst(InstValue(register_value(format(e->string_value()->to_s()))));
			}
			else{
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
			Xfor_as(const ArrayPtr& v, e->map_values()){
				compile_expr(v->at(0));
				compile_expr(v->at(1));
				put_inst(InstMapInsert());				
			}
		}

		XTAL_CASE(EXPR_MULTI_VALUE){
			if(e->multi_value_exprs()){
				Xfor(v, e->multi_value_exprs()){
					compile_expr(v);		
				}
				
				result_count = e->multi_value_exprs()->size();
			}
			else{
				result_count = 0;
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
		XTAL_CASE(EXPR_IN){ compile_comp_bin(e); }
		XTAL_CASE(EXPR_NIN){ compile_comp_bin(e); }
		XTAL_CASE(EXPR_IS){ compile_comp_bin(e); }
		XTAL_CASE(EXPR_NIS){ compile_comp_bin(e); }

		XTAL_CASE(EXPR_Q){
			int_t label_if = reserve_label();
			int_t label_end = reserve_label();

			compile_expr(e->q_cond());

			set_jump(InstIf::OFFSET_address, label_if);
			put_inst(InstIf());

			compile_expr(e->q_true());

			set_jump(InstGoto::OFFSET_address, label_end);
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
			
			set_jump(InstIf::OFFSET_address, label_if);
			put_inst(InstIf());

			put_inst(InstPop());
			
			compile_expr(e->bin_rhs());
			
			set_label(label_if);
		}

		XTAL_CASE(EXPR_OROR){
			int_t label_if = reserve_label();
			compile_expr(e->bin_lhs());

			put_inst(InstDup());
			
			set_jump(InstUnless::OFFSET_address, label_if);
			put_inst(InstUnless());
			
			put_inst(InstPop());
			
			compile_expr(e->bin_rhs());
			
			set_label(label_if);
		}

		XTAL_CASE(EXPR_RANGE){
			compile_expr(e->range_lhs());
			compile_expr(e->range_rhs());
			put_inst(InstRange((int_t)e->range_kind()));
		}

		XTAL_CASE(EXPR_POS){ compile_expr(e->una_term()); put_inst(InstPos()); }
		XTAL_CASE(EXPR_NEG){ compile_expr(e->una_term()); put_inst(InstNeg()); }
		XTAL_CASE(EXPR_COM){ compile_expr(e->una_term()); put_inst(InstCom()); }
		XTAL_CASE(EXPR_NOT){ compile_expr(e->una_term()); put_inst(InstNot()); }

		XTAL_CASE(EXPR_ONCE){
			int_t label_end = reserve_label();
			
			set_jump(InstOnce::OFFSET_address, label_end);
			int_t num = result_->once_table_->size();
			result_->once_table_->push_back(undefined);
			put_inst(InstOnce(0, num));
						
			compile_expr(e->una_term());
			put_inst(InstDup());		
			put_inst(InstSetOnce(num));
			
			set_label(label_end);	
		}

		XTAL_CASE(EXPR_SEND){
			compile_expr(e->send_term());
			put_send_code(e->send_name(), info.need_result_count, info.tail, false, e->send_ns());
			result_count = info.need_result_count;
		}

		XTAL_CASE(EXPR_SEND_Q){
			compile_expr(e->send_term());
			put_send_code(e->send_name(), info.need_result_count, info.tail, true, e->send_ns());
			result_count = info.need_result_count;
		}

		XTAL_CASE(EXPR_CALL){
			int_t ordered = 0;
			int_t named = 0;

			Xfor_as(const ExprPtr& v, e->call_args()){
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
			
			Xfor_as(const ExprPtr& v, e->call_args()){
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

			if(e->call_term()->tag()==EXPR_SEND){ // a.b(); メッセージ送信式

				ExprPtr e2 = e->call_term();
				compile_expr(e2->send_term());
				int_t key = register_identifier_or_compile_expr(e2->send_name());

				if(e2->send_ns()){
					compile_expr(e2->send_ns());
					put_inst(InstSendNS(ordered, named, info.need_result_count, flags, key));
				}
				else{
					put_inst(InstSend(ordered, named, info.need_result_count, flags, key));
				}
			}
			else if(e->call_term()->tag()==EXPR_SEND_Q){ // a.?b(); メッセージ送信式

				ExprPtr e2 = e->call_term();
				compile_expr(e2->send_term());
				int_t key = register_identifier_or_compile_expr(e2->send_name());

				if(e2->send_ns()){
					compile_expr(e2->send_ns());
					put_inst(InstSendQNS(ordered, named, info.need_result_count, flags, key));
				}
				else{
					put_inst(InstSendQ(ordered, named, info.need_result_count, flags, key));
				}
			}
			else{
				compile_expr(e->call_term());
				put_inst(InstCall(ordered, named, info.need_result_count, flags));
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
			int_t key = register_identifier_or_compile_expr(e->member_name());

			if(e->member_ns()){
				compile_expr(e->member_ns());
				put_inst(InstMemberNS(key));
			}
			else{
				put_inst(InstMember(key));
			}	
		}

		XTAL_CASE(EXPR_MEMBER_Q){
			compile_expr(e->member_term());
			int_t key = register_identifier_or_compile_expr(e->member_name());
			
			if(e->member_ns()){
				compile_expr(e->member_ns());
				put_inst(InstMemberQNS(key));
			}
			else{
				put_inst(InstMemberQ(key));
			}	
		}

		XTAL_CASE(EXPR_CLASS){
			compile_class(e);
		}

		XTAL_CASE(EXPR_BRACKET){
			compile_expr(e->una_term(), info.need_result_count);
			result_count = info.need_result_count;
		}

	}
	
	if(e->lineno()!=0){
		result_->set_lineno_info(e->lineno());
		linenos_.pop();
	}

	if(info.need_result_count!=result_count){
		put_inst(InstAdjustResult(result_count, info.need_result_count));
	}

	return undefined;
}

void CodeBuilder::compile_stmt(const AnyPtr& p){

	if(!p){
		return;
	}

	ExprPtr e = ep(p);

	if(e->lineno()!=0){
		linenos_.push(e->lineno());
		result_->set_lineno_info(e->lineno());
		if(linenos_[1]!=e->lineno()){
			put_inst(InstNop());
		}
	}

	switch(e->tag()){

		XTAL_DEFAULT{
			compile_expr(e, 0);
		}
		
		XTAL_CASE(EXPR_DEFINE){
			if(e->bin_lhs()->tag()==EXPR_LVAR){
				put_define_local_code(e->bin_lhs()->lvar_name(), e->bin_rhs());

			}
			else if(e->bin_lhs()->tag()==EXPR_MEMBER){
				compile_expr(e->bin_lhs()->member_term());
				compile_expr(e->bin_rhs());

				put_define_member_code(e->bin_lhs()->member_name(), e->bin_lhs()->member_ns());
			}
			else{
				error_->error(lineno(), Xt("Xtal Compile Error 1012"));
			}
		}
		
		XTAL_CASE(EXPR_ASSIGN){
			if(e->bin_lhs()->tag()==EXPR_LVAR){
				compile_expr(e->bin_rhs());
				put_set_local_code(e->bin_lhs()->lvar_name());
			}
			else if(e->bin_lhs()->tag()==EXPR_IVAR){
				compile_expr(e->bin_rhs());
				put_set_instance_variable_code(e->bin_lhs()->ivar_name());
			}
			else if(e->bin_lhs()->tag()==EXPR_SEND){
				compile_expr(e->bin_rhs());
				compile_expr(e->bin_lhs()->send_term());
				put_set_send_code(e->bin_lhs()->send_name(), false, e->bin_lhs()->send_ns());
			}
			else if(e->bin_lhs()->tag()==EXPR_SEND_Q){
				compile_expr(e->bin_rhs());
				compile_expr(e->bin_lhs()->send_term());
				put_set_send_code(e->bin_lhs()->send_name(), true, e->bin_lhs()->send_ns());
			}
			else if(e->bin_lhs()->tag()==EXPR_AT){
				compile_expr(e->bin_rhs());
				compile_expr(e->bin_lhs()->bin_lhs());
				compile_expr(e->bin_lhs()->bin_rhs());
				put_inst(InstSetAt());
			}
			else{
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
			int_t exprs_size = compile_exprs(e->yield_exprs());
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
				if(front->tag()==EXPR_CALL || front->tag()==EXPR_SEND || front->tag()==EXPR_SEND_Q){
					compile_expr(front, CompileInfo(1, true));
					break;
				}
			}

			compile_exprs(e->return_exprs());
			
			break_off(ff().var_frame_count+1);

			put_inst(InstReturn(exprs_size));
			if(exprs_size>=256){
				error_->error(lineno(), Xt("Xtal Compile Error 1022"));
			}	
		}

		XTAL_CASE(EXPR_ASSERT){		
			int_t exprs_size = compile_exprs(e->assert_exprs());
			if(exprs_size==1){
				put_inst(InstValue(0));
				put_inst(InstValue(0));	
			}
			else if(exprs_size==2){
				put_inst(InstValue(0));
			}
			else if(exprs_size==3){

			}
			else{
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
			
			set_jump(InstPushGoto::OFFSET_address, end_label);
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

				// 例外を受け取るために変数スコープを構築
				var_begin(VarFrame::SCOPE);
				var_define(e->try_catch_var(), null, 0, true, false, true);
				check_lvar_assign_stmt(e->try_catch());
				scope_begin();

				put_set_local_code(e->try_catch_var());
				compile_stmt(e->try_catch());
				
				scope_end();
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
			ExprPtr cond = e->if_cond();

			// 条件式の部分が変数定義式である場合
			if(cond->tag()==EXPR_DEFINE && cond->bin_lhs()->tag()==EXPR_LVAR){
				// スコープを形成する
				var_begin(VarFrame::SCOPE);
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
				int_t label_if = reserve_label();
				int_t label_if2 = reserve_label();
				int_t label_end = reserve_label();

				put_if_code(cond, label_if, label_if2);

				compile_stmt(e->if_body());
				
				if(e->if_else()){
					set_jump(InstGoto::OFFSET_address, label_end);
					put_inst(InstGoto());
				}
				
				set_label(label_if);
				set_label(label_if2);
				compile_stmt(e->if_else());
			
				set_label(label_end);
			}
	
			if(rawne(cond, e->if_cond())){
				var_end();
				scope_end();
			}
		}

		XTAL_CASE(EXPR_FOR){
			compile_for(e);
		}

		XTAL_CASE(EXPR_MASSIGN){
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
				if(v->tag()==EXPR_LVAR){
					put_set_local_code(v->lvar_name());
				}
				else if(v->tag()==EXPR_SEND){
					compile_expr(v->send_term());
					put_set_send_code(v->send_name(), false, v->send_ns());
				}
				else if(v->tag()==EXPR_SEND_Q){
					compile_expr(v->send_term());
					put_set_send_code(v->send_name(), true, v->send_ns());
				}
				else if(v->tag()==EXPR_IVAR){
					put_set_instance_variable_code(v->ivar_name());					
				}
				else if(v->tag()==EXPR_AT){
					compile_expr(v->bin_lhs());
					compile_expr(v->bin_rhs());
					put_inst(InstSetAt());
				}
				else{
					error_->error(lineno(), Xt("Xtal Compile Error 1008"));
				}
			}
		}

		XTAL_CASE(EXPR_MDEFINE){
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
				if(v->tag()==EXPR_LVAR){
					put_define_local_code(v->lvar_name());
				}
				else if(v->tag()==EXPR_MEMBER){
					compile_expr(v->member_term());
					put_define_member_code(v->member_name(), v->member_ns());
				}
				else{
					error_->error(lineno(), Xt("Xtal Compile Error 1008"));
				}
			}
		}

		XTAL_CASE2(EXPR_BREAK, EXPR_CONTINUE){
			compile_loop_control_statement(e);
		}	
		
		XTAL_CASE(EXPR_SCOPE){
			var_begin(VarFrame::SCOPE);
			var_define_stmts(e->scope_stmts());
			check_lvar_assign_stmt(e);
			scope_begin();{
				Xfor(v, e->scope_stmts()){
					compile_stmt(v);
				}
			}scope_end();
			var_end();
		}

		XTAL_CASE(EXPR_SWITCH){
			ExprPtr cond = e->switch_cond();

			// 条件式の部分が変数定義式である場合
			if(cond->tag()==EXPR_DEFINE && cond->bin_lhs()->tag()==EXPR_LVAR){
				// スコープを形成する
				var_begin(VarFrame::SCOPE);
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
			ExprPtr default_case;
			Xfor_as(const ExprPtr& v, e->switch_cases()){
				if(v->tag()==EXPR_SWITCH_CASE){
					Xfor_as(const ExprPtr& k, v->at(0)){
						case_map->set_at(k, v->at(1));
					}
				}
				else if(v->tag()==EXPR_SWITCH_DEFAULT){
					default_case = ep(v->at(0));
				}
			}

			MapPtr jump_map = xnew<Map>();
			Xfor2(k, v, case_map){
				int_t jump_to = reserve_label();
				jump_map->set_at(k, jump_to);
			}

			put_inst(InstMakeMap());
			Xfor2(k, v, case_map){
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
				var_end();
				scope_end();
			}
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

#define XTAL_CB_DO_EXPR(name, expr) AnyPtr name = do_expr(expr); if(raweq(name, undefined)) return undefined;

AnyPtr CodeBuilder::do_bin(const ExprPtr& e, const IDPtr& name, bool swap){
	if(is_comp_bin(e->bin_lhs()) || is_comp_bin(e->bin_rhs())){ 
		error_->error(lineno(), Xt("Xtal Compile Error 1025")); 
	}

	XTAL_CB_DO_EXPR(lhs, e->bin_lhs());
	XTAL_CB_DO_EXPR(rhs, e->bin_rhs());

	if(swap){
		return do_send(rhs, name, lhs);
	}
	else{
		return do_send(lhs, name, rhs);
	}
}
	
AnyPtr CodeBuilder::do_send(const AnyPtr& a, const IDPtr& name){
	AnyPtr ret = undefined;
	XTAL_TRY{
		VMachinePtr vm = vmachine();
		vm->setup_call();
		a->rawsend(vm, name, null, null, false);
		if(!vm->processed()){ vm->return_result(undefined); }
		ret = vm->result_and_cleanup_call();
	}
	XTAL_CATCH(e){
		(void)e;
		ret = undefined;
	}
	return ret;
}
	
AnyPtr CodeBuilder::do_send(const AnyPtr& a, const IDPtr& name, const AnyPtr& b){
	AnyPtr ret = undefined;
	XTAL_TRY{
		VMachinePtr vm = vmachine();
		vm->setup_call(1, b);
		a->rawsend(vm, name, b->get_class(), null, false);
		if(!vm->processed()){ vm->return_result(undefined); }
		ret = vm->result_and_cleanup_call();
		if(ret->is(get_cpp_class<Int>()) || ret->is(get_cpp_class<Float>()) || ret->is(get_cpp_class<String>())
			|| ret->is(get_cpp_class<Array>()) || ret->is(get_cpp_class<Map>())){
			return ret;
		}

		return undefined;
	}
	XTAL_CATCH(e){
		(void)e;
		ret = undefined;
	}
	return ret;
}

AnyPtr CodeBuilder::do_not(const AnyPtr& v){
	if(raweq(v, undefined)) return undefined;
	return !v;
}

AnyPtr CodeBuilder::do_expr(const AnyPtr& p){

	if(!p){
		return null;
	}

	ExprPtr e = ep(p);

	switch(e->tag()){

		XTAL_NODEFAULT;

		XTAL_CASE(EXPR_NULL){ return null; }
		XTAL_CASE(EXPR_UNDEFINED){ return undefined; }
		XTAL_CASE(EXPR_TRUE){ return true; }
		XTAL_CASE(EXPR_FALSE){ return false; }
		XTAL_CASE(EXPR_THIS){ return undefined; }
		XTAL_CASE(EXPR_CURRENT_CONTEXT){ return undefined; }
		XTAL_CASE(EXPR_CALLEE){ return undefined; }
		XTAL_CASE(EXPR_ARGS){ return undefined; }
		XTAL_CASE(EXPR_INT){ return e->int_value(); }
		XTAL_CASE(EXPR_FLOAT){ return e->float_value(); }
		XTAL_CASE(EXPR_STRING){
			if(e->string_kind()==KIND_TEXT){
				return undefined;
			}
			else if(e->string_kind()==KIND_FORMAT){
				return undefined;
			}
			else{
				return e->string_value();
			}
		}

		XTAL_CASE(EXPR_ARRAY){
			return undefined;
		}

		XTAL_CASE(EXPR_MAP){
			return undefined;
		}

		XTAL_CASE(EXPR_MULTI_VALUE){
			return undefined;
		}

		XTAL_CASE(EXPR_ADD){ return do_bin(e, Xid(op_add)); }
		XTAL_CASE(EXPR_SUB){ return do_bin(e, Xid(op_sub)); }
		XTAL_CASE(EXPR_CAT){ return do_bin(e, Xid(op_cat)); }
		XTAL_CASE(EXPR_MUL){ return do_bin(e, Xid(op_mul)); }
		XTAL_CASE(EXPR_DIV){ return do_bin(e, Xid(op_div)); }
		XTAL_CASE(EXPR_MOD){ return do_bin(e, Xid(op_mod)); }
		XTAL_CASE(EXPR_OR){ return do_bin(e, Xid(op_or)); }
		XTAL_CASE(EXPR_AND){ return do_bin(e, Xid(op_and)); }
		XTAL_CASE(EXPR_XOR){ return do_bin(e, Xid(op_xor)); }
		XTAL_CASE(EXPR_SHR){ return do_bin(e, Xid(op_shr)); }
		XTAL_CASE(EXPR_SHL){ return do_bin(e, Xid(op_shl)); }
		XTAL_CASE(EXPR_USHR){ return do_bin(e, Xid(op_ushr)); }

		XTAL_CASE(EXPR_EQ){ return do_bin(e, Xid(op_eq)); }
		XTAL_CASE(EXPR_NE){ return do_not(do_bin(e, Xid(op_eq))); }
		XTAL_CASE(EXPR_LT){ return do_bin(e, Xid(op_lt)); }
		XTAL_CASE(EXPR_GT){ return do_bin(e, Xid(op_lt), true); }
		XTAL_CASE(EXPR_LE){ return do_not(do_bin(e, Xid(op_lt), true)); }
		XTAL_CASE(EXPR_GE){ return do_not(do_bin(e, Xid(op_lt))); }

		XTAL_CASE(EXPR_RAWEQ){
			XTAL_CB_DO_EXPR(lhs, e->bin_lhs());
			XTAL_CB_DO_EXPR(rhs, e->bin_rhs());
			return raweq(lhs, rhs);
		}

		XTAL_CASE(EXPR_RAWNE){
			XTAL_CB_DO_EXPR(lhs, e->bin_lhs());
			XTAL_CB_DO_EXPR(rhs, e->bin_rhs());
			return rawne(lhs, rhs);
		}

		XTAL_CASE(EXPR_IN){
			return undefined;
		}

		XTAL_CASE(EXPR_NIN){
			return undefined;
		}

		XTAL_CASE(EXPR_IS){
			XTAL_CB_DO_EXPR(lhs, e->bin_lhs());
			XTAL_CB_DO_EXPR(rhs, e->bin_rhs());
			if(ClassPtr cp = ptr_as<Class>(rhs)){
				return lhs->is(cp);
			}
			else{
				return undefined;
			}
		}

		XTAL_CASE(EXPR_NIS){
			XTAL_CB_DO_EXPR(lhs, e->bin_lhs());
			XTAL_CB_DO_EXPR(rhs, e->bin_rhs());
			if(ClassPtr cp = ptr_as<Class>(rhs)){
				return !lhs->is(cp);
			}
			else{
				return undefined;
			}
		}

		XTAL_CASE(EXPR_Q){
			XTAL_CB_DO_EXPR(cond, e->q_cond());
			if(cond){
				XTAL_CB_DO_EXPR(qtrue, e->q_true());
				return qtrue;
			}
			else{
				XTAL_CB_DO_EXPR(qfalse, e->q_false());
				return qfalse;
			}
		}
		
		XTAL_CASE(EXPR_AT){
			XTAL_CB_DO_EXPR(lhs, e->bin_lhs());
			XTAL_CB_DO_EXPR(rhs, e->bin_rhs());
			return do_send(lhs, Xid(op_at), rhs);
		}

		XTAL_CASE(EXPR_ANDAND){
			XTAL_CB_DO_EXPR(lhs, e->bin_lhs());
			if(lhs){
				XTAL_CB_DO_EXPR(rhs, e->bin_rhs());
				return rhs;
			}
			return lhs;
		}

		XTAL_CASE(EXPR_OROR){
			XTAL_CB_DO_EXPR(lhs, e->bin_lhs());
			if(lhs){ return lhs; }
			XTAL_CB_DO_EXPR(rhs, e->bin_rhs());
			return rhs;
		}

		XTAL_CASE(EXPR_RANGE){ return undefined; }

		XTAL_CASE(EXPR_POS){
			XTAL_CB_DO_EXPR(term, e->una_term());
			return do_send(term, Xid(op_pos));
		}

		XTAL_CASE(EXPR_NEG){
			XTAL_CB_DO_EXPR(term, e->una_term());
			return do_send(term, Xid(op_neg));
		}

		XTAL_CASE(EXPR_COM){
			XTAL_CB_DO_EXPR(term, e->una_term());
			return do_send(term, Xid(op_com));
		}

		XTAL_CASE(EXPR_NOT){
			XTAL_CB_DO_EXPR(term, e->una_term());
			return !term;
		}

		XTAL_CASE(EXPR_ONCE){
			return undefined;
		}

		XTAL_CASE(EXPR_SEND){
			return undefined;
		}

		XTAL_CASE(EXPR_SEND_Q){
			return undefined;
		}

		XTAL_CASE(EXPR_CALL){
			return undefined;
		}

		XTAL_CASE(EXPR_FUN){
			return undefined;
		}

		XTAL_CASE(EXPR_LVAR){
			LVarInfo info = var_find(e->lvar_name(), false, true);
			if(info.pos>=0){
				if((info.var_frame->fun_frames_size==fun_frames_.size() || !info.entry->assigned)){
					info.entry->referenced = true;
					return info.entry->value;
				}
			}
			return undefined;
		}

		XTAL_CASE(EXPR_IVAR){
			return undefined;
		}

		XTAL_CASE(EXPR_MEMBER){
			XTAL_CB_DO_EXPR(term, e->member_term());
			XTAL_CB_DO_EXPR(ns, e->member_ns());
			XTAL_TRY{
				if(ptr_as<Expr>(e->member_name())){
					return undefined;
				}
				else{
					return term->member(ptr_as<ID>(e->member_name()), ns, null, false);
				}
			}
			XTAL_CATCH(e){
				(void)e;
				return undefined;
			}
		}		
		
		XTAL_CASE(EXPR_MEMBER_Q){
			return undefined;
		}

		XTAL_CASE(EXPR_CLASS){
			return undefined;
		}

		XTAL_CASE(EXPR_BRACKET){
			return undefined;
		}
	}

	return undefined;
}

void CodeBuilder::check_lvar_assign(const ExprPtr& e){
	if(e->tag()==EXPR_LVAR){
		LVarInfo info = var_find(e->lvar_name(), true, true);
		if(info.pos>=0){
			info.entry->assigned = true;
			info.entry->value = undefined;
		}
	}
}

void CodeBuilder::check_lvar_assign_stmt(const AnyPtr& p){
	if(!p){
		return;
	}

	ExprPtr e = ep(p);

	switch(e->tag()){
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
			if(v){ check_lvar_assign(ep(v)); }
		}
		break;

	case EXPR_TRY:
		check_lvar_assign_stmt(e->try_body());
		check_lvar_assign_stmt(e->try_catch());
		check_lvar_assign_stmt(e->try_finally());
		break;

	
	case EXPR_IF:
		check_lvar_assign_stmt(e->if_body());
		check_lvar_assign_stmt(e->if_else());
		break;

	case EXPR_FOR:
		check_lvar_assign_stmt(e->for_init());
		check_lvar_assign_stmt(e->for_body());
		check_lvar_assign_stmt(e->for_else());
		check_lvar_assign_stmt(e->for_next());
		check_lvar_assign_stmt(e->for_nobreak());
		break;
		
	case EXPR_SCOPE:
		Xfor(v, e->scope_stmts()){
			check_lvar_assign_stmt(v);
		}
		break;

	case EXPR_TOPLEVEL:
		Xfor(v, e->toplevel_stmts()){
			check_lvar_assign_stmt(v);
		}
		break;
	}
}

}

#endif

