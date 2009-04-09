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
class FixedAllocator{
public:

	typedef void* data_t;	

	struct Chunk{
		data_t* head;
		data_t* tail;
		uint_t count;
		Chunk* next;

		data_t* buf(){
			return reinterpret_cast<data_t*>(this+1);
		}
	};

	size_t calc_size(size_t block_size){
		return 128/block_size + 4;
	}

private:

	data_t* free_data_;
	Chunk* chunk_;
	uint_t all_count_;
	uint_t used_count_;

	bool cant_fit_;

public:

	FixedAllocator();

	void* malloc(size_t block_size);

	void free(void* p, size_t block_size);

	void release(size_t block_size);

	void fit(size_t block_size);

private:

	void add_chunk(size_t block_size);

	XTAL_DISALLOW_COPY_AND_ASSIGN(FixedAllocator);
};

/**
* \internal
* \breif 小さいサイズのメモリアロケータ
*/
class SmallObjectAllocator{	

	typedef FixedAllocator::data_t data_t;

public:

	enum{
		POOL_SIZE = 16,
		HANDLE_MAX_SIZE = POOL_SIZE*sizeof(data_t)
	};

public:

	SmallObjectAllocator(){}
	
	void* malloc(size_t size);

	void free(void* p, size_t size);

	void release();

	void fit();

private:

	FixedAllocator pool_[POOL_SIZE];

	XTAL_DISALLOW_COPY_AND_ASSIGN(SmallObjectAllocator);
};

#endif

}//namespace 

#endif // XTAL_ALLOCATOR_H_INCLUDE_GUARD
