
#pragma once

#include "xtal.h"

#include "xtal_stack.h"
#include "xtal_codeimpl.h"
#include "xtal_funimpl.h"
#include "xtal_constant.h"
#include "xtal_fun.h"
 

namespace xtal{

// 引数オブジェクト
class ArgumentsImpl : public AnyImpl{
public:

	ArgumentsImpl(){
		set_class(TClass<Arguments>::get());
	}

	const Any& op_at(const Any& index){
		if(index.type()==TYPE_INT){
			return ordered_.at(index.ivalue());
		}
		return named_.at(index); 
	}
	
	int_t length(){
		return ordered_.length();
	}
	
	Any each_ordered_arg(){
		return ordered_.each();
	}
	
	Any each_named_arg(){
		return named_.each_pair();
	}

public:

	Array ordered_;
	Map named_;

	virtual void visit_members(Visitor& m){
		AnyImpl::visit_members(m);
		m & ordered_ & named_;
	}

};

// XTAL仮想マシン
class VMachineImpl : public GCObserverImpl{
public:

	VMachineImpl();

	~VMachineImpl();


	VMachineImpl& operator=(const VMachineImpl& vm){
		if(this==&vm)
			return *this;
		//GCObserverImpl::operator=(vm);
		resume_pc_= vm.resume_pc_;
		yield_result_count_= vm.yield_result_count_;
		stack_= vm.stack_;
		fun_frames_= vm.fun_frames_;
		except_frames_= vm.except_frames_;	
		return *this;
	}

private:

	VMachineImpl(const VMachineImpl& vm);

public:

	void setup_call(int_t need_result_count);
	void setup_call(int_t need_result_count, const Any& a1);
	void setup_call(int_t need_result_count, const Any& a1, const Any& a2);
	void setup_call(int_t need_result_count, const Any& a1, const Any& a2, const Any& a3);
	void setup_call(int_t need_result_count, const Any& a1, const Any& a2, const Any& a3, const Any& a4);
	
	void set_call_flags(int_t flags){
		ff().result_flag = flags;
	}

	void push_arg(const Any& value){
		XTAL_ASSERT(named_arg_count() == 0);
		ff().ordered_arg_count++;
		push(value);
	}
		
	void push_arg(const ID& name, const Any& value){
		ff().named_arg_count++;
		push(name);
		push(value);
	}
		
	const Any& result(int_t pos = 0);
		
	Any result_and_cleanup_call(int_t pos){
		const Any& ret = result(pos);
		downsize(ff().need_result_count);
		pop_ff();
		return ret;
	}
		
	void cleanup_call(){
		result(0);
		downsize(ff().need_result_count);
		pop_ff();
	}
	
	void set_arg_this(const Any& self){ 
		ff().self(self);
	}

	void set_hint(const Any& hint1, const String& hint2){ 
		ff().hint1(hint1);
		ff().hint2(hint2);
	}
	
public:

	const Any& arg(int_t pos){
		if(pos<ordered_arg_count())
			return get((ordered_arg_count()+named_arg_count()*2)-1-pos);
		return null;
	}

	const Any& arg(const ID& name){
		for(int_t i = 0; i<named_arg_count(); ++i){
			if(get(named_arg_count()*2-1-(i*2+0)).raweq(name)){
				return get(named_arg_count()*2-1-(i*2+1));
			}
		}
		return null;
	}

	const Any& arg(int_t pos, const ID& name){
		if(pos<ordered_arg_count())
			return get((ordered_arg_count()+named_arg_count()*2)-1-pos);
		return arg(name);
	}

	const Any& arg_default(int_t pos, const Any& def){
		if(pos<ordered_arg_count())
			return get((ordered_arg_count()+named_arg_count()*2)-1-pos);
		return def;
	}
		
	const Any& arg_default(const ID& name, const Any& def){
		for(int_t i = 0; i<named_arg_count(); ++i){
			if(get(named_arg_count()*2-1-(i*2+0)).raweq(name)){
				return get(named_arg_count()*2-1-(i*2+1));
			}
		}
		return def;
	}
		
	const Any& arg_default(int_t pos, const ID& name, const Any& def){
		if(pos<ordered_arg_count())
			return get((ordered_arg_count()+named_arg_count()*2)-1-pos);
		return arg_default(name, def);
	}

	const ID& arg_name(int_t pos){
		return (const ID&)get(named_arg_count()*2-1-(pos*2+0));
	}

	const Any& get_arg_this(){ 
		return ff().self(); 
	}

	int_t ordered_arg_count(){ 
		return ff().ordered_arg_count; 
	}

	int_t named_arg_count(){ 
		return ff().named_arg_count; 
	}
		
	int_t need_result_count(){ 
		return ff().need_result_count; 
	}
	
	bool need_result(){ 
		return ff().need_result_count!=0; 
	}
	
	/*
	* Argumentsオブジェクトを生成する。
	* return_result()を呼んだ後は正常な値は得られない。
	*/
	Arguments make_arguments();

	void adjust_result(int_t n){		
		ff().result_count = n;
		adjust_result(n, ff().need_result_count, ff().result_flag);
	}

	void adjust_result(int_t n, int_t need_result_count, int_t result_flag);
	
	void return_result(){
		downsize(ordered_arg_count()+(named_arg_count()*2));
		FunFrame& f = ff();
		adjust_result(0);
		f.pc = &cleanup_call_code_;
		f.calling_state = FunFrame::CALLING_STATE_PUSHED_RESULT;
	}

	void return_result(const Any& value1){
		downsize(ordered_arg_count()+(named_arg_count()*2));
		FunFrame& f = ff();
		push(value1);
		adjust_result(1);
		f.pc = &cleanup_call_code_;
		f.calling_state = FunFrame::CALLING_STATE_PUSHED_RESULT;
	}
		
	void return_result(const Any& value1, const Any& value2){
		downsize(ordered_arg_count()+(named_arg_count()*2));
		FunFrame& f = ff();
		push(value1);
		push(value2);
		adjust_result(2);
		f.pc = &cleanup_call_code_;
		f.calling_state = FunFrame::CALLING_STATE_PUSHED_RESULT;
	}

	void return_result(const Any& value1, const Any& value2, const Any& value3){
		downsize(ordered_arg_count()+(named_arg_count()*2));
		FunFrame& f = ff();
		push(value1);
		push(value2);
		push(value3);
		adjust_result(3);
		f.pc = &cleanup_call_code_;
		f.calling_state = FunFrame::CALLING_STATE_PUSHED_RESULT;
	}
		
	void return_result(const Any& value1, const Any& value2, const Any& value3, const Any& value4){
		downsize(ordered_arg_count()+(named_arg_count()*2));
		FunFrame& f = ff();
		push(value1);
		push(value2);
		push(value3);
		push(value4);
		adjust_result(4);
		f.pc = &cleanup_call_code_;
		f.calling_state = FunFrame::CALLING_STATE_PUSHED_RESULT;
	}

	void return_result(const Array& values){
		downsize(ordered_arg_count()+(named_arg_count()*2));
		FunFrame& f = ff();
		int_t size = values.size();
		for(int_t i=0; i<size; ++i){
			push(values.at(i));
		}
		adjust_result(size);
		f.pc = &cleanup_call_code_;
		f.calling_state = FunFrame::CALLING_STATE_PUSHED_RESULT;
	}

	void carry_over(FunImpl* p);
	
	void mv_carry_over(FunImpl* fun);

	bool processed(){ 
		return ff().calling_state!=FunFrame::CALLING_STATE_NONE; 
	}
	
	void replace_result(int_t pos, const Any& v){
		result(0);
		set(ff().need_result_count-pos-1, v);
	}
	
	void recycle_call();
	
	void recycle_call(const Any& a1);

	void execute_inner(const u8* start);
	void execute_try(const u8* start);

	void execute(const Fun& fun, const u8* start_pc = 0){
		setup_call(0);
		
		carry_over(fun.impl());
		
		const u8* pc = prev_ff().pc;
		prev_ff().pc = &end_code_;
		execute_try(start_pc ? start_pc : ff().pc);
		fun_frames_.upsize(1);
		ff().pc = &cleanup_call_code_;
		prev_ff().pc = pc;
		ff().calling_state = FunFrame::CALLING_STATE_PUSHED_RESULT;

		downsize(ff().need_result_count);
		pop_ff();
	}
	
public:

	const u8* resume_pc(){ 
		return resume_pc_; 
	}

	void present_for_vm(const Fiber& fun, VMachineImpl* vm, bool add_succ_or_fail_result);

	const u8* start_fiber(const Fiber& fun, VMachineImpl* vm, bool add_succ_or_fail_result);

	const u8* resume_fiber(const Fiber& fun, const u8* pc, VMachineImpl* vm, bool add_succ_or_fail_result);

	void exit_fiber();

	void reset();
		
public:

	const Code& current_code(){ return code(); }

	const Fun& current_fun(){ return fun(); }

public:

	// スタックのi番目の値を取得する。
	const Any& get(int_t i){ return stack_[i].cref(); }

	// スタックの0番目の値を取得する。
	const Any& get(){ return stack_.top().cref(); }

	// スタックのi番目の値を設定する。
	void set(int_t i, const Any& v){ stack_[i]=v; }

	// スタックの0番目の値を設定する。
	void set(const Any& v){ stack_.top()=v; }

	// スタックをn拡大する。
	void upsize(int_t n){ stack_.upsize_unchecked(n); }

	// スタックをn縮小する
	void downsize(int_t n){ stack_.downsize(n); }

	// スタックをn個にする。
	void resize(int_t n){ stack_.resize(n); }

	// スタックに値vをプッシュする。
	void push(const Any& v){ stack_.push_unchecked(v); }

	// スタックに値vをプッシュする。
	void push_unchecked(const Any& v){ stack_.push_unchecked(v); }

	// スタックから値をポップする。
	const Any& pop(){ return stack_.pop().cref(); }

	// 先頭の値をプッシュする。
	void dup(){ push(get()); }

	// i番目の値をプッシュする。
	void dup(int_t i){ push(get(i)); }

	// スタックの大きさを返す。
	int_t stack_size(){ return (int_t)stack_.size(); }
	
	// srcのスタックの内容をsize個プッシュする。
	void push(VMachineImpl* src, int_t size){ stack_.push(src->stack_, size); }
	
	// srcのスタックの内容をsize個プッシュする。
	void push(VMachineImpl* src, int_t src_offset, int_t size){ stack_.push(src->stack_, src_offset, size); }

	// srcのスタックの内容をsize個取り除いて、プッシュする。
	void move(VMachineImpl* src, int_t size){ stack_.move(src->stack_, size); }
	
public:

	const VMachine& inner_setup_call(const u8* pc, int_t need_result_count){
		push_ff(pc, need_result_count, 0, 0, 0, null);
		return myself();
	}

	const VMachine& inner_setup_call(const u8* pc, int_t need_result_count, const Any& a1){
		push_ff(pc, need_result_count, 0, 1, 0, null);
		push(a1);
		return myself();
	}

	const VMachine& inner_setup_call(const u8* pc, int_t need_result_count, const Any& a1, const Any& a2){
		push_ff(pc, need_result_count, 0, 2, 0, null);
		push(a1); push(a2);
		return myself();
	}

	static void inc_ref_count(const UncountedAny& a){
		if(a.cref().type()==TYPE_BASE && a.impl()){
			a.cref().impl()->inc_ref_count();
		}
	}
	
	static void dec_ref_count(const UncountedAny& a){
		if(a.cref().type()==TYPE_BASE && a.impl()){
			a.cref().impl()->dec_ref_count();
		}
	}

	struct FunFrame{

		// 保存されたプログラムカウント
		const u8* pc;

		// スコープ情報 
		PStack<FrameCore*> scopes;

		// 関数が呼ばれたときの順番指定引数の数
		int_t ordered_arg_count;
		
		// 関数が呼ばれたときの名前指定引数の数
		int_t named_arg_count;

		// 関数呼び出し側が必要とする戻り値の数
		int_t need_result_count;

		int_t result_count;

		int_t result_flag;

		enum{
			CALLING_STATE_NONE, // 何もなってない状態
			CALLING_STATE_PUSHED_FUN, // 関数が積まれている状態
			CALLING_STATE_PUSHED_RESULT // 結果が積まれている状態
		};

		// 呼び出し状態
		int_t calling_state;

		// yieldが可能かフラグ。このフラグは呼び出しを跨いで伝播する。
		int_t yieldable;

		// 呼び出された関数オブジェクト
		UncountedAny fun_; 

		// スコープの外側のフレームオブジェクト
		UncountedAny outer_;

		// スコープがオブジェクト化されてない時のローカル変数領域
		Stack<UncountedAny> variables_;

		// 関数が呼ばれたときのthisオブジェクト
		UncountedAny self_;

		// オブジェクト化した引数。
		UncountedAny arguments_;
		
		// デバッグメッセージ出力用のヒント
		UncountedAny hint1_;
		
		// デバッグメッセージ出力用のヒント
		UncountedAny hint2_;

		const Fun& fun() const{ return (const Fun&)fun_.cref(); }
		const Frame& outer() const{ return (const Frame&)outer_.cref(); }
		const Any& variable(int_t i) const{ return (const Any&)variables_[i].cref(); }
		const Any& self() const{ return (const Any&)self_.cref(); }
		const Arguments& arguments() const{ return (const Arguments&)arguments_.cref(); }
		const Any& hint1() const{ return (const Any&)hint1_.cref(); }
		const String& hint2() const{ return (const String&)hint2_.cref(); }

		void fun(const UncountedAny& v){ fun_ = v; }
		void outer(const UncountedAny& v){ outer_ = v; }
		void variable(int_t i, const UncountedAny& v){ variables_[i] = v; }
		void self(const UncountedAny& v){ self_ = v; }
		void arguments(const UncountedAny& v){ arguments_ = v; }
		void hint1(const UncountedAny& v){ hint1_ = v; }
		void hint2(const UncountedAny& v){ hint2_ = v; }

		void inc_ref(){
			inc_ref_count(fun_);
			inc_ref_count(outer_);
			
			for(int_t i=0, size=variables_.size(); i<size; ++i){
				inc_ref_count(variables_[i]);
			}
			
			inc_ref_count(self_);
			inc_ref_count(arguments_);
			inc_ref_count(hint1_);
			inc_ref_count(hint2_);
		}
		
		void dec_ref(){
			dec_ref_count(fun_);
			dec_ref_count(outer_);
			
			for(int_t i=0, size=variables_.size(); i<size; ++i){
				dec_ref_count(variables_[i]);
			}
			
			dec_ref_count(self_);
			dec_ref_count(arguments_);
			dec_ref_count(hint1_);
			dec_ref_count(hint2_);
		}
	};

	friend void visit_members(Visitor& m, const FunFrame& v){
		m & v.fun() & v.outer() & v.arguments() & v.hint1() & v.hint2() & v.self();
		for(int_t i=0, size=v.variables_.size(); i<size; ++i){
			m & v.variable(i);
		}
	}

	// 例外を処理するためのフレーム
	struct ExceptFrame{
		ExceptFrame(){}
		ExceptFrame(const u8* catch_pc, const u8* finally_pc, const u8* end_pc)
			:catch_pc(catch_pc), finally_pc(finally_pc), end_pc(end_pc){}
		const u8* catch_pc;
		const u8* finally_pc;
		const u8* end_pc;
		int_t scope_count;
		int_t stack_count;
		int_t fun_frame_count;
	};

	void push_ff(const u8* pc, int_t need_result_count, int_t result_flag, int_t ordered_count, int_t named_count, const Any& self);
	
	void push_ff_args(const u8* pc, int_t need_result_count, int_t result_flag, int_t ordered_count, int_t named_count, const Any& self);
	void recycle_ff(const u8* pc, int_t ordered_count, int_t named_count, const Any& self);
	void recycle_ff_args(const u8* pc, int_t ordered_count, int_t named_count, const Any& self);
	void pop_ff(){ fun_frames_.pop(); }

	void push_args(int_t named_arg_count);

	FunFrame& ff(){ return fun_frames_.top(); }
	FunFrame& prev_ff(){ return fun_frames_[1]; }

	const Fun& fun(){ return ff().fun(); }
	const Fun& prev_fun(){ return prev_ff().fun(); }

//	const u8* fun_pc(){ return fun().impl()->pc(); }
//	const u8* prev_fun_pc(){ return prev_fun().impl()->pc(); }

	const Frame& outer(){ return ff().outer(); }
	const Frame& prev_outer(){ return prev_ff().outer(); }

	const Code& code(){ return fun().impl()->code(); }
	const Code& prev_code(){ return prev_fun().impl()->code(); }

	const u8* source(){ return code().impl()->data(); }
	const u8* prev_source(){ return prev_code().impl()->data(); }

	const ID& symbol(int_t n){ return code().impl()->get_symbol(n); }
	const ID& prev_symbol(int_t n){ return prev_code().impl()->get_symbol(n); }
	
	Frame decolonize();
	
	Arguments make_args(const Fun& fun);

	Any append_backtrace(const u8* pc, const Any& ep);
	
	const VMachine& myself(){ 
		return *(const VMachine*)&myself_;
	}

private:

	const u8* send1(const u8* pc, const ID& id, int_t n = 1);
	const u8* send2(const u8* pc, const ID& id, int_t n = 1);
	const u8* send2r(const u8* pc, const ID& id, int_t n = 1);

	const u8* ARRAY_APPEND(const u8* pc);
	const u8* MAP_APPEND(const u8* pc);
	const u8* SET_NAME(const u8* pc);
	
	const u8* PUSH_ARGS(const u8* pc);

	void SET_LOCAL_VARIABLE(int_t pos, const Any&);
	const Any& LOCAL_VARIABLE(int_t pos);

	const u8* GLOBAL_VARIABLE(const u8* pc);
	const u8* SET_GLOBAL_VARIABLE(const u8* pc);
	const u8* DEFINE_GLOBAL_VARIABLE(const u8* pc);

	const u8* ONCE(const u8* pc);

	const u8* SET_INSTANCE_VARIABLE(const u8* pc);
	const u8* INSTANCE_VARIABLE(const u8* pc);

	const u8* CALL(const u8* pc);
	const u8* CALLEE(const u8* pc);
	const u8* SEND(const u8* pc);
	const u8* SEND_IF_DEFINED(const u8* pc);

	const u8* MEMBER(const u8* pc);
	const u8* MEMBER_IF_DEFINED(const u8* pc);
	const u8* DEFINE_MEMBER(const u8* pc);

	const u8* AT(const u8* pc);
	const u8* SET_AT(const u8* pc);

	const u8* PUSH_ARRAY(const u8* pc);
	const u8* PUSH_MAP(const u8* pc);
	const u8* PUSH_FUN(const u8* pc);
	
	const u8* CLASS_BEGIN(const u8* pc);
	const u8* CLASS_END(const u8* pc);

	const u8* BLOCK_END(const u8* pc);

	const u8* TRY_BEGIN(const u8* pc);
	const u8* CATCH_BODY(const u8* pc, int_t stack_size, int_t fun_frames_size);
	void THROW(const u8* pc);
	void THROW_UNSUPPROTED_ERROR();
	const u8* CHECK_ASSERT(const u8* lpc);
	const u8* BREAKPOINT(const u8* pc);

	void YIELD(const u8* pc);

	const u8* POS(const u8* pc);
	const u8* NEG(const u8* pc);
	const u8* COM(const u8* pc);
	const u8* CLONE(const u8* pc);
	const u8* ADD(const u8* pc);
	const u8* SUB(const u8* pc);
	const u8* CAT(const u8* pc);
	const u8* MUL(const u8* pc);
	const u8* DIV(const u8* pc);
	const u8* MOD(const u8* pc);
	const u8* AND(const u8* pc);
	const u8* OR(const u8* pc);
	const u8* XOR(const u8* pc);
	const u8* SHR(const u8* pc);
	const u8* USHR(const u8* pc);
	const u8* SHL(const u8* pc);

	const u8* EQ(const u8* pc);
	const u8* NE(const u8* pc);
	const u8* LT(const u8* pc);
	const u8* GT(const u8* pc);
	const u8* LE(const u8* pc);
	const u8* GE(const u8* pc);
	const u8* RAW_EQ(const u8* pc);
	const u8* RAW_NE(const u8* pc);
	const u8* IS(const u8* pc);
	const u8* NIS(const u8* pc);

	const u8* EQ_IF(const u8* pc);
	const u8* NE_IF(const u8* pc);
	const u8* LT_IF(const u8* pc);
	const u8* GT_IF(const u8* pc);
	const u8* LE_IF(const u8* pc);
	const u8* GE_IF(const u8* pc);

	const u8* INC(const u8* pc);
	const u8* DEC(const u8* pc);
	const u8* LOCAL_NOT_ON_HEAP_INC(const u8* pc);
	const u8* LOCAL_NOT_ON_HEAP_DEC(const u8* pc);

	const u8* ADD_ASSIGN(const u8* pc);
	const u8* SUB_ASSIGN(const u8* pc);
	const u8* CAT_ASSIGN(const u8* pc);
	const u8* MUL_ASSIGN(const u8* pc);
	const u8* DIV_ASSIGN(const u8* pc);
	const u8* MOD_ASSIGN(const u8* pc);
	const u8* AND_ASSIGN(const u8* pc);
	const u8* OR_ASSIGN(const u8* pc);
	const u8* XOR_ASSIGN(const u8* pc);
	const u8* SHR_ASSIGN(const u8* pc);
	const u8* USHR_ASSIGN(const u8* pc);
	const u8* SHL_ASSIGN(const u8* pc);

	const u8* SET_ACCESSIBILITY(const u8* pc);
	const u8* CURRENT_CONTEXT(const u8* pc);

	void hook_return(const u8* pc);

	VMachine clone(){
		VMachine vm;
		*vm.impl() = *this;
		return vm;
	}

private:

	u8 end_code_;
	u8 throw_unsupported_error_code_;
	u8 check_unsupported_code_;
	u8 cleanup_call_code_;
	u8 throw_nop_code_;
	
	const u8* resume_pc_;
	int_t yield_result_count_;

	UncountedAny myself_;

	// 計算用スタック
	Stack<UncountedAny> stack_;

	// 関数呼び出しの度に積まれるフレーム
	Stack<FunFrame> fun_frames_;

	// tryの度に積まれるフレーム。
	PODStack<ExceptFrame> except_frames_;
	
	debug::Info debug_info_;

	UncountedAny last_except_;

protected:

	virtual void visit_members(Visitor& m){
		GCObserverImpl::visit_members(m);
		m & debug_info_ & last_except_.cref();
		
		for(int_t i=0, size=stack_.size(); i<size; ++i){
			m & stack_[i].cref();
		}

		for(int_t i=0, size=fun_frames_.capacity(); i<size; ++i){
			m & fun_frames_.reverse_at_unchecked(i);
		}
	}

	virtual void before_gc(){
		//fun_frames_.fill_over();
		//stack_.fill_over();

		inc_ref_count(last_except_);

		for(int_t i=0, size=stack_.size(); i<size; ++i){
			inc_ref_count(stack_[i]);
		}

		for(int_t i=0, size=fun_frames_.capacity(); i<size; ++i){
			fun_frames_.reverse_at_unchecked(i).inc_ref();
		}
	}

	virtual void after_gc(){
		dec_ref_count(last_except_);

		for(int_t i=0, size=stack_.size(); i<size; ++i){
			dec_ref_count(stack_[i]);
		}

		for(int_t i=0, size=fun_frames_.capacity(); i<size; ++i){
			fun_frames_.reverse_at_unchecked(i).dec_ref();
		}
	}

public:

	void print_info(){
		printf("stack size %d\n", stack_.size());
		printf("fun_frames size %d\n", fun_frames_.size()-1);
		printf("except_frames size %d\n", except_frames_.size());
	}

};

}
