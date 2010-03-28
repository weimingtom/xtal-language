#include "xtal.h"
#include "xtal_macro.h"

#include "xtal_stringspace.h"

namespace xtal{

void StringSpace::initialize(){
	blocks_ = 0;
	blocks_size_ = 0;
	blocks_capa_ = 0;
	add_block();
	alloc(16);

	buckets_ = 0;
	buckets_size_ = 0;
	buckets_capa_ = 0;
	expand_buckets(127);

	static const char_t* ids[] = {
		XTAL_L("op_call"),

		XTAL_L("op_inc"),
		XTAL_L("op_dec"),
		XTAL_L("op_pos"),
		XTAL_L("op_neg"),
		XTAL_L("op_com"),

		XTAL_L("op_at"),
		XTAL_L("op_set_at"),
		XTAL_L("op_range"),

		XTAL_L("op_add"),
		XTAL_L("op_sub"),
		XTAL_L("op_cat"),
		XTAL_L("op_mul"),
		XTAL_L("op_div"),
		XTAL_L("op_mod"),
		XTAL_L("op_and"),
		XTAL_L("op_or"),
		XTAL_L("op_xor"),
		XTAL_L("op_shl"),
		XTAL_L("op_shr"),
		XTAL_L("op_ushr"),

		XTAL_L("op_add_assign"),
		XTAL_L("op_sub_assign"),
		XTAL_L("op_cat_assign"),
		XTAL_L("op_mul_assign"),
		XTAL_L("op_div_assign"),
		XTAL_L("op_mod_assign"),
		XTAL_L("op_and_assign"),
		XTAL_L("op_or_assign"),
		XTAL_L("op_xor_assign"),
		XTAL_L("op_shl_assign"),
		XTAL_L("op_shr_assign"),
		XTAL_L("op_ushr_assign"),

		XTAL_L("op_eq"),
		XTAL_L("op_lt"),
		XTAL_L("op_in"),
	};

	for(int i=0; i<IDOp::id_op_MAX; ++i){
		id_op_list_[i] = intern(XTAL_LONG_LIVED_STRING(ids[i]));
	}
}

void StringSpace::uninitialize(){
	for(uint_t i=0; i<buckets_capa_; ++i){
		Node* node = buckets_[i];
		while(node!=0){
			if(node->flags==2){
				xfree(node->pointer(), (node->size+1)*sizeof(char_t));
			}
			node = node->next;
		}	
	}

	for(uint_t i=0; i<blocks_size_; ++i){
		xfree(blocks_[i], sizeof(Block)+LIMIT);
	}

	xfree(blocks_, sizeof(Block*)*blocks_capa_);
	xfree(buckets_, sizeof(Node*)*buckets_capa_);

	for(int i=0; i<IDOp::id_op_MAX; ++i){
		id_op_list_[i] = null;
	}
}

const char_t* StringSpace::register_string(const char_t* str, uint_t size, uint_t hashcode, bool long_lived){
	uint_t hash = (hashcode&0xffff) ^ ((hashcode>>16)&0xffff);

	uint_t hn = hash % buckets_capa_;
	Node* node = buckets_[hn];
	while(node!=0){
		char_t* data = node->data();
		if(node->hash==hash && string_compare(data, node->size, str, size)==0){
			return data;
		}
		node = node->next;
	}

	buckets_size_++;

	Node* p;
	char_t* newstr;
	if(!long_lived){
		if(size<48){
			p = (Node*)alloc(sizeof(Node) + (size+1)*sizeof(char_t));
			p->flags = 0;
			newstr = p->buf();
		}
		else{
			p = (Node*)alloc(sizeof(Node) + sizeof(char_t*));
			p->flags = 2;
			newstr = (char_t*)xmalloc((size+1)*sizeof(char_t));
			p->set_pointer(newstr);
		}

		string_copy(newstr, str, size);
		newstr[size] = 0;
	}
	else{
		p = (Node*)alloc(sizeof(Node) + sizeof(char_t*));
		p->flags = 1;
		newstr = (char_t*)str;
		p->set_pointer(newstr);
	}

	p->size = size;
	p->hash = hash;

	p->next = buckets_[hn];
	buckets_[hn] = p;

	if(buckets_size_ > buckets_capa_){
		expand_buckets(1);
	}

	return newstr;
}

void* StringSpace::Block::alloc(int size){
	size = align(size, sizeof(int_t));
	if(pos+size>=LIMIT){ return 0; }
	void* ret = ((char*)(this+1) + pos);
	pos += size;
	return ret;
}

void StringSpace::add_block(){
	if(blocks_size_==blocks_capa_){
		uint_t newcapa = blocks_capa_ + 16;
		Block** newblocks = (Block**)xmalloc(sizeof(Block*)*newcapa);
		std::memcpy(newblocks, blocks_, sizeof(Block*)*blocks_capa_);
		xfree(blocks_, sizeof(Block*)*blocks_capa_);
		blocks_ = newblocks;
		blocks_capa_ = newcapa;
	}

	Block* p = (Block*)xmalloc(sizeof(Block)+LIMIT);
	p->pos = 0;
	blocks_[blocks_size_++] = p;
	current_block_ = p;
}

void StringSpace::expand_buckets(uint_t n){
	uint_t newcapa = buckets_capa_*2 + n;
	Node** newbuckets = (Node**)xmalloc(sizeof(Node*)*newcapa);
	for(uint_t i=0; i<newcapa; ++i){
		newbuckets[i] = 0;
	}

	for(uint_t i=0; i<buckets_capa_; ++i){
		Node* node = buckets_[i];
		while(node!=0){
			Node* next = node->next;
			uint_t hn2 = node->hash % newcapa;
			node->next = newbuckets[hn2]; 
			newbuckets[hn2] = node;
			node = next;
		}	
	}

	xfree(buckets_, sizeof(Node*)*buckets_capa_);
	buckets_ = newbuckets;
	buckets_capa_ = newcapa;
}

void* StringSpace::alloc(int size){
	void* p = current_block_->alloc(size);
	if(!p){
		add_block();
		p = current_block_->alloc(size);
	}
	return p;
}


}
