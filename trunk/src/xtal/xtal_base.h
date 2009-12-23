/** \file src/xtal/xtal_base.h
* \brief src/xtal/xtal_base.h
*/

#ifndef XTAL_BASE_H_INCLUDE_GUARD
#define XTAL_BASE_H_INCLUDE_GUARD

#pragma once

namespace xtal{

void* xmalloc(size_t);
void xfree(void*p, size_t);
void register_gc(RefCountingBase* p);
void register_gc_observer(GCObserver* p);
void unregister_gc_observer(GCObserver* p);

/**
* \brief 参照カウンタの機能を有するクラス
*/
class RefCountingBase : public Any{
public:
	
	RefCountingBase()
		:Any(noinit_t()){}

	virtual ~RefCountingBase(){}

public:

	/**
	* \brief 関数オブジェクトとみなし、関数呼び出しをする。
	* 引数や戻り値はvmを通してやり取りする。
	*/
	virtual void rawcall(const VMachinePtr& vm);

	/**
	* \brief nameメンバを初期値valueで定義する。
	*/
	virtual void def(const IDPtr& primary_key, const AnyPtr& value, const AnyPtr& secondary_key = (const AnyPtr&)undefined, int_t accessibility = 0);

	/**
	* \brief nameメンバを取得する。
	* \retval null そのメンバは存在しない
	* \retval 非null nameに対応したメンバ  
	*/
	virtual const AnyPtr& rawmember(const IDPtr& primary_key, const AnyPtr& secondary_key, bool inherited_too, int_t& accessibility, bool& nocache);

	/**
	* \brief このオブジェクトがメンバとなっている親のクラスを返す。
	*/
	virtual const ClassPtr& object_parent();
	
	/**
	* \brief このオブジェクトに親を設定する。
	* 親を持てないオブジェクトや、前に付けられた親の方が強い場合無視される。
	* \param parent 親
	*/
	virtual void set_object_parent(const ClassPtr& parent);

	/**
	* \brief ファイナライザ
	* full_gc時に、死ぬ予定のオブジェクトとなった時に破棄をするための関数。
	* ただし、set_finalizer_flag()を呼んでいなければ呼ばれることはない。
	* 破棄時にXtalのメンバ変数を触ることがあるのであればこれを使う。
	* そうでないのであれば、デストラクタに破棄処理を書くこと。
	*/
	virtual void finalize();

	virtual void visit_members(Visitor& m){}

public:

	bool have_finalizer(){ return value_.have_finalizer(); }
	void set_finalizer_flag(){ value_.set_finalizer_flag(); }
	void unset_finalizer_flag(){ value_.unset_finalizer_flag(); }

	uint_t ref_count(){ return value_.ref_count(); }
	void add_ref_count(int_t rc){ value_.add_ref_count(rc); }
	void inc_ref_count(){ value_.inc_ref_count(); }
	void dec_ref_count(){ value_.dec_ref_count(); }

	uint_t ungc(){ return value_.ungc(); }

	void destroy(){ delete this; }

	void object_free(){ xfree(this, value_.object_size()); }

public:

	static void* operator new(size_t size){ 
		return xmalloc(size);
	}

	static void operator delete(void* p, size_t size){
		if(p){ ((RefCountingBase*)p)->value_.set_object_size(size); }
	}
	
	static void* operator new(size_t, void* p){ return p; }
	static void operator delete(void*, void*){}

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
	
	Base();

protected:

	Base(const Base& b);

	Base& operator =(const Base& b);

public:

	virtual ~Base();
			
public:

	InstanceVariables* instance_variables(){ return instance_variables_; }

	void make_instance_variables();

	void set_class(const ClassPtr& c);

	virtual void visit_members(Visitor& m);

protected:

	// インスタンス変数テーブル
	InstanceVariables* instance_variables_;

	// 所属クラス
	Class* class_;
};

}


#endif // XTAL_BASE_H_INCLUDE_GUARD
