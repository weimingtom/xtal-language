 #include "xtal.h"

namespace xtal{

void expand_simple_dynamic_pointer_array(void**& begin, void**& end, void**& current, int addsize){
	uint_t size = (uint_t)(end-begin);
	uint_t newsize = size+addsize+size;
	void** newbegin=(void**)user_malloc(sizeof(void*)*newsize);
	std::memcpy(newbegin, begin, sizeof(void*)*size);
	end = newbegin+newsize;
	current = newbegin+(current-begin);
	user_free(begin);
	begin = newbegin;	
}

void fit_simple_dynamic_pointer_array(void**& begin, void**& end, void**& current){
	//uint_t size = (uint_t)(end-begin);
	uint_t newsize = (uint_t)(current-begin);
	if(newsize){
		void** newbegin=(void**)user_malloc(sizeof(void*)*newsize);
		std::memcpy(newbegin, begin, sizeof(void*)*newsize);
		end = newbegin+newsize;
		current = newbegin+newsize;
		user_free(begin);
		begin = newbegin;	
	}
	else{
		user_free(begin);
		begin = end = current = 0;
	}
}

///////////////////////////////////////

FixedAllocator::FixedAllocator(){
	chunk_ = 0;
	free_data_ = 0;

	all_count_ = 0;
	used_count_ = 0;
}

void FixedAllocator::add_chunk(size_t block_size){
	uint_t blocks = (256/block_size)+4;
	all_count_ += blocks;
	Chunk *new_chunk = (Chunk*)user_malloc(sizeof(Chunk)+block_size*blocks*sizeof(data_t));
	
	{
		new_chunk->next = 0;
		data_t* p = new_chunk->buf();
		for(uint_t i=0; i<blocks-1; ++i){
			data_t* next_block = p+block_size;
			*p = next_block;
			p = next_block;
		}
		*p = free_data_;
		free_data_ = new_chunk->buf();
	}

	new_chunk->next = chunk_;
	chunk_ = new_chunk;
}

void* FixedAllocator::malloc(size_t block_size){
	++used_count_;

	if(free_data_){
		void* ret = free_data_;
		free_data_ = static_cast<data_t*>(*free_data_);
		return ret;
	}

	gc();
	gc();

	if(used_count_>(all_count_>>1)){
		add_chunk(block_size);
	}

	{
		void* ret = free_data_;
		free_data_ = static_cast<data_t*>(*free_data_);
		return ret;
	}
}

void FixedAllocator::free(void* mem, size_t block_size){
	*static_cast<data_t*>(mem) = free_data_;
	free_data_ = static_cast<data_t*>(mem);
	--used_count_;
}
	
void FixedAllocator::release(size_t block_size){
	for(Chunk* p=chunk_; p; ){
		Chunk* next = p->next;
		user_free(p);
		p = next;
	}
	
	chunk_ = 0;
	free_data_ = 0;
}

void* SmallObjectAllocator::malloc(size_t size){
	if(size>HANDLE_MAX_SIZE){
		return user_malloc(size);
	}else{
		size_t wsize = align(size, sizeof(data_t))/sizeof(data_t);
		if(wsize==0){ return 0; }
		return pool_[wsize-1].malloc(wsize);
	}
}

void SmallObjectAllocator::free(void* p, size_t size){
	if(size>HANDLE_MAX_SIZE){
		user_free(p);
	}else{
		if(p==0){
			return;
		}
		if(size_t wsize = align(size, sizeof(data_t))/sizeof(data_t)){
			pool_[wsize-1].free(p, wsize);
		}
	}
}

void SmallObjectAllocator::release(){
	for(int i=0; i<POOL_SIZE; ++i){
		pool_[i].release(i+1);
	}	
}

}
