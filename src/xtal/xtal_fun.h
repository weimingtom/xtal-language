/** \file src/xtal/xtal_fun.h
* \brief src/xtal/xtal_fun.h
*/

#ifndef XTAL_FUN_H_INCLUDE_GUARD
#define XTAL_FUN_H_INCLUDE_GUARD

#pragma once

namespace xtal{

/**
* \xbind lib::builtin
* \xinherit lib::builtin::Any
* \brief 引数オブジェクト
*/
class Arguments : public Base{
public:

	/**
	* \xbind
	* \brief 順番指定引数、名前付き引数を渡して引数オブジェクトを構築する
	*/
	Arguments(const AnyPtr& ordered = undefined, const AnyPtr& named = undefined);

	const AnyPtr& op_at_int(uint_t index);

	const AnyPtr& op_at_string(const StringPtr& key);

	/**
	* \xbind
	* \brief keyに対応する引数を返す
	* keyが整数であれば、順番指定引数、文字列であれば名前付き引数を返す
	*/
	const AnyPtr& op_at(const AnyPtr& key){
		if(type(key)==TYPE_INT){
			return op_at_int(ivalue(key));
		}
		return op_at_string(key->to_s());
	}

	/**
	* \xbind
	* \brief 順番指定引数の数を返す
	*/
	uint_t length();
	
	/**
	* \xbind
	* \brief 順番指定引数を列挙するためのイテレータを返す
	*/
	AnyPtr ordered_arguments();
	
	/**
	* \xbind
	* \brief 名前付き引数を列挙するためのイテレータを返す
	*/
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

class InstanceVariableGetter : public HaveParentRefCountingBase{
public:
	enum{ TYPE = TYPE_IVAR_GETTER };

	InstanceVariableGetter(int_t number, ClassInfo* info);

	void rawcall(const VMachinePtr& vm);

public:
	int_t number_;
	ClassInfo* info_;
};

class InstanceVariableSetter : public HaveParentRefCountingBase{
public:
	enum{ TYPE = TYPE_IVAR_SETTER };

	InstanceVariableSetter(int_t number, ClassInfo* info);

	void rawcall(const VMachinePtr& vm);

public:
	int_t number_;
	ClassInfo* info_;
};

/**
* \xbind lib::builtin
* \xinherit lib::builtin::Any
* \brief メソッドオブジェクト
*/
class Method : public HaveParentBase{
public:

	Method(const FramePtr& outer, const CodePtr& code, FunInfo* info);

	const FramePtr& outer(){ return outer_; }

	const CodePtr& code(){ return code_; }

	int_t pc(){ return info_->pc; }

	const inst_t* source(){ return code_->data()+info_->pc; }

	const IDPtr& param_name_at(size_t i){ return code_->identifier(i+info_->variable_identifier_offset); }

	int_t param_size(){ return info_->variable_size-(int)extendable_param(); }	

	bool extendable_param(){ return (info_->flags&FunInfo::FLAG_EXTENDABLE_PARAM)!=0; }

	FunInfo* info(){ return info_; }

	void set_info(FunInfo* fc){ info_ = fc; }

	bool check_arg(const VMachinePtr& vm);

	const StringPtr& object_temporary_name();

public:
		
	virtual void rawcall(const VMachinePtr& vm);
	
protected:

	FramePtr outer_;
	CodePtr code_;
	FunInfo* info_;
	
	virtual void visit_members(Visitor& m){
		HaveParentBase::visit_members(m);
		m & outer_ & code_;
	}

};

/**
* \xbind lib::builtin
* \xinherit lib::builtin::Method
* \brief 関数オブジェクト
*/
class Fun : public Method{
public:

	Fun(const FramePtr& outer, const AnyPtr& athis, const CodePtr& code, FunInfo* info)
		:Method(outer, code, info), this_(athis){
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

/**
* \xbind lib::builtin
* \xinherit lib::builtin::Fun
* \brief ラムダオブジェクト
*/
class Lambda : public Fun{
public:

	Lambda(const FramePtr& outer, const AnyPtr& th, const CodePtr& code, FunInfo* info)
		:Fun(outer, th, code, info){
	}

public:
	
	virtual void rawcall(const VMachinePtr& vm);
};

/**
* \xbind lib::builtin
* \xinherit lib::builtin::Fun
* \brief ファイバーオブジェクト
*/
class Fiber : public Fun{
public:

	Fiber(const FramePtr& outer, const AnyPtr& th, const CodePtr& code, FunInfo* info);

	virtual void finalize();
			
public:

	void block_next(const VMachinePtr& vm){
		call_helper(vm, true);
	}

	/**
	* \brief 実行を強制停止する
	*/
	void halt();

	void rawcall(const VMachinePtr& vm){
		call_helper(vm, false);
	}

	/**
	* \brief ファイバーオブジェクトが実行中かどうか
	*/
	bool is_alive(){
		return alive_;
	}

	/**
	* \brief ファイバーを強制停止し、初期状態に戻す
	*/
	AnyPtr reset();

	void call_helper(const VMachinePtr& vm, bool add_succ_or_fail_result);

private:

	VMachinePtr vm_;
	const inst_t* resume_pc_;
	bool alive_;

	void visit_members(Visitor& m){
		Fun::visit_members(m);
		m & vm_;
	}
};

class BindedThis : public Base{
public:

	BindedThis(const AnyPtr& fun, const AnyPtr& athis)
		:fun_(fun), this_(athis){
	}

public:
	
	virtual void rawcall(const VMachinePtr& vm);

protected:

	virtual void visit_members(Visitor& m){
		Base::visit_members(m);
		m & fun_ & this_;
	}

	AnyPtr fun_;
	AnyPtr this_;
};

inline AnyPtr bind_this(const AnyPtr& fun, const AnyPtr& athis){
	return xnew<BindedThis>(fun, athis);
}

}


#endif // XTAL_FUN_H_INCLUDE_GUARD
