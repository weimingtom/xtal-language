/** \file src/xtal/xtal_code.h
* \brief src/xtal/xtal_code.h
*/

#ifndef XTAL_CODE_H_INCLUDE_GUARD
#define XTAL_CODE_H_INCLUDE_GUARD

#pragma once

namespace xtal{

/**
* \brief コンパイルされたバイトコード
*/
class Code : public Class{
public:
	
	Code();

	~Code();

public:

	/**
	* \brief コードに対応したソース行数を返す。
	*/
	int_t compliant_lineno(const inst_t* p);

	/**
	* \brief ソース行数に対応したコード位置を返す。
	*/
	const inst_t* compliant_pc(int_t p);
		
	bool set_lineno_info(uint_t line);

	int_t final_lineno();

	const inst_t* data(){
		return &code_[0];
	}

	int_t size(){
		return (int_t)code_.size();
	}

	/**
	* \brief シンボルテーブルからi番目のシンボルを取り出す。
	*/
	const IDPtr& identifier(int_t i){
		XTAL_ASSERT(i<(int_t)identifier_table_->size());
		return unchecked_ptr_cast<ID>(identifier_table_->at(i));
	}

	const IDPtr* identifier_data(){
		return (IDPtr*)identifier_table_->data();
	}

	uint_t identifier_size(){
		return identifier_table_->size();
	}

	/**
	* \brief 値テーブルからi番目の値を取り出す。
	*/
	const AnyPtr& value(int_t i){
		XTAL_ASSERT(i<(int_t)value_table_->size());
		return value_table_->at(i);
	}

	/**
	* \brief onceテーブルからi番目の値を取り出す。
	*/
	const AnyPtr& once_value(int_t i){
		XTAL_ASSERT(i<(int_t)once_table_->size());
		return once_table_->at(i);
	}

	/**
	* \brief onceテーブルのi番目に値を設定する。
	*/
	void set_once_value(int_t i, const AnyPtr& v){
		XTAL_ASSERT(i<(int_t)once_table_->size());
		once_table_->set_at(i, v);
	}

	const StringPtr& source_file_name(){ 
		return source_file_name_; 
	}

	void set_source_file_name(const StringPtr& file_name){
		source_file_name_ = file_name;
	}

	const ClassPtr& filelocal(){ 
		return to_smartptr(static_cast<Class*>(this)); 
	}

	ScopeInfo* scope_info(int_t i){
		return &scope_info_table_[i];
	}

	ClassInfo* class_info(int_t i){
		return &class_info_table_[i];
	}

	FunInfo* fun_info(int_t i){
		return &xfun_info_table_[i];
	}	

	ExceptInfo* except_info(int_t i){
		return &except_info_table_[i];
	}

	MethodPtr first_fun(){
		return first_fun_;
	}

	StringPtr inspect();

	StringPtr inspect_range(int_t start, int_t end);

	void on_rawcall(const VMachinePtr& vm);

	void check_implicit_lookup();

	IDPtr find_near_variable(const IDPtr& primary_key);

	void set_breakpoint(int_t lineno, bool set = true){
		for(uint_t i=0, sz=lineno_table_.size(); i<sz; ++i){
			LineNumberInfo& info = lineno_table_[i];
			if(info.lineno==lineno){
				if(set){
					if(!info.breakpoint){
						info.op = code_[info.start_pc];
						code_[info.start_pc] = InstBreakPoint::NUMBER;
					}
				}
				else{
					if(info.breakpoint){
						code_[info.start_pc] = info.op;
					}
				}
				info.breakpoint = (int)set;
			}
		}
	}

	inst_t original_op(const inst_t* pc){
		if(LineNumberInfo* lni = compliant_lineno_info(pc)){
			return lni->op;
		}
		return 0;
	}

	void on_visit_members(Visitor& m){
		Class::on_visit_members(m);
		m & identifier_table_ & value_table_ & source_file_name_ & first_fun_ & once_table_;
	}

	void generated();

protected:

	void find_near_variable_inner(const IDPtr& primary_key, const ScopeInfo& info, IDPtr& pick, int_t& minv);

private:

	friend class CodeBuilder;
	friend class VMachine;
	friend class Serializer;

	typedef PODArrayList<inst_t> code_t;
	code_t code_;

	ArrayPtr identifier_table_;
	ArrayPtr value_table_;
	ArrayPtr once_table_;
	StringPtr source_file_name_;
	MethodPtr first_fun_;

private:

	PODArrayList<FunInfo> xfun_info_table_;
	PODArrayList<ScopeInfo> scope_info_table_;
	PODArrayList<ClassInfo> class_info_table_;
	PODArrayList<ExceptInfo> except_info_table_;

	struct LineNumberInfo{
		u32 start_pc;
		u16 lineno;
		inst_t op;
		u8 breakpoint;
	};

	struct LineNumberCmp{
		bool operator ()(const LineNumberInfo& lnt, uint_t pc){
			return lnt.start_pc<pc;
		}
		bool operator ()(uint_t pc, const LineNumberInfo& lnt){
			return pc<lnt.start_pc;
		}
		bool operator ()(const LineNumberInfo& l, const LineNumberInfo& r){
			return l.start_pc<r.start_pc;
		}
	};

	PODArrayList<LineNumberInfo> lineno_table_;

	LineNumberInfo* compliant_lineno_info(const inst_t* p);

	struct ImplcitInfo{
		u16 id;
		u16 lineno;
	};

	PODArrayList<ImplcitInfo> implicit_table_;
};

}

#endif // XTAL_CODE_H_INCLUDE_GUARD
