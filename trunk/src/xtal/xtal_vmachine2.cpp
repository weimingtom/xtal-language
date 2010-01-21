#include "xtal.h"
#include "xtal_macro.h"

namespace xtal{

VMachine::VMachine(){
	id_ = id_op_list();

	end_code_ = InstExit::NUMBER;
	throw_code_ = InstThrow::NUMBER;
	resume_pc_ = 0;

	variables_top_ = 0;
	variables_.resize(128);
	set_variables_top(0);
	stack_.reserve(32);

	setup_call();
	fun_frames_.top()->processed = 0;

	parent_vm_ = 0;

	thread_yield_count_ = 1000;

	static uint_t dummy = 0;
	hook_setting_bit_ = &dummy;
}

VMachine::~VMachine(){
	variables_.clear_unref();
	fun_frames_.resize(fun_frames_.capacity());
	for(int_t i=0, size=fun_frames_.size(); i<size; ++i){
		if(fun_frames_[i]){
			fun_frames_[i]->~FunFrame();
			xfree(fun_frames_[i], sizeof(FunFrame));
		}
	}

	while(scopes_.size()){
		pop_scope();
	}
}

void VMachine::reset(){
	stack_.resize(0);
	except_frames_.resize(0);
	fun_frames_.resize(0);

	setup_call();
	fun_frames_.top()->processed = 0;
	
	while(scopes_.size()){
		pop_scope();
	}

	set_variables_top(0);

	except_[0] = null;
	except_[1] = null;
	except_[2] = null;
	debug_info_ = null;

	parent_vm_ = 0;

	for(int_t i=0, size=(int)variables_.size(); i<size; ++i){
		(Any&)variables_.at(i) = null;
	}
}

VMachine::FunFrame::FunFrame(){
}

void VMachine::FunFrame::set_fun(const MethodPtr& v){ 
	fun_ = v; outer_ = v->outer();  identifier_ = &fun()->code()->identifier(0); 
}

void VMachine::FunFrame::set_null(){
	set_fun();
	set_self();
}

void VMachine::push_arg(const AnyPtr& value){
	XTAL_ASSERT(ff().named_arg_count==0); //順番指定引数は 名前指定引数の前だけ

	set_local_variable(ff().ordered_arg_count, value);
	ff().ordered_arg_count++;
}
	
void VMachine::push_arg(const IDPtr& name, const AnyPtr& value){
	XTAL_ASSERT(ff().ordered_arg_count==0); // 名前指定引数は順番指定引数の後だけ

	int_t offset = ff().ordered_arg_count + ff().named_arg_count*2;
	set_local_variable(offset+0, name);
	set_local_variable(offset+1, value);
	ff().named_arg_count++;
}

void VMachine::push_ordered_args(const ArrayPtr& p){ 
	Xfor(v, p){
		push_arg(v);
	}
}

void VMachine::push_named_args(const MapPtr& p){ 
	Xfor2(k, v, p){
		push_arg(ptr_cast<ID>(k), v);
	}
}

void VMachine::push_arg(const char_t* s){ push_arg(AnyPtr(s)); }
void VMachine::push_arg(const char8_t* s){ push_arg(AnyPtr(s)); }
void VMachine::push_arg(const StringLiteral& s){ push_arg(AnyPtr(s)); }
void VMachine::push_arg(char v){ push_arg(ap(Any(v))); }
void VMachine::push_arg(signed char v){ push_arg(ap(Any(v))); }
void VMachine::push_arg(unsigned char v){ push_arg(ap(Any(v))); }
void VMachine::push_arg(short v){ push_arg(ap(Any(v))); }
void VMachine::push_arg(unsigned short v){ push_arg(ap(Any(v))); }
void VMachine::push_arg(int v){ push_arg(ap(Any(v))); }
void VMachine::push_arg(unsigned int v){ push_arg(ap(Any(v))); }
void VMachine::push_arg(long v){ push_arg(ap(Any(v))); }
void VMachine::push_arg(unsigned long v){ push_arg(ap(Any(v))); }
void VMachine::push_arg(long long v){ push_arg(ap(Any(v))); }
void VMachine::push_arg(unsigned long long v){ push_arg(ap(Any(v))); }
void VMachine::push_arg(float v){ push_arg(ap(Any(v))); }
void VMachine::push_arg(double v){ push_arg(ap(Any(v))); }
void VMachine::push_arg(long double v){ push_arg(ap(Any(v))); }
void VMachine::push_arg(bool v){ push_arg(ap(Any(v))); }

void VMachine::setup_call(int_t need_result_count){
	int_t base;
	if(!fun_frames_.empty()){
		FunFrame& f= ff();
		if(f.fun()){
			base = 128;
		}
		else{
			base = f.ordered_arg_count+f.named_arg_count*2+1;
		}
	}
	else{
		base = 0;
	}

	CallState call_state;
	call_state.self = undefined;
	call_state.npc = &end_code_;
	call_state.result = base;
	call_state.need_result_count = need_result_count;
	call_state.stack_base = base;
	call_state.ordered = 0;
	call_state.named = 0;
	push_ff(call_state);	
}

void VMachine::recycle_call(){
	FunFrame& f = ff();
	f.ordered_arg_count = 0;
	f.named_arg_count = 0;
	f.called_pc = 0;
}

void VMachine::recycle_call(const AnyPtr& a1){
	recycle_call();
	push_arg(a1);
}
	
void VMachine::execute(Method* fun, const inst_t* start_pc){
	setup_call(0);
	carry_over(fun);
	const inst_t* temp;

	{
		FunFrame& f = ff();

		temp = f.poped_pc;
		f.poped_pc = &end_code_;

		execute_inner(start_pc ? start_pc : f.called_pc);
	}
}

void VMachine::execute(){
	FunFrame& f = ff();

	// 既に実行されている
	if(f.processed==2){
		return;
	}

	// 関数も戻り値も設定されていない
	if(f.processed==0){
		return;
	}

	// 例外がクリアされていないなら実行できない
	if(ap(except_[0])){
		return;
	}

	const inst_t* pc = f.called_pc;
	f.called_pc = 0;

	execute_inner(pc);
}
	
const AnyPtr& VMachine::result(int_t pos){
	execute();
	return local_variable(result_base_+pos);
}
	
AnyPtr VMachine::result_and_cleanup_call(int_t pos){
	execute();
	ff().processed = 0;
	return local_variable(result_base_+pos);
}
	
void VMachine::cleanup_call(){
	execute();
	ff().processed = 0;
}

const AnyPtr& VMachine::arg(int_t pos){
	FunFrame& f = ff();
	if(pos<f.ordered_arg_count)
		return local_variable(pos);
	return undefined;
}

const AnyPtr& VMachine::arg(int_t pos, const IDPtr& name){
	FunFrame& f = ff();
	if(pos<f.ordered_arg_count){
		return local_variable(pos);
	}
	return arg(name);
}

const AnyPtr& VMachine::arg_default(int_t pos, const AnyPtr& def){
	FunFrame& f = ff();
	if(pos<f.ordered_arg_count){
		return local_variable(pos);
	}
	return def;
}
	
const AnyPtr& VMachine::arg_default(const IDPtr& name, const AnyPtr& def){
	FunFrame& f = ff();
	for(int_t i = 0, sz = f.named_arg_count; i<sz; ++i){
		if(raweq(local_variable(f.ordered_arg_count + i*2 + 0), name)){
			return local_variable(f.ordered_arg_count + i*2 + 1);
		}
	}
	return def;
}
	
const AnyPtr& VMachine::arg_default(int_t pos, const IDPtr& name, const AnyPtr& def){
	FunFrame& f = ff();
	if(pos<f.ordered_arg_count){
		return local_variable(pos);
	}
	return arg_default(name, def);
}

void VMachine::adjust_args(const NamedParam* params, int_t num){
	FunFrame& f = ff();
	int_t base = f.ordered_arg_count + f.named_arg_count*2;
	int_t offset = base;
	for(int_t j=f.ordered_arg_count; j<num; ++j){
		bool hit = false;
		for(int_t i = 0, sz = f.named_arg_count; i<sz; ++i){
			if(raweq(local_variable(f.ordered_arg_count + i*2 + 0), params[j].name)){
				set_local_variable(offset++, local_variable(f.ordered_arg_count + i*2 + 1));
				hit = true;
				break;
			}
		}

		if(!hit){
			set_local_variable(offset++, params[j].value);
		}
	}

	variables_.move_unref(variables_top() + f.ordered_arg_count, base + variables_top(), offset-base);
}

void VMachine::adjust_args(Method* names, int_t num){
	FunFrame& f = ff();
	int_t base = f.ordered_arg_count + f.named_arg_count*2;
	int_t offset = base;
	for(int_t j=f.ordered_arg_count; j<num; ++j){
		bool hit = false;
		for(int_t i = 0, sz = f.named_arg_count; i<sz; ++i){
			if(raweq(local_variable(f.ordered_arg_count + i*2 + 0), names->param_name_at(j))){
				set_local_variable(offset++, local_variable(f.ordered_arg_count + i*2 + 1));
				hit = true;
				break;
			}
		}

		if(!hit){
			set_local_variable(offset++, undefined);
		}
	}

	variables_.move_unref(variables_top() + f.ordered_arg_count, base + variables_top(), offset-base);
}

void VMachine::return_result(){
	XTAL_ASSERT(!processed());
	FunFrame& f = ff();

	f.result_count += 0;
	pop_ff();
}

void VMachine::return_result(const AnyPtr& value1){
	XTAL_ASSERT(!processed());
	FunFrame& f = ff();

	stack_.push(value1);
	f.result_count += 1;
	pop_ff();
}
	
void VMachine::return_result(const AnyPtr& value1, const AnyPtr& value2){
	XTAL_ASSERT(!processed());
	FunFrame& f = ff();

	stack_.push(value1);
	stack_.push(value2);
	f.result_count += 2;
	pop_ff();
}

void VMachine::return_result(const AnyPtr& value1, const AnyPtr& value2, const AnyPtr& value3){
	XTAL_ASSERT(!processed());
	FunFrame& f = ff();

	stack_.push(value1);
	stack_.push(value2);
	stack_.push(value3);
	f.result_count += 3;
	pop_ff();
}
	
void VMachine::return_result(const AnyPtr& value1, const AnyPtr& value2, const AnyPtr& value3, const AnyPtr& value4){
	XTAL_ASSERT(!processed());
	FunFrame& f = ff();

	stack_.push(value1);
	stack_.push(value2);
	stack_.push(value3);
	stack_.push(value4);
	f.result_count += 4;
	pop_ff();
}

void VMachine::return_result_mv(const ValuesPtr& values){
	XTAL_ASSERT(!processed());
	FunFrame& f = ff();

	int_t size = values->size();
	for(int_t i=0; i<size; ++i){
		stack_.push(values->at(i));
	}

	f.result_count += size;
	pop_ff();
}

void VMachine::prereturn_result(const AnyPtr& v){
	ff().result_count++;
	stack_.push(v);
}

void VMachine::present_for_vm(Fiber* fun, VMachine* vm, bool add_succ_or_fail_result){
	if(const AnyPtr& e = catch_except()){
		vm->set_except_x(e);
		vm->stack_.push(e);
		vm->fun_frames_.top()->called_pc = &throw_code_;
		resume_pc_ = 0;
		return;
	}

	// 結果をvmに渡す
	if(add_succ_or_fail_result){
		if(resume_pc_!=0){
			vm->stack_.push(to_smartptr(fun));
		}
		else{
			vm->stack_.push(null);
		}
		vm->ff().result_count += 1;
	}

	for(int_t i=0; i<yield_target_count_; ++i){
		vm->stack_.push(stack_[yield_target_count_-1-i]);
	}
	stack_.downsize(yield_target_count_);
	
	vm->ff().result_count += yield_target_count_;

	vm->pop_ff();
}

const inst_t* VMachine::start_fiber(Fiber* fun, VMachine* vm, bool add_succ_or_fail_result){
	yield_target_count_ = 0;

	CallState call_state;
	call_state.self = vm->arg_this();
	call_state.npc = &end_code_;
	call_state.result = variables_top();
	call_state.need_result_count = vm->need_result_count();
	call_state.stack_base = 0;
	call_state.ordered = vm->ordered_arg_count();
	call_state.named = vm->named_arg_count();
	push_ff(call_state);
	
	move_variables(vm, vm->ordered_arg_count()+vm->named_arg_count()*2);

	resume_pc_ = 0;

	carry_over(fun);
	ff().yieldable = true;

	parent_vm_ = vm;
	execute_inner(ff().called_pc);
	parent_vm_ = 0;
	//XTAL_ASSERT(resume_pc_);
	present_for_vm(fun, vm, add_succ_or_fail_result);
	return resume_pc_;
}

const inst_t* VMachine::resume_fiber(Fiber* fun, const inst_t* pc, VMachine* vm, bool add_succ_or_fail_result){
	yield_target_count_ = 0;
	ff().called_pc = pc;
	resume_pc_ = 0;

	if(yield_need_result_count_!=0){
		vm->adjust_values2(0, vm->ordered_arg_count()+vm->named_arg_count()*2, yield_need_result_count_);

		for(int_t i=0; i<yield_need_result_count_; ++i){
			set_local_variable(yield_result_+i, vm->local_variable(i));
		}
	}

	parent_vm_ = vm;
	execute_inner(ff().called_pc);
	parent_vm_ = 0;
	present_for_vm(fun, vm, add_succ_or_fail_result);
	return resume_pc_;
}

void VMachine::exit_fiber(){
	yield_target_count_ = 0;
	ff().called_pc = &throw_code_;
	resume_pc_ = 0;

	stack_.push(undefined);
	execute_inner(&throw_code_);

	reset();
}

void VMachine::flatten_args(){
	XTAL_ASSERT(false);
	/*todo
	FunFrame& f = ff();

	stack_.downsize(f.named_arg_count*2);
	adjust_values(variables_top_, f.ordered_arg_count, 1);
	f.named_arg_count = 0;
	f.ordered_arg_count = 1;

	AnyPtr a = arg(0);
	if(type(a)==TYPE_UNDEFINED){
		f.ordered_arg_count = 0;
	}
	else if(type(a)==TYPE_VALUES){
		int_t size = values->size();
		for(int_t i=0; i<size; ++i){
			stack_.push(values->at(i));
		}
		f.ordered_arg_count = push_mv(unchecked_ptr_cast<Values>(a));
	}
	*/
}

ArgumentsPtr VMachine::make_arguments(){
	ArgumentsPtr p = xnew<Arguments>();

	for(int_t i = 0, size = ordered_arg_count(); i<size; ++i){
		p->add_ordered(arg(i));
	}

	for(int_t i = 0, size = named_arg_count(); i<size; ++i){
		p->add_named(local_variable(ff().ordered_arg_count+i*2+0), local_variable(ff().ordered_arg_count+i*2+1));
	}
	return p;
}

ArgumentsPtr VMachine::inner_make_arguments(Method* fun){
	ArgumentsPtr p = xnew<Arguments>();

	for(int_t i = fun->param_size(), size = ff().ordered_arg_count; i<size; ++i){
		p->add_ordered(local_variable(i));
	}

	IDPtr name;
	for(int_t i = 0, size = ff().named_arg_count; i<size; ++i){
		name = (IDPtr&)local_variable(ff().ordered_arg_count+i*2+0);
		for(int_t j = 0; j<fun->param_size(); ++j){
			if(fun->param_name_at(j)==name){
				name = null;
				break;
			}
		}
		if(name){
			p->add_named(name, local_variable(ff().ordered_arg_count+i*2+1));
		}
	}

	return p;
}

AnyPtr VMachine::append_backtrace(const inst_t* pc, const AnyPtr& e){
	if(e){
		AnyPtr ep = e;
		if(!ep->is(cpp_class<Exception>())){
			ep = cpp_class<RuntimeError>()->call(ep);
		}

		if(fun() && fun()->code()){
			if((pc !=  fun()->code()->data() + fun()->code()->size()-1)){
				unchecked_ptr_cast<Exception>(ep)->append_backtrace(
					 fun()->code()->source_file_name(),
					 fun()->code()->compliant_lineno(pc),
					fun()->object_name());
			}
		}
		else{
			/*ep->send(Xid(append_backtrace))(
				"C++ function",
				-1,
				ap(ff().temp1_)->object_name()
			);*/
		}
		return ep;
	}
	return e;
}

void VMachine::set_except_0(const Any& e){
	except_[0] = e;
}
	
void VMachine::make_debug_info(const inst_t* pc, int_t kind){
	if(!debug_info_){ 
		debug_info_ = xnew<debug::HookInfo>(); 
	}

	debug_info_->set_kind(kind);
	if(fun()){
		debug_info_->set_line(fun()->code()->compliant_lineno(pc));
		debug_info_->set_file_name(fun()->code()->source_file_name());
		debug_info_->set_fun_name(fun()->object_name());
	}
	else{
		debug_info_->set_line(0);
		debug_info_->set_file_name("?");
		debug_info_->set_fun_name("?");
	}

	debug_info_->set_exception(ap(except_[0]));

	make_outer_outer(0, 0, true);
	debug_info_->set_variables_frame(scopes_.top().frame);

	debug_info_->set_vm(to_smartptr(this));
}

debug::CallerInfoPtr VMachine::caller(uint_t n){
	if((int_t)n>call_stack_size()){
		return nul<debug::CallerInfo>();
	}

	if(n>=fun_frames_.size()-1){
		if(parent_vm_){
			return parent_vm_->caller(n-(fun_frames_.size()-1));
		}
		return nul<debug::CallerInfo>();
	}

	FunFrame& f = *fun_frames_[n];
	FunFrame& pf = *fun_frames_[n+1];
	int_t scope_lower = n==0 ? 0 : fun_frames_[n-1]->scope_lower;

	debug::CallerInfoPtr ret = xnew<debug::CallerInfo>();

	if(!f.fun()){
		ret->set_line(0);
		ret->set_fun(nul<Fun>());
		return ret;
	}

	ret->set_line(f.fun()->code()->compliant_lineno(f.poped_pc));
	ret->set_fun(f.fun());

	make_outer_outer(0, 0, true);
	ret->set_variables_frame(scopes_.reverse_at(scope_lower-1).frame);
	return ret;
}

int_t VMachine::call_stack_size(){
	int_t n = parent_vm_ ? parent_vm_->call_stack_size()-1 : -1; 
	return n + fun_frames_.size()-1;
}

AnyPtr VMachine::eval_local_variable(const IDPtr& var, uint_t call_n){
	if(call_n<fun_frames_.size()-1){
		int_t scope_upper = fun_frames_[call_n-1]->scope_lower;
		int_t scope_lower = fun_frames_[call_n]->scope_lower;

		FramePtr frame;
		for(int_t i=0; i<scope_upper - scope_lower; ++i){
			frame = scopes_.reverse_at(scope_upper-i-1).frame;
			const AnyPtr& ret = frame->member(var);
			if(rawne(ret, undefined)){
				return ret;
			}
		}

		frame = fun_frames_[call_n]->outer();
		while(frame){
			const AnyPtr& ret = frame->member(var);
			if(rawne(ret, undefined)){
				return ret;
			}

			frame = frame->outer();
		}

		if(fun_frames_[call_n]->fun()){
			const AnyPtr& ret = fun_frames_[call_n]->code()->member(var);
			if(rawne(ret, undefined)){
				return ret;
			}
		}
	}
	else{
		if(parent_vm_){
			return parent_vm_->eval_local_variable(var, call_n-(fun_frames_.size()-1));
		}
	}
		
	return undefined;
}

bool VMachine::eval_set_local_variable(const IDPtr& var, const AnyPtr& value, uint_t call_n){
	if(call_n<fun_frames_.size()-1){

		int_t scope_upper = fun_frames_[call_n-1]->scope_lower;
		int_t scope_lower = fun_frames_[call_n]->scope_lower;

		FramePtr frame;
		for(int_t i=0; i<scope_upper - scope_lower; ++i){
			frame = scopes_.reverse_at(scope_upper-i-1).frame;

			if(frame->replace_member(var, value)){
				return true;
			}
		}

		frame = fun_frames_[call_n]->outer();
		while(frame){
			if(frame->replace_member(var, value)){
				return true;
			}

			frame = frame->outer();
		}

		if(fun_frames_[call_n]->fun()){
			if(fun_frames_[call_n]->code()->replace_member(var, value)){
				return true;
			}
		}
	}
	else{
		if(parent_vm_){
			return parent_vm_->eval_set_local_variable(var, value, call_n-(fun_frames_.size()-1));
		}
	}
		
	return false;
}

AnyPtr VMachine::eval_instance_variable(const AnyPtr& self, const IDPtr& key){
	if(type(self)!=TYPE_BASE){
		return undefined;
	}

	ClassPtr klass = self->get_class();
	ArrayPtr ary = klass->ancestors()->to_a();
	ary->push_back(klass);

	Xfor_cast(const ClassPtr& p, ary){
		if(InstanceVariables* iv = pvalue(self)->instance_variables()){
			if(const CodePtr& code = p->code()){
				ClassInfo* info = p->info();
				for(uint_t i=0; i<info->instance_variable_size; ++i){
					if(raweq(key, code->identifier(info->instance_variable_identifier_offset+i))){
						return iv->variable(i, info);
					}
				}
			}
		}
	}
	return undefined;
}

bool VMachine::eval_set_instance_variable(const AnyPtr& self, const IDPtr& key, const AnyPtr& value){
	if(type(self)!=TYPE_BASE){
		return false;
	}

	ClassPtr klass = self->get_class();
	ArrayPtr ary = klass->ancestors()->to_a();
	ary->push_back(klass);

	Xfor_cast(const ClassPtr& p, ary){
		if(InstanceVariables* iv = pvalue(self)->instance_variables()){
			if(const CodePtr& code = p->code()){
				ClassInfo* info = p->info();
				for(uint_t i=0; i<info->instance_variable_size; ++i){
					if(raweq(key, code->identifier(info->instance_variable_identifier_offset+i))){
						iv->set_variable(i, info, value);
						return true;
					}
				}
			}
		}
	}
	return false;
}

void VMachine::debug_hook(const inst_t* pc, int_t kind){
	{
		struct guard{
			int_t count;
			guard(){ count = debug::disable_force(); }
			~guard(){ debug::enable_force(count); }
		} g;

		make_debug_info(pc, kind);

		// 現在発生している例外を退避させる
		AnyPtr e = ap(except_[0]);
		except_[0] = null;

		debug::call_debug_hook(kind, debug_info_);

		except_[0] = debug_info_->exception();
	}
}

const inst_t* VMachine::catch_body(const inst_t* pc, const ExceptFrame& nef){
	AnyPtr e = catch_except();

	ExceptFrame ef;
	if(except_frames_.empty() || except_frames_.top().fun_frame_size<nef.fun_frame_size){
		ef = nef;
	}
	else{
		ef = except_frames_.top();
	}

	// Xtalの関数を脱出していく
	while((size_t)ef.fun_frame_size<fun_frames_.size()){
		while(scopes_.size()>ff().scope_lower){
			pop_scope();
		}

		check_debug_hook(pc, BREAKPOINT_RETURN);
		pop_ff2();
		pc = ff().called_pc;
		e = append_backtrace(pc, e);

		// Cの関数にぶつかった
		if(pc==&end_code_){
			ef.info = 0;
			break;
		}
	}

	//XTAL_ASSERT(scopes_.size()>=ef.scope_size);

	while(scopes_.size()<ef.scope_size){
		push_scope();
	}

	while(scopes_.size()>ef.scope_size){
		pop_scope();
	}

	stack_.resize(ef.stack_size);
	set_variables_top(ef.variables_top);

	if(ef.info){
		if(ef.info->catch_pc && e){
			pc = ef.info->catch_pc +  fun()->code()->data();
			stack_.push(AnyPtr(ef.info->end_pc));
			stack_.push(e);
		}
		else{
			pc = ef.info->finally_pc +  fun()->code()->data();
			stack_.push(e);
			stack_.push(AnyPtr(fun()->code()->size()-1));
		}

		except_frames_.downsize(1);
		return pc;
	}
	else{
		set_except_0(e);
	}

	while(scopes_.size()>ff().scope_lower){
		pop_scope();
	}

	pop_ff2();

	return 0;
}

const AnyPtr& VMachine::catch_except(){
	except_[2] = except();
	except_[0] = null;
	return ap(except_[2]);
}

void VMachine::set_except(const AnyPtr& e){
	if(!ap(except_[0])){
		except_[0] = e;
	}
	else{
		//XTAL_ASSERT(false); // 例外をハンドルせずに次の例外を設定した
	}
}

void VMachine::set_except_x(const AnyPtr& e){
	if(!ap(except_[0])){
		except_[0] = e;
	}
}

const inst_t* VMachine::push_except(const inst_t* pc){
	stack_.push(except_[0]);
	except_[0] = null;
	throw_pc_ = pc+1;
	return &throw_code_;
}
	
const inst_t* VMachine::push_except(const inst_t* pc, const AnyPtr& e){
	stack_.push(e);
	except_[0] = null;
	throw_pc_ = pc+1;
	return &throw_code_;
}

void VMachine::on_visit_members(Visitor& m){
	GCObserver::on_visit_members(m);
	m & debug_info_ & except_[0] & except_[1] & except_[2];

	for(int_t i=0, size=stack_.size(); i<size; ++i){
		m & stack_[i];
	}

	for(int_t i=0, size=fun_frames_.size(); i<size; ++i){
		if(FunFrame* f = fun_frames_[i]){
			m & f->fun_;
			m & f->self_;
		}
	}

	for(int_t i=0, size=scopes_.size(); i<size; ++i){
		m & scopes_[i].frame;
	}

	for(int_t i=0, size=std::min((int)variables_top()+256, (int)variables_.size()); i<size; ++i){
		m & variables_.at(i);
	}
}

void VMachine::add_ref_count_members(int_t n){
	add_ref_count_force(except_[0], n);
	add_ref_count_force(except_[1], n);
	add_ref_count_force(except_[2], n);

	for(int_t i=0, size=stack_.size(); i<size; ++i){
		add_ref_count_force(stack_[i], n);
	}

	for(int_t i=0, size=fun_frames_.size(); i<size; ++i){
		if(FunFrame* f = fun_frames_[i]){
			add_ref_count_force(f->fun_, n);
			add_ref_count_force(f->self_, n);
		}
	}

	for(int_t i=0, size=scopes_.size(); i<size; ++i){
		Scope& scope = scopes_[i];
		if(scope.flags==Scope::CLASS && scope.frame){
			scope.frame->add_ref_count_members(n);
		}
	}

	for(int_t i=0, size=std::min((int)variables_top()+128, (int)variables_.size()); i<size; ++i){
		add_ref_count_force(variables_.at(i), n);
	}
}

void VMachine::on_gc_signal(int_t flag){
	if(flag==0){
		for(int_t i=fun_frames_.size(), size=fun_frames_.capacity(); i<size; ++i){
			if(FunFrame* f = fun_frames_.reverse_at_unchecked(i)){
				f->fun_ = null;
				f->self_ = null;
			}
		}

		for(int_t i=(int)variables_top()+128, size=(int)variables_.size(); i<size; ++i){
			(Any&)variables_.at(i) = null;
		}

		add_ref_count_members(1);
	}
	else if(flag==1){
		add_ref_count_members(-1);
	}
}

void VMachine::print_info(){
#ifdef XTAL_DEBUG_PRINT
	std::printf("stack size %d\n", stack_.size());
	for(uint_t i=0; i<stack_.size(); ++i){
//		std::printf("\tstack value %d = %s\n", i, ap(stack_[i])->to_s()->c_str());
	}

	std::printf("fun_frames size %d\n", fun_frames_.size());
	std::printf("except_frames size %d\n", except_frames_.size());
	std::printf("scopes size %d\n", scopes_.size());
#endif
}

}//namespace

