
#pragma once

namespace xtal{

struct CoreSetting{
	ThreadLib* thread_lib;
	StreamLib* stream_lib;
	FilesystemLib* filesystem_lib;
	AllocatorLib* allocator_lib;
	ChCodeLib* chcode_lib;

	CoreSetting();
};

class Core{
public:

	Core()
		:cpp_class_map_(cpp_class_map_t::no_use_memory_t()){}

	~Core(){
		uninitialize();
	}

	void initialize(const CoreSetting& setting);

	void uninitialize();

	void debug_print();

public:

	void* user_malloc(size_t size);

	void* user_malloc_nothrow(size_t size);

	void user_free(void* p);

	void* so_malloc(size_t size);

	void so_free(void* p, size_t size);

	void print_alive_objects();

	const SmartPtr<Filesystem>& filesystem(){
		return filesystem_;
	}

	ChCodeLib* chcode_lib(){
		return setting_.chcode_lib;
	}

public:

	void expand_objects_list();

	void enable_gc();

	void disable_gc();

	void gc();

	void full_gc();

	void register_gc(RefCountingBase* p);

	void register_gc_observer(GCObserver* p);

	void unregister_gc_observer(GCObserver* p);

public:

	const ClassPtr& new_cpp_class(const StringPtr& name, void* key);

	bool exists_cpp_class(void* key){
		return cpp_class_map_.find(key)!=cpp_class_map_.end();
	}

	const ClassPtr& get_cpp_class(void* key){
		XTAL_ASSERT(exists_cpp_class(key));
		return cpp_class_map_.find(key)->second;
	}

	void set_cpp_class(const ClassPtr& cls, void* key){
		cpp_class_map_.insert(key, cls);
	}

	template<class T>
	const ClassPtr& new_cpp_class(const StringPtr& name = empty_string){
		return new_cpp_class(name, &CppClassSymbol<T>::value);
	}

	template<class T>
	const SmartPtr<T>& new_cpp_singleton(){
		ClassPtr& p = cpp_class_map_[&CppClassSymbol<T>::value];
		if(!p){ p = xnew<T>(); }
		return unchecked_ptr_cast<T>(p);
	}

	template<class T>
	bool exists_cpp_class(){
		return exists_cpp_class(&CppClassSymbol<T>::value);
	}

	template<class T>
	const ClassPtr& get_cpp_class(){
		return get_cpp_class(&CppClassSymbol<T>::value);
	}

	template<class T>
	void set_cpp_class(const ClassPtr& cls){
		set_cpp_class(cls, &CppClassSymbol<T>::value);
	}

	const ClassPtr& Iterator(){
		return Iterator_;
	}

	const ClassPtr& Iterable(){
		return Iterable_;
	}

	const ClassPtr& builtin(){
		return builtin_;
	}

	const LibPtr& lib(){
		return lib_;
	}

	VMachinePtr vm_take_over();

	void vm_take_back(const VMachinePtr& vm);

	const AnyPtr& cache_member(const Any& target_class, const IDPtr& primary_key, const Any& secondary_key, const Any& self, bool inherited_too){
		return member_cache_table_.cache(target_class, primary_key, secondary_key, self, inherited_too, global_mutate_count_);
	}

	bool cache_is(const Any& target_class, const Any& klass){
		return is_cache_table_.cache_is(target_class, klass, global_mutate_count_);
	}

	bool cache_is_inherited(const Any& target_class, const Any& klass){
		return is_inherited_cache_table_.cache_is_inherited(target_class, klass, global_mutate_count_);
	}

	void inc_global_mutate_count(){
		global_mutate_count_++;
	}

	const SmartPtr<StringMgr>& string_mgr(){
		return string_mgr_;
	}

	const SmartPtr<ThreadMgr>& thread_mgr(){
		return thread_mgr_;
	}

public:

	enum{
		id_op_call,
		id_op_pos,
		id_op_neg,
		id_op_com,
		id_op_at,
		id_op_set_at,
		id_op_range,
		id_op_add,
		id_op_cat,
		id_op_sub,
		id_op_mul,
		id_op_div,
		id_op_mod,
		id_op_and,
		id_op_or,
		id_op_xor,
		id_op_shl,
		id_op_shr,
		id_op_ushr,
		id_op_eq,
		id_op_lt,
		id_op_in,
		id_op_inc,
		id_op_dec,
		id_op_add_assign,
		id_op_sub_assign,
		id_op_cat_assign,
		id_op_mul_assign,
		id_op_div_assign,
		id_op_mod_assign,
		id_op_and_assign,
		id_op_or_assign,
		id_op_xor_assign,
		id_op_shl_assign,
		id_op_shr_assign,
		id_op_ushr_assign,

		id_op_MAX
	};

	const IDPtr* id_op_list(){
		return id_op_list_;
	}

private:

	void bind();
	void exec_script();
	
private:

	SmallObjectAllocator so_alloc_;
	CoreSetting setting_;

	RefCountingBase** objects_begin_ ;
	RefCountingBase** objects_current_;
	RefCountingBase** objects_end_;

	RefCountingBase*** objects_list_begin_;
	RefCountingBase*** objects_list_current_;
	RefCountingBase*** objects_list_end_;

	GCObserver** gcobservers_begin_;
	GCObserver** gcobservers_current_;
	GCObserver** gcobservers_end_;

	uint_t objects_count_;

	uint_t cycle_count_;

	IDPtr id_op_list_[id_op_MAX];

	struct KeyFun{
		static uint_t hash(const void* p){
			return reinterpret_cast<uint_t>(p)>>3;
		}

		static bool eq(const void* a, const void* b){
			return a==b;
		}
	};

	typedef Hashtable<void*, ClassPtr, KeyFun> cpp_class_map_t;
	cpp_class_map_t cpp_class_map_;

	SmartPtr<Filesystem> filesystem_;

	ClassPtr Iterator_;
	ClassPtr Iterable_;
	ClassPtr builtin_;
	LibPtr lib_;

	ArrayPtr vm_list_;

	struct MemberCacheTable{
		struct Unit{
			uint_t mutate_count;
			uint_t target_class;
			uint_t secondary_key;
			uint_t primary_key;
			Any member;
		};

		enum{ CACHE_MAX = 257, CACHE_MASK = CACHE_MAX-1 };

		static uint_t calc_index(uint_t hash){
			return hash % CACHE_MAX;
		}

		Unit table_[CACHE_MAX];

		int_t hit_;
		int_t miss_;

		MemberCacheTable(){
			hit_ = 0;
			miss_ = 0;
		}

		int_t hit_count(){
			return hit_;
		}

		int_t miss_count(){
			return miss_;
		}

		const AnyPtr& cache(const Any& target_class, const IDPtr& primary_key, const Any& secondary_key, const Any& self, bool inherited_too, uint_t global_mutate_count);
	};

	struct IsInheritedCacheTable{
		struct Unit{
			uint_t mutate_count;
			uint_t target_class;
			uint_t klass;
			bool result;
		};

		enum{ CACHE_MAX = 64, CACHE_MASK = CACHE_MAX-1 };

		Unit table_[CACHE_MAX];

		int_t hit_;
		int_t miss_;

		IsInheritedCacheTable(){
			hit_ = 0;
			miss_ = 0;
		}

		int_t hit_count(){
			return hit_;
		}

		int_t miss_count(){
			return miss_;
		}

		bool cache_is(const Any& target_class, const Any& klass, uint_t global_mutate_count);

		bool cache_is_inherited(const Any& target_class, const Any& klass, uint_t global_mutate_count);
	};

	MemberCacheTable member_cache_table_;
	IsInheritedCacheTable is_cache_table_;
	IsInheritedCacheTable is_inherited_cache_table_;
	uint_t global_mutate_count_;

	SmartPtr<StringMgr> string_mgr_;
	SmartPtr<ThreadMgr> thread_mgr_;
};

Core* core();

void set_core(Core* e);


/**
* @brief ユーザーが登録したメモリアロケート関数を使ってメモリ確保する。
*
* メモリ確保失敗は例外で返される。
*/
inline void* user_malloc(size_t size){
	return core()->user_malloc(size);
}

/**
* @brief ユーザーが登録したメモリアロケート関数を使ってメモリ確保する。
*
* メモリ確保失敗はNULL値で返される。
*/
inline void* user_malloc_nothrow(size_t size){
	return core()->user_malloc_nothrow(size);
}

/**
* @brief ユーザーが登録したメモリデアロケート関数を使ってメモリ解放する。
*
*/
inline void user_free(void* p){
	return core()->user_free(p);
}

/**
* @brief 小さいオブジェクト用にメモリをアロケートする。
*/
inline void* so_malloc(size_t size){
	return core()->so_malloc(size);
}

/**
* @brief 小さいオブジェクト用のメモリを解放する。
*/
inline void so_free(void* p, size_t size){
	return core()->so_free(p, size);
}

/**
* @brief ガーベジコレクションを実行する
*
* さほど時間はかからないが、完全にゴミを解放できないガーベジコレクト関数
* 例えば循環参照はこれでは検知できない。
* ゲームで使用する場合、毎フレームこれを呼ぶことを推奨する。
*/
inline void gc(){
	return core()->gc();
}

/**
* @brief 循環オブジェクトも解放するフルガーベジコレクションを実行する
*
* 時間はかかるが、現在ゴミとなっているものはなるべく全て解放するガーベジコレクト関数
* 循環参照も検知できる。
* ゲームで使用する場合、シーンの切り替え時など、節目節目に呼ぶことを推奨する。
*/
inline void full_gc(){
	return core()->full_gc();
}

/**
* @brief ガーベジコレクションを無効化する
*
* gcやfull_gcの呼び出しを無効化する関数。
* 内部でこれが何回呼び出されたか記憶されており、呼び出した回数enable_gcを呼びないとガーベジコレクションは有効にならない
*/
inline void disable_gc(){
	return core()->disable_gc();
}

/**
* @brief ガーベジコレクションを有効化する
*
* disable_gcが呼ばれた回数と同じだけ呼び出すとガーベジコレクションが有効になる
* 
*/
inline void enable_gc(){
	return core()->enable_gc();
}

/**
* @brief T形をxtalで扱えるクラスを生成し、登録する。
* 既に生成されている場合、生成済みのクラスを返す。
*/
template<class T>
const ClassPtr& new_cpp_class(const StringPtr& name){
	return core()->new_cpp_class<T>(name);
}

/**
* @brief T形をxtalで扱えるクラスを生成し、登録する。
* 既に生成されている場合、生成済みのクラスを返す。
*/
template<class T>
const SmartPtr<T>& new_cpp_singleton(){
	return core()->new_cpp_singleton<T>();
}

/**
* @brief 既にnew_cpp_class<T>()で生成させれているか調べる。
*/
template<class T>
bool exists_cpp_class(){
	return core()->exists_cpp_class<T>();
}

/**
* @brief new_cpp_class<T>()で生成されたクラスを取得する。
*/
template<class T>
const ClassPtr& get_cpp_class(){
	return core()->get_cpp_class<T>();
}

/**
* @brief get_cpp_class<T>などで返されるクラスを設定する。
*/
template<class T>
void set_cpp_class(const ClassPtr& cls){
	return core()->set_cpp_class<T>(cls);
}

/**
* @brief Iteratorクラスを返す
*/
inline const ClassPtr& Iterator(){
	return core()->Iterator();
}

/**
* @brief Iterableクラスを返す
*/
inline const ClassPtr& Iterable(){
	return core()->Iterable();
}

/**
* @brief builtinシングルトンクラスを返す
*/
inline const ClassPtr& builtin(){
	return core()->builtin();
}

/**
* @brief libクラスを返す
*/
inline const LibPtr& lib(){
	return core()->lib();
}

const StreamPtr& stdin_stream();

const StreamPtr& stdout_stream();

const StreamPtr& stderr_stream();

const ClassPtr& RuntimeError();

const ClassPtr& CompileError();

const ClassPtr& UnsupportedError();

const ClassPtr& ArgumentError();

inline void inc_global_mutate_count(){
	core()->inc_global_mutate_count();
}

inline const IDPtr& intern_literal(const char_t* str){
	return core()->string_mgr()->insert_literal(str);
}

inline const IDPtr& intern(const char_t* str){
	return core()->string_mgr()->insert(str);
}

inline const IDPtr& intern(const char_t* str, uint_t data_size){
	return core()->string_mgr()->insert(str, data_size);
}

inline const IDPtr& intern(const char_t* str, uint_t data_size, uint_t hash){
	return core()->string_mgr()->insert(str, data_size, hash);
}

inline AnyPtr interned_strings(){
	return core()->string_mgr()->interned_strings();
}

inline bool thread_enabled(){
	return core()->thread_mgr()->thread_enabled();
}

inline void yield_thread(){
	return core()->thread_mgr()->yield_thread();
}

inline void sleep_thread(float_t sec){
	return core()->thread_mgr()->sleep_thread(sec);
}

inline ThreadPtr new_thread(const AnyPtr& callback_fun){
	return core()->thread_mgr()->new_thread(callback_fun);
}

inline MutexPtr new_mutex(){
	return core()->thread_mgr()->new_mutex();
}

inline void lock_mutex(const MutexPtr& p){
	return core()->thread_mgr()->lock_mutex(p);
}

/**
* @brief VMachinePtrオブジェクトを返す
*
* グローバルなVMachinePtrオブジェクトを返す。
* スレッド毎にこのグローバルVMachinePtrオブジェクトは存在する。
*/
inline const VMachinePtr& vmachine(){
	return core()->thread_mgr()->vmachine();
}

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

}
