
#include "xtal.h"

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
