
#include "xtal.h"

int main(int argc, char** argv){
	using namespace xtal;
 
	XTAL_TRY{
		set_thread();
		initialize();

		ix();

	}
	XTAL_CATCH(e){
		fprintf(stderr, "%s\n", e->to_s()->c_str());
	}

	uninitialize();
	return 0;
}
