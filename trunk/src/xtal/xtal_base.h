
#pragma once

namespace xtal{

class Base{
public:

	/**
	* @brief 関数オブジェクトとみなし、関数呼び出しをする。
	*
	* 引数や戻り値はvmを通してやり取りする。
	*/
	virtual void call(const VMachinePtr& vm);
	
	/**
	* @brief nameメンバを取得する。
	* 可触性を考慮したメンバ取得
	*
	* @retval undefined そのメンバは存在しない
	* @retval 非undefined nameに対応したメンバ  
	*/
	const AnyPtr& member(const IDPtr& primary_key, const AnyPtr& secondary_key = (const AnyPtr& )null, const AnyPtr& self = (const AnyPtr& )null, bool inherited_too = true);

	/**
	* @brief nameメソッド呼び出しをする
	*
	* 引数や戻り値はvmを通してやり取りする。
	*/
	void rawsend(const VMachinePtr& vm, const IDPtr& primary_key, const AnyPtr& secondary_key = (const AnyPtr& )null, const AnyPtr& self = (const AnyPtr& )null, bool inherited_too = true);

	/**
	* @brief nameメンバを初期値valueで定義する。
	*
	*/
	virtual void def(const IDPtr& primary_key, const AnyPtr& value, const AnyPtr& secondary_key = (const AnyPtr&)null, int_t accessibility = 0);

	/**
	* @brief このオブジェクトが所属するクラスを返す。
	*
	*/
	const ClassPtr& get_class(){ return (const ClassPtr&)class_; }

	/**
	* @brief 整数に変換して返す。
	*
	*/
	int_t to_i();
	
	/**
	* @brief 浮動小数点数に変換して返す。
	*
	*/
	float_t to_f();
	
	/**
	* @brief 文字列に変換して返す。
	*
	*/
	StringPtr to_s();

	/**
	* @brief 配列に変換して返す。
	*
	*/
	ArrayPtr to_a();

	/**
	* @brief 連想配列に変換して返す。
	*
	*/
	MapPtr to_m();

	/**
	* @brief clsクラスのインスタンスか調べる。
	*
	*/
	bool is(const AnyPtr& klass);

	/**
	* @brief nameメンバを取得する。
	* 可触性を考慮したメンバ取得
	*
	* @retval null そのメンバは存在しない
	* @retval 非null nameに対応したメンバ  
	*/
	virtual const AnyPtr& do_member(const IDPtr& primary_key, const AnyPtr& secondary_key = (const AnyPtr&)null, const AnyPtr& self = (const AnyPtr&)null, bool inherited_too = true);

	/**
	* @brief このオブジェクトに付けられた名前を返す。
	*
	*/
	virtual StringPtr object_name();
		
	/**
	* @brief このオブジェクトに付けられた名前の強さを返す。
	*
	*/
	virtual int_t object_name_force();
	
	/**
	* @brief このオブジェクトに名前をつける。
	*
	* 名前を持てないオブジェクトや、前に付けられた名前の方が強い場合無視される。
	* @param name つける名前
	* @param force 名前の強さ
	* @param parent 親
	*/
	virtual void set_object_name(const StringPtr& name, int_t force, const AnyPtr& parent);

	/**
	* @brief ハッシュ値を返す
	*
	*/
	virtual uint_t hashcode();

	/**
	* @brief 自身を文字列化してprintlnする。
	* @return 自身を返す。
	*/
	AnyPtr p();

public:

	SendProxy send(const IDPtr& primary_key, const AnyPtr& secondary_key = (const AnyPtr&)null);

public:
	
	Base();

	Base(const Base& b);

	Base& operator =(const Base& b);

	virtual ~Base();
			
public:
	
	static void* operator new(size_t size){ return user_malloc(size); }
	static void operator delete(void* p){}

	static void* operator new(size_t size, size_t add){ return user_malloc(size+add); }
	static void operator delete(void* p, size_t add){}
	
	static void* operator new(size_t, void* p){ return p; }
	static void operator delete(void*, void*){}

public:

	enum{
		XTAL_INSTANCE_FLAG_BIT = 1 << (sizeof(uint_t)*8-1),
		REF_COUNT_MASK = ~(XTAL_INSTANCE_FLAG_BIT)
	};

	InstanceVariables* instance_variables(){ return instance_variables_; }
	void make_instance_variables();

	uint_t ref_count(){ return ref_count_ & REF_COUNT_MASK; }
	void add_ref_count(int_t rc){ ref_count_+=rc; }
	void inc_ref_count(){ ++ref_count_; }
	void dec_ref_count(){ --ref_count_; }

	void set_class(const ClassPtr& c);

	virtual void visit_members(Visitor& m);

	bool is_xtal_instance(){ return (ref_count_ & XTAL_INSTANCE_FLAG_BIT)!=0; }
	void set_xtal_instance_flag(){ ref_count_ |= XTAL_INSTANCE_FLAG_BIT; }
	
private:

	// 参照カウンタ値
	uint_t ref_count_;

	// インスタンス変数テーブル
	InstanceVariables* instance_variables_;

	// 所属クラス
	Innocence class_;
	
private:

	friend void gc();
	friend void full_gc();
	friend void initialize();
};

inline void Innocence::inc_ref_count(){
	if(type(*this)==TYPE_BASE){
		pvalue(*this)->inc_ref_count();
	}
}

inline void Innocence::dec_ref_count(){
	if(type(*this)==TYPE_BASE){
		pvalue(*this)->dec_ref_count();
	}
}

class GCObserver : public Base{
public:
	GCObserver();
	GCObserver(const GCObserver& v);
	virtual ~GCObserver();
	virtual void before_gc(){}
	virtual void after_gc(){}
};

}

