
#include <vector>
#include <iostream>
#include <fstream>

#include "xtal.h"

#include "xtal_fwd.h"
#include "xtal_any.h"
#include "xtal_frame.h"
#include "xtal_fun.h"
#include "xtal_utilimpl.h"
#include "xtal_stringimpl.h"
#include "xtal_codeimpl.h"
#include "xtal_lexer.h"
#include "xtal_visitor.h"
#include "xtal_frameimpl.h"
#include "xtal_macro.h"
#include "xtal_stream.h"
#include "xtal_vmachineimpl.h"

namespace xtal{

void InitAny(){
	TClass<Any> p("Any");
	p.method("class", &Any::get_class);
	p.method("get_class", &Any::get_class);
	p.method("object_name", &Any::object_name);
	p.method("op_eq", &Any::raweq);
}


void InitDebug();
void InitString();
void InitID();
void InitAny();
void InitInt();
void InitFloat();
void InitArray();
void InitMap();
void InitArguments();
void InitFun();
void InitFiber();
void InitFormat();
void InitClass();
void InitStream();
void InitFileStream();
void InitMemoryStream();
void InitCode();


Null null;
Nop nop;

Any operator +(const Any& a, const Any& b){ return a.send(Xid(op_add), b); }
Any operator -(const Any& a, const Any& b){ return a.send(Xid(op_sub), b); }
Any operator *(const Any& a, const Any& b){ return a.send(Xid(op_mul), b); }
Any operator /(const Any& a, const Any& b){ return a.send(Xid(op_div), b); }
Any operator %(const Any& a, const Any& b){ return a.send(Xid(op_mod), b); }
Any operator |(const Any& a, const Any& b){ return a.send(Xid(op_or), b); }
Any operator &(const Any& a, const Any& b){ return a.send(Xid(op_and), b); }
Any operator ^(const Any& a, const Any& b){ return a.send(Xid(op_xor), b); }
Any operator >>(const Any& a, const Any& b){ return a.send(Xid(op_shr), b); }
Any operator <<(const Any& a, const Any& b){ return a.send(Xid(op_shl), b); }
Any operator ==(const Any& a, const Any& b){ return a.raweq(b) || a.send(Xid(op_eq), b); }
Any operator !=(const Any& a, const Any& b){ return !(a==b); }
Any operator <(const Any& a, const Any& b){ return a.send(Xid(op_lt), b); }
Any operator >(const Any& a, const Any& b){ return b<a; }
Any operator <=(const Any& a, const Any& b){ return !(b<a); }
Any operator >=(const Any& a, const Any& b){ return !(a<b); }

Any& operator +=(Any& a, const Any& b){ a = a.send(Xid(op_add_assign), b); return a; }
Any& operator -=(Any& a, const Any& b){ a = a.send(Xid(op_sub_assign), b); return a; }
Any& operator *=(Any& a, const Any& b){ a = a.send(Xid(op_mul_assign), b); return a; }
Any& operator /=(Any& a, const Any& b){ a = a.send(Xid(op_div_assign), b); return a; }
Any& operator %=(Any& a, const Any& b){ a = a.send(Xid(op_mod_assign), b); return a; }
Any& operator |=(Any& a, const Any& b){ a = a.send(Xid(op_or_assign), b); return a; }
Any& operator &=(Any& a, const Any& b){ a = a.send(Xid(op_and_assign), b); return a; }
Any& operator ^=(Any& a, const Any& b){ a = a.send(Xid(op_xor_assign), b); return a; }
Any& operator >>=(Any& a, const Any& b){ a = a.send(Xid(op_shr_assign), b); return a; }
Any& operator <<=(Any& a, const Any& b){ a = a.send(Xid(op_shl_assign), b); return a; }

void visit_members(Visitor& m, const Any& value){
	if(value.type()==TYPE_BASE){
		XTAL_ASSERT((int)value.impl()->ref_count() >= -m.value());
		value.impl()->add_ref_count(m.value());
	}
}

namespace{

AnyImpl** objects_begin_ = 0;
AnyImpl** objects_current_ = 0;
AnyImpl** objects_end_ = 0;

GCObserverImpl** gcobservers_begin_ = 0;
GCObserverImpl** gcobservers_current_ = 0;
GCObserverImpl** gcobservers_end_ = 0;

Any** llvars_begin_ = 0;
Any** llvars_current_ = 0;
Any** llvars_end_ = 0;

Any** place_begin_ = 0;
Any** place_current_ = 0;
Any** place_end_ = 0;

uint_t cycle_count_ = 0;
uint_t objects_gene_line_ = 0;

void print_alive_objects(){
	for(AnyImpl** it = objects_begin_; it!=objects_current_; ++it){
		//StringImpl* str = dynamic_cast<StringImpl*>(*it);
		//fprintf(stderr, " %s rc=%d %s\n", typeid(**it).name(), (*it)->ref_count(), str ? str->debug_c_str() : "");
		//fprintf(stderr, " rc=%d\n", (*it)->ref_count());
	}
}

}

void enable_gc(){
	cycle_count_++;
}

void disable_gc(){
	cycle_count_--;
}

bool initialized(){
	return objects_begin_ ? true : false;
}

void initialize(){
	if(initialized()){ return; } 

	disable_gc();

	expand_simple_dynamic_pointer_array((void**&)objects_begin_, (void**&)objects_end_, (void**&)objects_current_);

	empty_have_instance_variables.init();

	TClass<Any>::set((ClassImpl*)AnyImpl::operator new(sizeof(ClassImpl)));  
	TClass<Any>::get().impl()->set_ref_count(1);
		
	TClass<Class>::set((ClassImpl*)AnyImpl::operator new(sizeof(ClassImpl)));  
	TClass<Class>::get().impl()->set_ref_count(1);
	
	int_t temp_ref_count;
	
	temp_ref_count = TClass<Any>::get().impl()->ref_count(); 
	new(TClass<Any>::get().impl()) ClassImpl();
	TClass<Any>::get().impl()->add_ref_count(temp_ref_count-1);
		
	temp_ref_count = TClass<Class>::get().impl()->ref_count(); 
	new(TClass<Class>::get().impl()) ClassImpl();
	TClass<Class>::get().impl()->add_ref_count(temp_ref_count-1);
	

	TClass<String>();
	TClass<Null>();
	TClass<True>();
	TClass<False>();
	TClass<Int>();
	TClass<Float>();
	TClass<Array>();
	TClass<Map>();
	TClass<Arguments>();
	TClass<Fiber>();
	
	InitDebug();

	//finalize_id = ID("finalize");
	
	InitString();
	InitID();
	InitAny();
	InitThread();
	InitInt();
	InitFloat();
	InitArray();
	InitMap();
	InitArguments();
	InitFun();
	InitFiber();
	InitFormat();
	InitClass();
	InitStream();
	InitFileStream();
	InitMemoryStream();
	InitCode();
	
	atexit(&uninitialize); // uninitializeを登録する

	initialize_lib();

	enable_gc();
}

void uninitialize(){
	//print_alive_objects();

	UninitThread();

	full_gc();

	for(Any** p = llvars_begin_; p!=llvars_current_; ++p){
		**p = null;
	}
	
	for(Any** p = place_begin_; p!=place_current_; ++p){
		**p = null;
	}
		
	full_gc();
	
	if(objects_current_-objects_begin_ != 0){
		//fprintf(stderr, "finished gc\n");
		//fprintf(stderr, " alive object = %d\n", objects_current_-objects_begin_);
		print_alive_objects();
		XTAL_ASSERT(false); // オブジェクトが全て解放されていない。
	}
	
	for(Any** p = place_begin_; p!=place_current_; ++p){
		user_free(*p, sizeof(Any));
	}
	
	llvars_current_ = llvars_begin_;
	place_current_ = place_begin_;

	fit_simple_dynamic_pointer_array((void**&)llvars_begin_, (void**&)llvars_end_, (void**&)llvars_current_);
	fit_simple_dynamic_pointer_array((void**&)gcobservers_begin_, (void**&)gcobservers_end_, (void**&)gcobservers_current_);
	fit_simple_dynamic_pointer_array((void**&)objects_begin_, (void**&)objects_end_, (void**&)objects_current_);
	fit_simple_dynamic_pointer_array((void**&)place_begin_, (void**&)place_end_, (void**&)place_current_);
}

void add_long_life_var(Any* a, int_t n){
	for(Any** p = llvars_begin_; p!=llvars_current_; ++p){
		if(*p==a){
			return;
		}
	}

	for(int_t i = 0; i<n; ++i){
		if(llvars_current_==llvars_end_){
			expand_simple_dynamic_pointer_array((void**&)llvars_begin_, (void**&)llvars_end_, (void**&)llvars_current_);
		}
		*llvars_current_++ = a++;
	}
}

void remove_long_life_var(Any* a, int_t n){
	for(Any** p = llvars_begin_; p!=llvars_current_; ++p){
		if(*p==a){
			std::memmove(p, p+n, sizeof(Any*)*n);
			llvars_current_ -= n;
			break;
		}
	}
}

Any* make_place(){
	if(place_current_==place_end_){
		expand_simple_dynamic_pointer_array((void**&)place_begin_, (void**&)place_end_, (void**&)place_current_);
	}
	*place_current_ = new(user_malloc(sizeof(Any))) Any();
	return *place_current_++;
}

struct CycleCounter{
	uint_t* p;
	CycleCounter(uint_t* p):p(p){ *p+=1; }
	~CycleCounter(){ *p-=1; }
};


void gc(){
	if(cycle_count_!=0){ return; }
	if(stop_the_world()){
		const VMachine& vm = vmachine();

		CycleCounter cc(&cycle_count_);

		for(GCObserverImpl** it = gcobservers_begin_; it!=gcobservers_current_; ++it){
			(*it)->before_gc();
		}

		if((int)objects_gene_line_>objects_current_-objects_begin_){
			objects_gene_line_ = 0;
		}

		AnyImpl** objects_alive = objects_begin_+objects_gene_line_;

		for(AnyImpl** it = objects_alive; it!=objects_current_; ++it){
			if((*it)->ref_count()!=0){
				std::swap(*it, *objects_alive++);
			}
		}

		for(GCObserverImpl** it = gcobservers_begin_; it!=gcobservers_current_; ++it){
			(*it)->after_gc();
		}

		for(AnyImpl** it = objects_alive; it!=objects_current_; ++it){
			delete *it;
		}

		for(AnyImpl** it = objects_alive; it!=objects_current_; ++it){
			user_free(*it, 0);
		}
		objects_current_ = objects_alive;

		// 生きているオブジェクトの2/3の位置にラインを設定する
		objects_gene_line_ = (objects_current_-objects_begin_)*2/3;

		//fprintf(stderr, "finished gc\n");
		//fprintf(stderr, " alive object = %d\n", objects_current_-objects_begin_);

		restart_the_world();
	}
}


void full_gc(){
	if(cycle_count_!=0){ return; }
	if(stop_the_world()){
		const VMachine& vm = vmachine();

		CycleCounter cc(&cycle_count_);
		
		AnyImpl** prev_oc;
		do{
			
			for(GCObserverImpl** it = gcobservers_begin_; it!=gcobservers_current_; ++it){
				(*it)->before_gc();
			}

			prev_oc = objects_current_;

			{
				Visitor m(-1);	
				for(AnyImpl** it = objects_begin_; it!=objects_current_; ++it){
					(*it)->visit_members(m);
				}
			}

			{
				AnyImpl** objects_alive = objects_begin_;

				{
					Visitor m(1);
					bool end = false;
					while(!end){
						end = true;
						for(AnyImpl** it = objects_alive; it!=objects_current_; ++it){
							if((*it)->ref_count()!=0){
								end = false;
								(*it)->visit_members(m);
								std::swap(*it, *objects_alive++);
							}
						}
					}
				}
	

				{// 削除されるオブジェクトだが、整合性をとるため参照カウンタを元に戻す
					Visitor m(1);
					for(AnyImpl** it = objects_alive; it!=objects_current_; ++it){
						(*it)->visit_members(m);
					}
				}

				for(GCObserverImpl** it = gcobservers_begin_; it!=gcobservers_current_; ++it){
					(*it)->after_gc();
				}

				for(AnyImpl** it = objects_alive; it!=objects_current_; ++it){
					delete *it;
				}

				for(AnyImpl** it = objects_alive; it!=objects_current_; ++it){
					user_free(*it, 0);
				}
				objects_current_ = objects_alive;
			}

		}while(prev_oc!=objects_current_);
		
		//fprintf(stderr, "finished full_gc\n");
		//fprintf(stderr, " alive object = %d\n", objects_current_-objects_begin_);

		restart_the_world();
	}
}

GCObserverImpl::GCObserverImpl(){
	if(gcobservers_current_==gcobservers_end_){
		expand_simple_dynamic_pointer_array((void**&)gcobservers_begin_, (void**&)gcobservers_end_, (void**&)gcobservers_current_);
	}
	*gcobservers_current_++ = this;
}

GCObserverImpl::GCObserverImpl(const GCObserverImpl& v)
:AnyImpl(v){
	if(gcobservers_current_==gcobservers_end_){
		expand_simple_dynamic_pointer_array((void**&)gcobservers_begin_, (void**&)gcobservers_end_, (void**&)gcobservers_current_);
	}
	*gcobservers_current_++ = this;
}
	
GCObserverImpl::~GCObserverImpl(){
	for(GCObserverImpl** p = gcobservers_begin_; p!=gcobservers_current_; ++p){
		if(*p==this){
			std::swap(*p, *--gcobservers_current_);
			break;
		}
	}
}
	
void* AnyImpl::operator new(size_t size){
	if(objects_current_==objects_end_){
		expand_simple_dynamic_pointer_array((void**&)objects_begin_, (void**&)objects_end_, (void**&)objects_current_);
	}	
	AnyImpl* p = static_cast<AnyImpl*>(user_malloc(size));
	*objects_current_++=p;
	
	p->ref_count_ = 1;
	p->class_ = null;
	
	return p;
}
	
void AnyImpl::operator delete(void* p, size_t size){

}

void* AnyImpl::operator new(size_t size, Any& guard){
	if(objects_current_==objects_end_){
		expand_simple_dynamic_pointer_array((void**&)objects_begin_, (void**&)objects_end_, (void**&)objects_current_);
	}	
	AnyImpl* p = static_cast<AnyImpl*>(user_malloc(size));
	*objects_current_++ = p;
	
	p->ref_count_ = 0;
	p->class_ = TClass<Any>::get();

	guard = p;
	return p;
}
	
void AnyImpl::operator delete(void* p, Any& guard){
	guard = null;
}

void AnyImpl::set_class(const Class& c){
	class_ = c;
}

void AnyImpl::visit_members(Visitor& m){
	//m & class_;
}

int_t AnyImpl::arity(){
	return 0;
}
	
void AnyImpl::call(const VMachine& vm){
	UncountedAny(this).cref().send(Xid(op_call), vm);
}

const Any& AnyImpl::member(const ID& name, const Any& self, const Any& ns){ 
	return null; 
}

void AnyImpl::def(const ID& name, const Any& value, int_t accessibility, const Any& ns){

}

HaveInstanceVariables* AnyImpl::have_instance_variables(){ 
	return &empty_have_instance_variables; 
}

String AnyImpl::object_name(){ 
	return String("instance of ").cat(get_class().object_name());
}

int_t AnyImpl::object_name_force(){ 
	return 0;
}

void AnyImpl::set_object_name(const String& name, int_t force, const Any& parent){

}

uint_t AnyImpl::hashcode(){
	return (uint_t)this;
}

int_t Any::arity() const{
	switch(type()){
		XTAL_DEFAULT;
		XTAL_CASE(TYPE_BASE){
			return impl()->arity();
		}
	}
	return 0;
}

#define XTAL_ANY_CALL0(op, call, args) \
Any Any::op args const{\
	const VMachine& vm = vmachine();\
	vm.setup_call(1);\
	call;\
	return vm.result_and_cleanup_call();\
}

#define XTAL_ANY_CALL1(op, call, args) \
Any Any::op args const{\
	const VMachine& vm = vmachine();\
	vm.setup_call(1);\
	vm.push_arg(a0);\
	call;\
	return vm.result_and_cleanup_call();\
}

#define XTAL_ANY_CALL2(op, call, args) \
Any Any::op args const{\
	const VMachine& vm = vmachine();\
	vm.setup_call(1);\
	vm.push_arg(a0);\
	vm.push_arg(a1);\
	call;\
	return vm.result_and_cleanup_call();\
}

#define XTAL_ANY_CALL3(op, call, args) \
Any Any::op args const{\
	const VMachine& vm = vmachine();\
	vm.setup_call(1);\
	vm.push_arg(a0);\
	vm.push_arg(a1);\
	vm.push_arg(a2);\
	call;\
	return vm.result_and_cleanup_call();\
}

#define XTAL_ANY_CALL4(op, call, args) \
Any Any::op args const{\
	const VMachine& vm = vmachine();\
	vm.setup_call(1);\
	vm.push_arg(a0);\
	vm.push_arg(a1);\
	vm.push_arg(a2);\
	vm.push_arg(a3);\
	call;\
	return vm.result_and_cleanup_call();\
}

#define XTAL_ANY_CALL5(op, call, args) \
Any Any::op args const{\
	const VMachine& vm = vmachine();\
	vm.setup_call(1);\
	vm.push_arg(a0);\
	vm.push_arg(a1);\
	vm.push_arg(a2);\
	vm.push_arg(a3);\
	vm.push_arg(a4);\
	call;\
	return vm.result_and_cleanup_call();\
}

XTAL_ANY_CALL0(operator(), call(vm), ());
XTAL_ANY_CALL1(operator(), call(vm), (const Any& a0));
XTAL_ANY_CALL2(operator(), call(vm), (const Any& a0, const Any& a1));
XTAL_ANY_CALL3(operator(), call(vm), (const Any& a0, const Any& a1, const Any& a2));
XTAL_ANY_CALL4(operator(), call(vm), (const Any& a0, const Any& a1, const Any& a2, const Any& a3));
XTAL_ANY_CALL5(operator(), call(vm), (const Any& a0, const Any& a1, const Any& a2, const Any& a3, const Any& a4));
XTAL_ANY_CALL1(operator(), call(vm), (const Named& a0));
XTAL_ANY_CALL2(operator(), call(vm), (const Any& a0, const Named& a1));
XTAL_ANY_CALL3(operator(), call(vm), (const Any& a0, const Any& a1, const Named& a2));
XTAL_ANY_CALL4(operator(), call(vm), (const Any& a0, const Any& a1, const Any& a2, const Named& a3));
XTAL_ANY_CALL5(operator(), call(vm), (const Any& a0, const Any& a1, const Any& a2, const Any& a3, const Named& a4));
XTAL_ANY_CALL2(operator(), call(vm), (const Named& a0, const Named& a1));
XTAL_ANY_CALL3(operator(), call(vm), (const Any& a0, const Named& a1, const Named& a2));
XTAL_ANY_CALL4(operator(), call(vm), (const Any& a0, const Any& a1, const Named& a2, const Named& a3));
XTAL_ANY_CALL5(operator(), call(vm), (const Any& a0, const Any& a1, const Any& a2, const Named& a3, const Named& a4));
XTAL_ANY_CALL3(operator(), call(vm), (const Named& a0, const Named& a1, const Named& a2));
XTAL_ANY_CALL4(operator(), call(vm), (const Any& a0, const Named& a1, const Named& a2, const Named& a3));
XTAL_ANY_CALL5(operator(), call(vm), (const Any& a0, const Any& a1, const Named& a2, const Named& a3, const Named& a4));
XTAL_ANY_CALL4(operator(), call(vm), (const Named& a0, const Named& a1, const Named& a2, const Named& a3));
XTAL_ANY_CALL5(operator(), call(vm), (const Any& a0, const Named& a1, const Named& a2, const Named& a3, const Named& a4));
XTAL_ANY_CALL5(operator(), call(vm), (const Named& a0, const Named& a1, const Named& a2, const Named& a3, const Named& a4));

XTAL_ANY_CALL0(send, send(name, vm), (const ID& name));
XTAL_ANY_CALL1(send, send(name, vm), (const ID& name, const Any& a0));
XTAL_ANY_CALL2(send, send(name, vm), (const ID& name, const Any& a0, const Any& a1));
XTAL_ANY_CALL3(send, send(name, vm), (const ID& name, const Any& a0, const Any& a1, const Any& a2));
XTAL_ANY_CALL4(send, send(name, vm), (const ID& name, const Any& a0, const Any& a1, const Any& a2, const Any& a3));
XTAL_ANY_CALL5(send, send(name, vm), (const ID& name, const Any& a0, const Any& a1, const Any& a2, const Any& a3, const Any& a4));
XTAL_ANY_CALL1(send, send(name, vm), (const ID& name, const Named& a0));
XTAL_ANY_CALL2(send, send(name, vm), (const ID& name, const Any& a0, const Named& a1));
XTAL_ANY_CALL3(send, send(name, vm), (const ID& name, const Any& a0, const Any& a1, const Named& a2));
XTAL_ANY_CALL4(send, send(name, vm), (const ID& name, const Any& a0, const Any& a1, const Any& a2, const Named& a3));
XTAL_ANY_CALL5(send, send(name, vm), (const ID& name, const Any& a0, const Any& a1, const Any& a2, const Any& a3, const Named& a4));
XTAL_ANY_CALL2(send, send(name, vm), (const ID& name, const Named& a0, const Named& a1));
XTAL_ANY_CALL3(send, send(name, vm), (const ID& name, const Any& a0, const Named& a1, const Named& a2));
XTAL_ANY_CALL4(send, send(name, vm), (const ID& name, const Any& a0, const Any& a1, const Named& a2, const Named& a3));
XTAL_ANY_CALL5(send, send(name, vm), (const ID& name, const Any& a0, const Any& a1, const Any& a2, const Named& a3, const Named& a4));
XTAL_ANY_CALL3(send, send(name, vm), (const ID& name, const Named& a0, const Named& a1, const Named& a2));
XTAL_ANY_CALL4(send, send(name, vm), (const ID& name, const Any& a0, const Named& a1, const Named& a2, const Named& a3));
XTAL_ANY_CALL5(send, send(name, vm), (const ID& name, const Any& a0, const Any& a1, const Named& a2, const Named& a3, const Named& a4));
XTAL_ANY_CALL4(send, send(name, vm), (const ID& name, const Named& a0, const Named& a1, const Named& a2, const Named& a3));
XTAL_ANY_CALL5(send, send(name, vm), (const ID& name, const Any& a0, const Named& a1, const Named& a2, const Named& a3, const Named& a4));
XTAL_ANY_CALL5(send, send(name, vm), (const ID& name, const Named& a0, const Named& a1, const Named& a2, const Named& a3, const Named& a4));

#undef XTAL_ANY_CALL0
#undef XTAL_ANY_CALL1
#undef XTAL_ANY_CALL2
#undef XTAL_ANY_CALL3
#undef XTAL_ANY_CALL4
#undef XTAL_ANY_CALL5

Any::Any(const char* str){
	set_null();
	*this = String(str);
}

const Any& Any::member(const ID& name) const{
	switch(type()){
		XTAL_DEFAULT;
		XTAL_CASE(TYPE_BASE){ return impl()->member(name, *this, null); }
		XTAL_CASE(TYPE_NOP){ return *this; }
	}
	return null;
}

const Any& Any::member(const ID& name, const Any& self) const{
	switch(type()){
		XTAL_DEFAULT;
		XTAL_CASE(TYPE_BASE){ return impl()->member(name, self, null); }
		XTAL_CASE(TYPE_NOP){ return *this; }
	}
	return null;
}

const Any& Any::member(const ID& name, const Any& self, const Any& ns) const{
	switch(type()){
		XTAL_DEFAULT;
		XTAL_CASE(TYPE_BASE){ return impl()->member(name, self, ns); }
		XTAL_CASE(TYPE_NOP){ return *this; }
	}
	return null;
}

void Any::def(const ID& name, const Any& value, int_t accessibility, const Any& ns) const{
	switch(type()){
		XTAL_DEFAULT;
		XTAL_CASE(TYPE_BASE){
			value.set_object_name(name, object_name_force(), *this);
			impl()->def(name, value, accessibility, ns);
		}
	}
}

void Any::send(const ID& name, const VMachine& vm) const{
	const Class& cls = get_class();
	vm.impl()->set_hint(cls, name);
	if(const Any& ret = vm.impl()->member_cache(cls, name, *this, null)){
		vm.set_arg_this(*this);
		ret.call(vm);
	}
}

void Any::call(const VMachine& vm) const{
	switch(type()){
		XTAL_DEFAULT{}
		XTAL_CASE(TYPE_BASE){ impl()->call(vm); }
	}
}

int_t Any::to_i() const{
	switch(type()){
		XTAL_DEFAULT{ return cast<int_t>((*this).send("to_i")); }
		XTAL_CASE(TYPE_NULL){ return 0; }
		XTAL_CASE(TYPE_INT){ return ivalue(); }
		XTAL_CASE(TYPE_FLOAT){ return (int_t)fvalue(); }
	}
	return 0;
}

float_t Any::to_f() const{
	switch(type()){
		XTAL_DEFAULT{ return cast<float_t>((*this).send("to_f")); }
		XTAL_CASE(TYPE_NULL){ return 0; }
		XTAL_CASE(TYPE_INT){ return (float_t)ivalue(); }
		XTAL_CASE(TYPE_FLOAT){ return fvalue(); }
	}
	return 0;
}

String Any::to_s() const{
	if(const String* ret = as<const String*>(*this)){
		return *ret;
	}
	return cast<String>((*this).send("to_s"));
}

String Any::object_name() const{
	switch(type()){
		XTAL_NODEFAULT;
		XTAL_CASE(TYPE_NULL){ return String("instance of Null"); }
		XTAL_CASE(TYPE_BASE){ return impl()->object_name(); }
		XTAL_CASE(TYPE_INT){ return String("instance of Int"); }
		XTAL_CASE(TYPE_FLOAT){ return String("instance of Float"); }
		XTAL_CASE(TYPE_FALSE){ return String("instance of False"); }
		XTAL_CASE(TYPE_TRUE){ return String("instance of True"); }
		XTAL_CASE(TYPE_NOP){ return String("instance of Nop"); }
	}
	return null;	
}

int_t Any::object_name_force() const{
	if(type()==TYPE_BASE){ 
		impl()->object_name_force(); 
	}
	return 0;
}
	
void Any::set_object_name(const String& name, int_t force, const Any& parent) const{
	if(type()==TYPE_BASE){ 
		impl()->set_object_name(name, force, parent); 
	}
}

const Class& Any::get_class() const{
	switch(type()){
		XTAL_NODEFAULT;
		XTAL_CASE(TYPE_NULL){ return TClass<Null>::get(); }
		XTAL_CASE(TYPE_BASE){ return impl()->get_class(); }
		XTAL_CASE(TYPE_INT){ return TClass<Int>::get(); }
		XTAL_CASE(TYPE_FLOAT){ return TClass<Float>::get(); }
		XTAL_CASE(TYPE_FALSE){ return TClass<False>::get(); }
		XTAL_CASE(TYPE_TRUE){ return TClass<True>::get(); }
		XTAL_CASE(TYPE_NOP){ return TClass<Nop>::get(); }
	}
	return TClass<Any>::get();
}

Any Any::cat(const Any& v) const{
	return send(Xid(op_cat), v);
}

const Any Any::at(const Any& index) const{
	return send(Xid(op_at), index);
}

void Any::set_at(const Any& index, const Any& value) const{
	send(Xid(op_set_at), index, value);
}

const Any Any::operator[](const Any& a) const{
	return send(Xid(op_at), a);
}

int_t Any::size() const{
	return send(Xid(size)).to_i();
}

bool Any::is(const Class& v) const{
	return get_class().impl()->is_inherited(v);	
}


Any Any::p() const{
	VMachine vm = vmachine();
	vm.setup_call(0);
	vm.push_arg(*this);
	println(vm);
	vm.cleanup_call();
	return *this;
}


Any::Any(AnyImpl* v){
	if(v){ set_p(v); impl()->inc_ref_count(); }
	else{ set_null(); }
}

Any::Any(const AnyImpl* v){
	if(v){ set_p(v); impl()->inc_ref_count(); }
	else{ set_null(); }
}

Any& Any::operator =(const Any& v){
	dec_ref_count();
	UncountedAny::operator =(v);
	inc_ref_count();
	return *this;
}
	
Any& Any::operator =(AnyImpl* v){
	dec_ref_count();
	if(v){ set_p(v); impl()->inc_ref_count(); }
	else{ set_null(); }
	return *this;
}

Any& Any::operator =(const AnyImpl* v){
	dec_ref_count();
	if(v){ set_p(v); impl()->inc_ref_count(); }
	else{ set_null(); }
	return *this;
}

Any& Any::operator =(int_t v){
	dec_ref_count();
	set_i(v);
	return *this;
}
	
Any& Any::operator =(float_t v){
	dec_ref_count();
	set_f(v);
	return *this;
}

Any& Any::operator =(bool v){
	dec_ref_count();
	set_b(v);
	return *this;
}

Any& Any::operator =(const Null&){
	dec_ref_count();
	set_null();
	return *this;
}

}
