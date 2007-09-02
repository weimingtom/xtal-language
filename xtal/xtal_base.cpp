#include "xtal.h"

#include "xtal_base.h"
#include "xtal_macro.h"

namespace xtal{

int_t Base::to_i(){
	return cast<int_t>(send(Xid(to_i)));
}

float_t Base::to_f(){
	return cast<float_t>(send(Xid(to_f)));
}

StringPtr Base::to_s(){
	return cast<StringPtr>(send(Xid(to_s)));
}

AnyPtr Base::p(){
	return send(Xid(p));
}


void Base::set_class(const ClassPtr& c){
	class_ = c;
}
	
void Base::call(const VMachinePtr& vm){
	ap(Innocence(this))->rawsend(vm, Xid(op_call));
}

const AnyPtr& Base::do_member(const InternedStringPtr& name, const AnyPtr& self, const AnyPtr& ns){ 
	return nop;
}


const AnyPtr& Base::member(const InternedStringPtr& name, const AnyPtr& self, const AnyPtr& ns){ 
	return ap(Innocence(this))->member(name, self, ns); 
}

void Base::def(const InternedStringPtr& name, const AnyPtr& value, int_t accessibility, const AnyPtr& ns){

}

AnyPtr Base::send(const InternedStringPtr& name){
	const VMachinePtr& vm = vmachine();
	vm->setup_call(1);
	rawsend(vm, name);
	return vm->result_and_cleanup_call();
}

void Base::rawsend(const VMachinePtr& vm, const InternedStringPtr& name, const AnyPtr& self, const AnyPtr& ns){
	ap(Innocence(this))->rawsend(vm, name, self, ns);
}

HaveInstanceVariables* Base::have_instance_variables(){ 
	return &empty_have_instance_variables; 
}

StringPtr Base::object_name(){ 
	return xnew<String>("instance of ")->cat(get_class()->object_name());
}

int_t Base::object_name_force(){ 
	return 0;
}

void Base::set_object_name(const StringPtr& name, int_t force, const AnyPtr& parent){

}

bool Base::is(const ClassPtr& klass){
	return ap(Innocence(this))->is(klass);
}

uint_t Base::hashcode(){
	return (uint_t)this;
}

StringPtr HaveName::object_name(){
	if(!name_)
		return xnew<String>("<instance of ")->cat(get_class()->object_name())->cat(">");
	if(!parent_)
		return name_;
	return parent_->object_name()->cat("::")->cat(name_);
}

int_t HaveName::object_name_force(){
	return force_;
}

void HaveName::set_object_name(const StringPtr& name, int_t force, const AnyPtr& parent){
	if(!name_ || force_<force){
		name_ = name;
		force_ = force;
		parent_ = parent;
	}
}


}

