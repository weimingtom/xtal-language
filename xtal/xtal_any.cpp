
#include <vector>
#include <iostream>
#include <fstream>

#include "xtal.h"

#include "xtal_fwd.h"
#include "xtal_any.h"
#include "xtal_frame.h"
#include "xtal_fun.h"
#include "xtal_string.h"
#include "xtal_code.h"
#include "xtal_lexer.h"
#include "xtal_frame.h"
#include "xtal_macro.h"
#include "xtal_stream.h"
#include "xtal_vmachine.h"

namespace xtal{

void InitAny(){
	ClassPtr p = new_cpp_class<Any>("Any");
	p->method("class", &Any::get_class);
	p->method("get_class", &Any::get_class);
	p->method("object_name", &Any::object_name);
}

Null null;
Nop nop;

AnyPtr operator +(const AnyPtr& a, const AnyPtr& b){ return a->send(Xid(op_add), b); }
AnyPtr operator -(const AnyPtr& a, const AnyPtr& b){ return a->send(Xid(op_sub), b); }
AnyPtr operator *(const AnyPtr& a, const AnyPtr& b){ return a->send(Xid(op_mul), b); }
AnyPtr operator /(const AnyPtr& a, const AnyPtr& b){ return a->send(Xid(op_div), b); }
AnyPtr operator %(const AnyPtr& a, const AnyPtr& b){ return a->send(Xid(op_mod), b); }
AnyPtr operator |(const AnyPtr& a, const AnyPtr& b){ return a->send(Xid(op_or), b); }
AnyPtr operator &(const AnyPtr& a, const AnyPtr& b){ return a->send(Xid(op_and), b); }
AnyPtr operator ^(const AnyPtr& a, const AnyPtr& b){ return a->send(Xid(op_xor), b); }
AnyPtr operator >>(const AnyPtr& a, const AnyPtr& b){ return a->send(Xid(op_shr), b); }
AnyPtr operator <<(const AnyPtr& a, const AnyPtr& b){ return a->send(Xid(op_shl), b); }
AnyPtr operator ==(const AnyPtr& a, const AnyPtr& b){ return raweq(a, b) || a->send(Xid(op_eq), b); }
AnyPtr operator !=(const AnyPtr& a, const AnyPtr& b){ return !(a==b); }
AnyPtr operator <(const AnyPtr& a, const AnyPtr& b){ return a->send(Xid(op_lt), b); }
AnyPtr operator >(const AnyPtr& a, const AnyPtr& b){ return b<a; }
AnyPtr operator <=(const AnyPtr& a, const AnyPtr& b){ return !(b<a); }
AnyPtr operator >=(const AnyPtr& a, const AnyPtr& b){ return !(a<b); }

AnyPtr& operator +=(AnyPtr& a, const AnyPtr& b){ a = a->send(Xid(op_add_assign), b); return a; }
AnyPtr& operator -=(AnyPtr& a, const AnyPtr& b){ a = a->send(Xid(op_sub_assign), b); return a; }
AnyPtr& operator *=(AnyPtr& a, const AnyPtr& b){ a = a->send(Xid(op_mul_assign), b); return a; }
AnyPtr& operator /=(AnyPtr& a, const AnyPtr& b){ a = a->send(Xid(op_div_assign), b); return a; }
AnyPtr& operator %=(AnyPtr& a, const AnyPtr& b){ a = a->send(Xid(op_mod_assign), b); return a; }
AnyPtr& operator |=(AnyPtr& a, const AnyPtr& b){ a = a->send(Xid(op_or_assign), b); return a; }
AnyPtr& operator &=(AnyPtr& a, const AnyPtr& b){ a = a->send(Xid(op_and_assign), b); return a; }
AnyPtr& operator ^=(AnyPtr& a, const AnyPtr& b){ a = a->send(Xid(op_xor_assign), b); return a; }
AnyPtr& operator >>=(AnyPtr& a, const AnyPtr& b){ a = a->send(Xid(op_shr_assign), b); return a; }
AnyPtr& operator <<=(AnyPtr& a, const AnyPtr& b){ a = a->send(Xid(op_shl_assign), b); return a; }


Innocence::Innocence(const char_t* str){
	*this = xnew<String>(str);
}

AnyPtr Innocence::operator()() const{
	const VMachinePtr& vm = vmachine();
	vm->setup_call(1);
	ap(*this)->call(vm);
	return vm->result_and_cleanup_call();
}

AnyPtr Any::send(const InternedStringPtr& name) const{
	const VMachinePtr& vm = vmachine();
	vm->setup_call(1);
	rawsend(vm, name);
	return vm->result_and_cleanup_call();
}
const AtProxy& AtProxy::operator =(const AnyPtr& value){
	obj->send(Xid(set_at), key, value);
	return *this;
}

AtProxy Innocence::operator[](const AnyPtr& key) const{
	return AtProxy(ap(*this), key);
}

AtProxy::operator const AnyPtr&(){
	return obj = obj->send(Xid(at), key);
}

const AnyPtr& AtProxy::operator ->(){
	return obj = obj->send(Xid(at), key);
}

const Any& AtProxy::operator *(){
	return *(obj = obj->send(Xid(at), key));
}

struct MemberCacheTable{
	struct Unit{
		int_t klass;
		int_t name;
		int_t ns;
		Innocence member;
		uint_t mutate_count;
	};

	enum{ CACHE_MAX = /*179*/ 256 };

	Unit table_[CACHE_MAX];
	uint_t hit_;
	uint_t miss_;

	MemberCacheTable(){
		for(int_t i=0; i<CACHE_MAX; ++i){
			table_[i].klass = 0;
			table_[i].member = null;
		}
		hit_ = 0;
		miss_ = 0;
	}

	float cache_hit_rate(){
		return (float_t)hit_/(hit_+miss_);
	}

	uint_t hit(){
		return hit_;
	}

	uint_t miss(){
		return miss_;
	}

	const AnyPtr& cache(const Innocence& target_class, const InternedStringPtr& member_name, const Innocence& self, const Innocence& nsp){

		uint_t klass = rawvalue(target_class);
		uint_t name = rawvalue(member_name);
		uint_t ns = rawvalue(nsp);

		uint_t hash = (klass>>3) + (name>>2) + (ns);
		Unit& unit = table_[hash/* % CACHE_MAX*/ & (CACHE_MAX-1)];
		if(global_mutate_count==unit.mutate_count && klass==unit.klass && name==unit.name && ns==unit.ns){
			hit_++;
			return ap(unit.member);
		}else{
			if(type(target_class)!=TYPE_BASE)
				return null;

			miss_++;
			unit.member = pvalue(target_class)->member(member_name, ap(self), ap(nsp));
			unit.klass = klass;
			unit.name = name;
			unit.ns = ns;
			unit.mutate_count = global_mutate_count;
			return ap(unit.member);
		}
	}
};

namespace{
	MemberCacheTable member_cache_table;
}

const AnyPtr& Any::member(const InternedStringPtr& name, const AnyPtr& self, const AnyPtr& ns) const{
	return member_cache_table.cache(*this, name, self, ns);
}

void Any::def(const InternedStringPtr& name, const AnyPtr& value, int_t accessibility, const AnyPtr& ns) const{
	switch(type(*this)){
		XTAL_DEFAULT;
		XTAL_CASE(TYPE_BASE){
			value->set_object_name(name, object_name_force(), ap(*this));
			pvalue(*this)->def(name, value, accessibility, ns);
		}
	}
}

void Any::rawsend(const VMachinePtr& vm, const InternedStringPtr& name, const AnyPtr& self, const AnyPtr& ns) const{
	const ClassPtr& cls = get_class();
	vm->set_hint(cls, name);
	if(const AnyPtr& ret = member_cache_table.cache(cls, name, self, ns)){
		vm->set_arg_this(ap(*this));
		ret->call(vm);
	}
}

void Any::call(const VMachinePtr& vm) const{
	switch(type(*this)){
		XTAL_DEFAULT{}
		XTAL_CASE(TYPE_BASE){ pvalue(*this)->call(vm); }
	}
}

int_t Any::to_i() const{
	switch(type(*this)){
		XTAL_DEFAULT{ return cast<int_t>((*this).send("to_i")); }
		XTAL_CASE(TYPE_NULL){ return 0; }
		XTAL_CASE(TYPE_INT){ return ivalue(*this); }
		XTAL_CASE(TYPE_FLOAT){ return (int_t)fvalue(*this); }
	}
	return 0;
}

float_t Any::to_f() const{
	switch(type(*this)){
		XTAL_DEFAULT{ return cast<float_t>((*this).send("to_f")); }
		XTAL_CASE(TYPE_NULL){ return 0; }
		XTAL_CASE(TYPE_INT){ return (float_t)ivalue(*this); }
		XTAL_CASE(TYPE_FLOAT){ return fvalue(*this); }
	}
	return 0;
}

StringPtr Any::to_s() const{
	if(StringPtr ret = as<StringPtr>(ap(*this))){
		return ret;
	}
	return cast<StringPtr>((*this).send("to_s"));
}

StringPtr Any::object_name() const{
	switch(type(*this)){
		XTAL_NODEFAULT;
		XTAL_CASE(TYPE_NULL){ return xnew<String>("instance of Null"); }
		XTAL_CASE(TYPE_BASE){ return pvalue(*this)->object_name(); }
		XTAL_CASE(TYPE_INT){ return xnew<String>("instance of Int"); }
		XTAL_CASE(TYPE_FLOAT){ return xnew<String>("instance of Float"); }
		XTAL_CASE(TYPE_FALSE){ return xnew<String>("instance of False"); }
		XTAL_CASE(TYPE_TRUE){ return xnew<String>("instance of True"); }
		XTAL_CASE(TYPE_NOP){ return xnew<String>("instance of Nop"); }
	}
	return null;	
}

int_t Any::object_name_force() const{
	if(type(*this)==TYPE_BASE){ 
		pvalue(*this)->object_name_force(); 
	}
	return 0;
}
	
void Any::set_object_name(const StringPtr& name, int_t force, const AnyPtr& parent) const{
	if(type(*this)==TYPE_BASE){ 
		pvalue(*this)->set_object_name(name, force, parent); 
	}
}

const ClassPtr& Any::get_class() const{
	switch(type(*this)){
		XTAL_NODEFAULT;
		XTAL_CASE(TYPE_NULL){ return get_cpp_class<Null>(); }
		XTAL_CASE(TYPE_BASE){ return pvalue(*this)->get_class(); }
		XTAL_CASE(TYPE_INT){ return get_cpp_class<Int>(); }
		XTAL_CASE(TYPE_FLOAT){ return get_cpp_class<Float>(); }
		XTAL_CASE(TYPE_FALSE){ return get_cpp_class<False>(); }
		XTAL_CASE(TYPE_TRUE){ return get_cpp_class<True>(); }
		XTAL_CASE(TYPE_NOP){ return get_cpp_class<Nop>(); }
	}
	return get_cpp_class<Any>();
}

uint_t Any::hashcode() const{
	if(type(*this)==TYPE_BASE){
		return pvalue(*this)->hashcode();
	}
	return (uint_t)rawvalue(*this);
}


bool Any::is(const ClassPtr& v) const{
	return get_class()->is_inherited(v);	
}

AnyPtr Any::p() const{
	ap(*this)->send("p");
	return ap(*this);
}

void visit_members(Visitor& m, const AnyPtr& p){
	if(type(p)==TYPE_BASE){
		XTAL_ASSERT((int)pvalue(p)->ref_count() >= -m.value());
		pvalue(p)->add_ref_count(m.value());
	}
}




}
