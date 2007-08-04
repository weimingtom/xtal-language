
#pragma once

#ifndef XTAL_NO_PARSER

#include "xtal_expr.h"
#include "xtal_parser.h"
#include "xtal_macro.h"

namespace xtal{

class CodeBuilder{
public:

	CodeBuilder();

	~CodeBuilder();
	
	CodePtr compile(const StreamPtr& stream, const StringPtr& source_file_name = "anonymous");

	void interactive_compile();

	void adjust_result(int_t need_result_count, int_t result_count);

	struct CompileInfo{
		int_t need_result_count;
		bool tail;

		CompileInfo(int_t need_result_count = 1, bool tail = false)
			:need_result_count(need_result_count), tail(tail){}
	};

	void compile(Expr* p, const CompileInfo& info = CompileInfo());
	void compile(Stmt* p);	
	
	/**
	* コンパイルエラーを取得する。
	*/
	ArrayPtr errors();
		
	/**
	* 識別子が変数としてあるか探し、変数位置を返す。
	* @param key 識別子
	* @retval -1 登録されていない
	* @retval 非-1 変数位置
	*/
	int_t lookup_variable(int_t key);

	bool variable_on_heap(int_t n);
	
	/**
	* ラベル番号を予約し、返す。
	*/
	int_t reserve_label();
	
	/**
	* ラベルを設定する。
	*/
	void set_label(int_t lableno);

	/**
	* ラベルにジャンプするコードを埋め込めるように細工する。
	*/	
	void set_jump(int_t offset, int_t labelno);
	void process_labels();

	bool put_local_code(int_t var);
	bool put_set_local_code(int_t var);
	void put_define_local_code(int_t var);
	void put_send_code(int_t var, Expr* pvar, int_t need_result_count, bool tail, bool if_defined);
	void put_set_send_code(int_t var, Expr* pvar, bool if_defined);
	void put_member_code(int_t var, Expr* pvar, bool if_defined);
	void put_define_member_code(int_t var, Expr* pvar);

	/**
	* 識別子が変数としてあるか探し、変数位置を返す。
	* @param key 識別子
	* @retval -1 登録されていない
	* @retval 非-1 変数位置
	*/
	int_t lookup_instance_variable(int_t key);
	void put_set_instance_variable_code(int_t var);
	void put_instance_variable_code(int_t var);
	void put_if_code(Expr* cond, int_t label_if, int_t label_if2);
	void push_loop(int break_labelno, int continue_labelno, int_t name = 0, bool have_label = false);
	void pop_loop();
	
	/**
	* ブロックの終りを埋め込む
	*/
	void break_off(int_t to);

	template<class T>
	void put_inst(const T& t){

		if(t.op==255){
			com_->error(line(), Xt("Xtal Compile Error 1027"));
		}

		size_t cur = result_->code_.size();
		size_t sz = sizeof(T);
		result_->code_.resize(cur+sz/sizeof(inst_t));
		memcpy(&result_->code_[cur], &t, sz);
	}
	
	/**
	* コードのサイズを得る。
	*/
	int_t code_size();
		
	InternedStringPtr to_id(int_t i);

	void assign_lhs(Expr* lhs);
	void define_lhs(Expr* lhs);
	
	struct FunFrame{

		int_t frame_count;

		struct Label{
			int pos;
			struct From{
				int line;
				int pos;
				int set_pos;
			};
			AC<From>::vector froms;
		};
		
		AC<Label>::vector labels;
		
		struct Loop{
			int_t name; // ラベル名
			int frame_count; // フレームの数
			int break_label; // break_labelの番号
			int continue_label; // continue_labelの番号
			bool have_label; // 対応するラベルを持っているか
		};
		
		PODStack<Loop> loops;

		struct Finally{
			int frame_count;
			int finally_label;
		};

		PODStack<Finally> finallys;

		bool used_args_object;
	};


	void block_begin(Vars* vars);
	void block_end();
	void class_begin(Vars* vars, int_t mixins);
	void class_end();
	void fun_begin(Vars* vars, bool have_args, int_t offset, u8 min_param_count, u8 max_param_count);
	void fun_end();

	FunFrame& fun_frame();

	int_t line(){ return lines_.top(); }

	int_t frame_number(){
		return class_scopes_.empty() ? 0 : class_scopes_.top()->frame_number;
	}

private:
	
	Parser parser_;
	LPCCommon* com_;
	CodePtr result_;
	
	PODStack<int_t> label_names_;
	Stack<FunFrame> fun_frames_;
	PODStack<int_t> lines_;
	PStack<Vars*> vars_stack_;
	Stack<ClassExpr*> class_scopes_;

private:
	CodeBuilder(const CodeBuilder&);
	CodeBuilder& operator=(const CodeBuilder&);
};

}

#endif

