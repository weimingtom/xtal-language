#include "xtal.h"
#include "xtal_macro.h"

namespace xtal{

NativeFunPtr new_native_fun(const param_types_holder_n& pth, const void* val){
	return xnew<NativeMethod>(pth, val);
}

NativeFunPtr new_native_fun(const param_types_holder_n& pth, const void* val, const AnyPtr& this_){
	return xnew<NativeFun>(pth, val, this_);
}

namespace{
	void set_arg_error_except(const VMachinePtr& vm, const StringPtr& name, int_t n, int_t minv, int_t maxv){
		if(minv==0 && maxv==0){
			vm->set_except(cpp_class<ArgumentError>()->call(
				Xt("XRE1007")->call(
					Named(Xid(object), name),
					Named(Xid(value), n)
				)
			));
		}
		else{
			vm->set_except(cpp_class<ArgumentError>()->call(
				Xt("XRE1006")->call(
					Named(Xid(object), name),
					Named(Xid(min), minv),
					Named(Xid(max), maxv),
					Named(Xid(value), n)
				)
			));
		}
	}
}

StatelessNativeMethod::StatelessNativeMethod(const param_types_holder_n& pth)
	{
	value_.init_stateless_native_method(&pth);
}

void StatelessNativeMethod::on_rawcall(const VMachinePtr& vm){
	const param_types_holder_n& pth = *value_.pth();
	int_t param_n = pth.param_n;

	Any args[16];
	
	{ // check arg type

		const CppClassSymbolData* anycls = CppClassSymbol<Any>::value;

		{
			const AnyPtr& arg = vm->arg_this();
			args[0] = (Any&)arg;

			if(anycls!=*pth.param_types[0]){
				const ClassPtr& cls = cpp_class(*pth.param_types[0]);
				if(!arg->is(cls)){
					vm->set_except(argument_error(Xid(NativeFunction), 0, cls, arg->get_class()));
					return;
				}
			}
		}

		if(!pth.vm){
			if(pth.extendable){
				vm->set_local_variable(param_n-1, vm->make_arguments(param_n-1));
			}

			for(int_t i=0; i<param_n; ++i){
				const AnyPtr& arg = vm->arg_unchecked(i);
				args[i+1] = (Any&)arg;

				if(anycls!=*pth.param_types[i+1]){
					const ClassPtr& cls = cpp_class(*pth.param_types[i+1]);
					if(!arg->is(cls)){ 
						vm->set_except(argument_error(object_name(), i+1, cls, arg->get_class()));
						return;
					}
				}
			}
		}
	}

	pth.fun(&*vm, args, 0);
}

NativeMethod::NativeMethod(const param_types_holder_n& pth, const void* val){
	value_.init_rcbase(TYPE, this);

	vm_ = pth.vm;
	fun_ = pth.fun;
	if(pth.size==1){
		val_size_ = 0;
	}
	else{
		val_size_ = pth.size;
	}

	extendable_ = pth.extendable;

	if(pth.vm){
		min_param_count_ = 0;
		max_param_count_ = 255;
		param_n_ = pth.param_n;
	}
	else if(pth.extendable){
		min_param_count_ = pth.param_n-1;
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
		if(raweq(cls, cpp_class<Any>())){
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

	// 今のところ、VMachineを引数にする場合、paramは設定でない制限あり
	XTAL_ASSERT(!(min_param_count_==0 && max_param_count_==255));

	// iが引数の数より大きすぎる
	XTAL_ASSERT(i<=param_n_);

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

void NativeMethod::on_visit_members(Visitor& m){
	HaveParentRefCountingBase::on_visit_members(m);

	Class** param_types = (Class**)((u8*)data_ +  val_size_);
	NamedParam* params = (NamedParam*)((u8*)param_types + (param_n_+1)*sizeof(Class*));

	for(uint_t i=0; i<param_n_; ++i){
		m & params[i].name & params[i].value;
	}
}

void NativeMethod::on_rawcall(const VMachinePtr& vm){
	if(vm->ordered_arg_count()!=min_param_count_){
		int_t n = vm->ordered_arg_count();
		if(n<min_param_count_ || n>max_param_count_){
			set_arg_error_except(vm, object_name(), n, min_param_count_, max_param_count_);
		}
	}

	Any args[16];
	
	{ // check arg type
		Class** param_types = (Class**)((u8*)data_ +  val_size_);
		NamedParam* params = (NamedParam*)((u8*)param_types + (param_n_+1)*sizeof(Class*));

		{
			const AnyPtr& arg = vm->arg_this();
			args[0] = (Any&)arg;

			if(param_types[0]){
				if(!arg->is(to_smartptr(param_types[0]))){
					vm->set_except(argument_error(object_name(), 0, to_smartptr(param_types[0]), arg->get_class()));
					return;
				}
			}
		}

		if(!vm_){
			if(extendable_){
				vm->set_local_variable(param_n_-1, vm->inner_make_arguments(params, param_n_-1));
			}
			else{
				if(vm->ordered_arg_count()!=param_n_){
					vm->adjust_args(params, param_n_);
				}
			}

			for(int_t i=0; i<param_n_; ++i){
				const AnyPtr& arg = vm->arg_unchecked(i);
				args[i+1] = (Any&)arg;

				if(param_types[i+1]){
					if(!arg->is(to_smartptr(param_types[i+1]))){ 
						vm->set_except(argument_error(object_name(), i+1, to_smartptr(param_types[i+1]), arg->get_class()));
						return;
					}
				}
			}
		}
	}

	fun_(&*vm, args, data_);
}


NativeFun::NativeFun(const param_types_holder_n& pth, const void* val, const AnyPtr& this_)
:NativeMethod(pth, val), this_(this_){
	value_.init_rcbase(TYPE, this);
}

void NativeFun::on_visit_members(Visitor& m){
	NativeMethod::on_visit_members(m);
	m & this_;
}

void NativeFun::on_rawcall(const VMachinePtr& vm){
	vm->set_arg_this(this_);
	NativeMethod::on_rawcall(vm);
}


DoubleDispatchMethodPtr double_dispatch_method(const IDPtr& primary_key){
	return xnew<DoubleDispatchMethod>(primary_key);
}

DoubleDispatchFunPtr double_dispatch_fun(const ClassPtr& klass, const IDPtr& primary_key){
	return xnew<DoubleDispatchFun>(klass, primary_key);
}

}
