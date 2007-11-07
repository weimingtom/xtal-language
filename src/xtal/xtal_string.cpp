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

		vm->return_result(SmartPtr<StringEachIter>(this), ss_->get_s(1));
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
		vm->return_result(SmartPtr<ChRangeIter>(this), temp);
	}

private:

	virtual void visit_members(Visitor& m){
		Base::visit_members(m);
		m & it_ & end_;
	}

	StringPtr it_, end_;
};

AnyPtr ChRange::each(){
	return xnew<ChRangeIter>(ChRangePtr(this));
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
	}else{
		uint_t hash, length;
		make_hashcode_and_length(str, sz, hash, length);
		if(length<=1){
			set_p(pvalue(str_mgr_->insert(str, sz, hash, length).value));
		}else{
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
		register_gc(pvalue(*this));
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
			register_gc(pvalue(*this));
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
	register_gc(pvalue(*this));
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
	return StringPtr(this);
}

const IDPtr& String::intern(){
	if(type(*this)==TYPE_BASE){
		LargeString* p = ((LargeString*)pvalue(*this));
		if(p->is_interned()) return static_ptr_cast<ID>(ap(*this));
		return static_ptr_cast<ID>(str_mgr_->insert(p->c_str(), p->buffer_size(), p->hashcode(), p->length()).value);
	}else{
		return static_ptr_cast<ID>(ap(*this));
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
	return StringPtr(this);
}

int_t String::to_i(){ 
	return atoi(c_str()); 
}

float_t String::to_f(){ 
	return (float_t)atof(c_str()); 
}

AnyPtr String::each(){
	return xnew<StringEachIter>(StringPtr(this));
}

ChRangePtr String::op_range(const StringPtr& right, int_t kind){
	if(kind!=RANGE_CLOSED){
		XTAL_THROW(builtin()->member("RuntimeError")(Xt("Xtal Runtime Error 1025")), return xnew<ChRange>("", ""));		
	}

	if(length()==1 && right->length()==1){
		return xnew<ChRange>(StringPtr(this), right);
	}else{
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
	}else{
		*this = ID(&a, 1);
	}
}

ID::ID(char_t a, char_t b)
	:String(noinit_t()){
	if(2<=SMALL_STRING_MAX){
		set_small_string();
		svalue_[0] = a; svalue_[1] = b;
	}else{
		char_t buf[2] = {a, b};
		*this = ID(buf, 2);
	}
}

ID::ID(char_t a, char_t b, char_t c)
	:String(noinit_t()){
	if(3<=SMALL_STRING_MAX){
		set_small_string();
		svalue_[0] = a; svalue_[1] = b; svalue_[2] = c;
	}else{
		char_t buf[3] = {a, b, c};
		*this = ID(buf, 3);
	}
}

ID::ID(const StringPtr& name)
	:String(*(name ? name->intern() : (const IDPtr&)name)){}

AnyPtr SmartPtrCtor1<ID>::call(type v){
	return str_mgr_->insert(v).value;
}

AnyPtr SmartPtrCtor2<ID>::call(type v){
	if(v) return v->intern();
	return v;
}

#ifdef XTAL_USE_PREDEFINED_ID

//{ID{{
namespace id{
IDPtr idop_inc;
IDPtr idblock_catch;
IDPtr idcallee;
IDPtr idnew;
IDPtr idop_shl_assign;
IDPtr idop_at;
IDPtr idtest;
IDPtr idfor;
IDPtr idserial_new;
IDPtr idop_div_assign;
IDPtr idop_mul;
IDPtr idop_xor_assign;
IDPtr idto_a;
IDPtr idinitialize;
IDPtr idonce;
IDPtr iddo;
IDPtr idstring;
IDPtr idfalse;
IDPtr idancestors;
IDPtr idop_and_assign;
IDPtr idop_add_assign;
IDPtr idop_cat_assign;
IDPtr idsingleton;
IDPtr idop_shl;
IDPtr idblock_next;
IDPtr idyield;
IDPtr idop_shr_assign;
IDPtr idop_cat;
IDPtr idop_neg;
IDPtr idop_dec;
IDPtr idvalue;
IDPtr iddefault;
IDPtr idcase;
IDPtr idto_s;
IDPtr idop_shr;
IDPtr idpure;
IDPtr idfinally;
IDPtr idthis;
IDPtr idnull;
IDPtr idop_div;
IDPtr idserial_load;
IDPtr idIOError;
IDPtr id_dummy_lhs_parameter_;
IDPtr idin;
IDPtr idcatch;
IDPtr idop_mul_assign;
IDPtr idmethod;
IDPtr idop_lt;
IDPtr idset_at;
IDPtr id_switch_;
IDPtr idop_mod_assign;
IDPtr idbreak;
IDPtr idtry;
IDPtr idop_mod;
IDPtr idundefined;
IDPtr idto_i;
IDPtr idop_or;
IDPtr idcontinue;
IDPtr ide;
IDPtr iditerator;
IDPtr idthrow;
IDPtr idop_and;
IDPtr idelse;
IDPtr idfun;
IDPtr idto_f;
IDPtr idop_sub_assign;
IDPtr idlib;
IDPtr iddofun;
IDPtr ideach;
IDPtr idop_set_at;
IDPtr idop_in;
IDPtr ids_load;
IDPtr idclass;
IDPtr idop_com;
IDPtr idop_pos;
IDPtr idop_add;
IDPtr idop_ushr_assign;
IDPtr idnobreak;
IDPtr idcurrent_context;
IDPtr idto_m;
IDPtr idreturn;
IDPtr idop_eq;
IDPtr idfiber;
IDPtr idop_or_assign;
IDPtr ids_save;
IDPtr idstatic;
IDPtr idswitch;
IDPtr idop_sub;
IDPtr idop_ushr;
IDPtr idfirst_step;
IDPtr idblock_break;
IDPtr idserial_save;
IDPtr idop_range;
IDPtr id_dummy_fun_parameter_;
IDPtr id_dummy_block_parameter_;
IDPtr idunittest;
IDPtr idop_xor;
IDPtr idblock_first;
IDPtr idtrue;
IDPtr idop_call;
IDPtr id_initialize_;
IDPtr idis;
IDPtr idwhile;
IDPtr idit;
IDPtr idassert;
IDPtr idxtal;
IDPtr idif;
IDPtr idp;
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
id::idancestors = null;
id::idop_and_assign = null;
id::idop_add_assign = null;
id::idop_cat_assign = null;
id::idsingleton = null;
id::idop_shl = null;
id::idblock_next = null;
id::idyield = null;
id::idop_shr_assign = null;
id::idop_cat = null;
id::idop_neg = null;
id::idop_dec = null;
id::idvalue = null;
id::iddefault = null;
id::idcase = null;
id::idto_s = null;
id::idop_shr = null;
id::idpure = null;
id::idfinally = null;
id::idthis = null;
id::idnull = null;
id::idop_div = null;
id::idserial_load = null;
id::idIOError = null;
id::id_dummy_lhs_parameter_ = null;
id::idin = null;
id::idcatch = null;
id::idop_mul_assign = null;
id::idmethod = null;
id::idop_lt = null;
id::idset_at = null;
id::id_switch_ = null;
id::idop_mod_assign = null;
id::idbreak = null;
id::idtry = null;
id::idop_mod = null;
id::idundefined = null;
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
id::idop_in = null;
id::ids_load = null;
id::idclass = null;
id::idop_com = null;
id::idop_pos = null;
id::idop_add = null;
id::idop_ushr_assign = null;
id::idnobreak = null;
id::idcurrent_context = null;
id::idto_m = null;
id::idreturn = null;
id::idop_eq = null;
id::idfiber = null;
id::idop_or_assign = null;
id::ids_save = null;
id::idstatic = null;
id::idswitch = null;
id::idop_sub = null;
id::idop_ushr = null;
id::idfirst_step = null;
id::idblock_break = null;
id::idserial_save = null;
id::idop_range = null;
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
id::idop_inc = IDPtr("op_inc");
id::idblock_catch = IDPtr("block_catch");
id::idcallee = IDPtr("callee");
id::idnew = IDPtr("new");
id::idop_shl_assign = IDPtr("op_shl_assign");
id::idop_at = IDPtr("op_at");
id::idtest = IDPtr("test");
id::idfor = IDPtr("for");
id::idserial_new = IDPtr("serial_new");
id::idop_div_assign = IDPtr("op_div_assign");
id::idop_mul = IDPtr("op_mul");
id::idop_xor_assign = IDPtr("op_xor_assign");
id::idto_a = IDPtr("to_a");
id::idinitialize = IDPtr("initialize");
id::idonce = IDPtr("once");
id::iddo = IDPtr("do");
id::idstring = IDPtr("string");
id::idfalse = IDPtr("false");
id::idancestors = IDPtr("ancestors");
id::idop_and_assign = IDPtr("op_and_assign");
id::idop_add_assign = IDPtr("op_add_assign");
id::idop_cat_assign = IDPtr("op_cat_assign");
id::idsingleton = IDPtr("singleton");
id::idop_shl = IDPtr("op_shl");
id::idblock_next = IDPtr("block_next");
id::idyield = IDPtr("yield");
id::idop_shr_assign = IDPtr("op_shr_assign");
id::idop_cat = IDPtr("op_cat");
id::idop_neg = IDPtr("op_neg");
id::idop_dec = IDPtr("op_dec");
id::idvalue = IDPtr("value");
id::iddefault = IDPtr("default");
id::idcase = IDPtr("case");
id::idto_s = IDPtr("to_s");
id::idop_shr = IDPtr("op_shr");
id::idpure = IDPtr("pure");
id::idfinally = IDPtr("finally");
id::idthis = IDPtr("this");
id::idnull = IDPtr("null");
id::idop_div = IDPtr("op_div");
id::idserial_load = IDPtr("serial_load");
id::idIOError = IDPtr("IOError");
id::id_dummy_lhs_parameter_ = IDPtr("_dummy_lhs_parameter_");
id::idin = IDPtr("in");
id::idcatch = IDPtr("catch");
id::idop_mul_assign = IDPtr("op_mul_assign");
id::idmethod = IDPtr("method");
id::idop_lt = IDPtr("op_lt");
id::idset_at = IDPtr("set_at");
id::id_switch_ = IDPtr("_switch_");
id::idop_mod_assign = IDPtr("op_mod_assign");
id::idbreak = IDPtr("break");
id::idtry = IDPtr("try");
id::idop_mod = IDPtr("op_mod");
id::idundefined = IDPtr("undefined");
id::idto_i = IDPtr("to_i");
id::idop_or = IDPtr("op_or");
id::idcontinue = IDPtr("continue");
id::ide = IDPtr("e");
id::iditerator = IDPtr("iterator");
id::idthrow = IDPtr("throw");
id::idop_and = IDPtr("op_and");
id::idelse = IDPtr("else");
id::idfun = IDPtr("fun");
id::idto_f = IDPtr("to_f");
id::idop_sub_assign = IDPtr("op_sub_assign");
id::idlib = IDPtr("lib");
id::iddofun = IDPtr("dofun");
id::ideach = IDPtr("each");
id::idop_set_at = IDPtr("op_set_at");
id::idop_in = IDPtr("op_in");
id::ids_load = IDPtr("s_load");
id::idclass = IDPtr("class");
id::idop_com = IDPtr("op_com");
id::idop_pos = IDPtr("op_pos");
id::idop_add = IDPtr("op_add");
id::idop_ushr_assign = IDPtr("op_ushr_assign");
id::idnobreak = IDPtr("nobreak");
id::idcurrent_context = IDPtr("current_context");
id::idto_m = IDPtr("to_m");
id::idreturn = IDPtr("return");
id::idop_eq = IDPtr("op_eq");
id::idfiber = IDPtr("fiber");
id::idop_or_assign = IDPtr("op_or_assign");
id::ids_save = IDPtr("s_save");
id::idstatic = IDPtr("static");
id::idswitch = IDPtr("switch");
id::idop_sub = IDPtr("op_sub");
id::idop_ushr = IDPtr("op_ushr");
id::idfirst_step = IDPtr("first_step");
id::idblock_break = IDPtr("block_break");
id::idserial_save = IDPtr("serial_save");
id::idop_range = IDPtr("op_range");
id::id_dummy_fun_parameter_ = IDPtr("_dummy_fun_parameter_");
id::id_dummy_block_parameter_ = IDPtr("_dummy_block_parameter_");
id::idunittest = IDPtr("unittest");
id::idop_xor = IDPtr("op_xor");
id::idblock_first = IDPtr("block_first");
id::idtrue = IDPtr("true");
id::idop_call = IDPtr("op_call");
id::id_initialize_ = IDPtr("_initialize_");
id::idis = IDPtr("is");
id::idwhile = IDPtr("while");
id::idit = IDPtr("it");
id::idassert = IDPtr("assert");
id::idxtal = IDPtr("xtal");
id::idif = IDPtr("if");
id::idp = IDPtr("p");
}
//}}ID}































#else

void initialize_interned_string(){}

#endif









}
