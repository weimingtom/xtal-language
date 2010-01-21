/** \file src/xtal/xtal_any.h
* \brief src/xtal/xtal_any.h
*/

#ifndef XTAL_ANY_H_INCLUDE_GUARD
#define XTAL_ANY_H_INCLUDE_GUARD

#pragma once

namespace xtal{

enum{
	SMALL_STRING_MAX = sizeof(void*) / sizeof(char_t)
};

/*
struct AnyRawValue{
	enum{
		EMB_SHIFT = TYPE_SHIFT+1,
		EMB = 1<<EMB_SHIFT,

		HAVE_FINALIZER_FLAG_SHIFT = EMB_SHIFT+1,
		HAVE_FINALIZER_FLAG_BIT = 1<<HAVE_FINALIZER_FLAG_SHIFT,

		REF_COUNT_SHIFT = HAVE_FINALIZER_FLAG_SHIFT+1,
		REF_COUNT_MASK = ~((1<<REF_COUNT_SHIFT)-1) & 0xffffffff,

		INT_SHIFT = 8,
		STR_SHIFT = 16,
		VALUE_SHIFT = 32
	};

	void set_ivalue(int_t v){ value = TYPE_INT | (v << INT_SHIFT); }

	template<class T> 
	void set_value(int type, T v){
		union{
			SelectType<sizeof(T)>::uint_t iv;
			T tv;
		} u;
		u.tv = v;
		value = type | ((long_t)iv << VALUE_SHIFT); 
	}

	template<class T> 
	T get_value() const{
		union{
			SelectType<sizeof(T)>::uint_t iv;
			T tv;
		} u;
		u.iv = (SelectType<sizeof(T)>::uint_t)(value>>VALUE_SHIFT);
		return u.tv;
	}

	void init(PrimitiveType t){ value = t; }
	void init(char v){ set_ivalue(v); }
	void init(signed char v){ set_ivalue(v); }
	void init(unsigned char v){ set_ivalue(v); }
	void init(short v){ set_ivalue(v); }
	void init(unsigned short v){ set_ivalue(v); }
	void init(int v){ set_ivalue(v); }
	void init(unsigned int v){ set_ivalue(v); }
	void init(long v){ set_ivalue(v); }
	void init(unsigned long v){ set_ivalue(v); }
	void init(long long v){ set_ivalue(v); }
	void init(unsigned long long v){ set_ivalue(v); }

	void init(float v){ set_value(TYPE_FLOAT, (float_t)v); }
	void init(double v){ set_value(TYPE_FLOAT, (float_t)v); }
	void init(long double v){ set_value(TYPE_FLOAT, (float_t)v); }

	void init(bool b){ value = TYPE_FALSE + (int)b; }
	void init(const Base* v){ set_value(TYPE_BASE | EMB, v); }
	void init(int t, const RefCountingBase* v){ set_value(t | EMB, v); }

	bool have_finalizer(){ return (value & HAVE_FINALIZER_FLAG_BIT)!=0; }
	void set_finalizer_flag(){ value |= HAVE_FINALIZER_FLAG_BIT; }

	uint_t ref_count(){ return (value & REF_COUNT_MASK)>>REF_COUNT_SHIFT; }
	void add_ref_count(int_t rc){ value += rc<<REF_COUNT_SHIFT; }
	void inc_ref_count(){ value += 1<<REF_COUNT_SHIFT; }
	void dec_ref_count(){ value -= 1<<REF_COUNT_SHIFT; }

	uint_t ungc(){ return value&(REF_COUNT_MASK|HAVE_FINALIZER_FLAG_BIT); }

	int t() const{ return (int)value; }
	int_t i() const{ return (int_t)(value>>INT_SHIFT); }
	uint_t u() const{ return (uint_t)(value>>INT_SHIFT); }
	float_t f() const{ return get_value<float_t>(); }
	Base* p() const{ return get_value<Base*>(); }
	RefCountingBase* r() const{ return get_value<RefCountingBase*>(); }
	
	char_t* s(){ union{ u8 u8v[2]; u16 u16v; } u; u.u16v = 1; ((u8*)&value)[u.u8v[0]]; }
	const char_t* s() const{ union{ u8 u8v[2]; u16 u16v; } u; u.u16v = 1; ((u8*)&value)[u.u8v[0]]; }

	void set_object_size(int sz){ set_value<int>(t(), sz); }
	int object_size(){ return get_value<int>(); }
	
	u64 v() const{ return value & ~(u64)(((value&EMB) - (1<<EMB)) & 0xffffffff); }

	friend bool raweq(const AnyRawValue& a, const AnyRawValue& b){
		return a.v()==b.v();
	}

	friend bool rawlt(const AnyRawValue& a, const AnyRawValue& b){
		return a.v()<b.v();
	}

	friend uint_t rawbitxor(const AnyRawValue& a, const AnyRawValue& b){
		u64 t = a.v() ^ b.v();
		return (u32)t | (u32)(t>>32);
	}

private:
	u64 value;
};
*/

struct AnyRawValue{
	void init(PrimitiveType t){ type = t; ivalue = 0; }
	void init(char v){ type = TYPE_INT; ivalue = (int_t)v; }
	void init(signed char v){ type = TYPE_INT; ivalue = (int_t)v; }
	void init(unsigned char v){ type = TYPE_INT; ivalue = (int_t)v; }
	void init(short v){ type = TYPE_INT; ivalue = (int_t)v; }
	void init(unsigned short v){ type = TYPE_INT; ivalue = (int_t)v; }
	void init(int v){ type = TYPE_INT; ivalue = (int_t)v; }
	void init(unsigned int v){ type = TYPE_INT; ivalue = (int_t)v; }
	void init(long v){ type = TYPE_INT; ivalue = (int_t)v; }
	void init(unsigned long v){ type = TYPE_INT; ivalue = (int_t)v; }
	void init(long long v){ type = TYPE_INT; ivalue = (int_t)v; }
	void init(unsigned long long v){ type = TYPE_INT; ivalue = (int_t)v; }

	void init(float v){ type = TYPE_FLOAT; fvalue = (float_t)v; }
	void init(double v){ type = TYPE_FLOAT; fvalue = (float_t)v; }
	void init(long double v){ type = TYPE_FLOAT; fvalue = (float_t)v; }

	void init(bool b){ type = TYPE_FALSE + (int)b; ivalue = 0; }
	void init(const Base* v){  type = TYPE_BASE; pvalue = const_cast<Base*>(v); }
	void init(int t, const RefCountingBase* v){ type = t; rcpvalue = const_cast<RefCountingBase*>(v); }

	void init_string_literal(int_t t, const StringLiteral& v){ 
		type = t | (v.size()<<STRING_SIZE_SHIFT); 
		spvalue = v.str(); 
	}

	void init_small_string(uint_t size){ 
		type = TYPE_SMALL_STRING | (size<<STRING_SIZE_SHIFT);
		ivalue = 0;	
	}

	void init_small_string(const char_t* str, uint_t size){ 
		type = TYPE_SMALL_STRING | (size<<STRING_SIZE_SHIFT);
		ivalue = 0;
		for(uint_t i=0; i<size; ++i){
			svalue[i] = str[i];
		}
	}

	void init_pointer(const void* v, uint_t i){ 
		type = TYPE_POINTER | (i<<CPP_CLASS_INDEX_SHIFT); 
		vpvalue = const_cast<void*>(v);
	}

public:

	enum{
		HAVE_FINALIZER_FLAG_SHIFT = TYPE_SHIFT+1,
		HAVE_FINALIZER_FLAG_BIT = 1<<HAVE_FINALIZER_FLAG_SHIFT,

		REF_COUNT_SHIFT = HAVE_FINALIZER_FLAG_SHIFT+1,
		REF_COUNT_MASK = ~((1<<REF_COUNT_SHIFT)-1),

		STRING_SIZE_SHIFT = TYPE_SHIFT+1,
		STRING_SIZE_MASK = ~((1<<STRING_SIZE_SHIFT)-1),

		CPP_CLASS_INDEX_SHIFT = TYPE_SHIFT+1,
		CPP_CLASS_INDEX_MASK = ~((1<<CPP_CLASS_INDEX_SHIFT)-1)
	};

	bool have_finalizer() const{ return (type & HAVE_FINALIZER_FLAG_BIT)!=0; }
	void set_finalizer_flag(){ type |= HAVE_FINALIZER_FLAG_BIT; }
	void unset_finalizer_flag(){ type &= ~HAVE_FINALIZER_FLAG_BIT; }

	uint_t ref_count() const{ return (type & REF_COUNT_MASK)>>REF_COUNT_SHIFT; }
	void add_ref_count(int_t rc){ type += rc<<REF_COUNT_SHIFT; }
	void inc_ref_count(){ type += 1<<REF_COUNT_SHIFT; }
	void dec_ref_count(){ type -= 1<<REF_COUNT_SHIFT; }
	uint_t can_not_gc() const{ return type&(REF_COUNT_MASK|HAVE_FINALIZER_FLAG_BIT); }

	uint_t string_size() const{ return (type & STRING_SIZE_MASK)>>STRING_SIZE_SHIFT; }
	uint_t string_literal_size() const{ return (type & STRING_SIZE_MASK)>>STRING_SIZE_SHIFT; }
	uint_t cpp_class_index() const{ return (type & CPP_CLASS_INDEX_MASK)>>CPP_CLASS_INDEX_SHIFT; }
public:

	int_t t() const{ return type; }
	int_t i() const{ return ivalue; }
	uint_t u() const{ return uvalue; }
	float_t f() const{ return fvalue; }
	Base* p() const{ return pvalue; }
	RefCountingBase* r() const{ return rcpvalue; }
	char_t* s(){ return svalue; }
	const char_t* s() const{ return svalue; }
	const char_t* sp() const{ return spvalue; }
	void* vp() const{ return vpvalue; }
	
	friend bool raweq(const AnyRawValue& a, const AnyRawValue& b){
		return (a.type&TYPE_MASK)==(b.type&TYPE_MASK) && a.ivalue==b.ivalue;
	}

	friend bool rawlt(const AnyRawValue& a, const AnyRawValue& b){
		int atype = a.type&TYPE_MASK;
		int btype = b.type&TYPE_MASK;
		if(atype<btype)return true;
		if(btype<atype)return false;
		return a.ivalue<b.ivalue;
	}

	friend uint_t rawbitxor(const AnyRawValue& a, const AnyRawValue& b){
		return ((a.type^b.type)&TYPE_MASK) | (a.ivalue^b.ivalue);
	}

private:
	int_t type;
	union{
		int_t ivalue;
		uint_t uvalue;
		float_t fvalue;
		Base* pvalue;
		RefCountingBase* rcpvalue;
		const char_t* spvalue;
		char_t svalue[SMALL_STRING_MAX];
		void* vpvalue;
	};
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

	bool is(CppClassSymbolData* key) const;
		
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
	* \brief オブジェクトの名前を返す。
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
	* 文字列化にはto_sメソッドが呼ばれる。
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

	bool op_eq( const AnyPtr& v ) const;

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
	template<class A0 #COMMA_REPEAT#class A`i+1`#>
	AnyPtr send(const IDPtr& primary_key, const A0& a0  #COMMA_REPEAT#const A`i+1`& a`i+1`#) const{
		const VMachinePtr& vm = setup_call();
		vm->push_arg(a0); #REPEAT#vm->push_arg(a`i+1`); #
		return private_send(vm, primary_key);	
	}

	/// \brief primary_key#secondary_keyメソッドを呼び出す
	template<class A0 #COMMA_REPEAT#class A`i+1`#>
	AnyPtr send2(const IDPtr& primary_key, const AnyPtr& secondary_key, const A0& a0  #COMMA_REPEAT#const A`i+1`& a`i+1`#) const{
		const VMachinePtr& vm = setup_call();
		vm->push_arg(a0); #REPEAT#vm->push_arg(a`i+1`); #
		return private_send(vm, primary_key, secondary_key);	
	}

	/// \brief 関数を呼び出す
	template<class A0 #COMMA_REPEAT#class A`i+1`#>
	AnyPtr call(const A0& a0  #COMMA_REPEAT#const A`i+1`& a`i+1`#) const{
		const VMachinePtr& vm = setup_call();
		vm->push_arg(a0); #REPEAT#vm->push_arg(a`i+1`); #
		return private_call(vm);	
	}
*/

	/// \brief primary_keyメソッドを呼び出す
	template<class A0 >
	AnyPtr send(const IDPtr& primary_key, const A0& a0  ) const{
		const VMachinePtr& vm = setup_call();
		vm->push_arg(a0); 
		return private_send(vm, primary_key);	
	}

	/// \brief primary_key#secondary_keyメソッドを呼び出す
	template<class A0 >
	AnyPtr send2(const IDPtr& primary_key, const AnyPtr& secondary_key, const A0& a0  ) const{
		const VMachinePtr& vm = setup_call();
		vm->push_arg(a0); 
		return private_send(vm, primary_key, secondary_key);	
	}

	/// \brief 関数を呼び出す
	template<class A0 >
	AnyPtr call(const A0& a0  ) const{
		const VMachinePtr& vm = setup_call();
		vm->push_arg(a0); 
		return private_call(vm);	
	}

	/// \brief primary_keyメソッドを呼び出す
	template<class A0 , class A1>
	AnyPtr send(const IDPtr& primary_key, const A0& a0  , const A1& a1) const{
		const VMachinePtr& vm = setup_call();
		vm->push_arg(a0); vm->push_arg(a1); 
		return private_send(vm, primary_key);	
	}

	/// \brief primary_key#secondary_keyメソッドを呼び出す
	template<class A0 , class A1>
	AnyPtr send2(const IDPtr& primary_key, const AnyPtr& secondary_key, const A0& a0  , const A1& a1) const{
		const VMachinePtr& vm = setup_call();
		vm->push_arg(a0); vm->push_arg(a1); 
		return private_send(vm, primary_key, secondary_key);	
	}

	/// \brief 関数を呼び出す
	template<class A0 , class A1>
	AnyPtr call(const A0& a0  , const A1& a1) const{
		const VMachinePtr& vm = setup_call();
		vm->push_arg(a0); vm->push_arg(a1); 
		return private_call(vm);	
	}

	/// \brief primary_keyメソッドを呼び出す
	template<class A0 , class A1, class A2>
	AnyPtr send(const IDPtr& primary_key, const A0& a0  , const A1& a1, const A2& a2) const{
		const VMachinePtr& vm = setup_call();
		vm->push_arg(a0); vm->push_arg(a1); vm->push_arg(a2); 
		return private_send(vm, primary_key);	
	}

	/// \brief primary_key#secondary_keyメソッドを呼び出す
	template<class A0 , class A1, class A2>
	AnyPtr send2(const IDPtr& primary_key, const AnyPtr& secondary_key, const A0& a0  , const A1& a1, const A2& a2) const{
		const VMachinePtr& vm = setup_call();
		vm->push_arg(a0); vm->push_arg(a1); vm->push_arg(a2); 
		return private_send(vm, primary_key, secondary_key);	
	}

	/// \brief 関数を呼び出す
	template<class A0 , class A1, class A2>
	AnyPtr call(const A0& a0  , const A1& a1, const A2& a2) const{
		const VMachinePtr& vm = setup_call();
		vm->push_arg(a0); vm->push_arg(a1); vm->push_arg(a2); 
		return private_call(vm);	
	}

	/// \brief primary_keyメソッドを呼び出す
	template<class A0 , class A1, class A2, class A3>
	AnyPtr send(const IDPtr& primary_key, const A0& a0  , const A1& a1, const A2& a2, const A3& a3) const{
		const VMachinePtr& vm = setup_call();
		vm->push_arg(a0); vm->push_arg(a1); vm->push_arg(a2); vm->push_arg(a3); 
		return private_send(vm, primary_key);	
	}

	/// \brief primary_key#secondary_keyメソッドを呼び出す
	template<class A0 , class A1, class A2, class A3>
	AnyPtr send2(const IDPtr& primary_key, const AnyPtr& secondary_key, const A0& a0  , const A1& a1, const A2& a2, const A3& a3) const{
		const VMachinePtr& vm = setup_call();
		vm->push_arg(a0); vm->push_arg(a1); vm->push_arg(a2); vm->push_arg(a3); 
		return private_send(vm, primary_key, secondary_key);	
	}

	/// \brief 関数を呼び出す
	template<class A0 , class A1, class A2, class A3>
	AnyPtr call(const A0& a0  , const A1& a1, const A2& a2, const A3& a3) const{
		const VMachinePtr& vm = setup_call();
		vm->push_arg(a0); vm->push_arg(a1); vm->push_arg(a2); vm->push_arg(a3); 
		return private_call(vm);	
	}

	/// \brief primary_keyメソッドを呼び出す
	template<class A0 , class A1, class A2, class A3, class A4>
	AnyPtr send(const IDPtr& primary_key, const A0& a0  , const A1& a1, const A2& a2, const A3& a3, const A4& a4) const{
		const VMachinePtr& vm = setup_call();
		vm->push_arg(a0); vm->push_arg(a1); vm->push_arg(a2); vm->push_arg(a3); vm->push_arg(a4); 
		return private_send(vm, primary_key);	
	}

	/// \brief primary_key#secondary_keyメソッドを呼び出す
	template<class A0 , class A1, class A2, class A3, class A4>
	AnyPtr send2(const IDPtr& primary_key, const AnyPtr& secondary_key, const A0& a0  , const A1& a1, const A2& a2, const A3& a3, const A4& a4) const{
		const VMachinePtr& vm = setup_call();
		vm->push_arg(a0); vm->push_arg(a1); vm->push_arg(a2); vm->push_arg(a3); vm->push_arg(a4); 
		return private_send(vm, primary_key, secondary_key);	
	}

	/// \brief 関数を呼び出す
	template<class A0 , class A1, class A2, class A3, class A4>
	AnyPtr call(const A0& a0  , const A1& a1, const A2& a2, const A3& a3, const A4& a4) const{
		const VMachinePtr& vm = setup_call();
		vm->push_arg(a0); vm->push_arg(a1); vm->push_arg(a2); vm->push_arg(a3); vm->push_arg(a4); 
		return private_call(vm);	
	}

	/// \brief primary_keyメソッドを呼び出す
	template<class A0 , class A1, class A2, class A3, class A4, class A5>
	AnyPtr send(const IDPtr& primary_key, const A0& a0  , const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5) const{
		const VMachinePtr& vm = setup_call();
		vm->push_arg(a0); vm->push_arg(a1); vm->push_arg(a2); vm->push_arg(a3); vm->push_arg(a4); vm->push_arg(a5); 
		return private_send(vm, primary_key);	
	}

	/// \brief primary_key#secondary_keyメソッドを呼び出す
	template<class A0 , class A1, class A2, class A3, class A4, class A5>
	AnyPtr send2(const IDPtr& primary_key, const AnyPtr& secondary_key, const A0& a0  , const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5) const{
		const VMachinePtr& vm = setup_call();
		vm->push_arg(a0); vm->push_arg(a1); vm->push_arg(a2); vm->push_arg(a3); vm->push_arg(a4); vm->push_arg(a5); 
		return private_send(vm, primary_key, secondary_key);	
	}

	/// \brief 関数を呼び出す
	template<class A0 , class A1, class A2, class A3, class A4, class A5>
	AnyPtr call(const A0& a0  , const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5) const{
		const VMachinePtr& vm = setup_call();
		vm->push_arg(a0); vm->push_arg(a1); vm->push_arg(a2); vm->push_arg(a3); vm->push_arg(a4); vm->push_arg(a5); 
		return private_call(vm);	
	}

	/// \brief primary_keyメソッドを呼び出す
	template<class A0 , class A1, class A2, class A3, class A4, class A5, class A6>
	AnyPtr send(const IDPtr& primary_key, const A0& a0  , const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6) const{
		const VMachinePtr& vm = setup_call();
		vm->push_arg(a0); vm->push_arg(a1); vm->push_arg(a2); vm->push_arg(a3); vm->push_arg(a4); vm->push_arg(a5); vm->push_arg(a6); 
		return private_send(vm, primary_key);	
	}

	/// \brief primary_key#secondary_keyメソッドを呼び出す
	template<class A0 , class A1, class A2, class A3, class A4, class A5, class A6>
	AnyPtr send2(const IDPtr& primary_key, const AnyPtr& secondary_key, const A0& a0  , const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6) const{
		const VMachinePtr& vm = setup_call();
		vm->push_arg(a0); vm->push_arg(a1); vm->push_arg(a2); vm->push_arg(a3); vm->push_arg(a4); vm->push_arg(a5); vm->push_arg(a6); 
		return private_send(vm, primary_key, secondary_key);	
	}

	/// \brief 関数を呼び出す
	template<class A0 , class A1, class A2, class A3, class A4, class A5, class A6>
	AnyPtr call(const A0& a0  , const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6) const{
		const VMachinePtr& vm = setup_call();
		vm->push_arg(a0); vm->push_arg(a1); vm->push_arg(a2); vm->push_arg(a3); vm->push_arg(a4); vm->push_arg(a5); vm->push_arg(a6); 
		return private_call(vm);	
	}

	/// \brief primary_keyメソッドを呼び出す
	template<class A0 , class A1, class A2, class A3, class A4, class A5, class A6, class A7>
	AnyPtr send(const IDPtr& primary_key, const A0& a0  , const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7) const{
		const VMachinePtr& vm = setup_call();
		vm->push_arg(a0); vm->push_arg(a1); vm->push_arg(a2); vm->push_arg(a3); vm->push_arg(a4); vm->push_arg(a5); vm->push_arg(a6); vm->push_arg(a7); 
		return private_send(vm, primary_key);	
	}

	/// \brief primary_key#secondary_keyメソッドを呼び出す
	template<class A0 , class A1, class A2, class A3, class A4, class A5, class A6, class A7>
	AnyPtr send2(const IDPtr& primary_key, const AnyPtr& secondary_key, const A0& a0  , const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7) const{
		const VMachinePtr& vm = setup_call();
		vm->push_arg(a0); vm->push_arg(a1); vm->push_arg(a2); vm->push_arg(a3); vm->push_arg(a4); vm->push_arg(a5); vm->push_arg(a6); vm->push_arg(a7); 
		return private_send(vm, primary_key, secondary_key);	
	}

	/// \brief 関数を呼び出す
	template<class A0 , class A1, class A2, class A3, class A4, class A5, class A6, class A7>
	AnyPtr call(const A0& a0  , const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7) const{
		const VMachinePtr& vm = setup_call();
		vm->push_arg(a0); vm->push_arg(a1); vm->push_arg(a2); vm->push_arg(a3); vm->push_arg(a4); vm->push_arg(a5); vm->push_arg(a6); vm->push_arg(a7); 
		return private_call(vm);	
	}

	/// \brief primary_keyメソッドを呼び出す
	template<class A0 , class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8>
	AnyPtr send(const IDPtr& primary_key, const A0& a0  , const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8) const{
		const VMachinePtr& vm = setup_call();
		vm->push_arg(a0); vm->push_arg(a1); vm->push_arg(a2); vm->push_arg(a3); vm->push_arg(a4); vm->push_arg(a5); vm->push_arg(a6); vm->push_arg(a7); vm->push_arg(a8); 
		return private_send(vm, primary_key);	
	}

	/// \brief primary_key#secondary_keyメソッドを呼び出す
	template<class A0 , class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8>
	AnyPtr send2(const IDPtr& primary_key, const AnyPtr& secondary_key, const A0& a0  , const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8) const{
		const VMachinePtr& vm = setup_call();
		vm->push_arg(a0); vm->push_arg(a1); vm->push_arg(a2); vm->push_arg(a3); vm->push_arg(a4); vm->push_arg(a5); vm->push_arg(a6); vm->push_arg(a7); vm->push_arg(a8); 
		return private_send(vm, primary_key, secondary_key);	
	}

	/// \brief 関数を呼び出す
	template<class A0 , class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8>
	AnyPtr call(const A0& a0  , const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8) const{
		const VMachinePtr& vm = setup_call();
		vm->push_arg(a0); vm->push_arg(a1); vm->push_arg(a2); vm->push_arg(a3); vm->push_arg(a4); vm->push_arg(a5); vm->push_arg(a6); vm->push_arg(a7); vm->push_arg(a8); 
		return private_call(vm);	
	}

	/// \brief primary_keyメソッドを呼び出す
	template<class A0 , class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9>
	AnyPtr send(const IDPtr& primary_key, const A0& a0  , const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8, const A9& a9) const{
		const VMachinePtr& vm = setup_call();
		vm->push_arg(a0); vm->push_arg(a1); vm->push_arg(a2); vm->push_arg(a3); vm->push_arg(a4); vm->push_arg(a5); vm->push_arg(a6); vm->push_arg(a7); vm->push_arg(a8); vm->push_arg(a9); 
		return private_send(vm, primary_key);	
	}

	/// \brief primary_key#secondary_keyメソッドを呼び出す
	template<class A0 , class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9>
	AnyPtr send2(const IDPtr& primary_key, const AnyPtr& secondary_key, const A0& a0  , const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8, const A9& a9) const{
		const VMachinePtr& vm = setup_call();
		vm->push_arg(a0); vm->push_arg(a1); vm->push_arg(a2); vm->push_arg(a3); vm->push_arg(a4); vm->push_arg(a5); vm->push_arg(a6); vm->push_arg(a7); vm->push_arg(a8); vm->push_arg(a9); 
		return private_send(vm, primary_key, secondary_key);	
	}

	/// \brief 関数を呼び出す
	template<class A0 , class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9>
	AnyPtr call(const A0& a0  , const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8, const A9& a9) const{
		const VMachinePtr& vm = setup_call();
		vm->push_arg(a0); vm->push_arg(a1); vm->push_arg(a2); vm->push_arg(a3); vm->push_arg(a4); vm->push_arg(a5); vm->push_arg(a6); vm->push_arg(a7); vm->push_arg(a8); vm->push_arg(a9); 
		return private_call(vm);	
	}

	/// \brief primary_keyメソッドを呼び出す
	template<class A0 , class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10>
	AnyPtr send(const IDPtr& primary_key, const A0& a0  , const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8, const A9& a9, const A10& a10) const{
		const VMachinePtr& vm = setup_call();
		vm->push_arg(a0); vm->push_arg(a1); vm->push_arg(a2); vm->push_arg(a3); vm->push_arg(a4); vm->push_arg(a5); vm->push_arg(a6); vm->push_arg(a7); vm->push_arg(a8); vm->push_arg(a9); vm->push_arg(a10); 
		return private_send(vm, primary_key);	
	}

	/// \brief primary_key#secondary_keyメソッドを呼び出す
	template<class A0 , class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10>
	AnyPtr send2(const IDPtr& primary_key, const AnyPtr& secondary_key, const A0& a0  , const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8, const A9& a9, const A10& a10) const{
		const VMachinePtr& vm = setup_call();
		vm->push_arg(a0); vm->push_arg(a1); vm->push_arg(a2); vm->push_arg(a3); vm->push_arg(a4); vm->push_arg(a5); vm->push_arg(a6); vm->push_arg(a7); vm->push_arg(a8); vm->push_arg(a9); vm->push_arg(a10); 
		return private_send(vm, primary_key, secondary_key);	
	}

	/// \brief 関数を呼び出す
	template<class A0 , class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10>
	AnyPtr call(const A0& a0  , const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8, const A9& a9, const A10& a10) const{
		const VMachinePtr& vm = setup_call();
		vm->push_arg(a0); vm->push_arg(a1); vm->push_arg(a2); vm->push_arg(a3); vm->push_arg(a4); vm->push_arg(a5); vm->push_arg(a6); vm->push_arg(a7); vm->push_arg(a8); vm->push_arg(a9); vm->push_arg(a10); 
		return private_call(vm);	
	}

	/// \brief primary_keyメソッドを呼び出す
	template<class A0 , class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11>
	AnyPtr send(const IDPtr& primary_key, const A0& a0  , const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8, const A9& a9, const A10& a10, const A11& a11) const{
		const VMachinePtr& vm = setup_call();
		vm->push_arg(a0); vm->push_arg(a1); vm->push_arg(a2); vm->push_arg(a3); vm->push_arg(a4); vm->push_arg(a5); vm->push_arg(a6); vm->push_arg(a7); vm->push_arg(a8); vm->push_arg(a9); vm->push_arg(a10); vm->push_arg(a11); 
		return private_send(vm, primary_key);	
	}

	/// \brief primary_key#secondary_keyメソッドを呼び出す
	template<class A0 , class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11>
	AnyPtr send2(const IDPtr& primary_key, const AnyPtr& secondary_key, const A0& a0  , const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8, const A9& a9, const A10& a10, const A11& a11) const{
		const VMachinePtr& vm = setup_call();
		vm->push_arg(a0); vm->push_arg(a1); vm->push_arg(a2); vm->push_arg(a3); vm->push_arg(a4); vm->push_arg(a5); vm->push_arg(a6); vm->push_arg(a7); vm->push_arg(a8); vm->push_arg(a9); vm->push_arg(a10); vm->push_arg(a11); 
		return private_send(vm, primary_key, secondary_key);	
	}

	/// \brief 関数を呼び出す
	template<class A0 , class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11>
	AnyPtr call(const A0& a0  , const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8, const A9& a9, const A10& a10, const A11& a11) const{
		const VMachinePtr& vm = setup_call();
		vm->push_arg(a0); vm->push_arg(a1); vm->push_arg(a2); vm->push_arg(a3); vm->push_arg(a4); vm->push_arg(a5); vm->push_arg(a6); vm->push_arg(a7); vm->push_arg(a8); vm->push_arg(a9); vm->push_arg(a10); vm->push_arg(a11); 
		return private_call(vm);	
	}

	/// \brief primary_keyメソッドを呼び出す
	template<class A0 , class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12>
	AnyPtr send(const IDPtr& primary_key, const A0& a0  , const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8, const A9& a9, const A10& a10, const A11& a11, const A12& a12) const{
		const VMachinePtr& vm = setup_call();
		vm->push_arg(a0); vm->push_arg(a1); vm->push_arg(a2); vm->push_arg(a3); vm->push_arg(a4); vm->push_arg(a5); vm->push_arg(a6); vm->push_arg(a7); vm->push_arg(a8); vm->push_arg(a9); vm->push_arg(a10); vm->push_arg(a11); vm->push_arg(a12); 
		return private_send(vm, primary_key);	
	}

	/// \brief primary_key#secondary_keyメソッドを呼び出す
	template<class A0 , class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12>
	AnyPtr send2(const IDPtr& primary_key, const AnyPtr& secondary_key, const A0& a0  , const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8, const A9& a9, const A10& a10, const A11& a11, const A12& a12) const{
		const VMachinePtr& vm = setup_call();
		vm->push_arg(a0); vm->push_arg(a1); vm->push_arg(a2); vm->push_arg(a3); vm->push_arg(a4); vm->push_arg(a5); vm->push_arg(a6); vm->push_arg(a7); vm->push_arg(a8); vm->push_arg(a9); vm->push_arg(a10); vm->push_arg(a11); vm->push_arg(a12); 
		return private_send(vm, primary_key, secondary_key);	
	}

	/// \brief 関数を呼び出す
	template<class A0 , class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12>
	AnyPtr call(const A0& a0  , const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8, const A9& a9, const A10& a10, const A11& a11, const A12& a12) const{
		const VMachinePtr& vm = setup_call();
		vm->push_arg(a0); vm->push_arg(a1); vm->push_arg(a2); vm->push_arg(a3); vm->push_arg(a4); vm->push_arg(a5); vm->push_arg(a6); vm->push_arg(a7); vm->push_arg(a8); vm->push_arg(a9); vm->push_arg(a10); vm->push_arg(a11); vm->push_arg(a12); 
		return private_call(vm);	
	}

	/// \brief primary_keyメソッドを呼び出す
	template<class A0 , class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13>
	AnyPtr send(const IDPtr& primary_key, const A0& a0  , const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8, const A9& a9, const A10& a10, const A11& a11, const A12& a12, const A13& a13) const{
		const VMachinePtr& vm = setup_call();
		vm->push_arg(a0); vm->push_arg(a1); vm->push_arg(a2); vm->push_arg(a3); vm->push_arg(a4); vm->push_arg(a5); vm->push_arg(a6); vm->push_arg(a7); vm->push_arg(a8); vm->push_arg(a9); vm->push_arg(a10); vm->push_arg(a11); vm->push_arg(a12); vm->push_arg(a13); 
		return private_send(vm, primary_key);	
	}

	/// \brief primary_key#secondary_keyメソッドを呼び出す
	template<class A0 , class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13>
	AnyPtr send2(const IDPtr& primary_key, const AnyPtr& secondary_key, const A0& a0  , const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8, const A9& a9, const A10& a10, const A11& a11, const A12& a12, const A13& a13) const{
		const VMachinePtr& vm = setup_call();
		vm->push_arg(a0); vm->push_arg(a1); vm->push_arg(a2); vm->push_arg(a3); vm->push_arg(a4); vm->push_arg(a5); vm->push_arg(a6); vm->push_arg(a7); vm->push_arg(a8); vm->push_arg(a9); vm->push_arg(a10); vm->push_arg(a11); vm->push_arg(a12); vm->push_arg(a13); 
		return private_send(vm, primary_key, secondary_key);	
	}

	/// \brief 関数を呼び出す
	template<class A0 , class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13>
	AnyPtr call(const A0& a0  , const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8, const A9& a9, const A10& a10, const A11& a11, const A12& a12, const A13& a13) const{
		const VMachinePtr& vm = setup_call();
		vm->push_arg(a0); vm->push_arg(a1); vm->push_arg(a2); vm->push_arg(a3); vm->push_arg(a4); vm->push_arg(a5); vm->push_arg(a6); vm->push_arg(a7); vm->push_arg(a8); vm->push_arg(a9); vm->push_arg(a10); vm->push_arg(a11); vm->push_arg(a12); vm->push_arg(a13); 
		return private_call(vm);	
	}

//}}REPEAT}

private:

	AnyPtr private_send(const VMachinePtr& vm, const IDPtr& primary_key) const;
	AnyPtr private_send2(const VMachinePtr& vm, const IDPtr& primary_key, const AnyPtr& secondary_key) const;
	AnyPtr private_call(const VMachinePtr& vm) const;

private:

	// call(VMachinePtr)がコンパイルエラーとなるように
	struct cmpitle_error{ cmpitle_error(const VMachinePtr& vm); };
	void call(cmpitle_error) const;

public:

	Any(){ value_.init(TYPE_NULL); }

	Any(char v){ value_.init(v); }
	Any(signed char v){ value_.init(v); }
	Any(unsigned char v){ value_.init(v); }
	Any(short v){ value_.init(v); }
	Any(unsigned short v){ value_.init(v); }
	Any(int v){ value_.init(v); }
	Any(unsigned int v){ value_.init(v); }
	Any(long v){ value_.init(v); }
	Any(unsigned long v){ value_.init(v); }
	Any(long long v){ value_.init(v); }
	Any(unsigned long long v){ value_.init(v); }

	Any(float v){ value_.init(v); }
	Any(double v){ value_.init(v); }
	Any(long double v){ value_.init(v); }

	Any(bool v){ value_.init(v); }
	Any(const Base* v){ value_.init(v); }

	Any(PrimitiveType v){ value_.init(v); }

	Any(const AnyRawValue& v){ value_ = v; }

	struct noinit_t{};
	Any(noinit_t){}

private:

	StringPtr defined_place_name(const CodePtr& code, int_t pc, int_t name_number) const;

protected:

	AnyRawValue value_;

public:

	friend const AnyRawValue& rawvalue(const Any& v);
	friend AnyRawValue& rawvalue(Any& v);
};

class UninitializedAny : public Any{
public:
	UninitializedAny():Any(noinit_t()){}
};

/////////////////////////////////////////////////////////

inline const AnyRawValue& rawvalue(const Any& v){
	return v.value_;
}

inline AnyRawValue& rawvalue(Any& v){
	return v.value_;
}

inline uint_t rawtype(const Any& v){
	return (uint_t)(rawvalue(v).t());
}

inline int_t type(const Any& v){ 
	return rawtype(v) & TYPE_MASK; 
}

inline int_t ivalue(const Any& v){ 
	return rawvalue(v).i(); 
}

inline float_t fvalue(const Any& v){ 
	return rawvalue(v).f(); 
}

inline int_t chvalue(const Any& v){ 
	return rawvalue(v).s()[0];
}

inline Base* pvalue(const Any& v){ 
	XTAL_ASSERT(type(v)==TYPE_BASE || type(v)==TYPE_NULL); 
	return rawvalue(v).p(); 
}

inline RefCountingBase* rcpvalue(const Any& v){ 
	XTAL_ASSERT(type(v)>=TYPE_BASE || type(v)==TYPE_NULL); 
	return rawvalue(v).r(); 
}

inline bool is_undefined(const Any& a){
	return rawtype(a)==TYPE_UNDEFINED;
}

inline bool is_null(const Any& a){
	return rawtype(a)==TYPE_NULL;
}

inline bool raweq(const Any& a, const Any& b){
	return raweq(rawvalue(a), rawvalue(b));
}

inline bool rawne(const Any& a, const Any& b){
	return !raweq(a, b);
}

inline bool rawlt(const Any& a, const Any& b){
	return rawlt(rawvalue(a), rawvalue(b));
}

inline uint_t rawbitxor(const Any& a, const Any& b){
	return rawbitxor(rawvalue(a), rawvalue(b));
}

inline void copy_any(Any& v, const Any& u){
	v = u;
}

inline void swap(Any& a, Any& b){
	std::swap(a, b);
}

}

#endif // XTAL_ANY_H_INCLUDE_GUARD
