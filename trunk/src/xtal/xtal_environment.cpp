#include "xtal.h"
#include "xtal_macro.h"

#include "xtal_details.h"
#include "xtal_codebuilder.h"
#include "xtal_filesystem.h"

//#define XTAL_DEBUG_ALLOC

#ifdef XTAL_DEBUG_ALLOC
#include <map>
#include <typeinfo>
#include <string>

class DebugAllocatorLib : public xtal::AllocatorLib{
public:
	virtual void* malloc(std::size_t size){ 
		return debug_malloc(size); 
	}

	virtual void free(void* p, std::size_t size){ 
		debug_free(p, size); 
	}

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

	std::map<void*, SizeAndCount> mem_map_;
	int gcounter_;
	int used_memory_;
	int max_used_memory_;
	char debugstring_[32];

	DebugAllocatorLib(){
		gcounter_ = 1;
		used_memory_ = 0;
		max_used_memory_ = 0;
		memcpy(debugstring_, "12345678901234567890123456789012", 32);
	}

	void* debug_malloc(size_t size){
		//xtal::full_gc();
		//xtal::gc();
		void* ret = std::malloc(size + sizeof(debugstring_));
		mem_map_.insert(std::make_pair(ret, SizeAndCount(size, gcounter_)));
		used_memory_ += size;

		if(gcounter_==2011){
			gcounter_ = gcounter_;
		}

		gcounter_++;

		if(size>30000){
			size = size;
		}
		
		if(max_used_memory_<used_memory_){
			max_used_memory_ = used_memory_+1024; 
			printf("max used memory %dKB\n", max_used_memory_/1024);
		}
			
		memset(ret, 0xda, size);
		memcpy((char*)ret+size, debugstring_, sizeof(debugstring_));

		return ret;
	}

	void debug_free(void* p, size_t size){
		if(p){
			int gcount = mem_map_[p].count;

			if(gcount==13728){
				gcount = gcount;
			}

			XTAL_ASSERT(mem_map_[p].size==size);
			XTAL_ASSERT(!mem_map_[p].free);
			used_memory_ -= mem_map_[p].size;
			mem_map_[p].free = true;

			XTAL_ASSERT(memcmp((char*)p+mem_map_[p].size, debugstring_, sizeof(debugstring_))==0);
			memset(p, 0xcd, mem_map_[p].size);

			//*
			std::free(p);
			mem_map_.erase(p);
			//*/
		}
	}

	void display_debug_memory(){
		int notfree = 0;
		for(std::map<void*, SizeAndCount>::iterator it=mem_map_.begin(); it!=mem_map_.end(); ++it){
			int size = it->second.size;
			int count = it->second.count;
			size = size;
			if(!it->second.free){
				notfree++;
			}
		}

		XTAL_ASSERT(notfree==0); // 全部開放できてない
		XTAL_ASSERT(used_memory_==0);
	}

};

#endif

namespace xtal{

void initialize_math();
void initialize_xpeg();

void bind();
void exec_script();

XTAL_TLS_PTR(Environment) environment_;
XTAL_TLS_PTR(VMachine) vmachine_;

namespace{
	ThreadLib empty_thread_lib;
	StdStreamLib empty_std_stream_lib;
	FilesystemLib empty_filesystem_lib;
	AllocatorLib cstd_allocator_lib;
	ChCodeLib ascii_ch_code_lib;
}

Environment* environment(){
	return environment_;
}

void set_environment(Environment* environment){
	environment_ = environment;
}

const VMachinePtr& vmachine(){
	return to_smartptr((VMachine*)vmachine_);
}

const VMachinePtr& vmachine_checked(){
	return vmachine_ ? to_smartptr((VMachine*)vmachine_) : unchecked_ptr_cast<VMachine>(null);
}

void set_vmachine(const VMachinePtr& vm){
	if(vmachine_){
		vmachine_->dec_ref_count();
	}

	if(vm){
		vmachine_ = vm.get();
		vmachine_->inc_ref_count();
	}
	else{
		vmachine_ = 0;
	}
}


////////////////////////////////////

void* xmalloc(size_t size){
	Environment* env = environment_;
	
	if(env->gc_stress_){
		full_gc();
	}

#if !defined(XTAL_NO_SMALL_ALLOCATOR) && !defined(XTAL_DEBUG_ALLOC)
	if(size<=SmallObjectAllocator::HANDLE_MAX_SIZE){
		return env->so_alloc_.malloc(size);
	}
#endif

	env->used_memory_ += size;

	if(env->used_memory_>env->memory_threshold_){
		env->object_space_.lw_gc();
		env->memory_threshold_ = env->used_memory_ + 1024*20;
		//printf("mem %dKB\n", env->memory_threshold_/1024);
	}

	void* ret = env->setting_.allocator_lib->malloc(size);

	if(!ret){
		env->object_space_.gc();
		ret = env->setting_.allocator_lib->malloc(size);

		if(!ret){
			env->object_space_.full_gc();
			ret = env->setting_.allocator_lib->malloc(size);

			if(!ret){
				ret = env->setting_.allocator_lib->out_of_memory(size);

				if(!ret){
					// だめだ。メモリが確保できない。
					// XTAL_MEMORYまでジャンプしよう。

					// XTAL_MEMORYで囲まれていない！もうどうしようもない！
					XTAL_ASSERT(env->set_jmp_buf_);
					
					env->ignore_memory_assert_= true;
					longjmp(env->jmp_buf_.buf, 1);
				}
			}
		}
	}

	return ret;
} 

void xfree(void* p, size_t size){
	Environment* env = environment_;

	if(!p){
		return;
	}

#if !defined(XTAL_NO_SMALL_ALLOCATOR) && !defined(XTAL_DEBUG_ALLOC)
	if(size<=SmallObjectAllocator::HANDLE_MAX_SIZE){	
		env->so_alloc_.free(p, size);
		return;
	}
#endif

	env->used_memory_ -= size;
	env->setting_.allocator_lib->free(p, size);
}

JmpBuf& protect(){
	// XTAL_PROTECTが入れ子になっている場合assertに引っかかる
	XTAL_ASSERT(!environment_->set_jmp_buf_);

	environment_->set_jmp_buf_ = true;
	return environment_->jmp_buf_;
}

void reset_protect(){
	if(environment_){
		environment_->set_jmp_buf_ = false;
	}
}

bool ignore_memory_assert(){
	return environment_->ignore_memory_assert_;
}

Setting::Setting(){
	thread_lib = &empty_thread_lib;
	std_stream_lib = &empty_std_stream_lib;
	filesystem_lib = &empty_filesystem_lib;
	allocator_lib = &cstd_allocator_lib;
	ch_code_lib = &ascii_ch_code_lib;
}


void initialize(const Setting& setting){
#ifdef XTAL_DEBUG_ALLOC
	Setting setting2 = setting;
	setting2.allocator_lib = new DebugAllocatorLib;
	environment_ = (Environment*)setting2.allocator_lib->malloc(sizeof(Environment));
	new(environment_) Environment();
	environment_->initialize(setting2);
#else
	environment_ = (Environment*)setting.allocator_lib->malloc(sizeof(Environment));
	new(environment_) Environment();
	environment_->initialize(setting);
#endif
}

void uninitialize(){
	if(!environment_){
		return;
	}

	AllocatorLib* allocacator_lib = environment_->setting_.allocator_lib;
	environment_->uninitialize();
	environment_->~Environment();
	allocacator_lib->free(environment_, sizeof(Environment));
	environment_ = 0;

#ifdef XTAL_DEBUG_ALLOC
	((DebugAllocatorLib*)allocacator_lib)->display_debug_memory();
	delete ((DebugAllocatorLib*)allocacator_lib);
#endif

}

class cpp_classes;

XTAL_BIND(cpp_classes){
	def_all_cpp_classes();
}

void Environment::initialize(const Setting& setting){
	setting_ = setting;

//////////

	gc_stress_ = false;

	set_jmp_buf_ = false;
	ignore_memory_assert_ = false;
	used_memory_ = 0;
	memory_threshold_ = 1024*5;
	
	object_space_.initialize();
	string_space_.initialize();

	builtin_ = xnew<Class>();
	builtin_->set_singleton();

	lib_ = xnew<Lib>(Lib::most_top_level_t());
	lib_->append_load_path(XTAL_STRING("."));

	global_ = xnew<Global>();
	global_->set_singleton();
	builtin_->inherit(global_);
	builtin_->def(Xid(global), global_);

	vm_list_ = xnew<Array>();
	text_map_ = xnew<Map>();
	nfa_map_ = xnew<Map>();

	lib_->def(Xid(builtin), builtin_);

	cpp_ = cpp_class<cpp_classes>();
	cpp_->set_singleton();
	builtin_->def(Xid(cpp), cpp_);

	cpp_->set_object_force(100);

	bind();

	builtin_->def(Xid(filesystem), cpp_class<filesystem::Filesystem>());

	thread_space_.initialize(setting_.thread_lib);
	
	cpp_class<StdinStream>()->inherit(cpp_class<Stream>());
	cpp_class<StdoutStream>()->inherit(cpp_class<Stream>());
	cpp_class<StderrStream>()->inherit(cpp_class<Stream>());

	stdin_ = xnew<StdinStream>();
	stdout_ = xnew<StdoutStream>();
	stderr_ = xnew<StderrStream>();

	builtin_->def(Xid(stdin), stdin_);
	builtin_->def(Xid(stdout), stdout_);
	builtin()->def(Xid(stderr), stderr_);

	builtin_->def(Xid(debug), cpp_class<debug::Debug>());

	initialize_math();
	initialize_xpeg();

	enable_gc();
	exec_script();

	//bind_all();

	full_gc();
	object_space_.finish_initialize();
}

void Environment::uninitialize(){

//#ifdef XTAL_DEBUG_PRINT
	{
		//printf("member hit=%d miss=%d rate=%g\n", member_cache_table_.hit_count(), member_cache_table_.miss_count(), member_cache_table_.hit_count()/(float)(member_cache_table_.hit_count()+member_cache_table_.miss_count()));
		//printf("is hit=%d miss=%d rate=%g\n", is_cache_table_.hit_count(), is_cache_table_.miss_count(), is_cache_table_.hit_count()/(float)(is_cache_table_.hit_count()+is_cache_table_.miss_count()));
		//printf("ctor hit=%d miss=%d rate=%g\n", ctor_cache_table_.hit_count(), ctor_cache_table_.miss_count(), ctor_cache_table_.hit_count()/(float)(ctor_cache_table_.hit_count()+ctor_cache_table_.miss_count()));
	}
//#endif

	thread_space_.join_all_threads();

	clear_cache();
	full_gc();

	builtin_ = null;
	cpp_ = null;
	lib_ = null;
	global_ = null;
	vm_list_ = null;
	nfa_map_ = null;

	stdin_ = null;
	stdout_ = null;
	stderr_ = null;
	text_map_ = null;

	string_space_.uninitialize();
	thread_space_.uninitialize();
	object_space_.uninitialize();

#ifndef XTAL_NO_SMALL_ALLOCATOR
	so_alloc_.release();
#endif
}
	
VMachinePtr vmachine_take_over(){
	Environment* environment = xtal::environment_;
	if(environment->vm_list_->empty()){
		environment->vm_list_->push_back(xnew<VMachine>());
	}
	VMachinePtr vm = unchecked_ptr_cast<VMachine>(environment->vm_list_->back());
	environment->vm_list_->pop_back();
	return vm;
}

void vmachine_take_back(const VMachinePtr& vm){
	Environment* environment = xtal::environment_;
	vm->reset();
	if(environment->vm_list_->length()<16){
		environment->vm_list_->push_back(vm);
	}
}

void vmachine_swap_temp(){
	Environment* environment = xtal::environment_;
	environment->thread_space_.swap_temp();
}


const IDPtr* id_op_list(){
	return environment_->string_space_.id_op_list();
}

void lw_gc(){
	return environment_->object_space_.lw_gc();
}

void gc(){
	return environment_->object_space_.gc();
}

void full_gc(){
	environment_->object_space_.full_gc();
#ifndef XTAL_NO_SMALL_ALLOCATOR
	environment_->so_alloc_.fit();
#endif

#ifdef XTAL_DEBUG_PRINT
	//printf("used_memory %gKB\n", environment_->used_memory_/1024.0f);
	//environment_->object_space_.print_alive_objects();
	//environment_->so_alloc_.print();
#endif

}

void disable_gc(){
	return environment_->object_space_.disable_gc();
}

void enable_gc(){
	return environment_->object_space_.enable_gc();
}

void set_gc_stress(bool b){
	environment_->gc_stress_ = b;
}

void register_gc(RefCountingBase* p){
	return environment_->object_space_.register_gc(p);
}

void register_gc_observer(GCObserver* p){
	return environment_->object_space_.register_gc_observer(p);
}

void unregister_gc_observer(GCObserver* p){
	return environment_->object_space_.unregister_gc_observer(p);
}

uint_t alive_object_count(){
	return environment_->object_space_.alive_object_count();
}

AnyPtr alive_object(uint_t i){
	return to_smartptr(environment_->object_space_.alive_object(i));
}

const ClassPtr& cpp_class(CppClassSymbolData* key){
	return environment_->object_space_.cpp_class(key->value);
}

const ClassPtr& cpp_class(int_t index){
	return environment_->object_space_.cpp_class(index);
}

const AnyPtr& cpp_var(CppVarSymbolData* key){
	return environment_->object_space_.cpp_var(key->value);
}

void bind_all(){
	environment_->object_space_.bind_all();
}

void def_all_cpp_classes(){
	environment_->object_space_.def_all_cpp_classes();
}

bool cache_is(const AnyPtr& target_class, const AnyPtr& klass){
	return environment_->is_cache_table_.cache(target_class, klass);
}

bool cache_ctor(const AnyPtr& target_class, int_t kind){
	return environment_->ctor_cache_table_.cache(target_class, kind);
}

void clear_cache(){
	environment_->member_cache_table_.clear();
	environment_->is_cache_table_.clear();
	environment_->ctor_cache_table_.clear();
}

void invalidate_cache_member(){
	environment_->member_cache_table_.invalidate();
}

void invalidate_cache_is(){
	environment_->is_cache_table_.invalidate();
	environment_->member_cache_table_.invalidate();
	environment_->ctor_cache_table_.invalidate();
}

void invalidate_cache_ctor(){
	environment_->ctor_cache_table_.invalidate();
}

const ClassPtr& builtin(){
	return environment_->builtin_;
}

const ClassPtr& cpp(){
	return environment_->cpp_;
}

const LibPtr& lib(){
	return environment_->lib_;
}

const ClassPtr& global(){
	return environment_->global_;
}

namespace{
	
const IDPtr& intern(const char_t* str, uint_t size, uint_t hashcode, bool literal){
	return environment_->string_space_.insert(str, size, hashcode, literal);
}

}

const IDPtr& intern(const char_t* str){
	uint_t hashcode, size;
	string_data_size_and_hashcode(str, size, hashcode);
	return intern(str, size, hashcode, false);
}

const IDPtr& intern(const char_t* str, uint_t data_size){
	return intern(str, data_size, string_hashcode(str, data_size), false);
}

const IDPtr& intern(const StringLiteral& str){
	return intern(str, str.size(), string_hashcode(str, str.size()), true);
}

const IDPtr& intern(const StringLiteral2& str){
	return intern(str, str.size(), str.hashcode(), true);
}

AnyPtr interned_strings(){
	return environment_->string_space_.interned_strings();
}

void yield_thread(){
	return environment_->thread_space_.yield_thread();
}

void sleep_thread(float_t sec){
	return environment_->thread_space_.sleep_thread(sec);
}

void xlock(){
	environment_->thread_space_.xlock();
}

void xunlock(){
	environment_->thread_space_.xunlock();
}

void register_thread(Environment* env){
	env->thread_space_.register_thread();
}

void unregister_thread(Environment* env){
	env->thread_space_.unregister_thread();
}

ThreadLib* thread_lib(){
	return environment_->setting_.thread_lib;
}

StdStreamLib* std_stream_lib(){
	return environment_->setting_.std_stream_lib;
}

FilesystemLib* filesystem_lib(){
	return environment_->setting_.filesystem_lib;
}

const StreamPtr& stdin_stream(){
	return environment_->stdin_;
}

const StreamPtr& stdout_stream(){
	return environment_->stdout_;
}

const StreamPtr& stderr_stream(){
	return environment_->stderr_;
}

const MapPtr& text_map(){
	return environment_->text_map_;
}

int_t ch_len(char_t lead){
	return environment_->setting_.ch_code_lib->ch_len(lead);
}

int_t ch_len2(const char_t* str){
	return environment_->setting_.ch_code_lib->ch_len2(str);
}

StringPtr ch_inc(const char_t* data, int_t data_size){
	return environment_->setting_.ch_code_lib->ch_inc(data, data_size);
}

int_t ch_cmp(const char_t* a, uint_t asize, const char_t* b, uint_t bsize){
	return environment_->setting_.ch_code_lib->ch_cmp(a, asize, b, bsize);
}

StreamPtr open(const StringPtr& file_name, const StringPtr& mode){
	SmartPtr<FileStream> ret = xnew<FileStream>(file_name, mode);
	if(ret->is_open()){
		return ret;
	}
	XTAL_SET_EXCEPT(cpp_class<RuntimeError>()->call(Xt("XRE1032")->call(Named(Xid(name), file_name))));
	return null;
}

#ifndef XTAL_NO_PARSER

CodePtr compile_stream(const StreamPtr& stream){
	CodePtr ret;

	{
		CodeBuilder cb;
		if(CodePtr fun =  cb.compile(stream, "<eval>")){
			ret = fun;
		}
		else{
			XTAL_SET_EXCEPT(cpp_class<CompileError>()->call(Xt("XRE1002"), cb.errors()->to_a()));
			return null;
		}
	}

	full_gc();
	return ret;
}

CodePtr compile_file(const StringPtr& file_name){
	CodePtr ret;

	if(StreamPtr fs = open(file_name, Xid(r))){
		CodeBuilder cb;
		if(CodePtr fun = cb.compile(fs, file_name)){
			fs->close();
			ret = fun;
		}
		else{
			fs->close();
			XTAL_SET_EXCEPT(cpp_class<CompileError>()->call(Xt("XRE1016")->call(Named(Xid(name), file_name)), cb.errors()->to_a()));
			return null;
		}
	}
	else{
		return null;
	}

	full_gc();
	return ret;
}

CodePtr compile(const StringPtr& source){
	return compile_stream(xnew<StringStream>(source));
}

AnyPtr load(const StringPtr& file_name){
	AnyPtr ret = undefined;
	if(CodePtr code = compile_file(file_name)){
		ret = code->call();
	}
	full_gc();
	return ret;
}

struct RequireData{
	AnyPtr require_source_hook;
};

void set_require_source_hook(const AnyPtr& hook){
	const SmartPtr<RequireData>& r = cpp_var<RequireData>();
	r->require_source_hook = hook;
}

CodePtr require_source(const StringPtr& name){
	const SmartPtr<RequireData>& r = cpp_var<RequireData>();
	if(r->require_source_hook){
		return ptr_cast<Code>(r->require_source_hook->call(name));
	}
	else{
		StringPtr temp = ptr_cast<String>(Xf("%s.xtalc")->call(name));
		if(StreamPtr fs = open(name, Xid(r))){
			if(CodePtr code = ptr_cast<Code>(fs->deserialize())){
				return code;
			}
		}
		else{
			XTAL_CATCH_EXCEPT(e){
				if(e->is(cpp_class<CompileError>())){
					XTAL_SET_EXCEPT(e);
					return null;
				}
			}

			temp = ptr_cast<String>(Xf("%s.xtal")->call(name));
			if(CodePtr ret = compile_file(temp)){
				return ret;
			}
		}
	}

	return null;
}

AnyPtr require(const StringPtr& name){
	if(CodePtr ret = require_source(name)){
		return ret->call();
	}
	return undefined;
}

CodePtr source(const char_t* src, int_t size){
	CodePtr ret;

	{
		CodeBuilder cb;
		StreamPtr ms(xnew<PointerStream>(src, size*sizeof(char_t)));
		if(CodePtr fun = cb.compile(ms)){
			ret = fun;
		}
		else{
			XTAL_SET_EXCEPT(cpp_class<CompileError>()->call(Xt("XRE1010")->call(), cb.errors()->to_a()));
			return null;
		}
	}

	full_gc();
	return ret;
}

void exec_source(const char_t* src, int_t size){
	if(CodePtr code = source(src, size)){
		code->call();
	}
	else{
		XTAL_CATCH_EXCEPT(e){
			stderr_stream()->println(e);
		}
	}
}

#endif

CodePtr compiled_source(const void* src, int_t size){
	StreamPtr ms(xnew<PointerStream>(src, size));
	if(CodePtr fun = ptr_cast<Code>(ms->deserialize())){
		gc();
		return fun;
	}
	return null;
}

void exec_compiled_source(const void* src, int_t size){
	if(CodePtr code = compiled_source(src, size)){
		code->call();
	}
	else{
		XTAL_CATCH_EXCEPT(e){
			stderr_stream()->println(e);
		}
	}
}

namespace xpeg{

const NFAPtr& fetch_nfa(const ElementPtr& node){
	const AnyPtr& temp = environment_->nfa_map_->at(node);
	if(temp){
		return unchecked_ptr_cast<NFA>(temp);
	}
	else{
		environment_->nfa_map_->set_at(node, xnew<NFA>(node));
		return unchecked_ptr_cast<NFA>(environment_->nfa_map_->at(node));
	}
}

}

}
