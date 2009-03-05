#include "xtal.h"
#include "xtal_macro.h"

#define XTAL_DEBUG_ALLOC 0

#if XTAL_DEBUG_ALLOC!=0
#include <map>
#include <typeinfo>
#include <string>

struct SizeAndCount{
	SizeAndCount(int a = 0, int b = 0){
		size = a;
		count = b;
		free = false;
	}
	int size;
	int count;
	bool free;
};

namespace{
std::map<void*, SizeAndCount> mem_map_;
std::map<void*, SizeAndCount> so_mem_map_;
int gcounter = 1;
int max_used_memory = 0;
int used_memory = 0;
const char debugstring[]= "awerwoeekrlwsekrlewskrkerswer";
}

void* debug_malloc(size_t size){
	//xtal::full_gc();
	//xtal::gc();
	void* ret = malloc(size + sizeof(debugstring));
	mem_map_.insert(std::make_pair(ret, SizeAndCount(size, gcounter++)));
	used_memory += size;

	if(gcounter==35){
		gcounter = gcounter;
	}

	if(size>3000){
		size = size;
	}
	
	if(max_used_memory<used_memory){
		max_used_memory = used_memory+1024; 
		printf("max used memory %dKB\n", max_used_memory/1024);
	}
		
	memset(ret, 0xda, size);
	memcpy((char*)ret+size, debugstring, sizeof(debugstring));

	return ret;
}

void debug_free(void* p){
	if(p){
		int gcount = mem_map_[p].count;
		XTAL_ASSERT(!mem_map_[p].free);
		used_memory -= mem_map_[p].size;
		mem_map_[p].free = true;

		XTAL_ASSERT(memcmp((char*)p+mem_map_[p].size, debugstring, sizeof(debugstring))==0);
		memset(p, 0xcd, mem_map_[p].size);

		//free(p);
		//mem_map_.erase(p);
	}
}

void* debug_so_malloc(size_t size){
	void* ret = malloc(size);
	so_mem_map_.insert(std::make_pair(ret, SizeAndCount(size, gcounter++)));
	used_memory += size;
	
	if(gcounter==1304 || gcounter==1858){
		gcounter = gcounter;
	}

	if(max_used_memory<used_memory){
		max_used_memory = used_memory+1024; 
		printf("max used memory %dKB\n", max_used_memory/1024);
	}

	memset(ret, 0xdd, size);

	return ret;
}

void debug_so_free(void* p, size_t sz){
	if(p){
		int gcount = so_mem_map_[p].count;
		XTAL_ASSERT(!so_mem_map_[p].free);
		int sizeeee = so_mem_map_[p].size;
		XTAL_ASSERT(so_mem_map_[p].size==sz);
		memset(p, 0xcd, so_mem_map_[p].size);
		used_memory -= so_mem_map_[p].size;
		so_mem_map_[p].free = true;
		//free(p);
		//so_mem_map_.erase(p);
	}
}

void display_debug_memory(){
	bool allfree = true;
	for(std::map<void*, SizeAndCount>::iterator it=mem_map_.begin(); it!=mem_map_.end(); ++it){
		int size = it->second.size;
		int count = it->second.count;
		size = size;
		if(!it->second.free){
			allfree = false;
		}
	}

	XTAL_ASSERT(allfree); // 全部開放できてない

	for(std::map<void*, SizeAndCount>::iterator it=so_mem_map_.begin(); it!=so_mem_map_.end(); ++it){
		int size = it->second.size;
		int count = it->second.count;
		size = size;
		if(!it->second.free){
			allfree = false;
		}
	}

	XTAL_ASSERT(allfree); // 全部開放できてない

	XTAL_ASSERT(used_memory==0);
}

#endif

namespace xtal{


void initialize_math();
void initialize_xpeg();

namespace{

	enum{
		OBJECTS_ALLOCATE_SHIFT = 8,
		OBJECTS_ALLOCATE_SIZE = 1 << OBJECTS_ALLOCATE_SHIFT,
		OBJECTS_ALLOCATE_MASK = OBJECTS_ALLOCATE_SIZE-1
	};

	Core* core_;

	ThreadLib empty_thread_lib;
	StreamLib empty_stream_lib;
	FilesystemLib empty_filesystem_lib;
	AllocatorLib cstd_allocator_lib;
	ChCodeLib ascii_chcode_lib;
}

Core* core(){
	return core_;
}

void set_core(Core* core){
	core_ = core;
}

////////////////////////////////////

void* so_malloc(size_t size){
#if XTAL_DEBUG_ALLOC==2
	return debug_so_malloc(size);
#endif

	return core_->so_alloc_.malloc(size);
}

void so_free(void* p, size_t size){
#if XTAL_DEBUG_ALLOC==2
	return debug_so_free(p, size);
#endif

	core_->so_alloc_.free(p, size);
}

void* user_malloc(size_t size){
	void* ret = user_malloc_nothrow(size);
	if(!ret){
		XTAL_ASSERT(false); // メモリーが完全に取得できなかった。
		// アサートで止めても事態は改善しないんだが、
		// 例外を使わないとしたら、一体ここはどうしたらいいんだろう？
	}
	return ret;
} 

void* user_malloc_nothrow(size_t size){
#if XTAL_DEBUG_ALLOC!=0
	return debug_malloc(size);
#endif

	void* ret = core_->setting_.allocator_lib->malloc(size);

	if(!ret){
		gc();
		ret = core_->setting_.allocator_lib->malloc(size);

		if(!ret){
			full_gc();
			ret = core_->setting_.allocator_lib->malloc(size);
		}
	}

	return ret;
} 

void user_free(void* p){
#if XTAL_DEBUG_ALLOC!=0
	return debug_free(p);
#endif

	core_->setting_.allocator_lib->free(p);
}


CoreSetting::CoreSetting(){
	thread_lib = &empty_thread_lib;
	stream_lib = &empty_stream_lib;
	filesystem_lib = &empty_filesystem_lib;
	allocator_lib = &cstd_allocator_lib;
	chcode_lib = &ascii_chcode_lib;
}

void ObjectMgr::initialize(){
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

	CppClassSymbolData* symbols[] = { 
		&CppClassSymbol<void>::value,
		&CppClassSymbol<Any>::value,
		&CppClassSymbol<Class>::value,
		&CppClassSymbol<CppClass>::value,
		&CppClassSymbol<Array>::value,
	};


	uint_t nsize = sizeof(symbols)/sizeof(symbols[0]);
	uint_t table[sizeof(symbols)/sizeof(symbols[0])];

	for(uint_t i=0; i<nsize; ++i){
		table[i] = register_cpp_class(symbols[i]);
	}

	for(uint_t i=0; i<nsize; ++i){
		class_table_[table[i]] = (Class*)Base::operator new(sizeof(CppClass));
	}

	for(uint_t i=0; i<nsize; ++i){
		Base* p = class_table_[table[i]];
		new(p) Base();
	}
		
	for(uint_t i=0; i<nsize; ++i){
		Base* p = class_table_[table[i]];
		new(p) CppClass();
	}

	for(uint_t i=0; i<nsize; ++i){
		Base* p = class_table_[table[i]];
		p->set_class(get_cpp_class<CppClass>());
	}
	
	for(uint_t i=0; i<nsize; ++i){
		Base* p = class_table_[table[i]];
		register_gc(p);
	}
}

void ObjectMgr::uninitialize(){
	for(uint_t i=0; i<class_table_.size(); ++i){
		if(class_table_[i]){
			class_table_[i]->dec_ref_count();
			class_table_[i] = 0;
		}
	}

	class_table_.release();
	full_gc();
	
	int n = (objects_list_current_ - objects_list_begin_ - 1)*OBJECTS_ALLOCATE_SIZE + (objects_current_ - objects_begin_);
	if(n != 0){
		//fprintf(stderr, "finished gc\n");
		//fprintf(stderr, " alive object = %d\n", objects_current_-objects_begin_);
		//print_alive_objects();

		for(int i=0; i<n; ++i){
			RefCountingBase* p = objects_begin_[i];
			Base* pp = (Base*)p;
			uint_t count = p->ref_count();
			count = count;
		}

		//XTAL_ASSERT(false); // 全部開放できてない
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



const ClassPtr& ObjectMgr::new_cpp_class(const StringPtr& name, CppClassSymbolData* key){
	int_t index = register_cpp_class(key);

	if(Class* p = class_table_[index]){
		return from_this(p);
	}

	class_table_[index] = xnew<CppClass>(name).get();
	class_table_[index]->inc_ref_count();
	return from_this(class_table_[index]);
}
	
VMachinePtr vmachine_take_over(){
	Core* core = xtal::core();
	if(core->vm_list_->empty()){
		core->vm_list_->push_back(xnew<VMachine>());
	}
	VMachinePtr vm = unchecked_ptr_cast<VMachine>(core->vm_list_->back());
	core->vm_list_->pop_back();
	return vm;
}

void vmachine_take_back(const VMachinePtr& vm){
	Core* core = xtal::core();
	vm->reset();
	core->vm_list_->push_back(vm);
}

const AnyPtr& MemberCacheTable::cache(const AnyPtr& target_class, const IDPtr& primary_key, const AnyPtr& secondary_key, int_t& accessibility){
	//bool nocache = false;
	//return pvalue(target_class)->do_member(primary_key, secondary_key, true, accessibility, nocache);

	uint_t itarget_class = rawvalue(target_class)>>2;
	uint_t iprimary_key = rawvalue(primary_key);
	uint_t isecondary_key = rawvalue(secondary_key);

	uint_t hash = (itarget_class ^ iprimary_key ^ isecondary_key) ^ (iprimary_key>>3)*3 ^ isecondary_key*7;
	Unit& unit = table_[calc_index(hash)];

	//if(mutate_count_==unit.mutate_count && 
	//	raweq(primary_key, unit.primary_key) && 
	//	raweq(target_class, unit.target_class) &&
	//	raweq(secondary_key, unit.secondary_key)){
	if(((mutate_count_^unit.mutate_count) | 
		rawbitxor(primary_key, unit.primary_key) | 
		rawbitxor(target_class, unit.target_class) | 
		rawbitxor(secondary_key, unit.secondary_key))==0){
		hit_++;
		accessibility = unit.accessibility;
		return ap(unit.member);
	}
	else{

		// 次の番地にあるか調べる
		//Unit& unit2 = table_[calc_index(hash + 1)];
		//if(mutate_count_==unit2.mutate_count && 
		//	raweq(primary_key, unit2.primary_key) && 
		//	raweq(target_class, unit2.target_class) &&
		//	raweq(secondary_key, unit2.secondary_key)){
		//	collided_++;
		//	hit_++;
		//	accessibility = unit2.accessibility;
		//	return ap(unit2.member);
		//}

		miss_++;

		if(type(target_class)!=TYPE_BASE){
			return undefined;
		}

		// 今の番地にあるのが有効なキャッシュなら、それを退避させる
		//if(unit.mutate_count==mutate_count_){
		//	unit2 = unit;
		//}

		bool nocache = false;
		const AnyPtr& ret = pvalue(target_class)->do_member(primary_key, ap(secondary_key), true, accessibility, nocache);
		//if(rawne(ret, undefined)){
			unit.member = ret;
			if(!nocache){
				unit.target_class = target_class;
				unit.primary_key = primary_key;
				unit.secondary_key = secondary_key;
				unit.accessibility = accessibility;
				unit.mutate_count = mutate_count_;
			}
			else{
				unit.mutate_count = mutate_count_-1;
			}
			return ap(unit.member);
		//}
		//return undefined;
	}
}

bool IsCacheTable::cache(const AnyPtr& target_class, const AnyPtr& klass){
	//return unchecked_ptr_cast<Class>(target_class)->is_inherited(klass);

	uint_t itarget_class = rawvalue(target_class);
	uint_t iklass = rawvalue(klass);

	uint_t hash = (itarget_class>>3) ^ (iklass>>2);
	Unit& unit = table_[hash & CACHE_MASK];
	
	if(mutate_count_==unit.mutate_count && 
		raweq(target_class, unit.target_class) && 
		raweq(klass, unit.klass)){
		hit_++;
		return unit.result;
	}
	else{
		// 次の番地にあるか調べる
		//Unit& unit2 = table_[calc_index(hash + 1)];
		//if(mutate_count_==unit2.mutate_count && 
		//	raweq(target_class, unit2.target_class) && 
		//	raweq(klass, unit2.klass)){
		//	collided_++;
		//	hit_++;
		//	return unit2.result;
		//}

		miss_++;

		// 今の番地にあるのが有効なキャッシュなら、それを退避させる
		//if(unit.mutate_count==mutate_count_){
		//	unit2 = unit;
		//}

		bool ret = unchecked_ptr_cast<Class>(ap(target_class))->is_inherited(ap(klass));

		// キャッシュに保存
		//if(ret){
			unit.target_class = target_class;
			unit.klass = klass;
			unit.mutate_count = mutate_count_;
			unit.result = ret;
		//}
		return ret;
	}
}

void initialize(const CoreSetting& setting){
	core_ = (Core*)setting.allocator_lib->malloc(sizeof(Core));
	new(core_) Core();
	core_->initialize(setting);
}

void uninitialize(){
	AllocatorLib* allocacator_lib = core_->setting_.allocator_lib;
	core_->uninitialize();
	core_->~Core();
	allocacator_lib->free(core_);
	core_ = 0;
}

void Core::initialize(const CoreSetting& setting){
	setting_ = setting;

//////////
	
	string_mgr_.initialize();
	object_mgr_.initialize();

	set_cpp_class<Base>(get_cpp_class<Any>());
	set_cpp_class<Singleton>(get_cpp_class<CppClass>());
	set_cpp_class<IteratorClass>(get_cpp_class<CppClass>());

	builtin_ = xnew<Singleton>();
	lib_ = xnew<Lib>(true);
	lib_->append_load_path(".");
	Iterator_ = xnew<IteratorClass>();
	Iterable_ = xnew<Class>();
	vm_list_ = xnew<Array>();

	bind();

	setting_.filesystem_lib->initialize();
	filesystem_ = new_cpp_singleton<Filesystem>();
	filesystem_->initialize(setting_.filesystem_lib);

	{
		filesystem()->def_singleton_method(Xid(open), &Filesystem::open);
		filesystem()->def_singleton_method(Xid(entries), &Filesystem::entries);
		filesystem()->def_singleton_method(Xid(is_directory), &Filesystem::is_directory);
	}

	builtin()->def(Xid(filesystem), filesystem_);

	setting_.thread_lib->initialize();
	thread_mgr_.initialize(setting_.thread_lib);

	setting_.stream_lib->initialize();
	builtin()->def(Xid(stdin), setting_.stream_lib->new_stdin_stream());
	builtin()->def(Xid(stdout), setting_.stream_lib->new_stdout_stream());
	builtin()->def(Xid(stderr), setting_.stream_lib->new_stderr_stream());

	initialize_xpeg();
	initialize_math();

	enable_gc();

////////////////////////

	id_op_list_[IDOp::id_op_call] = Xid(op_call);
	id_op_list_[IDOp::id_op_pos] = Xid(op_pos);
	id_op_list_[IDOp::id_op_neg] = Xid(op_neg);
	id_op_list_[IDOp::id_op_com] = Xid(op_com);
	id_op_list_[IDOp::id_op_at] = Xid(op_at);
	id_op_list_[IDOp::id_op_set_at] = Xid(op_set_at);
	id_op_list_[IDOp::id_op_range] = Xid(op_range);
	id_op_list_[IDOp::id_op_add] = Xid(op_add);
	id_op_list_[IDOp::id_op_cat] = Xid(op_cat);
	id_op_list_[IDOp::id_op_sub] = Xid(op_sub);
	id_op_list_[IDOp::id_op_mul] = Xid(op_mul);
	id_op_list_[IDOp::id_op_div] = Xid(op_div);
	id_op_list_[IDOp::id_op_mod] = Xid(op_mod);
	id_op_list_[IDOp::id_op_and] = Xid(op_and);
	id_op_list_[IDOp::id_op_or] = Xid(op_or);
	id_op_list_[IDOp::id_op_xor] = Xid(op_xor);
	id_op_list_[IDOp::id_op_shl] = Xid(op_shl);
	id_op_list_[IDOp::id_op_shr] = Xid(op_shr);
	id_op_list_[IDOp::id_op_ushr] = Xid(op_ushr);
	id_op_list_[IDOp::id_op_eq] = Xid(op_eq);
	id_op_list_[IDOp::id_op_lt] = Xid(op_lt);
	id_op_list_[IDOp::id_op_in] = Xid(op_in);
	id_op_list_[IDOp::id_op_inc] = Xid(op_inc);
	id_op_list_[IDOp::id_op_dec] = Xid(op_dec);
	id_op_list_[IDOp::id_op_add_assign] = Xid(op_add_assign);
	id_op_list_[IDOp::id_op_cat_assign] = Xid(op_cat_assign);
	id_op_list_[IDOp::id_op_sub_assign] = Xid(op_sub_assign);
	id_op_list_[IDOp::id_op_mul_assign] = Xid(op_mul_assign);
	id_op_list_[IDOp::id_op_div_assign] = Xid(op_div_assign);
	id_op_list_[IDOp::id_op_mod_assign] = Xid(op_mod_assign);
	id_op_list_[IDOp::id_op_and_assign] = Xid(op_and_assign);
	id_op_list_[IDOp::id_op_or_assign] = Xid(op_or_assign);
	id_op_list_[IDOp::id_op_xor_assign] = Xid(op_xor_assign);
	id_op_list_[IDOp::id_op_shl_assign] = Xid(op_shl_assign);
	id_op_list_[IDOp::id_op_shr_assign] = Xid(op_shr_assign);
	id_op_list_[IDOp::id_op_ushr_assign] = Xid(op_ushr_assign);

	print_alive_objects();

	exec_script();
}

void Core::uninitialize(){
	full_gc();

	Iterator_ = null;
	Iterable_ = null;
	builtin_ = null;
	lib_ = null;
	vm_list_ = null;
	filesystem_ = null;

	string_mgr_.uninitialize();
	for(int i=0; i<IDOp::id_op_MAX; ++i){
		id_op_list_[i] = null;
	}

	full_gc();

	thread_mgr_.uninitialize();

	full_gc();

	object_mgr_.uninitialize();

	so_alloc_.release();

#if XTAL_DEBUG_ALLOC!=0
	display_debug_memory();
#endif

}

void debug_print(){
	{
		int_t hit = core()->member_cache_table_.hit_count();
		int_t miss = core()->member_cache_table_.miss_count();
		int_t collided = core()->member_cache_table_.collided_count();
		printf("member_cache_table hit=%d, miss=%d, collided=%d, rate=%f\n", hit, miss, collided, hit/(float_t)(hit+miss));
	}

	{
		int_t hit = core()->is_cache_table_.hit_count();
		int_t miss = core()->is_cache_table_.miss_count();
		int_t collided = core()->is_cache_table_.collided_count();
		printf("is_cache_table hit=%d, miss=%d, collided=%d, rate=%f\n", hit, miss, collided, hit/(float_t)(hit+miss));
	}
}

////////////////////////////////////


struct CycleCounter{
	uint_t* p;
	CycleCounter(uint_t* p):p(p){ *p+=1; }
	~CycleCounter(){ *p-=1; }
};

void ObjectMgr::enable_gc(){
	cycle_count_++;
}

void ObjectMgr::disable_gc(){
	cycle_count_--;
}
	
void ObjectMgr::expand_objects_list(){
	if(objects_list_current_==objects_list_end_){
		expand_simple_dynamic_pointer_array((void**&)objects_list_begin_, (void**&)objects_list_end_, (void**&)objects_list_current_, 4);
		for(RefCountingBase*** it=objects_list_current_; it!=objects_list_end_; ++it){
			RefCountingBase** begin = 0;
			RefCountingBase** current = 0;
			RefCountingBase** end = 0;
			expand_simple_dynamic_pointer_array((void**&)begin, (void**&)end, (void**&)current, OBJECTS_ALLOCATE_SIZE);
			*it = begin;
		}
	}
}

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

void print_alive_objects(){
#if XTAL_DEBUG_ALLOC!=0
/*	full_gc();

	ConnectedPointer current(objects_count_, objects_list_begin_);
	ConnectedPointer begin(0, objects_list_begin_);

	std::map<std::string, int> table;
	for(ConnectedPointer it = begin; it!=current; ++it){
		switch(type(**it)){
		XTAL_DEFAULT;
//		XTAL_CASE(TYPE_BASE){ table[typeid(*pvalue(**it)).name()]++; }
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

	printf("used_memory %d\n", used_memory);
*/
#endif
}

void ObjectMgr::gc(){
	if(cycle_count_!=0){ return; }
	if(stop_the_world()){
		CycleCounter cc(&cycle_count_);
		
		ConnectedPointer current(objects_count_, objects_list_begin_);
		ConnectedPointer begin(prev_objects_count_>objects_count_ ? objects_count_ : (prev_objects_count_-prev_objects_count_/4), objects_list_begin_);
		if(current==begin){
			restart_the_world();
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

		restart_the_world();
	}
}

void ObjectMgr::full_gc(){
	if(cycle_count_!=0){ return; }
	if(stop_the_world()){
		CycleCounter cc(&cycle_count_);
		//printf("used memory %dKB\n", used_memory/1024);
				
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

			//if(string_mgr_){
			//	string_mgr_->gc();
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

//		printf("used memory %dKB\n", used_memory/1024);
		restart_the_world();
	}
}

void ObjectMgr::register_gc(RefCountingBase* p){
	p->inc_ref_count();

	if(objects_current_==objects_end_){
		CycleCounter cc(&cycle_count_);
		//gc();
		expand_objects_list();
		objects_begin_ = objects_current_ = *objects_list_current_++;
		objects_end_ = objects_begin_+OBJECTS_ALLOCATE_SIZE;
	}

	*objects_current_++ = p;
	objects_count_++;
}

void ObjectMgr::register_gc_observer(GCObserver* p){
	if(gcobservers_current_==gcobservers_end_){
		expand_simple_dynamic_pointer_array((void**&)gcobservers_begin_, (void**&)gcobservers_end_, (void**&)gcobservers_current_);
	}
	*gcobservers_current_++ = p;
}

void ObjectMgr::unregister_gc_observer(GCObserver* p){
	for(GCObserver** it = gcobservers_begin_; it!=gcobservers_current_; ++it){
		if(*it==p){
			std::swap(*it, *--gcobservers_current_);
			break;
		}
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
	CodePtr ret;

	{
		if(StreamPtr fs = open(file_name, Xid(r))){
			CodeBuilder cb;
			if(CodePtr fun = cb.compile(fs, file_name)){
				fs->close();
				ret = fun;
			}
			else{
				fs->close();
				XTAL_SET_EXCEPT(CompileError()->call(Xt("Xtal Runtime Error 1016")->call(Named(Xid(name), file_name)), cb.errors()->to_a()));
				return null;
			}
		}
		else{
			return null;
		}
	}

	full_gc();
	return ret;
}

CodePtr compile(const StringPtr& source){
	CodePtr ret;

	{
		CodeBuilder cb;
		StringStreamPtr ms(xnew<StringStream>(source));
		if(CodePtr fun =  cb.compile(ms, "<eval>")){
			ret = fun;
		}
		else{
			XTAL_SET_EXCEPT(CompileError()->call(Xt("Xtal Runtime Error 1002"), cb.errors()->to_a()));
			return null;
		}
	}

	full_gc();
	return ret;
}

AnyPtr load(const StringPtr& file_name){
	AnyPtr ret;
	if(CodePtr code = compile_file(file_name)){
		ret = code->call();
	}
	gc();
	return null;
}

AnyPtr load_and_save(const StringPtr& file_name){
	if(CodePtr code = compile_file(file_name)){
		StreamPtr fs = open(file_name->cat(Xid(c)), Xid(w));
		fs->serialize(code);
		fs->close();
		XTAL_CHECK_EXCEPT(e){
			XTAL_UNUSED_VAR(e);
			return null;
		}
		gc();
		return code->call();
	}
	return null;
}

CodePtr source(const char_t* src, int_t size, const char* file){
	CodePtr ret;

	{
		CodeBuilder cb;
		StreamPtr ms(xnew<PointerStream>(src, size));
		if(CodePtr fun = cb.compile(ms, file)){
			ret = fun;
		}
		else{
			XTAL_SET_EXCEPT(CompileError()->call(Xt("Xtal Runtime Error 1010")->call(), cb.errors()->to_a()));
			return null;
		}
	}

	full_gc();
	return ret;
}

#endif

CodePtr compiled_source(const void* src, int_t size, const char* file){
	StreamPtr ms(xnew<PointerStream>(src, size));
	if(CodePtr fun = ptr_cast<Code>(ms->deserialize())){
		gc();
		return fun;
	}
	return null;
}

}
