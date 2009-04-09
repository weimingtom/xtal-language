/** \file src/xtal/xtal_environment.h
* \brief src/xtal/xtal_environment.h
*/

#ifndef XTAL_ENVIRONMENT_H_INCLUDE_GUARD
#define XTAL_ENVIRONMENT_H_INCLUDE_GUARD

#pragma once

namespace xtal{

/// \name Xtal環境の初期化、破棄周り
//@{

/**
* \brief アロケータライブラリ
*/
class AllocatorLib{
public:
	virtual ~AllocatorLib(){}
	virtual void* malloc(std::size_t size){ return std::malloc(size); }
	virtual void free(void* p, std::size_t size){ std::free(p); }
	virtual void* out_of_memory(std::size_t size){ return 0; }
};

/**
* \brief 文字コードライブラリ
*/
class ChCodeLib{
public:
	virtual ~ChCodeLib(){}

	virtual int_t ch_len(char_t lead){ return 1; }
	virtual int_t ch_len2(const char_t* str){ return ch_len(*str); }
	virtual StringPtr ch_inc(const char_t* data, int_t data_size);
	virtual int_t ch_cmp(const char_t* a, int_t asize, const char_t* b, int_t bsize);
};

/**
* \brief スレッドライブラリ
*/
class ThreadLib{
public:
	virtual ~ThreadLib(){}

	virtual void* new_thread(){ return 0; }
	virtual void delete_thread(void* thread_object){}
	virtual void start_thread(void* thread_object, void (*callback)(void*), void* data){}
	virtual void join_thread(void* thread_object){}

	virtual void* new_mutex(){ return 0; }
	virtual void delete_mutex(void* mutex_object){}
	virtual void lock_mutex(void* mutex_object){}
	virtual void unlock_mutex(void* mutex_object){}

	virtual void yield(){}
	virtual void sleep(float_t sec){}
};

/**
* \brief 標準入出力ライブラリ
*/
class StdStreamLib{
public:
	virtual ~StdStreamLib(){}

	virtual void* new_stdin_stream(){ return 0; }
	virtual void delete_stdin_stream(void* stdin_stream_object){}
	virtual uint_t read_stdin_stream(void* stdin_stream_object, void* dest, uint_t size){ return 0; }

	virtual void* new_stdout_stream(){ return 0; }
	virtual void delete_stdout_stream(void* stdout_stream_object){}
	virtual uint_t write_stdout_stream(void* stdout_stream_object, const void* src, uint_t size){ return 0; }

	virtual void* new_stderr_stream(){ return 0; }
	virtual void delete_stderr_stream(void* stderr_stream_object){}
	virtual uint_t write_stderr_stream(void* stderr_stream_object, const void* src, uint_t size){ return 0; }
};

/**
* \brief ファイルシステムライブラリ
*/
class FilesystemLib{
public:
	virtual ~FilesystemLib(){}

	virtual bool is_directory(const char_t* path){ return false; }

	virtual void* new_file_stream(const char_t* path, const char_t* flags){ return 0; }
	virtual void delete_file_stream(void* file_stream_object){}
	virtual uint_t read_file_stream(void* file_stream_object, void* dest, uint_t size){ return 0; }
	virtual uint_t write_file_stream(void* file_stream_object, const void* src, uint_t size){ return 0; }
	virtual void seek_file_stream(void* file_stream_object, uint_t pos){}
	virtual uint_t tell_file_stream(void* file_stream_object){ return 0; }
	virtual bool end_file_stream(void* file_stream_object){ return true; }
	virtual uint_t size_file_stream(void* file_stream_object){ return 0; }
	virtual void flush_file_stream(void* file_stream_object){}

	virtual void* new_entries(const char_t* path){ return 0; }
	virtual void delete_entries(void* entries_object){}
	virtual const char_t* next_entries(void* entries_object){ return 0; }
	virtual void break_entries(void* entries_object){}

};

/**
* \brief 使用ライブラリの指定のための構造体
*/
struct Setting{
	AllocatorLib* allocator_lib;
	ChCodeLib* ch_code_lib;
	ThreadLib* thread_lib;
	StdStreamLib* std_stream_lib;
	FilesystemLib* filesystem_lib;

	/**
	* \brief ほとんど何もしない動作を設定する。
	*/
	Setting();
};

/**
* \brief Xtal実行環境を作成し、初期化し、カレントに設定する。
*/
void initialize(const Setting& setting);

/**
* \brief カレントのXtal実行環境を破棄する。
*/
void uninitialize();

/**
* \brief 現在のカレントのXtal実行環境を取得する。
*/
Environment* environment();

/**
* \brief カレントのXtal実行環境を設定する。
*/
void set_environment(Environment* e);

//@}

ThreadLib* thread_lib();

StdStreamLib* std_stream_lib();

FilesystemLib* filesystem_lib();

/////////////////////////////////////////////////////

/// \name メモリ周り
//@{

/**
* \brief ユーザーが登録したメモリアロケート関数を使ってメモリ確保する。
*/
void* xmalloc(size_t size);

/**
* \brief ユーザーが登録したメモリデアロケート関数を使ってメモリ解放する。
*/
void xfree(void* p, size_t size);

/**
* \brief メモリ確保をスコープに閉じ込めるためのユーティリティクラス
*/
struct XMallocGuard{
	/**
	* \brief 確保せずに構築されるコンストラクタ
	*/
	XMallocGuard():p(0){}
	
	/**
	* \brief size分のメモリ確保するコンストラクタ
	*/
	explicit XMallocGuard(uint_t size):p(xmalloc(size)), sz(size){}

	/**
	* \brief メモリを解放する
	*/
	~XMallocGuard(){ xfree(p, sz); }
public:	

	/**
	* \brief メモリを確保する
	* それまでに確保されていたメモリは解放される
	*/
	void malloc(size_t size){ xfree(p, sz); p = xmalloc(size); sz = size; }

	/**
	* \brief メモリの先頭アドレスを返す
	*/
	void* get(){ return p; }

	/**
	* \brief 確保されているメモリのサイズを返す
	*/
	uint_t size(){ return sz; }

private:
	void* p;
	uint_t sz;

	XTAL_DISALLOW_COPY_AND_ASSIGN(XMallocGuard);
};

#define XTAL_PROTECT if(const ::xtal::Protect& xtalprotect = setjmp(::xtal::protect().buf)!=0)

#define XTAL_OUT_OF_MEMORY else 

//@}

/**
* \internal
* \breif jmp_bufを閉じ込める構造体
*/
struct JmpBuf{
	jmp_buf buf;
};

JmpBuf& protect();

void reset_protect();

bool ignore_memory_assert();

struct Protect{
	bool pass;
	Protect(bool pass):pass(pass){}
	~Protect(){ reset_protect(); }
	operator bool() const{ return !pass; }
};

/////////////////////////////////////////////////////

/**
* \xbind lib::builtin
* \brief ガーベジコレクションを実行する
*
* さほど時間はかからないが、完全にゴミを解放できないガーベジコレクト関数
* 例えば循環参照はこれでは検知できない。
*/
void gc();

/**
* \xbind lib::builtin
* \brief 循環オブジェクトも解放するフルガーベジコレクションを実行する
*
* 時間はかかるが、現在ゴミとなっているものはなるべく全て解放するガーベジコレクト関数
* 循環参照も検知できる。
*/
void full_gc();

/**
* \xbind lib::builtin
* \brief ガーベジコレクションを無効化する
*
* gcやfull_gcの呼び出しを無効化する関数。
* 内部でこれが何回呼び出されたか記憶されており、呼び出した回数enable_gcを呼びないとガーベジコレクションは有効にならない
*/
void disable_gc();

/**
* \xbind lib::builtin
* \brief ガーベジコレクションを有効化する

* disable_gcが呼ばれた回数と同じだけ呼び出すとガーベジコレクションが有効になる
*/
void enable_gc();

uint_t alive_object_count();

AnyPtr alive_object(uint_t i);

/////////////////////////////////////////////////////

/**
* \internal
* \brief keyに対応するC++のクラスのクラスオブジェクトを返す。
*/
const ClassPtr& cpp_class(CppClassSymbolData* key);

/*
* \internal
* \brief keyに対応するC++のクラスのクラスオブジェクトを設定する。
*/
void set_cpp_class(const ClassPtr& cls, CppClassSymbolData* key);

/**
* \brief クラスTに対応するC++のクラスのクラスオブジェクトを返す。
*/
template<class T>
inline const ClassPtr& cpp_class(){
	return cpp_class(&CppClassSymbol<T>::value);
}

/**
* \brief クラスTに対応するC++のクラスのクラスオブジェクトを設定する。
*/
template<class T>
inline void set_cpp_class(const ClassPtr& cls){
	return set_cpp_class(cls, &CppClassSymbol<T>::value);
}

/**
* \internal
* \brief T型のオブジェクトを環境から取り出す
*/
void* cpp_var(CppVarSymbolData* key);

/**
* \internal
* \brief T型のオブジェクトを環境に設定する
*/
void set_cpp_var(void* p, void (*deleter)(void*), CppVarSymbolData* key);

template<class T>
struct CppVarDeleter{
	static void deleter(void* p){
		((T*)p)->~T();
		xfree(p, sizeof(T));
	}
};

/**
* \brief T型のオブジェクトを環境から取り出す
*/
template<class T>
T& cpp_var(){
	if(void* p=cpp_var(&CppVarSymbol<T>::value)){
		return *(T*)p;
	}
	void* p=xmalloc(sizeof(T));
	new(p) T;
	set_cpp_var(p, &CppVarDeleter<T>::deleter, &CppVarSymbol<T>::value);
	return *(T*)p;
}

/////////////////////////////////////////////////////

/**
* \internal
* \brief クラスのメンバを取り出す。
*/
const AnyPtr& cache_member(const AnyPtr& target_class, const IDPtr& primary_key, const AnyPtr& secondary_key, int_t& accessibility);

/**
* \internal
* \brief クラスの継承関係を調べる。
*/
bool cache_is(const AnyPtr& target_class, const AnyPtr& klass);

/**
* \internal
* \brief クラスのコンストラクタがキャッシュされているから調べる。
*/
bool cache_ctor(const AnyPtr& target_class, int_t kind);

/**
* \internal
* \brief メンバーのキャッシュテーブルに登録されているデータを無効にする。
*/
void invalidate_cache_member();

/**
* \internal
* \brief 継承関係のキャッシュテーブルに登録されているデータを無効にする。
*/
void invalidate_cache_is();

/**
* \internal
* \brief クラスのコンストラクタのキャッシュテーブルに登録されているデータを無効にする。
*/
void invalidate_cache_ctor();

/////////////////////////////////////////////////////

/**
* \internal
* \brief VMachineインスタンスをレンタルする。
*/
VMachinePtr vmachine_take_over();

/**
* \internal
* \brief VMachineインスタンスを返却する。
*/
void vmachine_take_back(const VMachinePtr& vm);

/////////////////////////////////////////////////////

/// \name ライブラリオブジェクト取得関数
// @{

/**
* \brief builtinオブジェクトを返す
*/
const ClassPtr& builtin();

/**
* \brief libオブジェクトを返す
*/
const LibPtr& lib();

/**
* \brief stdinストリームオブジェクトを返す
*/
const StreamPtr& stdin_stream();

/**
* \brief stdoutストリームオブジェクトを返す
*/
const StreamPtr& stdout_stream();

/**
* \brief stderrストリームオブジェクトを返す
*/
const StreamPtr& stderr_stream();

/**
* \brief VMachinePtrオブジェクトを返す
*
* グローバルなVMachinePtrオブジェクトを返す。
* スレッド毎にこのグローバルVMachinePtrオブジェクトは存在する。
*/
const VMachinePtr& vmachine();

// @}

/**
* \internal
* \brief 文字列をインターン済み文字列に変換する
*/
const IDPtr& intern(const char_t* str, uint_t data_size);

/**
* \internal
* \brief 文字列をインターン済み文字列に変換する
*/
const IDPtr& intern(const char_t* str);

/**
* \internal
* \brief 文字列リテラルをインターン済み文字列に変換する
*/
const IDPtr& intern(const char_t* str, IdentifierData* iddata);

/**
* \internal
* \brief 文字列リテラルをインターン済み文字列に変換する
*/
inline const IDPtr& intern(const StringLiteral& str){
	return intern(str, str.size());
}

/**
* \brief インターン済み文字列を列挙する
*/
AnyPtr interned_strings();

/**
* \brief 環境をロックする
*/
void xlock();

/**
* \brief 環境をアンロックする
*/
void xunlock();

struct XUnlock{
	XUnlock(int){ xunlock(); }
	~XUnlock(){ xlock(); }
	operator bool() const{ return true; }
private:
	XTAL_DISALLOW_COPY_AND_ASSIGN(XUnlock);
};

void register_thread(Environment*);

void unregister_thread(Environment*);

/**
* \brief テキストマップを返す
*/
const MapPtr& text_map();

/**
* \internal
* \brief 先頭バイトを見て、そのマルチバイト文字が何文字かを調べる。
*
* マイナスの値が返された場合、最低文字数を返す。
* -2の場合、最低2文字以上の文字で、本当の長さは2文字目を読まないと判断できない、という意味となる。
*/
int_t ch_len(char_t lead);

/**
* \internal
* \brief マルチバイト文字が何文字かを調べる。
*
* int_t ch_len(char_t lead)が呼ばれた後、マイナスの値を返した場合に続けて呼ぶ。
* ch_lenで-2の値を返した後は、strの先には最低2バイト分のデータを格納すること。
*/
int_t ch_len2(const char_t* str);

/**
* \internal
* \brief 一つ先の文字を返す
*
* 例えば a を渡した場合、b が返る
*/
StringPtr ch_inc(const char_t* data, int_t data_size);

/**
* \internal
* \brief 文字の大小判定
*
* 負の値 a の文字の方がbの文字より小さい
* 0の値 等しい
* 正の値 bの文字の方がaの文字より小さい
*/
int_t ch_cmp(const char_t* a, uint_t asize, const char_t* b, uint_t bsize);

/**
* \internal
* \brief 演算子の名前をあらわすIDの配列を返す
*/
const IDPtr* id_op_list();

StreamPtr open(const StringPtr& file_name, const StringPtr& mode);

#ifndef XTAL_NO_PARSER

/// \name コンパイル系関数
//@{

/**
* \xbind lib::builtin
* \brief file_nameファイルをコンパイルする。
* この戻り値をserializeすると、バイトコード形式で保存される。
* \param file_name Xtalスクリプトが記述されたファイルの名前
* \return 実行できる関数オブジェクト
*/
CodePtr compile_file(const StringPtr& file_name);

/**
* \xbind lib::builtin
* \brief source文字列をコンパイルする。
* この戻り値をserializeすると、バイトコード形式で保存される。
* \param source Xtalスクリプトが記述された文字列
* \return 実行できる関数オブジェクト
*/
CodePtr compile(const StringPtr& source);

/**
* \xbind lib::builtin
* \brief file_nameファイルをコンパイルして実行する。
* \param file_name Xtalスクリプトが記述されたファイルの名前
* \return スクリプト内でreturnされた値
*/
AnyPtr load(const StringPtr& file_name);

/**
* \xbind lib::builtin
* \brief file_nameファイルをコンパイルしてコンパイル済みソースを保存し、実行する。
* \param file_name Xtalスクリプトが記述されたファイルの名前
* \return スクリプト内でreturnされた値
*/
AnyPtr load_and_save(const StringPtr& file_name);

//@}

CodePtr source(const char_t* src, int_t size);

#endif

CodePtr compiled_source(const void* src, int_t size);

}

#endif // XTAL_ENVIRONMENT_H_INCLUDE_GUARD
