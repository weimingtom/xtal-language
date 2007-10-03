
#pragma once

#ifndef XTAL_NO_PARSER

#include "xtal_expr.h"
#include "xtal_parser.h"
#include "xtal_macro.h"

namespace xtal{

class CodeBuilder{
public:

	CodeBuilder(){}
	
	CodePtr compile(const StreamPtr& stream, const StringPtr& source_file_name = "anonymous");

	void interactive_compile();

	/**
	* コンパイルエラーを取得する。
	*/
	AnyPtr errors();

private:

	CodePtr compile_toplevel(const ExprPtr& e, const StringPtr& source_file_name);

	void adjust_result(int_t need_result_count, int_t result_count);

	struct CompileInfo{
		int_t need_result_count;
		bool tail;

		CompileInfo(int_t need_result_count = 1, bool tail = false)
			:need_result_count(need_result_count), tail(tail){}
	};

	AnyPtr compile_expr(const AnyPtr& p, const CompileInfo& info = CompileInfo());
	void compile_stmt(const AnyPtr& p);	

	int_t reserve_label();
	void set_label(int_t lableno);
	void set_jump(int_t offset, int_t labelno);
	void process_labels();

	bool put_local_code(const InternedStringPtr& var);
	bool put_set_local_code(const InternedStringPtr& var);
	void put_define_local_code(const InternedStringPtr& var, const ExprPtr& val = null);
	void put_send_code(const InternedStringPtr& var, ExprPtr pvar, int_t need_result_count, bool tail, bool if_defined);
	void put_set_send_code(const InternedStringPtr& var, ExprPtr pvar, bool if_defined);
	void put_member_code(const InternedStringPtr& var, ExprPtr pvar, bool if_defined);
	void put_define_member_code(const InternedStringPtr& var, ExprPtr pvar);
	int_t lookup_instance_variable(const InternedStringPtr& key);
	void put_set_instance_variable_code(const InternedStringPtr& var);
	void put_instance_variable_code(const InternedStringPtr& var);
	void put_val_code(const AnyPtr& val);
	void put_if_code(const ExprPtr& cond, int_t label_if, int_t label_if2);
	void break_off(int_t to);
	
	void compile_bin(const ExprPtr& e);
	void compile_comp_bin(const ExprPtr& e);
	void compile_op_assign(const ExprPtr& e);
	void compile_incdec(const ExprPtr& e);
	void compile_loop_control_statement(const ExprPtr& e);
	void compile_class(const ExprPtr& e);
	void compile_fun(const ExprPtr& e);
	void compile_for(const ExprPtr& e);

	AnyPtr do_bin(const ExprPtr& e, const InternedStringPtr& name, bool swap = false);
	AnyPtr do_not(const AnyPtr& e);
	AnyPtr do_expr(const AnyPtr& e);
	AnyPtr do_send(const AnyPtr& a, const InternedStringPtr& name);
	AnyPtr do_send(const AnyPtr& a, const InternedStringPtr& name, const AnyPtr& b);

	AnyPtr do_bin_static(const ExprPtr& e, const InternedStringPtr& name, bool swap = false);
	AnyPtr do_expr_static(const AnyPtr& p);	

	void put_inst2(const Inst& t, uint_t sz);

	template<class T>
	void put_inst(const T& t){
		put_inst2(t, sizeof(t));
	}
	
	int_t code_size();
			
	struct FunFrame{

		int_t var_frame_count;
		int_t fun_core_num;

		struct Label{
			int_t pos;
			struct From{
				int_t lineno;
				int_t pos;
				int_t set_pos;
			};
			AC<From>::vector froms;
		};
		
		AC<Label>::vector labels;
		
		struct Loop{
			InternedStringPtr label; // ラベル名
			int_t frame_count; // フレームの数
			int_t control_statement_label[2]; // breakとcontinueのラベル番号
			bool have_label; // 対応するラベルを持っているか
		};
		
		Stack<Loop> loops;

		struct Finally{
			int_t frame_count;
			int_t finally_label;
		};

		PODStack<Finally> finallies;

		bool extendable_param;
	};

	struct VarFrame{
		struct Entry{
			InternedStringPtr name;
			ExprPtr expr;
			AnyPtr value;
			bool constant;
			bool initialized;
			bool referenced;
			bool assigned;
			bool removed;
			int_t accessibility;
		};

		AC<Entry>::vector entries;
		int_t real_entry_num;
		
		struct Direct{
			int_t pos;
		};

		AC<Direct>::vector directs;
		int_t block_core_num;
		int_t fun_frames_size;

		enum{
			SCOPE,
			CLASS,
			BLOCK		
		};

		int_t kind;
		bool scope_chain;
	};

	struct ClassFrame{
		struct Entry{
			InternedStringPtr name;
		};

		AC<Entry>::vector entries;
		int_t class_core_num;
	};

	struct LVarInfo{
		VarFrame* var_frame;
		VarFrame::Entry* entry;
		int_t pos;
	};

	void scope_chain(int_t var_frame_size);
	LVarInfo var_find(const InternedStringPtr& key, bool define = false, bool traceless = false);
	void var_begin(int_t kind);
	void var_define(const ArrayPtr& stmts);
	void var_define(const InternedStringPtr& name, const ExprPtr& expr = null, int_t accessibility = 0, bool define = false, bool constant = false, bool assign = false);
	void var_set_direct(VarFrame& vf);
	void var_set_on_heap(int_t i=0);
	void var_end();
	//int_t erase_not_referenced_lvar_code();

	void check_lvar_assign(const ExprPtr& e);
	void check_lvar_assign_stmt(const AnyPtr& p);

	void block_begin();
	void block_end();
	
	int_t lineno(){ return linenos_.top(); }

	int_t class_core_num(){
		return class_frames_.empty() ? 0 : class_frames_.top().class_core_num;
	}

	int_t register_identifier(const InternedStringPtr& v){
		if(const AnyPtr& pos = identifier_map_->at(v)){ return pos->to_i(); }
		result_->identifier_table_->push_back(v);
		identifier_map_->set_at(v, result_->identifier_table_->size()-1);
		return result_->identifier_table_->size()-1;
	}

	int_t register_value(const AnyPtr& v){
		if(const AnyPtr& pos = value_map_->at(v)){ return pos->to_i(); }
		result_->value_table_->push_back(v);
		value_map_->set_at(v, result_->value_table_->size()-1);
		return result_->value_table_->size()-1;
	}

	int_t append_identifier(const InternedStringPtr& identifier){
		result_->identifier_table_->push_back(identifier);
		return result_->identifier_table_->size()-1;
	}

	int_t append_value(const AnyPtr& v){
		result_->value_table_->push_back(v);
		return result_->value_table_->size()-1;
	}

	/*
	void erase_dead_code(int_t pos, int_t size){
		std::sort(addresses_.begin(), addresses_.end());
		for(int_t i=0, sz=addresses_.size(); i<sz; ++i){
			int_t address = addresses_[i];
			inst_address_t& ref = *(inst_address_t*)&result_->code_[address];

			// アドレスが書いてある所が削除対象
			if(pos<=address && pos+size>address){
				// アドレスリストから削除する
				addresses_.erase(i);
			}
			// 相対アドレスを縮める処理
			else if(ref<0){
				if(address > pos+size && address+ref < pos+size){
					if(address+ref > pos){
						ref = ref + (size - (address+ref - pos));
					}else{
						ref = ref + (size);
					}
				}
			}else{
				if(address < pos+size && address+ref > pos+size){
					if(address > pos){
						ref = ref - (size - (address - pos));
					}else{
						ref = ref - (size);
					}
				}
			}

			if(result_->)
		}

		result_->code_.erase(result_->code_.begin()+pos, result_->code_.begin()+pos+size);
	}
	*/

private:
	MapPtr value_map_;
	MapPtr identifier_map_;

	CompileError errorimpl_;
	CompileError* error_;
	
	Parser parser_;
	CodePtr result_;
	
	Stack<FunFrame> fun_frames_;
	Stack<VarFrame> var_frames_;
	Stack<ClassFrame> class_frames_;

	FunFrame& ff(){ return fun_frames_.top(); }
	VarFrame& vf(){ return var_frames_.top(); }
	ClassFrame& cf(){ return class_frames_.top(); }

	PODStack<int_t> label_names_;
	PODStack<int_t> linenos_;
	//PODStack<int_t> addresses_;

private:
	CodeBuilder(const CodeBuilder&);
	CodeBuilder& operator=(const CodeBuilder&);
};

}

#endif

