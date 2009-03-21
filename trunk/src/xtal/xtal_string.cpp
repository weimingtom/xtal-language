#include "xtal.h"
#include "xtal_macro.h"

namespace xtal{

uint_t string_hashcode(const char_t* str, uint_t size){
	uint_t hash = 2166136261U;
	for(uint_t i=0; i<size; ++i){
		hash = hash*137 ^ str[i];
	}
	return hash;
}

void string_data_size_and_hashcode(const char_t* str, uint_t& size, uint_t& hash){
	hash = 2166136261U;
	size = 0;

	ChMaker chm;

	uint_t i=0;
	while(str[i]){
		chm.clear();
		while(!chm.is_completed()){
			if(str[i]){ chm.add(str[i++]); } 
			else{ break; }
		}
	
		for(int_t j=0; j<chm.pos(); ++j){
			hash = hash*137 ^ chm.at(j);
		}

		size += chm.pos();
	}
}

uint_t string_length(const char_t* str){
	ChMaker chm;
	uint_t length = 0;
	uint_t i=0;
	while(str[i]){
		chm.clear();
		while(!chm.is_completed()){
			if(str[i]){ chm.add(str[i++]); } 
			else{ break; }
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
		int_t diff = (int_t)a[i] - (int_t)b[i];
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


void StringEachIter::visit_members(Visitor& m){
	Base::visit_members(m);
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

	vm->return_result(from_this(this), ss_->get_s(1));
}


void ChRangeIter::block_next(const VMachinePtr& vm){
	if(ch_cmp(it_->data(), it_->data_size(), end_->data(), end_->data_size())>0){
		vm->return_result(null, null);
		return;
	}

	StringPtr temp = it_;
	it_ = ch_inc(it_->data(), it_->data_size());
	vm->return_result(from_this(this), temp);
}

void ChRangeIter::visit_members(Visitor& m){
	Base::visit_members(m);
	m & it_ & end_;
}

AnyPtr ChRange::each(){
	return xnew<ChRangeIter>(from_this(this));
}

////////////////////////////////////////////////////////////////

int_t edit_distance(const StringPtr& str1, const StringPtr& str2){
	return edit_distance(str1->data(), str1->data_size()*sizeof(char_t), str2->data(), str2->data_size()*sizeof(char_t));
}

////////////////////////////////////////////////////////////////

void String::init_string(const char_t* str, uint_t size){
	if(size<SMALL_STRING_MAX){
		set_svalue(*this);
		string_copy(value_.svalue, str, size);
	}
	else{
		if(string_is_ch(str, size)){
			*this = ID(str, size);
		}
		else{
			StringData* sd = (StringData*)xmalloc(StringData::calc_size(size));
			new(sd) StringData(size);
			string_copy(sd->buf(), str, size);
			set_pvalue(*this, TYPE_STRING, sd);
			register_gc(sd);
		}
	}
}

String::String(const char_t* str, uint_t size, make_t){
	if(size<SMALL_STRING_MAX){
		set_svalue(*this);
		string_copy(value_.svalue, str, size);
	}
	else{
		StringData* sd = (StringData*)xmalloc(StringData::calc_size(size));
		new(sd) StringData(size);
		string_copy(sd->buf(), str, size);
		set_pvalue(*this, TYPE_STRING, sd);
		sd->set_interned();
		register_gc(sd);
	}
}

String::String()
:Any(noinit_t()){
	set_svalue(*this);
}

String::String(const char_t* str)
:Any(noinit_t()){
	init_string(str, string_data_size(str));
}

String::String(const avoid<char>::type* str)
:Any(noinit_t()){
	uint_t n = std::strlen((char*)str);
	XMallocGuard umg(n*sizeof(char_t));
	char_t* buf = (char_t*)umg.get();
	for(uint_t i=0; i<n; ++i){
		buf[i] = str[i];
	}
	init_string(buf, n);
}

String::String(const char_t* str, uint_t size)
:Any(noinit_t()){
	init_string(str, size);
}

String::String(const StringLiteral& str)
:Any(noinit_t()){
	init_string(str, str.size());
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
		set_svalue(*this);
		string_copy(value_.svalue, str1, size1);
		string_copy(&value_.svalue[size1], str2, size2);
	}
	else{
		StringData* sd = (StringData*)xmalloc(StringData::calc_size(sz));
		sd = new(sd) StringData(sz);
		string_copy(sd->buf(), str1, size1);
		string_copy(sd->buf()+size1, str2, size2);
		set_pvalue(*this, TYPE_STRING, sd);
		register_gc(sd);
	}
}

String::String(char_t a)
	:Any(noinit_t()){
	if(1<SMALL_STRING_MAX){
		set_svalue(*this);
		value_.svalue[0] = a;
	}
	else{
		init_string(&a, 1);
	}
}

String::String(char_t a, char_t b)
	:Any(noinit_t()){
	if(2<SMALL_STRING_MAX){
		set_svalue(*this);
		value_.svalue[0] = a; value_.svalue[1] = b;
	}
	else{
		char_t buf[2] = {a, b};
		init_string(buf, 2);
	}
}

String::String(char_t a, char_t b, char_t c)
	:Any(noinit_t()){
	if(3<SMALL_STRING_MAX){
		set_svalue(*this);
		value_.svalue[0] = a; value_.svalue[1] = b; value_.svalue[2] = c;
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

uint_t String::length(){
	return string_length(c_str());
}

const char_t* String::c_str(){
	if(type(*this)==TYPE_STRING){
		return ((StringData*)rcpvalue(*this))->buf();
	}
	else{
		return value_.svalue;
		//uint_t size, hash;
		//string_data_size_and_hashcode(value_.svalue, size, hash);
		//return xtal::intern(value_.svalue, size, hash)->data();
	}
}

const char_t* String::data(){
	if(type(*this)==TYPE_STRING){
		return ((StringData*)rcpvalue(*this))->buf();
	}
	else{
		return value_.svalue;
	}
}

uint_t String::data_size(){
	if(type(*this)==TYPE_STRING){
		return ((StringData*)rcpvalue(*this))->data_size();
	}
	else{
		for(uint_t i=0; i<SMALL_STRING_MAX; ++i){
			if(value_.svalue[i]=='\0'){
				return i;
			}
		}
		XTAL_ASSERT(false);
		return 0;
	}
}

StringPtr String::clone(){
	return from_this(this);
}

const IDPtr& String::intern(){
	if(type(*this)==TYPE_STRING){
		StringData* p = ((StringData*)rcpvalue(*this));
		if(p->is_interned()) return unchecked_ptr_cast<ID>(ap(*this));
		return xtal::intern(p->buf(), p->data_size());
	}
	else{
		return unchecked_ptr_cast<ID>(ap(*this));
	}
}

bool String::is_interned(){
	if(type(*this)==TYPE_STRING){
		return ((StringData*)rcpvalue(*this))->is_interned();
	}
	else{
		return true;
	}
}

StringPtr String::to_s(){
	return from_this(this);
}

int_t String::to_i(){
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

float_t String::to_f(){
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

	for(uint_t i=0, sz=data_size(); i<sz; ++i){
		if(str[i]=='.'){
			scale = 0.1f;
			continue;
		}

		ret *= scale;
		ret += str[i];
	} 
	return ret*sign; 
}

AnyPtr String::each(){
	return xnew<StringEachIter>(from_this(this));
}

bool String::is_ch(){
	return string_is_ch(data(), data_size());
}

int_t String::ascii(){
	if(data_size()==0){
		return -1;
	}

	int_t ch = (uchar_t)c_str()[0];
	return ch<128 ? ch : 255;
}

ChRangePtr String::op_range(const StringPtr& right, int_t kind){
	if(kind!=RANGE_CLOSED){
		XTAL_SET_EXCEPT(cpp_class<RuntimeError>()->call(Xt("Xtal Runtime Error 1025")));
		return xnew<ChRange>(empty_string, empty_string);
	}

	if(length()==1 && right->length()==1){
		return xnew<ChRange>(from_this(this), right);
	}
	else{
		XTAL_SET_EXCEPT(cpp_class<RuntimeError>()->call(Xt("Xtal Runtime Error 1023")));
		return xnew<ChRange>(empty_string, empty_string);
	}
}

StringPtr String::op_cat(const StringPtr& v){
	uint_t mysize = data_size();
	uint_t vsize = v->data_size();

	return xnew<String>(c_str(), data_size(), v->c_str(), v->data_size());
}

bool String::op_eq(const StringPtr& v){ 
	return string_compare(data(), data_size(), v->data(), v->data_size())==0; 
}

bool String::op_lt(const StringPtr& v){
	return string_compare(data(), data_size(), v->data(), v->data_size()) < 0;
}

StringPtr String::cat(const StringPtr& v){
	return op_cat(v);
}

////////////////////////////////////////////////////////////////

ID::ID(const char_t* str)
	:String(*xtal::intern(str)){}

ID::ID(const avoid<char>::type* str)	
	:String(noinit_t()){	
	uint_t n = std::strlen((char*)str);
	XMallocGuard umg(n*sizeof(char_t));
	char_t* buf = (char_t*)umg.get();
	for(uint_t i=0; i<n; ++i){
		buf[i] = str[i];
	}
	*this = ID(buf, n);
}

ID::ID(const StringLiteral& str)
	:String(*xtal::intern(str, str.size())){}

ID::ID(const char_t* str, uint_t size)
	:String(*xtal::intern(str, size)){}

ID::ID(const char_t* begin, const char_t* last)
	:String(*xtal::intern(begin, last-begin)){}

ID::ID(char_t a)
	:String(noinit_t()){
	if(1<SMALL_STRING_MAX){
		set_svalue(*this);
		value_.svalue[0] = a;
	}
	else{
		*this = ID(&a, 1);
	}
}

ID::ID(char_t a, char_t b)
	:String(noinit_t()){
	if(2<SMALL_STRING_MAX){
		set_svalue(*this);
		value_.svalue[0] = a; value_.svalue[1] = b;
	}
	else{
		char_t buf[2] = {a, b};
		*this = ID(buf, 2);
	}
}

ID::ID(char_t a, char_t b, char_t c)
	:String(noinit_t()){
	if(3<SMALL_STRING_MAX){
		set_svalue(*this);
		value_.svalue[0] = a; value_.svalue[1] = b; value_.svalue[2] = c;
	}
	else{
		char_t buf[3] = {a, b, c};
		*this = ID(buf, 3);
	}
}

ID::ID(const StringPtr& name)
	:String(*(name ? name->intern() : (const IDPtr&)name)){}


AnyPtr SmartPtrCtor1<String>::call(type v){
	return xnew<String>(v);
}

AnyPtr SmartPtrCtor2<String>::call(type v){
	return xnew<String>(v);
}

AnyPtr SmartPtrCtor3<String>::call(type v){
	return xnew<String>(v);
}

AnyPtr SmartPtrCtor1<ID>::call(type v){
	return intern(v);
}

AnyPtr SmartPtrCtor2<ID>::call(type v){
	if(v) return v->intern();
	return v;
}

AnyPtr SmartPtrCtor3<ID>::call(type v){
	return xnew<ID>(v);
}

AnyPtr SmartPtrCtor4<ID>::call(type v){
	return xnew<ID>(v);
}

}
