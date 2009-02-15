#include "xtal.h"
#include "xtal_macro.h"

namespace xtal{

static Named null_params[16];

CFunPtr new_cfun(const param_types_holder_n& pth, const void* val, int_t val_size){
	return xnew<CFun>(pth, val, val_size);
}


CFun::CFun(const param_types_holder_n& pth, const void* val, int_t val_size){
	fun_ = pth.fun;
	val_ = so_malloc(val_size);
	val_size_ = val_size;
	std::memcpy(val_, val, val_size);
	param_n_ = pi_.min_param_count = pi_.max_param_count = pth.param_n;
	pi_.params = null_params;
	this_ = undefined;
 
	int_t pn = (param_n_<0 ? -param_n_-1 : param_n_) + 1;
	param_types_ = (Class**)so_malloc(sizeof(Class*)*pn);
	for(int_t i=0; i<pn; ++i){
		const ClassPtr& cls = core()->get_cpp_class(pth.param_types[i]);
		if(raweq(cls, get_cpp_class<Any>()) || raweq(cls, get_cpp_class<void>())){
			param_types_[i] = 0;
		}
		else{
			param_types_[i] = cls.get();
		}
	}
}


CFun::~CFun(){
	for(int_t i=0; i<param_n_; ++i){
		pi_.params[i].~Named();
	}

	if(pi_.params!=null_params){
		so_free(pi_.params, sizeof(Named)*param_n_);		
	}

	int_t pn = (param_n_<0 ? -param_n_-1 : param_n_) + 1;
	so_free(param_types_, sizeof(Class*)*pn);

	so_free(val_, val_size_);
}

void CFun::make_params_place(){
	if(pi_.params==null_params && param_n_>0){
		pi_.params = (Named*)so_malloc(sizeof(Named)*param_n_);

		for(int_t i=0; i<param_n_; ++i){
			new(&pi_.params[i]) Named();
		}
	}
}

void CFun::check_params(){
	pi_.min_param_count = pi_.max_param_count = param_n_;
	for(int_t i=0; i<param_n_; ++i){
		if(rawne(pi_.params[i].value, undefined)){
			pi_.min_param_count--;
		}
	}
}

//{REPEAT{{
/*
const CFunPtr& CFun::params(#REPEAT_COMMA#const IDPtr& key`i`, const Any& value`i`#){
	make_params_place();
	#REPEAT#if(param_n_>`i`){ pi_.params[`i`].name = key`i`; pi_.params[`i`].value = ap(value`i`); }#
	check_params();
	return from_this(this);
}
*/

const CFunPtr& CFun::params(){
	make_params_place();
	
	check_params();
	return from_this(this);
}

const CFunPtr& CFun::params(const IDPtr& key0, const Any& value0){
	make_params_place();
	if(param_n_>0){ pi_.params[0].name = key0; pi_.params[0].value = ap(value0); }
	check_params();
	return from_this(this);
}

const CFunPtr& CFun::params(const IDPtr& key0, const Any& value0, const IDPtr& key1, const Any& value1){
	make_params_place();
	if(param_n_>0){ pi_.params[0].name = key0; pi_.params[0].value = ap(value0); }if(param_n_>1){ pi_.params[1].name = key1; pi_.params[1].value = ap(value1); }
	check_params();
	return from_this(this);
}

const CFunPtr& CFun::params(const IDPtr& key0, const Any& value0, const IDPtr& key1, const Any& value1, const IDPtr& key2, const Any& value2){
	make_params_place();
	if(param_n_>0){ pi_.params[0].name = key0; pi_.params[0].value = ap(value0); }if(param_n_>1){ pi_.params[1].name = key1; pi_.params[1].value = ap(value1); }if(param_n_>2){ pi_.params[2].name = key2; pi_.params[2].value = ap(value2); }
	check_params();
	return from_this(this);
}

const CFunPtr& CFun::params(const IDPtr& key0, const Any& value0, const IDPtr& key1, const Any& value1, const IDPtr& key2, const Any& value2, const IDPtr& key3, const Any& value3){
	make_params_place();
	if(param_n_>0){ pi_.params[0].name = key0; pi_.params[0].value = ap(value0); }if(param_n_>1){ pi_.params[1].name = key1; pi_.params[1].value = ap(value1); }if(param_n_>2){ pi_.params[2].name = key2; pi_.params[2].value = ap(value2); }if(param_n_>3){ pi_.params[3].name = key3; pi_.params[3].value = ap(value3); }
	check_params();
	return from_this(this);
}

const CFunPtr& CFun::params(const IDPtr& key0, const Any& value0, const IDPtr& key1, const Any& value1, const IDPtr& key2, const Any& value2, const IDPtr& key3, const Any& value3, const IDPtr& key4, const Any& value4){
	make_params_place();
	if(param_n_>0){ pi_.params[0].name = key0; pi_.params[0].value = ap(value0); }if(param_n_>1){ pi_.params[1].name = key1; pi_.params[1].value = ap(value1); }if(param_n_>2){ pi_.params[2].name = key2; pi_.params[2].value = ap(value2); }if(param_n_>3){ pi_.params[3].name = key3; pi_.params[3].value = ap(value3); }if(param_n_>4){ pi_.params[4].name = key4; pi_.params[4].value = ap(value4); }
	check_params();
	return from_this(this);
}

//}}REPEAT}

void CFun::visit_members(Visitor& m){
	HaveName::visit_members(m);
	m & this_;
	if(param_n_>0){
		std::for_each(pi_.params, pi_.params+param_n_, m);
	}
}


void CFun::rawcall(const VMachinePtr& vm){
	if(param_n_>=0 && vm->ordered_arg_count()!=pi_.min_param_count){
		int_t n = vm->ordered_arg_count();
		if(n<pi_.min_param_count || n>pi_.max_param_count){
			if(pi_.min_param_count==0 && pi_.max_param_count==0){
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
						Named(Xid(min), pi_.min_param_count),
						Named(Xid(max), pi_.max_param_count),
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
		{
			const AnyPtr& arg = vm->arg_this();
			if(param_types_[0] && !arg->is(from_this(param_types_[0]))){
				vm->set_except(argument_error(vm->ff().hint()->object_name(), 0, from_this(param_types_[0]), arg->get_class()));
				return;
			}
		}

		vm->adjust_args(pi_.params, param_n_);

		for(int_t i=0; i<param_n_; ++i){
			const AnyPtr& arg = vm->arg_unchecked(i);
			if(param_types_[i+1] && !arg->is(from_this(param_types_[i+1]))){
				vm->set_except(argument_error(vm->ff().hint()->object_name(), i+1, from_this(param_types_[i+1]), arg->get_class()));
				return;
			}
		}
	}

	VMAndData pvm(vm, val_);
	fun_(pvm);
}

DualDispatchMethodPtr dual_dispatch_method(const IDPtr& primary_key){
	return xnew<DualDispatchMethod>(primary_key);
}

DualDispatchFunPtr dual_dispatch_fun(const ClassPtr& klass, const IDPtr& primary_key){
	return xnew<DualDispatchFun>(klass, primary_key);
}

}
