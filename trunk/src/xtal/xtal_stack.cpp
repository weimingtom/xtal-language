#include "xtal.h"

namespace xtal{

namespace{
	int stack_dummy_buf[16] = {0};
}

void* stack_dummy_allocate(){
	return stack_dummy_buf;
}

void* stack_allocate(size_t size){
	return user_malloc(size);
}

void stack_deallocate(void* p, size_t size){
	if(stack_dummy_buf==p){
		return;
	}
	XTAL_ASSERT(!((char*)stack_dummy_buf-64 <= (char*)p && (char*)p <= (char*)stack_dummy_buf+64));
	user_free(p/*, size*/);
}


PODStackBase::PODStackBase(size_t onesize){
	one_size_ = onesize;
	begin_ = plusp(stack_dummy_allocate(), 1);
	current_ = minusp(begin_, 1);
	end_ = begin_;
}

PODStackBase::~PODStackBase(){
	stack_deallocate(minusp(begin_, 1), one_size_*(capacity()+1));
}

PODStackBase::PODStackBase(const PODStackBase &a){
	one_size_ = a.one_size_;
	if(a.capacity()==0){
		begin_ = plusp(stack_dummy_allocate(), 1);
		current_ = minusp(begin_, 1);
		end_ = begin_;
	}
	else{
		begin_ = plusp(stack_allocate(one_size_*(a.capacity()+1)), 1);
		current_ = plusp(begin_, a.size()-1);
		end_ = plusp(begin_, a.capacity());
		std::memcpy(begin_, a.begin_, one_size_*a.capacity());
	}
}

PODStackBase &PODStackBase::operator =(const PODStackBase &a){
	if(this==&a){
		return *this;
	}

	if(a.capacity()==0){
		stack_deallocate(minusp(begin_, 1), one_size_*(capacity()+1));
		one_size_ = a.one_size_;
		begin_ = plusp(stack_dummy_allocate(), 1);
		current_ = minusp(begin_, 1);
		end_ = begin_;
	}
	else{
		void* newp = plusp(stack_allocate(a.one_size_*(a.capacity()+1)), 1);
		stack_deallocate(minusp(begin_, 1), one_size_*(capacity()+1));
		one_size_ = a.one_size_;
		begin_ = newp;
		current_ = plusp(begin_, a.size()-1);
		end_ = plusp(begin_, a.capacity());
		std::memcpy(begin_, a.begin_, one_size_*a.capacity());
	}

	return *this;
}

void PODStackBase::resize(size_t newsize){
	XTAL_ASSERT(newsize>=0);

	size_t oldsize = size();
	if(newsize>oldsize){
		upsize(newsize-oldsize);
	}
	else{
		downsize_n(newsize);
	}
}

void PODStackBase::upsize_detail(size_t us){
	XTAL_ASSERT(us>=0);

	subp(current_, us);

	size_t oldsize = size();
	size_t oldcapa = capacity();
	size_t newsize = oldsize+us;
	void* oldp = begin_;
	size_t newcapa = 2 + us + oldcapa + oldcapa/2;
	void* newp = plusp(stack_allocate(one_size_*(newcapa+1)), 1);

	std::memcpy(newp, oldp, one_size_*oldsize);
	stack_deallocate(minusp(oldp, 1), one_size_*(oldcapa+1));

	begin_ = newp;
	current_ = plusp(begin_, newsize-1);
	end_ = plusp(begin_, newcapa);
}

void PODStackBase::erase(size_t i, size_t n){
	std::memmove((*this)[i], (*this)[i-n], one_size_*(i+1-n));
	downsize(n);
}	

void PODStackBase::insert(size_t i, const void* v, size_t n){
	upsize(n);
	std::memmove((*this)[i], (*this)[i+n], one_size_*(i+1-n));
	std::memcpy((*this)[i], v, one_size_*n);
}

void PODStackBase::reverse_erase(size_t i, size_t n){
	std::memmove(reverse_at(i), reverse_at(i+n), one_size_*(size()+1-n));
	downsize(n);
}	

void PODStackBase::reverse_insert(size_t i, const void* v, size_t n){
	upsize(n);
	std::memmove(reverse_at(i+n), reverse_at(i), one_size_*(size()+1-n));
	std::memcpy(reverse_at(i), v, one_size_*n);
}

void PODStackBase::reserve(size_t capa){
	if(capa<=capacity()){
		return;
	}

	size_t diff = capa-size();
	upsize(diff);
	downsize(diff);
}

void PODStackBase::release(){
	stack_deallocate(minusp(begin_, 1), one_size_*(capacity()+1));
	begin_ = plusp(stack_dummy_allocate(), 1);
	current_ = minusp(begin_, 1);
	end_ = begin_;
}

void PODStackBase::attach(void* p){
	begin_ = plusp(p, 1);
	current_ = minusp(begin_, 1);
	end_ = begin_;
	addp(current_, 1);
}

void PODStackBase::detach(){
	begin_ = plusp(stack_dummy_allocate(), 1);
	current_ = minusp(begin_, 1);
	end_ = begin_;
}


//////////////////////////

StackBase::StackBase(size_t onesize,
	void (*ctor)(void* p),
	void (*copy_ctor)(void* p, const void* q),
	void (*dtor)(void* p))
	:impl_(onesize), ctor_(ctor), copy_ctor_(copy_ctor), dtor_(dtor){} 

StackBase::StackBase(const StackBase& v)
	:impl_(v.impl_){
	for(size_t i=0, sz=size(); i<sz; ++i){
		copy_ctor_((*this)[i], v[i]);
	}
}

StackBase& StackBase::operator =(const StackBase& v){
	if(this==&v){
		return *this;
	}

	impl_ = v.impl_;

	for(size_t i=0, sz=size(); i<sz; ++i){
		copy_ctor_((*this)[i], v[i]);
	}

	return *this;
}

StackBase::~StackBase(){
	release();
}

void StackBase::erase(size_t i){
	dtor_((*this)[i]);
	for(size_t j = i; j != 0; --j){
		copy_ctor_((*this)[j], (*this)[j-1]);
		dtor_((*this)[j-1]);
	}
	impl_.downsize(1);
}
	
void StackBase::insert(size_t i, const void* v){
	impl_.upsize(1);
	for(size_t j = 0; j != i; ++j){
		copy_ctor_((*this)[j], (*this)[j+1]);
		dtor_((*this)[j+1]);
	}
	copy_ctor_((*this)[i], v);		
}

void StackBase::resize(size_t newsize){
	if(newsize>size()){
		upsize(newsize-size());
	}
	else if(newsize<size()){
		downsize(size()-newsize);
	}
}

void StackBase::downsize(size_t ds){
	for(size_t i=0; i<ds; ++i){
		dtor_((*this)[i]);
	}
	impl_.downsize(ds);
}

void StackBase::upsize(size_t us){
	impl_.upsize(us);
	for(size_t i=0; i<us; ++i){
		ctor_((*this)[i]);
	}		
}

void StackBase::release(){
	clear();
	impl_.release();
}

}
