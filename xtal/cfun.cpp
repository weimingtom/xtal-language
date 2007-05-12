
#include "cfun.h"
#include "xtal.h"
#include "xmacro.h"

namespace xtal{

void check_arg(const VMachine& vm, const ParamInfo& p){
	int_t n = vm.ordered_arg_count();
	if(n<p.min_param_count || n>p.max_param_count){
		if(p.min_param_count==0 && p.max_param_count==0){
			throw builtin().member("InvalidArgumentError")(
				Xt("Xtal Runtime Error 1007")(
					Xid(name)=p.fun.cref().object_name(),
					Xid(value)=n
				)
			);
		}else{
			throw builtin().member("InvalidArgumentError")(
				Xt("Xtal Runtime Error 1006")(
					Xid(name)=p.fun.cref().object_name(),
					Xid(min)=p.min_param_count,
					Xid(max)=p.max_param_count,
					Xid(value)=n
				)
			);
		}
	}
}

class CFunImpl : public HaveNameImpl{
public:
	typedef void (*fun_t)(const VMachine&, const ParamInfo&, void* data);

	CFunImpl(fun_t f, const void* val, int_t val_size, int_t param_n){
		fun_ = f;
		
		data_size_ = val_size;
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
	
	virtual ~CFunImpl(){
		for(int_t i=0; i<param_n_; ++i){
			pi_.params[i].~Named();
		}
		user_free(data_, data_size_);
		user_free(pi_.params, sizeof(Named)*param_n_);		
	}

	void param(
		const Named& value0, 
		const Named& value1,
		const Named& value2,
		const Named& value3,
		const Named& value4
	){
		if(param_n_>0)pi_.params[0] = value0;
		if(param_n_>1)pi_.params[1] = value1;
		if(param_n_>2)pi_.params[2] = value2;
		if(param_n_>3)pi_.params[3] = value3;
		if(param_n_>4)pi_.params[4] = value4;

		pi_.min_param_count = pi_.max_param_count = param_n_;
		for(int_t i=0; i<param_n_; ++i){
			if(pi_.params[i].value){
				pi_.min_param_count--;
			}
		}
	}

	virtual void visit_members(Visitor& m){
		HaveNameImpl::visit_members(m);
		std::for_each(pi_.params, pi_.params+param_n_, m);
	}

	virtual void call(const VMachine& vm){
		fun_(vm, pi_, data_);
	}

protected:
	
	void* data_;
	int_t data_size_;
	fun_t fun_;
	ParamInfo pi_;
	int_t param_n_;
};

CFun::CFun(CFunImpl::fun_t fun, const void* val, int_t val_size, int_t param_n)
	:Any(null){
	new(*this) CFunImpl(fun, val, val_size, param_n);
}

const CFun& CFun::param(
	const Named& value0, 
	const Named& value1,
	const Named& value2,
	const Named& value3,
	const Named& value4
) const{
	impl()->param(value0, value1, value2, value3, value4);
	return *this;
}


}
