
#pragma once

#include "xtal_fwd.h"
#include "xtal_any.h"
#include "xtal_smartptr.h"

namespace xtal{

/** @addtogroup lib */
/*@{*/

/**
* @brief file_nameファイルをコンパイルする。
*
* @param file_name Xtalスクリプトが記述されたファイルの名前
* @return 実行できる関数オブジェクト
* この戻り値をserializeすると、バイトコード形式で保存される。
*/
CodePtr compile_file(const StringPtr& file_name);

/**
* @brief source文字列をコンパイルする。
*
* @param source Xtalスクリプトが記述された文字列
* @return 実行できる関数オブジェクト
* この戻り値をserializeすると、バイトコード形式で保存される。
*/
CodePtr compile(const StringPtr& source);

/**
* @brief file_nameファイルをコンパイルして実行する。
*
* @param file_name Xtalスクリプトが記述されたファイルの名前
* @return スクリプト内でexportされた値
*/
AnyPtr load(const StringPtr& file_name);

/**
* @brief file_nameファイルをコンパイルしてコンパイル済みソースを保存し、実行する。
*
* @param file_name Xtalスクリプトが記述されたファイルの名前
* @return スクリプト内でexportされた値
*/
AnyPtr load_and_save(const StringPtr& file_name);

/**
* @brief interactive xtalの実行
*/
void ix();

/**
* @brief ガーベジコレクションを実行する
*
* さほど時間はかからないが、完全にゴミを解放できないガーベジコレクト関数
* 例えば循環参照はこれでは検知できない。
* ゲームで使用する場合、毎フレームこれを呼んだ方が良い。
*/
void gc();

/**
* @brief 循環オブジェクトも解放するフルガーベジコレクションを実行する
*
* 時間はかかるが、現在ゴミとなっているものはなるべく全て解放するガーベジコレクト関数
* 循環参照も検知できる。
* ゲームで使用する場合、シーンの切り替え時など、節目節目に呼んだほうが良い。
*/
void full_gc();


/**
* @brief ガーベジコレクションを無効化する
*
* gcやfull_gcの呼び出しを無効化する関数。
* 内部でこれが何回呼び出されたか記憶されており、呼び出した回数enable_gcを呼びないとガーベジコレクションは有効にならない
*/
void disable_gc();

/**
* @brief ガーベジコレクションを有効化する
*
* disable_gcが呼ばれた回数と同じだけ呼び出すとガーベジコレクションが有効になる
* 
*/
void enable_gc();

/**
* @brief VMachinePtrオブジェクトを返す
*
* グローバルなVMachinePtrオブジェクトを返す。
* スレッド毎にこのグローバルVMachinePtrオブジェクトは存在する。
*/
const VMachinePtr& vmachine();

/**
* @brief Iteratorクラスを返す
*/
const ClassPtr& Iterator();

/**
* @brief Enumeratorクラスを返す
*/
const ClassPtr& Enumerator();

/**
* @brief builtinクラスを返す
*/
const ClassPtr& builtin();

/**
* @brief libクラスを返す
*/
const ClassPtr& lib();

/**
* @brief グローバルに存在する仮想マシンオブジェクトを取得する
*/
const VMachinePtr& vmachine();


const StreamPtr& stdin_stream();
const StreamPtr& stdout_stream();
const StreamPtr& stderr_stream();


void add_long_life_var(AnyPtr* a, int_t n = 1);
void remove_long_life_var(AnyPtr* a, int_t n = 1);
AnyPtr* make_place();

void initialize();
void uninitialize();
bool initialized();

AnyPtr cast_error(const AnyPtr& from, const AnyPtr& to);
AnyPtr argument_error(const AnyPtr& from, const AnyPtr& to, int_t param_num, const AnyPtr& param_name);
AnyPtr unsupported_error(const AnyPtr& name, const AnyPtr& member);

AnyPtr get_text(const char* text);
AnyPtr format(const char* text);

void set_get_text_map(const MapPtr& map);
void add_get_text_map(const MapPtr& map);
MapPtr get_get_text_map();

AnyPtr source(const char* src, int_t size, const char* file);

typedef void (*except_handler_t)(const AnyPtr& except, const char* file, int line);
except_handler_t except_handler();
void set_except_handler(except_handler_t handler);

void iter_next(AnyPtr& target, AnyPtr& value, bool first);
void iter_next(AnyPtr& target, AnyPtr& value1, AnyPtr& value2, bool first);
void iter_next(AnyPtr& target, AnyPtr& value1, AnyPtr& value2, AnyPtr& value3, bool first);

struct IterBreaker{
	AnyPtr target;
	IterBreaker(const AnyPtr& tar=null):target(tar){}
	~IterBreaker();
	operator AnyPtr&(){ return target; }
	operator bool(){ return target; }
};

/*@}*/

namespace debug{

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

}

}

