
#pragma once

namespace xtal{ namespace debug{

class Info : public Base{
public:

	int_t kind(){ return kind_; } 
	int_t line(){ return line_; }
	StringPtr file_name(){ return file_name_; }
	StringPtr fun_name(){ return fun_name_; }
	FramePtr local_variables(){ return local_variables_; }

	void set_kind(int_t v){ kind_ = v; }
	void set_line(int_t v){ line_ = v; }
	void set_file_name(const StringPtr& v){ file_name_ = v; }
	void set_fun_name(const StringPtr& v){ fun_name_ = v; }
	void set_local_variables(const FramePtr& v){ local_variables_ = v; }

	SmartPtr<Info> clone(){
		return xnew<Info>(*this);
	}

private:

	void visit_members(Visitor& m);

	int_t kind_;
	int_t line_;
	StringPtr file_name_;
	StringPtr fun_name_;
	FramePtr local_variables_;
};


/**
* @brief デバッグ機能を有効にする
* デバッグ機能を有効化した状態でスクリプトをコンパイルするとデバッグ機能が埋め込まれる。
* デバッグ機能はデフォルトでは無効になっている。
*/
void enable();

/**
* @brief デバッグ機能を無効にする
*/
void disable();

/**
* @brief デバッグ機能が有効かどうか
*/
bool is_enabled();

/**
* @brief 行が実行される度に呼び出されるフック関数を登録する
*/
void set_line_hook(const AnyPtr& hook);

/**
* @brief 関数呼び出しされる度に呼び出されるフック関数を登録する
*/
void set_call_hook(const AnyPtr& hook);

/**
* @brief 関数からreturnされる度に呼び出されるフック関数を登録する
*/
void set_return_hook(const AnyPtr& hook);

/**
* @brief set_line_hook関数で登録した関数を取得する
*/
AnyPtr line_hook();

/**
* @brief set_call_hook関数で登録した関数を取得する
*/
AnyPtr call_hook();

/**
* @brief set_return_hook関数で登録した関数を取得する
*/
AnyPtr return_hook();

}}
