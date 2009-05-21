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

	const AnyPtr& cache(const AnyPtr& target_class, const IDPtr& primary_key, int_t& accessibility){
		uint_t itarget_class = rawvalue(target_class).uvalue>>2;
		uint_t iprimary_key = rawvalue(primary_key).uvalue;

		uint_t hash = (itarget_class ^ iprimary_key) ^ (iprimary_key>>3)*3;
		Unit& unit = table_[calc_index(hash)];

		if(((mutate_count_^unit.mutate_count) | 
			rawbitxor(primary_key, unit.primary_key) | 
			rawbitxor(target_class, unit.target_class))==0){
			hit_++;
			accessibility = unit.accessibility;
			return ap(unit.member);
		}
		else{

			miss_++;

			if(type(target_class)!=TYPE_BASE){
				return undefined;
			}

			bool nocache = false;
			const AnyPtr& ret = pvalue(target_class)->rawmember(primary_key, undefined, true, accessibility, nocache);
			//if(rawne(ret, undefined)){
				unit.member = ret;
				if(!nocache){
					unit.target_class = target_class;
					unit.primary_key = primary_key;
					unit.accessibility = accessibility;
					unit.mutate_count = mutate_count_;
				}
				else{
					unit.mutate_count = mutate_count_-1;
				}
				return ap(unit.member);
			//}
			//return undefined;
		}
	}

	void clear(){
		for(int_t i=0; i<CACHE_MAX; ++i){
			Unit& unit = table_[i];
			unit.target_class = null;
			unit.primary_key = null;
			unit.member = null;	
		}
		mutate_count_++;
	}
};

struct MemberCacheTable2{
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

	MemberCacheTable2(){
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

	const AnyPtr& cache(const AnyPtr& target_class, const IDPtr& primary_key, const AnyPtr& secondary_key, int_t& accessibility){
		uint_t itarget_class = rawvalue(target_class).uvalue>>2;
		uint_t iprimary_key = rawvalue(primary_key).uvalue;
		uint_t isecondary_key = rawvalue(secondary_key).uvalue;

		uint_t hash = (itarget_class ^ iprimary_key ^ isecondary_key) ^ (iprimary_key>>3)*3 ^ isecondary_key*7;
		Unit& unit = table_[calc_index(hash)];

		if(((mutate_count_^unit.mutate_count) | 
			rawbitxor(primary_key, unit.primary_key) | 
			rawbitxor(target_class, unit.target_class) | 
			rawbitxor(secondary_key, unit.secondary_key))==0){
			hit_++;
			accessibility = unit.accessibility;
			return ap(unit.member);
		}
		else{

			miss_++;

			if(type(target_class)!=TYPE_BASE){
				return undefined;
			}

			bool nocache = false;
			const AnyPtr& ret = pvalue(target_class)->rawmember(primary_key, ap(secondary_key), true, accessibility, nocache);
			//if(rawne(ret, undefined)){
				unit.member = ret;
				if(!nocache){
					unit.target_class = target_class;
					unit.primary_key = primary_key;
					unit.secondary_key = secondary_key;
					unit.accessibility = accessibility;
					unit.mutate_count = mutate_count_;
				}
				else{
					unit.mutate_count = mutate_count_-1;
				}
				return ap(unit.member);
			//}
			//return undefined;
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

	enum{ CACHE_MAX = 64, CACHE_MASK = CACHE_MAX-1 };

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

	bool cache(const AnyPtr& target_class, const AnyPtr& klass){
		uint_t itarget_class = rawvalue(target_class).uvalue;
		uint_t iklass = rawvalue(klass).uvalue;

		uint_t hash = (itarget_class>>3) ^ (iklass>>2);
		Unit& unit = table_[hash & CACHE_MASK];
		
		if(mutate_count_==unit.mutate_count && 
			raweq(target_class, unit.target_class) && 
			raweq(klass, unit.klass)){
			hit_++;
			return unit.result;
		}
		else{
			miss_++;

			bool ret = unchecked_ptr_cast<Class>(ap(target_class))->is_inherited(ap(klass));

			// キャッシュに保存
			//if(ret){
				unit.target_class = target_class;
				unit.klass = klass;
				unit.mutate_count = mutate_count_;
				unit.result = ret;
			//}
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

struct CtorCacheTable{
	struct Unit{
		uint_t mutate_count;
		uint_t kind;
		AnyPtr target_class;
	};

	enum{ CACHE_MAX = 64, CACHE_MASK = CACHE_MAX-1 };

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

	bool cache(const AnyPtr& target_class, int_t kind){
		uint_t itarget_class = rawvalue(target_class).uvalue;

		uint_t hash = (itarget_class>>3) ^ kind*37;
		Unit& unit = table_[hash & CACHE_MASK];
		
		if(mutate_count_==unit.mutate_count && 
			raweq(target_class, unit.target_class) &&
			kind==kind){
			hit_++;
			return true;
		}
		else{
			miss_++;

			// キャッシュに保存
			{
				unit.target_class = target_class;
				unit.kind = kind;
				unit.mutate_count = mutate_count_;
			}
			return false;
		}
	}

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
