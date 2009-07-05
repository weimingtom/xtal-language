/** \file src/xtal/xtal_any.h
* \brief src/xtal/xtal_any.h
*/

#ifndef XTAL_ANY_H_INCLUDE_GUARD
#define XTAL_ANY_H_INCLUDE_GUARD

#pragma once

namespace xtal{

enum{
	SMALL_STRING_MAX = sizeof(int_t) / sizeof(char_t)
};

union AnyRawValue{
	int_t ivalue;
	uint_t uvalue;
	float_t fvalue;
	Base* pvalue;
	RefCountingBase* rcpvalue;
	char_t svalue[SMALL_STRING_MAX];
};

/**
* \xbind lib::builtin
* \brief すべてのクラスの基底クラス
*/
class Any{
public:

	/**
	* \brief 関数オブジェクトとみなし、関数呼び出しをする。
	* 引数や戻り値はvmを通してやり取りする。
	*/
	void rawcall(const VMachinePtr& vm) const;
	
	/**
	* \brief メソッド呼び出しをする
	* 引数や戻り値はvmを通してやり取りする。
	* \param vm 仮想マシン
	* \param primary_key メンバ名
	* \param secondary_key セカンダリキー
	* \param self 可触性に影響するオブジェクト
	* \param inherited_too 継承元クラスからもメソッド検索をするかどうか
	*/
	void rawsend(const VMachinePtr& vm, const IDPtr& primary_key, const AnyPtr& secondary_key = (const AnyPtr&)undefined, bool inherited_too = true, bool q = false) const;

	/**
	* \brief メンバを取得する。
	* \retval undefined そのメンバは存在しない
	* \retval 非undefined nameに対応したメンバ  
	*/
	const AnyPtr& member(const IDPtr& primary_key, const AnyPtr& secondary_key = (const AnyPtr&)undefined, bool inherited_too = true, int_t& accessibility = Temp()) const;

	/**
	* \brief メンバを定義する。
	* \param primary_key メンバ名
	* \param secondary_key セカンダリキー
	*/
	void def(const IDPtr& primary_key, const AnyPtr& value, const AnyPtr& secondary_key = (const AnyPtr&)undefined, int_t accessibility = 0) const;

	/**
	* \xbind
	* \brief このオブジェクトが所属するクラスを返す。
	*/
	const ClassPtr& get_class() const;

	/**
	* \brief 整数に変換して返す。
	*/
	int_t to_i() const;
	
	/**
	* \brief 浮動小数点数に変換して返す。
	*/
	float_t to_f() const;
	
	/**
	* \brief 文字列に変換して返す。
	*/
	StringPtr to_s() const;

	/**
	* \brief 配列に変換して返す。
	*/
	ArrayPtr to_a() const;

	/**
	* \brief 連想配列に変換して返す。
	*/
	MapPtr to_m() const;

	/**
	* \brief klassクラスのインスタンスか調べる。
	*/
	bool is(const AnyPtr& klass) const;
		
	/**
	* \brief klassクラスを継承しているか調べる
	*/
	bool is_inherited(const AnyPtr& klass) const;
	
	/**
	* \brief このオブジェクトがメンバとなっている親のクラスを返す。
	*/
	const ClassPtr& object_parent() const;
	
	/**
	* \brief このオブジェクトに親を設定する。
	* 親を持てないオブジェクトや、前に付けられた親の方が強い場合無視される。
	* \param parent 親
	*/
	void set_object_parent(const ClassPtr& parent) const;

	/**
	* \xbind
	* \brief オブジェクトの名前を返す
	*/
	StringPtr object_name() const;

	/**
	* \xbind
	* \brief オブジェクトの名前のリストを返す
	* 一番最上位の親からの名前のリストを返す
	*/
	ArrayPtr object_name_list() const;

	/**
	* \xbind
	* \brief 自身を文字列化してprintlnする。
	* \return 自身を返す。
	*/
	AnyPtr p() const;

public:

	AnyPtr s_save() const;

	void s_load(const AnyPtr& v) const;

	AnyPtr save_instance_variables(const ClassPtr& cls) const;

	void load_instance_variables(const ClassPtr& cls, const AnyPtr& v) const;

	void visit_members(Visitor& m) const;

	StringPtr ask_object_name_to_parent() const;

	InstanceVariables* instance_variables() const;

public:

	/// \brief primary_keyメソッドを呼び出す
	AnyPtr send(const IDPtr& primary_key) const;

	/// \brief primary_key#secondary_keyメソッドを呼び出す
	AnyPtr send2(const IDPtr& primary_key, const AnyPtr& secondary_key) const;

	/// \brief 関数を呼び出す
	AnyPtr call() const;

//{REPEAT{{
/*
	/// \brief primary_keyメソッドを呼び出す
	AnyPtr send(const IDPtr& primary_key, const Param& a0 #COMMA_REPEAT#const Param& a`i+1`#) const;

	/// \brief primary_key#secondary_keyメソッドを呼び出す
	AnyPtr send2(const IDPtr& primary_key, const AnyPtr& secondary_key, const Param& a0 #COMMA_REPEAT#const Param& a`i+1`#) const;

	/// \brief 関数を呼び出す
	AnyPtr call(const Param& a0 #COMMA_REPEAT#const Param& a`i+1`#) const;
*/

	/// \brief primary_keyメソッドを呼び出す
	AnyPtr send(const IDPtr& primary_key, const Param& a0 ) const;

	/// \brief primary_key#secondary_keyメソッドを呼び出す
	AnyPtr send2(const IDPtr& primary_key, const AnyPtr& secondary_key, const Param& a0 ) const;

	/// \brief 関数を呼び出す
	AnyPtr call(const Param& a0 ) const;

	/// \brief primary_keyメソッドを呼び出す
	AnyPtr send(const IDPtr& primary_key, const Param& a0 , const Param& a1) const;

	/// \brief primary_key#secondary_keyメソッドを呼び出す
	AnyPtr send2(const IDPtr& primary_key, const AnyPtr& secondary_key, const Param& a0 , const Param& a1) const;

	/// \brief 関数を呼び出す
	AnyPtr call(const Param& a0 , const Param& a1) const;

	/// \brief primary_keyメソッドを呼び出す
	AnyPtr send(const IDPtr& primary_key, const Param& a0 , const Param& a1, const Param& a2) const;

	/// \brief primary_key#secondary_keyメソッドを呼び出す
	AnyPtr send2(const IDPtr& primary_key, const AnyPtr& secondary_key, const Param& a0 , const Param& a1, const Param& a2) const;

	/// \brief 関数を呼び出す
	AnyPtr call(const Param& a0 , const Param& a1, const Param& a2) const;

	/// \brief primary_keyメソッドを呼び出す
	AnyPtr send(const IDPtr& primary_key, const Param& a0 , const Param& a1, const Param& a2, const Param& a3) const;

	/// \brief primary_key#secondary_keyメソッドを呼び出す
	AnyPtr send2(const IDPtr& primary_key, const AnyPtr& secondary_key, const Param& a0 , const Param& a1, const Param& a2, const Param& a3) const;

	/// \brief 関数を呼び出す
	AnyPtr call(const Param& a0 , const Param& a1, const Param& a2, const Param& a3) const;

	/// \brief primary_keyメソッドを呼び出す
	AnyPtr send(const IDPtr& primary_key, const Param& a0 , const Param& a1, const Param& a2, const Param& a3, const Param& a4) const;

	/// \brief primary_key#secondary_keyメソッドを呼び出す
	AnyPtr send2(const IDPtr& primary_key, const AnyPtr& secondary_key, const Param& a0 , const Param& a1, const Param& a2, const Param& a3, const Param& a4) const;

	/// \brief 関数を呼び出す
	AnyPtr call(const Param& a0 , const Param& a1, const Param& a2, const Param& a3, const Param& a4) const;

	/// \brief primary_keyメソッドを呼び出す
	AnyPtr send(const IDPtr& primary_key, const Param& a0 , const Param& a1, const Param& a2, const Param& a3, const Param& a4, const Param& a5) const;

	/// \brief primary_key#secondary_keyメソッドを呼び出す
	AnyPtr send2(const IDPtr& primary_key, const AnyPtr& secondary_key, const Param& a0 , const Param& a1, const Param& a2, const Param& a3, const Param& a4, const Param& a5) const;

	/// \brief 関数を呼び出す
	AnyPtr call(const Param& a0 , const Param& a1, const Param& a2, const Param& a3, const Param& a4, const Param& a5) const;

//}}REPEAT}

private:

	// call(VMachinePtr)がコンパイルエラーとなるように
	struct cmpitle_error{ cmpitle_error(const VMachinePtr& vm); };
	void call(cmpitle_error) const;

public:

	Any(){ type_ = TYPE_NULL; value_.pvalue = 0; }

	Any(char v){ type_ = TYPE_INT; value_.ivalue = (int_t)v; }
	Any(signed char v){ type_ = TYPE_INT; value_.ivalue = (int_t)v; }
	Any(unsigned char v){ type_ = TYPE_INT; value_.ivalue = (int_t)v; }
	Any(short v){ type_ = TYPE_INT; value_.ivalue = (int_t)v; }
	Any(unsigned short v){ type_ = TYPE_INT; value_.ivalue = (int_t)v; }
	Any(int v){ type_ = TYPE_INT; value_.ivalue = (int_t)v; }
	Any(unsigned int v){ type_ = TYPE_INT; value_.ivalue = (int_t)v; }
	Any(long v){ type_ = TYPE_INT; value_.ivalue = (int_t)v; }
	Any(unsigned long v){ type_ = TYPE_INT; value_.ivalue = (int_t)v; }

	Any(float v){ type_ = TYPE_FLOAT; value_.fvalue = (float_t)v; }
	Any(double v){ type_ = TYPE_FLOAT; value_.fvalue = (float_t)v; }
	Any(long double v){ type_ = TYPE_FLOAT; value_.fvalue = (float_t)v; }

	Any(bool b){ type_ = TYPE_FALSE + (int)b; value_.ivalue = 0; }
	Any(const Base* v){ type_ = TYPE_BASE; value_.pvalue = (Base*)v; }

	Any(PrimitiveType type){ type_ = type; value_.ivalue = 0; }

	struct noinit_t{};
	Any(noinit_t){}

private:

	StringPtr defined_place_name(const CodePtr& code, int_t pc, int_t name_number) const;

protected:

	int_t type_;
	AnyRawValue value_;

public:

	friend uint_t rawtype(const Any& v);
	friend const AnyRawValue& rawvalue(const Any& v);
	friend void set_type_value(Any& v, int_t type, const AnyRawValue& value);
	friend void set_type(Any& v, int_t type);
};

/////////////////////////////////////////////////////////


inline uint_t rawtype(const Any& v){
	return (uint_t)(v.type_);
}

inline int_t type(const Any& v){ 
	return rawtype(v) & TYPE_MASK; 
}

inline const AnyRawValue& rawvalue(const Any& v){
	return v.value_;
}

inline void set_type_value(Any& v, int_t type, const AnyRawValue& value){
	v.type_ = type;
	v.value_ = value;
}

inline void set_type(Any& v, int_t type){
	v.type_ = type;
}

inline int_t ivalue(const Any& v){ 
	//XTAL_ASSERT(type(v)==TYPE_INT);
	return rawvalue(v).ivalue; 
}

inline float_t fvalue(const Any& v){ 
	//XTAL_ASSERT(type(v)==TYPE_FLOAT); 
	return rawvalue(v).fvalue; 
}

inline Base* pvalue(const Any& v){ 
	XTAL_ASSERT(type(v)==TYPE_BASE || type(v)==TYPE_NULL); 
	return rawvalue(v).pvalue; 
}

inline RefCountingBase* rcpvalue(const Any& v){ 
	XTAL_ASSERT(type(v)>=TYPE_BASE || type(v)==TYPE_NULL); 
	return rawvalue(v).rcpvalue; 
}

inline bool is_undefined(const Any& a){
	return rawtype(a)==TYPE_UNDEFINED;
}

inline bool is_null(const Any& a){
	return rawtype(a)==TYPE_NULL;
}

inline bool raweq(const Any& a, const Any& b){
	return type(a)==type(b) && rawvalue(a).ivalue==rawvalue(b).ivalue;
}

inline bool rawne(const Any& a, const Any& b){
	return !raweq(a, b);
}

inline bool rawlt(const Any& a, const Any& b){
	if(type(a)<type(b))
		return true;
	if(type(b)<type(a))
		return false;
	return rawvalue(a).ivalue<rawvalue(b).ivalue;
}

inline uint_t rawbitxor(const Any& a, const Any& b){
	return ((rawtype(a)^rawtype(b))&TYPE_MASK) | (rawvalue(a).ivalue^rawvalue(b).ivalue);
}

inline void set_nullt(Any& a){
	set_type(a, TYPE_NULL);
}

inline void set_null(Any& a){
	AnyRawValue value; value.ivalue = 0;
	set_type_value(a, TYPE_NULL, value);
}

inline void set_undefined(Any& a){
	AnyRawValue value; value.ivalue = 0;
	set_type_value(a, TYPE_UNDEFINED, value);
}

inline void set_pvalue(Any& a, const Base* p){
	XTAL_ASSERT(p!=0);
	AnyRawValue value; value.pvalue = (Base*)p;
	set_type_value(a, TYPE_BASE, value);
}

inline void set_ivalue(Any& a, int_t v){
	AnyRawValue value; value.ivalue = v;
	set_type_value(a, TYPE_INT, value);
}

inline void set_fvalue(Any& a, float_t v){
	AnyRawValue value; value.fvalue = v;
	set_type_value(a, TYPE_FLOAT, value);
}

inline void set_bvalue(Any& a, bool b){
	AnyRawValue value; value.ivalue = 0;
	set_type_value(a, TYPE_FALSE + (int)b, value);
}

inline void set_svalue(Any& a){
	AnyRawValue value; value.ivalue = 0;
	set_type_value(a, TYPE_SMALL_STRING, value);
}

inline void set_pvalue(Any& a, int_t type, const RefCountingBase* p){
	XTAL_ASSERT(p!=0);
	AnyRawValue value; value.rcpvalue = (RefCountingBase*)p;
	set_type_value(a, type, value);
}

inline void copy_any(Any& v, const Any& u){
	v = u;
}

inline void swap(Any& a, Any& b){
	std::swap(a, b);
}

}

#endif // XTAL_ANY_H_INCLUDE_GUARD
