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
	OBJECTS_ALLOCATE_SHIFT = 9,
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

public:

	pointer segment_begin(){
		return &(*bp)[pos>>OBJECTS_ALLOCATE_SHIFT][0];
	}

	pointer segment_end(){
		return &(*bp)[pos>>OBJECTS_ALLOCATE_SHIFT][OBJECTS_ALLOCATE_SIZE];
	}

	pointer segment_rbegin(){
		return segment_end()-1;
	}

	pointer segment_rend(){
		return segment_begin()-1;
	}

	pointer segment_get(){
		return &(*bp)[pos>>OBJECTS_ALLOCATE_SHIFT][pos&OBJECTS_ALLOCATE_MASK];
	}

	uint_t segment_pos(){
		return pos&OBJECTS_ALLOCATE_MASK;
	}

	uint_t segment_number(){
		return pos>>OBJECTS_ALLOCATE_SHIFT;
	}

	void move_next_segment(){
		pos = (pos+OBJECTS_ALLOCATE_SIZE) & ~OBJECTS_ALLOCATE_MASK;
	}

	void move_back_segment(){
		pos = (pos & ~OBJECTS_ALLOCATE_MASK) - 1;
	}

public:

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

struct ConnectedPointerEnumerator{
	ConnectedPointer begin_;
	ConnectedPointer end_;

	ConnectedPointerEnumerator(const ConnectedPointer& b, const ConnectedPointer& e){
		begin_ = b;
		end_ = e;
	}

	RefCountingBase** begin(){
		return begin_.segment_get();
	}

	RefCountingBase** end(){
		if(begin_.segment_number()==end_.segment_number()){
			return end_.segment_get();
		}
		else{
			return begin_.segment_end();
		}
	}

	bool move(){
		if(begin_.segment_number()==end_.segment_number()){
			return false;
		}

		begin_.move_next_segment();
		return true;
	}

//	do for(RefCountingBase** pp=e.begin(), **ppend=e.end(); pp!=ppend; ++pp){
//	}while(e.move());
};

struct ConnectedPointerReverseEnumerator{
	ConnectedPointer begin_;
	ConnectedPointer end_;

	ConnectedPointerReverseEnumerator(const ConnectedPointer& b, const ConnectedPointer& e){
		begin_ = e - 1;
		end_ = b - 1;
	}

	RefCountingBase** begin(){
		return begin_.segment_get();
	}

	RefCountingBase** end(){
		if(begin_.segment_number()==end_.segment_number()){
			return end_.segment_get();
		}
		else{
			return begin_.segment_rend();
		}
	}

	bool move(){
		if(begin_.segment_number()==end_.segment_number()){
			return false;
		}

		begin_.move_back_segment();
		return true;
	}


//	do for(RefCountingBase** pp=e.begin(), **ppend=e.end(); pp!=ppend; --pp){
//	}while(e.move());
};

void ObjectSpace::initialize(){
	objects_list_begin_ = 0;
	objects_list_current_ = 0;
	objects_list_end_ = 0;
	gcvms_begin_ = 0;
	gcvms_current_ = 0;
	gcvms_end_ = 0;
	objects_count_ = 0;
	objects_max_ = 0;
	processed_line_ = 0;
	cycle_count_ = 0;
	objects_builtin_line_ = 0;

	disable_finalizer_ = false;

	disable_gc();

	class_map_.expand(4);
	value_map_.expand(4);

	expand_objects_list();

	static CppClassSymbolData* symbols[] = { 
		&CppClassSymbol<Class>::value,
		&CppClassSymbol<Any>::value,
		&CppClassSymbol<Array>::value,
		&CppClassSymbol<String>::value,
	};

	uint_t nsize = sizeof(symbols)/sizeof(symbols[0]);

	for(uint_t i=0; i<nsize; ++i){
		Class* p = object_xmalloc<Class>();
		class_map_[symbols[i]->key()] = p;
		p->special_initialize();
	}

	for(uint_t i=0; i<nsize; ++i){
		Class* p = (Class*)class_map_[symbols[i]->key()];
		new(p) Class(Class::cpp_class_t());
	}
	
	for(uint_t i=0; i<nsize; ++i){
		Class* p = (Class*)class_map_[symbols[i]->key()];
		p->special_initialize(&VirtualMembersT<Class>::value);
	}

	for(uint_t i=0; i<nsize; ++i){
		Class* p = (Class*)class_map_[symbols[i]->key()];
		register_gc(p);
		p->inc_ref_count();
		p->set_symbol_data(symbols[i]);
	}
}

void ObjectSpace::uninitialize(){
	for(map_t::iterator it=class_map_.begin(), last=class_map_.end(); it!=last; ++it){
		it->second->dec_ref_count();
	}

	for(map_t::iterator it=value_map_.begin(), last=value_map_.end(); it!=last; ++it){
		it->second->dec_ref_count();
	}

	class_map_.destroy();
	value_map_.destroy();

	clear_cache();

	disable_finalizer_ = true;

	full_gc();

	if(objects_count_ != 0){
		if(!ignore_memory_assert()){
			//fprintf(stderr, "finished gc\n");
			//fprintf(stderr, "exists cycled objects %d\n", objects_count_);
			//print_alive_objects();

		print_all_objects();

#ifndef XTAL_CHECK_REF_COUNT

			// ����assert�łƂ܂�ꍇ�A�I�u�W�F�N�g�����ׂĊJ���ł��Ă��Ȃ��B
			// �O���[�o���ϐ��ȂǂŃI�u�W�F�N�g�������Ă��Ȃ����A�z�Q�Ƃ͂Ȃ������ׂ邱�ƁB
			XTAL_ASSERT(false);

#endif
		}

		// �����I�ɊJ�����Ă��܂���
		{
			ConnectedPointer current(objects_count_, objects_list_begin_);
			ConnectedPointer begin(0, objects_list_begin_);

			destroy_objects(begin, current);
			free_objects(begin, current);
		}
	}

	for(RefCountingBase*** it=objects_list_current_; it!=objects_list_end_; ++it){
		RefCountingBase** begin = *it;
		RefCountingBase** current = *it;
		RefCountingBase** end = *it+OBJECTS_ALLOCATE_SIZE;
		fit_simple_dynamic_pointer_array(&begin, &end, &current);
	}

	gcvms_current_ = gcvms_begin_;
	fit_simple_dynamic_pointer_array(&gcvms_begin_, &gcvms_end_, &gcvms_current_);

	objects_list_current_ = objects_list_begin_;
	fit_simple_dynamic_pointer_array(&objects_list_begin_, &objects_list_end_, &objects_list_current_);
}

void ObjectSpace::finish_initialize(){
	objects_builtin_line_ = objects_count_;
}
	
void ObjectSpace::shrink_to_fit(){
	if(cycle_count_>0){
		return;
	}

	ScopeCounter cc(&cycle_count_);

	if(objects_list_current_!=objects_list_end_){
		++objects_list_current_;

		for(RefCountingBase*** it=objects_list_current_; it!=objects_list_end_; ++it){
			RefCountingBase** begin = *it;
			RefCountingBase** current = *it;
			RefCountingBase** end = *it+OBJECTS_ALLOCATE_SIZE;
			fit_simple_dynamic_pointer_array(&begin, &end, &current);
		}

		fit_simple_dynamic_pointer_array(&gcvms_begin_, &gcvms_end_, &gcvms_current_);
		fit_simple_dynamic_pointer_array(&objects_list_begin_, &objects_list_end_, &objects_list_current_);

		objects_max_ = (objects_list_current_-objects_list_begin_)*OBJECTS_ALLOCATE_SIZE;
		
		--objects_list_current_;
	}

	ConnectedPointer current(objects_count_, objects_list_begin_);
	ConnectedPointer begin(0, objects_list_begin_);

	ConnectedPointerEnumerator e(begin, current);
	do for(RefCountingBase** pp=e.begin(), **ppend=e.end(); pp!=ppend; ++pp){
		(*pp)->shrink_to_fit();
	}while(e.move());
}

void ObjectSpace::print_all_objects(){
#ifdef XTAL_DEBUG_PRINT
			ConnectedPointer current(objects_count_, objects_list_begin_);
			ConnectedPointer begin(0, objects_list_begin_);

			std::map<std::string, int> table;
			std::string classpre = "CLASS: ";
			std::string stringpre = "STRING: ";
			for(ConnectedPointer it = begin; it!=current; ++it){
				switch(type(**it)){
					XTAL_DEFAULT{
						RefCountingBase* p = *it;
						int_t rc = p->ref_count();

						//table["Base"]++; 
						if(Class* cp=dynamic_cast<Class*>(p)){
							table["Class"]++;
							table[classpre + cp->object_temporary_name()->c_str()]++;
						}
						else{
							table[typeid(*p).name()]++;
							if(table[typeid(*p).name()]==20){
								table[typeid(*p).name()] = table[typeid(*p).name()];
							}
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
						table[stringpre+buf]++;
					}

				}
			}

			std::map<std::string, int>::iterator it=table.begin(), last=table.end();
			for(; it!=last; ++it){
				if(it->second>10){
					printf("alive %s %d\n", it->first.c_str(), it->second);
				}
			}
			int m = (objects_list_end_ - objects_list_begin_)*OBJECTS_ALLOCATE_SIZE;
			printf("object_list_size=%d, objects_count=%d\n", m, objects_count_);
			//printf("used_memory %d\n", used_memory);	
#endif
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
	for(VMachine** it = gcvms_begin_; it!=gcvms_current_; ++it){
		(*it)->gc_signal(flag);
	}
}

ConnectedPointer ObjectSpace::swap_dead_objects(ConnectedPointer first, ConnectedPointer last, ConnectedPointer end){
	if(first==last){
		return end;
	}

	ConnectedPointerReverseEnumerator cpre1(first+1, last);
	ConnectedPointerReverseEnumerator cpre2(first+1, end);

	RefCountingBase** endpp = cpre2.begin();
	RefCountingBase** endppend = cpre2.end();

	do for(RefCountingBase** pp=cpre1.begin(), **ppend=cpre1.end(); pp!=ppend; --pp){
		RefCountingBase* p = *pp;
		if(p->can_not_gc()==0){
			p->destroy();
			p->object_free();

			if(endpp==endppend){
				cpre2.move();
				endpp = cpre2.begin();
				endppend = cpre2.end();
			}

			--end;

			*pp = *endpp;
			--endpp;
		}

	}while(cpre1.move());

	return end;

	/*	
	ConnectedPointer it = last - 1;
	ConnectedPointer rend = first - 1;

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
*/
}

void ObjectSpace::destroy_objects(ConnectedPointer it, ConnectedPointer end){
	ConnectedPointerEnumerator e(it, end);
	do for(RefCountingBase** pp=e.begin(), **ppend=e.end(); pp!=ppend; ++pp){
		(*pp)->destroy();
	}while(e.move());
}

void ObjectSpace::free_objects(ConnectedPointer it, ConnectedPointer end){
	ConnectedPointerEnumerator e(it, end);
	do for(RefCountingBase** pp=e.begin(), **ppend=e.end(); pp!=ppend; ++pp){
		(*pp)->object_free();
	}while(e.move());
}

void ObjectSpace::adjust_objects_list(ConnectedPointer it){
	objects_count_ = it.pos;
	objects_list_current_ = objects_list_begin_ + (objects_count_>>OBJECTS_ALLOCATE_SHIFT);
	expand_objects_list();
	XTAL_ASSERT(objects_list_current_<=objects_list_end_);
}

void ObjectSpace::add_ref_count_objects(ConnectedPointer it, ConnectedPointer end, int_t v){
	Visitor m(v);
	ConnectedPointerEnumerator e(it, end);
	do for(RefCountingBase** pp=e.begin(), **ppend=e.end(); pp!=ppend; ++pp){
		(*pp)->visit_members(m);
	}while(e.move());
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
	// ���҂̒����畜�������҂�������
	Visitor m(1);
	bool end = false;
	while(!end){
		end = true;

		ConnectedPointerEnumerator e(alive, current);
		do for(RefCountingBase** pp=e.begin(), **ppend=e.end(); pp!=ppend; ++pp){
			if((*pp)->ref_count()!=0){
				end = false;
				(*pp)->visit_members(m); // �����m��I�u�W�F�N�g�́A�Q�ƃJ�E���^�����ɖ߂�
				std::swap(*pp, *alive++);
			}
		}while(e.move());
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

			// �Q�ƃJ�E���^�����炷
			// ����ɂ��A���[�g���玦����Ă���ȊO�̃I�u�W�F�N�g�͎Q�ƃJ�E���^��0�ƂȂ�
			add_ref_count_objects(begin, current, -1);
		
			ConnectedPointer alive = begin;

			alive = find_alive_objects(alive, current);

			// begin �` alive�܂ł̃I�u�W�F�N�g�͐����m��
			// alive �` current�܂ł̃I�u�W�F�N�g�͎��S�\��


			// ���҂��A�Q�ƃJ�E���^�����ɖ߂�
			add_ref_count_objects(alive, current, 1);

			gc_signal(1);

			if(!disable_finalizer_){
				bool exists_have_finalizer = false;
				
				// ���҂�finalizer�𑖂点��
				ConnectedPointerEnumerator e(alive, current);
				do for(RefCountingBase** pp=e.begin(), **ppend=e.end(); pp!=ppend; ++pp){
					RefCountingBase* p = *pp;
					if(p->have_finalizer()){
						exists_have_finalizer = true;
						((Base*)p)->finalize();
					}
				}while(e.move());

				if(exists_have_finalizer){
					// finalizer�ŃI�u�W�F�N�g�����ꂽ��������Ȃ��̂ŁAcurrent�𔽉f����
					current = ConnectedPointer(objects_count_, objects_list_begin_);
					begin = ConnectedPointer(0, objects_list_begin_);

					// ���҂������Ԃ��������m��Ȃ��̂Ń`�F�b�N����

					gc_signal(0);

					// �Q�ƃJ�E���^�����炷
					add_ref_count_objects(alive, current, -1);
					
					alive = find_alive_objects(alive, current);

					// begin �` alive�܂ł̃I�u�W�F�N�g�͐����m��
					// alive �` current�܂ł̃I�u�W�F�N�g�͎��S�m��

					// ���҂��A�Q�ƃJ�E���^�����ɖ߂�
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

		vmachine_swap_temp();
	}
}

void ObjectSpace::register_gc(RefCountingBase* p){
	if(objects_count_==objects_max_){
		ScopeCounter cc(&cycle_count_);
		expand_objects_list();
	}

	*ConnectedPointer(objects_count_, objects_list_begin_) = p;
	objects_count_++;
}

void ObjectSpace::register_gc_vm(VMachine* p){
	if(gcvms_current_==gcvms_end_){
		expand_simple_dynamic_pointer_array(&gcvms_begin_, &gcvms_end_, &gcvms_current_, 64);
	}
	*gcvms_current_++ = p;
}

void ObjectSpace::unregister_gc_vm(VMachine* p){
	for(VMachine** it = gcvms_begin_; it!=gcvms_current_; ++it){
		if(*it==p){
			std::swap(*it, *--gcvms_current_);
			break;
		}
	}
}

}