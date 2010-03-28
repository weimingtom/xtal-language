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

	enum{ CACHE_MAX = 277, CACHE_MASK = CACHE_MAX };

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

	const AnyPtr& cache(Base* target_class, const IDPtr& primary_key, const AnyPtr& secondary_key, int_t& accessibility){
		uint_t itarget_class = (uint_t)target_class >> 2;
		uint_t iprimary_key = rawvalue(primary_key).u();
		uint_t isecondary_key = rawvalue(secondary_key).u();

		uint_t hash = itarget_class ^ (iprimary_key ^ (iprimary_key>>24)) ^ isecondary_key;
		Unit& unit = table_[hash % CACHE_MASK];

		if(((mutate_count_ ^ unit.mutate_count) | 
			rawbitxor(primary_key, unit.primary_key) | 
			((uint_t)target_class ^ rawvalue(unit.target_class).u()) | 
			rawbitxor(secondary_key, unit.secondary_key))==0){

			hit_++;
			accessibility = unit.accessibility;
			return unit.member;
		}
		else{

			miss_++;

			bool nocache = false;
			const AnyPtr& ret = target_class->rawmember(primary_key, secondary_key, true, accessibility, nocache);
			if(!nocache){
				unit.member = ret;
				unit.target_class = to_smartptr(target_class);
				unit.primary_key = primary_key;
				unit.secondary_key = secondary_key;
				unit.accessibility = accessibility;
				unit.mutate_count = mutate_count_;
			}
			return ret;
		}
	}

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

	enum{ CACHE_MAX = 127, CACHE_MASK = CACHE_MAX };


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

	bool cache(const AnyPtr& target_class, const AnyPtr& klass){
		uint_t itarget_class = rawvalue(target_class).u();
		uint_t iklass = rawvalue(klass).u();

		uint_t hash = (itarget_class>>3) ^ (iklass>>2);
		Unit& unit = table_[hash % CACHE_MASK];
		
		if(mutate_count_==unit.mutate_count && 
			raweq(target_class, unit.target_class) && 
			raweq(klass, unit.klass)){

			hit_++;
			return unit.result;
		}
		else{
			miss_++;

			bool ret = unchecked_ptr_cast<Class>(target_class)->is_inherited(klass);

			unit.target_class = target_class;
			unit.klass = klass;
			unit.mutate_count = mutate_count_;
			unit.result = ret;
			return ret;
		}
	}

	void clear(){
		for(int_t i=0; i<CACHE_MAX; ++i){
			Unit& unit = table_[i];
			unit.target_class = null;
			unit.klass = null;
		}
		mutate_count_++;
	}
};

}


#endif // XTAL_CACHE_H_INCLUDE_GUARD
