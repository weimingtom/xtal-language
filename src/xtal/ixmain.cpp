
#include "xtal.h"
#include "xtal_lib/xtal_winthread.h"
#include "xtal_lib/xtal_cstdiostream.h"
#include "xtal_lib/xtal_winfilesystem.h"
#include "xtal_lib/xtal_chcode.h"
#include "xtal_lib/xtal_errormessage.h"
#include "xtal_codebuilder.h"

#include "xtal_macro.h"

#ifndef XTAL_NO_PARSER

void ix(){
	using namespace xtal;
	CodeBuilder cb;
	cb.interactive_compile(xnew<InteractiveStream>());
}

int main(int argc, char** argv){
	using namespace xtal;

	CStdioStdStreamLib cstd_std_stream_lib;
	WinThreadLib win_thread_lib;
	WinFilesystemLib win_filesystem_lib;
	SJISChCodeLib sjis_ch_code_lib;

	Setting setting;
	setting.thread_lib = &win_thread_lib;
	setting.std_stream_lib = &cstd_std_stream_lib;
	setting.filesystem_lib = &win_filesystem_lib;
	setting.ch_code_lib = &sjis_ch_code_lib;

	initialize(setting);
	bind_error_message();

	ix();

	XTAL_CATCH_EXCEPT(e){
		stderr_stream()->println(e);
	}

	uninitialize();

	return 0;
}

#else

int main(int argc, char** argv){
	puts("xtal no parser");
	return 0;
}

#endif
