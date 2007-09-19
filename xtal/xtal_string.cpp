
#include "xtal.h"

#include <string.h>

#include "xtal_string.h"
#include "xtal_macro.h"
#include "xtal_stack.h"
#include "xtal_peg.h"

#undef XTAL_USE_PEG

namespace xtal{

static uint_t make_hashcode(const char_t* str, uint_t size){
	uint_t hash = 2166136261;
	for(uint_t i=0; i<size; ++i){
		hash = hash*137 ^ str[i];
	}
	return hash;
}

static void make_hashcode_and_length(const char_t* str, uint_t size, uint_t& hash, uint_t& length){
	hash = 2166136261;
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
	hash = 2166136261;
	length = 0;
	size = 0;
	for(uint_t i=0; str[i]; ++i){
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
		size += len;
	}
}

#ifdef XTAL_USE_PEG

class StringScanIter : public Base{
	SmartPtr<StringStream> ss_;
	SmartPtr<peg::CharLexer> lex_;
	peg::ParserPtr patt_;
	ArrayPtr ret_;

	virtual void visit_members(Visitor& m){
		Base::visit_members(m);
		m & ss_ & lex_ & patt_ & ret_;
	}

public:

	StringScanIter(const StringPtr& str, const peg::ParserPtr& patt)
		:ss_(xnew<StringStream>(str)), lex_(xnew<peg::CharLexer>(ss_)), patt_(patt), ret_(xnew<Array>()){
	}
	
	AnyPtr reset(){
		ss_->seek(0);
		lex_ = xnew<peg::CharLexer>(ss_);
		ret_->clear();
		return SmartPtr<StringScanIter>::from_this(this);
	}

	void block_next(const VMachinePtr& vm){
		/*
		if(patt_->parse(lex_, ret_)){
			vm->return_result(SmartPtr<StringScanIter>::from_this(this), ret_->empty() ? null : ret_->front());
			ret_->clear();
		}else{
			vm->return_result(null);	
		}
		*/
	}
};

void InitString(){

	{
		ClassPtr p = new_cpp_class<StringScanIter>("StringScanIter");
		p->inherit(PseudoArray());
		p->method("reset", &StringScanIter::reset);
		p->method("block_first", &StringScanIter::block_next);
		p->method("block_next", &StringScanIter::block_next);
	}

	{
		ClassPtr p = new_cpp_class<String>("String");

		p->def("new", ctor<String>());
		p->method("to_i", &String::to_i);
		p->method("to_f", &String::to_f);
		p->method("to_s", &String::to_s);
		p->method("clone", &String::clone);

		//p->method("length", &String::length);
		//p->method("size", &String::size);
		p->method("intern", &String::intern);

		p->method("split", &String::split)->param("i", Named("n", 1));
		p->method("each", &String::each);
		p->method("replaced", &String::replaced);
		p->method("scan", &String::scan);

		p->method("op_cat", &String::op_cat);
		p->method("op_eq", &String::op_eq);
		p->method("op_lt", &String::op_lt);

		p->method("op_cat_r_String", &String::op_cat_r_String);
		p->method("op_eq_r_String", &String::op_eq_r_String);
		p->method("op_lt_r_String", &String::op_lt_r_String);
		
		p->method("op_cat_assign", &String::op_cat);
	}
}

#else

class StringSplit : public Base{
	StringPtr str_, sep_;
	uint_t index_;

	virtual void visit_members(Visitor& m){
		Base::visit_members(m);
		m & str_ & sep_;
	}

public:

	StringSplit(const StringPtr& str, const StringPtr& sep)
		:str_(str), sep_(sep), index_(0){
	}
	
	AnyPtr reset(){
		index_ = 0;
		return SmartPtr<StringSplit>::from_this(this);
	}


	void block_next(const VMachinePtr& vm){
		if(str_->buffer_size()<=index_){
			reset();
			vm->return_result(null);
			return;
		}
		const char* sep = sep_->c_str();
		const char* str = str_->c_str();
		uint_t sepsize = sep_->buffer_size();
		uint_t strsize = str_->buffer_size();

		if(sep[0]==0){
			int_t len = ch_len(str[index_]);
			vm->return_result(SmartPtr<StringSplit>::from_this(this), xnew<String>(&str[index_], len));
			index_ += len;
			return;
		}

		for(int_t j=index_, jsz=strsize; j<jsz; j+=ch_len(str[j])){
			for(int_t i=0, sz=sepsize+1; i<sz; ++i){
				if(sep[i]==0){
					vm->return_result(SmartPtr<StringSplit>::from_this(this), xnew<String>(&str[index_], j-index_));
					index_ = j+i;
					return;
				}
				if(str[j+i]==0){
					vm->return_result(SmartPtr<StringSplit>::from_this(this), xnew<String>(&str[index_], j-index_));
					index_ = j+i;
					return;
				}
				if(str[j+i]!=sep[i]){
					break;
				}
			}
		}
		vm->return_result(SmartPtr<StringSplit>::from_this(this), xnew<String>(&str[index_], strsize-index_));
		index_ = strsize;
	}
};




#endif

////////////////////////////////////////////////////////////////

class StringMgr : public GCObserver{
public:

	struct Node{
		uint_t hashcode;
		const char* str;
		uint_t size;
		StringPtr value;
		Node* next;

		Node()
			:value(null), next(0){}
	};


	StringMgr(){
		size_ = 0;
		begin_ = 0;
		used_size_ = 0;
		guard_ = 0;
		expand(753);
	}

	virtual ~StringMgr(){
		for(uint_t i = 0; i<size_; ++i){
			Node* p = begin_[i];
			while(p){
				Node* next = p->next;
				p->~Node();
				user_free(p);
				p = next;
			}
		}
		user_free(begin_);
	}
		
protected:

	float_t rate(){
		return used_size_/(float_t)size_;
	}
	
	void set_node(Node* node){
		Node** p = &begin_[node->hashcode % size_];
		while(*p){
			p = &(*p)->next;
		}
		*p = node;
	}

	void expand(int_t addsize){
		Node** oldbegin = begin_;
		uint_t oldsize = size_;

		size_ = size_ + size_ + addsize;
		begin_ = (Node**)user_malloc(sizeof(Node*)*size_);
		for(uint_t i = 0; i<size_; ++i){
			begin_[i] = 0;
		}

		for(uint_t i = 0; i<oldsize; ++i){
			Node* p = oldbegin[i];
			while(p){
				Node* next = p->next;
				set_node(p);
				p->next = 0;
				p = next;
			}
		}
		user_free(oldbegin);
	}
	
protected:

	Node** begin_;
	uint_t size_;
	uint_t used_size_;
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
		for(uint_t i = 0; i<size_; ++i){
			Node* p = begin_[i];
			while(p){
				Node* next = p->next;
				m & p->value;
				p = next;
			}
		}		
	}

public:

	const InternedStringPtr& insert(const char* str, uint_t size);
	const InternedStringPtr& insert(const char* str, uint_t size, uint_t hash, uint_t length);

	virtual void before_gc();
};

const InternedStringPtr& StringMgr::insert(const char* str, uint_t size){
	uint_t hashcode;
	uint_t length;
	make_hashcode_and_length(str, size, hashcode, length);
	return insert(str, size, hashcode, length);
}

const InternedStringPtr& StringMgr::insert(const char* str, uint_t size, uint_t hashcode, uint_t length){
	Guard guard(guard_);

	Node** p = &begin_[hashcode % size_];
	while(*p){
		if((*p)->size==size && memcmp((*p)->str, str, size)==0){
			return *(const InternedStringPtr*)&(*p)->value;
		}
		p = &(*p)->next;
	}

	*p = (Node*)user_malloc(sizeof(Node));
	new(*p) Node();
	
	(*p)->value = xnew<String>(str, size, hashcode, length, true);
	(*p)->hashcode = hashcode;
	(*p)->str = (*p)->value->c_str_direct();
	(*p)->size = size;

	used_size_++;
	if(rate()>0.5f){
		expand(17);

		p = &begin_[hashcode % size_];
		while(*p){
			if((*p)->size==size && memcmp((*p)->str, str, size)==0){
				return *(const InternedStringPtr*)&(*p)->value;
			}
			p = &(*p)->next;
		}
		return *(const InternedStringPtr*)&(*p)->value;
	}else{
		return *(const InternedStringPtr*)&(*p)->value;
	}
}

void StringMgr::before_gc(){
	return;

	if(guard_){
		return;
	}

	for(uint_t i = 0; i<size_; ++i){
		Node* p = begin_[i];
		Node* prev = 0;
		while(p){
			Node* next = p->next;
			if(pvalue(p->value)->ref_count()==1){
				p->~Node();
				user_free(p);
				used_size_--;
				if(prev){
					prev->next = next;
				}else{
					begin_[i] = next;
				}
			}else{
				prev = p;
			}
			p = next;
		}
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
		ClassPtr p = new_cpp_class<StringSplit>("StringSplit");
		p->inherit(Iterator());
		p->method("reset", &StringSplit::reset);
		p->method("block_first", &StringSplit::block_next);
		p->method("block_next", &StringSplit::block_next);
	}

	{
		ClassPtr p = new_cpp_class<String>("String");

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

		p->method("op_cat", &String::op_cat);
		p->method("op_eq", &String::op_eq);
		p->method("op_lt", &String::op_lt);

		p->method("op_cat_r_String", &String::op_cat_r_String);
		p->method("op_eq_r_String", &String::op_eq_r_String);
		p->method("op_lt_r_String", &String::op_lt_r_String);
		
		p->method("op_cat_assign", &String::op_cat);
	}
}

////////////////////////////////////////////////////////////////

void String::init_string(const char_t* str, uint_t sz){
	if(sz<SMALL_STRING_MAX){
		set_small_string();
		memcpy(svalue_, str, sz);
	}else{
		uint_t hash, length;
		make_hashcode_and_length(str, sz, hash, length);
		if(length<=1){
			set_p(pvalue(str_mgr_->insert(str, sz, hash, length)));
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
	if(sz<SMALL_STRING_MAX){
		set_small_string();
		memcpy(svalue_, str1, size1);
		memcpy(&svalue_[size1], str2, size2);
	}else{
		set_p(new LargeString(str1, size1, str2, size2));
		pvalue(*this)->set_class(new_cpp_class<String>());
		pvalue(*this)->dec_ref_count();
	}
}

String::String(const char* str, uint_t size, uint_t hashcode, uint_t length, bool intern_flag):Any(noinit_t()){
	uint_t sz = size;
	if(sz<SMALL_STRING_MAX){
		set_small_string();
		memcpy(svalue_, str, sz);
	}else{
		if(!intern_flag && length<=1){
			set_p(pvalue(str_mgr_->insert(str, sz, hashcode, length)));
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
		make_size_and_hashcode_and_length(svalue_, size, hash, length);
		return ((String*)&str_mgr_->insert(svalue_, size, hash, length))->svalue_;
	}
}

const char_t* String::c_str_direct(){
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
		return strlen(svalue_);
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
	return StringPtr::from_this(this);
}

const InternedStringPtr& String::intern(){
	if(type(*this)==TYPE_BASE){
		LargeString* p = ((LargeString*)pvalue(*this));
		if(p->is_interned()) return *(const InternedStringPtr*)this;
		return str_mgr_->insert(p->c_str(), p->buffer_size(), p->hashcode(), p->length());
	}else{
		return *(const InternedStringPtr*)this;
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
	return StringPtr::from_this(this);
}

int_t String::to_i(){ 
	return atoi(c_str()); 
}

float_t String::to_f(){ 
	return (float_t)atof(c_str()); 
}

#ifdef XTAL_USE_PEG

AnyPtr String::split(const AnyPtr& sep){
	using namespace peg;
	ParserPtr patt = to_parser(sep);
	return xnew<StringScanIter>(StringPtr::from_this(this), ~end() >> join((anych() - patt)*0) >> -(patt | end()));
}

AnyPtr String::each(){
	using namespace peg;
	return xnew<StringScanIter>(StringPtr::from_this(this), peg::anych());
}
	
AnyPtr String::scan(const AnyPtr& p){
	using namespace peg;
	ParserPtr patt = to_parser(p);
	return xnew<StringScanIter>(StringPtr::from_this(this), -(anych() - patt)*0 >> array(patt));
}

AnyPtr String::replaced(const AnyPtr& pattern, const StringPtr& str){
	using namespace peg;
	ParserPtr patt = to_parser(pattern);
	ParserPtr elem = ((anych() - patt)*0);
	ArrayPtr ret = xnew<Array>();
	join(elem >> (-patt >> val(str) >> elem)*0)->parse_string(StringPtr::from_this(this), ret);
	return ret->at(0);
}


#else

AnyPtr String::split(const AnyPtr& sep){
	return xnew<StringSplit>(StringPtr::from_this(this), sep->to_s());
}

AnyPtr String::each(){
	return xnew<StringSplit>(StringPtr::from_this(this), "");
}

#endif

StringPtr String::op_cat_String(const StringPtr& v){
	uint_t mysize = buffer_size();
	uint_t vsize = v->buffer_size();

	if(mysize+vsize <= 16 || mysize<SMALL_STRING_MAX || vsize<SMALL_STRING_MAX)
		return xnew<String>(c_str(), mysize, v->c_str(), vsize);
	return xnew<String>((LargeString*)pvalue(*this), (LargeString*)pvalue(v));
}

bool String::op_eq_r_String(const StringPtr& v){ 
	return v->buffer_size()==buffer_size() && memcmp(v->c_str(), c_str(), buffer_size())==0; 
}

bool String::op_lt_r_String(const StringPtr& v){ 
	return strcmp(v->c_str(), c_str())<0; 
}

StringPtr String::cat(const StringPtr& v){
	return op_cat_String(v);
}

StringPtr String::op_cat_r_String(const StringPtr& v){
	return v->op_cat_String(StringPtr::from_this(this));
}

void String::op_cat(const VMachinePtr& vm){
	AnyPtr a = vm->arg(0); 
	vm->recycle_call(StringPtr::from_this(this)); 
	a->rawsend(vm, Xid(op_cat_r_String));
}

void String::op_eq(const VMachinePtr& vm){
	AnyPtr a = vm->arg(0); 
	vm->recycle_call(StringPtr::from_this(this)); 
	a->rawsend(vm, Xid(op_eq_r_String));
	if(!vm->processed()){
		vm->return_result(null);
	}
}

void String::op_lt(const VMachinePtr& vm){
	AnyPtr a = vm->arg(0); 
	vm->recycle_call(StringPtr::from_this(this)); 
	a->rawsend(vm, Xid(op_lt_r_String));
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
	XTAL_ASSERT(size>=Innocence::SMALL_STRING_MAX);

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


InternedStringPtr::InternedStringPtr(const char* name)
	:StringPtr(!name ? StringPtr(null) : make(name, strlen(name))){}

InternedStringPtr::InternedStringPtr(const char* name, int_t size)
	:StringPtr(make(name, size)){}

InternedStringPtr::InternedStringPtr(const StringPtr& name)
	:StringPtr(!name ? StringPtr(null) : name->is_interned() ? name : str_mgr_->insert(name->c_str(), name->buffer_size())){}


StringPtr InternedStringPtr::make(const char* name, uint_t size){
	if(size<Innocence::SMALL_STRING_MAX){
		String temp(name, size);
		return StringPtr::from_this(&temp);
	}
	return str_mgr_->insert(name, size);
}


#ifdef XTAL_USE_PREDEFINED_ID

//{ID{{
namespace id{
InternedStringPtr idop_at;
InternedStringPtr idop_inc;
InternedStringPtr idcallee;
InternedStringPtr idnew;
InternedStringPtr idop_shl_assign;
InternedStringPtr idtest;
InternedStringPtr idfor;
InternedStringPtr idserial_new;
InternedStringPtr idop_div_assign;
InternedStringPtr idop_mul;
InternedStringPtr idop_xor_assign;
InternedStringPtr idto_a;
InternedStringPtr idinitialize;
InternedStringPtr idonce;
InternedStringPtr idexport;
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
InternedStringPtr idop_cat;
InternedStringPtr idop_neg;
InternedStringPtr idop_lt_r_String;
InternedStringPtr idop_shr_assign;
InternedStringPtr idop_dec;
InternedStringPtr idinstance_serial_save;
InternedStringPtr idvalue;
InternedStringPtr iddefault;
InternedStringPtr idcase;
InternedStringPtr idto_s;
InternedStringPtr idimport;
InternedStringPtr idop_shr;
InternedStringPtr idpure;
InternedStringPtr idfinally;
InternedStringPtr idserial_load;
InternedStringPtr idthis;
InternedStringPtr idnull;
InternedStringPtr idop_div;
InternedStringPtr idIOError;
InternedStringPtr id_dummy_lhs_parameter_;
InternedStringPtr idop_mul_assign;
InternedStringPtr idmethod;
InternedStringPtr idcatch;
InternedStringPtr idop_lt;
InternedStringPtr id_switch_;
InternedStringPtr idop_mod_assign;
InternedStringPtr idset_at;
InternedStringPtr idbreak;
InternedStringPtr idop_mod;
InternedStringPtr idto_i;
InternedStringPtr idop_or;
InternedStringPtr idtry;
InternedStringPtr idcontinue;
InternedStringPtr iditerator;
InternedStringPtr idop_and;
InternedStringPtr idelse;
InternedStringPtr idfun;
InternedStringPtr idto_f;
InternedStringPtr idop_sub_assign;
InternedStringPtr idthrow;
InternedStringPtr idlib;
InternedStringPtr iddofun;
InternedStringPtr ideach;
InternedStringPtr idop_set_at;
InternedStringPtr idop_com;
InternedStringPtr idop_pos;
InternedStringPtr idop_add;
InternedStringPtr idclass;
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
InternedStringPtr idop_ushr;
InternedStringPtr idserial_save;
InternedStringPtr idswitch;
InternedStringPtr idop_sub;
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
id::idop_at = null;
id::idop_inc = null;
id::idcallee = null;
id::idnew = null;
id::idop_shl_assign = null;
id::idtest = null;
id::idfor = null;
id::idserial_new = null;
id::idop_div_assign = null;
id::idop_mul = null;
id::idop_xor_assign = null;
id::idto_a = null;
id::idinitialize = null;
id::idonce = null;
id::idexport = null;
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
id::idop_cat = null;
id::idop_neg = null;
id::idop_lt_r_String = null;
id::idop_shr_assign = null;
id::idop_dec = null;
id::idinstance_serial_save = null;
id::idvalue = null;
id::iddefault = null;
id::idcase = null;
id::idto_s = null;
id::idimport = null;
id::idop_shr = null;
id::idpure = null;
id::idfinally = null;
id::idserial_load = null;
id::idthis = null;
id::idnull = null;
id::idop_div = null;
id::idIOError = null;
id::id_dummy_lhs_parameter_ = null;
id::idop_mul_assign = null;
id::idmethod = null;
id::idcatch = null;
id::idop_lt = null;
id::id_switch_ = null;
id::idop_mod_assign = null;
id::idset_at = null;
id::idbreak = null;
id::idop_mod = null;
id::idto_i = null;
id::idop_or = null;
id::idtry = null;
id::idcontinue = null;
id::iditerator = null;
id::idop_and = null;
id::idelse = null;
id::idfun = null;
id::idto_f = null;
id::idop_sub_assign = null;
id::idthrow = null;
id::idlib = null;
id::iddofun = null;
id::ideach = null;
id::idop_set_at = null;
id::idop_com = null;
id::idop_pos = null;
id::idop_add = null;
id::idclass = null;
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
id::idop_ushr = null;
id::idserial_save = null;
id::idswitch = null;
id::idop_sub = null;
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
id::idop_at = InternedStringPtr("op_at", 5);
id::idop_inc = InternedStringPtr("op_inc", 6);
id::idcallee = InternedStringPtr("callee", 6);
id::idnew = InternedStringPtr("new", 3);
id::idop_shl_assign = InternedStringPtr("op_shl_assign", 13);
id::idtest = InternedStringPtr("test", 4);
id::idfor = InternedStringPtr("for", 3);
id::idserial_new = InternedStringPtr("serial_new", 10);
id::idop_div_assign = InternedStringPtr("op_div_assign", 13);
id::idop_mul = InternedStringPtr("op_mul", 6);
id::idop_xor_assign = InternedStringPtr("op_xor_assign", 13);
id::idto_a = InternedStringPtr("to_a", 4);
id::idinitialize = InternedStringPtr("initialize", 10);
id::idonce = InternedStringPtr("once", 4);
id::idexport = InternedStringPtr("export", 6);
id::iddo = InternedStringPtr("do", 2);
id::idstring = InternedStringPtr("string", 6);
id::idfalse = InternedStringPtr("false", 5);
id::idop_and_assign = InternedStringPtr("op_and_assign", 13);
id::idop_add_assign = InternedStringPtr("op_add_assign", 13);
id::idop_cat_assign = InternedStringPtr("op_cat_assign", 13);
id::idsingleton = InternedStringPtr("singleton", 9);
id::idat = InternedStringPtr("at", 2);
id::idop_shl = InternedStringPtr("op_shl", 6);
id::idblock_next = InternedStringPtr("block_next", 10);
id::idyield = InternedStringPtr("yield", 5);
id::idop_cat = InternedStringPtr("op_cat", 6);
id::idop_neg = InternedStringPtr("op_neg", 6);
id::idop_lt_r_String = InternedStringPtr("op_lt_r_String", 14);
id::idop_shr_assign = InternedStringPtr("op_shr_assign", 13);
id::idop_dec = InternedStringPtr("op_dec", 6);
id::idinstance_serial_save = InternedStringPtr("instance_serial_save", 20);
id::idvalue = InternedStringPtr("value", 5);
id::iddefault = InternedStringPtr("default", 7);
id::idcase = InternedStringPtr("case", 4);
id::idto_s = InternedStringPtr("to_s", 4);
id::idimport = InternedStringPtr("import", 6);
id::idop_shr = InternedStringPtr("op_shr", 6);
id::idpure = InternedStringPtr("pure", 4);
id::idfinally = InternedStringPtr("finally", 7);
id::idserial_load = InternedStringPtr("serial_load", 11);
id::idthis = InternedStringPtr("this", 4);
id::idnull = InternedStringPtr("null", 4);
id::idop_div = InternedStringPtr("op_div", 6);
id::idIOError = InternedStringPtr("IOError", 7);
id::id_dummy_lhs_parameter_ = InternedStringPtr("_dummy_lhs_parameter_", 21);
id::idop_mul_assign = InternedStringPtr("op_mul_assign", 13);
id::idmethod = InternedStringPtr("method", 6);
id::idcatch = InternedStringPtr("catch", 5);
id::idop_lt = InternedStringPtr("op_lt", 5);
id::id_switch_ = InternedStringPtr("_switch_", 8);
id::idop_mod_assign = InternedStringPtr("op_mod_assign", 13);
id::idset_at = InternedStringPtr("set_at", 6);
id::idbreak = InternedStringPtr("break", 5);
id::idop_mod = InternedStringPtr("op_mod", 6);
id::idto_i = InternedStringPtr("to_i", 4);
id::idop_or = InternedStringPtr("op_or", 5);
id::idtry = InternedStringPtr("try", 3);
id::idcontinue = InternedStringPtr("continue", 8);
id::iditerator = InternedStringPtr("iterator", 8);
id::idop_and = InternedStringPtr("op_and", 6);
id::idelse = InternedStringPtr("else", 4);
id::idfun = InternedStringPtr("fun", 3);
id::idto_f = InternedStringPtr("to_f", 4);
id::idop_sub_assign = InternedStringPtr("op_sub_assign", 13);
id::idthrow = InternedStringPtr("throw", 5);
id::idlib = InternedStringPtr("lib", 3);
id::iddofun = InternedStringPtr("dofun", 5);
id::ideach = InternedStringPtr("each", 4);
id::idop_set_at = InternedStringPtr("op_set_at", 9);
id::idop_com = InternedStringPtr("op_com", 6);
id::idop_pos = InternedStringPtr("op_pos", 6);
id::idop_add = InternedStringPtr("op_add", 6);
id::idclass = InternedStringPtr("class", 5);
id::idop_ushr_assign = InternedStringPtr("op_ushr_assign", 14);
id::idop_eq_r_String = InternedStringPtr("op_eq_r_String", 14);
id::idop_cat_r_String = InternedStringPtr("op_cat_r_String", 15);
id::idnobreak = InternedStringPtr("nobreak", 7);
id::idcurrent_context = InternedStringPtr("current_context", 15);
id::idto_m = InternedStringPtr("to_m", 4);
id::idreturn = InternedStringPtr("return", 6);
id::idop_eq = InternedStringPtr("op_eq", 5);
id::idfiber = InternedStringPtr("fiber", 5);
id::idop_or_assign = InternedStringPtr("op_or_assign", 12);
id::idop_ushr = InternedStringPtr("op_ushr", 7);
id::idserial_save = InternedStringPtr("serial_save", 11);
id::idswitch = InternedStringPtr("switch", 6);
id::idop_sub = InternedStringPtr("op_sub", 6);
id::idblock_break = InternedStringPtr("block_break", 11);
id::id_dummy_fun_parameter_ = InternedStringPtr("_dummy_fun_parameter_", 21);
id::id_dummy_block_parameter_ = InternedStringPtr("_dummy_block_parameter_", 23);
id::idunittest = InternedStringPtr("unittest", 8);
id::idop_xor = InternedStringPtr("op_xor", 6);
id::idblock_first = InternedStringPtr("block_first", 11);
id::idtrue = InternedStringPtr("true", 4);
id::idop_call = InternedStringPtr("op_call", 7);
id::id_initialize_ = InternedStringPtr("_initialize_", 12);
id::idis = InternedStringPtr("is", 2);
id::idwhile = InternedStringPtr("while", 5);
id::idit = InternedStringPtr("it", 2);
id::idassert = InternedStringPtr("assert", 6);
id::idxtal = InternedStringPtr("xtal", 4);
id::idif = InternedStringPtr("if", 2);
id::idp = InternedStringPtr("p", 1);
}
//}}ID}
















#else

void initialize_interned_string(){}

#endif









}
