#ifndef XTAL_VMACHINE_H_INCLUDE_GUARD
#define XTAL_VMACHINE_H_INCLUDE_GUARD

#pragma once

namespace xtal{

// XTAL仮想マシン
class VMachine : public GCObserver{
public:

	VMachine();

	~VMachine();

public:

	// 関数呼び出し側が使うための関数群

	/**
	* @brief 引数を1個積む。
	*
	*/
	void push_arg(const AnyPtr& value);
	
	/**
	* @brief 名前付き引数を1個積む。
	*
	*/
	void push_arg(const IDPtr& name, const AnyPtr& value);
	
	/**
	* @brief 名前付き引数を1個積む。
	*
	*/
	void push_arg(const Named& p){ push_arg(p.name, p.value); }

	/**
	* @brief 引数を配列の要素数積む。
	*
	*/
	void push_ordered_args(const ArrayPtr& p);

	/**
	* @brief 名前つき引数を連想配列の要素数積む。
	*
	*/
	void push_named_args(const MapPtr& p);

	/**
	* @brief pos番目の戻り値を得る。
	*
	*/
	const AnyPtr& result(int_t pos = 0);

	/**
	* @brief 呼び出しの後始末をする。
	*
	*/
	void cleanup_call();	

	/**
	* @brief pos番目の戻り値を返し、呼び出しの後始末をする。
	*
	*/
	AnyPtr result_and_cleanup_call(int_t pos = 0);
		
	/**
	* @brief thisを差し替える。
	*
	*/	
	void set_arg_this(const AnyPtr& self){ 
		ff().self(self);
	}

	/**
	* @brief ヒントの設定
	*
	* 例外が起きたときのエラーメッセージのために、
	* 現在呼び出している関数を登録する。
	*/
	void set_hint(const AnyPtr& object){ 
		ff().hint(object);
	}
// 

	/// @brief 関数を呼び出す用意をし、同時に引数を`i`個積む
	void setup_call(int_t need_result_count = 1){
		push_ff(need_result_count);	
	}

//{REPEAT{{
/*
	/// @brief 関数を呼び出す用意をし、同時に引数を`n+1`個積む
	template<class A0 #COMMA_REPEAT#class A`i+1`#>
	void setup_call(int_t need_result_count, const A0& a0 #COMMA_REPEAT#const A`i+1`& a`i+1`#){
		push_ff(need_result_count);
		push_arg(a0);
		#REPEAT#push_arg(a`i+1`);# 
	}
*/

	/// @brief 関数を呼び出す用意をし、同時に引数を1個積む
	template<class A0 >
	void setup_call(int_t need_result_count, const A0& a0 ){
		push_ff(need_result_count);
		push_arg(a0);
		 
	}

	/// @brief 関数を呼び出す用意をし、同時に引数を2個積む
	template<class A0 , class A1>
	void setup_call(int_t need_result_count, const A0& a0 , const A1& a1){
		push_ff(need_result_count);
		push_arg(a0);
		push_arg(a1); 
	}

	/// @brief 関数を呼び出す用意をし、同時に引数を3個積む
	template<class A0 , class A1, class A2>
	void setup_call(int_t need_result_count, const A0& a0 , const A1& a1, const A2& a2){
		push_ff(need_result_count);
		push_arg(a0);
		push_arg(a1);push_arg(a2); 
	}

	/// @brief 関数を呼び出す用意をし、同時に引数を4個積む
	template<class A0 , class A1, class A2, class A3>
	void setup_call(int_t need_result_count, const A0& a0 , const A1& a1, const A2& a2, const A3& a3){
		push_ff(need_result_count);
		push_arg(a0);
		push_arg(a1);push_arg(a2);push_arg(a3); 
	}

	/// @brief 関数を呼び出す用意をし、同時に引数を5個積む
	template<class A0 , class A1, class A2, class A3, class A4>
	void setup_call(int_t need_result_count, const A0& a0 , const A1& a1, const A2& a2, const A3& a3, const A4& a4){
		push_ff(need_result_count);
		push_arg(a0);
		push_arg(a1);push_arg(a2);push_arg(a3);push_arg(a4); 
	}

	/// @brief 関数を呼び出す用意をし、同時に引数を6個積む
	template<class A0 , class A1, class A2, class A3, class A4, class A5>
	void setup_call(int_t need_result_count, const A0& a0 , const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5){
		push_ff(need_result_count);
		push_arg(a0);
		push_arg(a1);push_arg(a2);push_arg(a3);push_arg(a4);push_arg(a5); 
	}

//}}REPEAT}

public:

	// 関数呼び出され側が使うための関数群

	/**
	* @brief pos番目の引数を得る。
	*
	*/
	const AnyPtr& arg(int_t pos);

	/**
	* @brief nameに対応する引数を得る。
	*
	*/
	const AnyPtr& arg(const IDPtr& name);

	/**
	* @brief pos番目の引数を得る。もしpos番目の引数がなければnameに対応する引数を得る。
	*
	*/
	const AnyPtr& arg(int_t pos, const IDPtr& name);
	
	const AnyPtr& arg(int_t pos, Method* names);

	/**
	* @brief pos番目の引数を得る。
	*
	* もしpos番目の引数が無ければdefの値を返す。
	*/
	const AnyPtr& arg_default(int_t pos, const AnyPtr& def);

	/**
	* @brief nameに対応する引数を得る。
	*
	* もしnameに対応する引数が無ければdefの値を返す。
	*/
	const AnyPtr& arg_default(const IDPtr& name, const AnyPtr& def);

	/**
	* @brief pos番目の引数を得る。もしpos番目の引数がなければnameに対応する引数を得る。
	*
	* もしnameに対応する引数が無ければdefの値を返す。
	*/
	const AnyPtr& arg_default(int_t pos, const IDPtr& name, const AnyPtr& def);
	
	/**
	* @brief nameに対応する引数を得る。
	*
	* もしnameに対応する引数が無ければdefの値を返す。
	*/
	const AnyPtr& arg_default(const Named& name_and_def);

	/**
	* @brief pos番目の引数を得る。もしpos番目の引数がなければnameに対応する引数を得る。
	*
	* もしnameに対応する引数が無ければdefの値を返す。
	*/
	const AnyPtr& arg_default(int_t pos, const Named& name_and_def);

	/**
	* @brief pos番目の名前指定引数の名前を取得。
	*
	* @param pos 0からnamed_arg_count()-1まで
	*/
	const IDPtr& arg_name(int_t pos){
		return unchecked_ptr_cast<ID>(get(named_arg_count()*2-1-(pos*2+0)));
	}

	void adjust_args(const Named* params, int_t num);

	/**
	* @brief pos番目の引数を得る。
	*
	* adjust_argsを読んだ後だけ使える
	*/
	const AnyPtr& arg_unchecked(int_t pos){
		return get(ff().ordered_arg_count-1-pos);
	}

	/**
	* @brief thisを取得。
	*
	*/
	const AnyPtr& arg_this(){ 
		return ff().self(); 
	}

	/**
	* @brief 位置指定引数の数を取得。
	*
	*/
	int_t ordered_arg_count(){ 
		return ff().ordered_arg_count; 
	}
	
	/**
	* @brief 名前指定引数の数を取得
	*
	*/
	int_t named_arg_count(){ 
		return ff().named_arg_count; 
	}

	/**
	* @brief 引数の多値を平らにする
	*
	*/
	void flatten_args();
	
	/**
	* @brief 呼び出し元が必要としている戻り値の数。
	*
	*/
	int_t need_result_count(){ 
		return ff().need_result_count; 
	}
	
	/**
	* @brief 呼び出し元が戻り値を必要としているかどうか。
	*
	*/
	bool need_result(){ 
		return ff().need_result_count!=0; 
	}
	
	/*
	* Argumentsオブジェクトを生成する。
	* return_result()を呼んだ後は正常な値は得られない。
	*/
	ArgumentsPtr make_arguments();
	
	/**
	* @brief 呼び出し元に引数の数だけの戻り値を返す。
	*
	* need_result_countの方が数が多い場合はその分nullが返される。
	* need_result_countの方が数が少ない場合は返した値は捨てられる。
	*/
	void return_result();
	void return_result(const AnyPtr& value1);
	void return_result(const AnyPtr& value1, const AnyPtr& value2);
	void return_result(const AnyPtr& value1, const AnyPtr& value2, const AnyPtr& value3);
	void return_result(const AnyPtr& value1, const AnyPtr& value2, const AnyPtr& value3, const AnyPtr& value4);

	/**
	* @brief 多値を返す。
	*/
	void return_result_mv(const MultiValuePtr& values);

	/**
	* @brief return_resultやcarry_overを既に呼び出したならtrueを、そうでないならfalseを返す。
	*
	*/
	bool processed(){ 
		return *ff().called_pc!=InstThrowUnsupportedError::NUMBER; 
	}

	void return_result_args(const VMachinePtr& vm);
	
	void prereturn_result(const AnyPtr& v);

	void recycle_call();

	void recycle_call(const AnyPtr& a1);

public:

	const AnyPtr& catch_except(){
		except_[2] = except();
		except_[0] = null;
		return ap(except_[2]);
	}

	const AnyPtr& except(){
		return ap(except_[0]);
	}

	void set_except(const AnyPtr& e){
		if(!ap(except_[0])){
			except_[0] = e;
		}
		else{
			XTAL_ASSERT(false); // 例外をハンドルせずに次の例外を設定した
		}
	}

	void set_except_0(const Any& e);

	void set_unsuported_error_info(const Any& target, const Any& primary_key, const Any& secondary_key){
		ff().target_ = target;
		ff().primary_key_ = primary_key;
		ff().secondary_key_ = secondary_key;
	}

	void execute_inner(const inst_t* start);

	void execute(Method* fun, const inst_t* start_pc);

	void carry_over(Method* fun);

	void mv_carry_over(Method* fun);

	const inst_t* resume_pc(){
		return resume_pc_;
	}

	void present_for_vm(Fiber* fun, VMachine* vm, bool add_succ_or_fail_result);

	const inst_t* start_fiber(Fiber* fun, VMachine* vm, bool add_succ_or_fail_result);

	const inst_t* resume_fiber(Fiber* fun, const inst_t* pc, VMachine* vm, bool add_succ_or_fail_result);
	
	void exit_fiber();
	
	void reset();

public:

	void adjust_result(int_t n){		
		adjust_result(n, ff().need_result_count);
	}

	void adjust_result(int_t n, int_t need_result_count);

public:

	// スタックのi番目の値を取得する。
	AnyPtr& get(int_t i){ return (AnyPtr&)ap(stack_[i]); }

	// スタックの0番目の値を取得する。
	AnyPtr& get(){ return (AnyPtr&)ap(stack_.top()); }

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
	void push(const Named& v){ push(v.name); push(v.value); }

	// スタックから値をポップする。
	const AnyPtr& pop(){ return ap(stack_.pop()); }

	// 先頭の値をプッシュする。
	void dup(){ push(get()); }

	// i番目の値をプッシュする。
	void dup(int_t i){ push(get(i)); }

	// スタックの大きさを返す。
	int_t stack_size(){ return (int_t)stack_.size(); }
	
	// srcのスタックの内容をsize個プッシュする。
	void push(VMachine* src, int_t size){ stack_.push(src->stack_, size); }
	
	// srcのスタックの内容をsize個プッシュする。
	void push(VMachine* src, int_t src_offset, int_t size){ stack_.push(src->stack_, src_offset, size); }

	// srcのスタックの内容をsize個取り除いて、プッシュする。
	void move(VMachine* src, int_t size){ stack_.move(src->stack_, size); }

	int_t push_mv(const MultiValuePtr& mv){
		int_t size = mv->size();
		for(int_t i=0; i<size; ++i){
			push(mv->at(i));
		}
		return size;
	}

public:

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

		int_t result_count;

		// yieldが可能かフラグ。このフラグは呼び出しを跨いで伝播する。
		int_t yieldable;

		// thisが持つインスタンス変数へのポインタ
		InstanceVariables* instance_variables;

		// この関数で積まれたローカル変数の数
		uint_t variable_size;

		// 呼び出された関数オブジェクト
		Any fun_; 

		// スコープの外側のフレームオブジェクト
		Any outer_;

		// 関数が呼ばれたときのthisオブジェクト
		Any self_;

		// オブジェクト化した引数。
		Any arguments_;
		
		// デバッグメッセージ出力用のヒント
		Any hint_;

		// UnsuportedErrorのためにtargetをおくところ
		Any target_;

		// UnsuportedErrorのためにprimary_keyをおくところ
		Any primary_key_;

		// UnsuportedErrorのためにsecondary_keyをおくところ
		Any secondary_key_;

		void set_null(){
			xtal::set_null(fun_); 
			xtal::set_null(outer_);
			xtal::set_null(arguments_);
			xtal::set_null(self_);
			xtal::set_null(hint_);
			xtal::set_null(target_);
			xtal::set_null(primary_key_);
			xtal::set_null(secondary_key_);
		}

		const FunPtr& fun() const{ return unchecked_ptr_cast<Fun>(ap(fun_)); }
		const FramePtr& outer() const{ return unchecked_ptr_cast<Frame>(ap(outer_)); }
		const AnyPtr& self() const{ return ap(self_); }
		const ArgumentsPtr& arguments() const{ return unchecked_ptr_cast<Arguments>(ap(arguments_)); }
		const AnyPtr& hint() const{ return ap(hint_); }
		const AnyPtr& target() const{ return ap(target_); }
		const IDPtr& primary_key() const{ return unchecked_ptr_cast<ID>(ap(primary_key_)); }
		const AnyPtr& secondary_key() const{ return ap(secondary_key_); }

		int_t args_stack_size(){
			return ordered_arg_count+(named_arg_count<<1);
		}

		void fun(const Any& v){ fun_ = v; }
		void outer(const Any& v){ outer_ = v; }
		void self(const Any& v){ self_ = v; }
		void arguments(const Any& v){ arguments_ = v; }
		void hint(const Any& v){ hint_ = v; }
		void target(const Any& v){ target_ = v; }
		void primary_key(const Any& v){ primary_key_ = v; }
		void secondary_key(const Any& v){ secondary_key_ = v; }

		void inc_ref();
		void dec_ref();
	};

	friend void visit_members(Visitor& m, const FunFrame& v);

	// 例外を処理するためのフレーム
	struct ExceptFrame{
		ExceptInfo* info;
		uint_t stack_size;
		uint_t fun_frame_size;
		uint_t scope_size;
		Any outer;
	};

	void push_call(const inst_t* pc, 
		int_t need_result_count, int_t ordered_arg_count, int_t named_arg_count, 
		const IDPtr& primary_key, const AnyPtr& secondary_key, const AnyPtr& self);

	void push_call(const inst_t* pc, const InstCall& inst);

	void push_call(const inst_t* pc, const InstSend& inst);

	void push_ff(int_t need_result_count);

	const inst_t* pop_ff(){
		if(ScopeInfo* scope = scopes_.pop()){
			variables_.downsize(scope->variable_size);
		}
		return fun_frames_.pop()->poped_pc; 
	}

	void push_args(const ArgumentsPtr& args, int_t named_arg_count);

	FunFrame& ff(){ return *fun_frames_.top(); }

	FunFrame& prev_ff(){ return *fun_frames_[1]; }

	const FunPtr& fun(){ return ff().fun(); }

	const FunPtr& prev_fun(){ return prev_ff().fun(); }

	const FramePtr& outer(){ return ff().outer(); }

	const FramePtr& prev_outer(){ return prev_ff().outer(); }

	const CodePtr& code();

	const CodePtr& prev_code();

	const IDPtr& identifier(int_t n);

	const IDPtr& prev_identifier(int_t n);

	void return_result_instance_variable(int_t number, ClassInfo* info);

	ArgumentsPtr inner_make_arguments(Method* fun);

	AnyPtr append_backtrace(const inst_t* pc, const AnyPtr& ep);
	
	const VMachinePtr& myself(){ return to_smartptr(this); }

private:
	const inst_t* inner_send_from_stack(const inst_t* pc, int_t need_result_count, const IDPtr& primary_key, int_t ntarget);
	const inst_t* inner_send_from_stack(const inst_t* pc, int_t need_result_count, const IDPtr& primary_key, int_t ntarget, int_t na0);
	const inst_t* inner_send_from_stack(const inst_t* pc, int_t need_result_count, const IDPtr& primary_key, int_t ntarget, int_t na0, int_t na1);
	const inst_t* inner_send_from_stack_q(const inst_t* pc, int_t need_result_count, const IDPtr& primary_key, int_t ntarget, int_t na0);
	const inst_t* inner_send_from_stack_q(const inst_t* pc, int_t need_result_count, const IDPtr& primary_key, int_t ntarget, int_t na0, int_t na1);

	const inst_t* push_except();

	void set_local_variable_outer(uint_t pos, const Any& value);

	AnyPtr& local_variable_outer(uint_t pos);

	void set_local_variable(uint_t pos, const Any& value){
		if(pos<ff().variable_size){
			variables_[pos] = value;
			return;
		}
		set_local_variable_outer(pos - ff().variable_size, value);
	}

	AnyPtr& local_variable(uint_t pos){
		if(pos<ff().variable_size){
			return (AnyPtr&)variables_[pos];
		}
		return local_variable_outer(pos - ff().variable_size);
	}

	const inst_t* catch_body(const inst_t* pc, const ExceptFrame& cur);

	void make_debug_info(const inst_t* pc, int_t kind);

	void debug_hook(const inst_t* pc, int_t kind);

public:

//{DECLS{{
	const inst_t* FunNop(const inst_t* pc);
	const inst_t* FunPushNull(const inst_t* pc);
	const inst_t* FunPushUndefined(const inst_t* pc);
	const inst_t* FunPushTrue(const inst_t* pc);
	const inst_t* FunPushFalse(const inst_t* pc);
	const inst_t* FunPushTrueAndSkip(const inst_t* pc);
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
	const inst_t* FunLocalVariableInc(const inst_t* pc);
	const inst_t* FunLocalVariableDec(const inst_t* pc);
	const inst_t* FunLocalVariableInc2Byte(const inst_t* pc);
	const inst_t* FunLocalVariableDec2Byte(const inst_t* pc);
	const inst_t* FunLocalVariable1Byte(const inst_t* pc);
	const inst_t* FunLocalVariable1ByteX2(const inst_t* pc);
	const inst_t* FunLocalVariable1ByteX3(const inst_t* pc);
	const inst_t* FunLocalVariable2Byte(const inst_t* pc);
	const inst_t* FunSetLocalVariable1Byte(const inst_t* pc);
	const inst_t* FunSetLocalVariable2Byte(const inst_t* pc);
	const inst_t* FunInstanceVariable(const inst_t* pc);
	const inst_t* FunSetInstanceVariable(const inst_t* pc);
	const inst_t* FunCleanupCall(const inst_t* pc);
	const inst_t* FunReturn(const inst_t* pc);
	const inst_t* FunYield(const inst_t* pc);
	const inst_t* FunExit(const inst_t* pc);
	const inst_t* FunValue(const inst_t* pc);
	const inst_t* FunCheckUnsupported(const inst_t* pc);
	const inst_t* FunProperty(const inst_t* pc);
	const inst_t* FunSetProperty(const inst_t* pc);
	const inst_t* FunCall(const inst_t* pc);
	const inst_t* FunSend(const inst_t* pc);
	const inst_t* FunMember(const inst_t* pc);
	const inst_t* FunDefineMember(const inst_t* pc);
	const inst_t* FunBlockBegin(const inst_t* pc);
	const inst_t* FunBlockEnd(const inst_t* pc);
	const inst_t* FunTryBegin(const inst_t* pc);
	const inst_t* FunTryEnd(const inst_t* pc);
	const inst_t* FunPushGoto(const inst_t* pc);
	const inst_t* FunPopGoto(const inst_t* pc);
	const inst_t* FunIf(const inst_t* pc);
	const inst_t* FunUnless(const inst_t* pc);
	const inst_t* FunGoto(const inst_t* pc);
	const inst_t* FunIfEq(const inst_t* pc);
	const inst_t* FunIfNe(const inst_t* pc);
	const inst_t* FunIfLt(const inst_t* pc);
	const inst_t* FunIfLe(const inst_t* pc);
	const inst_t* FunIfGt(const inst_t* pc);
	const inst_t* FunIfGe(const inst_t* pc);
	const inst_t* FunIfRawEq(const inst_t* pc);
	const inst_t* FunIfRawNe(const inst_t* pc);
	const inst_t* FunIfIn(const inst_t* pc);
	const inst_t* FunIfNin(const inst_t* pc);
	const inst_t* FunIfIs(const inst_t* pc);
	const inst_t* FunIfNis(const inst_t* pc);
	const inst_t* FunIfArgIsUndefined(const inst_t* pc);
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
	const inst_t* FunRange(const inst_t* pc);
	const inst_t* FunGlobalVariable(const inst_t* pc);
	const inst_t* FunSetGlobalVariable(const inst_t* pc);
	const inst_t* FunDefineGlobalVariable(const inst_t* pc);
	const inst_t* FunOnce(const inst_t* pc);
	const inst_t* FunSetOnce(const inst_t* pc);
	const inst_t* FunClassBegin(const inst_t* pc);
	const inst_t* FunClassEnd(const inst_t* pc);
	const inst_t* FunDefineClassMember(const inst_t* pc);
	const inst_t* FunMakeArray(const inst_t* pc);
	const inst_t* FunArrayAppend(const inst_t* pc);
	const inst_t* FunMakeMap(const inst_t* pc);
	const inst_t* FunMapInsert(const inst_t* pc);
	const inst_t* FunMapSetDefault(const inst_t* pc);
	const inst_t* FunMakeFun(const inst_t* pc);
	const inst_t* FunMakeInstanceVariableAccessor(const inst_t* pc);
	const inst_t* FunThrow(const inst_t* pc);
	const inst_t* FunThrowUnsupportedError(const inst_t* pc);
	const inst_t* FunThrowUndefined(const inst_t* pc);
	const inst_t* FunIfDebug(const inst_t* pc);
	const inst_t* FunAssert(const inst_t* pc);
	const inst_t* FunBreakPoint(const inst_t* pc);
	const inst_t* FunMAX(const inst_t* pc);
//}}DECLS}
	
	const inst_t* OpAddConstantInt(const inst_t* pc1, const inst_t* pc2, int_t op, Any& a, int_t constant);
	const inst_t* OpAddConstantInt(const inst_t* pc, int_t op, int_t constant);

	const inst_t* OpAdd(const inst_t* pc, int_t op);
	const inst_t* OpSub(const inst_t* pc, int_t op);
	const inst_t* OpMul(const inst_t* pc, int_t op);
	const inst_t* OpDiv(const inst_t* pc, int_t op);
	const inst_t* OpMod(const inst_t* pc, int_t op);
	const inst_t* OpAnd(const inst_t* pc, int_t op);
	const inst_t* OpOr(const inst_t* pc, int_t op);
	const inst_t* OpXor(const inst_t* pc, int_t op);
	const inst_t* OpShl(const inst_t* pc, int_t op);
	const inst_t* OpShr(const inst_t* pc, int_t op);
	const inst_t* OpUshr(const inst_t* pc, int_t op);

private:

	const FramePtr& make_outer(ScopeInfo* scope);
	const FramePtr& make_outer_outer();

private:
	inst_t end_code_;
	inst_t throw_code_;
	inst_t throw_unsupported_error_code_;
	inst_t throw_undefined_code_;
	inst_t check_unsupported_code_;
	inst_t cleanup_call_code_;

	const inst_t* resume_pc_;
	int_t yield_result_count_;

	const IDPtr* id_;

	// 計算用スタック
	FastStack<Any> stack_;

	// 関数呼び出しの度に積まれるフレーム
	FastStack<FunFrame*> fun_frames_;

	// スコープ情報
	FastStack<ScopeInfo*> scopes_;

	// tryの度に積まれるフレーム
	FastStack<ExceptFrame> except_frames_;

	// スコープがオブジェクト化されてない時のローカル変数領域
	FastStack<Any> variables_;
	
	Any except_[3];

	SmartPtr<DebugInfo> debug_info_;
	SmartPtr<Debug> debug_;
	int_t hook_setting_bit_;

	int_t thread_yield_count_;

public:

	void assign(const VMachinePtr& vm){
		resume_pc_ = vm->resume_pc_;
		yield_result_count_ = vm->yield_result_count_;

		stack_ = vm->stack_;

		fun_frames_.resize(vm->fun_frames_.size());
		for(uint_t i=0; i<vm->fun_frames_.size(); ++i){
			if(fun_frames_[i]){
				*fun_frames_[i] = *vm->fun_frames_[i];
			}
			else{
				void* p = xmalloc(sizeof(FunFrame));
				fun_frames_[i] = new(p) FunFrame(*vm->fun_frames_[i]);
			}
		}

		variables_ = vm->variables_;

		except_frames_ = vm->except_frames_;
		
		except_[0] = vm->except_[0];
		except_[1] = vm->except_[1];
		except_[2] = vm->except_[2];

		debug_info_ = vm->debug_info_;
	}

	friend void swap(VMachine& a, VMachine& b){
		using namespace std;

		std::swap(a.resume_pc_, b.resume_pc_);
		std::swap(a.yield_result_count_, b.yield_result_count_);

		swap(a.stack_, b.stack_);
		swap(a.fun_frames_, b.fun_frames_);

		swap(a.variables_, b.variables_);

		swap(a.except_frames_, b.except_frames_);

		swap(a.except_[0], b.except_[0]);
		swap(a.except_[1], b.except_[1]);
		swap(a.except_[2], b.except_[2]);

		swap(a.debug_info_, b.debug_info_);
	}

protected:

	virtual void visit_members(Visitor& m);

	virtual void before_gc();
	virtual void after_gc();

public:

	void print_info();

private:

	XTAL_DISALLOW_COPY_AND_ASSIGN(VMachine);

};

}

#endif // XTAL_VMACHINE_H_INCLUDE_GUARD
