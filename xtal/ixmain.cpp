
#include "xtal/xtal.h"

//#include "xtal/all_src.h"
#pragma comment(lib, "../xtallib.lib") 

#include "xtal/message_jp_sjis.txt"

int main(){
	using namespace xtal;

	try{
		set_thread();
		initialize();

		MemoryStream ms(message_data, sizeof(message_data));
		set_get_text_map(object_load(ms));

		ix();

	}catch(Any e){
		fprintf(stderr, "%s\n", e.to_s().c_str());
	}

	uninitialize();
	return 0;
}