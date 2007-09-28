
#pragma once

#include "xtal_lib.h"
#include "xtal_smartptr.h"

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
			return xnew<String>("" #InstName ":");\
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
			return format("" #InstName ": "\
					#MemberName1 "=%s")\
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
			return format("" #InstName ": "\
					#MemberName1 "=%s, "\
					#MemberName2 "=%s")\
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
			return format("" #InstName ": "\
					#MemberName1 "=%s, "\
					#MemberName2 "=%s, "\
					#MemberName3 "=%s")\
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
			return format("" #InstName ": "\
					#MemberName1 "=%s, "\
					#MemberName2 "=%s, "\
					#MemberName3 "=%s, "\
					#MemberName4 "=%s")\
				(inst_inspect(MemberName1, this, code), inst_inspect(MemberName2, this, code), inst_inspect(MemberName3, this, code), inst_inspect(MemberName4, this, code))->to_s();\
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
* @brief スタックにnop値をプッシュする。
*
* stack [] -> [value]
*/
XTAL_DEF_INST_0(2, PushNop);

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
* stack [] -> []
*/
XTAL_DEF_INST_0(37, Return0);

/**
* @brief 関数から抜ける
*
* stack [value1] -> []
*/
XTAL_DEF_INST_0(38, Return1);

/**
* @brief 関数から抜ける
*
* stack [value1, value2] -> []
*/
XTAL_DEF_INST_0(39, Return2);

/**
* @brief 関数から抜ける
*
* stack [value1, value2, ...valueN] -> []
*/
XTAL_DEF_INST_1(40, Return,
        inst_u8_t, results
);

/**
* @brief fiberの実行を一時中断する
*
* stack [value1, value2, ...valueN] -> []
*/
XTAL_DEF_INST_1(41, Yield,
        inst_u16_t, results
);

/**
* @brief 仮想マシンのループから脱出する。
*
* stack [] -> []
*/
XTAL_DEF_INST_0(42, Exit);

/**
* @brief 値テーブルから値を取り出す
*
* stack [] -> [value]
*/
XTAL_DEF_INST_1(43, Value,
        inst_u16_t, value_number
);

XTAL_DEF_INST_0(44, CheckUnsupported);

// A=args flag, T=tail flag

XTAL_DEF_INST_4(45, Send,
        inst_u8_t, ordered,
        inst_u8_t, named,
        inst_u8_t, need_result,
        inst_u16_t, identifier_number
);

XTAL_DEF_INST_4(46, SendIfDefined,
        inst_u8_t, ordered,
        inst_u8_t, named,
        inst_u8_t, need_result,
        inst_u16_t, identifier_number
);

XTAL_DEF_INST_3(47, Call,
        inst_u8_t, ordered,
        inst_u8_t, named,
        inst_u8_t, need_result
);

XTAL_DEF_INST_3(48, CallCallee,
        inst_u8_t, ordered,
        inst_u8_t, named,
        inst_u8_t, need_result
);

XTAL_DEF_INST_4(49, Send_A,
        inst_u8_t, ordered,
        inst_u8_t, named,
        inst_u8_t, need_result,
        inst_u16_t, identifier_number
);

XTAL_DEF_INST_4(50, SendIfDefined_A,
        inst_u8_t, ordered,
        inst_u8_t, named,
        inst_u8_t, need_result,
        inst_u16_t, identifier_number
);

XTAL_DEF_INST_3(51, Call_A,
        inst_u8_t, ordered,
        inst_u8_t, named,
        inst_u8_t, need_result
);

XTAL_DEF_INST_3(52, CallCallee_A,
        inst_u8_t, ordered,
        inst_u8_t, named,
        inst_u8_t, need_result
);

XTAL_DEF_INST_4(53, Send_T,
        inst_u8_t, ordered,
        inst_u8_t, named,
        inst_u8_t, need_result,
        inst_u16_t, identifier_number
);

XTAL_DEF_INST_4(54, SendIfDefined_T,
        inst_u8_t, ordered,
        inst_u8_t, named,
        inst_u8_t, need_result,
        inst_u16_t, identifier_number
);

XTAL_DEF_INST_3(55, Call_T,
        inst_u8_t, ordered,
        inst_u8_t, named,
        inst_u8_t, need_result
);

XTAL_DEF_INST_3(56, CallCallee_T,
        inst_u8_t, ordered,
        inst_u8_t, named,
        inst_u8_t, need_result
);

XTAL_DEF_INST_4(57, Send_AT,
        inst_u8_t, ordered,
        inst_u8_t, named,
        inst_u8_t, need_result,
        inst_u16_t, identifier_number
);

XTAL_DEF_INST_4(58, SendIfDefined_AT,
        inst_u8_t, ordered,
        inst_u8_t, named,
        inst_u8_t, need_result,
        inst_u16_t, identifier_number
);

XTAL_DEF_INST_3(59, Call_AT,
        inst_u8_t, ordered,
        inst_u8_t, named,
        inst_u8_t, need_result
);

XTAL_DEF_INST_3(60, CallCallee_AT,
        inst_u8_t, ordered,
        inst_u8_t, named,
        inst_u8_t, need_result
);

XTAL_DEF_INST_1(61, BlockBegin,
        inst_u16_t, core_number
);

XTAL_DEF_INST_1(62, BlockBeginDirect,
        inst_u16_t, core_number
);

XTAL_DEF_INST_1(63, BlockEnd,
        inst_u16_t, core_number
);

XTAL_DEF_INST_1(64, BlockEndDirect,
        inst_u16_t, core_number
);

XTAL_DEF_INST_1(65, TryBegin,
        inst_u16_t, core_number
);

XTAL_DEF_INST_0(66, TryEnd);

XTAL_DEF_INST_1(67, PushGoto,
        inst_address_t, address
);

XTAL_DEF_INST_0(68, PopGoto);

XTAL_DEF_INST_1(69, IfEq,
        inst_address_t, address
);

XTAL_DEF_INST_1(70, IfNe,
        inst_address_t, address
);

XTAL_DEF_INST_1(71, IfLt,
        inst_address_t, address
);

XTAL_DEF_INST_1(72, IfLe,
        inst_address_t, address
);

XTAL_DEF_INST_1(73, IfGt,
        inst_address_t, address
);

XTAL_DEF_INST_1(74, IfGe,
        inst_address_t, address
);

XTAL_DEF_INST_1(75, IfRawEq,
        inst_address_t, address
);

XTAL_DEF_INST_1(76, IfRawNe,
        inst_address_t, address
);

XTAL_DEF_INST_1(77, IfIs,
        inst_address_t, address
);

XTAL_DEF_INST_1(78, IfNis,
        inst_address_t, address
);

XTAL_DEF_INST_2(79, IfArgIsNop,
	inst_u8_t, arg,
	inst_address_t, address
);

XTAL_DEF_INST_2(80, IfArgIsNopDirect,
	inst_u8_t, arg,
	inst_address_t, address
);

XTAL_DEF_INST_0(81, Pos);
XTAL_DEF_INST_0(82, Neg);
XTAL_DEF_INST_0(83, Com);
XTAL_DEF_INST_0(84, Not);

XTAL_DEF_INST_0(85, At);
XTAL_DEF_INST_0(86, SetAt);
XTAL_DEF_INST_0(87, Add);
XTAL_DEF_INST_0(88, Sub);
XTAL_DEF_INST_0(89, Cat);
XTAL_DEF_INST_0(90, Mul);
XTAL_DEF_INST_0(91, Div);
XTAL_DEF_INST_0(92, Mod);
XTAL_DEF_INST_0(93, And);
XTAL_DEF_INST_0(94, Or);
XTAL_DEF_INST_0(95, Xor);
XTAL_DEF_INST_0(96, Shl);
XTAL_DEF_INST_0(97, Shr);
XTAL_DEF_INST_0(98, Ushr);

XTAL_DEF_INST_0(99, Eq);
XTAL_DEF_INST_0(100, Ne);
XTAL_DEF_INST_0(101, Lt);
XTAL_DEF_INST_0(102, Le);
XTAL_DEF_INST_0(103, Gt);
XTAL_DEF_INST_0(104, Ge);
XTAL_DEF_INST_0(105, RawEq);
XTAL_DEF_INST_0(106, RawNe);
XTAL_DEF_INST_0(107, Is);
XTAL_DEF_INST_0(108, Nis);

XTAL_DEF_INST_0(109, Inc);
XTAL_DEF_INST_0(110, Dec);
XTAL_DEF_INST_0(111, AddAssign);
XTAL_DEF_INST_0(112, SubAssign);
XTAL_DEF_INST_0(113, CatAssign);
XTAL_DEF_INST_0(114, MulAssign);
XTAL_DEF_INST_0(115, DivAssign);
XTAL_DEF_INST_0(116, ModAssign);
XTAL_DEF_INST_0(117, AndAssign);
XTAL_DEF_INST_0(118, OrAssign);
XTAL_DEF_INST_0(119, XorAssign);
XTAL_DEF_INST_0(120, ShlAssign);
XTAL_DEF_INST_0(121, ShrAssign);
XTAL_DEF_INST_0(122, UshrAssign);

/**
* @brief グローバル変数を取り出す。
*
* stack [] -> [value]
*/
XTAL_DEF_INST_1(123, GlobalVariable,
        inst_u16_t, identifier_number
);

/**
* @brief グローバル変数に設定する。
*
* stack [value] -> []
*/
XTAL_DEF_INST_1(124, SetGlobalVariable,
        inst_u16_t, identifier_number
);

/**
* @brief グローバル変数を定義する。
*
* stack [value] -> []
*/
XTAL_DEF_INST_1(125, DefineGlobalVariable,
        inst_u16_t, identifier_number
);

/**
* @brief オブジェクトのメンバを取り出す。
*
* stack [object] -> [result]
*/
XTAL_DEF_INST_1(126, Member,
        inst_u16_t, identifier_number
);

/**
* @brief オブジェクトのメンバを取り出す。
*
* stack [object] -> [result]
*/
XTAL_DEF_INST_1(127, MemberIfDefined,
        inst_u16_t, identifier_number
);

/**
* @brief オブジェクトのメンバを定義する。
*
* stack [object, value] -> []
*/
XTAL_DEF_INST_1(128, DefineMember,
        inst_u16_t, identifier_number
);

XTAL_DEF_INST_3(129, DefineClassMember,
        inst_u16_t, number,
		inst_u16_t, identifier_number,
		inst_u8_t, accessibility
);

XTAL_DEF_INST_2(130, Once,
        inst_address_t, address,
        inst_u16_t, value_number
);

XTAL_DEF_INST_1(131, SetOnce,
        inst_u16_t, value_number
);

XTAL_DEF_INST_1(132, ClassBegin,
        inst_u16_t, core_number
);

XTAL_DEF_INST_0(133, ClassEnd);

XTAL_DEF_INST_0(134, MakeArray);

XTAL_DEF_INST_0(135, ArrayAppend);

XTAL_DEF_INST_0(136, MakeMap);

XTAL_DEF_INST_0(137, MapInsert);

XTAL_DEF_INST_2(138, MakeFun,
        inst_u16_t, core_number,
		inst_address_t, address
);

XTAL_DEF_INST_3(139, MakeInstanceVariableAccessor,
        inst_u8_t, type,
		inst_u8_t, number,
        inst_u16_t, core_number
);

XTAL_DEF_INST_0(140, Throw);

XTAL_DEF_INST_0(141, ThrowUnsupportedError);

XTAL_DEF_INST_0(142, ThrowNop);

XTAL_DEF_INST_0(143, Assert);

XTAL_DEF_INST_1(144, BreakPoint,
        inst_u8_t, type
);

XTAL_DEF_INST_0(145, SendToI);
XTAL_DEF_INST_0(146, SendToF);
XTAL_DEF_INST_0(147, SendToS);
XTAL_DEF_INST_0(148, SendToA);
XTAL_DEF_INST_0(149, SendToM);
XTAL_DEF_INST_0(150, SendLength);
XTAL_DEF_INST_0(151, SendSize);

XTAL_DEF_INST_0(152, MAX);

}
