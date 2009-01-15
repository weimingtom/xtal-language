
#pragma once

namespace xtal{

/** @addtogroup memory */
/*@{*/

/**
* @brief ユーザーが登録したメモリアロケート関数を使ってメモリ確保する。
*
* メモリ確保失敗は例外で返される。
*/
void* user_malloc(size_t size);

/**
* @brief ユーザーが登録したメモリアロケート関数を使ってメモリ確保する。
*
* メモリ確保失敗はNULL値で返される。
*/
void* user_malloc_nothrow(size_t size);

/**
* @brief ユーザーが登録したメモリデアロケート関数を使ってメモリ解放する。
*
*/
void user_free(void* p);

void* so_malloc(size_t size);

void so_free(void* p, size_t size);

/**
* @brief 使用するメモリアロケート関数を設定する。
*
*/
void set_user_malloc(void* (*malloc)(size_t), void (*free)(void*));

/**
* @brief 使用するメモリーを設定する。
*
*/
//void set_memory(void* memory, size_t size);

void initialize_memory();

void release_memory();

/**
* @brief 動的なポインタの配列を作成、拡張する関数。
*
* 一番最初の作成時は、引数全てがnullである必要がある。
* @param begin メモリの先頭
* @param end メモリの最後の一つ次
* @param current 使用中の要素の一つ次
*/
void expand_simple_dynamic_pointer_array(void**& begin, void**& end, void**& current, int addsize=1024);

/**
* @brief expand_simple_dynamic_pointer_arrayで生成した配列を、使われている実際のサイズまで縮小する。
*
* 要素が一つも無いなら(current==beginなら)完全に解放される。
* @param begin メモリの先頭
* @param end メモリの最後の一つ次
* @param current 使用中の要素の一つ次
*/
void fit_simple_dynamic_pointer_array(void**& begin, void**& end, void**& current);

/**
* @brief user_malloc, user_freeを使う、STLの要件に適合したアロケータクラス。
*
*/
template<class T>
struct Alloc{

    typedef size_t size_type;
    typedef ptrdiff_t difference_type;
    typedef T* pointer;
    typedef const T* const_pointer;
    typedef T &reference;
    typedef const T &const_reference;
    typedef T value_type;

	template<class U>
	struct rebind{ typedef Alloc<U> other; };

    Alloc(){}
	template<class U>
	Alloc(const Alloc<U>&){}
	template<class U>
	Alloc<T>& operator=(const Alloc<U>&){ return* this; }
	Alloc(const Alloc<T>&){}
    Alloc<T>& operator=(const Alloc<T>&){ return* this; }

    pointer address(reference x) const{ return &x; }
    const_pointer address(const_reference x) const{ return &x; }

    pointer allocate(size_type n, const void* = 0){ return static_cast<pointer>(user_malloc(sizeof(T)*n)); }
    void deallocate(pointer p, size_type n){ user_free(p); }
	
	void construct(pointer p, const T& val){ new(p) T(val); }
    void destroy(pointer p){ p->~T(); p = 0; }
	
    size_type max_size() const{ return 0xffffffffU/sizeof(T); }
};

template<class T, class U> 
inline bool operator==(const Alloc<T>&, const Alloc<U>&){	
	return true;
}

template<class T, class U> 
inline bool operator!=(const Alloc<T>&, const Alloc<U>&){	
	return false;
}

template<>
struct Alloc<void> {
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;
    typedef void* pointer;
    typedef const void* const_pointer;
    typedef void value_type;

	template<class U>
	struct rebind{ typedef Alloc<U> other; };

    Alloc(){}
	Alloc(const Alloc<void> &){}
    Alloc<void>& operator=(const Alloc<void>&){ return* this; }
	template<class U>
	Alloc(const Alloc<U>&){}
	template<class U>
	Alloc<void>& operator=(const Alloc<U>&){ return* this; }
};

struct UserMallocGuard{
	void* p;
	UserMallocGuard():p(0){}
	UserMallocGuard(uint_t size):p(user_malloc(size)){}
	~UserMallocGuard(){ user_free(p); }
	
	void malloc(size_t size){ user_free(p); p = user_malloc(size); }
	void* get(){ return p; }
	void* release(){ void* ret = p; p = 0; return ret; }
private:
	UserMallocGuard(const UserMallocGuard&);
	void operator =(const UserMallocGuard&);
};

namespace detail{
	struct AC_default{};

	template<class>
	struct AC_If{
		template<class T, class U>
		struct inner{ typedef U type; };
	};
	
	template<>
	struct AC_If<AC_default>{
		template<class T, class U>
		struct inner{ typedef T type; };
	};
	
	template<class T, class Then, class Else>
	struct AC_IfDefault{
		typedef typename AC_If<T>::template inner<Then, Else>::type type;
	};
}


/**
* Allocクラスを使ったSTLコンテナを使いやすくするためのユーティリティ
* 
* Alloc-Container の略
* 
* AC<int>::vector は std::vector<int, Alloc<int> > と同じ
* AC<int, float>::map は std::map<int, float, std::less<int>, Alloc<std::pair<const int, float> > > と同じ
* AC<int, float, Comp>::map は std::map<int, float, Comp, Alloc<std::pair<const int, float> > > と同じ
*/
template<class FIRST, class SECOND = detail::AC_default, class THIRD = detail::AC_default>
struct AC{
	typedef std::vector<FIRST, Alloc<FIRST> > vector;
	typedef std::deque<FIRST, Alloc<FIRST> > deque;
	typedef std::list<FIRST, Alloc<FIRST> > list;
	typedef typename detail::AC_IfDefault<SECOND,
		std::set<FIRST, std::less<FIRST>, Alloc<FIRST> >,
		std::set<FIRST, SECOND, Alloc<FIRST> >
		>::type set;
	typedef typename detail::AC_IfDefault<SECOND,
		std::basic_string<FIRST, std::char_traits<FIRST>, Alloc<FIRST> >,
		std::basic_string<FIRST, SECOND, Alloc<FIRST> >
		>::type string;
	typedef typename detail::AC_IfDefault<THIRD,
		std::map<FIRST, SECOND, std::less<FIRST>, Alloc<std::pair<const FIRST, SECOND> > >,
		std::map<FIRST, SECOND, THIRD, Alloc<std::pair<const FIRST, SECOND> > >
		>::type map;
	typedef typename detail::AC_IfDefault<THIRD,
		std::multimap<FIRST, SECOND, std::less<FIRST>, Alloc<std::pair<const FIRST, SECOND> > >,
		std::multimap<FIRST, SECOND, THIRD, Alloc<std::pair<const FIRST, SECOND> > >
		>::type multimap;
};

/**
* @brief メモリ管理機構
*
*/
class RBTreeAllocator{
	class Node{
	public:
		Node* next;
		Node* prev;

		Node* left;
		Node* right;

		uint_t flag;

		enum{
			COLOR_RED = 1<<31
		};

		uint_t used(){ return flag&~COLOR_RED; }
		bool red(){ return (flag&COLOR_RED)!=0; }
		void set_red(){ flag|=COLOR_RED; }
		void set_black(){ flag&=~COLOR_RED; }
		void set_same_color(Node* a){ if(a->red()){ set_red(); }else{ set_black(); } }
		void flip(){ flag^=COLOR_RED; }
		size_t size(){ return (u8*)next - (u8*)buf(); }
		void* buf(){ return this+1; }
	};

public:

	RBTreeAllocator();
	
	void init(void* begin, void* end);

	void* malloc(size_t size);

	void free(void* p);

	void release();

	void debug_print();

private:
	
	void* malloc(Node* node, size_t size);

	void insert(Node* newnode);

	Node* insert(Node* h , Node* newnode);

	void erase(Node* erasenode);

	Node* erase(Node* h , Node* erasenode);

	Node* erase_min(Node* h);

	Node* move_red_left(Node* h);

	Node* move_red_right(Node* h);

	Node* fixup(Node* h);

	Node* rotate_left(Node* h);

	Node* rotate_right(Node* h);
	
	bool right_leaning(Node* h){
		return h==end_ ? false : h->right->red() && !h->left->red();
	}

	void flip(Node* h){
		XTAL_ASSERT(h!=end_);
		XTAL_ASSERT(h->left!=end_);
		XTAL_ASSERT(h->right!=end_);
		h->flip();
		h->left->flip();
		h->right->flip();
	}

private:

	Node* begin(){ return begin_; }
	Node* end(){ return end_; }

	Node* to_node(void* p){ return (Node*)p-1; }
	void add_ref(void* p){ to_node(p)->flag++; }
	
private:

	Node* head_;
	Node* begin_;
	Node* end_;
	Node* root_;

	uint_t count_;
};

class FixedAllocator{

	struct Chunk{

		typedef void* data_t;

		uint_t blocks_;
		uint_t blocks_available_;
		data_t* first_available_block_;
		Chunk* next_;
		//data_t buf_[blocks_];

		void init(uint_t blocks,uint_t block_size);

		void* malloc();

		void free(void* p);

		data_t* buf(){
			return reinterpret_cast<data_t*>(this+1);
		}
	};

public:
	
	typedef Chunk::data_t data_t;

private:

	int_t blocks_;
	data_t* free_data_;
	Chunk* first_chunk_;
	Chunk* last_chunk_;

public:

	FixedAllocator();

	void init(size_t block_size);

	void* malloc(size_t block_size);

	void free(void* p, size_t block_size);

	void release(size_t block_size);

private:

	void add_chunk(size_t block_size);

	void* malloc_impl(size_t block_size);

	void free_impl(void* p, size_t block_size);
};

class SmallObjectAllocator{	

	typedef FixedAllocator::data_t data_t;

public:

	enum{
		POOL_SIZE = 32,
		HANDLE_MAX_SIZE = POOL_SIZE*sizeof(data_t)
	};

public:

	void init();
	
	void* malloc(size_t size);

	void free(void* p, size_t size);

	void release();

private:

	FixedAllocator pool_[POOL_SIZE];
};

/*@}*/


}//namespace 
