
#pragma once

#include "string.h"

namespace xtal{

class Visitor{
	int_t value_;
public:	

	Visitor(int_t value){
		value_ = value;
	}

	int_t value(){
		return value_;
	}

	template<class T> Visitor operator &(const T& value){
		visit_members(*this, value);
		return *this;
	}

	template<class T> Visitor operator ()(const T& value){
		visit_members(*this, value);
		return *this;
	}
};

template<class Key, class T, class Pr, class A>
void visit_members(Visitor& m, const std::map<Key, T, Pr, A>& value){
	std::for_each(value.begin(), value.end(), m);
}

template<class T, class A>
void visit_members(Visitor& m, const std::vector<T, A>& value){
	std::for_each(value.begin(), value.end(), m);
}

template<class F, class S>
void visit_members(Visitor& m, const std::pair<F, S>& value){
	m & value.first & value.second;
}

inline void visit_members(Visitor& m, const Named& p){
	m & p.name & p.value;
}

}

