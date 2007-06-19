
#include "xtal.h"

int main(int argc, char** argv){
	using namespace xtal;
 
	try{
		set_thread();
		initialize();
 
		{
			String path(argv[0]);

#ifdef WIN32
			String sep("\\");
#else
			String sep("/");
#endif

			Array temp = cast<Array>(path.split(sep).send("to_a"));
			temp.pop_back();
#ifdef WIN32
			temp.push_back("message.xtal");
#else
			temp.push_back("message_en.xtal");
#endif
			path = temp.join(sep).to_s();
			add_get_text_map(load(path));
		}

		ix();

	}catch(Any e){
		fprintf(stderr, "%s\n", e.to_s().c_str());
	}

	uninitialize();
	return 0;
}
