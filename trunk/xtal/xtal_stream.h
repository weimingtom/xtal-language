
#pragma once

#include "xtal_any.h"
#include "xtal_cast.h"

namespace xtal{

class StreamImpl;
class MemoryStreamImpl;
class FileStreamImpl;

class Stream : public Any{
public:

	Stream();

	Stream(const Null&)
		:Any(null){}

	explicit Stream(StreamImpl* p)
		:Any((AnyImpl*)p){}

	void put_i8(int_t v) const;
	void put_i16(int_t v) const;
	void put_i32(int_t v) const;
	void put_u8(uint_t v) const;
	void put_u16(uint_t v) const;
	void put_u32(uint_t v) const;

	i8 get_i8() const;
	i16 get_i16() const;
	i32 get_i32() const;
	u8 get_u8() const;
	u16 get_u16() const;
	u32 get_u32() const;

	void put_f32(float_t v) const;
	float_t get_f32() const;

	void put_s(const String& str) const;
	String get_s(uint_t length) const;

	void print(const String& str) const;
	void println(const String& str) const;

	uint_t tell() const;

	enum{
		XSEEK_SET,
		XSEEK_CUR,
		XSEEK_END
	};

	void seek(int_t offset, int_t whence = XSEEK_SET) const;

	uint_t write(const void* p, uint_t size) const;
	uint_t read(void* p, uint_t size) const;

	void close() const;

	uint_t pour(const Stream& in_stream, uint_t size) const;
	uint_t pour_all(const Stream& in_stream) const;

	uint_t size() const;

	/**
	* @brief オブジェクトを直列化して保存する
	*
	* @param obj 直列化して保存したいオブジェクト
	*/
	void serialize(const Any& obj) const;

	/**
	* @brief 直列化されたオブジェクトを読み出す
	*
	* @return 復元されたオブジェクト
	*/	
	Any deserialize() const;

public:

	void iter_first(const VMachine& vm);
	void iter_next(const VMachine& vm);
	void iter_break(const VMachine& vm);

	StreamImpl* impl() const{
		return (StreamImpl*)Any::impl();
	}
};

class FileStream : public Stream{
public:

	FileStream(const String& filename, const String& mode);

	FileStream(const Null&)
		:Stream(null){}

	explicit FileStream(FileStreamImpl* p)
		:Stream((StreamImpl*)p){}

public:

	FileStreamImpl* impl() const{
		return (FileStreamImpl*)Any::impl();
	}

};

class MemoryStream : public Stream{
public:

	MemoryStream();
	
	MemoryStream(const void* data, uint_t data_size);

	MemoryStream(const String& data);

	MemoryStream(const Null&)
		:Stream(null){}

	explicit MemoryStream(MemoryStreamImpl* p)
		:Stream((StreamImpl*)p){}

public:

	void* data() const;
	
	String to_s() const;

	MemoryStreamImpl* impl() const{
		return (MemoryStreamImpl*)Any::impl();
	}

};

}
