
#include "xtal.h"

#ifndef XTAL_NO_PARSER

int main(int argc, char** argv){
	using namespace xtal;
 
	XTAL_TRY{
		set_thread();
		initialize();

		ix();
	}
	XTAL_CATCH(e){
		stderr_stream()->put_s(e->to_s());
		stderr_stream()->put_s("\n");
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
