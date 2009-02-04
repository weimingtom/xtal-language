#include "xtal.h"
#include "xtal_macro.h"

namespace xtal{

Arguments::Arguments(const AnyPtr& ordered, const AnyPtr& named){
	if(ordered){ ordered_ = ptr_cast<Array>(ordered); }
	else{ ordered_ = xnew<Array>(); }
	if(named){ named_ = ptr_cast<Map>(named); }
	else{ named_ = xnew<Map>(); }
}

void Arguments::visit_members(Visitor& m){
	Base::visit_members(m);
	m & ordered_ & named_;
}

InstanceVariableGetter::InstanceVariableGetter(int_t number, ClassInfo* info)
	:number_(number), info_(info){
}

void InstanceVariableGetter::rawcall(const VMachinePtr& vm){
	const AnyPtr& self = vm->get_arg_this();
	InstanceVariables* p;
	if(type(self)==TYPE_BASE){
		p = pvalue(self)->instance_variables();
	}
	else{
		p = &empty_instance_variables;
	}
	vm->return_result(p->variable(number_, info_));
}

InstanceVariableSetter::InstanceVariableSetter(int_t number, ClassInfo* info)
	:number_(number), info_(info){
}

void InstanceVariableSetter::rawcall(const VMachinePtr& vm){
	const AnyPtr& self = vm->get_arg_this();
	InstanceVariables* p;
	if(type(self)==TYPE_BASE){
		p = pvalue(self)->instance_variables();
	}
	else{
		p = &empty_instance_variables;
	}
	p->set_variable(number_, info_, vm->arg(0));
	vm->return_result();
}


Fun::Fun(const FramePtr& outer, const AnyPtr& athis, const CodePtr& code, FunInfo* core)
	:outer_(outer), this_(athis), code_(code), info_(core){
}

StringPtr Fun::object_name(int_t depth){
	if(!name_){
		set_object_name(ptr_cast<String>(Xf("<(%s):%s:%d>")->call(get_class()->object_name(depth), code_->source_file_name(), code_->compliant_lineno(code_->data()+info_->pc))), 1, parent_);
	}

	return HaveName::object_name(depth);
}

void Fun::check_arg(const VMachinePtr& vm){
	int_t n = vm->ordered_arg_count();
	if(n<info_->min_param_count || (!(info_->flags&FunInfo::FLAG_EXTENDABLE_PARAM) && n>info_->max_param_count)){
		if(info_->min_param_count==0 && info_->max_param_count==0){
			XTAL_THROW(builtin()->member(Xid(ArgumentError))->call(
				Xt("Xtal Runtime Error 1007")->call(
					Named(Xid(object), object_name()),
					Named(Xid(value), n)
				)
			), return);
		}
		else{
			if(info_->flags&FunInfo::FLAG_EXTENDABLE_PARAM){
				XTAL_THROW(builtin()->member(Xid(ArgumentError))->call(
					Xt("Xtal Runtime Error 1005")->call(
						Named(Xid(object), object_name()),
						Named(Xid(min), info_->min_param_count),
						Named(Xid(value), n)
					)
				), return);
			}
			else{
				XTAL_THROW(builtin()->member(Xid(ArgumentError))->call(
					Xt("Xtal Runtime Error 1006")->call(
						Named(Xid(object), object_name()),
						Named(Xid(min), info_->min_param_count),
						Named(Xid(max), info_->max_param_count),
						Named(Xid(value), n)
					)
				), return);
			}
		}
	}
}

void Fun::rawcall(const VMachinePtr& vm){
	if(vm->ordered_arg_count()!=info_->max_param_count)
		check_arg(vm);
	vm->set_arg_this(this_);
	vm->carry_over(this);
}

void Lambda::rawcall(const VMachinePtr& vm){
	vm->set_arg_this(this_);
	vm->mv_carry_over(this);
}


void Method::rawcall(const VMachinePtr& vm){
	if(vm->ordered_arg_count()!=info_->max_param_count)
		check_arg(vm);
	vm->carry_over(this);
}


Fiber::Fiber(const FramePtr& outer, const AnyPtr& th, const CodePtr& code, FunInfo* core)
	:Fun(outer, th, code, core), vm_(null), resume_pc_(0), alive_(true){
	set_finalizer_flag();
}

void Fiber::finalize(){
	halt();
}

void Fiber::halt(){
	if(resume_pc_!=0){
		vm_->exit_fiber();
		resume_pc_ = 0;
		core()->vm_take_back(vm_);
		vm_ = null;
		alive_ = false;
	}
}

void Fiber::call_helper(const VMachinePtr& vm, bool add_succ_or_fail_result){
	if(alive_){
		vm->set_arg_this(this_);
		if(resume_pc_==0){
			if(!vm_){ vm_ = core()->vm_take_over(); }
			resume_pc_ = vm_->start_fiber(this, vm.get(), add_succ_or_fail_result);
		}
		else{ 
			resume_pc_ = vm_->resume_fiber(this, resume_pc_, vm.get(), add_succ_or_fail_result);
		}
		if(resume_pc_==0){
			core()->vm_take_back(vm_);
			vm_ = null;
			alive_ = false;
		}
	}
	else{
		vm->return_result();
	}
}

AnyPtr Fiber::reset(){
	halt();
	alive_ = true;
	return from_this(this);
}


void initialize_fun(){
	{
		ClassPtr p = new_cpp_class<Fun>(Xid(Fun));
	}

	{
		ClassPtr p = new_cpp_class<Method>(Xid(Method));
		p->inherit(get_cpp_class<Fun>());
	}

	{
		ClassPtr p = new_cpp_class<Fiber>(Xid(Fiber));
		p->inherit(get_cpp_class<Fun>());
		p->inherit(Iterator());
		p->def_method(Xid(reset), &Fiber::reset);
		p->def_method(Xid(block_first), &Fiber::block_next);
		p->def_method(Xid(block_next), &Fiber::block_next);
		p->def_method(Xid(halt), &Fiber::halt);
		p->def_method(Xid(is_alive), &Fiber::is_alive);
	}

	{
		ClassPtr p = new_cpp_class<Lambda>(Xid(Lambda));
		p->inherit(get_cpp_class<Fun>());
	}

	{
		ClassPtr p = new_cpp_class<Arguments>(Xid(Arguments));
		p->def(Xid(new), ctor<Arguments, const AnyPtr&, const AnyPtr&>()->params(Xid(ordered), null, Xid(named), null));
		p->def_method(Xid(size), &Arguments::length);
		p->def_method(Xid(length), &Arguments::length);
		p->def_method(Xid(ordered_arguments), &Arguments::ordered_arguments);
		p->def_method(Xid(named_arguments), &Arguments::named_arguments);
		
		p->def(Xid(op_at), method(&Arguments::op_at_int), get_cpp_class<Int>());
		p->def(Xid(op_at), method(&Arguments::op_at_string), get_cpp_class<String>());
	}

	builtin()->def(Xid(Arguments), get_cpp_class<Arguments>());
	builtin()->def(Xid(Fun), get_cpp_class<Fun>());
	builtin()->def(Xid(Fiber), get_cpp_class<Fiber>());
}

void initialize_fun_script(){
	Xemb((

Arguments::each: method{
	return fiber{ 
		this.ordered_arguments.with_index{ |i, v|
			yield i, v;
		}
		this.named_arguments{ |i, v|
			yield i, v;
		}
	}
}

Arguments::pairs: Arguments::each;

Fun::call: method(...){
	return this(...);
}
	),
"\x78\x74\x61\x6c\x01\x00\x00\x00\x00\x00\x00\x4b\x39\x00\x01\x89\x00\x01\x00\x0f\x0b\x2f\x00\x00\x00\x00\x00\x02\x0b\x25\x01\x25\x00\x37\x00\x03\x39\x00\x01\x89\x00\x02\x00\x0f\x0b\x2f\x00\x00\x00\x00\x00\x04\x01\x25\x01\x25\x00\x37\x00\x05\x39\x00\x01\x89"
"\x00\x03\x00\x0f\x0b\x2f\x00\x00\x00\x00\x00\x04\x01\x25\x01\x25\x00\x37\x00\x06\x25\x00\x8b\x00\x03\x08\x00\x00\x00\x00\x00\x02\x00\x00\x00\x12\x00\x20\x00\x00\x00\x00\x00\x04\x00\x00\x00\x12\x00\x38\x00\x00\x00\x00\x00\x06\x00\x00\x00\x12\x00\x00\x00\x00"
"\x04\x00\x00\x00\x00\x03\x06\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x08\x00\x00\x00\x05\x00\x02\x00\x00\x00\x00\x00\x00\x01\x00\x00\x20\x00\x00\x00\x05\x00\x04\x00\x00\x00\x00\x00\x00\x01\x00\x00\x38\x00\x00\x00\x05\x00\x06\x00\x00\x00\x00\x00\x00\x01\x00"
"\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x14\x00\x00\x00\x00\x11\x00\x00\x00\x00\x00\x00\x00\x06\x00\x00\x00\x00\x00\x00\x00\x03\x00\x00\x00\x03\x00\x00\x00\x06\x00\x00\x00\x08\x00\x00\x00\x04\x00\x00\x00\x10\x00\x00\x00\x05\x00\x00"
"\x00\x13\x00\x00\x00\x06\x00\x00\x00\x18\x00\x00\x00\x0b\x00\x00\x00\x18\x00\x00\x00\x08\x00\x00\x00\x1b\x00\x00\x00\x0b\x00\x00\x00\x20\x00\x00\x00\x09\x00\x00\x00\x28\x00\x00\x00\x0a\x00\x00\x00\x2b\x00\x00\x00\x0b\x00\x00\x00\x30\x00\x00\x00\x10\x00\x00"
"\x00\x30\x00\x00\x00\x0d\x00\x00\x00\x33\x00\x00\x00\x10\x00\x00\x00\x38\x00\x00\x00\x0e\x00\x00\x00\x40\x00\x00\x00\x0f\x00\x00\x00\x43\x00\x00\x00\x10\x00\x00\x00\x48\x00\x00\x00\x11\x00\x00\x00\x00\x01\x0b\x00\x00\x00\x03\x09\x00\x00\x00\x06\x73\x6f\x75"
"\x72\x63\x65\x09\x00\x00\x00\x11\x74\x6f\x6f\x6c\x2f\x74\x65\x6d\x70\x2f\x69\x6e\x2e\x78\x74\x61\x6c\x09\x00\x00\x00\x0b\x69\x64\x65\x6e\x74\x69\x66\x69\x65\x72\x73\x0a\x00\x00\x00\x07\x09\x00\x00\x00\x00\x09\x00\x00\x00\x05\x4d\x75\x74\x65\x78\x09\x00\x00"
"\x00\x04\x6c\x6f\x63\x6b\x09\x00\x00\x00\x0b\x62\x6c\x6f\x63\x6b\x5f\x66\x69\x72\x73\x74\x09\x00\x00\x00\x06\x75\x6e\x6c\x6f\x63\x6b\x09\x00\x00\x00\x0a\x62\x6c\x6f\x63\x6b\x5f\x6e\x65\x78\x74\x09\x00\x00\x00\x0b\x62\x6c\x6f\x63\x6b\x5f\x62\x72\x65\x61\x6b"
"\x09\x00\x00\x00\x06\x76\x61\x6c\x75\x65\x73\x0a\x00\x00\x00\x01\x03"
)->call();
}

}
