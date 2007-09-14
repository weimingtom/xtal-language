
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
	void compile_stmt(Stmt* p);	
	
	/**
	* コンパイルエラーを取得する。
	*/
	ArrayPtr errors();
		
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

	void put_inst2(const Inst& t, uint_t sz);

	template<class T>
	void put_inst(const T& t){
		put_inst2(t, sizeof(t));
	}
	
	/**
	* コードのサイズを得る。
	*/
	int_t code_size();
		
	InternedStringPtr to_id(int_t i);

	void assign_lhs(Expr* lhs);
	void define_lhs(Expr* lhs);
	
	struct FunFrame{

		int_t var_frame_count;
		int_t fun_core_num;

		struct Label{
			int_t pos;
			struct From{
				int_t line;
				int_t pos;
				int_t set_pos;
			};
			AC<From>::vector froms;
		};
		
		AC<Label>::vector labels;
		
		struct Loop{
			int_t name; // ラベル名
			int_t frame_count; // フレームの数
			int_t break_label; // break_labelの番号
			int_t continue_label; // continue_labelの番号
			bool have_label; // 対応するラベルを持っているか
		};
		
		PODStack<Loop> loops;

		struct Finally{
			int_t frame_count;
			int_t finally_label;
		};

		PODStack<Finally> finallys;

		bool used_args_object;


	};

	struct VarFrame{
		struct Entry{
			int_t name;
			bool constant;
			int_t accessibility;
		};

		AC<Entry>::vector entrys;
		
		struct Direct{
			int_t pos;
		};

		AC<Direct>::vector directs;
		int_t block_core_num;

		enum{
			SCOPE,
			CLASS,
			BLOCK		
		};

		int_t kind;
	};

	struct ClassFrame{
		struct Entry{
			int_t name;
		};

		AC<Entry>::vector entrys;
		int_t class_core_num;
	};

	struct LVarInfo{
		VarFrame* var_frame;
		int_t pos;
	};

	LVarInfo var_find(int_t key){
		LVarInfo ret = {0, 0};
		for(size_t i = 0, last = var_frames_.size(); i<last; ++i){
			VarFrame& vf = var_frames_[i];
			for(size_t j = 0, jlast = vf.entrys.size(); j<jlast; ++j){
				if(vf.entrys[vf.entrys.size()-1-j].name==key){
					ret.var_frame = &vf;
					return ret;
				}
				ret.pos++;
			}
		}
		ret.pos = -1;
		return ret;
	}

	void var_begin(int_t kind){
		VarFrame& vf = var_frames_.push();
		vf.entrys.clear();
		vf.directs.clear();
		vf.block_core_num = 0;
		vf.kind = kind;
	}

	void var_define(TList<Stmt*>& stmts){
		for(TList<Stmt*>::Node* p=stmts.head; p; p=p->next){
			if(DefineStmt* def_stmt = stmt_cast<DefineStmt>(p->value)){
				if(LocalExpr* lvar_expr = expr_cast<LocalExpr>(def_stmt->lhs)){
					var_define(lvar_expr->var);
				}
			}else if(MultipleAssignStmt* ma_stmt = stmt_cast<MultipleAssignStmt>(p->value)){
				if(ma_stmt->define){
					for(TList<Expr*>::Node* p=ma_stmt->lhs.head; p; p=p->next){
						if(LocalExpr* lvar_expr = expr_cast<LocalExpr>(p->value)){
							var_define(lvar_expr->var);
						}			
					}
				}
			}
		}
	}

	void var_set_direct(VarFrame& vf){
		VarFrame::Direct d;
		d.pos = code_size();
		vf.directs.push_back(d);
	}

	void var_define(int_t name, int_t accessibility = 0){
		VarFrame& vf = var_frames_.top();
		VarFrame::Entry entry;
		entry.name = name;
		entry.constant = false;
		entry.accessibility = accessibility;
		vf.entrys.push_back(entry);
	}

	void var_set_on_heap(){
		for(uint_t i=0; i<var_frames_.size(); ++i){
			if(var_frames_[i].kind==VarFrame::SCOPE){
				var_frames_[i].kind = VarFrame::BLOCK;
			}
		}
	}

	void var_end(){
		VarFrame& vf = var_frames_.top();
		if(vf.kind==VarFrame::SCOPE){
			for(uint_t i=0; i<vf.directs.size(); ++i){
				Inst* p = (Inst*)&result_->code_[vf.directs[i].pos];
				p->op += 1;
			}
		}
		var_frames_.pop();
	}

	void block_begin();
	void block_end();
	
	void fun_begin(int_t kind, bool have_args, int_t offset, u8 min_param_count, u8 max_param_count);
	void fun_end(int_t n);

	FunFrame& fun_frame();

	int_t line(){ return lines_.top(); }

	int_t class_core_num(){
		return class_frames_.empty() ? 0 : class_frames_.top().class_core_num;
	}

private:
	
	Parser parser_;
	LPCCommon* com_;
	CodePtr result_;
	
	Stack<FunFrame> fun_frames_;
	Stack<VarFrame> var_frames_;
	Stack<ClassFrame> class_frames_;

	PODStack<int_t> label_names_;
	PODStack<int_t> lines_;

private:
	CodeBuilder(const CodeBuilder&);
	CodeBuilder& operator=(const CodeBuilder&);
};

}

#endif

