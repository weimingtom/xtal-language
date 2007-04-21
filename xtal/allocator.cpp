 
#include "xtal.h"

#include "allocator.h"
#include "fwd.h"

namespace xtal{

namespace{

void* (*user_malloc_)(size_t) = &malloc;
void (*user_free_)(void*) = &free;

size_t used_user_malloc_size_ = 0;
size_t used_user_malloc_threshold_ = 1024*50;

}

void* user_malloc(size_t size){
	void* ret = user_malloc_nothrow(size);
	if(!ret){
		throw std::bad_alloc();
	}
	return ret;
} 

void* user_malloc_nothrow(size_t size){

	if(used_user_malloc_threshold_<used_user_malloc_size_){
		used_user_malloc_threshold_ += 1024*10; // 10KB‚Ù‚Çè‡’l‚ð‘‚â‚·
		gc();
	}
	
	void* ret = user_malloc_(size);
	if(!ret){
		full_gc();
		ret = user_malloc_(size);
	}
	
	if(ret){
		used_user_malloc_size_+=size;
	}
	
	return ret;
} 

void user_free(void* p, size_t size){
	used_user_malloc_size_-=size;
	user_free_(p);
}

void set_user_malloc(void* (*malloc)(size_t), void (*free)(void*)){
	XTAL_ASSERT(used_user_malloc_size_==0);

	user_malloc_ = malloc;
	user_free_ = free;
}


RegionAlloc::RegionAlloc(size_t first_buffer_size){
	alloced_size_ = first_buffer_size;
	begin_ = 0;
	pos_ = 0;
	end_ = 0;
}

RegionAlloc::~RegionAlloc(){
	release();
}

void *RegionAlloc::allocate(size_t size){
	if(pos_+size>=end_){
		add_chunk(size);
	}
	
	void *p = pos_;
	pos_ += align(size, sizeof(void*));
	return p;
}
	
void RegionAlloc::release(){
	while(begin_){
		void* next = *(void**)begin_;
		user_free(begin_, *((int_t*)((void**)begin_+1)));
		begin_=(char*)next;
	}
	begin_ = 0;
	pos_ = 0;
	end_ = 0;
}

void RegionAlloc::add_chunk(size_t minsize){
	if(alloced_size_<minsize+sizeof(void*)+sizeof(int_t))
		alloced_size_ = minsize+sizeof(void*)+sizeof(int_t);
	void* old_begin = begin_;
	begin_=(char*)user_malloc(alloced_size_);
	pos_ = begin_;
	end_ = begin_+alloced_size_;

	*((void**)allocate(sizeof(void*))) = old_begin;
	*((int_t*)allocate(sizeof(int_t))) = alloced_size_;
	alloced_size_*=2;

}


void expand_simple_dynamic_pointer_array(void**& begin, void**& end, void**& current, int addsize){
	uint_t size = end-begin;
	uint_t newsize = size+addsize+size/4;
	void** newbegin=(void**)user_malloc(sizeof(void*)*newsize);
	memcpy(newbegin, begin, sizeof(void*)*size);
	end = newbegin+newsize;
	current = newbegin+(current-begin);
	user_free(begin, sizeof(void*)*size);
	begin = newbegin;	
}

void fit_simple_dynamic_pointer_array(void**& begin, void**& end, void**& current){
	uint_t size = end-begin;
	uint_t newsize = current-begin;
	if(newsize){
		void** newbegin=(void**)user_malloc(sizeof(void*)*newsize);
		memcpy(newbegin, begin, sizeof(void*)*newsize);
		end = newbegin+newsize;
		current = newbegin+newsize;
		user_free(begin, sizeof(void*)*size);
		begin = newbegin;	
	}else{
		user_free(begin, sizeof(void*)*size);
		begin = end = current = 0;
	}
}

}
