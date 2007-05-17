
#include "xtal.h"

#ifdef _MSC_VER
#pragma comment(lib, "../xtallib.lib") 
#else
#include "all_src.h"
#endif

int main(){
	using namespace xtal;
 
	try{
		set_thread();
		initialize();
 
		ix();

	}catch(Any e){
		fprintf(stderr, "%s\n", e.to_s().c_str());
	}

	uninitialize();
	return 0;
}
