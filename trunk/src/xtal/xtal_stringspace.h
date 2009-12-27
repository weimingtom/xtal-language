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

	StringSpace()
		:table_(table_t::no_use_memory_t()){
	}

	void initialize(){
		table_.expand(4);

		const StringLiteral ids[] = {
			XTAL_STRING("op_call"),

			XTAL_STRING("op_inc"),
			XTAL_STRING("op_dec"),
			XTAL_STRING("op_pos"),
			XTAL_STRING("op_neg"),
			XTAL_STRING("op_com"),

			XTAL_STRING("op_at"),
			XTAL_STRING("op_set_at"),
			XTAL_STRING("op_range"),

			XTAL_STRING("op_add"),
			XTAL_STRING("op_sub"),
			XTAL_STRING("op_cat"),
			XTAL_STRING("op_mul"),
			XTAL_STRING("op_div"),
			XTAL_STRING("op_mod"),
			XTAL_STRING("op_and"),
			XTAL_STRING("op_or"),
			XTAL_STRING("op_xor"),
			XTAL_STRING("op_shl"),
			XTAL_STRING("op_shr"),
			XTAL_STRING("op_ushr"),

			XTAL_STRING("op_add_assign"),
			XTAL_STRING("op_sub_assign"),
			XTAL_STRING("op_cat_assign"),
			XTAL_STRING("op_mul_assign"),
			XTAL_STRING("op_div_assign"),
			XTAL_STRING("op_mod_assign"),
			XTAL_STRING("op_and_assign"),
			XTAL_STRING("op_or_assign"),
			XTAL_STRING("op_xor_assign"),
			XTAL_STRING("op_shl_assign"),
			XTAL_STRING("op_shr_assign"),
			XTAL_STRING("op_ushr_assign"),

			XTAL_STRING("op_eq"),
			XTAL_STRING("op_lt"),
			XTAL_STRING("op_in"),
		};

		for(int i=0; i<IDOp::id_op_MAX; ++i){
			id_op_list_[i] = intern(ids[i]);
		}
	}

	void uninitialize(){
		table_.destroy();

		for(int i=0; i<IDOp::id_op_MAX; ++i){
			id_op_list_[i] = null;
		}
	}

	void visit_members(Visitor& m){
		for(table_t::iterator it = table_.begin(); it!=table_.end(); ++it){
			m & it->second;
		}
	}

	const IDPtr& insert(const char_t* str, uint_t size, uint_t hashcode, bool literal){
		Key key = {str, size};
		table_t::iterator it = table_.find(key, hashcode);
		if(it!=table_.end()){
			return it->second;
		}

		StringPtr sp = literal ? 
			xnew<String>(StringLiteral(str, size), String::intern_t()) :
			xnew<String>(str, size, String::intern_t());

		it = table_.insert(key, unchecked_ptr_cast<ID>(sp), hashcode).first;
		it->first.str = it->second->data();
		return it->second;
	}

	AnyPtr interned_strings();

	void gc(){
		for(table_t::iterator it=table_.begin(), last=table_.end(); it!=last;){
			if(type(*it->second)==TYPE_STRING && ((StringData*)rcpvalue(*it->second))->ref_count()==1){
				it = table_.erase(it);
			}
			else{
				++it;
			}
		}
	}

	const IDPtr* id_op_list(){
		return id_op_list_;
	}

public:

	struct Key{
		const char_t* str;
		uint_t size;
	};

	struct Fun{
		static uint_t hash(const Key& key){
			return string_hashcode(key.str, key.size);
		}

		static bool eq(const Key& a, const Key& b){
			return string_compare(a.str, a.size, b.str, b.size)==0;
		}
	};

private:

	typedef Hashtable<Key, IDPtr, Fun> table_t; 
	table_t table_;

	IDPtr id_op_list_[IDOp::id_op_MAX];

protected:

	friend class InternedStringIter;
};

class InternedStringIter : public Base{
	StringSpace::table_t::iterator iter_, last_;
public:

	InternedStringIter(StringSpace::table_t::iterator begin, StringSpace::table_t::iterator end)
		:iter_(begin), last_(end){
	}
			
	void block_next(const VMachinePtr& vm){
		if(iter_!=last_){
			vm->return_result(to_smartptr(this), iter_->second);
			++iter_;
		}
		else{
			vm->return_result(null, null);
		}
	}

};

inline AnyPtr StringSpace::interned_strings(){
	return xnew<InternedStringIter>(table_.begin(), table_.end());
}

}

#endif // XTAL_STRINGSPACE_H_INCLUDE_GUARD
