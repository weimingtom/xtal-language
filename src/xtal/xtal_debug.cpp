#include "xtal.h"
#include "xtal_macro.h"

namespace xtal{ namespace debug{

StringPtr CallerInfo::file_name(){ 
	return fun_ ? fun_->code()->source_file_name() : StringPtr(XTAL_STRING("C++ Function")); 
}

StringPtr CallerInfo::fun_name(){ 
	return fun_ ? fun_->object_name() : empty_string; 
}

void CallerInfo::on_visit_members(Visitor& m){
	Base::on_visit_members(m);
	m & fun_ & variables_frame_;
}

void HookInfo::on_visit_members(Visitor& m){
	Base::on_visit_members(m);
	m & /*vm_ & */ code_ & file_name_ & fun_name_ & exception_ & variables_frame_;
}

SmartPtr<HookInfo> HookInfo::clone(){
	SmartPtr<HookInfo> ret = xnew<HookInfo>();
	ret->kind_ = kind_;
	ret->line_ = line_;
	ret->file_name_ = file_name_;
	ret->fun_name_ = fun_name_;
	ret->exception_ = exception_;
	ret->variables_frame_ = variables_frame_;
	ret->vm_ = vm_;
	return ret;
}
		
void HookInfo::set_vm(const VMachinePtr& v){ 
	vm_ = &*v; 
}
	
const VMachinePtr& HookInfo::vm(){ 
	return to_smartptr(vm_); 
}

CallerInfoPtr HookInfo::caller(uint_t n){
	return vm_->caller(n);
}

int_t HookInfo::call_stack_size(){
	return vm_->call_stack_size();
}


enum{
	BSTATE_NONE,
	BSTATE_GO,
	BSTATE_STEP_OVER,
	BSTATE_STEP_INTO,
	BSTATE_STEP_OUT,
	BSTATE_STEP_OUT2
};

class DebugData : public Base{
public:
	DebugData(){
		enable_count_ = 0;
		hook_setting_bit_ = 0;
		saved_hook_setting_bit_ = 0;
		breakpoint_state_ = BSTATE_GO;
		debug_compile_count_ = 0;
	}

	uint_t enable_count_;
	uint_t hook_setting_bit_;
	uint_t saved_hook_setting_bit_;

	AnyPtr hooks_[BREAKPOINT_MAX];

	int_t breakpoint_state_;
	int_t breakpoint_call_stack_size_;

	uint_t debug_compile_count_;
};

namespace{

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
	const SmartPtr<DebugData>& d = cpp_value<DebugData>();
	if(d->enable_count_==0){
		d->saved_hook_setting_bit_ = d->hook_setting_bit_;
	}
	d->enable_count_++;
}

void disable(){
	const SmartPtr<DebugData>& d = cpp_value<DebugData>();
	d->enable_count_--;

	if(d->enable_count_<=0){
		d->saved_hook_setting_bit_ = d->hook_setting_bit_;
		d->hook_setting_bit_ = 0;
	}
}

bool is_enabled(){
	const SmartPtr<DebugData>& d = cpp_value<DebugData>();
	return d->enable_count_>0;
}

void enable_force(uint_t count){
	const SmartPtr<DebugData>& d = cpp_value<DebugData>();
	if(d->enable_count_==0){
		d->hook_setting_bit_ = d->saved_hook_setting_bit_;
	}
	d->enable_count_ = count;
}

uint_t disable_force(){
	const SmartPtr<DebugData>& d = cpp_value<DebugData>();
	int_t temp = d->enable_count_;
	d->enable_count_ = 0;

	if(d->enable_count_<=0){
		d->saved_hook_setting_bit_ = d->hook_setting_bit_;
		d->hook_setting_bit_ = 0;
	}

	return temp;
}

bool is_debug_compile_enabled(){
	const SmartPtr<DebugData>& d = cpp_value<DebugData>();
	return d->debug_compile_count_>0;
}

void enable_debug_compile(){
	const SmartPtr<DebugData>& d = cpp_value<DebugData>();
	d->debug_compile_count_++;
}

void disable_debug_compile(){
	const SmartPtr<DebugData>& d = cpp_value<DebugData>();
	d->debug_compile_count_--;
}

uint_t hook_setting_bit(){
	const SmartPtr<DebugData>& d = cpp_value<DebugData>();
	return d->hook_setting_bit_;
}

uint_t* hook_setting_bit_ptr(){
	const SmartPtr<DebugData>& d = cpp_value<DebugData>();
	return &d->hook_setting_bit_;
}

void set_hook(int_t hooktype, const AnyPtr& hook){
	const SmartPtr<DebugData>& d = cpp_value<DebugData>();
	d->hooks_[hooktype] = hook;
	bitchange(d, hook, hooktype);
}

const AnyPtr& hook(int_t hooktype){
	const SmartPtr<DebugData>& d = cpp_value<DebugData>();
	return d->hooks_[hooktype];
}

void set_breakpoint_hook(const AnyPtr& hook){
	set_hook(BREAKPOINT, hook);
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

const AnyPtr& breakpoint_hook(){
	return hook(BREAKPOINT);
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

void call_breakpoint_hook(int_t kind, const HookInfoPtr& ainfo){
	const SmartPtr<DebugData>& d = cpp_value<DebugData>();
	HookInfoPtr info = ainfo;

	if(kind>=BREAKPOINT_LINE){
		int_t count = disable_force();
		d->hooks_[kind]->call(info);
		enable_force(count);
		return;
	}

	AnyPtr hook = d->hooks_[0];
	while(true){
		switch(d->breakpoint_state_){
			XTAL_NODEFAULT;

			XTAL_CASE(BSTATE_NONE){
				int_t count = disable_force();
				AnyPtr hookret = hook ? hook->call(info) : AnyPtr(0);
				int_t ret = type(hookret)==TYPE_INT ? hookret->to_i() : -1;
				enable_force(count);

				switch(ret){
					XTAL_DEFAULT{
						// 2 line
						// 3 return
						// 4 call
					}

					XTAL_CASE(RUN){
						d->breakpoint_state_ = BSTATE_NONE;
						bitchange(d, false, BREAKPOINT2);
						bitchange(d, false, BREAKPOINT3);
						bitchange(d, false, BREAKPOINT4);
					}

					XTAL_CASE(STEP_OVER){
						d->breakpoint_state_ = BSTATE_STEP_OVER;
						bitchange(d, true, BREAKPOINT2);
						bitchange(d, true, BREAKPOINT3);
						bitchange(d, true, BREAKPOINT4);
					}

					XTAL_CASE(STEP_INTO){
						d->breakpoint_state_ = BSTATE_STEP_INTO;
						bitchange(d, true, BREAKPOINT2);
						bitchange(d, true, BREAKPOINT3);
						bitchange(d, false, BREAKPOINT4);
					}

					XTAL_CASE(STEP_OUT){
						d->breakpoint_state_ = BSTATE_STEP_OUT;
						bitchange(d, true, BREAKPOINT2);
						bitchange(d, true, BREAKPOINT3);
						bitchange(d, false, BREAKPOINT4);
					}

					XTAL_CASE(REDO){
						return;
					}
				}

				d->breakpoint_call_stack_size_ = info->call_stack_size();
			}

			XTAL_CASE(BSTATE_GO){
				d->breakpoint_state_ = BSTATE_NONE;
				continue;
			}

			XTAL_CASE(BSTATE_STEP_OVER){
				if(kind==BREAKPOINT2 || kind==BREAKPOINT3 || kind==BREAKPOINT){
					d->breakpoint_state_ = BSTATE_NONE;
					continue;
				}

				if(kind==BREAKPOINT4){
					d->breakpoint_state_ = BSTATE_STEP_OUT2;
					bitchange(d, true, BREAKPOINT2);
					bitchange(d, true, BREAKPOINT3);
					bitchange(d, false, BREAKPOINT4);
					break;
				}
			}

			XTAL_CASE(BSTATE_STEP_INTO){
				d->breakpoint_state_ = BSTATE_NONE;
				continue;
			}

			XTAL_CASE(BSTATE_STEP_OUT){
				if(kind==BREAKPOINT || info->call_stack_size() < d->breakpoint_call_stack_size_){
					d->breakpoint_state_ = BSTATE_NONE;
					continue;
				}
			}

			XTAL_CASE(BSTATE_STEP_OUT2){
				if(kind==BREAKPOINT3 && info->call_stack_size() == d->breakpoint_call_stack_size_){
					break;
				}

				if(kind==BREAKPOINT || info->call_stack_size() <= d->breakpoint_call_stack_size_){
					d->breakpoint_state_ = BSTATE_NONE;
					continue;
				}
			}
		}

		break;
	}
}

}

}
