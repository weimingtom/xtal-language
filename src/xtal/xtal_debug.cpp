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
	m & /*vm_ & */file_name_ & fun_name_ & exception_ & variables_frame_;
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
	BSTATE_STEP_OVER,
	BSTATE_STEP_INTO,
	BSTATE_STEP_OUT
};

class DebugData : public Base{
public:
	DebugData(){
		enable_count_ = 0;
		hook_setting_bit_ = 0;
		saved_hook_setting_bit_ = 0;
		breakpoint_state_ = BSTATE_GO;
	}

	uint_t enable_count_;
	uint_t hook_setting_bit_;
	uint_t saved_hook_setting_bit_;

	AnyPtr hooks_[BREAKPOINT_MAX];

	int_t breakpoint_state_;
	int_t breakpoint_call_stack_size_;
};

namespace{

void debugenable(const SmartPtr<DebugData>& d){
	bind_all();
	for(int_t i=0, size=BREAKPOINT_MAX; i<size; ++i){
		set_hook(i, d->hooks_[i]);
	}
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
	if(d->enable_count_==0){
		d->saved_hook_setting_bit_ = d->hook_setting_bit_;
	}
	d->enable_count_++;
	debugenable(d);
}

void disable(){
	const SmartPtr<DebugData>& d = cpp_var<DebugData>();
	d->enable_count_--;

	if(d->enable_count_<=0){
		d->saved_hook_setting_bit_ = d->hook_setting_bit_;
		d->hook_setting_bit_ = 0;
	}
}

bool is_enabled(){
	const SmartPtr<DebugData>& d = cpp_var<DebugData>();
	return d->enable_count_>0;
}

void enable_force(uint_t count){
	const SmartPtr<DebugData>& d = cpp_var<DebugData>();
	if(d->enable_count_==0){
		d->saved_hook_setting_bit_ = d->hook_setting_bit_;
	}
	d->enable_count_ = count;
	debugenable(d);
}

uint_t disable_force(){
	const SmartPtr<DebugData>& d = cpp_var<DebugData>();
	int_t temp = d->enable_count_;
	d->enable_count_ = 0;

	if(d->enable_count_<=0){
		d->saved_hook_setting_bit_ = d->hook_setting_bit_;
		d->hook_setting_bit_ = 0;
	}

	return temp;
}

uint_t hook_setting_bit(){
	const SmartPtr<DebugData>& d = cpp_var<DebugData>();
	return d->hook_setting_bit_;
}

uint_t* hook_setting_bit_ptr(){
	const SmartPtr<DebugData>& d = cpp_var<DebugData>();
	return &d->hook_setting_bit_;
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

void call_debug_hook(int_t kind, const HookInfoPtr& ainfo){
	const SmartPtr<DebugData>& d = cpp_var<DebugData>();
	HookInfoPtr info = ainfo;
	
	AnyPtr hook = d->hooks_[kind];

	while(true){
		switch(d->breakpoint_state_){
			XTAL_NODEFAULT;

			XTAL_CASE(BSTATE_NONE){
				AnyPtr hookret = hook ? hook->call(info) : AnyPtr(0);
				int_t ret = type(hookret)==TYPE_INT ? hookret->to_i() : 0;

				switch(ret){
					XTAL_CASE(RUN){
						d->breakpoint_state_ = BSTATE_NONE;
						d->hooks_[BREAKPOINT_LINE] = null;
					}

					XTAL_CASE(STEP_OVER){
						d->breakpoint_state_ = BSTATE_STEP_OVER;
						d->hooks_[BREAKPOINT_LINE] = hook;
					}

					XTAL_CASE(STEP_INTO){
						d->breakpoint_state_ = BSTATE_STEP_INTO;
						d->hooks_[BREAKPOINT_LINE] = hook;
					}

					XTAL_CASE(STEP_OUT){
						d->breakpoint_state_ = BSTATE_STEP_OUT;
						d->hooks_[BREAKPOINT_LINE] = hook;
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

			XTAL_CASE(BSTATE_STEP_OVER){
				if(info->call_stack_size() <= d->breakpoint_call_stack_size_){
					d->breakpoint_state_ = BSTATE_NONE;
					continue;
				}
			}

			XTAL_CASE(BSTATE_STEP_INTO){
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

MapPtr make_debug_object(const AnyPtr& v, int_t depth){
	MapPtr ret = xnew<Map>();
	ret->set_at(Xid(class), v->get_class()->to_s());
	ret->set_at(Xid(value), v->to_s());

	// 基本型かチェック
	switch(type(v)){
		case TYPE_NULL:
		case TYPE_UNDEFINED:
		case TYPE_INT:
		case TYPE_FLOAT:
		case TYPE_FALSE:
		case TYPE_TRUE:
		case TYPE_SMALL_STRING:
		case TYPE_STRING_LITERAL:
		case TYPE_ID_LITERAL:
		case TYPE_STRING:
			return ret;
	}

	if(depth<=0){
		ret->set_at(Xid(children), XTAL_STRING("..."));
		return ret;
	}

	switch(type(v)){
		XTAL_DEFAULT{}

		XTAL_CASE(TYPE_ARRAY){
			ArrayPtr children = xnew<Array>();
			Xfor(it, v){
				children->push_back(make_debug_object(it, depth-1));
			}
			ret->set_at(Xid(children), children);
			return ret;
		}

		XTAL_CASE(TYPE_VALUES){
			ArrayPtr children = xnew<Array>();
			Xfor(it, v){
				children->push_back(make_debug_object(it, depth-1));
			}
			ret->set_at(Xid(children), children);
			return ret;
		}
	}

	if(const MapPtr& a = ptr_cast<Map>(v)){
		MapPtr children = xnew<Map>();
		Xfor2(key, val, a){
			children->set_at(key->to_s(), make_debug_object(val, depth-1));
		}
		ret->set_at(Xid(children), children);
		return ret;
	}

	if(const ClassPtr& a = ptr_cast<Class>(v)){
		MapPtr children = xnew<Map>();
		Xfor3(key, skey, val, a->members()){
			XTAL_UNUSED_VAR(skey);
			children->set_at(key->to_s(), make_debug_object(val, depth-1));
		}
		ret->set_at(Xid(children), children);
		return ret;
	}

	AnyPtr data = v->s_save();
	if(const MapPtr& a = ptr_cast<Map>(data)){
		MapPtr children = xnew<Map>();
		Xfor2(key, val, a){
			XTAL_UNUSED_VAR(key);
			Xfor2(key2, val2, val){
				children->set_at(key2->to_s(), make_debug_object(val2, depth-1));
			}
		}
		ret->set_at(Xid(children), children);
		return ret;
	}

	return ret;
}

}

}
