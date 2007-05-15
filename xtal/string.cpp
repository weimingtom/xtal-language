
#include <string.h>

#include "xtal.h"
#include "stringimpl.h"
#include "xmacro.h"

namespace xtal{

uint_t make_hashcode(const char* str, int_t size){
	uint_t value = 3;
	for(int_t i = 0; i<size; i++){
		value = value*137 + str[i] + (value>>16);
	}
	return value;
}

static void InitStringSplitImpl(){
	TClass<StringImpl::StringSplitImpl> p("StringSplit");
	p.inherit(Iterator());
	p.method("iter_first", &StringImpl::StringSplitImpl::iter_first);
	p.method("iter_next", &StringImpl::StringSplitImpl::iter_next);
}

void InitString(){
	TClass<String> p("String");
	
	InitStringSplitImpl();

	p.def("new", New<String>());
	p.method("to_i", &String::to_i);
	p.method("to_f", &String::to_f);
	p.method("to_s", &String::to_s);
	p.method("clone", &String::clone);

	p.method("length", &String::length);
	p.method("size", &String::size);
	p.method("slice", &String::slice);
	p.method("intern", &String::intern);

	p.method("split", &String::split);

	p.method("op_cat", &String::op_cat);
	p.method("op_eq", &String::op_eq);
	p.method("op_lt", &String::op_lt);

	p.method("op_cat_r_String", &String::op_cat_r_String);
	p.method("op_eq_r_String", &String::op_eq_r_String);
	p.method("op_lt_r_String", &String::op_lt_r_String);
	
	p.method("op_cat_assign", &String::op_cat);
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

class StringMgrImpl : public GCObserverImpl{
public:

	struct Node{
		uint_t hashcode;
		const char* str;
		uint_t size;
		String value;
		Node* next;

		Node()
			:next(0), value(null){}
	};


	StringMgrImpl(){
		size_ = 0;
		begin_ = 0;
		used_size_ = 0;
		guard_ = 0;
		expand(7);
	}

	virtual ~StringMgrImpl(){
		for(uint_t i = 0; i<size_; ++i){
			Node* p = begin_[i];
			while(p){
				Node* next = p->next;
				p->~Node();
				user_free(p, sizeof(Node));
				p = next;
			}
		}
		user_free(begin_, sizeof(Node*)*size_);
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

		size_ = size_ + size_/2 + addsize;
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
		user_free(oldbegin, sizeof(Node*)*oldsize);
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
	};

	/*
	virtual void visit_members(Visitor& m){
		AnyImpl::visit_members(m);
		for(uint_t i = 0; i<size_; ++i){
			Node* p = begin_[i];
			while(p){
				Node* next = p->next;
				m & p->key & p->value;
				p = next;
			}
		}		
	}
	*/

public:

	const String& insert(const char* str, int_t size);

	virtual void before_gc();
};

const String& StringMgrImpl::insert(const char* str, int_t size){
	Guard guard(guard_);

	uint_t hashcode = make_hashcode(str, size);
	Node** p = &begin_[hashcode % size_];
	while(*p){
		if((*p)->size==size && memcmp((*p)->str, str, size)==0){
			return (*p)->value;
		}
		p = &(*p)->next;
	}

	*p = (Node*)user_malloc(sizeof(Node));
	new(*p) Node();

	new((*p)->value) StringImpl(str, size, hashcode);
	(*p)->hashcode = hashcode;
	(*p)->str = (*p)->value.c_str();
	(*p)->size = size;

	used_size_++;
	if(rate()>0.25f){
		expand(17);

		p = &begin_[hashcode % size_];
		while(*p){
			if((*p)->size==size && memcmp((*p)->str, str, size)==0){
				return (*p)->value;
			}
			p = &(*p)->next;
		}
		return (*p)->value;
	}else{
		return (*p)->value;
	}
}

void StringMgrImpl::before_gc(){
	if(guard_){
		return;
	}

	for(uint_t i = 0; i<size_; ++i){
		Node* p = begin_[i];
		Node* prev = 0;
		while(p){
			Node* next = p->next;
			if(p->value.impl()->ref_count()==1){
				p->~Node();
				user_free(p, sizeof(Node));
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
	
static StringMgrImpl* str_mgr(){
	static LLVar<Any> p;
	if(!p){ 
		new(p) StringMgrImpl();
	}
	return (StringMgrImpl*)p.impl();
}

void StringImpl::common_init(int_t len){
	set_class(TClass<String>::get());
	size_ = len;
	str_ = static_cast<char*>(user_malloc(size_+1));
	str_[size_]=0;
	intern_ = false;
}

String::String(const char* str){
	new(*this) StringImpl(str);
}

String::String(const string_t& str){
	new(*this) StringImpl(str);
}

String::String(const char* str, int_t size){
	new(*this) StringImpl(str, size);
}
	
String::String(const char* begin, const char* last){
	new(*this) StringImpl(begin, last);
}

String::String(const char* str1, int_t size1, const char* str2, int_t size2){
	new(*this) StringImpl(str1, size1, str2, size2);
}

String::String(char* str, int_t size, int_t buffer_size, delegate_memory_t){
	new(*this) StringImpl(str, size, buffer_size, delegate_memory_t());
}

const char* String::c_str() const{
	return impl()->c_str();
}

int_t String::size() const{
	return impl()->size();
}

int_t String::length() const{
	return impl()->length();
}

String String::slice(int_t first, int_t last) const{
	return impl()->slice(first, last);
}

String String::clone() const{
	return impl()->clone();
}

ID String::intern() const{
	return *this;
}
	
bool String::is_interned() const{
	return impl()->is_interned();
}

int_t String::to_i() const{
	return impl()->to_i();
}

float_t String::to_f() const{
	return impl()->to_f();
}

String String::to_s() const{
	return impl()->to_s();
}

Any String::split(const String& sep) const{
	return impl()->split(sep);
}

String String::cat(const String& v) const{
	return impl()->op_cat_String(v);
}

String String::op_cat_String(const String& v) const{
	return impl()->op_cat_String(v);
}

void String::op_cat(const VMachine& vm) const{
	Any a = vm.arg(0); vm.recycle_call(*this); a.send(Xid(op_cat_r_String), vm);
}

void String::op_eq(const VMachine& vm) const{
	Any a = vm.arg(0); 
	vm.recycle_call(*this); 
	a.send(Xid(op_eq_r_String), vm);
	if(!vm.processed()){
		vm.return_result(null);
	}
}

void String::op_lt(const VMachine& vm) const{
	Any a = vm.arg(0); vm.recycle_call(*this); a.send(Xid(op_lt_r_String), vm);
}

String String::op_cat_r_String(const String& v) const{
	return v.op_cat_String(*this);
}

bool String::op_eq_r_String(const String& v) const{
	return impl()->op_eq_r_String(v);
}

bool String::op_lt_r_String(const String& v) const{
	return impl()->op_lt_r_String(v);
}

	

ID::ID(const char* name)
:String(name ? str_mgr()->insert(name, strlen(name)) : null){}

ID::ID(const char* name, int_t size)
:String(str_mgr()->insert(name, size)){}

ID::ID(const String& name)
:String(!name ? null : name.impl()->is_interned() ? name : str_mgr()->insert(name.c_str(), name.size())){}

//{{{
//}}}


}
