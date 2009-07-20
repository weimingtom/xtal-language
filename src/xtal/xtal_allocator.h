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

	enum{
		ONE_SIZE = sizeof(data_t)*2
	};

	std::size_t calc_size(std::size_t block_size){
		return 128/block_size + 4;
	}

private:

	data_t* free_data_;
	Chunk* chunk_;
	uint_t all_count_;
	uint_t used_count_;

	uint_t gc_count_;

	bool cant_fit_;

public:

	FixedAllocator();

	void* malloc(std::size_t block_size){
		if(free_data_){
			void* ret = free_data_;
			free_data_ = static_cast<data_t*>(*free_data_);
			++used_count_;
			return ret;
		}
		return malloc_inner(block_size);
	}

	void free(void* mem, std::size_t block_size){
		/*
		data_t* m = static_cast<data_t*>(mem);
		data_t** p = &free_data_;
		while(true){
			if(*p==0 || *p>m){
				break;
			}
			p = &(data_t*&)(**p);
		}
		
		*m = *p;
		*p = m;
		--used_count_;
		*/

		*static_cast<data_t*>(mem) = free_data_;
		free_data_ = static_cast<data_t*>(mem);
		--used_count_;
	}

	void release(std::size_t block_size);

	void fit(std::size_t block_size);

	void print(std::size_t block_size);

private:

	void* malloc_inner(std::size_t block_size);

	void add_chunk(std::size_t block_size);

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
		POOL_SIZE = 8,
		ONE_SIZE = FixedAllocator::ONE_SIZE,
		HANDLE_MAX_SIZE = POOL_SIZE*ONE_SIZE
	};

public:

	SmallObjectAllocator(){}
	
	void* malloc(std::size_t size){
		XTAL_ASSERT(size<=HANDLE_MAX_SIZE);

		std::size_t wsize = align(size, ONE_SIZE)/ONE_SIZE;
		if(wsize==0){ return 0; }
		return pool_[wsize-1].malloc(wsize);
	}

	void free(void* p, std::size_t size){
		XTAL_ASSERT(size<=HANDLE_MAX_SIZE);
		if(std::size_t wsize = align(size, ONE_SIZE)/ONE_SIZE){
			pool_[wsize-1].free(p, wsize);
		}
	}

	void release();

	void fit();

	void print();

private:

	FixedAllocator pool_[POOL_SIZE];

	XTAL_DISALLOW_COPY_AND_ASSIGN(SmallObjectAllocator);
};

#endif

}//namespace 

#endif // XTAL_ALLOCATOR_H_INCLUDE_GUARD
