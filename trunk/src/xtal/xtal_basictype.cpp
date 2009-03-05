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

HaveParent::HaveParent(const HaveParent& a)
:Base(a), parent_(0){
}

HaveParent& HaveParent::operator=(const HaveParent& a){
	Base::operator=(a);
	return *this;
}

HaveParent::~HaveParent(){
	if(parent_){
		parent_->dec_ref_count();
	}
}

const ClassPtr& HaveParent::object_parent(){
	if(parent_){
		return from_this(parent_);
	}
	else{
		return null;
	}
}

void HaveParent::set_object_parent(const ClassPtr& parent){
	if(!parent_ || parent_->object_force()<parent->object_force()){
		if(parent_){
			parent_->dec_ref_count();
		}

		if(parent){
			parent_ = parent.get();
			parent_->inc_ref_count();
		}
		else{
			parent_ = 0;
		}
	}
}

RefCountingHaveParent::RefCountingHaveParent(const RefCountingHaveParent& a)
:RefCountingBase(a), parent_(0){
}

RefCountingHaveParent& RefCountingHaveParent::operator=(const RefCountingHaveParent& a){
	RefCountingBase::operator=(a);
	return *this;
}

RefCountingHaveParent::~RefCountingHaveParent(){
	if(parent_){
		parent_->dec_ref_count();
	}
}

const ClassPtr& RefCountingHaveParent::object_parent(){
	if(parent_){
		return from_this(parent_);
	}
	else{
		return null;
	}
}

void RefCountingHaveParent::set_object_parent(const ClassPtr& parent){
	if(!parent_ || parent_->object_force()<parent->object_force()){
		if(parent_){
			parent_->dec_ref_count();
		}

		if(parent){
			parent_ = parent.get();
			parent_->inc_ref_count();
		}
		else{
			parent_ = 0;
		}
	}
}

GCObserver::GCObserver(){
	register_gc_observer(this);
}

GCObserver::GCObserver(const GCObserver& v)
:Base(v){
	register_gc_observer(this);
}
	
GCObserver::~GCObserver(){
	unregister_gc_observer(this);
}

}
