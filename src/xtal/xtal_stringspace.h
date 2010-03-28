/** \file src/xtal/xtal_stringspace.h
* \brief src/xtal/xtal_stringspace.h
*/

#ifndef XTAL_STRINGSPACE_H_INCLUDE_GUARD
#define XTAL_STRINGSPACE_H_INCLUDE_GUARD

#pragma once

namespace xtal{

struct IDOp{
enum{
	id_op_call,

	id_op_inc,
	id_op_dec,
	id_op_pos,
	id_op_neg,
	id_op_com,

	id_op_at,
	id_op_set_at,
	id_op_range,

	id_op_add,
	id_op_sub,
	id_op_cat,
	id_op_mul,
	id_op_div,
	id_op_mod,
	id_op_and,
	id_op_or,
	id_op_xor,
	id_op_shl,
	id_op_shr,
	id_op_ushr,

	id_op_add_assign,
	id_op_sub_assign,
	id_op_cat_assign,
	id_op_mul_assign,
	id_op_div_assign,
	id_op_mod_assign,
	id_op_and_assign,
	id_op_or_assign,
	id_op_xor_assign,
	id_op_shl_assign,
	id_op_shr_assign,
	id_op_ushr_assign,

	id_op_eq,
	id_op_lt,
	id_op_in,

	id_op_MAX
};};


/*
* \brief ï∂éöóÒÇä«óùÇ∑ÇÈÉNÉâÉX
*/
class StringSpace{
public:

	void initialize();

	void uninitialize();

	const char_t* register_string(const char_t* str, uint_t size, uint_t hashcode, bool long_lived);

	const IDPtr* id_op_list(){
		return id_op_list_;
	}

private:

	enum{
		LIMIT_SHIFT = 8,
		LIMIT_MASK = (1<<LIMIT_SHIFT)-1,
		LIMIT = 1<<(LIMIT_SHIFT+2)
	};

	struct Block{
		uint_t pos;

		void* alloc(int size);
	};

	void add_block();

	void expand_buckets(uint_t n);

	void* alloc(int size);

	struct Node{
		Node* next;
		u32 size;
		u16 hash;
		u16 flags; 

		void set_pointer(char_t* str){ *(char_t**)(this+1) = str; }
		char_t* pointer(){ return *(char_t**)(this+1); }
		char_t* buf(){ return (char_t*)(this+1); }
		char_t* data(){ return flags ? *(char_t**)(this+1) : (char_t*)(this+1); }
	};

private:
	IDPtr id_op_list_[IDOp::id_op_MAX];

	Block** blocks_;
	Block* current_block_;
	uint_t blocks_size_;
	uint_t blocks_capa_;

	Node** buckets_;
	uint_t buckets_size_;
	uint_t buckets_capa_;
};

}

#endif // XTAL_STRINGSPACE_H_INCLUDE_GUARD
