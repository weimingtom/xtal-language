#include "xtal.h"
#include "xtal_macro.h"

namespace xtal{

void initialize_any();
void initialize_string();
void initialize_map();
void initialize_array();
void initialize_basictype();
void initialize_code();
void initialize_frame();
void initialize_debug();
void initialize_text();
void initialize_fun();
void initialize_thread();
void initialize_math();
void initialize_stream();
void initialize_builtin();
void initialize_xpeg();
void initialize_except();
void initialize_iterator();

void initialize_basictype_script();
void initialize_iterator_script();
void initialize_string_script();
void initialize_stream_script();
void initialize_thread_script();
void initialize_frame_script();
void initialize_array_script();
void initialize_map_script();
void initialize_text_script();
void initialize_except_scrpt();

void display_debug_memory();

namespace{

	enum{
		OBJECTS_ALLOCATE_SHIFT = 12,
		OBJECTS_ALLOCATE_SIZE = 1 << OBJECTS_ALLOCATE_SHIFT,
		OBJECTS_ALLOCATE_MASK = OBJECTS_ALLOCATE_SIZE-1
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

	Environment** environments_begin_ = 0;
	Environment** environments_current_ = 0;
	Environment** environments_end_ = 0;

	Environment* current_environment_;

	uint_t cycle_count_ = 0;

	struct UninitializerList{
		UninitializerList* next;
		void (*uninitialize)();
	};

	UninitializerList* uninitializer_list_ = 0;
}

void register_uninitializer(void (*uninitializer)()){
	UninitializerList* p = (UninitializerList*)user_malloc(sizeof(UninitializerList));
	p->uninitialize = uninitializer;
	p->next = uninitializer_list_;
	uninitializer_list_ = p;
}

bool is_initialized(){
	return objects_begin_!=0;
}

Environment* environment(){
	return current_environment_;
}

Environment* new_environment(){
	if(environments_current_==environments_end_){
		expand_simple_dynamic_pointer_array((void**&)environments_begin_, (void**&)environments_end_, (void**&)environments_current_, 16);
	}
	Environment* p = (Environment*)user_malloc(sizeof(Environment));
	new(p) Environment();

	if(!current_environment_){
		current_environment_ = p;
	}

	p->initialize();
	return *environments_current_++ = p;
}

void delete_environment(Environment* p){
	p->~Environment();
	user_free(p);	
}

void initialize(){
	if(is_initialized()){ return; } 

	initialize_memory();
	set_thread();
	disable_gc();

	expand_simple_dynamic_pointer_array((void**&)objects_begin_, (void**&)objects_end_, (void**&)objects_current_, OBJECTS_ALLOCATE_SIZE);
	expand_simple_dynamic_pointer_array((void**&)objects_list_begin_, (void**&)objects_list_end_, (void**&)objects_list_current_);
	*objects_list_current_++ = objects_begin_;

	empty_instance_variables.init();

	Environment* env = new_environment();

	initialize_string();

	initialize_except();
	initialize_any();
	initialize_basictype();
	initialize_array();
	initialize_map();
	initialize_iterator();
	initialize_fun();
	initialize_code();
	initialize_frame();
	initialize_debug();
	initialize_math();
	initialize_stream();
	initialize_thread();
	initialize_text();
	
	std::atexit(&uninitialize); // uninitialize

	initialize_xpeg();
	initialize_builtin();

	initialize_except_scrpt();
	initialize_basictype_script();
	initialize_iterator_script();
	initialize_string_script();
	initialize_stream_script();
	initialize_thread_script();
	initialize_frame_script();
	initialize_array_script();
	initialize_map_script();
	initialize_text_script();

	enable_gc();
}

void uninitialize(){
	if(!is_initialized()){ return; } 

	gc();
	full_gc();

	{
		UninitializerList* next=0;
		for(UninitializerList* p = uninitializer_list_; p; p=next){
			next = p->next;
			uninitializer_list_ = next;
			p->uninitialize();
			user_free(p);
		}
	}

		
	for(Environment** it = environments_begin_; it!=environments_current_; ++it){
		(*it)->~Environment();
		user_free(*it);
	}
	environments_current_ = environments_begin_;

	full_gc();
	
	int n = (objects_list_current_ - objects_list_begin_ - 1)*OBJECTS_ALLOCATE_SIZE + (objects_current_ - objects_begin_);
	if(n != 0){
		//fprintf(stderr, "finished gc\n");
		//fprintf(stderr, " alive object = %d\n", objects_current_-objects_begin_);
		//print_alive_objects();
		Base* p = objects_begin_[0];
		uint_t count = p->ref_count();

		XTAL_ASSERT(false); // 全部開放できてない
	}

	objects_list_current_ = objects_list_begin_;

	fit_simple_dynamic_pointer_array((void**&)environments_begin_, (void**&)environments_end_, (void**&)environments_current_);
	fit_simple_dynamic_pointer_array((void**&)gcobservers_begin_, (void**&)gcobservers_end_, (void**&)gcobservers_current_);
	fit_simple_dynamic_pointer_array((void**&)objects_begin_, (void**&)objects_end_, (void**&)objects_current_);
	fit_simple_dynamic_pointer_array((void**&)objects_list_begin_, (void**&)objects_list_end_, (void**&)objects_list_current_);

	empty_instance_variables.uninit();

	release_memory();

	//
	display_debug_memory();
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
			if((*it)->ref_count()!=0 || (*it)->have_finalizer()){
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
			so_free(*it, ivalue((*it)->class_));
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
		return objects_list_begin_[pos>>OBJECTS_ALLOCATE_SHIFT][pos&OBJECTS_ALLOCATE_MASK];
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
				
		while(true){			
			ConnectedPointer current = (objects_list_current_ - objects_list_begin_ - 1)*OBJECTS_ALLOCATE_SIZE + (objects_current_ - objects_begin_);
			ConnectedPointer begin = 0;

			for(GCObserver** it = gcobservers_begin_; it!=gcobservers_current_; ++it){
				(*it)->before_gc();
			}

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
						(*it)->finalize();
					}
				}

				if(exists_have_finalizer){
					// finalizerでオブジェクトが作られたかもしれないので、currentを反映する
					current = (objects_list_current_ - objects_list_begin_ - 1)*OBJECTS_ALLOCATE_SIZE + (objects_current_ - objects_begin_);

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
				delete *it;
			}

			for(ConnectedPointer it = alive; it!=current; ++it){
				so_free(*it, ivalue((*it)->class_));
			}
			
			if(current==alive){
				break;
			}

			current = alive;

			{
				int_t list_count = objects_list_current_ - objects_list_begin_;
				bool first = true;
				for(int_t i=0; i<list_count; ++i){
					int_t pos = (i+1)*OBJECTS_ALLOCATE_SIZE;
					if(current.pos<pos){
						if(first){
							first = false;
							objects_begin_ = objects_list_begin_[i];
							objects_current_ = objects_begin_ + (current.pos&OBJECTS_ALLOCATE_MASK);
							objects_end_ = objects_begin_ + OBJECTS_ALLOCATE_SIZE;
							objects_list_current_ = objects_list_begin_ + i + 1;
						}
						else{
							user_free(objects_list_begin_[i]);
						}
					}
				}
			}
		}
		
		restart_the_world();
	}
}

void register_gc(Base* p){
	p->inc_ref_count();

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

	*objects_current_++ = p;
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
