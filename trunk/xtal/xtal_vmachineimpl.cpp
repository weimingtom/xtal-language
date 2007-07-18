
#include "xtal.h"

#include <map>
#include <fstream>
#include <math.h>

#include "xtal_funimpl.h"
#include "xtal_vmachineimpl.h"
#include "xtal_mapimpl.h"


namespace xtal{

#ifdef __GNUC__
//#	define XTAL_VM_OPT
#endif

#ifdef XTAL_VM_OPT

#	define XTAL_VM_NODEFAULT } XTAL_ASSERT(false); XTAL_NODEFAULT
#	define XTAL_VM_FIRST_CASE(key) Label##key: { Inst##key& inst = *(Inst##key*)pc;
#	define XTAL_VM_CASE(key) } XTAL_ASSERT(false); Label##key: { typedef Inst##key Inst; Inst& inst = *(Inst*)pc;
#	define XTAL_VM_SWITCH(x) goto *label_table[x];
#	define XTAL_VM_DEF_INST(key) typedef Inst##key Inst; Inst& inst = *(Inst*)pc
#	define XTAL_VM_CONTINUE(x) pc = (x); goto *label_table[*pc]

#else

//std::ofstream log("code.txt");
//#define XTAL_VM_CASE(key) goto begin; case key: log<<(#key)<<std::endl;
//#define XTAL_VM_CASE(key) } goto begin; case Inst##key::NUMBER: { typedef Inst##key Inst; Inst& inst = *(Inst*)pc; printf("%s\n", #key);
#	define XTAL_VM_NODEFAULT } XTAL_ASSERT(false); XTAL_NODEFAULT
#	define XTAL_VM_FIRST_CASE(key) case Inst##key::NUMBER: { Inst##key& inst = *(Inst##key*)pc;
#	define XTAL_VM_CASE(key) } XTAL_ASSERT(false); case Inst##key::NUMBER: { typedef Inst##key Inst; Inst& inst = *(Inst*)pc;
#	define XTAL_VM_SWITCH(x) switch(x)
#	define XTAL_VM_DEF_INST(key) typedef Inst##key Inst; Inst& inst = *(Inst*)pc
#	define XTAL_VM_CONTINUE(x) pc = (x); goto begin

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
			start = catch_body(ff().called_pc, stack_size, fun_frames_size);

			if(start){
				goto retry;
			}

			pop_ff();
			XTAL_THROW(last_except_.cref());	
		}
#ifndef XTAL_NO_EXCEPT
		catch(...){
			last_except_ = null;
			catch_body(start, stack_size, fun_frames_size);
			pop_ff();
			throw;
		}
#endif
	}
}

void VMachineImpl::execute_inner(const inst_t* start){

#ifdef XTAL_VM_OPT

	static void* label_table[] = {

//{LABELS{{
		&&LabelNop,
		&&LabelPushNull,
		&&LabelPushNop,
		&&LabelPushTrue,
		&&LabelPushFalse,
		&&LabelPushInt1Byte,
		&&LabelPushInt2Byte,
		&&LabelPushFloat1Byte,
		&&LabelPushFloat2Byte,
		&&LabelPushCallee,
		&&LabelPushArgs,
		&&LabelPushThis,
		&&LabelPushCurrentContext,
		&&LabelPop,
		&&LabelDup,
		&&LabelInsert1,
		&&LabelInsert2,
		&&LabelInsert3,
		&&LabelAdjustResult,
		&&LabelIf,
		&&LabelUnless,
		&&LabelGoto,
		&&LabelLocalVariableInc,
		&&LabelLocalVariableDec,
		&&LabelLocalVariableIncDirect,
		&&LabelLocalVariableDecDirect,
		&&LabelLocalVariable1ByteDirect,
		&&LabelLocalVariable1Byte,
		&&LabelLocalVariable2Byte,
		&&LabelSetLocalVariable1ByteDirect,
		&&LabelSetLocalVariable1Byte,
		&&LabelSetLocalVariable2Byte,
		&&LabelInstanceVariable,
		&&LabelSetInstanceVariable,
		&&LabelCleanupCall,
		&&LabelReturn0,
		&&LabelReturn1,
		&&LabelReturn2,
		&&LabelReturn,
		&&LabelYield,
		&&LabelExit,
		&&LabelValue,
		&&LabelSetValue,
		&&LabelCheckUnsupported,
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
		&&LabelBlockBegin,
		&&LabelBlockEnd,
		&&LabelBlockBeginDirect,
		&&LabelBlockEndDirect,
		&&LabelTryBegin,
		&&LabelTryEnd,
		&&LabelPushGoto,
		&&LabelPopGoto,
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
		&&LabelGlobalVariable,
		&&LabelSetGlobalVariable,
		&&LabelDefineGlobalVariable,
		&&LabelMember,
		&&LabelMemberIfDefined,
		&&LabelDefineMember,
		&&LabelDefineClassMember,
		&&LabelSetName,
		&&LabelSetMultipleLocalVariable2Direct,
		&&LabelSetMultipleLocalVariable3Direct,
		&&LabelSetMultipleLocalVariable4Direct,
		&&LabelOnce,
		&&LabelClassBegin,
		&&LabelClassEnd,
		&&LabelMakeArray,
		&&LabelArrayAppend,
		&&LabelMakeMap,
		&&LabelMapInsert,
		&&LabelMakeFun,
		&&LabelMakeInstanceVariableAccessor,
		&&LabelThrow,
		&&LabelThrowUnsupportedError,
		&&LabelThrowNull,
		&&LabelAssert,
		&&LabelBreakPoint,
		&&LabelMAX,
//}}LABELS}

	};
#else

#endif

	const inst_t* pc = start;
	int_t stack_size = stack_.size();
	int_t fun_frames_size = fun_frames_.size();

begin:
XTAL_VM_SWITCH(*pc){
	
	XTAL_VM_FIRST_CASE(Nop){ 
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

//{OPS{{
	XTAL_VM_CASE(PushNull){ // 3
		push(null); 
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(PushNop){ // 3
		push(nop); 
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(PushTrue){ // 3
		push(UncountedAny(true).cref()); 
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(PushFalse){ // 3
		push(UncountedAny(false).cref()); 
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(PushInt1Byte){ // 3
		push(UncountedAny((int_t)inst.value).cref()); 
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(PushInt2Byte){ // 3
		push(UncountedAny((int_t)inst.value).cref()); 
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(PushFloat1Byte){ // 3
		push(UncountedAny((float_t)inst.value).cref()); 
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(PushFloat2Byte){ // 3
		push(UncountedAny((float_t)inst.value).cref()); 
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(PushCallee){ // 3
		push(fun()); 
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(PushArgs){ // 3
		push(fun_frames_[0].arguments());
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}

	XTAL_VM_CASE(PushThis){ // 3
		push(ff().self()); 
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(PushCurrentContext){ XTAL_VM_CONTINUE(FunPushCurrentContext(pc)); /*
		XTAL_GLOBAL_INTERPRETER_LOCK{
			push(decolonize());
		}
		XTAL_VM_CONTINUE(pc + inst.ISIZE);;;;;;
	}*/ }

	XTAL_VM_CASE(Pop){ // 3
		downsize(1); 
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(Dup){ // 3
		dup(); 
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(Insert1){ // 5
		UncountedAny temp = get(); 
		set(get(1)); 
		set(1, temp.cref()); 
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(Insert2){ // 6
		UncountedAny temp = get(); 
		set(get(1)); 
		set(1, get(2)); 
		set(2, temp.cref()); 
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(Insert3){ XTAL_VM_CONTINUE(FunInsert3(pc)); /*
		UncountedAny temp = get(); 
		set(get(1)); 
		set(1, get(2)); 
		set(2, get(3)); 
		set(3, temp.cref()); 
		XTAL_VM_CONTINUE(pc + inst.ISIZE);;;;;;
	}*/ }

	XTAL_VM_CASE(AdjustResult){ // 3
		adjust_result(inst.result_count, inst.need_result_count);
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(If){ // 2
		XTAL_VM_CONTINUE(pc + (pop().to_b() ? inst.ISIZE : inst.address));
	}

	XTAL_VM_CASE(Unless){ // 2
		XTAL_VM_CONTINUE(pc + (!pop().to_b() ? inst.ISIZE : inst.address));
	}

	XTAL_VM_CASE(Goto){ // 2
		XTAL_VM_CONTINUE(pc + inst.address); 
	}

	XTAL_VM_CASE(LocalVariableInc){ XTAL_VM_CONTINUE(FunLocalVariableInc(pc)); /*
		UncountedAny a = local_variable(inst.number);
		switch(a.type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ set_local_variable(inst.number, UncountedAny(a.ivalue()+1).cref()); XTAL_VM_CONTINUE(pc + inst.ISIZE + InstSetLocalVariable2Byte::ISIZE); }
			XTAL_CASE(TYPE_FLOAT){ set_local_variable(inst.number, UncountedAny(a.fvalue()+1).cref()); XTAL_VM_CONTINUE(pc + inst.ISIZE + InstSetLocalVariable2Byte::ISIZE); }
		}
		a.cref().send(Xid(op_inc), inner_setup_call(pc + inst.ISIZE, 1));
		XTAL_VM_CONTINUE(ff().called_pc);
	}*/ }

	XTAL_VM_CASE(LocalVariableDec){ XTAL_VM_CONTINUE(FunLocalVariableDec(pc)); /*
		UncountedAny a = local_variable(inst.number);
		switch(a.type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ set_local_variable(inst.number, UncountedAny(a.ivalue()-1).cref()); XTAL_VM_CONTINUE(pc + inst.ISIZE + InstSetLocalVariable2Byte::ISIZE); }
			XTAL_CASE(TYPE_FLOAT){ set_local_variable(inst.number, UncountedAny(a.fvalue()-1).cref()); XTAL_VM_CONTINUE(pc + inst.ISIZE + InstSetLocalVariable2Byte::ISIZE); }
		}
		a.cref().send(Xid(op_dec), inner_setup_call(pc + inst.ISIZE, 1));
		XTAL_VM_CONTINUE(ff().called_pc);
	}*/ }

	XTAL_VM_CASE(LocalVariableIncDirect){ // 9
		UncountedAny& a = ff().variables_[inst.number];
		switch(a.type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ a = UncountedAny(a.ivalue()+1); XTAL_VM_CONTINUE(pc + inst.ISIZE + InstSetLocalVariable1ByteDirect::ISIZE); }
			XTAL_CASE(TYPE_FLOAT){ a = UncountedAny(a.fvalue()+1); XTAL_VM_CONTINUE(pc + inst.ISIZE + InstSetLocalVariable1ByteDirect::ISIZE); }
		}
		a.cref().send(Xid(op_inc), inner_setup_call(pc + inst.ISIZE, 1));
		XTAL_VM_CONTINUE(ff().called_pc);
	}

	XTAL_VM_CASE(LocalVariableDecDirect){ // 9
		UncountedAny& a = ff().variables_[inst.number];
		switch(a.type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ a = UncountedAny(a.ivalue()-1); XTAL_VM_CONTINUE(pc + inst.ISIZE + InstSetLocalVariable1ByteDirect::ISIZE); }
			XTAL_CASE(TYPE_FLOAT){ a = UncountedAny(a.fvalue()-1); XTAL_VM_CONTINUE(pc + inst.ISIZE + InstSetLocalVariable1ByteDirect::ISIZE); }
		}
		a.cref().send(Xid(op_dec), inner_setup_call(pc + inst.ISIZE, 1));
		XTAL_VM_CONTINUE(ff().called_pc);
	}

	XTAL_VM_CASE(LocalVariable1ByteDirect){ // 3
		push(ff().variable(inst.number)); 
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(LocalVariable1Byte){ // 3
		push(local_variable(inst.number)); 
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(LocalVariable2Byte){ // 3
		push(local_variable(inst.number)); 
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(SetLocalVariable1ByteDirect){ // 3
		ff().variable(inst.number, pop()); 
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(SetLocalVariable1Byte){ // 3
		set_local_variable(inst.number, pop()); 
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(SetLocalVariable2Byte){ // 3
		set_local_variable(inst.number, pop()); 
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(InstanceVariable){ // 4
		FunFrame& f = ff();
		XTAL_GLOBAL_INTERPRETER_LOCK{
			push(f.instance_variables->variable(inst.number, f.pcode->frame_core(inst.core_number)));
		}
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(SetInstanceVariable){ // 4
		FunFrame& f = ff();
		XTAL_GLOBAL_INTERPRETER_LOCK{ 
			f.instance_variables->set_variable(inst.number, f.pcode->frame_core(inst.core_number), pop());
		}
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(CleanupCall){ // 2
		XTAL_VM_CONTINUE(pop_ff());
	}

	XTAL_VM_CASE(Return0){ // 5
		for(int_t i=0, sz=ff().need_result_count; i<sz; ++i){
			push(null);
		}
		XTAL_VM_CONTINUE(pop_ff());  
	}

	XTAL_VM_CASE(Return1){ // 3
		if(ff().need_result_count!=1){
			adjust_result(1);
		}
		XTAL_VM_CONTINUE(pop_ff());  
	}

	XTAL_VM_CASE(Return2){ // 3
		adjust_result(2); 
		XTAL_VM_CONTINUE(pop_ff()); 
	}

	XTAL_VM_CASE(Return){ // 3
		adjust_result(inst.results); 
		XTAL_VM_CONTINUE(pop_ff());  
	}

	XTAL_VM_CASE(Yield){ // 6
		yield_result_count_ = inst.results;	
		if(ff().yieldable){
			resume_pc_ = pc + inst.ISIZE;
			return;
		}else{
			downsize(yield_result_count_);
			XTAL_GLOBAL_INTERPRETER_LOCK{ 
				XTAL_THROW(builtin().member("YieldError")(Xt("Xtal Runtime Error 1012"))); 
			}
		}
	}

	XTAL_VM_CASE(Exit){ // 3
		resume_pc_ = 0; 
		return; 
	}

	XTAL_VM_CASE(Value){ // 3
		push(ff().pcode->value(inst.value_number)); 
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(SetValue){ // 3
		ff().pcode->set_value(inst.value_number, pop()); 
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(CheckUnsupported){ // 3
		return_result(nop); 
		XTAL_VM_CONTINUE(pop_ff());
	}

	XTAL_VM_CASE(Send){ XTAL_VM_CONTINUE(FunSend(pc)); /*
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
		XTAL_VM_CONTINUE(ff().called_pc); 	
	}*/ }

	XTAL_VM_CASE(SendIfDefined){ XTAL_VM_CONTINUE(FunSendIfDefined(pc)); /*
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
		XTAL_VM_CONTINUE(ff().called_pc); 	
	}*/ }

	XTAL_VM_CASE(Call){ XTAL_VM_CONTINUE(FunCall(pc)); /*
		XTAL_GLOBAL_INTERPRETER_LOCK{
			UncountedAny self = ff().self();
			UncountedAny target = ff().temp_ = pop();
			push_ff(pc + inst.ISIZE, inst.need_result_count, inst.ordered_arg_count, inst.named_arg_count, self.cref());
			target.cref().call(myself());
		}
		XTAL_VM_CONTINUE(ff().called_pc);	
	}*/ }

	XTAL_VM_CASE(CallCallee){ // 6
		UncountedAny fn = fun();
		UncountedAny self = ff().self();
		push_ff(pc + inst.ISIZE, inst.need_result_count, inst.ordered_arg_count, inst.named_arg_count, self.cref());
		carry_over(((const Fun&)fn.cref()).impl());
		XTAL_VM_CONTINUE(ff().called_pc);	
	}

	XTAL_VM_CASE(Send_A){ XTAL_VM_CONTINUE(FunSend_A(pc)); /*
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
		XTAL_VM_CONTINUE(ff().called_pc); 	
	}*/ }

	XTAL_VM_CASE(SendIfDefined_A){ XTAL_VM_CONTINUE(FunSendIfDefined_A(pc)); /*
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
		XTAL_VM_CONTINUE(ff().called_pc); 	
	}*/ }

	XTAL_VM_CASE(Call_A){ XTAL_VM_CONTINUE(FunCall_A(pc)); /*
		XTAL_GLOBAL_INTERPRETER_LOCK{
			UncountedAny self = ff().self();
			UncountedAny target = ff().temp_ = pop();
			push_ff_args(pc + inst.ISIZE, inst.need_result_count, inst.ordered_arg_count, inst.named_arg_count, self.cref());
			target.cref().call(myself());
		}
		XTAL_VM_CONTINUE(ff().called_pc);	
	}*/ }

	XTAL_VM_CASE(CallCallee_A){ // 6
		UncountedAny fn = fun();
		UncountedAny self = ff().self();
		push_ff_args(pc + inst.ISIZE, inst.need_result_count, inst.ordered_arg_count, inst.named_arg_count, self.cref());
		carry_over(((const Fun&)fn.cref()).impl());
		XTAL_VM_CONTINUE(ff().called_pc);	
	}

	XTAL_VM_CASE(Send_T){ XTAL_VM_CONTINUE(FunSend_T(pc)); /*
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
		XTAL_VM_CONTINUE(ff().called_pc); 	
	}*/ }

	XTAL_VM_CASE(SendIfDefined_T){ XTAL_VM_CONTINUE(FunSendIfDefined_T(pc)); /*
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
		XTAL_VM_CONTINUE(ff().called_pc); 	
	}*/ }

	XTAL_VM_CASE(Call_T){ XTAL_VM_CONTINUE(FunCall_T(pc)); /*
		XTAL_GLOBAL_INTERPRETER_LOCK{
			UncountedAny self = ff().self();
			UncountedAny target = ff().temp_ = pop();
			recycle_ff(pc + inst.ISIZE, inst.ordered_arg_count, inst.named_arg_count, self.cref());
			target.cref().call(myself());
		}
		XTAL_VM_CONTINUE(ff().called_pc);	
	}*/ }

	XTAL_VM_CASE(CallCallee_T){ // 6
		UncountedAny fn = fun();
		UncountedAny self = ff().self();
		recycle_ff(pc + inst.ISIZE, inst.ordered_arg_count, inst.named_arg_count, self.cref());
		carry_over(((const Fun&)fn.cref()).impl());
		XTAL_VM_CONTINUE(ff().called_pc);	
	}

	XTAL_VM_CASE(Send_AT){ XTAL_VM_CONTINUE(FunSend_AT(pc)); /*
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
		XTAL_VM_CONTINUE(ff().called_pc); 	
	}*/ }

	XTAL_VM_CASE(SendIfDefined_AT){ XTAL_VM_CONTINUE(FunSendIfDefined_AT(pc)); /*
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
		XTAL_VM_CONTINUE(ff().called_pc); 	
	}*/ }

	XTAL_VM_CASE(Call_AT){ XTAL_VM_CONTINUE(FunCall_AT(pc)); /*
		XTAL_GLOBAL_INTERPRETER_LOCK{
			UncountedAny self = ff().self();
			UncountedAny target = ff().temp_ = pop();
			recycle_ff_args(pc + inst.ISIZE, inst.ordered_arg_count, inst.named_arg_count, self.cref());
			target.cref().call(myself());
		}
		XTAL_VM_CONTINUE(ff().called_pc);	
	}*/ }

	XTAL_VM_CASE(CallCallee_AT){ // 6
		UncountedAny fn = fun();
		UncountedAny self = ff().self();
		recycle_ff_args(pc + inst.ISIZE, inst.ordered_arg_count, inst.named_arg_count, self.cref());
		carry_over(((const Fun&)fn.cref()).impl());
		XTAL_VM_CONTINUE(ff().called_pc);	
	}

	XTAL_VM_CASE(BlockBegin){ // 6
		FunFrame& f = ff(); 
		FrameCore* p = ff().pcode->frame_core(inst.core_number); 
		f.scopes.push(p); 
		f.variables_.upsize(p->variable_size);
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}

	XTAL_VM_CASE(BlockEnd){ // 5
		if(ff().scopes.top()){ 
			ff().variables_.downsize(ff().scopes.top()->variable_size);
		}else{
			ff().outer(ff().outer().outer()); 
		}
		ff().scopes.downsize(1);
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}

	XTAL_VM_CASE(BlockEndDirect){ // 5
		FunFrame& f = ff(); 
		f.variables_.downsize(f.scopes.top()->variable_size);
		f.scopes.downsize(1);
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}

	XTAL_VM_CASE(TryBegin){ // 7
		ExceptFrame& ef = except_frames_.push();
		ef.core = &ff().pcode->except_core_table_[inst.core_number];
		ef.fun_frame_count = fun_frames_.size();
		ef.scope_count = ff().scopes.size();
		ef.stack_count = this->stack_size();
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}

	XTAL_VM_CASE(TryEnd){ // 2
		XTAL_VM_CONTINUE(except_frames_.pop().core->finally_pc + ff().psource); 
	}

	XTAL_VM_CASE(PushGoto){ // 3
		push(UncountedAny((int_t)((pc+inst.address)-ff().psource)).cref()); 
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(PopGoto){ // 2
		XTAL_VM_CONTINUE(ff().psource+pop().ivalue());
	}

	XTAL_VM_CASE(IfEq){ XTAL_VM_CONTINUE(FunIfEq(pc)); /*
		switch(get(1).type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ pc = get(1).ivalue() == get().ivalue() ? pc+inst.ISIZE+InstIf::ISIZE : pc+inst.address; downsize(2); XTAL_VM_CONTINUE(pc); }
				XTAL_CASE(TYPE_FLOAT){ pc = get(1).ivalue() == get().fvalue() ? pc+inst.ISIZE+InstIf::ISIZE : pc+inst.address; downsize(2); XTAL_VM_CONTINUE(pc); }
			}}
			XTAL_CASE(TYPE_FLOAT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ pc = get(1).fvalue() == get().ivalue() ? pc+inst.ISIZE+InstIf::ISIZE : pc+inst.address; downsize(2); XTAL_VM_CONTINUE(pc); }
				XTAL_CASE(TYPE_FLOAT){ pc = get(1).fvalue() == get().fvalue() ? pc+inst.ISIZE+InstIf::ISIZE : pc+inst.address; downsize(2); XTAL_VM_CONTINUE(pc); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc, Xid(op_eq), inst.ISIZE));
	}*/ }

	XTAL_VM_CASE(IfNe){ XTAL_VM_CONTINUE(FunIfNe(pc)); /*
		switch(get(1).type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ pc = get(1).ivalue() != get().ivalue() ? pc+inst.ISIZE+InstNot::ISIZE+InstIf::ISIZE : pc+inst.address; downsize(2); XTAL_VM_CONTINUE(pc); }
				XTAL_CASE(TYPE_FLOAT){ pc = get(1).ivalue() != get().fvalue() ? pc+inst.ISIZE+InstNot::ISIZE+InstIf::ISIZE : pc+inst.address; downsize(2); XTAL_VM_CONTINUE(pc); }
			}}
			XTAL_CASE(TYPE_FLOAT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ pc = get(1).fvalue() != get().ivalue() ? pc+inst.ISIZE+InstNot::ISIZE+InstIf::ISIZE : pc+inst.address; downsize(2); XTAL_VM_CONTINUE(pc); }
				XTAL_CASE(TYPE_FLOAT){ pc = get(1).fvalue() != get().fvalue() ? pc+inst.ISIZE+InstNot::ISIZE+InstIf::ISIZE : pc+inst.address; downsize(2); XTAL_VM_CONTINUE(pc); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc, Xid(op_eq), inst.ISIZE));
	}*/ }

	XTAL_VM_CASE(IfLt){ XTAL_VM_CONTINUE(FunIfLt(pc)); /*
		switch(get(1).type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ pc = get(1).ivalue() < get().ivalue() ? pc+inst.ISIZE+InstIf::ISIZE : pc+inst.address; downsize(2); XTAL_VM_CONTINUE(pc); }
				XTAL_CASE(TYPE_FLOAT){ pc = get(1).ivalue() < get().fvalue() ? pc+inst.ISIZE+InstIf::ISIZE : pc+inst.address; downsize(2); XTAL_VM_CONTINUE(pc); }
			}}
			XTAL_CASE(TYPE_FLOAT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ pc = get(1).fvalue() < get().ivalue() ? pc+inst.ISIZE+InstIf::ISIZE : pc+inst.address; downsize(2); XTAL_VM_CONTINUE(pc); }
				XTAL_CASE(TYPE_FLOAT){ pc = get(1).fvalue() < get().fvalue() ? pc+inst.ISIZE+InstIf::ISIZE : pc+inst.address; downsize(2); XTAL_VM_CONTINUE(pc); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc, Xid(op_lt), inst.ISIZE));
	}*/ }

	XTAL_VM_CASE(IfLe){ XTAL_VM_CONTINUE(FunIfLe(pc)); /*
		switch(get(1).type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ pc = get(1).ivalue() <= get().ivalue() ? pc+inst.ISIZE+InstNot::ISIZE+InstIf::ISIZE : pc+inst.address; downsize(2); XTAL_VM_CONTINUE(pc); }
				XTAL_CASE(TYPE_FLOAT){ pc = get(1).ivalue() <= get().fvalue() ? pc+inst.ISIZE+InstNot::ISIZE+InstIf::ISIZE : pc+inst.address; downsize(2); XTAL_VM_CONTINUE(pc); }
			}}
			XTAL_CASE(TYPE_FLOAT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ pc = get(1).fvalue() <= get().ivalue() ? pc+inst.ISIZE+InstNot::ISIZE+InstIf::ISIZE : pc+inst.address; downsize(2); XTAL_VM_CONTINUE(pc); }
				XTAL_CASE(TYPE_FLOAT){ pc = get(1).fvalue() <= get().fvalue() ? pc+inst.ISIZE+InstNot::ISIZE+InstIf::ISIZE : pc+inst.address; downsize(2); XTAL_VM_CONTINUE(pc); }
			}}
		}
		XTAL_VM_CONTINUE(send2r(pc, Xid(op_lt), inst.ISIZE));
	}*/ }

	XTAL_VM_CASE(IfGt){ XTAL_VM_CONTINUE(FunIfGt(pc)); /*
		switch(get(1).type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ pc = get(1).ivalue() > get().ivalue() ? pc+inst.ISIZE+InstIf::ISIZE : pc+inst.address; downsize(2); XTAL_VM_CONTINUE(pc); }
				XTAL_CASE(TYPE_FLOAT){ pc = get(1).ivalue() > get().fvalue() ? pc+inst.ISIZE+InstIf::ISIZE : pc+inst.address; downsize(2); XTAL_VM_CONTINUE(pc); }
			}}
			XTAL_CASE(TYPE_FLOAT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ pc = get(1).fvalue() > get().ivalue() ? pc+inst.ISIZE+InstIf::ISIZE : pc+inst.address; downsize(2); XTAL_VM_CONTINUE(pc); }
				XTAL_CASE(TYPE_FLOAT){ pc = get(1).fvalue() > get().fvalue() ? pc+inst.ISIZE+InstIf::ISIZE : pc+inst.address; downsize(2); XTAL_VM_CONTINUE(pc); }
			}}
		}
		XTAL_VM_CONTINUE(send2r(pc, Xid(op_lt), inst.ISIZE));
	}*/ }

	XTAL_VM_CASE(IfGe){ XTAL_VM_CONTINUE(FunIfGe(pc)); /*
		switch(get(1).type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ pc = get(1).ivalue() >= get().ivalue() ? pc+inst.ISIZE+InstNot::ISIZE+InstIf::ISIZE : pc+inst.address; downsize(2); XTAL_VM_CONTINUE(pc); }
				XTAL_CASE(TYPE_FLOAT){ pc = get(1).ivalue() >= get().fvalue() ? pc+inst.ISIZE+InstNot::ISIZE+InstIf::ISIZE : pc+inst.address; downsize(2); XTAL_VM_CONTINUE(pc); }
			}}
			XTAL_CASE(TYPE_FLOAT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ pc = get(1).fvalue() >= get().ivalue() ? pc+inst.ISIZE+InstNot::ISIZE+InstIf::ISIZE : pc+inst.address; downsize(2); XTAL_VM_CONTINUE(pc); }
				XTAL_CASE(TYPE_FLOAT){ pc = get(1).fvalue() >= get().fvalue() ? pc+inst.ISIZE+InstNot::ISIZE+InstIf::ISIZE : pc+inst.address; downsize(2); XTAL_VM_CONTINUE(pc); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc, Xid(op_lt), inst.ISIZE));
	}*/ }

	XTAL_VM_CASE(IfRawEq){ // 4
		pc += get(1).raweq(get()) ? inst.ISIZE : inst.address;
		downsize(2);
		XTAL_VM_CONTINUE(pc);
	}

	XTAL_VM_CASE(IfRawNe){ // 4
		pc += !get(1).raweq(get()) ? inst.ISIZE : inst.address;
		downsize(2);
		XTAL_VM_CONTINUE(pc);
	}

	XTAL_VM_CASE(IfIs){ // 4
		pc += get(1).is(cast<const Class&>(get())) ? inst.ISIZE : inst.address;
		downsize(2);
		XTAL_VM_CONTINUE(pc);
	}

	XTAL_VM_CASE(IfNis){ // 4
		pc += !get(1).is(cast<const Class&>(get())) ? inst.ISIZE : inst.address;
		downsize(2);
		XTAL_VM_CONTINUE(pc);
	}

	XTAL_VM_CASE(IfArgIsNull){ // 3
		if(local_variable(inst.arg).is_null()){
			XTAL_VM_CONTINUE(pc + inst.ISIZE);
		}else{
			XTAL_VM_CONTINUE(pc + inst.address); 
		}
	}

	XTAL_VM_CASE(Pos){ XTAL_VM_CONTINUE(FunPos(pc)); /*
		switch(get().type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ XTAL_VM_CONTINUE(pc + 1); }
			XTAL_CASE(TYPE_FLOAT){ XTAL_VM_CONTINUE(pc + 1); }
		}
		XTAL_VM_CONTINUE(send1(pc, Xid(op_pos)));
	}*/ }

	XTAL_VM_CASE(Neg){ XTAL_VM_CONTINUE(FunNeg(pc)); /*
		switch(get().type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ set(UncountedAny(-get().ivalue()).cref()); XTAL_VM_CONTINUE(pc + 1); }
			XTAL_CASE(TYPE_FLOAT){ set(UncountedAny(-get().fvalue()).cref()); XTAL_VM_CONTINUE(pc + 1); }
		}
		XTAL_VM_CONTINUE(send1(pc, Xid(op_neg)));
	}*/ }

	XTAL_VM_CASE(Com){ XTAL_VM_CONTINUE(FunCom(pc)); /*
		switch(get().type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ set(UncountedAny(~get().ivalue()).cref()); XTAL_VM_CONTINUE(pc + 1); }
		}
		XTAL_VM_CONTINUE(send1(pc, Xid(op_com)));
	}*/ }

	XTAL_VM_CASE(Not){ // 3
		set(UncountedAny(!get().to_b()).cref()); 
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}

	XTAL_VM_CASE(At){ XTAL_VM_CONTINUE(FunAt(pc)); /*
		XTAL_GLOBAL_INTERPRETER_LOCK{
			Any idx = pop();
			Any target = pop();
			inner_setup_call(pc+1, 1, idx);
			target.send(Xid(op_at), myself());
		}
		XTAL_VM_CONTINUE(ff().called_pc); 
	}*/ }

	XTAL_VM_CASE(SetAt){ XTAL_VM_CONTINUE(FunSetAt(pc)); /*
		XTAL_GLOBAL_INTERPRETER_LOCK{
			Any idx = pop();
			Any target = pop();
			Any value = pop();
			inner_setup_call(pc+1, 0, idx, value);
			target.send(Xid(op_set_at), myself());
		}
		XTAL_VM_CONTINUE(ff().called_pc);
	}*/ }

	XTAL_VM_CASE(Add){ XTAL_VM_CONTINUE(FunAdd(pc)); /*
		switch(get(1).type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).ivalue() + get().ivalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
				XTAL_CASE(TYPE_FLOAT){ set(1, UncountedAny(get(1).ivalue() + get().fvalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
			}}
			XTAL_CASE(TYPE_FLOAT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).fvalue() + get().ivalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
				XTAL_CASE(TYPE_FLOAT){ set(1, UncountedAny(get(1).fvalue() + get().fvalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc, Xid(op_add)));
	}*/ }

	XTAL_VM_CASE(Sub){ XTAL_VM_CONTINUE(FunSub(pc)); /*
		switch(get(1).type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).ivalue() - get().ivalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
				XTAL_CASE(TYPE_FLOAT){ set(1, UncountedAny(get(1).ivalue() - get().fvalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
			}}
			XTAL_CASE(TYPE_FLOAT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).fvalue() - get().ivalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
				XTAL_CASE(TYPE_FLOAT){ set(1, UncountedAny(get(1).fvalue() - get().fvalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc, Xid(op_sub)));
	}*/ }

	XTAL_VM_CASE(Cat){ // 2
		XTAL_VM_CONTINUE(send2(pc, Xid(op_cat)));
	}

	XTAL_VM_CASE(Mul){ XTAL_VM_CONTINUE(FunMul(pc)); /*
		switch(get(1).type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).ivalue() * get().ivalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
				XTAL_CASE(TYPE_FLOAT){ set(1, UncountedAny(get(1).ivalue() * get().fvalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
			}}
			XTAL_CASE(TYPE_FLOAT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).fvalue() * get().ivalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
				XTAL_CASE(TYPE_FLOAT){ set(1, UncountedAny(get(1).fvalue() * get().fvalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc, Xid(op_mul)));
	}*/ }

	XTAL_VM_CASE(Div){ XTAL_VM_CONTINUE(FunDiv(pc)); /*
		switch(get(1).type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).ivalue() / get().ivalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
				XTAL_CASE(TYPE_FLOAT){ set(1, UncountedAny(get(1).ivalue() / get().fvalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
			}}
			XTAL_CASE(TYPE_FLOAT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).fvalue() / get().ivalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
				XTAL_CASE(TYPE_FLOAT){ set(1, UncountedAny(get(1).fvalue() / get().fvalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc, Xid(op_div)));
	}*/ }

	XTAL_VM_CASE(Mod){ XTAL_VM_CONTINUE(FunMod(pc)); /*
		using namespace std;
		switch(get(1).type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).ivalue() % get().ivalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
				XTAL_CASE(TYPE_FLOAT){ set(1, UncountedAny(fmodf((float_t)get(1).ivalue(), get().fvalue())).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
			}}
			XTAL_CASE(TYPE_FLOAT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, UncountedAny(fmodf(get(1).fvalue(),(float_t)get().ivalue())).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
				XTAL_CASE(TYPE_FLOAT){ set(1, UncountedAny(fmodf(get(1).fvalue(), get().fvalue())).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc, Xid(op_mod)));
	}*/ }

	XTAL_VM_CASE(And){ XTAL_VM_CONTINUE(FunAnd(pc)); /*
		switch(get(1).type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).ivalue() & get().ivalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc, Xid(op_and)));
	}*/ }

	XTAL_VM_CASE(Or){ XTAL_VM_CONTINUE(FunOr(pc)); /*
		switch(get(1).type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).ivalue() | get().ivalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc, Xid(op_or)));
	}*/ }

	XTAL_VM_CASE(Xor){ XTAL_VM_CONTINUE(FunXor(pc)); /*
		switch(get(1).type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).ivalue() ^ get().ivalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc, Xid(op_xor)));
	}*/ }

	XTAL_VM_CASE(Shl){ XTAL_VM_CONTINUE(FunShl(pc)); /*
		switch(get(1).type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).ivalue() << get().ivalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc, Xid(op_shl)));
	}*/ }

	XTAL_VM_CASE(Shr){ XTAL_VM_CONTINUE(FunShr(pc)); /*
		switch(get(1).type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).ivalue() >> get().ivalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc, Xid(op_shr)));
	}*/ }

	XTAL_VM_CASE(Ushr){ XTAL_VM_CONTINUE(FunUshr(pc)); /*
		switch(get(1).type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, UncountedAny((int_t)((uint_t)get(1).ivalue() >> get().ivalue())).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc, Xid(op_ushr)));
	}*/ }

	XTAL_VM_CASE(Eq){ XTAL_VM_CONTINUE(FunEq(pc)); /*
		switch(get(1).type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).ivalue() == get().ivalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
				XTAL_CASE(TYPE_FLOAT){ set(1, UncountedAny(get(1).ivalue() == get().fvalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
			}}
			XTAL_CASE(TYPE_FLOAT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).fvalue() == get().ivalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
				XTAL_CASE(TYPE_FLOAT){ set(1, UncountedAny(get(1).fvalue() == get().fvalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc, Xid(op_eq)));
	}*/ }

	XTAL_VM_CASE(Ne){ XTAL_VM_CONTINUE(FunNe(pc)); /*
		switch(get(1).type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).ivalue() != get().ivalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1+1); }
				XTAL_CASE(TYPE_FLOAT){ set(1, UncountedAny(get(1).ivalue() != get().fvalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1+1); }
			}}
			XTAL_CASE(TYPE_FLOAT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).fvalue() != get().ivalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1+1); }
				XTAL_CASE(TYPE_FLOAT){ set(1, UncountedAny(get(1).fvalue() != get().fvalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1+1);}
			}}
		}
		XTAL_VM_CONTINUE(send2(pc, Xid(op_eq)));
	}*/ }

	XTAL_VM_CASE(Lt){ XTAL_VM_CONTINUE(FunLt(pc)); /*
		switch(get(1).type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).ivalue() < get().ivalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
				XTAL_CASE(TYPE_FLOAT){ set(1, UncountedAny(get(1).ivalue() < get().fvalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
			}}
			XTAL_CASE(TYPE_FLOAT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).fvalue() < get().ivalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
				XTAL_CASE(TYPE_FLOAT){ set(1, UncountedAny(get(1).fvalue() < get().fvalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc, Xid(op_lt)));
	}*/ }

	XTAL_VM_CASE(Le){ XTAL_VM_CONTINUE(FunLe(pc)); /*
		switch(get(1).type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).ivalue() <= get().ivalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1+1); }
				XTAL_CASE(TYPE_FLOAT){ set(1, UncountedAny(get(1).ivalue() <= get().fvalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1+1); }
			}}
			XTAL_CASE(TYPE_FLOAT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).fvalue() <= get().ivalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1+1); }
				XTAL_CASE(TYPE_FLOAT){ set(1, UncountedAny(get(1).fvalue() <= get().fvalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1+1); }
			}}
		}
		XTAL_VM_CONTINUE(send2r(pc, Xid(op_lt)));
	}*/ }

	XTAL_VM_CASE(Gt){ XTAL_VM_CONTINUE(FunGt(pc)); /*
		switch(get(1).type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).ivalue() > get().ivalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
				XTAL_CASE(TYPE_FLOAT){ set(1, UncountedAny(get(1).ivalue() > get().fvalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
			}}
			XTAL_CASE(TYPE_FLOAT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).fvalue() > get().ivalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
				XTAL_CASE(TYPE_FLOAT){ set(1, UncountedAny(get(1).fvalue() > get().fvalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
			}}
		}
		XTAL_VM_CONTINUE(send2r(pc, Xid(op_lt)));
	}*/ }

	XTAL_VM_CASE(Ge){ XTAL_VM_CONTINUE(FunGe(pc)); /*
		switch(get(1).type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).ivalue() >= get().ivalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1+1); }
				XTAL_CASE(TYPE_FLOAT){ set(1, UncountedAny(get(1).ivalue() >= get().fvalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1+1); }
			}}
			XTAL_CASE(TYPE_FLOAT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).fvalue() >= get().ivalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1+1); }
				XTAL_CASE(TYPE_FLOAT){ set(1, UncountedAny(get(1).fvalue() >= get().fvalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1+1); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc, Xid(op_lt)));
	}*/ }

	XTAL_VM_CASE(RawEq){ // 4
		set(1, UncountedAny(get(1).raweq(get())).cref());
		downsize(1);
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}

	XTAL_VM_CASE(RawNe){ // 4
		set(1, UncountedAny(get(1).rawne(get())).cref());
		downsize(1);
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}

	XTAL_VM_CASE(Is){ // 4
		set(1, UncountedAny(get(1).is(cast<const Class&>(get()))).cref());
		downsize(1);
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}

	XTAL_VM_CASE(Nis){ // 4
		set(1, UncountedAny(!get(1).is(cast<const Class&>(get()))).cref());
		downsize(1);
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}

	XTAL_VM_CASE(Inc){ XTAL_VM_CONTINUE(FunInc(pc)); /*
		switch(get().type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ set(UncountedAny(get().ivalue()+1).cref()); XTAL_VM_CONTINUE(pc + 1); }
			XTAL_CASE(TYPE_FLOAT){ set(UncountedAny(get().fvalue()+1).cref()); XTAL_VM_CONTINUE(pc + 1); }
		}
		XTAL_VM_CONTINUE(send1(pc, Xid(op_inc)));
	}*/ }

	XTAL_VM_CASE(Dec){ XTAL_VM_CONTINUE(FunDec(pc)); /*
		switch(get().type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ set(UncountedAny(get().ivalue()-1).cref()); XTAL_VM_CONTINUE(pc + 1); }
			XTAL_CASE(TYPE_FLOAT){ set(UncountedAny(get().fvalue()-1).cref()); XTAL_VM_CONTINUE(pc + 1); }
		}
		XTAL_VM_CONTINUE(send1(pc, Xid(op_dec)));
	}*/ }

	XTAL_VM_CASE(AddAssign){ XTAL_VM_CONTINUE(FunAddAssign(pc)); /*
		switch(get(1).type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).ivalue() + get().ivalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
				XTAL_CASE(TYPE_FLOAT){ set(1, UncountedAny(get(1).ivalue() + get().fvalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
			}}
			XTAL_CASE(TYPE_FLOAT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).fvalue() + get().ivalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
				XTAL_CASE(TYPE_FLOAT){ set(1, UncountedAny(get(1).fvalue() + get().fvalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc, Xid(op_add_assign)));
	}*/ }

	XTAL_VM_CASE(SubAssign){ XTAL_VM_CONTINUE(FunSubAssign(pc)); /*
		switch(get(1).type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).ivalue() - get().ivalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
				XTAL_CASE(TYPE_FLOAT){ set(1, UncountedAny(get(1).ivalue() - get().fvalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
			}}
			XTAL_CASE(TYPE_FLOAT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).fvalue() - get().ivalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
				XTAL_CASE(TYPE_FLOAT){ set(1, UncountedAny(get(1).fvalue() - get().fvalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc, Xid(op_sub_assign)));
	}*/ }

	XTAL_VM_CASE(CatAssign){ // 2
		XTAL_VM_CONTINUE(send2(pc, Xid(op_cat_assign)));
	}

	XTAL_VM_CASE(MulAssign){ XTAL_VM_CONTINUE(FunMulAssign(pc)); /*
		switch(get(1).type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).ivalue() * get().ivalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
				XTAL_CASE(TYPE_FLOAT){ set(1, UncountedAny(get(1).ivalue() * get().fvalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
			}}
			XTAL_CASE(TYPE_FLOAT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).fvalue() * get().ivalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
				XTAL_CASE(TYPE_FLOAT){ set(1, UncountedAny(get(1).fvalue() * get().fvalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc, Xid(op_mul_assign)));
	}*/ }

	XTAL_VM_CASE(DivAssign){ XTAL_VM_CONTINUE(FunDivAssign(pc)); /*
		switch(get(1).type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).ivalue() / get().ivalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
				XTAL_CASE(TYPE_FLOAT){ set(1, UncountedAny(get(1).ivalue() / get().fvalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
			}}
			XTAL_CASE(TYPE_FLOAT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).fvalue() / get().ivalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
				XTAL_CASE(TYPE_FLOAT){ set(1, UncountedAny(get(1).fvalue() / get().fvalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc, Xid(op_div_assign)));
	}*/ }

	XTAL_VM_CASE(ModAssign){ XTAL_VM_CONTINUE(FunModAssign(pc)); /*
		switch(get(1).type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).ivalue() % get().ivalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
				XTAL_CASE(TYPE_FLOAT){ set(1, UncountedAny(fmodf((float_t)get(1).ivalue(), get().fvalue())).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
			}}
			XTAL_CASE(TYPE_FLOAT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, UncountedAny(fmodf(get(1).fvalue(),(float_t)get().ivalue())).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
				XTAL_CASE(TYPE_FLOAT){ set(1, UncountedAny(fmodf(get(1).fvalue(), get().fvalue())).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc, Xid(op_mod_assign)));
	}*/ }

	XTAL_VM_CASE(AndAssign){ XTAL_VM_CONTINUE(FunAndAssign(pc)); /*
		switch(get(1).type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).ivalue() & get().ivalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc, Xid(op_and_assign)));
	}*/ }

	XTAL_VM_CASE(OrAssign){ XTAL_VM_CONTINUE(FunOrAssign(pc)); /*
		switch(get(1).type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).ivalue() | get().ivalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc, Xid(op_or_assign)));
	}*/ }

	XTAL_VM_CASE(XorAssign){ XTAL_VM_CONTINUE(FunXorAssign(pc)); /*
		switch(get(1).type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).ivalue() ^ get().ivalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc, Xid(op_xor_assign)));
	}*/ }

	XTAL_VM_CASE(ShlAssign){ XTAL_VM_CONTINUE(FunShlAssign(pc)); /*
		switch(get(1).type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).ivalue() << get().ivalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc, Xid(op_shl_assign)));
	}*/ }

	XTAL_VM_CASE(ShrAssign){ XTAL_VM_CONTINUE(FunShrAssign(pc)); /*
		switch(get(1).type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).ivalue() >> get().ivalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc, Xid(op_shr_assign)));
	}*/ }

	XTAL_VM_CASE(UshrAssign){ XTAL_VM_CONTINUE(FunUshrAssign(pc)); /*
		switch(get(1).type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, UncountedAny((int_t)((uint_t)get(1).ivalue() >> get().ivalue())).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc, Xid(op_ushr_assign)));
	}*/ }

	XTAL_VM_CASE(GlobalVariable){ XTAL_VM_CONTINUE(FunGlobalVariable(pc)); /*
		XTAL_GLOBAL_INTERPRETER_LOCK{
			if(const Any& ret = ff().pcode->toplevel().member(symbol(inst.symbol_number))){
				push(ret);
			}else{
				XTAL_THROW(unsupported_error("toplevel", symbol(inst.symbol_number)));
			}
		}
		XTAL_VM_CONTINUE(pc + inst.ISIZE);;;;;;
	}*/ }

	XTAL_VM_CASE(SetGlobalVariable){ XTAL_VM_CONTINUE(FunSetGlobalVariable(pc)); /*
		XTAL_GLOBAL_INTERPRETER_LOCK{
			ff().pcode->toplevel().set_member(symbol(inst.symbol_number), pop(), null);
		}
		XTAL_VM_CONTINUE(pc + inst.ISIZE);;;;;;
	}*/ }

	XTAL_VM_CASE(DefineGlobalVariable){ XTAL_VM_CONTINUE(FunDefineGlobalVariable(pc)); /*
		XTAL_GLOBAL_INTERPRETER_LOCK{
			ff().pcode->toplevel().def(symbol(inst.symbol_number), pop(), KIND_PUBLIC, null);
		}
		XTAL_VM_CONTINUE(pc + inst.ISIZE);;;;;;
	}*/ }

	XTAL_VM_CASE(Member){ XTAL_VM_CONTINUE(FunMember(pc)); /*
		XTAL_GLOBAL_INTERPRETER_LOCK{
			const ID& name = symbol_ex(inst.symbol_number);
			const Any& target = get();
			if(const Any& ret = member_cache(target, name, ff().self(), null)){
				set(ret);
			}else{
				XTAL_THROW(unsupported_error(target.object_name(), name));
			}
		}
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}*/ }

	XTAL_VM_CASE(MemberIfDefined){ XTAL_VM_CONTINUE(FunMemberIfDefined(pc)); /*
		XTAL_GLOBAL_INTERPRETER_LOCK{
			const ID& name = symbol_ex(inst.symbol_number);
			const Any& target = get();
			if(const Any& ret = member_cache(target, name, ff().self(), null)){
				set(ret);
			}else{
				set(nop);
			}
		}
		XTAL_VM_CONTINUE(pc + inst.ISIZE);  
	}*/ }

	XTAL_VM_CASE(DefineMember){ XTAL_VM_CONTINUE(FunDefineMember(pc)); /*
		XTAL_GLOBAL_INTERPRETER_LOCK{
			const ID& name = symbol_ex(inst.symbol_number);
			const Any& value = get();
			const Any& target = get(1);
			target.def(name, value); 
			downsize(2);
		}
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}*/ }

	XTAL_VM_CASE(DefineClassMember){ XTAL_VM_CONTINUE(FunDefineClassMember(pc)); /*
		XTAL_GLOBAL_INTERPRETER_LOCK{
			ClassImpl* p = cast<Class>(ff().outer()).impl();
			p->set_class_member(inst.number, symbol(inst.symbol_number), inst.accessibility, get(), get(1));
			downsize(2);
		}
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}*/ }

	XTAL_VM_CASE(SetName){ XTAL_VM_CONTINUE(FunSetName(pc)); /*
		XTAL_GLOBAL_INTERPRETER_LOCK{
			get().set_object_name(symbol(inst.symbol_number), 1, null);
		}
		XTAL_VM_CONTINUE(pc + inst.ISIZE);;;;;;
	}*/ }

	XTAL_VM_CASE(SetMultipleLocalVariable2Direct){ // 5
		FunFrame& f = ff();
		f.variable(inst.local_variable2, pop()); 
		f.variable(inst.local_variable1, pop()); 
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}

	XTAL_VM_CASE(SetMultipleLocalVariable3Direct){ // 6
		FunFrame& f = ff();
		f.variable(inst.local_variable3, pop()); 
		f.variable(inst.local_variable2, pop()); 
		f.variable(inst.local_variable1, pop()); 
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}

	XTAL_VM_CASE(SetMultipleLocalVariable4Direct){ // 7
		FunFrame& f = ff();
		f.variable(inst.local_variable4, pop()); 
		f.variable(inst.local_variable3, pop()); 
		f.variable(inst.local_variable2, pop()); 
		f.variable(inst.local_variable1, pop()); 
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}

	XTAL_VM_CASE(Once){ // 5
		const Any& ret = ff().pcode->value(inst.value_number);
		if(!ret.raweq(nop)){
			push(ret);
			XTAL_VM_CONTINUE(pc + inst.address);
		}else{
			XTAL_VM_CONTINUE(pc + inst.ISIZE);
		}
	}

	XTAL_VM_CASE(ClassBegin){ XTAL_VM_CONTINUE(FunClassBegin(pc)); /*
		XTAL_GLOBAL_INTERPRETER_LOCK{
			FrameCore* p = ff().pcode->frame_core(inst.core_number); 
			Class cp = new_xclass(decolonize(), code(), p);

			int_t n = inst.mixins;
			for(int_t i = 0; i<n; ++i){
				cp.inherit(cast<Class>(pop()));
			}

			ff().outer(decolonize());
			push_ff(pc + inst.ISIZE, 0, 0, 0, cp);
			ff().fun(prev_fun());

			ff().outer(cp);
			ff().scopes.push(0);
		}
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}*/ }

	XTAL_VM_CASE(ClassEnd){ XTAL_VM_CONTINUE(FunClassEnd(pc)); /*
		push(ff().outer());
		ff().outer(ff().outer().outer());
		ff().scopes.downsize(1);
		pop_ff();
		XTAL_VM_CONTINUE(pc + inst.ISIZE);;;;;;
	}*/ }

	XTAL_VM_CASE(MakeArray){ // 3
		XTAL_GLOBAL_INTERPRETER_LOCK{
			push(Array());
		}
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}

	XTAL_VM_CASE(ArrayAppend){ // 4
		XTAL_GLOBAL_INTERPRETER_LOCK{
			cast<Array*>(get(1))->push_back(get()); 
			downsize(1);
		}
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}

	XTAL_VM_CASE(MakeMap){ // 3
		XTAL_GLOBAL_INTERPRETER_LOCK{
			push(Map());
		}
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}

	XTAL_VM_CASE(MapInsert){ // 4
		XTAL_GLOBAL_INTERPRETER_LOCK{
			cast<Map*>(get(2))->set_at(get(1), get()); 
			downsize(2);	
		}
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}

	XTAL_VM_CASE(MakeFun){ XTAL_VM_CONTINUE(FunMakeFun(pc)); /*
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
		XTAL_VM_CONTINUE(pc + end);
	}*/ }

	XTAL_VM_CASE(MakeInstanceVariableAccessor){ XTAL_VM_CONTINUE(FunMakeInstanceVariableAccessor(pc)); /*
		XTAL_GLOBAL_INTERPRETER_LOCK{
			Any ret;
			switch(inst.type){
				XTAL_NODEFAULT;

				XTAL_CASE(0){ new(ret) InstanceVariableGetterImpl(inst.number, ff().pcode->frame_core(inst.core_number)); }
				XTAL_CASE(1){ new(ret) InstanceVariableSetterImpl(inst.number, ff().pcode->frame_core(inst.core_number)); }
			}
			push(ret);
		}
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
	}*/ }

	XTAL_VM_CASE(Throw){ // 6
		XTAL_GLOBAL_INTERPRETER_LOCK{
			Any except = pop();
			if(!except){
				except = last_except_.cref();
			}

			if(!except.is(cast<const Class&>(builtin().member("Exception")))){
				last_except_ = append_backtrace(pc, builtin().member("RuntimeError")(except));
			}else{
				last_except_ = append_backtrace(pc, except);
			}
		}
		goto except_catch; 
	}

	XTAL_VM_CASE(ThrowUnsupportedError){ // 3
		XTAL_GLOBAL_INTERPRETER_LOCK{
			last_except_ = unsupported_error(ff().hint1().object_name(), ff().hint2());
		}
		goto except_catch;
	}

	XTAL_VM_CASE(ThrowNull){ // 3
		last_except_ = null; 
		goto except_catch; 
	}

	XTAL_VM_CASE(Assert){ // 10
		bool failure = false;
		XTAL_GLOBAL_INTERPRETER_LOCK{
			Any expr = get(2);
			if(!expr){
				failure = true;
				Any expr_string = get(1) ? get(1) : Any("");
				Any message = get() ? get() : Any("");
				last_except_ = append_backtrace(pc, builtin().member("AssertionFailed")(message, expr_string));
			}
			downsize(3);
		}

		if(failure){
			goto except_catch; 
		}

		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}

	XTAL_VM_CASE(BreakPoint){ XTAL_VM_CONTINUE(FunBreakPoint(pc)); /*
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
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
	}*/ }

//}}OPS}

	XTAL_VM_NODEFAULT;
}

except_catch:
	pc = catch_body(pc, stack_size, fun_frames_size);
	if(pc){
		goto begin;
	}

	if(last_except_.cref()){
		XTAL_THROW(last_except_.cref());
	}
}

#undef XTAL_VM_CONTINUE
#define XTAL_VM_CONTINUE(x) return (x)

//{FUNS{{
const inst_t* VMachineImpl::FunPushCurrentContext(const inst_t* pc){
		XTAL_VM_DEF_INST(PushCurrentContext);
		XTAL_GLOBAL_INTERPRETER_LOCK{
			push(decolonize());
		}
		XTAL_VM_CONTINUE(pc + inst.ISIZE);;;;;;
}

const inst_t* VMachineImpl::FunInsert3(const inst_t* pc){
		XTAL_VM_DEF_INST(Insert3);
		UncountedAny temp = get(); 
		set(get(1)); 
		set(1, get(2)); 
		set(2, get(3)); 
		set(3, temp.cref()); 
		XTAL_VM_CONTINUE(pc + inst.ISIZE);;;;;;
}

const inst_t* VMachineImpl::FunLocalVariableInc(const inst_t* pc){
		XTAL_VM_DEF_INST(LocalVariableInc);
		UncountedAny a = local_variable(inst.number);
		switch(a.type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ set_local_variable(inst.number, UncountedAny(a.ivalue()+1).cref()); XTAL_VM_CONTINUE(pc + inst.ISIZE + InstSetLocalVariable2Byte::ISIZE); }
			XTAL_CASE(TYPE_FLOAT){ set_local_variable(inst.number, UncountedAny(a.fvalue()+1).cref()); XTAL_VM_CONTINUE(pc + inst.ISIZE + InstSetLocalVariable2Byte::ISIZE); }
		}
		a.cref().send(Xid(op_inc), inner_setup_call(pc + inst.ISIZE, 1));
		XTAL_VM_CONTINUE(ff().called_pc);
}

const inst_t* VMachineImpl::FunLocalVariableDec(const inst_t* pc){
		XTAL_VM_DEF_INST(LocalVariableDec);
		UncountedAny a = local_variable(inst.number);
		switch(a.type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ set_local_variable(inst.number, UncountedAny(a.ivalue()-1).cref()); XTAL_VM_CONTINUE(pc + inst.ISIZE + InstSetLocalVariable2Byte::ISIZE); }
			XTAL_CASE(TYPE_FLOAT){ set_local_variable(inst.number, UncountedAny(a.fvalue()-1).cref()); XTAL_VM_CONTINUE(pc + inst.ISIZE + InstSetLocalVariable2Byte::ISIZE); }
		}
		a.cref().send(Xid(op_dec), inner_setup_call(pc + inst.ISIZE, 1));
		XTAL_VM_CONTINUE(ff().called_pc);
}

const inst_t* VMachineImpl::FunSend(const inst_t* pc){
		XTAL_VM_DEF_INST(Send);
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
		XTAL_VM_CONTINUE(ff().called_pc); 	
}

const inst_t* VMachineImpl::FunSendIfDefined(const inst_t* pc){
		XTAL_VM_DEF_INST(SendIfDefined);
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
		XTAL_VM_CONTINUE(ff().called_pc); 	
}

const inst_t* VMachineImpl::FunCall(const inst_t* pc){
		XTAL_VM_DEF_INST(Call);
		XTAL_GLOBAL_INTERPRETER_LOCK{
			UncountedAny self = ff().self();
			UncountedAny target = ff().temp_ = pop();
			push_ff(pc + inst.ISIZE, inst.need_result_count, inst.ordered_arg_count, inst.named_arg_count, self.cref());
			target.cref().call(myself());
		}
		XTAL_VM_CONTINUE(ff().called_pc);	
}

const inst_t* VMachineImpl::FunSend_A(const inst_t* pc){
		XTAL_VM_DEF_INST(Send_A);
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
		XTAL_VM_CONTINUE(ff().called_pc); 	
}

const inst_t* VMachineImpl::FunSendIfDefined_A(const inst_t* pc){
		XTAL_VM_DEF_INST(SendIfDefined_A);
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
		XTAL_VM_CONTINUE(ff().called_pc); 	
}

const inst_t* VMachineImpl::FunCall_A(const inst_t* pc){
		XTAL_VM_DEF_INST(Call_A);
		XTAL_GLOBAL_INTERPRETER_LOCK{
			UncountedAny self = ff().self();
			UncountedAny target = ff().temp_ = pop();
			push_ff_args(pc + inst.ISIZE, inst.need_result_count, inst.ordered_arg_count, inst.named_arg_count, self.cref());
			target.cref().call(myself());
		}
		XTAL_VM_CONTINUE(ff().called_pc);	
}

const inst_t* VMachineImpl::FunSend_T(const inst_t* pc){
		XTAL_VM_DEF_INST(Send_T);
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
		XTAL_VM_CONTINUE(ff().called_pc); 	
}

const inst_t* VMachineImpl::FunSendIfDefined_T(const inst_t* pc){
		XTAL_VM_DEF_INST(SendIfDefined_T);
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
		XTAL_VM_CONTINUE(ff().called_pc); 	
}

const inst_t* VMachineImpl::FunCall_T(const inst_t* pc){
		XTAL_VM_DEF_INST(Call_T);
		XTAL_GLOBAL_INTERPRETER_LOCK{
			UncountedAny self = ff().self();
			UncountedAny target = ff().temp_ = pop();
			recycle_ff(pc + inst.ISIZE, inst.ordered_arg_count, inst.named_arg_count, self.cref());
			target.cref().call(myself());
		}
		XTAL_VM_CONTINUE(ff().called_pc);	
}

const inst_t* VMachineImpl::FunSend_AT(const inst_t* pc){
		XTAL_VM_DEF_INST(Send_AT);
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
		XTAL_VM_CONTINUE(ff().called_pc); 	
}

const inst_t* VMachineImpl::FunSendIfDefined_AT(const inst_t* pc){
		XTAL_VM_DEF_INST(SendIfDefined_AT);
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
		XTAL_VM_CONTINUE(ff().called_pc); 	
}

const inst_t* VMachineImpl::FunCall_AT(const inst_t* pc){
		XTAL_VM_DEF_INST(Call_AT);
		XTAL_GLOBAL_INTERPRETER_LOCK{
			UncountedAny self = ff().self();
			UncountedAny target = ff().temp_ = pop();
			recycle_ff_args(pc + inst.ISIZE, inst.ordered_arg_count, inst.named_arg_count, self.cref());
			target.cref().call(myself());
		}
		XTAL_VM_CONTINUE(ff().called_pc);	
}

const inst_t* VMachineImpl::FunIfEq(const inst_t* pc){
		XTAL_VM_DEF_INST(IfEq);
		switch(get(1).type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ pc = get(1).ivalue() == get().ivalue() ? pc+inst.ISIZE+InstIf::ISIZE : pc+inst.address; downsize(2); XTAL_VM_CONTINUE(pc); }
				XTAL_CASE(TYPE_FLOAT){ pc = get(1).ivalue() == get().fvalue() ? pc+inst.ISIZE+InstIf::ISIZE : pc+inst.address; downsize(2); XTAL_VM_CONTINUE(pc); }
			}}
			XTAL_CASE(TYPE_FLOAT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ pc = get(1).fvalue() == get().ivalue() ? pc+inst.ISIZE+InstIf::ISIZE : pc+inst.address; downsize(2); XTAL_VM_CONTINUE(pc); }
				XTAL_CASE(TYPE_FLOAT){ pc = get(1).fvalue() == get().fvalue() ? pc+inst.ISIZE+InstIf::ISIZE : pc+inst.address; downsize(2); XTAL_VM_CONTINUE(pc); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc, Xid(op_eq), inst.ISIZE));
}

const inst_t* VMachineImpl::FunIfNe(const inst_t* pc){
		XTAL_VM_DEF_INST(IfNe);
		switch(get(1).type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ pc = get(1).ivalue() != get().ivalue() ? pc+inst.ISIZE+InstNot::ISIZE+InstIf::ISIZE : pc+inst.address; downsize(2); XTAL_VM_CONTINUE(pc); }
				XTAL_CASE(TYPE_FLOAT){ pc = get(1).ivalue() != get().fvalue() ? pc+inst.ISIZE+InstNot::ISIZE+InstIf::ISIZE : pc+inst.address; downsize(2); XTAL_VM_CONTINUE(pc); }
			}}
			XTAL_CASE(TYPE_FLOAT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ pc = get(1).fvalue() != get().ivalue() ? pc+inst.ISIZE+InstNot::ISIZE+InstIf::ISIZE : pc+inst.address; downsize(2); XTAL_VM_CONTINUE(pc); }
				XTAL_CASE(TYPE_FLOAT){ pc = get(1).fvalue() != get().fvalue() ? pc+inst.ISIZE+InstNot::ISIZE+InstIf::ISIZE : pc+inst.address; downsize(2); XTAL_VM_CONTINUE(pc); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc, Xid(op_eq), inst.ISIZE));
}

const inst_t* VMachineImpl::FunIfLt(const inst_t* pc){
		XTAL_VM_DEF_INST(IfLt);
		switch(get(1).type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ pc = get(1).ivalue() < get().ivalue() ? pc+inst.ISIZE+InstIf::ISIZE : pc+inst.address; downsize(2); XTAL_VM_CONTINUE(pc); }
				XTAL_CASE(TYPE_FLOAT){ pc = get(1).ivalue() < get().fvalue() ? pc+inst.ISIZE+InstIf::ISIZE : pc+inst.address; downsize(2); XTAL_VM_CONTINUE(pc); }
			}}
			XTAL_CASE(TYPE_FLOAT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ pc = get(1).fvalue() < get().ivalue() ? pc+inst.ISIZE+InstIf::ISIZE : pc+inst.address; downsize(2); XTAL_VM_CONTINUE(pc); }
				XTAL_CASE(TYPE_FLOAT){ pc = get(1).fvalue() < get().fvalue() ? pc+inst.ISIZE+InstIf::ISIZE : pc+inst.address; downsize(2); XTAL_VM_CONTINUE(pc); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc, Xid(op_lt), inst.ISIZE));
}

const inst_t* VMachineImpl::FunIfLe(const inst_t* pc){
		XTAL_VM_DEF_INST(IfLe);
		switch(get(1).type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ pc = get(1).ivalue() <= get().ivalue() ? pc+inst.ISIZE+InstNot::ISIZE+InstIf::ISIZE : pc+inst.address; downsize(2); XTAL_VM_CONTINUE(pc); }
				XTAL_CASE(TYPE_FLOAT){ pc = get(1).ivalue() <= get().fvalue() ? pc+inst.ISIZE+InstNot::ISIZE+InstIf::ISIZE : pc+inst.address; downsize(2); XTAL_VM_CONTINUE(pc); }
			}}
			XTAL_CASE(TYPE_FLOAT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ pc = get(1).fvalue() <= get().ivalue() ? pc+inst.ISIZE+InstNot::ISIZE+InstIf::ISIZE : pc+inst.address; downsize(2); XTAL_VM_CONTINUE(pc); }
				XTAL_CASE(TYPE_FLOAT){ pc = get(1).fvalue() <= get().fvalue() ? pc+inst.ISIZE+InstNot::ISIZE+InstIf::ISIZE : pc+inst.address; downsize(2); XTAL_VM_CONTINUE(pc); }
			}}
		}
		XTAL_VM_CONTINUE(send2r(pc, Xid(op_lt), inst.ISIZE));
}

const inst_t* VMachineImpl::FunIfGt(const inst_t* pc){
		XTAL_VM_DEF_INST(IfGt);
		switch(get(1).type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ pc = get(1).ivalue() > get().ivalue() ? pc+inst.ISIZE+InstIf::ISIZE : pc+inst.address; downsize(2); XTAL_VM_CONTINUE(pc); }
				XTAL_CASE(TYPE_FLOAT){ pc = get(1).ivalue() > get().fvalue() ? pc+inst.ISIZE+InstIf::ISIZE : pc+inst.address; downsize(2); XTAL_VM_CONTINUE(pc); }
			}}
			XTAL_CASE(TYPE_FLOAT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ pc = get(1).fvalue() > get().ivalue() ? pc+inst.ISIZE+InstIf::ISIZE : pc+inst.address; downsize(2); XTAL_VM_CONTINUE(pc); }
				XTAL_CASE(TYPE_FLOAT){ pc = get(1).fvalue() > get().fvalue() ? pc+inst.ISIZE+InstIf::ISIZE : pc+inst.address; downsize(2); XTAL_VM_CONTINUE(pc); }
			}}
		}
		XTAL_VM_CONTINUE(send2r(pc, Xid(op_lt), inst.ISIZE));
}

const inst_t* VMachineImpl::FunIfGe(const inst_t* pc){
		XTAL_VM_DEF_INST(IfGe);
		switch(get(1).type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ pc = get(1).ivalue() >= get().ivalue() ? pc+inst.ISIZE+InstNot::ISIZE+InstIf::ISIZE : pc+inst.address; downsize(2); XTAL_VM_CONTINUE(pc); }
				XTAL_CASE(TYPE_FLOAT){ pc = get(1).ivalue() >= get().fvalue() ? pc+inst.ISIZE+InstNot::ISIZE+InstIf::ISIZE : pc+inst.address; downsize(2); XTAL_VM_CONTINUE(pc); }
			}}
			XTAL_CASE(TYPE_FLOAT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ pc = get(1).fvalue() >= get().ivalue() ? pc+inst.ISIZE+InstNot::ISIZE+InstIf::ISIZE : pc+inst.address; downsize(2); XTAL_VM_CONTINUE(pc); }
				XTAL_CASE(TYPE_FLOAT){ pc = get(1).fvalue() >= get().fvalue() ? pc+inst.ISIZE+InstNot::ISIZE+InstIf::ISIZE : pc+inst.address; downsize(2); XTAL_VM_CONTINUE(pc); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc, Xid(op_lt), inst.ISIZE));
}

const inst_t* VMachineImpl::FunPos(const inst_t* pc){
		XTAL_VM_DEF_INST(Pos);
		switch(get().type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ XTAL_VM_CONTINUE(pc + 1); }
			XTAL_CASE(TYPE_FLOAT){ XTAL_VM_CONTINUE(pc + 1); }
		}
		XTAL_VM_CONTINUE(send1(pc, Xid(op_pos)));
}

const inst_t* VMachineImpl::FunNeg(const inst_t* pc){
		XTAL_VM_DEF_INST(Neg);
		switch(get().type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ set(UncountedAny(-get().ivalue()).cref()); XTAL_VM_CONTINUE(pc + 1); }
			XTAL_CASE(TYPE_FLOAT){ set(UncountedAny(-get().fvalue()).cref()); XTAL_VM_CONTINUE(pc + 1); }
		}
		XTAL_VM_CONTINUE(send1(pc, Xid(op_neg)));
}

const inst_t* VMachineImpl::FunCom(const inst_t* pc){
		XTAL_VM_DEF_INST(Com);
		switch(get().type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ set(UncountedAny(~get().ivalue()).cref()); XTAL_VM_CONTINUE(pc + 1); }
		}
		XTAL_VM_CONTINUE(send1(pc, Xid(op_com)));
}

const inst_t* VMachineImpl::FunAt(const inst_t* pc){
		XTAL_VM_DEF_INST(At);
		XTAL_GLOBAL_INTERPRETER_LOCK{
			Any idx = pop();
			Any target = pop();
			inner_setup_call(pc+1, 1, idx);
			target.send(Xid(op_at), myself());
		}
		XTAL_VM_CONTINUE(ff().called_pc); 
}

const inst_t* VMachineImpl::FunSetAt(const inst_t* pc){
		XTAL_VM_DEF_INST(SetAt);
		XTAL_GLOBAL_INTERPRETER_LOCK{
			Any idx = pop();
			Any target = pop();
			Any value = pop();
			inner_setup_call(pc+1, 0, idx, value);
			target.send(Xid(op_set_at), myself());
		}
		XTAL_VM_CONTINUE(ff().called_pc);
}

const inst_t* VMachineImpl::FunAdd(const inst_t* pc){
		XTAL_VM_DEF_INST(Add);
		switch(get(1).type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).ivalue() + get().ivalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
				XTAL_CASE(TYPE_FLOAT){ set(1, UncountedAny(get(1).ivalue() + get().fvalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
			}}
			XTAL_CASE(TYPE_FLOAT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).fvalue() + get().ivalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
				XTAL_CASE(TYPE_FLOAT){ set(1, UncountedAny(get(1).fvalue() + get().fvalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc, Xid(op_add)));
}

const inst_t* VMachineImpl::FunSub(const inst_t* pc){
		XTAL_VM_DEF_INST(Sub);
		switch(get(1).type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).ivalue() - get().ivalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
				XTAL_CASE(TYPE_FLOAT){ set(1, UncountedAny(get(1).ivalue() - get().fvalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
			}}
			XTAL_CASE(TYPE_FLOAT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).fvalue() - get().ivalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
				XTAL_CASE(TYPE_FLOAT){ set(1, UncountedAny(get(1).fvalue() - get().fvalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc, Xid(op_sub)));
}

const inst_t* VMachineImpl::FunMul(const inst_t* pc){
		XTAL_VM_DEF_INST(Mul);
		switch(get(1).type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).ivalue() * get().ivalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
				XTAL_CASE(TYPE_FLOAT){ set(1, UncountedAny(get(1).ivalue() * get().fvalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
			}}
			XTAL_CASE(TYPE_FLOAT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).fvalue() * get().ivalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
				XTAL_CASE(TYPE_FLOAT){ set(1, UncountedAny(get(1).fvalue() * get().fvalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc, Xid(op_mul)));
}

const inst_t* VMachineImpl::FunDiv(const inst_t* pc){
		XTAL_VM_DEF_INST(Div);
		switch(get(1).type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).ivalue() / get().ivalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
				XTAL_CASE(TYPE_FLOAT){ set(1, UncountedAny(get(1).ivalue() / get().fvalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
			}}
			XTAL_CASE(TYPE_FLOAT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).fvalue() / get().ivalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
				XTAL_CASE(TYPE_FLOAT){ set(1, UncountedAny(get(1).fvalue() / get().fvalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc, Xid(op_div)));
}

const inst_t* VMachineImpl::FunMod(const inst_t* pc){
		XTAL_VM_DEF_INST(Mod);
		using namespace std;
		switch(get(1).type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).ivalue() % get().ivalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
				XTAL_CASE(TYPE_FLOAT){ set(1, UncountedAny(fmodf((float_t)get(1).ivalue(), get().fvalue())).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
			}}
			XTAL_CASE(TYPE_FLOAT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, UncountedAny(fmodf(get(1).fvalue(),(float_t)get().ivalue())).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
				XTAL_CASE(TYPE_FLOAT){ set(1, UncountedAny(fmodf(get(1).fvalue(), get().fvalue())).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc, Xid(op_mod)));
}

const inst_t* VMachineImpl::FunAnd(const inst_t* pc){
		XTAL_VM_DEF_INST(And);
		switch(get(1).type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).ivalue() & get().ivalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc, Xid(op_and)));
}

const inst_t* VMachineImpl::FunOr(const inst_t* pc){
		XTAL_VM_DEF_INST(Or);
		switch(get(1).type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).ivalue() | get().ivalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc, Xid(op_or)));
}

const inst_t* VMachineImpl::FunXor(const inst_t* pc){
		XTAL_VM_DEF_INST(Xor);
		switch(get(1).type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).ivalue() ^ get().ivalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc, Xid(op_xor)));
}

const inst_t* VMachineImpl::FunShl(const inst_t* pc){
		XTAL_VM_DEF_INST(Shl);
		switch(get(1).type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).ivalue() << get().ivalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc, Xid(op_shl)));
}

const inst_t* VMachineImpl::FunShr(const inst_t* pc){
		XTAL_VM_DEF_INST(Shr);
		switch(get(1).type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).ivalue() >> get().ivalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc, Xid(op_shr)));
}

const inst_t* VMachineImpl::FunUshr(const inst_t* pc){
		XTAL_VM_DEF_INST(Ushr);
		switch(get(1).type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, UncountedAny((int_t)((uint_t)get(1).ivalue() >> get().ivalue())).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc, Xid(op_ushr)));
}

const inst_t* VMachineImpl::FunEq(const inst_t* pc){
		XTAL_VM_DEF_INST(Eq);
		switch(get(1).type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).ivalue() == get().ivalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
				XTAL_CASE(TYPE_FLOAT){ set(1, UncountedAny(get(1).ivalue() == get().fvalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
			}}
			XTAL_CASE(TYPE_FLOAT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).fvalue() == get().ivalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
				XTAL_CASE(TYPE_FLOAT){ set(1, UncountedAny(get(1).fvalue() == get().fvalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc, Xid(op_eq)));
}

const inst_t* VMachineImpl::FunNe(const inst_t* pc){
		XTAL_VM_DEF_INST(Ne);
		switch(get(1).type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).ivalue() != get().ivalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1+1); }
				XTAL_CASE(TYPE_FLOAT){ set(1, UncountedAny(get(1).ivalue() != get().fvalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1+1); }
			}}
			XTAL_CASE(TYPE_FLOAT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).fvalue() != get().ivalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1+1); }
				XTAL_CASE(TYPE_FLOAT){ set(1, UncountedAny(get(1).fvalue() != get().fvalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1+1);}
			}}
		}
		XTAL_VM_CONTINUE(send2(pc, Xid(op_eq)));
}

const inst_t* VMachineImpl::FunLt(const inst_t* pc){
		XTAL_VM_DEF_INST(Lt);
		switch(get(1).type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).ivalue() < get().ivalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
				XTAL_CASE(TYPE_FLOAT){ set(1, UncountedAny(get(1).ivalue() < get().fvalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
			}}
			XTAL_CASE(TYPE_FLOAT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).fvalue() < get().ivalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
				XTAL_CASE(TYPE_FLOAT){ set(1, UncountedAny(get(1).fvalue() < get().fvalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc, Xid(op_lt)));
}

const inst_t* VMachineImpl::FunLe(const inst_t* pc){
		XTAL_VM_DEF_INST(Le);
		switch(get(1).type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).ivalue() <= get().ivalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1+1); }
				XTAL_CASE(TYPE_FLOAT){ set(1, UncountedAny(get(1).ivalue() <= get().fvalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1+1); }
			}}
			XTAL_CASE(TYPE_FLOAT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).fvalue() <= get().ivalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1+1); }
				XTAL_CASE(TYPE_FLOAT){ set(1, UncountedAny(get(1).fvalue() <= get().fvalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1+1); }
			}}
		}
		XTAL_VM_CONTINUE(send2r(pc, Xid(op_lt)));
}

const inst_t* VMachineImpl::FunGt(const inst_t* pc){
		XTAL_VM_DEF_INST(Gt);
		switch(get(1).type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).ivalue() > get().ivalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
				XTAL_CASE(TYPE_FLOAT){ set(1, UncountedAny(get(1).ivalue() > get().fvalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
			}}
			XTAL_CASE(TYPE_FLOAT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).fvalue() > get().ivalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
				XTAL_CASE(TYPE_FLOAT){ set(1, UncountedAny(get(1).fvalue() > get().fvalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
			}}
		}
		XTAL_VM_CONTINUE(send2r(pc, Xid(op_lt)));
}

const inst_t* VMachineImpl::FunGe(const inst_t* pc){
		XTAL_VM_DEF_INST(Ge);
		switch(get(1).type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).ivalue() >= get().ivalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1+1); }
				XTAL_CASE(TYPE_FLOAT){ set(1, UncountedAny(get(1).ivalue() >= get().fvalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1+1); }
			}}
			XTAL_CASE(TYPE_FLOAT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).fvalue() >= get().ivalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1+1); }
				XTAL_CASE(TYPE_FLOAT){ set(1, UncountedAny(get(1).fvalue() >= get().fvalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1+1); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc, Xid(op_lt)));
}

const inst_t* VMachineImpl::FunInc(const inst_t* pc){
		XTAL_VM_DEF_INST(Inc);
		switch(get().type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ set(UncountedAny(get().ivalue()+1).cref()); XTAL_VM_CONTINUE(pc + 1); }
			XTAL_CASE(TYPE_FLOAT){ set(UncountedAny(get().fvalue()+1).cref()); XTAL_VM_CONTINUE(pc + 1); }
		}
		XTAL_VM_CONTINUE(send1(pc, Xid(op_inc)));
}

const inst_t* VMachineImpl::FunDec(const inst_t* pc){
		XTAL_VM_DEF_INST(Dec);
		switch(get().type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ set(UncountedAny(get().ivalue()-1).cref()); XTAL_VM_CONTINUE(pc + 1); }
			XTAL_CASE(TYPE_FLOAT){ set(UncountedAny(get().fvalue()-1).cref()); XTAL_VM_CONTINUE(pc + 1); }
		}
		XTAL_VM_CONTINUE(send1(pc, Xid(op_dec)));
}

const inst_t* VMachineImpl::FunAddAssign(const inst_t* pc){
		XTAL_VM_DEF_INST(AddAssign);
		switch(get(1).type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).ivalue() + get().ivalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
				XTAL_CASE(TYPE_FLOAT){ set(1, UncountedAny(get(1).ivalue() + get().fvalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
			}}
			XTAL_CASE(TYPE_FLOAT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).fvalue() + get().ivalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
				XTAL_CASE(TYPE_FLOAT){ set(1, UncountedAny(get(1).fvalue() + get().fvalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc, Xid(op_add_assign)));
}

const inst_t* VMachineImpl::FunSubAssign(const inst_t* pc){
		XTAL_VM_DEF_INST(SubAssign);
		switch(get(1).type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).ivalue() - get().ivalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
				XTAL_CASE(TYPE_FLOAT){ set(1, UncountedAny(get(1).ivalue() - get().fvalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
			}}
			XTAL_CASE(TYPE_FLOAT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).fvalue() - get().ivalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
				XTAL_CASE(TYPE_FLOAT){ set(1, UncountedAny(get(1).fvalue() - get().fvalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc, Xid(op_sub_assign)));
}

const inst_t* VMachineImpl::FunMulAssign(const inst_t* pc){
		XTAL_VM_DEF_INST(MulAssign);
		switch(get(1).type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).ivalue() * get().ivalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
				XTAL_CASE(TYPE_FLOAT){ set(1, UncountedAny(get(1).ivalue() * get().fvalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
			}}
			XTAL_CASE(TYPE_FLOAT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).fvalue() * get().ivalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
				XTAL_CASE(TYPE_FLOAT){ set(1, UncountedAny(get(1).fvalue() * get().fvalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc, Xid(op_mul_assign)));
}

const inst_t* VMachineImpl::FunDivAssign(const inst_t* pc){
		XTAL_VM_DEF_INST(DivAssign);
		switch(get(1).type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).ivalue() / get().ivalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
				XTAL_CASE(TYPE_FLOAT){ set(1, UncountedAny(get(1).ivalue() / get().fvalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
			}}
			XTAL_CASE(TYPE_FLOAT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).fvalue() / get().ivalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
				XTAL_CASE(TYPE_FLOAT){ set(1, UncountedAny(get(1).fvalue() / get().fvalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc, Xid(op_div_assign)));
}

const inst_t* VMachineImpl::FunModAssign(const inst_t* pc){
		XTAL_VM_DEF_INST(ModAssign);
		switch(get(1).type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).ivalue() % get().ivalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
				XTAL_CASE(TYPE_FLOAT){ set(1, UncountedAny(fmodf((float_t)get(1).ivalue(), get().fvalue())).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
			}}
			XTAL_CASE(TYPE_FLOAT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, UncountedAny(fmodf(get(1).fvalue(),(float_t)get().ivalue())).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
				XTAL_CASE(TYPE_FLOAT){ set(1, UncountedAny(fmodf(get(1).fvalue(), get().fvalue())).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc, Xid(op_mod_assign)));
}

const inst_t* VMachineImpl::FunAndAssign(const inst_t* pc){
		XTAL_VM_DEF_INST(AndAssign);
		switch(get(1).type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).ivalue() & get().ivalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc, Xid(op_and_assign)));
}

const inst_t* VMachineImpl::FunOrAssign(const inst_t* pc){
		XTAL_VM_DEF_INST(OrAssign);
		switch(get(1).type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).ivalue() | get().ivalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc, Xid(op_or_assign)));
}

const inst_t* VMachineImpl::FunXorAssign(const inst_t* pc){
		XTAL_VM_DEF_INST(XorAssign);
		switch(get(1).type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).ivalue() ^ get().ivalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc, Xid(op_xor_assign)));
}

const inst_t* VMachineImpl::FunShlAssign(const inst_t* pc){
		XTAL_VM_DEF_INST(ShlAssign);
		switch(get(1).type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).ivalue() << get().ivalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc, Xid(op_shl_assign)));
}

const inst_t* VMachineImpl::FunShrAssign(const inst_t* pc){
		XTAL_VM_DEF_INST(ShrAssign);
		switch(get(1).type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).ivalue() >> get().ivalue()).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc, Xid(op_shr_assign)));
}

const inst_t* VMachineImpl::FunUshrAssign(const inst_t* pc){
		XTAL_VM_DEF_INST(UshrAssign);
		switch(get(1).type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ set(1, UncountedAny((int_t)((uint_t)get(1).ivalue() >> get().ivalue())).cref()); downsize(1); XTAL_VM_CONTINUE(pc + 1); }
			}}
		}
		XTAL_VM_CONTINUE(send2(pc, Xid(op_ushr_assign)));
}

const inst_t* VMachineImpl::FunGlobalVariable(const inst_t* pc){
		XTAL_VM_DEF_INST(GlobalVariable);
		XTAL_GLOBAL_INTERPRETER_LOCK{
			if(const Any& ret = ff().pcode->toplevel().member(symbol(inst.symbol_number))){
				push(ret);
			}else{
				XTAL_THROW(unsupported_error("toplevel", symbol(inst.symbol_number)));
			}
		}
		XTAL_VM_CONTINUE(pc + inst.ISIZE);;;;;;
}

const inst_t* VMachineImpl::FunSetGlobalVariable(const inst_t* pc){
		XTAL_VM_DEF_INST(SetGlobalVariable);
		XTAL_GLOBAL_INTERPRETER_LOCK{
			ff().pcode->toplevel().set_member(symbol(inst.symbol_number), pop(), null);
		}
		XTAL_VM_CONTINUE(pc + inst.ISIZE);;;;;;
}

const inst_t* VMachineImpl::FunDefineGlobalVariable(const inst_t* pc){
		XTAL_VM_DEF_INST(DefineGlobalVariable);
		XTAL_GLOBAL_INTERPRETER_LOCK{
			ff().pcode->toplevel().def(symbol(inst.symbol_number), pop(), KIND_PUBLIC, null);
		}
		XTAL_VM_CONTINUE(pc + inst.ISIZE);;;;;;
}

const inst_t* VMachineImpl::FunMember(const inst_t* pc){
		XTAL_VM_DEF_INST(Member);
		XTAL_GLOBAL_INTERPRETER_LOCK{
			const ID& name = symbol_ex(inst.symbol_number);
			const Any& target = get();
			if(const Any& ret = member_cache(target, name, ff().self(), null)){
				set(ret);
			}else{
				XTAL_THROW(unsupported_error(target.object_name(), name));
			}
		}
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
}

const inst_t* VMachineImpl::FunMemberIfDefined(const inst_t* pc){
		XTAL_VM_DEF_INST(MemberIfDefined);
		XTAL_GLOBAL_INTERPRETER_LOCK{
			const ID& name = symbol_ex(inst.symbol_number);
			const Any& target = get();
			if(const Any& ret = member_cache(target, name, ff().self(), null)){
				set(ret);
			}else{
				set(nop);
			}
		}
		XTAL_VM_CONTINUE(pc + inst.ISIZE);  
}

const inst_t* VMachineImpl::FunDefineMember(const inst_t* pc){
		XTAL_VM_DEF_INST(DefineMember);
		XTAL_GLOBAL_INTERPRETER_LOCK{
			const ID& name = symbol_ex(inst.symbol_number);
			const Any& value = get();
			const Any& target = get(1);
			target.def(name, value); 
			downsize(2);
		}
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
}

const inst_t* VMachineImpl::FunDefineClassMember(const inst_t* pc){
		XTAL_VM_DEF_INST(DefineClassMember);
		XTAL_GLOBAL_INTERPRETER_LOCK{
			ClassImpl* p = cast<Class>(ff().outer()).impl();
			p->set_class_member(inst.number, symbol(inst.symbol_number), inst.accessibility, get(), get(1));
			downsize(2);
		}
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
}

const inst_t* VMachineImpl::FunSetName(const inst_t* pc){
		XTAL_VM_DEF_INST(SetName);
		XTAL_GLOBAL_INTERPRETER_LOCK{
			get().set_object_name(symbol(inst.symbol_number), 1, null);
		}
		XTAL_VM_CONTINUE(pc + inst.ISIZE);;;;;;
}

const inst_t* VMachineImpl::FunClassBegin(const inst_t* pc){
		XTAL_VM_DEF_INST(ClassBegin);
		XTAL_GLOBAL_INTERPRETER_LOCK{
			FrameCore* p = ff().pcode->frame_core(inst.core_number); 
			Class cp = new_xclass(decolonize(), code(), p);

			int_t n = inst.mixins;
			for(int_t i = 0; i<n; ++i){
				cp.inherit(cast<Class>(pop()));
			}

			ff().outer(decolonize());
			push_ff(pc + inst.ISIZE, 0, 0, 0, cp);
			ff().fun(prev_fun());

			ff().outer(cp);
			ff().scopes.push(0);
		}
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
}

const inst_t* VMachineImpl::FunClassEnd(const inst_t* pc){
		XTAL_VM_DEF_INST(ClassEnd);
		push(ff().outer());
		ff().outer(ff().outer().outer());
		ff().scopes.downsize(1);
		pop_ff();
		XTAL_VM_CONTINUE(pc + inst.ISIZE);;;;;;
}

const inst_t* VMachineImpl::FunMakeFun(const inst_t* pc){
		XTAL_VM_DEF_INST(MakeFun);
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
		XTAL_VM_CONTINUE(pc + end);
}

const inst_t* VMachineImpl::FunMakeInstanceVariableAccessor(const inst_t* pc){
		XTAL_VM_DEF_INST(MakeInstanceVariableAccessor);
		XTAL_GLOBAL_INTERPRETER_LOCK{
			Any ret;
			switch(inst.type){
				XTAL_NODEFAULT;

				XTAL_CASE(0){ new(ret) InstanceVariableGetterImpl(inst.number, ff().pcode->frame_core(inst.core_number)); }
				XTAL_CASE(1){ new(ret) InstanceVariableSetterImpl(inst.number, ff().pcode->frame_core(inst.core_number)); }
			}
			push(ret);
		}
		XTAL_VM_CONTINUE(pc + inst.ISIZE); 
}

const inst_t* VMachineImpl::FunBreakPoint(const inst_t* pc){
		XTAL_VM_DEF_INST(BreakPoint);
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
		XTAL_VM_CONTINUE(pc + inst.ISIZE);
}

//}}FUNS}

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

const inst_t* VMachineImpl::send1(const inst_t* pc, const ID& name, int_t n){
	XTAL_GLOBAL_INTERPRETER_LOCK{
		UncountedAny target = ff().temp_ = pop();
		UncountedAny self = ff().self();
		push_ff(pc + n, 1, 0, 0, self.cref());
		const Class& cls = target.cref().get_class();
		set_hint(cls, name);
		if(const Any& ret = member_cache(cls, name, ff().self(), null)){
			set_arg_this(target.cref());
			ret.call(myself());
		}
	}
	return ff().called_pc;
}

const inst_t* VMachineImpl::send2(const inst_t* pc, const ID& name, int_t n){
	XTAL_GLOBAL_INTERPRETER_LOCK{
		const Any& temp = pop();
		UncountedAny target = ff().temp_ = get();
		set(temp);
		UncountedAny self = ff().self();
		push_ff(pc + n, 1, 1, 0, self.cref());
		const Class& cls = target.cref().get_class();
		set_hint(cls, name);
		if(const Any& ret = member_cache(cls, name, ff().self(), null)){
			set_arg_this(target.cref());
			ret.call(myself());
		}
	}
	return ff().called_pc;
}

const inst_t* VMachineImpl::send2r(const inst_t* pc, const ID& name, int_t n){
	XTAL_GLOBAL_INTERPRETER_LOCK{
		UncountedAny target = ff().temp_ = pop();
		UncountedAny self = ff().self();
		push_ff(pc + n, 1, 1, 0, self.cref());
		const Class& cls = target.cref().get_class();
		set_hint(cls, name);
		if(const Any& ret = member_cache(cls, name, ff().self(), null)){
			set_arg_this(target.cref());
			ret.call(myself());
		}
	}
	return ff().called_pc;
}
	
void VMachineImpl::carry_over(FunImpl* fun){
	FunFrame& f = ff();
	
	f.fun(fun);
	f.outer(fun->outer());

	f.variables_.clear();
	f.scopes.clear();

	f.called_pc = fun->pc()+f.psource;

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

	f.called_pc = fun->pc()+f.psource;

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


void VMachineImpl::set_local_variable(int_t pos, const Any& value){
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

const Any& VMachineImpl::local_variable(int_t pos){
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

Any VMachineImpl::append_backtrace(const inst_t* pc, const Any& e){
	if(e){
		Any ep = e;
		if(!ep.is(cast<const Class&>(builtin().member("Exception")))){
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

const inst_t* VMachineImpl::catch_body(const inst_t* pc, int_t stack_size, int_t fun_frames_size){
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
			pc = ef.core->catch_pc + ff().psource;
			push(Any(ef.core->end_pc));
			push(e);
		}else{
			pc = ef.core->finally_pc + ff().psource;
			push(e);
			push(Any(code().size()-1));
		}
		except_frames_.downsize(1);
		return pc;
	}

	return 0;
}

}//namespace
