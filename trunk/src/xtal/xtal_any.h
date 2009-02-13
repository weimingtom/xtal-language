
#pragma once

namespace xtal{

/**
* @brief any
*
*/
class Any{
public:

	Any(){ type_ = TYPE_NULL; pvalue_ = 0; }
	Any(int_t v){ set_i(v); }
	Any(float_t v){ set_f(v); }
	Any(Base* v){ set_p(v); }
	Any(bool b){ set_b(b); }

	// 基本型の整数、浮動小数点数から構築するコンストラクタ
	Any(avoid<int>::type v){ set_i((int_t)v); }
	Any(avoid<long>::type v){ set_i((int_t)v); }
	Any(avoid<short>::type v){ set_i((int_t)v); }
	Any(avoid<char>::type v){ set_i((int_t)v); }
	Any(avoid<unsigned int>::type v){ set_i((int_t)v); }
	Any(avoid<unsigned long>::type v){ set_i((int_t)v); }
	Any(avoid<unsigned short>::type v){ set_i((int_t)v); }
	Any(avoid<unsigned char>::type v){ set_i((int_t)v); }
	Any(avoid<signed char>::type v){ set_i((int_t)v); }
	Any(avoid<float>::type v){ set_f((float_t)v); }
	Any(avoid<double>::type v){ set_f((float_t)v); }
	Any(avoid<long double>::type v){ set_f((float_t)v); }
	
	Any(PrimitiveType type){
		type_ = type;
		value_ = 0;
	}

	struct noinit_t{};
	Any(noinit_t){}
	
public:

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
		*(float_t*)fvalue_bytes = v;
	}

	void set_b(bool b){
		type_ = TYPE_FALSE + (int)b;
		value_ = 0;
	}

	void set_small_string(){
		type_ = TYPE_SMALL_STRING;
		value_ = 0;
	}

public:

	friend int_t type(const Any& v);
	friend int_t ivalue(const Any& v);
	friend float_t fvalue(const Any& v);
	friend Base* pvalue(const Any& v);
	friend uint_t rawtype(const Any& v);
	friend uint_t rawvalue(const Any& v);
	friend bool raweq(const Any& a, const Any& b);
	friend bool rawne(const Any& a, const Any& b);
	friend bool rawlt(const Any& a, const Any& b);
	friend void swap(Any& a, Any& b);
	friend void set_null_force(Any& v);
	friend void set_undefined_force(Any& v);
	friend void copy_innocence(Any& v, const Any& u);

public:

	enum{
		SMALL_STRING_MAX = sizeof(int_t) / sizeof(char_t)
	};

protected:

	struct SmallString{
		char_t buf[SMALL_STRING_MAX];

		operator char_t*(){
			return buf;
		}
	};

	int_t type_;

	union{
		int_t value_;
		byte_t fvalue_bytes[sizeof(float_t)];
		float_t fvalue_;
		Base* pvalue_;
		SmallString svalue_;
	};

public:

	/**
	* @brief 関数オブジェクトとみなし、関数呼び出しをする。
	*
	* 引数や戻り値はvmを通してやり取りする。
	*/
	void rawcall(const VMachinePtr& vm) const;
	
	/**
	* @brief nameメソッド呼び出しをする
	*
	* 引数や戻り値はvmを通してやり取りする。
	*/
	void rawsend(const VMachinePtr& vm, const IDPtr& primary_key, const AnyPtr& secondary_key, const AnyPtr& self = (const AnyPtr&)null, bool inherited_too = true) const;
	void rawsend(const VMachinePtr& vm, const IDPtr& primary_key) const;

	/**
	* @brief nameメンバを取得する。
	* 可触性を考慮したメンバ取得
	*
	* @retval undefined そのメンバは存在しない
	* @retval 非undefined nameに対応したメンバ  
	*/
	const AnyPtr& member(const IDPtr& primary_key, const AnyPtr& secondary_key = (const AnyPtr&)null, const AnyPtr& self = (const AnyPtr&)null, bool inherited_too = true) const;

	/**
	* @brief nameメンバを初期値valueで定義する。
	*
	*/
	void def(const IDPtr& primary_key, const AnyPtr& value, const AnyPtr& secondary_key = (const AnyPtr&)null, int_t accessibility = 0) const;

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
	* @brief 配列に変換して返す。
	*
	*/
	ArrayPtr to_a() const;

	/**
	* @brief 連想配列に変換して返す。
	*
	*/
	MapPtr to_m() const;
	
	/**
	* @brief このオブジェクトに付けられた名前を返す。
	*
	*/
	StringPtr object_name(int_t depth = -1) const;

	/**
	* @brief klassクラスのインスタンスか調べる。
	*
	*/
	bool is(const AnyPtr& klass) const;
		
	/**
	* @brief klassクラスを継承しているか調べる
	*
	*/
	bool is_inherited(const AnyPtr& klass) const;

	/**
	* @brief このオブジェクトに付けられた名前の強さを返す。
	*
	*/
	int_t object_name_force() const;

	ArrayPtr object_name_list() const;
	
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


	AnyPtr s_save() const;

	void s_load(const AnyPtr& v) const;

	AnyPtr serial_save(const ClassPtr& cls) const;

	void serial_load(const ClassPtr& cls, const AnyPtr& v) const;

	/**
	* @brief 自身を返す。
	*
	*/
	const AnyPtr& self() const{
		return (const AnyPtr&)*this;
	}

	MultiValuePtr to_mv() const;

	MultiValuePtr flatten_mv() const;

	MultiValuePtr flatten_all_mv() const;
	
public:

	/// @brief primary_keyメソッドを呼び出す
	AnyPtr send(const IDPtr& primary_key) const;

	/// @brief primary_key#secondary_keyメソッドを呼び出す
	AnyPtr send2(const IDPtr& primary_key, const AnyPtr& secondary_key) const;

	/// @brief 関数を呼び出す
	AnyPtr call() const;

//{REPEAT{{
/*
	/// @brief primary_keyメソッドを呼び出す
	template<class A0 #COMMA_REPEAT#class A`i+1`#>
	AnyPtr send(const IDPtr& primary_key, const A0& a0 #COMMA_REPEAT#const A`i+1`& a`i+1`#) const;

	/// @brief primary_key#secondary_keyメソッドを呼び出す
	template<class A0 #COMMA_REPEAT#class A`i+1`#>
	AnyPtr send2(const IDPtr& primary_key, const AnyPtr& secondary_key, const A0& a0 #COMMA_REPEAT#const A`i+1`& a`i+1`#) const;

	/// @brief 関数を呼び出す
	template<class A0 #COMMA_REPEAT#class A`i+1`#>
	AnyPtr call(const A0& a0 #COMMA_REPEAT#const A`i+1`& a`i+1`#) const;
*/

	/// @brief primary_keyメソッドを呼び出す
	template<class A0 >
	AnyPtr send(const IDPtr& primary_key, const A0& a0 ) const;

	/// @brief primary_key#secondary_keyメソッドを呼び出す
	template<class A0 >
	AnyPtr send2(const IDPtr& primary_key, const AnyPtr& secondary_key, const A0& a0 ) const;

	/// @brief 関数を呼び出す
	template<class A0 >
	AnyPtr call(const A0& a0 ) const;

	/// @brief primary_keyメソッドを呼び出す
	template<class A0 , class A1>
	AnyPtr send(const IDPtr& primary_key, const A0& a0 , const A1& a1) const;

	/// @brief primary_key#secondary_keyメソッドを呼び出す
	template<class A0 , class A1>
	AnyPtr send2(const IDPtr& primary_key, const AnyPtr& secondary_key, const A0& a0 , const A1& a1) const;

	/// @brief 関数を呼び出す
	template<class A0 , class A1>
	AnyPtr call(const A0& a0 , const A1& a1) const;

	/// @brief primary_keyメソッドを呼び出す
	template<class A0 , class A1, class A2>
	AnyPtr send(const IDPtr& primary_key, const A0& a0 , const A1& a1, const A2& a2) const;

	/// @brief primary_key#secondary_keyメソッドを呼び出す
	template<class A0 , class A1, class A2>
	AnyPtr send2(const IDPtr& primary_key, const AnyPtr& secondary_key, const A0& a0 , const A1& a1, const A2& a2) const;

	/// @brief 関数を呼び出す
	template<class A0 , class A1, class A2>
	AnyPtr call(const A0& a0 , const A1& a1, const A2& a2) const;

	/// @brief primary_keyメソッドを呼び出す
	template<class A0 , class A1, class A2, class A3>
	AnyPtr send(const IDPtr& primary_key, const A0& a0 , const A1& a1, const A2& a2, const A3& a3) const;

	/// @brief primary_key#secondary_keyメソッドを呼び出す
	template<class A0 , class A1, class A2, class A3>
	AnyPtr send2(const IDPtr& primary_key, const AnyPtr& secondary_key, const A0& a0 , const A1& a1, const A2& a2, const A3& a3) const;

	/// @brief 関数を呼び出す
	template<class A0 , class A1, class A2, class A3>
	AnyPtr call(const A0& a0 , const A1& a1, const A2& a2, const A3& a3) const;

	/// @brief primary_keyメソッドを呼び出す
	template<class A0 , class A1, class A2, class A3, class A4>
	AnyPtr send(const IDPtr& primary_key, const A0& a0 , const A1& a1, const A2& a2, const A3& a3, const A4& a4) const;

	/// @brief primary_key#secondary_keyメソッドを呼び出す
	template<class A0 , class A1, class A2, class A3, class A4>
	AnyPtr send2(const IDPtr& primary_key, const AnyPtr& secondary_key, const A0& a0 , const A1& a1, const A2& a2, const A3& a3, const A4& a4) const;

	/// @brief 関数を呼び出す
	template<class A0 , class A1, class A2, class A3, class A4>
	AnyPtr call(const A0& a0 , const A1& a1, const A2& a2, const A3& a3, const A4& a4) const;

	/// @brief primary_keyメソッドを呼び出す
	template<class A0 , class A1, class A2, class A3, class A4, class A5>
	AnyPtr send(const IDPtr& primary_key, const A0& a0 , const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5) const;

	/// @brief primary_key#secondary_keyメソッドを呼び出す
	template<class A0 , class A1, class A2, class A3, class A4, class A5>
	AnyPtr send2(const IDPtr& primary_key, const AnyPtr& secondary_key, const A0& a0 , const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5) const;

	/// @brief 関数を呼び出す
	template<class A0 , class A1, class A2, class A3, class A4, class A5>
	AnyPtr call(const A0& a0 , const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5) const;

//}}REPEAT}

private:

	// call(VMachinePtr)がコンパイルエラーとなるように
	struct cmpitle_error{ cmpitle_error(const VMachinePtr& vm); };
	void call(cmpitle_error) const;

};

inline int_t type(const Any& v){ 
	return v.type_ & TYPE_MASK; 
}

inline int_t ivalue(const Any& v){ 
	XTAL_ASSERT(type(v)==TYPE_INT);
	return v.value_; 
}

inline float_t fvalue(const Any& v){ 
	XTAL_ASSERT(type(v)==TYPE_FLOAT); 
	return *(float_t*)v.fvalue_bytes; 
}

inline Base* pvalue(const Any& v){ 
	XTAL_ASSERT(type(v)==TYPE_BASE || type(v)==TYPE_NULL); 
	return v.pvalue_; 
}

inline uint_t rawtype(const Any& v){
	return (uint_t)(v.type_);
}

inline uint_t rawvalue(const Any& v){
	return (uint_t)(v.value_);
}

inline bool raweq(const Any& a, const Any& b){
	return type(a)==type(b) && a.value_==b.value_;
}

inline bool rawne(const Any& a, const Any& b){
	return !raweq(a, b);
}

inline bool rawlt(const Any& a, const Any& b){
	if(type(a)<type(b))
		return true;
	if(type(b)<type(a))
		return false;
	return a.value_<b.value_;
}

inline void set_null_force(Any& v){
	v.type_ = TYPE_NULL;
	v.value_ = 0;
}

inline void set_undefined_force(Any& v){
	v.type_ = TYPE_UNDEFINED;
	v.value_ = 0;
}

inline void copy_innocence(Any& v, const Any& u){
	v = u;
}

}
