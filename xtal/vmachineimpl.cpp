#include "xtal.h"

#include <map>
#include <fstream>
#include <math.h>

#include "funimpl.h"
#include "vmachineimpl.h"
#include "mapimpl.h"


namespace xtal{

namespace{

inline int_t get_u8(const u8* p){ return p[0]; }
inline int_t get_s8(const u8* p){ return (s8)p[0]; }

inline int_t get_u16(const u8* p){ return p[0]<<8 | p[1]; }
inline int_t get_s16(const u8* p){ return (s16)(p[0]<<8 | p[1]); }

}

VMachineImpl::VMachineImpl(){	
	
	myself_ = this;

	stack_.reserve(32);

	end_code_ = CODE_EXIT;
	throw_unsupported_error_code_ = CODE_THROW_UNSUPPORTED_ERROR;
	check_unsupported_code_ = CODE_CHECK_UNSUPPORTED;
	cleanup_call_code_ = CODE_CLEANUP_CALL;
	throw_nop_code_ = CODE_THROW_NULL;

	FunFrame& f = fun_frames_.push();
	f.pc = &end_code_;
	f.calling_state = FunFrame::CALLING_STATE_NONE;
	f.yieldable = false;

	resume_pc_ = 0;
}

VMachineImpl::~VMachineImpl(){

}

void VMachineImpl::push_ff(const u8* pc, int_t need_result_count, int_t result_flag, int_t ordered_arg_count, int_t named_arg_count, const Any& self){
	ff().pc = pc;
	FunFrame& f = fun_frames_.push();
	f.need_result_count = need_result_count;
	f.result_count = 0;
	f.result_flag = result_flag;
	f.ordered_arg_count = ordered_arg_count;
	f.named_arg_count = named_arg_count;
	f.calling_state = FunFrame::CALLING_STATE_NONE;
	f.pc = &throw_unsupported_error_code_;
	f.self(self);
	f.hint1(null);
	f.hint2(null);
	f.fun(null);

	f.outer(null);
	f.variables_.clear();
	f.arguments(null);
}

const Any& VMachineImpl::result(int_t pos){
	if(ff().calling_state==FunFrame::CALLING_STATE_PUSHED_FUN){
		const u8* pc = prev_ff().pc;
		prev_ff().pc = &end_code_;
		execute(ff().pc);
		fun_frames_.upsize(1);
		ff().pc = &cleanup_call_code_;
		prev_ff().pc = pc;
		ff().calling_state = FunFrame::CALLING_STATE_PUSHED_RESULT;
	}else if(ff().calling_state==FunFrame::CALLING_STATE_NONE){
		String hint1 = ff().hint1().get_class().object_name();
		String hint2 = ff().hint2();
		int_t rrc = need_result_count();
		pop_ff();
		//downsize(rrc);
		if(Any uerror = builtin().member("UnsupportedError")){
			throw uerror(Xt("%s :: '%s' は定義されていません")(
				hint1 ? hint1 : String("?"),
				hint2 ? hint2 : String("()"))); 
		}else{
			//printf("UnsupportedError %s %s\n", hint1 ? hint1.c_str() : "?", hint2 ? hint2.c_str() : "()");
			throw "未定義エラー";
		}
	}

	XTAL_ASSERT(ff().calling_state==FunFrame::CALLING_STATE_PUSHED_RESULT);
	
	if(pos<ff().need_result_count){
		return get(ff().need_result_count-pos-1);
	}else{
		return null;
	}
}
	
void VMachineImpl::carry_over(const Fun& fun){
	FunFrame& f = ff();
	f.calling_state = FunFrame::CALLING_STATE_PUSHED_FUN;
	
	f.fun(fun);
	f.outer(fun.outer());

	f.variables_.clear();
	f.scopes.clear();

	f.pc = fun.pc()+fun.code().data();
	f.yieldable = prev_ff().pc==&end_code_ ? false : prev_ff().yieldable;

	if(fun.used_args_object()){
		f.arguments(make_args(fun));
	}

	if(int_t size = fun.param_size()){
		f.scopes.push(fun.core());
		f.variables_.upsize(size);
		UncountedAny* vars=&f.variables_[size-1];
		for(int_t n = 0; n<size; ++n){
			vars[n] = arg(n, fun.param_name_at(n));
		}
	}
	
	int_t max_stack = f.fun().core()->max_stack;
	stack_.upsize(max_stack);
	stack_.downsize(f.ordered_arg_count+f.named_arg_count*2 + max_stack);
}
	
void VMachineImpl::adjust_result(int_t n){
	int_t need_result_count = ff().need_result_count;
	int_t result_flag = ff().result_flag;		
	ff().result_count = n;

	// 戻り値なんて要求してない
	if(need_result_count==0){
		downsize(n);
		return;
	}

	// 戻り値が一つも無いのでnullで埋める
	if(n==0){
		for(int_t i = n; i<need_result_count; ++i){
			push(null);
		}
	}

	// 戻り値の数と要求している戻り値の数が等しい
	if(need_result_count==n){
		return;
	}

	// この時点で、nもneed_result_countも1以上

	// 戻り値を切り捨てるフラグがついている
	if(result_flag&RESULT_DISCARD){
	
		// 要求している戻り値の数の方が、関数が返す戻り値より少ない
		if(need_result_count<n){
			// 戻り値を捨てる
			downsize(n-need_result_count);
		}else{
			// 要求している戻り値の数の方が、関数が返す戻り値より多い
			
			// nullで埋めとく
			for(int_t i = n; i<need_result_count; ++i){
				push(null);
			}
		}
	}else{
			
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
			const Array& temp(xtal::as<const Array&>(get()));
			if(temp){
				// 最後の要素の配列を展開する。
				Array ary(temp);
				XTAL_GLOBAL_INTERPRETER_LOCK{
					downsize(1);
					for(int_t i = 0; i<need_result_count-n+1; ++i){
						push(ary.at(i));
					}
				}
			}else{
				// 最後の要素が配列ではないので、nullで埋めとく
				for(int_t i = n; i<need_result_count; ++i){
					push(null);
				}
			}
		}
	}
}
	
void VMachineImpl::present_for_vm(const Fiber& fun, VMachineImpl* vm, bool add_succ_or_fail_result){
	// 結果をvmに渡す
	if(int_t need_result_count = vm->need_result_count()){
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

const u8* VMachineImpl::start_fiber(const Fiber& fun, VMachineImpl* vm, bool add_succ_or_fail_result){

	yield_result_count_ = 0;
	
	push_ff(&end_code_, vm->need_result_count(), vm->ff().result_flag, vm->ordered_arg_count(), vm->named_arg_count(), vm->get_arg_this());

	move(vm, vm->ordered_arg_count()+vm->named_arg_count()*2);
	
	resume_pc_ = 0;
	
	carry_over(fun);
	ff().yieldable = true;

	execute(ff().pc);
	ff().calling_state = FunFrame::CALLING_STATE_NONE;

	present_for_vm(fun, vm, add_succ_or_fail_result);

	vm->ff().pc = &cleanup_call_code_;
	vm->ff().calling_state = FunFrame::CALLING_STATE_PUSHED_RESULT;	
	
	return resume_pc_;
}

const u8* VMachineImpl::resume_fiber(const Fiber& fun, const u8* pc, VMachineImpl* vm, bool add_succ_or_fail_result){

	yield_result_count_ = 0;

	ff().calling_state = FunFrame::CALLING_STATE_PUSHED_FUN;
	ff().pc = pc;
	
	resume_pc_ = 0;

	move(vm, vm->ordered_arg_count()+vm->named_arg_count()*2);

	execute(ff().pc);
	ff().calling_state = FunFrame::CALLING_STATE_NONE;
	
	present_for_vm(fun, vm, add_succ_or_fail_result);

	vm->ff().pc = &cleanup_call_code_;
	vm->ff().calling_state = FunFrame::CALLING_STATE_PUSHED_RESULT;	

	return resume_pc_;
}

void VMachineImpl::exit_fiber(){
	try{
		yield_result_count_ = 0;

		ff().calling_state = FunFrame::CALLING_STATE_PUSHED_FUN;
		ff().pc = resume_pc_;
	
		resume_pc_ = 0;

		execute(&throw_nop_code_);
		ff().calling_state = FunFrame::CALLING_STATE_NONE;
	}catch(const Any&){
		reset();
	}
}

void VMachineImpl::reset(){
	stack_.resize(0);
	except_frames_.resize(0);
	fun_frames_.resize(1);
	FunFrame& f = fun_frames_.top();
	f.pc = &end_code_;
	f.calling_state = FunFrame::CALLING_STATE_NONE;
	f.yieldable = false;
	
	f.self(null);
	f.hint1(null);
	f.hint2(null);
	f.fun(null);
	f.outer(null);
	f.variables_.clear();
	f.arguments(null);
}
	
void VMachineImpl::recycle_call(){
	FunFrame& f = ff();
	downsize(f.ordered_arg_count+f.named_arg_count*2);
	f.ordered_arg_count = 0;
	f.named_arg_count = 0;
	f.pc = &throw_unsupported_error_code_;
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

void VMachineImpl::push_ff_args(const u8* pc, int_t need_result_count, int_t result_flag, int_t ordered_arg_count, int_t named_arg_count, const Any& self){
	push_args(named_arg_count);
	const Arguments& a = ff().arguments();
	push_ff(pc, need_result_count, result_flag, a.impl()->ordered_.size()+ordered_arg_count, a.impl()->named_.size()+named_arg_count, self);
}

void VMachineImpl::recycle_ff(const u8* pc, int_t ordered_arg_count, int_t named_arg_count, const Any& self){
	ff().pc = pc;
	FunFrame& f = ff();
	f.ordered_arg_count = ordered_arg_count;
	f.named_arg_count = named_arg_count;
	f.self(self);
	f.calling_state = FunFrame::CALLING_STATE_NONE;
	f.pc = &throw_unsupported_error_code_;
}

void VMachineImpl::recycle_ff_args(const u8* pc, int_t ordered_arg_count, int_t named_arg_count, const Any& self){
	push_args(named_arg_count);
	const Arguments& a = ff().arguments();
	recycle_ff(pc, a.impl()->ordered_.size()+ordered_arg_count, a.impl()->named_.size()+named_arg_count, self);
}

void VMachineImpl::setup_call(int_t need_result_count){
	push_ff(&end_code_, need_result_count, 0, 0, 0, null);
}

void VMachineImpl::setup_call(int_t need_result_count, const Any& a1){
	push_ff(&end_code_, need_result_count, 0, 1, 0, null);
	push(a1);
}

void VMachineImpl::setup_call(int_t need_result_count, const Any& a1, const Any& a2){
	push_ff(&end_code_, need_result_count, 0, 2, 0, null);
	push(a1); push(a2);
}

void VMachineImpl::setup_call(int_t need_result_count, const Any& a1, const Any& a2, const Any& a3){
	push_ff(&end_code_, need_result_count, 0, 3, 0, null);
	push(a1); push(a2); push(a3);
}

void VMachineImpl::setup_call(int_t need_result_count, const Any& a1, const Any& a2, const Any& a3, const Any& a4){
	push_ff(&end_code_, need_result_count, 0, 4, 0, null);
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

const u8* VMachineImpl::send1(const u8* pc, const ID& id, int_t n){
	XTAL_GLOBAL_INTERPRETER_LOCK{
		Any a = pop();
		a.send(id, inner_setup_call(pc+n, 0)); 
	}
	return ff().pc;
}

const u8* VMachineImpl::send2(const u8* pc, const ID& id, int_t n){
	XTAL_GLOBAL_INTERPRETER_LOCK{
		Any a = get(1), b = get(); downsize(2);
		a.send(id, inner_setup_call(pc+n, 1, b)); 
	}
	return ff().pc;
}

const u8* VMachineImpl::send2r(const u8* pc, const ID& id, int_t n){
	XTAL_GLOBAL_INTERPRETER_LOCK{
		Any a = get(), b = get(1); downsize(2);
		a.send(id, inner_setup_call(pc+n, 1, b)); 
	}
	return ff().pc;
}

//std::ofstream log("code.txt");
//#define XTAL_VM_CASE(key) goto begin; case key: log<<(#key)<<std::endl;
//#define XTAL_VM_CASE(key) goto begin; case key: printf("%s\n", #key);
#define XTAL_VM_CASE(key) goto begin; case key:

void VMachineImpl::execute(const u8* start){ XTAL_GLOBAL_INTERPRETER_UNLOCK{
	
	typedef const u8* pc_t;
	register pc_t pc = start;

	int_t stack_size = stack_.size();
	int_t fun_frames_size = fun_frames_.size();

retry:try{begin:

/*
CodeType ct=(CodeType)*pc;
CodeType ct1=(CodeType)*(pc+1);
CodeType ct2=(CodeType)*(pc+2);
CodeType ct3=(CodeType)*(pc+3);
CodeType ct4=(CodeType)*(pc+4);
*/

switch(*pc){

	XTAL_NODEFAULT;
	
	XTAL_VM_CASE(CODE_NOP){ pc+=1; }
	XTAL_VM_CASE(CODE_PUSH_NULL){ push(null); pc+=1; }
	XTAL_VM_CASE(CODE_PUSH_TRUE){ push(UncountedAny(true).cref()); pc+=1; }
	XTAL_VM_CASE(CODE_PUSH_NOP){ push(nop()); pc+=1; }
	
	XTAL_VM_CASE(CODE_PUSH_INT_0){ push(UncountedAny(0).cref()); pc+=1; }
	XTAL_VM_CASE(CODE_PUSH_INT_1){ push(UncountedAny(1).cref()); pc+=1; }
	XTAL_VM_CASE(CODE_PUSH_INT_2){ push(UncountedAny(2).cref()); pc+=1; }
	XTAL_VM_CASE(CODE_PUSH_INT_3){ push(UncountedAny(3).cref()); pc+=1; }
	XTAL_VM_CASE(CODE_PUSH_INT_4){ push(UncountedAny(4).cref()); pc+=1; }
	XTAL_VM_CASE(CODE_PUSH_INT_5){ push(UncountedAny(5).cref()); pc+=1; }
	
	XTAL_VM_CASE(CODE_PUSH_FLOAT_0){ push(UncountedAny(0.0f).cref()); pc+=1; }
	XTAL_VM_CASE(CODE_PUSH_FLOAT_0_25){ push(UncountedAny(0.25f).cref()); pc+=1; }
	XTAL_VM_CASE(CODE_PUSH_FLOAT_0_5){ push(UncountedAny(0.5f).cref()); pc+=1; }
	XTAL_VM_CASE(CODE_PUSH_FLOAT_1){ push(UncountedAny(1.0f).cref()); pc+=1; }
	XTAL_VM_CASE(CODE_PUSH_FLOAT_2){ push(UncountedAny(2.0f).cref()); pc+=1; }
	XTAL_VM_CASE(CODE_PUSH_FLOAT_3){ push(UncountedAny(3.0f).cref()); pc+=1; }

	XTAL_VM_CASE(CODE_PUSH_INT_1BYTE){ push(UncountedAny(get_u8(pc+1)).cref()); pc+=2; }
	XTAL_VM_CASE(CODE_PUSH_INT_2BYTE){ push(UncountedAny(get_s16(pc+1)).cref()); pc+=3; }
	
	XTAL_VM_CASE(CODE_PUSH_ARRAY){ pc = PUSH_ARRAY(pc); }
	XTAL_VM_CASE(CODE_ARRAY_APPEND){ pc = ARRAY_APPEND(pc); }
	XTAL_VM_CASE(CODE_PUSH_MAP){ pc = PUSH_MAP(pc); }
	XTAL_VM_CASE(CODE_MAP_APPEND){ pc = MAP_APPEND(pc); }

	XTAL_VM_CASE(CODE_PUSH_CALLEE){ push(ff().fun()); pc+=1; }
	XTAL_VM_CASE(CODE_PUSH_FUN){ pc = PUSH_FUN(pc); }	
	XTAL_VM_CASE(CODE_PUSH_CURRENT_CONTEXT){ push(decolonize()); pc+=1; }	
	XTAL_VM_CASE(CODE_PUSH_CURRENT_CONTINUATION){ pc = CURRENT_CONTINUATION(pc); }	
	XTAL_VM_CASE(CODE_PUSH_ARGS){ push(ff().arguments()); pc+=1; }
	XTAL_VM_CASE(CODE_PUSH_THIS){ push(ff().self()); pc+=1; }

	XTAL_VM_CASE(CODE_IF){ pc = pop()!=null ? pc+3 : pc+get_s16(pc+1); }
	XTAL_VM_CASE(CODE_UNLESS){ pc = pop()==null ? pc+3 : pc+get_s16(pc+1); }
	XTAL_VM_CASE(CODE_GOTO){ pc = pc+get_s16(pc+1); }

	XTAL_VM_CASE(CODE_INSERT_1){ UncountedAny temp = get(); set(get(1)); set(1, temp.cref()); pc+=1; }
	XTAL_VM_CASE(CODE_INSERT_2){ UncountedAny temp = get(); set(get(1)); set(1, get(2)); set(2, temp.cref()); pc+=1; }
	XTAL_VM_CASE(CODE_INSERT_3){ UncountedAny temp = get(); set(get(1)); set(1, get(2)); set(2, get(3)); set(3, temp.cref()); pc+=1; }
	
	XTAL_VM_CASE(CODE_POP){ downsize(1); pc+=1; }
	XTAL_VM_CASE(CODE_DUP){ dup(); pc+=1; }

	XTAL_VM_CASE(CODE_SET_NAME){ pc = SET_NAME(pc); }

	XTAL_VM_CASE(CODE_CHECK_UNSUPPORTED){ return_result(nop()); pc = ff().pc; }
	XTAL_VM_CASE(CODE_ASSERT){ pc = CHECK_ASSERT(pc, stack_size, fun_frames_size); }

	XTAL_VM_CASE(CODE_CALL){ pc = CALL(pc); }
	XTAL_VM_CASE(CODE_CALLEE){ pc = CALLEE(pc); }
	XTAL_VM_CASE(CODE_SEND){ pc = SEND(pc); }
	XTAL_VM_CASE(CODE_SEND_IF_DEFINED){ pc = SEND_IF_DEFINED(pc); }

	XTAL_VM_CASE(CODE_CLEANUP_CALL){ pop_ff(); pc = ff().pc; }

	XTAL_VM_CASE(CODE_YIELD){ YIELD(pc); return; }

	XTAL_VM_CASE(CODE_RETURN_0){ pc = RETURN(0); }
	XTAL_VM_CASE(CODE_RETURN_1){ pc = RETURN(1); }
	XTAL_VM_CASE(CODE_RETURN_2){ pc = RETURN(2); }
	XTAL_VM_CASE(CODE_RETURN_N){ pc = RETURN(get_u8(pc+1)); }

	XTAL_VM_CASE(CODE_EXIT){ resume_pc_ = 0; return; }

	XTAL_VM_CASE(CODE_THROW_UNSUPPORTED_ERROR){ THROW_UNSUPPROTED_ERROR(stack_size, fun_frames_size); }
	XTAL_VM_CASE(CODE_THROW_NULL){ throw null; }

	XTAL_VM_CASE(CODE_BLOCK_BEGIN){ 
		FrameCore* p = code().get_frame_core(get_u16(pc+1)); 
		ff().scopes.push(p); 
		ff().variables_.upsize(p->variable_size); 
		pc+=3; 
	}

	XTAL_VM_CASE(CODE_BLOCK_END){ pc = BLOCK_END(pc); }

	XTAL_VM_CASE(CODE_BLOCK_END_NOT_ON_HEAP){
		ff().variables_.downsize(ff().scopes.top()->variable_size);
		ff().scopes.downsize(1);
		pc+=1;
	}

	XTAL_VM_CASE(CODE_FRAME_BEGIN){ pc = FRAME_BEGIN(pc); }
	XTAL_VM_CASE(CODE_FRAME_END){ pc = FRAME_END(pc); }

	XTAL_VM_CASE(CODE_TRY_BEGIN){ pc = TRY_BEGIN(pc); }
	XTAL_VM_CASE(CODE_TRY_END){ pc = TRY_END(pc); }
	XTAL_VM_CASE(CODE_THROW){ THROW(pc, stack_size, fun_frames_size); }

	XTAL_VM_CASE(CODE_INSTANCE_VARIABLE){ pc = INSTANCE_VARIABLE(pc); }
	XTAL_VM_CASE(CODE_SET_INSTANCE_VARIABLE){ pc = SET_INSTANCE_VARIABLE(pc); }

	XTAL_VM_CASE(CODE_LOCAL_0){ push(ff().variable(0)); pc+=1; }
	XTAL_VM_CASE(CODE_LOCAL_1){ push(ff().variable(1)); pc+=1; }
	XTAL_VM_CASE(CODE_LOCAL_2){ push(ff().variable(2)); pc+=1; }
	XTAL_VM_CASE(CODE_LOCAL_3){ push(ff().variable(3)); pc+=1; }
	XTAL_VM_CASE(CODE_LOCAL){ LOCAL_VARIABLE(get_u8(pc+1)); pc+=2; }
	XTAL_VM_CASE(CODE_LOCAL_NOT_ON_HEAP){ push(ff().variable(get_u8(pc+1))); pc+=2; }
	XTAL_VM_CASE(CODE_LOCAL_W){ LOCAL_VARIABLE(get_u16(pc+1)); pc+=3; }
	XTAL_VM_CASE(CODE_SET_LOCAL_0){ ff().variable(0, pop()); pc+=1; }
	XTAL_VM_CASE(CODE_SET_LOCAL_1){ ff().variable(1, pop()); pc+=1; }
	XTAL_VM_CASE(CODE_SET_LOCAL_2){ ff().variable(2, pop()); pc+=1; }
	XTAL_VM_CASE(CODE_SET_LOCAL_3){ ff().variable(3, pop()); pc+=1; }
	XTAL_VM_CASE(CODE_SET_LOCAL){ SET_LOCAL_VARIABLE(get_u8(pc+1)); pc+=2; }
	XTAL_VM_CASE(CODE_SET_LOCAL_NOT_ON_HEAP){ ff().variable(get_u8(pc+1), pop()); pc+=2; }
	XTAL_VM_CASE(CODE_SET_LOCAL_W){ SET_LOCAL_VARIABLE(get_u16(pc+1)); pc+=3; }

	XTAL_VM_CASE(CODE_GLOBAL){ pc = GLOBAL_VARIABLE(pc); }
	XTAL_VM_CASE(CODE_SET_GLOBAL){ pc = SET_GLOBAL_VARIABLE(pc); }
	XTAL_VM_CASE(CODE_DEFINE_GLOBAL){ pc = DEFINE_GLOBAL_VARIABLE(pc); }

	XTAL_VM_CASE(CODE_MEMBER){ pc = MEMBER(pc); }
	XTAL_VM_CASE(CODE_MEMBER_IF_DEFINED){ pc = MEMBER_IF_DEFINED(pc); }
	XTAL_VM_CASE(CODE_DEFINE_MEMBER){ pc = DEFINE_MEMBER(pc); }

	XTAL_VM_CASE(CODE_AT){ pc = AT(pc); }
	XTAL_VM_CASE(CODE_SET_AT){ pc = SET_AT(pc); }

	XTAL_VM_CASE(CODE_ONCE){ pc = ONCE(pc); }

	XTAL_VM_CASE(CODE_GET_VALUE){ push(code().get_value(get_u16(pc+1))); pc+=3; }
	XTAL_VM_CASE(CODE_SET_VALUE){ code().set_value(get_u16(pc+1), pop()); pc+=3; }
	
	XTAL_VM_CASE(CODE_PUSH_GOTO){ push(UncountedAny((pc+get_s16(pc+1))-source()).cref()); pc+=3; }
	XTAL_VM_CASE(CODE_POP_GOTO){ pc = source()+pop().ivalue(); }

	XTAL_VM_CASE(CODE_NOT){ set(UncountedAny(!get().to_b()).cref()); pc+=1; }
	XTAL_VM_CASE(CODE_POS){ pc = POS(pc); }
	XTAL_VM_CASE(CODE_NEG){ pc = NEG(pc); }
	XTAL_VM_CASE(CODE_COM){ pc = COM(pc); }
	XTAL_VM_CASE(CODE_CLONE){ pc = CLONE(pc); }

	XTAL_VM_CASE(CODE_ADD){ pc = ADD(pc); }
	XTAL_VM_CASE(CODE_SUB){ pc = SUB(pc); }
	XTAL_VM_CASE(CODE_CAT){ pc = CAT(pc); }
	XTAL_VM_CASE(CODE_MUL){ pc = MUL(pc); }
	XTAL_VM_CASE(CODE_DIV){ pc = DIV(pc); }
	XTAL_VM_CASE(CODE_MOD){ pc = MOD(pc); }
	XTAL_VM_CASE(CODE_AND){ pc = AND(pc); }
	XTAL_VM_CASE(CODE_OR){ pc = OR(pc); }
	XTAL_VM_CASE(CODE_XOR){ pc = XOR(pc); }
	XTAL_VM_CASE(CODE_SHR){ pc = SHR(pc); }
	XTAL_VM_CASE(CODE_USHR){ pc = USHR(pc); }
	XTAL_VM_CASE(CODE_SHL){ pc = SHL(pc); }

	XTAL_VM_CASE(CODE_EQ){ pc = EQ(pc); }
	XTAL_VM_CASE(CODE_NE){ pc = NE(pc); }
	XTAL_VM_CASE(CODE_LT){ pc = LT(pc); }
	XTAL_VM_CASE(CODE_GT){ pc = GT(pc); }
	XTAL_VM_CASE(CODE_LE){ pc = LE(pc); }
	XTAL_VM_CASE(CODE_GE){ pc = GE(pc); }
	XTAL_VM_CASE(CODE_RAW_EQ){ pc = RAW_EQ(pc); }
	XTAL_VM_CASE(CODE_RAW_NE){ pc = RAW_NE(pc); }
	XTAL_VM_CASE(CODE_IS){ pc = IS(pc); }
	XTAL_VM_CASE(CODE_NIS){ pc = NIS(pc); }

	XTAL_VM_CASE(CODE_EQ_IF){ pc = EQ_IF(pc); }
	XTAL_VM_CASE(CODE_NE_IF){ pc = NE_IF(pc); }
	XTAL_VM_CASE(CODE_LT_IF){ pc = LT_IF(pc); }
	XTAL_VM_CASE(CODE_GT_IF){ pc = GT_IF(pc); }
	XTAL_VM_CASE(CODE_LE_IF){ pc = LE_IF(pc); }
	XTAL_VM_CASE(CODE_GE_IF){ pc = GE_IF(pc); }

	XTAL_VM_CASE(CODE_INC){ pc = INC(pc); }
	XTAL_VM_CASE(CODE_DEC){ pc = DEC(pc); }
	XTAL_VM_CASE(CODE_LOCAL_NOT_ON_HEAP_INC){ pc = LOCAL_NOT_ON_HEAP_INC(pc); }
	XTAL_VM_CASE(CODE_LOCAL_NOT_ON_HEAP_DEC){ pc = LOCAL_NOT_ON_HEAP_DEC(pc); }

	XTAL_VM_CASE(CODE_ADD_ASSIGN){ pc = ADD_ASSIGN(pc); }
	XTAL_VM_CASE(CODE_SUB_ASSIGN){ pc = SUB_ASSIGN(pc); }
	XTAL_VM_CASE(CODE_CAT_ASSIGN){ pc = CAT_ASSIGN(pc); }
	XTAL_VM_CASE(CODE_MUL_ASSIGN){ pc = MUL_ASSIGN(pc); }
	XTAL_VM_CASE(CODE_DIV_ASSIGN){ pc = DIV_ASSIGN(pc); }
	XTAL_VM_CASE(CODE_MOD_ASSIGN){ pc = MOD_ASSIGN(pc); }
	XTAL_VM_CASE(CODE_AND_ASSIGN){ pc = AND_ASSIGN(pc); }
	XTAL_VM_CASE(CODE_OR_ASSIGN){ pc = OR_ASSIGN(pc); }
	XTAL_VM_CASE(CODE_XOR_ASSIGN){ pc = XOR_ASSIGN(pc); }
	XTAL_VM_CASE(CODE_SHR_ASSIGN){ pc = SHR_ASSIGN(pc); }
	XTAL_VM_CASE(CODE_USHR_ASSIGN){ pc = USHR_ASSIGN(pc); }
	XTAL_VM_CASE(CODE_SHL_ASSIGN){ pc = SHL_ASSIGN(pc); }

	XTAL_VM_CASE(CODE_BREAKPOINT){ pc = BREAKPOINT(pc); }

goto begin;

}}catch(const Any& e){
	pc = CATCH_BODY(pc, e, stack_size, fun_frames_size);
	goto retry;
}/*catch(...){
	pc = CATCH_BODY(pc, null, stack_size, fun_frames_size);
	throw;
}*/

}}
	
const u8* VMachineImpl::ARRAY_APPEND(const u8* pc){
	XTAL_GLOBAL_INTERPRETER_LOCK{
		cast<Array*>(get(1))->push_back(get()); 
		downsize(1);
	}
	return pc + 1;
}
	
const u8* VMachineImpl::MAP_APPEND(const u8* pc){
	XTAL_GLOBAL_INTERPRETER_LOCK{
		cast<Map*>(get(2))->set_at(get(1), get()); 
		downsize(2);	
	}
	return pc + 1;
}

const u8* VMachineImpl::SET_NAME(const u8* pc){
	XTAL_GLOBAL_INTERPRETER_LOCK{
		get().set_object_name(symbol(get_u16(pc+1)), 1, null);
	}
	return pc + 3;
}

const u8* VMachineImpl::CALL(const u8* pc){
	XTAL_GLOBAL_INTERPRETER_LOCK{
		UncountedAny self = ff().self();
		Any target = pop();
		switch(get_u8(pc + 4) & 3){
			XTAL_NODEFAULT;
			XTAL_CASE(0){ push_ff(pc+5, get_u8(pc+3), get_u8(pc+4), get_u8(pc+1), get_u8(pc+2), self.cref()); }
			XTAL_CASE(1){ push_ff_args(pc+5, get_u8(pc+3), get_u8(pc+4), get_u8(pc+1), get_u8(pc+2), self.cref()); }
			XTAL_CASE(2){ recycle_ff(pc+5, get_u8(pc+1), get_u8(pc+2), self.cref()); }
			XTAL_CASE(3){ recycle_ff_args(pc+5, get_u8(pc+1), get_u8(pc+2), self.cref()); }
		}
		target.call(myself());
	}
	return ff().pc;	
}
	
const u8* VMachineImpl::CALLEE(const u8* pc){
	UncountedAny fun = ff().fun();
	UncountedAny self = ff().self();
	switch(get_u8(pc + 4) & 3){
		XTAL_NODEFAULT;
		XTAL_CASE(0){ push_ff(pc+5, get_u8(pc+3), get_u8(pc+4), get_u8(pc+1), get_u8(pc+2), self.cref()); }
		XTAL_CASE(1){ push_ff_args(pc+5, get_u8(pc+3), get_u8(pc+4), get_u8(pc+1), get_u8(pc+2), self.cref()); }
		XTAL_CASE(2){ recycle_ff(pc+5, get_u8(pc+1), get_u8(pc+2), self.cref()); }
		XTAL_CASE(3){ recycle_ff_args(pc+5, get_u8(pc+1), get_u8(pc+2), self.cref()); }
	}
	carry_over((const Fun&)fun);
	return ff().pc;	
}

const u8* VMachineImpl::SEND(const u8* pc){
	XTAL_GLOBAL_INTERPRETER_LOCK{
		const ID& sym = symbol(get_u16(pc+1));
		UncountedAny self = ff().self();
		Any target = pop();
		switch(get_u8(pc + 6) & 3){
			XTAL_NODEFAULT;
			XTAL_CASE(0){ push_ff(pc+7, get_u8(pc+5), get_u8(pc+6), get_u8(pc+3), get_u8(pc+4), self.cref()); }
			XTAL_CASE(1){ push_ff_args(pc+7, get_u8(pc+5), get_u8(pc+6), get_u8(pc+3), get_u8(pc+4), self.cref()); }
			XTAL_CASE(2){ recycle_ff(pc+7, get_u8(pc+3), get_u8(pc+4), self.cref()); }
			XTAL_CASE(3){ recycle_ff_args(pc+7, get_u8(pc+3), get_u8(pc+4), self.cref()); }
		}
		target.send(sym, myself());
	}
	return ff().pc; 	
}

const u8* VMachineImpl::SEND_IF_DEFINED(const u8* pc){
	XTAL_GLOBAL_INTERPRETER_LOCK{
		const ID& sym = symbol(get_u16(pc+1));
		UncountedAny self = ff().self();
		Any target = pop();
		switch(get_u8(pc + 6) & 3){
			XTAL_NODEFAULT;
			XTAL_CASE(0){ push_ff(pc+7, get_u8(pc+5), get_u8(pc + 6), get_u8(pc+3), get_u8(pc+4), self.cref()); }
			XTAL_CASE(1){ push_ff_args(pc+7, get_u8(pc+5), get_u8(pc + 6), get_u8(pc+3), get_u8(pc+4), self.cref()); }
			XTAL_CASE(2){ recycle_ff(pc+7, get_u8(pc+3), get_u8(pc+4), self.cref()); }
			XTAL_CASE(3){ recycle_ff_args(pc+7, get_u8(pc+3), get_u8(pc+4), self.cref()); }
		}
		ff().pc = &check_unsupported_code_;
		target.send(sym, myself()); 
	}
	return ff().pc; 	
}

const u8* VMachineImpl::FRAME_BEGIN(const u8* pc){
	XTAL_GLOBAL_INTERPRETER_LOCK{
		FrameCore* p = code().get_frame_core(get_u16(pc+1)); 
		switch(p->kind){
			XTAL_NODEFAULT;
			
			XTAL_CASE(KIND_BLOCK){ 
				ff().outer(Frame(decolonize(), code(), p));
				ff().scopes.push(0); 
				return pc + 3;
			}
			
			XTAL_CASE(KIND_CLASS){ 
				Class cp = new_xclass(decolonize(), code(), p);
				
				int_t n = get_u8(pc+3);
				for(int_t i = 0; i<n; ++i){
					cp.inherit(pop());
				}

				ff().outer(cp);
				ff().scopes.push(0); 
				return pc + 4;
			}
		}
	}
	return 0;
}

const u8* VMachineImpl::FRAME_END(const u8* pc){
	push(ff().outer());
	ff().outer(ff().outer().outer());
	ff().scopes.downsize(1);
	return pc+1;
}

const u8* VMachineImpl::BLOCK_END(const u8* pc){
	if(ff().scopes.top()){ 
		ff().variables_.downsize(ff().scopes.top()->variable_size);
	}else{
		ff().outer(ff().outer().outer()); 
	}
	ff().scopes.downsize(1);
	return pc+1;
}

const u8* VMachineImpl::TRY_BEGIN(const u8* pc){
	ExceptFrame& ef = except_frames_.push();
	
	ef.catch_pc = pc+get_s16(pc+1);
	if(ef.catch_pc==pc)
		ef.catch_pc = 0;
	ef.finally_pc = pc+get_s16(pc+3);
	ef.end_pc = pc+get_s16(pc+5);
	ef.scope_count = ff().scopes.size();
	ef.stack_count = stack_size();
	ef.fun_frame_count = fun_frames_.size();
	
	return pc+7;
}

const u8* VMachineImpl::TRY_END(const u8* pc){
	return except_frames_.pop().finally_pc;
}

void VMachineImpl::SET_LOCAL_VARIABLE(int_t pos){
	int_t variables_size = ff().variables_.size();
	if(pos<variables_size){
		ff().variable(pos, pop());
		return;
	}	
	pos-=variables_size;
	const Frame* outer = &ff().outer();
	while(1){
		variables_size = outer->block_size();
		if(pos<variables_size){
			outer->set_member_direct(pos, pop());
			return;
		}
		pos-=variables_size;
		outer = &outer->outer();
	}
}

void VMachineImpl::LOCAL_VARIABLE(int_t pos){
	int_t variables_size = ff().variables_.size();
	if(pos<variables_size){
		push(ff().variable(pos));
		return;
	}
	pos-=variables_size;
	const Frame* outer = &ff().outer();
	XTAL_GLOBAL_INTERPRETER_LOCK{
		while(1){
			variables_size = outer->block_size();
			if(pos<variables_size){
				push(outer->member_direct(pos));
				return;
			}
			pos-=variables_size;
			outer = &outer->outer();
		}
	}
}

const u8* VMachineImpl::GLOBAL_VARIABLE(const u8* pc){
	XTAL_GLOBAL_INTERPRETER_LOCK{
		if(const Any& ret = code().toplevel().member(symbol(get_u16(pc+1)))){
			push(ret);
		}else{
			throw unsupported_error("toplevel", symbol(get_u16(pc+1)));
		}
	}
	return pc+3;
}

const u8* VMachineImpl::SET_GLOBAL_VARIABLE(const u8* pc){
	XTAL_GLOBAL_INTERPRETER_LOCK{
		code().toplevel().set_member(symbol(get_u16(pc+1)), pop());
	}
	return pc+3;
}

const u8* VMachineImpl::DEFINE_GLOBAL_VARIABLE(const u8* pc){
	XTAL_GLOBAL_INTERPRETER_LOCK{
		code().toplevel().def(symbol(get_u16(pc+1)), pop());
	}
	return pc+3;
}

const u8* VMachineImpl::SET_INSTANCE_VARIABLE(const u8* pc){
	int_t n = get_u8(pc+1);
	int_t m = get_u16(pc+2);
	UncountedAny self = ff().self();
	if(HaveInstanceVariables* p = self.cref().type()==TYPE_BASE ? self.cref().impl()->have_instance_variables() : (HaveInstanceVariables*)0){
		p->set_variable(n, code().get_frame_core(m), pop());
	}else{
		XTAL_GLOBAL_INTERPRETER_LOCK{
			throw builtin().member("BadInstanceVariableError")(Xt("Xtal Runtime Error 1003"));
		}
	}
	return pc+4;
}

const u8* VMachineImpl::INSTANCE_VARIABLE(const u8* pc){
	int_t n = get_u8(pc+1);
	int_t m = get_u16(pc+2);
	UncountedAny self = ff().self();
	if(HaveInstanceVariables* p = self.cref().type()==TYPE_BASE ? self.cref().impl()->have_instance_variables() : (HaveInstanceVariables*)0){
		push(p->variable(n, code().get_frame_core(m)));
	}else{
		XTAL_GLOBAL_INTERPRETER_LOCK{
			throw builtin().member("BadInstanceVariableError")(Xt("Xtal Runtime Error 1003"));
		}
	}
	return pc+4;
}

const u8* VMachineImpl::ONCE(const u8* pc){
	const Any& ret = code().get_value(get_u16(pc+3));
	if(!ret.raweq(nop())){
		push(ret);
		return pc+get_s16(pc+1);
	}
	return pc+5;
}
	
const u8* VMachineImpl::MEMBER(const u8* pc){
	XTAL_GLOBAL_INTERPRETER_LOCK{
		const ID& name = symbol(get_u16(pc+1));
		const Any& target = get();
		if(const Any& ret = target.member(name)){
			set(ret);
		}else{
			throw unsupported_error(target.object_name(), symbol(get_u16(pc+1)));
		}
	}
	return pc+3; 
}

const u8* VMachineImpl::MEMBER_IF_DEFINED(const u8* pc){
	XTAL_GLOBAL_INTERPRETER_LOCK{
		const ID& name = symbol(get_u16(pc+1));
		const Any& target = get();
		if(const Any& ret = target.member(name)){
			set(ret);
		}else{
			set(nop());
		}
	}
	return pc+3; 
}

const u8* VMachineImpl::DEFINE_MEMBER(const u8* pc){
	XTAL_GLOBAL_INTERPRETER_LOCK{
		const ID& name = symbol(get_u16(pc+1));
		const Any& value = get();
		const Any& target = get(1);
		target.def(name, value); 
		downsize(2);
	}
	return pc+3; 
}

const u8* VMachineImpl::AT(const u8* pc){ 
	XTAL_GLOBAL_INTERPRETER_LOCK{
		Any idx = pop();
		Any target = pop();
		inner_setup_call(pc+1, 1, idx);
		target.send(Xid(op_at), myself());
	}
	return ff().pc; 
}

const u8* VMachineImpl::SET_AT(const u8* pc){ 
	XTAL_GLOBAL_INTERPRETER_LOCK{
		Any idx = pop();
		Any target = pop();
		Any value = pop();
		inner_setup_call(pc+1, 0, idx, value);
		target.send(Xid(op_set_at), myself());
	}
	return ff().pc;
}

const u8* VMachineImpl::PUSH_ARRAY(const u8* pc){
	XTAL_GLOBAL_INTERPRETER_LOCK{
		int_t size = get_u8(pc+1);
		Array ary(size);
		for(int_t i = 0; i<size; ++i){
			ary.set_at(i, get(size-1-i));
		}
		downsize(size);
		push(ary);
	}
	return pc+2;
}
	
const u8* VMachineImpl::PUSH_MAP(const u8* pc){
	XTAL_GLOBAL_INTERPRETER_LOCK{
		int_t size = get_u8(pc+1);
		Map map;
		for(int_t i = 0; i<size; ++i){
			map.set_at(get(size*2-i*2-0-1), get(size*2-i*2-1-1));
		}
		downsize(size*2);
		push(map);
	}
	return pc+2;
}
	
const u8* VMachineImpl::PUSH_FUN(const u8* pc){
	int_t type = get_u8(pc+1), table_n = get_u16(pc+2), end = get_u16(pc+4);
	XTAL_GLOBAL_INTERPRETER_LOCK{
		switch(type){
			XTAL_NODEFAULT;
			
			XTAL_CASE(KIND_FUN){ 
				push(Fun(decolonize(), ff().self(), code(), code().get_fun_core(table_n))); 
			}

			XTAL_CASE(KIND_METHOD){ 
				push(Method(decolonize(), code(), code().get_fun_core(table_n))); 
			}

			XTAL_CASE(KIND_FIBER){ 
				push(Fiber(decolonize(), ff().self(), code(), code().get_fun_core(table_n)));
			}
		}
	}
	return pc+end;
}

const u8* VMachineImpl::RETURN(int_t n){
	adjust_result(n);
	pop_ff();
	return ff().pc;
}

void VMachineImpl::YIELD(const u8* pc){
	yield_result_count_ = get_u8(pc + 1);	
	if(ff().yieldable){
		resume_pc_ = pc + 2;
	}else{
		downsize(yield_result_count_);
		XTAL_GLOBAL_INTERPRETER_LOCK{
			throw builtin().member("BadYieldError")(Xt("Xtal Runtime Error 1012")); 
		}
	}
}
const u8* VMachineImpl::POS(const u8* pc){
	switch(get().type()){XTAL_DEFAULT;
		XTAL_CASE(TYPE_INT){ return pc+1; }
		XTAL_CASE(TYPE_FLOAT){ return pc+1; }
	}
	return send1(pc, Xid(op_pos));
}

const u8* VMachineImpl::NEG(const u8* pc){
	switch(get().type()){XTAL_DEFAULT;
		XTAL_CASE(TYPE_INT){ set(UncountedAny(-get().ivalue()).cref()); return pc+1; }
		XTAL_CASE(TYPE_FLOAT){ set(UncountedAny(-get().fvalue()).cref()); return pc+1; }
	}
	return send1(pc, Xid(op_neg));
}

const u8* VMachineImpl::COM(const u8* pc){
	switch(get().type()){XTAL_DEFAULT;
		XTAL_CASE(TYPE_INT){ set(UncountedAny(~get().ivalue()).cref()); return pc; }
	}
	return send1(pc, Xid(op_com));
}

const u8* VMachineImpl::CLONE(const u8* pc){
	switch(get().type()){XTAL_DEFAULT;
		XTAL_CASE(TYPE_INT){ return pc+1; }
		XTAL_CASE(TYPE_FLOAT){ return pc+1; }
	}
	return send1(pc, Xid(op_clone));
}

const u8* VMachineImpl::ADD(const u8* pc){ 
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

const u8* VMachineImpl::SUB(const u8* pc){ 
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

const u8* VMachineImpl::CAT(const u8* pc){ 
	return send2(pc, Xid(op_cat));
}

const u8* VMachineImpl::MUL(const u8* pc){ 
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

const u8* VMachineImpl::DIV(const u8* pc){ 
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

const u8* VMachineImpl::MOD(const u8* pc){ 
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

const u8* VMachineImpl::AND(const u8* pc){ 
	switch(get(1).type()){XTAL_DEFAULT;
		XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).ivalue() & get().ivalue()).cref()); downsize(1); return pc+1; }
		}}
	}
	return send2(pc, Xid(op_and));
}

const u8* VMachineImpl::OR(const u8* pc){ 
	switch(get(1).type()){XTAL_DEFAULT;
		XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).ivalue() | get().ivalue()).cref()); downsize(1); return pc+1; }
		}}
	}
	return send2(pc, Xid(op_or));
}

const u8* VMachineImpl::XOR(const u8* pc){ 
	switch(get(1).type()){XTAL_DEFAULT;
		XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).ivalue() ^ get().ivalue()).cref()); downsize(1); return pc+1; }
		}}
	}
	return send2(pc, Xid(op_xor));
}

const u8* VMachineImpl::SHR(const u8* pc){ 
	switch(get(1).type()){XTAL_DEFAULT;
		XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).ivalue() >> get().ivalue()).cref()); downsize(1); return pc+1; }
		}}
	}
	return send2(pc, Xid(op_shr));
}

const u8* VMachineImpl::USHR(const u8* pc){ 
	switch(get(1).type()){XTAL_DEFAULT;
		XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ set(1, UncountedAny((int_t)((uint_t)get(1).ivalue() >> get().ivalue())).cref()); downsize(1); return pc+1; }
		}}
	}
	return send2(pc, Xid(op_ushr));
}

const u8* VMachineImpl::SHL(const u8* pc){ 
	switch(get(1).type()){XTAL_DEFAULT;
		XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).ivalue() << get().ivalue()).cref()); downsize(1); return pc+1; }
		}}
	}
	return send2(pc, Xid(op_shl));
}

const u8* VMachineImpl::EQ(const u8* pc){ 
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

const u8* VMachineImpl::NE(const u8* pc){ 
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

const u8* VMachineImpl::LT(const u8* pc){ 
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

const u8* VMachineImpl::GT(const u8* pc){ 
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

const u8* VMachineImpl::LE(const u8* pc){ 
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

const u8* VMachineImpl::GE(const u8* pc){ 
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

const u8* VMachineImpl::RAW_EQ(const u8* pc){ 
	set(1, UncountedAny(get(1).raweq(get())).cref());
	downsize(1);
	return pc + 1;
}

const u8* VMachineImpl::RAW_NE(const u8* pc){ 
	set(1, UncountedAny(get(1).rawne(get())).cref());
	downsize(1);
	return pc + 1;
}

const u8* VMachineImpl::IS(const u8* pc){ 
	set(1, UncountedAny(get(1).is(get())).cref());
	downsize(1);
	return pc + 1;
}

const u8* VMachineImpl::NIS(const u8* pc){ 
	set(1, UncountedAny(!get(1).is(get())).cref());
	downsize(1);
	return pc + 1;
}

const u8* VMachineImpl::EQ_IF(const u8* pc){
	switch(get(1).type()){XTAL_DEFAULT;
		XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ pc = get(1).ivalue() == get().ivalue() ? pc+6 : pc+get_s16(pc+1); downsize(2); return pc; }
			XTAL_CASE(TYPE_FLOAT){ pc = get(1).ivalue() == get().fvalue() ? pc+6 : pc+get_s16(pc+1); downsize(2); return pc; }
		}}
		XTAL_CASE(TYPE_FLOAT){switch(get().type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ pc = get(1).fvalue() == get().ivalue() ? pc+6 : pc+get_s16(pc+1); downsize(2); return pc; }
			XTAL_CASE(TYPE_FLOAT){ pc = get(1).fvalue() == get().fvalue() ? pc+6 : pc+get_s16(pc+1); downsize(2); return pc; }
		}}
	}
	return send2(pc, Xid(op_eq), 3);
}

const u8* VMachineImpl::NE_IF(const u8* pc){
	switch(get(1).type()){XTAL_DEFAULT;
		XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ pc = get(1).ivalue() != get().ivalue() ? pc+7 : pc+=get_s16(pc+1); downsize(2); return pc; }
			XTAL_CASE(TYPE_FLOAT){ pc = get(1).ivalue() != get().fvalue() ? pc+7 : pc+=get_s16(pc+1); downsize(2); return pc; }
		}}
		XTAL_CASE(TYPE_FLOAT){switch(get().type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ pc = get(1).fvalue() != get().ivalue() ? pc+7 : pc+=get_s16(pc+1); downsize(2); return pc; }
			XTAL_CASE(TYPE_FLOAT){ pc = get(1).fvalue() != get().fvalue() ? pc+7 : pc+=get_s16(pc+1); downsize(2); return pc; }
		}}
	}
	return send2(pc, Xid(op_eq), 3);
}

const u8* VMachineImpl::LT_IF(const u8* pc){ 
	switch(get(1).type()){XTAL_DEFAULT;
		XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ pc = get(1).ivalue() < get().ivalue() ? pc+6 : pc+=get_s16(pc+1); downsize(2); return pc; }
			XTAL_CASE(TYPE_FLOAT){ pc = get(1).ivalue() < get().fvalue() ? pc+6 : pc+=get_s16(pc+1); downsize(2); return pc; }
		}}
		XTAL_CASE(TYPE_FLOAT){switch(get().type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ pc = get(1).fvalue() < get().ivalue() ? pc+6 : pc+=get_s16(pc+1); downsize(2); return pc; }
			XTAL_CASE(TYPE_FLOAT){ pc = get(1).fvalue() < get().fvalue() ? pc+6 : pc+=get_s16(pc+1); downsize(2); return pc; }
		}}
	}
	return send2(pc, Xid(op_lt), 3);
}

const u8* VMachineImpl::GT_IF(const u8* pc){
	switch(get(1).type()){XTAL_DEFAULT;
		XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ pc = get(1).ivalue() > get().ivalue() ? pc+6 : pc+=get_s16(pc+1); downsize(2); return pc; }
			XTAL_CASE(TYPE_FLOAT){ pc = get(1).ivalue() > get().fvalue() ? pc+6 : pc+=get_s16(pc+1); downsize(2); return pc; }
		}}
		XTAL_CASE(TYPE_FLOAT){switch(get().type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ pc = get(1).fvalue() > get().ivalue() ? pc+6 : pc+=get_s16(pc+1); downsize(2); return pc; }
			XTAL_CASE(TYPE_FLOAT){ pc = get(1).fvalue() > get().fvalue() ? pc+6 : pc+=get_s16(pc+1); downsize(2); return pc; }
		}}
	}
	return send2r(pc, Xid(op_lt), 3);
}

const u8* VMachineImpl::LE_IF(const u8* pc){
	switch(get(1).type()){XTAL_DEFAULT;
		XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ pc = get(1).ivalue() <= get().ivalue() ? pc+7 : pc+=get_s16(pc+1); downsize(2); return pc; }
			XTAL_CASE(TYPE_FLOAT){ pc = get(1).ivalue() <= get().fvalue() ? pc+7 : pc+=get_s16(pc+1); downsize(2); return pc; }
		}}
		XTAL_CASE(TYPE_FLOAT){switch(get().type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ pc = get(1).fvalue() <= get().ivalue() ? pc+7 : pc+=get_s16(pc+1); downsize(2); return pc; }
			XTAL_CASE(TYPE_FLOAT){ pc = get(1).fvalue() <= get().fvalue() ? pc+7 : pc+=get_s16(pc+1); downsize(2); return pc; }
		}}
	}
	return send2r(pc, Xid(op_lt), 3);
}

const u8* VMachineImpl::GE_IF(const u8* pc){
	switch(get(1).type()){XTAL_DEFAULT;
		XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ pc = get(1).ivalue() >= get().ivalue() ? pc+7 : pc+=get_s16(pc+1); downsize(2); return pc; }
			XTAL_CASE(TYPE_FLOAT){ pc = get(1).ivalue() >= get().fvalue() ? pc+7 : pc+=get_s16(pc+1); downsize(2); return pc; }
		}}
		XTAL_CASE(TYPE_FLOAT){switch(get().type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ pc = get(1).fvalue() >= get().ivalue() ? pc+7 : pc+=get_s16(pc+1); downsize(2); return pc; }
			XTAL_CASE(TYPE_FLOAT){ pc = get(1).fvalue() >= get().fvalue() ? pc+7 : pc+=get_s16(pc+1); downsize(2); return pc; }
		}}
	}
	return send2(pc, Xid(op_lt), 3);
}

const u8* VMachineImpl::INC(const u8* pc){
	switch(get().type()){XTAL_DEFAULT;
		XTAL_CASE(TYPE_INT){ set(UncountedAny(get().ivalue()+1).cref()); return pc+1; }
		XTAL_CASE(TYPE_FLOAT){ set(UncountedAny(get().fvalue()+1).cref()); return pc+1; }
	}
	return send1(pc, Xid(op_inc));
}

const u8* VMachineImpl::DEC(const u8* pc){
	switch(get().type()){XTAL_DEFAULT;
		XTAL_CASE(TYPE_INT){ set(UncountedAny(get().ivalue()-1).cref()); return pc+1; }
		XTAL_CASE(TYPE_FLOAT){ set(UncountedAny(get().fvalue()-1).cref()); return pc+1; }
	}
	return send1(pc, Xid(op_dec));
}

const u8* VMachineImpl::LOCAL_NOT_ON_HEAP_INC(const u8* pc){
	UncountedAny& a = ff().variables_[get_u8(pc+1)];
	switch(a.type()){XTAL_DEFAULT;
		XTAL_CASE(TYPE_INT){ a = UncountedAny(a.ivalue()+1).cref(); return pc+4; }
		XTAL_CASE(TYPE_FLOAT){ a = UncountedAny(a.fvalue()+1).cref(); return pc+4; }
	}
	a.cref().send(Xid(op_inc), inner_setup_call(pc+2, 1)); 
	return ff().pc;
}

const u8* VMachineImpl::LOCAL_NOT_ON_HEAP_DEC(const u8* pc){
	UncountedAny& a = ff().variables_[get_u8(pc+1)];
	switch(a.type()){XTAL_DEFAULT;
		XTAL_CASE(TYPE_INT){ a = UncountedAny(a.ivalue()-1).cref(); return pc+4; }
		XTAL_CASE(TYPE_FLOAT){ a = UncountedAny(a.fvalue()-1).cref(); return pc+4; }
	}
	a.cref().send(Xid(op_dec), inner_setup_call(pc+2, 1)); 
	return ff().pc;
}

const u8* VMachineImpl::ADD_ASSIGN(const u8* pc){ 
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

const u8* VMachineImpl::SUB_ASSIGN(const u8* pc){ 
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

const u8* VMachineImpl::CAT_ASSIGN(const u8* pc){ 
	return send2(pc, Xid(op_cat_assign));
}

const u8* VMachineImpl::MUL_ASSIGN(const u8* pc){ 
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

const u8* VMachineImpl::DIV_ASSIGN(const u8* pc){ 
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

const u8* VMachineImpl::MOD_ASSIGN(const u8* pc){ 
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

const u8* VMachineImpl::AND_ASSIGN(const u8* pc){ 
	switch(get(1).type()){XTAL_DEFAULT;
		XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).ivalue() & get().ivalue()).cref()); downsize(1); return pc+1; }
		}}
	}
	return send2(pc, Xid(op_and_assign));
}

const u8* VMachineImpl::OR_ASSIGN(const u8* pc){ 
	switch(get(1).type()){XTAL_DEFAULT;
		XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).ivalue() | get().ivalue()).cref()); downsize(1); return pc+1; }
		}}
	}
	return send2(pc, Xid(op_or_assign));
}

const u8* VMachineImpl::XOR_ASSIGN(const u8* pc){ 
	switch(get(1).type()){XTAL_DEFAULT;
		XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).ivalue() ^ get().ivalue()).cref()); downsize(1); return pc+1; }
		}}
	}
	return send2(pc, Xid(op_xor_assign));
}

const u8* VMachineImpl::SHR_ASSIGN(const u8* pc){ 
	switch(get(1).type()){XTAL_DEFAULT;
		XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).ivalue() >> get().ivalue()).cref()); downsize(1); return pc+1; }
		}}
	}
	return send2(pc, Xid(op_shr_assign));
}

const u8* VMachineImpl::USHR_ASSIGN(const u8* pc){ 
	switch(get(1).type()){XTAL_DEFAULT;
		XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ set(1, UncountedAny((int_t)((uint_t)get(1).ivalue() >> get().ivalue())).cref()); downsize(1); return pc+1; }
		}}
	}
	return send2(pc, Xid(op_ushr_assign));
}

const u8* VMachineImpl::SHL_ASSIGN(const u8* pc){ 
	switch(get(1).type()){XTAL_DEFAULT;
		XTAL_CASE(TYPE_INT){switch(get().type()){XTAL_DEFAULT;
			XTAL_CASE(TYPE_INT){ set(1, UncountedAny(get(1).ivalue() << get().ivalue()).cref()); downsize(1); return pc+1; }
		}}
	}
	return send2(pc, Xid(op_shl_assign));
}

const u8* VMachineImpl::CURRENT_CONTINUATION(const u8* pc){
	XTAL_GLOBAL_INTERPRETER_LOCK{
		Any ret; new(ret) ContinuationImpl(clone(), pc);
		push(ret);
	}
	return pc + 1;
}

Any VMachineImpl::append_backtrace(const u8* pc, const Any& e){
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

void VMachineImpl::hook_return(const u8* pc){
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

void VMachineImpl::THROW(const u8* pc, int_t stack_size, int_t fun_frames_size){
	XTAL_GLOBAL_INTERPRETER_LOCK{
		Any e = pop();
		if(!e){
			e = String("");
		}
		if(!e.is(builtin().member("Exception"))){
			e = builtin().member("RuntimeError")(e);
		}
		//e = append_backtrace(pc, e);
		throw e;
	}
}

void VMachineImpl::THROW_UNSUPPROTED_ERROR(int_t stack_size, int_t fun_frames_size){
	XTAL_GLOBAL_INTERPRETER_LOCK{
		Any hint1a = ff().hint1();
		String hint1 = ff().hint1().object_name();
		String hint2 = ff().hint2();
		throw unsupported_error(hint1, hint2);
		//pop_ff();
		//const u8* pc = ff().pc;
		//THROW2(pc, unsupported_error(hint1, hint2), stack_size, fun_frames_size);
	}
}
	
void VMachineImpl::THROW2(const u8* pc, const Any& e, int_t stack_size, int_t fun_frames_size){
	Any ep = e;
	ep = append_backtrace(pc+1, ep);
	if(fun_frames_size<=(int_t)fun_frames_.size()){
		hook_return(pc);
		pop_ff();
	}
	while(fun_frames_size<=(int_t)fun_frames_.size()){
		ep = append_backtrace(ff().pc, ep);
		hook_return(pc);
		pop_ff();
	}
	resize(stack_size);
	throw ep;
}	

const u8* VMachineImpl::CHECK_ASSERT(const u8* pc, int_t stack_size, int_t fun_frames_size){
	XTAL_GLOBAL_INTERPRETER_LOCK{
		Any expr = get(2);
		Any expr_string = get(1) ? get(1) : Any("");
		Any message = get() ? get() : Any("");
		if(!expr){ 
			throw append_backtrace(pc, builtin().member("AssertionFailed")(message, expr_string));
		}
		downsize(3);
	}
	return pc+1;
}

const u8* VMachineImpl::BREAKPOINT(const u8* pc){
	if(debug::is_enabled()){
		XTAL_GLOBAL_INTERPRETER_LOCK{
			int_t kind = get_u8(pc+1);
			
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
	return pc+2;
}

const u8* VMachineImpl::CATCH_BODY(const u8* lpc, const Any& e, int_t stack_size, int_t fun_frames_size){
	XTAL_GLOBAL_INTERPRETER_LOCK{
		if(fun_frames_size>(int_t)fun_frames_.size()){
			throw e;
		}
		
		if(except_frames_.empty()){
			THROW2(lpc, e, stack_size, fun_frames_size);
		}

		ExceptFrame& ef = except_frames_.top();

		const u8* pc = &end_code_;
		Any ep = e;

		while(ef.fun_frame_count!=fun_frames_.size()){
			if(fun_frames_.size()==1)
				break;
			hook_return(pc);
			if(prev_ff().pc != &end_code_){
				pop_ff();
				pc = ff().pc - 1;	
				ep = append_backtrace(pc, ep);
			}else{
				pop_ff();
				resize(stack_size);
				throw e;
			}
		}

		while(ff().scopes.size()!=ef.scope_count){
			if(ff().scopes.top()){
				ff().variables_.downsize(ff().scopes.top()->variable_size);
			}else{
				ff().outer(ff().outer().outer());
			}
			ff().scopes.downsize(1);
		}

		resize(ef.stack_count);
		if(ef.catch_pc && e){
			pc = ef.catch_pc;
			push(Any(ef.end_pc-source()));
			push(e);
		}else{
			pc = ef.finally_pc;
			push(e);
			push(Any(code().size()-1));
		}
		except_frames_.downsize(1);
		return pc;
	}
	return 0;
}
	
}//namespace
