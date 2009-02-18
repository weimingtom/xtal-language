#include "xtal.h"
#include "xtal_macro.h"

namespace xtal{

CFunPtr new_cfun(const param_types_holder_n& pth, const void* val, int_t val_size){
	return xnew<CFun>(pth, val, val_size);
}

CFun::CFun(const param_types_holder_n& pth, const void* val, int_t val_size){
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
	data_ = so_malloc(data_size);

	// 関数をコピー
	std::memcpy(data_, val, val_size);

	this_ = undefined;
 
	Class** param_types = (Class**)((char*)data_ +  val_size_);
	for(int_t i=0; i<param_n_+1; ++i){
		const ClassPtr& cls = core()->get_cpp_class(pth.param_types[i]);
		if(raweq(cls, get_cpp_class<Any>()) || raweq(cls, get_cpp_class<void>())){
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


CFun::~CFun(){
	Class** param_types = (Class**)((char*)data_ +  val_size_);
	Named* params = (Named*)((char*)param_types + (param_n_+1)*sizeof(Class*));

	for(int_t i=0; i<param_n_; ++i){
		params[i].~Named();
	}

	uint_t data_size = val_size_ + (param_n_+1)*sizeof(Class*) + param_n_*sizeof(Named);
	so_free(data_, data_size);
}

void CFun::set_param(uint_t n, const IDPtr& key, const Any& value){
	if(min_param_count_==0 && max_param_count_==255){
		return;
	}

	// 名前つきデフォルト引数を与えすぎな場合にassertに失敗する
	XTAL_ASSERT(n<param_n_);

	Class** param_types = (Class**)((char*)data_ +  val_size_);
	Named* params = (Named*)((char*)param_types + (param_n_+1)*sizeof(Class*));

	if(raweq(params[n].value, undefined)){
		params[n].name = key;
		params[n].value = ap(value);

		if(rawne(value, undefined)){
			min_param_count_--;
		}
	}
	else{
		params[n].name = key;
		params[n].value = ap(value);
	}
}

//{REPEAT{{
/*
const CFunPtr& CFun::params(#REPEAT_COMMA#const IDPtr& key`i`, const Any& value`i`#){
	#REPEAT#set_param(`i`, key`i`, ap(value`i`));#
	return from_this(this);
}
*/

const CFunPtr& CFun::params(){
	
	return from_this(this);
}

const CFunPtr& CFun::params(const IDPtr& key0, const Any& value0){
	set_param(0, key0, ap(value0));
	return from_this(this);
}

const CFunPtr& CFun::params(const IDPtr& key0, const Any& value0, const IDPtr& key1, const Any& value1){
	set_param(0, key0, ap(value0));set_param(1, key1, ap(value1));
	return from_this(this);
}

const CFunPtr& CFun::params(const IDPtr& key0, const Any& value0, const IDPtr& key1, const Any& value1, const IDPtr& key2, const Any& value2){
	set_param(0, key0, ap(value0));set_param(1, key1, ap(value1));set_param(2, key2, ap(value2));
	return from_this(this);
}

const CFunPtr& CFun::params(const IDPtr& key0, const Any& value0, const IDPtr& key1, const Any& value1, const IDPtr& key2, const Any& value2, const IDPtr& key3, const Any& value3){
	set_param(0, key0, ap(value0));set_param(1, key1, ap(value1));set_param(2, key2, ap(value2));set_param(3, key3, ap(value3));
	return from_this(this);
}

const CFunPtr& CFun::params(const IDPtr& key0, const Any& value0, const IDPtr& key1, const Any& value1, const IDPtr& key2, const Any& value2, const IDPtr& key3, const Any& value3, const IDPtr& key4, const Any& value4){
	set_param(0, key0, ap(value0));set_param(1, key1, ap(value1));set_param(2, key2, ap(value2));set_param(3, key3, ap(value3));set_param(4, key4, ap(value4));
	return from_this(this);
}

//}}REPEAT}

void CFun::visit_members(Visitor& m){
	HaveParent::visit_members(m);
	m & this_;

	Class** param_types = (Class**)((char*)data_ +  val_size_);
	Named* params = (Named*)((char*)param_types + (param_n_+1)*sizeof(Class*));

	for(uint_t i=0; i<param_n_; ++i){
		m & params[i];
	}
}


void CFun::rawcall(const VMachinePtr& vm){
	if(vm->ordered_arg_count()!=min_param_count_){
		int_t n = vm->ordered_arg_count();
		if(n<min_param_count_ || n>max_param_count_){
			if(min_param_count_==0 && max_param_count_==0){
				vm->set_except(builtin()->member(Xid(ArgumentError))->call(
					Xt("Xtal Runtime Error 1007")->call(
						Named(Xid(object), vm->ff().hint()->object_name()),
						Named(Xid(value), n)
					)
				));
				return;
			}
			else{
				vm->set_except(builtin()->member(Xid(ArgumentError))->call(
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

	if(rawne(this_, undefined)){
		vm->set_arg_this(this_);
	}
	
	{ // check arg type
		Class** param_types = (Class**)((char*)data_ +  val_size_);
		Named* params = (Named*)((char*)param_types + (param_n_+1)*sizeof(Class*));

		{
			const AnyPtr& arg = vm->arg_this();
			if(param_types[0]){
				if(!arg->is(from_this(param_types[0]))){
					vm->set_except(argument_error(vm->ff().hint()->object_name(), 0, from_this(param_types[0]), arg->get_class()));
					return;
				}
			}
		}

		vm->adjust_args(params, param_n_);

		for(int_t i=0; i<param_n_; ++i){
			const AnyPtr& arg = vm->arg_unchecked(i);

			if(param_types[i+1]){
				if(!arg->is(from_this(param_types[i+1]))){ 
					vm->set_except(argument_error(vm->ff().hint()->object_name(), i+1, from_this(param_types[i+1]), arg->get_class()));
					return;
				}
			}
		}
	}

	VMAndData pvm(vm, data_);
	fun_(pvm);
}

DualDispatchMethodPtr dual_dispatch_method(const IDPtr& primary_key){
	return xnew<DualDispatchMethod>(primary_key);
}

DualDispatchFunPtr dual_dispatch_fun(const ClassPtr& klass, const IDPtr& primary_key){
	return xnew<DualDispatchFun>(klass, primary_key);
}

}
