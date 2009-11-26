#include "xtal.h"
#include "xtal_macro.h"

namespace xtal{

NullPtr null(unchecked_ptr_cast<Null>(ap(Null())));
UndefinedPtr undefined(unchecked_ptr_cast<Undefined>(ap(Undefined())));

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
	return xnew<IntRangeIter>(to_smartptr(this));
}

void IntRangeIter::block_next(const VMachinePtr& vm){
	if(it_<end_){
		vm->return_result(to_smartptr(this), it_);
		++it_;
	}
	else{
		vm->return_result(null, null);
	}
}

void Values::block_next(const VMachinePtr& vm){
	if(!raweq(tail_, undefined)){
		vm->return_result(tail_, head_);
	}
	else{
		vm->return_result(0, head_);
	}
}

int_t Values::size(){
	const ValuesPtr* cur = &to_smartptr(this);
	int_t size = 1;
	while(true){
		if(raweq((*cur)->tail_, undefined)){
			return size;
		}
		cur = &(*cur)->tail_;
		++size;
	}
}

const AnyPtr& Values::at(int_t i){
	const ValuesPtr* cur = &to_smartptr(this);
	const AnyPtr* ret = &head_;
	for(int_t n=0; n<i; ++n){
		if(raweq((*cur)->tail_, undefined)){
			ret = &undefined;
			break;
		}
		cur = &(*cur)->tail_;
		ret = &(*cur)->head_;
	}
	return *ret;
}

bool Values::op_eq(const ValuesPtr& other){
	const VMachinePtr& vm = vmachine();

	const ValuesPtr* cur1 = &to_smartptr(this);
	const ValuesPtr* cur2 = &other;

	while(true){
		if(rawne((*cur1)->head_, (*cur2)->head_)){
			vm->setup_call(1, (*cur2)->head_);
			(*cur1)->head_->rawsend(vm, Xid(op_eq), undefined, true, true);

			if(!vm->processed() || !vm->result()){
				vm->cleanup_call();
				return false;
			}
			vm->cleanup_call();
		}

		if(raweq((*cur1)->tail_, undefined)){
			if(raweq((*cur2)->tail_, undefined)){
				return true;
			}
			else{
				return false;
			}
		}

		if(!(*cur2)->tail_){
			return false;
		}

		cur1 = &(*cur1)->tail_;
		cur2 = &(*cur2)->tail_;
	}

	return true;
}

///////////////////////////////////

HaveParent::HaveParent(const HaveParent& a)
:parent_(a.parent_){
	if(parent_){
		parent_->inc_ref_count();
	}
}

HaveParent& HaveParent::operator=(const HaveParent& a){
	if(parent_){
		parent_->dec_ref_count();
	}
	parent_ = a.parent_;
	if(parent_){
		parent_->inc_ref_count();
	}

	return *this;
}

HaveParent::~HaveParent(){
	if(parent_){
		parent_->dec_ref_count();
	}
}

const ClassPtr& HaveParent::object_parent(){
	if(parent_){
		return to_smartptr(parent_);
	}
	else{
		return unchecked_ptr_cast<Class>(null);
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

void HaveParent::visit_members(Visitor& m){
	if(parent_){
		ClassPtr temp = to_smartptr(parent_);
		m & temp;
		parent_ = temp.get();
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
