
#include "xtal.h"
#include "xtal_lib/xtal_winthread.h"
#include "xtal_lib/xtal_cstdiostream.h"

#ifndef XTAL_NO_PARSER

int main(int argc, char** argv){
	using namespace xtal;

	CStdioStreamLib stream_lib;
	WinThreadLib win_thread_lib;
	CoreSetting setting;
	setting.thread_lib = &win_thread_lib;
	setting.stream_lib = &stream_lib;

	Core core;
	core.initialize(setting);
 
	XTAL_TRY{
		ix();
	}
	XTAL_CATCH(e){
		stderr_stream()->put_s(e->to_s());
		stderr_stream()->put_s("\n");
	}

	return 0;
}

#else

int main(int argc, char** argv){
	puts("xtal no parser");
	return 0;
}

#endif
