#include "xtal.h"
#include "xtal_macro.h"

namespace xtal{

NativeFunPtr new_native_fun(const param_types_holder_n& pth, const void* val){
	return xnew<NativeMethod>(pth, val);
}

NativeFunPtr new_native_fun(const param_types_holder_n& pth, const void* val, const AnyPtr& this_){
	return xnew<NativeFun>(pth, val, this_);
}

NativeMethod::NativeMethod(const param_types_holder_n& pth, const void* val){
	set_pvalue(*this, TYPE, this);

	fun_ = pth.fun;
	if(pth.size<(int_t)sizeof(int_t)){
		val_size_ = 0;
	}
	else{
		val_size_ = pth.size;
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

	uint_t data_size = val_size_ + (param_n_+1)*sizeof(Class*) + param_n_*sizeof(NamedParam);
	data_ = xmalloc(data_size);

	// 関数をコピー
	std::memcpy(data_, val, val_size_);

	Class** param_types = (Class**)((u8*)data_ +  val_size_);
	for(int_t i=0; i<param_n_+1; ++i){
		const ClassPtr& cls = cpp_class(*pth.param_types[i]);
		if(raweq(cls, cpp_class<Any>()) || raweq(cls, cpp_class<void>())){
			param_types[i] = 0;
		}
		else{
			param_types[i] = cls.get();
		}
	}

	if(param_n_){
		NamedParam* params = (NamedParam*)((u8*)param_types + (param_n_+1)*sizeof(Class*));
		for(int_t i=0; i<param_n_; ++i){
			new(&params[i]) NamedParam();
		}
	}
}

NativeMethod::~NativeMethod(){
	Class** param_types = (Class**)((u8*)data_ +  val_size_);
	NamedParam* params = (NamedParam*)((u8*)param_types + (param_n_+1)*sizeof(Class*));

	for(int_t i=0; i<param_n_; ++i){
		params[i].~NamedParam();
	}

	uint_t data_size = val_size_ + (param_n_+1)*sizeof(Class*) + param_n_*sizeof(NamedParam);
	xfree(data_, data_size);
}

const NativeFunPtr& NativeMethod::param(int_t i, const IDPtr& key, const AnyPtr& value){
	// iは1始まり
	XTAL_ASSERT(i!=0);

	// iが引数の数より大きすぎる
	XTAL_ASSERT(i<=param_n_);

	// 今のところ、VMachinePtrを引数にする場合、paramは設定でない制限あり
	XTAL_ASSERT(!(min_param_count_==0 && max_param_count_==255));

	i--;

	Class** param_types = (Class**)((u8*)data_ +  val_size_);
	NamedParam* params = (NamedParam*)((u8*)param_types + (param_n_+1)*sizeof(Class*));

	// 既に設定済み
	XTAL_ASSERT(raweq(params[i].name, null) && raweq(params[i].value, undefined));

	params[i].name = key;
	params[i].value = value;

	if(min_param_count_>i){
		min_param_count_ = i;
	}

	return to_smartptr(this);
}

void NativeMethod::visit_members(Visitor& m){
	HaveParentRefCountingBase::visit_members(m);

	Class** param_types = (Class**)((u8*)data_ +  val_size_);
	NamedParam* params = (NamedParam*)((u8*)param_types + (param_n_+1)*sizeof(Class*));

	for(uint_t i=0; i<param_n_; ++i){
		m & params[i].name & params[i].value;
	}
}


void NativeMethod::rawcall(const VMachinePtr& vm){
	if(vm->ordered_arg_count()!=min_param_count_){
		int_t n = vm->ordered_arg_count();
		if(n<min_param_count_ || n>max_param_count_){
			if(min_param_count_==0 && max_param_count_==0){
				vm->set_except(cpp_class<ArgumentError>()->call(
					Xt("Xtal Runtime Error 1007")->call(
						Named(Xid(object), object_name()),
						Named(Xid(value), n)
					)
				));
				return;
			}
			else{
				vm->set_except(cpp_class<ArgumentError>()->call(
					Xt("Xtal Runtime Error 1006")->call(
						Named(Xid(object), object_name()),
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
		Class** param_types = (Class**)((u8*)data_ +  val_size_);
		NamedParam* params = (NamedParam*)((u8*)param_types + (param_n_+1)*sizeof(Class*));

		{
			const AnyPtr& arg = vm->arg_this();
			if(param_types[0]){
				if(!arg->is(to_smartptr(param_types[0]))){
					vm->set_except(argument_error(object_name(), 0, to_smartptr(param_types[0]), arg->get_class()));
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
					vm->set_except(argument_error(object_name(), i+1, to_smartptr(param_types[i+1]), arg->get_class()));
					return;
				}
			}
		}
	}

	fun_(vm, data_);
}


NativeFun::NativeFun(const param_types_holder_n& pth, const void* val, const AnyPtr& this_)
:NativeMethod(pth, val), this_(this_){
	set_pvalue(*this, TYPE, this);
}

void NativeFun::visit_members(Visitor& m){
	NativeMethod::visit_members(m);
	m & this_;
}

void NativeFun::rawcall(const VMachinePtr& vm){
	vm->set_arg_this(this_);
	NativeMethod::rawcall(vm);
}


DoubleDispatchMethodPtr double_dispatch_method(const IDPtr& primary_key){
	return xnew<DoubleDispatchMethod>(primary_key);
}

DoubleDispatchFunPtr double_dispatch_fun(const ClassPtr& klass, const IDPtr& primary_key){
	return xnew<DoubleDispatchFun>(klass, primary_key);
}

}
