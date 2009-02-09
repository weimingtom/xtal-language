
#pragma once

namespace xtal{

struct CoreSetting{
	ThreadLib* thread_lib;
	StreamLib* stream_lib;
	FilesystemLib* filesystem_lib;
	AllocatorLib* allocator_lib;

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

public:

	void* user_malloc(size_t size);

	void* user_malloc_nothrow(size_t size);

	void user_free(void* p);

	void* so_malloc(size_t size);

	void so_free(void* p, size_t size);

	const SmartPtr<Filesystem>& filesystem(){
		return filesystem_;
	}

public:

	void enable_gc();

	void disable_gc();

	void gc();

	void full_gc();

	void register_gc(Base* p);

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

	Base** objects_begin_ ;
	Base** objects_current_;
	Base** objects_end_;

	Base*** objects_list_begin_;
	Base*** objects_list_current_;
	Base*** objects_list_end_;

	GCObserver** gcobservers_begin_;
	GCObserver** gcobservers_current_;
	GCObserver** gcobservers_end_;

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
			Any primary_key;
			Any member;
		};

		enum{ CACHE_MAX = 256, CACHE_MASK = CACHE_MAX-1 };

		Unit table_[CACHE_MAX];

		int_t hit_;
		int_t miss_;

		MemberCacheTable(){
			hit_ = 0;
			miss_ = 0;
		}

		void print_result(){
			//std::printf("MemberCacheTable hit count=%d, miss count=%d, hit rate=%g, miss rate=%g\n", hit_, miss_, hit_/(float)(hit_+miss_), miss_/(float)(hit_+miss_));
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

		void print_result(){
			//std::printf("IsInheritedCacheTable hit count=%d, miss count=%d, hit rate=%g, miss rate=%g\n", hit_, miss_, hit_/(float)(hit_+miss_), miss_/(float)(hit_+miss_));
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

inline const IDPtr& intern(const char_t* str, uint_t data_size, uint_t hash, uint_t length){
	return core()->string_mgr()->insert(str, data_size, hash, length);
}

inline AnyPtr interned_strings(){
	return core()->string_mgr()->interned_strings();
}

inline bool thread_enabled(){
	return core()->thread_mgr()->thread_enabled();
}

inline void check_yield_thread(){
	return core()->thread_mgr()->check_yield_thread();
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

/**
* @brief interactive xtalの実行
*/
void ix();

CodePtr source(const char_t* src, int_t size, const char* file);

#endif

CodePtr compiled_source(const void* src, int_t size, const char* file);

/////////////////////////////

//{REPEAT{{
/*
/// @brief primary_keyメソッドを呼び出す
template<class A0 #COMMA_REPEAT#class A`i+1`#>
inline AnyPtr Any::send(const IDPtr& primary_key, const A0& a0 #COMMA_REPEAT#const A`i+1`& a`i+1`#) const{
	const VMachinePtr& vm = vmachine();
	vm->setup_call(1, a0 #COMMA_REPEAT#a`i+1`#);
	rawsend(vm, primary_key);
	return vm->result_and_cleanup_call();
}

/// @brief primary_key#secondary_keyメソッドを呼び出す
template<class A0 #COMMA_REPEAT#class A`i+1`#>
inline AnyPtr Any::send2(const IDPtr& primary_key, const AnyPtr& secondary_key, const A0& a0 #COMMA_REPEAT#const A`i+1`& a`i+1`#) const{
	const VMachinePtr& vm = vmachine();
	vm->setup_call(1, a0 #COMMA_REPEAT#a`i+1`#);
	rawsend(vm, primary_key, secondary_key);
	return vm->result_and_cleanup_call();
}

/// @brief 関数を呼び出す
template<class A0 #COMMA_REPEAT#class A`i+1`#>
inline AnyPtr Any::call(const A0& a0 #COMMA_REPEAT#const A`i+1`& a`i+1`#) const{
	const VMachinePtr& vm = vmachine();
	vm->setup_call(1, a0 #COMMA_REPEAT#a`i+1`#);
	rawcall(vm);
	return vm->result_and_cleanup_call();
}
*/

/// @brief primary_keyメソッドを呼び出す
template<class A0 >
inline AnyPtr Any::send(const IDPtr& primary_key, const A0& a0 ) const{
	const VMachinePtr& vm = vmachine();
	vm->setup_call(1, a0 );
	rawsend(vm, primary_key);
	return vm->result_and_cleanup_call();
}

/// @brief primary_key#secondary_keyメソッドを呼び出す
template<class A0 >
inline AnyPtr Any::send2(const IDPtr& primary_key, const AnyPtr& secondary_key, const A0& a0 ) const{
	const VMachinePtr& vm = vmachine();
	vm->setup_call(1, a0 );
	rawsend(vm, primary_key, secondary_key);
	return vm->result_and_cleanup_call();
}

/// @brief 関数を呼び出す
template<class A0 >
inline AnyPtr Any::call(const A0& a0 ) const{
	const VMachinePtr& vm = vmachine();
	vm->setup_call(1, a0 );
	rawcall(vm);
	return vm->result_and_cleanup_call();
}

/// @brief primary_keyメソッドを呼び出す
template<class A0 , class A1>
inline AnyPtr Any::send(const IDPtr& primary_key, const A0& a0 , const A1& a1) const{
	const VMachinePtr& vm = vmachine();
	vm->setup_call(1, a0 , a1);
	rawsend(vm, primary_key);
	return vm->result_and_cleanup_call();
}

/// @brief primary_key#secondary_keyメソッドを呼び出す
template<class A0 , class A1>
inline AnyPtr Any::send2(const IDPtr& primary_key, const AnyPtr& secondary_key, const A0& a0 , const A1& a1) const{
	const VMachinePtr& vm = vmachine();
	vm->setup_call(1, a0 , a1);
	rawsend(vm, primary_key, secondary_key);
	return vm->result_and_cleanup_call();
}

/// @brief 関数を呼び出す
template<class A0 , class A1>
inline AnyPtr Any::call(const A0& a0 , const A1& a1) const{
	const VMachinePtr& vm = vmachine();
	vm->setup_call(1, a0 , a1);
	rawcall(vm);
	return vm->result_and_cleanup_call();
}

/// @brief primary_keyメソッドを呼び出す
template<class A0 , class A1, class A2>
inline AnyPtr Any::send(const IDPtr& primary_key, const A0& a0 , const A1& a1, const A2& a2) const{
	const VMachinePtr& vm = vmachine();
	vm->setup_call(1, a0 , a1, a2);
	rawsend(vm, primary_key);
	return vm->result_and_cleanup_call();
}

/// @brief primary_key#secondary_keyメソッドを呼び出す
template<class A0 , class A1, class A2>
inline AnyPtr Any::send2(const IDPtr& primary_key, const AnyPtr& secondary_key, const A0& a0 , const A1& a1, const A2& a2) const{
	const VMachinePtr& vm = vmachine();
	vm->setup_call(1, a0 , a1, a2);
	rawsend(vm, primary_key, secondary_key);
	return vm->result_and_cleanup_call();
}

/// @brief 関数を呼び出す
template<class A0 , class A1, class A2>
inline AnyPtr Any::call(const A0& a0 , const A1& a1, const A2& a2) const{
	const VMachinePtr& vm = vmachine();
	vm->setup_call(1, a0 , a1, a2);
	rawcall(vm);
	return vm->result_and_cleanup_call();
}

/// @brief primary_keyメソッドを呼び出す
template<class A0 , class A1, class A2, class A3>
inline AnyPtr Any::send(const IDPtr& primary_key, const A0& a0 , const A1& a1, const A2& a2, const A3& a3) const{
	const VMachinePtr& vm = vmachine();
	vm->setup_call(1, a0 , a1, a2, a3);
	rawsend(vm, primary_key);
	return vm->result_and_cleanup_call();
}

/// @brief primary_key#secondary_keyメソッドを呼び出す
template<class A0 , class A1, class A2, class A3>
inline AnyPtr Any::send2(const IDPtr& primary_key, const AnyPtr& secondary_key, const A0& a0 , const A1& a1, const A2& a2, const A3& a3) const{
	const VMachinePtr& vm = vmachine();
	vm->setup_call(1, a0 , a1, a2, a3);
	rawsend(vm, primary_key, secondary_key);
	return vm->result_and_cleanup_call();
}

/// @brief 関数を呼び出す
template<class A0 , class A1, class A2, class A3>
inline AnyPtr Any::call(const A0& a0 , const A1& a1, const A2& a2, const A3& a3) const{
	const VMachinePtr& vm = vmachine();
	vm->setup_call(1, a0 , a1, a2, a3);
	rawcall(vm);
	return vm->result_and_cleanup_call();
}

/// @brief primary_keyメソッドを呼び出す
template<class A0 , class A1, class A2, class A3, class A4>
inline AnyPtr Any::send(const IDPtr& primary_key, const A0& a0 , const A1& a1, const A2& a2, const A3& a3, const A4& a4) const{
	const VMachinePtr& vm = vmachine();
	vm->setup_call(1, a0 , a1, a2, a3, a4);
	rawsend(vm, primary_key);
	return vm->result_and_cleanup_call();
}

/// @brief primary_key#secondary_keyメソッドを呼び出す
template<class A0 , class A1, class A2, class A3, class A4>
inline AnyPtr Any::send2(const IDPtr& primary_key, const AnyPtr& secondary_key, const A0& a0 , const A1& a1, const A2& a2, const A3& a3, const A4& a4) const{
	const VMachinePtr& vm = vmachine();
	vm->setup_call(1, a0 , a1, a2, a3, a4);
	rawsend(vm, primary_key, secondary_key);
	return vm->result_and_cleanup_call();
}

/// @brief 関数を呼び出す
template<class A0 , class A1, class A2, class A3, class A4>
inline AnyPtr Any::call(const A0& a0 , const A1& a1, const A2& a2, const A3& a3, const A4& a4) const{
	const VMachinePtr& vm = vmachine();
	vm->setup_call(1, a0 , a1, a2, a3, a4);
	rawcall(vm);
	return vm->result_and_cleanup_call();
}

/// @brief primary_keyメソッドを呼び出す
template<class A0 , class A1, class A2, class A3, class A4, class A5>
inline AnyPtr Any::send(const IDPtr& primary_key, const A0& a0 , const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5) const{
	const VMachinePtr& vm = vmachine();
	vm->setup_call(1, a0 , a1, a2, a3, a4, a5);
	rawsend(vm, primary_key);
	return vm->result_and_cleanup_call();
}

/// @brief primary_key#secondary_keyメソッドを呼び出す
template<class A0 , class A1, class A2, class A3, class A4, class A5>
inline AnyPtr Any::send2(const IDPtr& primary_key, const AnyPtr& secondary_key, const A0& a0 , const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5) const{
	const VMachinePtr& vm = vmachine();
	vm->setup_call(1, a0 , a1, a2, a3, a4, a5);
	rawsend(vm, primary_key, secondary_key);
	return vm->result_and_cleanup_call();
}

/// @brief 関数を呼び出す
template<class A0 , class A1, class A2, class A3, class A4, class A5>
inline AnyPtr Any::call(const A0& a0 , const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5) const{
	const VMachinePtr& vm = vmachine();
	vm->setup_call(1, a0 , a1, a2, a3, a4, a5);
	rawcall(vm);
	return vm->result_and_cleanup_call();
}

//}}REPEAT}

}
