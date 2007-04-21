
#pragma once

#include "any.h"
#include "stream.h"
#include "frame.h"
#include "lib.h"
#include "xmacro.h"

namespace xtal{

class StreamImpl : public AnyImpl{
public:

	void p1(int_t v){
		struct{ u8 data[1]; } data;
		data.data[0] = (v>>0)&0xff;
		do_write(data.data, 1);
	}

	void p2(int_t v){
		struct{ u8 data[2]; } data;
		data.data[0] = (v>>8)&0xff;
		data.data[1] = (v>>0)&0xff;
		do_write(data.data, 2);
	}

	void p4(int_t v){
		struct{ u8 data[4]; } data;
		data.data[0] = (v>>24)&0xff;
		data.data[1] = (v>>16)&0xff;
		data.data[2] = (v>>8)&0xff;
		data.data[3] = (v>>0)&0xff;
		do_write(data.data, 4);
	}

	int_t s1(){
		struct{ u8 data[1]; } data;
		do_read(data.data, 1);
		return (s8)data.data[0];
	}

	int_t s2(){
		struct{ u8 data[2]; } data;
		do_read(data.data, 2);
		return (s16)((data.data[0]<<8) | data.data[1]);
	}

	int_t s4(){
		struct{ u8 data[4]; } data;
		do_read(data.data, 4);
		return ((data.data[0]<<24) | (data.data[1]<<16) | (data.data[2]<<8) | data.data[3]);
	}

	uint_t u1(){
		struct{ u8 data[1]; } data;
		do_read(data.data, 1);
		return (u8)data.data[0];
	}

	uint_t u2(){
		struct{ u8 data[2]; } data;
		do_read(data.data, 2);
		return (u16)((data.data[0]<<8) | data.data[1]);
	}

	uint_t u4(){
		struct{ u8 data[4]; } data;
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
				throw builtin().member(Xid(IOError))(Xt("ファイル '%s' は開けません")(filename));
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
		return fwrite(p, 1, size, fp_);
	}

	virtual uint_t do_read(void* p, uint_t size){
		return fread(p, 1, size, fp_);
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
			data_.resize(pos_+4);
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

private:

	AC<xtal::u8>::vector data_;
	uint_t pos_;
};

}
