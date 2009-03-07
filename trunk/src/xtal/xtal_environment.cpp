#include "xtal.h"
#include "xtal_macro.h"

#include "xtal_objectspace.h"
#include "xtal_stringspace.h"
#include "xtal_threadspace.h"
#include "xtal_cache.h"
#include "xtal_codebuilder.h"

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

void bind();
void exec_script();

class Environment{
public:

	void initialize(const Setting& setting);
	void uninitialize();
	
	Setting setting_;
	SmallObjectAllocator so_alloc_;
	ObjectSpace object_space_;	
	StringSpace string_space_;
	ThreadSpace thread_space_;

	SmartPtr<Filesystem> filesystem_;

	ClassPtr Iterator_;
	ClassPtr Iterable_;
	ClassPtr builtin_;
	LibPtr lib_;

	ArrayPtr vm_list_;

	MemberCacheTable member_cache_table_;
	IsCacheTable is_cache_table_;
};

namespace{

	Environment* environment_;

	ThreadLib empty_thread_lib;
	StreamLib empty_stream_lib;
	FilesystemLib empty_filesystem_lib;
	AllocatorLib cstd_allocator_lib;
	ChCodeLib ascii_chcode_lib;
}

Environment* environment(){
	return environment_;
}

void set_environment(Environment* environment){
	environment_ = environment;
}

////////////////////////////////////

void* so_malloc(size_t size){
#if XTAL_DEBUG_ALLOC==2
	return debug_so_malloc(size);
#endif

	return environment_->so_alloc_.malloc(size);
}

void so_free(void* p, size_t size){
#if XTAL_DEBUG_ALLOC==2
	return debug_so_free(p, size);
#endif

	environment_->so_alloc_.free(p, size);
}

void* user_malloc(size_t size){
#if XTAL_DEBUG_ALLOC!=0
	return debug_malloc(size);
#endif

	void* ret = environment_->setting_.allocator_lib->malloc(size);

	if(!ret){
		gc();
		ret = environment_->setting_.allocator_lib->malloc(size);

		if(!ret){
			full_gc();
			ret = environment_->setting_.allocator_lib->malloc(size);
		}
	}

	return ret;
} 

void user_free(void* p){
#if XTAL_DEBUG_ALLOC!=0
	return debug_free(p);
#endif

	environment_->setting_.allocator_lib->free(p);
}


Setting::Setting(){
	thread_lib = &empty_thread_lib;
	stream_lib = &empty_stream_lib;
	filesystem_lib = &empty_filesystem_lib;
	allocator_lib = &cstd_allocator_lib;
	chcode_lib = &ascii_chcode_lib;
}


void initialize(const Setting& setting){
	environment_ = (Environment*)setting.allocator_lib->malloc(sizeof(Environment));
	new(environment_) Environment();
	environment_->initialize(setting);
}

void uninitialize(){
	AllocatorLib* allocacator_lib = environment_->setting_.allocator_lib;
	environment_->uninitialize();
	environment_->~Environment();
	allocacator_lib->free(environment_);
	environment_ = 0;
}

void Environment::initialize(const Setting& setting){
	setting_ = setting;

//////////
	
	object_space_.initialize();
	string_space_.initialize();

	set_cpp_class<Base>(cpp_class<Any>());
	set_cpp_class<Singleton>(cpp_class<CppClass>());
	set_cpp_class<IteratorClass>(cpp_class<CppClass>());

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
	thread_space_.initialize(setting_.thread_lib);

	setting_.stream_lib->initialize();
	builtin()->def(Xid(stdin), setting_.stream_lib->new_stdin_stream());
	builtin()->def(Xid(stdout), setting_.stream_lib->new_stdout_stream());
	builtin()->def(Xid(stderr), setting_.stream_lib->new_stderr_stream());

	initialize_xpeg();
	initialize_math();

	enable_gc();

	print_alive_objects();

	exec_script();
}

void Environment::uninitialize(){
	full_gc();

	Iterator_ = null;
	Iterable_ = null;
	builtin_ = null;
	lib_ = null;
	vm_list_ = null;
	filesystem_ = null;

	string_space_.uninitialize();

	full_gc();

	thread_space_.uninitialize();

	full_gc();

	object_space_.uninitialize();

	so_alloc_.release();

#if XTAL_DEBUG_ALLOC!=0
	display_debug_memory();
#endif

}
	
VMachinePtr vmachine_take_over(){
	Environment* environment = xtal::environment();
	if(environment->vm_list_->empty()){
		environment->vm_list_->push_back(xnew<VMachine>());
	}
	VMachinePtr vm = unchecked_ptr_cast<VMachine>(environment->vm_list_->back());
	environment->vm_list_->pop_back();
	return vm;
}

void vmachine_take_back(const VMachinePtr& vm){
	Environment* environment = xtal::environment();
	vm->reset();
	environment->vm_list_->push_back(vm);
}

void debug_print(){
	{
		int_t hit = environment()->member_cache_table_.hit_count();
		int_t miss = environment()->member_cache_table_.miss_count();
		int_t collided = environment()->member_cache_table_.collided_count();
		printf("member_cache_table hit=%d, miss=%d, collided=%d, rate=%f\n", hit, miss, collided, hit/(float_t)(hit+miss));
	}

	{
		int_t hit = environment()->is_cache_table_.hit_count();
		int_t miss = environment()->is_cache_table_.miss_count();
		int_t collided = environment()->is_cache_table_.collided_count();
		printf("is_cache_table hit=%d, miss=%d, collided=%d, rate=%f\n", hit, miss, collided, hit/(float_t)(hit+miss));
	}
}

const IDPtr* id_op_list(){
	return environment()->string_space_.id_op_list();
}

void gc(){
	return environment()->object_space_.gc();
}

void full_gc(){
	environment()->member_cache_table_.clear();
	environment()->is_cache_table_.clear();
	return environment()->object_space_.full_gc();
}

void disable_gc(){
	return environment()->object_space_.disable_gc();
}

void enable_gc(){
	return environment()->object_space_.enable_gc();
}

void register_gc(RefCountingBase* p){
	return environment()->object_space_.register_gc(p);
}

void register_gc_observer(GCObserver* p){
	return environment()->object_space_.register_gc_observer(p);
}

void unregister_gc_observer(GCObserver* p){
	return environment()->object_space_.unregister_gc_observer(p);
}

const ClassPtr& new_cpp_class(const StringPtr& name, CppClassSymbolData* key){
	return environment()->object_space_.new_cpp_class(name, key);
}

const ClassPtr& cpp_class(CppClassSymbolData* key){
	return environment()->object_space_.cpp_class(key);
}

bool exists_cpp_class(CppClassSymbolData* key){
	return environment()->object_space_.exists_cpp_class(key);
}

void set_cpp_class(const ClassPtr& cls, CppClassSymbolData* key){
	return environment()->object_space_.set_cpp_class(cls, key);
}

const AnyPtr& cache_member(const AnyPtr& target_class, const IDPtr& primary_key, const AnyPtr& secondary_key, int_t& accessibility){
	return environment()->member_cache_table_.cache(target_class, primary_key, secondary_key, accessibility);
}

bool cache_is(const AnyPtr& target_class, const AnyPtr& klass){
	return environment()->is_cache_table_.cache(target_class, klass);
}

void invalidate_cache_member(){
	environment()->member_cache_table_.invalidate();
}

void invalidate_cache_is(){
	environment()->is_cache_table_.invalidate();
	environment()->member_cache_table_.invalidate();
}

const VMachinePtr& vmachine(){
	return environment()->thread_space_.vmachine();
}

const ClassPtr& Iterator(){
	return environment()->Iterator_;
}

const ClassPtr& Iterable(){
	return environment()->Iterable_;
}

const ClassPtr& builtin(){
	return environment()->builtin_;
}

const LibPtr& lib(){
	return environment()->lib_;
}

const IDPtr& intern_literal(const char_t* str){
	return environment()->string_space_.insert_literal(str);
}

const IDPtr& intern(const char_t* str){
	return environment()->string_space_.insert(str);
}

const IDPtr& intern(const char_t* str, uint_t data_size){
	return environment()->string_space_.insert(str, data_size);
}

const IDPtr& intern(const char_t* str, uint_t data_size, uint_t hash){
	return environment()->string_space_.insert(str, data_size, hash);
}

AnyPtr interned_strings(){
	return environment()->string_space_.interned_strings();
}

bool thread_enabled(){
	return environment()->thread_space_.thread_enabled();
}

void yield_thread(){
	return environment()->thread_space_.yield_thread();
}

void sleep_thread(float_t sec){
	return environment()->thread_space_.sleep_thread(sec);
}

ThreadPtr new_thread(const AnyPtr& callback_fun){
	return environment()->thread_space_.new_thread(callback_fun);
}

MutexPtr new_mutex(){
	return environment()->thread_space_.new_mutex();
}

void lock_mutex(const MutexPtr& p){
	return environment()->thread_space_.lock_mutex(p);
}

void xlock(){
	environment()->thread_space_.xlock();
}

void xunlock(){
	environment()->thread_space_.xunlock();
}

bool stop_the_world(){
	return true;
}

void restart_the_world(){

}

void thread_entry(const ThreadPtr& thread){
	environment()->thread_space_.thread_entry(thread);
}

void register_thread(){
	environment()->thread_space_.register_thread();
}

void unregister_thread(){
	environment()->thread_space_.unregister_thread();
}

const SmartPtr<Filesystem>& filesystem(){
	return environment()->filesystem_;
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

int_t ch_len(char_t lead){
	return environment()->setting_.chcode_lib->ch_len(lead);
}

int_t ch_len2(const char_t* str){
	return environment()->setting_.chcode_lib->ch_len2(str);
}

StringPtr ch_inc(const char_t* data, int_t data_size){
	return environment()->setting_.chcode_lib->ch_inc(data, data_size);
}

int_t ch_cmp(const char_t* a, uint_t asize, const char_t* b, uint_t bsize){
	return environment()->setting_.chcode_lib->ch_cmp(a, asize, b, bsize);
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
	return ret;
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
