
#pragma once

#include "xtal.h"

#include "xtal_stack.h"
#include "xtal_codeimpl.h"
#include "xtal_funimpl.h"
#include "xtal_constant.h"
#include "xtal_fun.h"
#include "xtal_any.h"


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

	const Any& arg(int_t pos, const Fun& names){
		FunFrame& f = ff();
		if(pos<f.ordered_arg_count)
			return get((f.ordered_arg_count+f.named_arg_count*2)-1-pos);
		return arg(names.param_name_at(pos));
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
		adjust_result(n, ff().need_result_count);
	}

	void adjust_result(int_t n, int_t need_result_count);
	
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

	void carry_over(const Fun& fun);
	
	void mv_carry_over(const Fun& fun);

	bool processed(){ 
		return ff().calling_state!=FunFrame::CALLING_STATE_NONE; 
	}
	
	void replace_result(int_t pos, const Any& v){
		result(0);
		set(ff().need_result_count-pos-1, v);
	}
	
	void recycle_call();
	
	void recycle_call(const Any& a1);

	void execute_inner(const inst_t* start);
	void execute_try(const inst_t* start);

	void execute(const Fun& fun, const inst_t* start_pc = 0){
		setup_call(0);
		
		carry_over(fun);
		
		const inst_t* pc = prev_ff().pc;
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

	const inst_t* resume_pc(){ 
		return resume_pc_; 
	}

	void present_for_vm(const Fiber& fun, VMachineImpl* vm, bool add_succ_or_fail_result);

	const inst_t* start_fiber(const Fiber& fun, VMachineImpl* vm, bool add_succ_or_fail_result);

	const inst_t* resume_fiber(const Fiber& fun, const inst_t* pc, VMachineImpl* vm, bool add_succ_or_fail_result);

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

	const VMachine& inner_setup_call(const inst_t* pc, int_t need_result_count){
		push_ff(pc, need_result_count, 0, 0, null);
		return myself();
	}

	const VMachine& inner_setup_call(const inst_t* pc, int_t need_result_count, const Any& a1){
		push_ff(pc, need_result_count, 1, 0, null);
		push(a1);
		return myself();
	}

	const VMachine& inner_setup_call(const inst_t* pc, int_t need_result_count, const Any& a1, const Any& a2){
		push_ff(pc, need_result_count, 2, 0, null);
		push(a1); push(a2);
		return myself();
	}

	static void inc_ref_count(const UncountedAny& a){
		if(a.cref().type()==TYPE_BASE){
			a.cref().impl()->inc_ref_count();
		}
	}
	
	static void dec_ref_count(const UncountedAny& a){
		if(a.cref().type()==TYPE_BASE){
			a.cref().impl()->dec_ref_count();
		}
	}

	struct FunFrame{

		// 保存されたプログラムカウント
		const inst_t* pc;

		// スコープ情報 
		PStack<FrameCore*> scopes;

		// 関数が呼ばれたときの順番指定引数の数
		int_t ordered_arg_count;
		
		// 関数が呼ばれたときの名前指定引数の数
		int_t named_arg_count;

		// 関数呼び出し側が必要とする戻り値の数
		int_t need_result_count;

		int_t result_count;

		enum{
			CALLING_STATE_NONE, // 何もなってない状態
			CALLING_STATE_PUSHED_FUN, // 関数が積まれている状態
			CALLING_STATE_PUSHED_RESULT // 結果が積まれている状態
		};

		// 呼び出し状態
		int_t calling_state;

		// yieldが可能かフラグ。このフラグは呼び出しを跨いで伝播する。
		int_t yieldable;

		HaveInstanceVariables* instance_variables;

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
		ExceptCore* core;
		int_t scope_count;
		int_t stack_count;
		int_t fun_frame_count;
	};

	void push_ff(const inst_t* pc, int_t need_result_count, int_t ordered_count, int_t named_count, const Any& self);
	
	void push_ff_args(const inst_t* pc, int_t need_result_count, int_t ordered_count, int_t named_count, const Any& self);
	void recycle_ff(const inst_t* pc, int_t ordered_count, int_t named_count, const Any& self);
	void recycle_ff_args(const inst_t* pc, int_t ordered_count, int_t named_count, const Any& self);
	void pop_ff(){ fun_frames_.pop(); }

	void push_args(int_t named_arg_count);

	FunFrame& ff(){ return fun_frames_.top(); }
	FunFrame& prev_ff(){ return fun_frames_[1]; }

	const Fun& fun(){ return ff().fun(); }
	const Fun& prev_fun(){ return prev_ff().fun(); }

	const Frame& outer(){ return ff().outer(); }
	const Frame& prev_outer(){ return prev_ff().outer(); }

	const Code& code(){ return fun().impl()->code(); }
	const Code& prev_code(){ return prev_fun().impl()->code(); }

	const inst_t* source(){ return code().impl()->data(); }
	const inst_t* prev_source(){ return prev_code().impl()->data(); }

	const ID& symbol(int_t n){ return code().impl()->symbol(n); }
	const ID& prev_symbol(int_t n){ return prev_code().impl()->symbol(n); }

	void return_result_instance_variable(int_t number, FrameCore* core){
		return_result((ff().instance_variables->variable(number, core)));
	}
	
	Frame decolonize();
	
	Arguments make_args(const Fun& fun);

	Any append_backtrace(const inst_t* pc, const Any& ep);
	
	const VMachine& myself(){ 
		return *(const VMachine*)&myself_;
	}

private:

	const inst_t* VMachineImpl::send1(const inst_t* pc, const ID& name, int_t n = 1){
		XTAL_GLOBAL_INTERPRETER_LOCK{
			Any target = pop();
			UncountedAny self = ff().self();
			push_ff(pc + n, 1, 0, 0, self.cref());
			const Class& cls = target.get_class();
			set_hint(cls, name);
			if(const Any& ret = member_cache(cls, name, ff().self())){
				set_arg_this(target);
				ret.call(myself());
			}
		}
		return ff().pc;
	}

	const inst_t* VMachineImpl::send2(const inst_t* pc, const ID& name, int_t n = 1){
		XTAL_GLOBAL_INTERPRETER_LOCK{
			const Any& temp = pop();
			Any target = get();
			set(temp);
			UncountedAny self = ff().self();
			push_ff(pc + n, 1, 1, 0, self.cref());
			const Class& cls = target.get_class();
			set_hint(cls, name);
			if(const Any& ret = member_cache(cls, name, ff().self())){
				set_arg_this(target);
				ret.call(myself());
			}
		}
		return ff().pc;
	}

	const inst_t* VMachineImpl::send2r(const inst_t* pc, const ID& name, int_t n = 1){
		XTAL_GLOBAL_INTERPRETER_LOCK{
			Any target = pop();
			UncountedAny self = ff().self();
			push_ff(pc + n, 1, 1, 0, self.cref());
			const Class& cls = target.get_class();
			set_hint(cls, name);
			if(const Any& ret = member_cache(cls, name, ff().self())){
				set_arg_this(target);
				ret.call(myself());
			}
		}
		return ff().pc;
	}

	const inst_t* ARRAY_APPEND(const inst_t* pc);
	const inst_t* MAP_INSERT(const inst_t* pc);
	const inst_t* SET_NAME(const inst_t* pc);
	
	const inst_t* PUSH_ARGS(const inst_t* pc);

	void SET_LOCAL_VARIABLE(int_t pos, const Any&);
	const Any& LOCAL_VARIABLE(int_t pos);

	const inst_t* GLOBAL_VARIABLE(const inst_t* pc);
	const inst_t* SET_GLOBAL_VARIABLE(const inst_t* pc);
	const inst_t* DEFINE_GLOBAL_VARIABLE(const inst_t* pc);

	const inst_t* ONCE(const inst_t* pc);

	void SET_INSTANCE_VARIABLE(int_t number, int_t core_number, const Any&);
	const Any& INSTANCE_VARIABLE(int_t number, int_t core_number);

	const inst_t* MEMBER(const inst_t* pc);
	const inst_t* MEMBER_IF_DEFINED(const inst_t* pc);
	const inst_t* DEFINE_MEMBER(const inst_t* pc);

	const inst_t* AT(const inst_t* pc);
	const inst_t* SET_AT(const inst_t* pc);

	const inst_t* PUSH_ARRAY(const inst_t* pc);
	const inst_t* PUSH_MAP(const inst_t* pc);
	const inst_t* PUSH_FUN(const inst_t* pc);
	
	const inst_t* CLASS_BEGIN(const inst_t* pc);
	const inst_t* CLASS_END(const inst_t* pc);

	const inst_t* BLOCK_END(const inst_t* pc);

	const inst_t* TRY_BEGIN(const inst_t* pc);
	const inst_t* CATCH_BODY(const inst_t* pc, int_t stack_size, int_t fun_frames_size);
	void THROW(const inst_t* pc);
	void THROW_UNSUPPROTED_ERROR();
	const inst_t* CHECK_ASSERT(const inst_t* lpc);
	const inst_t* BREAKPOINT(const inst_t* pc);

	void YIELD(const inst_t* pc);

	const inst_t* POS(const inst_t* pc);
	const inst_t* NEG(const inst_t* pc);
	const inst_t* COM(const inst_t* pc);
	const inst_t* CLONE(const inst_t* pc);
	const inst_t* ADD(const inst_t* pc);
	const inst_t* SUB(const inst_t* pc);
	const inst_t* CAT(const inst_t* pc);
	const inst_t* MUL(const inst_t* pc);
	const inst_t* DIV(const inst_t* pc);
	const inst_t* MOD(const inst_t* pc);
	const inst_t* AND(const inst_t* pc);
	const inst_t* OR(const inst_t* pc);
	const inst_t* XOR(const inst_t* pc);
	const inst_t* SHR(const inst_t* pc);
	const inst_t* USHR(const inst_t* pc);
	const inst_t* SHL(const inst_t* pc);

	const inst_t* EQ(const inst_t* pc);
	const inst_t* NE(const inst_t* pc);
	const inst_t* LT(const inst_t* pc);
	const inst_t* GT(const inst_t* pc);
	const inst_t* LE(const inst_t* pc);
	const inst_t* GE(const inst_t* pc);
	const inst_t* RAW_EQ(const inst_t* pc);
	const inst_t* RAW_NE(const inst_t* pc);
	const inst_t* IS(const inst_t* pc);
	const inst_t* NIS(const inst_t* pc);

	const inst_t* IF_EQ(const inst_t* pc);
	const inst_t* IF_NE(const inst_t* pc);
	const inst_t* IF_LT(const inst_t* pc);
	const inst_t* IF_GT(const inst_t* pc);
	const inst_t* IF_LE(const inst_t* pc);
	const inst_t* IF_GE(const inst_t* pc);
	const inst_t* IF_RAW_EQ(const inst_t* pc);
	const inst_t* IF_RAW_NE(const inst_t* pc);
	const inst_t* IF_IS(const inst_t* pc);
	const inst_t* IF_NIS(const inst_t* pc);

	const inst_t* INC(const inst_t* pc);
	const inst_t* DEC(const inst_t* pc);
	const inst_t* LOCAL_VARIABLE_INC(const inst_t* pc);
	const inst_t* LOCAL_VARIABLE_DEC(const inst_t* pc);
	const inst_t* LocalVariableIncDirect(const inst_t* pc);
	const inst_t* LocalVariableDecDirect(const inst_t* pc);

	const inst_t* ADD_ASSIGN(const inst_t* pc);
	const inst_t* SUB_ASSIGN(const inst_t* pc);
	const inst_t* CAT_ASSIGN(const inst_t* pc);
	const inst_t* MUL_ASSIGN(const inst_t* pc);
	const inst_t* DIV_ASSIGN(const inst_t* pc);
	const inst_t* MOD_ASSIGN(const inst_t* pc);
	const inst_t* AND_ASSIGN(const inst_t* pc);
	const inst_t* OR_ASSIGN(const inst_t* pc);
	const inst_t* XOR_ASSIGN(const inst_t* pc);
	const inst_t* SHR_ASSIGN(const inst_t* pc);
	const inst_t* USHR_ASSIGN(const inst_t* pc);
	const inst_t* SHL_ASSIGN(const inst_t* pc);

	const inst_t* SET_ACCESSIBILITY(const inst_t* pc);
	const inst_t* CURRENT_CONTEXT(const inst_t* pc);

	void hook_return(const inst_t* pc);

	struct Add{
		template<class T, class U> static inline typename NumericCalcResultType<T, U>::type calc(T a, U b){ return (typename NumericCalcResultType<T, U>::type)(a + b); }
		static inline const ID& id(){ return Xid(op_add_assign); }
	};

	VMachine clone(){
		VMachine vm;
		*vm.impl() = *this;
		return vm;
	}

	struct MemberCacheTable{
		struct Unit{
			AnyImpl* klass;
			StringImpl* name;
			UncountedAny member;
			uint_t mutate_count;
		};

		enum{ CACHE_MAX = /*179*/ 256 };

		Unit table_[CACHE_MAX];
		uint_t hit_;
		uint_t miss_;

		MemberCacheTable(){
			for(int_t i=0; i<CACHE_MAX; ++i){
				table_[i].klass = 0;
			}
			hit_ = 0;
			miss_ = 0;
		}

		float cache_hit_rate(){
			return (float_t)hit_/(hit_+miss_);
		}

		uint_t hit(){
			return hit_;
		}

		uint_t miss(){
			return miss_;
		}

		const Any& cache(const Any& target_class, const ID& member_name, const Any& self){
			if(target_class.type()!=TYPE_BASE)
				return null;

			AnyImpl* klass = target_class.impl();
			StringImpl* name = member_name.impl();

			uint_t hash = (((uint_t)klass)>>3) + (((uint_t)name)>>2);
			Unit& unit = table_[hash/* % CACHE_MAX*/ & (CACHE_MAX-1)];
			if(global_mutate_count==unit.mutate_count && klass==unit.klass && name==unit.name){
				hit_++;
				return unit.member.cref();
			}else{
				miss_++;
				unit.member = klass->member(member_name, self);
				unit.klass = klass;
				unit.name = name;
				unit.mutate_count = global_mutate_count;
				return unit.member.cref();
			}
		}
	};
	
public:

	const Any& member_cache(const Any& target_class, const ID& member_name, const Any& self){
		return member_cache_table_.cache(target_class, member_name, self);
	}

private:

	inst_t end_code_;
	inst_t throw_unsupported_error_code_;
	inst_t check_unsupported_code_;
	inst_t cleanup_call_code_;
	inst_t throw_nop_code_;
	
	const inst_t* resume_pc_;
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

	MemberCacheTable member_cache_table_;

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
		printf("cache hit=%d, miss=%d, rate=%g\n", member_cache_table_.hit(), member_cache_table_.miss(), member_cache_table_.cache_hit_rate());
	}

};

}
