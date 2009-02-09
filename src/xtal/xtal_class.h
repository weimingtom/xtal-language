
#pragma once

namespace xtal{

class InstanceVariables{
public:

	struct uninit_t{};

	InstanceVariables(uninit_t){}

	InstanceVariables();
			
	~InstanceVariables();
		
	void init_variables(ClassInfo* core);

	const AnyPtr& variable(int_t index, ClassInfo* core){
		return variables_->at(find_core(core) + index);
	}

	void set_variable(int_t index, ClassInfo* core, const AnyPtr& value){
		variables_->set_at(find_core(core) + index, value);
	}

	int_t find_core(ClassInfo* core){
		VariablesInfo& info = variables_info_.top();
		if(info.core == core)
			return info.pos;
		return find_core_inner(core);
	}

	bool is_included(ClassInfo* core);

	int_t find_core_inner(ClassInfo* core);

	bool empty(){
		return variables_->empty();
	}

	void visit_members(Visitor& m){
		m & variables_;
	}

protected:
	
	struct VariablesInfo{
		ClassInfo* core;
		int_t pos;
	};

	PODStack<VariablesInfo> variables_info_;
	ArrayPtr variables_;
};

class EmptyInstanceVariables : public InstanceVariables{
public:

	EmptyInstanceVariables();

	~EmptyInstanceVariables();

	static VariablesInfo vi;
};
	
class Class : public Frame{
public:

	Class(const StringPtr& name = empty_string);

	Class(const FramePtr& outer, const CodePtr& code, ClassInfo* core);

	void overwrite(const ClassPtr& p);

public:
	
	/**
	* @brief 新しいメンバを定義する
	*
	* @param name 新しく定義するメンバの名前
	*/
	virtual void def(const IDPtr& primary_key, const AnyPtr& value, const AnyPtr& secondary_key = null, int_t accessibility = KIND_PUBLIC);

	/**
	* @brief メンバを取り出す
	* 可触性を考慮して取り出す
	*
	* @param name 取り出したいメンバの名前
	* @param self 可視性を判定するためのオブジェクト
	*/
	virtual const AnyPtr& do_member(const IDPtr& primary_key, const AnyPtr& secondary_key, const AnyPtr& self, bool inherited_too, bool* nocache);

	/**
	* @brief メンバを再設定する
	* Xtalレベルでは禁止されている操作だが、C++レベルでは可能にしておく
	*
	* @param name 再設定したいメンバの名前
	*/
	void set_member(const IDPtr& primary_key, const AnyPtr& value, const AnyPtr& secondary_key);

	/**
	* @brief Mix-inする
	*
	* @param md Mix-inするクラスオブジェクト
	*/
	void inherit(const ClassPtr& md);

	/**
	* @brief Mix-inする
	*
	* @param md Mix-inするクラスオブジェクト
	*/
	void inherit_strict(const ClassPtr& md);

	/**
	* @brief Mix-inされているか調べる
	*
	* @param md Mix-inされている調べたいクラスオブジェクト
	*/
	bool is_inherited(const AnyPtr& md);

	/**
	* @brief C++のクラスがMix-inされているか調べる
	*
	*/
	bool is_inherited_cpp_class();

	/**
	* @brief Mix-inされているクラスのIteratorを返す
	*
	*/
	AnyPtr inherited_classes();

	/**
	* @brief 近い名前のメンバを検索する
	*/
	IDPtr find_near_member(const IDPtr& primary_key, const AnyPtr& secondary_key = null);
	
	AnyPtr ancestors();

public:
	/**
	* @brief 関数を定義する
	*
	* cls->fun("name", &foo); は cls->def("name", xtal::fun(&foo)); と同一
	*/
	template<class TFun>
	const CFunPtr& def_fun(const IDPtr& primary_key, const TFun& f, const AnyPtr& secondary_key = null, int_t accessibility = KIND_PUBLIC){
		typedef cfun_holder<TFun> fun_t;
		fun_t fun(f);
		return def_and_return(primary_key, secondary_key, accessibility, &cfun<fun_t::PARAMS, fun_t>::f, &fun, sizeof(fun), fun_t::PARAMS2);
	}

	template<class TFun, TFun fun_s>
	const CFunPtr& def_fun_static(const IDPtr& primary_key, const AnyPtr& secondary_key = null, int_t accessibility = KIND_PUBLIC){
		typedef cfun_holder_static<TFun, fun_s> fun_t;
		fun_t fun;
		return def_and_return(primary_key, secondary_key, accessibility, &cfun<fun_t::PARAMS, fun_t>::f, &fun, sizeof(fun), fun_t::PARAMS2);
	}

	/**
	* @brief 関数を定義する
	*
	* cls->method("name", &Klass::foo); は cls->def("name", xtal::method(&Klass::foo)); と同一
	*/
	template<class TFun>
	const CFunPtr& def_method(const IDPtr& primary_key, const TFun& f, const AnyPtr& secondary_key = null, int_t accessibility = KIND_PUBLIC){
		typedef cmemfun_holder<TFun> fun_t;
		fun_t fun(f);
		return def_and_return(primary_key, secondary_key, accessibility, &cfun<fun_t::PARAMS, fun_t>::f, &fun, sizeof(fun), fun_t::PARAMS2);
	}

	template<class TFun, TFun fun_s>
	const CFunPtr& def_method_static(const IDPtr& primary_key, const AnyPtr& secondary_key = null, int_t accessibility = KIND_PUBLIC){
		typedef cmemfun_holder_static<TFun, fun_s> fun_t;
		fun_t fun;
		return def_and_return(primary_key, secondary_key, accessibility, &cfun<fun_t::PARAMS, fun_t>::f, &fun, 0, fun_t::PARAMS2);
	}

	/**
	* @brief メンバ変数へのポインタからゲッターを生成し、定義する
	*
	*/
	template<class T, class C>
	const CFunPtr& def_getter(const IDPtr& primary_key, T C::* v, const AnyPtr& secondary_key = null, int_t accessibility = KIND_PUBLIC){
		typedef getter_holder<C, T> fun_t;
		fun_t fun(v);
		return new_cfun(&cfun<fun_t::PARAMS, fun_t>::f, &fun, sizeof(fun), fun_t::PARAMS2);
	}
	
	/**
	* @brief メンバ変数へのポインタからセッターを生成し、定義する
	*
	* Xtalでは、obj.name = 10; とするにはset_nameとset_を前置したメソッドを定義する必要があるため、
	* 単純なセッターを定義したい場合、set_xxxとすることを忘れないこと。
	*/
	template<class T, class C>
	const CFunPtr& def_setter(const IDPtr& primary_key, T C::* v, const AnyPtr& secondary_key = null, int_t accessibility = KIND_PUBLIC){
		typedef setter_holder<C, T> fun_t;
		fun_t fun(v);
		return new_cfun(&cfun<fun_t::PARAMS, fun_t>::f, &fun, sizeof(fun), fun_t::PARAMS2);
	}
	
	/**
	* @brief メンバ変数へのポインタからゲッター、セッターを両方生成し、定義する
	*
	* cls->getter(primary_key, var, policy);
	* cls->setter(StringPtr("set_")->cat(primary_key), v, policy);
	* と等しい	
	*/	
	template<class T, class U>
	void def_var(const IDPtr& primary_key, T U::* v, const AnyPtr& secondary_key = null, int_t accessibility = KIND_PUBLIC){
		def_getter(primary_key, v, secondary_key, accessibility);
		def_setter(String("set_").cat(primary_key), v, secondary_key, accessibility);
	}

	/**
	* @brief 2重ディスパッチメソッドを定義する。
	*/
	void def_dual_dispatch_method(const IDPtr& primary_key, int_t accessibility = KIND_PUBLIC);

	/**
	* @brief 2重ディスパッチ関数を定義する。
	*/
	void def_dual_dispatch_fun(const IDPtr& primary_key, int_t accessibility = KIND_PUBLIC);

public:

	virtual void rawcall(const VMachinePtr& vm);
	
	virtual void s_new(const VMachinePtr& vm);

	void init_instance(const AnyPtr& self, const VMachinePtr& vm);
	
	const AnyPtr& any_member(const IDPtr& primary_key, const AnyPtr& secondary_key);
	
	const AnyPtr& bases_member(const IDPtr& primary_key);

	const AnyPtr& find_member(const IDPtr& primary_key, const AnyPtr& secondary_key = null, const AnyPtr& self = null, bool inherited_too = true);

	ClassInfo* core(){
		return (ClassInfo*)core_;
	}
	
	struct cpp_class_t{};

	Class(cpp_class_t, const StringPtr& name = empty_string);

	bool is_cpp_class(){
		return is_cpp_class_;
	}

protected:

	const CFunPtr& def_and_return(const IDPtr& primary_key, const AnyPtr& secondary_key, int_t accessibility, void (*fun_t)(ParamInfoAndVM& pvm), const void* val, int_t val_size, int_t param_n);
	
	const AnyPtr& def2(const IDPtr& primary_key, const AnyPtr& value, const AnyPtr& secondary_key = null, int_t accessibility = KIND_PUBLIC);

	ArrayPtr mixins_;
	bool is_cpp_class_;

	virtual void visit_members(Visitor& m){
		Frame::visit_members(m);
		m & mixins_;
	}

};

class CppClass : public Class{
public:
		
	CppClass(const StringPtr& name = empty_string);

public:

	virtual void rawcall(const VMachinePtr& vm);

	virtual void s_new(const VMachinePtr& vm);
};

class Singleton : public Class{
public:

	Singleton(const StringPtr& name = empty_string);

	Singleton(const FramePtr& outer, const CodePtr& code, ClassInfo* core);
	
public:

	void init_singleton(const VMachinePtr& vm);

	// クラスの設定はスルーする
	void set_class(const ClassPtr&){}

	virtual void rawcall(const VMachinePtr& vm);
	
	virtual void s_new(const VMachinePtr& vm);
};

class CppSingleton : public Class{
public:
		
	CppSingleton(const StringPtr& name = empty_string);

public:

	// クラスの設定はスルーする
	void set_class(const ClassPtr&){}

};

}
