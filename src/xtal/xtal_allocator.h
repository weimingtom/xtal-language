
#pragma once

#include <map>

namespace xtal{

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
* @brief 動的なポインタの配列を作成、拡張する関数。
*
* 一番最初の作成時は、引数全てがnullである必要がある。
* @param begin メモリの先頭
* @param end メモリの最後の一つ次
* @param current 使用中の要素の一つ次
*/
void expand_simple_dynamic_pointer_array(void**& begin, void**& end, void**& current, int addsize=1024);

/**
* @brief expand_simple_dynamic_pointer_arrayで生成した配列を、使われている実際のサイズまで縮小する。
*
* 要素が一つも無いなら(current==beginなら)完全に解放される。
* @param begin メモリの先頭
* @param end メモリの最後の一つ次
* @param current 使用中の要素の一つ次
*/
void fit_simple_dynamic_pointer_array(void**& begin, void**& end, void**& current);

/**
* @brief メモリ確保をスコープに閉じ込めるためのユーティリティクラス
*/
struct UserMallocGuard{
	UserMallocGuard():p(0){}
	UserMallocGuard(uint_t size):p(user_malloc(size)){}
	~UserMallocGuard(){ user_free(p); }
	
	void malloc(size_t size){ user_free(p); p = user_malloc(size); }

	void* get(){ return p; }

	void* release(){ void* ret = p; p = 0; return ret; }
private:
	void* p;

	XTAL_DISALLOW_COPY_AND_ASSIGN(UserMallocGuard);
};

/**
* @brief メモリ確保をスコープに閉じ込めるためのユーティリティクラス
*/
struct SOMallocGuard{
	SOMallocGuard():p(0){}
	SOMallocGuard(uint_t size):p(so_malloc(size)), sz(size){}
	~SOMallocGuard(){ so_free(p, sz); }
	
	void malloc(size_t size){ so_free(p, sz); p = so_malloc(size); sz = size; }

	void* get(){ return p; }

	void* release(){ void* ret = p; p = 0; return ret; }

	uint_t size(){ return sz; }
private:
	void* p;
	uint_t sz;

	XTAL_DISALLOW_COPY_AND_ASSIGN(SOMallocGuard);
};

class FixedAllocator{
public:

	typedef void* data_t;	

	struct Chunk{
		Chunk* next;
		//data_t buf_[blocks_];

		data_t* buf(){
			return reinterpret_cast<data_t*>(this+1);
		}
	};

private:

	data_t* free_data_;
	Chunk* chunk_;
	uint_t all_count_;
	uint_t used_count_;

public:

	FixedAllocator();

	void* malloc(size_t block_size);

	void free(void* p, size_t block_size);

	void release(size_t block_size);

private:

	void add_chunk(size_t block_size);

	XTAL_DISALLOW_COPY_AND_ASSIGN(FixedAllocator);
};

class SmallObjectAllocator{	

	typedef FixedAllocator::data_t data_t;

public:

	enum{
		POOL_SIZE = 32,
		HANDLE_MAX_SIZE = POOL_SIZE*sizeof(data_t)
	};

public:
	
	void* malloc(size_t size);

	void free(void* p, size_t size);

	void release();

private:

	FixedAllocator pool_[POOL_SIZE];
};

class AllocatorLib{
public:
	virtual ~AllocatorLib(){}
	virtual void initialize(){}
	virtual void* malloc(std::size_t size){ return std::malloc(size); }
	virtual void free(void* p){ std::free(p); }
};

}//namespace 
