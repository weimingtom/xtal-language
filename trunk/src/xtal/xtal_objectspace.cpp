#include "xtal.h"
#include "xtal_macro.h"

#include "xtal_objectspace.h"

#ifdef XTAL_DEBUG_PRINT
#include <map>
#include <string>
#endif

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

	typedef std::random_access_iterator_tag iterator_category;

	typedef std::random_access_iterator_tag iterator_category;
	typedef RefCountingBase* value_type;
	typedef int_t difference_type;
	typedef int_t distance_type;	// retained
	typedef RefCountingBase** pointer;
	typedef RefCountingBase*& reference;

	ConnectedPointer()
		:pos(0), bp(0){}

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

	friend bool operator<(const ConnectedPointer& a, const ConnectedPointer& b){
		XTAL_ASSERT(a.bp==b.bp);
		return a.pos<b.pos;
	}

	friend bool operator>(const ConnectedPointer& a, const ConnectedPointer& b){
		XTAL_ASSERT(a.bp==b.bp);
		return a.pos>b.pos;
	}

	friend bool operator<=(const ConnectedPointer& a, const ConnectedPointer& b){
		XTAL_ASSERT(a.bp==b.bp);
		return a.pos<=b.pos;
	}

	friend bool operator>=(const ConnectedPointer& a, const ConnectedPointer& b){
		XTAL_ASSERT(a.bp==b.bp);
		return a.pos>=b.pos;
	}

	friend int_t operator-(const ConnectedPointer& a, const ConnectedPointer& b){
		XTAL_ASSERT(a.bp==b.bp);
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


void xtal::ObjectSpace::print_alive_objects(){
#ifdef XTAL_DEBUG_PRINT

	ConnectedPointer current(objects_count_, objects_list_begin_);
	ConnectedPointer begin(0, objects_list_begin_);

	std::map<std::string, int> table;
	for(ConnectedPointer it = begin; it!=current; ++it){
		switch(type(**it)){
		XTAL_DEFAULT;
		XTAL_CASE(TYPE_BASE){ table["Base"]++; }
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
			table[buf]++;
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
}

void ObjectSpace::initialize(){
	objects_list_begin_ = 0;
	objects_list_current_ = 0;
	objects_list_end_ = 0;
	gcobservers_begin_ = 0;
	gcobservers_current_ = 0;
	gcobservers_end_ = 0;
	objects_count_ = 0;
	processed_line_ = 0;
	cycle_count_ = 0;

	disable_finalizer_ = false;

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
		CppClassSymbol<void>::value,
		CppClassSymbol<Any>::value,
		CppClassSymbol<Class>::value,
		CppClassSymbol<Array>::value,
		CppClassSymbol<String>::value,
	};

	uint_t nsize = sizeof(symbols)/sizeof(symbols[0]);

	for(uint_t i=0; i<nsize; ++i){
		class_table_[symbols[i]->value] = (Class*)Base::operator new(sizeof(Class));
	}

	for(uint_t i=0; i<nsize; ++i){
		Base* p = class_table_[symbols[i]->value];
		new(p) Base();
	}
		
	for(uint_t i=0; i<nsize; ++i){
		Base* p = class_table_[symbols[i]->value];
		new(p) Class(Class::cpp_class_t());
	}

	for(uint_t i=0; i<nsize; ++i){
		Base* p = class_table_[symbols[i]->value];
		p->set_class(xtal::cpp_class<Class>());
	}
	
	for(uint_t i=0; i<nsize; ++i){
		Base* p = class_table_[symbols[i]->value];
		register_gc(p);
	}

	//////////////////////////////////////////////////

	for(uint_t i=0; i<class_table_.size(); ++i){
		if(!class_table_[i]){
			class_table_[i] = xnew<Class>(Class::cpp_class_t()).get();
			class_table_[i]->inc_ref_count();
		}
	}

	CppClassSymbolData* prev = key.prev;
	for(uint_t i=class_table_.size(); i>1; --i){
		class_table_[i-1]->set_symbol_data(prev);
		prev = prev->prev;
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
			print_alive_objects();

			// このassertでとまる場合、オブジェクトをすべて開放できていない。
			// グローバル変数などでオブジェクトを握っていないか、循環参照はないか調べること。
			XTAL_ASSERT(false);
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

void ObjectSpace::enable_gc(){
	cycle_count_--;
}

void ObjectSpace::disable_gc(){
	cycle_count_++;
}
	
void ObjectSpace::expand_objects_list(){
	if(objects_list_current_==objects_list_end_){
		expand_simple_dynamic_pointer_array(&objects_list_begin_, &objects_list_end_, &objects_list_current_, 1);
		for(RefCountingBase*** it=objects_list_current_; it!=objects_list_end_; ++it){
			RefCountingBase** begin = 0;
			RefCountingBase** current = 0;
			RefCountingBase** end = 0;
			expand_simple_dynamic_pointer_array(&begin, &end, &current, OBJECTS_ALLOCATE_SIZE);
			*it = begin;
		}
	}
}

uint_t ObjectSpace::alive_object_count(){
	return (objects_list_current_ - objects_list_begin_ - 1)*OBJECTS_ALLOCATE_SIZE + (objects_current_ - objects_begin_);
}

RefCountingBase* ObjectSpace::alive_object(uint_t i){
	ConnectedPointer current(i, objects_list_begin_);
	return *current;
}

void ObjectSpace::before_gc(){
	for(GCObserver** it = gcobservers_begin_; it!=gcobservers_current_; ++it){
		(*it)->before_gc();
	}
}

void ObjectSpace::after_gc(){
	for(GCObserver** it = gcobservers_begin_; it!=gcobservers_current_; ++it){
		(*it)->after_gc();
	}
}

ConnectedPointer ObjectSpace::swap_dead_objects(ConnectedPointer first, ConnectedPointer last, ConnectedPointer end){
	
	/*
	uint_t num = 0;
	ConnectedPointer end = alive - 1;
	ConnectedPointer it = current - 1;
	while(it!=end){
		if((*it)->ungc()==0){
			--current;
			++num;

			(*it)->destroy();
			(*it)->object_free();

			*it = *current;
		}
			
		--it;
	}
	//alive = current;
	//return num;
	*/

	/*
	//uint_t num = 0;
	while(alive!=current){
		if((*alive)->ungc()==0){
			--current;

			(*alive)->destroy();
			(*alive)->object_free();

			*alive = *current;
		}
		else{
			++alive;
		}
	}
	return num;
	//*/

	/*
	while(first!=last){
		if((*first)->ungc()==0){
			if(end==last){
				--last;
			}
			--end;

			(*first)->destroy();
			(*first)->object_free();

			*first = *end;
		}
		else{
			++first;
		}
	}
	*/

	ConnectedPointer rend = first - 1;
	ConnectedPointer it = last - 1;
	while(it!=rend){
		if((*it)->ungc()==0){
			--end;

			(*it)->destroy();
			(*it)->object_free();

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
	objects_begin_ = *objects_list_current_++;
	objects_current_ = objects_begin_ + (objects_count_&OBJECTS_ALLOCATE_MASK);
	objects_end_ = objects_begin_ + OBJECTS_ALLOCATE_SIZE;
	XTAL_ASSERT(objects_list_current_<=objects_list_end_);
}

void ObjectSpace::add_ref_count_objects(ConnectedPointer it, ConnectedPointer current, int_t v){
	Visitor m(v);
	for(; it!=current; ++it){
		(*it)->visit_members(m);
	}
}

void ObjectSpace::gc(){
	if(cycle_count_!=0){ return; }

	ScopeCounter cc(&cycle_count_);

	if(processed_line_>objects_count_){
		processed_line_ = 0;
	}
	
	ConnectedPointer first(processed_line_, objects_list_begin_);
	ConnectedPointer last(processed_line_+objects_count_/20, objects_list_begin_);
	ConnectedPointer end(objects_count_, objects_list_begin_);

	if(last>end){ last = end; }

	before_gc();
	end = swap_dead_objects(first, last, end);

	first = ConnectedPointer(end.pos*19/20, objects_list_begin_);
	last = ConnectedPointer(end.pos, objects_list_begin_);
	end = swap_dead_objects(first, last, end);
	after_gc();

	adjust_objects_list(end);

	processed_line_ += objects_count_/20;
}

void ObjectSpace::gc2(){
	if(cycle_count_!=0){ return; }

	ScopeCounter cc(&cycle_count_);

	ConnectedPointer first(0, objects_list_begin_);
	ConnectedPointer last(objects_count_, objects_list_begin_);
	ConnectedPointer end(objects_count_, objects_list_begin_);

	before_gc();
	end = swap_dead_objects(first, last, end);
	after_gc();

	adjust_objects_list(end);
}

void ObjectSpace::full_gc(){
	if(cycle_count_!=0){ return; }
	{
		ScopeCounter cc(&cycle_count_);
				
		while(true){			
			ConnectedPointer current(objects_count_, objects_list_begin_);
			ConnectedPointer begin(0, objects_list_begin_);
			if(current==begin){
				break;
			}

			before_gc();

			// 参照カウンタを減らす
			// これにより、ルートから示されている以外のオブジェクトは参照カウンタが0となる
			add_ref_count_objects(begin, current, -1);
		
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


			// 死者も、参照カウンタを元に戻す
			add_ref_count_objects(alive, current, 1);

			after_gc();

			if(!disable_finalizer_){
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

					before_gc();

					// 参照カウンタを減らす
					add_ref_count_objects(alive, current, -1);
					
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

					// 死者も、参照カウンタを元に戻す
					add_ref_count_objects(alive, current, 1);

					after_gc();
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

		for(RefCountingBase*** it=objects_list_current_; it!=objects_list_end_; ++it){
			RefCountingBase** begin = *it;
			RefCountingBase** current = *it;
			RefCountingBase** end = *it+OBJECTS_ALLOCATE_SIZE;
			fit_simple_dynamic_pointer_array(&begin, &end, &current);
		}
		fit_simple_dynamic_pointer_array(&objects_list_begin_, &objects_list_end_, &objects_list_current_);
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
	for(uint_t i=0; i<class_table_.size(); ++i){
		if(class_table_[i]){
			class_table_[i]->bind();
		}
	}
}

}
