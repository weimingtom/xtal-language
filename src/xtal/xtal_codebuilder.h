
#pragma once

#ifndef XTAL_NO_PARSER

namespace xtal{

class CodeBuilder{
public:

	CodeBuilder();

	~CodeBuilder();
	
	CodePtr compile(const StreamPtr& stream, const StringPtr& source_file_name = XTAL_STRING("anonymous"));

	void interactive_compile(const StreamPtr& stream);

	AnyPtr errors();

public:

	CodePtr compile_toplevel(const ExprPtr& e, const StringPtr& source_file_name);

	void adjust_result(int_t need_result_count, int_t result_count);

	struct CompileInfo{
		int_t need_result_count;
		bool tail;

		CompileInfo(int_t need_result_count = 1, bool tail = false)
			:need_result_count(need_result_count), tail(tail){}
	};

	bool compile_expr(const AnyPtr& p, const CompileInfo& info, AnyPtr& ret);
	void compile_expr(const AnyPtr& p, const CompileInfo& info = CompileInfo());
	void compile_stmt(const AnyPtr& p);	

	int_t reserve_label();
	void set_label(int_t lableno);
	void set_jump(int_t offset, int_t labelno);
	void process_labels();

	bool put_local_code(const IDPtr& var);
	bool put_set_local_code(const IDPtr& var);
	void put_define_local_code(const IDPtr& var, const ExprPtr& val = null);

	void put_send_code(const AnyPtr& var, int_t need_result_count, bool tail, bool q, const ExprPtr& secondary_key);
	void put_set_send_code(const AnyPtr& var, bool q, const ExprPtr& secondary_key);

	void put_member_code(const AnyPtr& var, bool q, const ExprPtr& secondary_key);
	void put_define_member_code(const AnyPtr& var, const ExprPtr& secondary_key);

	int_t lookup_instance_variable(const IDPtr& key);
	void put_set_instance_variable_code(const IDPtr& var);
	void put_instance_variable_code(const IDPtr& var);
	void put_val_code(const AnyPtr& val);
	void put_if_code(const ExprPtr& cond, int_t label_if, int_t label_if2);
	void break_off(int_t to);
	
	void compile_bin(const ExprPtr& e);
	void compile_comp_bin(const ExprPtr& e);
	void compile_comp_bin_assert(const AnyPtr& f, const ExprPtr& e, const ExprPtr& str, const ExprPtr& mes, int_t label);
	void compile_op_assign(const ExprPtr& e);
	void compile_incdec(const ExprPtr& e);
	void compile_loop_control_statement(const ExprPtr& e);
	void compile_class(const ExprPtr& e);
	void compile_fun(const ExprPtr& e);
	void compile_for(const ExprPtr& e);
	int_t compile_exprs(const ExprPtr& e);

	AnyPtr do_bin(const ExprPtr& e, const IDPtr& name, bool swap = false);
	AnyPtr do_not(const AnyPtr& e);
	AnyPtr do_expr(const AnyPtr& e);
	AnyPtr do_send(const AnyPtr& a, const IDPtr& name);
	AnyPtr do_send(const AnyPtr& a, const IDPtr& name, const AnyPtr& b);

	void put_inst2(const Inst& t, uint_t sz);

	template<class T>
	void put_inst(const T& t){
		put_inst2(t, sizeof(t));
	}
	
	int_t code_size();
			
	struct FunFrame{
		int_t stack_count;
		int_t max_stack_count;

		int_t var_frame_count;
		int_t fun_info_num;

		struct Label{
			int_t pos;
			struct From{
				int_t lineno;
				int_t pos;
				int_t set_pos;
			};
			PODArrayList<From> froms;
		};
		
		ArrayList<Label> labels;
		
		struct Loop{
			IDPtr label; // ラベル名
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
			IDPtr name;
			ExprPtr expr;
			AnyPtr value;
			bool constant;
			bool initialized;
			bool referenced;
			bool assigned;
			bool removed;
			int_t accessibility;
			int_t number;
		};

		ArrayList<Entry> entries;
		int_t real_entry_num;
		
		struct Direct{
			int_t pos;
		};

		PODArrayList<Direct> directs;
		int_t scope_info_num;
		int_t fun_frames_size;

		enum{
			SCOPE,
			CLASS,
			FRAME		
		};

		int_t kind;
		bool scope_chain;
	};

	struct ClassFrame{
		struct Entry{
			IDPtr name;
		};

		ArrayList<Entry> entries;
		int_t class_info_num;
	};

	struct LVarInfo{
		VarFrame* var_frame;
		VarFrame::Entry* entry;
		int_t pos;
	};

	LVarInfo var_find(const IDPtr& key, bool define = false, bool traceless = false, int_t number = -1);
	void var_begin(int_t kind);
	void var_define_stmts(const ExprPtr& stmts);
	void var_define_stmt(const AnyPtr& stmt);
	void var_define(const IDPtr& name, const ExprPtr& expr = null, int_t accessibility = 0, bool define = false, bool constant = false, bool assign = false, int_t number = -1);
	void var_set_direct(VarFrame& vf);
	void var_set_on_heap(int_t i=0);
	void var_end();

	void check_lvar_assign(const ExprPtr& e);
	void check_lvar_assign_stmt(const AnyPtr& p);

	void scope_begin();
	void scope_end();
	void scope_chain(int_t var_frame_size);

	void add_stack_count(int_t i){
		FunFrame& f = fun_frames_.top();
		f.stack_count += i;
		if(f.stack_count>f.max_stack_count){
			f.max_stack_count = f.stack_count;
		}
	}

	int_t lineno(){ return linenos_.top(); }

	int_t class_info_num(){
		return class_frames_.empty() ? 0 : class_frames_.top().class_info_num;
	}

	int_t regster_identifier_or_compile_expr(const AnyPtr& var);
	int_t regster_identifier(const IDPtr& v);
	int_t register_value(const AnyPtr& v);
	int_t append_identifier(const IDPtr& identifier);
	int_t append_value(const AnyPtr& v);

private:
	MapPtr value_map_;
	MapPtr identifier_map_;

	CompileErrors errorimpl_;
	CompileErrors* error_;
	
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

	ExprBuilder eb_;

private:
	XTAL_DISALLOW_COPY_AND_ASSIGN(CodeBuilder);
};

}

#endif

