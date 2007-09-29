
#pragma once

#include "xtal_frame.h"

namespace xtal{

// 引数オブジェクト
class Arguments : public Base{
public:

	Arguments(const AnyPtr& ordered = nop, const AnyPtr& named = nop){
		if(ordered){ ordered_ = ptr_cast<Array>(ordered); }
		else{ ordered_ = xnew<Array>(); }
		if(named){ named_ = ptr_cast<Map>(named); }
		else{ named_ = xnew<Map>(); }
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

}

