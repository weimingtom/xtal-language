
#pragma once

namespace xtal{

void* stack_dummy_allocate();

void* stack_allocate(size_t size);

void stack_deallocate(void* p);

/*
* VMachineクラスが使う色々なスタック用に、特定の操作の実行速度を重視して実装したスタックコンテナ。
*
* 速度を稼ぐため、capacity分のオブジェクトは常に生成された状態となっている。
* sizeが減ったり増えたりしたからといって、コンストラクタ、デストラクタを呼んだりはしない。
* そのため、pop関数はポップされたオブジェクトの参照を返しているが、これは安全である。
*/
template<class T>
class FastStack{

	T* begin_; // 確保したメモリの先頭の次を指す。
	T* end_; // 確保したメモリの一番最後の次を指す
	T* current_; // スタックトップの要素を指す

private:

	static void* dummy_allocate(){
		return stack_dummy_allocate();
	}

	static void* allocate(size_t size){
		return stack_allocate(size);
	}

	static void deallocate(void* p){
		stack_deallocate(p);
	}

	void upsize_detail(size_t us);

public:

	FastStack();

	FastStack(const FastStack<T>& a);
	
	FastStack<T> &operator =(const FastStack<T>& a);

	~FastStack();

public:
	
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

	void insert(size_t i, const T& v){
		upsize(1);
		for(size_t j = 0; j != i; ++j){
			(*this)[j] = (*this)[j+1];
		}
		(*this)[i] = v;
	}
	
	void resize(size_t newsize);

	void downsize(size_t ds){
		XTAL_ASSERT(size()>=ds);
		current_-=ds;
	}

	void downsize_n(size_t newsize){
		XTAL_ASSERT(newsize<=size());
		current_ = begin_-1+newsize;
	}

	void upsize_unchecked(size_t us){
		XTAL_ASSERT(current_+us>current_);
		current_+=us;
	}

	void upsize(size_t us){		
		XTAL_ASSERT(current_+us>current_);
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
	
	void push(const FastStack<T>& other, size_t sz){
		for(size_t i = sz; i!=0; --i){
			push(other[i-1]);
		} 
	}
	
	void push(const FastStack<T>& other, size_t other_offset, size_t sz){
		for(size_t i = sz; i!=0; --i){
			push(other[other_offset+i-1]);
		} 
	}
		
	void move(FastStack<T>& other, size_t sz){
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
FastStack<T>::FastStack(){
	begin_=(T*)dummy_allocate()+1;
	current_ = begin_-1;
	end_ = begin_+0;
}

template<class T>
FastStack<T>::~FastStack(){
	for(size_t i = 0, last = capacity(); i<last; ++i){
		begin_[i].~T();
	}
	deallocate(begin_-1);
}

template<class T>
FastStack<T>::FastStack(const FastStack<T> &a){
	if(a.capacity()==0){
		begin_=(T*)dummy_allocate()+1;
		current_ = begin_-1;
		end_ = begin_+0;
	}
	else{
		begin_=(T*)allocate(sizeof(T)*(a.capacity()+1))+1;
		current_ = begin_+a.size()-1;
		end_ = begin_+a.capacity();

		size_t capa = a.capacity();
		for(size_t i = 0; i<capa; ++i){
			new(&begin_[i]) T(a.begin_[i]);
		}
	}
}

template<class T>
FastStack<T> &FastStack<T>::operator =(const FastStack<T> &a){
	if(this==&a){
		return *this;
	}
	
	if(a.capacity()==0){
		for(size_t i = 0, last = capacity(); i<last; ++i){
			begin_[i].~T();
		}
		deallocate(begin_-1);

		begin_=(T*)dummy_allocate()+1;
		current_ = begin_-1;
		end_ = begin_+0;
	}
	else{
		T* newp = (T*)allocate(sizeof(T)*(a.capacity()+1))+1;

		for(size_t i = 0, last = capacity(); i<last; ++i){
			begin_[i].~T();
		}
		deallocate(begin_-1);

		begin_ = newp;
		current_ = begin_+a.size()-1;
		end_ = begin_+a.capacity();

		size_t capa = a.capacity();
		for(size_t i = 0; i<capa; ++i){
			new(&begin_[i]) T(a.begin_[i]);
		}
	}

	return *this;
}

template<class T>
void FastStack<T>::resize(size_t newsize){
	XTAL_ASSERT(newsize>=0);

	size_t oldsize = size();
	if(newsize>oldsize){
		upsize(newsize-oldsize);
	}
	else{
		downsize_n(newsize);
	}
}

template<class T>
void FastStack<T>::upsize_detail(size_t us){
	XTAL_ASSERT(us>=0);

	current_-=us;

	size_t oldsize = size();
	size_t oldcapa = capacity();
	size_t newsize = oldsize+us;
	T* oldp = begin_;
	size_t newcapa = 16 + newsize + newsize/2;
	T* newp = (T*)allocate(sizeof(T)*(newcapa+1))+1;

	for(size_t i = 0; i<oldsize; ++i){
		new(&newp[i]) T(oldp[i]);
	}

	for(size_t i = oldsize; i<newcapa; ++i){
		new(&newp[i]) T;
	}

	for(size_t i = 0; i<oldcapa; ++i){
		oldp[i].~T();
	}

	deallocate(oldp-1);

	begin_ = newp;
	current_ = begin_+newsize-1;
	end_ = begin_+newcapa;
}


template<class T>
void FastStack<T>::reserve(size_t capa){
	if(capa<=capacity()){
		return;
	}

	size_t diff = capa-size();
	upsize(diff);
	downsize(diff);
}

template<class T>
void visit_members(Visitor& m, const FastStack<T>& value){
	for(int_t i = 0, size = value.capacity(); i<size; ++i){
		m & value.reverse_at_unchecked(i);
	}
}

/*
* POD専用に書き下ろしたスタックのベース部分
*/
class PODStackBase{
protected:

	void* begin_; // 確保したメモリの先頭の次を指す。
	void* end_; // 確保したメモリの一番最後の次を指す
	void* current_; // スタックトップの要素を指す
	int one_size_;

private:

	static void* dummy_allocate(){
		return stack_dummy_allocate();
	}

	static void* allocate(size_t size){
		return stack_allocate(size);
	}

	static void deallocate(void* p){
		stack_deallocate(p);
	}

	void* plusp(const void* p, size_t v) const{
		return (u8*)p + v*one_size_;
	}
	
	void* minusp(const void* p, size_t v) const{
		return (u8*)p - v*one_size_;
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

	void push(const void* val){
		upsize(1);
		std::memcpy(top(), val, one_size_);
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
		
	void erase(size_t i, size_t n=1);

	void insert(size_t i, const void* v, size_t n=1);

	void reverse_erase(size_t i, size_t n=1);

	void reverse_insert(size_t i, const void* v, size_t n=1);

	void resize(size_t newsize);

	void downsize(size_t ds){
		XTAL_ASSERT(size()>=ds);
		subp(current_, ds);
	}

	void downsize_n(size_t newsize){
		current_ = plusp(begin_, -1+newsize);
	}

	void upsize(size_t us){		
		addp(current_, us);
		if(current_>=end_){
			upsize_detail(us);
		}
	}

	size_t size() const{
		return ((u8*)plusp(current_, 1) - (u8*)begin_)/one_size_;
	}

	size_t capacity() const{
		return ((u8*)end_ - (u8*)begin_)/one_size_;
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

	bool empty() const{
		XTAL_ASSERT(begin_<=plusp(current_, 1));
		return begin_==plusp(current_, 1);
	}

	void clear(){
		current_ = minusp(begin_, 1);
	}

	void release();

public:

	void attach(void* p);

	void detach();
};

/*
* POD専用に書き下ろしたスタック
* TをPOD以外にすると未定義の動作となる
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
	
	void push(const T &val){ impl_.push(&val); }
	T& push(){ return *(T*)impl_.push(); }
	T& pop(){ return *(T*)impl_.pop(); }
	T& top(){ return *(T*)impl_.top(); }
	const T &top() const{ return *(const T*)impl_.top(); }
	void resize(size_t newsize){ impl_.resize(newsize); }
	void downsize(size_t ds){ impl_.downsize(ds); }
	void downsize_n(size_t newsize){ impl_.downsize_n(newsize); }
	void upsize(size_t us){ impl_.upsize(us); }
	void insert(size_t i, const T& val){ impl_.insert(i, &val); }
	void insert(size_t i, const T* val, size_t n){ impl_.insert(i, val, n); }
	void reverse_erase(size_t i, size_t n){ impl_.reverse_erase(i, n); }
	void reverse_insert(size_t i, const T& val){ impl_.reverse_insert(i, &val); }
	void reverse_insert(size_t i, const T* val, size_t n){ impl_.reverse_insert(i, val, n); }
	void erase(size_t i, size_t n){ impl_.erase(i, n); }
	size_t size() const{ return impl_.size(); }
	size_t capacity() const{ return impl_.capacity(); }
	void reserve(size_t capa){ impl_.reserve(capa); }
	T& operator [](size_t i){ return *(T*)impl_[i]; }
	const T& operator [](size_t i) const{ return *(const T*)impl_[i]; }
	T& reverse_at(size_t i){ return *(T*)impl_.reverse_at(i); }
	const T& reverse_at(size_t i) const{ return *(const T*)impl_.reverse_at(i); }
	bool empty() const{ return impl_.empty(); }
	void clear(){ impl_.clear(); }
	void release(){ impl_.release(); }

public:
	void attach(T* p){ impl_.attach(p); }
	void detach(){ impl_.detach(); }
};

/*
* 非PODもOKなスタックのベース
*/
class StackBase{
	PODStackBase impl_;
	void (*ctor_)(void* p);
	void (*copy_ctor_)(void* p, const void* q);
	void (*dtor_)(void* p);

public:

	StackBase(size_t onesize,
		void (*ctor)(void* p),
		void (*copy_ctor)(void* p, const void* q),
		void (*dtor)(void* p));

	StackBase(const StackBase& v);

	StackBase& operator =(const StackBase& v);

	~StackBase();

public:

	void push(const void* val){
		copy_ctor_(impl_.push(), val);
	}

	void* push(){ 
		ctor_(impl_.push());
		return top();
	}
	
	void pop(){ 
		dtor_(impl_.pop());
	}
		
	void erase(size_t i);
		
	void insert(size_t i, const void* v);

	void resize(size_t newsize);

	void downsize(size_t ds);

	void downsize_n(size_t newsize){
		downsize(size()-newsize);
	}

	void upsize(size_t us);

	void clear(){
		downsize_n(0);
	}

	void release();

	void* top(){ return impl_.top(); }
	const void* top() const{ return impl_.top(); }
	size_t size() const{ return impl_.size(); }
	size_t capacity() const{ return impl_.capacity(); }
	void reserve(size_t capa){ impl_.reserve(capa); }
	void* operator [](size_t i){ return impl_[i]; }
	const void* operator [](size_t i) const{ return impl_[i]; }
	void* reverse_at(size_t i){ return impl_.reverse_at(i); }
	const void* reverse_at(size_t i) const{ return impl_.reverse_at(i); }
	bool empty() const{ return impl_.empty(); }
};

/*
* 非PODもOKなスタック
* コードの複製が抑えられる分、速度が遅い
*/
template<class T>
class Stack{

	StackBase impl_;
	
	static void ctor(void* p){
		new(p) T();
	}
	
	static void copy_ctor(void* p, const void* q){
		new(p) T(*(T*)q);
	}
	
	static void dtor(void* p){
		((T*)p)->~T();
	}
	
public:

	Stack():impl_(sizeof(T), &ctor, &copy_ctor, &dtor){}

public:

	void push(const T &val){ impl_.push(&val); }
	T& push(){ return *(T*)impl_.push(); }
	void pop(){ impl_.pop(); }
	T& top(){ return *(T*)impl_.top(); }
	const T &top() const{ return *(const T*)impl_.top(); }
	void resize(size_t newsize){ impl_.resize(newsize); }
	void downsize(size_t ds){ impl_.downsize(ds); }
	void downsize_n(size_t newsize){ impl_.downsize_n(newsize); }
	void upsize(size_t us){ impl_.upsize(us); }
	void insert(size_t i, const T &val){ impl_.insert(i, &val); }
	void erase(size_t i){ impl_.erase(i); }
	size_t size() const{ return impl_.size(); }
	size_t capacity() const{ return impl_.capacity(); }
	void reserve(size_t capa){ impl_.reserve(capa); }
	T& operator [](size_t i){ return *(T*)impl_[i]; }
	const T& operator [](size_t i) const{ return *(const T*)impl_[i]; }
	T& reverse_at(size_t i){ return *(T*)impl_.reverse_at(i); }
	const T& reverse_at(size_t i) const{ return *(const T*)impl_.reverse_at(i); }
	bool empty() const{ return impl_.empty(); }
	void clear(){ impl_.clear(); }
	void release(){ impl_.release(); }
};

/*
* スタックを用いて配列を作る。
* 実装を再利用してコードの膨張を押さえるための策
*/
template<class T, class TStack = Stack<T> >
class ArrayList{
	TStack impl_;
	
public:
	
	ArrayList(){}
	
public:
	
	void push_back(const T &val){ impl_.push(val); }
	T& push_back(){ return impl_.push(); }
	void pop_back(){ impl_.pop(); }
	T& back(){ return impl_.top(); }
	void resize(size_t newsize){ impl_.resize(newsize); }
	void downsize(size_t ds){ impl_.downsize(ds); }
	void downsize_n(size_t newsize){ impl_.downsize_n(newsize); }
	void upsize(size_t us){ impl_.upsize(us); }
	void insert(size_t i, const T *val, int_t n){ impl_.reverse_insert(i, val, n); }
	void insert(size_t i, const T &val){ impl_.reverse_insert(i, val); }
	void erase(size_t i, size_t n){ impl_.reverse_erase(i, n); }
	size_t size() const{ return impl_.size(); }
	size_t capacity() const{ return impl_.capacity(); }
	void reserve(size_t capa){ impl_.reserve(capa); }
	T& operator [](size_t i){ return impl_.reverse_at(i); }
	const T& operator [](size_t i) const{ return impl_.reverse_at(i); }
	bool empty() const{ return impl_.empty(); }
	void clear(){ impl_.clear(); }
	void release(){ impl_.release(); }

};
	
template<class T>
class PODArrayList : public ArrayList<T, PODStack<T> >{
	
};
	
}

