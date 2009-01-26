
#pragma once

namespace xtal{

class DebugInfo : public Base{
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

	SmartPtr<DebugInfo> clone(){
		return xnew<DebugInfo>(*this);
	}

private:

	void visit_members(Visitor& m);

	int_t kind_;
	int_t line_;
	StringPtr file_name_;
	StringPtr fun_name_;
	FramePtr local_variables_;
};

typedef SmartPtr<DebugInfo> DebugInfoPtr;


class Debug : public CppSingleton{
public:

	Debug(){
		enable_count_ = 0;
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
	* @brief デバッグ機能が有効かどうか
	*/
	bool is_enabled();

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

private:

	int_t enable_count_;
	AnyPtr break_point_hook_;
	AnyPtr call_hook_;
	AnyPtr return_hook_;
	AnyPtr throw_hook_;

public:
	void visit_members(Visitor& m){
		CppSingleton::visit_members(m);
		m & break_point_hook_ & call_hook_ & return_hook_ & throw_hook_;
	}
};

const SmartPtr<Debug>& debug(); 

}
