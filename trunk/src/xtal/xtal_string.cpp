#include "xtal.h"
#include "xtal_macro.h"

namespace xtal{

uint_t string_hashcode(const char_t* data, uint_t len){
	uint_t h = hash_seed ^ len;

	while(len >= 4){
		uint_t k = data[0];
		k |= data[1] << 8;
		k |= data[2] << 16;
		k |= data[3] << 24;

		k *= hash_m; 
		k ^= k >> hash_r; 
		k *= hash_m;

		h *= hash_m;
		h ^= k;

		data += 4;
		len -= 4;
	}
	
	switch(len){
	case 3: h ^= data[2] << 16;
	case 2: h ^= data[1] << 8;
	case 1: h ^= data[0];
	        h *= hash_m;
	};

	h ^= h >> 13;
	h *= hash_m;
	h ^= h >> 15;

	return h;
}

void string_data_size_and_hashcode(const char_t* str, uint_t& size, uint_t& hash){
	size = string_data_size(str);
	hash = string_hashcode(str, size);
}

uint_t string_length(const char_t* str){
	ChMaker chm;
	uint_t length = 0;
	uint_t i=0;
	while(str[i]){
		chm.clear();
		while(!chm.is_completed()){
			if(str[i]){ 
				chm.add(str[i++]); 
			} 
			else{ 
				break; 
			}
		}
		length += 1;
	}
	return length;
}

uint_t string_length_with_limit(const char_t* str, uint_t limit){
	ChMaker chm;
	uint_t length = 0;
	uint_t i=0;
	while(str[i] && i<limit){
		chm.clear();
		while(!chm.is_completed()){
			if(str[i] && i<limit){ 
				chm.add(str[i++]); 
			} 
			else{ 
				break; 
			}
		}
		length += 1;
	}
	return length;
}

uint_t string_data_size(const char_t* str){
	uint_t ret = 0;
	while(*str++){
		ret++;
	}
	return ret;
}

int_t string_compare(const char_t* a, uint_t asize, const char_t* b, uint_t bsize){
	if(asize<bsize){ return -1; }
	if(bsize<asize){ return 1; }

	for(uint_t i=0; i<asize; ++i){
		int_t diff = (int_t)((uchar_t*)a)[i] - (int_t)((uchar_t*)b)[i];
		if(diff!=0){
			return diff;
		}
	}

	return 0;
}

void string_copy(char_t* a, const char_t* b, uint_t size){
	while(size!=0){
		*a++ = *b++;
		size--;
	}
}

bool string_is_ch(const char_t* str, uint_t size){
	ChMaker chm;
	for(uint_t i=0; i<size; ++i){
		chm.add(str[i]);
		if(chm.is_completed()){
			return i+1==size;
		}
	}
	return false;
}


struct Conv{
	XMallocGuard memory;
	Conv(const char8_t* str);
};

Conv::Conv(const char8_t* str)
	:memory((std::strlen((char*)str)+1)*sizeof(char_t)){
	char_t* buf = (char_t*)memory.get();
	for(uint_t i=0; i<memory.size()/sizeof(char_t); ++i){
		buf[i] = str[i];
	}
}

void StringEachIter::on_visit_members(Visitor& m){
	Base::on_visit_members(m);
	m & ss_;
}

StringEachIter::StringEachIter(const StringPtr& str)
	:ss_(xnew<StringStream>(str)){
}

void StringEachIter::block_next(const VMachinePtr& vm){
	if(ss_->eos()){
		vm->return_result(null, null);
		return;
	}

	vm->return_result(to_smartptr(this), ss_->get_s(1));
}


void ChRangeIter::block_next(const VMachinePtr& vm){
	if(ch_cmp(it_->data(), it_->data_size(), end_->data(), end_->data_size())>0){
		vm->return_result(null, null);
		return;
	}

	StringPtr temp = it_;
	it_ = ch_inc(it_->data(), it_->data_size());
	vm->return_result(to_smartptr(this), temp);
}

AnyPtr ChRange::each(){
	return xnew<ChRangeIter>(to_smartptr(this));
}

////////////////////////////////////////////////////////////////

int_t edit_distance(const StringPtr& str1, const StringPtr& str2){
	return edit_distance(str1->data(), str1->data_size()*sizeof(char_t), str2->data(), str2->data_size()*sizeof(char_t));
}

////////////////////////////////////////////////////////////////

StringData* String::new_string_data(uint_t size){
	StringData* sd = new(object_xmalloc<StringData>()) StringData(size);
	sd->set_virtual_members<StringData>();
	value_.init(TYPE_STRING, sd);
	register_gc(sd);
	return sd;
}

void String::init_string(const char_t* str, uint_t size){
	if(size<SMALL_STRING_MAX){
		value_.init(TYPE_SMALL_STRING);
		string_copy(value_.s(), str, size);
	}
	else{
		if(string_is_ch(str, size)){
			*this = ID(str, size);
		}
		else{
			StringData* sd = new_string_data(size);
			string_copy(sd->buf(), str, size);
		}
	}
}

String::String(const char_t* str, uint_t size, intern_t){
	if(size<SMALL_STRING_MAX){
		value_.init(TYPE_SMALL_STRING);
		string_copy(value_.s(), str, size);
	}
	else{
		StringData* sd = new_string_data(size);
		string_copy(sd->buf(), str, size);
		sd->set_interned();
	}
}

String::String(const StringLiteral& str, intern_t){
	if(str.size()<SMALL_STRING_MAX){
		value_.init(TYPE_SMALL_STRING);
		string_copy(value_.s(), str.str(), str.size());
	}
	else{
		value_.init_string_literal(TYPE_ID_LITERAL, str);
	}
}

String::String()
:Any(noinit_t()){
	value_.init(TYPE_SMALL_STRING);
}

String::String(const char_t* str)
:Any(noinit_t()){
	init_string(str, string_data_size(str));
}

String::String(const char8_t* str)
:Any(noinit_t()){
	Conv conv(str);
	init_string((char_t*)conv.memory.release(), conv.memory.size()/sizeof(char_t)-1);
}

String::String(const char_t* str, uint_t size)
:Any(noinit_t()){
	init_string(str, size);
}

String::String(const StringLiteral& str)
:Any(noinit_t()){

	if(str.size()<SMALL_STRING_MAX){
		value_.init(TYPE_SMALL_STRING);
		string_copy(value_.s(), str.str(), str.size());
	}
	else{
		value_.init_string_literal(TYPE_STRING_LITERAL, str);
	}
}

String::String(const char_t* begin, const char_t* last)
:Any(noinit_t()){
	init_string(begin, last-begin);
}

String::String(const char_t* str1, uint_t size1, const char_t* str2, uint_t size2)
:Any(noinit_t()){
	if(size1==0){
		init_string(str2, size2);
		return;
	}
	else if(size2==0){
		init_string(str1, size1);
		return;
	}

	uint_t sz = size1 + size2;
	if(sz<SMALL_STRING_MAX){
		value_.init(TYPE_SMALL_STRING);
		string_copy(value_.s(), str1, size1);
		string_copy(&value_.s()[size1], str2, size2);
	}
	else{
		StringData* sd = new_string_data(sz);
		string_copy(sd->buf(), str1, size1);
		string_copy(sd->buf()+size1, str2, size2);
	}
}

String::String(char_t a)
	:Any(noinit_t()){
	if(1<SMALL_STRING_MAX){
		value_.init(TYPE_SMALL_STRING);
		value_.s()[0] = a;
	}
	else{
		init_string(&a, 1);
	}
}

String::String(char_t a, char_t b)
	:Any(noinit_t()){
	if(2<SMALL_STRING_MAX){
		value_.init(TYPE_SMALL_STRING);
		value_.s()[0] = a; value_.s()[1] = b;
	}
	else{
		char_t buf[2] = {a, b};
		init_string(buf, 2);
	}
}

String::String(char_t a, char_t b, char_t c)
	:Any(noinit_t()){
	if(3<SMALL_STRING_MAX){
		value_.init(TYPE_SMALL_STRING);
		value_.s()[0] = a; value_.s()[1] = b; value_.s()[2] = c;
	}
	else{
		char_t buf[3] = {a, b, c};
		init_string(buf, 3);
	}
}

String::String(const String& s)
	:Any(s){
	inc_ref_count_force(s);
}
	
String& String::operator= (const String& s){
	if(this==&s){
		return *this;
	}

	Any::operator=(s);
	inc_ref_count_force(s);

	return *this;
}

uint_t String::length() const{
	return string_length(c_str());
}

const char_t* String::c_str() const{
	return data();
}

const char_t* String::data() const{
	switch(type(*this)){
		XTAL_NODEFAULT;
		XTAL_CASE(TYPE_SMALL_STRING){ return value_.s(); }
		XTAL_CASE2(TYPE_ID_LITERAL, TYPE_STRING_LITERAL){ return value_.sp(); }
		XTAL_CASE(TYPE_STRING){ return ((StringData*)rcpvalue(*this))->buf(); }
	}
	return "";
}

uint_t String::data_size() const{
	switch(type(*this)){
		XTAL_NODEFAULT;
		XTAL_CASE(TYPE_SMALL_STRING){ 
			for(uint_t i=0; i<SMALL_STRING_MAX; ++i){
				if(value_.s()[i]=='\0'){
					return i;
				}
			}
			XTAL_ASSERT(false);
		}

		XTAL_CASE2(TYPE_ID_LITERAL, TYPE_STRING_LITERAL){ 
			return value_.string_literal_size();
		}
		XTAL_CASE(TYPE_STRING){ return ((StringData*)rcpvalue(*this))->data_size(); }
	}
	return 0;
}

StringPtr String::clone() const{
	return to_smartptr(this);
}

const IDPtr& String::intern() const{
	switch(type(*this)){
		XTAL_NODEFAULT;
		XTAL_CASE(TYPE_SMALL_STRING){ return unchecked_ptr_cast<ID>(ap(*this)); }
		XTAL_CASE(TYPE_STRING_LITERAL){ return xtal::intern(value_.sp(), value_.string_literal_size()); }
		XTAL_CASE(TYPE_ID_LITERAL){ return unchecked_ptr_cast<ID>(ap(*this)); }
		XTAL_CASE(TYPE_STRING){
			StringData* p = ((StringData*)rcpvalue(*this));
			if(p->is_interned()) return unchecked_ptr_cast<ID>(ap(*this));
			return xtal::intern(p->buf(), p->data_size());
		}
	}
	return empty_id;
}

bool String::is_interned() const{
	switch(type(*this)){
		XTAL_NODEFAULT;
		XTAL_CASE(TYPE_SMALL_STRING){ return true; }
		XTAL_CASE(TYPE_STRING_LITERAL){ return false; }
		XTAL_CASE(TYPE_ID_LITERAL){ return true; }
		XTAL_CASE(TYPE_STRING){ return ((StringData*)rcpvalue(*this))->is_interned(); }
	}
	return false;
}

StringPtr String::to_s() const{
	return to_smartptr(this);
}

int_t String::to_i() const{
	const char_t* str = data();
	int_t ret = 0;
	int_t start = 0, sign = 1;
	
	if(str[0]=='-'){
		sign = -1;
		start = 1;
	}
	else if(str[0]=='+'){
		start = 1;
	}

	for(uint_t i=start, sz=data_size(); i<sz; ++i){
		ret *= 10;
		ret += str[i] - '0';
	}
	return ret*sign; 
}

float_t String::to_f() const{
	const char_t* str = data();
	float_t ret = 0;
	float_t scale = 10;
	int_t start = 0, sign = 1;
	
	if(str[0]=='-'){
		sign = -1;
		start = 1;
	}
	else if(str[0]=='+'){
		start = 1;
	}

	for(uint_t i=start, sz=data_size(); i<sz; ++i){
		if(str[i]=='.'){
			scale = 0.1f;
			continue;
		}

		ret *= scale;
		ret += str[i];
	} 
	return ret*sign; 
}

AnyPtr String::each() const{
	return xnew<StringEachIter>(to_smartptr(this));
}

bool String::is_ch() const{
	return string_is_ch(data(), data_size());
}

bool String::is_empty() const{
	return raweq(*this, empty_string);
}

int_t String::ascii() const{
	if(data_size()==0){
		return -1;
	}

	int_t ch = (uchar_t)c_str()[0];
	return ch<128 ? ch : 255;
}
	
bool String::op_in(const ChRangePtr& range) const{
	const char_t* str = c_str();
	return ch_cmp(str, data_size(), range->left()->c_str(), range->left()->data_size())>=0 &&
		ch_cmp(str, data_size(), range->right()->c_str(), range->right()->data_size())<=0;
}

ChRangePtr String::op_range(const StringPtr& right, int_t kind) const{
	if(kind!=RANGE_CLOSED){
		XTAL_SET_EXCEPT(cpp_class<RuntimeError>()->call(Xt("XRE1025")));
		return xnew<ChRange>(empty_string, empty_string);
	}

	if(length()==1 && right->length()==1){
		return xnew<ChRange>(to_smartptr(this), right);
	}
	else{
		XTAL_SET_EXCEPT(cpp_class<RuntimeError>()->call(Xt("XRE1023")));
		return xnew<ChRange>(empty_string, empty_string);
	}
}

StringPtr String::op_cat(const StringPtr& v) const{
	return xnew<String>(data(), data_size(), v->data(), v->data_size());
}

bool String::op_eq(const StringPtr& v) const{ 
	return string_compare(data(), data_size(), v->data(), v->data_size())==0; 
}

bool String::op_lt(const StringPtr& v) const{
	return string_compare(data(), data_size(), v->data(), v->data_size()) < 0;
}

StringPtr String::cat(const StringPtr& v) const{
	return op_cat(v);
}

AnyPtr String::scan(const AnyPtr& pattern) const{
	return send(Xid(scan), pattern);
}

AnyPtr String::split(const AnyPtr& pattern) const{
	return send(Xid(split), pattern);
}
	
bool String::match(const AnyPtr& pattern) const{
	return send(Xid(match), pattern);
}

StringPtr String::gsub(const AnyPtr& pattern, const AnyPtr& fn) const{
	return ptr_cast<String>(send(Xid(gsub), pattern, fn));
}

StringPtr String::sub(const AnyPtr& pattern, const AnyPtr& fn) const{
	return ptr_cast<String>(send(Xid(sub), pattern, fn));
}

String::iterator String::begin() const{
	return iterator(data());
}

String::iterator String::end() const{
	return iterator(data() + data_size());
}

////////////////////////////////////////////////////////////////

StringData::StringData(uint_t size){
	value_.init(TYPE_STRING, this);
	data_size_ = size<<SIZE_SHIFT;
	buf_ = (char_t*)xmalloc(sizeof(char_t)*(size+1));
	buf()[size] = 0;
}

StringData::~StringData(){
	xfree(buf_, sizeof(char_t)*(data_size()+1));
}

////////////////////////////////////////////////////////////////

ID::ID(const char_t* str)
	:String(*xtal::intern(str)){}

ID::ID(const char8_t* str)	
	:String(noinit_t()){	
	Conv conv(str);
	*this = *xtal::intern((char_t*)conv.memory.release(), conv.memory.size()/sizeof(char_t)-1);
}

ID::ID(const StringLiteral& str)
	:String(*xtal::intern(str)){}

ID::ID(const char_t* str, uint_t size)
	:String(*xtal::intern(str, size)){}

ID::ID(const char_t* begin, const char_t* last)
	:String(*xtal::intern(begin, last-begin)){}

ID::ID(char_t a)
	:String(noinit_t()){
	if(1<SMALL_STRING_MAX){
		value_.init(TYPE_SMALL_STRING);
		value_.s()[0] = a;
	}
	else{
		*this = ID(&a, 1);
	}
}

ID::ID(char_t a, char_t b)
	:String(noinit_t()){
	if(2<SMALL_STRING_MAX){
		value_.init(TYPE_SMALL_STRING);
		value_.s()[0] = a; value_.s()[1] = b;
	}
	else{
		char_t buf[2] = {a, b};
		*this = ID(buf, 2);
	}
}

ID::ID(char_t a, char_t b, char_t c)
	:String(noinit_t()){
	if(3<SMALL_STRING_MAX){
		value_.init(TYPE_SMALL_STRING);
		value_.s()[0] = a; value_.s()[1] = b; value_.s()[2] = c;
	}
	else{
		char_t buf[3] = {a, b, c};
		*this = ID(buf, 3);
	}
}

ID::ID(const StringPtr& name)
	:String(*(name ? name->intern() : (const IDPtr&)name)){}


StringPtr SmartPtrCtor1<String>::call(type v){
	return xnew<String>(v);
}

StringPtr SmartPtrCtor2<String>::call(type v){
	return xnew<String>(v);
}

StringPtr SmartPtrCtor3<String>::call(type v){
	return xnew<String>(v);
}

IDPtr  SmartPtrCtor1<ID>::call(type v){
	return intern(v);
}

IDPtr SmartPtrCtor2<ID>::call(type v){
	if(v) return v->intern();
	return v;
}

IDPtr SmartPtrCtor3<ID>::call(type v){
	return xnew<ID>(v);
}

IDPtr SmartPtrCtor4<ID>::call(type v){
	return xnew<ID>(v);
}

}
