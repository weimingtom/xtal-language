
#pragma once

namespace xtal{

struct Setting{
	ThreadLib* thread_lib;
	StreamLib* stream_lib;
	FilesystemLib* filesystem_lib;
	AllocatorLib* allocator_lib;
	ChCodeLib* chcode_lib;

	Setting();
};

class Environment;

/**
* @brief Xtal実行環境を初期化する。
*/
void initialize(const Setting& setting);

/**
* @brief Xtal実行環境を破棄する。
*/
void uninitialize();

/**
* @brief 現在のカレントのXtal実行環境を取得する。
*/
Environment* environment();

/**
* @brief カレントのXtal実行環境を設定する。
*/
void set_environment(Environment* e);
	
void debug_print();

void print_alive_objects();

/**
* @brief ユーザーが登録したメモリアロケート関数を使ってメモリ確保する。
*
* メモリ確保失敗は例外で返される。
*/
void* user_malloc(size_t size);

/**
* @brief ユーザーが登録したメモリアロケート関数を使ってメモリ確保する。
*
* メモリ確保失敗はNULL値で返される。
*/
void* user_malloc_nothrow(size_t size);

/**
* @brief ユーザーが登録したメモリデアロケート関数を使ってメモリ解放する。
*
*/
void user_free(void* p);

/**
* @brief 小さいオブジェクト用にメモリをアロケートする。
*/
void* so_malloc(size_t size);

/**
* @brief 小さいオブジェクト用のメモリを解放する。
*/
void so_free(void* p, size_t size);

/**
* @brief ガーベジコレクションを実行する
*
* さほど時間はかからないが、完全にゴミを解放できないガーベジコレクト関数
* 例えば循環参照はこれでは検知できない。
*/
void gc();

/**
* @brief 循環オブジェクトも解放するフルガーベジコレクションを実行する
*
* 時間はかかるが、現在ゴミとなっているものはなるべく全て解放するガーベジコレクト関数
* 循環参照も検知できる。
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
*/
void enable_gc();

void register_gc(RefCountingBase* p);

void register_gc_observer(GCObserver* p);

void unregister_gc_observer(GCObserver* p);

const ClassPtr& cpp_class(CppClassSymbolData* key);

const ClassPtr& new_cpp_class(const StringPtr& name, CppClassSymbolData* key);

bool exists_cpp_class(CppClassSymbolData* key);

const ClassPtr& cpp_class(CppClassSymbolData* key);

void set_cpp_class(const ClassPtr& cls, CppClassSymbolData* key);

/**
* @brief T形をxtalで扱えるクラスを生成し、登録する。
* 既に生成されている場合、生成済みのクラスを返す。
*/
template<class T>
inline const ClassPtr& new_cpp_class(const StringPtr& name){
	return new_cpp_class(name, &CppClassSymbol<T>::value);
}

/**
* @brief T形をxtalで扱えるクラスを生成し、登録する。
* 既に生成されている場合、生成済みのクラスを返す。
*/
template<class T>
const SmartPtr<T>& new_cpp_singleton(){
	if(exists_cpp_class<T>()){
		return unchecked_ptr_cast<T>(cpp_class<T>());
	}
	set_cpp_class<T>(xnew<T>());
	return unchecked_ptr_cast<T>(cpp_class<T>());
}

/**
* @brief 既にnew_cpp_class<T>()で生成させれているか調べる。
*/
template<class T>
inline bool exists_cpp_class(){
	return exists_cpp_class(&CppClassSymbol<T>::value);
}

/**
* @brief new_cpp_class<T>()で生成されたクラスを取得する。
*/
template<class T>
inline const ClassPtr& cpp_class(){
	return cpp_class(&CppClassSymbol<T>::value);
}

/**
* @brief cpp_class<T>などで返されるクラスを設定する。
*/
template<class T>
inline void set_cpp_class(const ClassPtr& cls){
	return set_cpp_class(cls, &CppClassSymbol<T>::value);
}

/**
* @brief クラスのメンバを取り出す。
*/
const AnyPtr& cache_member(const AnyPtr& target_class, const IDPtr& primary_key, const AnyPtr& secondary_key, int_t& accessibility);

/**
* @brief クラスの継承関係を調べる。
*/
bool cache_is(const AnyPtr& target_class, const AnyPtr& klass);

/**
* @brief メンバーのキャッシュテーブルに登録されているデータを無効にする。
*/
void invalidate_cache_member();

/**
* @brief 継承関係のキャッシュテーブルに登録されているデータを無効にする。
*/
void invalidate_cache_is();

/**
* @brief VMachineインスタンスをレンタルする。
*/
VMachinePtr vmachine_take_over();

/**
* @brief VMachineインスタンスを返却する。
*/
void vmachine_take_back(const VMachinePtr& vm);

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
const LibPtr& lib();

const StreamPtr& stdin_stream();

const StreamPtr& stdout_stream();

const StreamPtr& stderr_stream();

const ClassPtr& RuntimeError();

const ClassPtr& CompileError();

const ClassPtr& UnsupportedError();

const ClassPtr& ArgumentError();

const IDPtr& intern_literal(const char_t* str);

const IDPtr& intern(const char_t* str);

const IDPtr& intern(const char_t* str, uint_t data_size);

const IDPtr& intern(const char_t* str, uint_t data_size, uint_t hash);

AnyPtr interned_strings();

bool thread_enabled();

void yield_thread();

void sleep_thread(float_t sec);

ThreadPtr new_thread(const AnyPtr& callback_fun);

MutexPtr new_mutex();

void lock_mutex(const MutexPtr& p);

/**
* @brief VMachinePtrオブジェクトを返す
*
* グローバルなVMachinePtrオブジェクトを返す。
* スレッド毎にこのグローバルVMachinePtrオブジェクトは存在する。
*/
const VMachinePtr& vmachine();


/**
* @brief 先頭バイトを見て、そのマルチバイト文字が何文字かを調べる。
*
* マイナスの値が返された場合、最低文字数を返す。
* -2の場合、最低2文字以上の文字で、本当の長さは2文字目を読まないと判断できない、という意味となる。
*/
int_t ch_len(char_t lead);

/**
* @brief マルチバイト文字が何文字かを調べる。
*
* int_t ch_len(char_t lead)が呼ばれた後、マイナスの値を返した場合に続けて呼ぶ。
* ch_lenで-2の値を返した後は、strの先には最低2バイト分のデータを格納すること。
*/
int_t ch_len2(const char_t* str);


/**
* @brief 一つ先の文字を返す
*
* 例えば a を渡した場合、b が返る
*/
StringPtr ch_inc(const char_t* data, int_t data_size);

/**
* @brief 文字の大小判定
*
* 負の値 a の文字の方がbの文字より小さい
* 0の値 等しい
* 正の値 bの文字の方がaの文字より小さい
*/
int_t ch_cmp(const char_t* a, uint_t asize, const char_t* b, uint_t bsize);

/**
* @biref 演算子の名前をあらわす文字列の配列を返す
*/
const IDPtr* id_op_list();


#ifndef XTAL_NO_PARSER

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

CodePtr source(const char_t* src, int_t size, const char* file);

#endif

CodePtr compiled_source(const void* src, int_t size, const char* file);

}
