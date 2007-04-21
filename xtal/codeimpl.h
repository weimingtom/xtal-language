
#pragma once

#include "algorithm"

#include "allocator.h"
#include "stack.h"
#include "string.h"

namespace xtal{

class CodeImpl : public AnyImpl{
public:
	
	CodeImpl();

	int_t compliant_line_number(const u8* p);
		
	void set_line_number_info(int_t line){
		if(!line_number_table_.empty() && line_number_table_.back().line_number==line)
			return;
		LineNumberTable lnt={code_.size(), line};
		line_number_table_.push_back(lnt);
	}

	const u8* data(){
		return &code_[0];
	}

	int_t size(){
		return code_.size();
	}

	const ID& get_symbol(int_t i){
		XTAL_ASSERT(i<(int_t)symbol_table_.size());
		return (const ID&)symbol_table_[i];
	}

	const Any& get_value(int_t i){
		XTAL_ASSERT(i<(int_t)code_.size());
		return value_table_[i];
	}

	void set_value(int_t i, const Any& v){
		XTAL_ASSERT(i<(int_t)value_table_.size());
		value_table_.set_at(i, v);
	}

	const String& source_file_name(){ 
		return source_file_name_; 
	}

	const Class& toplevel() const{ 
		return toplevel_; 
	}
	
public:

	typedef AC<u8>::vector code_t;
	code_t code_;
	Array symbol_table_;
	Array value_table_;
	Class toplevel_;
	String source_file_name_;

protected:

	virtual void visit_members(Visitor& m){
		AnyImpl::visit_members(m);
		m & symbol_table_ & value_table_ & toplevel_ & source_file_name_;
	}

public:

	AC<FunCore>::vector xfun_core_table_;
	AC<FrameCore>::vector frame_core_table_;

	static FunCore empty_xfun_core_;
	static FrameCore empty_frame_core_;

	struct LineNumberTable{
		u16 start_pc;
		u16 line_number;
	};

	struct LineNumberCmp{
		bool operator ()(const LineNumberTable& lnt, int_t pc){
			return lnt.start_pc<pc;
		}
		bool operator ()(int_t pc, const LineNumberTable& lnt){
			return pc<lnt.start_pc;
		}
		bool operator ()(const LineNumberTable& l, const LineNumberTable& r){
			return l.start_pc<r.start_pc;
		}
	};

	AC<LineNumberTable>::vector line_number_table_;
};
	
}
