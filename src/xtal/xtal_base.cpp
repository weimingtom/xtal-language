#include "xtal.h"
#include "xtal_macro.h"

namespace xtal{

Base::Base()
	:Any(noinit_t()){
	set_p(this);
	//ref_count_ = 0;
	class_ = null;
	instance_variables_ = &empty_instance_variables; 
}

Base::Base(const Base& b)
	:Any(noinit_t()){
	set_p(this);
	//ref_count_ = 0;
	if(b.instance_variables_!=&empty_instance_variables){
		instance_variables_ = (InstanceVariables*)user_malloc(sizeof(InstanceVariables));
		new(instance_variables_) InstanceVariables(*b.instance_variables_);		

		class_ = b.class_;
		if(type(class_)==TYPE_BASE){
			pvalue(class_)->inc_ref_count();
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

		if(type(class_)==TYPE_BASE){
			pvalue(class_)->dec_ref_count();
		}
		class_ = b.class_;
		if(type(class_)==TYPE_BASE){
			pvalue(class_)->inc_ref_count();
		}
	}
	else{
		if(instance_variables_!=&empty_instance_variables){
			instance_variables_->~InstanceVariables();
			user_free(instance_variables_);

			if(type(class_)==TYPE_BASE){
				pvalue(class_)->dec_ref_count();
			}

			class_ = null;
			instance_variables_ = &empty_instance_variables; 
		}
	}

	return *this;
}

Base::~Base(){
	if(instance_variables_!=&empty_instance_variables){
		instance_variables_->~InstanceVariables();
		user_free(instance_variables_);

		if(type(class_)==TYPE_BASE){
			pvalue(class_)->dec_ref_count();
		}
	}
}

void Base::set_class(const ClassPtr& c){
	if(instance_variables_==&empty_instance_variables){
		class_ = c;
	}
	else{
		if(type(class_)==TYPE_BASE){
			pvalue(class_)->dec_ref_count();
		}
		class_ = c;
		if(type(class_)==TYPE_BASE){
			pvalue(class_)->inc_ref_count();
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

StringPtr Base::object_name(int_t depth){
	return xnew<String>("instance of ")->cat(get_class()->object_name(depth));
}

int_t Base::object_name_force(){ 
	return 0;
}

void Base::set_object_name(const StringPtr& name, int_t force, const AnyPtr& parent){

}

ArrayPtr Base::object_name_list(){
	return null;
}

void Base::finalize(){

}

uint_t Base::hashcode(){
	return ((uint_t)this)>>2;
}

void Base::make_instance_variables(){
	if(instance_variables_==&empty_instance_variables){
		InstanceVariables* temp = (InstanceVariables*)user_malloc(sizeof(InstanceVariables));
		new(temp) InstanceVariables();
		instance_variables_ = temp;

		if(type(class_)==TYPE_BASE){
			pvalue(class_)->inc_ref_count();
		}
	}
}

void Base::visit_members(Visitor& m){
	if(instance_variables_!=&empty_instance_variables){
		if(type(class_)==TYPE_BASE){
			m & class_;
		}
		instance_variables_->visit_members(m);
	}
}

}

