
#pragma once

#include <cstdio>

namespace xtal{

class CStdioFileStream : public Stream{
public:

	CStdioFileStream(std::FILE* fp){
		fp_ = fp;
	}

	~CStdioFileStream(){
		close();
	}

	virtual uint_t tell(){
		if(!fp_){ XTAL_THROW(builtin()->member(XTAL_ID(IOError))->call(text("Xtal Runtime Error 1018")), return 0); }
		return std::ftell(fp_);
	}

	virtual uint_t write(const void* p, uint_t size){
		if(!fp_){ XTAL_THROW(builtin()->member(XTAL_ID(IOError))->call(text("Xtal Runtime Error 1018")), return 0); }
		XTAL_UNLOCK{
	#ifdef XTAL_USE_WCHAR
			char_t buf[256];
			XTAL_SPRINTF(buf, 256, L"%%.%ds", size/sizeof(char_t));
			uint_t ret = std::fwprintf(fp_, buf, p);
	#else
			uint_t ret = std::fwrite(p, size, 1, fp_);
	#endif
			std::fflush(fp_);
			return ret;
		}
		return 0;
	}

	virtual uint_t read(void* p, uint_t size){
		if(!fp_){ XTAL_THROW(builtin()->member(XTAL_ID(IOError))->call(text("Xtal Runtime Error 1018")), return 0); }
		XTAL_UNLOCK{
			return std::fread(p, 1, size, fp_);
		}
		return 0;
	}

	virtual void seek(int_t offset, int_t whence = XSEEK_SET){
		if(!fp_){ XTAL_THROW(builtin()->member(XTAL_ID(IOError))->call(text("Xtal Runtime Error 1018")), return); }
		int wh = whence==XSEEK_END ? SEEK_END : whence==XSEEK_CUR ? SEEK_CUR : SEEK_SET;
		std::fseek(fp_, offset, wh);
	}

	virtual void close(){
		if(fp_){
			std::fclose(fp_);
			fp_ = 0;
		}
	}

	virtual bool eos(){
		if(!fp_){ return true; }
		int ch = std::getc(fp_);
		if(std::feof(fp_)){
			return true;
		}
		std::ungetc(ch, fp_);
		return false;
	}

	static void initialize_class(){
		if(!exists_cpp_class<CStdioFileStream>()){
			new_cpp_class<CStdioFileStream>()->inherit(get_cpp_class<Stream>());
		}
	}

protected:
	std::FILE* fp_;
};

class CStdioStreamLib : public StreamLib{
public:
	virtual void initialize(){
		CStdioFileStream::initialize_class();
	}

	virtual StreamPtr new_stdin_stream(){
		return xnew<CStdioFileStream>(stdin);
	}

	virtual StreamPtr new_stdout_stream(){
		return xnew<CStdioFileStream>(stdout);
	}

	virtual StreamPtr new_stderr_stream(){
		return xnew<CStdioFileStream>(stderr);
	}
};

}
