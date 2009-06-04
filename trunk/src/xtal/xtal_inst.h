/** \file src/xtal/xtal_inst.h
* \brief src/xtal/xtal_inst.h
*/

#ifndef XTAL_INST_H_INCLUDE_GUARD
#define XTAL_INST_H_INCLUDE_GUARD

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

	inst_16_t(int value){
		values[0] = (u8)(value>>8);
		values[1] = (u8)(value>>0);
	}
	
	operator T() const{
		return (T)((values[0]<<8) | values[1]);
	}
};

typedef inst_16_t<i16, 0> inst_i16_t;
typedef inst_16_t<u16, 0> inst_u16_t;
typedef inst_16_t<i16, 1> inst_address_t;


inline int_t inst_inspect(i8 value, Inst*, const CodePtr&){ return (int_t)value; }
inline int_t inst_inspect(u8 value, Inst*, const CodePtr&){ return (int_t)value; }
inline int_t inst_inspect(inst_i16_t value, Inst*, const CodePtr&){ return (int_t)value; }
inline int_t inst_inspect(inst_u16_t value, Inst*, const CodePtr&){ return (int_t)value; }

int_t inst_inspect(inst_address_t& value, Inst* inst, const CodePtr& code);

StringPtr make_inst_string(const char_t* InstName);

StringPtr make_inst_string(const char_t* InstName, 
						const char_t* MemberName1, int_t MemberValue1);

StringPtr make_inst_string(const char_t* InstName, 
						const char_t* MemberName1, int_t MemberValue1,
						const char_t* MemberName2, int_t MemberValue2);

StringPtr make_inst_string(const char_t* InstName, 
						const char_t* MemberName1, int_t MemberValue1,
						const char_t* MemberName2, int_t MemberValue2,
						const char_t* MemberName3, int_t MemberValue3);
						
StringPtr make_inst_string(const char_t* InstName, 
						const char_t* MemberName1, int_t MemberValue1,
						const char_t* MemberName2, int_t MemberValue2,
						const char_t* MemberName3, int_t MemberValue3,
						const char_t* MemberName4, int_t MemberValue4);

StringPtr make_inst_string(const char_t* InstName, 
						const char_t* MemberName1, int_t MemberValue1,
						const char_t* MemberName2, int_t MemberValue2,
						const char_t* MemberName3, int_t MemberValue3,
						const char_t* MemberName4, int_t MemberValue4,
						const char_t* MemberName5, int_t MemberValue5);

StringPtr inspect_range(const CodePtr& code, const inst_t* start, const inst_t* end);

u8 inst_size(uint_t no);

struct Inst{
	inst_t op;

	Inst(inst_t v = 0)
		:op(v){}

	operator const inst_t*() const{
		return &op;
	}

	template<class T>
	void checked_assign(T& ref, int v);
};

template<class T>
void Inst::checked_assign(T& ref, int v){
	ref = T(v);
	if(static_cast<int>(ref) != v){ op = 0xff; }
}

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
		StringPtr inspect(const CodePtr&){\
			return make_inst_string(\
					XTAL_STRING(#InstName));\
		}\
	}

#define XTAL_DEF_INST_1(N, InstName, MemberType1, MemberName1) \
	struct Inst##InstName : public Inst{\
		enum{ \
			NUMBER = N,\
			SIZE = sizeof(Inst) + sizeof(MemberType1),\
			ISIZE = SIZE/sizeof(inst_t),\
			OFFSET_##MemberName1 = sizeof(inst_t)\
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
			return make_inst_string(\
					XTAL_STRING(#InstName),\
					XTAL_STRING(#MemberName1), inst_inspect(MemberName1, this, code));\
		}\
	}

#define XTAL_DEF_INST_2(N, InstName, MemberType1, MemberName1, MemberType2, MemberName2) \
	struct Inst##InstName : public Inst{\
		enum{ \
			NUMBER = N,\
			SIZE = sizeof(Inst) + sizeof(MemberType1) + sizeof(MemberType2),\
			ISIZE = SIZE/sizeof(inst_t),\
			OFFSET_##MemberName1 = sizeof(inst_t),\
			OFFSET_##MemberName2 = OFFSET_##MemberName1 + sizeof(MemberType1)\
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
			return make_inst_string(\
					XTAL_STRING(#InstName),\
					XTAL_STRING(#MemberName1), inst_inspect(MemberName1, this, code),\
					XTAL_STRING(#MemberName2), inst_inspect(MemberName2, this, code));\
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
			OFFSET_##MemberName3 = OFFSET_##MemberName2 + sizeof(MemberType2)\
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
			return make_inst_string(\
					XTAL_STRING(#InstName),\
					XTAL_STRING(#MemberName1), inst_inspect(MemberName1, this, code),\
					XTAL_STRING(#MemberName2), inst_inspect(MemberName2, this, code),\
					XTAL_STRING(#MemberName3), inst_inspect(MemberName3, this, code));\
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
			OFFSET_##MemberName4 = OFFSET_##MemberName3 + sizeof(MemberType3)\
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
			return make_inst_string(\
					XTAL_STRING(#InstName),\
					XTAL_STRING(#MemberName1), inst_inspect(MemberName1, this, code),\
					XTAL_STRING(#MemberName2), inst_inspect(MemberName2, this, code),\
					XTAL_STRING(#MemberName3), inst_inspect(MemberName3, this, code),\
					XTAL_STRING(#MemberName4), inst_inspect(MemberName4, this, code));\
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
			OFFSET_##MemberName5 = OFFSET_##MemberName4 + sizeof(MemberType4)\
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
			return make_inst_string(\
					XTAL_STRING(#InstName),\
					XTAL_STRING(#MemberName1), inst_inspect(MemberName1, this, code),\
					XTAL_STRING(#MemberName2), inst_inspect(MemberName2, this, code),\
					XTAL_STRING(#MemberName3), inst_inspect(MemberName3, this, code),\
					XTAL_STRING(#MemberName4), inst_inspect(MemberName4, this, code),\
					XTAL_STRING(#MemberName5), inst_inspect(MemberName5, this, code));\
		}\
	}

/**
* \internal
* \brief 何もしない。
*
* stack [] -> []
*/
XTAL_DEF_INST_0(0, Nop);

/**
* \internal
* \brief スタックにnull値をプッシュする。
*
* stack [] -> [value]
*/
XTAL_DEF_INST_0(1, PushNull);

/**
* \internal
* \brief スタックにundefined値をプッシュする。
*
* stack [] -> [value]
*/
XTAL_DEF_INST_0(2, PushUndefined);

/**
* \internal
* \brief スタックにtrue値をプッシュする。
*
* stack [] -> [value]
*/
XTAL_DEF_INST_0(3, PushTrue);

/**
* \internal
* \brief スタックにfalse値をプッシュする。
*
* stack [] -> [value]
*/
XTAL_DEF_INST_0(4, PushFalse);

/**
* \internal
* \brief スタックにtrue値をプッシュする。
*
* stack [] -> [value]
*/
XTAL_DEF_INST_0(5, PushTrueAndSkip);

/**
* \internal
* \brief スタックにint値をプッシュする。
*
* stack [] -> [value]
*/
XTAL_DEF_INST_1(6, PushInt1Byte,
        inst_i8_t, value
);

/**
* \internal
* \brief スタックにfloat値をプッシュする。
*
* stack [] -> [value]
*/
XTAL_DEF_INST_1(7, PushFloat1Byte,
        inst_i8_t, value
);

/**
* \internal
* \brief スタックに現在実行中の関数をプッシュする。
*
* stack [] -> [value]
*/
XTAL_DEF_INST_0(8, PushCallee);

/**
* \internal
* \brief 現在のthisをスタックに積む。
*
* stack [] -> [value]
*/
XTAL_DEF_INST_0(9, PushThis);

/**
* \internal
* \brief 現在の文脈をスタックに積む。
*
* stack [] -> [value]
*/
XTAL_DEF_INST_0(10, PushCurrentContext);

/**
* \internal
* \brief スタックトップをポップする
*
* stack [value] -> []
*/
XTAL_DEF_INST_0(11, Pop);

/**
* \internal
* \brief スタックトップの値をプッシュする
*
* stack [value] -> [value, value2]
*/
XTAL_DEF_INST_0(12, Dup);

/**
* \internal
* \brief スタックトップの値を一つ前に入れる。
*
* stack [value1, value2] -> [value2, value1]
*/
XTAL_DEF_INST_0(13, Insert1);

/**
* \internal
* \brief スタックトップの値を二つ前に入れる。
*
* stack [value1, value2, value3] -> [value3, value1, value2]
*/
XTAL_DEF_INST_0(14, Insert2);

/**
* \internal
* \brief 値の数を調整する
*
* stack [] -> []
*/
XTAL_DEF_INST_2(15, AdjustResult,
        inst_u8_t, result_count,
        inst_u8_t, need_result_count
);

XTAL_DEF_INST_1(16, LocalVariableInc1Byte,
		inst_u8_t, number
);

XTAL_DEF_INST_1(17, LocalVariableDec1Byte,
		inst_u8_t, number
);

/**
* \internal
* \brief ローカル変数を取り出す。
*
* stack [] -> [value]
*/
XTAL_DEF_INST_1(18, LocalVariable1Byte,
        inst_u8_t, number
);

/**
* \internal
* \brief ローカル変数を取り出す。
*
* stack [] -> [value]
*/
XTAL_DEF_INST_2(19, LocalVariable1ByteX2,
        inst_u8_t, number1,
        inst_u8_t, number2
);

/**
* \internal
* \brief ローカル変数を取り出す。
*
* stack [] -> [value]
*/
XTAL_DEF_INST_2(20, LocalVariable2Byte,
        inst_u16_t, number,
		inst_u8_t, depth
);

/**
* \internal
* \brief ローカル変数に値を設定する。
*
* stack [value] -> []
*/
XTAL_DEF_INST_1(21, SetLocalVariable1Byte,
        inst_u8_t, number
);

/**
* \internal
* \brief ローカル変数に値を設定する。
*
* stack [value] -> []
*/
XTAL_DEF_INST_2(22, SetLocalVariable1ByteX2,
        inst_u8_t, number1,
        inst_u8_t, number2
);

/**
* \internal
* \brief ローカル変数に値を設定する。
*
* stack [value] -> []
*/
XTAL_DEF_INST_2(23, SetLocalVariable2Byte,
        inst_u16_t, number,
		inst_u8_t, depth
);

/**
* \internal
* \brief インスタンス変数を取り出す。
*
* stack [] -> [value]
*/
XTAL_DEF_INST_2(24, InstanceVariable,
        inst_u8_t, number,
        inst_u16_t, info_number
);

/**
* \internal
* \brief インスタンス変数に値を設定する。
*
* stack [value] -> []
*/
XTAL_DEF_INST_2(25, SetInstanceVariable,
        inst_u8_t, number,
        inst_u16_t, info_number
);

/**
* \internal
* \brief ファイルローカル変数を取り出す。
*
* stack [] -> [value]
*/
XTAL_DEF_INST_1(26, FilelocalVariable,
        inst_u16_t, identifier_number
);

/**
* \internal
* \brief 関数呼び出しの後始末をする
*
* stack [] -> []
*/
XTAL_DEF_INST_0(27, CleanupCall);

/**
* \internal
* \brief 関数から抜ける
*
* stack [value1, value2, ...valueN] -> []
*/
XTAL_DEF_INST_1(28, Return,
        inst_u8_t, result_count
);

/**
* \internal
* \brief fiberの実行を一時中断する
*
* stack [value1, value2, ...valueN] -> []
*/
XTAL_DEF_INST_2(29, Yield,
        inst_u8_t, result_count,
		inst_u8_t, need_result_count
);

/**
* \internal
* \brief 仮想マシンのループから脱出する。
*
* stack [] -> []
*/
XTAL_DEF_INST_0(30, Exit);

/**
* \internal
* \brief 値テーブルから値を取り出す
*
* stack [] -> [value]
*/
XTAL_DEF_INST_1(31, Value,
        inst_u16_t, value_number
);

XTAL_DEF_INST_0(32, CheckUnsupported);

enum{
	CALL_FLAG_TAIL_SHIFT = 0,
	CALL_FLAG_ARGS_SHIFT = 1,
	CALL_FLAG_NS_SHIFT = 2,
	CALL_FLAG_Q_SHIFT = 3,

	CALL_FLAG_NONE = 0,

	CALL_FLAG_TAIL = 1<<CALL_FLAG_TAIL_SHIFT,
	CALL_FLAG_ARGS = 1<<CALL_FLAG_ARGS_SHIFT,
	CALL_FLAG_NS = 1<<CALL_FLAG_NS_SHIFT,
	CALL_FLAG_Q = 1<<CALL_FLAG_Q_SHIFT
};

XTAL_DEF_INST_1(33, Property,
        inst_u16_t, identifier_number
);

XTAL_DEF_INST_1(34, SetProperty,
        inst_u16_t, identifier_number
);

XTAL_DEF_INST_4(35, Call,
        inst_u8_t, ordered,
        inst_u8_t, named,
        inst_u8_t, need_result,
		inst_u8_t, flags
);

XTAL_DEF_INST_5(36, Send,
        inst_u8_t, ordered,
        inst_u8_t, named,
        inst_u8_t, need_result,
		inst_u8_t, flags,
        inst_u16_t, identifier_number
);

/**
* \internal
* \brief オブジェクトのメンバを取り出す。
*
* stack [object] -> [result]
*/
XTAL_DEF_INST_2(37, Member,
        inst_u16_t, identifier_number,
		inst_u8_t, flags
);

/**
* \internal
* \brief オブジェクトのメンバを定義する。
*
* stack [object, value] -> []
*/
XTAL_DEF_INST_2(38, DefineMember,
        inst_u16_t, identifier_number,
		inst_u8_t, flags
);

XTAL_DEF_INST_1(39, ScopeBegin,
        inst_u16_t, info_number
);

XTAL_DEF_INST_0(40, ScopeEnd);

XTAL_DEF_INST_1(41, TryBegin,
        inst_u16_t, info_number
);

XTAL_DEF_INST_0(42, TryEnd);

XTAL_DEF_INST_1(43, PushGoto,
        inst_address_t, address
);

XTAL_DEF_INST_0(44, PopGoto);

/**
* \internal
* \brief 無条件分岐
*
* stack [] -> []
*/
XTAL_DEF_INST_1(45, Goto,
        inst_address_t, address
);

/**
* \internal
* \brief 条件分岐
*
* stack [value] -> []
*/
XTAL_DEF_INST_2(46, If,
        inst_address_t, address_true,
		inst_address_t, address_false
);

XTAL_DEF_INST_0(47, IfEq);

XTAL_DEF_INST_0(48, IfLt);

XTAL_DEF_INST_0(49, IfRawEq);

XTAL_DEF_INST_0(50, IfIn);

XTAL_DEF_INST_0(51, IfIs);

XTAL_DEF_INST_2(52, IfArgIsUndefined,
	inst_u8_t, arg,
	inst_address_t, address
);

XTAL_DEF_INST_0(53, Pos);
XTAL_DEF_INST_0(54, Neg);
XTAL_DEF_INST_0(55, Com);
XTAL_DEF_INST_0(56, Not);

XTAL_DEF_INST_0(57, At);
XTAL_DEF_INST_0(58, SetAt);

/*
enum{
	ARITH_ADD,
	ARITH_SUB,
	ARITH_CAT,
	ARITH_MUL,
	ARITH_DIV,
	ARITH_MOD,
	ARITH_AND,
	ARITH_OR,
	ARITH_XOR,
	ARITH_SHL,
	ARITH_SHR,
	ARITH_USHR
};
*/

XTAL_DEF_INST_0(59, Add);
XTAL_DEF_INST_0(60, Sub);
XTAL_DEF_INST_0(61, Cat);
XTAL_DEF_INST_0(62, Mul);
XTAL_DEF_INST_0(63, Div);
XTAL_DEF_INST_0(64, Mod);
XTAL_DEF_INST_0(65, And);
XTAL_DEF_INST_0(66, Or);
XTAL_DEF_INST_0(67, Xor);
XTAL_DEF_INST_0(68, Shl);
XTAL_DEF_INST_0(69, Shr);
XTAL_DEF_INST_0(70, Ushr);

XTAL_DEF_INST_0(71, Inc);
XTAL_DEF_INST_0(72, Dec);
XTAL_DEF_INST_0(73, AddAssign);
XTAL_DEF_INST_0(74, SubAssign);
XTAL_DEF_INST_0(75, CatAssign);
XTAL_DEF_INST_0(76, MulAssign);
XTAL_DEF_INST_0(77, DivAssign);
XTAL_DEF_INST_0(78, ModAssign);
XTAL_DEF_INST_0(79, AndAssign);
XTAL_DEF_INST_0(80, OrAssign);
XTAL_DEF_INST_0(81, XorAssign);
XTAL_DEF_INST_0(82, ShlAssign);
XTAL_DEF_INST_0(83, ShrAssign);
XTAL_DEF_INST_0(84, UshrAssign);

XTAL_DEF_INST_1(85, Range,
		inst_u8_t, kind
);

XTAL_DEF_INST_2(86, Once,
        inst_address_t, address,
        inst_u16_t, value_number
);

XTAL_DEF_INST_1(87, SetOnce,
        inst_u16_t, value_number
);

XTAL_DEF_INST_1(88, ClassBegin,
        inst_u16_t, info_number
);

XTAL_DEF_INST_0(89, ClassEnd);

XTAL_DEF_INST_3(90, DefineClassMember,
        inst_u16_t, number,
		inst_u16_t, identifier_number,
		inst_u8_t, accessibility
);

XTAL_DEF_INST_0(91, MakeArray);

XTAL_DEF_INST_0(92, ArrayAppend);

XTAL_DEF_INST_0(93, MakeMap);

XTAL_DEF_INST_0(94, MapInsert);

XTAL_DEF_INST_0(95, MapSetDefault);

XTAL_DEF_INST_2(96, MakeFun,
        inst_u16_t, info_number,
		inst_address_t, address
);

XTAL_DEF_INST_3(97, MakeInstanceVariableAccessor,
        inst_u8_t, type,
		inst_u8_t, number,
        inst_u16_t, info_number
);

XTAL_DEF_INST_0(98, Throw);

XTAL_DEF_INST_0(99, ThrowUnsupportedError);

XTAL_DEF_INST_1(100, IfDebug,
		inst_address_t, address);

XTAL_DEF_INST_0(101, Assert);

XTAL_DEF_INST_0(102, BreakPoint);

XTAL_DEF_INST_0(103, MAX);

}

#endif // XTAL_INST_H_INCLUDE_GUARD
