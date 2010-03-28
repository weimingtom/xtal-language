/** \file src/xtal/xtal_allocator.h
* \brief src/xtal/xtal_allocator.h
*/

#ifndef XTAL_ALLOCATOR_H_INCLUDE_GUARD
#define XTAL_ALLOCATOR_H_INCLUDE_GUARD

#pragma once

namespace xtal{

void expand_simple_dynamic_pointer_array(void*** begin, void*** end, void*** current, int addsize);

/**
* \internal
* \brief 動的なポインタの配列を作成、拡張する関数。
*
* 一番最初の作成時は、引数全てがnullである必要がある。
* \param begin メモリの先頭
* \param end メモリの最後の一つ次
* \param current 使用中の要素の一つ次
*/
template<class T>
inline void expand_simple_dynamic_pointer_array(T*** begin, T*** end, T*** current, int addsize=1024){
	expand_simple_dynamic_pointer_array((void***)begin, (void***)end, (void***)current, addsize);
}

void fit_simple_dynamic_pointer_array(void*** begin, void*** end, void*** current);

/**
* \internal
* \brief expand_simple_dynamic_pointer_arrayで生成した配列を、使われている実際のサイズまで縮小する。
*
* 要素が一つも無いなら(current==beginなら)完全に解放される。
* \param begin メモリの先頭
* \param end メモリの最後の一つ次
* \param current 使用中の要素の一つ次
*/
template<class T>
inline void fit_simple_dynamic_pointer_array(T*** begin, T*** end, T*** current){
	fit_simple_dynamic_pointer_array((void***)begin, (void***)end, (void***)current);
}

#ifndef XTAL_NO_SMALL_ALLOCATOR

/**
* \internal
* \brief 固定サイズメモリアロケータ
*/
class MemoryPool{
public:

	typedef void* data_t;

	enum{
		BLOCK_SIZE = 256,
		BLOCK_COUNT = 16,
		BLOCK_MEMORY_SIZE = BLOCK_SIZE*BLOCK_COUNT
	};

	struct Chunk{
		data_t* free_data;
		uptr_t count;
		Chunk* next;
		Chunk* prev;

		data_t* buf(){
			return (data_t*)((u8*)this - BLOCK_MEMORY_SIZE);
		}
	};

private:
	Chunk* free_chunk_;
	Chunk* full_chunk_;

public:

	MemoryPool();

	void* malloc(Chunk** chunk);

	void free(void* mem, Chunk* chunk);

	void release();

private:

	void add_chunk();

	XTAL_DISALLOW_COPY_AND_ASSIGN(MemoryPool);
};

/**
* \internal
* \brief 固定サイズメモリアロケータ
*/
class FixedAllocator{
public:

	typedef void* data_t;	

	struct Chunk{
		data_t* free_data;
		uptr_t count;
		Chunk* next;
		Chunk* prev;
		MemoryPool::Chunk* parent;
		data_t dummy;

		data_t* buf(){
			return reinterpret_cast<data_t*>(this+1);
		}
	};

	enum{
		ONE_SIZE = sizeof(data_t)
	};

private:
	Chunk* free_chunk_;
	Chunk* full_chunk_;
	
	uint_t block_size_;
	uint_t block_count_;
	
	MemoryPool* pool_;

public:

	FixedAllocator();

	void init(MemoryPool* pool, uint_t block_size);

	void* malloc();

	void free(void* mem);

	void release();

private:

	Chunk* to_chunk(void* mem){
		return (Chunk*)(void*)((uptr_t)mem & ~(MemoryPool::BLOCK_SIZE-1));
	}

	void add_chunk();

	XTAL_DISALLOW_COPY_AND_ASSIGN(FixedAllocator);
};

/**
* \internal
* \brief 小さいサイズのメモリアロケータ
*/
class SmallObjectAllocator{	

	typedef FixedAllocator::data_t data_t;

public:

	enum{
		POOL_SIZE = 10,
		ONE_SIZE = FixedAllocator::ONE_SIZE,
		ONE_SIZE_SHIFT = static_ntz<ONE_SIZE>::value,
		HANDLE_MAX_SIZE = POOL_SIZE*ONE_SIZE
	};

public:

	SmallObjectAllocator();
	
	void* malloc(std::size_t size){
		XTAL_ASSERT(size<=HANDLE_MAX_SIZE);
		std::size_t wsize = align(size, ONE_SIZE)>>ONE_SIZE_SHIFT;
		if(wsize==0){ return 0; }
		return pool_[wsize-1].malloc();
	}

	void free(void* p, std::size_t size){
		XTAL_ASSERT(size<=HANDLE_MAX_SIZE);
		if(std::size_t wsize = align(size, ONE_SIZE)>>ONE_SIZE_SHIFT){
			pool_[wsize-1].free(p);
		}
	}

	void release();

private:

	MemoryPool mpool_;
	FixedAllocator pool_[POOL_SIZE];

	XTAL_DISALLOW_COPY_AND_ASSIGN(SmallObjectAllocator);
};

#endif

}//namespace 

#endif // XTAL_ALLOCATOR_H_INCLUDE_GUARD
