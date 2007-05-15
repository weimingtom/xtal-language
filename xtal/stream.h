
#pragma once

#include "any.h"
#include "cast.h"

namespace xtal{

class StreamImpl;
class StringStreamImpl;
class FileStreamImpl;

void InitStream();
void InitStringStream();
void InitFileStream();

class Stream : public Any{
public:

	Stream();

	Stream(const Null&)
		:Any(null){}

	explicit Stream(StreamImpl* p)
		:Any((AnyImpl*)p){}

	void p8(int_t v) const;
	void p16(int_t v) const;
	void p32(int_t v) const;

	int_t s8() const;
	int_t s16() const;
	int_t s32() const;

	uint_t u8() const;
	uint_t u16() const;
	uint_t u32() const;

	uint_t tell() const;

	void seek(uint_t offset, const String& whence) const;

	uint_t write(const void* p, uint_t size) const;
	uint_t read(void* p, uint_t size) const;

	uint_t write(const String& str) const;
	String read(uint_t size) const;

	void close();

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

class StringStream : public Stream{
public:

	StringStream();
	
	StringStream(const void* data, uint_t data_size);

	StringStream(const Null&)
		:Stream(null){}

	explicit StringStream(StringStreamImpl* p)
		:Stream((StreamImpl*)p){}

public:

	void* data() const;
	
	String to_s() const;

	StringStreamImpl* impl() const{
		return (StringStreamImpl*)Any::impl();
	}

};

}
