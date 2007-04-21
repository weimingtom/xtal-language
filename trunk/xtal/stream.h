
#pragma once

#include "any.h"
#include "cast.h"

namespace xtal{

class StreamImpl;
class MemoryStreamImpl;
class FileStreamImpl;

void InitStream();
void InitMemoryStream();
void InitFileStream();

class Stream : public Any{
public:

	Stream();

	Stream(const Null&)
		:Any(null){}

	explicit Stream(StreamImpl* p)
		:Any((AnyImpl*)p){}

	void p1(int_t v) const;
	void p2(int_t v) const;
	void p4(int_t v) const;

	int_t s1() const;
	int_t s2() const;
	int_t s4() const;

	uint_t u1() const;
	uint_t u2() const;
	uint_t u4() const;

	uint_t tell() const;

	uint_t write(const void* p, uint_t size) const;
	uint_t read(void* p, uint_t size) const;

	void close();

public:

	template<int N> void p(int_t v) const{ p(v, I2T<N>()); }
	void p(int_t v, I2T<1>) const{ p1(v); }
	void p(int_t v, I2T<2>) const{ p2(v); }
	void p(int_t v, I2T<4>) const{ p4(v); }

	template<int N> int_t s() const{ s(I2T<N>()); }
	int_t s(I2T<1>) const{ return s1(); }
	int_t s(I2T<2>) const{ return s2(); }
	int_t s(I2T<4>) const{ return s4(); }

	template<int N> int_t u() const{ u(I2T<N>()); }
	int_t u(I2T<1>) const{ return u1(); }
	int_t u(I2T<2>) const{ return u2(); }
	int_t u(I2T<4>) const{ return u4(); }

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

	FileStream(FileStreamImpl* p)
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

	MemoryStream(const Null&)
		:Stream(null){}

	MemoryStream(MemoryStreamImpl* p)
		:Stream((StreamImpl*)p){}

public:

	void* data() const;

	MemoryStreamImpl* impl() const{
		return (MemoryStreamImpl*)Any::impl();
	}

};

}
