
#include "xtal.h"

#include <algorithm>

#include "xtal_array.h"
#include "xtal_stream.h"
#include "xtal_macro.h"

namespace xtal{

class ArrayIter : public Base{
	ArrayPtr array_;
	int_t index_;
	bool reverse_;

	virtual void visit_members(Visitor& m){
		Base::visit_members(m);
		m & array_;
	}

public:

	ArrayIter(const ArrayPtr& a, bool reverse = false)
		:array_(a), index_(-1), reverse_(reverse){
	}

	AnyPtr reset(){
		index_ = -1;
		return SmartPtr<ArrayIter>::from_this(this);
	}
			
	void iter_next(const VMachinePtr& vm){
		++index_;
		if(index_<array_->size()){
			vm->return_result(SmartPtr<ArrayIter>::from_this(this), array_->at(reverse_ ? array_->size()-1-index_ : index_));
		}else{
			reset();
			vm->return_result(null, null);
		}
	}

	void remove(){
		if(-1<=index_ && index_<array_->size()-1){
			array_->erase(index_+1);
		}
	}
};


void InitArray(){

	{
		ClassPtr p = new_cpp_class<ArrayIter>("ArrayIter");
		p->inherit(Iterator());
		p->method("reset", &ArrayIter::reset);
		p->method("iter_first", &ArrayIter::iter_next);
		p->method("iter_next", &ArrayIter::iter_next);
		p->method("remove", &ArrayIter::remove);
	}

	{
		ClassPtr p = new_cpp_class<Array>("Array");
		p->inherit(Enumerator());

		p->def("new", ctor<Array, int_t>()->param(Named("size", 0)));
		p->method("size", &Array::size);
		p->method("length", &Array::length);
		p->method("resize", &Array::resize);
		p->method("empty", &Array::empty);
		p->method("at", &Array::op_at);
		p->method("set_at", &Array::op_set_at);
		p->method("op_at", &Array::op_at);
		p->method("op_set_at", &Array::op_set_at);
		p->method("slice", &Array::slice)->param("i", Named("n", 1));
		p->method("splice", &Array::splice)->param("i", Named("n", 1));
		p->method("pop_back", &Array::pop_back);
		p->method("push_back", &Array::push_back);
		p->method("pop_front", &Array::pop_front);
		p->method("push_front", &Array::push_front);
		p->method("erase", &Array::erase)->param("i", Named("n", 1));
		p->method("insert", &Array::insert);
		p->method("cat", &Array::cat);
		p->method("cat_assign", &Array::cat_assign);
		p->method("op_cat", &Array::cat);
		p->method("op_cat_assign", &Array::cat_assign);
		p->method("to_s", &Array::to_s);
		p->method("to_a", &Array::to_a);
		p->method("join", &Array::join)->param(Named("sep"));
		p->method("each", &Array::each);
		p->method("r_each", &Array::r_each);
		p->method("clone", &Array::clone);
		p->method("front", &Array::front);
		p->method("back", &Array::back);
		p->method("clear", &Array::clear);
		p->method("op_eq", &Array::op_eq);
		p->method("reverse", &Array::reverse);
		p->method("reversed", &Array::reversed);
		p->method("assign", &Array::assign);
	}

	builtin()->def("Array", get_cpp_class<Array>());
}

void Array::visit_members(Visitor& m){
	Base::visit_members(m);
	for(uint_t i=0; i<size_; ++i){
		m & values_[i];
	}
}

Array::Array(int_t size){
	capa_ = size + 7;
	size_ = size;
	values_ = (AnyPtr*)user_malloc(sizeof(AnyPtr)*capa_);
	memset(values_, 0, sizeof(AnyPtr)*size_);
}

Array::Array(const AnyPtr* first, const AnyPtr* end){
	int_t size = end-first;

	capa_ = size;
	size_ = size;
	values_ = (AnyPtr*)user_malloc(sizeof(AnyPtr)*capa_);

	for(int_t i=0; i<size; ++i){
		copy_innocence(values_[i], first[i]);
		inc_ref_count_force(values_[i]);
	}
}

Array::Array(const Array& v){
	size_ = capa_ = ((Array&)v).size();
	values_ = (AnyPtr*)user_malloc(sizeof(AnyPtr)*capa_);
	
	for(uint_t i=0; i<size_; ++i){
		copy_innocence(values_[i], v.values_[i]);
		inc_ref_count_force(values_[i]);
	}
}

Array& Array::operator =(const Array& v){
	Array temp(v);
	std::swap(values_, temp.values_);
	std::swap(size_, temp.size_);
	std::swap(capa_, temp.capa_);
	return *this;
}

Array::~Array(){
	for(uint_t i=0; i<size_; ++i){
		dec_ref_count_force(values_[i]);
	}
	user_free(values_);
}

void Array::clear(){
	for(uint_t i=0; i<size_; ++i){
		dec_ref_count_force(values_[i]);
	}
	size_ = 0;
}

void Array::resize(int_t sz){
	if(sz<0) sz = 0;

	if(sz<(int_t)size_){
		for(uint_t i=sz; i<size_; ++i){
			dec_ref_count_force(values_[i]);
		}
		size_ = sz;
	}else if(sz>(int_t)size_){
		if(sz>(int_t)capa_){
			uint_t newcapa = sz+capa_;
			AnyPtr* newp = (AnyPtr*)user_malloc(sizeof(AnyPtr)*newcapa);
			memcpy(newp, values_, sizeof(AnyPtr)*size_);
			memset(&newp[size_], 0, sizeof(AnyPtr)*(sz-size_));
			user_free(values_);
			values_ = newp;
			size_ = sz;
			capa_ = newcapa;
		}else{
			memset(&values_[size_], 0, sizeof(AnyPtr)*(sz-size_));
			size_ = sz;
		}
	}
}

const AnyPtr& Array::op_at(int_t i){
	return values_[calc_offset(i)];

}

void Array::op_set_at(int_t i, const AnyPtr& v){
	values_[calc_offset(i)] = v;
}

void Array::erase(int_t start, int_t n){
	int pos = calc_offset(start);
	if(n<0 || (uint_t)(n + pos)>size_){
		throw_index_error();
		return;
	}

	for(int_t i=0; i<n; ++i){
		dec_ref_count_force(values_[pos + i]);
	}

	size_ -= n;
	if(size_!=0){
		memmove(&values_[pos], &values_[pos+n], sizeof(AnyPtr)*(size_-pos));
	}
}

void Array::insert(int_t i, const AnyPtr& v){
	if(capa_==size_){
		resize(size_ + 1);
	}else{
		size_++;
	}
	int_t pos = calc_offset(i);
	memmove(&values_[pos+1], &values_[pos], sizeof(AnyPtr)*(size_-1-pos));
	copy_innocence(values_[pos], v);
	inc_ref_count_force(values_[pos]);
}

void Array::push_back(const AnyPtr& v){
	if(capa_==size_){
		resize(size_ + 1);
	}else{
		size_++;
	}
	copy_innocence(values_[size_-1], v);
	inc_ref_count_force(values_[size_-1]);
}

void Array::pop_back(){
	if(!empty()){
		dec_ref_count_force(values_[size_-1]);
		size_--;
	}
}

ArrayPtr Array::slice(int_t start, int_t n){
	if(n==0){
		return xnew<Array>(0);
	}

	int_t pos = calc_offset(start);
	if(n<0 || (uint_t)(n + pos)>size_){
		throw_index_error();
		return null;
	}

	return xnew<Array>(&values_[pos], &values_[pos+n]);
}

ArrayPtr Array::splice(int_t start, int_t n){
	ArrayPtr ret = slice(start, n);
	erase(start, n);
	return ret;
}

void Array::reverse(){
	uint_t sz = size();
	for(uint_t i=0; i<sz/2; ++i){
		swap(values_[sz-1-i], values_[i]);
	}
}

ArrayPtr Array::reversed(){
	uint_t sz = size();
	ArrayPtr ret = xnew<Array>(size());
	for(uint_t i=0; i<sz; ++i){
		ret->values_[sz-1-i] = values_[i];
	}
	return ret;
}

ArrayPtr Array::clone(){
	return xnew<Array>(*this);
}

ArrayPtr Array::cat(const ArrayPtr& a){
	ArrayPtr ret(clone());
	ret->cat_assign(a);
	return ret;
}

ArrayPtr Array::cat_assign(const ArrayPtr& a){
	for(uint_t i = 0, size = a->size(); i<size; ++i){
		push_back(a->at(i));
	}
	return ArrayPtr::from_this(this);
}

StringPtr Array::join(const StringPtr& sep){
	MemoryStreamPtr ret(xnew<MemoryStream>());
	if(sep->buffer_size()==0){
		for(uint_t i = 0, sz = size(); i<sz; ++i){
			ret->put_s(at(i)->to_s());
		}
	}else{
		for(uint_t i = 0, sz = size(); i<sz; ++i){
			ret->put_s(at(i)->to_s());
			if(i<sz-1){
				ret->put_s(sep);
			}
		}
	}
	return ret->to_s();
}

StringPtr Array::to_s(){
	MemoryStreamPtr ret(xnew<MemoryStream>());
	ret->put_s("[");
	ret->put_s(join(","));
	ret->put_s("]");
	return ret->to_s();
}

bool Array::op_eq(const ArrayPtr& other){
	if(size()!=other->size())
		return false;

	const VMachinePtr& vm = vmachine();
	for(uint_t i=0, size=other->size(); i<size; ++i){
		vm->setup_call(1, other->at(i));
		at(i)->rawsend(vm, Xid(op_eq));
		if(!vm->processed() || !vm->result()){
			vm->cleanup_call();
			return false;
		}
		vm->cleanup_call();
	}
	return true;
}

AnyPtr Array::each(){
	return xnew<ArrayIter>(ArrayPtr::from_this(this));
}

AnyPtr Array::r_each(){
	return xnew<ArrayIter>(ArrayPtr::from_this(this), true);
}

void Array::assign(const ArrayPtr& other){
	resize(other->size());
	for(int_t i=0, sz=size(); i<sz; ++i){
		values_[i] = other->values_[i];
	}
}

int_t Array::calc_offset(int_t i){
	if(i<0){
		i = size_ + i;
		if(i<0){
			throw_index_error();
			return 0;
		}
	}else{
		if((uint_t)i >= size_){
			throw_index_error();
			return 0;
		}
	}
	return i;
}

void Array::throw_index_error(){
	XTAL_THROW(builtin()->member("RuntimeError")(Xt("Xtal Runtime Error 1020")), return);
}

}
