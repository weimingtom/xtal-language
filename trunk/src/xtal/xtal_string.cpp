
#include "xtal.h"

#include <string.h>

#include "xtal_string.h"
#include "xtal_macro.h"
#include "xtal_stack.h"
#include "xtal_hashtable.h"
#include "xtal_peg.h"


namespace xtal{

static uint_t make_hashcode(const char_t* str, uint_t size){
	uint_t hash = 2166136261U;
	for(uint_t i=0; i<size; ++i){
		hash = hash*137 ^ str[i];
	}
	return hash;
}

static void make_hashcode_and_length(const char_t* str, uint_t size, uint_t& hash, uint_t& length){
	hash = 2166136261U;
	length = 0;

	for(uint_t i=0; i<size; ++i){
		int_t len = ch_len(str[i]);
		if(len<0){
			if(i + -len > size){
				len = size - i;
			}else{
				len = ch_len2(str+i);
			}
		}
		for(int_t j=0; j<len; ++j){
			hash = hash*137 ^ str[i+j];
		}

		length += 1;
	}
}

static void make_size_and_hashcode_and_length(const char_t* str, uint_t& size, uint_t& hash, uint_t& length){
	hash = 2166136261U;
	length = 0;
	size = 0;
	for(uint_t i=0; str[i]; ++i){
		int_t len = ch_len(str[i]);
		if(len<0){
			len = ch_len2(str+i);
		}
		for(int_t j=0; j<len; ++j){
			hash = hash*137 ^ str[i+j];
		}

		length += 1;
		size += len;
	}
}

static void make_size_and_hashcode_and_length_limit(const char_t* str, uint_t& size, uint_t& hash, uint_t& length){
	hash = 2166136261U;
	length = 0;
	size = 0;
	for(uint_t i=0; str[i]; ++i){
		int_t len = ch_len(str[i]);
		if(len<0){
			if(i + -len > Innocence::SMALL_STRING_MAX){
				len = size - i;
			}else{
				len = ch_len2(str+i);
			}
		}

		for(int_t j=0; j<len; ++j){
			hash = hash*137 ^ str[i+j];
		}

		length += 1;
		size += len;

		if(size==Innocence::SMALL_STRING_MAX){
			break;
		}
	}
}

class StringScanIter : public Base{
	peg::ScannerPtr scanner_;
	AnyPtr pattern_;

	virtual void visit_members(Visitor& m){
		Base::visit_members(m);
		m & scanner_ & pattern_;
	}

public:

	StringScanIter(const StringPtr& str, const AnyPtr& pattern)
		:scanner_(xnew<peg::StreamScanner>(xnew<StringStream>(str))), pattern_(peg::P(pattern)){
	}
	
	void block_next(const VMachinePtr& vm){
		if(scanner_->eof()){
			return vm->return_result(null);
		}

		for(;;){
			peg::parse_scanner(pattern_, scanner_);
			if(scanner_->success()){
				vm->return_result(from_this(this), scanner_->pop_result());
				return;
			}else{
				scanner_->read();
				if(scanner_->eof()){
					vm->return_result(null);
					return;
				}
			}
		}
	}
};

class StringEachIter : public Base{
	StringStreamPtr ss_;

	virtual void visit_members(Visitor& m){
		Base::visit_members(m);
		m & ss_;
	}

public:

	StringEachIter(const StringPtr& str)
		:ss_(xnew<StringStream>(str)){
	}

	void block_next(const VMachinePtr& vm){
		if(ss_->eof()){
			return vm->return_result(null);
		}

		vm->return_result(from_this(this), ss_->get_s(1));
	}
};

////////////////////////////////////////////////////////////////

class StringMgr : public GCObserver{
public:

	struct Key{
		const char_t* str;
		uint_t size;
	};

	struct Value{
		uint_t hashcode;
		StringPtr value;
		char_t buf[Innocence::SMALL_STRING_MAX+1];
	};

	struct Fun{
		static uint_t hash(const Key& key){
			return make_hashcode(key.str, key.size);
		}

		static bool eq(const Key& a, const Key& b){
			return a.size==b.size && memcmp(a.str, b.str, a.size)==0;
		}
	};

	typedef Hashtable<Key, Value, Fun> table_t; 
	table_t table_;

	StringMgr(){
		guard_ = 0;
	}

protected:

	int_t guard_;

	struct Guard{
		int_t& count;
		Guard(int_t& c):count(c){ count++; }
		~Guard(){ count--; }
	private:
		void operator=(const Guard&);
	};

	virtual void visit_members(Visitor& m){
		GCObserver::visit_members(m);
		for(table_t::iterator it = table_.begin(); it!=table_.end(); ++it){
			m & it->second.value;
		}		
	}

public:

	const Value& insert(const char* str, uint_t size){
		uint_t hashcode;
		uint_t length;
		make_hashcode_and_length(str, size, hashcode, length);
		return insert(str, size, hashcode, length);
	}

	const Value& insert(const char* str){
		uint_t hashcode;
		uint_t length;
		uint_t size;
		make_size_and_hashcode_and_length(str, size, hashcode, length);
		return insert(str, size, hashcode, length);
	}

	const Value& insert(const char* str, uint_t size, uint_t hash, uint_t length);

	virtual void before_gc();
};

const StringMgr::Value& StringMgr::insert(const char* str, uint_t size, uint_t hashcode, uint_t length){
	Guard guard(guard_);

	Key key = {str, size};
	table_t::iterator it = table_.find(key, hashcode);
	if(it!=table_.end()){
		return it->second;
	}

	Value value = {hashcode, xnew<String>(str, size, hashcode, length, true)};
	if(size<=Innocence::SMALL_STRING_MAX){
		memcpy(value.buf, str, sizeof(char_t)*size);
		value.buf[size] = 0;
	}

	it = table_.insert(key, value, hashcode).first;
	it->first.str = it->second.value->data();
	return it->second;
}

void StringMgr::before_gc(){
	return;

	if(guard_){
		return;
	}
}

namespace{

SmartPtr<StringMgr> str_mgr_;

void uninitialize_string(){
	str_mgr_ = null;
}

}

////////////////////////////////////////////////////////////////

void initialize_string(){
	register_uninitializer(&uninitialize_string);
	str_mgr_ = xnew<StringMgr>();

	{
		ClassPtr p = new_cpp_class<StringScanIter>("StringScanIter");
		p->inherit(Iterator());
		p->method("block_next", &StringScanIter::block_next);
	}
	
	{
		ClassPtr p = new_cpp_class<StringEachIter>("StringEachIter");
		p->inherit(Iterator());
		p->method("block_next", &StringEachIter::block_next);
	}

	{
		ClassPtr p = new_cpp_class<String>("String");
		p->inherit(Enumerator());

		p->def("new", ctor<String>());
		p->method("to_i", &String::to_i);
		p->method("to_f", &String::to_f);
		p->method("to_s", &String::to_s);
		p->method("clone", &String::clone);

		p->method("length", &String::length);
		p->method("size", &String::size);
		p->method("intern", &String::intern);

		p->method("split", &String::split)->param("i", Named("n", 1));
		p->method("each", &String::each);
		p->method("replace", &String::replace);
		p->method("scan", &String::scan);

		p->def("op_cat", method(&String::op_cat), get_cpp_class<String>());
		p->def("op_cat_assign", method(&String::op_cat), get_cpp_class<String>());
		p->def("op_eq", method(&String::op_eq), get_cpp_class<String>());
		p->def("op_lt", method(&String::op_lt), get_cpp_class<String>());
	}

	set_cpp_class<InternedString>(get_cpp_class<String>());
}

////////////////////////////////////////////////////////////////

void String::init_string(const char_t* str, uint_t sz){
	if(sz<=SMALL_STRING_MAX){
		set_small_string();
		memcpy(svalue_, str, sz);
	}else{
		uint_t hash, length;
		make_hashcode_and_length(str, sz, hash, length);
		if(length<=1){
			set_p(pvalue(str_mgr_->insert(str, sz, hash, length).value));
		}else{
			set_p(new LargeString(str, sz, hash, length));
			pvalue(*this)->set_class(new_cpp_class<String>());
			pvalue(*this)->dec_ref_count();
		}
	}
}


String::String(const char* str):Any(noinit_t()){
	init_string(str, strlen(str));
}

String::String(const string_t& str):Any(noinit_t()){
	init_string(str.c_str(), str.size());
}

String::String(const char* str, uint_t size):Any(noinit_t()){
	init_string(str, size);
}

String::String(const char* begin, const char* last):Any(noinit_t()){
	init_string(begin, last-begin);
}

String::String(const char* str1, uint_t size1, const char* str2, uint_t size2):Any(noinit_t()){
	if(size1==0){
		init_string(str2, size2);
		return;
	}else if(size2==0){
		init_string(str1, size1);
		return;
	}

	uint_t sz = size1 + size2;
	if(sz<=SMALL_STRING_MAX){
		set_small_string();
		memcpy(svalue_, str1, size1);
		memcpy(&svalue_[size1], str2, size2);
	}else{
		set_p(new LargeString(str1, size1, str2, size2));
		pvalue(*this)->set_class(new_cpp_class<String>());
		pvalue(*this)->dec_ref_count();
	}
}

String::String(char_t a)
	:Any(noinit_t()){
	if(1<=SMALL_STRING_MAX){
		set_small_string();
		svalue_[0] = a;
	}else{
		init_string(&a, 1);
	}
}

String::String(char_t a, char_t b)
	:Any(noinit_t()){
	if(2<=SMALL_STRING_MAX){
		set_small_string();
		svalue_[0] = a; svalue_[1] = b;
	}else{
		char_t buf[2] = {a, b};
		init_string(buf, 2);
	}
}

String::String(char_t a, char_t b, char_t c)
	:Any(noinit_t()){
	if(3<=SMALL_STRING_MAX){
		set_small_string();
		svalue_[0] = a; svalue_[1] = b; svalue_[2] = c;
	}else{
		char_t buf[3] = {a, b, c};
		init_string(buf, 3);
	}
}

String::String(const char* str, uint_t size, uint_t hashcode, uint_t length, bool intern_flag):Any(noinit_t()){
	uint_t sz = size;
	if(sz<=SMALL_STRING_MAX){
		set_small_string();
		memcpy(svalue_, str, sz);
	}else{
		if(!intern_flag && length<=1){
			set_p(pvalue(str_mgr_->insert(str, sz, hashcode, length).value));
		}else{
			set_p(new LargeString(str, sz, hashcode, length, intern_flag));
			pvalue(*this)->set_class(new_cpp_class<String>());
			pvalue(*this)->dec_ref_count();
		}
	}
}

String::String(LargeString* left, LargeString* right):Any(noinit_t()){
	if(left->buffer_size()==0){
		init_string(right->c_str(), right->buffer_size());
		return;
	}else if(right->buffer_size()==0){
		init_string(left->c_str(), left->buffer_size());
		return;
	}

	set_p(new LargeString(left, right));
	pvalue(*this)->set_class(new_cpp_class<String>());
	pvalue(*this)->dec_ref_count();
}

const char* String::c_str(){
	if(type(*this)==TYPE_BASE){
		return ((LargeString*)pvalue(*this))->c_str();
	}else{
		uint_t size, hash, length;
		make_size_and_hashcode_and_length_limit(svalue_, size, hash, length);
		return str_mgr_->insert(svalue_, size, hash, length).buf;
	}
}

const char_t* String::data(){
	if(type(*this)==TYPE_BASE){
		return ((LargeString*)pvalue(*this))->c_str();
	}else{
		return svalue_;
	}
}

uint_t String::buffer_size(){
	if(type(*this)==TYPE_BASE){
		return ((LargeString*)pvalue(*this))->buffer_size();
	}else{
		for(uint_t i=0; i<=SMALL_STRING_MAX; ++i){
			if(svalue_[i]=='\0'){
				return i;
			}
		}
		return SMALL_STRING_MAX;
	}
}

uint_t String::length(){
	if(type(*this)==TYPE_BASE){
		return ((LargeString*)pvalue(*this))->length();
	}else{
		uint_t size, hash, length;
		make_size_and_hashcode_and_length(svalue_, size, hash, length);
		return length;
	}
}

uint_t String::size(){
	return length();
}

StringPtr String::clone(){
	return from_this(this);
}

const InternedStringPtr& String::intern(){
	if(type(*this)==TYPE_BASE){
		LargeString* p = ((LargeString*)pvalue(*this));
		if(p->is_interned()) return static_ptr_cast<InternedString>(ap(*this));
		return static_ptr_cast<InternedString>(str_mgr_->insert(p->c_str(), p->buffer_size(), p->hashcode(), p->length()).value);
	}else{
		return static_ptr_cast<InternedString>(ap(*this));
	}
}

bool String::is_interned(){
	if(type(*this)==TYPE_BASE){
		return ((LargeString*)pvalue(*this))->is_interned();
	}else{
		return true;
	}
}

StringPtr String::to_s(){
	return from_this(this);
}

int_t String::to_i(){ 
	return atoi(c_str()); 
}

float_t String::to_f(){ 
	return (float_t)atof(c_str()); 
}

AnyPtr String::split(const AnyPtr& sep){
	return xnew<StringScanIter>(from_this(this), 
		peg::join((peg::any - sep)*0) >> ~sep*-1
	);
}

AnyPtr String::each(){
	return xnew<StringEachIter>(from_this(this));
}
	
AnyPtr String::scan(const AnyPtr& p){
	return xnew<StringScanIter>(from_this(this), 
		peg::join(p)
	);
}

AnyPtr String::replace(const AnyPtr& pattern, const StringPtr& str){
	AnyPtr elem = peg::sub(peg::any, pattern)*0;
	peg::ScannerPtr scanner = peg::parse_string(peg::join(
		elem >> (~pattern >> peg::val(str) >> elem)*0
	), from_this(this));

	return scanner->success() ? scanner->pop_result() : "";
}


StringPtr String::op_cat(const StringPtr& v){
	uint_t mysize = buffer_size();
	uint_t vsize = v->buffer_size();

	if(mysize+vsize <= 16 || mysize<=SMALL_STRING_MAX || vsize<=SMALL_STRING_MAX)
		return xnew<String>(data(), mysize, v->data(), vsize);
	return xnew<String>((LargeString*)pvalue(*this), (LargeString*)pvalue(v));
}

bool String::op_eq(const StringPtr& v){ 
	return buffer_size()==v->buffer_size() && memcmp(data(), v->data(), buffer_size())==0; 
}

bool String::op_lt(const StringPtr& v){ 
	return strcmp(c_str(), v->c_str())<0; 
}

StringPtr String::cat(const StringPtr& v){
	return op_cat(v);
}

uint_t String::hashcode(){
	if(type(*this)==TYPE_BASE){
		return ((LargeString*)pvalue(*this))->hashcode();
	}else{
		return make_hashcode(svalue_, buffer_size());
	}
}

int_t String::calc_offset(int_t i){
	uint_t sz = buffer_size();
	if(i<0){
		i = sz + i;
		if(i<0){
			throw_index_error();
			return 0;
		}
	}else{
		if((uint_t)i >= sz){
			throw_index_error();
			return 0;
		}
	}
	return i;
}

void String::throw_index_error(){
	XTAL_THROW(builtin()->member("RuntimeError")(Xt("Xtal Runtime Error 1020")), return);
}

////////////////////////////////////////////////////////////////


void LargeString::visit_members(Visitor& m){
	Base::visit_members(m);
	if((flags_ & ROPE)!=0){
		m & rope_.left & rope_.right;
	}
}

void visit_members(Visitor& m, const Named& p){
	m & p.name & p.value;
}

struct StringKey{
	const char* str;
	int_t size;

	StringKey(const char* str, int_t size)
		:str(str), size(size){}

	friend bool operator <(const StringKey& a, const StringKey& b){
		if(a.size<b.size)
			return true;
		if(a.size>b.size)
			return false;
		return memcmp(a.str, b.str, a.size)<0;
	}
};

void LargeString::common_init(uint_t size){
	XTAL_ASSERT(size>Innocence::SMALL_STRING_MAX);

	buffer_size_ = size;
	str_.p = static_cast<char*>(user_malloc(buffer_size_+1));
	str_.p[buffer_size_] = 0;
	flags_ = 0;
}

LargeString::LargeString(const char* str1, uint_t size1, const char* str2, uint_t size2){
	common_init(size1 + size2);
	memcpy(str_.p, str1, size1);
	memcpy(str_.p+size1, str2, size2);
	make_hashcode_and_length(str_.p, buffer_size_, str_.hashcode, length_);
}

LargeString::LargeString(const char* str, uint_t size, uint_t hashcode, uint_t length, bool intern_flag){
	common_init(size);
	memcpy(str_.p, str, buffer_size_);
	str_.hashcode = hashcode;
	flags_ = intern_flag ? INTERNED : 0;
	length_ = length;
}

LargeString::LargeString(LargeString* left, LargeString* right){
	left->inc_ref_count();
	right->inc_ref_count();
	rope_.left = left;
	rope_.right = right;
	buffer_size_ = left->buffer_size() + right->buffer_size();
	flags_ = ROPE;
	length_ = left->length() + right->length();
}

LargeString::~LargeString(){
	if((flags_ & ROPE)==0){
		user_free(str_.p);
	}else{
		rope_.left->dec_ref_count();
		rope_.right->dec_ref_count();
	}
}

const char* LargeString::c_str(){ 
	if((flags_ & ROPE)!=0)
		became_unified();
	return str_.p; 
}

uint_t LargeString::hashcode(){ 
	if((flags_ & ROPE)!=0)
		became_unified();
	return str_.hashcode; 
}

void LargeString::became_unified(){
	uint_t pos = 0;
	char_t* memory = (char_t*)user_malloc(sizeof(char_t)*(buffer_size_+1));
	write_to_memory(this, memory, pos);
	memory[pos] = 0;
	rope_.left->dec_ref_count();
	rope_.right->dec_ref_count();
	str_.p = memory;
	flags_ = 0;
	make_hashcode_and_length(str_.p, buffer_size_, str_.hashcode, length_);
}

void LargeString::write_to_memory(LargeString* p, char_t* memory, uint_t& pos){
	PStack<LargeString*> stack;
	for(;;){
		if((p->flags_ & ROPE)==0){
			memcpy(&memory[pos], p->str_.p, p->buffer_size_);
			pos += p->buffer_size_;
			if(stack.empty())
				return;
			p = stack.pop();
		}else{
			stack.push(p->rope_.right);
			p = p->rope_.left;
		}
	}
}



InternedString::InternedString(const char_t* str)
	:String(*str_mgr_->insert(str).value){}

InternedString::InternedString(const string_t& str)
	:String(*str_mgr_->insert(str.c_str(), str.size()).value){}

InternedString::InternedString(const char_t* str, uint_t size)
	:String(*str_mgr_->insert(str, size).value){}

InternedString::InternedString(const char_t* begin, const char_t* last)
	:String(*str_mgr_->insert(begin, last-begin).value){}

InternedString::InternedString(char_t a)
	:String(noinit_t()){
	if(1<=SMALL_STRING_MAX){
		set_small_string();
		svalue_[0] = a;
	}else{
		*this = InternedString(&a, 1);
	}
}

InternedString::InternedString(char_t a, char_t b)
	:String(noinit_t()){
	if(2<=SMALL_STRING_MAX){
		set_small_string();
		svalue_[0] = a; svalue_[1] = b;
	}else{
		char_t buf[2] = {a, b};
		*this = InternedString(buf, 2);
	}
}

InternedString::InternedString(char_t a, char_t b, char_t c)
	:String(noinit_t()){
	if(3<=SMALL_STRING_MAX){
		set_small_string();
		svalue_[0] = a; svalue_[1] = b; svalue_[2] = c;
	}else{
		char_t buf[3] = {a, b, c};
		*this = InternedString(buf, 3);
	}
}

InternedString::InternedString(const StringPtr& name)
	:String(*(name ? name->intern() : (const InternedStringPtr&)name)){}


AnyPtr SmartPtrCtor1<InternedString>::call(type v){
	return str_mgr_->insert(v).value;
}

AnyPtr SmartPtrCtor2<InternedString>::call(type v){
	if(v) return v;
	return v->intern();
}

#ifdef XTAL_USE_PREDEFINED_ID

//{ID{{
namespace id{
InternedStringPtr idop_inc;
InternedStringPtr idblock_catch;
InternedStringPtr idcallee;
InternedStringPtr idnew;
InternedStringPtr idop_shl_assign;
InternedStringPtr idop_at;
InternedStringPtr idtest;
InternedStringPtr idfor;
InternedStringPtr idserial_new;
InternedStringPtr idop_div_assign;
InternedStringPtr idop_mul;
InternedStringPtr idop_xor_assign;
InternedStringPtr idto_a;
InternedStringPtr idinitialize;
InternedStringPtr idonce;
InternedStringPtr iddo;
InternedStringPtr idstring;
InternedStringPtr idfalse;
InternedStringPtr idop_and_assign;
InternedStringPtr idop_add_assign;
InternedStringPtr idop_cat_assign;
InternedStringPtr idsingleton;
InternedStringPtr idat;
InternedStringPtr idop_shl;
InternedStringPtr idblock_next;
InternedStringPtr idyield;
InternedStringPtr idop_lt_r_String;
InternedStringPtr idop_shr_assign;
InternedStringPtr idop_cat;
InternedStringPtr idop_neg;
InternedStringPtr idop_dec;
InternedStringPtr idinstance_serial_save;
InternedStringPtr idvalue;
InternedStringPtr iddefault;
InternedStringPtr idcase;
InternedStringPtr idto_s;
InternedStringPtr idop_shr;
InternedStringPtr idpure;
InternedStringPtr idfinally;
InternedStringPtr idserial_load;
InternedStringPtr idthis;
InternedStringPtr idnull;
InternedStringPtr idop_div;
InternedStringPtr idIOError;
InternedStringPtr id_dummy_lhs_parameter_;
InternedStringPtr idcatch;
InternedStringPtr idop_mul_assign;
InternedStringPtr idmethod;
InternedStringPtr idop_lt;
InternedStringPtr id_switch_;
InternedStringPtr idop_mod_assign;
InternedStringPtr idset_at;
InternedStringPtr idbreak;
InternedStringPtr idtry;
InternedStringPtr idop_mod;
InternedStringPtr idnop;
InternedStringPtr idto_i;
InternedStringPtr idop_or;
InternedStringPtr idcontinue;
InternedStringPtr ide;
InternedStringPtr iditerator;
InternedStringPtr idthrow;
InternedStringPtr idop_and;
InternedStringPtr idelse;
InternedStringPtr idfun;
InternedStringPtr idto_f;
InternedStringPtr idop_sub_assign;
InternedStringPtr idlib;
InternedStringPtr iddofun;
InternedStringPtr ideach;
InternedStringPtr idop_set_at;
InternedStringPtr idclass;
InternedStringPtr idop_com;
InternedStringPtr idop_pos;
InternedStringPtr idop_add;
InternedStringPtr idop_ushr_assign;
InternedStringPtr idop_eq_r_String;
InternedStringPtr idop_cat_r_String;
InternedStringPtr idnobreak;
InternedStringPtr idcurrent_context;
InternedStringPtr idto_m;
InternedStringPtr idreturn;
InternedStringPtr idop_eq;
InternedStringPtr idfiber;
InternedStringPtr idop_or_assign;
InternedStringPtr idserial_save;
InternedStringPtr idstatic;
InternedStringPtr idswitch;
InternedStringPtr idop_sub;
InternedStringPtr idop_ushr;
InternedStringPtr idfirst_step;
InternedStringPtr idblock_break;
InternedStringPtr id_dummy_fun_parameter_;
InternedStringPtr id_dummy_block_parameter_;
InternedStringPtr idunittest;
InternedStringPtr idop_xor;
InternedStringPtr idblock_first;
InternedStringPtr idtrue;
InternedStringPtr idop_call;
InternedStringPtr id_initialize_;
InternedStringPtr idis;
InternedStringPtr idwhile;
InternedStringPtr idit;
InternedStringPtr idassert;
InternedStringPtr idxtal;
InternedStringPtr idif;
InternedStringPtr idp;
}
void uninitialize_interned_string(){
id::idop_inc = null;
id::idblock_catch = null;
id::idcallee = null;
id::idnew = null;
id::idop_shl_assign = null;
id::idop_at = null;
id::idtest = null;
id::idfor = null;
id::idserial_new = null;
id::idop_div_assign = null;
id::idop_mul = null;
id::idop_xor_assign = null;
id::idto_a = null;
id::idinitialize = null;
id::idonce = null;
id::iddo = null;
id::idstring = null;
id::idfalse = null;
id::idop_and_assign = null;
id::idop_add_assign = null;
id::idop_cat_assign = null;
id::idsingleton = null;
id::idat = null;
id::idop_shl = null;
id::idblock_next = null;
id::idyield = null;
id::idop_lt_r_String = null;
id::idop_shr_assign = null;
id::idop_cat = null;
id::idop_neg = null;
id::idop_dec = null;
id::idinstance_serial_save = null;
id::idvalue = null;
id::iddefault = null;
id::idcase = null;
id::idto_s = null;
id::idop_shr = null;
id::idpure = null;
id::idfinally = null;
id::idserial_load = null;
id::idthis = null;
id::idnull = null;
id::idop_div = null;
id::idIOError = null;
id::id_dummy_lhs_parameter_ = null;
id::idcatch = null;
id::idop_mul_assign = null;
id::idmethod = null;
id::idop_lt = null;
id::id_switch_ = null;
id::idop_mod_assign = null;
id::idset_at = null;
id::idbreak = null;
id::idtry = null;
id::idop_mod = null;
id::idnop = null;
id::idto_i = null;
id::idop_or = null;
id::idcontinue = null;
id::ide = null;
id::iditerator = null;
id::idthrow = null;
id::idop_and = null;
id::idelse = null;
id::idfun = null;
id::idto_f = null;
id::idop_sub_assign = null;
id::idlib = null;
id::iddofun = null;
id::ideach = null;
id::idop_set_at = null;
id::idclass = null;
id::idop_com = null;
id::idop_pos = null;
id::idop_add = null;
id::idop_ushr_assign = null;
id::idop_eq_r_String = null;
id::idop_cat_r_String = null;
id::idnobreak = null;
id::idcurrent_context = null;
id::idto_m = null;
id::idreturn = null;
id::idop_eq = null;
id::idfiber = null;
id::idop_or_assign = null;
id::idserial_save = null;
id::idstatic = null;
id::idswitch = null;
id::idop_sub = null;
id::idop_ushr = null;
id::idfirst_step = null;
id::idblock_break = null;
id::id_dummy_fun_parameter_ = null;
id::id_dummy_block_parameter_ = null;
id::idunittest = null;
id::idop_xor = null;
id::idblock_first = null;
id::idtrue = null;
id::idop_call = null;
id::id_initialize_ = null;
id::idis = null;
id::idwhile = null;
id::idit = null;
id::idassert = null;
id::idxtal = null;
id::idif = null;
id::idp = null;
}
void initialize_interned_string(){
register_uninitializer(&uninitialize_interned_string);
id::idop_inc = InternedStringPtr("op_inc");
id::idblock_catch = InternedStringPtr("block_catch");
id::idcallee = InternedStringPtr("callee");
id::idnew = InternedStringPtr("new");
id::idop_shl_assign = InternedStringPtr("op_shl_assign");
id::idop_at = InternedStringPtr("op_at");
id::idtest = InternedStringPtr("test");
id::idfor = InternedStringPtr("for");
id::idserial_new = InternedStringPtr("serial_new");
id::idop_div_assign = InternedStringPtr("op_div_assign");
id::idop_mul = InternedStringPtr("op_mul");
id::idop_xor_assign = InternedStringPtr("op_xor_assign");
id::idto_a = InternedStringPtr("to_a");
id::idinitialize = InternedStringPtr("initialize");
id::idonce = InternedStringPtr("once");
id::iddo = InternedStringPtr("do");
id::idstring = InternedStringPtr("string");
id::idfalse = InternedStringPtr("false");
id::idop_and_assign = InternedStringPtr("op_and_assign");
id::idop_add_assign = InternedStringPtr("op_add_assign");
id::idop_cat_assign = InternedStringPtr("op_cat_assign");
id::idsingleton = InternedStringPtr("singleton");
id::idat = InternedStringPtr("at");
id::idop_shl = InternedStringPtr("op_shl");
id::idblock_next = InternedStringPtr("block_next");
id::idyield = InternedStringPtr("yield");
id::idop_lt_r_String = InternedStringPtr("op_lt_r_String");
id::idop_shr_assign = InternedStringPtr("op_shr_assign");
id::idop_cat = InternedStringPtr("op_cat");
id::idop_neg = InternedStringPtr("op_neg");
id::idop_dec = InternedStringPtr("op_dec");
id::idinstance_serial_save = InternedStringPtr("instance_serial_save");
id::idvalue = InternedStringPtr("value");
id::iddefault = InternedStringPtr("default");
id::idcase = InternedStringPtr("case");
id::idto_s = InternedStringPtr("to_s");
id::idop_shr = InternedStringPtr("op_shr");
id::idpure = InternedStringPtr("pure");
id::idfinally = InternedStringPtr("finally");
id::idserial_load = InternedStringPtr("serial_load");
id::idthis = InternedStringPtr("this");
id::idnull = InternedStringPtr("null");
id::idop_div = InternedStringPtr("op_div");
id::idIOError = InternedStringPtr("IOError");
id::id_dummy_lhs_parameter_ = InternedStringPtr("_dummy_lhs_parameter_");
id::idcatch = InternedStringPtr("catch");
id::idop_mul_assign = InternedStringPtr("op_mul_assign");
id::idmethod = InternedStringPtr("method");
id::idop_lt = InternedStringPtr("op_lt");
id::id_switch_ = InternedStringPtr("_switch_");
id::idop_mod_assign = InternedStringPtr("op_mod_assign");
id::idset_at = InternedStringPtr("set_at");
id::idbreak = InternedStringPtr("break");
id::idtry = InternedStringPtr("try");
id::idop_mod = InternedStringPtr("op_mod");
id::idnop = InternedStringPtr("nop");
id::idto_i = InternedStringPtr("to_i");
id::idop_or = InternedStringPtr("op_or");
id::idcontinue = InternedStringPtr("continue");
id::ide = InternedStringPtr("e");
id::iditerator = InternedStringPtr("iterator");
id::idthrow = InternedStringPtr("throw");
id::idop_and = InternedStringPtr("op_and");
id::idelse = InternedStringPtr("else");
id::idfun = InternedStringPtr("fun");
id::idto_f = InternedStringPtr("to_f");
id::idop_sub_assign = InternedStringPtr("op_sub_assign");
id::idlib = InternedStringPtr("lib");
id::iddofun = InternedStringPtr("dofun");
id::ideach = InternedStringPtr("each");
id::idop_set_at = InternedStringPtr("op_set_at");
id::idclass = InternedStringPtr("class");
id::idop_com = InternedStringPtr("op_com");
id::idop_pos = InternedStringPtr("op_pos");
id::idop_add = InternedStringPtr("op_add");
id::idop_ushr_assign = InternedStringPtr("op_ushr_assign");
id::idop_eq_r_String = InternedStringPtr("op_eq_r_String");
id::idop_cat_r_String = InternedStringPtr("op_cat_r_String");
id::idnobreak = InternedStringPtr("nobreak");
id::idcurrent_context = InternedStringPtr("current_context");
id::idto_m = InternedStringPtr("to_m");
id::idreturn = InternedStringPtr("return");
id::idop_eq = InternedStringPtr("op_eq");
id::idfiber = InternedStringPtr("fiber");
id::idop_or_assign = InternedStringPtr("op_or_assign");
id::idserial_save = InternedStringPtr("serial_save");
id::idstatic = InternedStringPtr("static");
id::idswitch = InternedStringPtr("switch");
id::idop_sub = InternedStringPtr("op_sub");
id::idop_ushr = InternedStringPtr("op_ushr");
id::idfirst_step = InternedStringPtr("first_step");
id::idblock_break = InternedStringPtr("block_break");
id::id_dummy_fun_parameter_ = InternedStringPtr("_dummy_fun_parameter_");
id::id_dummy_block_parameter_ = InternedStringPtr("_dummy_block_parameter_");
id::idunittest = InternedStringPtr("unittest");
id::idop_xor = InternedStringPtr("op_xor");
id::idblock_first = InternedStringPtr("block_first");
id::idtrue = InternedStringPtr("true");
id::idop_call = InternedStringPtr("op_call");
id::id_initialize_ = InternedStringPtr("_initialize_");
id::idis = InternedStringPtr("is");
id::idwhile = InternedStringPtr("while");
id::idit = InternedStringPtr("it");
id::idassert = InternedStringPtr("assert");
id::idxtal = InternedStringPtr("xtal");
id::idif = InternedStringPtr("if");
id::idp = InternedStringPtr("p");
}
//}}ID}
























#else

void initialize_interned_string(){}

#endif









}
