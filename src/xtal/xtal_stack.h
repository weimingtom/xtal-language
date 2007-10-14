
#pragma once

namespace xtal{

/*
* VMachineクラスのスタック用に、特定の操作の実行速度を重視して実装したスタックコンテナ。
*/
template<class T>
class Stack{

	T* begin_; // 確保したメモリの先頭の次を指す。
	T* end_; // 確保したメモリの一番最後の次を指す
	T* current_; // スタックトップの要素を指す

private:

	struct Buf{
		char buf[sizeof(T)];
	};

	static Buf buf_;

	static void* dummy_allocate(){
		return &buf_;
	}

	static void* allocate(size_t size){
		return user_malloc(size);
	}

	static void deallocate(void* p){
		if(p==&buf_)
			return;
		user_free(p);
	}

	void upsize_detail(size_t us);

public:

	typedef T value_type;
	typedef T* iterator;
	typedef const T* const_iterator; 

public:

	Stack();

	Stack(const Stack<T>& a);
	
	Stack<T> &operator =(const Stack<T>& a);

	~Stack();

public:

	iterator begin(){
		return begin_;
	}
	
	const_iterator begin() const{
		return begin_;
	}
	
	iterator end(){
		return current_+1;
	}
	
	const_iterator end() const{
		return current_+1;
	}
		
	T& push_unchecked(const T &val){
		upsize_unchecked(1);
		return top()=val;
	}

	T& push(const T &val){
		upsize(1);
		return top()=val;
	}

	T& push_unchecked(){
		upsize_unchecked(1);
		return top();
	}

	T& push(){
		upsize(1);
		return top();
	}

	T& pop(){
		XTAL_ASSERT(!empty());
		--current_;
		return current_[1];
	}

	T& top(){
		XTAL_ASSERT(!empty());
		return *current_;
	}

	const T& top() const{
		XTAL_ASSERT(!empty());
		return *current_;
	}

	void erase(size_t i){
		for(size_t j = i; j != 0; --j){
			(*this)[j] = (*this)[j-1];
		}
		downsize(1);
	}
	
	void resize(size_t newsize);

	void downsize(size_t ds){
		XTAL_ASSERT(size()>=ds);
		current_-=ds;
	}

	void downsize_n(size_t newsize){
		current_ = begin_-1+newsize;
	}

	void upsize_unchecked(size_t us){
		current_+=us;
	}

	void upsize(size_t us){		
		current_+=us;
		if(current_>=end_)
			upsize_detail(us);
	}

	size_t size() const{
		return (current_+1)-begin_;
	}

	size_t capacity() const{
		return end_-begin_;
	}

	void reserve(size_t capa);

	T& operator [](size_t i){
		XTAL_ASSERT(i<size());
		return *(current_-i);
	}

	const T &operator [](size_t i) const{
		XTAL_ASSERT(i<size());
		return *(current_-i);
	}

	T& reverse_at(size_t i){
		XTAL_ASSERT(i<size());
		return *(begin_+i);
	}

	const T &reverse_at(size_t i) const{
		XTAL_ASSERT(i<size());
		return *(begin_+i);
	}

	T& reverse_at_unchecked(size_t i){
		return *(begin_+i);
	}

	const T &reverse_at_unchecked(size_t i) const{
		return *(begin_+i);
	}

	bool empty() const{
		XTAL_ASSERT(begin_<=current_+1);
		return begin_==current_+1;
	}

	void clear(){
		current_ = begin_-1;
	}
	
	void push(const Stack<T>& other, size_t sz){
		for(size_t i = sz; i!=0; --i){
			push(other[i-1]);
		} 
	}
	
	void push(const Stack<T>& other, size_t other_offset, size_t sz){
		for(size_t i = sz; i!=0; --i){
			push(other[other_offset+i-1]);
		} 
	}
		
	void move(Stack<T>& other, size_t sz){
		push(other, sz);
		other.downsize(sz);
	}

	void fill_over(const T& val = T()){
		for(size_t i = size(); i<capacity(); ++i){
			reverse_at_unchecked(i)=val;
		}
	}
};

template<class T>
typename Stack<T>::Buf Stack<T>::buf_;

template<class T>
Stack<T>::Stack(){
	begin_=(T*)dummy_allocate()+1;
	current_ = begin_-1;
	end_ = begin_+0;
}

template<class T>
Stack<T>::~Stack(){
	for(size_t i = 0, last = capacity(); i<last; ++i)
		begin_[i].~T();
	deallocate(begin_-1);
}

template<class T>
Stack<T>::Stack(const Stack<T> &a){
	if(a.empty()){
		begin_=(T*)dummy_allocate()+1;
		current_ = begin_-1;
		end_ = begin_+0;
	}else{
		begin_=(T*)allocate(sizeof(T)*(a.capacity()+1))+1;
		current_ = begin_+a.size()-1;
		end_ = begin_+a.capacity();

		size_t capa = a.capacity();
		for(size_t i = 0; i<capa; ++i)
			new(&begin_[i]) T(a.begin_[i]);
	}
}

template<class T>
Stack<T> &Stack<T>::operator =(const Stack<T> &a){
	if(this==&a)
		return *this;

	T* newp = (T*)allocate(sizeof(T)*(a.capacity()+1))+1;

	for(size_t i = 0, last = capacity(); i<last; ++i)
		begin_[i].~T();
	deallocate(begin_-1);

	begin_ = newp;
	current_ = begin_+a.size()-1;
	end_ = begin_+a.capacity();

	size_t capa = a.capacity();
	for(size_t i = 0; i<capa; ++i)
		new(&begin_[i]) T(a.begin_[i]);

	return *this;
}

template<class T>
void Stack<T>::resize(size_t newsize){
	XTAL_ASSERT(newsize>=0);

	size_t oldsize = size();
	if(newsize>oldsize)
		upsize(newsize-oldsize);
	else
		downsize_n(newsize);
}

template<class T>
void Stack<T>::upsize_detail(size_t us){
	XTAL_ASSERT(us>=0);

	current_-=us;

	size_t oldsize = size();
	size_t oldcapa = capacity();
	size_t newsize = oldsize+us;
	T* oldp = begin_;
	size_t newcapa = 16 + newsize + newsize/2;
	T* newp = (T*)allocate(sizeof(T)*(newcapa+1))+1;

	for(size_t i = 0; i<oldsize; ++i)
		new(&newp[i]) T(oldp[i]);
	for(size_t i = oldsize; i<newcapa; ++i)
		new(&newp[i]) T();

	for(size_t i = 0; i<oldcapa; ++i)
		oldp[i].~T();
	deallocate(oldp-1);

	begin_ = newp;
	current_ = begin_+newsize-1;
	end_ = begin_+newcapa;
}


template<class T>
void Stack<T>::reserve(size_t capa){
	if(capa<=capacity())
		return;
	size_t diff = capa-size();
	upsize(diff);
	downsize(diff);
}

template<class T>
void visit_members(Visitor& m, const Stack<T>& value){
	for(int_t i = 0, size = value.capacity(); i<size; ++i){
		m & value.reverse_at_unchecked(i);
	}
}

/*
* ポインタ専用スタック
*/
template<class T>
class PStack{

	Stack<void*> impl_;

public:

	typedef T value_type;
	typedef T* iterator;
	typedef const T* const_iterator; 

public:

	iterator begin(){ return (iterator)impl_.begin(); }	
	const_iterator begin() const{ return (iterator)impl_.begin(); }
	iterator end(){ return (iterator)impl_.end(); }	
	const_iterator end() const{ return (iterator)impl_.end(); }		
	T& push_unchecked(const T &val){ return (T&)impl_.push_unchecked(val); }
	T& push(const T &val){ return (T&)impl_.push(val); }
	T& push_unchecked(){ return (T&)impl_.push_unchecked(); }
	T& push(){ return (T&)impl_.push(); }
	T& pop(){ return (T&)impl_.pop();}
	T& top(){ return (T&)impl_.top();}
	const T& top() const{ return (const T&)impl_.top();}
	void resize(size_t newsize){ impl_.resize(newsize);}
	void downsize(size_t ds){ impl_.downsize(ds);}
	void downsize_n(size_t newsize){ impl_.downsize_n(newsize);}
	void upsize_unchecked(size_t us){ impl_.upsize_unchecked(us);}
	void upsize(size_t us){ impl_.upsize(us);}
	size_t size() const{ return impl_.size();}
	size_t capacity() const{ return impl_.capacity();}
	void reserve(size_t capa){ impl_.reserve(capa);}
	T& operator [](size_t i){ return (T&)impl_[i];}
	const T& operator [](size_t i) const{ return (const T&)impl_[i];}
	T& reverse_at(size_t i){ return (T&)impl_.reverse_at(i);}
	const T& reverse_at(size_t i) const{ return (const T&)impl_.reverse_at(i);}
	T& reverse_at_unchecked(size_t i){ return (T&)impl_.reverse_at_unchecked(i);}
	const T& reverse_at_unchecked(size_t i) const{ return (const T&)impl_.reverse_at_unchecked(i);}
	bool empty() const{ return impl_.empty();}
	void clear(){ impl_.clear();}
	void push(const PStack<T>& other, size_t sz){ impl_.push((const Stack<void*>&)other, sz);}
	void push(const PStack<T>& other, size_t other_offset, size_t sz){ impl_.push((const Stack<void*>&)other, other_offset, sz);}	
	void move(PStack<T>& other, size_t sz){ impl_.move((Stack<void*>&)other, sz);}
};

/*
* POD専用に書き下ろしたスタックのベース部分
*/
class PODStackBase{

	void* begin_; // 確保したメモリの先頭の次を指す。
	void* end_; // 確保したメモリの一番最後の次を指す
	void* current_; // スタックトップの要素を指す
	int one_size_;

private:

	struct Buf{
		char buf[64];
	};

	static Buf buf_;

	static void* dummy_allocate(){
		return &buf_;
	}

	static void* allocate(size_t size){
		return user_malloc(size);
	}

	static void deallocate(void* p){
		if(p==&buf_)
			return;
		user_free(p);
	}

	void* plusp(const void* p, size_t v) const{
		return (char*)p + v*one_size_;
	}
	
	void* minusp(const void* p, size_t v) const{
		return (char*)p - v*one_size_;
	}

	void* addp(void*& p, size_t v){
		return p = plusp(p, v);
	}

	void* subp(void*& p, size_t v){
		return p = minusp(p, v);
	}

	void* incp(void*& p){
		return addp(p, 1);
	}

	void* decp(void*& p){
		return subp(p, 1);
	}

	void upsize_detail(size_t us);

public:

	explicit PODStackBase(size_t onesize);

	PODStackBase(const PODStackBase &a);
	
	PODStackBase &operator =(const PODStackBase &a);

	~PODStackBase();

public:

	void* begin(){
		return begin_;
	}
	
	const void* begin() const{
		return begin_;
	}
	
	void* end(){
		return plusp(current_, 1);
	}
	
	const void* end() const{
		return plusp(current_, 1);
	}

	void push_unchecked(const void* val){
		upsize_unchecked(1);
		memcpy(top(), val, one_size_);
	}

	void push(const void* val){
		upsize(1);
		memcpy(top(), val, one_size_);
	}

	void* push_unchecked(){
		upsize_unchecked(1);
		return top();
	}

	void* push(){
		upsize(1);
		return top();
	}

	void* pop(){
		XTAL_ASSERT(!empty());
		decp(current_);
		return plusp(current_, 1);
	}

	void* top(){
		XTAL_ASSERT(!empty());
		return current_;
	}

	const void* top() const{
		XTAL_ASSERT(!empty());
		return current_;
	}

	void erase(size_t i){
		for(size_t j = i; j != 0; --j){
			memcpy((*this)[j], (*this)[j-1], one_size_);
		}
		downsize(1);
	}

	void resize(size_t newsize);

	void downsize(size_t ds){
		XTAL_ASSERT(size()>=ds);
		subp(current_, ds);
	}

	void downsize_n(size_t newsize){
		current_ = plusp(begin_, -1+newsize);
	}

	void upsize_unchecked(size_t us){
		addp(current_, us);
	}

	void upsize(size_t us){		
		addp(current_, us);
		if(current_>=end_)
			upsize_detail(us);
	}

	size_t size() const{
		return ((char*)plusp(current_, 1) - (char*)begin_)/one_size_;
	}

	size_t capacity() const{
		return ((char*)end_ - (char*)begin_)/one_size_;
	}

	void reserve(size_t capa);

	void* operator [](size_t i){
		XTAL_ASSERT(i<size());
		return minusp(current_, i);
	}

	const void* operator [](size_t i) const{
		XTAL_ASSERT(i<size());
		return minusp(current_, i);
	}

	void* reverse_at(size_t i){
		XTAL_ASSERT(i<size());
		return plusp(begin_, i);
	}

	const void* reverse_at(size_t i) const{
		XTAL_ASSERT(i<size());
		return plusp(begin_, i);
	}

	void* reverse_at_unchecked(size_t i){
		return plusp(begin_, i);
	}

	const void* reverse_at_unchecked(size_t i) const{
		return plusp(begin_, i);
	}

	bool empty() const{
		XTAL_ASSERT(begin_<=plusp(current_, 1));
		return begin_==plusp(current_, 1);
	}

	void clear(){
		current_ = minusp(begin_, 1);
	}

	void release(){
		deallocate(minusp(begin_, 1));
		begin_ = plusp(dummy_allocate(), 1);
		current_ = minusp(begin_, 1);
		end_ = begin_;
	}
};

/*
* POD専用に書き下ろしたスタック
*/
template<class T>
class PODStack{

	PODStackBase impl_;

public:

	PODStack():impl_(sizeof(T)){}

public:

	typedef T value_type;
	typedef T* iterator;
	typedef const T* const_iterator; 

public:

	iterator begin(){ return (iterator)impl_.begin(); }	
	const_iterator begin() const{ return (iterator)impl_.begin(); }
	iterator end(){ return (iterator)impl_.end(); }	
	const_iterator end() const{ return (iterator)impl_.end(); }		
	void push_unchecked(const T &val){ impl_.push_unchecked(&val); }
	void push(const T &val){ impl_.push(&val); }
	T& push_unchecked(){ return *(T*)impl_.push_unchecked(); }
	T& push(){ return *(T*)impl_.push(); }
	T& pop(){ return *(T*)impl_.pop(); }
	T& top(){ return *(T*)impl_.top(); }
	const T &top() const{ return *(const T*)impl_.top(); }
	void resize(size_t newsize){ impl_.resize(newsize); }
	void downsize(size_t ds){ impl_.downsize(ds); }
	void downsize_n(size_t newsize){ impl_.downsize_n(newsize); }
	void upsize_unchecked(size_t us){ impl_.upsize_unchecked(us); }
	void upsize(size_t us){ impl_.upsize(us); }
	void erase(size_t i){ impl_.erase(i); }
	size_t size() const{ return impl_.size(); }
	size_t capacity() const{ return impl_.capacity(); }
	void reserve(size_t capa){ impl_.reserve(capa); }
	T& operator [](size_t i){ return *(T*)impl_[i]; }
	const T& operator [](size_t i) const{ return *(const T*)impl_[i]; }
	T& reverse_at(size_t i){ return *(T*)impl_.reverse_at(i); }
	const T& reverse_at(size_t i) const{ return *(const T*)impl_.reverse_at(i); }
	T& reverse_at_unchecked(size_t i){ return *(T*)impl_.reverse_at_unchecked(i); }
	const T& reverse_at_unchecked(size_t i) const{ return *(const T*)impl_.reverse_at_unchecked(i); }
	bool empty() const{ return impl_.empty();}
	void clear(){ impl_.clear(); }
	void release(){ impl_.release(); }
};

}
