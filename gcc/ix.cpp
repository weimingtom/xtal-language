
#include "../src/xtal/xtal.h"

#include "../src/xtal/xtal_lib/xtal_pthread.h"
#include "../src/xtal/xtal_lib/xtal_cstdiostream.h"
#include "../src/xtal/xtal_lib/xtal_posixfilesystem.h"
#include "../src/xtal/xtal_lib/xtal_chcode.h"
#include "../src/xtal/xtal_lib/xtal_errormessage.h"

#include "../src/xtal/xtal_codebuilder.h"
#include "../src/xtal/xtal_macro.h"

void ix(){
	using namespace xtal;
	CodeBuilder cb;
	cb.interactive_compile(xnew<InteractiveStream>());
}

int main(int argc, char** argv){
	using namespace xtal;

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

	ix();

	XTAL_CATCH_EXCEPT(e){
		stderr_stream()->println(e);
	}

	uninitialize();

	return 0;
}

