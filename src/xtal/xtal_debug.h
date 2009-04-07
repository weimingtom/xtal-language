/** \file src/xtal/xtal_debug.h
* \brief src/xtal/xtal_debug.h
*/

#ifndef XTAL_DEBUG_H_INCLUDE_GUARD
#define XTAL_DEBUG_H_INCLUDE_GUARD

#pragma once

namespace xtal{

/**
* \brief デバッグフック関数で渡されるオブジェクト
*/
class DebugInfo : public Base{
public:
	
	/**
	* \brief フックの種類を返す
	*/
	int_t kind(){ return kind_; } 

	/**
	* \brief フックされた場所の行数を返す
	*/
	int_t line(){ return line_; }

	/**
	* \brief フックされた場所のファイル名を返す
	*/
	const StringPtr& file_name(){ return file_name_; }

	/**
	* \brief フックされた場所のファイル名を返す
	*/
	const StringPtr& fun_name(){ return fun_name_; }

	/**
	* \brief assertionのメッセージを返す
	*/
	const StringPtr& assertion_message(){ return assertion_message_; }

	/**
	* \brief 例外オブジェクトを返す
	*/
	const AnyPtr& exception(){ return exception_; }

	/**
	* \brief フックされた場所の変数フレームオブジェクトを返す
	*/
	const FramePtr& variables_frame(){ return variables_frame_; }

	void set_kind(int_t v){ kind_ = v; }
	void set_line(int_t v){ line_ = v; }
	void set_file_name(const StringPtr& v){ file_name_ = v; }
	void set_fun_name(const StringPtr& v){ fun_name_ = v; }
	void set_assertion_message(const StringPtr& v){ assertion_message_ = v; }
	void set_exception(const AnyPtr& e){ exception_ = e; }
	void set_variables_frame(const FramePtr& v){ variables_frame_ = v; }

	/**
	* \brief クローンを作る
	* DebugInfoオブジェクトは使いまわされるため、情報を保持したい場合、クローンを作る必要がある。
	*/
	SmartPtr<DebugInfo> clone(){
		return xnew<DebugInfo>(*this);
	}

private:

	virtual void visit_members(Visitor& m);

	int_t kind_;
	int_t line_;
	StringPtr file_name_;
	StringPtr fun_name_;
	StringPtr assertion_message_;
	AnyPtr exception_;
	FramePtr variables_frame_;
	VMachinePtr vm_;
};

typedef SmartPtr<DebugInfo> DebugInfoPtr;

/**
* \brief デバッグ
*/
namespace debug{

class Debug;

/**
* \brief デバッグ機能を有効にする
* デバッグ機能はデフォルトでは無効になっている。
*/
void enable();

/**
* \brief デバッグ機能を無効にする
*/
void disable();

/**
* \brief デバッグ機能を強制的に有効にする
* デバッグ機能はデフォルトでは無効になっている。
*/
void enable_force(int_t count);

/**
* \brief デバッグ機能を強制的に無効にする
*/
int_t disable_force();

/**
* \brief デバッグ機能が有効かどうか
*/
bool is_enabled();

uint_t hook_setting_bit();

/**
* \brief ブレークポイントがある度に呼び出されるフック関数を登録する
*
* \param hook 登録するフック関数
*/
void set_break_point_hook(const AnyPtr& hook);

/**
* \brief 関数呼び出しされる度に呼び出されるフック関数を登録する
*
* \param hook 登録するフック関数
*/
void set_call_hook(const AnyPtr& hook);

/**
* \brief 関数からreturnされる度に呼び出されるフック関数を登録する
*
* \param hook 登録するフック関数
*/
void set_return_hook(const AnyPtr& hook);

/**
* \brief 例外かthrowされる度に呼び出されるフック関数を登録する
*
* \param hook 登録するフック関数
*/
void set_throw_hook(const AnyPtr& hook);

/**
* \brief アサートが失敗される度に呼び出されるフック関数を登録する
*
* \param hook 登録するフック関数
*/
void set_assert_hook(const AnyPtr& hook);

/**
* \brief set_break_point_hook関数で登録した関数を取得する
*/
const AnyPtr& break_point_hook();

/**
* \brief set_call_hook関数で登録した関数を取得する
*/
const AnyPtr& call_hook();

/**
* \brief set_return_hook関数で登録した関数を取得する
*/
const AnyPtr& return_hook();

/**
* \brief set_throw_hook関数で登録した関数を取得する
*/
const AnyPtr& throw_hook();

/**
* \brief set_assert_hook関数で登録した関数を取得する
*/
const AnyPtr& assert_hook();

};

}

#endif // XTAL_DEBUG_H_INCLUDE_GUARD
