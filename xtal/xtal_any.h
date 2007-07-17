
#pragma once

#include "xtal_utility.h"
#include "xtal_fwd.h"

namespace xtal{


#ifdef XTAL_USE_COMPRESSED_ANY

/**
* @brief 参照カウントをしないAny
*
* 一般ユーザーはこれを直接使用することは無い
*/
class UncountedAny{
public:

	UncountedAny(){ value_ = TYPE_NULL; }
	UncountedAny(int_t v){ set_i(v); }
	UncountedAny(float_t v){ set_f(v); }
	UncountedAny(AnyImpl* v){ set_p(v); }
	UncountedAny(bool b){ set_b(b); }
	UncountedAny(check_xtype<int>::type v){ set_i(v); }
	UncountedAny(check_xtype<float>::type v){ set_f(v); }
	UncountedAny(PrimitiveType type){ value_ = type; }
	
	struct noinit_t{};
	UncountedAny(noinit_t){}

public:

	void set_null(){
		value_ = TYPE_NULL;
	}

protected:

	void set_p(AnyImpl* p){
		XTAL_ASSERT(p!=0);
		union{
			int_t value;
			AnyImpl* pvalue;
		} u;
		u.pvalue = p;
		u.value |= TYPE_BASE;
		value_ = u.value;
	}
	
	void set_p(const AnyImpl* p){
		set_p((AnyImpl*)p);
	}

	void set_i(int_t v){
		value_ = (v<<TYPE_SHIFT) | TYPE_INT;
	}

	void set_f(float_t v){
		union{
			int_t value;
			float_t fvalue;
		} u;
		u.fvalue = v;
		u.value = (u.value & ~TYPE_MASK) | TYPE_FLOAT;
		value_ = u.value;
	}

	void set_b(bool b){
		value_ = TYPE_FALSE + (int)b;
	}

public:

	const Any& cref() const{
		return (const Any&)*this;
	}

	int_t type() const{
		return value_ & TYPE_MASK; 
	}
	
	int_t ivalue() const{ 
		XTAL_ASSERT(type()==TYPE_INT);
		return value_ >> TYPE_SHIFT; 
	}
	
	float_t fvalue() const{ 
		XTAL_ASSERT(type()==TYPE_FLOAT);
		union{
			int_t value;
			float_t fvalue;
		} u;
		u.value = (value_ & ~TYPE_MASK);
		return u.fvalue;
	}
	
	AnyImpl* impl() const{
		XTAL_ASSERT(type()==TYPE_BASE || type()==TYPE_NULL);
		union{
			int_t value;
			AnyImpl* pvalue;
		} u;
		u.value = (value_ & ~TYPE_MASK);
		return u.pvalue; 
	}

	int_t rawvalue() const{
		return value_;
	}

public:

	bool raweq(const UncountedAny& a) const{
		return value_==a.value_;
	}

	bool rawne(const UncountedAny& a) const{
		return !raweq(a);
	}
	
	bool rawlt(const UncountedAny& a) const{
		return value_<a.value_;
	}

	void swap(UncountedAny& a){
		std::swap(value_, a.value_);
	}

private:

	union{
		int_t value_;
		float_t fvalue_;
		AnyImpl* pvalue_;
	};
	
};
	
#else
	
/**
* @brief 参照カウントをしないAny
*
* 一般ユーザーはこれを直接使用することは無い
*/
class UncountedAny{
public:

	UncountedAny(){ type_ = TYPE_NULL; pvalue_ = 0; }
	UncountedAny(u8 v){ set_i(v); }
	UncountedAny(u16 v){ set_i(v); }
	UncountedAny(i8 v){ set_i(v); }
	UncountedAny(i16 v){ set_i(v); }
	UncountedAny(int_t v){ set_i(v); }
	UncountedAny(float_t v){ set_f(v); }
	UncountedAny(AnyImpl* v){ set_p(v); }
	UncountedAny(bool b){ set_b(b); }
	UncountedAny(check_xtype<int>::type v){ set_i(v); }
	UncountedAny(check_xtype<float>::type v){ set_f(v); }
	
	UncountedAny(PrimitiveType type){
		type_ = type;
		value_ = 0;
	}

	struct noinit_t{};
	UncountedAny(noinit_t){}
	
public:

	void set_null(){
		type_ = TYPE_NULL;
		pvalue_ = 0;
	}

protected:

	void set_p(AnyImpl* p){
		XTAL_ASSERT(p!=0);
		type_ = TYPE_BASE;
		pvalue_ = p;
	}
	
	void set_p(const AnyImpl* p){
		XTAL_ASSERT(p!=0);
		type_ = TYPE_BASE;
		pvalue_ = (AnyImpl*)p;
	}

	void set_i(int_t v){
		type_ = TYPE_INT;
		value_ = v;
	}

	void set_f(float_t v){
		type_ = TYPE_FLOAT;
		fvalue_ = v;
	}

	void set_b(bool b){
		type_ = TYPE_FALSE + (int)b;
		value_ = 0;
	}

public:

	const Any& cref() const{
		return (const Any&)*this;
	}

	int_t type() const{ 
		return type_; 
	}
	
	int_t ivalue() const{ 
		XTAL_ASSERT(type()==TYPE_INT);
		return value_; 
	}
	
	float_t fvalue() const{ 
		XTAL_ASSERT(type()==TYPE_FLOAT); 
		return fvalue_; 
	}
	
	AnyImpl* impl() const { 
		XTAL_ASSERT(type()==TYPE_BASE || type()==TYPE_NULL); 
		return pvalue_; 
	}

	uint_t rawvalue() const{
		return (uint_t)value_;
	}
	
public:

	bool raweq(const UncountedAny& a) const{
		return type_==a.type_ && value_==a.value_;
	}

	bool rawne(const UncountedAny& a) const{
		return !raweq(a);
	}
	
	bool rawlt(const UncountedAny& a) const{
		if(type_<a.type_)
			return true;
		if(a.type_<type_)
			return false;
		return value_<a.value_;
	}

	void swap(UncountedAny& a){
		std::swap(type_, a.type_);
		std::swap(value_, a.value_);
	}

private:

	int_t type_;
	
	union{
		int_t value_;
		float_t fvalue_;
		AnyImpl* pvalue_;
	};
	
};
	
#endif
	
/**
* @brief Xtalの値を保持するための型
*
*/
class Any : public UncountedAny{	
public:

	/**
	* @brief デフォルトコンストラクタ。nullが格納される。
	*
	*/
	Any(){ set_null(); }
	
	/**
	* @brief 整数値から構築するコンストラクタ。
	*
	*/
	Any(int_t v){ set_i(v); }
	
	/**
	* @brief 浮動小数点数値から構築するコンストラクタ。
	*
	*/
	Any(float_t v){ set_f(v); }
	
	/**
	* @brief 文字列から構築するコンストラクタ。
	*
	*/
	Any(bool b){ set_b(b); }

	/**
	* @brief 文字列から構築するコンストラクタ。
	*
	*/
	Any(const char* str);

	// 基本型の整数、浮動小数点数から構築するコンストラクタ
	Any(check_xtype<int>::type v){ set_i((int_t)v); }
	Any(check_xtype<long>::type v){ set_i((int_t)v); }
	Any(check_xtype<short>::type v){ set_i((int_t)v); }
	Any(check_xtype<char>::type v){ set_i((int_t)v); }
	Any(check_xtype<unsigned int>::type v){ set_i((int_t)v); }
	Any(check_xtype<unsigned long>::type v){ set_i((int_t)v); }
	Any(check_xtype<unsigned short>::type v){ set_i((int_t)v); }
	Any(check_xtype<unsigned char>::type v){ set_i((int_t)v); }
	Any(check_xtype<signed char>::type v){ set_i((int_t)v); }
	Any(check_xtype<float>::type v){ set_f((float_t)v); }
	Any(check_xtype<double>::type v){ set_f((float_t)v); }
	Any(check_xtype<long double>::type v){ set_f((float_t)v); }
	
public:

	Any(AnyImpl* v);
	
	Any(const AnyImpl* v);

	Any(const Any& v):UncountedAny(v){
		inc_ref_count();
	}

	Any& operator =(const Any& v);

	Any& operator =(AnyImpl* v);
	
	Any& operator =(const AnyImpl* v);

	Any& operator =(int_t v);
	
	Any& operator =(float_t v);

	Any& operator =(bool v);

	Any& operator =(const Null&);

	~Any(){ 
		dec_ref_count();
		set_null(); // 必須
	}
	
private:

	// AnyImplを継承していないポインタ型が渡されたとき、
	// Any(bool)が使われるのを阻止するためのコンストラクタ
	// 実装は無い。
	Any(void*);
	
	using UncountedAny::set_null;

public:
	
	/**
	* @brief プリミティブな型番号を用いてAnyを構築する。
	*
	* 一般ユーザーはこれを参照する必要はほとんどない。
	*/
	Any(PrimitiveType type):UncountedAny(type){}

	/**
	* @brief プリミティブな型番号を返す。
	*
	* 一般ユーザーはこれを参照する必要はほとんどない。
	*/
	int_t type() const{ return UncountedAny::type(); }
	
	/**
	* @brief プリミティブな整数値を返す。
	*
	* 一般ユーザーはこれより、to_i()を使う方がよい。
	*/
	int_t ivalue() const{ return UncountedAny::ivalue(); }
	
	/**
	* @brief プリミティブな浮動小数点数値を返す。
	*
	* 一般ユーザーはこれより、to_f()を使う方がよい。
	*/
	float_t fvalue() const{ return UncountedAny::fvalue(); }
	
	/**
	* @brief プリミティブなオブジェクトのポインタ値を返す。
	*
	* 一般ユーザーはこれを参照する必要はほとんどない。
	*/
	AnyImpl* impl() const{ return UncountedAny::impl(); }
	
public:
	
	/**
	* @brief 関数オブジェクトとみなし、関数呼び出しをする。
	*
	* 引数や戻り値はvmを通してやり取りする。
	*/
	void call(const VMachine& vm) const;
	
	/**
	* @brief nameメソッド呼び出しをする
	*
	* 引数や戻り値はvmを通してやり取りする。
	*/
	void send(const ID& name, const VMachine& vm) const;

	/**
	* @brief nameメンバを取得する。
	*
	* @retval null そのメンバは存在しない
	* @retval 非null nameに対応したメンバ  
	*/
	const Any& member(const ID& name) const;
	
	/**
	* @brief nameメンバを取得する。
	* 可触性を考慮したメンバ取得
	*
	* @retval null そのメンバは存在しない
	* @retval 非null nameに対応したメンバ  
	*/
	const Any& member(const ID& name, const Any& self) const;

	/**
	* @brief nameメンバを取得する。
	* 可触性を考慮したメンバ取得
	*
	* @retval null そのメンバは存在しない
	* @retval 非null nameに対応したメンバ  
	*/
	const Any& member(const ID& name, const Any& self, const Any& ns) const;

	/**
	* @brief nameメンバを初期値valueで定義する。
	*
	*/
	void def(const ID& name, const Any& value, int_t accessibility = 0, const Any& ns = (const Any&)null) const;

	/**
	* @brief このオブジェクトが所属するクラスを返す。
	*
	*/
	const Class& get_class() const;

	/**
	* @brief 整数に変換して返す。
	*
	*/
	int_t to_i() const;
	
	/**
	* @brief 浮動小数点数に変換して返す。
	*
	*/
	float_t to_f() const;
	
	/**
	* @brief 文字列に変換して返す。
	*
	*/
	String to_s() const;
	
	/**
	* @brief 真偽値に変換して返す。
	*
	*/
	bool to_b() const{
		return type()>TYPE_FALSE;
	}
	
	/**
	* @brief このオブジェクトに付けられた名前を返す。
	*
	*/
	String object_name() const;

	/**
	* @brief clsクラスのインスタンスか調べる。
	*
	*/
	bool is(const Class& cls) const;
		
	/**
	* @brief このオブジェクトに付けられた名前の強さを返す。
	*
	*/
	int_t object_name_force() const;
	
	/**
	* @brief このオブジェクトに名前をつける。
	*
	* 名前を持てないオブジェクトや、前に付けられた名前の方が強い場合無視される。
	* @param name つける名前
	* @param force 名前の強さ
	* @param parent 親
	*/
	void set_object_name(const String& name, int_t force, const Any& parent) const;

	/**
	* @brief ハッシュ値を返す
	*
	*/
	uint_t hashcode() const;

	/**
	* @brief 自身を文字列化してprintlnする。
	* @return 自身を返す。
	*/
	Any p() const;
	
	/**
	* @brief 自身がnullか返す
	*/ 
	bool is_null() const{ return type()==TYPE_NULL; }

public:

	int_t arity() const;

	// 関数呼び出し。5つまでの引数を受け取るバージョンが定義されている。
	Any operator()() const;
	Any operator()(const Any& a0) const;
	Any operator()(const Any& a0, const Any& a1) const;
	Any operator()(const Any& a0, const Any& a1, const Any& a2) const;
	Any operator()(const Any& a0, const Any& a1, const Any& a2, const Any& a3) const;
	Any operator()(const Any& a0, const Any& a1, const Any& a2, const Any& a3, const Any& a4) const;
	Any operator()(const Named& a0) const;
	Any operator()(const Any& a0, const Named& a1) const;
	Any operator()(const Any& a0, const Any& a1, const Named& a2) const;
	Any operator()(const Any& a0, const Any& a1, const Any& a2, const Named& a3) const;
	Any operator()(const Any& a0, const Any& a1, const Any& a2, const Any& a3, const Named& a4) const;
	Any operator()(const Named& a0, const Named& a1) const;
	Any operator()(const Any& a0, const Named& a1, const Named& a2) const;
	Any operator()(const Any& a0, const Any& a1, const Named& a2, const Named& a3) const;
	Any operator()(const Any& a0, const Any& a1, const Any& a2, const Named& a3, const Named& a4) const;
	Any operator()(const Named& a0, const Named& a1, const Named& a2) const;
	Any operator()(const Any& a0, const Named& a1, const Named& a2, const Named& a3) const;
	Any operator()(const Any& a0, const Any& a1, const Named& a2, const Named& a3, const Named& a4) const;
	Any operator()(const Named& a0, const Named& a1, const Named& a2, const Named& a3) const;
	Any operator()(const Any& a0, const Named& a1, const Named& a2, const Named& a3, const Named& a4) const;
	Any operator()(const Named& a0, const Named& a1, const Named& a2, const Named& a3, const Named& a4) const;

	// メソッド呼び出し。5つまでの引数を受け取るバージョンが定義されている。
	Any send(const ID& name) const;
	Any send(const ID& name, const Any& a0) const;
	Any send(const ID& name, const Any& a0, const Any& a1) const;
	Any send(const ID& name, const Any& a0, const Any& a1, const Any& a2) const;
	Any send(const ID& name, const Any& a0, const Any& a1, const Any& a2, const Any& a3) const;
	Any send(const ID& name, const Any& a0, const Any& a1, const Any& a2, const Any& a3, const Any& a4) const;
	Any send(const ID& name, const Named& a0) const;
	Any send(const ID& name, const Any& a0, const Named& a1) const;
	Any send(const ID& name, const Any& a0, const Any& a1, const Named& a2) const;
	Any send(const ID& name, const Any& a0, const Any& a1, const Any& a2, const Named& a3) const;
	Any send(const ID& name, const Any& a0, const Any& a1, const Any& a2, const Any& a3, const Named& a4) const;
	Any send(const ID& name, const Named& a0, const Named& a1) const;
	Any send(const ID& name, const Any& a0, const Named& a1, const Named& a2) const;
	Any send(const ID& name, const Any& a0, const Any& a1, const Named& a2, const Named& a3) const;
	Any send(const ID& name, const Any& a0, const Any& a1, const Any& a2, const Named& a3, const Named& a4) const;
	Any send(const ID& name, const Named& a0, const Named& a1, const Named& a2) const;
	Any send(const ID& name, const Any& a0, const Named& a1, const Named& a2, const Named& a3) const;
	Any send(const ID& name, const Any& a0, const Any& a1, const Named& a2, const Named& a3, const Named& a4) const;
	Any send(const ID& name, const Named& a0, const Named& a1, const Named& a2, const Named& a3) const;
	Any send(const ID& name, const Any& a0, const Named& a1, const Named& a2, const Named& a3, const Named& a4) const;
	Any send(const ID& name, const Named& a0, const Named& a1, const Named& a2, const Named& a3, const Named& a4) const;

public:

	/**
	* @brief 連結する
	*
	* send("op_cat", v)と等しい
	*/
	Any cat(const Any& v) const;

	/**
	* @brief 配列の要素を返す
	*
	* send("op_at", index)と等しい
	*/	
	const Any at(const Any& index) const;
	
	/**
	* @brief 配列の要素を設定する
	*
	* send("op_set_at", index, value)と等しい
	*/	
	void set_at(const Any& index, const Any& value) const;
		
	/**
	* @brief 配列の長さを取得する
	*
	* send("size").to_i()と等しい
	*/
	int_t size() const;

	/**
	* @brief 配列の要素を返す
	*
	* send("op_at", index)と等しい
	*/
	const Any operator[](const Any& index) const;

	/**
	* @brief 同一のオブジェクトか調べる
	*
	*/
	bool raweq(const Any& a) const{ return UncountedAny::raweq(a); }

	/**
	* @brief 同一のオブジェクトではないか調べる
	*
	*/
	bool rawne(const Any& a) const{ return UncountedAny::rawne(a); }
	
	/**
	* @brief オブジェクトのアドレス等でより小さいか求める
	*
	*/
	bool rawlt(const Any& a) const{ return UncountedAny::rawlt(a); }

protected:

	void inc_ref_count() const;

	void dec_ref_count() const;	

	const Any& cref() const{
		return (const Any&)*this;
	}

private:

	struct dummy_bool_tag{ void safe_true(dummy_bool_tag){} };
	typedef void (dummy_bool_tag::*safe_bool)(dummy_bool_tag);

public:
	
	/**
	* @brief bool型に自動変換する。
	*
	*/
	operator safe_bool() const{
		return to_b() ? &dummy_bool_tag::safe_true : (safe_bool)0;
	}
};

/**
* @brief 足す
*
* a.send("op_add", b)と等しい
*/	
Any operator +(const Any& a, const Any& b);

/**
* @brief 引く
*
* a.send("op_sub", b)と等しい
*/	
Any operator -(const Any& a, const Any& b);

/**
* @brief 掛ける
*
* a.send("op_mul", b)と等しい
*/	
Any operator *(const Any& a, const Any& b);

/**
* @brief 割る
*
* a.send("op_div", b)と等しい
*/	
Any operator /(const Any& a, const Any& b);

/**
* @brief 余りを求める
*
* a.send("op_mod", b)と等しい
*/	
Any operator %(const Any& a, const Any& b);

/**
* @brief bitwise OR
*
* a.send("op_or", b)と等しい
*/	
Any operator |(const Any& a, const Any& b);

/**
* @brief bitwise AND
*
* a.send("op_and", b)と等しい
*/	
Any operator &(const Any& a, const Any& b);

/**
* @brief bitwise XOR
* a.send("op_xor", b)と等しい
*/	
Any operator ^(const Any& a, const Any& b);

/**
* @brief 右シフト
*
* a.send("op_shr", b)と等しい
*/	
Any operator >>(const Any& a, const Any& b);

/**
* @brief 左シフト
*
* a.send("op_shl", b)と等しい
*/	
Any operator <<(const Any& a, const Any& b);

/**
* @brief 等しいか
*
* a.send("op_eq", b)と等しい
*/	
Any operator ==(const Any& a, const Any& b);

/**
* @brief 等しくないか
*
* !a.send("op_eq", b)と等しい
*/	
Any operator !=(const Any& a, const Any& b);

/**
* @brief より小さいか
*
* a.send("op_lt", b)と等しい
*/	
Any operator <(const Any& a, const Any& b);

/**
* @brief より大きいか
*
* b.send("op_lt", a)と等しい
*/	
Any operator >(const Any& a, const Any& b);

/**
* @brief 以上か
*
* !b.send("op_lt", a)と等しい
*/	
Any operator <=(const Any& a, const Any& b);

/**
* @brief 以下か
* !a.send("op_lt", b)と等しい
*/	
Any operator >=(const Any& a, const Any& b);

/**
* @brief 足し合わせる
*
* a = a.send("op_add_assign", b)と等しい
*/	
Any& operator +=(Any& a, const Any& b);

/**
* @brief 引き合わせる
*
* a = a.send("op_sub_assign", b)と等しい
*/	
Any& operator -=(Any& a, const Any& b);

/**
* @brief 掛け合わせる
*
* a = a.send("op_mod_assign", b)と等しい
*/	
Any& operator *=(Any& a, const Any& b);

/**
* @brief 割り合わせる
*
* a = a.send("op_div_assign", b)と等しい
*/	
Any& operator /=(Any& a, const Any& b);

/**
* @brief 余りを求め合わせる
*
* a = a.send("op_mod_assign", b)と等しい
*/	
Any& operator %=(Any& a, const Any& b);

/**
* @brief bitwise ORして代入する
*
* a = a.send("op_or_assign", b)と等しい
*/	
Any& operator |=(Any& a, const Any& b);

/**
* @brief bitwise ANDして代入する
*
* a = a.send("op_and_assign", b)と等しい
*/	
Any& operator &=(Any& a, const Any& b);

/**
* @brief bitwise XORして代入する
*
* a = a.send("op_xor_assign", b)と等しい
*/	
Any& operator ^=(Any& a, const Any& b);

/**
* @brief 右シフトして代入する
*
* a = a.send("op_shr_assign", b)と等しい
*/	
Any& operator >>=(Any& a, const Any& b);

/**
* @brief 左シフトして代入する
*
* a = a.send("op_shl_assign", b)と等しい
*/	
Any& operator <<=(Any& a, const Any& b);


// Anyの実装
class AnyImpl{
public:
	
	AnyImpl()
		:class_(UncountedAny::noinit_t()){}

	AnyImpl(const AnyImpl& b)
		:class_(b.class_){}

	virtual ~AnyImpl(){}
	
public:

	struct noinit_t{};
	AnyImpl(noinit_t)
		:class_(UncountedAny::noinit_t()){}
		
public:
	
	static void* operator new(size_t size);
	static void operator delete(void* p, size_t size);
	
	static void* operator new(size_t size, Any& guard);
	static void operator delete(void* p, Any& guard);

	static void* operator new(size_t, void* p){ return p; }
	static void operator delete(void*, void*){}

public:

	uint_t ref_count(){ return ref_count_; }
	void add_ref_count(int_t rc){ ref_count_+=rc; }
	void inc_ref_count(){ ++ref_count_; }
	void dec_ref_count(){ --ref_count_; }
	void set_ref_count(uint_t rc){ ref_count_ = rc; }
	const Class& get_class(){ return *(const Class*)&class_; }
	void set_class(const Class& c);
	
public:

	virtual void visit_members(Visitor& m);
	virtual void call(const VMachine& vm);
	virtual int_t arity();
	virtual const Any& member(const ID& name, const Any& self, const Any& ns);
	virtual void def(const ID& name, const Any& value, int_t accessibility, const Any& ns);
	virtual HaveInstanceVariables* have_instance_variables();
	virtual String object_name();
	virtual int_t object_name_force();
	virtual void set_object_name(const String& name, int_t force, const Any& parent);
	virtual uint_t hashcode();

private:

	uint_t ref_count_;
	UncountedAny class_;
	
private:

	friend void gc();
	friend void full_gc();
};


class Null : public Any{};
class Nop : public Any{ public: Nop():Any(TYPE_NOP){} };
class True : public Any{ public: True():Any(TYPE_TRUE){} };
class False : public Any{ public: False():Any(TYPE_FALSE){} };

inline bool operator ==(const Any& a, const Null&){ return !a; }
inline bool operator !=(const Any& a, const Null&){ return a; }
inline bool operator ==(const Null&, const Any& a){ return !a; }
inline bool operator !=(const Null&, const Any& a){ return a; }

inline void Any::inc_ref_count() const{
	if(type()==TYPE_BASE){ impl()->inc_ref_count(); }
}

inline void Any::dec_ref_count() const{
	if(type()==TYPE_BASE){ impl()->dec_ref_count(); }
}

inline uint_t Any::hashcode() const{
	if(type()==TYPE_BASE){
		return impl()->hashcode();
	}
	return (uint_t)rawvalue();
}

}
