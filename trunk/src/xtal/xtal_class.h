/** \file src/xtal/xtal_class.h
* \brief src/xtal/xtal_class.h
*/

#ifndef XTAL_CLASS_H_INCLUDE_GUARD
#define XTAL_CLASS_H_INCLUDE_GUARD

#pragma once

namespace xtal{

class InstanceVariables{
public:

	struct uninit_t{};

	InstanceVariables(uninit_t){}

	InstanceVariables();
			
	~InstanceVariables();
		
	void init_variables(ClassInfo* class_info);

	uint_t find_class_info(ClassInfo* class_info, uint_t index){
		VariablesInfo& info = variables_info_.top();
		if(info.class_info == class_info){
			return info.pos + index;
		}
		return find_class_info_inner(class_info, index);
	}

	const AnyPtr& variable(uint_t index, ClassInfo* class_info){
		return variables_.at(find_class_info(class_info, index));
	}

	void set_variable(uint_t index, ClassInfo* class_info, const AnyPtr& value){
		variables_.set_at(find_class_info(class_info, index), value);
	}

	bool is_included(ClassInfo* class_info);

	uint_t find_class_info_inner(ClassInfo* class_info, uint_t index);

	void replace(ClassInfo* from, ClassInfo* to);

	bool empty(){
		return variables_.empty();
	}

	void clear(){
		variables_info_.clear();
		variables_.clear();
	}

	void visit_members(Visitor& m){
		for(uint_t i=0; i<variables_.size(); ++i){
			m & variables_.at(i);
		}
	}

protected:
	
	struct VariablesInfo{
		ClassInfo* class_info;
		int_t pos;
	};

	PODStack<VariablesInfo> variables_info_;
	Array variables_;
};

class EmptyInstanceVariables : public InstanceVariables{
public:

	EmptyInstanceVariables();

	~EmptyInstanceVariables();

	static VariablesInfo vi;
};

class Instance : public Base{
public:

	Instance(){
		instance_variables_ = &variables_;
	}

	~Instance();

private:
	InstanceVariables variables_;
};

/**
* \xbind lib::builtin
* \xinherit lib::builtin::Frame
* \brief クラス
*/
class Class : public Frame{
public:

	Class(const StringPtr& name = empty_string);

	Class(const FramePtr& outer, const CodePtr& code, ClassInfo* info);

	virtual ~Class();

	void overwrite(const ClassPtr& p);

	void overwrite_member(const IDPtr& primary_key, const AnyPtr& value, const AnyPtr& secondary_key = undefined, int_t accessibility = KIND_PUBLIC);

public:

	/**
	* \brief 新しいメンバを定義する
	* \param primary_key 新しく定義するメンバの名前
	* \param value 設定する値
	* \param secondary_key セカンダリキー
	* \param accessibility 可蝕性
	*/
	virtual void def(const IDPtr& primary_key, const AnyPtr& value, const AnyPtr& secondary_key, int_t accessibility = KIND_PUBLIC);
	
	void def(const char_t* primary_key, const AnyPtr& value, const AnyPtr& secondary_key, int_t accessibility = KIND_PUBLIC);
	void def(const char8_t* primary_key, const AnyPtr& value, const AnyPtr& secondary_key, int_t accessibility = KIND_PUBLIC);

	void def(const IDPtr& primary_key, const AnyPtr& value);
	void def(const char_t* primary_key, const AnyPtr& value);
	void def(const char8_t* primary_key, const AnyPtr& value);

	/**
	* \internal
	* \brief メンバを取り出す
	*
	* この関数を使うのではなく、Any::memberを使うこと。
	*/
	virtual const AnyPtr& rawmember(const IDPtr& primary_key, const AnyPtr& secondary_key, bool inherited_too, int_t& accessibility, bool& nocache);

	/**
	* \brief メンバを再設定する
	* Xtalレベルでは禁止されている操作だが、C++レベルでは可能にしておく
	* \param primary_key 新しく定義するメンバの名前
	* \param value 設定する値
	* \param secondary_key セカンダリキー
	*/
	bool set_member(const IDPtr& primary_key, const AnyPtr& value, const AnyPtr& secondary_key);

	/**
	* \xbind
	* \brief 継承
	* \param cls 継承するクラスオブジェクト
	*/
	void inherit(const ClassPtr& cls);

	/**
	* \brief 継承する
	* Xtalレベルで、クラス定義時に継承する場合に呼ばれる
	* \param cls 継承するクラスオブジェクト
	*/
	void inherit_first(const ClassPtr& cls);

	/**
	* \brief 継承する
	* \param cls 継承するクラスオブジェクト
	*/
	void inherit_strict(const ClassPtr& cls);

	/**
	* \brief 継承されているか調べる
	* \param cls 継承されている調べたいクラスオブジェクト
	*/
	bool is_inherited(const AnyPtr& cls);

	/**
	* \brief C++のクラスが継承されているか調べる
	*/
	bool is_inherited_cpp_class();

	/**
	* \xbind
	* \brief 継承されているクラスを列挙するIteratorを返す
	*/
	AnyPtr inherited_classes();

	/**
	* \brief 近い名前のメンバを検索する
	*/
	IDPtr find_near_member(const IDPtr& primary_key, const AnyPtr& secondary_key = undefined){
		int_t dist = 0xffffff;
		return find_near_member(primary_key, secondary_key, dist);
	}

	IDPtr find_near_member(const IDPtr& primary_key, const AnyPtr& secondary_key, int_t& dist);
	
	/**
	* \xbind
	* \brief 祖先クラスを列挙するIteratorを返す
	*/
	AnyPtr ancestors();

public:

	/**
	* \brief 関数を定義する
	* cls->def_fun("name", &foo); は cls->def("name", xtal::fun(&foo)); と同一
	* \param primary_key 新しく定義するメンバの名前
	* \param f 設定する関数
	* \param secondary_key セカンダリキー
	* \param accessibility 可蝕性
	*/
	template<class TFun, class Str>
	const NativeFunPtr& def_fun(const Str& primary_key, const TFun& f, const AnyPtr& secondary_key, int_t accessibility = KIND_PUBLIC){
		return def_and_return(primary_key, secondary_key, accessibility, fun_param_holder<cfun_holder<TFun> >::value, &f);
	}

	template<class TFun, class Str>
	const NativeFunPtr& def_fun(const Str& primary_key, const TFun& f){
		return def_and_return(primary_key, fun_param_holder<cfun_holder<TFun> >::value, &f);
	}

	/**
	* \brief メソッドを定義する
	* cls->def_method("name", &Klass::foo); は cls->def("name", xtal::method(&Klass::foo)); と同一
	* \param primary_key 新しく定義するメンバの名前
	* \param f 設定する関数
	* \param secondary_key セカンダリキー
	* \param accessibility 可蝕性
	*/
	template<class TFun, class Str>
	const NativeFunPtr& def_method(const Str& primary_key, const TFun& f, const AnyPtr& secondary_key, int_t accessibility = KIND_PUBLIC){
		return def_and_return(primary_key, secondary_key, accessibility, fun_param_holder<cmemfun_holder<TFun> >::value, &f);
	}

	template<class TFun, class Str>
	const NativeFunPtr& def_method(const Str& primary_key, const TFun& f){
		return def_and_return(primary_key, fun_param_holder<cmemfun_holder<TFun> >::value, &f);
	}

	/**
	* \brief メンバ変数へのポインタからゲッターを生成し、定義する
	* \param primary_key 新しく定義するメンバの名前
	* \param value 設定する値
	* \param secondary_key セカンダリキー
	* \param accessibility 可蝕性
	*/
	template<class T, class C, class Str>
	const NativeFunPtr& def_getter(const Str& primary_key, T C::* v, const AnyPtr& secondary_key = undefined, int_t accessibility = KIND_PUBLIC){
		return def_and_return(primary_key, secondary_key, accessibility, fun_param_holder<getter_holder<C, T> >::value, &v);
	}
	
	/**
	* \brief メンバ変数へのポインタからセッターを生成し、定義する
	* Xtalでは、obj.name = 10; とするにはset_nameとset_を前置したメソッドを定義する必要があるため、
	* 単純なセッターを定義したい場合、set_xxxとすることを忘れないこと。
	* \param primary_key 新しく定義するメンバの名前
	* \param v 設定するメンバ変数へのポインタ
	* \param secondary_key セカンダリキー
	* \param accessibility 可蝕性
	*/
	template<class T, class C, class Str>
	const NativeFunPtr& def_setter(const Str& primary_key, T C::* v, const AnyPtr& secondary_key = undefined, int_t accessibility = KIND_PUBLIC){
		return def_and_return(primary_key, secondary_key, accessibility, fun_param_holder<setter_holder<C, T> >::value, &v);
	}
	
	/**
	* \brief メンバ変数へのポインタからゲッター、セッターを両方生成し、定義する
	* cls->def_getter(primary_key, v, policy);
	* cls->def_setter(StringPtr("set_")->cat(primary_key), v, policy);
	* と等しい	
	* \param primary_key 新しく定義するメンバの名前
	* \param v 設定するメンバ変数へのポインタ
	* \param secondary_key セカンダリキー
	* \param accessibility 可蝕性
	*/	
	template<class T, class U, class Str>
	void def_var(const Str& primary_key, T U::* v, const AnyPtr& secondary_key = undefined, int_t accessibility = KIND_PUBLIC){
		def_getter(primary_key, v, secondary_key, accessibility);
		def_setter(StringPtr("set_")->cat(primary_key), v, secondary_key, accessibility);
	}

	/**
	* \brief 2重ディスパッチメソッドを定義する。
	* \param primary_key 新しく定義するメンバの名前
	* \param accessibility 可蝕性
	*/
	void def_double_dispatch_method(const IDPtr& primary_key, int_t accessibility = KIND_PUBLIC);

	/**
	* \brief 2重ディスパッチ関数を定義する。
	* \param primary_key 新しく定義するメンバの名前
	* \param accessibility 可蝕性
	*/
	void def_double_dispatch_fun(const IDPtr& primary_key, int_t accessibility = KIND_PUBLIC);

public:

	/**
	* \brief コンストラクタ関数を登録する
	*/
	const NativeFunPtr& def_ctor(const NativeFunPtr& ctor_fun);

	/**
	* \brief 登録されたコンストラクタ関数を返す
	*/
	const NativeFunPtr& ctor();

	/**
	* \brief シリアライズに使われるコンストラクタ関数を登録する
	*/
	const NativeFunPtr& def_serial_ctor(const NativeFunPtr& ctor_fun);

	/**
	* \brief 登録されたシリアライズに使われるコンストラクタ関数を返す
	*/	
	const NativeFunPtr& serial_ctor();

//{REPEAT{{
/*
	/// `n`引数のコンストラクタ関数を登録する
	template<class T #COMMA_REPEAT#class A`i`#>
	const NativeFunPtr& def_ctor`n`(){
		return def_ctor(xtal::ctor<T #COMMA_REPEAT#A`i`#>());
	}
*/

	/// 0引数のコンストラクタ関数を登録する
	template<class T >
	const NativeFunPtr& def_ctor0(){
		return def_ctor(xtal::ctor<T >());
	}

	/// 1引数のコンストラクタ関数を登録する
	template<class T , class A0>
	const NativeFunPtr& def_ctor1(){
		return def_ctor(xtal::ctor<T , A0>());
	}

	/// 2引数のコンストラクタ関数を登録する
	template<class T , class A0, class A1>
	const NativeFunPtr& def_ctor2(){
		return def_ctor(xtal::ctor<T , A0, A1>());
	}

	/// 3引数のコンストラクタ関数を登録する
	template<class T , class A0, class A1, class A2>
	const NativeFunPtr& def_ctor3(){
		return def_ctor(xtal::ctor<T , A0, A1, A2>());
	}

	/// 4引数のコンストラクタ関数を登録する
	template<class T , class A0, class A1, class A2, class A3>
	const NativeFunPtr& def_ctor4(){
		return def_ctor(xtal::ctor<T , A0, A1, A2, A3>());
	}

	/// 5引数のコンストラクタ関数を登録する
	template<class T , class A0, class A1, class A2, class A3, class A4>
	const NativeFunPtr& def_ctor5(){
		return def_ctor(xtal::ctor<T , A0, A1, A2, A3, A4>());
	}

//}}REPEAT}

public:

	struct cpp_class_t{};

	Class(cpp_class_t);

	virtual void rawcall(const VMachinePtr& vm);
	
	virtual void s_new(const VMachinePtr& vm);

	void init_instance(const AnyPtr& self, const VMachinePtr& vm);
	
	void set_member_direct(int_t i, const IDPtr& primary_key, const AnyPtr& value, const AnyPtr& secondary_key, int_t accessibility);

	const AnyPtr& any_member(const IDPtr& primary_key, const AnyPtr& secondary_key);
	
	const AnyPtr& bases_member(const IDPtr& primary_key);

	const AnyPtr& find_member(const IDPtr& primary_key, const AnyPtr& secondary_key, bool inherited_too, int_t& accessibility, bool& nocache);

	ValuesPtr child_object_name(const AnyPtr& a);

	void set_object_name(const StringPtr& name);

	StringPtr object_name();

	void set_object_parent(const ClassPtr& parent);

	uint_t object_force(){
		return object_force_;
	}

	void set_object_force(uint_t force){
		object_force_ = (u16)force;
	}

	ClassInfo* info(){
		return (ClassInfo*)Frame::info();
	}

	void unset_native(){
		flags_ &= ~FLAG_NATIVE;
	}
	
	bool is_native(){
		return (flags_&FLAG_NATIVE)!=0;
	}

	bool is_final(){
		return (flags_&FLAG_FINAL)!=0;
	}

	void set_final(){
		flags_ |= FLAG_FINAL;
	}

	bool is_singleton(){
		return (flags_&FLAG_SINGLETON)!=0;
	}

	void prebind();

	void bind();

	void set_symbol_data(CppClassSymbolData* data){
		symbol_data_ = data;
	}

	void set_singleton();

	void init_singleton(const VMachinePtr& vm);

protected:

	const NativeFunPtr& def_and_return(const IDPtr& primary_key, const AnyPtr& secondary_key, int_t accessibility, const param_types_holder_n& pth, const void* val);
	const NativeFunPtr& def_and_return(const char_t* primary_key, const AnyPtr& secondary_key, int_t accessibility, const param_types_holder_n& pth, const void* val);
	const NativeFunPtr& def_and_return(const char8_t* primary_key, const AnyPtr& secondary_key, int_t accessibility, const param_types_holder_n& pth, const void* val);

	const NativeFunPtr& def_and_return(const IDPtr& primary_key, const param_types_holder_n& pth, const void* val);
	const NativeFunPtr& def_and_return(const char_t* primary_key, const param_types_holder_n& pth, const void* val);
	const NativeFunPtr& def_and_return(const char8_t* primary_key, const param_types_holder_n& pth, const void* val);
	
	const AnyPtr& def2(const IDPtr& primary_key, const AnyPtr& value, const AnyPtr& secondary_key = null, int_t accessibility = KIND_PUBLIC);

	/*
	struct InstanceVariablesInfo{
		ClassInfo* class_info;
		int_t pos;

		InstanceVariablesInfo(ClassInfo* info = 0, int_t n = 0){
			class_info = info;
			pos = n;
		}
	};

	PODStack<InstanceVariablesInfo> instance_variables_layout_;
	*/

	StringPtr name_;

	NativeFunPtr ctor_;
	NativeFunPtr serial_ctor_;

	PODArrayList<Class*> inherited_classes_;
	CppClassSymbolData* symbol_data_;
	u16 object_force_;
	u16 flags_;

	enum{
		FLAG_NATIVE = 1<<0,
		FLAG_FINAL = 1<<1,
		FLAG_SINGLETON = 1<<2,
		FLAG_PREBINDED = 1<<3,
		FLAG_BINDED = 1<<4
	};

	virtual void visit_members(Visitor& m){
		Frame::visit_members(m);
		for(uint_t i=0; i<inherited_classes_.size(); ++i){
			m & inherited_classes_[i];
		}
	}

	friend class ClassInheritedClassesIter;
};


class ClassInheritedClassesIter : public Base{
public:

	ClassInheritedClassesIter(const ClassPtr& cls)
		:class_(cls), index_(0){}
	
	void block_next(const VMachinePtr& vm){
		++index_;
		if(index_<=class_->inherited_classes_.size()){
			vm->return_result(to_smartptr(this), to_smartptr(class_->inherited_classes_[index_-1]));
		}
		else{
			vm->return_result(null, null);
		}
	}

private:
	ClassPtr class_;
	uint_t index_;

	virtual void visit_members(Visitor& m){
		Base::visit_members(m);
		m & class_;
	}
};

}

#endif // XTAL_CLASS_H_INCLUDE_GUARD
