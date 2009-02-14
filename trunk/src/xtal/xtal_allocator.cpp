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


void FixedAllocator::Chunk::init(uint_t blocks, uint_t block_size){
	blocks_available_ = blocks_ = blocks;
	data_t* p = buf();
	for(uint_t i=0; i<blocks; p+=block_size,++i){
		*p = p+block_size;
	}
	first_available_block_ = buf();
}

void* FixedAllocator::Chunk::malloc(){
	data_t* ret = first_available_block_;
	first_available_block_ = static_cast<data_t*>(*ret);
	--blocks_available_;
	return ret;
}

void FixedAllocator::Chunk::free(void* p){
	data_t *dp = static_cast<data_t*>(p);
	*dp = first_available_block_;
	first_available_block_ = dp;
	++blocks_available_;
}

FixedAllocator::FixedAllocator(){
	blocks_ = 0;
	first_chunk_ = 0;
	last_chunk_ = 0;
	free_data_ = 0;
}

void FixedAllocator::init(size_t block_size){		
	blocks_ = (256/block_size)+1;
	last_chunk_ = first_chunk_ = static_cast<Chunk*>(user_malloc(sizeof(Chunk)+(block_size)*blocks_*sizeof(data_t)));
	first_chunk_->init(blocks_, block_size);
	blocks_ *= 2;
	first_chunk_->next_ = first_chunk_;
}

void FixedAllocator::add_chunk(size_t block_size){
	Chunk *new_chunk = (Chunk*)user_malloc_nothrow(sizeof(Chunk)+(block_size)*blocks_*sizeof(data_t));
	while(!new_chunk){
		blocks_ /= 2;
		if(blocks_<4){
			new_chunk = (Chunk*)user_malloc_nothrow(sizeof(Chunk)+(block_size)*blocks_*sizeof(data_t));
		}
		else{
			new_chunk = (Chunk*)user_malloc(sizeof(Chunk)+(block_size)*blocks_*sizeof(data_t));
		}
	}

	new_chunk->init(blocks_, block_size);
	blocks_ *= 2;
	new_chunk->next_ = first_chunk_;
	first_chunk_ = new_chunk;
	last_chunk_->next_ = first_chunk_;
}

void* FixedAllocator::malloc(size_t block_size){	
	if(free_data_){
		void* ret = free_data_;
		free_data_ = static_cast<data_t*>(*free_data_);
		return ret;
	}

	return malloc_impl(block_size);
}

void FixedAllocator::free(void* mem, size_t block_size){
	*static_cast<data_t*>(mem) = free_data_;
	free_data_ = static_cast<data_t*>(mem);
}
	
void FixedAllocator::release(size_t block_size){
	while(free_data_){
		void* ret = free_data_;
		free_data_ = static_cast<data_t*>(*free_data_);
		free_impl(ret, block_size);
	}

	if(!first_chunk_){
		return;
	}

	for(Chunk *p=first_chunk_->next_, *last=first_chunk_; p!=last; ){
		Chunk *next = p->next_;
		XTAL_ASSERT(p->blocks_==p->blocks_available_);
		user_free(p);
		p = next;
	}
	user_free(first_chunk_);

	blocks_ = 0;
	first_chunk_ = 0;
	last_chunk_ = 0;
	free_data_ = 0;
}

void* FixedAllocator::malloc_impl(size_t block_size){	
	if(first_chunk_->blocks_available_==0){
		for(Chunk* p=first_chunk_->next_, *last=first_chunk_, *prev=first_chunk_; ; p=p->next_){
			if(p==last){
				add_chunk(block_size);
				break;
			}
			if(p->blocks_available_>0){
				first_chunk_ = p;
				last_chunk_ = prev;
				break;
			}
			prev = p;
		}
	}
	return first_chunk_->malloc();
}

void FixedAllocator::free_impl(void* mem, size_t block_size){
	if(first_chunk_->buf()<=mem && mem<first_chunk_->buf()+first_chunk_->blocks_*block_size){
		first_chunk_->free(mem);
		return;
	}
				
	for(Chunk *p=first_chunk_->next_,*last=first_chunk_; p!=last; p=p->next_){
		if(p->buf()<=mem && mem<p->buf()+p->blocks_*block_size){
			p->free(mem);
			return;
		}
	}

	XTAL_ASSERT(false);
}

void SmallObjectAllocator::init(){
	for(int i=0; i<POOL_SIZE; ++i){
		pool_[i].init(i+1);
	}
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
		user_free(p/*, size*/);
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
