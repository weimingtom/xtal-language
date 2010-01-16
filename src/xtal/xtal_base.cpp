#include "xtal.h"
#include "xtal_macro.h"

namespace xtal{

void RefCountingBase::on_rawcall(const VMachinePtr& vm){
	to_smartptr(this)->rawsend(vm, Xid(op_call));
}

const AnyPtr& RefCountingBase::on_rawmember(const IDPtr& primary_key, const AnyPtr& secondary_key, bool inherited_too, int_t& accessibility, bool& nocache){
	return undefined;
}

void RefCountingBase::on_def(const IDPtr& primary_key, const AnyPtr& value, const AnyPtr& secondary_key, int_t accessibility){

}

const ClassPtr& RefCountingBase::on_object_parent(){
	return unchecked_ptr_cast<Class>(null);
}

void RefCountingBase::on_set_object_parent(const ClassPtr& parent){

}

void RefCountingBase::on_finalize(){

}

Base::Base(){
	value_.init(TYPE_BASE, this);
	class_ = (Class*)&null;
	instance_variables_ = &empty_instance_variables; 
}

Base::Base(const Base& b){
	value_.init(TYPE_BASE, this);
	if(b.instance_variables_!=&empty_instance_variables){
		instance_variables_ = (InstanceVariables*)xmalloc(sizeof(InstanceVariables));
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
			xfree(instance_variables_, sizeof(InstanceVariables));

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
		xfree(instance_variables_, sizeof(InstanceVariables));

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
	
void Base::make_instance_variables(){
	if(instance_variables_==&empty_instance_variables){
		InstanceVariables* temp = (InstanceVariables*)xmalloc(sizeof(InstanceVariables));
		new(temp) InstanceVariables();
		instance_variables_ = temp;

		if(get_class()){
			class_->inc_ref_count();
		}
	}
}

void Base::on_visit_members(Visitor& m){
	if(instance_variables_!=&empty_instance_variables){
		ClassPtr temp = to_smartptr(class_);
		m & temp;
		class_ = temp.get();
		instance_variables_->visit_members(m);
	}
}

}

