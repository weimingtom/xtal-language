/** \file src/xtal/xtal_cache.h
* \brief src/xtal/xtal_cache.h
*/

#ifndef XTAL_CACHE_H_INCLUDE_GUARD
#define XTAL_CACHE_H_INCLUDE_GUARD
#pragma once

namespace xtal{

struct MemberCacheTable{
	struct Unit{
		uint_t mutate_count;
		uint_t accessibility;
		AnyPtr target_class;
		AnyPtr primary_key;
		AnyPtr secondary_key;
		AnyPtr member;
	};

	enum{ CACHE_MAX = 128, CACHE_MASK = CACHE_MAX-1 };

	static uint_t calc_index(uint_t hash){
		return hash & CACHE_MASK;
	}

	Unit table_[CACHE_MAX];

	int_t hit_;
	int_t miss_;
	uint_t mutate_count_;

	MemberCacheTable(){
		hit_ = 0;
		miss_ = 0;
		mutate_count_ = 0;
	}

	int_t hit_count(){
		return hit_;
	}

	int_t miss_count(){
		return miss_;
	}

	void invalidate(){
		mutate_count_++;
	}

	const AnyPtr& cache(Base* target_class, const IDPtr& primary_key, const AnyPtr& secondary_key, int_t& accessibility);

	void clear(){
		for(int_t i=0; i<CACHE_MAX; ++i){
			Unit& unit = table_[i];
			unit.target_class = null;
			unit.primary_key = null;
			unit.secondary_key = null;
			unit.member = null;	
		}
		mutate_count_++;
	}
};

struct IsCacheTable{
	struct Unit{
		uint_t mutate_count;
		AnyPtr target_class;
		AnyPtr klass;
		bool result;
	};

	enum{ CACHE_MAX = 32, CACHE_MASK = CACHE_MAX-1 };

	static uint_t calc_index(uint_t hash){
		return hash & CACHE_MASK;
	}

	Unit table_[CACHE_MAX];

	int_t hit_;
	int_t miss_;
	uint_t mutate_count_;

	IsCacheTable(){
		hit_ = 0;
		miss_ = 0;
		mutate_count_ = 0;
	}

	int_t hit_count(){
		return hit_;
	}

	int_t miss_count(){
		return miss_;
	}
	
	void invalidate(){
		mutate_count_++;
	}

	bool cache(const AnyPtr& target_class, const AnyPtr& klass);

	void clear(){
		for(int_t i=0; i<CACHE_MAX; ++i){
			Unit& unit = table_[i];
			unit.target_class = null;
			unit.klass = null;
		}
		mutate_count_++;
	}
};

struct CtorCacheTable{
	struct Unit{
		uint_t mutate_count;
		uint_t kind;
		AnyPtr target_class;
	};

	enum{ CACHE_MAX = 32, CACHE_MASK = CACHE_MAX-1 };

	static uint_t calc_index(uint_t hash){
		return hash & CACHE_MASK;
	}

	Unit table_[CACHE_MAX];

	int_t hit_;
	int_t miss_;
	uint_t mutate_count_;

	CtorCacheTable(){
		hit_ = 0;
		miss_ = 0;
		mutate_count_ = 0;
	}

	int_t hit_count(){
		return hit_;
	}

	int_t miss_count(){
		return miss_;
	}
	
	void invalidate(){
		mutate_count_++;
	}

	bool cache(const AnyPtr& target_class, int_t kind);

	void clear(){
		for(int_t i=0; i<CACHE_MAX; ++i){
			Unit& unit = table_[i];
			unit.target_class = null;
		}
		mutate_count_++;
	}
};

}


#endif // XTAL_CACHE_H_INCLUDE_GUARD
