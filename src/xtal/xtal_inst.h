
#pragma once

namespace xtal{

struct Inst;

typedef u8 inst_t;
typedef i8 inst_i8_t;
typedef u8 inst_u8_t;

template<class T, int Kind>
struct inst_16_t{
	u8 values[2];
	
	inst_16_t(){}

	inst_16_t(T value){
		values[0] = (value>>8) & 0xff;
		values[1] = (value>>0) & 0xff;
	}
	
	operator T() const{
		return (T)(values[0]<<8 | values[1]);
	}
};

typedef inst_16_t<i16, 0> inst_i16_t;
typedef inst_16_t<u16, 0> inst_u16_t;
typedef inst_16_t<i16, 1> inst_address_t;


inline AnyPtr inst_inspect(i8 value, Inst* inst, const CodePtr& code){ return (int_t)value; }
inline AnyPtr inst_inspect(u8 value, Inst* inst, const CodePtr& code){ return (int_t)value; }
inline AnyPtr inst_inspect(inst_i16_t value, Inst* inst, const CodePtr& code){ return (int_t)value; }
inline AnyPtr inst_inspect(inst_u16_t value, Inst* inst, const CodePtr& code){ return (int_t)value; }

AnyPtr inst_inspect(inst_address_t value, Inst* inst, const CodePtr& code);

struct Inst{
	inst_t op;

	Inst(inst_t v = 0)
		:op(v){}

	operator const inst_t*() const{
		return &op;
	}

	template<class T>
	void checked_assign(T& ref, int v){
		ref = v;
		if((int)ref != v){ op = 0xff; }
	}
};

#define XTAL_DEF_INST_0(N, InstName) \
	struct Inst##InstName : public Inst{\
		enum{ \
			NUMBER = N,\
			SIZE = sizeof(Inst),\
			ISIZE = SIZE/sizeof(inst_t)\
		};\
		Inst##InstName(){\
			op = N;\
		}\
		StringPtr inspect(const CodePtr& code){\
			return xnew<String>(XTAL_STRING(#InstName) XTAL_STRING(":"));\
		}\
	}

#define XTAL_DEF_INST_1(N, InstName, MemberType1, MemberName1) \
	struct Inst##InstName : public Inst{\
		enum{ \
			NUMBER = N,\
			SIZE = sizeof(Inst) + sizeof(MemberType1),\
			ISIZE = SIZE/sizeof(inst_t),\
			OFFSET_##MemberName1 = sizeof(inst_t),\
		};\
		MemberType1 MemberName1;\
		Inst##InstName(){\
			op = N;\
		}\
		Inst##InstName(int m1){\
			op = N;\
			checked_assign(MemberName1, m1);\
		}\
		StringPtr inspect(const CodePtr& code){\
			return format(\
					XTAL_STRING(#InstName) XTAL_STRING(": ")\
					XTAL_STRING(#MemberName1) XTAL_STRING("=%s"))\
				(inst_inspect(MemberName1, this, code))->to_s();\
		}\
	}

#define XTAL_DEF_INST_2(N, InstName, MemberType1, MemberName1, MemberType2, MemberName2) \
	struct Inst##InstName : public Inst{\
		enum{ \
			NUMBER = N,\
			SIZE = sizeof(Inst) + sizeof(MemberType1) + sizeof(MemberType2),\
			ISIZE = SIZE/sizeof(inst_t),\
			OFFSET_##MemberName1 = sizeof(inst_t),\
			OFFSET_##MemberName2 = OFFSET_##MemberName1 + sizeof(MemberType1),\
		};\
		MemberType1 MemberName1;\
		MemberType2 MemberName2;\
		Inst##InstName(){\
			op = N;\
		}\
		Inst##InstName(int m1, int m2){\
			op = N;\
			checked_assign(MemberName1, m1);\
			checked_assign(MemberName2, m2);\
		}\
		StringPtr inspect(const CodePtr& code){\
			return format(\
					XTAL_STRING(#InstName) XTAL_STRING(": ")\
					XTAL_STRING(#MemberName1) XTAL_STRING("=%s, ")\
					XTAL_STRING(#MemberName2) XTAL_STRING("=%s"))\
				(inst_inspect(MemberName1, this, code), inst_inspect(MemberName2, this, code))->to_s();\
		}\
	}
	
#define XTAL_DEF_INST_3(N, InstName, MemberType1, MemberName1, MemberType2, MemberName2, MemberType3, MemberName3) \
	struct Inst##InstName : public Inst{\
		enum{ \
			NUMBER = N,\
			SIZE = sizeof(Inst) + sizeof(MemberType1) + sizeof(MemberType2) + sizeof(MemberType3),\
			ISIZE = SIZE/sizeof(inst_t),\
			OFFSET_##MemberName1 = sizeof(inst_t),\
			OFFSET_##MemberName2 = OFFSET_##MemberName1 + sizeof(MemberType1),\
			OFFSET_##MemberName3 = OFFSET_##MemberName2 + sizeof(MemberType2),\
		};\
		MemberType1 MemberName1;\
		MemberType2 MemberName2;\
		MemberType3 MemberName3;\
		Inst##InstName(){\
			op = N;\
		}\
		Inst##InstName(int m1, int m2, int m3){\
			op = N;\
			checked_assign(MemberName1, m1);\
			checked_assign(MemberName2, m2);\
			checked_assign(MemberName3, m3);\
		}\
		StringPtr inspect(const CodePtr& code){\
			return format(\
					XTAL_STRING(#InstName) XTAL_STRING(": ")\
					XTAL_STRING(#MemberName1) XTAL_STRING("=%s, ")\
					XTAL_STRING(#MemberName2) XTAL_STRING("=%s, ")\
					XTAL_STRING(#MemberName3) XTAL_STRING("=%s"))\
				(inst_inspect(MemberName1, this, code), inst_inspect(MemberName2, this, code), inst_inspect(MemberName3, this, code))->to_s();\
		}\
	}
	
#define XTAL_DEF_INST_4(N, InstName, MemberType1, MemberName1, MemberType2, MemberName2, MemberType3, MemberName3, MemberType4, MemberName4) \
	struct Inst##InstName : public Inst{\
		enum{ \
			NUMBER = N,\
			SIZE = sizeof(Inst) + sizeof(MemberType1) + sizeof(MemberType2) + sizeof(MemberType3) + sizeof(MemberType4),\
			ISIZE = SIZE/sizeof(inst_t),\
			OFFSET_##MemberName1 = sizeof(inst_t),\
			OFFSET_##MemberName2 = OFFSET_##MemberName1 + sizeof(MemberType1),\
			OFFSET_##MemberName3 = OFFSET_##MemberName2 + sizeof(MemberType2),\
			OFFSET_##MemberName4 = OFFSET_##MemberName3 + sizeof(MemberType3),\
		};\
		MemberType1 MemberName1;\
		MemberType2 MemberName2;\
		MemberType3 MemberName3;\
		MemberType4 MemberName4;\
		Inst##InstName(){\
			op = N;\
		}\
		Inst##InstName(int m1, int m2, int m3, int m4){\
			op = N;\
			checked_assign(MemberName1, m1);\
			checked_assign(MemberName2, m2);\
			checked_assign(MemberName3, m3);\
			checked_assign(MemberName4, m4);\
		}\
		StringPtr inspect(const CodePtr& code){\
			return format(\
					XTAL_STRING(#InstName) XTAL_STRING(": ")\
					XTAL_STRING(#MemberName1) XTAL_STRING("=%s, ")\
					XTAL_STRING(#MemberName2) XTAL_STRING("=%s, ")\
					XTAL_STRING(#MemberName3) XTAL_STRING("=%s, ")\
					XTAL_STRING(#MemberName4) XTAL_STRING("=%s"))\
				(inst_inspect(MemberName1, this, code), inst_inspect(MemberName2, this, code), inst_inspect(MemberName3, this, code), inst_inspect(MemberName4, this, code))->to_s();\
		}\
	}

#define XTAL_DEF_INST_5(N, InstName, MemberType1, MemberName1, MemberType2, MemberName2, MemberType3, MemberName3, MemberType4, MemberName4, MemberType5, MemberName5) \
	struct Inst##InstName : public Inst{\
		enum{ \
			NUMBER = N,\
			SIZE = sizeof(Inst) + sizeof(MemberType1) + sizeof(MemberType2) + sizeof(MemberType3) + sizeof(MemberType4) + sizeof(MemberType5),\
			ISIZE = SIZE/sizeof(inst_t),\
			OFFSET_##MemberName1 = sizeof(inst_t),\
			OFFSET_##MemberName2 = OFFSET_##MemberName1 + sizeof(MemberType1),\
			OFFSET_##MemberName3 = OFFSET_##MemberName2 + sizeof(MemberType2),\
			OFFSET_##MemberName4 = OFFSET_##MemberName3 + sizeof(MemberType3),\
			OFFSET_##MemberName5 = OFFSET_##MemberName4 + sizeof(MemberType4),\
		};\
		MemberType1 MemberName1;\
		MemberType2 MemberName2;\
		MemberType3 MemberName3;\
		MemberType4 MemberName4;\
		MemberType5 MemberName5;\
		Inst##InstName(){\
			op = N;\
		}\
		Inst##InstName(int m1, int m2, int m3, int m4, int m5){\
			op = N;\
			checked_assign(MemberName1, m1);\
			checked_assign(MemberName2, m2);\
			checked_assign(MemberName3, m3);\
			checked_assign(MemberName4, m4);\
			checked_assign(MemberName5, m5);\
		}\
		StringPtr inspect(const CodePtr& code){\
			return format(\
					XTAL_STRING(#InstName) XTAL_STRING(": ")\
					XTAL_STRING(#MemberName1) XTAL_STRING("=%s, ")\
					XTAL_STRING(#MemberName2) XTAL_STRING("=%s, ")\
					XTAL_STRING(#MemberName3) XTAL_STRING("=%s, ")\
					XTAL_STRING(#MemberName4) XTAL_STRING("=%s, ")\
					XTAL_STRING(#MemberName5) XTAL_STRING("=%s"))\
				(inst_inspect(MemberName1, this, code), inst_inspect(MemberName2, this, code), inst_inspect(MemberName3, this, code), inst_inspect(MemberName4, this, code), inst_inspect(MemberName5, this, code))->to_s();\
		}\
	}

/**
* @brief 何もしない。
*
* stack [] -> []
*/
XTAL_DEF_INST_0(0, Nop);

/**
* @brief スタックにnull値をプッシュする。
*
* stack [] -> [value]
*/
XTAL_DEF_INST_0(1, PushNull);

/**
* @brief スタックにundefined値をプッシュする。
*
* stack [] -> [value]
*/
XTAL_DEF_INST_0(2, PushUndefined);

/**
* @brief スタックにtrue値をプッシュする。
*
* stack [] -> [value]
*/
XTAL_DEF_INST_0(3, PushTrue);

/**
* @brief スタックにfalse値をプッシュする。
*
* stack [] -> [value]
*/
XTAL_DEF_INST_0(4, PushFalse);

/**
* @brief スタックにint値をプッシュする。
*
* stack [] -> [value]
*/
XTAL_DEF_INST_1(5, PushInt1Byte,
        inst_i8_t, value
);

/**
* @brief スタックにint値をプッシュする。
*
* stack [] -> [value]
*/
XTAL_DEF_INST_1(6, PushInt2Byte,
        inst_i16_t, value
);

/**
* @brief スタックにfloat値をプッシュする。
*
* stack [] -> [value]
*/
XTAL_DEF_INST_1(7, PushFloat1Byte,
        inst_i8_t, value
);

/**
* @brief スタックにfloat値をプッシュする。
*
* stack [] -> [value]
*/
XTAL_DEF_INST_1(8, PushFloat2Byte,
        inst_i16_t, value
);

/**
* @brief スタックに現在実行中の関数をプッシュする。
*
* stack [] -> [value]
*/
XTAL_DEF_INST_0(9, PushCallee);

/**
* @brief スタックにArgumentsオブジェクトをプッシュする。
*
* stack [] -> [value]
*/
XTAL_DEF_INST_0(10, PushArgs);

/**
* @brief 現在のthisをスタックに積む。
*
* stack [] -> [value]
*/
XTAL_DEF_INST_0(11, PushThis);

/**
* @brief 現在の文脈をスタックに積む。
*
* stack [] -> [value]
*/
XTAL_DEF_INST_0(12, PushCurrentContext);

/**
* @brief スタックトップをポップする
*
* stack [value] -> []
*/
XTAL_DEF_INST_0(13, Pop);

/**
* @brief スタックトップの値をプッシュする
*
* stack [value] -> [value, value2]
*/
XTAL_DEF_INST_0(14, Dup);

/**
* @brief スタックトップの値を一つ前に入れる。
*
* stack [value1, value2] -> [value2, value1]
*/
XTAL_DEF_INST_0(15, Insert1);

/**
* @brief スタックトップの値を二つ前に入れる。
*
* stack [value1, value2, value3] -> [value3, value1, value2]
*/
XTAL_DEF_INST_0(16, Insert2);

/**
* @brief スタックトップの値を三つ前に入れる。
*
* stack [value1, value2, value3, value4] -> [value4, value1, value2, value3]
*/
XTAL_DEF_INST_0(17, Insert3);

/**
* @brief 値の数を調整する
*
* stack [] -> []
*/
XTAL_DEF_INST_2(18, AdjustResult,
        inst_u8_t, result_count,
        inst_u8_t, need_result_count
);

/**
* @brief 条件分岐
*
* stack [value] -> []
*/
XTAL_DEF_INST_1(19, If,
        inst_address_t, address
);

/**
* @brief 条件分岐
*
* stack [value] -> []
*/
XTAL_DEF_INST_1(20, Unless,
        inst_address_t, address
);

/**
* @brief 無条件分岐
*
* stack [] -> []
*/
XTAL_DEF_INST_1(21, Goto,
        inst_address_t, address
);

XTAL_DEF_INST_1(22, LocalVariableInc,
		inst_u8_t, number
);

XTAL_DEF_INST_1(23, LocalVariableIncDirect,
		inst_u8_t, number
);

XTAL_DEF_INST_1(24, LocalVariableDec,
		inst_u8_t, number
);

XTAL_DEF_INST_1(25, LocalVariableDecDirect,
		inst_u8_t, number
);


XTAL_DEF_INST_1(26, LocalVariableInc2Byte,
		inst_u8_t, number
);

XTAL_DEF_INST_1(27, LocalVariableDec2Byte,
		inst_u8_t, number
);

/**
* @brief ローカル変数を取り出す。
*
* stack [] -> [value]
*/
XTAL_DEF_INST_1(28, LocalVariable1Byte,
        inst_u8_t, number
);

/**
* @brief ローカル変数を取り出す。
*
* stack [] -> [value]
*/
XTAL_DEF_INST_1(29, LocalVariable1ByteDirect,
        inst_u8_t, number
);

/**
* @brief ローカル変数を取り出す。
*
* stack [] -> [value]
*/
XTAL_DEF_INST_1(30, LocalVariable2Byte,
        inst_u16_t, number
);

/**
* @brief ローカル変数に値を設定する。
*
* stack [value] -> []
*/
XTAL_DEF_INST_1(31, SetLocalVariable1Byte,
        inst_u8_t, number
);

/**
* @brief ローカル変数に値を設定する。
*
* stack [value] -> []
*/
XTAL_DEF_INST_1(32, SetLocalVariable1ByteDirect,
        inst_u8_t, number
);

/**
* @brief ローカル変数に値を設定する。
*
* stack [value] -> []
*/
XTAL_DEF_INST_1(33, SetLocalVariable2Byte,
        inst_u16_t, number
);

/**
* @brief インスタンス変数を取り出す。
*
* stack [] -> [value]
*/
XTAL_DEF_INST_2(34, InstanceVariable,
        inst_u8_t, number,
        inst_u16_t, core_number
);

/**
* @brief インスタンス変数に値を設定する。
*
* stack [value] -> []
*/
XTAL_DEF_INST_2(35, SetInstanceVariable,
        inst_u8_t, number,
        inst_u16_t, core_number
);

/**
* @brief 関数呼び出しの後始末をする
*
* stack [] -> []
*/
XTAL_DEF_INST_0(36, CleanupCall);

/**
* @brief 関数から抜ける
*
* stack [value1, value2, ...valueN] -> []
*/
XTAL_DEF_INST_1(37, Return,
        inst_u8_t, results
);

/**
* @brief fiberの実行を一時中断する
*
* stack [value1, value2, ...valueN] -> []
*/
XTAL_DEF_INST_1(38, Yield,
        inst_u16_t, results
);

/**
* @brief 仮想マシンのループから脱出する。
*
* stack [] -> []
*/
XTAL_DEF_INST_0(39, Exit);

/**
* @brief 値テーブルから値を取り出す
*
* stack [] -> [value]
*/
XTAL_DEF_INST_1(40, Value,
        inst_u16_t, value_number
);

XTAL_DEF_INST_0(41, CheckUnsupported);

XTAL_DEF_INST_1(42, Property,
        inst_u16_t, identifier_number
);

XTAL_DEF_INST_1(43, PropertyNS,
        inst_u16_t, identifier_number
);

XTAL_DEF_INST_1(44, SetProperty,
        inst_u16_t, identifier_number
);

XTAL_DEF_INST_1(45, SetPropertyNS,
        inst_u16_t, identifier_number
);

enum{
	CALL_FLAG_TAIL_SHIFT = 0,
	CALL_FLAG_ARGS_SHIFT = 1,

	CALL_FLAG_TAIL = 1<<CALL_FLAG_TAIL_SHIFT,
	CALL_FLAG_ARGS = 1<<CALL_FLAG_ARGS_SHIFT,
};

XTAL_DEF_INST_4(46, Call,
        inst_u8_t, ordered,
        inst_u8_t, named,
        inst_u8_t, need_result,
		inst_u8_t, flags
);

XTAL_DEF_INST_5(47, Send,
        inst_u8_t, ordered,
        inst_u8_t, named,
        inst_u8_t, need_result,
		inst_u8_t, flags,
        inst_u16_t, identifier_number
);

XTAL_DEF_INST_5(48, SendNS,
        inst_u8_t, ordered,
        inst_u8_t, named,
        inst_u8_t, need_result,
		inst_u8_t, flags,
        inst_u16_t, identifier_number
);

XTAL_DEF_INST_5(49, SendQ,
        inst_u8_t, ordered,
        inst_u8_t, named,
        inst_u8_t, need_result,
		inst_u8_t, flags,
        inst_u16_t, identifier_number
);

XTAL_DEF_INST_5(50, SendQNS,
        inst_u8_t, ordered,
        inst_u8_t, named,
        inst_u8_t, need_result,
		inst_u8_t, flags,
        inst_u16_t, identifier_number
);

/**
* @brief オブジェクトのメンバを取り出す。
*
* stack [object] -> [result]
*/
XTAL_DEF_INST_1(51, Member,
        inst_u16_t, identifier_number
);

/**
* @brief オブジェクトのメンバを取り出す。
*
* stack [object] -> [result]
*/
XTAL_DEF_INST_1(52, MemberNS,
        inst_u16_t, identifier_number
);

/**
* @brief オブジェクトのメンバを取り出す。
*
* stack [object] -> [result]
*/
XTAL_DEF_INST_1(53, MemberQ,
        inst_u16_t, identifier_number
);

/**
* @brief オブジェクトのメンバを取り出す。
*
* stack [object] -> [result]
*/
XTAL_DEF_INST_1(54, MemberQNS,
        inst_u16_t, identifier_number
);

/**
* @brief オブジェクトのメンバを定義する。
*
* stack [object, value] -> []
*/
XTAL_DEF_INST_1(55, DefineMember,
        inst_u16_t, identifier_number
);

/**
* @brief オブジェクトのメンバを定義する。
*
* stack [object, value] -> []
*/
XTAL_DEF_INST_1(56, DefineMemberNS,
        inst_u16_t, identifier_number
);

/**
* @brief グローバル変数を取り出す。
*
* stack [] -> [value]
*/
XTAL_DEF_INST_1(57, GlobalVariable,
        inst_u16_t, identifier_number
);
/**
* @brief グローバル変数に設定する。
*
* stack [value] -> []
*/
XTAL_DEF_INST_1(58, SetGlobalVariable,
        inst_u16_t, identifier_number
);

/**
* @brief グローバル変数を定義する。
*
* stack [value] -> []
*/
XTAL_DEF_INST_1(59, DefineGlobalVariable,
        inst_u16_t, identifier_number
);

XTAL_DEF_INST_2(60, Once,
        inst_address_t, address,
        inst_u16_t, value_number
);

XTAL_DEF_INST_1(61, SetOnce,
        inst_u16_t, value_number
);

XTAL_DEF_INST_1(62, BlockBegin,
        inst_u16_t, core_number
);

XTAL_DEF_INST_1(63, BlockBeginDirect,
        inst_u16_t, core_number
);

XTAL_DEF_INST_1(64, BlockEnd,
        inst_u16_t, core_number
);

XTAL_DEF_INST_1(65, BlockEndDirect,
        inst_u16_t, core_number
);

XTAL_DEF_INST_1(66, TryBegin,
        inst_u16_t, core_number
);

XTAL_DEF_INST_0(67, TryEnd);

XTAL_DEF_INST_1(68, PushGoto,
        inst_address_t, address
);

XTAL_DEF_INST_0(69, PopGoto);

XTAL_DEF_INST_1(70, IfEq,
        inst_address_t, address
);

XTAL_DEF_INST_1(71, IfNe,
        inst_address_t, address
);

XTAL_DEF_INST_1(72, IfLt,
        inst_address_t, address
);

XTAL_DEF_INST_1(73, IfLe,
        inst_address_t, address
);

XTAL_DEF_INST_1(74, IfGt,
        inst_address_t, address
);

XTAL_DEF_INST_1(75, IfGe,
        inst_address_t, address
);

XTAL_DEF_INST_1(76, IfRawEq,
        inst_address_t, address
);

XTAL_DEF_INST_1(77, IfRawNe,
        inst_address_t, address
);

XTAL_DEF_INST_1(78, IfIn,
        inst_address_t, address
);

XTAL_DEF_INST_1(79, IfNin,
        inst_address_t, address
);

XTAL_DEF_INST_1(80, IfIs,
        inst_address_t, address
);

XTAL_DEF_INST_1(81, IfNis,
        inst_address_t, address
);

XTAL_DEF_INST_2(82, IfArgIsUndefined,
	inst_u8_t, arg,
	inst_address_t, address
);

XTAL_DEF_INST_2(83, IfArgIsUndefinedDirect,
	inst_u8_t, arg,
	inst_address_t, address
);

XTAL_DEF_INST_0(84, Pos);
XTAL_DEF_INST_0(85, Neg);
XTAL_DEF_INST_0(86, Com);
XTAL_DEF_INST_0(87, Not);

XTAL_DEF_INST_0(88, At);
XTAL_DEF_INST_0(89, SetAt);
XTAL_DEF_INST_0(90, Add);
XTAL_DEF_INST_0(91, Sub);
XTAL_DEF_INST_0(92, Cat);
XTAL_DEF_INST_0(93, Mul);
XTAL_DEF_INST_0(94, Div);
XTAL_DEF_INST_0(95, Mod);
XTAL_DEF_INST_0(96, And);
XTAL_DEF_INST_0(97, Or);
XTAL_DEF_INST_0(98, Xor);
XTAL_DEF_INST_0(99, Shl);
XTAL_DEF_INST_0(100, Shr);
XTAL_DEF_INST_0(101, Ushr);

XTAL_DEF_INST_0(102, Eq);
XTAL_DEF_INST_0(103, Ne);
XTAL_DEF_INST_0(104, Lt);
XTAL_DEF_INST_0(105, Le);
XTAL_DEF_INST_0(106, Gt);
XTAL_DEF_INST_0(107, Ge);
XTAL_DEF_INST_0(108, RawEq);
XTAL_DEF_INST_0(109, RawNe);
XTAL_DEF_INST_0(110, In);
XTAL_DEF_INST_0(111, Nin);
XTAL_DEF_INST_0(112, Is);
XTAL_DEF_INST_0(113, Nis);

XTAL_DEF_INST_0(114, Inc);
XTAL_DEF_INST_0(115, Dec);
XTAL_DEF_INST_0(116, AddAssign);
XTAL_DEF_INST_0(117, SubAssign);
XTAL_DEF_INST_0(118, CatAssign);
XTAL_DEF_INST_0(119, MulAssign);
XTAL_DEF_INST_0(120, DivAssign);
XTAL_DEF_INST_0(121, ModAssign);
XTAL_DEF_INST_0(122, AndAssign);
XTAL_DEF_INST_0(123, OrAssign);
XTAL_DEF_INST_0(124, XorAssign);
XTAL_DEF_INST_0(125, ShlAssign);
XTAL_DEF_INST_0(126, ShrAssign);
XTAL_DEF_INST_0(127, UshrAssign);

XTAL_DEF_INST_1(128, Range,
		inst_u8_t, kind
);

XTAL_DEF_INST_1(129, ClassBegin,
        inst_u16_t, core_number
);

XTAL_DEF_INST_0(130, ClassEnd);

XTAL_DEF_INST_3(131, DefineClassMember,
        inst_u16_t, number,
		inst_u16_t, identifier_number,
		inst_u8_t, accessibility
);

XTAL_DEF_INST_0(132, MakeArray);

XTAL_DEF_INST_0(133, ArrayAppend);

XTAL_DEF_INST_0(134, MakeMap);

XTAL_DEF_INST_0(135, MapInsert);

XTAL_DEF_INST_0(136, MapSetDefault);

XTAL_DEF_INST_2(137, MakeFun,
        inst_u16_t, core_number,
		inst_address_t, address
);

XTAL_DEF_INST_3(138, MakeInstanceVariableAccessor,
        inst_u8_t, type,
		inst_u8_t, number,
        inst_u16_t, core_number
);

XTAL_DEF_INST_0(139, Throw);

XTAL_DEF_INST_0(140, ThrowUnsupportedError);

XTAL_DEF_INST_0(141, ThrowUndefined);

XTAL_DEF_INST_0(142, Assert);

XTAL_DEF_INST_0(143, MAX);

}
