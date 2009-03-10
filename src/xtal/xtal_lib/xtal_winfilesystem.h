
#pragma once

#include <windows.h>
#include "xtal_cstdiostream.h"

namespace xtal{

class WinFindNextFileIter : public Base{
	HANDLE h_;
	WIN32_FIND_DATAA fd_;
public:

	WinFindNextFileIter(const StringPtr& path){
		StringPtr npath = path->cat("*");
		h_ = FindFirstFileA(npath->c_str(), &fd_);
	}
	
	virtual ~WinFindNextFileIter(){
		block_break();
	}
	
	void block_next(const VMachinePtr& vm){
		if(h_==INVALID_HANDLE_VALUE){
			vm->return_result(null, null);
			return;
		}
		
		vm->return_result(from_this(this), fd_.cFileName);
	
		if(!FindNextFileA(h_, &fd_)){
			block_break();
		}
	}
	
	void block_break(){
		if(h_!=INVALID_HANDLE_VALUE){
			FindClose(h_);
			h_ = INVALID_HANDLE_VALUE;
		}
	}
};

class WinFilesystemLib : public FilesystemLib{
public:
	virtual void initialize(){
		CStdioFileStream::initialize_class();

		{
			ClassPtr p = new_cpp_class<WinFindNextFileIter>(XTAL_ID(WinFindNextFileIter));
			p->inherit(Iterator());
			p->def_method(XTAL_ID(block_next), &WinFindNextFileIter::block_next);
			p->def_method(XTAL_ID(block_break), &WinFindNextFileIter::block_break);
		}
	}
	
	virtual AnyPtr entries(const StringPtr& path){
		return xnew<WinFindNextFileIter>(path);
	}
	
	virtual StreamPtr open(const StringPtr& path, const StringPtr& flags){
#ifdef XTAL_USE_WCHAR
		FILE* fp = _wfopen(path->c_str(), flags->c_str());
#else
		FILE* fp = std::fopen(path->c_str(), flags->c_str());
#endif
		if(!fp){ 
			return null; 
		}
		return xnew<CStdioFileStream>(fp);
	}

	virtual bool is_directory(const StringPtr& path){
		return false;
	}
};

}
