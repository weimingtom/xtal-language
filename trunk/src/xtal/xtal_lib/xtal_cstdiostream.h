
#pragma once

#include <cstdio>

namespace xtal{

	/*
class InteractiveStream : public Stream{
public:

	InteractiveStream(){
		line_ = 1;
		continue_stmt_ = false;
		fp_ = stdin;
	}

	virtual uint_t read(void* p, uint_t size){
		if(!fp_)
			return 0;

		if(continue_stmt_){
			stdout_stream()->put_s(format("ix:%03d>    ")->call(line_)->to_s());
		}
		else{
			stdout_stream()->put_s(format("ix:%03d>")->call(line_)->to_s());
		}

		continue_stmt_ = true;
		if(fgets((char*)p, size, stdin)){
			uint_t sz = std::strlen((char*)p);
			if(sz!=size-1){
				line_++;
			}
			return sz;
		}
		fp_ = 0;
		return 0;
	}

	virtual void close(){
		if(fp_){
			fclose(fp_);
			fp_ = 0;
		}
	}

	virtual void flush(){
		continue_stmt_ = false;
	}

private:
	int_t line_;
	bool continue_stmt_;
	std::FILE* fp_;
};
*/
class CStdioStdStreamLib : public StdStreamLib{
public:
	virtual uint_t read_stdin_stream(void* stdin_stream_object, void* dest, uint_t size){ 
		return fread(dest, 1, size, stdin); 
	}

	virtual uint_t write_stdout_stream(void* stdout_stream_object, const void* src, uint_t size){ 
		return fwrite(src, 1, size, stdout); 
	}

	virtual uint_t write_stderr_stream(void* stderr_stream_object, const void* src, uint_t size){ 
		return fwrite(src, 1, size, stderr); 
	}
};

}
