
#pragma once

#include "xtal_allocator.h"
#include "xtal_stack.h"
#include "xtal_string.h"
#include "xtal_constant.h"

namespace xtal{

void InitPrototypes();

struct FrameCore{
	FrameCore()
		:kind(0),
		variable_symbol_offset(0), variable_size(0),
		instance_variable_symbol_offset(0), instance_variable_size(0),
		line_number(0){}
	u8 kind;
	u16 variable_symbol_offset;
	u16 variable_size;
	u16 instance_variable_symbol_offset;
	u16 instance_variable_size;
	u16 line_number;
};

struct FunCore : public FrameCore{
	FunCore()
		:pc(0), max_stack(256), min_param_count(0), max_param_count(0), used_args_object(0){}
	u16 pc;
	u16 max_stack;
	u8 min_param_count;
	u8 max_param_count;
	u8 used_args_object;
};

// 例外を処理するためのフレーム
struct ExceptCore{
	ExceptCore(u16 catch_pc = 0, u16 finally_pc = 0, u16 end_pc = 0)
		:catch_pc(catch_pc), finally_pc(finally_pc), end_pc(end_pc){}
	u16 catch_pc;
	u16 finally_pc;
	u16 end_pc;
};

extern FrameCore empty_frame_core;
extern FunCore empty_fun_core;
extern ExceptCore empty_except_core;

// fwd decl
class CodeImpl;

/**
* @brief Xtalのバイトコードを表す
*/
class Code : public Any{
public:
		
	Code();

	Code(CodeImpl* p)
		:Any((AnyImpl*)p){}

	Code(const Null&)
		:Any(null){}

	/**
	* @brief コードの先頭ポインタを得る。
	*/
	const inst_t* data() const;
			
	/**
	* @brief コードのサイズを得る。
	*/
	int_t size() const;
		
	/**
	* @brief シンボルテーブルからi番目のシンボルを取り出す。
	*/
	const ID& symbol(int_t i) const;
	
	/**
	* @brief 値テーブルからi番目の値を取り出す。
	*/
	const Any& value(int_t i) const;
	
	/**
	* @brief 値テーブルのi番目に値を設定する。
	*/
	void set_value(int_t i, const Any& v) const;

	/**
	* @brief コードに対応したソース行数を返す。
	*/
	int_t compliant_line_number(const inst_t* p) const;

	FrameCore* frame_core(int_t i) const;

	FunCore* fun_core(int_t i) const;

	String source_file_name() const;

	const Class& toplevel() const;

	CodeImpl* impl() const{ return (CodeImpl*)Any::impl(); }
};
	
}
