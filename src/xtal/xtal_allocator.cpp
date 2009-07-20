 #include "xtal.h"

namespace xtal{

void expand_simple_dynamic_pointer_array(void*** begin, void*** end, void*** current, int addsize){
	uint_t size = (uint_t)(*end-*begin);
	uint_t newsize = size+addsize+size;
	void** newbegin=(void**)xmalloc(sizeof(void*)*newsize);
	std::memcpy(newbegin, *begin, sizeof(void*)*size);
	*end = newbegin+newsize;
	*current = newbegin+(*current-*begin);
	xfree(*begin, sizeof(void*)*size);
	*begin = newbegin;	
}

void fit_simple_dynamic_pointer_array(void*** begin, void*** end, void*** current){
	uint_t size = (uint_t)(*end-*begin);
	uint_t newsize = (uint_t)(*current-*begin);
	if(newsize){
		void** newbegin=(void**)xmalloc(sizeof(void*)*newsize);
		std::memcpy(newbegin, *begin, sizeof(void*)*newsize);
		*end = newbegin+newsize;
		*current = newbegin+newsize;
		xfree(*begin, sizeof(void*)*size);
		*begin = newbegin;	
	}
	else{
		xfree(*begin, sizeof(void*)*size);
		*begin = *end = *current = 0;
	}
}

///////////////////////////////////////

#ifndef XTAL_NO_SMALL_ALLOCATOR

FixedAllocator::FixedAllocator(){
	chunk_ = 0;
	free_data_ = 0;

	all_count_ = 0;
	used_count_ = 0;

	gc_count_ = 0;

	cant_fit_ = false;
}

void FixedAllocator::add_chunk(std::size_t block_size){
	uint_t blocks = calc_size(block_size);
	Chunk* new_chunk = (Chunk*)xmalloc(sizeof(Chunk)+block_size*ONE_SIZE*blocks);
	all_count_ += blocks;
	
	{
		new_chunk->next = 0;
		data_t* p = new_chunk->buf();
		for(uint_t i=0; i<blocks-1; ++i){
			data_t* next_block = (data_t*)((u8*)p+block_size*ONE_SIZE);
			*p = next_block;
			p = next_block;
		}
		*p = free_data_;
		free_data_ = new_chunk->buf();
	}

	new_chunk->next = chunk_;
	chunk_ = new_chunk;
}

void* FixedAllocator::malloc_inner(std::size_t block_size){
	uint_t blocks = calc_size(block_size);

	gc_count_ = (gc_count_ + 1) & 0x8;
	if(gc_count_==0){
		gc();
	}

	cant_fit_ = true;

	if(all_count_-used_count_<blocks){
		add_chunk(block_size);
	}

	{
		void* ret = free_data_;
		free_data_ = static_cast<data_t*>(*free_data_);
		cant_fit_ = false;
		++used_count_;
		return ret;
	}
}


void FixedAllocator::fit(std::size_t block_size){
	if(cant_fit_){
		return;
	}

	uint_t blocks = calc_size(block_size);

	for(Chunk* p=chunk_; p; p=p->next){
		p->head = 0;
		p->tail = 0;
		p->count = 0;
	}

	uint_t buffer_size = sizeof(Chunk)+block_size*blocks*ONE_SIZE;
	for(data_t* q=free_data_; q;){
		data_t* next = static_cast<data_t*>(*q);
		for(Chunk* p=chunk_; p; p=p->next){
			if((u8*)p<(u8*)q && (u8*)q<(u8*)p+buffer_size){
				p->count++;
				if(!p->tail){
					p->tail = q;	
				}
				*q = p->head;
				p->head = q;
				break;
			}
		}
		q = next;
	}
	free_data_ = 0;

	Chunk** prev = &chunk_;
	for(Chunk* p=chunk_; p;){
		if(p->count==blocks){
			*prev = p->next;
			xfree(p, buffer_size);
			p = *prev;
			all_count_ -= blocks;
		}
		else{
			prev = &p->next;
			p = p->next;
		}
	}

	for(Chunk* p=chunk_; p; p=p->next){
		if(p->head){
			*p->tail = free_data_;
			free_data_ = p->head;
		}
	}
}
	
void FixedAllocator::release(std::size_t block_size){
	/*
	fit(block_size);

	if(!ignore_memory_assert()){
		XTAL_ASSERT(chunk_==0);
		XTAL_ASSERT(free_data_==0);
	}
	*/

	uint_t blocks = calc_size(block_size);
	uint_t buffer_size = sizeof(Chunk)+block_size*blocks*ONE_SIZE;
	for(Chunk* p=chunk_; p; ){
		Chunk* next = p->next;
		xfree(p, buffer_size);
		p = next;
	}
	
	chunk_ = 0;
	free_data_ = 0;
}

void FixedAllocator::print(std::size_t block_size){
#ifdef XTAL_DEBUG_PRINT
	printf("sm %d, used=%d=%gKB, free=%d=%gKB\n", block_size*ONE_SIZE,
		used_count_, (used_count_*block_size*ONE_SIZE)/1024.0f, 
		all_count_-used_count_, ((all_count_-used_count_)*block_size*ONE_SIZE)/1024.0f);
#endif
}

void SmallObjectAllocator::fit(){
	return;
	for(int i=0; i<POOL_SIZE; ++i){
		pool_[i].fit(i+1);
	}	
}

void SmallObjectAllocator::release(){
	for(int i=0; i<POOL_SIZE; ++i){
		pool_[i].release(i+1);
	}	
}
	
void SmallObjectAllocator::print(){
	for(int i=0; i<POOL_SIZE; ++i){
		pool_[i].print(i+1);
	}
}

#endif

}
