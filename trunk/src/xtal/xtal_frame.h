
#pragma once

namespace xtal{

class InstanceVariables{
public:

	struct uninit_t{};

	InstanceVariables(uninit_t){}

	InstanceVariables()		
		:variables_(xnew<Array>()){
		VariablesInfo vi;
		vi.core = 0;
		vi.pos = 0;
		variables_info_.push(vi);
	}
			
	~InstanceVariables(){}
		
	void init_variables(ClassCore* core){
		VariablesInfo vi;
		vi.core = core;
		vi.pos = (int_t)variables_->size();
		variables_->resize(vi.pos+core->instance_variable_size);
		variables_info_.push(vi);
	}

	const AnyPtr& variable(int_t index, ClassCore* core){
		return variables_->at(find_core(core) + index);
	}

	void set_variable(int_t index, ClassCore* core, const AnyPtr& value){
		variables_->set_at(find_core(core) + index, value);
	}

	int_t find_core(ClassCore* core){
		VariablesInfo& info = variables_info_.top();
		if(info.core == core)
			return info.pos;
		return find_core_inner(core);
	}

	bool is_included(ClassCore* core){
		VariablesInfo& info = variables_info_.top();
		if(info.core == core)
			return true;
		for(int_t i = 1, size = (int_t)variables_info_.size(); i<size; ++i){
			if(variables_info_[i].core==core){
				std::swap(variables_info_[0], variables_info_[i]);
				return true;
			}	
		}
		return false;
	}

	int_t find_core_inner(ClassCore* core);

	bool empty(){
		return variables_->empty();
	}

	void visit_members(Visitor& m){
		m & variables_;
	}

protected:
	
	struct VariablesInfo{
		ClassCore* core;
		int_t pos;
	};

	PODStack<VariablesInfo> variables_info_;
	ArrayPtr variables_;
};

class EmptyInstanceVariables : public InstanceVariables{
public:
	EmptyInstanceVariables():InstanceVariables(uninit_t()){}

	void init(){
		VariablesInfo vi;
		vi.core = 0;
		vi.pos = 0;
		variables_info_.push(vi);
	}

	void uninit(){
		variables_info_.release();
	}	
};
	
class Frame : public HaveName{
public:
	
	Frame(const FramePtr& outer, const CodePtr& code, BlockCore* core);
	
	Frame();
		
	~Frame();

public:
	
	/**
	* @brief 外側のスコープを表すFrameオブジェクトを返す。
	*
	*/
	const FramePtr& outer(){ 
		return outer_; 
	}

	const CodePtr& code(){ 
		return code_; 
	}

	/**
	* @brief リテラル時に定義された要素の数を返す。
	*
	*/
	int_t block_size(){ 
		return core_->variable_size; 
	}

	/**
	* @brief i番目のメンバーをダイレクトに取得。
	*
	*/
	const AnyPtr& member_direct(int_t i){
		return members_->at(i);
	}

	/**
	* @brief i番目のメンバーをダイレクトに設定。
	*
	*/
	void set_member_direct(int_t i, const AnyPtr& value){
		members_->set_at(i, value);
	}

	void set_class_member(int_t i, const IDPtr& primary_key, const AnyPtr& value, const AnyPtr& secondary_key, int_t accessibility);
		
	void set_object_name(const StringPtr& name, int_t force, const AnyPtr& parent);

	virtual StringPtr object_name();

public:

	/**
	* @brief メンバが格納された、Iteratorを返す
	*
	* @code
	* Xfor2(key, value, frame.members()){
	*   puts(Xf("%s %s")(key, value).to_s().c_str());
	* }
	* @endcode
	*/
	AnyPtr members();

protected:

	void make_map_members();

	friend class MembersIter;

	FramePtr outer_;
	CodePtr code_;
	BlockCore* core_;
	
	ArrayPtr members_;

	struct Key{
		IDPtr primary_key;
		AnyPtr secondary_key;

		friend void visit_members(Visitor& m, const Key& a){
			m & a.primary_key & a.secondary_key;
		}
	};

	struct Value{
		u16 num;
		u16 flags;

		friend void visit_members(Visitor& m, const Value&){}
	};

	struct Fun{
		static uint_t hash(const Key& key){
			return (rawvalue(key.primary_key)>>3) ^ rawvalue(key.secondary_key);
		}

		static bool eq(const Key& a, const Key& b){
			return raweq(a.primary_key, b.primary_key) && raweq(a.secondary_key, b.secondary_key);
		}
	};

	typedef Hashtable<Key, Value, Fun> map_t; 
	map_t table_;

	map_t* map_members_;


	virtual void visit_members(Visitor& m){
		HaveName::visit_members(m);
		m & outer_ & code_ & members_;
		if(map_members_){
			m & *map_members_;
		}
	}
};

class Class : public Frame{
public:

	Class(const char* name = "");

	Class(const FramePtr& outer, const CodePtr& code, ClassCore* core);

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
	virtual const AnyPtr& do_member(const IDPtr& primary_key, const AnyPtr& secondary_key = null, const AnyPtr& self = null, bool inherited_too = true);

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
	* @brief 関数を定義する
	*
	* cls.fun("name", &foo); は cls.def("name", xtal::fun(&foo)); と同一
	*/
	template<class Fun, class Policy>
	CFunPtr fun(const IDPtr& primary_key, Fun fun, const AnyPtr& secondary_key, int_t accessibility, const Policy& policy){
		return def_and_return(primary_key, xtal::fun(fun, policy), secondary_key, accessibility);
	}

	/**
	* @brief 関数を定義する
	*
	* cls.fun("name", &foo); は cls.def("name", xtal::fun(&foo)); と同一
	*/
	template<class Fun>
	CFunPtr fun(const IDPtr& primary_key, Fun f, const AnyPtr& secondary_key = null, int_t accessibility = KIND_PUBLIC){
		return fun(primary_key, f, secondary_key, accessibility, result);
	}

	/**
	* @brief 関数を定義する
	*
	* cls.method("name", &foo); は cls.def("name", xtal::method(&foo)); と同一
	*/
	template<class Fun, class Policy>
	CFunPtr method(const IDPtr& primary_key, Fun fun, const AnyPtr& secondary_key, int_t accessibility, const Policy& policy){
		return def_and_return(primary_key, xtal::method(fun, policy), secondary_key, accessibility);
	}

	/**
	* @brief 関数を定義する
	*
	* cls.method("name", &foo); は cls.def("name", xtal::method(&foo)); と同一
	*/
	template<class Fun>
	CFunPtr method(const IDPtr& primary_key, Fun fun, const AnyPtr& secondary_key = null, int_t accessibility = KIND_PUBLIC){
		return method(primary_key, fun, secondary_key, accessibility, result);
	}

	
	/**
	* @brief メンバ変数へのポインタからゲッターを生成し、定義する
	*
	*/
	template<class T, class U, class Policy>
	CFunPtr getter(const IDPtr& primary_key, T U::* v, const AnyPtr& secondary_key, int_t accessibility, const Policy& policy){
		return def_and_return(primary_key, xtal::getter(v, policy), secondary_key, accessibility);
	}
	
	/**
	* @brief メンバ変数へのポインタからセッターを生成し、定義する
	*
	* Xtalでは、obj.name = 10; とするにはset_nameとset_を前置したメソッドを定義する必要があるため、
	* 単純なセッターを定義したい場合、set_xxxとすることを忘れないこと。
	*/
	template<class T, class U, class Policy>
	CFunPtr setter(const IDPtr& primary_key, T U::* v, const AnyPtr& secondary_key, int_t accessibility, const Policy& policy){
		return def_and_return(primary_key, xtal::setter(v, policy), secondary_key, accessibility);
	}
	
	/**
	* @brief メンバ変数へのポインタからゲッター、セッターを両方生成し、定義する
	*
	* cls->getter(name, var, policy);
	* cls->setter(StringPtr("set_")->cat(name), v, policy);
	* と等しい	
	*/	
	template<class T, class U, class Policy>
	void var(const IDPtr& primary_key, T U::* v, const AnyPtr& secondary_key, int_t accessibility, const Policy& policy){
		getter(primary_key, v, secondary_key, accessibility, policy);
		setter(String("set_").cat(primary_key), v, secondary_key, accessibility, policy);
	}
	
	/**
	* @brief メンバ変数へのポインタからゲッターを生成し、定義する
	*
	*/
	template<class T, class U>
	CFunPtr getter(const IDPtr& primary_key, T U::* v, const AnyPtr& secondary_key = null, int_t accessibility = KIND_PUBLIC){
		return getter(primary_key, v, secondary_key, accessibility, result);
	}
	
	/**
	* @brief メンバ変数へのポインタからセッターを生成し、定義する
	*
	*/
	template<class T, class U>
	CFunPtr setter(const IDPtr& primary_key, T U::* v, const AnyPtr& secondary_key = null, int_t accessibility = KIND_PUBLIC){
		return setter(primary_key, v, secondary_key, accessibility, result);
	}
	
	/**
	* @brief メンバ変数へのポインタからゲッター、セッターを両方生成し、定義する
	*
	* cls->getter(name, v);
	* cls->setter(StringPtr("set_")->cat(name), v);
	* と等しい	
	*/	
	template<class T, class U>
	void var(const IDPtr& primary_key, T U::* v, const AnyPtr& secondary_key = null, int_t accessibility = KIND_PUBLIC){
		var(primary_key, v, secondary_key, accessibility, result);
	}

	/**
	* @brief 2重ディスパッチメソッドを定義する。
	*/
	void dual_dispatch_method(const IDPtr& primary_key, int_t accessibility = KIND_PUBLIC){
		def(primary_key, xtal::dual_dispatch_method(primary_key), null, accessibility);
	}

public:

	virtual void call(const VMachinePtr& vm);
	
	virtual void s_new(const VMachinePtr& vm);

	void init_instance(const AnyPtr& self, const VMachinePtr& vm);
	
	const AnyPtr& any_member(const IDPtr& primary_key, const AnyPtr& secondary_key);
	
	const AnyPtr& bases_member(const IDPtr& primary_key);

	const AnyPtr& find_member(const IDPtr& primary_key, const AnyPtr& secondary_key = null, const AnyPtr& self = null, bool inherited_too = true);

	ClassCore* core(){
		return (ClassCore*)core_;
	}
	
	struct cpp_class_t{};

	Class(cpp_class_t, const char* name = "");

	bool is_cpp_class(){
		return is_cpp_class_;
	}

protected:

	CFunPtr def_and_return(const IDPtr& primary_key, const CFunPtr& cfun, const AnyPtr& secondary_key = null, int_t accessibility = KIND_PUBLIC){
		def(primary_key, cfun, secondary_key, accessibility);
		return cfun;
	}

	ArrayPtr mixins_;
	bool is_cpp_class_;

	virtual void visit_members(Visitor& m){
		Frame::visit_members(m);
		m & mixins_;
	}

};

class CppClass : public Class{
public:
		
	CppClass(const char* name = "");

public:

	virtual void call(const VMachinePtr& vm);

	virtual void s_new(const VMachinePtr& vm);
};

class Lib : public Class{
public:

	Lib();

	Lib(const ArrayPtr& path);
	
	virtual const AnyPtr& do_member(const IDPtr& primary_key, const AnyPtr& secondary_key, const AnyPtr& self, bool inherited_too = true);

	virtual void def(const IDPtr& primary_key, const AnyPtr& value, const AnyPtr& secondary_key, int_t accessibility);

	void append_load_path(const StringPtr& path){
		load_path_list_->push_back(path);
	}

private:

	const AnyPtr& rawdef(const IDPtr& primary_key, const AnyPtr& value, const AnyPtr& secondary_key);

	StringPtr join_path(const StringPtr& sep);

private:

	ArrayPtr load_path_list_;
	ArrayPtr path_;

	virtual void visit_members(Visitor& m){
		Class::visit_members(m);
		m & path_ & load_path_list_;
	}
};

class Singleton : public Class{
public:

	Singleton(const char* name = "");

	Singleton(const FramePtr& outer, const CodePtr& code, ClassCore* core);
	
public:

	void init_singleton(const VMachinePtr& vm);

	// クラスの設定はスルーする
	void set_class(const ClassPtr&){}

	virtual void call(const VMachinePtr& vm);
	
	virtual void s_new(const VMachinePtr& vm);
};



struct CppClassHolderList{
	ClassPtr value;
	CppClassHolderList* next;
};

void chain_cpp_class(CppClassHolderList& link);

// C++のクラスの保持のためのクラス
template<class T>
struct CppClassHolder{
	static CppClassHolderList value;
};

template<class T>
CppClassHolderList CppClassHolder<T>::value;

template<class T>
const ClassPtr& new_cpp_class(const char* name){
	if(!CppClassHolder<T>::value.value){
		chain_cpp_class(CppClassHolder<T>::value);
		CppClassHolder<T>::value.value = xnew<CppClass>(name);
	}
	return CppClassHolder<T>::value.value;
}

template<class T>
inline bool exists_cpp_class(){
	return CppClassHolder<T>::value.value;
}

template<class T>
inline const ClassPtr& get_cpp_class(){
	XTAL_ASSERT(exists_cpp_class<T>());
	return CppClassHolder<T>::value.value;
}

template<class T>
void set_cpp_class(const ClassPtr& cls){
	if(!CppClassHolder<T>::value.value){
		chain_cpp_class(CppClassHolder<T>::value);
	}
	CppClassHolder<T>::value.value = cls;
}

}
