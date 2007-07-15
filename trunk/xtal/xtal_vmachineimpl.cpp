
#include "xtal.h"

#include <map>
#include <fstream>
#include <math.h>

#include "xtal_funimpl.h"
#include "xtal_vmachineimpl.h"
#include "xtal_mapimpl.h"


namespace xtal{

MemberCacheTable::MemberCacheTable(){
	for(int_t i=0; i<CACHE_MAX; ++i){
		table_[i].klass = 0;
		table_[i].member = null;
	}
	hit_ = 0;
	miss_ = 0;
}

const Any& MemberCacheTable::cache(const Any& target_class, const ID& member_name, const Any& self, const Any& nsp){

	uint_t klass = target_class.rawvalue();
	uint_t name = member_name.rawvalue();
	uint_t ns = nsp.rawvalue();

	uint_t hash = (klass>>3) + (name>>2) + (ns);
	Unit& unit = table_[hash/* % CACHE_MAX*/ & (CACHE_MAX-1)];
	if(global_mutate_count==unit.mutate_count && klass==unit.klass && name==unit.name && ns==unit.ns){
		hit_++;
		return unit.member.cref();
	}else{
		if(target_class.type()!=TYPE_BASE)
			return null;

		miss_++;
		unit.member = target_class.impl()->member(member_name, self, nsp);
		unit.klass = klass;
		unit.name = name;
		unit.ns = ns;
		unit.mutate_count = global_mutate_count;
		return unit.member.cref();
	}
}

VMachineImpl::VMachineImpl(){	
	
	myself_ = this;

	stack_.reserve(32);

	end_code_ = InstExit::NUMBER;
	throw_unsupported_error_code_ = InstThrowUnsupportedError::NUMBER;
	check_unsupported_code_ = InstCheckUnsupported::NUMBER;
	cleanup_call_code_ = InstCleanupCall::NUMBER;
	throw_nop_code_ = InstThrowNull::NUMBER;
	resume_pc_ = 0;
}

VMachineImpl::~VMachineImpl(){

}
	
void VMachineImpl::present_for_vm(FunImpl* fun, VMachineImpl* vm, bool add_succ_or_fail_result){
	// 結果をvmに渡す
	if(vm->need_result()){
		if(add_succ_or_fail_result){
			if(resume_pc_!=0){
				vm->push(fun);
			}else{
				vm->push(null);
			}
			vm->push(this, yield_result_count_);
			downsize(yield_result_count_);
			vm->adjust_result(yield_result_count_+1);
		}else{
			vm->push(this, yield_result_count_);
			downsize(yield_result_count_);
			vm->adjust_result(yield_result_count_);
		}
	}
}

const inst_t* VMachineImpl::start_fiber(FiberImpl* fun, VMachineImpl* vm, bool add_succ_or_fail_result){

	yield_result_count_ = 0;
	
	push_ff(&end_code_, vm->need_result_count(), vm->ordered_arg_count(), vm->named_arg_count(), vm->get_arg_this());

	move(vm, vm->ordered_arg_count()+vm->named_arg_count()*2);
	
	resume_pc_ = 0;
	
	carry_over(fun);
	ff().yieldable = true;

	execute_try(ff().called_pc);

	present_for_vm(fun, vm, add_succ_or_fail_result);

	vm->ff().called_pc = &cleanup_call_code_;
	vm->ff().calling_state = FunFrame::CALLING_STATE_PUSHED_RESULT;	
	
	return resume_pc_;
}

const inst_t* VMachineImpl::resume_fiber(FiberImpl* fun, const inst_t* pc, VMachineImpl* vm, bool add_succ_or_fail_result){

	yield_result_count_ = 0;

	ff().called_pc = pc;
	ff().calling_state = FunFrame::CALLING_STATE_PUSHED_FUN;
	
	resume_pc_ = 0;

	move(vm, vm->ordered_arg_count()+vm->named_arg_count()*2);

	execute_try(ff().called_pc);
	
	present_for_vm(fun, vm, add_succ_or_fail_result);

	vm->ff().called_pc = &cleanup_call_code_;
	vm->ff().calling_state = FunFrame::CALLING_STATE_PUSHED_RESULT;	

	return resume_pc_;
}

void VMachineImpl::exit_fiber(){
	XTAL_TRY{
		yield_result_count_ = 0;
		ff().calling_state = FunFrame::CALLING_STATE_PUSHED_FUN;
		ff().called_pc = resume_pc_;
		resume_pc_ = 0;
		execute_try(&throw_nop_code_);
	}XTAL_CATCH(e){
		(void)e;
	}
	reset();
}

void VMachineImpl::reset(){
	stack_.resize(0);
	except_frames_.resize(0);
	fun_frames_.resize(0);
}
	
void VMachineImpl::recycle_call(){
	FunFrame& f = ff();
	downsize(f.ordered_arg_count+f.named_arg_count*2);
	f.ordered_arg_count = 0;
	f.named_arg_count = 0;
	f.called_pc = &throw_unsupported_error_code_;
	f.calling_state = FunFrame::CALLING_STATE_NONE;
}

void VMachineImpl::recycle_call(const Any& a1){
	recycle_call();
	push_arg(a1);
}

Frame VMachineImpl::decolonize(){
	FunFrame& f = ff();
	int_t sum = 0;
	for(int_t k = 0, ksize = f.scopes.size(); k<ksize; ++k){
		if(FrameCore* p = f.scopes.reverse_at(k)){
			Frame block(f.outer(), code(), p);
			for(int_t i = 0; i<p->variable_size; ++i){
				block.set_member_direct(i, f.variables_.reverse_at(sum+p->variable_size-1-i).cref());
			}
			f.outer(block);
			sum+=p->variable_size;
			f.scopes.reverse_at(k)=0;
		}
	}
	f.variables_.downsize(sum);	
	return f.outer();
}

void VMachineImpl::push_args(int_t named_arg_count){
	const Arguments& a = ff().arguments();
	if(!named_arg_count){
		for(int_t i = 0; i<a.impl()->ordered_.size(); ++i){
			push(a.impl()->ordered_.at(i));
		}
	}else{
		int_t usize = a.impl()->ordered_.size();
		upsize(usize);
		int_t offset = named_arg_count*2;
		for(int_t i = 0; i<offset; ++i){
			set(i+usize, get(i));
		}

		for(int_t i = 0; i<usize; ++i){
			set(offset-1-i, a.impl()->ordered_.at(i));
		}
	}

	a.impl()->named_.impl()->push_all(myself());
}

void VMachineImpl::push_ff_args(const inst_t* pc, int_t need_result_count, int_t ordered_arg_count, int_t named_arg_count, const Any& self){
	push_args(named_arg_count);
	const Arguments& a = ff().arguments();
	push_ff(pc, need_result_count, a.impl()->ordered_.size()+ordered_arg_count, a.impl()->named_.size()+named_arg_count, self);
}

void VMachineImpl::recycle_ff(const inst_t* pc, int_t ordered_arg_count, int_t named_arg_count, const Any& self){
	FunFrame& f = ff();
	f.ordered_arg_count = ordered_arg_count;
	f.named_arg_count = named_arg_count;
	f.self(self);
	//f.poped_pc = pc;
	f.called_pc = &throw_unsupported_error_code_;
	f.calling_state = FunFrame::CALLING_STATE_NONE;
}

void VMachineImpl::recycle_ff_args(const inst_t* pc, int_t ordered_arg_count, int_t named_arg_count, const Any& self){
	push_args(named_arg_count);
	const Arguments& a = ff().arguments();
	recycle_ff(pc, a.impl()->ordered_.size()+ordered_arg_count, a.impl()->named_.size()+named_arg_count, self);
}

void VMachineImpl::setup_call(int_t need_result_count){
	push_ff(&end_code_, need_result_count, 0, 0, null);
}

void VMachineImpl::setup_call(int_t need_result_count, const Any& a1){
	push_ff(&end_code_, need_result_count, 1, 0, null);
	push(a1);
}

void VMachineImpl::setup_call(int_t need_result_count, const Any& a1, const Any& a2){
	push_ff(&end_code_, need_result_count, 2, 0, null);
	push(a1); push(a2);
}

void VMachineImpl::setup_call(int_t need_result_count, const Any& a1, const Any& a2, const Any& a3){
	push_ff(&end_code_, need_result_count, 3, 0, null);
	push(a1); push(a2); push(a3);
}

void VMachineImpl::setup_call(int_t need_result_count, const Any& a1, const Any& a2, const Any& a3, const Any& a4){
	push_ff(&end_code_, need_result_count, 4, 0, null);
	push(a1); push(a2); push(a3); push(a4);
}

Arguments VMachineImpl::make_arguments(){
	Arguments p;

	for(int_t i = 0, size = ordered_arg_count(); i<size; ++i){
		p.impl()->ordered_.push_back(arg(i));
	}

	for(int_t i = 0, size = named_arg_count(); i<size; ++i){
		p.impl()->named_.set_at(get(size*2-1-(i*2+0)), get(size*2-1-(i*2+1)));
	}
	return p;
}

Arguments VMachineImpl::make_args(const Fun& fun){
	Arguments p;

	for(int_t i = fun.param_size(), size = ff().ordered_arg_count; i<size; ++i){
		p.impl()->ordered_.push_back(get(ff().ordered_arg_count+ff().named_arg_count*2-1-i));
	}

	ID name;
	for(int_t i = 0, size = ff().named_arg_count; i<size; ++i){
		name = (ID&)get(size*2-1-(i*2+0));
		for(int_t j = 0; j<fun.param_size(); ++j){
			if(fun.param_name_at(j)==name){
				name = null;
				break;
			}
		}
		if(name){
			p.impl()->named_.set_at(name, get(size*2-1-(i*2+1)));
		}
	}

	return p;
}

void VMachineImpl::push_ff(const inst_t* pc, int_t need_result_count, int_t ordered_arg_count, int_t named_arg_count, const Any& self){
	FunFrame& f = fun_frames_.push();
	f.need_result_count = need_result_count;
	f.result_count = 0;
	f.ordered_arg_count = ordered_arg_count;
	f.named_arg_count = named_arg_count;
	f.called_pc = &throw_unsupported_error_code_;
	f.calling_state = FunFrame::CALLING_STATE_NONE;
	f.poped_pc = pc;
	f.self(self);
	f.hint1(null);
	f.hint2(null);
	f.fun(null);
	f.outer(null);
	f.variables_.clear();
	f.scopes.clear();
	f.arguments(null);
	f.temp_ = null;
	f.temp2_ = null;
	f.instance_variables = &empty_have_instance_variables;
}

const Any& VMachineImpl::result(int_t pos){
	if(ff().calling_state==FunFrame::CALLING_STATE_PUSHED_FUN){
		const inst_t* temp = ff().poped_pc;
		ff().poped_pc = &end_code_;
		execute_try(ff().called_pc);
		fun_frames_.upsize(1);
		ff().poped_pc = temp;
		ff().called_pc = &cleanup_call_code_;
		ff().calling_state = FunFrame::CALLING_STATE_PUSHED_RESULT;
	}else if(ff().calling_state==FunFrame::CALLING_STATE_NONE){
		String hint1 = ff().hint1().get_class().object_name();
		String hint2 = ff().hint2();
		pop_ff();
		if(Any uerror = builtin().member("UnsupportedError")){
			XTAL_THROW(uerror(Xt("%s :: '%s' は定義されていません")(
				hint1 ? hint1 : String("?"),
				hint2 ? hint2 : String("()")))); 
		}else{
			//printf("UnsupportedError %s %s\n", hint1 ? hint1.c_str() : "?", hint2 ? hint2.c_str() : "()");
			XTAL_THROW(null);
		}
	}

	XTAL_ASSERT(ff().calling_state==FunFrame::CALLING_STATE_PUSHED_RESULT);
	
	if(pos<ff().need_result_count){
		return get(ff().need_result_count-pos-1);
	}else{
		return null;
	}
}
	
void VMachineImpl::carry_over(FunImpl* fun){
	FunFrame& f = ff();
	
	f.fun(fun);
	f.outer(fun->outer());

	f.variables_.clear();
	f.scopes.clear();

	f.called_pc = fun->pc()+fun->code().impl()->data();
	f.calling_state = FunFrame::CALLING_STATE_PUSHED_FUN;

	f.yieldable = f.poped_pc==&end_code_ ? false : prev_ff().yieldable;
	
	if(f.self().type()==TYPE_BASE){
		f.instance_variables = f.self().impl()->have_instance_variables();
	}

	if(fun->used_args_object()){
		f.arguments(make_args(Fun(fun)));
	}
	
	FunCore* core = fun->core();

	if(int_t size = core->variable_size){
		f.scopes.push(core);
		f.variables_.upsize(size);
		UncountedAny* vars=&f.variables_[size-1];
		for(int_t n = 0; n<size; ++n){
			vars[n] = arg(n, fun);
		}
	}
	
	int_t max_stack = core->max_stack;
	stack_.upsize(max_stack);
	stack_.downsize(f.ordered_arg_count+f.named_arg_count*2 + max_stack);
}

void VMachineImpl::mv_carry_over(FunImpl* fun){
	FunFrame& f = ff();
	
	f.fun(fun);
	f.outer(fun->outer());

	f.variables_.clear();
	f.scopes.clear();

	f.called_pc = fun->pc()+fun->code().impl()->data();
	f.calling_state = FunFrame::CALLING_STATE_PUSHED_FUN;

	f.yieldable = f.poped_pc==&end_code_ ? false : prev_ff().yieldable;

	if(f.self().type()==TYPE_BASE){
		f.instance_variables = f.self().impl()->have_instance_variables();
	}

	if(fun->used_args_object()){
		f.arguments(make_args(Fun(fun)));
	}
	
	// 名前付き引数は邪魔
	stack_.downsize(f.named_arg_count*2);

	FunCore* core = fun->core();
	
	if(int_t size = core->variable_size){
		f.scopes.push(core);
		f.variables_.upsize(size);
	
		adjust_result(f.ordered_arg_count, size);
		
		UncountedAny* vars=&f.variables_[size-1];
		for(int_t n = 0; n<size; ++n){
			vars[n] = get(size-1-n);
		}
		
		stack_.downsize(size);
	}
	
	int_t max_stack = core->max_stack;
	stack_.upsize(max_stack);
	stack_.downsize(max_stack);
}

void VMachineImpl::adjust_result(int_t n, int_t need_result_count){

	// 戻り値の数と要求している戻り値の数が等しい
	if(need_result_count==n){
		return;
	}

	// 戻り値なんて要求してない
	if(need_result_count==0){
		downsize(n);
		return;
	}

	// 戻り値が一つも無いのでnullで埋める
	if(n==0){
		for(int_t i = 0; i<need_result_count; ++i){
			push(null);
		}
		return;
	}

	// この時点で、nもneed_result_countも1以上


	// 要求している戻り値の数の方が、関数が返す戻り値より少ない
	if(need_result_count<n){

		// 余った戻り値を配列に直す。
		int_t size = n-need_result_count+1;
		XTAL_GLOBAL_INTERPRETER_LOCK{
			Array ret(size);
			for(int_t i=0; i<size; ++i){
				ret.set_at(i, get(size-1-i));
			}
			downsize(size);
			push(ret);
		}	
	}else{
		// 要求している戻り値の数の方が、関数が返す戻り値より多い

		if(const Array& temp = xtal::as<const Array&>(get())){

			int_t len;

			// 配列を展開し埋め込む
			XTAL_GLOBAL_INTERPRETER_LOCK{
				// 最後の要素の配列を展開する。
				Array ary(temp);
				downsize(1);

				len = ary.size();
				for(int_t i=0; i<len; ++i){
					push(ary.at(i));
				}
			}

			adjust_result(len-1, need_result_count-n);
		}else{
			// 最後の要素が配列ではないので、nullで埋めとく
			for(int_t i = n; i<need_result_count; ++i){
				push(null);
			}
		}
	}
}


#ifdef __GNUC__
//#	define XTAL_VM_OPT
#endif

#ifdef XTAL_VM_OPT

#define XTAL_VM_NODEFAULT } goto *label_table[*pc]; XTAL_NODEFAULT
#define XTAL_VM_FIRST_CASE(key) Label##key: { Inst##key& inst = *(Inst##key*)pc;
#define XTAL_VM_CASE(key) } goto *label_table[*pc]; Label##key: { typedef Inst##key Inst; Inst& inst = *(Inst*)pc;
#define XTAL_VM_SWITCH(x) goto *label_table[x];
#define XTAL_VM_DEF(key) Inst##key& inst = *(Inst##key*)pc
#define XTAL_VM_CONTINUE goto *label_table[*pc]

#else

//std::ofstream log("code.txt");
//#define XTAL_VM_CASE(key) goto begin; case key: log<<(#key)<<std::endl;
//#define XTAL_VM_CASE(key) } goto begin; case Inst##key::NUMBER: { typedef Inst##key Inst; Inst& inst = *(Inst*)pc; printf("%s\n", #key);
#define XTAL_VM_NODEFAULT } goto begin; XTAL_NODEFAULT
#define XTAL_VM_FIRST_CASE(key) case Inst##key::NUMBER: { Inst##key& inst = *(Inst##key*)pc;
#define XTAL_VM_CASE(key) } goto begin; case Inst##key::NUMBER: { typedef Inst##key Inst; Inst& inst = *(Inst*)pc;
#define XTAL_VM_SWITCH(x) switch(x)
#define XTAL_VM_DEF(key) Inst##key& inst = *(Inst##key*)pc
#define XTAL_VM_CONTINUE goto begin
#endif

void VMachineImpl::execute_try(const inst_t* start){ 
	int_t stack_size = stack_.size();
	int_t fun_frames_size = fun_frames_.size();
	XTAL_GLOBAL_INTERPRETER_UNLOCK{
retry:
		XTAL_TRY{
			execute_inner(start);
		}XTAL_CATCH(e){
			last_except_ = e;
			start = CATCH_BODY(ff().called_pc, stack_size, fun_frames_size);

			if(start){
				goto retry;
			}

			pop_ff();
			XTAL_THROW(last_except_.cref());	
		}
#ifndef XTAL_NO_EXCEPT
		catch(...){
			last_except_ = null;
			CATCH_BODY(start, stack_size, fun_frames_size);
			pop_ff();
			throw;
		}
#endif
	}
}

void VMachineImpl::execute_inner(const inst_t* start){

#ifdef XTAL_VM_OPT

	static void* label_table[] = {
		&&LabelNop,
		&&LabelPushNull,
		&&LabelPushNop,
		&&LabelPushTrue,
		&&LabelPushFalse,
		&&LabelPushInt0,
		&&LabelPushInt1,
		&&LabelPushInt2,
		&&LabelPushInt3,
		&&LabelPushInt4,
		&&LabelPushInt5,
		&&LabelPushInt1Byte,
		&&LabelPushInt2Byte,
		&&LabelPushFloat0,
		&&LabelPushFloat025,
		&&LabelPushFloat05,
		&&LabelPushFloat1,
		&&LabelPushFloat2,
		&&LabelPushFloat3,
		&&LabelMakeArray,
		&&LabelArrayAppend,
		&&LabelMakeMap,
		&&LabelMapInsert,
		&&LabelMakeFun,
		&&LabelMakeInstanceVariableAccessor,
		&&LabelPushCallee,
		&&LabelPushArgs,
		&&LabelPushThis,
		&&LabelPushCurrentContext,
		&&LabelAssert,
		&&LabelPop,
		&&LabelDup,
		&&LabelInsert1,
		&&LabelInsert2,
		&&LabelInsert3,
		&&LabelSetName,
		&&LabelSetAccessibility,
		&&LabelBlockBegin,
		&&LabelBlockEnd,
		&&LabelBlockEndDirect,
		&&LabelClassBegin,
		&&LabelClassEnd,
		&&LabelLocalVariable0Direct,
		&&LabelLocalVariable1Direct,
		&&LabelLocalVariable2Direct,
		&&LabelLocalVariable3Direct,
		&&LabelLocalVariable4Direct,
		&&LabelLocalVariable5Direct,
		&&LabelLocalVariable1ByteDirect,
		&&LabelLocalVariable1Byte,
		&&LabelLocalVariable2Byte,
		&&LabelSetLocalVariable0Direct,
		&&LabelSetLocalVariable1Direct,
		&&LabelSetLocalVariable2Direct,
		&&LabelSetLocalVariable3Direct,
		&&LabelSetLocalVariable4Direct,
		&&LabelSetLocalVariable5Direct,
		&&LabelSetLocalVariable1ByteDirect,
		&&LabelSetLocalVariable1Byte,
		&&LabelSetLocalVariable2Byte,
		&&LabelGlobalVariable,
		&&LabelSetGlobalVariable,
		&&LabelDefineGlobalVariable,
		&&LabelMember,
		&&LabelMemberIfDefined,
		&&LabelDefineMember,
		&&LabelInstanceVariable,
		&&LabelSetInstanceVariable,
		&&LabelPushGoto,
		&&LabelPopGoto,
		&&LabelOnce,
		&&LabelCleanupCall,
		&&LabelReturn0,
		&&LabelReturn1,
		&&LabelReturn2,
		&&LabelReturn,
		&&LabelYield,
		&&LabelYield,
		&&LabelYield,
		&&LabelYield,
		&&LabelExit,
		&&LabelThrowUnsupportedError,
		&&LabelThrowNull,
		&&LabelValue,
		&&LabelSetValue,
		&&LabelCheckUnsupported,
		&&LabelSend,
		&&LabelSendIfDefined,
		&&LabelCall,
		&&LabelCallCallee,
		&&LabelSend,
		&&LabelSendIfDefined,
		&&LabelCall,
		&&LabelCallCallee,
		&&LabelSend_A,
		&&LabelSendIfDefined_A,
		&&LabelCall_A,
		&&LabelCallCallee_A,
		&&LabelSend_T,
		&&LabelSendIfDefined_T,
		&&LabelCall_T,
		&&LabelCallCallee_T,
		&&LabelSend_AT,
		&&LabelSendIfDefined_AT,
		&&LabelCall_AT,
		&&LabelCallCallee_AT,
		&&LabelTryBegin,
		&&LabelTryEnd,
		&&LabelThrow,
		&&LabelIf,
		&&LabelUnless,
		&&LabelGoto,
		&&LabelIfEq,
		&&LabelIfNe,
		&&LabelIfLt,
		&&LabelIfLe,
		&&LabelIfGt,
		&&LabelIfGe,
		&&LabelIfRawEq,
		&&LabelIfRawNe,
		&&LabelIfIs,
		&&LabelIfNis,
		&&LabelIfArgIsNull,
		&&LabelPos,
		&&LabelNeg,
		&&LabelCom,
		&&LabelNot,
		&&LabelAt,
		&&LabelSetAt,
		&&LabelAdd,
		&&LabelSub,
		&&LabelCat,
		&&LabelMul,
		&&LabelDiv,
		&&LabelMod,
		&&LabelAnd,
		&&LabelOr,
		&&LabelXor,
		&&LabelShl,
		&&LabelShr,
		&&LabelUshr,
		&&LabelEq,
		&&LabelNe,
		&&LabelLt,
		&&LabelLe,
		&&LabelGt,
		&&LabelGe,
		&&LabelRawEq,
		&&LabelRawNe,
		&&LabelIs,
		&&LabelNis,
		&&LabelInc,
		&&LabelDec,
		&&LabelAddAssign,
		&&LabelSubAssign,
		&&LabelCatAssign,
		&&LabelMulAssign,
		&&LabelDivAssign,
		&&LabelModAssign,
		&&LabelAndAssign,
		&&LabelOrAssign,
		&&LabelXorAssign,
		&&LabelShlAssign,
		&&LabelShrAssign,
		&&LabelUshrAssign,
		&&LabelLocalVariableInc,
		&&LabelLocalVariableDec,
		&&LabelLocalVariableIncDirect,
		&&LabelLocalVariableDecDirect,
		&&LabelLocalVariableInc,
		&&LabelLocalVariableInc,
		&&LabelLocalVariableInc,
		&&LabelLocalVariableInc,
		&&LabelLocalVariableInc,
		&&LabelLocalVariableInc,
		&&LabelLocalVariableInc,
		&&LabelLocalVariableInc,
		&&LabelLocalVariableInc,
		&&LabelLocalVariableInc,
		&&LabelLocalVariableInc,
		&&LabelLocalVariableInc,
		&&LabelLocalVariableInc,
		&&LabelLocalVariableInc,
		&&LabelLocalVariableInc,
		&&LabelLocalVariableInc,
		&&LabelLocalVariableInc,
		&&LabelLocalVariableInc,
		&&LabelLocalVariableInc,
		&&LabelLocalVariableInc,
		&&LabelLocalVariableInc,
		&&LabelLocalVariableInc,
		&&LabelLocalVariableInc,
		&&LabelLocalVariableInc,
		&&LabelLocalVariableInc,
		&&LabelLocalVariableInc,
		&&LabelBreakPoint,
		&&LabelAdjustResult,
		&&LabelSendIterFirst,
		&&LabelSendIterNext,
		&&LabelSendIterBreak,
	};
#else

#endif

	const inst_t* pc = start;
	int_t stack_size = stack_.size();
	int_t fun_frames_size = fun_frames_.size();

begin:
XTAL_VM_SWITCH(*pc){
	
	XTAL_VM_FIRST_CASE(Nop){ pc += inst.ISIZE; }
	XTAL_VM_CASE(PushNull){ push(null); pc += inst.ISIZE; }
	XTAL_VM_CASE(PushTrue){ push(UncountedAny(true).cref()); pc += inst.ISIZE; }
	XTAL_VM_CASE(PushFalse){ push(UncountedAny(false).cref()); pc += inst.ISIZE; }
	XTAL_VM_CASE(PushNop){ push(nop); pc += inst.ISIZE; }
	
	XTAL_VM_CASE(PushInt0){ push(UncountedAny(0).cref()); pc += inst.ISIZE; }
	XTAL_VM_CASE(PushInt1){ push(UncountedAny(1).cref()); pc += inst.ISIZE; }
	XTAL_VM_CASE(PushInt2){ push(UncountedAny(2).cref()); pc += inst.ISIZE; }
	XTAL_VM_CASE(PushInt3){ push(UncountedAny(3).cref()); pc += inst.ISIZE; }
	XTAL_VM_CASE(PushInt4){ push(UncountedAny(4).cref()); pc += inst.ISIZE; }
	XTAL_VM_CASE(PushInt5){ push(UncountedAny(5).cref()); pc += inst.ISIZE; }
	
	XTAL_VM_CASE(PushFloat0){ push(UncountedAny(0.0f).cref()); pc += inst.ISIZE; }
	XTAL_VM_CASE(PushFloat025){ push(UncountedAny(0.25f).cref()); pc += inst.ISIZE; }
	XTAL_VM_CASE(PushFloat05){ push(UncountedAny(0.5f).cref()); pc += inst.ISIZE; }
	XTAL_VM_CASE(PushFloat1){ push(UncountedAny(1.0f).cref()); pc += inst.ISIZE; }
	XTAL_VM_CASE(PushFloat2){ push(UncountedAny(2.0f).cref()); pc += inst.ISIZE; }
	XTAL_VM_CASE(PushFloat3){ push(UncountedAny(3.0f).cref()); pc += inst.ISIZE; }

	XTAL_VM_CASE(PushInt1Byte){ push(UncountedAny((int_t)inst.value).cref()); pc += inst.ISIZE; }
	XTAL_VM_CASE(PushInt2Byte){ push(UncountedAny((int_t)inst.value).cref()); pc += inst.ISIZE; }
	
	XTAL_VM_CASE(PushThis){ push(ff().self()); pc += inst.ISIZE; }
	XTAL_VM_CASE(IfArgIsNull){ LOCAL_VARIABLE(inst.arg).is_null() ? pc += inst.ISIZE : pc += inst.address; }
	XTAL_VM_CASE(Insert1){ UncountedAny temp = get(); set(get(1)); set(1, temp.cref()); pc += inst.ISIZE; }
	XTAL_VM_CASE(Insert2){ UncountedAny temp = get(); set(get(1)); set(1, get(2)); set(2, temp.cref()); pc += inst.ISIZE; }
	XTAL_VM_CASE(Insert3){ UncountedAny temp = get(); set(get(1)); set(1, get(2)); set(2, get(3)); set(3, temp.cref()); pc += inst.ISIZE; }
	
	XTAL_VM_CASE(Pop){ downsize(1); pc += inst.ISIZE; }
	XTAL_VM_CASE(Dup){ dup(); pc += inst.ISIZE; }
	XTAL_VM_CASE(AdjustResult){ adjust_result(inst.result_count, inst.need_result_count); pc += inst.ISIZE; }
	
	XTAL_VM_CASE(If){ pop().to_b() ? pc += inst.ISIZE : pc += inst.address; }
	XTAL_VM_CASE(Unless){ !pop().to_b() ? pc += inst.ISIZE : pc += inst.address; }
	XTAL_VM_CASE(Goto){ pc += inst.address; }
	
	XTAL_VM_CASE(Return0){ 
		for(int_t i=0, sz=ff().need_result_count; i<sz; ++i){
			push(null);
		}
		pc = pop_ff();  
	}
	XTAL_VM_CASE(Return1){
		if(ff().need_result_count!=1){
			adjust_result(1);
		}
		pc = pop_ff();  
	}

	XTAL_VM_CASE(Return2){ 
		adjust_result(2); 
		pc = pop_ff(); 
	}

	XTAL_VM_CASE(Return){ 
		adjust_result(inst.results); 
		pc = pop_ff();  
	}
	
	XTAL_VM_CASE(CleanupCall){ 
		pc = pop_ff();
	}

	XTAL_VM_CASE(Yield){
		yield_result_count_ = inst.results;	
		if(ff().yieldable){
			resume_pc_ = pc + inst.ISIZE;
			return;
		}else{
			downsize(yield_result_count_);
			XTAL_GLOBAL_INTERPRETER_LOCK{ XTAL_THROW(builtin().member("YieldError")(Xt("Xtal Runtime Error 1012"))); }
		}
	}
	
	XTAL_VM_CASE(TryBegin){ pc = TRY_BEGIN(pc); }
	XTAL_VM_CASE(TryEnd){ pc = except_frames_.pop().core->finally_pc + source(); }
	
	XTAL_VM_CASE(BlockBegin){ 
		FunFrame& f = ff(); 
		FrameCore* p = code().frame_core(inst.core_number); 
		f.scopes.push(p); 
		f.variables_.upsize(p->variable_size);
		pc += inst.ISIZE;
	}

	XTAL_VM_CASE(BlockEnd){ pc = BLOCK_END(pc); }

	XTAL_VM_CASE(BlockEndDirect){
		FunFrame& f = ff(); 
		f.variables_.downsize(f.scopes.top()->variable_size);
		f.scopes.downsize(1);
		pc += inst.ISIZE;
	}
	
	XTAL_VM_CASE(InstanceVariable){
		XTAL_GLOBAL_INTERPRETER_LOCK{
			push(ff().instance_variables->variable(inst.number, code().impl()->frame_core(inst.core_number)));
		}
		pc += inst.ISIZE; 
	}
	XTAL_VM_CASE(SetInstanceVariable){ 
		XTAL_GLOBAL_INTERPRETER_LOCK{ 
			ff().instance_variables->set_variable(inst.number, code().impl()->frame_core(inst.core_number), pop());
		}
		pc += inst.ISIZE; 
	}

	XTAL_VM_CASE(LocalVariable0Direct){ push(ff().variable(0)); pc += inst.ISIZE; }
	XTAL_VM_CASE(LocalVariable1Direct){ push(ff().variable(1)); pc += inst.ISIZE; }
	XTAL_VM_CASE(LocalVariable2Direct){ push(ff().variable(2)); pc += inst.ISIZE; }
	XTAL_VM_CASE(LocalVariable3Direct){ push(ff().variable(3)); pc += inst.ISIZE; }
	XTAL_VM_CASE(LocalVariable4Direct){ push(ff().variable(4)); pc += inst.ISIZE; }
	XTAL_VM_CASE(LocalVariable5Direct){ push(ff().variable(5)); pc += inst.ISIZE; }
	XTAL_VM_CASE(LocalVariable1ByteDirect){ push(ff().variable(inst.number)); pc += inst.ISIZE; }
	XTAL_VM_CASE(LocalVariable1Byte){ push(LOCAL_VARIABLE(inst.number)); pc += inst.ISIZE; }
	XTAL_VM_CASE(LocalVariable2Byte){ push(LOCAL_VARIABLE(inst.number)); pc += inst.ISIZE; }

	XTAL_VM_CASE(SetLocalVariable0Direct){ ff().variable(0, pop()); pc += inst.ISIZE; }
	XTAL_VM_CASE(SetLocalVariable1Direct){ ff().variable(1, pop()); pc += inst.ISIZE; }
	XTAL_VM_CASE(SetLocalVariable2Direct){ ff().variable(2, pop()); pc += inst.ISIZE; }
	XTAL_VM_CASE(SetLocalVariable3Direct){ ff().variable(3, pop()); pc += inst.ISIZE; }
	XTAL_VM_CASE(SetLocalVariable4Direct){ ff().variable(4, pop()); pc += inst.ISIZE; }
	XTAL_VM_CASE(SetLocalVariable5Direct){ ff().variable(5, pop()); pc += inst.ISIZE; }
	XTAL_VM_CASE(SetLocalVariable1ByteDirect){ ff().variable(inst.number, pop()); pc += inst.ISIZE; }
	XTAL_VM_CASE(SetLocalVariable1Byte){ SET_LOCAL_VARIABLE(inst.number, pop()); pc += inst.ISIZE; }
	XTAL_VM_CASE(SetLocalVariable2Byte){ SET_LOCAL_VARIABLE(inst.number, pop()); pc += inst.ISIZE; }

	XTAL_VM_CASE(SetMultipleLocalVariable2Direct){
		FunFrame& f = ff();
		f.variable(inst.local_variable2, pop()); 
		f.variable(inst.local_variable1, pop()); 
		pc += inst.ISIZE;
	}

	XTAL_VM_CASE(SetMultipleLocalVariable3Direct){
		FunFrame& f = ff();
		f.variable(inst.local_variable3, pop()); 
		f.variable(inst.local_variable2, pop()); 
		f.variable(inst.local_variable1, pop()); 
		pc += inst.ISIZE;
	}

	XTAL_VM_CASE(SetMultipleLocalVariable2){
		SET_LOCAL_VARIABLE(inst.local_variable2, pop()); 
		SET_LOCAL_VARIABLE(inst.local_variable1, pop()); 
		pc += inst.ISIZE;
	}

	XTAL_VM_CASE(SetMultipleLocalVariable3){
		SET_LOCAL_VARIABLE(inst.local_variable3, pop()); 
		SET_LOCAL_VARIABLE(inst.local_variable2, pop()); 
		SET_LOCAL_VARIABLE(inst.local_variable1, pop()); 
		pc += inst.ISIZE;
	}

	XTAL_VM_CASE(GlobalVariable){ pc = GLOBAL_VARIABLE(pc); }
	XTAL_VM_CASE(SetGlobalVariable){ pc = SET_GLOBAL_VARIABLE(pc); }
	XTAL_VM_CASE(DefineGlobalVariable){ pc = DEFINE_GLOBAL_VARIABLE(pc); }

	XTAL_VM_CASE(Member){ pc = MEMBER(pc); }
	XTAL_VM_CASE(MemberIfDefined){ pc = MEMBER_IF_DEFINED(pc); }
	XTAL_VM_CASE(DefineMember){ pc = DEFINE_MEMBER(pc); }

	XTAL_VM_CASE(At){ pc = AT(pc); }
	XTAL_VM_CASE(SetAt){ pc = SET_AT(pc); }

	XTAL_VM_CASE(Once){ pc = ONCE(pc); }

	XTAL_VM_CASE(Value){ push(code().impl()->value(inst.value_number)); pc += inst.ISIZE; }
	XTAL_VM_CASE(SetValue){ code().impl()->set_value(inst.value_number, pop()); pc += inst.ISIZE; }
	
	XTAL_VM_CASE(PushGoto){ push(UncountedAny((pc+inst.address)-source()).cref()); pc += inst.ISIZE; }
	XTAL_VM_CASE(PopGoto){ pc = source()+pop().ivalue(); }

	XTAL_VM_CASE(Not){ set(UncountedAny(!get().to_b()).cref()); pc += inst.ISIZE; }
	XTAL_VM_CASE(Pos){ pc = POS(pc); }
	XTAL_VM_CASE(Neg){ pc = NEG(pc); }
	XTAL_VM_CASE(Com){ pc = COM(pc); }

	XTAL_VM_CASE(Add){ pc = ADD(pc); }
	XTAL_VM_CASE(Sub){ pc = SUB(pc); }
	XTAL_VM_CASE(Cat){ pc = CAT(pc); }
	XTAL_VM_CASE(Mul){ pc = MUL(pc); }
	XTAL_VM_CASE(Div){ pc = DIV(pc); }
	XTAL_VM_CASE(Mod){ pc = MOD(pc); }
	XTAL_VM_CASE(And){ pc = AND(pc); }
	XTAL_VM_CASE(Or){ pc = OR(pc); }
	XTAL_VM_CASE(Xor){ pc = XOR(pc); }
	XTAL_VM_CASE(Shr){ pc = SHR(pc); }
	XTAL_VM_CASE(Ushr){ pc = USHR(pc); }
	XTAL_VM_CASE(Shl){ pc = SHL(pc); }

	XTAL_VM_CASE(Eq){ pc = EQ(pc); }
	XTAL_VM_CASE(Ne){ pc = NE(pc); }
	XTAL_VM_CASE(Lt){ pc = LT(pc); }
	XTAL_VM_CASE(Gt){ pc = GT(pc); }
	XTAL_VM_CASE(Le){ pc = LE(pc); }
	XTAL_VM_CASE(Ge){ pc = GE(pc); }
	XTAL_VM_CASE(RawEq){ pc = RAW_EQ(pc); }
	XTAL_VM_CASE(RawNe){ pc = RAW_NE(pc); }
	XTAL_VM_CASE(Is){ pc = IS(pc); }
	XTAL_VM_CASE(Nis){ pc = NIS(pc); }

	XTAL_VM_CASE(IfEq){ pc = IF_EQ(pc); }
	XTAL_VM_CASE(IfNe){ pc = IF_NE(pc); }
	XTAL_VM_CASE(IfLt){ pc = IF_LT(pc); }
	XTAL_VM_CASE(IfGt){ pc = IF_GT(pc); }
	XTAL_VM_CASE(IfLe){ pc = IF_LE(pc); }
	XTAL_VM_CASE(IfGe){ pc = IF_GE(pc); }
	XTAL_VM_CASE(IfRawEq){ pc = IF_RAW_EQ(pc); }
	XTAL_VM_CASE(IfRawNe){ pc = IF_RAW_NE(pc); }
	XTAL_VM_CASE(IfIs){ pc = IF_IS(pc); }
	XTAL_VM_CASE(IfNis){ pc = IF_NIS(pc); }

	XTAL_VM_CASE(Inc){ pc = INC(pc); }
	XTAL_VM_CASE(Dec){ pc = DEC(pc); }
	XTAL_VM_CASE(LocalVariableInc){ pc = LOCAL_VARIABLE_INC(pc); }
	XTAL_VM_CASE(LocalVariableDec){ pc = LOCAL_VARIABLE_DEC(pc); }
	XTAL_VM_CASE(LocalVariableIncDirect){ pc = LocalVariableIncDirect(pc); }
	XTAL_VM_CASE(LocalVariableDecDirect){ pc = LocalVariableDecDirect(pc); }

	XTAL_VM_CASE(AddAssign){ pc = ADD_ASSIGN(pc); }
	XTAL_VM_CASE(SubAssign){ pc = SUB_ASSIGN(pc); }
	XTAL_VM_CASE(CatAssign){ pc = CAT_ASSIGN(pc); }
	XTAL_VM_CASE(MulAssign){ pc = MUL_ASSIGN(pc); }
	XTAL_VM_CASE(DivAssign){ pc = DIV_ASSIGN(pc); }
	XTAL_VM_CASE(ModAssign){ pc = MOD_ASSIGN(pc); }
	XTAL_VM_CASE(AndAssign){ pc = AND_ASSIGN(pc); }
	XTAL_VM_CASE(OrAssign){ pc = OR_ASSIGN(pc); }
	XTAL_VM_CASE(XorAssign){ pc = XOR_ASSIGN(pc); }
	XTAL_VM_CASE(ShrAssign){ pc = SHR_ASSIGN(pc); }
	XTAL_VM_CASE(UshrAssign){ pc = USHR_ASSIGN(pc); }
	XTAL_VM_CASE(ShlAssign){ pc = SHL_ASSIGN(pc); }
	XTAL_VM_CASE(MakeArray){ pc = PUSH_ARRAY(pc); }
	XTAL_VM_CASE(MakeMap){ pc = PUSH_MAP(pc); }
	XTAL_VM_CASE(PushCallee){ push(fun()); pc += inst.ISIZE; }
	
	XTAL_VM_CASE(MakeFun){ pc = PUSH_FUN(pc); }	

	XTAL_VM_CASE(MakeInstanceVariableAccessor){
		XTAL_GLOBAL_INTERPRETER_LOCK{
			Any ret;
			switch(inst.type){
				XTAL_NODEFAULT;

				XTAL_CASE(0){ new(ret) InstanceVariableGetterImpl(inst.number, code().impl()->frame_core(inst.core_number)); }
				XTAL_CASE(1){ new(ret) InstanceVariableSetterImpl(inst.number, code().impl()->frame_core(inst.core_number)); }
			}
			push(ret);
		}
		pc += inst.ISIZE; 
	}

	XTAL_VM_CASE(PushCurrentContext){ pc = CURRENT_CONTEXT(pc); }	
	XTAL_VM_CASE(PushArgs){ pc = PUSH_ARGS(pc); }
	
	XTAL_VM_CASE(ClassBegin){ pc = CLASS_BEGIN(pc); }
	XTAL_VM_CASE(ClassEnd){ pc = CLASS_END(pc); }
	XTAL_VM_CASE(SetName){ pc = SET_NAME(pc); }

	XTAL_VM_CASE(CheckUnsupported){ 
		return_result(nop); 
		pc = pop_ff();
	}
	
	XTAL_VM_CASE(DefineClassMember){
		XTAL_GLOBAL_INTERPRETER_LOCK{
			ClassImpl* p = cast<Class>(ff().outer()).impl();
			p->set_class_member(inst.number, symbol(inst.symbol_number), inst.accessibility, get(), get(1));
			downsize(2);
		}
		pc += inst.ISIZE;
	}
	
	XTAL_VM_CASE(ArrayAppend){ pc = ARRAY_APPEND(pc); }
	XTAL_VM_CASE(MapInsert){ pc = MAP_INSERT(pc); }

	XTAL_VM_CASE(Exit){ resume_pc_ = 0; return; }
	
	XTAL_VM_CASE(BreakPoint){ pc = BREAKPOINT(pc); }

	XTAL_VM_CASE(Call){
		XTAL_GLOBAL_INTERPRETER_LOCK{
			UncountedAny self = ff().self();
			UncountedAny target = ff().temp_ = pop();
			push_ff(pc + inst.ISIZE, inst.need_result_count, inst.ordered_arg_count, inst.named_arg_count, self.cref());
			target.cref().call(myself());
		}
		pc = ff().called_pc;	
	}

	XTAL_VM_CASE(Call_A){
		XTAL_GLOBAL_INTERPRETER_LOCK{
			UncountedAny self = ff().self();
			UncountedAny target = ff().temp_ = pop();
			push_ff_args(pc + inst.ISIZE, inst.need_result_count, inst.ordered_arg_count, inst.named_arg_count, self.cref());
			target.cref().call(myself());
		}
		pc = ff().called_pc;	
	}

	XTAL_VM_CASE(Call_T){
		XTAL_GLOBAL_INTERPRETER_LOCK{
			UncountedAny self = ff().self();
			UncountedAny target = ff().temp_ = pop();
			recycle_ff(pc + inst.ISIZE, inst.ordered_arg_count, inst.named_arg_count, self.cref());
			target.cref().call(myself());
		}
		pc = ff().called_pc;	
	}

	XTAL_VM_CASE(Call_AT){
		XTAL_GLOBAL_INTERPRETER_LOCK{
			UncountedAny self = ff().self();
			UncountedAny target = ff().temp_ = pop();
			recycle_ff_args(pc + inst.ISIZE, inst.ordered_arg_count, inst.named_arg_count, self.cref());
			target.cref().call(myself());
		}
		pc = ff().called_pc;	
	}


	XTAL_VM_CASE(CallCallee){
		UncountedAny fn = fun();
		UncountedAny self = ff().self();
		push_ff(pc + inst.ISIZE, inst.need_result_count, inst.ordered_arg_count, inst.named_arg_count, self.cref());
		carry_over(((const Fun&)fn.cref()).impl());
		pc = ff().called_pc;	
	}

	XTAL_VM_CASE(CallCallee_A){
		UncountedAny fn = fun();
		UncountedAny self = ff().self();
		push_ff_args(pc + inst.ISIZE, inst.need_result_count, inst.ordered_arg_count, inst.named_arg_count, self.cref());
		carry_over(((const Fun&)fn.cref()).impl());
		pc = ff().called_pc;	
	}

	XTAL_VM_CASE(CallCallee_T){
		UncountedAny fn = fun();
		UncountedAny self = ff().self();
		recycle_ff(pc + inst.ISIZE, inst.ordered_arg_count, inst.named_arg_count, self.cref());
		carry_over(((const Fun&)fn.cref()).impl());
		pc = ff().called_pc;	
	}

	XTAL_VM_CASE(CallCallee_AT){
		UncountedAny fn = fun();
		UncountedAny self = ff().self();
		recycle_ff_args(pc + inst.ISIZE, inst.ordered_arg_count, inst.named_arg_count, self.cref());
		carry_over(((const Fun&)fn.cref()).impl());
		pc = ff().called_pc;	
	}


	XTAL_VM_CASE(Send){
		XTAL_GLOBAL_INTERPRETER_LOCK{
			const ID& sym = symbol_ex(inst.symbol_number);
			UncountedAny self = ff().self();
			UncountedAny target = ff().temp_ = pop();
			push_ff(pc + inst.ISIZE, inst.need_result_count, inst.ordered_arg_count, inst.named_arg_count, self.cref());
			const Class& cls = target.cref().get_class();
			set_hint(cls, sym);
			if(const Any& ret = member_cache(cls, sym, ff().self(), null)){
				set_arg_this(target.cref());
				ret.call(myself());
			}
		}
		pc = ff().called_pc; 	
	}

	XTAL_VM_CASE(Send_A){
		XTAL_GLOBAL_INTERPRETER_LOCK{
			const ID& sym = symbol_ex(inst.symbol_number);
			UncountedAny self = ff().self();
			UncountedAny target = ff().temp_ = pop();
			push_ff_args(pc + inst.ISIZE, inst.need_result_count, inst.ordered_arg_count, inst.named_arg_count, self.cref());
			const Class& cls = target.cref().get_class();
			set_hint(cls, sym);
			if(const Any& ret = member_cache(cls, sym, ff().self(), null)){
				set_arg_this(target.cref());
				ret.call(myself());
			}
		}
		pc = ff().called_pc; 	
	}

	XTAL_VM_CASE(Send_T){
		XTAL_GLOBAL_INTERPRETER_LOCK{
			const ID& sym = symbol_ex(inst.symbol_number);
			UncountedAny self = ff().self();
			UncountedAny target = ff().temp_ = pop();
			recycle_ff(pc + inst.ISIZE, inst.ordered_arg_count, inst.named_arg_count, self.cref());
			const Class& cls = target.cref().get_class();
			set_hint(cls, sym);
			if(const Any& ret = member_cache(cls, sym, ff().self(), null)){
				set_arg_this(target.cref());
				ret.call(myself());
			}
		}
		pc = ff().called_pc; 	
	}

	XTAL_VM_CASE(Send_AT){
		XTAL_GLOBAL_INTERPRETER_LOCK{
			const ID& sym = symbol_ex(inst.symbol_number);
			UncountedAny self = ff().self();
			UncountedAny target = ff().temp_ = pop();
			recycle_ff_args(pc + inst.ISIZE, inst.ordered_arg_count, inst.named_arg_count, self.cref());
			const Class& cls = target.cref().get_class();
			set_hint(cls, sym);
			if(const Any& ret = member_cache(cls, sym, ff().self(), null)){
				set_arg_this(target.cref());
				ret.call(myself());
			}
		}
		pc = ff().called_pc; 	
	}

	XTAL_VM_CASE(SendIfDefined){
		XTAL_GLOBAL_INTERPRETER_LOCK{
			const ID& sym = symbol_ex(inst.symbol_number);
			UncountedAny self = ff().self();
			UncountedAny target = ff().temp_ = pop();
			push_ff(pc + inst.ISIZE, inst.need_result_count, inst.ordered_arg_count, inst.named_arg_count, self.cref());
			ff().called_pc = &check_unsupported_code_;
			const Class& cls = target.cref().get_class();
			set_hint(cls, sym);
			if(const Any& ret = member_cache(cls, sym, ff().self(), null)){
				set_arg_this(target.cref());
				ret.call(myself());
			}
		}
		pc = ff().called_pc; 	
	}


	XTAL_VM_CASE(SendIfDefined_A){
		XTAL_GLOBAL_INTERPRETER_LOCK{
			const ID& sym = symbol_ex(inst.symbol_number);
			UncountedAny self = ff().self();
			UncountedAny target = ff().temp_ = pop();
			push_ff_args(pc + inst.ISIZE, inst.need_result_count, inst.ordered_arg_count, inst.named_arg_count, self.cref());
			ff().called_pc = &check_unsupported_code_;
			const Class& cls = target.cref().get_class();
			set_hint(cls, sym);
			if(const Any& ret = member_cache(cls, sym, ff().self(), null)){
				set_arg_this(target.cref());
				ret.call(myself());
			}
		}
		pc = ff().called_pc; 	
	}


	XTAL_VM_CASE(SendIfDefined_T){
		XTAL_GLOBAL_INTERPRETER_LOCK{
			const ID& sym = symbol_ex(inst.symbol_number);
			UncountedAny self = ff().self();
			UncountedAny target = ff().temp_ = pop();
			recycle_ff(pc + inst.ISIZE, inst.ordered_arg_count, inst.named_arg_count, self.cref());
			ff().called_pc = &check_unsupported_code_;
			const Class& cls = target.cref().get_class();
			set_hint(cls, sym);
			if(const Any& ret = member_cache(cls, sym, ff().self(), null)){
				set_arg_this(target.cref());
				ret.call(myself());
			}
		}
		pc = ff().called_pc; 	
	}


	XTAL_VM_CASE(SendIfDefined_AT){
		XTAL_GLOBAL_INTERPRETER_LOCK{
			const ID& sym = symbol_ex(inst.symbol_number);
			UncountedAny self = ff().self();
			UncountedAny target = ff().temp_ = pop();
			recycle_ff_args(pc + inst.ISIZE, inst.ordered_arg_count, inst.named_arg_count, self.cref());
			ff().called_pc = &check_unsupported_code_;
			const Class& cls = target.cref().get_class();
			set_hint(cls, sym);
			if(const Any& ret = member_cache(cls, sym, ff().self(), null)){
				set_arg_this(target.cref());
				ret.call(myself());
			}
		}
		pc = ff().called_pc; 	
	}

	XTAL_VM_CASE(SetMultipleLocalVariable4Direct){
		FunFrame& f = ff();
		f.variable(inst.local_variable4, pop()); 
		f.variable(inst.local_variable3, pop()); 
		f.variable(inst.local_variable2, pop()); 
		f.variable(inst.local_variable1, pop()); 
		pc += inst.ISIZE;
	}

	XTAL_VM_CASE(SetMultipleLocalVariable5Direct){
		FunFrame& f = ff();
		f.variable(inst.local_variable5, pop()); 
		f.variable(inst.local_variable4, pop()); 
		f.variable(inst.local_variable3, pop()); 
		f.variable(inst.local_variable2, pop()); 
		f.variable(inst.local_variable1, pop()); 
		pc += inst.ISIZE;
	}

	XTAL_VM_CASE(SetMultipleLocalVariable4){
		SET_LOCAL_VARIABLE(inst.local_variable4, pop()); 
		SET_LOCAL_VARIABLE(inst.local_variable3, pop()); 
		SET_LOCAL_VARIABLE(inst.local_variable2, pop()); 
		SET_LOCAL_VARIABLE(inst.local_variable1, pop()); 
		pc += inst.ISIZE;
	}

	XTAL_VM_CASE(SetMultipleLocalVariable5){
		SET_LOCAL_VARIABLE(inst.local_variable5, pop()); 
		SET_LOCAL_VARIABLE(inst.local_variable4, pop()); 
		SET_LOCAL_VARIABLE(inst.local_variable3, pop()); 
		SET_LOCAL_VARIABLE(inst.local_variable2, pop()); 
		SET_LOCAL_VARIABLE(inst.local_variable1, pop()); 
		pc += inst.ISIZE;
	}

	XTAL_VM_CASE(Throw){ 
		XTAL_GLOBAL_INTERPRETER_LOCK{
			Any except = pop();
			if(!except){
				except = last_except_.cref();
			}

			if(!except.is(builtin().member("Exception"))){
				last_except_ = append_backtrace(pc, builtin().member("RuntimeError")(except));
			}else{
				last_except_ = append_backtrace(pc, except);
			}
		}
		goto except_catch; 
	}

	XTAL_VM_CASE(ThrowUnsupportedError){ 
		XTAL_GLOBAL_INTERPRETER_LOCK{
			last_except_ = unsupported_error(ff().hint1().object_name(), ff().hint2());
		}
		goto except_catch;
	}

	XTAL_VM_CASE(ThrowNull){ 
		last_except_ = null; 
		goto except_catch; 
	}

	XTAL_VM_CASE(Assert){ 
		XTAL_GLOBAL_INTERPRETER_LOCK{
			Any expr = get(2);
			Any expr_string = get(1) ? get(1) : Any("");
			Any message = get() ? get() : Any("");
			downsize(3);
			if(!expr){
				last_except_ = builtin().member("AssertionFailed")(message, expr_string);
				goto except_catch; 
			}
		}
		pc += inst.ISIZE;
	}

	XTAL_VM_NODEFAULT;
}

except_catch:
	pc = CATCH_BODY(pc, stack_size, fun_frames_size);
	if(pc){
		goto begin;
	}

	if(last_except_.cref()){
		XTAL_THROW(last_except_.cref());
	}
}
	
const inst_t* VMachineImpl::ARRAY_APPEND(const inst_t* pc){
	XTAL_GLOBAL_INTERPRETER_LOCK{
		cast<Array*>(get(1))->push_back(get()); 
		downsize(1);
	}
	return pc + 1;
}
	
const inst_t* VMachineImpl::MAP_INSERT(const inst_t* pc){
	XTAL_GLOBAL_INTERPRETER_LOCK{
		cast<Map*>(get(2))->set_at(get(1), get()); 
		downsize(2);	
	}
	return pc + 1;
}

const inst_t* VMachineImpl::SET_NAME(const inst_t* pc){
	XTAL_VM_DEF(SetName);
	XTAL_GLOBAL_INTERPRETER_LOCK{
		get().set_object_name(symbol(inst.symbol_number), 1, null);
	}
	return pc + inst.ISIZE;
}

const inst_t* VMachineImpl::CLASS_BEGIN(const inst_t* pc){
	XTAL_VM_DEF(ClassBegin);
	XTAL_GLOBAL_INTERPRETER_LOCK{
		FrameCore* p = code().frame_core(inst.core_number); 
		Class cp = new_xclass(decolonize(), code(), p);

		int_t n = inst.mixins;
		for(int_t i = 0; i<n; ++i){
			cp.inherit(pop());
		}

		ff().outer(decolonize());
		push_ff(pc + inst.ISIZE, 0, 0, 0, cp);
		ff().fun(prev_fun());

		ff().outer(cp);
		ff().scopes.push(0); 

		return pc + inst.ISIZE;
	}
	return 0;
}

const inst_t* VMachineImpl::CLASS_END(const inst_t* pc){
	push(ff().outer());
	ff().outer(ff().outer().outer());
	ff().scopes.downsize(1);
	pop_ff();
	return pc+1;
}

const inst_t* VMachineImpl::BLOCK_END(const inst_t* pc){
	if(ff().scopes.top()){ 
		ff().variables_.downsize(ff().scopes.top()->variable_size);
	}else{
		ff().outer(ff().outer().outer()); 
	}
	ff().scopes.downsize(1);
	return pc+1;
}

const inst_t* VMachineImpl::TRY_BEGIN(const inst_t* pc){
	XTAL_VM_DEF(TryBegin);
	ExceptFrame& ef = except_frames_.push();
	
	ef.core = &code().impl()->except_core_table_[inst.core_number];
	ef.fun_frame_count = fun_frames_.size();
	ef.scope_count = ff().scopes.size();
	ef.stack_count = stack_size();
	
	return pc + inst.ISIZE;
}

void VMachineImpl::SET_LOCAL_VARIABLE(int_t pos, const Any& value){
	int_t variables_size = ff().variables_.size();
	if(pos<variables_size){
		ff().variable(pos, value);
		return;
	}	
	
	pos-=variables_size;
	const Frame* outer = &ff().outer();
	
	XTAL_GLOBAL_INTERPRETER_LOCK{
		while(1){
			variables_size = outer->impl()->block_size();
			if(pos<variables_size){
				outer->impl()->set_member_direct(pos, value);
				return;
			}
			pos-=variables_size;
			outer = &outer->impl()->outer();
		}
	}
}

const Any& VMachineImpl::LOCAL_VARIABLE(int_t pos){
	int_t variables_size = ff().variables_.size();
	if(pos<variables_size){
		return ff().variable(pos);
	}
	
	pos-=variables_size;
	const Frame* outer = &ff().outer();
	
	XTAL_GLOBAL_INTERPRETER_LOCK{
		for(;;){
			variables_size = outer->impl()->block_size();
			if(pos<variables_size){
				return outer->impl()->member_direct(pos);
			}
			pos-=variables_size;
			outer = &outer->impl()->outer();
		}
	}
	return null;
}

const inst_t* VMachineImpl::GLOBAL_VARIABLE(const inst_t* pc){
	XTAL_VM_DEF(GlobalVariable);
	XTAL_GLOBAL_INTERPRETER_LOCK{
		if(const Any& ret = code().toplevel().member(symbol(inst.symbol_number))){
			push(ret);
		}else{
			XTAL_THROW(unsupported_error("toplevel", symbol(inst.symbol_number)));
		}
	}
	return pc + inst.ISIZE;
}

const inst_t* VMachineImpl::SET_GLOBAL_VARIABLE(const inst_t* pc){
	XTAL_VM_DEF(GlobalVariable);
	XTAL_GLOBAL_INTERPRETER_LOCK{
		code().toplevel().set_member(symbol(inst.symbol_number), pop(), null);
	}
	return pc + inst.ISIZE;
}

const inst_t* VMachineImpl::DEFINE_GLOBAL_VARIABLE(const inst_t* pc){
	XTAL_VM_DEF(GlobalVariable);
	XTAL_GLOBAL_INTERPRETER_LOCK{
		code().toplevel().def(symbol(inst.symbol_number), pop(), KIND_PUBLIC, null);
	}
	return pc + inst.ISIZE;
}

const inst_t* VMachineImpl::ONCE(const inst_t* pc){
	XTAL_VM_DEF(Once);
	const Any& ret = code().impl()->value(inst.value_number);
	if(!ret.raweq(nop)){
		push(ret);
		return pc + inst.address;
	}
	return pc + inst.ISIZE;
}
	
const inst_t* VMachineImpl::MEMBER(const inst_t* pc){
	XTAL_VM_DEF(Member);
	XTAL_GLOBAL_INTERPRETER_LOCK{
		const ID& name = symbol_ex(inst.symbol_number);
		const Any& target = get();
		if(const Any& ret = member_cache(target, name, ff().self(), null)){
			set(ret);
		}else{
			XTAL_THROW(unsupported_error(target.object_name(), name));
		}
	}
	return pc + inst.ISIZE; 
}

const inst_t* VMachineImpl::MEMBER_IF_DEFINED(const inst_t* pc){
	XTAL_VM_DEF(MemberIfDefined);
	XTAL_GLOBAL_INTERPRETER_LOCK{
		const ID& name = symbol_ex(inst.symbol_number);
		const Any& target = get();
		if(const Any& ret = member_cache(target, name, ff().self(), null)){
			set(ret);
		}else{
			set(nop);
		}
	}
	return pc + inst.ISIZE; 
}

const inst_t* VMachineImpl::DEFINE_MEMBER(const inst_t* pc){
	XTAL_VM_DEF(DefineMember);
	XTAL_GLOBAL_INTERPRETER_LOCK{
		const ID& name = symbol_ex(inst.symbol_number);
		const Any& value = get();
		const Any& target = get(1);
		target.def(name, value); 
		downsize(2);
	}
	return pc + inst.ISIZE; 
}

const inst_t* VMachineImpl::AT(const inst_t* pc){ 
	XTAL_GLOBAL_INTERPRETER_LOCK{
		Any idx = pop();
		Any target = pop();
		inner_setup_call(pc+1, 1, idx);
		target.send(Xid(op_at), myself());
	}
	return ff().called_pc; 
}

const inst_t* VMachineImpl::SET_AT(const inst_t* pc){ 
	XTAL_GLOBAL_INTERPRETER_LOCK{
		Any idx = pop();
		Any target = pop();
		Any value = pop();
		inner_setup_call(pc+1, 0, idx, value);
		target.send(Xid(op_set_at), myself());
	}
	return ff().called_pc;
}

const inst_t* VMachineImpl::PUSH_ARRAY(const inst_t* pc){
	XTAL_VM_DEF(MakeArray);
	XTAL_GLOBAL_INTERPRETER_LOCK{
		int_t size = inst.size;
		Array ary(size);
		for(int_t i = 0; i<size; ++i){
			ary.set_at(i, get(size-1-i));
		}
		downsize(size);
		push(ary);
	}
	return pc + inst.ISIZE;
}
	
const inst_t* VMachineImpl::PUSH_MAP(const inst_t* pc){
	XTAL_VM_DEF(MakeMap);
	XTAL_GLOBAL_INTERPRETER_LOCK{
		int_t size = inst.size;
		Map map;
		for(int_t i = 0; i<size; ++i){
			map.set_at(get(size*2-i*2-0-1), get(size*2-i*2-1-1));
		}
		downsize(size*2);
		push(map);
	}
	return pc + inst.ISIZE;
}
	
const inst_t* VMachineImpl::PUSH_FUN(const inst_t* pc){
	XTAL_VM_DEF(MakeFun);
	int_t type = inst.type, table_n = inst.core_number, end = inst.address;
	XTAL_GLOBAL_INTERPRETER_LOCK{
		switch(type){
			XTAL_NODEFAULT;
			
			XTAL_CASE(KIND_FUN){ 
				push(Fun(decolonize(), ff().self(), code(), code().fun_core(table_n))); 
			}

			XTAL_CASE(KIND_LAMBDA){ 
				push(Lambda(decolonize(), ff().self(), code(), code().fun_core(table_n))); 
			}

			XTAL_CASE(KIND_METHOD){ 
				push(Method(decolonize(), code(), code().fun_core(table_n))); 
			}

			XTAL_CASE(KIND_FIBER){ 
				push(Fiber(decolonize(), ff().self(), code(), code().fun_core(table_n)));
			}
		}
	}
	return pc+end;
}

const inst_t* VMachineImpl::PUSH_ARGS(const inst_t* pc){
	push(fun_frames_[0].arguments());
	return pc + 1;
}

void VMachineImpl::YIELD(const inst_t* pc){
	XTAL_VM_DEF(Yield);
	yield_result_count_ = inst.results;	
	if(ff().yieldable){
		resume_pc_ = pc + inst.ISIZE;
	}else{
		downsize(yield_result_count_);
		XTAL_GLOBAL_INTERPRETER_LOCK{
			XTAL_THROW(builtin().member("YieldError")(Xt("Xtal Runtime Error 1012"))); 
		}
	}
}
const inst_t* VMachineImpl::POS(const inst_t* pc){
	switch(get().type()){XTAL_DEFAULT;
		XTAL_CASE(TYPE_INT){ return pc+1; }
		XTAL_CASE(TYPE_FLOAT){ return pc+1; }
	}
	return send1(pc, Xid(op_pos));
}

const inst_t* VMachineImpl::NEG(const inst_t* pc){
	switch(get().type()){XTAL_DEFAULT;
		XTAL_CASE(TYPE_INT){ set(UncountedAny(-get().ivalue()).cref()); return pc+1; }
		XTAL_CASE(TYPE_FLOAT){ set(UncountedAny(-get().fvalue()).cref()); return pc+1; }
	}
	return send1(pc, Xid(op_neg));
}

const inst_t* VMachineImpl::COM(const inst_t* pc){
	switch(get().type()){XTAL_DEFAULT;
		XTAL_CASE(TYPE_INT){ set(UncountedAny(~get().ivalue()).cref()); return pc; }
	}
	return send1(pc, Xid(op_com));
}

const inst_t* VMachineImpl::CLONE(const inst_t* pc){
	switch(get().type()){XTAL_DEFAULT;
		XTAL_CASE(TYPE_INT){ return pc+1; }
		XTAL_CASE(TYPE_FLOAT){ return pc+1; }
	}
	return send1(pc, Xid(op_clone));
}

const inst_t* VMachineImpl::ADD(const inst_t* pc){ 
	switch(get(1).type()){XTAL_DEFAULT;
		XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).ivalue() + get().ivalue()).cref()); downsize(1); return pc+1; }
			XTAL_CASE(TYPE_FLOAT){ set(1, UncountedAny(get(1).ivalue() + get().fvalue()).cref()); downsize(1); return pc+1; }
		}}
		XTAL_CASE(TYPE_FLOAT){switch(get().type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).fvalue() + get().ivalue()).cref()); downsize(1); return pc+1; }
			XTAL_CASE(TYPE_FLOAT){ set(1, UncountedAny(get(1).fvalue() + get().fvalue()).cref()); downsize(1); return pc+1; }
		}}
	}
	return send2(pc, Xid(op_add));
}

const inst_t* VMachineImpl::SUB(const inst_t* pc){ 
	switch(get(1).type()){XTAL_DEFAULT;
		XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).ivalue() - get().ivalue()).cref()); downsize(1); return pc+1; }
			XTAL_CASE(TYPE_FLOAT){ set(1, UncountedAny(get(1).ivalue() - get().fvalue()).cref()); downsize(1); return pc+1; }
		}}
		XTAL_CASE(TYPE_FLOAT){switch(get().type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).fvalue() - get().ivalue()).cref()); downsize(1); return pc+1; }
			XTAL_CASE(TYPE_FLOAT){ set(1, UncountedAny(get(1).fvalue() - get().fvalue()).cref()); downsize(1); return pc+1; }
		}}
	}
	return send2(pc, Xid(op_sub));
}

const inst_t* VMachineImpl::CAT(const inst_t* pc){ 
	return send2(pc, Xid(op_cat));
}

const inst_t* VMachineImpl::MUL(const inst_t* pc){ 
	switch(get(1).type()){XTAL_DEFAULT;
		XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).ivalue() * get().ivalue()).cref()); downsize(1); return pc+1; }
			XTAL_CASE(TYPE_FLOAT){ set(1, UncountedAny(get(1).ivalue() * get().fvalue()).cref()); downsize(1); return pc+1; }
		}}
		XTAL_CASE(TYPE_FLOAT){switch(get().type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).fvalue() * get().ivalue()).cref()); downsize(1); return pc+1; }
			XTAL_CASE(TYPE_FLOAT){ set(1, UncountedAny(get(1).fvalue() * get().fvalue()).cref()); downsize(1); return pc+1; }
		}}
	}
	return send2(pc, Xid(op_mul));
}

const inst_t* VMachineImpl::DIV(const inst_t* pc){ 
	switch(get(1).type()){XTAL_DEFAULT;
		XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).ivalue() / get().ivalue()).cref()); downsize(1); return pc+1; }
			XTAL_CASE(TYPE_FLOAT){ set(1, UncountedAny(get(1).ivalue() / get().fvalue()).cref()); downsize(1); return pc+1; }
		}}
		XTAL_CASE(TYPE_FLOAT){switch(get().type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).fvalue() / get().ivalue()).cref()); downsize(1); return pc+1; }
			XTAL_CASE(TYPE_FLOAT){ set(1, UncountedAny(get(1).fvalue() / get().fvalue()).cref()); downsize(1); return pc+1; }
		}}
	}
	return send2(pc, Xid(op_div));
}

const inst_t* VMachineImpl::MOD(const inst_t* pc){ 
	using namespace std;
	switch(get(1).type()){XTAL_DEFAULT;
		XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).ivalue() % get().ivalue()).cref()); downsize(1); return pc+1; }
			XTAL_CASE(TYPE_FLOAT){ set(1, UncountedAny(fmodf((float_t)get(1).ivalue(), get().fvalue())).cref()); downsize(1); return pc+1; }
		}}
		XTAL_CASE(TYPE_FLOAT){switch(get().type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ set(1, UncountedAny(fmodf(get(1).fvalue(),(float_t)get().ivalue())).cref()); downsize(1); return pc+1; }
			XTAL_CASE(TYPE_FLOAT){ set(1, UncountedAny(fmodf(get(1).fvalue(), get().fvalue())).cref()); downsize(1); return pc+1; }
		}}
	}
	return send2(pc, Xid(op_mod));
}

const inst_t* VMachineImpl::AND(const inst_t* pc){ 
	switch(get(1).type()){XTAL_DEFAULT;
		XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).ivalue() & get().ivalue()).cref()); downsize(1); return pc+1; }
		}}
	}
	return send2(pc, Xid(op_and));
}

const inst_t* VMachineImpl::OR(const inst_t* pc){ 
	switch(get(1).type()){XTAL_DEFAULT;
		XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).ivalue() | get().ivalue()).cref()); downsize(1); return pc+1; }
		}}
	}
	return send2(pc, Xid(op_or));
}

const inst_t* VMachineImpl::XOR(const inst_t* pc){ 
	switch(get(1).type()){XTAL_DEFAULT;
		XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).ivalue() ^ get().ivalue()).cref()); downsize(1); return pc+1; }
		}}
	}
	return send2(pc, Xid(op_xor));
}

const inst_t* VMachineImpl::SHR(const inst_t* pc){ 
	switch(get(1).type()){XTAL_DEFAULT;
		XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).ivalue() >> get().ivalue()).cref()); downsize(1); return pc+1; }
		}}
	}
	return send2(pc, Xid(op_shr));
}

const inst_t* VMachineImpl::USHR(const inst_t* pc){ 
	switch(get(1).type()){XTAL_DEFAULT;
		XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ set(1, UncountedAny((int_t)((uint_t)get(1).ivalue() >> get().ivalue())).cref()); downsize(1); return pc+1; }
		}}
	}
	return send2(pc, Xid(op_ushr));
}

const inst_t* VMachineImpl::SHL(const inst_t* pc){ 
	switch(get(1).type()){XTAL_DEFAULT;
		XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).ivalue() << get().ivalue()).cref()); downsize(1); return pc+1; }
		}}
	}
	return send2(pc, Xid(op_shl));
}

const inst_t* VMachineImpl::EQ(const inst_t* pc){ 
	switch(get(1).type()){XTAL_DEFAULT;
		XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).ivalue() == get().ivalue()).cref()); downsize(1); return pc+1; }
			XTAL_CASE(TYPE_FLOAT){ set(1, UncountedAny(get(1).ivalue() == get().fvalue()).cref()); downsize(1); return pc+1; }
		}}
		XTAL_CASE(TYPE_FLOAT){switch(get().type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).fvalue() == get().ivalue()).cref()); downsize(1); return pc+1; }
			XTAL_CASE(TYPE_FLOAT){ set(1, UncountedAny(get(1).fvalue() == get().fvalue()).cref()); downsize(1); return pc+1; }
		}}
	}
	return send2(pc, Xid(op_eq));
}

const inst_t* VMachineImpl::NE(const inst_t* pc){ 
	switch(get(1).type()){XTAL_DEFAULT;
		XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).ivalue() != get().ivalue()).cref()); downsize(1); return pc+1+1; }
			XTAL_CASE(TYPE_FLOAT){ set(1, UncountedAny(get(1).ivalue() != get().fvalue()).cref()); downsize(1); return pc+1+1; }
		}}
		XTAL_CASE(TYPE_FLOAT){switch(get().type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).fvalue() != get().ivalue()).cref()); downsize(1); return pc+1+1; }
			XTAL_CASE(TYPE_FLOAT){ set(1, UncountedAny(get(1).fvalue() != get().fvalue()).cref()); downsize(1); return pc+1+1; }
		}}
	}
	return send2(pc, Xid(op_eq));
}

const inst_t* VMachineImpl::LT(const inst_t* pc){ 
	switch(get(1).type()){XTAL_DEFAULT;
		XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).ivalue() < get().ivalue()).cref()); downsize(1); return pc+1; }
			XTAL_CASE(TYPE_FLOAT){ set(1, UncountedAny(get(1).ivalue() < get().fvalue()).cref()); downsize(1); return pc+1; }
		}}
		XTAL_CASE(TYPE_FLOAT){switch(get().type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).fvalue() < get().ivalue()).cref()); downsize(1); return pc+1; }
			XTAL_CASE(TYPE_FLOAT){ set(1, UncountedAny(get(1).fvalue() < get().fvalue()).cref()); downsize(1); return pc+1; }
		}}
	}
	return send2(pc, Xid(op_lt));
}

const inst_t* VMachineImpl::GT(const inst_t* pc){ 
	switch(get(1).type()){XTAL_DEFAULT;
		XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).ivalue() > get().ivalue()).cref()); downsize(1); return pc+1; }
			XTAL_CASE(TYPE_FLOAT){ set(1, UncountedAny(get(1).ivalue() > get().fvalue()).cref()); downsize(1); return pc+1; }
		}}
		XTAL_CASE(TYPE_FLOAT){switch(get().type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).fvalue() > get().ivalue()).cref()); downsize(1); return pc+1; }
			XTAL_CASE(TYPE_FLOAT){ set(1, UncountedAny(get(1).fvalue() > get().fvalue()).cref()); downsize(1); return pc+1; }
		}}
	}
	return send2r(pc, Xid(op_lt));
}

const inst_t* VMachineImpl::LE(const inst_t* pc){ 
	switch(get(1).type()){XTAL_DEFAULT;
		XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).ivalue() <= get().ivalue()).cref()); downsize(1); return pc+1+1; }
			XTAL_CASE(TYPE_FLOAT){ set(1, UncountedAny(get(1).ivalue() <= get().fvalue()).cref()); downsize(1); return pc+1+1; }
		}}
		XTAL_CASE(TYPE_FLOAT){switch(get().type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).fvalue() <= get().ivalue()).cref()); downsize(1); return pc+1+1; }
			XTAL_CASE(TYPE_FLOAT){ set(1, UncountedAny(get(1).fvalue() <= get().fvalue()).cref()); downsize(1); return pc+1+1; }
		}}
	}
	return send2r(pc, Xid(op_lt));
}

const inst_t* VMachineImpl::GE(const inst_t* pc){ 
	switch(get(1).type()){XTAL_DEFAULT;
		XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).ivalue() >= get().ivalue()).cref()); downsize(1); return pc+1+1; }
			XTAL_CASE(TYPE_FLOAT){ set(1, UncountedAny(get(1).ivalue() >= get().fvalue()).cref()); downsize(1); return pc+1+1; }
		}}
		XTAL_CASE(TYPE_FLOAT){switch(get().type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).fvalue() >= get().ivalue()).cref()); downsize(1); return pc+1+1; }
			XTAL_CASE(TYPE_FLOAT){ set(1, UncountedAny(get(1).fvalue() >= get().fvalue()).cref()); downsize(1); return pc+1+1; }
		}}
	}
	return send2(pc, Xid(op_lt));
}

const inst_t* VMachineImpl::RAW_EQ(const inst_t* pc){ 
	set(1, UncountedAny(get(1).raweq(get())).cref());
	downsize(1);
	return pc + 1;
}

const inst_t* VMachineImpl::RAW_NE(const inst_t* pc){ 
	set(1, UncountedAny(get(1).rawne(get())).cref());
	downsize(1);
	return pc + 1;
}

const inst_t* VMachineImpl::IS(const inst_t* pc){ 
	set(1, UncountedAny(get(1).is(get())).cref());
	downsize(1);
	return pc + 1;
}

const inst_t* VMachineImpl::NIS(const inst_t* pc){ 
	set(1, UncountedAny(!get(1).is(get())).cref());
	downsize(1);
	return pc + 1;
}

const inst_t* VMachineImpl::IF_EQ(const inst_t* pc){
	XTAL_VM_DEF(IfEq);
	switch(get(1).type()){XTAL_DEFAULT;
		XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ pc = get(1).ivalue() == get().ivalue() ? pc+inst.ISIZE+InstIf::ISIZE : pc+inst.address; downsize(2); return pc; }
			XTAL_CASE(TYPE_FLOAT){ pc = get(1).ivalue() == get().fvalue() ? pc+inst.ISIZE+InstIf::ISIZE : pc+inst.address; downsize(2); return pc; }
		}}
		XTAL_CASE(TYPE_FLOAT){switch(get().type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ pc = get(1).fvalue() == get().ivalue() ? pc+inst.ISIZE+InstIf::ISIZE : pc+inst.address; downsize(2); return pc; }
			XTAL_CASE(TYPE_FLOAT){ pc = get(1).fvalue() == get().fvalue() ? pc+inst.ISIZE+InstIf::ISIZE : pc+inst.address; downsize(2); return pc; }
		}}
	}
	return send2(pc, Xid(op_eq), inst.ISIZE);
}

const inst_t* VMachineImpl::IF_NE(const inst_t* pc){
	XTAL_VM_DEF(IfNe);
	switch(get(1).type()){XTAL_DEFAULT;
		XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ pc = get(1).ivalue() != get().ivalue() ? pc+inst.ISIZE+InstNot::ISIZE+InstIf::ISIZE : pc+inst.address; downsize(2); return pc; }
			XTAL_CASE(TYPE_FLOAT){ pc = get(1).ivalue() != get().fvalue() ? pc+inst.ISIZE+InstNot::ISIZE+InstIf::ISIZE : pc+inst.address; downsize(2); return pc; }
		}}
		XTAL_CASE(TYPE_FLOAT){switch(get().type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ pc = get(1).fvalue() != get().ivalue() ? pc+inst.ISIZE+InstNot::ISIZE+InstIf::ISIZE : pc+inst.address; downsize(2); return pc; }
			XTAL_CASE(TYPE_FLOAT){ pc = get(1).fvalue() != get().fvalue() ? pc+inst.ISIZE+InstNot::ISIZE+InstIf::ISIZE : pc+inst.address; downsize(2); return pc; }
		}}
	}
	return send2(pc, Xid(op_eq), inst.ISIZE);
}

const inst_t* VMachineImpl::IF_LT(const inst_t* pc){ 
	XTAL_VM_DEF(IfLt);
	switch(get(1).type()){XTAL_DEFAULT;
		XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ pc = get(1).ivalue() < get().ivalue() ? pc+inst.ISIZE+InstIf::ISIZE : pc+inst.address; downsize(2); return pc; }
			XTAL_CASE(TYPE_FLOAT){ pc = get(1).ivalue() < get().fvalue() ? pc+inst.ISIZE+InstIf::ISIZE : pc+inst.address; downsize(2); return pc; }
		}}
		XTAL_CASE(TYPE_FLOAT){switch(get().type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ pc = get(1).fvalue() < get().ivalue() ? pc+inst.ISIZE+InstIf::ISIZE : pc+inst.address; downsize(2); return pc; }
			XTAL_CASE(TYPE_FLOAT){ pc = get(1).fvalue() < get().fvalue() ? pc+inst.ISIZE+InstIf::ISIZE : pc+inst.address; downsize(2); return pc; }
		}}
	}
	return send2(pc, Xid(op_lt), inst.ISIZE);
}

const inst_t* VMachineImpl::IF_GT(const inst_t* pc){
	XTAL_VM_DEF(IfGt);
	switch(get(1).type()){XTAL_DEFAULT;
		XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ pc = get(1).ivalue() > get().ivalue() ? pc+inst.ISIZE+InstIf::ISIZE : pc+inst.address; downsize(2); return pc; }
			XTAL_CASE(TYPE_FLOAT){ pc = get(1).ivalue() > get().fvalue() ? pc+inst.ISIZE+InstIf::ISIZE : pc+inst.address; downsize(2); return pc; }
		}}
		XTAL_CASE(TYPE_FLOAT){switch(get().type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ pc = get(1).fvalue() > get().ivalue() ? pc+inst.ISIZE+InstIf::ISIZE : pc+inst.address; downsize(2); return pc; }
			XTAL_CASE(TYPE_FLOAT){ pc = get(1).fvalue() > get().fvalue() ? pc+inst.ISIZE+InstIf::ISIZE : pc+inst.address; downsize(2); return pc; }
		}}
	}
	return send2r(pc, Xid(op_lt), inst.ISIZE);
}

const inst_t* VMachineImpl::IF_LE(const inst_t* pc){
	XTAL_VM_DEF(IfLe);
	switch(get(1).type()){XTAL_DEFAULT;
		XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ pc = get(1).ivalue() <= get().ivalue() ? pc+inst.ISIZE+InstNot::ISIZE+InstIf::ISIZE : pc+inst.address; downsize(2); return pc; }
			XTAL_CASE(TYPE_FLOAT){ pc = get(1).ivalue() <= get().fvalue() ? pc+inst.ISIZE+InstNot::ISIZE+InstIf::ISIZE : pc+inst.address; downsize(2); return pc; }
		}}
		XTAL_CASE(TYPE_FLOAT){switch(get().type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ pc = get(1).fvalue() <= get().ivalue() ? pc+inst.ISIZE+InstNot::ISIZE+InstIf::ISIZE : pc+inst.address; downsize(2); return pc; }
			XTAL_CASE(TYPE_FLOAT){ pc = get(1).fvalue() <= get().fvalue() ? pc+inst.ISIZE+InstNot::ISIZE+InstIf::ISIZE : pc+inst.address; downsize(2); return pc; }
		}}
	}
	return send2r(pc, Xid(op_lt), inst.ISIZE);
}

const inst_t* VMachineImpl::IF_GE(const inst_t* pc){
	XTAL_VM_DEF(IfGe);
	switch(get(1).type()){XTAL_DEFAULT;
		XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ pc = get(1).ivalue() >= get().ivalue() ? pc+inst.ISIZE+InstNot::ISIZE+InstIf::ISIZE : pc+inst.address; downsize(2); return pc; }
			XTAL_CASE(TYPE_FLOAT){ pc = get(1).ivalue() >= get().fvalue() ? pc+inst.ISIZE+InstNot::ISIZE+InstIf::ISIZE : pc+inst.address; downsize(2); return pc; }
		}}
		XTAL_CASE(TYPE_FLOAT){switch(get().type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ pc = get(1).fvalue() >= get().ivalue() ? pc+inst.ISIZE+InstNot::ISIZE+InstIf::ISIZE : pc+inst.address; downsize(2); return pc; }
			XTAL_CASE(TYPE_FLOAT){ pc = get(1).fvalue() >= get().fvalue() ? pc+inst.ISIZE+InstNot::ISIZE+InstIf::ISIZE : pc+inst.address; downsize(2); return pc; }
		}}
	}
	return send2(pc, Xid(op_lt), inst.ISIZE);
}

const inst_t* VMachineImpl::IF_RAW_EQ(const inst_t* pc){
	XTAL_VM_DEF(IfRawEq);
	pc += get(1).raweq(get()) ? inst.ISIZE : inst.address;
	downsize(2);
	return pc;
}

const inst_t* VMachineImpl::IF_RAW_NE(const inst_t* pc){
	XTAL_VM_DEF(IfRawEq);
	pc += !get(1).raweq(get()) ? inst.ISIZE : inst.address;
	downsize(2);
	return pc;
}

const inst_t* VMachineImpl::IF_IS(const inst_t* pc){
	XTAL_VM_DEF(IfRawEq);
	pc += get(1).is(get()) ? inst.ISIZE : inst.address;
	downsize(2);
	return pc;
}

const inst_t* VMachineImpl::IF_NIS(const inst_t* pc){
	XTAL_VM_DEF(IfRawEq);
	pc += !get(1).is(get()) ? inst.ISIZE : inst.address;
	downsize(2);
	return pc;
}

const inst_t* VMachineImpl::INC(const inst_t* pc){
	switch(get().type()){XTAL_DEFAULT;
		XTAL_CASE(TYPE_INT){ set(UncountedAny(get().ivalue()+1).cref()); return pc+1; }
		XTAL_CASE(TYPE_FLOAT){ set(UncountedAny(get().fvalue()+1).cref()); return pc+1; }
	}
	return send1(pc, Xid(op_inc));
}

const inst_t* VMachineImpl::DEC(const inst_t* pc){
	switch(get().type()){XTAL_DEFAULT;
		XTAL_CASE(TYPE_INT){ set(UncountedAny(get().ivalue()-1).cref()); return pc+1; }
		XTAL_CASE(TYPE_FLOAT){ set(UncountedAny(get().fvalue()-1).cref()); return pc+1; }
	}
	return send1(pc, Xid(op_dec));
}

const inst_t* VMachineImpl::LOCAL_VARIABLE_INC(const inst_t* pc){
	XTAL_VM_DEF(LocalVariableInc);
	UncountedAny a = LOCAL_VARIABLE(inst.number);
	switch(a.type()){XTAL_DEFAULT;
		XTAL_CASE(TYPE_INT){ SET_LOCAL_VARIABLE(inst.number, UncountedAny(a.ivalue()+1).cref()); return pc + inst.ISIZE + InstSetLocalVariable2Byte::ISIZE; }
		XTAL_CASE(TYPE_FLOAT){ SET_LOCAL_VARIABLE(inst.number, UncountedAny(a.fvalue()+1).cref()); return pc + inst.ISIZE + InstSetLocalVariable2Byte::ISIZE; }
	}
	a.cref().send(Xid(op_inc), inner_setup_call(pc + inst.ISIZE, 1));
	return ff().called_pc;
}

const inst_t* VMachineImpl::LOCAL_VARIABLE_DEC(const inst_t* pc){
	XTAL_VM_DEF(LocalVariableDec);
	UncountedAny a = LOCAL_VARIABLE(inst.number);
	switch(a.type()){XTAL_DEFAULT;
		XTAL_CASE(TYPE_INT){ SET_LOCAL_VARIABLE(inst.number, UncountedAny(a.ivalue()-1).cref()); return pc + inst.ISIZE + InstSetLocalVariable2Byte::ISIZE; }
		XTAL_CASE(TYPE_FLOAT){ SET_LOCAL_VARIABLE(inst.number, UncountedAny(a.fvalue()-1).cref()); return pc + inst.ISIZE + InstSetLocalVariable2Byte::ISIZE; }
	}
	a.cref().send(Xid(op_dec), inner_setup_call(pc + inst.ISIZE, 1));
	return ff().called_pc;
}

const inst_t* VMachineImpl::LocalVariableIncDirect(const inst_t* pc){
	XTAL_VM_DEF(LocalVariableIncDirect);
	UncountedAny& a = ff().variables_[inst.number];
	switch(a.type()){XTAL_DEFAULT;
		XTAL_CASE(TYPE_INT){ a = UncountedAny(a.ivalue()+1); return pc + inst.ISIZE + InstSetLocalVariable1ByteDirect::ISIZE; }
		XTAL_CASE(TYPE_FLOAT){ a = UncountedAny(a.fvalue()+1); return pc + inst.ISIZE + InstSetLocalVariable1ByteDirect::ISIZE; }
	}
	a.cref().send(Xid(op_inc), inner_setup_call(pc + inst.ISIZE, 1));
	return ff().called_pc;
}

const inst_t* VMachineImpl::LocalVariableDecDirect(const inst_t* pc){
	XTAL_VM_DEF(LocalVariableDecDirect);
	UncountedAny& a = ff().variables_[inst.number];
	switch(a.type()){XTAL_DEFAULT;
		XTAL_CASE(TYPE_INT){ a = UncountedAny(a.ivalue()-1); return pc + inst.ISIZE + InstSetLocalVariable1ByteDirect::ISIZE; }
		XTAL_CASE(TYPE_FLOAT){ a = UncountedAny(a.fvalue()-1); return pc + inst.ISIZE + InstSetLocalVariable1ByteDirect::ISIZE; }
	}
	a.cref().send(Xid(op_dec), inner_setup_call(pc + inst.ISIZE, 1));
	return ff().called_pc;
}

const inst_t* VMachineImpl::ADD_ASSIGN(const inst_t* pc){ 
	switch(get(1).type()){XTAL_DEFAULT;
		XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).ivalue() + get().ivalue()).cref()); downsize(1); return pc+1; }
			XTAL_CASE(TYPE_FLOAT){ set(1, UncountedAny(get(1).ivalue() + get().fvalue()).cref()); downsize(1); return pc+1; }
		}}
		XTAL_CASE(TYPE_FLOAT){switch(get().type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).fvalue() + get().ivalue()).cref()); downsize(1); return pc+1; }
			XTAL_CASE(TYPE_FLOAT){ set(1, UncountedAny(get(1).fvalue() + get().fvalue()).cref()); downsize(1); return pc+1; }
		}}
	}
	return send2(pc, Xid(op_add_assign));
}

const inst_t* VMachineImpl::SUB_ASSIGN(const inst_t* pc){ 
	switch(get(1).type()){XTAL_DEFAULT;
		XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).ivalue() - get().ivalue()).cref()); downsize(1); return pc+1; }
			XTAL_CASE(TYPE_FLOAT){ set(1, UncountedAny(get(1).ivalue() - get().fvalue()).cref()); downsize(1); return pc+1; }
		}}
		XTAL_CASE(TYPE_FLOAT){switch(get().type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).fvalue() - get().ivalue()).cref()); downsize(1); return pc+1; }
			XTAL_CASE(TYPE_FLOAT){ set(1, UncountedAny(get(1).fvalue() - get().fvalue()).cref()); downsize(1); return pc+1; }
		}}
	}
	return send2(pc, Xid(op_sub_assign));
}

const inst_t* VMachineImpl::CAT_ASSIGN(const inst_t* pc){ 
	return send2(pc, Xid(op_cat_assign));
}

const inst_t* VMachineImpl::MUL_ASSIGN(const inst_t* pc){ 
	switch(get(1).type()){XTAL_DEFAULT;
		XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).ivalue() * get().ivalue()).cref()); downsize(1); return pc+1; }
			XTAL_CASE(TYPE_FLOAT){ set(1, UncountedAny(get(1).ivalue() * get().fvalue()).cref()); downsize(1); return pc+1; }
		}}
		XTAL_CASE(TYPE_FLOAT){switch(get().type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).fvalue() * get().ivalue()).cref()); downsize(1); return pc+1; }
			XTAL_CASE(TYPE_FLOAT){ set(1, UncountedAny(get(1).fvalue() * get().fvalue()).cref()); downsize(1); return pc+1; }
		}}
	}
	return send2(pc, Xid(op_mul_assign));
}

const inst_t* VMachineImpl::DIV_ASSIGN(const inst_t* pc){ 
	switch(get(1).type()){XTAL_DEFAULT;
		XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).ivalue() / get().ivalue()).cref()); downsize(1); return pc+1; }
			XTAL_CASE(TYPE_FLOAT){ set(1, UncountedAny(get(1).ivalue() / get().fvalue()).cref()); downsize(1); return pc+1; }
		}}
		XTAL_CASE(TYPE_FLOAT){switch(get().type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).fvalue() / get().ivalue()).cref()); downsize(1); return pc+1; }
			XTAL_CASE(TYPE_FLOAT){ set(1, UncountedAny(get(1).fvalue() / get().fvalue()).cref()); downsize(1); return pc+1; }
		}}
	}
	return send2(pc, Xid(op_div_assign));
}

const inst_t* VMachineImpl::MOD_ASSIGN(const inst_t* pc){ 
	switch(get(1).type()){XTAL_DEFAULT;
		XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).ivalue() % get().ivalue()).cref()); downsize(1); return pc+1; }
			XTAL_CASE(TYPE_FLOAT){ set(1, UncountedAny(fmodf((float_t)get(1).ivalue(), get().fvalue())).cref()); downsize(1); return pc+1; }
		}}
		XTAL_CASE(TYPE_FLOAT){switch(get().type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ set(1, UncountedAny(fmodf(get(1).fvalue(),(float_t)get().ivalue())).cref()); downsize(1); return pc+1; }
			XTAL_CASE(TYPE_FLOAT){ set(1, UncountedAny(fmodf(get(1).fvalue(), get().fvalue())).cref()); downsize(1); return pc+1; }
		}}
	}
	return send2(pc, Xid(op_mod_assign));
}

const inst_t* VMachineImpl::AND_ASSIGN(const inst_t* pc){ 
	switch(get(1).type()){XTAL_DEFAULT;
		XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).ivalue() & get().ivalue()).cref()); downsize(1); return pc+1; }
		}}
	}
	return send2(pc, Xid(op_and_assign));
}

const inst_t* VMachineImpl::OR_ASSIGN(const inst_t* pc){ 
	switch(get(1).type()){XTAL_DEFAULT;
		XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).ivalue() | get().ivalue()).cref()); downsize(1); return pc+1; }
		}}
	}
	return send2(pc, Xid(op_or_assign));
}

const inst_t* VMachineImpl::XOR_ASSIGN(const inst_t* pc){ 
	switch(get(1).type()){XTAL_DEFAULT;
		XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).ivalue() ^ get().ivalue()).cref()); downsize(1); return pc+1; }
		}}
	}
	return send2(pc, Xid(op_xor_assign));
}

const inst_t* VMachineImpl::SHR_ASSIGN(const inst_t* pc){ 
	switch(get(1).type()){XTAL_DEFAULT;
		XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).ivalue() >> get().ivalue()).cref()); downsize(1); return pc+1; }
		}}
	}
	return send2(pc, Xid(op_shr_assign));
}

const inst_t* VMachineImpl::USHR_ASSIGN(const inst_t* pc){ 
	switch(get(1).type()){XTAL_DEFAULT;
		XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ set(1, UncountedAny((int_t)((uint_t)get(1).ivalue() >> get().ivalue())).cref()); downsize(1); return pc+1; }
		}}
	}
	return send2(pc, Xid(op_ushr_assign));
}

const inst_t* VMachineImpl::SHL_ASSIGN(const inst_t* pc){ 
	switch(get(1).type()){XTAL_DEFAULT;
		XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).ivalue() << get().ivalue()).cref()); downsize(1); return pc+1; }
		}}
	}
	return send2(pc, Xid(op_shl_assign));
}

const inst_t* VMachineImpl::CURRENT_CONTEXT(const inst_t* pc){
	XTAL_GLOBAL_INTERPRETER_LOCK{
		push(decolonize());
	}
	return pc+1;
}

Any VMachineImpl::append_backtrace(const inst_t* pc, const Any& e){
	if(e){
		Any ep = e;
		if(!ep.is(builtin().member("Exception"))){
			ep = builtin().member("RuntimeError")(ep);
		}
		if(fun() && code()){
			if((pc != code().data()+code().size()-1)){
				ep.send("append_backtrace",
					code().source_file_name(),
					code().compliant_line_number(pc),
					fun().object_name());
			}
		}else{
			ep.send("append_backtrace",
				"?",
				"?",
				"C++ function");
		}
		return ep;
	}
	return e;
}

void VMachineImpl::hook_return(const inst_t* pc){
	if(debug::is_enabled()){
		debug_info_.set_kind(BREAKPOINT_RETURN);
		debug_info_.set_line(code().compliant_line_number(pc));
		debug_info_.set_file_name(code().source_file_name());
		debug_info_.set_fun_name(fun().object_name());
		debug_info_.set_local_variables(decolonize());

		struct guard{
			guard(){ debug::disable(); }
			~guard(){ debug::enable(); }
		} g;
	
		if(Any hook = debug::return_hook()){
			hook(debug_info_);
		}				
	}
}

const inst_t* VMachineImpl::BREAKPOINT(const inst_t* pc){
	XTAL_VM_DEF(BreakPoint);
	if(debug::is_enabled()){
		XTAL_GLOBAL_INTERPRETER_LOCK{
			int_t kind = inst.type;
			
			debug_info_.set_kind(kind);
			debug_info_.set_line(code().compliant_line_number(pc));
			debug_info_.set_file_name(code().source_file_name());
			debug_info_.set_fun_name(fun().object_name());
			debug_info_.set_local_variables(decolonize());

			struct guard{
				guard(){ debug::disable(); }
				~guard(){ debug::enable(); }
			} g;
		
			switch(kind){
				XTAL_NODEFAULT;
				
				XTAL_CASE(BREAKPOINT_LINE){
					if(Any hook = debug::line_hook()){
						hook(debug_info_);
					}
				}

				XTAL_CASE(BREAKPOINT_CALL){
					if(Any hook = debug::call_hook()){
						hook(debug_info_);
					}				
				}

				XTAL_CASE(BREAKPOINT_RETURN){
					if(Any hook = debug::return_hook()){
						hook(debug_info_);
					}				
				}
			}
		}
	}
	return pc + inst.ISIZE;
}

const inst_t* VMachineImpl::CATCH_BODY(const inst_t* pc, int_t stack_size, int_t fun_frames_size){
	XTAL_GLOBAL_INTERPRETER_LOCK{

		Any e = last_except_.cref();

		// try .. catch .. finally文で囲われていない
		if(except_frames_.empty()){
			while((size_t)fun_frames_size<fun_frames_.size()){
				hook_return(pc);
				pc = pop_ff();
				e = append_backtrace(pc, e);
			}
			resize(stack_size);
			last_except_ = e;
			return 0;
		}

		ExceptFrame& ef = except_frames_.top();

		while((size_t)ef.fun_frame_count<fun_frames_.size()){
			hook_return(pc);
			pc = pop_ff();
			e = append_backtrace(pc, e);

			if(pc==&end_code_){
				resize(stack_size);
				last_except_ = e;
				return 0;
			}
		}

		while((size_t)ef.scope_count<ff().scopes.size()){
			if(ff().scopes.top()){
				ff().variables_.downsize(ff().scopes.top()->variable_size);
			}else{
				ff().outer(ff().outer().outer());
			}
			ff().scopes.downsize(1);
		}

		stack_.downsize_n(ef.stack_count);
		if(ef.core->catch_pc && e){
			pc = ef.core->catch_pc + source();
			push(Any(ef.core->end_pc));
			push(e);
		}else{
			pc = ef.core->finally_pc + source();
			push(e);
			push(Any(code().size()-1));
		}
		except_frames_.downsize(1);
		return pc;
	}

	return 0;
}
	
}//namespace
