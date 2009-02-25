
#include "xtal.h"
#include "xtal_macro.h"

#include "xtal_lib/xtal_winthread.h"
#include "xtal_lib/xtal_cstdiostream.h"
#include "xtal_lib/xtal_winfilesystem.h"
#include "xtal_lib/xtal_chcode.h"

#include "time.h"

using namespace xtal;

class List;
typedef SmartPtr<List> ListPtr;

class List : public Base{
public:

	List(const AnyPtr& head, const ListPtr& tail = ListPtr())
		:head_(head), tail_(tail){
	}

	const AnyPtr& head(){
		return head_;
	}

	const ListPtr& tail(){
		return tail_;
	}

	void block_next(const VMachinePtr& vm){
		if(tail_){
			vm->return_result(tail_, head_);
		}
		else{
			vm->return_result(0, head_);
		}
	}

	int_t size(){
		const ListPtr* cur = &from_this(this);
		int_t size = 1;
		while(true){
			if(!(*cur)->tail_){
				return size;
			}
			cur = &(*cur)->tail_;
			++size;
		}
	}

	const AnyPtr& at(int_t i){
		const ListPtr* cur = &from_this(this);
		const AnyPtr* ret = &head_;
		for(int_t n=0; n<i; ++n){
			if(!(*cur)->tail_){
				ret = &undefined;
				break;
			}
			cur = &(*cur)->tail_;
			ret = &(*cur)->head_;
		}
		return *ret;
	}

	void visit_members(Visitor& m){
		Base::visit_members(m);
		m & head_ & tail_;
	}

private:
	AnyPtr head_;
	ListPtr tail_;
};




#ifndef XTAL_NO_PARSER

int main2(int argc, char** argv){
	//_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF | /*_CRTDBG_CHECK_ALWAYS_DF |*/ _CRTDBG_DELAY_FREE_MEM_DF);
	
	using namespace std;

	CStdioStreamLib cstd_stream_lib;
	WinThreadLib win_thread_lib;
	WinFilesystemLib win_filesystem_lib;
	SJISChCodeLib sjis_chcode_lib;

	CoreSetting setting;
	setting.thread_lib = &win_thread_lib;
	setting.stream_lib = &cstd_stream_lib;
	setting.filesystem_lib = &win_filesystem_lib;
	setting.chcode_lib = &sjis_chcode_lib;

	Core core;
	core.initialize(setting);

	{
		ClassPtr p = new_cpp_class<List>();
		p->inherit(Iterator());
		p->def("new", ctor<List, const AnyPtr&, const ListPtr&>());
		p->def_method(Xid(block_next), &List::block_next);
		p->def_method(Xid(at), &List::at);
		p->def_method(Xid(size), &List::size);
		lib()->def("List", p);
	}

		//debug()->enable();
		//debug()->set_throw_hook(fun(&debug_throw));

	
	if(CodePtr code = Xsrc((
		return fun(a){
			a.at(1).p;
		}
	))){
		code->call()->call(xnew<List>(10, xnew<List>(20, xnew<List>(30))));
	}
	
	
	XTAL_CATCH_EXCEPT(e){
		stderr_stream()->println(e);
		return 1;
	}


#if 1

	int c;

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
	CodePtr code = compile_file("../test/test.xtal_");
	XTAL_CATCH_EXCEPT(e){
		stderr_stream()->println(e);
		return 1;
	}

	code->call();

	XTAL_CATCH_EXCEPT(e){
		stderr_stream()->println(e);
		return 1;
	}

	lib()->member("test")->send("run_dir", "../test/");
	
	//*/
#endif

	XTAL_CATCH_EXCEPT(e){
		stderr_stream()->println(e);
		return 1;
	}

	vmachine()->print_info();

	return 0;
}

int main(int argc, char** argv){
	int ret = main2(argc, argv);
	return ret;
}

#else

int main(int argc, char** argv){
	CStdioStreamLib cstd_stream_lib;
	WinThreadLib win_thread_lib;
	WinFilesystemLib win_filesystem_lib;
	SJISChCodeLib sjis_chcode_lib;

	CoreSetting setting;
	setting.thread_lib = &win_thread_lib;
	setting.stream_lib = &cstd_stream_lib;
	setting.filesystem_lib = &win_filesystem_lib;
	setting.chcode_lib = &sjis_chcode_lib;

	Core core;
	core.initialize(setting);

	XTAL_CATCH_EXCEPT(e){
		stderr_stream()->put_s(e->to_s());
		stderr_stream()->put_s("\n");
	}

	vmachine()->print_info();
	printf("-------------------\n");
	return 0;
}

#endif