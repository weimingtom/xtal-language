
#pragma once

#include "xtal.h"

#include "xtal_frameimpl.h"
#include "xtal_utilimpl.h"

namespace xtal{

class InstanceVariableGetterImpl : public HaveNameImpl{
public:

	InstanceVariableGetterImpl(int_t number, FrameCore* core);

	virtual void call(const VMachine& vm);

private:
	int_t number_;
	FrameCore* core_;
};

class InstanceVariableSetterImpl : public HaveNameImpl{
public:

	InstanceVariableSetterImpl(int_t number, FrameCore* core);

	virtual void call(const VMachine& vm);

private:
	int_t number_;
	FrameCore* core_;
};

class FunImpl : public HaveNameImpl{
public:

	FunImpl(const Frame& outer, const Any& athis, const Code& code, FunCore* core);

	const Frame& outer() const{ return outer_; }
	const Code& code() const{ return code_; }
	int_t pc() const{ return core_->pc; }
	const inst_t* source() const{ return code_.data()+core_->pc; }
	const ID& param_name_at(size_t i) const{ return code_.symbol(i+core_->variable_symbol_offset); }
	int_t param_size() const{ return core_->variable_size; }	
	bool used_args_object() const{ return core_->used_args_object!=0; }
	int_t defined_file_line_number(){ return core_->line_number; }
	FunCore* core() const{ return core_; }
	void set_core(FunCore* fc){ core_ = fc; }
	void check_arg(const VMachine& vm);

public:
		
	virtual void call(const VMachine& vm);
	
	virtual int_t arity();

protected:

	Frame outer_;
	Any this_;
	Code code_;
	FunCore* core_;
	
	virtual void visit_members(Visitor& m){
		HaveNameImpl::visit_members(m);
		m & outer_ & this_ & code_;
	}

};

class LambdaImpl : public FunImpl{
public:

	LambdaImpl(const Frame& outer, const Any& th, const Code& code, FunCore* core)
		:FunImpl(outer, th, code, core){
	}

public:
	
	virtual void call(const VMachine& vm);
};

class MethodImpl : public FunImpl{
public:

	MethodImpl(const Frame& outer, const Code& code, FunCore* core)
		:FunImpl(outer, null, code, core){
	}

public:
	
	virtual void call(const VMachine& vm);
};

class FiberImpl : public FunImpl{
public:

	FiberImpl(const Frame& outer, const Any& th, const Code& code, FunCore* core);
		
	~FiberImpl();
	
public:

	void iter_next(const VMachine& vm){
		call_helper(vm, true);
	}

	void halt();

	void call(const VMachine& vm){
		call_helper(vm, false);
	}

	void call_helper(const VMachine& vm, bool add_succ_or_fail_result);

	VMachine vm_;
	const inst_t* resume_pc_;

	void visit_members(Visitor& m){
		FunImpl::visit_members(m);
		m & vm_;
	}
};



}
