#include "xtal.h"
#include "xtal_macro.h"

namespace xtal{ namespace debug{

void CallerInfo::visit_members(Visitor& m){
	Base::visit_members(m);
	m & fun_ & variables_frame_;
}

void HookInfo::visit_members(Visitor& m){
	Base::visit_members(m);
	m & /*vm_ & */file_name_ & fun_name_ & assertion_message_ & exception_ & variables_frame_;
}

CallerInfoPtr HookInfo::caller(uint_t n){
	return ptr_cast<VMachine>(ap(vm_))->caller(n);
}

int_t HookInfo::call_stack_size(){
	return ptr_cast<VMachine>(ap(vm_))->call_stack_size();
}


enum{
	BSTATE_NONE,
	BSTATE_GO,
	BSTATE_STEP,
	BSTATE_STEP_IN,
	BSTATE_STEP_OUT
};

class DebugData{
public:
	DebugData(){
		enable_count_ = 0;
		hook_setting_bit_ = 0;
		breakpoint_state_ = BSTATE_GO;
	}

	int_t enable_count_;
	uint_t hook_setting_bit_;

	AnyPtr hooks_[BREAKPOINT_MAX];

	AnyPtr breakpoint_hook_;
	int_t breakpoint_state_;
	int_t breakpoint_call_stack_size_;
};

namespace{

void debugenable(const SmartPtr<DebugData>& d){
	bind_all();
	for(int_t i=0, size=BREAKPOINT_MAX; i<size; ++i){
		set_hook(i, d->hooks_[i]);
	}
	vmachine()->set_hook_setting_bit(d->hook_setting_bit_);
}

void bitchange(const SmartPtr<DebugData>& d, bool b, int_t type){
	if(b){
		d->hook_setting_bit_ |= 1<<type;
	}
	else{
		d->hook_setting_bit_ &= ~(1<<type);
	}
}

}

void enable(){
	const SmartPtr<DebugData>& d = cpp_var<DebugData>();
	d->enable_count_++;
	debugenable(d);
}

void disable(){
	const SmartPtr<DebugData>& d = cpp_var<DebugData>();
	d->enable_count_--;

	if(d->enable_count_<=0){
		d->hook_setting_bit_ = 0;
		vmachine()->set_hook_setting_bit(0);
	}
}

bool is_enabled(){
	const SmartPtr<DebugData>& d = cpp_var<DebugData>();
	return d->enable_count_>0;
}

void enable_force(int_t count){
	const SmartPtr<DebugData>& d = cpp_var<DebugData>();
	d->enable_count_ = count;
	debugenable(d);
}

int_t disable_force(){
	const SmartPtr<DebugData>& d = cpp_var<DebugData>();
	int_t temp = d->enable_count_;
	d->enable_count_ = 0;

	if(d->enable_count_<=0){
		d->hook_setting_bit_ = 0;
		vmachine()->set_hook_setting_bit(0);
	}

	return temp;
}

uint_t hook_setting_bit(){
	const SmartPtr<DebugData>& d = cpp_var<DebugData>();
	return d->hook_setting_bit_;
}

void set_hook(int_t hooktype, const AnyPtr& hook){
	const SmartPtr<DebugData>& d = cpp_var<DebugData>();
	d->hooks_[hooktype] = hook;
	bitchange(d, hook, hooktype);
}

const AnyPtr& hook(int_t hooktype){
	const SmartPtr<DebugData>& d = cpp_var<DebugData>();
	return d->hooks_[hooktype];
}

void set_line_hook(const AnyPtr& hook){
	set_hook(BREAKPOINT_LINE, hook);
}

void set_call_hook(const AnyPtr& hook){
	set_hook(BREAKPOINT_CALL, hook);
}

void set_return_hook(const AnyPtr& hook){
	set_hook(BREAKPOINT_RETURN, hook);
}

void set_throw_hook(const AnyPtr& hook){
	set_hook(BREAKPOINT_THROW, hook);
}

void set_assert_hook(const AnyPtr& hook){
	set_hook(BREAKPOINT_ASSERT, hook);
}

const AnyPtr& line_hook(){
	return hook(BREAKPOINT_LINE);
}

const AnyPtr& call_hook(){
	return hook(BREAKPOINT_CALL);
}

const AnyPtr& return_hook(){
	return hook(BREAKPOINT_RETURN);
}

const AnyPtr& throw_hook(){
	return hook(BREAKPOINT_THROW);
}

const AnyPtr& assert_hook(){
	return hook(BREAKPOINT_ASSERT);
}

void breakpoint_hook_helper(const HookInfoPtr& ainfo){
	const SmartPtr<DebugData>& d = cpp_var<DebugData>();
	HookInfoPtr info = ainfo;

	while(true){
		switch(d->breakpoint_state_){
			XTAL_NODEFAULT;

			XTAL_CASE(BSTATE_NONE){
				int_t ret = d->breakpoint_hook_->call(info)->to_i();
				switch(ret){
					XTAL_CASE(GO){
						d->breakpoint_state_ = BSTATE_NONE;
					}

					XTAL_CASE(STEP){
						d->breakpoint_state_ = BSTATE_STEP;
					}

					XTAL_CASE(STEP_IN){
						d->breakpoint_state_ = BSTATE_STEP_IN;
					}

					XTAL_CASE(STEP_OUT){
						d->breakpoint_state_ = BSTATE_STEP_OUT;
					}
				}

				d->breakpoint_call_stack_size_ = info->call_stack_size();
			}

			XTAL_CASE(BSTATE_GO){
				if(true){
					d->breakpoint_state_ = BSTATE_NONE;
					continue;
				}
			}

			XTAL_CASE(BSTATE_STEP){
				if(info->call_stack_size() <= d->breakpoint_call_stack_size_){
					d->breakpoint_state_ = BSTATE_NONE;
					continue;
				}
			}

			XTAL_CASE(BSTATE_STEP_IN){
				d->breakpoint_state_ = BSTATE_NONE;
				continue;
			}

			XTAL_CASE(BSTATE_STEP_OUT){
				if(info->call_stack_size() < d->breakpoint_call_stack_size_){
					d->breakpoint_state_ = BSTATE_NONE;
					continue;
				}
			}
		}

		break;
	}
}

void set_breakpoint_hook(const AnyPtr& f){
	const SmartPtr<DebugData>& d = cpp_var<DebugData>();
	d->breakpoint_hook_ = f;
	set_line_hook(fun(&breakpoint_hook_helper));
}

}

}
