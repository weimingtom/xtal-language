 #include "xtal.h"

namespace xtal{

namespace{

bool initialized_memory_ = false;

size_t used_user_malloc_size_ = 0;
size_t used_user_malloc_threshold_ = 1024*10;

RBTreeAllocator rbtree_alloc_;
SmallObjectAllocator so_alloc_;

void* rbtree_alloc_malloc(size_t size){
	return rbtree_alloc_.malloc(size);
}

void rbtree_alloc_free(void* p){
	rbtree_alloc_.free(p);
}

}

#if 0//def XTAL_DEBUG

struct SizeAndCount{
	SizeAndCount(int a = 0, int b = 0){
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
	if(gcounter==18150){
		gcounter = gcounter;
	}
	mem_map_.insert(std::make_pair(ret, SizeAndCount(size, gcounter++)));
	return ret;
}

void debug_free(void* p){
	memset(p, 0xcd, mem_map_[p].size);
	free(p);
	mem_map_.erase(p);
}

void* (*user_malloc_)(size_t) = &debug_malloc;
void (*user_free_)(void*) = &debug_free;

void display_debug_memory(){
	for(std::map<void*, SizeAndCount>::iterator it=mem_map_.begin(); it!=mem_map_.end(); ++it){
		int size = it->second.size;
		int count = it->second.count;
		size = size;
	}

	XTAL_ASSERT(mem_map_.empty()); // ‘S•”ŠJ•ú‚Å‚«‚Ä‚È‚¢
}

#else

void* (*user_malloc_)(size_t) = &std::malloc;
void (*user_free_)(void*) = &std::free;

void display_debug_memory(){
	//printf("-------------------\n");
	//rbtree_alloc_.debug_print();
}

#endif

void set_memory(void* memory, size_t size){
	rbtree_alloc_.init(memory, (u8*)memory + size);
	set_user_malloc(&rbtree_alloc_malloc, &rbtree_alloc_free);
}

void* user_malloc(size_t size){
	void* ret = user_malloc_nothrow(size);
	if(!ret){
		XTAL_THROW(AnyPtr(undefined), return 0);
	}
	return ret;
} 

void* user_malloc_nothrow(size_t size){
	//full_gc();

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

void* so_malloc(size_t size){
	return so_alloc_.malloc(size);
}

void so_free(void* p, size_t size){
	so_alloc_.free(p, size);
}

void set_user_malloc(void* (*malloc)(size_t), void (*free)(void*)){
	XTAL_ASSERT(used_user_malloc_size_==0);

	user_malloc_ = malloc;
	user_free_ = free;

	initialize_memory();
}

void initialize_memory(){
	if(initialized_memory_){
		return;
	}

	initialized_memory_ = true;
	so_alloc_.init();
}

void release_memory(){
	so_alloc_.release();
	rbtree_alloc_.release();
}

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
	uint_t size = (uint_t)(end-begin);
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



RBTreeAllocator::RBTreeAllocator(){
	head_ = begin_ = end_ = root_ = 0; 
}

void RBTreeAllocator::init(void* begin, void* end){
	head_ = (Node*)begin;
	begin_ = head_+1;
	end_ = (Node*)end-1;
	
	head_->next = begin_;
	head_->prev = 0;
	head_->flag = 1;
	head_->left = 0;
	head_->right = 0;
	
	end_->next = end_+1;
	end_->prev = begin_;
	end_->flag = 1;
	end_->left = 0;
	end_->right = 0;

	begin_->next = end_;
	begin_->prev = head_;
	begin_->flag = 0;
	begin_->left = end_;
	begin_->right = end_;

	XTAL_ASSERT(!end_->red());

	count_ = 1;

	root_ = end_;
	insert(begin_);
}

void* RBTreeAllocator::malloc(size_t size){
	size = ((size+(8-1)) & ~(8-1)) + sizeof(Node);

	Node* node = root_;
	Node* near = end_;
	while(node!=end_){
		if(size<=node->size()){
			near = node;
			node = node->left;
		}
		else{
			node = node->right;
		}
	}

	if(near!=end_){
		return malloc(near, size);
	}
	
	debug_print();

	return 0;
}

void* RBTreeAllocator::malloc(Node* node, size_t size){
	uint_t nodesize = node->size();
	XTAL_ASSERT(node->size()>=size);

	erase(node);

	Node* newnode = (Node*)(((u8*)node->buf())+size-sizeof(Node));
	newnode->flag = 0;
	node->next->prev = newnode;
	newnode->next = node->next;
	node->next = newnode;
	newnode->prev = node;
	node->flag = 1;

	insert(newnode);
	return node->buf();
}

void RBTreeAllocator::free(void* p){
	if(p){
		Node* node = to_node(p);
		node->flag--;
		if(node->used()==0){
			if(node->prev->used()==0){
				erase(node->prev);
				node->prev->next = node->next;
				node->next->prev = node->prev;
				node = node->prev;
			}
		
			if(node->next->used()==0){
				erase(node->next);
				node->next->next->prev = node;
				node->next = node->next->next;
			}
			insert(node);
		}		
	}
}

void RBTreeAllocator::insert(Node* newnode){
	newnode->flag = count_++;
	newnode->set_red();
	newnode->right = end_;
	newnode->left = end_;

	root_ = insert(root_, newnode);
	root_->set_black();
	check();
}

RBTreeAllocator::Node* RBTreeAllocator::insert(Node* h , Node* newnode){
	if(h == end_){
		return newnode; 
	}
	
	if(newnode->size() < h->size() || (newnode->size()==h->size() && newnode->used() < h->used())){
		h->left = insert(h->left, newnode);
	}
	else{
		h->right = insert(h->right , newnode);
	}

	if(h->right->red() && !h->left->red()){
		h = rotate_left(h);
	}
	
	if(h->left->red() && h->left->left->red()){
		h = rotate_right(h);
	}

	if(h->left->red() && h->right->red()){
		flip_colors(h);
	}
	
	return h;
}

void RBTreeAllocator::erase(Node* erasenode){
	root_ = erase(root_ , erasenode);
	root_->set_black();
	check();
}

RBTreeAllocator::Node* RBTreeAllocator::erase(Node* h , Node* erasenode){
	if(erasenode->size() < h->size() || (erasenode->size()==h->size() && erasenode->used() < h->used())){
		if(!h->left->red() && !h->left->left->red()){
			h = move_red_left(h);
		}
		h->left = erase(h->left, erasenode);
	}
	else{
		if(h->left->red()){
			h = rotate_right(h);
		}
		
		if(erasenode==h && h->right==end_){
			return end_;
		}

		if(!h->right->red() && !h->right->left->red()){
			int sizeeee = erasenode->size();
			h = move_red_right(h);
		}

		if(erasenode==h){
			Node* min_node = h->right;
			Node* min_node_parent = h;
			while(min_node->left!=end_){
				min_node_parent = min_node;
				min_node = min_node->left;
			}

			std::swap(min_node->left, h->left);
			std::swap(min_node->right, h->right);

			bool n = min_node->red();
			bool m = h->red();

			std::swap(min_node->flag, h->flag);

			if(n){ min_node->set_red(); }else{ min_node->set_black(); }
			if(m){ h->set_red(); }else{ h->set_black(); }

			if(min_node_parent==h){
				min_node->right = h;
			}
			else{
				min_node_parent->left = h; 
			}

			h = min_node;

			h->right = erase_min(h->right);
		}
		else{
			h->right = erase(h->right , erasenode);
		}
	}
	return fixup(h);
}

RBTreeAllocator::Node* RBTreeAllocator::erase_min(Node* h){
	if(h->left==end_){
		return end_;
	}

	if(!h->left->red() && !h->left->left->red()){
		h = move_red_left(h);
	}

	h->left = erase_min(h->left);
	return fixup(h);
}

RBTreeAllocator::Node* RBTreeAllocator::move_red_right(Node* h){
	flip_colors(h);
	if(h->left->left->red()){
		h = rotate_right(h);
		flip_colors(h);
	}
	return h;
}

RBTreeAllocator::Node* RBTreeAllocator::move_red_left(Node* h){
	flip_colors(h);
	if(h->right->left->red()){
		h->right = rotate_right(h->right);
		h = rotate_left(h);
		flip_colors(h);
	}
	return h;
}

RBTreeAllocator::Node* RBTreeAllocator::fixup(Node* h){
	if(h->right->red()){
		h = rotate_left(h);
	}

	if(h->left->red() && h->left->left->red()){
		h = rotate_right(h);
	}

	if(h->left->red() && h->right->red()){
		flip_colors(h);
	}

	return h;
}

RBTreeAllocator::Node* RBTreeAllocator::rotate_left(Node* h){
	Node* x = h->right;
	h->right = x->left;
	x->left = h;
	x->set_same_color(x->left);
	XTAL_ASSERT(x->left!=end_);
	x->left->set_red();
	return x;
}

RBTreeAllocator::Node* RBTreeAllocator::rotate_right(Node* h){
	Node* x = h->left;
	h->left = x->right;
	x->right = h;
	x->set_same_color(x->right);
	XTAL_ASSERT(x->right!=end_);
	x->right->set_red();
	return x;
}

void RBTreeAllocator::release(){
	XTAL_ASSERT(root_==end_);
	head_ = begin_ = end_ = root_ = 0; 
}

void RBTreeAllocator::check(){
	std::vector<Node*> nodes, next_nodes;
	nodes.push_back(root_);

	while(true){
		bool all_end = true;
		for(uint_t i=0; i<nodes.size(); ++i){
			if(nodes[i]==end_){
				
			}
			else{
				next_nodes.push_back(nodes[i]->left);
				next_nodes.push_back(nodes[i]->right);

				if(nodes[i]->left!=end_){
					if(nodes[i]->left->size()>nodes[i]->size()){
						debug_print();
						return;
					}
				}

				if(nodes[i]->right!=end_){
					if(nodes[i]->right->size()<nodes[i]->size()){
						debug_print();
						return;
					}
				}

				if(nodes[i]->left!=end_ || nodes[i]->right!=end_){
					all_end = false;
				}
			}
		}

		nodes = next_nodes;
		next_nodes.clear();

		if(all_end){
			break;
		}
	}
}


void RBTreeAllocator::debug_print(){
	for(Node* it = begin_; it!=end_; it = it->next){
		printf("%s pos=%d, size=%d\n", it->used() ? "used" : "free", (char*)it - (char*)begin_, it->size());
	}

	std::vector<Node*> nodes, next_nodes;
	nodes.push_back(root_);

	while(true){
		bool all_end = true;
		for(uint_t i=0; i<nodes.size(); ++i){
			if(nodes[i]==end_){
				printf("(NIL)");
			}
			else{
				printf("(%s%d)", nodes[i]->red() ? "R" : "B", nodes[i]->size());
				next_nodes.push_back(nodes[i]->left);
				next_nodes.push_back(nodes[i]->right);

				if(nodes[i]->left!=end_ || nodes[i]->right!=end_){
					all_end = false;
				}
			}
		}

		nodes = next_nodes;
		next_nodes.clear();
		printf("\n");

		if(all_end){
			break;
		}
	}
	printf("-------------------\n");

}

///////////////////////////////////////


void FixedAllocator::Chunk::init(uint_t blocks, uint_t block_size){
	blocks_available_ = blocks_ = blocks;
	data_t*p = buf();
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
	blocks_ = (1024/block_size)+1;
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
		size_t wsize = align(size, sizeof(data_t))/sizeof(data_t);
		pool_[wsize-1].free(p, wsize);
	}
}

void SmallObjectAllocator::release(){
	for(int i=0; i<POOL_SIZE; ++i){
		pool_[i].release(sizeof(data_t)*(i+1));
	}	
}

}
