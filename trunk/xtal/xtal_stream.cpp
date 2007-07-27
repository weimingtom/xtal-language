
#include "xtal.h"
#include "xtal_streamimpl.h"

namespace xtal{

void InitStream(){
	TClass<Stream> cls("Stream");
	cls.method("put_i8", &Stream::put_i8);
	cls.method("put_i16", &Stream::put_i16);
	cls.method("put_i32", &Stream::put_i32);
	cls.method("put_u8", &Stream::put_u8);
	cls.method("put_u16", &Stream::put_u16);
	cls.method("put_u32", &Stream::put_u32);
	cls.method("get_i8", &Stream::get_i8);
	cls.method("get_i16", &Stream::get_i16);
	cls.method("get_i32", &Stream::get_i32);
	cls.method("get_u8", &Stream::get_u8);
	cls.method("get_u16", &Stream::get_u16);
	cls.method("get_u32", &Stream::get_u32);
	cls.method("put_f32", &Stream::put_f32);
	cls.method("get_f32", &Stream::get_f32);
	
	cls.method("get_s", &Stream::get_s);
	cls.method("put_s", &Stream::put_s);

	cls.method("print", &Stream::print);
	cls.method("println", &Stream::println);

	cls.method("seek", &Stream::seek).param(null, Named("whence", Stream::XSEEK_SET));
	cls.method("tell", &Stream::tell);
	cls.method("pour", &Stream::pour);
	cls.method("pour_all", &Stream::pour_all);
	cls.method("size", &Stream::size);

	cls.method("serialize", &Stream::serialize);
	cls.method("deserialize", &Stream::deserialize);
	cls.method("xtalize", &Stream::xtalize);
	cls.method("dextalize", &Stream::dextalize);

	cls.method("iter_first", &Stream::iter_first);
	cls.method("iter_next", &Stream::iter_next);
	cls.method("iter_break", &Stream::iter_break);

	cls.def("SEEK_SET", Stream::XSEEK_SET);
	cls.def("SEEK_CUR", Stream::XSEEK_CUR);
	cls.def("SEEK_END", Stream::XSEEK_END);
}

void InitMemoryStream(){
	TClass<MemoryStream> cls("MemoryStream");
	cls.inherit(TClass<Stream>::get());
	cls.def("new", New<MemoryStream>());
	cls.method("to_s", &MemoryStream::to_s);
}

void InitFileStream(){
	TClass<FileStream> cls("FileStream");
	cls.inherit(TClass<Stream>::get());
	cls.def("new", New<FileStream, const String&, const String&>().param(Named("name"), Named("mode", "r")));
}

Stream::Stream(){

}

void Stream::put_i8(int_t v) const{
	impl()->put_i8(v);
}

void Stream::put_i16(int_t v) const{
	impl()->put_i16(v);
}

void Stream::put_i32(int_t v) const{
	impl()->put_i32(v);
}

void Stream::put_u8(uint_t v) const{
	impl()->put_u8(v);
}

void Stream::put_u16(uint_t v) const{
	impl()->put_u16(v);
}

void Stream::put_u32(uint_t v) const{
	impl()->put_u32(v);
}

i8 Stream::get_i8() const{
	return impl()->get_i8();
}

i16 Stream::get_i16() const{
	return impl()->get_i16();
}

i32 Stream::get_i32() const{
	return impl()->get_i32();
}

u8 Stream::get_u8() const{
	return impl()->get_u8();
}

u16 Stream::get_u16() const{
	return impl()->get_u16();
}

u32 Stream::get_u32() const{
	return impl()->get_u32();
}

void Stream::put_f32(float_t v) const{
	impl()->put_f32(v);
}

float_t Stream::get_f32() const{
	return impl()->get_f32();
}

void Stream::put_s(const String& str) const{
	impl()->put_s(str);
}

String Stream::get_s(uint_t length) const{
	return impl()->get_s(length);
}

void Stream::print(const String& str) const{
	impl()->print(str);
}

void Stream::println(const String& str) const{
	impl()->println(str);
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

void Stream::seek(int_t offset, int_t whence) const{
	impl()->seek(offset, whence);
}

void Stream::close() const{
	impl()->close();
}

uint_t Stream::pour(const Stream& in_stream, uint_t size) const{
	return impl()->pour(in_stream, size);
}

uint_t Stream::pour_all(const Stream& in_stream) const{
	return impl()->pour_all(in_stream);
}

uint_t Stream::size() const{
	return impl()->size();
}

void Stream::serialize(const Any& v) const{
	Marshal m(*this);
	m.serialize(v);
}

Any Stream::deserialize() const{
	Marshal m(*this);
	return m.deserialize();
}

void Stream::xtalize(const Any& v) const{
	Marshal m(*this);
	m.xtalize(v);
}

Any Stream::dextalize() const{
	//Marshal m(*this);
	//return m.load();
	return null;
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

MemoryStream::MemoryStream(const String& data)
:Stream(null){
	new(*this) MemoryStreamImpl(data.c_str(), data.size()*sizeof(char_t));
}

String MemoryStream::to_s() const{
	return impl()->to_s();
}

void* MemoryStream::data() const{
	return impl()->data();
}

}
