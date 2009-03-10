#include "xtal.h"

namespace xtal{

StreamPtr Filesystem::open(const StringPtr& file_name, const StringPtr& aflags){
	const char_t* flags = aflags->c_str();
	char_t flags_temp[16];
	bool text = false;
	uint_t i = 0;
	for(; flags[i]!=0 && i<10; ++i){
		if(flags[i]=='t'){
			text = true;
		}
		else{
			flags_temp[i] = flags[i];
		}
	}

	if(!text){
		flags_temp[i++] = 'b';
	}
	flags_temp[i++] = 0;
	
	return filesystem_lib_->open(file_name, flags_temp);
}

AnyPtr Filesystem::entries(const StringPtr& path){
	return filesystem_lib_->entries(path);
}

bool Filesystem::is_directory(const StringPtr& path){
	return filesystem_lib_->is_directory(path);
}

StreamPtr open(const StringPtr& path, const StringPtr& flags){
	StreamPtr ret = filesystem()->open(path, flags);
	if(!ret){
		XTAL_SET_EXCEPT(builtin()->member(XTAL_ID(IOError))->call(text("Xtal Runtime Error 1014")->call(Named(XTAL_ID(name), path))));
	}
	return ret;
}

}
