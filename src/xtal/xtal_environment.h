
#pragma once

namespace xtal{

template<class T>
struct CppClassSymbol{
	static int_t value;
};

template<class T>
int_t CppClassSymbol<T>::value;

class Environment{
public:

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
	const ClassPtr& new_cpp_class(const StringPtr& name){
		return new_cpp_class(name, &CppClassSymbol<T>::value);
	}

	template<class T>
	const ClassPtr& new_cpp_singleton(){
		ClassPtr& p = cpp_class_map_[&CppClassSymbol<T>::value];
		if(!p){ p = xnew<T>();}
		return p;
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

	void initialize();

	void uninitialize();

	ClassPtr Iterator_;
	ClassPtr Iterable_;
	ClassPtr builtin_;
	ClassPtr lib_;

	const ClassPtr& Iterator(){
		return Iterator_;
	}

	const ClassPtr& Iterable(){
		return Iterable_;
	}

	const ClassPtr& builtin(){
		return builtin_;
	}

	const ClassPtr& lib(){
		return lib_;
	}

	ArrayPtr vm_list_;

	VMachinePtr vm_take_over();

	void vm_take_back(const VMachinePtr& vm);

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
			std::printf("MemberCacheTable hit count=%d, miss count=%d, hit rate=%g, miss rate=%g\n", hit_, miss_, hit_/(float)(hit_+miss_), miss_/(float)(hit_+miss_));
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
			std::printf("IsInheritedCacheTable hit count=%d, miss count=%d, hit rate=%g, miss rate=%g\n", hit_, miss_, hit_/(float)(hit_+miss_), miss_/(float)(hit_+miss_));
		}

		bool cache_is(const Any& target_class, const Any& klass, uint_t global_mutate_count);

		bool cache_is_inherited(const Any& target_class, const Any& klass, uint_t global_mutate_count);
	};

	MemberCacheTable member_cache_table_;
	IsInheritedCacheTable is_cache_table_;
	IsInheritedCacheTable is_inherited_cache_table_;
	uint_t global_mutate_count_;

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

	AnyPtr string_mgr_;

////////////////////////////////////

};

Environment* environment();

void set_environment(Environment* e);

/**
* @brief T形をxtalで扱えるクラスを生成し、登録する。
* 既に生成されている場合、生成済みのクラスを返す。
*/
template<class T>
const ClassPtr& new_cpp_class(const StringPtr& name){
	return environment()->new_cpp_class<T>(name);
}

/**
* @brief T形をxtalで扱えるクラスを生成し、登録する。
* 既に生成されている場合、生成済みのクラスを返す。
*/
template<class T>
const ClassPtr& new_cpp_singleton(){
	return environment()->new_cpp_singleton<T>();
}

/**
* @brief 既にnew_cpp_class<T>()で生成させれているか調べる。
*/
template<class T>
bool exists_cpp_class(){
	return environment()->exists_cpp_class<T>();
}

/**
* @brief new_cpp_class<T>()で生成されたクラスを取得する。
*/
template<class T>
const ClassPtr& get_cpp_class(){
	return environment()->get_cpp_class<T>();
}

/**
* @brief get_cpp_class<T>などで返されるクラスを設定する。
*/
template<class T>
void set_cpp_class(const ClassPtr& cls){
	return environment()->set_cpp_class<T>(cls);
}

/**
* @brief Iteratorクラスを返す
*/
inline const ClassPtr& Iterator(){
	return environment()->Iterator();
}

/**
* @brief Iterableクラスを返す
*/
inline const ClassPtr& Iterable(){
	return environment()->Iterable();
}

/**
* @brief builtinシングルトンクラスを返す
*/
inline const ClassPtr& builtin(){
	return environment()->builtin();
}

/**
* @brief libクラスを返す
*/
inline const ClassPtr& lib(){
	return environment()->lib();
}

const StreamPtr& stdin_stream();

const StreamPtr& stdout_stream();

const StreamPtr& stderr_stream();

const ClassPtr& RuntimeError();

const ClassPtr& CompileError();

const ClassPtr& UnsupportedError();

const ClassPtr& ArgumentError();

inline void inc_global_mutate_count(){
	environment()->inc_global_mutate_count();
}

const IDPtr& intern_literal(const char_t* str);
const IDPtr& intern(const char_t* str);
const IDPtr& intern(const char_t* str, uint_t data_size);
const IDPtr& intern(const char_t* str, uint_t data_size, uint_t hash, uint_t length);

}
