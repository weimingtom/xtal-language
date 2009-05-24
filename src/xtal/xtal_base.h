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

	enum{
		HAVE_FINALIZER_FLAG_SHIFT = TYPE_SHIFT+1,
		HAVE_FINALIZER_FLAG_BIT = 1<<HAVE_FINALIZER_FLAG_SHIFT,

		GENE_COUNT_BIT_SHIFT = HAVE_FINALIZER_FLAG_SHIFT+1,
		GENE_COUNT_BIT_MASK = (1<<GENE_COUNT_BIT_SHIFT) | (1<<(GENE_COUNT_BIT_SHIFT+1)),

		REF_COUNT_SHIFT = GENE_COUNT_BIT_SHIFT+2,
		REF_COUNT_MASK = ~((1<<REF_COUNT_SHIFT)-1)
	};

	bool have_finalizer(){ return (type_ & HAVE_FINALIZER_FLAG_BIT)!=0; }
	void set_finalizer_flag(){ type_ |= HAVE_FINALIZER_FLAG_BIT; }

	uint_t ref_count(){ return (type_ & REF_COUNT_MASK)>>REF_COUNT_SHIFT; }
	void add_ref_count(int_t rc){ type_ += rc<<REF_COUNT_SHIFT; }
	void inc_ref_count(){ type_ += 1<<REF_COUNT_SHIFT; }
	void dec_ref_count(){ type_ -= 1<<REF_COUNT_SHIFT; }

	uint_t ungc(){ return type_&(REF_COUNT_MASK|HAVE_FINALIZER_FLAG_BIT); }

	uint_t gene_count(){ return type_&GENE_COUNT_BIT_MASK; }
	void inc_gene_count(){ if(gene_count()!=GENE_COUNT_BIT_MASK)type_+=1<<GENE_COUNT_BIT_SHIFT; }

	void destroy(){ delete this; }

	void object_free(){ xfree(this, value_.uvalue); }

public:

	static void* operator new(size_t size){ 
		return xmalloc(size);
	}

	static void operator delete(void* p, size_t size){
		if(p){ ((RefCountingBase*)p)->value_.ivalue = (int_t)size; }
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

	// 参照カウンタ値
	//uint_t ref_count_;

	// インスタンス変数テーブル
	InstanceVariables* instance_variables_;

	// 所属クラス
	Class* class_;
};

}


#endif // XTAL_BASE_H_INCLUDE_GUARD
