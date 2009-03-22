#include "xtal.h"
#include "xtal_macro.h"

namespace xtal{

void Array::visit_members(Visitor& m){
	for(uint_t i=0; i<size_; ++i){
		m & values_[i];
	}
}

Array::Array(uint_t size){
	set_pvalue(*this, TYPE, this);

	capa_ = size;
	size_ = size;

	if(capa_!=0){
		values_ = (AnyPtr*)xmalloc(sizeof(AnyPtr)*capa_);
		std::memset(values_, 0, sizeof(AnyPtr)*size_);
	}
	else{
		values_ = 0;
	}
}

Array::Array(const AnyPtr* first, const AnyPtr* end){
	set_pvalue(*this, TYPE, this);

	int_t size = end-first;

	capa_ = size;
	size_ = size;
	values_ = (AnyPtr*)xmalloc(sizeof(AnyPtr)*capa_);

	for(int_t i=0; i<size; ++i){
		copy_any(values_[i], first[i]);
		inc_ref_count_force(values_[i]);
	}
}

Array::Array(const Array& v)
:RefCountingBase(v){
	set_pvalue(*this, TYPE, this);

	size_ = capa_ = ((Array&)v).size();
	if(capa_!=0){
		values_ = (AnyPtr*)xmalloc(sizeof(AnyPtr)*capa_);
		
		for(uint_t i=0; i<size_; ++i){
			copy_any(values_[i], v.values_[i]);
			inc_ref_count_force(values_[i]);
		}
	}
	else{
		values_ = 0;
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
	xfree(values_, sizeof(AnyPtr)*capa_);
}

void Array::clear(){
	for(uint_t i=0; i<size_; ++i){
		dec_ref_count_force(values_[i]);
	}
	size_ = 0;
}

void Array::resize(uint_t sz){
	if(sz<size_){
		downsize(size_-sz);
	}
	else if(sz>size_){
		upsize(sz-size_);
	}
}

void Array::upsize(uint_t sz){
	if(sz==0){
		return;
	}

	if(size_+sz>capa_){ // todo overflow check
		if(capa_!=0){
			uint_t newcapa = size_+sz+capa_+3;
			AnyPtr* newp = (AnyPtr*)xmalloc(sizeof(AnyPtr)*newcapa);
			std::memcpy(newp, values_, sizeof(AnyPtr)*size_);
			std::memset(&newp[size_], 0, sizeof(AnyPtr)*sz);
			xfree(values_, sizeof(AnyPtr)*capa_);
			values_ = newp;
			size_ += sz;
			capa_ = newcapa;
		}
		else{
			uint_t newcapa = 3+sz; // todo overflow check
			values_ = (AnyPtr*)xmalloc(sizeof(AnyPtr)*newcapa);
			std::memset(&values_[0], 0, sizeof(AnyPtr)*sz);
			size_ = sz;
			capa_ = newcapa;
		}
	}
	else{
		std::memset(&values_[size_], 0, sizeof(AnyPtr)*sz);
		size_ += sz;
	}
}

void Array::downsize(uint_t sz){
	if(sz>size_){
		sz = size_;
	}

	for(uint_t i=size_-sz; i<size_; ++i){
		dec_ref_count_force(values_[i]);
	}
	size_ -= sz;
}

const AnyPtr& Array::op_at(int_t i){
	i = calc_offset(i);
	if(i<0){
		return undefined;
	}

	XTAL_ASSERT(0<=i && (uint_t)i<size_);
	return values_[i];

}

void Array::op_set_at(int_t i, const AnyPtr& v){
	i = calc_offset(i);
	if(i<0){
		return;
	}

	XTAL_ASSERT(0<=i && (uint_t)i<size_);
	values_[i] = v;
}

void Array::erase(int_t start, int_t n){
	if(n==0){
		return;
	}

	int_t pos = calc_offset(start);
	if(pos<0){
		return;
	}

	if(n<0 || (uint_t)(n + pos)>size_){
		throw_index_error();
		return;
	}

	for(int_t i=0; i<n; ++i){
		dec_ref_count_force(values_[pos+i]);
	}

	XTAL_ASSERT(0<=pos && (uint_t)pos<size_);
	//XTAL_ASSERT(0<=pos+n && (uint_t)pos+n<size_);

	if(size_-(pos+n)!=0){
		std::memmove(&values_[pos], &values_[pos+n], sizeof(AnyPtr)*(size_-(pos+n)));
	}
	size_ -= n;
}

void Array::insert(int_t i, const AnyPtr& v){
	if(capa_==size_){
		upsize(1);
	}
	else{
		size_++;
	}

	int_t pos = calc_offset(i);
	if(pos<0){
		size_--;
		return;
	}

	std::memmove(&values_[pos+1], &values_[pos], sizeof(AnyPtr)*(size_-(pos+1)));
	copy_any(values_[pos], v);
	inc_ref_count_force(values_[pos]);
}

void Array::push_back(const AnyPtr& v){
	if(capa_==size_){
		upsize(1);
	}
	else{
		size_++;
	}
	copy_any(values_[size_-1], v);
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
	if(pos<0){
		return null;
	}

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
	return to_smartptr(this);
}
	
StringPtr Array::join(const StringPtr& sep){
	return ptr_cast<String>(send(Xid(join), sep));
}

bool Array::op_eq(const ArrayPtr& other){
	if(size()!=other->size())
		return false;

	const VMachinePtr& vm = vmachine();
	for(uint_t i=0, size=other->size(); i<size; ++i){
		if(rawne(at(i), other->at(i))){
			vm->setup_call(1, other->at(i));
			at(i)->rawsend(vm, Xid(op_eq));
			if(!vm->processed() || !vm->result()){
				vm->cleanup_call();
				return false;
			}
			vm->cleanup_call();
		}
	}
	return true;
}

AnyPtr Array::each(){
	return xnew<ArrayIter>(to_smartptr(this));
}

AnyPtr Array::reverse(){
	return xnew<ArrayIter>(to_smartptr(this), true);
}

void Array::assign(const AnyPtr& iterator){
	clear();

	uint_t i = 0;
	Xfor(v, iterator){
		if(i>=size_){
			upsize(1);
		}

		set_at(i, v);
		++i;
	}
}

void Array::concat(const AnyPtr& iterator){
	Xfor(v, iterator){
		push_back(v);
	}
}

int_t Array::calc_offset(int_t i){
	if(i<0){
		i = size_ + i;
		if(i<0){
			throw_index_error();
			return -1;
		}
	}
	else{
		if((uint_t)i >= size_){
			throw_index_error();
			return -1;
		}
	}
	return i;
}

void Array::throw_index_error(){
	XTAL_SET_EXCEPT(cpp_class<RuntimeError>()->call(Xt("Xtal Runtime Error 1020")));
}

//////////////////////////////////////////////////

ArrayIter::ArrayIter(const ArrayPtr& a, bool reverse)
	:array_(a), index_(0), reverse_(reverse){
}
		
void ArrayIter::block_next(const VMachinePtr& vm){
	++index_;
	if(index_<=array_->size()){
		vm->return_result(to_smartptr(this), array_->at(reverse_ ? array_->size()-index_ : index_-1));
	}
	else{
		vm->return_result(null, null);
	}
}
	
bool ArrayIter::block_next_direct(AnyPtr& ret){
	++index_;
	if(index_<=array_->size()){
		ret = array_->at(reverse_ ? array_->size()-index_ : index_-1);
		return true;
	}
	else{
		ret = null;
		return false;
	}
}

void ArrayIter::visit_members(Visitor& m){
	Base::visit_members(m);
	m & array_;
}

//////////////////////////////////////////////////

void MultiValue::block_next(const VMachinePtr& vm){
	if(tail_){
		vm->return_result(tail_, head_);
	}
	else{
		vm->return_result(0, head_);
	}
}

int_t MultiValue::size(){
	const MultiValuePtr* cur = &to_smartptr(this);
	int_t size = 1;
	while(true){
		if(!(*cur)->tail_){
			return size;
		}
		cur = &(*cur)->tail_;
		++size;
	}
}

const AnyPtr& MultiValue::at(int_t i){
	const MultiValuePtr* cur = &to_smartptr(this);
	const AnyPtr* ret = &head_;
	for(int_t n=0; n<i; ++n){
		if(!(*cur)->tail_){
			ret = &undefined;
			break;
		}
		cur = &(*cur)->tail_;
		ret = &(*cur)->head_;
	}
	return *ret;
}

}
