#include "xtal.h"
#include "xtal_macro.h"

#include <string.h>

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

	ChMaker chm;

	uint_t i=0;
	while(i<size){
		chm.clear();
		while(!chm.is_completed()){
			if(i<size){ chm.add(str[i++]); } 
			else{ break; }
		}
	
		for(int_t j=0; j<chm.pos; ++j){
			hash = hash*137 ^ chm.buf[j];
		}

		length += 1;
	}
}

static void make_size_and_hashcode_and_length(const char_t* str, uint_t& size, uint_t& hash, uint_t& length){
	hash = 2166136261U;
	length = 0;
	size = 0;

	ChMaker chm;

	uint_t i=0;
	while(str[i]){
		chm.clear();
		while(!chm.is_completed()){
			if(str[i]){ chm.add(str[i++]); } 
			else{ break; }
		}
	
		for(int_t j=0; j<chm.pos; ++j){
			hash = hash*137 ^ chm.buf[j];
		}

		length += 1;
		size += chm.pos;
	}
}

static void make_size_and_hashcode_and_length_limit(const char_t* str, uint_t& size, uint_t& hash, uint_t& length){
	hash = 2166136261U;
	length = 0;
	size = 0;

	ChMaker chm;

	uint_t i=0;
	while(str[i] && i<Innocence::SMALL_STRING_MAX){
		chm.clear();
		while(!chm.is_completed()){
			if(str[i] && i<Innocence::SMALL_STRING_MAX){ chm.add(str[i++]); } 
			else{ break; }
		}
	
		for(int_t j=0; j<chm.pos; ++j){
			hash = hash*137 ^ chm.buf[j];
		}

		length += 1;
		size += chm.pos;
	}
}

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
		if(ss_->eos()){
			vm->return_result(null, null);
			return;
		}

		vm->return_result(from_this(this), ss_->get_s(1));
	}
};

class ChRangeIter : public Base{
public:

	ChRangeIter(const ChRangePtr& range)
		:it_(range->left()), end_(range->right()){}

	void block_next(const VMachinePtr& vm){
		if(ch_cmp(it_->data(), it_->buffer_size(), end_->data(), end_->buffer_size())>0){
			vm->return_result(null, null);
			return;
		}

		StringPtr temp = it_;
		it_ = ch_inc(it_->data(), it_->buffer_size());
		vm->return_result(from_this(this), temp);
	}

private:

	virtual void visit_members(Visitor& m){
		Base::visit_members(m);
		m & it_ & end_;
	}

	StringPtr it_, end_;
};

AnyPtr ChRange::each(){
	return xnew<ChRangeIter>(from_this(this));
}

////////////////////////////////////////////////////////////////

class StringMgr : public GCObserver{
public:

	struct Key{
		const char_t* str;
		uint_t size;
	};

	struct Value{
		uint_t hashcode;
		IDPtr value;
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

	Value value = {hashcode, xnew<ID>(str, size, hashcode, length)};
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
		ClassPtr p = new_cpp_class<StringEachIter>("StringEachIter");
		p->inherit(Iterator());
		p->method("block_next", &StringEachIter::block_next);
	}

	{
		ClassPtr p = new_cpp_class<ChRangeIter>("ChRangeIter");
		p->inherit(Iterator());
		p->method("block_next", &ChRangeIter::block_next);
	}

	{
		ClassPtr p = new_cpp_class<ChRange>("ChRange");
		p->inherit(Iterable());
		p->def("new", ctor<ChRange, const StringPtr&, const StringPtr&>()->param(Named("left", null), Named("right", null)));
		p->method("left", &ChRange::left);
		p->method("right", &ChRange::right);
		p->method("each", &ChRange::each);
	}


	{
		ClassPtr p = new_cpp_class<String>("String");
		p->inherit(Iterable());

		p->def("new", ctor<String>());
		p->method("to_i", &String::to_i);
		p->method("to_f", &String::to_f);
		p->method("to_s", &String::to_s);
		p->method("clone", &String::clone);

		p->method("length", &String::length);
		p->method("size", &String::size);
		p->method("intern", &String::intern);

		p->method("each", &String::each);

		p->method("op_range", &String::op_range, get_cpp_class<String>());
		p->method("op_cat", &String::op_cat, get_cpp_class<String>());
		p->method("op_cat_assign", &String::op_cat, get_cpp_class<String>());
		p->method("op_eq", &String::op_eq, get_cpp_class<String>());
		p->method("op_lt", &String::op_lt, get_cpp_class<String>());
	}

	set_cpp_class<ID>(get_cpp_class<String>());
}

////////////////////////////////////////////////////////////////

void String::init_string(const char_t* str, uint_t sz){
	if(sz<=SMALL_STRING_MAX){
		set_small_string();
		memcpy(svalue_, str, sz);
	}
	else{
		uint_t hash, length;
		make_hashcode_and_length(str, sz, hash, length);
		if(length<=1){
			set_p(pvalue(str_mgr_->insert(str, sz, hash, length).value));
			pvalue(*this)->inc_ref_count();
		}
		else{
			set_p(new LargeString(str, sz, hash, length));
			pvalue(*this)->set_class(new_cpp_class<String>());
			register_gc(pvalue(*this));
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
	}
	else if(size2==0){
		init_string(str1, size1);
		return;
	}

	uint_t sz = size1 + size2;
	if(sz<=SMALL_STRING_MAX){
		set_small_string();
		memcpy(svalue_, str1, size1);
		memcpy(&svalue_[size1], str2, size2);
	}
	else{
		set_p(new LargeString(str1, size1, str2, size2));
		pvalue(*this)->set_class(new_cpp_class<String>());
		register_gc(pvalue(*this));
	}
}

String::String(char_t a)
	:Any(noinit_t()){
	if(1<=SMALL_STRING_MAX){
		set_small_string();
		svalue_[0] = a;
	}
	else{
		init_string(&a, 1);
	}
}

String::String(char_t a, char_t b)
	:Any(noinit_t()){
	if(2<=SMALL_STRING_MAX){
		set_small_string();
		svalue_[0] = a; svalue_[1] = b;
	}
	else{
		char_t buf[2] = {a, b};
		init_string(buf, 2);
	}
}

String::String(char_t a, char_t b, char_t c)
	:Any(noinit_t()){
	if(3<=SMALL_STRING_MAX){
		set_small_string();
		svalue_[0] = a; svalue_[1] = b; svalue_[2] = c;
	}
	else{
		char_t buf[3] = {a, b, c};
		init_string(buf, 3);
	}
}

String::String(const char* str, uint_t size, uint_t hashcode, uint_t length, bool intern_flag):Any(noinit_t()){
	uint_t sz = size;
	if(sz<=SMALL_STRING_MAX){
		set_small_string();
		memcpy(svalue_, str, sz);
	}
	else{
		if(!intern_flag && length<=1){
			set_p(pvalue(str_mgr_->insert(str, sz, hashcode, length).value));
			pvalue(*this)->inc_ref_count();
		}
		else{
			set_p(new LargeString(str, sz, hashcode, length, intern_flag));
			pvalue(*this)->set_class(new_cpp_class<String>());
			register_gc(pvalue(*this));
		}
	}
}

String::String(LargeString* left, LargeString* right):Any(noinit_t()){
	if(left->buffer_size()==0){
		init_string(right->c_str(), right->buffer_size());
		return;
	}
	else if(right->buffer_size()==0){
		init_string(left->c_str(), left->buffer_size());
		return;
	}

	set_p(new LargeString(left, right));
	pvalue(*this)->set_class(new_cpp_class<String>());
	register_gc(pvalue(*this));
}

String::String(const String& s)
	:Any(s){
	inc_ref_count_force(s);
}

const char* String::c_str(){
	if(type(*this)==TYPE_BASE){
		return ((LargeString*)pvalue(*this))->c_str();
	}
	else{
		uint_t size, hash, length;
		make_size_and_hashcode_and_length_limit(svalue_, size, hash, length);
		return str_mgr_->insert(svalue_, size, hash, length).buf;
	}
}

const char_t* String::data(){
	if(type(*this)==TYPE_BASE){
		return ((LargeString*)pvalue(*this))->c_str();
	}
	else{
		return svalue_;
	}
}

uint_t String::buffer_size(){
	if(type(*this)==TYPE_BASE){
		return ((LargeString*)pvalue(*this))->buffer_size();
	}
	else{
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
	}
	else{
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

const IDPtr& String::intern(){
	if(type(*this)==TYPE_BASE){
		LargeString* p = ((LargeString*)pvalue(*this));
		if(p->is_interned()) return static_ptr_cast<ID>(ap(*this));
		return static_ptr_cast<ID>(str_mgr_->insert(p->c_str(), p->buffer_size(), p->hashcode(), p->length()).value);
	}
	else{
		return static_ptr_cast<ID>(ap(*this));
	}
}

bool String::is_interned(){
	if(type(*this)==TYPE_BASE){
		return ((LargeString*)pvalue(*this))->is_interned();
	}
	else{
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

AnyPtr String::each(){
	return xnew<StringEachIter>(from_this(this));
}

ChRangePtr String::op_range(const StringPtr& right, int_t kind){
	if(kind!=RANGE_CLOSED){
		XTAL_THROW(builtin()->member("RuntimeError")(Xt("Xtal Runtime Error 1025")), return xnew<ChRange>("", ""));		
	}

	if(length()==1 && right->length()==1){
		return xnew<ChRange>(from_this(this), right);
	}
	else{
		XTAL_THROW(builtin()->member("RuntimeError")(Xt("Xtal Runtime Error 1023")), return xnew<ChRange>("", ""));		
	}
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
	return strcmp(c_str(), v->c_str()) < 0;
}

StringPtr String::cat(const StringPtr& v){
	return op_cat(v);
}

uint_t String::hashcode(){
	if(type(*this)==TYPE_BASE){
		return ((LargeString*)pvalue(*this))->hashcode();
	}
	else{
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
	}
	else{
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
	}
	else{
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
		}
		else{
			stack.push(p->rope_.right);
			p = p->rope_.left;
		}
	}
}



ID::ID(const char_t* str)
	:String(*str_mgr_->insert(str).value){}

ID::ID(const string_t& str)
	:String(*str_mgr_->insert(str.c_str(), str.size()).value){}

ID::ID(const char_t* str, uint_t size)
	:String(*str_mgr_->insert(str, size).value){}

ID::ID(const char_t* begin, const char_t* last)
	:String(*str_mgr_->insert(begin, last-begin).value){}

ID::ID(char_t a)
	:String(noinit_t()){
	if(1<=SMALL_STRING_MAX){
		set_small_string();
		svalue_[0] = a;
	}
	else{
		*this = ID(&a, 1);
	}
}

ID::ID(char_t a, char_t b)
	:String(noinit_t()){
	if(2<=SMALL_STRING_MAX){
		set_small_string();
		svalue_[0] = a; svalue_[1] = b;
	}
	else{
		char_t buf[2] = {a, b};
		*this = ID(buf, 2);
	}
}

ID::ID(char_t a, char_t b, char_t c)
	:String(noinit_t()){
	if(3<=SMALL_STRING_MAX){
		set_small_string();
		svalue_[0] = a; svalue_[1] = b; svalue_[2] = c;
	}
	else{
		char_t buf[3] = {a, b, c};
		*this = ID(buf, 3);
	}
}

ID::ID(const char_t* str, uint_t len, uint_t hashcode, uint_t length)
	:String(str, len, hashcode, length, true){}


ID::ID(const StringPtr& name)
	:String(*(name ? name->intern() : (const IDPtr&)name)){}


AnyPtr SmartPtrCtor1<String>::call(type v){
	return xnew<String>(v);
}

AnyPtr SmartPtrCtor1<ID>::call(type v){
	return str_mgr_->insert(v).value;
}

AnyPtr SmartPtrCtor2<ID>::call(type v){
	if(v) return v->intern();
	return v;
}

#ifdef XTAL_USE_PREDEFINED_ID

namespace id{
	IDPtr id_list[IDMAX];
}

void uninitialize_interned_string(){
	for(int i=0; i<id::IDMAX; ++i){
		id::id_list[i] = null;
	}
}


void initialize_interned_string(){
	register_uninitializer(&uninitialize_interned_string);

//{ID{{
	id::id_list[id::idop_inc] = xnew<ID>("op_inc", 6);
	id::id_list[id::idblock_catch] = xnew<ID>("block_catch", 11);
	id::id_list[id::idcallee] = xnew<ID>("callee", 6);
	id::id_list[id::idnew] = xnew<ID>("new", 3);
	id::id_list[id::idop_shl_assign] = xnew<ID>("op_shl_assign", 13);
	id::id_list[id::idop_at] = xnew<ID>("op_at", 5);
	id::id_list[id::idtest] = xnew<ID>("test", 4);
	id::id_list[id::idfor] = xnew<ID>("for", 3);
	id::id_list[id::idserial_new] = xnew<ID>("serial_new", 10);
	id::id_list[id::idop_div_assign] = xnew<ID>("op_div_assign", 13);
	id::id_list[id::idop_mul] = xnew<ID>("op_mul", 6);
	id::id_list[id::idop_xor_assign] = xnew<ID>("op_xor_assign", 13);
	id::id_list[id::idto_a] = xnew<ID>("to_a", 4);
	id::id_list[id::idinitialize] = xnew<ID>("initialize", 10);
	id::id_list[id::idonce] = xnew<ID>("once", 4);
	id::id_list[id::iddo] = xnew<ID>("do", 2);
	id::id_list[id::idstring] = xnew<ID>("string", 6);
	id::id_list[id::idfalse] = xnew<ID>("false", 5);
	id::id_list[id::idancestors] = xnew<ID>("ancestors", 9);
	id::id_list[id::idop_and_assign] = xnew<ID>("op_and_assign", 13);
	id::id_list[id::idop_add_assign] = xnew<ID>("op_add_assign", 13);
	id::id_list[id::idop_cat_assign] = xnew<ID>("op_cat_assign", 13);
	id::id_list[id::idsingleton] = xnew<ID>("singleton", 9);
	id::id_list[id::idop_shl] = xnew<ID>("op_shl", 6);
	id::id_list[id::idblock_next] = xnew<ID>("block_next", 10);
	id::id_list[id::idyield] = xnew<ID>("yield", 5);
	id::id_list[id::idop_shr_assign] = xnew<ID>("op_shr_assign", 13);
	id::id_list[id::idop_cat] = xnew<ID>("op_cat", 6);
	id::id_list[id::idop_neg] = xnew<ID>("op_neg", 6);
	id::id_list[id::idop_dec] = xnew<ID>("op_dec", 6);
	id::id_list[id::idvalue] = xnew<ID>("value", 5);
	id::id_list[id::iddefault] = xnew<ID>("default", 7);
	id::id_list[id::idcase] = xnew<ID>("case", 4);
	id::id_list[id::idto_s] = xnew<ID>("to_s", 4);
	id::id_list[id::idop_shr] = xnew<ID>("op_shr", 6);
	id::id_list[id::idpure] = xnew<ID>("pure", 4);
	id::id_list[id::idfinally] = xnew<ID>("finally", 7);
	id::id_list[id::idthis] = xnew<ID>("this", 4);
	id::id_list[id::idnull] = xnew<ID>("null", 4);
	id::id_list[id::idop_div] = xnew<ID>("op_div", 6);
	id::id_list[id::idserial_load] = xnew<ID>("serial_load", 11);
	id::id_list[id::idIOError] = xnew<ID>("IOError", 7);
	id::id_list[id::id_dummy_lhs_parameter_] = xnew<ID>("_dummy_lhs_parameter_", 21);
	id::id_list[id::idin] = xnew<ID>("in", 2);
	id::id_list[id::idcatch] = xnew<ID>("catch", 5);
	id::id_list[id::idop_mul_assign] = xnew<ID>("op_mul_assign", 13);
	id::id_list[id::idmethod] = xnew<ID>("method", 6);
	id::id_list[id::idop_lt] = xnew<ID>("op_lt", 5);
	id::id_list[id::idset_at] = xnew<ID>("set_at", 6);
	id::id_list[id::id_switch_] = xnew<ID>("_switch_", 8);
	id::id_list[id::idop_mod_assign] = xnew<ID>("op_mod_assign", 13);
	id::id_list[id::idbreak] = xnew<ID>("break", 5);
	id::id_list[id::idtry] = xnew<ID>("try", 3);
	id::id_list[id::idop_mod] = xnew<ID>("op_mod", 6);
	id::id_list[id::idto_i] = xnew<ID>("to_i", 4);
	id::id_list[id::idop_or] = xnew<ID>("op_or", 5);
	id::id_list[id::idcontinue] = xnew<ID>("continue", 8);
	id::id_list[id::ide] = xnew<ID>("e", 1);
	id::id_list[id::iditerator] = xnew<ID>("iterator", 8);
	id::id_list[id::idthrow] = xnew<ID>("throw", 5);
	id::id_list[id::idop_and] = xnew<ID>("op_and", 6);
	id::id_list[id::idundefined] = xnew<ID>("undefined", 9);
	id::id_list[id::idelse] = xnew<ID>("else", 4);
	id::id_list[id::idfun] = xnew<ID>("fun", 3);
	id::id_list[id::idto_f] = xnew<ID>("to_f", 4);
	id::id_list[id::idop_sub_assign] = xnew<ID>("op_sub_assign", 13);
	id::id_list[id::idlib] = xnew<ID>("lib", 3);
	id::id_list[id::iddofun] = xnew<ID>("dofun", 5);
	id::id_list[id::ideach] = xnew<ID>("each", 4);
	id::id_list[id::idop_set_at] = xnew<ID>("op_set_at", 9);
	id::id_list[id::idop_in] = xnew<ID>("op_in", 5);
	id::id_list[id::ids_load] = xnew<ID>("s_load", 6);
	id::id_list[id::idclass] = xnew<ID>("class", 5);
	id::id_list[id::idop_com] = xnew<ID>("op_com", 6);
	id::id_list[id::idop_pos] = xnew<ID>("op_pos", 6);
	id::id_list[id::idop_add] = xnew<ID>("op_add", 6);
	id::id_list[id::idop_ushr_assign] = xnew<ID>("op_ushr_assign", 14);
	id::id_list[id::idnobreak] = xnew<ID>("nobreak", 7);
	id::id_list[id::idcurrent_context] = xnew<ID>("current_context", 15);
	id::id_list[id::idto_m] = xnew<ID>("to_m", 4);
	id::id_list[id::idreturn] = xnew<ID>("return", 6);
	id::id_list[id::idop_eq] = xnew<ID>("op_eq", 5);
	id::id_list[id::idfiber] = xnew<ID>("fiber", 5);
	id::id_list[id::idop_or_assign] = xnew<ID>("op_or_assign", 12);
	id::id_list[id::ids_save] = xnew<ID>("s_save", 6);
	id::id_list[id::idswitch] = xnew<ID>("switch", 6);
	id::id_list[id::idop_sub] = xnew<ID>("op_sub", 6);
	id::id_list[id::idop_ushr] = xnew<ID>("op_ushr", 7);
	id::id_list[id::idfirst_step] = xnew<ID>("first_step", 10);
	id::id_list[id::idblock_break] = xnew<ID>("block_break", 11);
	id::id_list[id::idserial_save] = xnew<ID>("serial_save", 11);
	id::id_list[id::idop_range] = xnew<ID>("op_range", 8);
	id::id_list[id::id_dummy_fun_parameter_] = xnew<ID>("_dummy_fun_parameter_", 21);
	id::id_list[id::id_dummy_block_parameter_] = xnew<ID>("_dummy_block_parameter_", 23);
	id::id_list[id::idunittest] = xnew<ID>("unittest", 8);
	id::id_list[id::idop_xor] = xnew<ID>("op_xor", 6);
	id::id_list[id::idblock_first] = xnew<ID>("block_first", 11);
	id::id_list[id::idtrue] = xnew<ID>("true", 4);
	id::id_list[id::idop_call] = xnew<ID>("op_call", 7);
	id::id_list[id::id_initialize_] = xnew<ID>("_initialize_", 12);
	id::id_list[id::idis] = xnew<ID>("is", 2);
	id::id_list[id::idwhile] = xnew<ID>("while", 5);
	id::id_list[id::idit] = xnew<ID>("it", 2);
	id::id_list[id::idassert] = xnew<ID>("assert", 6);
	id::id_list[id::idxtal] = xnew<ID>("xtal", 4);
	id::id_list[id::idif] = xnew<ID>("if", 2);
	id::id_list[id::idp] = xnew<ID>("p", 1);
//}}ID}


}































#else

void initialize_interned_string(){}

#endif









}
