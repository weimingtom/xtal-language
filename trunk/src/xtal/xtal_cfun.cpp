#include "xtal.h"
#include "xtal_macro.h"

namespace xtal{

Result result;
ReturnThis return_this;
ReturnUndefined return_void;


CFun::CFun(fun_t f, void* val, int_t param_n){
	fun_ = f;
	val_ = val;

	if(param_n){
		pi_.params = (Named*)user_malloc(sizeof(Named)*param_n);
	}
	else{
		pi_.params = 0;
	}

	param_n_ = pi_.min_param_count = pi_.max_param_count = param_n;
	for(int_t i=0; i<param_n; ++i){
		new(&pi_.params[i]) Named();
	}

	pi_.fun = this;
}

CFun::~CFun(){
	for(int_t i=0; i<param_n_; ++i){
		pi_.params[i].~Named();
	}
	user_free(pi_.params);		
	user_free(val_);
}

CFunPtr CFun::param(
	const Named2& value0, 
	const Named2& value1,
	const Named2& value2,
	const Named2& value3,
	const Named2& value4
){
	if(param_n_>0)pi_.params[0] = Named(value0.name, value0.value);
	if(param_n_>1)pi_.params[1] = Named(value1.name, value1.value);
	if(param_n_>2)pi_.params[2] = Named(value2.name, value2.value);
	if(param_n_>3)pi_.params[3] = Named(value3.name, value3.value);
	if(param_n_>4)pi_.params[4] = Named(value4.name, value4.value);

	pi_.min_param_count = pi_.max_param_count = param_n_;
	for(int_t i=0; i<param_n_; ++i){
		if(rawne(pi_.params[i].value, undefined)){
			pi_.min_param_count--;
		}
	}

	return from_this(this);
}

void CFun::visit_members(Visitor& m){
	HaveName::visit_members(m);
	std::for_each(pi_.params, pi_.params+param_n_, m);
}

void check_arg(const VMachinePtr& vm);

void CFun::call(const VMachinePtr& vm){
	if(vm->ordered_arg_count()!=pi_.min_param_count){
		check_arg(vm);
	}
	fun_(vm, pi_, val_);
}

void CFun::check_arg(const VMachinePtr& vm){
	int_t n = vm->ordered_arg_count();
	if(n<pi_.min_param_count || n>pi_.max_param_count){
		if(pi_.min_param_count==0 && pi_.max_param_count==0){
			XTAL_THROW(builtin()->member("ArgumentError")(
				Xt("Xtal Runtime Error 1007")(
					Named("name", ap(pi_.fun)->object_name()),
					Named("value", n)
				)
			), return);
		}else{
			XTAL_THROW(builtin()->member("ArgumentError")(
				Xt("Xtal Runtime Error 1006")(
					Named("name", ap(pi_.fun)->object_name()),
					Named("min", pi_.min_param_count),
					Named("max", pi_.max_param_count),
					Named("value", n)
				)
			), return);
		}
	}
}

CFunArgs::CFunArgs(fun_t f, void* val, int_t param_n)
	:CFun(f, val, param_n){}

void CFunArgs::call(const VMachinePtr& vm){
	fun_(vm, pi_, val_);
}
	
CFunEssence::CFunEssence(CFun::fun_t f, const void* data, int_t val_size, int_t param_n, bool args)
	:f(f), val(user_malloc(val_size)), param_n(param_n), args(args){
	memcpy(val, data, val_size);
}

CFunPtr new_cfun(const CFunEssence& essence){
	if(essence.args){
		return xnew<CFunArgs>(essence.f, essence.val, essence.param_n);
	}
	else{
		return xnew<CFun>(essence.f, essence.val, essence.param_n);
	}
}

DualDispatchMethodPtr dual_dispatch_method(const IDPtr& primary_key){
	return xnew<DualDispatchMethod>(primary_key);
}

DualDispatchFunPtr dual_dispatch_fun(const ClassPtr& klass, const IDPtr& primary_key){
	return xnew<DualDispatchFun>(klass, primary_key);
}

}
