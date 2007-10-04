 
#include "xtal.h"

#include "xtal_allocator.h"
#include "xtal_fwd.h"

namespace xtal{


#if 0//def XTAL_DEBUG

struct SizeAndCount{
	SizeAndCount(int a, int b){
		size = a;
		count = b;
	}
	int size;
	int count;
};

std::map<void*, SizeAndCount> mem_map_;
int gcounter = 0;

void* debug_malloc(size_t size){
	void* ret = malloc(size);
	mem_map_.insert(std::make_pair(ret, SizeAndCount(size, gcounter++)));
	return ret;
}

void debug_free(void* p){
	free(p);
	mem_map_.erase(p);
}

void* (*user_malloc_)(size_t) = &debug_malloc;
void (*user_free_)(void*) = &debug_free;

void display_debug_memory(){
	for(std::map<void*, SizeAndCount>::iterator it=mem_map_.begin(); it!=mem_map_.end(); ++it){
		int size = it->second.size;
		int count = it->second.count;
	}

	XTAL_ASSERT(mem_map_.empty()); // ‘S•”ŠJ•ú‚Å‚«‚Ä‚È‚¢
}

#else

void* (*user_malloc_)(size_t) = &malloc;
void (*user_free_)(void*) = &free;

void display_debug_memory(){
}

#endif

namespace{


size_t used_user_malloc_size_ = 0;
size_t used_user_malloc_threshold_ = 1024*100;

SimpleMemoryManager smm_;

void* smm_malloc(size_t size){
	return smm_.malloc(size);
}

void smm_free(void* p){
	smm_.free(p);
}

}

void set_memory(void* memory, size_t size){
	smm_.init(memory, (u8*)memory + size);
	set_user_malloc(&smm_malloc, &smm_free);
}

void* user_malloc(size_t size){
	void* ret = user_malloc_nothrow(size);
	if(!ret){
		XTAL_THROW(AnyPtr(nop), return 0);
	}
	return ret;
} 

void* user_malloc_nothrow(size_t size){
	if(used_user_malloc_size_ > used_user_malloc_threshold_){
		used_user_malloc_size_ = 0; 
		gc();
	}
	used_user_malloc_size_ += size;
	
	void* ret = user_malloc_(size);

	if(!ret){
		gc();
		ret = user_malloc_(size);
	}

	return ret;
} 

void user_free(void* p){
	user_free_(p);
}

void set_user_malloc(void* (*malloc)(size_t), void (*free)(void*)){
	XTAL_ASSERT(used_user_malloc_size_==0);

	user_malloc_ = malloc;
	user_free_ = free;
}

void SimpleMemoryManager::init(void* begin, void* end){
	head_ = (Chunk*)begin;
	begin_ = head_+1;
	end_ = (Chunk*)end-1;
	
	head_->next = begin_;
	head_->prev = 0;
	head_->used = 1;
	
	begin_->next = end_;
	begin_->prev = head_;
	begin_->used = 0;
	
	end_->next = 0;
	end_->prev = begin_;
	end_->used = 1;
}

void* SimpleMemoryManager::malloc(size_t size){
	size = (size+(8-1)) & ~(8-1);
	for(Chunk* it = begin_; it!=end_; it = it->next){
		if(!it->used && size + sizeof(Chunk) <= it->size()){
			Chunk* newchunk = (Chunk*)(((u8*)it->buf())+size);
			newchunk->used = 0;
			it->next->prev = newchunk;
			newchunk->next = it->next;
			it->next = newchunk;
			newchunk->prev = it;
			it->used = 1;
			return it->buf();
		}
	}
	return 0;
}

void SimpleMemoryManager::free(void* p){
	if(p){
		Chunk* it = to_chunk(p);
		it->used--;
		if(it->used==0){
			if(it->prev->used==0){
				it->prev->next = it->next;
				it->next->prev = it->prev;
				it = it->prev;
			}
		
			if(it->next->used==0){
				it->next->next->prev = it;
				it->next = it->next->next;
			}
		}		
	}
}


void expand_simple_dynamic_pointer_array(void**& begin, void**& end, void**& current, int addsize){
	uint_t size = (uint_t)(end-begin);
	uint_t newsize = size+addsize+size;
	void** newbegin=(void**)user_malloc(sizeof(void*)*newsize);
	memcpy(newbegin, begin, sizeof(void*)*size);
	end = newbegin+newsize;
	current = newbegin+(current-begin);
	user_free(begin);
	begin = newbegin;	
}

void fit_simple_dynamic_pointer_array(void**& begin, void**& end, void**& current){
	uint_t size = (uint_t)(end-begin);
	uint_t newsize = (uint_t)(current-begin);
	if(newsize){
		void** newbegin=(void**)user_malloc(sizeof(void*)*newsize);
		memcpy(newbegin, begin, sizeof(void*)*newsize);
		end = newbegin+newsize;
		current = newbegin+newsize;
		user_free(begin);
		begin = newbegin;	
	}else{
		user_free(begin);
		begin = end = current = 0;
	}
}

}
