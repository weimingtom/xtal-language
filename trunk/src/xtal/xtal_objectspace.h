/** \file src/xtal/xtal_objectspace.h
* \brief src/xtal/xtal_objectspace.h
*/

#ifndef XTAL_OBJECTSPACE_H_INCLUDE_GUARD
#define XTAL_OBJECTSPACE_H_INCLUDE_GUARD

#pragma once

namespace xtal{

struct ConnectedPointer;

class ObjectSpace{
public:

	ObjectSpace()
		:class_map_(map_t::no_use_memory_t())
		,value_map_(map_t::no_use_memory_t()){}

	void initialize();

	void uninitialize();

	void print_all_objects();

	void enable_gc();

	void disable_gc();

	void lw_gc();

	void gc();

	void full_gc();

	void register_gc(RefCountingBase* p);

	void register_gc_vm(VMachine* p);

	void unregister_gc_vm(VMachine* p);

	void make_cpp_class(CppClassSymbolData* key){
		RefCountingBase*& ret = class_map_[key->key()];
		ret = xnew<Class>(Class::cpp_class_t()).get();
		ret->inc_ref_count();
		((Class*)ret)->set_symbol_data(key);
	}

	const ClassPtr& cpp_class(CppClassSymbolData* key){
		map_t::iterator it = class_map_.find(key->key());
		if(it==class_map_.end()){
			make_cpp_class(key);
			it = class_map_.find(key->key());
		}
		return to_smartptr((Class*)it->second);
	}

	const ClassPtr& cpp_class(uint_t key){
		return to_smartptr((Class*)class_map_[key]);
	}

	void make_cpp_value(CppValueSymbolData* key){
		RefCountingBase*& ret = value_map_[key->key()];
		ret = rcpvalue(key->maker());
		ret->inc_ref_count();
	}

	const AnyPtr& cpp_value(CppValueSymbolData* key){
		map_t::iterator it = value_map_.find(key->key());
		if(it==value_map_.end()){
			make_cpp_value(key);
			it = value_map_.find(key->key());
		}
		return to_smartptr(it->second);
	}

	uint_t alive_object_count();

	RefCountingBase* alive_object(uint_t i);

	void finish_initialize();

	void shrink_to_fit();

private:

	void gc_signal(int_t flag);
	
	ConnectedPointer swap_dead_objects(ConnectedPointer first, ConnectedPointer last, ConnectedPointer end);
	void destroy_objects(ConnectedPointer it, ConnectedPointer current);
	void free_objects(ConnectedPointer it, ConnectedPointer current);
	void adjust_objects_list(ConnectedPointer it);
	ConnectedPointer find_alive_objects(ConnectedPointer alive, ConnectedPointer current);

	void add_ref_count_objects(ConnectedPointer it, ConnectedPointer current, int_t v);

	void expand_objects_list();

	RefCountingBase*** objects_list_begin_;
	RefCountingBase*** objects_list_current_;
	RefCountingBase*** objects_list_end_;

	VMachine** gcvms_begin_;
	VMachine** gcvms_current_;
	VMachine** gcvms_end_;

	uint_t objects_builtin_line_;
	uint_t objects_count_;
	uint_t objects_max_;
	uint_t processed_line_;

	bool disable_finalizer_;

	uint_t cycle_count_;

	struct Fun{
		static uint_t hash(uint_t key){
			return key;
		}

		static bool eq(uint_t a, uint_t b){
			return a==b;
		}
	};

	typedef Hashtable<uint_t, RefCountingBase*, Fun> map_t; 
	map_t class_map_;
	map_t value_map_;
};

}

#endif // XTAL_OBJECTSPACE_H_INCLUDE_GUARD
