
#pragma once

#include "xtal_fwd.h"
#include "xtal_vmachine.h"

namespace xtal{

/** @addtogroup lib */
/*@{*/

/**
* @brief file_nameファイルをコンパイルする。
*
* @param file_name Xtalスクリプトが記述されたファイルの名前
* @return 実行できる関数オブジェクト
* この戻り値をobject_dumpすると、バイトコード形式で保存される。
*/
Code compile_file(const String& file_name);

/**
* @brief source文字列をコンパイルする。
*
* @param source Xtalスクリプトが記述された文字列
* @return 実行できる関数オブジェクト
* この戻り値をobject_dumpすると、バイトコード形式で保存される。
*/
Code compile(const String& source);

/**
* @brief file_nameファイルをコンパイルして実行する。
*
* @param file_name Xtalスクリプトが記述されたファイルの名前
* @return スクリプト内でexportされた値
*/
Any load(const String& file_name);

/**
* @brief file_nameファイルをコンパイルしてコンパイル済みソースを保存し、実行する。
*
* @param file_name Xtalスクリプトが記述されたファイルの名前
* @return スクリプト内でexportされた値
*/
Any load_and_save(const String& file_name);

/**
* @brief interactive xtalの実行
*/
void ix();

/**
* @brief ガーベジコレクションを実行する
*
* さほど時間はかからないが、完全にゴミを解放できないガーベジコレクト関数
*/
void gc();

/**
* @brief 循環オブジェクトも解放するフルガーベジコレクションを実行する
*
* 時間はかかるが、現在ゴミとなっているものはなるべく全て解放するガーベジコレクト関数
*/
void full_gc();

/**
* @brief ガーベジコレクションを有効化する
*
* disable_gcが呼ばれた回数と同じだけ呼び出すとガーベジコレクションが有効になる
*/
void enable_gc();

/**
* @brief ガーベジコレクションを無効化する
*
* 内部でこれが何回呼び出されたか記憶されており、呼び出した回数enable_gcを呼びないとガーベジコレクションは有効にならない
*/
void disable_gc();

/**
* @brief オブジェクトを直列化して保存する
*
* @param obj 直列化して保存したいオブジェクト
* @param out 直列化先のストリーム
*/
void object_dump(const Any& obj, const Stream& out);

/**
* @brief オブジェクトを直列化して読み込む
*
* @param in 直列化されたオブジェクトを読み込むストリーム
* @return 復元されたオブジェクト
*/
Any object_load(const Stream& in);

/**
* @brief オブジェクトをスクリプト化して保存する
*
* @param obj スクリプト化して保存したいオブジェクト
* @param out 直列化先のストリーム
*/
void object_to_script(const Any& obj, const Stream& out);

/**
* @brief オブジェクトをC++に埋め込める形にして保存する
*
* @param obj C++化したいオブジェクト
* @param out 直列化先のストリーム
*/
void object_to_cpp(const Any& obj, const Stream& out);

/**
* @brief VMachineオブジェクトを返す
* グローバルなVMachineオブジェクトを返す。
* スレッド毎にこのグローバルVMachineオブジェクトは存在する。
*/
const VMachine& vmachine();

/**
* @brief Iteratorクラスを返す
*/
const Class& Iterator();

/**
* @brief Enumeratorクラスを返す
*/
const Class& Enumerator();

/**
* @brief builtinクラスを返す
*/
const Class& builtin();

/**
* @brief libオブジェクトを返す
*/
const Any& lib();

Any get_text(const char* text);

Any format(const char* text);

void set_get_text_map(const Any& map);
void add_get_text_map(const Any& map);
Any get_get_text_map();

Any source(const char* src, int_t size, const char* file);

void print(const VMachine& vm);
void println(const VMachine& vm);

typedef void (*except_handler_t)(const Any& except, const char* file, int line);
except_handler_t except_handler();
void set_except_handler(except_handler_t handler);

Any except();
void set_except(const Any& except);

void iter_next(Any& target, Any& value, bool first);
void iter_next(Any& target, Any& value1, Any& value2, bool first);
void iter_next(Any& target, Any& value1, Any& value2, Any& value3, bool first);

struct IterBreaker{
	Any target;
	IterBreaker(const Any& tar=null):target(tar){}
	~IterBreaker();
	operator Any&(){ return target; }
	operator bool(){ return target; }
};

/*@}*/

namespace debug{

class InfoImpl;

class Info : public Any{
public:

	Info();

	explicit Info(InfoImpl* p)
		:Any((AnyImpl*)p){}

	Info(const Null&)
		:Any(null){}

	int_t kind() const;
	int_t line() const;
	String file_name() const;
	String fun_name() const;
	Frame local_variables() const;

	void set_kind(int_t v) const;
	void set_line(int_t v) const;
	void set_file_name(const String& v) const;
	void set_fun_name(const String& v) const;
	void set_local_variables(const Frame& v) const;

	InfoImpl* impl() const{
		return (InfoImpl*)Any::impl();
	}
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
void set_line_hook(const Any& hook);

/**
* @brief 関数呼び出しされる度に呼び出されるフック関数を登録する
*/
void set_call_hook(const Any& hook);

/**
* @brief 関数からreturnされる度に呼び出されるフック関数を登録する
*/
void set_return_hook(const Any& hook);

/**
* @brief set_line_hook関数で登録した関数を取得する
*/
Any line_hook();

/**
* @brief set_call_hook関数で登録した関数を取得する
*/
Any call_hook();

/**
* @brief set_return_hook関数で登録した関数を取得する
*/
Any return_hook();

}

}

