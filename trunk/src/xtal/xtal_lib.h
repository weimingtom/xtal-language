
#pragma once

namespace xtal{

/**
* @brief xtalシステムを初期化する
*/
void initialize();

/**
* @brief xtalシステムを破棄する
*/
void uninitialize();

/**
* @brief xtalシステムが初期化済みか調べる
*/
bool is_initialized();

/**
* @brief uninitializeの時にコールバックされる破棄関数を登録する
*/
void register_uninitializer(void (*uninitializer)());


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
* @return スクリプト内でreturnされた値
*/
AnyPtr load(const StringPtr& file_name);

/**
* @brief file_nameファイルをコンパイルしてコンパイル済みソースを保存し、実行する。
*
* @param file_name Xtalスクリプトが記述されたファイルの名前
* @return スクリプト内でreturnされた値
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
* ゲームで使用する場合、毎フレームこれを呼ぶことを推奨する。
*/
void gc();

/**
* @brief 循環オブジェクトも解放するフルガーベジコレクションを実行する
*
* 時間はかかるが、現在ゴミとなっているものはなるべく全て解放するガーベジコレクト関数
* 循環参照も検知できる。
* ゲームで使用する場合、シーンの切り替え時など、節目節目に呼ぶことを推奨する。
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
* @brief Iterableクラスを返す
*/
const ClassPtr& Iterable();

/**
* @brief builtinシングルトンクラスを返す
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

AnyPtr* make_place();


CodePtr source(const char_t* src, int_t size, const char* file);


/*@}*/

}

