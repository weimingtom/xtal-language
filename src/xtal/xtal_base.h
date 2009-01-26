
#pragma once

namespace xtal{

class Base : public Any{
public:

	/**
	* @brief 関数オブジェクトとみなし、関数呼び出しをする。
	*
	* 引数や戻り値はvmを通してやり取りする。
	*/
	virtual void rawcall(const VMachinePtr& vm);
	
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
	const ClassPtr& get_class(){ return *(const ClassPtr*)&class_; }

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
	virtual const AnyPtr& do_member(const IDPtr& primary_key, const AnyPtr& secondary_key, const AnyPtr& self, bool inherited_too, bool* nocache);

	/**
	* @brief このオブジェクトに付けられた名前を返す。
	*
	*/
	virtual StringPtr object_name(int_t depth = -1);
		
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
	
	Base();

protected:

	Base(const Base& b);

	Base& operator =(const Base& b);

public:

	virtual ~Base();
			
public:
	
	static void* operator new(size_t size){ 
		return so_malloc(size);
	}

	static void operator delete(void* p, size_t size){
		if(p){ ((Base*)p)->class_ = Innocence((int_t)size); }
	}
	
	static void* operator new(size_t, void* p){ return p; }
	static void operator delete(void*, void*){}

public:

	enum{
		XTAL_INSTANCE_FLAG_SHIFT = TYPE_SHIFT+1,
		XTAL_INSTANCE_FLAG_BIT = 1<<(XTAL_INSTANCE_FLAG_SHIFT),
		REF_COUNT_SHIFT = XTAL_INSTANCE_FLAG_SHIFT+1,
		REF_COUNT_MASK = ~((1<<XTAL_INSTANCE_FLAG_SHIFT)-1)
	};

	InstanceVariables* instance_variables(){ return instance_variables_; }
	void make_instance_variables();

	uint_t ref_count(){ return (type_ & REF_COUNT_MASK)>>REF_COUNT_SHIFT; }
	void add_ref_count(int_t rc){ type_ += rc<<REF_COUNT_SHIFT; }
	void inc_ref_count(){ type_ += 1<<REF_COUNT_SHIFT; }
	void dec_ref_count(){ type_ -= 1<<REF_COUNT_SHIFT; }

	void set_class(const ClassPtr& c);

	virtual void visit_members(Visitor& m);

	bool is_xtal_instance(){ return (type_ & XTAL_INSTANCE_FLAG_BIT)!=0; }
	void set_xtal_instance_flag(){ type_ |= XTAL_INSTANCE_FLAG_BIT; }
	
private:

	// 参照カウンタ値
	//uint_t ref_count_;

	// インスタンス変数テーブル
	InstanceVariables* instance_variables_;

	// 所属クラス
	Innocence class_;
	
private:

	friend void gc();
	friend void full_gc();
	friend void initialize();
};

class GCObserver : public Base{
public:
	GCObserver();
	GCObserver(const GCObserver& v);
	virtual ~GCObserver();
	virtual void before_gc(){}
	virtual void after_gc(){}
};

}

