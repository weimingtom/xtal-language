
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


	VMachineImpl(){	
		myself_ = this;

		stack_.reserve(32);

		end_code_ = InstExit::NUMBER;
		throw_unsupported_error_code_ = InstThrowUnsupportedError::NUMBER;
		check_unsupported_code_ = InstCheckUnsupported::NUMBER;
		cleanup_call_code_ = InstCleanupCall::NUMBER;
		throw_nop_code_ = InstThrowNull::NUMBER;
		resume_pc_ = 0;
	}

private:

	VMachineImpl(const VMachineImpl&);
	VMachineImpl& operator=(const VMachineImpl&);

public:

	void setup_call(int_t need_result_count){
		push_ff(&end_code_, need_result_count, 0, 0, null);
	}

	void setup_call(int_t need_result_count, const Any& a1){
		push_ff(&end_code_, need_result_count, 1, 0, null);
		push(a1);
	}

	void setup_call(int_t need_result_count, const Any& a1, const Any& a2){
		push_ff(&end_code_, need_result_count, 2, 0, null);
		push(a1); push(a2);
	}

	void setup_call(int_t need_result_count, const Any& a1, const Any& a2, const Any& a3){
		push_ff(&end_code_, need_result_count, 3, 0, null);
		push(a1); push(a2); push(a3);
	}

	void setup_call(int_t need_result_count, const Any& a1, const Any& a2, const Any& a3, const Any& a4){
		push_ff(&end_code_, need_result_count, 4, 0, null);
		push(a1); push(a2); push(a3); push(a4);
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
		
	const Any& result(int_t pos = 0){
		const inst_t* temp;
		{
			FunFrame& f = ff();

			if(*f.called_pc==InstCleanupCall::NUMBER){
				if(pos<f.need_result_count){
					return get(f.need_result_count-pos-1);
				}else{
					return null;
				}
			}

			temp = f.poped_pc;
			f.poped_pc = &end_code_;
			execute_inner(f.called_pc);
		}

		fun_frames_.upsize(1);

		{
			FunFrame& f = ff();

			f.poped_pc = temp;
			f.called_pc = &cleanup_call_code_;

			if(pos<f.need_result_count){
				return get(f.need_result_count-pos-1);
			}else{
				return null;
			}
		}
	}
		
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
		ff().hint(hint1, hint2);
	}
	
public:

	const Any& arg(int_t pos){
		FunFrame& f = ff();
		if(pos<f.ordered_arg_count)
			return get(f.args_stack_size()-1-pos);
		return null;
	}

	const Any& arg(const ID& name){
		FunFrame& f = ff();
		for(int_t i = 0, sz = f.named_arg_count; i<sz; ++i){
			if(get(sz*2-1-(i*2+0)).raweq(name)){
				return get(sz*2-1-(i*2+1));
			}
		}
		return null;
	}

	const Any& arg(int_t pos, const ID& name){
		FunFrame& f = ff();
		if(pos<f.ordered_arg_count)
			return get(f.args_stack_size()-1-pos);
		return arg(name);
	}

	const Any& arg(int_t pos, FunImpl* names){
		FunFrame& f = ff();
		if(pos<f.ordered_arg_count)
			return get(f.args_stack_size()-1-pos);
		return arg(names->param_name_at(pos));
	}

	const Any& arg_default(int_t pos, const Any& def){
		FunFrame& f = ff();
		if(pos<f.ordered_arg_count)
			return get(f.args_stack_size()-1-pos);
		return def;
	}
		
	const Any& arg_default(const ID& name, const Any& def){
		FunFrame& f = ff();
		for(int_t i = 0, sz = f.named_arg_count; i<sz; ++i){
			if(get(sz*2-1-(i*2+0)).raweq(name)){
				return get(sz*2-1-(i*2+1));
			}
		}
		return def;
	}
		
	const Any& arg_default(int_t pos, const ID& name, const Any& def){
		FunFrame& f = ff();
		if(pos<f.ordered_arg_count)
			return get(f.args_stack_size()-1-pos);
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
		FunFrame& f = ff();

		downsize(f.args_stack_size());
		for(int_t i=0, sz=f.need_result_count; i<sz; ++i){
			push(null);
		}

		f.called_pc = &cleanup_call_code_;
	}

	void return_result(const Any& value1){
		FunFrame& f = ff();

		downsize(f.args_stack_size());
		push(value1);
		if(f.need_result_count!=1){
			adjust_result(1);
		}

		f.called_pc = &cleanup_call_code_;
	}
		
	void return_result(const Any& value1, const Any& value2){
		FunFrame& f = ff();

		downsize(f.args_stack_size());
		push(value1);
		push(value2);
		adjust_result(2);

		f.called_pc = &cleanup_call_code_;
	}

	void return_result(const Any& value1, const Any& value2, const Any& value3){
		FunFrame& f = ff();

		downsize(f.args_stack_size());
		push(value1);
		push(value2);
		push(value3);
		adjust_result(3);

		f.called_pc = &cleanup_call_code_;
	}
		
	void return_result(const Any& value1, const Any& value2, const Any& value3, const Any& value4){
		FunFrame& f = ff();

		downsize(f.args_stack_size());
		push(value1);
		push(value2);
		push(value3);
		push(value4);
		adjust_result(4);

		f.called_pc = &cleanup_call_code_;
	}

	void return_result(const Array& values){
		FunFrame& f = ff();

		downsize(f.args_stack_size());
		int_t size = values.size();
		for(int_t i=0; i<size; ++i){
			push(values.at(i));
		}
		adjust_result(size);

		f.called_pc = &cleanup_call_code_;
	}

	void carry_over(FunImpl* fun);
	void mv_carry_over(FunImpl* fun);

	bool processed(){ 
		return *ff().called_pc!=InstThrowUnsupportedError::NUMBER; 
	}
	
	void replace_result(int_t pos, const Any& v){
		result(0);
		set(ff().need_result_count-pos-1, v);
	}
	
	void recycle_call(){
		FunFrame& f = ff();
		downsize(f.ordered_arg_count+f.named_arg_count*2);
		f.ordered_arg_count = 0;
		f.named_arg_count = 0;
		f.called_pc = &throw_unsupported_error_code_;
	}

	void recycle_call(const Any& a1){
		recycle_call();
		push_arg(a1);
	}

	void execute_inner(const inst_t* start);
	
public:

	const inst_t* resume_pc(){ 
		return resume_pc_; 
	}

	void present_for_vm(FunImpl* fun, VMachineImpl* vm, bool add_succ_or_fail_result){
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

	const inst_t* start_fiber(FiberImpl* fun, VMachineImpl* vm, bool add_succ_or_fail_result){
		yield_result_count_ = 0;
		push_ff(&end_code_, vm->need_result_count(), vm->ordered_arg_count(), vm->named_arg_count(), vm->get_arg_this());
		move(vm, vm->ordered_arg_count()+vm->named_arg_count()*2);
		resume_pc_ = 0;
		carry_over(fun);
		ff().yieldable = true;
		execute_inner(ff().called_pc);
		present_for_vm(fun, vm, add_succ_or_fail_result);
		vm->ff().called_pc = &cleanup_call_code_;
		return resume_pc_;
	}

	const inst_t* resume_fiber(FiberImpl* fun, const inst_t* pc, VMachineImpl* vm, bool add_succ_or_fail_result){
		yield_result_count_ = 0;
		ff().called_pc = pc;
		resume_pc_ = 0;
		move(vm, vm->ordered_arg_count()+vm->named_arg_count()*2);
		execute_inner(ff().called_pc);
		present_for_vm(fun, vm, add_succ_or_fail_result);
		vm->ff().called_pc = &cleanup_call_code_;
		return resume_pc_;
	}

	void exit_fiber(){
		XTAL_TRY{
			yield_result_count_ = 0;
			ff().called_pc = resume_pc_;
			resume_pc_ = 0;
			execute_inner(&throw_nop_code_);
		}XTAL_CATCH(e){
			(void)e;
		}
		reset();
	}

	void reset(){
		stack_.resize(0);
		except_frames_.resize(0);
		fun_frames_.resize(0);
	}

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
		push(a1);
		push_ff(pc, need_result_count, 1, 0, null);
		return myself();
	}

	const VMachine& inner_setup_call(const inst_t* pc, int_t need_result_count, const Any& a1, const Any& a2){
		push(a1); push(a2);
		push_ff(pc, need_result_count, 2, 0, null);
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

		// pop_ffしたときはこのpcから実行する
		const inst_t* poped_pc;

		// callしたときはこのpcから実行する
		const inst_t* called_pc;

		// 関数が呼ばれたときの順番指定引数の数
		int_t ordered_arg_count;
		
		// 関数が呼ばれたときの名前指定引数の数
		int_t named_arg_count;

		// 関数呼び出し側が必要とする戻り値の数
		int_t need_result_count;

		// 関数が返した戻り値の数
		int_t result_count;

		// yieldが可能かフラグ。このフラグは呼び出しを跨いで伝播する。
		int_t yieldable;

		HaveInstanceVariables* instance_variables;

		// 呼び出された関数オブジェクト
		UncountedAny fun_; 

		// 一時的に関数オブジェクトかレシーバオブジェクトを置くための場所
		UncountedAny temp_;

		// 一時的に何かを置くための場所
		UncountedAny temp2_;

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

		CodeImpl* pcode;
		const inst_t* psource;

		void set_null(){
			fun_.set_null(); 
			temp_.set_null();
			temp2_.set_null();
			outer_.set_null();
			arguments_.set_null();
			hint1_.set_null();
			hint2_.set_null();
		}

		const Fun& fun() const{ return (const Fun&)fun_.cref(); }
		const Frame& outer() const{ return (const Frame&)outer_.cref(); }
		const Any& variable(int_t i) const{ return (const Any&)variables_[i].cref(); }
		const Any& self() const{ return (const Any&)self_.cref(); }
		const Arguments& arguments() const{ return (const Arguments&)arguments_.cref(); }
		const Any& hint1() const{ return (const Any&)hint1_.cref(); }
		const String& hint2() const{ return (const String&)hint2_.cref(); }

		int_t args_stack_size(){
			return ordered_arg_count+(named_arg_count<<1);
		}

		void fun(const UncountedAny& v){ 
			fun_ = v;
			pcode = fun().impl()->code().impl();
			psource = pcode->data();
		}

		void outer(const UncountedAny& v){ outer_ = v; }
		void variable(int_t i, const UncountedAny& v){ variables_[i] = v; }
		void self(const UncountedAny& v){ self_ = v; }
		void arguments(const UncountedAny& v){ arguments_ = v; }
		void hint(const UncountedAny& v1, const UncountedAny& v2){ hint1_ = v1; hint2_ = v2; }

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

			inc_ref_count(temp_);
			inc_ref_count(temp2_);
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

			dec_ref_count(temp_);
			dec_ref_count(temp2_);
		}
	};

	friend void visit_members(Visitor& m, const FunFrame& v){
		m & v.fun_.cref() & v.outer_.cref() & v.arguments_.cref() & v.hint1_.cref() & v.hint2_.cref() & v.self_.cref() & v.temp_.cref() & v.temp2_.cref();
		for(int_t i=0, size=v.variables_.size(); i<size; ++i){
			m & v.variable(i);
		}
	}

	// 例外を処理するためのフレーム
	struct ExceptFrame{
		ExceptCore* core;
		int_t stack_count;
		int_t fun_frame_count;
		int_t variable_size;
		UncountedAny outer;
	};

	void push_ff_args(const inst_t* pc, int_t need_result_count, int_t ordered_arg_count, int_t named_arg_count, const Any& self){
		push_args(named_arg_count);
		const Arguments& a = ff().arguments();
		push_ff(pc, need_result_count, a.impl()->ordered_.size()+ordered_arg_count, a.impl()->named_.size()+named_arg_count, self);
	}

	void recycle_ff(const inst_t* pc, int_t ordered_arg_count, int_t named_arg_count, const Any& self){
		FunFrame& f = ff();
		f.ordered_arg_count = ordered_arg_count;
		f.named_arg_count = named_arg_count;
		f.self(self);
		//f.poped_pc = pc;
		f.called_pc = &throw_unsupported_error_code_;
	}

	void recycle_ff_args(const inst_t* pc, int_t ordered_arg_count, int_t named_arg_count, const Any& self){
		push_args(named_arg_count);
		const Arguments& a = ff().arguments();
		recycle_ff(pc, a.impl()->ordered_.size()+ordered_arg_count, a.impl()->named_.size()+named_arg_count, self);
	}

	void push_ff(const inst_t* pc, int_t need_result_count, int_t ordered_arg_count, int_t named_arg_count, const Any& self){
		FunFrame& f = fun_frames_.push();
		f.need_result_count = need_result_count;
		f.result_count = 0;
		f.ordered_arg_count = ordered_arg_count;
		f.named_arg_count = named_arg_count;
		f.called_pc = &throw_unsupported_error_code_;
		f.poped_pc = pc;
		f.variables_.clear();
		f.instance_variables = &empty_have_instance_variables;
		f.self(self);
		f.set_null();
	}

	const inst_t* pop_ff(){ return fun_frames_.pop().poped_pc; }

	void push_args(int_t named_arg_count){
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

	FunFrame& ff(){ return fun_frames_.top(); }
	FunFrame& prev_ff(){ return fun_frames_[1]; }

	const Fun& fun(){ return ff().fun(); }
	const Fun& prev_fun(){ return prev_ff().fun(); }

	const Frame& outer(){ return ff().outer(); }
	const Frame& prev_outer(){ return prev_ff().outer(); }

	const Code& code(){ return fun().impl()->code(); }
	const Code& prev_code(){ return prev_fun().impl()->code(); }

	const ID& symbol(int_t n){ return ff().pcode->symbol(n); }
	const ID& prev_symbol(int_t n){ return prev_ff().pcode->symbol(n); }

	const ID& symbol_ex(int_t n){ 
		if(n!=0){
			return ff().pcode->symbol(n); 
		}else{
			return (const ID&)(ff().temp2_ = pop().to_s().intern()).cref();
		}
	}

	void return_result_instance_variable(int_t number, ClassCore* core){
		return_result((ff().instance_variables->variable(number, core)));
	}
		
	Arguments make_args(const Fun& fun);

	Any append_backtrace(const inst_t* pc, const Any& ep);
	
	const VMachine& myself(){ 
		return *(const VMachine*)&myself_;
	}

private:

	const inst_t* send1(const inst_t* pc, const ID& name){
		XTAL_GLOBAL_INTERPRETER_LOCK{
			UncountedAny target = ff().temp_ = pop();
			UncountedAny self = ff().self();
			push_ff(pc, 1, 0, 0, self.cref());
			target.cref().rawsend(myself(), name, ff().self(), null);
		}
		return ff().called_pc;
	}

	const inst_t* send2(const inst_t* pc, const ID& name){
		XTAL_GLOBAL_INTERPRETER_LOCK{
			const Any& temp = pop();
			UncountedAny target = ff().temp_ = get();
			set(temp);
			UncountedAny self = ff().self();
			push_ff(pc, 1, 1, 0, self.cref());
			target.cref().rawsend(myself(), name, ff().self(), null);
		}
		return ff().called_pc;
	}

	const inst_t* send2r(const inst_t* pc, const ID& name){
		XTAL_GLOBAL_INTERPRETER_LOCK{
			UncountedAny target = ff().temp_ = pop();
			UncountedAny self = ff().self();
			push_ff(pc, 1, 1, 0, self.cref());
			target.cref().rawsend(myself(), name, ff().self(), null);
		}
		return ff().called_pc;
	}

	void set_local_variable(int_t pos, const Any&);
	const Any& local_variable(int_t pos);

	const inst_t* catch_body(const inst_t* pc, int_t stack_size, int_t fun_frames_size);

	void hook_return(const inst_t* pc);

public:

//{DECLS{{
	const inst_t* FunNop(const inst_t* pc);
	const inst_t* FunPushNull(const inst_t* pc);
	const inst_t* FunPushNop(const inst_t* pc);
	const inst_t* FunPushTrue(const inst_t* pc);
	const inst_t* FunPushFalse(const inst_t* pc);
	const inst_t* FunPushInt1Byte(const inst_t* pc);
	const inst_t* FunPushInt2Byte(const inst_t* pc);
	const inst_t* FunPushFloat1Byte(const inst_t* pc);
	const inst_t* FunPushFloat2Byte(const inst_t* pc);
	const inst_t* FunPushCallee(const inst_t* pc);
	const inst_t* FunPushArgs(const inst_t* pc);
	const inst_t* FunPushThis(const inst_t* pc);
	const inst_t* FunPushCurrentContext(const inst_t* pc);
	const inst_t* FunPop(const inst_t* pc);
	const inst_t* FunDup(const inst_t* pc);
	const inst_t* FunInsert1(const inst_t* pc);
	const inst_t* FunInsert2(const inst_t* pc);
	const inst_t* FunInsert3(const inst_t* pc);
	const inst_t* FunAdjustResult(const inst_t* pc);
	const inst_t* FunIf(const inst_t* pc);
	const inst_t* FunUnless(const inst_t* pc);
	const inst_t* FunGoto(const inst_t* pc);
	const inst_t* FunLocalVariableInc(const inst_t* pc);
	const inst_t* FunLocalVariableDec(const inst_t* pc);
	const inst_t* FunLocalVariableIncDirect(const inst_t* pc);
	const inst_t* FunLocalVariableDecDirect(const inst_t* pc);
	const inst_t* FunLocalVariable1ByteDirect(const inst_t* pc);
	const inst_t* FunLocalVariable1Byte(const inst_t* pc);
	const inst_t* FunLocalVariable2Byte(const inst_t* pc);
	const inst_t* FunSetLocalVariable1ByteDirect(const inst_t* pc);
	const inst_t* FunSetLocalVariable1Byte(const inst_t* pc);
	const inst_t* FunSetLocalVariable2Byte(const inst_t* pc);
	const inst_t* FunInstanceVariable(const inst_t* pc);
	const inst_t* FunSetInstanceVariable(const inst_t* pc);
	const inst_t* FunCleanupCall(const inst_t* pc);
	const inst_t* FunReturn0(const inst_t* pc);
	const inst_t* FunReturn1(const inst_t* pc);
	const inst_t* FunReturn2(const inst_t* pc);
	const inst_t* FunReturn(const inst_t* pc);
	const inst_t* FunYield(const inst_t* pc);
	const inst_t* FunExit(const inst_t* pc);
	const inst_t* FunValue(const inst_t* pc);
	const inst_t* FunSetValue(const inst_t* pc);
	const inst_t* FunCheckUnsupported(const inst_t* pc);
	const inst_t* FunSend(const inst_t* pc);
	const inst_t* FunSendIfDefined(const inst_t* pc);
	const inst_t* FunCall(const inst_t* pc);
	const inst_t* FunCallCallee(const inst_t* pc);
	const inst_t* FunSend_A(const inst_t* pc);
	const inst_t* FunSendIfDefined_A(const inst_t* pc);
	const inst_t* FunCall_A(const inst_t* pc);
	const inst_t* FunCallCallee_A(const inst_t* pc);
	const inst_t* FunSend_T(const inst_t* pc);
	const inst_t* FunSendIfDefined_T(const inst_t* pc);
	const inst_t* FunCall_T(const inst_t* pc);
	const inst_t* FunCallCallee_T(const inst_t* pc);
	const inst_t* FunSend_AT(const inst_t* pc);
	const inst_t* FunSendIfDefined_AT(const inst_t* pc);
	const inst_t* FunCall_AT(const inst_t* pc);
	const inst_t* FunCallCallee_AT(const inst_t* pc);
	const inst_t* FunBlockBegin(const inst_t* pc);
	const inst_t* FunBlockEnd(const inst_t* pc);
	const inst_t* FunBlockBeginDirect(const inst_t* pc);
	const inst_t* FunBlockEndDirect(const inst_t* pc);
	const inst_t* FunTryBegin(const inst_t* pc);
	const inst_t* FunTryEnd(const inst_t* pc);
	const inst_t* FunPushGoto(const inst_t* pc);
	const inst_t* FunPopGoto(const inst_t* pc);
	const inst_t* FunIfEq(const inst_t* pc);
	const inst_t* FunIfNe(const inst_t* pc);
	const inst_t* FunIfLt(const inst_t* pc);
	const inst_t* FunIfLe(const inst_t* pc);
	const inst_t* FunIfGt(const inst_t* pc);
	const inst_t* FunIfGe(const inst_t* pc);
	const inst_t* FunIfRawEq(const inst_t* pc);
	const inst_t* FunIfRawNe(const inst_t* pc);
	const inst_t* FunIfIs(const inst_t* pc);
	const inst_t* FunIfNis(const inst_t* pc);
	const inst_t* FunIfArgIsNull(const inst_t* pc);
	const inst_t* FunIfArgIsNullDirect(const inst_t* pc);
	const inst_t* FunPos(const inst_t* pc);
	const inst_t* FunNeg(const inst_t* pc);
	const inst_t* FunCom(const inst_t* pc);
	const inst_t* FunNot(const inst_t* pc);
	const inst_t* FunAt(const inst_t* pc);
	const inst_t* FunSetAt(const inst_t* pc);
	const inst_t* FunAdd(const inst_t* pc);
	const inst_t* FunSub(const inst_t* pc);
	const inst_t* FunCat(const inst_t* pc);
	const inst_t* FunMul(const inst_t* pc);
	const inst_t* FunDiv(const inst_t* pc);
	const inst_t* FunMod(const inst_t* pc);
	const inst_t* FunAnd(const inst_t* pc);
	const inst_t* FunOr(const inst_t* pc);
	const inst_t* FunXor(const inst_t* pc);
	const inst_t* FunShl(const inst_t* pc);
	const inst_t* FunShr(const inst_t* pc);
	const inst_t* FunUshr(const inst_t* pc);
	const inst_t* FunEq(const inst_t* pc);
	const inst_t* FunNe(const inst_t* pc);
	const inst_t* FunLt(const inst_t* pc);
	const inst_t* FunLe(const inst_t* pc);
	const inst_t* FunGt(const inst_t* pc);
	const inst_t* FunGe(const inst_t* pc);
	const inst_t* FunRawEq(const inst_t* pc);
	const inst_t* FunRawNe(const inst_t* pc);
	const inst_t* FunIs(const inst_t* pc);
	const inst_t* FunNis(const inst_t* pc);
	const inst_t* FunInc(const inst_t* pc);
	const inst_t* FunDec(const inst_t* pc);
	const inst_t* FunAddAssign(const inst_t* pc);
	const inst_t* FunSubAssign(const inst_t* pc);
	const inst_t* FunCatAssign(const inst_t* pc);
	const inst_t* FunMulAssign(const inst_t* pc);
	const inst_t* FunDivAssign(const inst_t* pc);
	const inst_t* FunModAssign(const inst_t* pc);
	const inst_t* FunAndAssign(const inst_t* pc);
	const inst_t* FunOrAssign(const inst_t* pc);
	const inst_t* FunXorAssign(const inst_t* pc);
	const inst_t* FunShlAssign(const inst_t* pc);
	const inst_t* FunShrAssign(const inst_t* pc);
	const inst_t* FunUshrAssign(const inst_t* pc);
	const inst_t* FunGlobalVariable(const inst_t* pc);
	const inst_t* FunSetGlobalVariable(const inst_t* pc);
	const inst_t* FunDefineGlobalVariable(const inst_t* pc);
	const inst_t* FunMember(const inst_t* pc);
	const inst_t* FunMemberIfDefined(const inst_t* pc);
	const inst_t* FunDefineMember(const inst_t* pc);
	const inst_t* FunDefineClassMember(const inst_t* pc);
	const inst_t* FunSetName(const inst_t* pc);
	const inst_t* FunSetMultipleLocalVariable2Direct(const inst_t* pc);
	const inst_t* FunSetMultipleLocalVariable3Direct(const inst_t* pc);
	const inst_t* FunSetMultipleLocalVariable4Direct(const inst_t* pc);
	const inst_t* FunOnce(const inst_t* pc);
	const inst_t* FunClassBegin(const inst_t* pc);
	const inst_t* FunClassEnd(const inst_t* pc);
	const inst_t* FunMakeArray(const inst_t* pc);
	const inst_t* FunArrayAppend(const inst_t* pc);
	const inst_t* FunMakeMap(const inst_t* pc);
	const inst_t* FunMapInsert(const inst_t* pc);
	const inst_t* FunMakeFun(const inst_t* pc);
	const inst_t* FunMakeInstanceVariableAccessor(const inst_t* pc);
	const inst_t* FunThrow(const inst_t* pc);
	const inst_t* FunThrowUnsupportedError(const inst_t* pc);
	const inst_t* FunThrowNull(const inst_t* pc);
	const inst_t* FunAssert(const inst_t* pc);
	const inst_t* FunBreakPoint(const inst_t* pc);
	const inst_t* FunMAX(const inst_t* pc);
//}}DECLS}


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

protected:

	virtual void visit_members(Visitor& m){
		GCObserverImpl::visit_members(m);
		m & debug_info_ & last_except_.cref();
		
		for(int_t i=0, size=stack_.size(); i<size; ++i){
			m & stack_[i].cref();
		}

		for(int_t i=0, size=fun_frames_.size(); i<size; ++i){
			m & fun_frames_[i];
		}
	}

	virtual void before_gc(){
		//fun_frames_.fill_over();
		//stack_.fill_over();

		inc_ref_count(last_except_);

		for(int_t i=0, size=stack_.size(); i<size; ++i){
			inc_ref_count(stack_[i]);
		}

		for(int_t i=0, size=fun_frames_.size(); i<size; ++i){
			fun_frames_[i].inc_ref();
		}
	}

	virtual void after_gc(){
		dec_ref_count(last_except_);

		for(int_t i=0, size=stack_.size(); i<size; ++i){
			dec_ref_count(stack_[i]);
		}

		for(int_t i=0, size=fun_frames_.size(); i<size; ++i){
			fun_frames_[i].dec_ref();
		}
	}

public:

	void print_info(){
		printf("stack size %d\n", stack_.size());
		printf("fun_frames size %d\n", fun_frames_.size());
		printf("except_frames size %d\n", except_frames_.size());
	}

};

}
