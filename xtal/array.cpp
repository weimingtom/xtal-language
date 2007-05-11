
#include "xtal.h"

#include "arrayimpl.h"

namespace xtal{

static void InitArrayIterImpl(){
	TClass<ArrayImpl::ArrayIterImpl> p("ArrayIter");
	p.inherit(Iterator());
	p.method("iter_first", &ArrayImpl::ArrayIterImpl::iter_first);
	p.method("iter_next", &ArrayImpl::ArrayIterImpl::iter_next);
}

void InitArray(){
	InitArrayIterImpl();

	TClass<Array> p("Array");
	p.inherit(Enumerable());

	p.def("new", New<Array, int_t>().param(Named("size", 0)));
	p.method("size", &Array::size);
	p.method("length", &Array::length);
	p.method("resize", &Array::resize);
	p.method("empty", &Array::empty);
	p.method("at", &Array::at);
	p.method("set_at", &Array::set_at);
	p.method("op_at", &Array::at);
	p.method("op_set_at", &Array::set_at);
	p.method("slice", &Array::slice);
	p.method("pop_back", &Array::pop_back);
	p.method("push_back", &Array::push_back);
	p.method("pop_front", &Array::pop_front);
	p.method("push_front", &Array::push_front);
	p.method("erase", &Array::erase);
	p.method("insert", &Array::insert);
	p.method("cat", &Array::cat);
	p.method("cat_assign", &Array::cat_assign);
	p.method("op_cat", &Array::cat);
	p.method("op_cat_assign", &Array::cat_assign);
	p.method("to_s", &Array::to_s);
	p.method("to_a", &Array::to_a);
	p.method("join", &Array::join).param(Named("sep"));
	p.method("each", &Array::each);
	p.method("clone", &Array::clone);
	p.method("front", &Array::front);
	p.method("back", &Array::back);
	p.method("clear", &Array::clear);
}

Array::Array(int_t size)
	:Any(null){
	new(*this) ArrayImpl(size);
}

Array::Array(check_xtype<int>::type size)
	:Any(null){
	new(*this) ArrayImpl(size);
}

int_t Array::size() const{
	return impl()->size();
}

void Array::resize(int_t sz) const{
	impl()->resize(sz);
}

int_t Array::length() const{
	return impl()->size();
}

const Any& Array::at(int_t i) const{
	return impl()->at(i);
}

void Array::set_at(int_t i, const Any& v) const{
	impl()->set_at(i, v);
}

void Array::push_front(const Any& v) const{
	impl()->push_front(v);
}

void Array::pop_front() const{
	impl()->pop_front();
}

void Array::push_back(const Any& v) const{
	impl()->push_back(v);
}

void Array::pop_back() const{
	impl()->pop_back();
}

Array Array::slice(int_t first, int_t last) const{
	return impl()->slice(first, last);
}

void Array::erase(int_t i) const{
	impl()->erase(i);
}

void Array::insert(int_t i, const Any& v) const{
	impl()->insert(i, v);
}

Array Array::clone() const{
	return impl()->clone();
}

Array Array::cat(const Array& a) const{
	return impl()->cat(a);
}

Array Array::cat_assign(const Array& a) const{
	return impl()->cat_assign(a);
}

String Array::join(const String& sep) const{
	return impl()->join(sep);
}

String Array::to_s() const{
	return impl()->to_s();
}

Array Array::to_a() const{
	return *this;
}

Any Array::each() const{
	return impl()->each();
}

bool Array::empty() const{
	return impl()->empty();
}
	
const Any& Array::front() const{
	return impl()->at(0);
}

const Any& Array::back() const{
	return impl()->at(-1);
}

void Array::clear() const{
	return impl()->clear();
}

}
