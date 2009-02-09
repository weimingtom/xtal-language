
#pragma once

#include <dirent.h>
#include "xtal_lib/xtal_cstdiostream.h"

namespace xtal{

class DirentIter : public Base{
	DIR* dir_;
public:

	DirentIter(const StringPtr& path){
		dir_ = opendir(path->c_str());
	}
	
	virtual ~DirentIter(){
		block_break();
	}
	
	void block_next(const VMachinePtr& vm){
		if(!dir_){
			vm->return_result(null, null);
			return;
		}
		
		dirent* dp = readdir(dir);
		if(!dp){
			vm->return_result(null, null);
			return;
		}
		
		vm->return_result(from_this(this), dp->d_name);
	}
	
	void block_break(){
		if(dir_){
			closedir(dir_);
			dir_ = 0;
		}
	}
};

class PosixFilesystemLib : public FilesystemLib{
public:
	virtual void initialize(){
		CStdioFileStream::initialize_class();

		{
			ClassPtr p = new_cpp_class<DirentIter>();
			p->inherit(Iterator());
			p->def_method(XTAL_ID(block_next), &DirentIter::block_next);
			p->def_method(XTAL_ID(block_break), &DirentIter::block_break);
		}
	}
	
	virtual AnyPtr entries(const StringPtr& path){
		return xnew<DirentIter>(path);
	}
	
	virtual StreamPtr open(const StringPtr& path, const StringPtr& flags){
		FILE* fp = std::fopen(path->c_str(), flags->c_str());
		if(!fp){ XTAL_THROW(builtin()->member(XTAL_ID(IOError))->call(text("Xtal Runtime Error 1014")->call(Named(XTAL_ID(name), path))), return null); }
		return xnew<CStdioFileStream>(fp);
	}

	virtual bool is_directory(const StringPtr& path){
		return false;		
	}
};

}
