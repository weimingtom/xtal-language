#include "xtal.h"
#include "xtal_macro.h"

namespace xtal{

Null null;
Undefined undefined;
Named null_named;
IDPtr empty_id(unchecked_ptr_cast<ID>(ap(Any(TYPE_SMALL_STRING))));
StringPtr empty_string(unchecked_ptr_cast<String>(ap(Any(TYPE_SMALL_STRING))));

FunInfo empty_xfun_info;
ScopeInfo empty_scope_info;
ClassInfo empty_class_info;
ExceptInfo empty_except_info;
EmptyInstanceVariables empty_instance_variables;

undeleter_t undeleter;
deleter_t deleter;


bool Int::op_in(const IntRangePtr& range){
	return range->begin() <= ivalue(*this) && ivalue(*this) < range->end();
}

bool Int::op_in(const FloatRangePtr& range){
	return (range->is_left_closed() ? (range->left() <= ivalue(*this)) : (range->left() < ivalue(*this))) && (range->is_right_closed() ? (ivalue(*this) <= range->right()) : (ivalue(*this) < range->right()));
}

IntRangePtr Int::op_range(int_t right, int_t kind){
	return xnew<IntRange>(ivalue(*this), right, kind);	
}

FloatRangePtr Int::op_range(float_t right, int_t kind){
	return xnew<FloatRange>(fvalue(*this), right, kind);	
}

FloatRangePtr Float::op_range(int_t right, int_t kind){
	return xnew<FloatRange>(fvalue(*this), (float_t)right, kind);	
}

FloatRangePtr Float::op_range(float_t right, int_t kind){
	return xnew<FloatRange>(fvalue(*this), right, kind);	
}

bool Float::op_in(const IntRangePtr& range){
	return range->begin() <= fvalue(*this) && fvalue(*this) < range->end();
}

bool Float::op_in(const FloatRangePtr& range){
	return (range->is_left_closed() ? (range->left() <= fvalue(*this)) : (range->left() < fvalue(*this))) && (range->is_right_closed() ? (fvalue(*this) <= range->right()) : (fvalue(*this) < range->right()));
}


AnyPtr IntRange::each(){
	return xnew<IntRangeIter>(from_this(this));
}

void IntRangeIter::block_next(const VMachinePtr& vm){
	if(it_<end_){
		vm->return_result(from_this(this), it_);
		++it_;
	}
	else{
		vm->return_result(null, null);
	}
}

StringPtr HaveName::object_name(int_t depth){
	if(!name_){
		return xnew<String>("instance of ")->cat(get_class()->object_name(depth));
	}

	if(!parent_ || depth==0){
		return name_;
	}

	return parent_->object_name()->cat("::")->cat(name_);
}

ArrayPtr HaveName::object_name_list(){
	if(!name_){
		return null;
	}

	ArrayPtr ret = parent_->object_name_list();
	if(ret){
		ret->push_back(name_);
	}

	return ret;
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

GCObserver::GCObserver(){
	core()->register_gc_observer(this);
}

GCObserver::GCObserver(const GCObserver& v)
:Base(v){
	core()->register_gc_observer(this);
}
	
GCObserver::~GCObserver(){
	core()->unregister_gc_observer(this);
}

}
