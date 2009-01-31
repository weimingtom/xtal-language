#include "xtal.h"
#include "xtal_macro.h"

namespace xtal{

const ClassPtr& Environment::new_cpp_class(const StringPtr& name, void* key){
	ClassPtr& p = cpp_class_map_[key];
	if(!p){ p = xnew<CppClass>(name); }
	return p;
}
	
VMachinePtr Environment::vm_take_over(){
	if(vm_list_->empty()){
		vm_list_->push_back(xnew<VMachine>());
	}
	VMachinePtr vm = unchecked_ptr_cast<VMachine>(vm_list_->back());
	vm_list_->pop_back();
	return vm;
}

void Environment::vm_take_back(const VMachinePtr& vm){
	vm->reset();
	vm_list_->push_back(vm);
}

const AnyPtr& Environment::MemberCacheTable::cache(const Any& target_class, const IDPtr& primary_key, const Any& secondary_key, const Any& self, bool inherited_too, uint_t global_mutate_count){
	uint_t itarget_class = rawvalue(target_class) | (uint_t)inherited_too;
	uint_t iprimary_key = rawvalue(primary_key);
	uint_t ins = rawvalue(secondary_key);

	uint_t hash = itarget_class ^ (iprimary_key>>2) ^ ins + iprimary_key ^ type(primary_key);
	Unit& unit = table_[hash & CACHE_MASK];
	if(global_mutate_count==unit.mutate_count && itarget_class==unit.target_class && raweq(primary_key, unit.primary_key) && ins==unit.secondary_key){
		hit_++;
		return ap(unit.member);
	}
	else{
		miss_++;

		if(type(target_class)!=TYPE_BASE)
			return undefined;

		bool nocache = false;
		unit.member = pvalue(target_class)->do_member(primary_key, ap(secondary_key), ap(self), inherited_too, &nocache);
		if(!nocache){
			unit.target_class = itarget_class;
			unit.primary_key = primary_key;
			unit.secondary_key = ins;
			unit.mutate_count = global_mutate_count;
		}
		return ap(unit.member);
	}
}

bool Environment::IsInheritedCacheTable::cache_is(const Any& target_class, const Any& klass, uint_t global_mutate_count){
	uint_t itarget_class = rawvalue(target_class);
	uint_t iklass = rawvalue(klass);

	uint_t hash = (itarget_class>>3) ^ (iklass>>2);
	Unit& unit = table_[hash & CACHE_MASK];

	if(global_mutate_count==unit.mutate_count && itarget_class==unit.target_class && iklass==unit.klass){
		hit_++;
		return unit.result;
	}
	else{
		miss_++;
		// キャッシュに保存
		unit.target_class = itarget_class;
		unit.klass = iklass;
		unit.mutate_count = global_mutate_count;
		unit.result = unchecked_ptr_cast<Class>(ap(target_class))->is_inherited(ap(klass));

		return unit.result;
	}
}

bool Environment::IsInheritedCacheTable::cache_is_inherited(const Any& target_class, const Any& klass, uint_t global_mutate_count){
	uint_t itarget_class = rawvalue(target_class);
	uint_t iklass = rawvalue(klass);

	uint_t hash = (itarget_class>>3) ^ (iklass>>2);
	Unit& unit = table_[hash & CACHE_MASK];

	if(global_mutate_count==unit.mutate_count && itarget_class==unit.target_class && iklass==unit.klass){
		hit_++;
		return unit.result;
	}
	else{
		miss_++;
		// キャッシュに保存
		unit.target_class = itarget_class;
		unit.klass = iklass;
		unit.mutate_count = global_mutate_count;

		if(const ClassPtr& cls = ptr_as<Class>(ap(target_class))){
			unit.result = cls->is_inherited(ap(klass));
		}
		else{
			unit.result = false;
		}

		return unit.result;
	}
}


const ClassPtr& RuntimeError(){
	return ptr_cast<Class>(builtin()->member(Xid(RuntimeError)));
}

const ClassPtr& CompileError(){
	return ptr_cast<Class>(builtin()->member(Xid(CompileError)));
}

const ClassPtr& UnsupportedError(){
	return ptr_cast<Class>(builtin()->member(Xid(UnsupportedError)));
}

const ClassPtr& ArgumentError(){
	return ptr_cast<Class>(builtin()->member(Xid(ArgumentError)));
}

const StreamPtr& stdin_stream(){
	return ptr_cast<Stream>(builtin()->member(Xid(stdin)));
}

const StreamPtr& stdout_stream(){
	return ptr_cast<Stream>(builtin()->member(Xid(stdout)));
}

const StreamPtr& stderr_stream(){
	return ptr_cast<Stream>(builtin()->member(Xid(stderr)));
}

}