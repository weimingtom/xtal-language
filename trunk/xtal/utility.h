
#pragma once

#include "allocator.h"

#include <cassert>
#include <stddef.h>
#include <time.h>
#include <iostream>
#include <vector>
#include <deque>
#include <list>
#include <map>
#include <set>

#if defined(_DEBUG) || defined(DEBUG)
#	define XTAL_DEBUG
#endif

#ifdef XTAL_DEBUG
# define XTAL_ASSERT(expr) assert(expr)
#else
# define XTAL_ASSERT(expr)
#endif

#define XTAL_ASSERT_TYPE_CHECK(expr, type) XTAL_ASSERT_1(::xtal::cast<type*>(expr), #type)

#ifdef XTAL_DEBUG
#	define XTAL_NODEFAULT default: XTAL_ASSERT(false)
#else
#	ifdef WIN32 
#		define XTAL_NODEFAULT default: __assume(0)
#	else
#		define XTAL_NODEFAULT default:
#	endif
#endif

#define XTAL_DEFAULT default:
#define XTAL_CASE(key) break; case key:
#define XTAL_CASE1(key) break; case key:
#define XTAL_CASE2(key, key2) break; case key:case key2:
#define XTAL_CASE3(key, key2, key3) break; case key:case key2:case key3:
#define XTAL_CASE4(key, key2, key3, key4) break; case key:case key2:case key3:case key4:

#define XTAL_GLOBAL_INTERPRETER_LOCK if(GlobalInterpreterLock global_interpreger_lock = 0)
#define XTAL_GLOBAL_INTERPRETER_UNLOCK if(GlobalInterpreterUnlock global_interpreger_unlock = 0)
#define XTAL_UNLOCK if(XUnlock xxunlock = 0)

namespace xtal{

template<class T>
inline T align(T v, int N){
	return (v+(N-1)) & ~(N-1);
}

template<class T>
inline T* align_p(T* v, int N){
	return (T*)align((unsigned long)v, N);
}

template<class T>
inline T align_2(T v){
	v-=1;
	v|=v>>1;
	v|=v>>2;
	v|=v>>4;
	v|=v>>8;
	v|=v>>16;
	return v+1;
}

template<int N>
class static_count_bits{
	enum{
		N1 = (N & 0x55555555) + (N >> 1 & 0x55555555),
		N2 = (N1 & 0x33333333) + (N1 >> 2 & 0x33333333),
		N3 = (N2 & 0x0f0f0f0f) + (N2 >> 4 & 0x0f0f0f0f),
		N4 = (N3 & 0x00ff00ff) + (N3 >> 8 & 0x00ff00ff)
	};
public:	
	enum{
		value = (N4 & 0x0000ffff) + (N4>>16 & 0x0000ffff)	
	};
};

template<int N>
struct static_ntz{
	enum{
		value = static_count_bits<((N&(-N))-1)>::value
	};
};


// 最低限のメタプログラミング下地

template<bool>
struct IfHelper{
	template<class T, class U>
	struct inner{
		typedef T type;
	};
};

template<>
struct IfHelper<false>{
	template<class T, class U>
	struct inner{
		typedef U type;
	};
};

template<bool N, class T, class U>
struct If{
	typedef typename IfHelper<N>::template inner<T, U>::type type;
};

template<bool>
struct Bool{};

template<class T, class U>
struct Convertible{
	typedef char (&yes)[2];
	typedef char (&no)[1];
	static yes test(U);
	static no test(...);
	static T makeT();

	enum{ value = sizeof(test(makeT()))==sizeof(yes) };
};

template<int N>
struct I2T{
	enum{ value = N };
};

template<class T>
struct T2T{
	typedef T type;
};

template<class T>
struct IsConst{
	typedef char (&yes)[2];
	typedef char (&no)[1];
	template<class U>
	static yes test(const U (*)());
	static no test(...);

	enum{ value = sizeof(test((T (*)())0))==sizeof(yes) };
};

template<class T>
struct IsReference{
	typedef char (&yes)[2];
	typedef char (&no)[1];
	template<class U>
	static yes test(U& (*)());
	static no test(...);

	enum{ value = sizeof(test((T (*)())0))==sizeof(yes) };
};

template<class T>
struct IsNotVoid{ enum{ value = 1 }; };
template<>
struct IsNotVoid<void>{ enum{ value = 0 }; };


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
* Allocator-Container の略
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


#if _MSC_VER <= 1200
	typedef typename detail::AC_IfDefault<THIRD,
		std::map<FIRST, SECOND, std::less<FIRST>, Alloc<SECOND> >,
		std::map<FIRST, SECOND, THIRD, Alloc<SECOND> >
		>::type map;
#else
	typedef typename detail::AC_IfDefault<THIRD,
		std::map<FIRST, SECOND, std::less<FIRST>, Alloc<std::pair<const FIRST, SECOND> > >,
		std::map<FIRST, SECOND, THIRD, Alloc<std::pair<const FIRST, SECOND> > >
		>::type map;
#endif

	typedef typename detail::AC_IfDefault<THIRD,
		std::multimap<FIRST, SECOND, std::less<FIRST>, Alloc<std::pair<const FIRST, SECOND> > >,
		std::multimap<FIRST, SECOND, THIRD, Alloc<std::pair<const FIRST, SECOND> > >
		>::type multimap;
};

}
