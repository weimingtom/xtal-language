
#pragma once

namespace xtal{

// 引数オブジェクト
class Arguments : public Base{
public:

	Arguments(const AnyPtr& ordered = undefined, const AnyPtr& named = undefined);

	const AnyPtr& op_at_int(uint_t index);

	const AnyPtr& op_at_string(const StringPtr& key);

	uint_t length();
	
	AnyPtr ordered_arguments();
	
	AnyPtr named_arguments();

public:

	void add_ordered(const AnyPtr& v);

	void add_named(const AnyPtr& k, const AnyPtr& v);

	void add_named(const VMachinePtr& vm);

	uint_t ordered_size();

	uint_t named_size();

private:

	ArrayPtr ordered_;
	MapPtr named_;

	virtual void visit_members(Visitor& m);
};

class InstanceVariableGetter : public HaveParent{
public:

	InstanceVariableGetter(int_t number, ClassInfo* info);

	virtual void rawcall(const VMachinePtr& vm);

private:
	int_t number_;
	ClassInfo* info_;
};

class InstanceVariableSetter : public HaveParent{
public:

	InstanceVariableSetter(int_t number, ClassInfo* info);

	virtual void rawcall(const VMachinePtr& vm);

private:
	int_t number_;
	ClassInfo* info_;
};

class Method : public HaveParent{
public:

	Method(const FramePtr& outer, const CodePtr& code, FunInfo* core);

	const FramePtr& outer(){ return outer_; }
	const CodePtr& code(){ return code_; }
	int_t pc(){ return info_->pc; }
	const inst_t* source(){ return code_->data()+info_->pc; }
	const IDPtr& param_name_at(size_t i){ return code_->identifier(i+info_->variable_identifier_offset); }
	int_t param_size(){ return info_->variable_size; }	
	bool extendable_param(){ return (info_->flags&FunInfo::FLAG_EXTENDABLE_PARAM)!=0; }
	FunInfo* info(){ return info_; }
	void set_info(FunInfo* fc){ info_ = fc; }
	bool check_arg(const VMachinePtr& vm);

public:
		
	virtual void rawcall(const VMachinePtr& vm);
	
protected:

	FramePtr outer_;
	CodePtr code_;
	FunInfo* info_;
	
	virtual void visit_members(Visitor& m){
		HaveParent::visit_members(m);
		m & outer_ & code_;
	}

};

class Fun : public Method{
public:

	Fun(const FramePtr& outer, const AnyPtr& athis, const CodePtr& code, FunInfo* core)
		:Method(outer, code, core), this_(athis){
	}

public:
	
	virtual void rawcall(const VMachinePtr& vm);

protected:

	virtual void visit_members(Visitor& m){
		Method::visit_members(m);
		m & this_;
	}

	AnyPtr this_;
};

class Lambda : public Fun{
public:

	Lambda(const FramePtr& outer, const AnyPtr& th, const CodePtr& code, FunInfo* core)
		:Fun(outer, th, code, core){
	}

public:
	
	virtual void rawcall(const VMachinePtr& vm);
};

class Fiber : public Fun{
public:

	Fiber(const FramePtr& outer, const AnyPtr& th, const CodePtr& code, FunInfo* core);

	virtual void finalize();
			
public:

	void block_next(const VMachinePtr& vm){
		call_helper(vm, true);
	}

	void halt();

	void rawcall(const VMachinePtr& vm){
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

