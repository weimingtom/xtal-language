#include "xtal.h"
#include "xtal_macro.h"

namespace xtal{

NativeFunPtr new_native_fun(const param_types_holder_n& pth, const void* val, int_t val_size){
	return xnew<NativeFun>(pth, val, val_size);
}

NativeFunPtr new_native_fun(const param_types_holder_n& pth, const void* val, int_t val_size, const AnyPtr& this_){
	return xnew<NativeFunBindedThis>(pth, val, val_size, this_);
}

NativeFun::NativeFun(const param_types_holder_n& pth, const void* val, int_t val_size){
	set_pvalue(*this, TYPE, this);

	fun_ = pth.fun;
	if(val_size<sizeof(int_t)){
		val_size_ = 0;
	}
	else{
		val_size_ = val_size;
	}

	if(pth.extendable!=0){
		min_param_count_ = 0;
		max_param_count_ = 255;
		param_n_ = pth.param_n;
	}
	else{
		min_param_count_ = pth.param_n;
		max_param_count_ = pth.param_n;
		param_n_ = pth.param_n;
	}

	uint_t data_size = val_size_ + (param_n_+1)*sizeof(Class*) + param_n_*sizeof(Named);
	data_ = xmalloc(data_size);

	// 関数をコピー
	std::memcpy(data_, val, val_size);

	Class** param_types = (Class**)((char*)data_ +  val_size_);
	for(int_t i=0; i<param_n_+1; ++i){
		const ClassPtr& cls = cpp_class(pth.param_types[i]);
		if(raweq(cls, cpp_class<Any>()) || raweq(cls, cpp_class<void>())){
			param_types[i] = 0;
		}
		else{
			param_types[i] = cls.get();
		}
	}

	if(param_n_){
		Named* params = (Named*)((char*)param_types + (param_n_+1)*sizeof(Class*));
		for(int_t i=0; i<param_n_; ++i){
			new(&params[i]) Named();
		}
	}
}


NativeFun::~NativeFun(){
	Class** param_types = (Class**)((char*)data_ +  val_size_);
	Named* params = (Named*)((char*)param_types + (param_n_+1)*sizeof(Class*));

	for(int_t i=0; i<param_n_; ++i){
		params[i].~Named();
	}

	uint_t data_size = val_size_ + (param_n_+1)*sizeof(Class*) + param_n_*sizeof(Named);
	xfree(data_, data_size);
}

const NativeFunPtr& NativeFun::param(int_t i, const IDPtr& key, const Any& value){
	// iは1始まり
	XTAL_ASSERT(i!=0);

	// iが引数の数より大きすぎる
	XTAL_ASSERT(i<=param_n_);

	// 今のところ、VMachinePtrを引数にする場合、paramは設定でない制限あり
	XTAL_ASSERT(!(min_param_count_==0 && max_param_count_==255));

	i--;

	Class** param_types = (Class**)((char*)data_ +  val_size_);
	Named* params = (Named*)((char*)param_types + (param_n_+1)*sizeof(Class*));

	// 既に設定済み
	XTAL_ASSERT(raweq(params[i].name, null) && raweq(params[i].value, undefined));

	params[i].name = key;
	params[i].value = ap(value);

	if(min_param_count_>i){
		min_param_count_ = i;
	}

	return to_smartptr(this);
}

void NativeFun::visit_members(Visitor& m){
	RefCountingHaveParent::visit_members(m);

	Class** param_types = (Class**)((char*)data_ +  val_size_);
	Named* params = (Named*)((char*)param_types + (param_n_+1)*sizeof(Class*));

	for(uint_t i=0; i<param_n_; ++i){
		m & params[i];
	}
}


void NativeFun::rawcall(const VMachinePtr& vm){
	if(vm->ordered_arg_count()!=min_param_count_){
		int_t n = vm->ordered_arg_count();
		if(n<min_param_count_ || n>max_param_count_){
			if(min_param_count_==0 && max_param_count_==0){
				vm->set_except(cpp_class<ArgumentError>()->call(
					Xt("Xtal Runtime Error 1007")->call(
						Named(Xid(object), vm->ff().hint()->object_name()),
						Named(Xid(value), n)
					)
				));
				return;
			}
			else{
				vm->set_except(cpp_class<ArgumentError>()->call(
					Xt("Xtal Runtime Error 1006")->call(
						Named(Xid(object), vm->ff().hint()->object_name()),
						Named(Xid(min), min_param_count_),
						Named(Xid(max), max_param_count_),
						Named(Xid(value), n)
					)
				));
				return;
			}
		}
	}
	
	{ // check arg type
		Class** param_types = (Class**)((char*)data_ +  val_size_);
		Named* params = (Named*)((char*)param_types + (param_n_+1)*sizeof(Class*));

		{
			const AnyPtr& arg = vm->arg_this();
			if(param_types[0]){
				if(!arg->is(to_smartptr(param_types[0]))){
					vm->set_except(argument_error(vm->ff().hint()->object_name(), 0, to_smartptr(param_types[0]), arg->get_class()));
					return;
				}
			}
		}

		if(vm->ordered_arg_count()!=param_n_){
			vm->adjust_args(params, param_n_);
		}

		for(int_t i=0; i<param_n_; ++i){
			const AnyPtr& arg = vm->arg_unchecked(i);

			if(param_types[i+1]){
				if(!arg->is(to_smartptr(param_types[i+1]))){ 
					vm->set_except(argument_error(vm->ff().hint()->object_name(), i+1, to_smartptr(param_types[i+1]), arg->get_class()));
					return;
				}
			}
		}
	}

	VMAndData pvm(vm, data_);
	fun_(pvm);
}


NativeFunBindedThis::NativeFunBindedThis(const param_types_holder_n& pth, const void* val, int_t val_size, const AnyPtr& this_)
:NativeFun(pth, val, val_size), this_(this_){
	set_pvalue(*this, TYPE, this);
}

void NativeFunBindedThis::visit_members(Visitor& m){
	NativeFun::visit_members(m);
	m & this_;
}

void NativeFunBindedThis::rawcall(const VMachinePtr& vm){
	vm->set_arg_this(this_);
	NativeFun::rawcall(vm);
}


DoubleDispatchMethodPtr double_dispatch_method(const IDPtr& primary_key){
	return xnew<DoubleDispatchMethod>(primary_key);
}

DoubleDispatchFunPtr double_dispatch_fun(const ClassPtr& klass, const IDPtr& primary_key){
	return xnew<DoubleDispatchFun>(klass, primary_key);
}

}
