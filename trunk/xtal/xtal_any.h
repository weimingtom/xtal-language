
#pragma once

#include "xtal_utility.h"
#include "xtal_fwd.h"

namespace xtal{


/**
* @brief イノセンス
*
* 一般ユーザーはこれを直接使用することは無い
*/
class Innocence{
public:

	AnyPtr operator()() const;

	template<class A0>
	AnyPtr operator()(const A0& a0) const;

	template<class A0, class A1>
	AnyPtr operator()(const A0& a0, const A1& a1) const;

	template<class A0, class A1, class A2>
	AnyPtr operator()(const A0& a0, const A1& a1, const A2& a2) const;

	template<class A0, class A1, class A2, class A3>
	AnyPtr operator()(const A0& a0, const A1& a1, const A2& a2, const A3& a3) const;

	template<class A0, class A1, class A2, class A3, class A4>
	AnyPtr operator()(const A0& a0, const A1& a1, const A2& a2, const A3& a3, const A4& a4) const;

	AtProxy operator[](const AnyPtr& key) const;

protected:

	Innocence(const char* str);

#ifdef XTAL_USE_COMPRESSED_INNOCENCE

public:

	Innocence(){ value_ = TYPE_NULL; }
	Innocence(int_t v){ set_i(v); }
	Innocence(float_t v){ set_f(v); }
	Innocence(Base* v){ set_p(v); }
	Innocence(bool b){ set_b(b); }
	Innocence(check_xtype<int>::type v){ set_i(v); }
	Innocence(check_xtype<float>::type v){ set_f(v); }
	Innocence(PrimitiveType type){ value_ = type; }
	
	struct noinit_t{};
	Innocence(noinit_t){}

protected:

	void set_null(){
		value_ = TYPE_NULL;
	}

	void set_p(Base* p){
		XTAL_ASSERT(p!=0);
		union{
			int_t value;
			Base* pvalue;
		} u;
		u.pvalue = p;
		u.value |= TYPE_BASE;
		value_ = u.value;
	}
	
	void set_p(const Base* p){
		set_p((Base*)p);
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

	friend int_t type(const Innocence& v){
		return v.value_ & TYPE_MASK; 
	}
	
	friend int_t ivalue(const Innocence& v){ 
		XTAL_ASSERT(type(v)==TYPE_INT);
		return v.value_ >> TYPE_SHIFT; 
	}
	
	friend float_t fvalue(const Innocence& v){ 
		XTAL_ASSERT(type(v)==TYPE_FLOAT);
		union{
			int_t value;
			float_t fvalue;
		} u;
		u.value = (v.value_ & ~TYPE_MASK);
		return u.fvalue;
	}
	
	friend Base* pvalue(const Innocence& v){
		XTAL_ASSERT(type(v)==TYPE_BASE || type(v)==TYPE_NULL);
		union{
			int_t value;
			Base* pvalue;
		} u;
		u.value = (v.value_ & ~TYPE_MASK);
		return u.pvalue; 
	}

	friend int_t rawvalue(const Innocence& v){
		return v.value_;
	}

	friend bool raweq(const Innocence& a, const Innocence& b){
		return a.value_==b.value_;
	}

	friend bool rawne(const Innocence& a, const Innocence& b){
		return !raweq(a, b);
	}
	
	friend bool rawlt(const Innocence& a, const Innocence& b){
		return a.value_<b.value_;
	}

	friend void swap(Innocence& a, Innocence& b){
		std::swap(a.value_, b.value_);
	}

	friend void set_null_force(Innocence& v){
		v.value_ = TYPE_NULL;
	}

private:

	union{
		int_t value_;
		float_t fvalue_;
		Base* pvalue_;
	};

#else
	

public:

	Innocence(){ type_ = TYPE_NULL; pvalue_ = 0; }
	Innocence(u8 v){ set_i(v); }
	Innocence(u16 v){ set_i(v); }
	Innocence(i8 v){ set_i(v); }
	Innocence(i16 v){ set_i(v); }
	Innocence(int_t v){ set_i(v); }
	Innocence(float_t v){ set_f(v); }
	Innocence(Base* v){ set_p(v); }
	Innocence(bool b){ set_b(b); }
	Innocence(check_xtype<int>::type v){ set_i(v); }
	Innocence(check_xtype<float>::type v){ set_f(v); }
	
	Innocence(PrimitiveType type){
		type_ = type;
		value_ = 0;
	}

	struct noinit_t{};
	Innocence(noinit_t){}
	
protected:

	void set_null(){
		type_ = TYPE_NULL;
		pvalue_ = 0;
	}


	void set_p(Base* p){
		XTAL_ASSERT(p!=0);
		type_ = TYPE_BASE;
		pvalue_ = p;
	}
	
	void set_p(const Base* p){
		XTAL_ASSERT(p!=0);
		type_ = TYPE_BASE;
		pvalue_ = (Base*)p;
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

	friend int_t type(const Innocence& v){ 
		return v.type_; 
	}
	
	friend int_t ivalue(const Innocence& v){ 
		XTAL_ASSERT(type(v)==TYPE_INT);
		return v.value_; 
	}
	
	friend float_t fvalue(const Innocence& v){ 
		XTAL_ASSERT(type(v)==TYPE_FLOAT); 
		return v.fvalue_; 
	}
	
	friend Base* pvalue(const Innocence& v){ 
		XTAL_ASSERT(type(v)==TYPE_BASE || type(v)==TYPE_NULL); 
		return v.pvalue_; 
	}

	friend uint_t rawvalue(const Innocence& v){
		return (uint_t)v.value_;
	}
	
	friend bool raweq(const Innocence& a, const Innocence& b){
		return a.type_==b.type_ && a.value_==b.value_;
	}

	friend bool rawne(const Innocence& a, const Innocence& b){
		return !raweq(a, b);
	}
	
	friend bool rawlt(const Innocence& a, const Innocence& b){
		if(a.type_<b.type_)
			return true;
		if(b.type_<a.type_)
			return false;
		return a.value_<b.value_;
	}

	friend void swap(Innocence& a, Innocence& b){
		std::swap(a.type_, b.type_);
		std::swap(a.value_, b.value_);
	}

	friend void set_null_force(Innocence& v){
		v.type_ = TYPE_NULL;
		v.value_ = 0;
	}

private:

	int_t type_;
	
	union{
		int_t value_;
		float_t fvalue_;
		Base* pvalue_;
	};

#endif

};


/**
* @brief any
*
*/
class Any : public Innocence{
private:

	// 直接 Anyを作ることは出来ない

	Any();
	Any(const Any& v);
	~Any();

public:

	/**
	* @brief 関数オブジェクトとみなし、関数呼び出しをする。
	*
	* 引数や戻り値はvmを通してやり取りする。
	*/
	void call(const VMachinePtr& vm) const;
	
	/**
	* @brief nameメソッド呼び出しをする
	*
	* 引数や戻り値はvmを通してやり取りする。
	*/
	void rawsend(const VMachinePtr& vm, const InternedStringPtr& name, const AnyPtr& self = (const AnyPtr& )null, const AnyPtr& ns = (const AnyPtr& )null) const;

	/**
	* @brief nameメンバを取得する。
	* 可触性を考慮したメンバ取得
	*
	* @retval null そのメンバは存在しない
	* @retval 非null nameに対応したメンバ  
	*/
	const AnyPtr& member(const InternedStringPtr& name, const AnyPtr& self = (const AnyPtr& )null, const AnyPtr& ns = (const AnyPtr& )null) const;

	/**
	* @brief nameメンバを初期値valueで定義する。
	*
	*/
	void def(const InternedStringPtr& name, const AnyPtr& value, int_t accessibility = 0, const AnyPtr& ns = (const AnyPtr&)null) const;

	/**
	* @brief このオブジェクトが所属するクラスを返す。
	*
	*/
	const ClassPtr& get_class() const;

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
	StringPtr to_s() const;
	
	/**
	* @brief このオブジェクトに付けられた名前を返す。
	*
	*/
	StringPtr object_name() const;

	/**
	* @brief clsクラスのインスタンスか調べる。
	*
	*/
	bool is(const ClassPtr& cls) const;
		
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
	void set_object_name(const StringPtr& name, int_t force, const AnyPtr& parent) const;

	/**
	* @brief ハッシュ値を返す
	*
	*/
	uint_t hashcode() const;

	/**
	* @brief 自身を文字列化してprintlnする。
	* @return 自身を返す。
	*/
	AnyPtr p() const;

public:

	AnyPtr send(const InternedStringPtr& name) const;

	template<class A0>
	AnyPtr send(const InternedStringPtr& name, const A0& a0) const;

	template<class A0, class A1>
	AnyPtr send(const InternedStringPtr& name, const A0& a0, const A1& a1) const;

	template<class A0, class A1, class A2>
	AnyPtr send(const InternedStringPtr& name, const A0& a0, const A1& a1, const A2& a2) const;

	template<class A0, class A1, class A2, class A3>
	AnyPtr send(const InternedStringPtr& name, const A0& a0, const A1& a1, const A2& a2, const A3& a3) const;

	template<class A0, class A1, class A2, class A3, class A4>
	AnyPtr send(const InternedStringPtr& name, const A0& a0, const A1& a1, const A2& a2, const A3& a3, const A4& a4) const;
};


AnyPtr operator +(const AnyPtr& a, const AnyPtr& b);
AnyPtr operator -(const AnyPtr& a, const AnyPtr& b);
AnyPtr operator *(const AnyPtr& a, const AnyPtr& b);
AnyPtr operator /(const AnyPtr& a, const AnyPtr& b);
AnyPtr operator %(const AnyPtr& a, const AnyPtr& b);
AnyPtr operator |(const AnyPtr& a, const AnyPtr& b);
AnyPtr operator &(const AnyPtr& a, const AnyPtr& b);
AnyPtr operator ^(const AnyPtr& a, const AnyPtr& b);
AnyPtr operator >>(const AnyPtr& a, const AnyPtr& b);
AnyPtr operator <<(const AnyPtr& a, const AnyPtr& b);
AnyPtr operator ==(const AnyPtr& a, const AnyPtr& b);
AnyPtr operator !=(const AnyPtr& a, const AnyPtr& b);
AnyPtr operator <(const AnyPtr& a, const AnyPtr& b);
AnyPtr operator >(const AnyPtr& a, const AnyPtr& b);
AnyPtr operator <=(const AnyPtr& a, const AnyPtr& b);
AnyPtr operator >=(const AnyPtr& a, const AnyPtr& b);

AnyPtr& operator +=(AnyPtr& a, const AnyPtr& b);
AnyPtr& operator -=(AnyPtr& a, const AnyPtr& b);
AnyPtr& operator *=(AnyPtr& a, const AnyPtr& b);
AnyPtr& operator /=(AnyPtr& a, const AnyPtr& b);
AnyPtr& operator %=(AnyPtr& a, const AnyPtr& b);
AnyPtr& operator |=(AnyPtr& a, const AnyPtr& b);
AnyPtr& operator &=(AnyPtr& a, const AnyPtr& b);
AnyPtr& operator ^=(AnyPtr& a, const AnyPtr& b);
AnyPtr& operator >>=(AnyPtr& a, const AnyPtr& b);
AnyPtr& operator <<=(AnyPtr& a, const AnyPtr& b);

}
