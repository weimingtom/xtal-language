#include "xtal.h"
#include "xtal_macro.h"

namespace xtal{

namespace{

	bool op_in_Any_Array(const AnyPtr& v, const ArrayPtr& values){
		Xfor(v2, values){
			if(v == v2){
				return true;
			}
		}
		return false;
	}

	bool op_in_Any_Set(const AnyPtr& v, const SetPtr& values){
		return values->at(v);
	}

}

void initialize_any(){
	{
		ClassPtr p = new_cpp_class<Any>("Any");
		p->method("class", &Any::get_class);
		p->method("get_class", &Any::get_class);
		p->method("self", &Any::self);
		p->method("object_name", &Any::object_name);
		p->method("s_save", &Any::s_save);
		p->method("s_load", &Any::s_load);

		p->dual_dispatch_method("op_add");
		p->dual_dispatch_method("op_sub");
		p->dual_dispatch_method("op_cat");
		p->dual_dispatch_method("op_mul");
		p->dual_dispatch_method("op_div");
		p->dual_dispatch_method("op_mod");
		p->dual_dispatch_method("op_and");
		p->dual_dispatch_method("op_or");
		p->dual_dispatch_method("op_xor");
		p->dual_dispatch_method("op_shr");
		p->dual_dispatch_method("op_shl");
		p->dual_dispatch_method("op_ushr");

		p->dual_dispatch_method("op_add_assign");
		p->dual_dispatch_method("op_sub_assign");
		p->dual_dispatch_method("op_cat_assign");
		p->dual_dispatch_method("op_mul_assign");
		p->dual_dispatch_method("op_div_assign");
		p->dual_dispatch_method("op_mod_assign");
		p->dual_dispatch_method("op_and_assign");
		p->dual_dispatch_method("op_or_assign");
		p->dual_dispatch_method("op_xor_assign");
		p->dual_dispatch_method("op_shr_assign");
		p->dual_dispatch_method("op_shl_assign");
		p->dual_dispatch_method("op_ushr_assign");

		p->dual_dispatch_method("op_eq");
		p->dual_dispatch_method("op_lt");

		p->dual_dispatch_method("op_at");
		p->dual_dispatch_method("op_set_at");

		p->dual_dispatch_method("op_range");
		p->method("op_in", &op_in_Any_Array, new_cpp_class<Array>());
		p->method("op_in", &op_in_Any_Set, new_cpp_class<Set>());
	}

	builtin()->def("Any", get_cpp_class<Any>());
}

AnyPtr operator +(const AnyPtr& a){ return a->send(Xid(op_pos)); }
AnyPtr operator -(const AnyPtr& a){ return a->send(Xid(op_neg)); }
AnyPtr operator ~(const AnyPtr& a){ return a->send(Xid(op_com)); }

AnyPtr operator +(const AnyPtr& a, const AnyPtr& b){ return a->send(Xid(op_add), b->get_class())(b); }
AnyPtr operator -(const AnyPtr& a, const AnyPtr& b){ return a->send(Xid(op_sub), b->get_class())(b); }
AnyPtr operator *(const AnyPtr& a, const AnyPtr& b){ return a->send(Xid(op_mul), b->get_class())(b); }
AnyPtr operator /(const AnyPtr& a, const AnyPtr& b){ return a->send(Xid(op_div), b->get_class())(b); }
AnyPtr operator %(const AnyPtr& a, const AnyPtr& b){ return a->send(Xid(op_mod), b->get_class())(b); }
AnyPtr operator |(const AnyPtr& a, const AnyPtr& b){ return a->send(Xid(op_or), b->get_class())(b); }
AnyPtr operator &(const AnyPtr& a, const AnyPtr& b){ return a->send(Xid(op_and), b->get_class())(b); }
AnyPtr operator ^(const AnyPtr& a, const AnyPtr& b){ return a->send(Xid(op_xor), b->get_class())(b); }
AnyPtr operator >>(const AnyPtr& a, const AnyPtr& b){ return a->send(Xid(op_shr), b->get_class())(b); }
AnyPtr operator <<(const AnyPtr& a, const AnyPtr& b){ return a->send(Xid(op_shl), b->get_class())(b); }

AnyPtr operator ==(const AnyPtr& a, const AnyPtr& b){ 
	if(raweq(a, b))
		return true;

	const VMachinePtr& vm = vmachine();
	vm->setup_call(1, b);
	a->rawsend(vm, Xid(op_eq), b->get_class());
	if(vm->processed() && vm->result()){
		vm->cleanup_call();
		return true;
	}
	vm->return_result();
	vm->cleanup_call();
	return false;
}

AnyPtr operator !=(const AnyPtr& a, const AnyPtr& b){ return !(a==b); }
AnyPtr operator <(const AnyPtr& a, const AnyPtr& b){ return a->send(Xid(op_lt), b->get_class())(b); }
AnyPtr operator >(const AnyPtr& a, const AnyPtr& b){ return b<a; }
AnyPtr operator <=(const AnyPtr& a, const AnyPtr& b){ return !(b<a); }
AnyPtr operator >=(const AnyPtr& a, const AnyPtr& b){ return !(a<b); }

AnyPtr& operator +=(AnyPtr& a, const AnyPtr& b){ a = a->send(Xid(op_add_assign), b->get_class())(b); return a; }
AnyPtr& operator -=(AnyPtr& a, const AnyPtr& b){ a = a->send(Xid(op_sub_assign), b->get_class())(b); return a; }
AnyPtr& operator *=(AnyPtr& a, const AnyPtr& b){ a = a->send(Xid(op_mul_assign), b->get_class())(b); return a; }
AnyPtr& operator /=(AnyPtr& a, const AnyPtr& b){ a = a->send(Xid(op_div_assign), b->get_class())(b); return a; }
AnyPtr& operator %=(AnyPtr& a, const AnyPtr& b){ a = a->send(Xid(op_mod_assign), b->get_class())(b); return a; }
AnyPtr& operator |=(AnyPtr& a, const AnyPtr& b){ a = a->send(Xid(op_or_assign), b->get_class())(b); return a; }
AnyPtr& operator &=(AnyPtr& a, const AnyPtr& b){ a = a->send(Xid(op_and_assign), b->get_class())(b); return a; }
AnyPtr& operator ^=(AnyPtr& a, const AnyPtr& b){ a = a->send(Xid(op_xor_assign), b->get_class())(b); return a; }
AnyPtr& operator >>=(AnyPtr& a, const AnyPtr& b){ a = a->send(Xid(op_shr_assign), b->get_class())(b); return a; }
AnyPtr& operator <<=(AnyPtr& a, const AnyPtr& b){ a = a->send(Xid(op_shl_assign), b->get_class())(b); return a; }

Innocence::Innocence(const char_t* str){
	*this = xnew<String>(str);
}

AnyPtr Innocence::operator()() const{
	const VMachinePtr& vm = vmachine();
	vm->setup_call(1);
	ap(*this)->call(vm);
	return vm->result_and_cleanup_call();
}

SendProxy Any::send(const IDPtr& primary_key, const AnyPtr& secondary_key) const{
	return SendProxy(ap(*this), primary_key, secondary_key);
}

const AnyPtr& Any::do_lazy() const{
	return ((Lazy*)pvalue(*this))->ret = ((Lazy*)pvalue(*this))->value();
}

struct MemberCacheTable{
	struct Unit{
		uint_t mutate_count;
		uint_t target_class;
		uint_t secondary_key;
		Innocence primary_key;
		Innocence member;
	};

	enum{ CACHE_MAX = 1024, CACHE_MASK = CACHE_MAX-1 };

	Unit table_[CACHE_MAX];

	int_t hit_;
	int_t miss_;

	MemberCacheTable(){
		hit_ = 0;
		miss_ = 0;
	}

	void print_result(){
		printf("MemberCacheTable hit count=%d, miss count=%d, hit rate=%g, miss rate=%g\n", hit_, miss_, hit_/(float)(hit_+miss_), miss_/(float)(hit_+miss_));
	}

	const AnyPtr& cache(const Innocence& target_class, const IDPtr& primary_key, const Innocence& secondary_key, const Innocence& self, bool inherited_too){
		uint_t itarget_class = rawvalue(target_class) | (uint_t)inherited_too;
		uint_t iprimary_key = rawvalue(primary_key);
		uint_t ins = rawvalue(secondary_key);

		uint_t hash = itarget_class ^ (iprimary_key>>2) ^ ins + iprimary_key ^ type(primary_key);
		Unit& unit = table_[hash & CACHE_MASK];
		if(global_mutate_count==unit.mutate_count && itarget_class==unit.target_class && raweq(primary_key, unit.primary_key) && ins==unit.secondary_key){
			hit_++;
			return ap(unit.member);
		}else{
			miss_++;

			if(type(target_class)!=TYPE_BASE)
				return nop;

			unit.member = pvalue(target_class)->do_member(primary_key, ap(secondary_key), ap(self));
			unit.target_class = itarget_class;
			unit.primary_key = primary_key;
			unit.secondary_key = ins;
			unit.mutate_count = global_mutate_count;
			return ap(unit.member);
		}
	}
};

struct IsInheritedCacheTable{
	struct Unit{
		uint_t mutate_count;
		uint_t target_class;
		uint_t klass;
		bool result;
	};

	enum{ CACHE_MAX = 512, CACHE_MASK = CACHE_MAX-1 };

	Unit table_[CACHE_MAX];

	int_t hit_;
	int_t miss_;

	IsInheritedCacheTable(){
		hit_ = 0;
		miss_ = 0;
	}

	void print_result(){
		printf("IsInheritedCacheTable hit count=%d, miss count=%d, hit rate=%g, miss rate=%g\n", hit_, miss_, hit_/(float)(hit_+miss_), miss_/(float)(hit_+miss_));
	}

	bool cache(const Innocence& target_class, const Innocence& klass){
		uint_t itarget_class = rawvalue(target_class);
		uint_t iklass = rawvalue(klass);

		uint_t hash = (itarget_class>>3) ^ (iklass>>2);
		Unit& unit = table_[hash & CACHE_MASK];

		if(global_mutate_count==unit.mutate_count && itarget_class==unit.target_class && iklass==unit.klass){
			hit_++;
			return unit.result;
		}else{
			miss_++;
			// キャッシュに保存
			unit.target_class = itarget_class;
			unit.klass = iklass;
			unit.mutate_count = global_mutate_count;
			unit.result = static_ptr_cast<Class>(ap(target_class))->is_inherited(ap(klass));

			return unit.result;
		}
	}

	bool cache2(const Innocence& target_class, const Innocence& klass){
		uint_t itarget_class = rawvalue(target_class);
		uint_t iklass = rawvalue(klass);

		uint_t hash = (itarget_class>>3) ^ (iklass>>2);
		Unit& unit = table_[hash & CACHE_MASK];

		if(global_mutate_count==unit.mutate_count && itarget_class==unit.target_class && iklass==unit.klass){
			hit_++;
			return unit.result;
		}else{
			miss_++;
			// キャッシュに保存
			unit.target_class = itarget_class;
			unit.klass = iklass;
			unit.mutate_count = global_mutate_count;
	
			if(const ClassPtr& cls = ptr_as<Class>(ap(target_class))){
				unit.result = cls->is_inherited(ap(klass));
			}else{
				unit.result = false;
			}

			return unit.result;
		}
	}
};

namespace{
	MemberCacheTable member_cache_table;
	IsInheritedCacheTable is_inherited_cache_table;
}

void print_result_of_cache(){
	member_cache_table.print_result();
	is_inherited_cache_table.print_result();
}

const AnyPtr& Any::member(const IDPtr& primary_key, const AnyPtr& secondary_key, const AnyPtr& self, bool inherited_too) const{
	return member_cache_table.cache(*this, primary_key, secondary_key, self, inherited_too);
}

void Any::def(const IDPtr& primary_key, const AnyPtr& value, const AnyPtr& secondary_key, int_t accessibility) const{
	switch(type(*this)){
		XTAL_DEFAULT;
		XTAL_CASE(TYPE_BASE){
			value->set_object_name(primary_key, object_name_force(), ap(*this));
			pvalue(*this)->def(primary_key, value, secondary_key, accessibility);
		}
		XTAL_CASE(TYPE_LAZY){
			do_lazy()->def(primary_key, value, secondary_key, accessibility);
		}
	}
}

void Any::rawsend(const VMachinePtr& vm, const IDPtr& primary_key, const AnyPtr& secondary_key, const AnyPtr& self, bool inherited_too) const{
	const ClassPtr& cls = get_class();
	vm->set_hint(cls, primary_key, secondary_key);
	const AnyPtr& ret = member_cache_table.cache(cls, primary_key, secondary_key, self, inherited_too);
	if(rawne(ret, nop)){
		vm->set_arg_this(ap(*this));
		ret->call(vm);
	}
}

void Any::call(const VMachinePtr& vm) const{
	switch(type(*this)){
		XTAL_DEFAULT{}
		XTAL_CASE(TYPE_BASE){ pvalue(*this)->call(vm); }
		XTAL_CASE(TYPE_LAZY){ do_lazy()->call(vm); }
	}
}

int_t Any::to_i() const{
	switch(type(*this)){
		XTAL_DEFAULT{ return cast<int_t>((*this).send(Xid(to_i))); }
		XTAL_CASE(TYPE_NULL){ return 0; }
		XTAL_CASE(TYPE_INT){ return ivalue(*this); }
		XTAL_CASE(TYPE_FLOAT){ return (int_t)fvalue(*this); }
		XTAL_CASE(TYPE_LAZY){ return do_lazy()->to_i(); }
	}
	return 0;
}

float_t Any::to_f() const{
	switch(type(*this)){
		XTAL_DEFAULT{ return cast<float_t>((*this).send(Xid(to_f))); }
		XTAL_CASE(TYPE_NULL){ return 0; }
		XTAL_CASE(TYPE_INT){ return (float_t)ivalue(*this); }
		XTAL_CASE(TYPE_FLOAT){ return fvalue(*this); }
		XTAL_CASE(TYPE_LAZY){ return do_lazy()->to_f(); }
	}
	return 0;
}

StringPtr Any::to_s() const{
	switch(type(*this)){
		XTAL_DEFAULT{}
		XTAL_CASE(TYPE_BASE){ if(const StringPtr& ret = ptr_as<String>(ap(*this)))return ret; }
		XTAL_CASE(TYPE_SMALL_STRING){ static_ptr_cast<String>(ap(*this)); }
		XTAL_CASE(TYPE_LAZY){ do_lazy()->to_s(); }
	}
	return ptr_cast<String>((*this).send(Xid(to_s)));
}

ArrayPtr Any::to_a() const{
	return ptr_cast<Array>((*this).send(Xid(to_a)));
}

MapPtr Any::to_m() const{
	return ptr_cast<Map>((*this).send(Xid(to_m)));
}

StringPtr Any::object_name() const{
	switch(type(*this)){
		XTAL_DEFAULT{ return StringPtr("instance of ")->cat(get_class()->object_name()); }
		XTAL_CASE(TYPE_BASE){ return pvalue(*this)->object_name(); }
		XTAL_CASE(TYPE_LAZY){ return do_lazy()->object_name(); }
	}
	return null;	
}

int_t Any::object_name_force() const{
	switch(type(*this)){
		XTAL_DEFAULT{}
		XTAL_CASE(TYPE_BASE){ return pvalue(*this)->object_name_force();  }
		XTAL_CASE(TYPE_LAZY){ return do_lazy()->object_name_force(); }
	}
	return 0;
}
	
void Any::set_object_name(const StringPtr& name, int_t force, const AnyPtr& parent) const{
	switch(type(*this)){
		XTAL_DEFAULT{}
		XTAL_CASE(TYPE_BASE){ return pvalue(*this)->set_object_name(name, force, parent);  }
		XTAL_CASE(TYPE_LAZY){ return do_lazy()->set_object_name(name, force, parent); }
	}
}

const ClassPtr& Any::get_class() const{
	switch(type(*this)){
		XTAL_NODEFAULT;
		XTAL_CASE(TYPE_NULL){ return get_cpp_class<Null>(); }
		XTAL_CASE(TYPE_NOP){ return get_cpp_class<Nop>(); }
		XTAL_CASE(TYPE_BASE){ return pvalue(*this)->get_class(); }
		XTAL_CASE(TYPE_INT){ return get_cpp_class<Int>(); }
		XTAL_CASE(TYPE_FLOAT){ return get_cpp_class<Float>(); }
		XTAL_CASE(TYPE_FALSE){ return get_cpp_class<False>(); }
		XTAL_CASE(TYPE_TRUE){ return get_cpp_class<True>(); }
		XTAL_CASE(TYPE_SMALL_STRING){ return get_cpp_class<String>(); }
		XTAL_CASE(TYPE_LAZY){ return do_lazy()->get_class(); }
	}
	return get_cpp_class<Any>();
}

uint_t Any::hashcode() const{
	switch(type(*this)){
		XTAL_DEFAULT{}
		XTAL_CASE(TYPE_BASE){ pvalue(*this)->hashcode();  }
		XTAL_CASE(TYPE_LAZY){ return do_lazy()->hashcode(); }
	}
	return (uint_t)rawvalue(*this);
}


bool Any::is(const AnyPtr& klass) const{
	const ClassPtr& my_class = get_class();
	if(raweq(my_class, klass)) return true;
	return is_inherited_cache_table.cache(my_class, klass)!=0;
}

bool Any::is_inherited(const AnyPtr& klass) const{
	if(raweq(*this, klass)) return true;
	return is_inherited_cache_table.cache2(*this, klass);
}

AnyPtr Any::p() const{
	ap(*this)->send(Xid(p));
	return ap(*this);
}

AnyPtr Any::s_save() const{
	MapPtr ret = xnew<Map>();
	ClassPtr klass = get_class();

	ArrayPtr ary = klass->send(Xid(ancestors))->to_a();
	ary->push_back(klass);

	Xfor(it, ary){
		if(const AnyPtr& member = it->member(Xid(serial_save), null, null, false)){
			const VMachinePtr& vm = vmachine();
			vm->setup_call(1);
			vm->set_arg_this(ap(*this));
			member->call(vm);
			ret->set_at(it->object_name(), vm->result_and_cleanup_call());
		}else{
			ret->set_at(it->object_name(), serial_save(static_ptr_cast<Class>(it)));
		}
	}

	return ret;
}

void Any::s_load(const AnyPtr& v) const{
	MapPtr ret = ptr_cast<Map>(v);
	ClassPtr klass = get_class();

	ArrayPtr ary = klass->send(Xid(ancestors))->to_a();
	ary->push_back(klass);

	Xfor(it, ary){
		if(const AnyPtr& member = it->member(Xid(serial_load), null, null, false)){
			const VMachinePtr& vm = vmachine();
			vm->setup_call(1, ret->at(it->object_name()));
			vm->set_arg_this(ap(*this));
			member->call(vm);
			vm->cleanup_call();
		}else{
			serial_load(static_ptr_cast<Class>(it), ret->at(it->object_name()));
		}
	}
}

AnyPtr Any::serial_save(const ClassPtr& p) const{
	if(type(*this)==TYPE_LAZY){
		return do_lazy()->serial_save(p); 
	}

	if(type(*this)!=TYPE_BASE){
		return null;
	}

	if(InstanceVariables* iv = pvalue(*this)->instance_variables()){
		if(CodePtr code = p->code()){
			ClassCore* core = p->core();
			if(core->instance_variable_size!=0){	
				MapPtr insts = xnew<Map>();
				for(int_t i=0; i<(int_t)core->instance_variable_size; ++i){
					insts->set_at(code->identifier(core->instance_variable_identifier_offset+i), iv->variable(i, core));
				}

				return insts;
			}
		}
	}
	return null;
}

void Any::serial_load(const ClassPtr& p, const AnyPtr& v) const{
	if(type(*this)==TYPE_LAZY){
		do_lazy()->serial_load(p, v);
		return;
	}

	if(type(*this)!=TYPE_BASE){
		return;
	}

	if(InstanceVariables* iv = pvalue(*this)->instance_variables()){
		if(MapPtr insts = ptr_as<Map>(v)){
			if(CodePtr code = p->code()){
				ClassCore* core = p->core();
				if(core->instance_variable_size!=0){	
					for(int_t i=0; i<(int_t)core->instance_variable_size; ++i){
						StringPtr str = code->identifier(core->instance_variable_identifier_offset+i);
						iv->set_variable(i, core, insts->at(code->identifier(core->instance_variable_identifier_offset+i)));
					}
				}
			}
		}
	}
}

AnyPtr fun2lazy(const AnyPtr& value){
	switch(type(value)){
		XTAL_DEFAULT{}
		XTAL_CASE(TYPE_BASE){ 
			AnyPtr ret = xnew<Lazy>(value);
			((Innocence&)ret).set_lazy();
			return ret;
		}

		XTAL_CASE(TYPE_LAZY){ return value; }
	}
	return null;
}

void visit_members(Visitor& m, const AnyPtr& p){
	switch(type(p)){
		XTAL_DEFAULT{}
		XTAL_CASE2(TYPE_BASE, TYPE_LAZY){ 
			XTAL_ASSERT((int)pvalue(p)->ref_count() >= -m.value());
			pvalue(p)->add_ref_count(m.value());
		}
	}
}




}
