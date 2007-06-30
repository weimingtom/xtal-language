
#pragma once

#include "xtal_any.h"
#include "xtal_stream.h"
#include "xtal_frame.h"
#include "xtal_lib.h"
#include "xtal_macro.h"
#include "xtal_thread.h"

#include <stdio.h>

namespace xtal{

class StreamImpl : public AnyImpl{
public:

	void put_i8(int_t v){
		put_u8(v);
	}

	void put_i16(int_t v){
		put_u16(v);
	}

	void put_i32(int_t v){
		put_u32(v);
	}

	void put_u8(int_t v){
		struct{ u8 data[1]; } data;
		data.data[0] = (v>>0)&0xff;
		do_write(data.data, 1);
	}

	void put_u16(int_t v){
		struct{ u8 data[2]; } data;
		data.data[0] = (v>>8)&0xff;
		data.data[1] = (v>>0)&0xff;
		do_write(data.data, 2);
	}

	void put_u32(int_t v){
		struct{ u8 data[4]; } data;
		data.data[0] = (v>>24)&0xff;
		data.data[1] = (v>>16)&0xff;
		data.data[2] = (v>>8)&0xff;
		data.data[3] = (v>>0)&0xff;
		do_write(data.data, 4);
	}

	i8 get_i8(){
		struct{ xtal::u8 data[1]; } data;
		do_read(data.data, 1);
		return (xtal::i8)data.data[0];
	}

	i16 get_i16(){
		struct{ xtal::u8 data[2]; } data;
		do_read(data.data, 2);
		return (xtal::i16)((data.data[0]<<8) | data.data[1]);
	}

	i32 get_i32(){
		struct{ xtal::u8 data[4]; } data;
		do_read(data.data, 4);
		return ((data.data[0]<<24) | (data.data[1]<<16) | (data.data[2]<<8) | data.data[3]);
	}

	u8 get_u8(){
		struct{ xtal::u8 data[1]; } data;
		do_read(data.data, 1);
		return (xtal::u8)data.data[0];
	}

	u16 get_u16(){
		struct{ xtal::u8 data[2]; } data;
		do_read(data.data, 2);
		return (xtal::u16)((data.data[0]<<8) | data.data[1]);
	}

	u32 get_u32(){
		struct{ xtal::u8 data[4]; } data;
		do_read(data.data, 4);
		return ((data.data[0]<<24) | (data.data[1]<<16) | (data.data[2]<<8) | data.data[3]);
	}

	void put_f32(float_t v){
		union{
			u32 u;
			f32 f;
		} u;
		u.f = v;
		put_u32(u.u);
	}

	float_t get_f32(){
		union{
			u32 u;
			f32 f;
		} u;
		u.u = get_u32();
		return u.f;
	}

	void put_s(const String& str){
		do_write(str.c_str(), str.length());
	}

	String get_s(int_t length){
		char* buf = (char*)user_malloc(length+1);
		uint_t len = do_read(buf, length);
		buf[len] = 0;
		return String(buf, len, length+1, String::delegate_memory_t());
	}

	uint_t print(const String& str){
		return do_write(str.c_str(), str.length());
	}

	void println(const String& str){
		do_write(str.c_str(), str.length());
		do_write("\n", str.length());
	}

	virtual uint_t tell() = 0;

	virtual uint_t do_write(const void* p, uint_t size) = 0;

	virtual uint_t do_read(void* p, uint_t size) = 0;

	virtual void seek(int_t offset, int_t whence) = 0;

	virtual void close() = 0;

	virtual uint_t inpour(const Stream& in_stream, uint_t size){
		xtal::u8* buf = (xtal::u8*)user_malloc(size*sizeof(xtal::u8));
		uint_t len = in_stream.read(buf, size);
		do_write(buf, len);
		user_free(buf, size*sizeof(xtal::u8));
		return len;
	}

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
			fp_ = fopen(filename.c_str(), mode.cat("b").c_str());
			if(!fp_){
				XTAL_THROW(builtin().member(Xid(IOError))(Xt("Xtal Runtime Error 1014")(Named("name", filename))));
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

	virtual void seek(int_t offset, int_t whence){
		int wh = whence==Stream::XSEEK_END ? SEEK_END : whence==Stream::XSEEK_CUR ? SEEK_CUR : SEEK_SET;
		fseek(fp_, offset, wh);
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

class MemoryStreamImpl : public StreamImpl{
public:

	MemoryStreamImpl(){
		set_class(TClass<MemoryStream>::get());
		pos_ = 0;
	}
	
	MemoryStreamImpl(const void* data, uint_t data_size){
		set_class(TClass<MemoryStream>::get());
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

	virtual void seek(int_t offset, int_t whence){
		switch(whence){
			case Stream::XSEEK_END:
				pos_ = data_.size()-offset;
				break;
			case Stream::XSEEK_CUR:
				pos_ += offset;
				break;
			default:
				pos_ = offset;
				break;
		}
	}

	virtual void close(){

	}

	virtual uint_t inpour(const Stream& in_stream, uint_t size){
		if(size==0){
			return 0;
		}

		if(data_.size()-pos_<=size){
			data_.resize(data_.size()-pos_+size);
		}

		return in_stream.read(&data_[pos_], size);
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

	virtual void seek(int_t offset, int_t whence){

	}

	virtual void close(){
		closed_ = true;
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
