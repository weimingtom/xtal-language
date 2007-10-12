#include "xtal.h"

#include "xtal_cfun.h"
#include "xtal_macro.h"
#include "xtal_vmachine.h"

namespace xtal{

Result result;
ReturnThis return_this;
ReturnVoid return_void;


CFun::CFun(fun_t f, const void* val, int_t val_size, int_t param_n){
	fun_ = f;
	
	buffer_size_ = val_size;
	data_ = user_malloc(val_size);
	memcpy(data_, val, val_size);
	
	if(param_n){
		pi_.params = (Named*)user_malloc(sizeof(Named)*param_n);
	}else{
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
	user_free(data_);
	user_free(pi_.params);		
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
		if(rawne(pi_.params[i].value, nop)){
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
	fun_(vm, pi_, data_);
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

CFunArgs::CFunArgs(fun_t f, const void* val, int_t val_size, int_t param_n)
	:CFun(f, val, val_size, param_n){}

void CFunArgs::call(const VMachinePtr& vm){
	fun_(vm, pi_, data_);
}

}
