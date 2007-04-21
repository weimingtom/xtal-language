
#include "streamimpl.h"

namespace xtal{

void InitStream(){
	TClass<Stream> cls("Stream");
	cls.method("p1", &Stream::p1);
	cls.method("p2", &Stream::p2);
	cls.method("p4", &Stream::p4);
	cls.method("s1", &Stream::s1);
	cls.method("s2", &Stream::s2);
	cls.method("s4", &Stream::s4);
	cls.method("u1", &Stream::u1);
	cls.method("u2", &Stream::u2);
	cls.method("u4", &Stream::u4);

	cls.method("iter_first", &Stream::iter_first);
	cls.method("iter_next", &Stream::iter_next);
	cls.method("iter_break", &Stream::iter_break);
}

void InitMemoryStream(){
	TClass<MemoryStream> cls("MemoryStream");
	cls.inherit(TClass<Stream>::get());
	cls.def("new", New<MemoryStream>());
}

void InitFileStream(){
	TClass<FileStream> cls("FileStream");
	cls.inherit(TClass<Stream>::get());
	cls.def("new", New<FileStream, const String&, const String&>());
}

void Stream::p1(int_t v) const{
	impl()->p1(v);
}

void Stream::p2(int_t v) const{
	impl()->p2(v);
}

void Stream::p4(int_t v) const{
	impl()->p4(v);
}

int_t Stream::s1() const{
	return impl()->s1();
}

int_t Stream::s2() const{
	return impl()->s2();
}

int_t Stream::s4() const{
	return impl()->s4();
}

uint_t Stream::u1() const{
	return impl()->u1();
}

uint_t Stream::u2() const{
	return impl()->u2();
}

uint_t Stream::u4() const{
	return impl()->u4();
}

uint_t Stream::tell() const{
	return impl()->tell();
}
	
uint_t Stream::write(const void* p, uint_t size) const{
	return impl()->do_write(p, size);
}
	
uint_t Stream::read(void* p, uint_t size) const{
	return impl()->do_read(p, size);
}

void Stream::close(){
	impl()->close();
}

void Stream::iter_first(const VMachine& vm){
	return impl()->iter_first(vm);
}

void Stream::iter_next(const VMachine& vm){
	return impl()->iter_next(vm);
}

void Stream::iter_break(const VMachine& vm){
	return impl()->iter_break(vm);
}

FileStream::FileStream(const String& filename, const String& mode)
:Stream(null){
	new(*this) FileStreamImpl(filename, mode);
}

MemoryStream::MemoryStream()
:Stream(null){
	new(*this) MemoryStreamImpl();
}

MemoryStream::MemoryStream(const void* data, uint_t data_size)
:Stream(null){
	new(*this) MemoryStreamImpl(data, data_size);
}


void* MemoryStream::data() const{
	return impl()->data();
}

}
