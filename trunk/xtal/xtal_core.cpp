#include "xtal.h"

#include "xtal_base.h"
#include "xtal_macro.h"

namespace xtal{

void initialize_any();
void initialize_string();
void initialize_interned_string();
void initialize_map();
void initialize_array();
void initialize_basic_type();
void initialize_code();
void initialize_frame();
void initialize_debug();
void initialize_format();
void initialize_fun();
void initialize_thread();
void initialize_math();
void initialize_stream();
void initialize_builtin();
void initialize_peg();

void display_debug_memory();

namespace{

	enum{
		OBJECTS_ALLOCATE_SIZE = 4096
	};

	Base** objects_begin_ = 0;
	Base** objects_current_ = 0;
	Base** objects_end_ = 0;

	Base*** objects_list_begin_ = 0;
	Base*** objects_list_current_ = 0;
	Base*** objects_list_end_ = 0;

	GCObserver** gcobservers_begin_ = 0;
	GCObserver** gcobservers_current_ = 0;
	GCObserver** gcobservers_end_ = 0;

	AnyPtr** place_begin_ = 0;
	AnyPtr** place_current_ = 0;
	AnyPtr** place_end_ = 0;

	uint_t cycle_count_ = 0;

	struct UninitializerList{
		UninitializerList* next;
		void (*uninitialize)();
	};

	UninitializerList* uninitializer_list_ = 0;

	ClassPtr iterator_;
	ClassPtr enumerator_;
	ClassPtr builtin_;
	ClassPtr lib_;
}

void register_uninitializer(void (*uninitializer)()){
	UninitializerList* p = (UninitializerList*)user_malloc(sizeof(UninitializerList));
	p->uninitialize = uninitializer;
	p->next = uninitializer_list_;
	uninitializer_list_ = p;
}

const ClassPtr& Iterator(){
	return iterator_;
}

const ClassPtr& Enumerator(){
	return enumerator_;
}

const ClassPtr& builtin(){
	return builtin_;
}

const ClassPtr& lib(){
	return lib_;
}

bool is_initialized(){
	return objects_begin_!=0;
}

void initialize(){
	if(is_initialized()){ return; } 

	set_thread();

	disable_gc();

	expand_simple_dynamic_pointer_array((void**&)objects_begin_, (void**&)objects_end_, (void**&)objects_current_, OBJECTS_ALLOCATE_SIZE);
	expand_simple_dynamic_pointer_array((void**&)objects_list_begin_, (void**&)objects_list_end_, (void**&)objects_list_current_);
	*objects_list_current_++ = objects_begin_;

	empty_have_instance_variables.init();

	// 生成の際お互いに必要となる、Any, Class, CppClass を特別な方法で生成

	//AnyPtr* holders[] = { &CppClassHolder<Any>::value, CppClassHolder<Class>::value, CppClassHolder<CppClass>::value };

	CppClassHolder<Any>::value = make_place();
	CppClassHolder<Class>::value = make_place();
	CppClassHolder<CppClass>::value = make_place();

	*CppClassHolder<Any>::value = ap(Innocence((Class*)Base::operator new(sizeof(Class))));
	*CppClassHolder<Class>::value = ap(Innocence((Class*)Base::operator new(sizeof(Class))));
	*CppClassHolder<CppClass>::value = ap(Innocence((Class*)Base::operator new(sizeof(Class))));

	pvalue(get_cpp_class<Any>())->set_ref_count(1);
	pvalue(get_cpp_class<Class>())->set_ref_count(1);
	pvalue(get_cpp_class<CppClass>())->set_ref_count(1);
	
	int_t temp_ref_count;
	
	temp_ref_count = pvalue(get_cpp_class<Any>())->ref_count(); 
	new(pvalue(get_cpp_class<Any>())) Class(Class::cpp_class_t());
	pvalue(get_cpp_class<Any>())->add_ref_count(temp_ref_count-1);
		
	temp_ref_count = pvalue(get_cpp_class<Class>())->ref_count(); 
	new(pvalue(get_cpp_class<Class>())) Class(Class::cpp_class_t());
	pvalue(get_cpp_class<Class>())->add_ref_count(temp_ref_count-1);

	temp_ref_count = pvalue(get_cpp_class<CppClass>())->ref_count(); 
	new(pvalue(get_cpp_class<CppClass>())) Class(Class::cpp_class_t());
	pvalue(get_cpp_class<CppClass>())->add_ref_count(temp_ref_count-1);

	pvalue(get_cpp_class<Any>())->set_class(get_cpp_class<Class>());
	pvalue(get_cpp_class<Class>())->set_class(get_cpp_class<Class>());
	pvalue(get_cpp_class<CppClass>())->set_class(get_cpp_class<Class>());

	
	builtin_ = xnew<Singleton>();
	lib_ = xnew<Lib>();
	iterator_ = xnew<Class>();
	enumerator_ = xnew<Class>();


	initialize_string();
	initialize_interned_string();
	initialize_any();
	initialize_array();
	initialize_map();
	initialize_basic_type();
	initialize_fun();
	initialize_code();
	initialize_frame();
	initialize_debug();
	initialize_format();
	initialize_math();
	initialize_stream();
	initialize_peg();
	initialize_thread();
	
	atexit(&uninitialize); // uninitialize

	enable_gc();

	initialize_builtin();
}

void uninitialize(){
	if(!is_initialized()){ return; } 

	gc();

	UninitializerList* next=0;
	for(UninitializerList* p = uninitializer_list_; p; p=next){
		next = p->next;
		uninitializer_list_ = next;
		p->uninitialize();
		user_free(p);
	}

	for(AnyPtr** p = place_begin_; p!=place_current_; ++p){
		**p = null;
	}

	builtin_ = null;
	lib_ = null;
	iterator_ = null;
	enumerator_ = null;

	full_gc();
	
	if(objects_current_-objects_begin_ != 0){
		//fprintf(stderr, "finished gc\n");
		//fprintf(stderr, " alive object = %d\n", objects_current_-objects_begin_);
		int n = objects_current_-objects_begin_;
		//print_alive_objects();
		XTAL_ASSERT(false); // 全部開放できてない
	}
	
	for(AnyPtr** p = place_begin_; p!=place_current_; ++p){
		user_free(*p);
	}
	
	place_current_ = place_begin_;
	objects_list_current_ = objects_list_begin_;

	fit_simple_dynamic_pointer_array((void**&)gcobservers_begin_, (void**&)gcobservers_end_, (void**&)gcobservers_current_);
	fit_simple_dynamic_pointer_array((void**&)objects_begin_, (void**&)objects_end_, (void**&)objects_current_);
	fit_simple_dynamic_pointer_array((void**&)objects_list_begin_, (void**&)objects_list_end_, (void**&)objects_list_current_);
	fit_simple_dynamic_pointer_array((void**&)place_begin_, (void**&)place_end_, (void**&)place_current_);

	empty_have_instance_variables.uninit();

	//
	display_debug_memory();
}

AnyPtr* make_place(){
	if(place_current_==place_end_){
		expand_simple_dynamic_pointer_array((void**&)place_begin_, (void**&)place_end_, (void**&)place_current_);
	}
	*place_current_ = new(user_malloc(sizeof(AnyPtr))) AnyPtr();
	return *place_current_++;
}


struct CycleCounter{
	uint_t* p;
	CycleCounter(uint_t* p):p(p){ *p+=1; }
	~CycleCounter(){ *p-=1; }
};

void enable_gc(){
	cycle_count_++;
}

void disable_gc(){
	cycle_count_--;
}

void gc(){
	if(cycle_count_!=0){ return; }
	if(stop_the_world()){
		CycleCounter cc(&cycle_count_);

		for(GCObserver** it = gcobservers_begin_; it!=gcobservers_current_; ++it){
			(*it)->before_gc();
		}

		Base** objects_alive = objects_begin_;

		for(Base** it = objects_alive; it!=objects_current_; ++it){
			if((*it)->ref_count()!=0){
				std::swap(*it, *objects_alive++);
			}
		}

		for(GCObserver** it = gcobservers_begin_; it!=gcobservers_current_; ++it){
			(*it)->after_gc();
		}

		for(Base** it = objects_alive; it!=objects_current_; ++it){
			delete *it;
		}

		for(Base** it = objects_alive; it!=objects_current_; ++it){
			user_free(*it);
		}

		objects_current_ = objects_alive;

		restart_the_world();
	}
}


struct ConnectedPointer{
	int_t pos;

	ConnectedPointer(int_t p = 0){
		pos = p;
	}

	Base*& operator *(){
		return objects_list_begin_[pos/OBJECTS_ALLOCATE_SIZE][pos&(OBJECTS_ALLOCATE_SIZE-1)];
	}

	ConnectedPointer& operator ++(){
		++pos;
		return *this;
	}

	ConnectedPointer operator ++(int){
		ConnectedPointer temp(pos);
		++pos;
		return temp; 
	}

	ConnectedPointer& operator --(){
		--pos;
	}

	ConnectedPointer operator --(int){
		ConnectedPointer temp(pos);
		--pos;
		return temp; 
	}

	friend bool operator==(const ConnectedPointer& a, const ConnectedPointer& b){
		return a.pos==b.pos;
	}

	friend bool operator!=(const ConnectedPointer& a, const ConnectedPointer& b){
		return a.pos!=b.pos;
	}
};

void full_gc(){
	if(cycle_count_!=0){ return; }
	if(stop_the_world()){
		CycleCounter cc(&cycle_count_);

		ConnectedPointer prev_oc;
		ConnectedPointer current = (objects_list_current_ - objects_list_begin_ - 1)*OBJECTS_ALLOCATE_SIZE + (objects_current_ - objects_begin_);
		ConnectedPointer begin = 0;
		
		do{
			
			for(GCObserver** it = gcobservers_begin_; it!=gcobservers_current_; ++it){
				(*it)->before_gc();
			}

			prev_oc = current;

			{
				Visitor m(-1);	
				for(ConnectedPointer it = begin; it!=current; ++it){
					(*it)->visit_members(m);
				}
			}

			{
				ConnectedPointer alive = begin;

				{
					Visitor m(1);
					bool end = false;
					while(!end){
						end = true;
						for(ConnectedPointer it = alive; it!=current; ++it){
							if((*it)->ref_count()!=0){
								end = false;
								(*it)->visit_members(m);
								std::swap(*it, *alive++);
							}
						}
					}
				}
	

				{// 
					Visitor m(1);
					for(ConnectedPointer it = alive; it!=current; ++it){
						(*it)->visit_members(m);
					}
				}

				for(GCObserver** it = gcobservers_begin_; it!=gcobservers_current_; ++it){
					(*it)->after_gc();
				}

				for(ConnectedPointer it = alive; it!=current; ++it){
					delete *it;
				}

				for(ConnectedPointer it = alive; it!=current; ++it){
					user_free(*it);
				}

				current = alive;
			}

		}while(prev_oc!=current);

		int_t list_count = objects_list_current_ - objects_list_begin_;
		bool first = true;
		for(int_t i=0; i<list_count; ++i){
			int_t pos = (i+1)*OBJECTS_ALLOCATE_SIZE;
			if(current.pos<pos){
				if(first){
					first = false;
					objects_begin_ = objects_list_begin_[i];
					objects_current_ = objects_begin_ + (current.pos&(OBJECTS_ALLOCATE_SIZE-1));
					objects_end_ = objects_begin_ + OBJECTS_ALLOCATE_SIZE;
					objects_list_current_ = objects_list_begin_ + i + 1;
				}else{
					user_free(objects_list_begin_[i]);
				}
			}
		}
		
		restart_the_world();
	}
}
	
void* Base::operator new(size_t size){
	if(objects_current_==objects_end_){
		gc();

		if(objects_current_==objects_end_){
			if(objects_list_current_==objects_list_end_){
				expand_simple_dynamic_pointer_array((void**&)objects_list_begin_, (void**&)objects_list_end_, (void**&)objects_list_current_);
			}
			objects_begin_ = objects_current_ = objects_end_ = 0;
			expand_simple_dynamic_pointer_array((void**&)objects_begin_, (void**&)objects_end_, (void**&)objects_current_, OBJECTS_ALLOCATE_SIZE);
			*objects_list_current_++ = objects_begin_;
		}
	}

	Base* p = static_cast<Base*>(user_malloc(size));
	*objects_current_++ = p;
	
	p->ref_count_ = 1;
	p->class_ = null;
	
	return p;
}

void Base::operator delete(void* p){

}


GCObserver::GCObserver(){
	if(gcobservers_current_==gcobservers_end_){
		expand_simple_dynamic_pointer_array((void**&)gcobservers_begin_, (void**&)gcobservers_end_, (void**&)gcobservers_current_);
	}
	*gcobservers_current_++ = this;
}

GCObserver::GCObserver(const GCObserver& v)
:Base(v){
	if(gcobservers_current_==gcobservers_end_){
		expand_simple_dynamic_pointer_array((void**&)gcobservers_begin_, (void**&)gcobservers_end_, (void**&)gcobservers_current_);
	}
	*gcobservers_current_++ = this;
}
	
GCObserver::~GCObserver(){
	for(GCObserver** p = gcobservers_begin_; p!=gcobservers_current_; ++p){
		if(*p==this){
			std::swap(*p, *--gcobservers_current_);
			break;
		}
	}
}


}