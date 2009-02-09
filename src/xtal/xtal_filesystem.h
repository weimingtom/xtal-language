
#pragma once

namespace xtal{

class Filesystem : public CppSingleton{
	FilesystemLib* filesystem_lib_;
public:

	void initialize(FilesystemLib* p=0){
		filesystem_lib_ = p;
	}

	StreamPtr open(const StringPtr& file_name, const StringPtr& flags);

	AnyPtr entries(const StringPtr& path);

	bool is_directory(const StringPtr& path);

};

const SmartPtr<Filesystem>& filesystem();

inline StreamPtr open(const StringPtr& file_name, const StringPtr& flags){
	return filesystem()->open(file_name, flags);
}

}

