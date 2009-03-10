
#pragma once

namespace xtal{

class ObjectSpace{
public:

	void initialize();

	void uninitialize();

	void enable_gc();

	void disable_gc();

	void gc();

	void full_gc();

	void register_gc(RefCountingBase* p);

	void register_gc_observer(GCObserver* p);

	void unregister_gc_observer(GCObserver* p);

	int_t ObjectSpace::register_cpp_class(CppClassSymbolData* key){
		// ‰“o˜^‚ÌC++‚ÌƒNƒ‰ƒX‚©
		if(key->value>=class_table_.size()){
			while(key->value>=(int_t)class_table_.size()){
				class_table_.push_back(0);
			}
		}

		return key->value;
	}

	const ClassPtr& new_cpp_class(CppClassSymbolData* key);

	bool exists_cpp_class(CppClassSymbolData* key){
		return key->value<class_table_.size() && class_table_[key->value];
	}

	const ClassPtr& cpp_class(CppClassSymbolData* key){
		XTAL_ASSERT(exists_cpp_class(key));
		return from_this(class_table_[key->value]);
	}

	void set_cpp_class(const ClassPtr& cls, CppClassSymbolData* key){
		int_t index = register_cpp_class(key);
		if(class_table_[index]){
			class_table_[index]->dec_ref_count();
		}
		class_table_[index] = cls.get();
		class_table_[index]->inc_ref_count();
	}

	void print_alive_objects();

private:

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
	uint_t prev_objects_count_;

	uint_t cycle_count_;

	PODArrayList<Class*> class_table_;
};

}
