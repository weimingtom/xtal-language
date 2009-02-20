#include "xtal.h"
#include "xtal_macro.h"

namespace xtal{

Base::Base(){
	set_p(this);
	//ref_count_ = 0;
	class_ = (Class*)&null;
	instance_variables_ = &empty_instance_variables; 
}

Base::Base(const Base& b){
	set_p(this);
	//ref_count_ = 0;
	if(b.instance_variables_!=&empty_instance_variables){
		instance_variables_ = (InstanceVariables*)so_malloc(sizeof(InstanceVariables));
		new(instance_variables_) InstanceVariables(*b.instance_variables_);		

		class_ = b.class_;
		if(get_class()){
			class_->inc_ref_count();
		}
	}
	else{
		instance_variables_ = &empty_instance_variables;
		class_ = b.class_;
	}
}

Base& Base::operator =(const Base& b){
	if(this==&b){ return *this; }

	if(b.instance_variables_!=&empty_instance_variables){
		*instance_variables_ = *b.instance_variables_;		

		if(get_class()){
			class_->dec_ref_count();
		}
		class_ = b.class_;
		if(get_class()){
			class_->inc_ref_count();
		}
	}
	else{
		if(instance_variables_!=&empty_instance_variables){
			instance_variables_->~InstanceVariables();
			so_free(instance_variables_, sizeof(InstanceVariables));

			if(get_class()){
				class_->dec_ref_count();
			}

			class_ = (Class*)&null;
			instance_variables_ = &empty_instance_variables; 
		}
	}

	return *this;
}

Base::~Base(){
	if(instance_variables_!=&empty_instance_variables){
		instance_variables_->~InstanceVariables();
		so_free(instance_variables_, sizeof(InstanceVariables));

		if(get_class()){
			class_->dec_ref_count();
		}
	}
}

void Base::set_class(const ClassPtr& c){
	if(instance_variables_==&empty_instance_variables){
		class_ = c.get();
	}
	else{
		if(get_class()){
			class_->dec_ref_count();
		}
		class_ = c.get();
		if(get_class()){
			class_->inc_ref_count();
		}
	}
}
	
void Base::rawcall(const VMachinePtr& vm){
	ap(Any(this))->rawsend(vm, Xid(op_call));
}

const AnyPtr& Base::do_member(const IDPtr& primary_key, const AnyPtr& secondary_key, const AnyPtr& self, bool inherited_too, bool* nocache){ 
	return undefined;
}

void Base::def(const IDPtr& primary_key, const AnyPtr& value, const AnyPtr& secondary_key, int_t accessibility){

}

const ClassPtr& Base::object_parent(){
	return null;
}

void Base::set_object_parent(const ClassPtr& parent){

}

void Base::finalize(){

}

void Base::make_instance_variables(){
	if(instance_variables_==&empty_instance_variables){
		InstanceVariables* temp = (InstanceVariables*)so_malloc(sizeof(InstanceVariables));
		new(temp) InstanceVariables();
		instance_variables_ = temp;

		if(get_class()){
			class_->inc_ref_count();
		}
	}
}

void Base::visit_members(Visitor& m){
	if(instance_variables_!=&empty_instance_variables){
		m & get_class();
		instance_variables_->visit_members(m);
	}
}

}

