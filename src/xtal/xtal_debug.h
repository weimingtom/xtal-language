#ifndef XTAL_DEBUG_H_INCLUDE_GUARD
#define XTAL_DEBUG_H_INCLUDE_GUARD

#pragma once

namespace xtal{

class DebugInfo : public Base{
public:

	int_t kind(){ return kind_; } 
	int_t line(){ return line_; }
	const StringPtr& file_name(){ return file_name_; }
	const StringPtr& fun_name(){ return fun_name_; }
	const StringPtr& assertion_message(){ return assertion_message_; }
	const AnyPtr& exception(){ return exception_; }
	const FramePtr& local_variables_frame(){ return local_variables_frame_; }

	void set_kind(int_t v){ kind_ = v; }
	void set_line(int_t v){ line_ = v; }
	void set_file_name(const StringPtr& v){ file_name_ = v; }
	void set_fun_name(const StringPtr& v){ fun_name_ = v; }
	void set_assertion_message(const StringPtr& v){ assertion_message_ = v; }
	void set_exception(const AnyPtr& e){ exception_ = e; }
	void set_local_variables_frame(const FramePtr& v){ local_variables_frame_ = v; }

	SmartPtr<DebugInfo> clone(){
		return xnew<DebugInfo>(*this);
	}

private:

	void visit_members(Visitor& m);

	int_t kind_;
	int_t line_;
	StringPtr file_name_;
	StringPtr fun_name_;
	StringPtr assertion_message_;
	AnyPtr exception_;
	FramePtr local_variables_frame_;
};

typedef SmartPtr<DebugInfo> DebugInfoPtr;

class Debug : public CppSingleton{
public:

	Debug(){
		enable_count_ = 0;
		hook_setting_bit_ = 0;
	}

	/**
	* @brief デバッグ機能を有効にする
	* デバッグ機能はデフォルトでは無効になっている。
	*/
	void enable();

	/**
	* @brief デバッグ機能を無効にする
	*/
	void disable();

	/**
	* @brief デバッグ機能を強制的に有効にする
	* デバッグ機能はデフォルトでは無効になっている。
	*/
	void enable_force(int_t count){
		enable_count_ = count;
	}

	/**
	* @brief デバッグ機能を強制的に無効にする
	*/
	int_t disable_force(){
		int_t temp = enable_count_;
		enable_count_ = 0;
		return temp;
	}

	/**
	* @brief デバッグ機能が有効かどうか
	*/
	bool is_enabled();

	uint_t hook_setting_bit(){
		return hook_setting_bit_;
	}

	/**
	* @brief ブレークポイントがある度に呼び出されるフック関数を登録する
	*/
	void set_break_point_hook(const AnyPtr& hook);

	/**
	* @brief 関数呼び出しされる度に呼び出されるフック関数を登録する
	*/
	void set_call_hook(const AnyPtr& hook);

	/**
	* @brief 関数からreturnされる度に呼び出されるフック関数を登録する
	*/
	void set_return_hook(const AnyPtr& hook);

	/**
	* @brief 例外かthrowされる度に呼び出されるフック関数を登録する
	*/
	void set_throw_hook(const AnyPtr& hook);

	/**
	* @brief アサートが失敗される度に呼び出されるフック関数を登録する
	*/
	void set_assert_hook(const AnyPtr& hook);

	/**
	* @brief set_break_point_hook関数で登録した関数を取得する
	*/
	const AnyPtr& break_point_hook();

	/**
	* @brief set_call_hook関数で登録した関数を取得する
	*/
	const AnyPtr& call_hook();

	/**
	* @brief set_return_hook関数で登録した関数を取得する
	*/
	const AnyPtr& return_hook();

	/**
	* @brief set_throw_hook関数で登録した関数を取得する
	*/
	const AnyPtr& throw_hook();

	/**
	* @brief set_assert_hook関数で登録した関数を取得する
	*/
	const AnyPtr& assert_hook();

private:

	int_t enable_count_;
	uint_t hook_setting_bit_;
	AnyPtr break_point_hook_;
	AnyPtr call_hook_;
	AnyPtr return_hook_;
	AnyPtr throw_hook_;
	AnyPtr assert_hook_;

public:
	void visit_members(Visitor& m){
		CppSingleton::visit_members(m);
		m & break_point_hook_ & call_hook_ & return_hook_ & throw_hook_ & assert_hook_;
	}
};

}

#endif // XTAL_DEBUG_H_INCLUDE_GUARD
