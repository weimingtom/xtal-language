
#include "xtal.h"

int main(int argc, char** argv){
	using namespace xtal;
 
	try{
		set_thread();
		initialize();

		ix();

	}catch(AnyPtr e){
		fprintf(stderr, "%s\n", e->to_s()->c_str());
	}

	uninitialize();
	return 0;
}
