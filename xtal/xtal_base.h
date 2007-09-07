
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
	* @retval null そのメンバは存在しない
	* @retval 非null nameに対応したメンバ  
	*/
	const AnyPtr& member(const InternedStringPtr& name, const AnyPtr& self = (const AnyPtr& )null, const AnyPtr& ns = (const AnyPtr& )null);

	/**
	* @brief nameメソッド呼び出しをする
	*
	* 引数や戻り値はvmを通してやり取りする。
	*/
	void rawsend(const VMachinePtr& vm, const InternedStringPtr& name, const AnyPtr& self = (const AnyPtr& )null, const AnyPtr& ns = (const AnyPtr& )null);

	/**
	* @brief nameメンバを初期値valueで定義する。
	*
	*/
	virtual void def(const InternedStringPtr& name, const AnyPtr& value, int_t accessibility = 0, const AnyPtr& ns = (const AnyPtr&)null);

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
	* @brief clsクラスのインスタンスか調べる。
	*
	*/
	bool is(const ClassPtr& klass);

	/**
	* @brief nameメンバを取得する。
	* 可触性を考慮したメンバ取得
	*
	* @retval null そのメンバは存在しない
	* @retval 非null nameに対応したメンバ  
	*/
	virtual const AnyPtr& do_member(const InternedStringPtr& name, const AnyPtr& self = (const AnyPtr& )null, const AnyPtr& ns = (const AnyPtr& )null);

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

	AnyPtr send(const InternedStringPtr& name);

	template<class A0>
	AnyPtr send(const InternedStringPtr& name, const A0& a0);

	template<class A0, class A1>
	AnyPtr send(const InternedStringPtr& name, const A0& a0, const A1& a1);

	template<class A0, class A1, class A2>
	AnyPtr send(const InternedStringPtr& name, const A0& a0, const A1& a1, const A2& a2);

	template<class A0, class A1, class A2, class A3>
	AnyPtr send(const InternedStringPtr& name, const A0& a0, const A1& a1, const A2& a2, const A3& a3);

	template<class A0, class A1, class A2, class A3, class A4>
	AnyPtr send(const InternedStringPtr& name, const A0& a0, const A1& a1, const A2& a2, const A3& a3, const A4& a4);

public:
	
	Base();

	Base(const Base& b);

	Base& operator =(const Base& b);

	virtual ~Base();
			
public:
	
	static void* operator new(size_t size);
	static void operator delete(void* p);
	
	static void* operator new(size_t, void* p){ return p; }
	static void operator delete(void*, void*){}

public:

	InstanceVariables* instance_variables(){ return instance_variables_; }
	void make_instance_variables();

	uint_t ref_count(){ return ref_count_; }
	void add_ref_count(int_t rc){ ref_count_+=rc; }
	void inc_ref_count(){ ++ref_count_; }
	void dec_ref_count(){ --ref_count_; }
	void set_ref_count(uint_t rc){ ref_count_ = rc; }

	void set_class(const ClassPtr& c);

	virtual void visit_members(Visitor& m);
	
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

