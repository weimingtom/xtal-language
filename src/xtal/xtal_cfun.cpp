#include "xtal.h"
#include "xtal_macro.h"

namespace xtal{

static Named null_params[16];
static char dummy_address;

CFunPtr new_cfun(void (*fun)(ParamInfoAndVM& pvm), const void* val, int_t val_size, int_t param_n){
	return xnew<CFun>(fun, val, val_size, param_n);
}

CFun::CFun(fun_t f, const void* val, int_t val_size, int_t param_n){
	fun_ = f;
	if(val_size==0){
		val_ = &dummy_address;
	}
	else{
		val_ = user_malloc(val_size);
		std::memcpy(val_, val, val_size);
	}
	param_n_ = pi_.min_param_count = pi_.max_param_count = param_n;
	pi_.params = null_params;
}


CFun::~CFun(){
	for(int_t i=0; i<param_n_; ++i){
		pi_.params[i].~Named();
	}

	if(pi_.params!=null_params){
		user_free(pi_.params);		
	}

	if(val_!=&dummy_address){
		user_free(val_);
	}
}

const CFunPtr& CFun::param(
		const Named2& value0, 
		const Named2& value1,
		const Named2& value2,
		const Named2& value3,
		const Named2& value4,
		const Named2& value5,
		const Named2& value6,
		const Named2& value7,
		const Named2& value8,
		const Named2& value9,
		const Named2& value10
	){

	if(pi_.params==null_params && param_n_>0){
		pi_.params = (Named*)user_malloc(sizeof(Named)*param_n_);

		for(int_t i=0; i<param_n_; ++i){
			new(&pi_.params[i]) Named();
		}
	}

	if(param_n_>0)pi_.params[0] = Named(value0.name, value0.value);
	if(param_n_>1)pi_.params[1] = Named(value1.name, value1.value);
	if(param_n_>2)pi_.params[2] = Named(value2.name, value2.value);
	if(param_n_>3)pi_.params[3] = Named(value3.name, value3.value);
	if(param_n_>4)pi_.params[4] = Named(value4.name, value4.value);
	if(param_n_>5)pi_.params[5] = Named(value5.name, value5.value);
	if(param_n_>6)pi_.params[6] = Named(value6.name, value6.value);
	if(param_n_>7)pi_.params[7] = Named(value7.name, value7.value);
	if(param_n_>8)pi_.params[8] = Named(value8.name, value8.value);
	if(param_n_>9)pi_.params[9] = Named(value9.name, value9.value);
	if(param_n_>10)pi_.params[10] = Named(value10.name, value10.value);

	pi_.min_param_count = pi_.max_param_count = param_n_;
	for(int_t i=0; i<param_n_; ++i){
		if(rawne(pi_.params[i].value, undefined)){
			pi_.min_param_count--;
		}
	}

	return from_this(this);
}

//{REPEAT{{
/*
const CFunPtr& CFun::params(#REPEAT_COMMA#const IDPtr& key`i`, const Any& value`i`#){
	return param(#REPEAT_COMMA#Named(key`i`, ap(value`i`))#);
}
*/

const CFunPtr& CFun::params(){
	return param();
}

const CFunPtr& CFun::params(const IDPtr& key0, const Any& value0){
	return param(Named(key0, ap(value0)));
}

const CFunPtr& CFun::params(const IDPtr& key0, const Any& value0, const IDPtr& key1, const Any& value1){
	return param(Named(key0, ap(value0)), Named(key1, ap(value1)));
}

const CFunPtr& CFun::params(const IDPtr& key0, const Any& value0, const IDPtr& key1, const Any& value1, const IDPtr& key2, const Any& value2){
	return param(Named(key0, ap(value0)), Named(key1, ap(value1)), Named(key2, ap(value2)));
}

const CFunPtr& CFun::params(const IDPtr& key0, const Any& value0, const IDPtr& key1, const Any& value1, const IDPtr& key2, const Any& value2, const IDPtr& key3, const Any& value3){
	return param(Named(key0, ap(value0)), Named(key1, ap(value1)), Named(key2, ap(value2)), Named(key3, ap(value3)));
}

const CFunPtr& CFun::params(const IDPtr& key0, const Any& value0, const IDPtr& key1, const Any& value1, const IDPtr& key2, const Any& value2, const IDPtr& key3, const Any& value3, const IDPtr& key4, const Any& value4){
	return param(Named(key0, ap(value0)), Named(key1, ap(value1)), Named(key2, ap(value2)), Named(key3, ap(value3)), Named(key4, ap(value4)));
}

//}}REPEAT}

void CFun::visit_members(Visitor& m){
	HaveName::visit_members(m);
	if(param_n_>0){
		std::for_each(pi_.params, pi_.params+param_n_, m);
	}
}

void CFun::check_arg(const VMachinePtr& vm){
	int_t n = vm->ordered_arg_count();
	if(n<pi_.min_param_count || n>pi_.max_param_count){
		if(pi_.min_param_count==0 && pi_.max_param_count==0){
			XTAL_THROW(builtin()->member(Xid(ArgumentError))->call(
				Xt("Xtal Runtime Error 1007")->call(
					Named(Xid(object), vm->ff().hint()->object_name()),
					Named(Xid(value), n)
				)
			), return);
		}
		else{
			XTAL_THROW(builtin()->member(Xid(ArgumentError))->call(
				Xt("Xtal Runtime Error 1006")->call(
					Named(Xid(object), vm->ff().hint()->object_name()),
					Named(Xid(min), pi_.min_param_count),
					Named(Xid(max), pi_.max_param_count),
					Named(Xid(value), n)
				)
			), return);
		}
	}
}

void CFun::check_args(ParamInfoAndVM& pvm){
	for(int_t i=-1; pvm.flags!=0; ++i){
		if(pvm.flags&1){
			if(i==-1){
				XTAL_THROW(argument_error(pvm.vm->ff().hint()->object_name(), -1), return);
			}
			else{
				XTAL_THROW(argument_error(pvm.vm->ff().hint()->object_name(), i), return);
			}
		}

		pvm.flags >>= 1;
	}
}


void CFun::rawcall(const VMachinePtr& vm){
	if(param_n_>=0 && vm->ordered_arg_count()!=pi_.min_param_count){
		check_arg(vm);
	}

	ParamInfoAndVM pvm(pi_, vm, val_);
	fun_(pvm);
	if(pvm.flags){
		check_args(pvm);
	}
}

DualDispatchMethodPtr dual_dispatch_method(const IDPtr& primary_key){
	return xnew<DualDispatchMethod>(primary_key);
}

DualDispatchFunPtr dual_dispatch_fun(const ClassPtr& klass, const IDPtr& primary_key){
	return xnew<DualDispatchFun>(klass, primary_key);
}

}
