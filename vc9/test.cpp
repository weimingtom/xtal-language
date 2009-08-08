
#include "../src/xtal/xtal.h"
#include "../src/xtal/xtal_macro.h"

#include "../src/xtal/xtal_lib/xtal_winthread.h"
#include "../src/xtal/xtal_lib/xtal_cstdiostream.h"
#include "../src/xtal/xtal_lib/xtal_winfilesystem.h"
#include "../src/xtal/xtal_lib/xtal_chcode.h"
#include "../src/xtal/xtal_lib/xtal_errormessage.h"

#include "time.h"

#include <iostream>

class TestGetterSetterBind{
public:
    float x, y;
    TestGetterSetterBind(): x(0), y(0) {}

	void foomethod(){}
};

XTAL_PREBIND(TestGetterSetterBind){
    it->def_ctor(ctor<TestGetterSetterBind>());
}

XTAL_BIND(TestGetterSetterBind){
   it->def_getter(Xid(x), &TestGetterSetterBind::x);
   it->def_setter(Xid(set_x), &TestGetterSetterBind::x);

   it->def_var(Xid(y), &TestGetterSetterBind::y);
}

struct MyData{
	int a;
};

struct MyDeleter{
	void operator()(MyData* p){
		delete p;
	}
};

XTAL_BIND(MyData){
	it->def_var(Xid(a), &MyData::a);
}

using namespace xtal;

void foofun(){}

void test(){
    lib()->def(Xid(TestGetterSetterBind), cpp_class<TestGetterSetterBind>());
	lib()->def(Xid(MyData), SmartPtr<MyData>(new MyData, MyDeleter()));

	AnyPtr m = method(&TestGetterSetterBind::foomethod);
	AnyPtr f = fun(&foofun);

	if(CodePtr code = Xsrc((
		foo: lib::TestGetterSetterBind();
		foo.x = 0.5;
		foo.x.p;

		foo.y = 100.5;
		foo.y.p;

		mydata: lib::MyData;
		mydata.a = 10;
		mydata.a.p;
	))){
		code->call();
	}
}

void benchmark(const char* file, const AnyPtr& arg){
	int c = clock();
	if(CodePtr code = compile_file(file)){
		code->call(arg);
		printf("%s %g\n\n", file, (clock()-c)/1000.0f);
	}

	XTAL_CATCH_EXCEPT(e){
		stderr_stream()->println(e);
	}
}

void debughook(const debug::HookInfoPtr& info){

	// ブレーク位置のローカル変数フレームを取得する
	FramePtr frame = info->variables_frame();

	// ローカル変数をイテレートする
	Xfor3(key1st, key2nd, value, frame->members()){
		// key1stにプライマリキー
		// key2ndにセカンダリキー(基本的にundefinedなので無視してかまわない)
		// valueに値が入る

		key1st->p(); // key1stをプリントする
	}

	for(uint_t i=0;; ++i){
		// 呼び出し元情報を取得する
		if(debug::CallerInfoPtr caller = info->caller(i)){

			// 呼び出し元のローカル変数フレームを取得する
			FramePtr frame = caller->variables_frame();
			while(frame){

				// ローカル変数をイテレートする
				Xfor3(key1st, key2nd, value, frame->members()){
					// key1stにプライマリキー
					// key2ndにセカンダリキー(基本的にundefinedなので無視してかまわない)
					// valueに値が入る

					key1st->p(); // key1stをプリントする
				}

				// 外側のスコープのローカル変数フレームを取得する
				frame = frame->outer();
			}

			break;
		}
		else{
			break;
		}
	}

}

void linehook(const debug::HookInfoPtr& info){
	int line = info->line();
	//printf("line %d\n", line);
}


int print(const AnyPtr& a){
	return 111;	
}

void breakpoint(){

}

int main2(int argc, char** argv){
	//_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF | /*_CRTDBG_CHECK_ALWAYS_DF |*/ _CRTDBG_DELAY_FREE_MEM_DF);
	
	using namespace std;

	//debug::enable();
	//debug::set_line_hook(fun(&linehook));

	//test();

//*
	if(CodePtr code = Xsrc((
		a : []; 
		b : null; 
		if( a==b ){ a.p; } else { b.p; } 
	))){
		code->filelocal()->def("ppp", fun(&print));
		//code->inspect()->p();
		AnyPtr ret = code->call(500, 108);
		//ret = ret;
	}

	full_gc();

	//load("../struct.xtal");

	//xnew<Array>()->call("test");

	XTAL_CATCH_EXCEPT(e){
		StringPtr str = e->to_s();
		const char_t* cstr = str->c_str();
		stderr_stream()->println(e);
		return 1;
	}

	//compile_file("../bench/ao.xtal")->inspect()->p();
	
	if(0){
		int c = clock();
		load("../bench/ao.xtal");
		printf("ao %g\n\n", (clock()-c)/1000.0f);		
	}

	XTAL_CATCH_EXCEPT(e){
		stderr_stream()->println(e);
		return 1;
	}
//*/

#if 1

	int c;

	//benchmark("../fannkuch.xtal", 12);
	//benchmark("../n-body.xtal", 12);
	//benchmark("../special-norm.xtal", 12);
	//benchmark("../binary-trees.xtal", 3);
	//benchmark("../mandelbrot.xtal", 10);

	/*		
	c = clock();
	load("../bench/vec.xtal");
	printf("vec %g\n\n", (clock()-c)/1000.0f);		
	
	c = clock();
	load("../bench/inst.xtal");
	printf("inst %g\n\n", (clock()-c)/1000.0f);

	c = clock();
	load("../bench/gc.xtal");
	printf("gc %g\n\n", (clock()-c)/1000.0f);

	c = clock();
	load("../bench/loop.xtal");
	printf("loop %g\n\n", (clock()-c)/1000.0f);

	c = clock();
	load("../bench/nested_loops.xtal");
	printf("nested_loops %g\n\n", (clock()-c)/1000.0f);

	c = clock();
	load("../bench/fib.xtal");
	printf("fib %g\n\n", (clock()-c)/1000.0f);

	c = clock();
	load("../bench/loop_iter.xtal");
	printf("loop_iter %g\n\n", (clock()-c)/1000.0f);

	c = clock();
	load("../bench/array_for.xtal");
	printf("array_for %g\n\n", (clock()-c)/1000.0f);

	c = clock();
	load("../bench/array_each.xtal");
	printf("array_each %g\n\n", (clock()-c)/1000.0f);

	//*/

	//*
	builtin()->def("breakpoint", fun(&breakpoint));

#ifdef XTAL_USE_WCHAR
	CodePtr code = compile_file("../utf16le-test/test.xtal_");
#else
	CodePtr code = compile_file("../test/test.xtal_");
#endif

	XTAL_CATCH_EXCEPT(e){
		stderr_stream()->println(e);
		return 1;
	}

	code->call();

	XTAL_CATCH_EXCEPT(e){
		stderr_stream()->println(e);
		return 1;
	}

#ifdef XTAL_USE_WCHAR
	lib()->member("test")->send("run_dir", "../utf16le-test");
#else
	lib()->member("test")->send("run_dir", "../test");
#endif

	//lib()->member("test")->send("run_file", "../test/test_xpeg.xtal");
	//lib()->member("test")->send("print_result");
	//*/
#endif

	full_gc();

	XTAL_CATCH_EXCEPT(e){
		stderr_stream()->println(e);
		return 1;
	}

	return 0;
}


namespace xxx{
class SimpleMemoryManager{
public:
	enum{
		USED = 1<<0,
		RED = 1<<1,

		OFFSET_SHIFT = 2,
		OFFSET_MASK = 0xffff<<OFFSET_SHIFT
	};

	static int offset(void* p){ return ((*(int*)p)&OFFSET_MASK)>>OFFSET_SHIFT; }
	static void set_offset(void* p, int a){ (*(int*)p) &= ~OFFSET_MASK; (*(int*)p) |= a<<OFFSET_SHIFT; }

	struct Chunk{
		Chunk* next;
		Chunk* prev;
		int_t flags;

		Chunk* left;
		Chunk* right;

		void init(){ next = prev = left = right = 0; flags = 0; }

		int size(){ return (unsigned char*)next - (unsigned char*)(&flags+1); }
		int buf_size(){ return (unsigned char*)next - (unsigned char*)buf(); }
		void* buf(){ return (unsigned char*)this+offset(); }
		
		bool used(){ return (flags&USED)!=0; }
		void set_used(){ flags |= USED; }
		void unset_used(){ flags &= ~USED; }

		int offset(){ return SimpleMemoryManager::offset(&flags); }
		void set_offset(int a){ SimpleMemoryManager::set_offset(&flags, a); }

		bool red(){ return (flags&RED)!=0; }
		void set_red(){ flags |= RED; }
		void set_black(){ flags &= ~RED; }
		void flip_color(){ flags ^= RED; }
		void set_same_color(Chunk* a){ set_black(); flags |= a->flags&RED; }
	};

	SimpleMemoryManager(){ 
		head_ = begin_ = end_ = 0; 
	}
	
	SimpleMemoryManager(void* buffer, size_t size){
		init(buffer, size);
	}

	void init(void* buffer, size_t size);

	void* malloc(size_t size, int alignment = sizeof(int_t));
	void free(void* p);

	Chunk* begin(){ return begin_; }
	Chunk* end(){ return end_; }

	Chunk* to_chunk(void* p){ return (Chunk*)((unsigned char*)p-offset((int_t*)p-1)); }
	
private:

	int compare(Chunk* a, Chunk* b){
		int ret = a->size() - b->size();
		if(ret==0){
			return a<b ? -1 : a>b ? 1 : 0;
		}
		return ret;
	}

	Chunk* find_in(Chunk* l, int key){
		while(l!=end()){
			int cmp = key - l->size();

			if(cmp==0){
				return l;
			}

			if(cmp<0){
				Chunk* ret = find_in(l->left, key);
				if(ret!=end()){
					return ret;
				}
				return l;
			}

			l = l->right;
		}

		return end();
	}

	Chunk* find(int key){
		return find_in(root_, key);
	}

	bool is_red(Chunk* n){
		return n->red();
	}

	void flip_colors(Chunk* n){
		n->flip_color();
		n->left->flip_color();
		n->right->flip_color();
	}

	Chunk* rotate_left(Chunk* n){
		Chunk* x = n->right;
		n->right = x->left;
		x->left = n;
		x->set_same_color(n);
		n->set_red();
		return x;
	}

	Chunk* rotate_right(Chunk* n){
		Chunk* x = n->left;
		n->left = x->right;
		x->right = n;
		x->set_same_color(n);
		n->set_red();
		return x;
	}

	Chunk* fixup(Chunk* n){
		if(is_red(n->right)){
			n = rotate_left(n);
		}
		
		if(is_red(n->left) && is_red(n->left->left)){
			n = rotate_right(n);
		}
		
		if(is_red(n->left) && is_red(n->right)){
			flip_colors(n);
		}

		return n;
	}

	Chunk* insert_in(Chunk* n, Chunk* key){
		if(n==end()){
			return key;
		}

		int cmp = compare(key, n);
		
		if(cmp<0){
			n->left = insert_in(n->left, key);
		}
		else{
			n->right = insert_in(n->right, key);
		}
		
		if(is_red(n->right) && !is_red(n->left)){
			n = rotate_left(n);
		}
		
		if(is_red(n->left) && is_red(n->left->left)){
			n = rotate_right(n);
		}
		
		if(is_red(n->left) && is_red(n->right)){
			flip_colors(n);
		}
		
		return n;
	}

	void insert(Chunk* key){
		key->right = key->left = end();
		key->set_red();

		root_ = insert_in(root_, key);
		root_->set_black();
	}

	Chunk* minv(Chunk* n){
		while(n->left!=end()){
			n = n->left;
		}
		return n;
	}

	Chunk* move_red_left(Chunk* n){
		flip_colors(n);
		if(is_red(n->right->left)){
			n->right = rotate_right(n->right);
			n = rotate_left(n);
			flip_colors(n);
		}
		return n;
	}

	Chunk* move_red_right(Chunk* n){
		flip_colors(n);
		
		if(is_red(n->left->left)){
			n = rotate_right(n);
			flip_colors(n);
		}
		return n;
	}

	Chunk* remove_min(Chunk* n){
		if(n->left==end()){
			return end();
		}
		
		if(!is_red(n->left) && !is_red(n->left->left)){
			n = move_red_left(n);
		}

		n->left = remove_min(n->left);
		return fixup(n);
	}

	Chunk* remove_in(Chunk* n, Chunk* key){
		if(compare(key, n) < 0){
			if(!is_red(n->left) && !is_red(n->left->left)){
				n = move_red_left(n);
			}
			
			n->left = remove_in(n->left, key);
		}
		else{
			if(is_red(n->left)){
				n = rotate_right(n);
			}
			
			if(n==key && n->right==end()){
				return end();
			}
			
			if(!is_red(n->right) && !is_red(n->right->left)){
				n = move_red_right(n);
			}
			
			if(n==key){
				Chunk* x = minv(n->right);
				x->right = remove_min(n->right);
				x->left = n->left;
				x->set_same_color(n);
				n = x;
			}
			else{
				n->right = remove_in(n->right, key);
			}
		}

		return fixup(n);
	}

	void remove(Chunk* key){
		root_ = remove_in(root_, key);
		root_->set_black();
	}

public:

	enum{
		COLOR_FREE,
		COLOR_USED
	};

	void dump_bitmap(unsigned char* dest, size_t size, unsigned char* marks){
		Chunk* p = head_;
		int_t* it = (int_t*)head_;

		size -= 1;

		size_t n = 0;
		while(p!=end()){
			double sz = p->size()*size/(double)buffer_size_;
			size_t szm = (size_t)sz+1;
			if(szm!=0){
				memset(dest+(size_t)n, marks[!p->used()], szm);
			}

			n += sz;
			p = p->next;
		}

		size_t m = (size_t)n;
		memset(dest+m, 0, (size+1)-m);
	}

	void dump_bitmap(unsigned char* dest, size_t size){
		unsigned char marks[] = {'O', 'X'};
		dump_bitmap(dest, size, marks);
	}

private:
	Chunk* head_;
	Chunk* begin_;
	Chunk* end_;
	Chunk* root_;

	size_t buffer_size_;
};

void SimpleMemoryManager::init(void* buffer, size_t size){
	buffer_size_ = size;

	head_ = (Chunk*)align_p(buffer, sizeof(int_t));
	begin_ = head_+1;
	end_ = (Chunk*)align_p((Chunk*)((char*)buffer+size)-2, sizeof(int_t));
	
	head_->init();
	head_->next = begin_;
	head_->prev = 0;
	head_->set_black();
	head_->set_used();
	
	begin_->init();
	begin_->next = end_;
	begin_->prev = head_;
	begin_->set_red();
	begin_->unset_used();
	
	end_->init();
	end_->next = 0;
	end_->prev = begin_;
	end_->set_black();
	end_->set_used();
	end_->left = end();
	end_->right = end();
	
	root_ = begin_;
	root_->left = end();
	root_->right = end();
	root_->set_red();
}

void* SimpleMemoryManager::malloc(size_t size, int alignment){
	if(alignment<=sizeof(int_t)){
		size = align(size, sizeof(int_t));
	}
	else{
		size = align(size+alignment-sizeof(int_t), sizeof(int_t));
	}

	Chunk* it = find(size);
	if(it!=end()){
		remove(it);
		it->set_used();

		void* p = (unsigned char*)(&it->flags+1);
		p = align_p(p, alignment);
		int ofst = (unsigned char*)p - (unsigned char*)it;
		it->set_offset(ofst);
		set_offset((int_t*)p-1, ofst);

		if(it->buf_size()>=size+sizeof(Chunk)){
			Chunk* newchunk = (Chunk*)((unsigned char*)it->buf()+size);
			newchunk->init();
			newchunk->unset_used();
			it->next->prev = newchunk;
			newchunk->next = it->next;
			it->next = newchunk;
			newchunk->prev = it;

			insert(newchunk);
		}

		return p;
	}
		
	return 0;
}

void SimpleMemoryManager::free(void* p){
	if(p){
		Chunk* it = to_chunk(p);

		if(!it->prev->used()){
			remove(it->prev);
			it->prev->next = it->next;
			it->next->prev = it->prev;
			it = it->prev;
		}
	
		if(!it->next->used()){
			remove(it->next);
			it->next->next->prev = it;
			it->next = it->next->next;
		}

		insert(it);
	}
}

}

char memory[1024*1024*1];
xxx::SimpleMemoryManager smm(memory, 1024*1024*1);

class AAllocatorLib : public AllocatorLib{
public:
	virtual ~AAllocatorLib(){}
	virtual void* malloc(std::size_t size){ return smm.malloc(size); }
	virtual void free(void* p, std::size_t size){ smm.free(p); }
	virtual void* out_of_memory(std::size_t size){ return 0; }
};

int main(int argc, char** argv){
	CStdioStdStreamLib cstd_std_stream_lib;
	WinThreadLib win_thread_lib;
	WinFilesystemLib win_filesystem_lib;
	SJISChCodeLib sjis_ch_code_lib;
	AAllocatorLib alloc_lib;

	Setting setting;
	setting.thread_lib = &win_thread_lib;
	setting.std_stream_lib = &cstd_std_stream_lib;
	setting.filesystem_lib = &win_filesystem_lib;
	setting.ch_code_lib = &sjis_ch_code_lib;
	setting.allocator_lib = &alloc_lib;

	initialize(setting);

	int ret = 1;
	XTAL_PROTECT{
		bind_error_message();

		ret = main2(argc, argv);

		static unsigned char dest[10000];
		smm.dump_bitmap(dest, 9999);
		dest[9999] = 0;
		puts((char*)dest);

		vmachine()->print_info();
	}
	XTAL_OUT_OF_MEMORY{
		puts("out of memory");
	}
		
	uninitialize();

	return ret;
}
