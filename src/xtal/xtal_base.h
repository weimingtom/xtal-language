/** \file src/xtal/xtal_base.h
* \brief src/xtal/xtal_base.h
*/

#ifndef XTAL_BASE_H_INCLUDE_GUARD
#define XTAL_BASE_H_INCLUDE_GUARD

#pragma once

namespace xtal{

void* xmalloc(size_t);
void xfree(void*p, size_t);
void* xmalloc_align(size_t, size_t);
void xfree_align(void*p, size_t, size_t);

enum{
	ALIGN_MIN = 8
};

template<int Size, int Align>
struct ObjectXMalloc{
	static void* xm(){ return xmalloc_align(Size, Align); }
	static void xf(void* p){ xfree_align(p, Size, Align); }
};

template<int Size>
struct ObjectXMalloc<Size, 0>{
	static void* xm(){ return xmalloc(Size); }
	static void xf(void* p){ xfree(p, Size); }
};

template<class T>
inline T* object_xmalloc(){
	return (T*)ObjectXMalloc<sizeof(T), ((int)AlignOf<T>::value<=(int)ALIGN_MIN) ? 0 : AlignOf<T>::value>::xm();
}

template<class T>
inline void object_xfree(T* p){
	ObjectXMalloc<sizeof(T), ((int)AlignOf<T>::value<=(int)ALIGN_MIN) ? 0 : AlignOf<T>::value>::xf(p);
}

template<int N, class T>
struct RCBaseClassType{
	enum{ value = 0 };
};

template<class T>
struct RCBaseClassType<INHERITED_RCBASE, T>{
	enum{ value = T::TYPE };
};

struct VirtualMembers{
	CppClassSymbolData* cpp_class_symbol_data;
	u8 rcbase_class_type;
	u8 object_align;
	u16 object_size;

	void (*object_free)(RefCountingBase*);
	void (*destroy)(RefCountingBase*);
	void (*visit_members)(RefCountingBase* p, Visitor& m);
	
	void (*rawcall)(RefCountingBase* p, const VMachinePtr& vm);
	const AnyPtr& (*rawmember)(RefCountingBase* p, const IDPtr& primary_key, const AnyPtr& secondary_key, bool inherited_too, int_t& accessibility, bool& nocache);
	void (*def)(RefCountingBase* p, const IDPtr& primary_key, const AnyPtr& value, const AnyPtr& secondary_key, int_t accessibility);
	const ClassPtr& (*object_parent)(RefCountingBase* p);
	void (*set_object_parent)(RefCountingBase* p, const ClassPtr& parent);	
	void (*finalize)(RefCountingBase* p);
	void (*shrink_to_fit)(RefCountingBase* p);
};

struct VirtualMembersN{	
	static void visit_members(RefCountingBase* p, Visitor& m);
	static void rawcall(RefCountingBase* p, const VMachinePtr& vm);
	static const AnyPtr& rawmember(RefCountingBase* p, const IDPtr& primary_key, const AnyPtr& secondary_key, bool inherited_too, int_t& accessibility, bool& nocache);
	static void def(RefCountingBase* p, const IDPtr& primary_key, const AnyPtr& value, const AnyPtr& secondary_key, int_t accessibility);
	static const ClassPtr& object_parent(RefCountingBase* p);
	static void set_object_parent(RefCountingBase* p, const ClassPtr& parent);
	static void finalize(RefCountingBase* p);
	static void shrink_to_fit(RefCountingBase* p);
};

template<class T>
struct VirtualMembersM{
	static T* cast(RefCountingBase* p){ return static_cast<T*>(p); }
	
	static void object_free(RefCountingBase* p){ object_xfree<T>(cast(p)); }
	static void destroy(RefCountingBase* p){ cast(p)->~T(); }
	static void visit_members(RefCountingBase* p, Visitor& m){ cast(p)->on_visit_members(m); }
	
	static void rawcall(RefCountingBase* p, const VMachinePtr& vm){ cast(p)->on_rawcall(vm); }
	static const AnyPtr& rawmember(RefCountingBase* p, const IDPtr& primary_key, const AnyPtr& secondary_key, bool inherited_too, int_t& accessibility, bool& nocache){ return cast(p)->on_rawmember(primary_key, secondary_key, inherited_too, accessibility, nocache); }
	static void def(RefCountingBase* p, const IDPtr& primary_key, const AnyPtr& value, const AnyPtr& secondary_key, int_t accessibility){ cast(p)->on_def(primary_key, value, secondary_key, accessibility); }
	static const ClassPtr& object_parent(RefCountingBase* p){ return cast(p)->on_object_parent(); }
	static void set_object_parent(RefCountingBase* p, const ClassPtr& parent){ cast(p)->on_set_object_parent(parent); }
	static void finalize(RefCountingBase* p){ cast(p)->on_finalize(); }
	static void shrink_to_fit(RefCountingBase* p){ cast(p)->on_shrink_to_fit(); }
};

template<class T>
struct VirtualMembersOverwriteChecker{
	typedef char (&yes)[2];
	typedef char (&no)[1];

	static no rawcall(void (RefCountingBase::*)(const VMachinePtr&));
	template<class U> static yes rawcall(void (U::*)(const VMachinePtr&));

	static no rawmember(const AnyPtr& (RefCountingBase::*)(const IDPtr&, const AnyPtr&, bool, int_t&, bool&));
	template<class U> static yes rawmember(const AnyPtr& (U::*)(const IDPtr&, const AnyPtr&, bool, int_t&, bool&));

	static no def(void (RefCountingBase::*)(const IDPtr&, const AnyPtr&, const AnyPtr&, int_t));
	template<class U> static yes def(void (U::*)(const IDPtr&, const AnyPtr&, const AnyPtr&, int_t));

	static no object_parent(const ClassPtr& (RefCountingBase::*)());
	template<class U> static yes object_parent(const ClassPtr& (U::*)());

	static no set_object_parent(void (RefCountingBase::*)(const ClassPtr&));
	template<class U> static yes set_object_parent(void (U::*)(const ClassPtr&));

	static no visit_members(void (RefCountingBase::*)(Visitor&));
	template<class U> static yes visit_members(void (U::*)(Visitor&));

	static no finalize(void (RefCountingBase::*)());
	template<class U> static yes finalize(void (U::*)());

	static no shrink_to_fit(void (RefCountingBase::*)());
	template<class U> static yes shrink_to_fit(void (U::*)());

	enum{
		rawcall_overwrite = sizeof(rawcall(&T::on_rawcall))!=sizeof(no),
		rawmember_overwrite = sizeof(rawmember(&T::on_rawmember))!=sizeof(no),
		def_overwrite = sizeof(def(&T::on_def))!=sizeof(no),
		object_parent_overwrite = sizeof(object_parent(&T::on_object_parent))!=sizeof(no),
		set_object_parent_overwrite = sizeof(set_object_parent(&T::on_set_object_parent))!=sizeof(no),
		visit_members_overwrite = sizeof(visit_members(&T::on_visit_members))!=sizeof(no),
		finaize_overwrite = sizeof(finalize(&T::on_finalize))!=sizeof(no),
		shrink_to_fit_overwrite = sizeof(shrink_to_fit(&T::on_shrink_to_fit))!=sizeof(no)
	};

/*
	enum{
		value = 1,
		rawcall_overwrite = value,
		rawmember_overwrite = value,
		def_overwrite = value,
		object_parent_overwrite = value,
		set_object_parent_overwrite = value,
		visit_members_overwrite = value,
		finaize_overwrite = value
	};
*/	
};

template<class T>
struct VirtualMembersT{
	static const VirtualMembers value;

	typedef VirtualMembersOverwriteChecker<T> checker;

	typedef typename If<checker::rawcall_overwrite, VirtualMembersM<T>, VirtualMembersN>::type rawcall;
	typedef typename If<checker::rawmember_overwrite, VirtualMembersM<T>, VirtualMembersN>::type rawmember;
	typedef typename If<checker::def_overwrite, VirtualMembersM<T>, VirtualMembersN>::type def;
	typedef typename If<checker::object_parent_overwrite, VirtualMembersM<T>, VirtualMembersN>::type object_parent;
	typedef typename If<checker::set_object_parent_overwrite, VirtualMembersM<T>, VirtualMembersN>::type set_object_parent;
	typedef typename If<checker::visit_members_overwrite, VirtualMembersM<T>, VirtualMembersN>::type visit_members;
	typedef typename If<checker::finaize_overwrite, VirtualMembersM<T>, VirtualMembersN>::type finalize;
	typedef typename If<checker::shrink_to_fit_overwrite, VirtualMembersM<T>, VirtualMembersN>::type shrink_to_fit;
};

template<class T>
const VirtualMembers VirtualMembersT<T>::value = {
	&CppClassSymbol<T>::value,
	(u8)RCBaseClassType<InheritedN<T>::value, T>::value,
	(u8)AlignOf<T>::value,
	(u16)sizeof(T),

	&VirtualMembersM<T>::object_free,
	&VirtualMembersM<T>::destroy,
	&visit_members::visit_members,
	
	&rawcall::rawcall,
	&rawmember::rawmember,
	&def::def,
	&object_parent::object_parent,
	&set_object_parent::set_object_parent,
	&finalize::finalize,
	&shrink_to_fit::shrink_to_fit,
};

/**
* \brief 参照カウンタの機能を有するクラス
*/
class RefCountingBase : public Any{
public:

	/**
	* \brief 関数オブジェクトとみなし、関数呼び出しをする。
	* 引数や戻り値はvmを通してやり取りする。
	*/
	void rawcall(const VMachinePtr& vm){
		virtual_members()->rawcall(this, vm);
	}

	/**
	* \brief nameメンバを初期値valueで定義する。
	*/
	void def(const IDPtr& primary_key, const AnyPtr& value, const AnyPtr& secondary_key, int_t accessibility){
		virtual_members()->def(this, primary_key, value, secondary_key, accessibility);
	}

	/**
	* \brief nameメンバを取得する。
	* \retval undefined そのメンバは存在しない
	* \retval 非undefined nameに対応したメンバ  
	*/
	const AnyPtr& rawmember(const IDPtr& primary_key, const AnyPtr& secondary_key, bool inherited_too, int_t& accessibility, bool& nocache){
		return virtual_members()->rawmember(this, primary_key, secondary_key, inherited_too, accessibility, nocache);
	}

	/**
	* \brief このオブジェクトがメンバとなっている親のクラスを返す。
	*/
	const ClassPtr& object_parent(){
		return virtual_members()->object_parent(this);
	}
	
	/**
	* \brief このオブジェクトに親を設定する。
	* 親を持てないオブジェクトや、前に付けられた親の方が強い場合無視される。
	* \param parent 親
	*/
	void set_object_parent(const ClassPtr& parent){
		virtual_members()->set_object_parent(this, parent);
	}

public:

	void finalize(){
		virtual_members()->finalize(this);
	}

	void visit_members(Visitor& m){
		virtual_members()->visit_members(this, m);
	}

	void shrink_to_fit(){
		virtual_members()->shrink_to_fit(this);
	}

public:

	void on_rawcall(const VMachinePtr& vm);
	void on_def(const IDPtr& primary_key, const AnyPtr& value, const AnyPtr& secondary_key, int_t accessibility){}
	const AnyPtr& on_rawmember(const IDPtr& primary_key, const AnyPtr& secondary_key, bool inherited_too, int_t& accessibility, bool& nocache){ return *(AnyPtr*)&undefined; }
	const ClassPtr& on_object_parent(){ return *(ClassPtr*)&null; }
	void on_set_object_parent(const ClassPtr& parent){}
	void on_finalize(){}
	void on_visit_members(Visitor& m){}
	void on_shrink_to_fit(){}

public:
	RefCountingBase(){}

	bool have_finalizer(){ return value_.have_finalizer(); }
	void set_finalizer_flag(){ value_.set_finalizer_flag(); }
	void unset_finalizer_flag(){ value_.unset_finalizer_flag(); }

	uint_t ref_count(){ return value_.ref_count(); }
	void add_ref_count(int_t rc){ value_.add_ref_count(rc); }
	void inc_ref_count(){ value_.inc_ref_count(); }
	void dec_ref_count(){ value_.dec_ref_count(); }

	uint_t can_not_gc(){ return value_.can_not_gc(); }

	void destroy();

	void object_free(){ 
		virtual_members()->object_free(this); 
	}

public:

	enum{
		BUILD = 0
	};

	void special_initialize(const VirtualMembers* vmembers);

	const VirtualMembers* virtual_members() const{
		return vmembers_;
	}

protected:
	const VirtualMembers* vmembers_;
	
private:

#ifdef XTAL_DEBUG
	virtual void virtual_function(){}
#endif

private:
	XTAL_DISALLOW_COPY_AND_ASSIGN(RefCountingBase);

};

/**
* @brief 基底クラス
*/
class Base : public RefCountingBase{
public:

	/**
	* \brief このオブジェクトが所属するクラスを返す。
	*/
	const ClassPtr& get_class();

public:
	
	Base(){}
			
public:

	void special_initialize();
	void special_initialize(const VirtualMembers* vmembers);
	void special_uninitialize();

	InstanceVariables* instance_variables(){ return instance_variables_; }

	void make_instance_variables();

	void set_class(const ClassPtr& c);

	void on_visit_members(Visitor& m);
	
private:

	// インスタンス変数テーブル
	InstanceVariables* instance_variables_;

	// 所属クラス
	Class* class_;

private:
	XTAL_DISALLOW_COPY_AND_ASSIGN(Base);
};

}


#endif // XTAL_BASE_H_INCLUDE_GUARD
