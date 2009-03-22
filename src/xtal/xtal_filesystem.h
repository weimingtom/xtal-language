
#pragma once

namespace xtal{

class FilesystemLib;

class Filesystem : public CppSingleton{
	FilesystemLib* filesystem_lib_;
public:

	Filesystem(){}

	void initialize(FilesystemLib* p=0){
		filesystem_lib_ = p;
	}

	StreamPtr open(const StringPtr& file_name, const StringPtr& flags);

	bool is_directory(const StringPtr& path);

	AnyPtr entries(const StringPtr& path);
};

class Entries : public Base{
public:
	Entries(const StringPtr& path){
		impl_ = filesystem_lib()->new_entries(path->c_str());
	}

	~Entries(){
		block_break();
		filesystem_lib()->delete_entries(impl_);
	}

	void block_next(const VMachinePtr& vm){
		if(const char_t* next = filesystem_lib()->next_entries(impl_)){
			vm->return_result(to_smartptr(this), xnew<String>(next));
		}
		else{
			vm->return_result(null, null);
		}
	}

	void block_break(){
		filesystem_lib()->break_entries(impl_);
	}

private:
	void* impl_;
};

}

