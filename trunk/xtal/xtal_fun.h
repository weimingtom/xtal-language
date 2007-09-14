
#pragma once

#include "xtal_frame.h"

namespace xtal{

class InstanceVariableGetter : public HaveName{
public:

	InstanceVariableGetter(int_t number, ClassCore* core);

	virtual void call(const VMachinePtr& vm);

private:
	int_t number_;
	ClassCore* core_;
};

class InstanceVariableSetter : public HaveName{
public:

	InstanceVariableSetter(int_t number, ClassCore* core);

	virtual void call(const VMachinePtr& vm);

private:
	int_t number_;
	ClassCore* core_;
};

class Fun : public HaveName{
public:

	Fun(const FramePtr& outer, const AnyPtr& athis, const CodePtr& code, FunCore* core);

	const FramePtr& outer(){ return outer_; }
	const CodePtr& code(){ return code_; }
	int_t pc(){ return core_->pc; }
	const inst_t* source();
	const InternedStringPtr& param_name_at(size_t i);
	int_t param_size(){ return core_->variable_size; }	
	bool used_args_object(){ return core_->used_args_object!=0; }
	int_t defined_file_lineno(){ return core_->lineno; }
	FunCore* core(){ return core_; }
	void set_core(FunCore* fc){ core_ = fc; }
	void check_arg(const VMachinePtr& vm);

public:
		
	virtual void call(const VMachinePtr& vm);
	
protected:

	FramePtr outer_;
	AnyPtr this_;
	CodePtr code_;
	FunCore* core_;
	
	virtual void visit_members(Visitor& m){
		HaveName::visit_members(m);
		m & outer_ & this_ & code_;
	}

};

class Lambda : public Fun{
public:

	Lambda(const FramePtr& outer, const AnyPtr& th, const CodePtr& code, FunCore* core)
		:Fun(outer, th, code, core){
	}

public:
	
	virtual void call(const VMachinePtr& vm);
};

class Method : public Fun{
public:

	Method(const FramePtr& outer, const CodePtr& code, FunCore* core)
		:Fun(outer, null, code, core){
	}

public:
	
	virtual void call(const VMachinePtr& vm);
};

class Fiber : public Fun{
public:

	Fiber(const FramePtr& outer, const AnyPtr& th, const CodePtr& code, FunCore* core);
			
public:

	void block_next(const VMachinePtr& vm){
		call_helper(vm, true);
	}

	void halt();

	void call(const VMachinePtr& vm){
		call_helper(vm, false);
	}

	void call_helper(const VMachinePtr& vm, bool add_succ_or_fail_result);

	bool is_alive(){
		return alive_;
	}

	AnyPtr reset();

private:

	VMachinePtr vm_;
	const inst_t* resume_pc_;
	bool alive_;

	void visit_members(Visitor& m){
		Fun::visit_members(m);
		m & vm_;
	}
};

// 引数オブジェクト
class Arguments : public Base{
public:

	Arguments(){
		ordered_ = xnew<Array>();
		named_ = xnew<Map>();
	}

	const AnyPtr& op_at(const AnyPtr& index){
		if(type(index)==TYPE_INT){
			return ordered_->at(index->to_i());
		}
		return named_->at(index); 
	}
	
	int_t length(){
		return ordered_->length();
	}
	
	AnyPtr ordered_arguments(){
		return ordered_->each();
	}
	
	AnyPtr named_arguments(){
		return named_->each();
	}

public:

	ArrayPtr ordered_;
	MapPtr named_;

	virtual void visit_members(Visitor& m);
};

}

