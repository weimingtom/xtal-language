
#pragma once

#include "any.h"
#include "stream.h"
#include "frame.h"
#include "lib.h"
#include "xmacro.h"
#include "thread.h"

#include <stdio.h>

namespace xtal{

class StreamImpl : public AnyImpl{
public:

	void p8(int_t v){
		struct{ xtal::u8 data[1]; } data;
		data.data[0] = (v>>0)&0xff;
		do_write(data.data, 1);
	}

	void p16(int_t v){
		struct{ xtal::u8 data[2]; } data;
		data.data[0] = (v>>8)&0xff;
		data.data[1] = (v>>0)&0xff;
		do_write(data.data, 2);
	}

	void p32(int_t v){
		struct{ xtal::u8 data[4]; } data;
		data.data[0] = (v>>24)&0xff;
		data.data[1] = (v>>16)&0xff;
		data.data[2] = (v>>8)&0xff;
		data.data[3] = (v>>0)&0xff;
		do_write(data.data, 4);
	}

	int_t s8(){
		struct{ xtal::u8 data[1]; } data;
		do_read(data.data, 1);
		return (xtal::s8)data.data[0];
	}

	int_t s16(){
		struct{ xtal::u8 data[2]; } data;
		do_read(data.data, 2);
		return (xtal::s16)((data.data[0]<<8) | data.data[1]);
	}

	int_t s32(){
		struct{ xtal::u8 data[4]; } data;
		do_read(data.data, 4);
		return ((data.data[0]<<24) | (data.data[1]<<16) | (data.data[2]<<8) | data.data[3]);
	}

	uint_t u8(){
		struct{ xtal::u8 data[1]; } data;
		do_read(data.data, 1);
		return (xtal::u8)data.data[0];
	}

	uint_t u16(){
		struct{ xtal::u8 data[2]; } data;
		do_read(data.data, 2);
		return (xtal::u16)((data.data[0]<<8) | data.data[1]);
	}

	uint_t u32(){
		struct{ xtal::u8 data[4]; } data;
		do_read(data.data, 4);
		return ((data.data[0]<<24) | (data.data[1]<<16) | (data.data[2]<<8) | data.data[3]);
	}

	virtual uint_t tell() = 0;

	virtual uint_t do_write(const void* p, uint_t size) = 0;

	virtual uint_t do_read(void* p, uint_t size) = 0;

	virtual void close() = 0;

	void iter_first(const VMachine& vm){
		vm.return_result(this, this);
	}

	void iter_next(const VMachine& vm){
		vm.return_result(null);
		close();
	}

	void iter_break(const VMachine& vm){
		close();
	}
};


class FileStreamImpl : public StreamImpl{
public:

	FileStreamImpl(const String& filename, const String& mode){
		set_class(TClass<FileStream>::get());
		fp_ = fopen(filename.c_str(), mode.cat("b").c_str());
		if(!fp_){
			full_gc();
			fp_ = fopen(filename.c_str(), mode.c_str());
			if(!fp_){
				throw builtin().member(Xid(IOError))(Xt("Xtal Runtime Error 1014")(Xid(name)=filename));
			}
		}
	}

	~FileStreamImpl(){
		close();
	}

	virtual uint_t tell(){
		return ftell(fp_);
	}

	virtual uint_t do_write(const void* p, uint_t size){
		XTAL_UNLOCK{
			return fwrite(p, 1, size, fp_);
		}
		return 0;
	}

	virtual uint_t do_read(void* p, uint_t size){
		XTAL_UNLOCK{
			return fread(p, 1, size, fp_);
		}
		return 0;
	}

	virtual void close(){
		if(fp_){
			fclose(fp_);
			fp_ = 0;
		}
	}

private:

	FILE* fp_;
};

class StringStreamImpl : public StreamImpl{
public:

	StringStreamImpl(){
		set_class(TClass<StringStream>::get());
		pos_ = 0;
	}
	
	StringStreamImpl(const void* data, uint_t data_size){
		set_class(TClass<StringStream>::get());
		data_.resize(data_size);
		if(data_size>0){
			memcpy(&data_[0], data, data_size);
		}
		pos_ = 0;
	}
	
	virtual uint_t tell(){
		return pos_;
	}

	virtual uint_t do_write(const void* p, uint_t size){
		if(pos_+size>data_.size()){ 
			data_.resize(pos_+size);
		}
		if(size>0){
			memcpy(&data_[pos_], p, size);
		}
		pos_ += size;
		return size;	
	}

	virtual uint_t do_read(void* p, uint_t size){
		if(pos_+size>data_.size()){ 
			uint_t diff = data_.size()-pos_;
			if(diff>0){
				memcpy(p, &data_[pos_], diff);
			}
			pos_ += diff;
			return diff; 
		}
		
		if(size>0){
			memcpy(p, &data_[pos_], size);
		}
		pos_ += size;
		return size;
	}

	void* data(){
		return &data_[0];
	}

	virtual void close(){

	}

	String to_s(){
		if(data_.empty())
			return String("");
		return String((char*)&data_[0], data_.size());
	}

public:

	AC<xtal::u8>::vector data_;
	uint_t pos_;
};


class InteractiveStreamImpl : public StreamImpl{
public:

	InteractiveStreamImpl(){
		//set_class(TClass<InteractiveStreamImpl>::get());
		line_ = 1;
		continue_stmt_ = false;
		closed_ = false;
	}
	
	virtual uint_t tell(){
		unsupported_error("InteractiveStream", "tell");
		return 0;
	}

	virtual uint_t do_write(const void* p, uint_t size){
		unsupported_error("InteractiveStream", "do_write");
		return 0;
	}

	virtual uint_t do_read(void* p, uint_t size){
		if(closed_)
			return 0;
		if(continue_stmt_){
			printf("ix:%03d>    ", line_);
		}else{
			printf("ix:%03d>", line_);
		}
		continue_stmt_ = true;
		if(fgets((char*)p, size, stdin)){
			uint_t sz = strlen((char*)p);
			if(sz!=size-1){
				line_++;
			}
			return sz;
		}
		closed_ = true;
		return 0;
	}

	virtual void close(){

	}

	void set_continue_stmt(bool b){
		continue_stmt_ = b;
	}

private:
	int_t line_;
	bool continue_stmt_;
	bool closed_;
};

}
