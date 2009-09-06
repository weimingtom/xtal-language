/** \file src/xtal/xtal_vmachine.h
* \brief src/xtal/xtal_vmachine.h
*/

#ifndef XTAL_VMACHINE_H_INCLUDE_GUARD
#define XTAL_VMACHINE_H_INCLUDE_GUARD

#pragma once

namespace xtal{
	
/**
* \brief 関数呼び出しで、名前付き引数として渡すためのクラス
*
*/
struct Named{
	const IDPtr& name;
	const Any& value;

	/**
	* \brief 名前と値を指定して構築する。
	*/
	Named(const IDPtr& name, const Any& value)
		:name(name), value(value){}

	/**
	* \brief 空な状態で生成する
	*/
	Named()
		:name((const IDPtr&)null), value(undefined){}
};

struct NamedParam{
	NamedParam()
		:name(null), value(undefined){}
	IDPtr name;
	AnyPtr value;
};

struct Param{
	enum{
		SHIFT = 16,

		NAMED = (1<<SHIFT),
		STR = (2<<SHIFT),
		STR8 = (3<<SHIFT)
	};

	Param(const Named& n){
		type = NAMED;
		name.name = &n.name;
		name.value = &n.value;
	}

	Param(const char_t* s){
		type = STR;
		str = s;
	}

	Param(const char8_t* s){
		type = STR8;
		str8 = s;
	}

	Param(const AnyPtr& v){
		type = xtal::type(v);
		value = rawvalue(v);
	}

	Param(char v){ type = TYPE_INT; value.ivalue = (int_t)v; }
	Param(signed char v){ type = TYPE_INT; value.ivalue = (int_t)v; }
	Param(unsigned char v){ type = TYPE_INT; value.ivalue = (int_t)v; }
	Param(short v){ type = TYPE_INT; value.ivalue = (int_t)v; }
	Param(unsigned short v){ type = TYPE_INT; value.ivalue = (int_t)v; }
	Param(int v){ type = TYPE_INT; value.ivalue = (int_t)v; }
	Param(unsigned int v){ type = TYPE_INT; value.ivalue = (int_t)v; }
	Param(long v){ type = TYPE_INT; value.ivalue = (int_t)v; }
	Param(unsigned long v){ type = TYPE_INT; value.ivalue = (int_t)v; }

	Param(float v){ type = TYPE_FLOAT; value.fvalue = (float_t)v; }
	Param(double v){ type = TYPE_FLOAT; value.fvalue = (float_t)v; }
	Param(long double v){ type = TYPE_FLOAT; value.fvalue = (float_t)v; }

	Param(bool b){ type = TYPE_FALSE + (int)b; value.ivalue = 0; }

	int_t type;

	struct NamedPair{
		const IDPtr* name;
		const Any* value;		
	};

	union{
		const char_t* str;
		const char8_t* str8;
		NamedPair name;
		AnyRawValue value;
	};
};

struct f2{
	float_t a, b;
};

inline void to_f2(f2& ret, int_t atype, const AnyPtr& a, int_t btype, const AnyPtr& b){
	float_t aa[2] = {(float_t)ivalue(a), fvalue(a)};
	float_t bb[2] = {(float_t)ivalue(b), fvalue(b)};
	ret.a = aa[atype];
	ret.b = bb[btype];
}

// XTAL仮想マシン
class VMachine : public GCObserver{
public:

	VMachine();

	~VMachine();

public:

	// 関数呼び出し側が使うための関数群

	/**
	* \brief 引数を1個積む。
	*
	*/
	void push_arg(const AnyPtr& value);
	
	/**
	* \brief 名前付き引数を1個積む。
	*
	*/
	void push_arg(const IDPtr& name, const AnyPtr& value);
	
	/**
	* \brief 名前付き引数を1個積む。
	*
	*/
	void push_arg(const Named& p){ push_arg(p.name, ap(p.value)); }

	/**
	* \brief 引数を1個積む。
	*
	*/
	void push_arg(const Param& p);

	/**
	* \brief 引数を配列の要素数積む。
	*
	*/
	void push_ordered_args(const ArrayPtr& p);

	/**
	* \brief 名前つき引数を連想配列の要素数積む。
	*
	*/
	void push_named_args(const MapPtr& p);

	/**
	* \brief pos番目の戻り値を得る。
	*
	*/
	const AnyPtr& result(int_t pos = 0);

	/**
	* \brief 呼び出しの後始末をする。
	*
	*/
	void cleanup_call();	

	/**
	* \brief pos番目の戻り値を返し、呼び出しの後始末をする。
	*
	*/
	AnyPtr result_and_cleanup_call(int_t pos = 0);
		
	/**
	* \brief thisを差し替える。
	*
	*/	
	void set_arg_this(const AnyPtr& self){ 
		ff().set_self(self);
	}

// 

	/// \brief 関数を呼び出す用意をし、同時に引数を0個積む
	void setup_call(int_t need_result_count = 1);

//{REPEAT{{
/*
	/// \brief 関数を呼び出す用意をし、同時に引数を`n+1`個積む
	void setup_call(int_t need_result_count, const Param& a0 #COMMA_REPEAT#const Param& a`i+1`#);
*/

	/// \brief 関数を呼び出す用意をし、同時に引数を1個積む
	void setup_call(int_t need_result_count, const Param& a0 );

	/// \brief 関数を呼び出す用意をし、同時に引数を2個積む
	void setup_call(int_t need_result_count, const Param& a0 , const Param& a1);

	/// \brief 関数を呼び出す用意をし、同時に引数を3個積む
	void setup_call(int_t need_result_count, const Param& a0 , const Param& a1, const Param& a2);

	/// \brief 関数を呼び出す用意をし、同時に引数を4個積む
	void setup_call(int_t need_result_count, const Param& a0 , const Param& a1, const Param& a2, const Param& a3);

	/// \brief 関数を呼び出す用意をし、同時に引数を5個積む
	void setup_call(int_t need_result_count, const Param& a0 , const Param& a1, const Param& a2, const Param& a3, const Param& a4);

	/// \brief 関数を呼び出す用意をし、同時に引数を6個積む
	void setup_call(int_t need_result_count, const Param& a0 , const Param& a1, const Param& a2, const Param& a3, const Param& a4, const Param& a5);

//}}REPEAT}

public:

	// 関数呼び出され側が使うための関数群

	/**
	* \brief pos番目の引数を得る。
	*
	*/
	const AnyPtr& arg(int_t pos);

	/**
	* \brief nameに対応する引数を得る。
	*
	*/
	const AnyPtr& arg(const IDPtr& name){
		return arg_default(name, undefined);
	}

	/**
	* \brief pos番目の引数を得る。もしpos番目の引数がなければnameに対応する引数を得る。
	*
	*/
	const AnyPtr& arg(int_t pos, const IDPtr& name);
	
	/**
	* \brief pos番目の引数を得る。
	*
	* もしpos番目の引数が無ければdefの値を返す。
	*/
	const AnyPtr& arg_default(int_t pos, const AnyPtr& def);

	/**
	* \brief nameに対応する引数を得る。
	*
	* もしnameに対応する引数が無ければdefの値を返す。
	*/
	const AnyPtr& arg_default(const IDPtr& name, const AnyPtr& def);

	/**
	* \brief pos番目の引数を得る。もしpos番目の引数がなければnameに対応する引数を得る。
	*
	* もしnameに対応する引数が無ければdefの値を返す。
	*/
	const AnyPtr& arg_default(int_t pos, const IDPtr& name, const AnyPtr& def);

	/**
	* \brief pos番目の名前指定引数の名前を取得。
	*
	* \param pos 0からnamed_arg_count()-1まで
	*/
	const IDPtr& arg_name(int_t pos){
		return unchecked_ptr_cast<ID>(local_variable(ordered_arg_count()+pos*2));
	}

	void adjust_args(const NamedParam* params, int_t num);

	void adjust_args(Method* params, int_t num);

	/**
	* \brief pos番目の引数を得る。
	*
	* adjust_argsを読んだ後だけ使える
	*/
	const AnyPtr& arg_unchecked(int_t pos){
		return local_variable(pos);
	}

	/**
	* \brief thisを取得。
	*
	*/
	const AnyPtr& arg_this(){ 
		return ff().self(); 
	}

	/**
	* \brief 位置指定引数の数を取得。
	*
	*/
	int_t ordered_arg_count(){ 
		return ff().ordered_arg_count; 
	}
	
	/**
	* \brief 名前指定引数の数を取得
	*
	*/
	int_t named_arg_count(){ 
		return ff().named_arg_count; 
	}

	/**
	* \brief 引数の多値を平らにする
	*
	*/
	void flatten_args();
	
	/**
	* \brief 呼び出し元が必要としている戻り値の数。
	*
	*/
	int_t need_result_count(){ 
		return ff().need_result_count; 
	}
	
	/**
	* \brief 呼び出し元が戻り値を必要としているかどうか。
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
	* \brief 呼び出し元に引数の数だけの戻り値を返す。
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
	* \brief 多値を返す。
	*/
	void return_result_mv(const ValuesPtr& values);

	/**
	* \brief return_resultやcarry_overを既に呼び出したならtrueを、そうでないならfalseを返す。
	*
	*/
	int_t processed(){
		return ff().processed; 
	}
	
	void prereturn_result(const AnyPtr& v);

	void recycle_call();

	void recycle_call(const AnyPtr& a1);

public:

	FramePtr current_context(){
		return make_outer_outer();
	}

public:

	const AnyPtr& catch_except();

	const AnyPtr& except(){
		return ap(except_[0]);
	}

	void set_except(const AnyPtr& e);

	void set_except_0(const Any& e);

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

	void adjust_values(int_t n, int_t need_result_count);
	void adjust_values2(int_t stack_base, int_t n, int_t need_result_count);
	void adjust_values3(Any* values, int_t n, int_t need_result_count);

	void set_hook_setting_bit(uint_t v){
		hook_setting_bit_ = v;
	}

public:

	void move_variables(VMachine* src, int_t size){
		for(int_t i=0; i<size; ++i){
			set_local_variable(i, src->local_variable(i));
		}
	}

	/*
	int_t push_mv(const ValuesPtr& mv){
		int_t size = mv->size();
		for(int_t i=0; i<size; ++i){
			push(mv->at(i));
		}
		return size;
	}*/

public:

	debug::CallerInfoPtr caller(uint_t n);

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

		// 実際の戻り値の数
		int_t result_count;

		// yieldが可能かフラグ。このフラグは呼び出しを跨いで伝播する。
		int_t yieldable;

		// thisが持つインスタンス変数へのポインタ
		InstanceVariables* instance_variables;

		// この関数までに積まれたスコープの数
		uint_t scope_size;

		int_t processed;

		int_t result;

		uint_t stack_base;

		uint_t prev_stack_base;

		// 呼び出された関数オブジェクト
		Any fun_; 

		const IDPtr* identifier_;

		// 関数の外側のフレームオブジェクト
		Any outer_;

		// 関数が呼ばれたときのthisオブジェクト
		Any self_;

		FunFrame();

		void set_null();

		const FunPtr& fun() const{ return unchecked_ptr_cast<Fun>(ap(fun_)); }
		const FramePtr& outer() const{ return unchecked_ptr_cast<Frame>(ap(outer_)); }
		//const FramePtr& outer() const{ return fun()->outer(); }
		//const IDPtr& identifier(int_t n){ return fun()->code()->identifier(n); }
		const IDPtr& identifier(int_t n){ return identifier_[n]; }
		const AnyPtr& self() const{ return ap(self_); }

		int_t args_stack_size(){
			return ordered_arg_count+(named_arg_count<<1);
		}

		//void set_fun(const MethodPtr& v){ fun_ = v; }
		void set_fun(){ fun_ = null; outer_ = null; }
		//void set_fun(const MethodPtr& v){ fun_ = v; identifier_ = &fun()->code()->identifier(0); }
		void set_fun(const MethodPtr& v){ fun_ = v; outer_ = v->outer();  identifier_ = &fun()->code()->identifier(0); }
		//void set_fun(){ fun_ = null; outer_ = null; }
		void set_outer(const Any& v){ outer_ = v; }
		void set_self(const Any& v){ self_ = v; }
		void set_self(){ self_ = null; }
	};

	friend void visit_members(Visitor& m, FunFrame& v);

	// 例外を処理するためのフレーム
	struct ExceptFrame{
		ExceptInfo* info;
		uint_t stack_size;
		uint_t fun_frame_size;
		uint_t scope_size;
		uint_t variables_top;
	};

	FunFrame& ff(){ 
		return *fun_frames_.top(); 
	}

private:

	void push_args(const ArgumentsPtr& args, int_t stack_base, int_t ordered_arg_count, int_t named_arg_count);

	struct CallState{
		Any cls;
		Any target;
		Any primary;
		Any secondary;
		Any self;
		Any member;

		const inst_t* pc;
		const inst_t* npc;
		int_t result;
		int_t need_result_count;
		int_t stack_base;
		int_t ordered;
		int_t named;
		int_t flags;

		void set(const inst_t* pc, const inst_t* npc,
			int_t result, int_t need_result_count, 
			int_t stack_base, int_t ordered, int_t named, 
			int_t flags){

			this->pc = pc;
			this->npc = npc; 
			this->result = result; 
			this->need_result_count = need_result_count;
			this->stack_base = stack_base;
			this->ordered = ordered;
			this->named = named;
			this->flags = flags;
		}
	};

	const inst_t* check_accessibility(CallState& call_state, int_t accessibility);

	FramePtr& push_scope(ScopeInfo* info = &empty_scope_info);

	void pop_scope();

	void push_ff(CallState& call_state);

	void pop_ff();

	void pop_ff2();

	void execute();

	void upsize_variables(uint_t upsize);

public:

	FunFrame& prev_ff(){ 
		return *fun_frames_[1]; 
	}

	const FunPtr& fun(){ 
		return ff().fun(); 
	}

	const FunPtr& prev_fun(){ 
		return prev_ff().fun(); 
	}

	const FramePtr& outer(){ 
		return ff().outer(); 
	}

	const FramePtr& prev_outer(){ 
		return prev_ff().outer(); 
	}

	const CodePtr& code(){ 
		return ff().fun()->code(); 
	}

	const CodePtr& prev_code(){ 
		return prev_ff().fun()->code(); 
	}

	const IDPtr& identifier(int_t n){ 
		return ff().identifier(n); 
	}

	const IDPtr& prev_identifier(int_t n){ 
		return prev_ff().identifier(n);  
	}

	ArgumentsPtr inner_make_arguments(Method* fun);

	AnyPtr append_backtrace(const inst_t* pc, const AnyPtr& ep);

private:

	const inst_t* push_except(const inst_t* pc);
	const inst_t* push_except(const inst_t* pc, const AnyPtr& e);

	void set_local_variable_out_of_fun(uint_t pos, uint_t depth, const Any& value);

	AnyPtr& local_variable_out_of_fun(uint_t pos, uint_t depth);

	AnyPtr& local_variable(int_t pos){
		return (AnyPtr&)variables_top_[pos];
	}

	void set_local_variable(int_t pos, const Any& value){
		(Any&)variables_top_[pos] = value;
	}

	int_t variables_top(){
		return variables_top_-(Any*)variables_.data();
	}
	
	void set_variables_top(int_t top){
		variables_top_ = (Any*)variables_.data() + top;
	}

	const inst_t* catch_body(const inst_t* pc, const ExceptFrame& cur);

	void make_debug_info(const inst_t* pc, int_t kind);

	void debug_hook(const inst_t* pc, int_t kind);

	void check_debug_hook(const inst_t* pc, int_t kind){
		if(disable_debug_ || (hook_setting_bit_&(1<<kind))==0 || !debug::is_enabled()){
			return;
		}
		debug_hook(pc, kind);
	}

public:

//{DECLS{{
	const inst_t* FunLine(const inst_t* pc);
	const inst_t* FunLoadValue(const inst_t* pc);
	const inst_t* FunLoadConstant(const inst_t* pc);
	const inst_t* FunLoadInt1Byte(const inst_t* pc);
	const inst_t* FunLoadFloat1Byte(const inst_t* pc);
	const inst_t* FunLoadCallee(const inst_t* pc);
	const inst_t* FunLoadThis(const inst_t* pc);
	const inst_t* FunCopy(const inst_t* pc);
	const inst_t* FunMember(const inst_t* pc);
	const inst_t* FunUna(const inst_t* pc);
	const inst_t* FunArith(const inst_t* pc);
	const inst_t* FunBitwise(const inst_t* pc);
	const inst_t* FunAt(const inst_t* pc);
	const inst_t* FunSetAt(const inst_t* pc);
	const inst_t* FunGoto(const inst_t* pc);
	const inst_t* FunNot(const inst_t* pc);
	const inst_t* FunIf(const inst_t* pc);
	const inst_t* FunIfComp(const inst_t* pc);
	const inst_t* FunIfRawEq(const inst_t* pc);
	const inst_t* FunIfIs(const inst_t* pc);
	const inst_t* FunIfUndefined(const inst_t* pc);
	const inst_t* FunIfDebug(const inst_t* pc);
	const inst_t* FunPush(const inst_t* pc);
	const inst_t* FunPop(const inst_t* pc);
	const inst_t* FunAdjustValues(const inst_t* pc);
	const inst_t* FunLocalVariable(const inst_t* pc);
	const inst_t* FunSetLocalVariable(const inst_t* pc);
	const inst_t* FunInstanceVariable(const inst_t* pc);
	const inst_t* FunSetInstanceVariable(const inst_t* pc);
	const inst_t* FunFilelocalVariable(const inst_t* pc);
	const inst_t* FunCall(const inst_t* pc);
	const inst_t* FunSend(const inst_t* pc);
	const inst_t* FunProperty(const inst_t* pc);
	const inst_t* FunSetProperty(const inst_t* pc);
	const inst_t* FunScopeBegin(const inst_t* pc);
	const inst_t* FunScopeEnd(const inst_t* pc);
	const inst_t* FunReturn(const inst_t* pc);
	const inst_t* FunYield(const inst_t* pc);
	const inst_t* FunExit(const inst_t* pc);
	const inst_t* FunRange(const inst_t* pc);
	const inst_t* FunOnce(const inst_t* pc);
	const inst_t* FunSetOnce(const inst_t* pc);
	const inst_t* FunMakeArray(const inst_t* pc);
	const inst_t* FunArrayAppend(const inst_t* pc);
	const inst_t* FunMakeMap(const inst_t* pc);
	const inst_t* FunMapInsert(const inst_t* pc);
	const inst_t* FunMapSetDefault(const inst_t* pc);
	const inst_t* FunClassBegin(const inst_t* pc);
	const inst_t* FunClassEnd(const inst_t* pc);
	const inst_t* FunDefineClassMember(const inst_t* pc);
	const inst_t* FunDefineMember(const inst_t* pc);
	const inst_t* FunMakeFun(const inst_t* pc);
	const inst_t* FunMakeInstanceVariableAccessor(const inst_t* pc);
	const inst_t* FunTryBegin(const inst_t* pc);
	const inst_t* FunTryEnd(const inst_t* pc);
	const inst_t* FunPushGoto(const inst_t* pc);
	const inst_t* FunPopGoto(const inst_t* pc);
	const inst_t* FunThrow(const inst_t* pc);
	const inst_t* FunAssert(const inst_t* pc);
	const inst_t* FunMAX(const inst_t* pc);
//}}DECLS}

private:

	const FramePtr& make_outer(ScopeInfo* scope);
	const FramePtr& make_outer_outer(uint_t i=0);

private:
	inst_t end_code_;
	inst_t throw_code_;

	int_t result_base_;

	const inst_t* resume_pc_;
	
	int_t yield_result_;
	int_t yield_need_result_count_;
	int_t yield_target_count_;

	const inst_t* throw_pc_;

	const IDPtr* id_;

	// 計算用スタック
	FastStack<Any> stack_;

	// 関数呼び出しの度に積まれるフレーム
	FastStack<FunFrame*> fun_frames_;

	struct Scope{
		FramePtr frame;
		uint_t pos;
		uint_t size;
		uint_t flags;

		enum{
			NONE = 0,
			FRAME = 1,
			CLASS = 2,
		};
	};

	// スコープ情報
	FastStack<Scope> scopes_;

	Any* variables_top_;
	xarray variables_;

	// tryの度に積まれるフレーム
	PODStack<ExceptFrame> except_frames_;
	
	Any except_[3];
	StringPtr assertion_message_;

	debug::HookInfoPtr debug_info_;
	bool disable_debug_;
	int_t hook_setting_bit_;

	int_t thread_yield_count_;

protected:

	virtual void visit_members(Visitor& m);

	virtual void before_gc();
	virtual void after_gc();

	void add_ref_count_members(int_t i);

public:

	void print_info();

private:

	XTAL_DISALLOW_COPY_AND_ASSIGN(VMachine);

};


}

#endif // XTAL_VMACHINE_H_INCLUDE_GUARD
