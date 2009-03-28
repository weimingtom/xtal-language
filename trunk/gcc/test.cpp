#include "../src/xtal/xtal.h"
#include "../src/xtal/xtal_macro.h"

#include "../src/xtal/xtal_lib/xtal_pthread.h"
#include "../src/xtal/xtal_lib/xtal_cstdiostream.h"
#include "../src/xtal/xtal_lib/xtal_posixfilesystem.h"
#include "../src/xtal/xtal_lib/xtal_chcode.h"
#include "../src/xtal/xtal_lib/xtal_errormessage.h"

#include "time.h"

#include <iostream>

using namespace xtal;

int main2(int argc, char** argv){
	
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

	lib()->member("test")->send("run_dir", "../test");

	XTAL_CATCH_EXCEPT(e){
		stderr_stream()->println(e);
		return 1;
	}

	return 0;
}

int main(int argc, char** argv){
	CStdioStdStreamLib stream_lib;
	PThreadLib thread_lib;
	PosixFilesystemLib filesystem_lib;
	SJISChCodeLib ch_code_lib;

	Setting setting;
	setting.thread_lib = &thread_lib;
	setting.std_stream_lib = &stream_lib;
	setting.filesystem_lib = &filesystem_lib;
	setting.ch_code_lib = &ch_code_lib;

	initialize(setting);
	bind_error_message();

	int ret = main2(argc, argv);

	vmachine()->print_info();
	uninitialize();

	return ret;
}

