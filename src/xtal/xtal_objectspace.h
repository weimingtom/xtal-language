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

	void initialize();

	void uninitialize();

	void enable_gc();

	void disable_gc();

	void gc();

	void gc2();

	void full_gc();

	void register_gc(RefCountingBase* p);

	void register_gc_observer(GCObserver* p);

	void unregister_gc_observer(GCObserver* p);

	bool exists_cpp_class(uint_t key){
		return class_table_[key]!=0;
	}

	const ClassPtr& cpp_class(uint_t key){
		return to_smartptr(class_table_[key]);
	}

	const AnyPtr& cpp_var(uint_t key){
		return var_table_->at(key);
	}

	void print_alive_objects();

	uint_t alive_object_count();

	RefCountingBase* alive_object(uint_t i);

	void bind_all();

private:

	void before_gc();
	void after_gc();
	ConnectedPointer swap_dead_objects(ConnectedPointer first, ConnectedPointer last, ConnectedPointer end);
	void destroy_objects(ConnectedPointer it, ConnectedPointer current);
	void free_objects(ConnectedPointer it, ConnectedPointer current);
	void adjust_objects_list(ConnectedPointer it);

	void add_ref_count_objects(ConnectedPointer it, ConnectedPointer current, int_t v);

	void expand_objects_list();

	RefCountingBase** objects_begin_ ;
	RefCountingBase** objects_current_;
	RefCountingBase** objects_end_;

	RefCountingBase*** objects_list_begin_;
	RefCountingBase*** objects_list_current_;
	RefCountingBase*** objects_list_end_;

	GCObserver** gcobservers_begin_;
	GCObserver** gcobservers_current_;
	GCObserver** gcobservers_end_;

	uint_t objects_count_;
	uint_t processed_line_;

	bool disable_finalizer_;

	uint_t cycle_count_;

	PODArrayList<Class*> class_table_;
	ArrayPtr var_table_;
};

}

#endif // XTAL_OBJECTSPACE_H_INCLUDE_GUARD