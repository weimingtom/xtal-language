#include "xtal.h"
#include "xtal_macro.h"

//#define XTAL_DEBUG_ALLOC

#ifdef XTAL_DEBUG_ALLOC
#include <map>

struct SizeAndCount{
	SizeAndCount(int a = 0, int b = 0){
		size = a;
		count = b;
	}
	int size;
	int count;
};

std::map<void*, SizeAndCount> mem_map_;
std::map<void*, SizeAndCount> so_mem_map_;
int gcounter = 0;

void* debug_malloc(size_t size){
	void* ret = malloc(size);
	if(gcounter==18150){
		gcounter = gcounter;
	}
	mem_map_.insert(std::make_pair(ret, SizeAndCount(size, gcounter++)));
	return ret;
}

void debug_free(void* p){
	if(p){
		XTAL_ASSERT(mem_map_[p].size);
		memset(p, 0xcd, mem_map_[p].size);
		free(p);
		mem_map_.erase(p);
	}
}

void* debug_so_malloc(size_t size){
	void* ret = malloc(size);
	if(gcounter==18150){
		gcounter = gcounter;
	}
	so_mem_map_.insert(std::make_pair(ret, SizeAndCount(size, gcounter++)));
	return ret;
}

void debug_so_free(void* p, size_t sz){
	if(p){
		XTAL_ASSERT(so_mem_map_[p].size==sz);
		memset(p, 0xcd, so_mem_map_[p].size);
		free(p);
		so_mem_map_.erase(p);
	}
}

void display_debug_memory(){
	for(std::map<void*, SizeAndCount>::iterator it=mem_map_.begin(); it!=mem_map_.end(); ++it){
		int size = it->second.size;
		int count = it->second.count;
		size = size;
	}

	XTAL_ASSERT(mem_map_.empty()); // 全部開放できてない

	for(std::map<void*, SizeAndCount>::iterator it=so_mem_map_.begin(); it!=so_mem_map_.end(); ++it){
		int size = it->second.size;
		int count = it->second.count;
		size = size;
	}

	XTAL_ASSERT(so_mem_map_.empty()); // 全部開放できてない
}

#endif

namespace xtal{

void initialize_math();
void initialize_xpeg();

namespace{

	enum{
		OBJECTS_ALLOCATE_SHIFT = 12,
		OBJECTS_ALLOCATE_SIZE = 1 << OBJECTS_ALLOCATE_SHIFT,
		OBJECTS_ALLOCATE_MASK = OBJECTS_ALLOCATE_SIZE-1
	};

	Core* current_core_;
}

Core* core(){
	return current_core_;
}

void set_core(Core* core){
	current_core_ = core;
}

////////////////////////////////////

namespace{
	ThreadLib empty_thread_lib;
	StreamLib empty_stream_lib;
	FilesystemLib empty_filesystem_lib;
	AllocatorLib cstd_allocator_lib;
	ChCodeLib ascii_chcode_lib;
}

CoreSetting::CoreSetting(){
	thread_lib = &empty_thread_lib;
	stream_lib = &empty_stream_lib;
	filesystem_lib = &empty_filesystem_lib;
	allocator_lib = &cstd_allocator_lib;
	chcode_lib = &ascii_chcode_lib;
}

void Core::initialize(const CoreSetting& setting){
	set_core(this);

	setting_ = setting;

	objects_begin_  = 0;
	objects_current_ = 0;
	objects_end_ = 0;

	objects_list_begin_ = 0;
	objects_list_current_ = 0;
	objects_list_end_ = 0;

	gcobservers_begin_ = 0;
	gcobservers_current_ = 0;
	gcobservers_end_ = 0;

	cycle_count_ = 0;

	disable_gc();
	so_alloc_.init();

	expand_simple_dynamic_pointer_array((void**&)objects_begin_, (void**&)objects_end_, (void**&)objects_current_, OBJECTS_ALLOCATE_SIZE);
	expand_simple_dynamic_pointer_array((void**&)objects_list_begin_, (void**&)objects_list_end_, (void**&)objects_list_current_);
	*objects_list_current_++ = objects_begin_;

	cpp_class_map_.expand(4);

//////////////////

	global_mutate_count_ = 0;

	set_cpp_class(null, &CppClassSymbol<void>::value);

	ClassPtr* holders[] = { 
		&cpp_class_map_.insert(&CppClassSymbol<Any>::value, null).first->second,
		&cpp_class_map_.insert(&CppClassSymbol<Class>::value, null).first->second,
		&cpp_class_map_.insert(&CppClassSymbol<CppClass>::value, null).first->second,
		&cpp_class_map_.insert(&CppClassSymbol<Array>::value, null).first->second,
	};

	for(int i=0; i<sizeof(holders)/sizeof(holders[0]); ++i){
		*holders[i] = (ClassPtr&)ap(Any((Class*)Base::operator new(sizeof(CppClass))));
	}

	for(int i=0; i<sizeof(holders)/sizeof(holders[0]); ++i){
		Base* p = pvalue(*holders[i]);
		new(p) Base();
	}
		
	for(int i=0; i<sizeof(holders)/sizeof(holders[0]); ++i){
		Base* p = pvalue(*holders[i]);
		new(p) CppClass();
	}

	for(int i=0; i<sizeof(holders)/sizeof(holders[0]); ++i){
		Base* p = pvalue(*holders[i]);
		p->set_class(get_cpp_class<CppClass>());
	}
	
	for(int i=0; i<sizeof(holders)/sizeof(holders[0]); ++i){
		Base* p = pvalue(*holders[i]);
		register_gc(p);
	}

//////////

	set_cpp_class<Base>(get_cpp_class<Any>());
	set_cpp_class<Singleton>(get_cpp_class<CppClass>());
	set_cpp_class<IteratorClass>(get_cpp_class<CppClass>());

	builtin_ = xnew<Singleton>();
	lib_ = xnew<Lib>(true);
	lib_->append_load_path(".");
	Iterator_ = xnew<IteratorClass>();
	Iterable_ = xnew<Class>();

	vm_list_ = xnew<Array>();

	string_mgr_ = xnew<StringMgr>();

/////////////////////

	bind();

	initialize_xpeg();
	initialize_math();

////////////////////////
	enable_gc();
////////////////////////


	id_op_list_[id_op_call] = Xid(op_call);
	id_op_list_[id_op_pos] = Xid(op_pos);
	id_op_list_[id_op_neg] = Xid(op_neg);
	id_op_list_[id_op_com] = Xid(op_com);
	id_op_list_[id_op_at] = Xid(op_at);
	id_op_list_[id_op_set_at] = Xid(op_set_at);
	id_op_list_[id_op_range] = Xid(op_range);
	id_op_list_[id_op_add] = Xid(op_add);
	id_op_list_[id_op_cat] = Xid(op_cat);
	id_op_list_[id_op_sub] = Xid(op_sub);
	id_op_list_[id_op_mul] = Xid(op_mul);
	id_op_list_[id_op_div] = Xid(op_div);
	id_op_list_[id_op_mod] = Xid(op_mod);
	id_op_list_[id_op_and] = Xid(op_and);
	id_op_list_[id_op_or] = Xid(op_or);
	id_op_list_[id_op_xor] = Xid(op_xor);
	id_op_list_[id_op_shl] = Xid(op_shl);
	id_op_list_[id_op_shr] = Xid(op_shr);
	id_op_list_[id_op_ushr] = Xid(op_ushr);
	id_op_list_[id_op_eq] = Xid(op_eq);
	id_op_list_[id_op_lt] = Xid(op_lt);
	id_op_list_[id_op_in] = Xid(op_in);
	id_op_list_[id_op_inc] = Xid(op_inc);
	id_op_list_[id_op_dec] = Xid(op_dec);
	id_op_list_[id_op_add_assign] = Xid(op_add_assign);
	id_op_list_[id_op_cat_assign] = Xid(op_cat_assign);
	id_op_list_[id_op_sub_assign] = Xid(op_sub_assign);
	id_op_list_[id_op_mul_assign] = Xid(op_mul_assign);
	id_op_list_[id_op_div_assign] = Xid(op_div_assign);
	id_op_list_[id_op_mod_assign] = Xid(op_mod_assign);
	id_op_list_[id_op_and_assign] = Xid(op_and_assign);
	id_op_list_[id_op_or_assign] = Xid(op_or_assign);
	id_op_list_[id_op_xor_assign] = Xid(op_xor_assign);
	id_op_list_[id_op_shl_assign] = Xid(op_shl_assign);
	id_op_list_[id_op_shr_assign] = Xid(op_shr_assign);
	id_op_list_[id_op_ushr_assign] = Xid(op_ushr_assign);

	exec_script();
}

void Core::uninitialize(){
	if(!objects_begin_){ return; }
	
	full_gc();

	thread_mgr_->destroy();
	Iterator_ = null;
	Iterable_ = null;
	builtin_ = null;
	lib_ = null;
	vm_list_ = null;
	filesystem_ = null;

	string_mgr_ = null;

	full_gc();

	for(int i=0; i<id_op_MAX; ++i){
		id_op_list_[i] = null;
	}

	thread_mgr_ = null;

	full_gc();

	for(cpp_class_map_t::iterator it=cpp_class_map_.begin(); it!=cpp_class_map_.end(); ++it){
		it->second = null;
	}

	cpp_class_map_.destroy();
	full_gc();
	
	int n = (objects_list_current_ - objects_list_begin_ - 1)*OBJECTS_ALLOCATE_SIZE + (objects_current_ - objects_begin_);
	if(n != 0){
		//fprintf(stderr, "finished gc\n");
		//fprintf(stderr, " alive object = %d\n", objects_current_-objects_begin_);
		//print_alive_objects();

		for(int i=0; i<n; ++i){
			Base* p = objects_begin_[i];
			uint_t count = p->ref_count();
		}

		XTAL_ASSERT(false); // 全部開放できてない
	}

	objects_list_current_ = objects_list_begin_;

	fit_simple_dynamic_pointer_array((void**&)gcobservers_begin_, (void**&)gcobservers_end_, (void**&)gcobservers_current_);
	fit_simple_dynamic_pointer_array((void**&)objects_begin_, (void**&)objects_end_, (void**&)objects_current_);
	fit_simple_dynamic_pointer_array((void**&)objects_list_begin_, (void**&)objects_list_end_, (void**&)objects_list_current_);

	so_alloc_.release();

#ifdef XTAL_DEBUG_ALLOC
	display_debug_memory();
#endif

}

////////////////////////////////////

void* Core::so_malloc(size_t size){
#ifdef XTAL_DEBUG_ALLOC
	return debug_so_malloc(size);
#endif

	return so_alloc_.malloc(size);
}

void Core::so_free(void* p, size_t size){
#ifdef XTAL_DEBUG_ALLOC
	return debug_so_free(p, size);
#endif

	so_alloc_.free(p, size);
}

void* Core::user_malloc(size_t size){
	void* ret = user_malloc_nothrow(size);
	if(!ret){
		
	}
	return ret;
} 

void* Core::user_malloc_nothrow(size_t size){
#ifdef XTAL_DEBUG_ALLOC
	return debug_malloc(size);
#endif

	//full_gc();
		
	void* ret = setting_.allocator_lib->malloc(size);

	if(!ret){
		gc();
		ret = setting_.allocator_lib->malloc(size);

		if(!ret){
			full_gc();
			ret = setting_.allocator_lib->malloc(size);
		}
	}

	return ret;
} 

void Core::user_free(void* p){
#ifdef XTAL_DEBUG_ALLOC
	return debug_free(p);
#endif

	setting_.allocator_lib->free(p);
}

struct CycleCounter{
	uint_t* p;
	CycleCounter(uint_t* p):p(p){ *p+=1; }
	~CycleCounter(){ *p-=1; }
};

void Core::enable_gc(){
	cycle_count_++;
}

void Core::disable_gc(){
	cycle_count_--;
}

void Core::gc(){
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
			so_free(*it, (*it)->value_);
		}

		objects_current_ = objects_alive;

		restart_the_world();
	}
}


struct ConnectedPointer{
	int_t pos;
	Base**** bp;

	ConnectedPointer(int_t p, Base***& pp)
		:pos(p), bp(&pp){}

	Base*& operator *(){
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
};

void Core::full_gc(){
	if(cycle_count_!=0){ return; }
	if(stop_the_world()){
		CycleCounter cc(&cycle_count_);
				
		while(true){			
			ConnectedPointer current((objects_list_current_ - objects_list_begin_ - 1)*OBJECTS_ALLOCATE_SIZE + (objects_current_ - objects_begin_), objects_list_begin_);
			ConnectedPointer begin(0, objects_list_begin_);
			if(current==begin){
				break;
			}

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
					current = ConnectedPointer((objects_list_current_ - objects_list_begin_ - 1)*OBJECTS_ALLOCATE_SIZE + (objects_current_ - objects_begin_), objects_list_begin_);

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
				so_free(*it, (*it)->value_);
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

void Core::register_gc(Base* p){
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

void Core::register_gc_observer(GCObserver* p){
	if(gcobservers_current_==gcobservers_end_){
		expand_simple_dynamic_pointer_array((void**&)gcobservers_begin_, (void**&)gcobservers_end_, (void**&)gcobservers_current_);
	}
	*gcobservers_current_++ = p;
}

void Core::unregister_gc_observer(GCObserver* p){
	for(GCObserver** it = gcobservers_begin_; it!=gcobservers_current_; ++it){
		if(*it==p){
			std::swap(*it, *--gcobservers_current_);
			break;
		}
	}
}

const ClassPtr& Core::new_cpp_class(const StringPtr& name, void* key){
	ClassPtr& p = cpp_class_map_[key];
	if(!p){ p = xnew<CppClass>(name); }
	return p;
}
	
VMachinePtr Core::vm_take_over(){
	if(vm_list_->empty()){
		vm_list_->push_back(xnew<VMachine>());
	}
	VMachinePtr vm = unchecked_ptr_cast<VMachine>(vm_list_->back());
	vm_list_->pop_back();
	return vm;
}

void Core::vm_take_back(const VMachinePtr& vm){
	vm->reset();
	vm_list_->push_back(vm);
}

const AnyPtr& Core::MemberCacheTable::cache(const Any& target_class, const IDPtr& primary_key, const Any& secondary_key, const Any& self, bool inherited_too, uint_t global_mutate_count){
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

		if(type(target_class)!=TYPE_BASE){
			return undefined;
		}

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

bool Core::IsInheritedCacheTable::cache_is(const Any& target_class, const Any& klass, uint_t global_mutate_count){
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

bool Core::IsInheritedCacheTable::cache_is_inherited(const Any& target_class, const Any& klass, uint_t global_mutate_count){
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

#ifndef XTAL_NO_PARSER

CodePtr compile_file(const StringPtr& file_name){
	CodeBuilder cb;
	if(StreamPtr fs = open(file_name, Xid(r))){
		if(CodePtr fun = cb.compile(fs, file_name)){
			fs->close();
			return fun;
		}
		fs->close();
		XTAL_SET_EXCEPT(CompileError()->call(Xt("Xtal Runtime Error 1016")->call(Named(Xid(name), file_name)), cb.errors()->to_a()));
	}
	return null;
}

CodePtr compile(const StringPtr& source){
	CodeBuilder cb;
	StringStreamPtr ms(xnew<StringStream>(source));
	if(CodePtr fun =  cb.compile(ms, "<eval>")){
		return fun;
	}

	XTAL_SET_EXCEPT(CompileError()->call(Xt("Xtal Runtime Error 1002"), cb.errors()->to_a()));
	return null;
}

AnyPtr load(const StringPtr& file_name){
	if(CodePtr code = compile_file(file_name)){
		gc();
		return code->call();
	}
	return null;
}

AnyPtr load_and_save(const StringPtr& file_name){
	if(CodePtr code = compile_file(file_name)){
		StreamPtr fs = open(file_name->cat(Xid(c)), Xid(w));
		fs->serialize(code);
		fs->close();
		XTAL_CHECK_EXCEPT(e){
			return null;
		}
		gc();
		return code->call();
	}
	return null;
}

CodePtr source(const char_t* src, int_t size, const char* file){
	CodeBuilder cb;
	StreamPtr ms(xnew<PointerStream>(src, size));
	if(CodePtr fun = cb.compile(ms, file)){
		return fun;
	}

	XTAL_SET_EXCEPT(CompileError()->call(Xt("Xtal Runtime Error 1010")->call(), cb.errors()->to_a()));
	return null;
}

#endif

CodePtr compiled_source(const void* src, int_t size, const char* file){
	StreamPtr ms(xnew<PointerStream>(src, size));
	if(CodePtr fun = ptr_cast<Code>(ms->deserialize())){
		return fun;
	}
	return null;
}

}
