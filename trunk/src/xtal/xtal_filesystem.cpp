#include "xtal.h"

namespace xtal{

StreamPtr Filesystem::open(const StringPtr& path, const StringPtr& flags){
	return xnew<FileStream>(path, flags);
}

AnyPtr Filesystem::entries(const StringPtr& path){
	return xnew<Entries>(path);
}

bool Filesystem::is_directory(const StringPtr& path){
	return filesystem_lib_->is_directory(path);
}

}
