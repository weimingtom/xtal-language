
#pragma once

#include "xtal_any.h"
#include "xtal_cast.h"

namespace xtal{

class Stream : public Base{
public:

	enum{
		XSEEK_SET,
		XSEEK_CUR,
		XSEEK_END
	};

	/**
	* @brief 符号付整数8-bitをストリームに書き込む
	*/
	void put_i8(int_t v){
		put_u8(v);
	}

	/**
	* @brief 符号付整数16-bitをストリームに書き込む
	*/
	void put_i16(int_t v){
		put_u16(v);
	}

	/**
	* @brief 符号付整数32-bitをストリームに書き込む
	*/
	void put_i32(int_t v){
		put_u32(v);
	}

	/**
	* @brief 符号無整数8-bitをストリームに書き込む
	*/
	void put_u8(int_t v){
		struct{ u8 data[1]; } data;
		data.data[0] = (v>>0)&0xff;
		write(data.data, 1);
	}

	/**
	* @brief 符号無整数16-bitをストリームに書き込む
	*/
	void put_u16(int_t v){
		struct{ u8 data[2]; } data;
		data.data[0] = (v>>8)&0xff;
		data.data[1] = (v>>0)&0xff;
		write(data.data, 2);
	}

	/**
	* @brief 符号無整数32-bitをストリームに書き込む
	*/
	void put_u32(int_t v){
		struct{ u8 data[4]; } data;
		data.data[0] = (v>>24)&0xff;
		data.data[1] = (v>>16)&0xff;
		data.data[2] = (v>>8)&0xff;
		data.data[3] = (v>>0)&0xff;
		write(data.data, 4);
	}

	i8 get_i8(){
		return (i8)get_u8();
	}

	i16 get_i16(){
		return (i16)get_u16();
	}

	i32 get_i32(){
		return (i32)get_u32();
	}

	u8 get_u8(){
		struct{ u8 data[1]; } data;
		read(data.data, 1);
		return (u8)data.data[0];
	}

	u16 get_u16(){
		struct{ u8 data[2]; } data;
		read(data.data, 2);
		return (u16)((data.data[0]<<8) | data.data[1]);
	}

	u32 get_u32(){
		struct{ u8 data[4]; } data;
		read(data.data, 4);
		return (u32)((data.data[0]<<24) | (data.data[1]<<16) | (data.data[2]<<8) | data.data[3]);
	}

	void put_f32(float_t v){
		union{ u32 u; f32 f; } u;
		u.f = v;
		put_u32(u.u);
	}

	float_t get_f32(){
		union{ u32 u; f32 f; } u;
		u.u = get_u32();
		return u.f;
	}

	void put_s(const StringPtr& str);

	StringPtr get_s(int_t length = 1);

	uint_t print(const StringPtr& str);

	void println(const StringPtr& str);

	virtual uint_t tell() = 0;

	virtual uint_t write(const void* p, uint_t size) = 0;

	virtual uint_t read(void* p, uint_t size) = 0;

	virtual void seek(int_t offset, int_t whence = XSEEK_SET) = 0;

	virtual void close() = 0;

	virtual uint_t pour(const StreamPtr& in_stream, uint_t size);

	virtual uint_t pour_all(const StreamPtr& in_stream);

	virtual uint_t size();

	void iter_first(const VMachinePtr& vm);

	void iter_next(const VMachinePtr& vm);

	void iter_break(const VMachinePtr& vm);

	/**
	* @brief オブジェクトを直列化してストリームに書き込む
	*
	* @param obj 直列化して保存したいオブジェクト
	*/
	void serialize(const AnyPtr& obj);

	/**
	* @brief 直列化されたオブジェクトをストリームから読み出す
	*
	* @return 復元されたオブジェクト
	*/	
	AnyPtr deserialize();

	/**
	* @brief 
	*/
	void xtalize(const AnyPtr& obj);

	/**
	* @brief 
	*/	
	AnyPtr dextalize();


};

class FileStream : public Stream{
public:

	FileStream(const StringPtr& filename, const StringPtr& mode);

	FileStream(FILE* fp);

	~FileStream();

	virtual uint_t tell();

	virtual uint_t write(const void* p, uint_t size);

	virtual uint_t read(void* p, uint_t size);

	virtual void seek(int_t offset, int_t whence = XSEEK_SET);

	virtual void close();

private:

	FILE* fp_;
};

class MemoryStream : public Stream{
public:

	MemoryStream();
	
	MemoryStream(const void* data, uint_t data_size);
	
	virtual uint_t tell();

	virtual uint_t write(const void* p, uint_t size);

	virtual uint_t read(void* p, uint_t size);

	virtual void seek(int_t offset, int_t whence = XSEEK_SET);

	virtual void close(){}

	virtual uint_t pour(const StreamPtr& in_stream, uint_t size);

	virtual uint_t pour_all(const StreamPtr& in_stream);

	void* data(){
		return &data_[0];
	}

	virtual uint_t size(){
		return data_.size();
	}

	void resize(uint_t size){
		data_.resize(size);
	}

	StringPtr to_s();

public:

	AC<xtal::u8>::vector data_;
	uint_t pos_;
};

class InteractiveStream : public Stream{
public:

	InteractiveStream();
	
	virtual uint_t tell();

	virtual uint_t write(const void* p, uint_t size);

	virtual uint_t read(void* p, uint_t size);

	virtual void seek(int_t offset, int_t whence = XSEEK_SET);

	virtual void close();

	void set_continue_stmt(bool b);

private:
	int_t line_;
	bool continue_stmt_;
	FILE* fp_;
};

}
