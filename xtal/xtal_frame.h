
#pragma once

#include "xtal_any.h"
#include "xtal_stack.h"
#include "xtal_cfun.h"

namespace xtal{

// C++のクラスの保持のためのクラス
template<class T>
struct CppClassHolder{
	static AnyPtr* value;
};

template<class T>
AnyPtr* CppClassHolder<T>::value = 0;

template<class T>
const ClassPtr& new_cpp_class(const char* name){
	if(!CppClassHolder<T>::value){
		CppClassHolder<T>::value = make_place();
		*CppClassHolder<T>::value = xnew<CppClass>(name);
	}
	return (const ClassPtr&)*CppClassHolder<T>::value;
}

template<class T>
bool exists_cpp_class(){
	return CppClassHolder<T>::value!=0 && ap(*CppClassHolder<T>::value);
}

template<class T>
inline const ClassPtr& get_cpp_class(){
	XTAL_ASSERT(exists_cpp_class<T>());
	return (const ClassPtr&)*CppClassHolder<T>::value;
}

template<class T>
const ClassPtr& set_cpp_class(const ClassPtr& cls){
	if(!CppClassHolder<T>::value){
		CppClassHolder<T>::value = make_place();
	}
	*CppClassHolder<T>::value = cls;
	return (const ClassPtr&)*CppClassHolder<T>::value;
}


class HaveInstanceVariables{
public:

	struct uninit_t{};

	HaveInstanceVariables(uninit_t){}

	HaveInstanceVariables()		
		:variables_(xnew<Array>()){
		VariablesInfo vi;
		vi.core = 0;
		vi.pos = 0;
		variables_info_.push(vi);
	}
			
	~HaveInstanceVariables(){}
		
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

	int_t find_core_inner(ClassCore* core);

	bool empty(){
		return variables_->empty();
	}

protected:
	
	struct VariablesInfo{
		ClassCore* core;
		int_t pos;
	};

	PODStack<VariablesInfo> variables_info_;
	ArrayPtr variables_;
	
	void visit_members(Visitor& m){
		m & variables_;
	}

};

class EmptyHaveInstanceVariables : public HaveInstanceVariables{
public:
	EmptyHaveInstanceVariables():HaveInstanceVariables(uninit_t()){}

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

extern EmptyHaveInstanceVariables empty_have_instance_variables;
extern uint_t global_mutate_count;

class Instance : public Base, public HaveInstanceVariables{
public:

	Instance(const ClassPtr& c);
	
	~Instance();
public:

	void set_class(const ClassPtr& c);

	AnyPtr instance_serial_save(const ClassPtr& cls);

	void instance_serial_load(const ClassPtr& cls, const AnyPtr& v);

	virtual HaveInstanceVariables* have_instance_variables(){
		return this;
	}

	virtual void visit_members(Visitor& m){
		HaveInstanceVariables::visit_members(m);
		Base::visit_members(m);
		m & get_class();
	}
};

class IdMap{
public:

	struct Node{
		InternedStringPtr key;
		AnyPtr ns;
		u16 num;
		u16 flags;
		Node* next;
		
		Node(const InternedStringPtr& key = null, const AnyPtr& ns=null, u16 num = 0xffff, u16 flags = 0)
			:key(key), ns(ns), num(num), flags(flags), next(0){}
	};
	
	friend class iterator;
	
	class iterator{
		IdMap* map_;
		uint_t pos_;
		Node* node_;
	public:
	
		iterator(IdMap* map){
			map_ = map;
			pos_ = 0;
			node_ = map_->begin_[pos_];
			
			while(!node_){
				if(pos_<map_->size_-1){
					++pos_;
					node_ = map_->begin_[pos_];
				}else{
					break;
				}
			}
		}
		
		bool is_done() const{
			return node_==0;
		}
		
		Node* operator->() const{
			return node_;
		}
		
		Node& operator*() const{
			return *node_;
		}
		
		iterator& operator++(){
			if(node_){
				node_ = node_->next;
			}
			while(!node_){
				if(pos_<map_->size_-1){
					++pos_;
					node_ = map_->begin_[pos_];
				}else{
					return *this;
				}
			}
			return *this;
		}
	};


	IdMap();

	~IdMap();
		
	Node* find(const InternedStringPtr& key, const AnyPtr& ns);

	Node* insert(const InternedStringPtr& key, const AnyPtr& ns);

	int_t size(){
		return used_size_;
	}
	
	bool empty(){
		return used_size_==0;
	}

	void visit_members(Visitor& m);
	
private:

	float_t rate(){
		return used_size_/(float_t)size_;
	}
	
	void set_node(Node* node);

	void expand(int_t addsize);
	
private:

	Node** begin_;
	uint_t size_;
	uint_t used_size_;

private:

	IdMap(const IdMap&);
	IdMap& operator = (const IdMap&);
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

	void set_class_member(int_t i, const InternedStringPtr& name, int_t accessibility, const AnyPtr& ns, const AnyPtr& value);
		
	void set_object_name(const StringPtr& name, int_t force, const AnyPtr& parent);
	
	bool is_defined_by_xtal(){
		return code_;
	}

public:

	/**
	* @brief メンバを順次取り出すIteratorを返す
	*
	* @code
	* Xfor2(key, value, frame.each_member()){
	*   puts(Xf("%s %s")(key, value).to_s().c_str());
	* }
	* @endcode
	*/
	AnyPtr each_member();

protected:

	void make_map_members();

	friend class MembersIter;

	FramePtr outer_;
	CodePtr code_;
	BlockCore* core_;
	
	ArrayPtr members_;
	IdMap* map_members_;
	
	virtual void visit_members(Visitor& m){
		HaveName::visit_members(m);
		m & outer_ & code_ & members_;
		if(map_members_){
			map_members_->visit_members(m);
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
	void def(const InternedStringPtr& name, const AnyPtr& value, int_t accessibility = KIND_PUBLIC, const AnyPtr& ns = null);

	/**
	* @brief メンバを取り出す
	* 可触性を考慮して取り出す
	*
	* @param name 取り出したいメンバの名前
	* @param self 可視性を判定するためのオブジェクト
	*/
	const AnyPtr& member(const InternedStringPtr& name, const AnyPtr& self = null, const AnyPtr& ns = null);

	/**
	* @brief メンバを再設定する
	* Xtalレベルでは禁止されている操作だが、C++レベルでは可能にしておく
	*
	* @param name 再設定したいメンバの名前
	*/
	void set_member(const InternedStringPtr& name, const AnyPtr& value, const AnyPtr& ns);

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
	bool is_inherited(const ClassPtr& md);

	/**
	* @brief Mix-inされているクラスをイテレートできるイテレータを返す
	*
	*/
	AnyPtr each_inherited_class();

	/**
	* @brief 関数を定義する
	*
	* cls.fun("name", &foo); は cls.def("name", xtal::fun(&foo)); と同一
	*/
	template<class Fun, class Policy>
	CFunPtr fun(const InternedStringPtr& name, Fun fun, const Policy& policy){
		return def_and_return(name, xtal::fun(fun, policy));
	}

	/**
	* @brief 関数を定義する
	*
	* cls.fun("name", &foo); は cls.def("name", xtal::fun(&foo)); と同一
	*/
	template<class Fun>
	CFunPtr fun(const InternedStringPtr& name, Fun f){
		return fun(name, f, result);
	}

	/**
	* @brief 関数を定義する
	*
	* cls.method("name", &foo); は cls.def("name", xtal::method(&foo)); と同一
	*/
	template<class Fun, class Policy>
	CFunPtr method(const InternedStringPtr& name, Fun fun, const Policy& policy){
		return def_and_return(name, xtal::method(fun, policy));
	}

	/**
	* @brief 関数を定義する
	*
	* cls.method("name", &foo); は cls.def("name", xtal::method(&foo)); と同一
	*/
	template<class Fun>
	CFunPtr method(const InternedStringPtr& name, Fun fun){
		return method(name, fun, result);
	}

	
	/**
	* @brief メンバ変数へのポインタからゲッターを生成し、定義する
	*
	*/
	template<class T, class U, class Policy>
	CFunPtr getter(const InternedStringPtr& name, T U::* v, const Policy& policy) const{
		return def_and_return(name, xtal::getter(v, policy));
	}
	
	/**
	* @brief メンバ変数へのポインタからセッターを生成し、定義する
	*
	* Xtalでは、obj.name = 10; とするにはset_nameとset_を前置したメソッドを定義する必要があるため、
	* 単純なセッターを定義したい場合、set_xxxとすることを忘れないこと。
	*/
	template<class T, class U, class Policy>
	CFunPtr setter(const InternedStringPtr& name, T U::* v, const Policy& policy) const{
		return def_and_return(name, xtal::setter(v, policy));
	}
	
	/**
	* @brief メンバ変数へのポインタからゲッター、セッターを両方生成し、定義する
	*
	* cls->getter(name, var, policy);
	* cls->setter(StringPtr("set_")->cat(name), v, policy);
	* と等しい	
	*/	
	template<class T, class U, class Policy>
	void var(const InternedStringPtr& name, T U::* v, const Policy& policy) const{
		getter(name, v, policy);
		setter(String("set_").cat(name), v, policy);
	}
	
	/**
	* @brief メンバ変数へのポインタからゲッターを生成し、定義する
	*
	*/
	template<class T, class U>
	CFunPtr getter(const InternedStringPtr& name, T U::* v) const{
		return getter(name, v, result);
	}
	
	/**
	* @brief メンバ変数へのポインタからセッターを生成し、定義する
	*
	*/
	template<class T, class U>
	CFunPtr setter(const InternedStringPtr& name, T U::* v) const{
		return setter(name, v, result);
	}
	
	/**
	* @brief メンバ変数へのポインタからゲッター、セッターを両方生成し、定義する
	*
	* cls->getter(name, v);
	* cls->setter(StringPtr("set_")->cat(name), v);
	* と等しい	
	*/	
	template<class T, class U>
	void var(const InternedStringPtr& name, T U::* v) const{
		var(name, v, result);
	}

public:

	virtual void call(const VMachinePtr& vm);
	
	virtual void s_new(const VMachinePtr& vm);

	void init_instance(HaveInstanceVariables* inst, const VMachinePtr& vm, const AnyPtr& self);
	
	const AnyPtr& any_member(const InternedStringPtr& name, const AnyPtr& ns);
	
	const AnyPtr& bases_member(const InternedStringPtr& name);
	
	ClassCore* core(){
		return (ClassCore*)core_;
	}
	
	struct cpp_class_t{};

	Class(cpp_class_t, const char* name = "");

protected:

	CFunPtr def_and_return(const InternedStringPtr& name, const CFunPtr& cfun){
		def(name, cfun, KIND_PUBLIC, null);
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
	
	virtual const AnyPtr& member(const InternedStringPtr& name, const AnyPtr& self, const AnyPtr& ns);

	virtual void def(const InternedStringPtr& name, const AnyPtr& value, int_t accessibility, const AnyPtr& ns);

	void append_load_path(const StringPtr& path){
		load_path_list_->push_back(path);
	}

private:

	const AnyPtr& rawdef(const InternedStringPtr& name, const AnyPtr& value, const AnyPtr& ns);

	StringPtr join_path(const StringPtr& sep);

private:

	ArrayPtr load_path_list_;
	ArrayPtr path_;

	virtual void visit_members(Visitor& m){
		Class::visit_members(m);
		m & path_ & load_path_list_;
	}
};


}
