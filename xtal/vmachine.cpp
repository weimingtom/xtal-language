
#include "xtal.h"

#include "vmachineimpl.h"

namespace xtal{

void InitArguments(){
	TClass<Arguments> p("Arguments");
	p.method("length", &Arguments::length);
	p.method("op_at", &Arguments::op_at);
	p.method("each_ordered_arg", &Arguments::each_ordered_arg);
	p.method("each_named_arg", &Arguments::each_named_arg);
}

Arguments::Arguments(){
	new(*this) ArgumentsImpl();
}

const Any& Arguments::op_at(const Any& index) const{
	return impl()->op_at(index);
}
	
int_t Arguments::length() const{
	return impl()->length();
}
	
Any Arguments::each_ordered_arg() const{
	return impl()->each_ordered_arg();
}
	
Any Arguments::each_named_arg() const{
	return impl()->each_named_arg();
}

VMachine::VMachine(){
	new(*this) VMachineImpl();
}

void VMachine::setup_call(int_t need_result_count) const{
	impl()->setup_call(need_result_count);
}

void VMachine::setup_call(int_t need_result_count, const Any& a1) const{
	impl()->setup_call(need_result_count, a1);
}

void VMachine::setup_call(int_t need_result_count, const Any& a1, const Any& a2) const{
	impl()->setup_call(need_result_count, a1, a2);
}

void VMachine::setup_call(int_t need_result_count, const Any& a1, const Any& a2, const Any& a3) const{
	impl()->setup_call(need_result_count, a1, a2, a3);
}

void VMachine::setup_call(int_t need_result_count, const Any& a1, const Any& a2, const Any& a3, const Any& a4) const{
	impl()->setup_call(need_result_count, a1, a2, a3, a4);
}
	
void VMachine::set_call_flags(int_t flags) const{
	impl()->set_call_flags(flags);
}

void VMachine::push_arg(const Any& value) const{
	impl()->push_arg(value);
}

void VMachine::push_arg(const ID& name, const Any& value) const{
	impl()->push_arg(name, value);
}
	
const Any& VMachine::result(int_t pos) const{
	return impl()->result(pos);
}

Any VMachine::result_and_cleanup_call(int_t pos) const{
	return impl()->result_and_cleanup_call(pos);
}

void VMachine::cleanup_call() const{
	impl()->cleanup_call();
}

void VMachine::set_arg_this(const Any& self) const{
	impl()->set_arg_this(self);
}

void VMachine::set_hint(const Any& hint1, const String& hint2) const{
	impl()->set_hint(hint1, hint2);
}

const Any& VMachine::arg(int_t pos) const{
	return impl()->arg(pos);
}

const Any& VMachine::arg(const ID& name) const{
	return impl()->arg(name);
}

const Any& VMachine::arg(int_t pos, const ID& name) const{
	return impl()->arg(pos, name);
}

const Any& VMachine::arg_default(int_t pos, const Any& def) const{
	return impl()->arg_default(pos, def);
}

const Any& VMachine::arg_default(const ID& name, const Any& def) const{
	return impl()->arg_default(name, def);
}

const Any& VMachine::arg_default(int_t pos, const ID& name, const Any& def) const{
	return impl()->arg_default(pos, name, def);
}	

const Any& VMachine::get_arg_this() const{
	return impl()->get_arg_this();
}

const ID& VMachine::arg_name(int_t pos) const{
	return impl()->arg_name(pos);
}

int_t VMachine::ordered_arg_count() const{
	return impl()->ordered_arg_count();
}

int_t VMachine::named_arg_count() const{
	return impl()->named_arg_count();
}

bool VMachine::need_result() const{
	return impl()->need_result();
}

void VMachine::return_result() const{
	impl()->return_result();
}

void VMachine::return_result(const Any& value1) const{
	impl()->return_result(value1);
}

void VMachine::return_result(const Any& value1, const Any& value2) const{
	impl()->return_result(value1, value2);
}

void VMachine::return_result(const Any& value1, const Any& value2, const Any& value3) const{
	impl()->return_result(value1, value2, value3);
}

void VMachine::return_result(const Any& value1, const Any& value2, const Any& value3, const Any& value4) const{
	impl()->return_result(value1, value2, value3, value4);
}

void VMachine::return_result_array(const Array& values) const{
	impl()->return_result(values);
}


bool VMachine::processed() const{
	return impl()->processed();
}

void VMachine::replace_result(int_t pos, const Any& v) const{
	impl()->replace_result(pos, v);
}

void VMachine::recycle_call() const{
	impl()->recycle_call();
}

void VMachine::recycle_call(const Any& a1) const{
	impl()->recycle_call(a1);
}

void VMachine::push(const Any& v) const{
	impl()->push(v);
}

void VMachine::reset() const{
	impl()->reset();
}

}
