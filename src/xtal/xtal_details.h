/** \file src/xtal/xtal_details.h
* \brief src/xtal/xtal_details.h
*/

#ifndef XTAL_DETAILS_H_INCLUDE_GUARD
#define XTAL_DETAILS_H_INCLUDE_GUARD

#pragma once

#include "xtal_objectspace.h"
#include "xtal_stringspace.h"
#include "xtal_threadspace.h"
#include "xtal_cache.h"

namespace xtal{

class Environment{
public:
	
	Environment(){}
	~Environment(){}

	void initialize(const Setting& setting);
	void uninitialize();

	Setting setting_;

	ObjectSpace object_space_;	
	StringSpace string_space_;
	ThreadSpace thread_space_;
	MemberCacheTable member_cache_table_;
	IsCacheTable is_cache_table_;
	CtorCacheTable ctor_cache_table_;

	ClassPtr builtin_;
	ClassPtr global_;
	LibPtr lib_;

	ArrayPtr vm_list_;
	MapPtr text_map_;

	StreamPtr stdin_;
	StreamPtr stdout_;
	StreamPtr stderr_;

	bool set_jmp_buf_;
	bool ignore_memory_assert_;
	JmpBuf jmp_buf_;

	uint_t malloc_count_;
	uint_t used_memory_;
	uint_t memory_threshold_;

	bool gc_stress_;

#ifndef XTAL_NO_SMALL_ALLOCATOR
	SmallObjectAllocator so_alloc_;
#endif
};

extern XTAL_TLS_PTR(Environment) environment_;
extern XTAL_TLS_PTR(VMachine) vmachine_;

inline const AnyPtr& cache_member(Base* target_class, const IDPtr& primary_key, const AnyPtr& secondary_key, int_t& accessibility){
	return environment_->member_cache_table_.cache(target_class, primary_key, secondary_key, accessibility);
}

class Builtin{};

}

#endif //XTAL_DETAILS_H_INCLUDE_GUARD
