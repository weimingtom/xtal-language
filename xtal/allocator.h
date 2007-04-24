
#pragma once

#include <malloc.h>
#include <stddef.h>
#include <string>

namespace xtal{

/** @addtogroup memory */
/*@{*/

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
void user_free(void* p, size_t size);

/**
* @brief 使用するメモリアロケート関数を設定する。
*
*/
void set_user_malloc(void* (*malloc)(size_t), void (*free)(void*));


/**
* @brief xtalで使用するメモリーを設定する。
*
*/
void set_memory(void* memory, size_t size);

/**
* 動的な、ポインタの配列を作成、拡張する関数。
*
* 一番最初の作成時は、引数全てがnullである必要がある。
* @param begin メモリの先頭
* @param end メモリの最後の一つ次
* @param current 使用中の要素の一つ次
*/
void expand_simple_dynamic_pointer_array(void**& begin, void**& end, void**& current, int addsize=1024);

/**
* expand_simple_dynamic_pointer_arrayで生成した配列を、使われている実際のサイズまで縮小する。
*
* 要素が一つも無いなら(current==beginなら)完全に解放される。
* @param begin メモリの先頭
* @param end メモリの最後の一つ次
* @param current 使用中の要素の一つ次
*/
void fit_simple_dynamic_pointer_array(void**& begin, void**& end, void**& current);

/*@}*/


/**
* @brief user_malloc, user_freeを使う、STLの要件に適合したアロケータクラス。
*
*/
template<class T>
struct Alloc{

    typedef size_t size_type;
    typedef ptrdiff_t difference_type;
    typedef T* pointer;
    typedef const T* const_pointer;
    typedef T &reference;
    typedef const T &const_reference;
    typedef T value_type;

	template<class U>
	struct rebind{
		typedef Alloc<U> other;
    };

    Alloc(){}

	template<class U>
	Alloc(const Alloc<U>&){}

	template<class U>
	Alloc<T>& operator=(const Alloc<U>&){
		return* this;
	}

	Alloc(const Alloc<T>&){}

    Alloc<T>& operator=(const Alloc<T>&){
		return* this;
	}

    pointer address(reference x) const{
		return &x;
	}

    const_pointer address(const_reference x) const{
		return &x;
	}

    pointer allocate(size_type n, const void* hint = 0){
		return static_cast<pointer>(user_malloc(sizeof(T)*n));
	}

    void deallocate(pointer p, size_type n){
		user_free(p, sizeof(T)*n);
    }
	
	void construct(pointer p, const T& val){
		new(p) T(val);
	}

    void destroy(pointer p){
		p->~T();
	}
	
    size_type max_size() const{
		return 0xffffffffU/sizeof(T);
	}

};

template<class T, class U> 
inline bool operator==(const Alloc<T>&, const Alloc<U>&){	
	return true;
}

template<class T, class U> 
inline bool operator!=(const Alloc<T>&, const Alloc<U>&){	
	return false;
}

template<>
struct Alloc<void> {
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;
    typedef void* pointer;
    typedef const void* const_pointer;
    typedef void value_type;

	template<class U>
	struct rebind{
		typedef Alloc<U> other;
    };

    Alloc(){}

	Alloc(const Alloc<void> &){}

    Alloc<void>& operator=(const Alloc<void>&){
		return* this;
	}

	template<class U>
	Alloc(const Alloc<U>&){}

	template<class U>
	Alloc<void>& operator=(const Alloc<U>&){
		return* this;
	}
};

/*
* スタック型のメモリアロケータ。
* STLのアロケータには適合しない。
*/
class RegionAlloc{
	char* begin_;
	char* pos_;
	char* end_;
	size_t alloced_size_;
public:

	RegionAlloc(size_t first_buffer_size = 1024);

	~RegionAlloc();

	void* allocate(size_t size);

	void release();

private:

	void add_chunk(size_t minsize);

private:

	RegionAlloc(const RegionAlloc&);
	RegionAlloc& operator =(const RegionAlloc&);

};

/**
* @brief とてもシンプルなメモリ管理機構
*
*/
class SimpleMemoryManager{
public:
	
	struct Chunk{
		Chunk* next;
		Chunk* prev;
		int used;
		
		size_t size(){ return (char*)next - (char*)buf(); }
		void* buf(){ return this+1; }
	};

	SimpleMemoryManager(){
		head_ = begin_ = end_ = 0;
	}
	
	void init(void* begin, void* end);
	
	void* malloc(size_t size);
	
	void free(void* p);

	Chunk* begin(){
		return begin_;
	}
	
	Chunk* end(){
		return end_;
	}

	Chunk* to_chunk(void* p){
		return (Chunk*)p-1;
	}

	void add_ref(void* p){
		to_chunk(p)->used++;
	}
	
private:
	Chunk* head_;
	Chunk* begin_;
	Chunk* end_;
};

}//namespace 
