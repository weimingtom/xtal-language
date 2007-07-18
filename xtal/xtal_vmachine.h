
#pragma once

#include "xtal_any.h"
#include "xtal_string.h"

namespace xtal{

// fwd decl
class VMachineImpl;
class ArgumentsImpl;

class Arguments : public Any{
public:

	Arguments();

	explicit Arguments(ArgumentsImpl* p)
		:Any((AnyImpl*)p){}

	Arguments(const Null&)
		:Any(null){}

	const Any& op_at(const Any& index) const;
	
	int_t length() const;
	
	Any each_ordered_arg() const;
	
	Any each_named_arg() const;

	ArgumentsImpl* impl() const{ return (ArgumentsImpl*)Any::impl(); }
};

/**
* @brief XTAL仮想マシン
*
*/
class VMachine : public Any{
public:

	VMachine();

	explicit VMachine(VMachineImpl* p)
		:Any((AnyImpl*)p){}

	VMachine(const Null&)
		:Any(null){}

public:

	// 関数呼び出し側が使うための関数群

	/**
	* @brief 関数を呼び出す用意をする。
	*
	*/
	void setup_call(int_t need_result_count) const;
	void setup_call(int_t need_result_count, const Any& a1) const;
	void setup_call(int_t need_result_count, const Any& a1, const Any& a2) const;
	void setup_call(int_t need_result_count, const Any& a1, const Any& a2, const Any& a3) const;
	void setup_call(int_t need_result_count, const Any& a1, const Any& a2, const Any& a3, const Any& a4) const;
	
	/**
	* @brief 引数を積む。
	*
	*/
	void push_arg(const Any& value) const;
	
	/**
	* @brief 名前付き引数を積む。
	*
	*/
	void push_arg(const ID& name, const Any& value) const;

	/**
	* @brief 名前付き引数を積む。
	*
	*/
	void push_arg(const Named& p) const{ push_arg(p.name, p.value); }

	/**
	* @brief pos番目の戻り値を得る。
	*
	*/
	const Any& result(int_t pos = 0) const;
	
	/**
	* @brief pos番目の戻り値を返し、呼び出しの後始末をする。
	*
	*/
	Any result_and_cleanup_call(int_t pos = 0) const;
	
	/**
	* @brief 呼び出しの後始末をする。
	*
	*/
	void cleanup_call() const;
	
	/**
	* @brief thisを差し替える。
	*
	*/
	void set_arg_this(const Any& self) const;

	/**
	* @brief ヒントの設定
	*
	* 例外が起きたときのエラーメッセージのために、
	* 現在呼び出しているオブジェクトとメソッド名を登録する。
	*/
	void set_hint(const Any& hint1, const String& hint2) const;
	
public:

	// 関数呼び出され側が使うための関数群

	/**
	* @brief pos番目の引数を得る。
	*
	*/
	const Any& arg(int_t pos) const;
	
	/**
	* @brief nameに対応する引数を得る。
	*
	*/
	const Any& arg(const ID& name) const;
	
	/**
	* @brief pos番目の引数を得る。もしpos番目の引数がなければnameに対応する引数を得る。
	*
	*/
	const Any& arg(int_t pos, const ID& name) const;
	
	/**
	* @brief pos番目の引数を得る。
	*
	* もしpos番目の引数が無ければdefの値を返す。
	*/
	const Any& arg_default(int_t pos, const Any& def) const;
	
	/**
	* @brief nameに対応する引数を得る。
	*
	* もしnameに対応する引数が無ければdefの値を返す。
	*/
	const Any& arg_default(const ID& name, const Any& def) const;
	
	/**
	* @brief pos番目の引数を得る。もしpos番目の引数がなければnameに対応する引数を得る。
	*
	* もしnameに対応する引数が無ければdefの値を返す。
	*/
	const Any& arg_default(int_t pos, const ID& name, const Any& def) const;	

	/**
	* @brief thisを取得。
	*
	*/
	const Any& get_arg_this() const;

	/**
	* @brief pos番目の名前指定引数の名前を取得。
	*
	* @param pos 0〜named_arg_count()-1まで
	*/
	const ID& arg_name(int_t pos) const;

	/**
	* @brief 位置指定引数の数を取得。
	*
	*/
	int_t ordered_arg_count() const;
	
	/**
	* @brief 名前指定引数の数を取得
	*
	*/
	int_t named_arg_count() const;
	
	/**
	* @brief 呼び出し元が戻り値を必要としているかどうか。
	*
	*/
	bool need_result() const;
	
	/**
	* @brief 呼び出し元に引数の数だけの戻り値を返す。
	*
	* need_result_countの方が数が多い場合はその分nullが返される。
	* need_result_countの方が数が少ない場合は返した値は捨てられる。
	*/
	void return_result() const;
	void return_result(const Any& value1) const;
	void return_result(const Any& value1, const Any& value2) const;
	void return_result(const Any& value1, const Any& value2, const Any& value3) const;
	void return_result(const Any& value1, const Any& value2, const Any& value3, const Any& value4) const;

	/**
	* @brief 呼び出し元に配列の要素の数だけの戻り値を返す。
	*
	* values.lengthの方が数が多い場合はその分nullが返される。
	* values.lengthの方が数が少ない場合は返した値は捨てられる。
	*/
	void return_result_array(const Array& values) const;

	/**
	* @brief return_resultやcarry_overを既に呼び出したならtrueを、そうでないならfalseを返す。
	*
	*/
	bool processed() const;
	
	void replace_result(int_t pos, const Any& v) const;
	
	void recycle_call() const;
	
	void recycle_call(const Any& a1) const;
		
public:

	void push(const Any& v) const;

	VMachineImpl* impl() const{ return (VMachineImpl*)Any::impl(); }

	void reset() const;
};

}

