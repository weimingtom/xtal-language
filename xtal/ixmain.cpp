
#include "xtal.h"

int main(int argc, char** argv){
	using namespace xtal;
 
	try{
		set_thread();
		initialize();
 
		{
			StringPtr path(argv[0]);

#ifdef WIN32
			StringPtr sep("\\");
#else
			StringPtr sep("/");
#endif

			ArrayPtr temp = cast<ArrayPtr>(path->split(sep)->send("to_a"));
			temp->pop_back();
#ifdef WIN32
			temp->push_back("message.xtal");
#else
			temp->push_back("message_en.xtal");
#endif
			path = temp->join(sep)->to_s();
			add_get_text_map(cast<MapPtr>(load(path)));
		}

		ix();

	}catch(AnyPtr e){
		fprintf(stderr, "%s\n", e->to_s()->c_str());
	}

	uninitialize();
	return 0;
}
