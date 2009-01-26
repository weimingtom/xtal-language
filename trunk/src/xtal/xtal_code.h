
#pragma once

namespace xtal{

class Code : public Base{
public:
	
	Code();

	/**
	* @brief コードに対応したソース行数を返す。
	*/
	int_t compliant_lineno(const inst_t* p);
		
	void set_lineno_info(int_t line);

	const inst_t* data(){
		return &code_[0];
	}

	int_t size(){
		return (int_t)code_.size();
	}

	/**
	* @brief シンボルテーブルからi番目のシンボルを取り出す。
	*/
	const IDPtr& identifier(int_t i){
		XTAL_ASSERT(i<(int_t)identifier_table_->size());
		return static_ptr_cast<ID>(identifier_table_->at(i));
	}

	/**
	* @brief 値テーブルからi番目の値を取り出す。
	*/
	const AnyPtr& value(int_t i){
		XTAL_ASSERT(i<(int_t)value_table_->size());
		return value_table_->at(i);
	}

	/**
	* @brief onceテーブルからi番目の値を取り出す。
	*/
	const AnyPtr& once_value(int_t i){
		XTAL_ASSERT(i<(int_t)once_table_->size());
		return once_table_->at(i);
	}

	/**
	* @brief onceテーブルのi番目に値を設定する。
	*/
	void set_once_value(int_t i, const AnyPtr& v){
		XTAL_ASSERT(i<(int_t)once_table_->size());
		once_table_->set_at(i, v);
	}

	const StringPtr& source_file_name(){ 
		return source_file_name_; 
	}

	const ClassPtr& filelocal(){ 
		return filelocal_; 
	}

	ScopeCore* scope_core(int_t i){
		return &scope_core_table_[i];
	}

	ClassCore* class_core(int_t i){
		return &class_core_table_[i];
	}

	FunCore* fun_core(int_t i){
		return &xfun_core_table_[i];
	}	

	ExceptCore* except_core(int_t i){
		return &except_core_table_[i];
	}

	FunPtr first_fun(){
		return first_fun_;
	}

	StringPtr inspect();
	StringPtr inspect_range(int_t start, int_t end);

	virtual void rawcall(const VMachinePtr& vm);

	void insert_code(inst_t* p, inst_t* code, int_t size);

	void erase_code(inst_t* p, int_t size);

	bool add_break_point(int_t lineno);

	void remove_break_point(int_t lineno);

protected:

	void insert_erase_common(inst_t* p, int_t size);

	virtual void visit_members(Visitor& m){
		Base::visit_members(m);
		m & identifier_table_ & value_table_ & filelocal_ & source_file_name_ & first_fun_ & once_table_;
	}

private:

	friend class CodeBuilder;
	friend class VMachine;
	friend class Serializer;

	typedef AC<inst_t>::vector code_t;
	code_t code_;
	ArrayPtr identifier_table_;
	ArrayPtr value_table_;
	ArrayPtr once_table_;
	ClassPtr filelocal_;
	StringPtr source_file_name_;
	FunPtr first_fun_;


private:

	AC<FunCore>::vector xfun_core_table_;
	AC<ScopeCore>::vector scope_core_table_;
	AC<ClassCore>::vector class_core_table_;
	AC<ExceptCore>::vector except_core_table_;

	struct LineNumberTable{
		u32 start_pc;
		u32 lineno;
	};

	struct LineNumberCmp{
		bool operator ()(const LineNumberTable& lnt, uint_t pc){
			return lnt.start_pc<pc;
		}
		bool operator ()(uint_t pc, const LineNumberTable& lnt){
			return pc<lnt.start_pc;
		}
		bool operator ()(const LineNumberTable& l, const LineNumberTable& r){
			return l.start_pc<r.start_pc;
		}
	};

	AC<LineNumberTable>::vector lineno_table_;

	struct AddressJump{
		u32 pos;
	};

	AC<AddressJump>::vector address_jump_table_;
};

}
