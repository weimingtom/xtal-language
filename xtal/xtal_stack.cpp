
#include "xtal.h"
#include "xtal_stack.h"

namespace xtal{

PODStackBase::Buf PODStackBase::buf_;


PODStackBase::PODStackBase(size_t onesize){
	one_size_ = onesize;
	begin_ = plusp(dummy_allocate(), 1);
	current_ = minusp(begin_, 1);
	end_ = begin_;
}

PODStackBase::~PODStackBase(){
	deallocate(minusp(begin_, 1), one_size_*(capacity()+1));
}

PODStackBase::PODStackBase(const PODStackBase &a){
	one_size_ = a.one_size_;
	if(a.capacity()==0){
		begin_ = plusp(dummy_allocate(), 1);
		current_ = minusp(begin_, 1);
		end_ = begin_;
	}else{
		begin_ = plusp(allocate(one_size_*(a.capacity()+1)), 1);
		current_ = plusp(begin_, a.size()-1);
		end_ = plusp(begin_, a.capacity());
		memcpy(begin_, a.begin_, one_size_*a.capacity());
	}
}

PODStackBase &PODStackBase::operator =(const PODStackBase &a){
	if(this==&a)
		return *this;

	if(a.capacity()==0){
		deallocate(minusp(begin_, 1), one_size_*(capacity()+1));
		one_size_ = a.one_size_;
		begin_ = plusp(dummy_allocate(), 1);
		current_ = minusp(begin_, 1);
		end_ = begin_;
	}else{
		void* newp = plusp(allocate(a.one_size_*(a.capacity()+1)), 1);
		deallocate(minusp(begin_, 1), one_size_*(capacity()+1));
		one_size_ = a.one_size_;
		begin_ = newp;
		current_ = plusp(begin_, a.size()-1);
		end_ = plusp(begin_, a.capacity());
		memcpy(begin_, a.begin_, one_size_*a.capacity());
	}

	return *this;
}

void PODStackBase::resize(size_t newsize){
	XTAL_ASSERT(newsize>=0);

	size_t oldsize = size();
	if(newsize>oldsize)
		upsize(newsize-oldsize);
	else
		downsize_n(newsize);
}

void PODStackBase::upsize_detail(size_t us){
	XTAL_ASSERT(us>=0);

	subp(current_, us);

	size_t oldsize = size();
	size_t oldcapa = capacity();
	size_t newsize = oldsize+us;
	void* oldp = begin_;
	size_t newcapa = 16 + newsize + newsize/2;
	void* newp = plusp(allocate(one_size_*(newcapa+1)), 1);

	memcpy(newp, oldp, one_size_*oldsize);
	deallocate(minusp(oldp, 1), one_size_*(oldcapa+1));

	begin_ = newp;
	current_ = plusp(begin_, newsize-1);
	end_ = plusp(begin_, newcapa);
}

void PODStackBase::reserve(size_t capa){
	if(capa<=capacity())
		return;
	size_t diff = capa-size();
	upsize(diff);
	downsize(diff);
}

}
