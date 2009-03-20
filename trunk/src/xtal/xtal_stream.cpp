#include "xtal.h"
#include "xtal_macro.h"

#include "xtal_serializer.h"

namespace xtal{

uint_t Stream::tell(){
	XTAL_SET_EXCEPT(unsupported_error(get_class(), Xid(tell), null));
	return 0;
}

uint_t Stream::write(const void* p, uint_t size){
	XTAL_SET_EXCEPT(unsupported_error(get_class(), Xid(write), null));
	return 0;
}

uint_t Stream::read(void* p, uint_t size){
	XTAL_SET_EXCEPT(unsupported_error(get_class(), Xid(read), null));
	return 0;
}

void Stream::seek(uint_t offset){
	XTAL_SET_EXCEPT(unsupported_error(get_class(), Xid(seek), null));
}

StringPtr Stream::get_s(uint_t length){
	if(eos())
		return empty_string;

	if(length==1){
		char_t ch;
		ChMaker chm;
		while(!chm.is_completed()){
			read(&ch, sizeof(char_t));
			chm.add(ch);
		}
		return chm.to_s();
	}

	MemoryStreamPtr ms = xnew<MemoryStream>();
	for(uint_t i=0; i<length; ++i){
		ms->put_s(get_s(1));
	}
	return ms->to_s();
}

StringPtr Stream::get_s_all(){
	if(eos())
		return empty_string;

	MemoryStreamPtr ms = xnew<MemoryStream>();
	while(!eos()){
		ms->put_s(get_s(1));
	}
	return ms->to_s();
}

uint_t Stream::read_charactors(AnyPtr* buffer, uint_t max){
	for(uint_t i=0; i<max; ++i){
		if(eos()){
			return i;
		}

		char_t ch;
		ChMaker chm;
		while(!chm.is_completed()){
			read(&ch, sizeof(char_t));
			chm.add(ch);
		}
	
		buffer[i] = chm.to_s();
	}
	return max;
}

uint_t Stream::print(const AnyPtr& value){
	StringPtr str = value->to_s(); 
	return write(str->data(), str->data_size()*sizeof(char_t));
}

void Stream::println(const AnyPtr& value){
	StringPtr str = value->to_s(); 
	write(str->data(), str->data_size()*sizeof(char_t));
	write(XTAL_STRING("\n"), sizeof(char_t)*1);
}

uint_t Stream::pour(const StreamPtr& in_stream, uint_t size){
	XMallocGuard umg(size*sizeof(xtal::u8));
	xtal::u8* buf = (xtal::u8*)umg.get();
	uint_t len = in_stream->read(buf, size);
	write(buf, len);
	return len;
}

uint_t Stream::pour_all(const StreamPtr& in_stream){
	uint_t size = 1024*10, len, sum = 0;
	XMallocGuard umg(size*sizeof(xtal::u8));
	xtal::u8* buf = (xtal::u8*)umg.get();
	do{
		len = in_stream->read(buf, size);
		sum += len;
		write(buf, len);
	}while(len==size);
	return sum;
}

uint_t Stream::size(){
	XTAL_SET_EXCEPT(unsupported_error(get_class(), Xid(size), null));
	return 0;
}

void Stream::block_first(const VMachinePtr& vm){
	vm->return_result(from_this(this), from_this(this));
}

void Stream::block_next(const VMachinePtr& vm){
	vm->return_result(null, null);
	close();
}

void Stream::block_break(const VMachinePtr& vm){
	close();
}

void Stream::serialize(const AnyPtr& obj){
	Serializer s(from_this(this));
	s.serialize(obj);
}

AnyPtr Stream::deserialize(){
	Serializer s(from_this(this));
	return s.deserialize();
}

void Stream::xtalize(const AnyPtr& obj){

}

AnyPtr Stream::dextalize(){
	return null;
}

/////////////////////////////////////////////////////////////////

PointerStream::PointerStream(const void* data, uint_t size){
	static u8 temp = 0;
	if(data){
		data_ = (u8*)data;
	}
	else{
		data_ = &temp;
	}
	size_ = size;
	pos_ = 0;
}
	
uint_t PointerStream::tell(){
	return pos_;
}

uint_t PointerStream::read(void* p, uint_t size){
	if(pos_+size>size_){ 
		uint_t diff = size_-pos_;
		if(diff>0){
			std::memcpy(p, &data_[pos_], diff);
		}
		pos_ += diff;
		return diff; 
	}
	
	if(size>0){
		std::memcpy(p, &data_[pos_], size);
	}

	pos_ += size;
	return size;
}

void PointerStream::seek(uint_t offset){
	pos_ = offset;
}

StringPtr PointerStream::get_s(uint_t length){
	if(pos_ >= size_)
		return empty_string;

	if(length==1){
		char_t ch;
		ChMaker chm;
		while(!chm.is_completed()){
			read(&ch, sizeof(char_t));
			chm.add(ch);
		}
		return chm.to_s();
	}

	uint_t saved = pos_;

	uint_t slen = 0;
	char_t ch;
	ChMaker chm;
	while(slen<length){
		chm.clear();
		while(!chm.is_completed()){
			read(&ch, sizeof(char_t));
			chm.add(ch);
		}
		slen += 1;
	}

	return xnew<String>((char_t*)&data_[saved], (pos_ - saved)/sizeof(char_t));	
}

StringPtr PointerStream::get_s_all(){
	if(pos_ >= size_)
		return empty_string;

	char_t* data = (char_t*)data_;
	StringPtr ret = xnew<String>((char_t*)&data[pos_], size_ - pos_);
	pos_ = size_;
	return ret;
}

bool PointerStream::eos(){
	return pos_>=size_;
}

///////////////////////////////////////////////////////////////

MemoryStream::MemoryStream(){
	pos_ = 0;
	capa_ = 0;
}

MemoryStream::MemoryStream(const void* data, uint_t data_size){
	pos_ = 0;
	capa_ = 0;
	resize(data_size);
	std::memcpy((void*)data_, data, data_size);
}

MemoryStream::~MemoryStream(){
	if(capa_){
		xfree((void*)data_, capa_);
	}
}

uint_t MemoryStream::write(const void* p, uint_t size){
	if(pos_+size>capa_){
		resize(pos_+size);
	}
	else{
		size_ = pos_+size;
	}

	std::memcpy((void*)&data_[pos_], p, size);

	pos_ += size;

	return size;	
}

uint_t MemoryStream::pour(const StreamPtr& in_stream, uint_t size){
	if(size==0){
		return 0;
	}

	resize(pos_+size);

	uint_t len = in_stream->read((void*)&data_[pos_], size);
	resize(size_ - (size - len));
	pos_ += len;
	return len;
}

uint_t MemoryStream::pour_all(const StreamPtr& in_stream){
	uint_t size = 1024*10, len, sum = 0;
	do{
		if(size_ <= pos_ + size){
			resize(pos_+size);
		}

		len = in_stream->read((void*)&data_[pos_], size);
		sum += len;
	}while(len==size);
	resize(size_ - (size - len));
	pos_ += sum;
	return sum;
}

StringPtr MemoryStream::to_s(){
	return xnew<String>((char_t*)data_, size_/sizeof(char_t));
}

void MemoryStream::clear(){
	seek(0);
	resize(0);
}

void MemoryStream::resize(uint_t size){
	if(size>capa_){
		uint_t newcapa = size + capa_;
		void* newp = xmalloc(newcapa);
		std::memcpy(newp, data_, size_);
		if(capa_){
			xfree((void*)data_, capa_);
		}
		data_ = (u8*)newp;
		capa_ = newcapa;
	}
	size_ = size;
}

//////////////////////////////////////////////////////////////////////////

StringStream::StringStream(const StringPtr& str)
:str_(str ? str : empty_string){
	data_ = (u8*)str_->data();
	size_ = str_->data_size()*sizeof(char_t);
	pos_ = 0;
}

//////////////////////////////////////////////////////////////////////////

void FileStream::open(const StringPtr& path, const StringPtr& aflags){
	close();

	const char_t* flags = aflags->c_str();
	char_t flags_temp[256];
	bool text = false;
	uint_t i = 0;
	for(; flags[i]!=0 && i<10; ++i){
		if(flags[i]=='t'){
			text = true;
		}
		else{
			flags_temp[i] = flags[i];
		}
	}

	if(!text){
		flags_temp[i++] = 'b';
	}
	flags_temp[i++] = 0;

	impl_ = filesystem_lib()->new_file_stream(path->c_str(), flags_temp);
}

}
