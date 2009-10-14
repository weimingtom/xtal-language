/** \file src/xtal/xtal_codebuilder.h
* \brief src/xtal/xtal_codebuilder.h
*/

#ifndef XTAL_CODEBUILDER_H_INCLUDE_GUARD
#define XTAL_CODEBUILDER_H_INCLUDE_GUARD

#pragma once

#include "xtal_parser.h"

#ifndef XTAL_NO_PARSER

namespace xtal{

class CodeBuilder{
public:

	CodeBuilder();

	~CodeBuilder();
	
	CodePtr compile(const StreamPtr& stream, const StringPtr& source_file_name = XTAL_STRING("anonymous"));

	void interactive_compile(const StreamPtr& stream);

	CodePtr eval_compile(const StringPtr& code);

	AnyPtr errors();

public:

	CodePtr compile_toplevel(const ExprPtr& e, const StringPtr& source_file_name);

	void adjust_values(int_t need_result_count, int_t result_count);

	inst_address_t calc_address(const inst_t* pc, inst_address_t address);
	void opt_jump();

	int_t reserve_label();
	void set_label(int_t lableno);
	void set_jump(int_t offset, int_t labelno);
	void process_labels();

	void put_member_code(const AnyPtr& var, bool q, const ExprPtr& secondary_key);
	void put_define_member_code(const AnyPtr& var, const ExprPtr& secondary_key);

	int_t lookup_instance_variable(const IDPtr& key);
	void put_set_instance_variable_code(int_t value, const IDPtr& var);
	void put_instance_variable_code(int_t result, const IDPtr& var);
	void put_val_code(const AnyPtr& val);
	void put_if_code(const ExprPtr& cond, int_t label_false, int_t stack_top);
	void break_off(int_t to);

	int_t make_result_number(int_t stack_top, int_t result, int_t result_count);
	int_t compile_bin2(const ExprPtr& e, int_t stack_top, int_t result);

	int_t compile_member(const AnyPtr& eterm, const AnyPtr& eprimary, const AnyPtr& esecondary, int_t flags, int_t stack_top, int_t result);

	bool compile_expr(const AnyPtr& p, int_t stack_top, int_t result, int_t result_count, AnyPtr& ret);

	int_t compile_expr(const AnyPtr& p, int_t stack_top, int_t result, int_t result_count = 1);
	int_t compile_expr(const AnyPtr& p, int_t& stack_top);

	void compile_stmt(const AnyPtr& p);	

	void compile_bin(const ExprPtr& e);
	void compile_comp_bin(const ExprPtr& e);
	int_t compile_comp_bin2(const ExprPtr& e, int_t stack_top, int_t result);

	int_t compile_send(const AnyPtr& eterm, const AnyPtr& eprimary, const AnyPtr& esecondary, const ExprPtr& args, const ExprPtr& eargs, int_t flags, int_t stack_top, int_t result, int_t result_count);

	int_t compile_property(const AnyPtr& eterm, const AnyPtr& eprimary, const AnyPtr& esecondary, int_t flags, int_t stack_top, int_t result, int_t result_count = 1);
	int_t compile_set_property(const AnyPtr& eterm, const AnyPtr& eprimary, const AnyPtr& esecondary, const AnyPtr& set, int_t flags, int_t stack_top, int_t result, int_t result_count = 1);

	int_t compile_call(int_t target, int_t self, const ExprPtr& args, const ExprPtr& eargs, int_t flags, int_t stack_top, int_t result, int_t result_count);
	void compile_lassign(int_t target, const IDPtr& var);
	void put_if_code(int_t tag, int_t target, int_t lhs, int_t rhs, inst_t label_true, int_t label_false, int_t stack_top);
	void put_bin(int_t result, const ExprPtr& e, int_t a, int_t b, int_t stack_top);

	void compile_comp_bin_assert(const AnyPtr& f, const ExprPtr& e, const ExprPtr& str, const ExprPtr& mes, int_t label, int_t stack_top);
	void compile_op_assign(const ExprPtr& e, int_t stack_top);
	void put_incdec(const ExprPtr& e, int_t ret, int_t target, int_t stack_top);
	void compile_incdec(const ExprPtr& e, int_t stack_top);
	void compile_loop_control_statement(const ExprPtr& e);
	void compile_class(const ExprPtr& e, int_t stack_top, int_t result, const IDPtr& id = empty_id);
	int_t compile_fun(const ExprPtr& e, int_t stack_top, int_t result, const IDPtr& id = empty_id);

	int_t compile_e(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count);

	bool is_comp_bin(const ExprPtr& e);

	AnyPtr do_expr(const AnyPtr& e);

	void put_inst2(const Inst& t, uint_t sz);

	template<class T>
	void put_inst(const T& t){
		put_inst2(t, sizeof(t));
	}
	
	int_t code_size();
			
	struct FunFrame{
		int_t stack_count;
		int_t max_stack_count;

		int_t variable_count;
		int_t max_variable_count;

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
			int_t accessibility;
			int_t number;
		};

		ArrayList<Entry> entries;
		int_t real_entry_num;
		
		int_t scope_info_num;
		uint_t fun_frames_size;

		enum{
			CLASS,
			FRAME,
			FUN,
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
		int_t var_frame;
		int_t entry;
		int_t depth;
		int_t pos;
		int_t vpos;
		bool out_of_fun;
	};

	VarFrame& var_frame(const LVarInfo& vi){
		return var_frames_.reverse_at(vi.var_frame);
	}

	VarFrame::Entry& entry(const LVarInfo& vi){
		return var_frames_.reverse_at(vi.var_frame).entries[vi.entry];
	}

	LVarInfo var_find(const IDPtr& key, bool define = false, bool traceless = false, int_t number = -1);
	void var_begin(int_t kind);
	void var_define_stmts(const ExprPtr& stmts);
	void var_define_stmt(const AnyPtr& stmt);
	void var_define(const IDPtr& name, const ExprPtr& expr = null, int_t accessibility = 0, bool define = false, bool constant = false, bool assign = false, int_t number = -1);
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

	int_t register_identifier_or_compile_expr(const AnyPtr& var);
	int_t register_identifier(const IDPtr& v);
	int_t register_value(const AnyPtr& v);
	int_t append_identifier(const IDPtr& identifier);
	int_t append_value(const AnyPtr& v);

	ExprPtr setup_expr(const ExprPtr& e);

private:
	MapPtr value_map_;
	MapPtr identifier_map_;

	MapPtr implicit_ref_map_;

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

	int_t prev_inst_op_;

public:

//{STMT_DECLS{{
	int_t compile_expr_LIST(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count);
	int_t compile_expr_NULL(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count);
	int_t compile_expr_UNDEFINED(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count);
	int_t compile_expr_TRUE(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count);
	int_t compile_expr_FALSE(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count);
	int_t compile_expr_CALLEE(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count);
	int_t compile_expr_THIS(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count);
	int_t compile_expr_NUMBER(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count);
	int_t compile_expr_STRING(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count);
	int_t compile_expr_ARRAY(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count);
	int_t compile_expr_MAP(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count);
	int_t compile_expr_VALUES(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count);
	int_t compile_expr_ADD(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count);
	int_t compile_expr_SUB(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count);
	int_t compile_expr_CAT(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count);
	int_t compile_expr_MUL(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count);
	int_t compile_expr_DIV(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count);
	int_t compile_expr_MOD(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count);
	int_t compile_expr_AND(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count);
	int_t compile_expr_OR(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count);
	int_t compile_expr_XOR(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count);
	int_t compile_expr_SHL(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count);
	int_t compile_expr_SHR(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count);
	int_t compile_expr_USHR(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count);
	int_t compile_expr_EQ(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count);
	int_t compile_expr_NE(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count);
	int_t compile_expr_LT(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count);
	int_t compile_expr_LE(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count);
	int_t compile_expr_GT(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count);
	int_t compile_expr_GE(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count);
	int_t compile_expr_RAWEQ(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count);
	int_t compile_expr_RAWNE(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count);
	int_t compile_expr_IN(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count);
	int_t compile_expr_NIN(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count);
	int_t compile_expr_IS(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count);
	int_t compile_expr_NIS(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count);
	int_t compile_expr_ANDAND(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count);
	int_t compile_expr_OROR(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count);
	int_t compile_expr_CATCH(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count);
	int_t compile_expr_POS(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count);
	int_t compile_expr_NEG(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count);
	int_t compile_expr_COM(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count);
	int_t compile_expr_NOT(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count);
	int_t compile_expr_RANGE(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count);
	int_t compile_expr_FUN(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count);
	int_t compile_expr_CLASS(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count);
	int_t compile_expr_ONCE(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count);
	int_t compile_expr_IVAR(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count);
	int_t compile_expr_LVAR(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count);
	int_t compile_expr_AT(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count);
	int_t compile_expr_Q(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count);
	int_t compile_expr_MEMBER(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count);
	int_t compile_expr_MEMBER_Q(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count);
	int_t compile_expr_PROPERTY(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count);
	int_t compile_expr_PROPERTY_Q(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count);
	int_t compile_expr_CALL(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count);
	int_t compile_expr_INC(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count);
	int_t compile_expr_DEC(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count);
	int_t compile_expr_ADD_ASSIGN(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count);
	int_t compile_expr_SUB_ASSIGN(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count);
	int_t compile_expr_CAT_ASSIGN(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count);
	int_t compile_expr_MUL_ASSIGN(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count);
	int_t compile_expr_DIV_ASSIGN(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count);
	int_t compile_expr_MOD_ASSIGN(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count);
	int_t compile_expr_AND_ASSIGN(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count);
	int_t compile_expr_OR_ASSIGN(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count);
	int_t compile_expr_XOR_ASSIGN(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count);
	int_t compile_expr_SHL_ASSIGN(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count);
	int_t compile_expr_SHR_ASSIGN(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count);
	int_t compile_expr_USHR_ASSIGN(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count);
	int_t compile_expr_RETURN(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count);
	int_t compile_expr_YIELD(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count);
	int_t compile_expr_ASSERT(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count);
	int_t compile_expr_THROW(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count);
	int_t compile_expr_TRY(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count);
	int_t compile_expr_IF(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count);
	int_t compile_expr_FOR(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count);
	int_t compile_expr_MASSIGN(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count);
	int_t compile_expr_MDEFINE(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count);
	int_t compile_expr_ASSIGN(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count);
	int_t compile_expr_DEFINE(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count);
	int_t compile_expr_CDEFINE_MEMBER(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count);
	int_t compile_expr_CDEFINE_IVAR(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count);
	int_t compile_expr_BREAK(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count);
	int_t compile_expr_CONTINUE(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count);
	int_t compile_expr_BRACKET(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count);
	int_t compile_expr_SCOPE(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count);
	int_t compile_expr_SWITCH(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count);
	int_t compile_expr_TOPLEVEL(const ExprPtr& e, int_t stack_top, int_t result, int_t result_count);
//}}STMT_DECLS}

private:
	XTAL_DISALLOW_COPY_AND_ASSIGN(CodeBuilder);
};

}

#endif


#endif // XTAL_CODEBUILDER_H_INCLUDE_GUARD
