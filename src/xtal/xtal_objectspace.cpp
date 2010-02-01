#include "xtal.h"
#include "xtal_macro.h"

#include "xtal_objectspace.h"

//#define XTAL_DEBUG_PRINT

#ifdef XTAL_DEBUG_PRINT
#include <map>
#include <string>
#endif

namespace xtal{

enum{
	OBJECTS_ALLOCATE_SHIFT = 10,
	OBJECTS_ALLOCATE_SIZE = 1 << OBJECTS_ALLOCATE_SHIFT,
	OBJECTS_ALLOCATE_MASK = OBJECTS_ALLOCATE_SIZE-1
};

struct ScopeCounter{
	uint_t* p;
	ScopeCounter(uint_t* p):p(p){ *p+=1; }
	~ScopeCounter(){ *p-=1; }
};

struct ConnectedPointer{
	uint_t pos;
	RefCountingBase**** bp;

	typedef std::random_access_iterator_tag iterator_category;
	typedef RefCountingBase* value_type;
	typedef int_t difference_type;
	typedef int_t distance_type;	// retained
	typedef value_type* pointer;
	typedef value_type& reference;

	ConnectedPointer()
		:pos(0), bp(0){}

	ConnectedPointer(int_t p, RefCountingBase***& pp)
		:pos(p), bp(&pp){}

	reference operator *(){
		return (*bp)[pos>>OBJECTS_ALLOCATE_SHIFT][pos&OBJECTS_ALLOCATE_MASK];
	}

	reference operator [](int_t index){
		int_t pos2 = pos+index;
		return (*bp)[pos2>>OBJECTS_ALLOCATE_SHIFT][pos2&OBJECTS_ALLOCATE_MASK];
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
		return a.pos==b.pos;
	}

	friend bool operator!=(const ConnectedPointer& a, const ConnectedPointer& b){
		return a.pos!=b.pos;
	}

	friend bool operator<(const ConnectedPointer& a, const ConnectedPointer& b){
		return a.pos<b.pos;
	}

	friend bool operator>(const ConnectedPointer& a, const ConnectedPointer& b){
		return a.pos>b.pos;
	}

	friend bool operator<=(const ConnectedPointer& a, const ConnectedPointer& b){
		return a.pos<=b.pos;
	}

	friend bool operator>=(const ConnectedPointer& a, const ConnectedPointer& b){
		return a.pos>=b.pos;
	}

	friend int_t operator-(const ConnectedPointer& a, const ConnectedPointer& b){
		return a.pos - b.pos;
	}

	friend ConnectedPointer operator-(const ConnectedPointer& a, int_t b){
		return ConnectedPointer(a.pos - b, *a.bp);
	}

	friend ConnectedPointer operator-(int_t a, const ConnectedPointer& b){
		return ConnectedPointer(a - b.pos, *b.bp);
	}

	friend ConnectedPointer operator+(const ConnectedPointer& a, int_t b){
		return ConnectedPointer(a.pos + b, *a.bp);
	}

	friend ConnectedPointer operator+(int_t a, const ConnectedPointer& b){
		return ConnectedPointer(a + b.pos, *b.bp);
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
	objects_max_ = 0;
	processed_line_ = 0;
	cycle_count_ = 0;
	objects_builtin_line_ = 0;

	disable_finalizer_ = false;
	def_all_cpp_classes_ = false;
	binded_all_ = false;

	disable_gc();

	expand_objects_list();

	static CppClassSymbolData key;
	symbol_data_ = &key;

	uint_t maxv = key.value;
	class_table_.resize(maxv);
	for(uint_t i=0; i<maxv; ++i){
		class_table_[i] = 0;
	}

	CppClassSymbolData* symbols[] = { 
		CppClassSymbol<void>::value,
		CppClassSymbol<Any>::value,
		CppClassSymbol<Class>::value,
		CppClassSymbol<Array>::value,
		CppClassSymbol<String>::value,
	};

	uint_t nsize = sizeof(symbols)/sizeof(symbols[0]);

	for(uint_t i=0; i<nsize; ++i){
		class_table_[symbols[i]->value] = (Class*)object_xmalloc<Class>();
	}

	for(uint_t i=0; i<nsize; ++i){
		Class* p = class_table_[symbols[i]->value];
		Base* bp = p;
		new(bp) Base();
	}
		
	for(uint_t i=0; i<nsize; ++i){
		Class* p = class_table_[symbols[i]->value];
		new(p) Class(Class::cpp_class_t());
	}

	for(uint_t i=0; i<nsize; ++i){
		Class* p = class_table_[symbols[i]->value];
		p->set_class(xtal::cpp_class<Class>());
		p->set_virtual_members<Class>();
	}
	
	for(uint_t i=0; i<nsize; ++i){
		Class* p = class_table_[symbols[i]->value];
		register_gc(p);
	}

	//////////////////////////////////////////////////

	for(uint_t i=0; i<class_table_.size(); ++i){
		if(!class_table_[i]){
			class_table_[i] = xnew<Class>(Class::cpp_class_t()).get();
			class_table_[i]->inc_ref_count();
		}
	}

	{
		CppClassSymbolData* prev = key.prev;
		while(prev){
			class_table_[prev->value]->set_symbol_data(prev);
			prev = prev->prev;
		}
	}

	//////////////////////////////////////////////////

	{
		static CppVarSymbolData key(0);
		var_table_ = xnew<Array>(key.value);
		CppVarSymbolData* prev = key.prev;
		for(uint_t i=var_table_->size(); i>1; --i){
			var_table_->set_at(i-1, prev->maker ? prev->maker() : null);
			prev = prev->prev;
		}
	}
}

void ObjectSpace::uninitialize(){
	for(uint_t i=0; i<class_table_.size(); ++i){
		if(class_table_[i]){
			class_table_[i]->dec_ref_count();
			class_table_[i] = 0;
		}
	}

	for(uint_t i=0; i<var_table_->size(); ++i){
		var_table_->set_at(i, undefined);
	}
	var_table_ = null;

	class_table_.release();
	clear_cache();

	disable_finalizer_ = true;

	full_gc();

	if(objects_count_ != 0){
		if(!ignore_memory_assert()){
			//fprintf(stderr, "finished gc\n");
			//fprintf(stderr, "exists cycled objects %d\n", objects_count_);
			//print_alive_objects();

#ifdef XTAL_DEBUG_PRINT
			ConnectedPointer current(objects_count_, objects_list_begin_);
			ConnectedPointer begin(objects_builtin_line_, objects_list_begin_);

			std::map<std::string, int> table;
			for(ConnectedPointer it = begin; it!=current; ++it){
				switch(type(**it)){
				XTAL_DEFAULT;

				XTAL_CASE(TYPE_BASE){ 
					//table["Base"]++; 
					if(Class* p=dynamic_cast<Class*>((Base*)*it)){
						table[p->object_temporary_name()->c_str()]++;
					}
					else{
						table[typeid(*pvalue(**it)).name()]++;
					}
				}

				XTAL_CASE(TYPE_STRING){ 
					unchecked_ptr_cast<String>(ap(**it))->is_interned() ? table["iString"]++ : table["String"]++; 
					const char_t* str = unchecked_ptr_cast<String>(ap(**it))->c_str();
					str = str;
					uint_t n = string_data_size(str);
					XMallocGuard umg((n+1)*sizeof(char));
					char* buf = (char*)umg.get();
					for(uint_t i=0; i<n; ++i){
						buf[i] = str[i];
					}
					buf[n] = 0;
					//table[buf]++;
				}

				XTAL_CASE(TYPE_ARRAY){ table["Array"]++; }
				XTAL_CASE(TYPE_VALUES){ table["Values"]++; }
				XTAL_CASE(TYPE_TREE_NODE){ table["xpeg::TreeNode"]++; }
				XTAL_CASE(TYPE_NATIVE_METHOD){ table["NativeMethod"]++; }
				XTAL_CASE(TYPE_NATIVE_FUN){ table["NativeFun"]++; }
				}
			}

			std::map<std::string, int>::iterator it=table.begin(), last=table.end();
			for(; it!=last; ++it){
				printf("alive %s %d\n", it->first.c_str(), it->second);
			}
			int m = (objects_list_end_ - objects_list_begin_)*OBJECTS_ALLOCATE_SIZE;
			printf("m %d\n", m);

			//printf("used_memory %d\n", used_memory);	
#endif

#ifndef XTAL_CHECK_REF_COUNT

			// このassertでとまる場合、オブジェクトをすべて開放できていない。
			// グローバル変数などでオブジェクトを握っていないか、循環参照はないか調べること。
			XTAL_ASSERT(false);

#endif
		}

		// 強制的に開放してしまおう
		{
			ConnectedPointer current(objects_count_, objects_list_begin_);
			ConnectedPointer begin(0, objects_list_begin_);

			destroy_objects(begin, current);
			free_objects(begin, current);
		}

	}

	for(RefCountingBase*** it=objects_list_begin_; it!=objects_list_end_; ++it){
		RefCountingBase** begin = *it;
		RefCountingBase** current = *it;
		RefCountingBase** end = *it+OBJECTS_ALLOCATE_SIZE;
		fit_simple_dynamic_pointer_array(&begin, &end, &current);
	}

	objects_list_current_ = objects_list_begin_;

	fit_simple_dynamic_pointer_array(&gcobservers_begin_, &gcobservers_end_, &gcobservers_current_);
	fit_simple_dynamic_pointer_array(&objects_list_begin_, &objects_list_end_, &objects_list_current_);
}

void ObjectSpace::finish_initialize(){
	objects_builtin_line_ = objects_count_;
}

void ObjectSpace::enable_gc(){
	cycle_count_--;
}

void ObjectSpace::disable_gc(){
	cycle_count_++;
}
	
void ObjectSpace::expand_objects_list(){
	if(objects_count_==objects_max_){
		expand_simple_dynamic_pointer_array(&objects_list_begin_, &objects_list_end_, &objects_list_current_, 1);

		RefCountingBase** begin = 0;
		RefCountingBase** current = 0;
		RefCountingBase** end = 0;
		expand_simple_dynamic_pointer_array(&begin, &end, &current, OBJECTS_ALLOCATE_SIZE);
		objects_list_begin_[objects_max_>>OBJECTS_ALLOCATE_SHIFT] = begin;
		objects_max_ += OBJECTS_ALLOCATE_SIZE;
	}
}

uint_t ObjectSpace::alive_object_count(){
	return objects_count_;
}

RefCountingBase* ObjectSpace::alive_object(uint_t i){
	ConnectedPointer current(i, objects_list_begin_);
	return *current;
}

void ObjectSpace::gc_signal(int_t flag){
	for(GCObserver** it = gcobservers_begin_; it!=gcobservers_current_; ++it){
		(*it)->gc_signal(flag);
	}
}

ConnectedPointer ObjectSpace::swap_dead_objects(ConnectedPointer first, ConnectedPointer last, ConnectedPointer end){
	ConnectedPointer rend = first - 1;
	ConnectedPointer it = last - 1;
	for(; it!=rend; ){
		RefCountingBase* p = *it;

		if(p->can_not_gc()==0){
			--end;

			p->destroy();
			p->object_free();

			*it = *end;
		}

		--it;
	}

	return end;
}

void ObjectSpace::destroy_objects(ConnectedPointer it, ConnectedPointer current){
	for(; it!=current; ++it){
		(*it)->destroy();
	}
}

void ObjectSpace::free_objects(ConnectedPointer it, ConnectedPointer current){
	for(; it!=current; ++it){
		(*it)->object_free();
	}
}

void ObjectSpace::adjust_objects_list(ConnectedPointer it){
	objects_count_ = it.pos;
	objects_list_current_ = objects_list_begin_ + (objects_count_>>OBJECTS_ALLOCATE_SHIFT);
	expand_objects_list();
	XTAL_ASSERT(objects_list_current_<=objects_list_end_);
}

void ObjectSpace::add_ref_count_objects(ConnectedPointer it, ConnectedPointer current, int_t v){
	Visitor m(v);
	for(; it!=current; ++it){
		(*it)->visit_members(m);
	}
}

void ObjectSpace::lw_gc(){
	if(cycle_count_!=0){ return; }

	ScopeCounter cc(&cycle_count_);

	ConnectedPointer first, last, end;
	
	gc_signal(0);

	uint_t N = 256;

	first = ConnectedPointer(objects_count_<=N ? 0 : objects_count_-N, objects_list_begin_);
	last = ConnectedPointer(objects_count_, objects_list_begin_);
	end = ConnectedPointer(objects_count_, objects_list_begin_);

	end = swap_dead_objects(first, last, end);

	first = ConnectedPointer(processed_line_, objects_list_begin_);
	last = ConnectedPointer(processed_line_+N, objects_list_begin_);

	if(first>end){ first = end; }
	if(last>end){ last = end; }
	end = swap_dead_objects(first, last, end);

	gc_signal(1);

	adjust_objects_list(end);

	processed_line_ += N;
	if(processed_line_>objects_count_){
		processed_line_ = objects_builtin_line_;
	}
}

void ObjectSpace::gc(){
	if(cycle_count_!=0){ return; }

	ScopeCounter cc(&cycle_count_);

	ConnectedPointer first(objects_builtin_line_, objects_list_begin_);
	ConnectedPointer last(objects_count_, objects_list_begin_);
	ConnectedPointer end(objects_count_, objects_list_begin_);

	gc_signal(0);
	end = swap_dead_objects(first, last, end);
	gc_signal(1);

	adjust_objects_list(end);
}

ConnectedPointer ObjectSpace::find_alive_objects(ConnectedPointer alive, ConnectedPointer current){ 
	// 死者の中から復活した者を見つける
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

	return alive;
}

void ObjectSpace::full_gc(){
	if(cycle_count_!=0){ return; }
	{
		vmachine_swap_temp();

		ScopeCounter cc(&cycle_count_);

		//std::sort(ConnectedPointer(0, objects_list_begin_), ConnectedPointer(objects_count_, objects_list_begin_));
				
		while(true){			
			ConnectedPointer current(objects_count_, objects_list_begin_);
			ConnectedPointer begin(0, objects_list_begin_);
			if(current==begin){
				break;
			}

			gc_signal(0);

			// 参照カウンタを減らす
			// これにより、ルートから示されている以外のオブジェクトは参照カウンタが0となる
			add_ref_count_objects(begin, current, -1);
		
			ConnectedPointer alive = begin;

			alive = find_alive_objects(alive, current);

			// begin ～ aliveまでのオブジェクトは生存確定
			// alive ～ currentまでのオブジェクトは死亡予定


			// 死者も、参照カウンタを元に戻す
			add_ref_count_objects(alive, current, 1);

			gc_signal(1);

			if(!disable_finalizer_){
				bool exists_have_finalizer = false;
				
				// 死者のfinalizerを走らせる
				for(ConnectedPointer it = alive; it!=current; ++it){
					RefCountingBase* p = *it;
					if(p->have_finalizer()){
						exists_have_finalizer = true;
						((Base*)p)->finalize();
					}
				}

				if(exists_have_finalizer){
					// finalizerでオブジェクトが作られたかもしれないので、currentを反映する
					current = ConnectedPointer(objects_count_, objects_list_begin_);
					begin = ConnectedPointer(0, objects_list_begin_);

					// 死者が生き返ったかも知れないのでチェックする

					gc_signal(0);

					// 参照カウンタを減らす
					add_ref_count_objects(alive, current, -1);
					
					alive = find_alive_objects(alive, current);

					// begin ～ aliveまでのオブジェクトは生存確定
					// alive ～ currentまでのオブジェクトは死亡確定

					// 死者も、参照カウンタを元に戻す
					add_ref_count_objects(alive, current, 1);

					gc_signal(1);
				}
			}

			destroy_objects(alive, current);
			free_objects(alive, current);
			
			adjust_objects_list(alive);

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

		vmachine_swap_temp();
	}
}

void ObjectSpace::register_gc(RefCountingBase* p){
	p->inc_ref_count();

	if(objects_count_==objects_max_){
		ScopeCounter cc(&cycle_count_);
		expand_objects_list();
	}

	*ConnectedPointer(objects_count_, objects_list_begin_) = p;
	objects_count_++;
}

void ObjectSpace::register_gc_observer(GCObserver* p){
	if(gcobservers_current_==gcobservers_end_){
		expand_simple_dynamic_pointer_array(&gcobservers_begin_, &gcobservers_end_, &gcobservers_current_, 64);
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

void ObjectSpace::bind_all(){
	if(!binded_all_){
		binded_all_ = true;
		for(uint_t i=0; i<class_table_.size(); ++i){
			if(class_table_[i]){
				class_table_[i]->bind();
			}
		}
	}
}

void ObjectSpace::def_all_cpp_classes(){
	if(!def_all_cpp_classes_){
		ClassPtr it = cpp();
		for(uint_t i=0; i<class_table_.size(); ++i){
			if(class_table_[i] && class_table_[i]->symbol_data() && class_table_[i]->symbol_data()->name.str()){
				IDPtr id = class_table_[i]->symbol_data()->name;
				if(!it->member(id)){
					it->def(id, to_smartptr(class_table_[i]));
				}
			}
		}
		def_all_cpp_classes_ = true;
	}
}

}
