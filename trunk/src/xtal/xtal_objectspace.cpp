#include "xtal.h"
#include "xtal_macro.h"

#include "xtal_objectspace.h"

#include <map>
#include <string>

namespace xtal{

enum{
	OBJECTS_ALLOCATE_SHIFT = 8,
	OBJECTS_ALLOCATE_SIZE = 1 << OBJECTS_ALLOCATE_SHIFT,
	OBJECTS_ALLOCATE_MASK = OBJECTS_ALLOCATE_SIZE-1
};

struct ScopeCounter{
	uint_t* p;
	ScopeCounter(uint_t* p):p(p){ *p+=1; }
	~ScopeCounter(){ *p-=1; }
};

struct ConnectedPointer{
	int_t pos;
	RefCountingBase**** bp;

	ConnectedPointer(int_t p, RefCountingBase***& pp)
		:pos(p), bp(&pp){}

	RefCountingBase*& operator *(){
		return (*bp)[pos>>OBJECTS_ALLOCATE_SHIFT][pos&OBJECTS_ALLOCATE_MASK];
	}

	ConnectedPointer& operator ++(){
		++pos;
		return *this;
	}

	ConnectedPointer operator ++(int){
		ConnectedPointer temp(pos, *bp);
		++pos;
		return temp; 
	}

	ConnectedPointer& operator --(){
		--pos;
		return *this;
	}

	ConnectedPointer operator --(int){
		ConnectedPointer temp(pos, *bp);
		--pos;
		return temp; 
	}

	friend bool operator==(const ConnectedPointer& a, const ConnectedPointer& b){
		XTAL_ASSERT(a.bp==b.bp);
		return a.pos==b.pos;
	}

	friend bool operator!=(const ConnectedPointer& a, const ConnectedPointer& b){
		XTAL_ASSERT(a.bp==b.bp);
		return a.pos!=b.pos;
	}
};


void ObjectSpace::initialize(){
	objects_list_begin_ = 0;
	objects_list_current_ = 0;
	objects_list_end_ = 0;
	gcobservers_begin_ = 0;
	gcobservers_current_ = 0;
	gcobservers_end_ = 0;
	objects_count_ = 0;
	prev_objects_count_ = 0;
	cycle_count_ = 0;

	disable_gc();

	expand_objects_list();
	objects_begin_ = objects_current_ = *objects_list_current_++;
	objects_end_ = objects_begin_+OBJECTS_ALLOCATE_SIZE;

	static CppClassSymbolData key;
	class_table_.resize(key.value);
	for(uint_t i=0; i<key.value; ++i){
		class_table_[i] = 0;
	}

	CppClassSymbolData* symbols[] = { 
		&CppClassSymbol<void>::value,
		&CppClassSymbol<Any>::value,
		&CppClassSymbol<Class>::value,
		&CppClassSymbol<Array>::value,
		&CppClassSymbol<String>::value,
	};

	uint_t nsize = sizeof(symbols)/sizeof(symbols[0]);
	uint_t table[sizeof(symbols)/sizeof(symbols[0])];

	for(uint_t i=0; i<nsize; ++i){
		table[i] = symbols[i]->value;
	}

	for(uint_t i=0; i<nsize; ++i){
		class_table_[table[i]] = (Class*)Base::operator new(sizeof(Class));
	}

	for(uint_t i=0; i<nsize; ++i){
		Base* p = class_table_[table[i]];
		new(p) Base();
	}
		
	for(uint_t i=0; i<nsize; ++i){
		Base* p = class_table_[table[i]];
		new(p) Class(Class::cpp_class_t());
	}

	for(uint_t i=0; i<nsize; ++i){
		Base* p = class_table_[table[i]];
		p->set_class(xtal::cpp_class<Class>());
	}
	
	for(uint_t i=0; i<nsize; ++i){
		Base* p = class_table_[table[i]];
		register_gc(p);
	}

	for(uint_t i=0; i<class_table_.size(); ++i){
		if(!class_table_[i]){
			class_table_[i] = xnew<Class>(Class::cpp_class_t()).get();
			class_table_[i]->inc_ref_count();
		}
	}

	CppClassSymbolData* prev = key.prev;
	for(uint_t i=class_table_.size(); i>1; --i){
		class_table_[i-1]->set_prebinder(prev->prebind);
		class_table_[i-1]->set_binder(prev->bind);
		if(prev->name){
			class_table_[i-1]->set_object_name(prev->name);
		}
		prev = prev->prev;
	}
}

void ObjectSpace::uninitialize(){
	for(uint_t i=0; i<class_table_.size(); ++i){
		if(class_table_[i]){
			class_table_[i]->dec_ref_count();
			class_table_[i] = 0;
		}
	}

	class_table_.release();
	full_gc();

	if(objects_count_ != 0){
		if(!ignore_memory_assert()){
			//fprintf(stderr, "finished gc\n");
			//fprintf(stderr, "exists cycled objects %d\n", objects_count_);
			//print_alive_objects();

			// このassertでとまる場合、オブジェクトをすべて開放できていない。
			// グローバル変数などでオブジェクトを握っていないか、循環参照はないか調べること。
			XTAL_ASSERT(false);
		}

		// 強制的に開放してしまおう
		{
			ConnectedPointer current(objects_count_, objects_list_begin_);
			ConnectedPointer begin(0, objects_list_begin_);

			for(ConnectedPointer it = begin; it!=current; ++it){
				(*it)->destroy();
			}

			for(ConnectedPointer it = begin; it!=current; ++it){
				(*it)->object_free();
			}
		}

	}

	for(RefCountingBase*** it=objects_list_begin_; it!=objects_list_end_; ++it){
		RefCountingBase** begin = *it;
		RefCountingBase** current = *it;
		RefCountingBase** end = *it+OBJECTS_ALLOCATE_SIZE;
		fit_simple_dynamic_pointer_array((void**&)begin, (void**&)end, (void**&)current);
	}

	objects_list_current_ = objects_list_begin_;

	fit_simple_dynamic_pointer_array((void**&)gcobservers_begin_, (void**&)gcobservers_end_, (void**&)gcobservers_current_);
	fit_simple_dynamic_pointer_array((void**&)objects_list_begin_, (void**&)objects_list_end_, (void**&)objects_list_current_);

}

void ObjectSpace::enable_gc(){
	cycle_count_--;
}

void ObjectSpace::disable_gc(){
	cycle_count_++;
}
	
void ObjectSpace::expand_objects_list(){
	if(objects_list_current_==objects_list_end_){
		expand_simple_dynamic_pointer_array((void**&)objects_list_begin_, (void**&)objects_list_end_, (void**&)objects_list_current_, 1);
		for(RefCountingBase*** it=objects_list_current_; it!=objects_list_end_; ++it){
			RefCountingBase** begin = 0;
			RefCountingBase** current = 0;
			RefCountingBase** end = 0;
			expand_simple_dynamic_pointer_array((void**&)begin, (void**&)end, (void**&)current, OBJECTS_ALLOCATE_SIZE);
			*it = begin;
		}
	}
}

void ObjectSpace::print_alive_objects(){
	full_gc();

	ConnectedPointer current(objects_count_, objects_list_begin_);
	ConnectedPointer begin(0, objects_list_begin_);

	std::map<std::string, int> table;
	for(ConnectedPointer it = begin; it!=current; ++it){
		switch(type(**it)){
		XTAL_DEFAULT;
		//XTAL_CASE(TYPE_BASE){ table[typeid(*pvalue(**it)).name()]++; }
		XTAL_CASE(TYPE_STRING){ unchecked_ptr_cast<String>(ap(**it))->is_interned() ? table["iString"]++ : table["String"]++; }
		XTAL_CASE(TYPE_ARRAY){ table["Array"]++; }
		XTAL_CASE(TYPE_MULTI_VALUE){ table["MultiValue"]++; }
		XTAL_CASE(TYPE_TREE_NODE){ table["xpeg::TreeNode"]++; }
		XTAL_CASE(TYPE_NATIVE_FUN){ table["NativeFun"]++; }
		XTAL_CASE(TYPE_NATIVE_FUN_BINDED_THIS){ table["NativeFunBindedThis"]++; }
		}
	}

	std::map<std::string, int>::iterator it=table.begin(), last=table.end();
	for(; it!=last; ++it){
		printf("alive %s %d\n", it->first.c_str(), it->second);
	}
	int m = (objects_list_end_ - objects_list_begin_)*OBJECTS_ALLOCATE_SIZE;
	printf("m %d\n", m);

	//printf("used_memory %d\n", used_memory);
}

uint_t ObjectSpace::alive_object_count(){
	return (objects_list_current_ - objects_list_begin_ - 1)*OBJECTS_ALLOCATE_SIZE + (objects_current_ - objects_begin_);
}

RefCountingBase* ObjectSpace::alive_object(uint_t i){
	ConnectedPointer current(i, objects_list_begin_);
	return *current;
}

void ObjectSpace::gc(){
	if(cycle_count_!=0){ return; }

	{
		ScopeCounter cc(&cycle_count_);
		
		ConnectedPointer current(objects_count_, objects_list_begin_);
		ConnectedPointer begin(prev_objects_count_>objects_count_ ? objects_count_ : (prev_objects_count_-prev_objects_count_/4), objects_list_begin_);
		if(current==begin){
			return;
		}

		for(GCObserver** it = gcobservers_begin_; it!=gcobservers_current_; ++it){
			(*it)->before_gc();
		}

		ConnectedPointer alive = begin;

		for(ConnectedPointer it = alive; it!=current; ++it){
			if((*it)->ref_count()!=0 || (*it)->have_finalizer()){
				std::swap(*it, *alive++);
			}
		}

		for(GCObserver** it = gcobservers_begin_; it!=gcobservers_current_; ++it){
			(*it)->after_gc();
		}

		for(ConnectedPointer it = alive; it!=current; ++it){
			(*it)->destroy();
		}

		for(ConnectedPointer it = alive; it!=current; ++it){
			XTAL_ASSERT((*it)->ref_count()==0);
			(*it)->object_free();
			*it = 0;
		}

		{
			prev_objects_count_ = objects_count_ = alive.pos;
			objects_list_current_ = objects_list_begin_ + (objects_count_>>OBJECTS_ALLOCATE_SHIFT);
			expand_objects_list();
			objects_begin_ = *objects_list_current_++;
			objects_current_ = objects_begin_ + (objects_count_&OBJECTS_ALLOCATE_MASK);
			objects_end_ = objects_begin_ + OBJECTS_ALLOCATE_SIZE;
			XTAL_ASSERT(objects_list_current_<=objects_list_end_);
		}
	}
}

void ObjectSpace::full_gc(){
	if(cycle_count_!=0){ return; }
	{
		ScopeCounter cc(&cycle_count_);
				
		while(true){			
			//member_cache_table_.clear();
			//is_cache_table_.clear();

			ConnectedPointer current(objects_count_, objects_list_begin_);
			ConnectedPointer begin(0, objects_list_begin_);
			if(current==begin){
				break;
			}

			for(GCObserver** it = gcobservers_begin_; it!=gcobservers_current_; ++it){
				(*it)->before_gc();
			}

			//if(string_space_){
			//	string_space_->gc();
			//}

			{ // 参照カウンタを減らす
				Visitor m(-1);	
				for(ConnectedPointer it = begin; it!=current; ++it){
					(*it)->visit_members(m);
				}

				// これにより、ルートから示されている以外のオブジェクトは参照カウンタが0となる
			}
		
			ConnectedPointer alive = begin;

			{ // 生存者を見つける
				// 生存者と手をつないでる人も生存者というふうに関係を洗い出す

				Visitor m(1);
				bool end = false;
				while(!end){
					end = true;
					for(ConnectedPointer it = alive; it!=current; ++it){
						if((*it)->ref_count()!=0){
							end = false;
							(*it)->visit_members(m); // 生存確定オブジェクトは、参照カウンタを元に戻す
							std::swap(*it, *alive++);
						}
					}
				}
			}

			// begin ～ aliveまでのオブジェクトは生存確定
			// alive ～ currentまでのオブジェクトは死亡予定


			{// 死者も、参照カウンタを元に戻す
				Visitor m(1);
				for(ConnectedPointer it = alive; it!=current; ++it){
					(*it)->visit_members(m);
				}
			}

			for(GCObserver** it = gcobservers_begin_; it!=gcobservers_current_; ++it){
				(*it)->after_gc();
			}

			{
				bool exists_have_finalizer = false;
				// 死者のfinalizerを走らせる
				for(ConnectedPointer it = alive; it!=current; ++it){
					if((*it)->have_finalizer()){
						exists_have_finalizer = true;
						((Base*)(*it))->finalize();
					}
				}

				if(exists_have_finalizer){
					// finalizerでオブジェクトが作られたかもしれないので、currentを反映する
					current = ConnectedPointer(objects_count_, objects_list_begin_);
					begin = ConnectedPointer(0, objects_list_begin_);

					// 死者が生き返ったかも知れないのでチェックする

					for(GCObserver** it = gcobservers_begin_; it!=gcobservers_current_; ++it){
						(*it)->before_gc();
					}

					{ // 参照カウンタを減らす
						Visitor m(-1);	
						for(ConnectedPointer it = alive; it!=current; ++it){
							(*it)->visit_members(m);
						}
					}
					
					{ // 死者の中から復活した者を見つける
						Visitor m(1);
						bool end = false;
						while(!end){
							end = true;
							for(ConnectedPointer it = alive; it!=current; ++it){
								if((*it)->ref_count()!=0){
									end = false;
									(*it)->visit_members(m); // 生存確定オブジェクトは、参照カウンタを元に戻す
									std::swap(*it, *alive++);
								}
							}
						}
					}

					// begin ～ aliveまでのオブジェクトは生存確定
					// alive ～ currentまでのオブジェクトは死亡確定


					{// 死者も、参照カウンタを元に戻す
						Visitor m(1);
						for(ConnectedPointer it = alive; it!=current; ++it){
							(*it)->visit_members(m);
						}
					}

					for(GCObserver** it = gcobservers_begin_; it!=gcobservers_current_; ++it){
						(*it)->after_gc();
					}
				}
			}

			for(ConnectedPointer it = alive; it!=current; ++it){
				//printf("delete %s\n", typeid(**it).name());
				(*it)->destroy();
			}

			for(ConnectedPointer it = alive; it!=current; ++it){
				XTAL_ASSERT((*it)->ref_count()==0);
				(*it)->object_free();
				*it = 0;
			}
			
			{
				prev_objects_count_ = objects_count_ = alive.pos;
				objects_list_current_ = objects_list_begin_ + (objects_count_>>OBJECTS_ALLOCATE_SHIFT);
				expand_objects_list();
				objects_begin_ = *objects_list_current_++;
				objects_current_ = objects_begin_ + (objects_count_&OBJECTS_ALLOCATE_MASK);
				objects_end_ = objects_begin_ + OBJECTS_ALLOCATE_SIZE;
				XTAL_ASSERT(objects_list_current_<=objects_list_end_);
			}

			if(current==alive){
				break;
			}

			current = alive;
		}

		/*{
			ConnectedPointer current(objects_count_, objects_list_begin_);
			ConnectedPointer begin(0, objects_list_begin_);

			for(ConnectedPointer it = begin; it!=current; ++it){
				if(typeid(**it)==typeid(Code))
				printf("alive %s\n", typeid(**it).name());
			}
		}*/

		for(RefCountingBase*** it=objects_list_current_; it!=objects_list_end_; ++it){
			RefCountingBase** begin = *it;
			RefCountingBase** current = *it;
			RefCountingBase** end = *it+OBJECTS_ALLOCATE_SIZE;
			fit_simple_dynamic_pointer_array((void**&)begin, (void**&)end, (void**&)current);
		}
		fit_simple_dynamic_pointer_array((void**&)objects_list_begin_, (void**&)objects_list_end_, (void**&)objects_list_current_);
	}
}

void ObjectSpace::register_gc(RefCountingBase* p){
	p->inc_ref_count();

	if(objects_current_==objects_end_){
		ScopeCounter cc(&cycle_count_);

		expand_objects_list();
		objects_begin_ = objects_current_ = *objects_list_current_++;
		objects_end_ = objects_begin_+OBJECTS_ALLOCATE_SIZE;
	}

	*objects_current_++ = p;
	objects_count_++;
}

void ObjectSpace::register_gc_observer(GCObserver* p){
	if(gcobservers_current_==gcobservers_end_){
		expand_simple_dynamic_pointer_array((void**&)gcobservers_begin_, (void**&)gcobservers_end_, (void**&)gcobservers_current_, 64);
	}
	*gcobservers_current_++ = p;
}

void ObjectSpace::unregister_gc_observer(GCObserver* p){
	for(GCObserver** it = gcobservers_begin_; it!=gcobservers_current_; ++it){
		if(*it==p){
			std::swap(*it, *--gcobservers_current_);
			break;
		}
	}
}


}
