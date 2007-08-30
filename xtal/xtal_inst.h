
#pragma once

#include "xtal_lib.h"
#include "xtal_smartptr.h"

namespace xtal{

#ifdef XTAL_USE_WORD_CODE

typedef int_t inst_i8_t;
typedef int_t inst_i16_t;
typedef uint_t inst_u8_t;
typedef uint_t inst_u16_t;
typedef uint_t inst_t;

#else

typedef i8 inst_i8_t;

struct inst_i16_t{
	u8 values[2];
	
	inst_i16_t(){}

	inst_i16_t(int_t value){
		values[0] = (value>>8) & 0xff;
		values[1] = (value>>0) & 0xff;
	}
	
	operator i16() const{
		return (i16)(values[0]<<8 | values[1]);
	}
};
	
typedef u8 inst_u8_t;

struct inst_u16_t{
	u8 values[2];
	
	inst_u16_t(){}

	inst_u16_t(uint_t value){
		values[0] = (value>>8) & 0xff;
		values[1] = (value>>0) & 0xff;
	}
	
	operator u16() const{
		return (u16)(values[0]<<8 | values[1]);
	}
};

typedef u8 inst_t;

#endif

typedef inst_i16_t inst_address_t;


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
		StringPtr inspect(){\
			return xnew<String>("" #InstName ":");\
		}\
	}

#define XTAL_DEF_INST_1(N, InstName, MemberType1, MemberName1) \
	struct Inst##InstName : public Inst{\
		enum{ \
			NUMBER = N,\
			SIZE = sizeof(Inst) + sizeof(MemberType1),\
			ISIZE = SIZE/sizeof(inst_t)\
		};\
		MemberType1 MemberName1;\
		Inst##InstName(){\
			op = N;\
		}\
		Inst##InstName(int m1){\
			op = N;\
			checked_assign(MemberName1, m1);\
		}\
		StringPtr inspect(){\
			return format("" #InstName ": "\
					#MemberName1 "=%s")\
				((int_t)MemberName1)->to_s();\
		}\
	}

#define XTAL_DEF_INST_2(N, InstName, MemberType1, MemberName1, MemberType2, MemberName2) \
	struct Inst##InstName : public Inst{\
		enum{ \
			NUMBER = N,\
			SIZE = sizeof(Inst) + sizeof(MemberType1) + sizeof(MemberType2),\
			ISIZE = SIZE/sizeof(inst_t)\
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
		StringPtr inspect(){\
			return format("" #InstName ": "\
					#MemberName1 "=%s, "\
					#MemberName2 "=%s")\
				((int_t)MemberName1, (int_t)MemberName2)->to_s();\
		}\
	}
	
#define XTAL_DEF_INST_3(N, InstName, MemberType1, MemberName1, MemberType2, MemberName2, MemberType3, MemberName3) \
	struct Inst##InstName : public Inst{\
		enum{ \
			NUMBER = N,\
			SIZE = sizeof(Inst) + sizeof(MemberType1) + sizeof(MemberType2) + sizeof(MemberType3),\
			ISIZE = SIZE/sizeof(inst_t)\
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
		StringPtr inspect(){\
			return format("" #InstName ": "\
					#MemberName1 "=%s, "\
					#MemberName2 "=%s, "\
					#MemberName3 "=%s")\
				((int_t)MemberName1, (int_t)MemberName2, (int_t)MemberName3)->to_s();\
		}\
	}
	
#define XTAL_DEF_INST_4(N, InstName, MemberType1, MemberName1, MemberType2, MemberName2, MemberType3, MemberName3, MemberType4, MemberName4) \
	struct Inst##InstName : public Inst{\
		enum{ \
			NUMBER = N,\
			SIZE = sizeof(Inst) + sizeof(MemberType1) + sizeof(MemberType2) + sizeof(MemberType3) + sizeof(MemberType4),\
			ISIZE = SIZE/sizeof(inst_t)\
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
		StringPtr inspect(){\
			return format("" #InstName ": "\
					#MemberName1 "=%s, "\
					#MemberName2 "=%s, "\
					#MemberName3 "=%s, "\
					#MemberName4 "=%s")\
				((int_t)MemberName1, (int_t)MemberName2, (int_t)MemberName3, (int_t)MemberName4)->to_s();\
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
		inst_u16_t, number
);

XTAL_DEF_INST_1(23, LocalVariableDec,
		inst_u16_t, number
);

XTAL_DEF_INST_1(24, LocalVariableIncDirect,
		inst_u8_t, number
);

XTAL_DEF_INST_1(25, LocalVariableDecDirect,
		inst_u8_t, number
);

/**
* @brief ローカル変数を取り出す。
*
* stack [] -> [value]
*/
XTAL_DEF_INST_1(26, LocalVariable1ByteDirect,
        inst_u8_t, number
);

/**
* @brief ローカル変数を取り出す。
*
* stack [] -> [value]
*/
XTAL_DEF_INST_1(27, LocalVariable1Byte,
        inst_u8_t, number
);

/**
* @brief ローカル変数を取り出す。
*
* stack [] -> [value]
*/
XTAL_DEF_INST_1(28, LocalVariable2Byte,
        inst_u16_t, number
);

/**
* @brief ローカル変数に値を設定する。
*
* stack [value] -> []
*/
XTAL_DEF_INST_1(29, SetLocalVariable1ByteDirect,
        inst_u8_t, number
);

/**
* @brief ローカル変数に値を設定する。
*
* stack [value] -> []
*/
XTAL_DEF_INST_1(30, SetLocalVariable1Byte,
        inst_u8_t, number
);

/**
* @brief ローカル変数に値を設定する。
*
* stack [value] -> []
*/
XTAL_DEF_INST_1(31, SetLocalVariable2Byte,
        inst_u16_t, number
);

/**
* @brief インスタンス変数を取り出す。
*
* stack [] -> [value]
*/
XTAL_DEF_INST_2(32, InstanceVariable,
        inst_u8_t, number,
        inst_u16_t, core_number
);

/**
* @brief インスタンス変数に値を設定する。
*
* stack [value] -> []
*/
XTAL_DEF_INST_2(33, SetInstanceVariable,
        inst_u8_t, number,
        inst_u16_t, core_number
);

/**
* @brief 関数呼び出しの後始末をする
*
* stack [] -> []
*/
XTAL_DEF_INST_0(34, CleanupCall);

/**
* @brief 関数から抜ける
*
* stack [] -> []
*/
XTAL_DEF_INST_0(35, Return0);

/**
* @brief 関数から抜ける
*
* stack [value1] -> []
*/
XTAL_DEF_INST_0(36, Return1);

/**
* @brief 関数から抜ける
*
* stack [value1, value2] -> []
*/
XTAL_DEF_INST_0(37, Return2);

/**
* @brief 関数から抜ける
*
* stack [value1, value2, ...valueN] -> []
*/
XTAL_DEF_INST_1(38, Return,
        inst_u8_t, results
);

/**
* @brief fiberの実行を一時中断する
*
* stack [value1, value2, ...valueN] -> []
*/
XTAL_DEF_INST_1(39, Yield,
        inst_u16_t, results
);

/**
* @brief 仮想マシンのループから脱出する。
*
* stack [] -> []
*/
XTAL_DEF_INST_0(40, Exit);

/**
* @brief 値テーブルから値を取り出す
*
* stack [] -> [value]
*/
XTAL_DEF_INST_1(41, Value,
        inst_u16_t, value_number
);

XTAL_DEF_INST_0(42, CheckUnsupported);

// A=args flag, T=tail flag

XTAL_DEF_INST_4(43, Send,
        inst_u8_t, ordered_arg_count,
        inst_u8_t, named_arg_count,
        inst_u8_t, need_result_count,
        inst_u16_t, symbol_number
);

XTAL_DEF_INST_4(44, SendIfDefined,
        inst_u8_t, ordered_arg_count,
        inst_u8_t, named_arg_count,
        inst_u8_t, need_result_count,
        inst_u16_t, symbol_number
);

XTAL_DEF_INST_3(45, Call,
        inst_u8_t, ordered_arg_count,
        inst_u8_t, named_arg_count,
        inst_u8_t, need_result_count
);

XTAL_DEF_INST_3(46, CallCallee,
        inst_u8_t, ordered_arg_count,
        inst_u8_t, named_arg_count,
        inst_u8_t, need_result_count
);

XTAL_DEF_INST_4(47, Send_A,
        inst_u8_t, ordered_arg_count,
        inst_u8_t, named_arg_count,
        inst_u8_t, need_result_count,
        inst_u16_t, symbol_number
);

XTAL_DEF_INST_4(48, SendIfDefined_A,
        inst_u8_t, ordered_arg_count,
        inst_u8_t, named_arg_count,
        inst_u8_t, need_result_count,
        inst_u16_t, symbol_number
);

XTAL_DEF_INST_3(49, Call_A,
        inst_u8_t, ordered_arg_count,
        inst_u8_t, named_arg_count,
        inst_u8_t, need_result_count
);

XTAL_DEF_INST_3(50, CallCallee_A,
        inst_u8_t, ordered_arg_count,
        inst_u8_t, named_arg_count,
        inst_u8_t, need_result_count
);

XTAL_DEF_INST_4(51, Send_T,
        inst_u8_t, ordered_arg_count,
        inst_u8_t, named_arg_count,
        inst_u8_t, need_result_count,
        inst_u16_t, symbol_number
);

XTAL_DEF_INST_4(52, SendIfDefined_T,
        inst_u8_t, ordered_arg_count,
        inst_u8_t, named_arg_count,
        inst_u8_t, need_result_count,
        inst_u16_t, symbol_number
);

XTAL_DEF_INST_3(53, Call_T,
        inst_u8_t, ordered_arg_count,
        inst_u8_t, named_arg_count,
        inst_u8_t, need_result_count
);

XTAL_DEF_INST_3(54, CallCallee_T,
        inst_u8_t, ordered_arg_count,
        inst_u8_t, named_arg_count,
        inst_u8_t, need_result_count
);

XTAL_DEF_INST_4(55, Send_AT,
        inst_u8_t, ordered_arg_count,
        inst_u8_t, named_arg_count,
        inst_u8_t, need_result_count,
        inst_u16_t, symbol_number
);

XTAL_DEF_INST_4(56, SendIfDefined_AT,
        inst_u8_t, ordered_arg_count,
        inst_u8_t, named_arg_count,
        inst_u8_t, need_result_count,
        inst_u16_t, symbol_number
);

XTAL_DEF_INST_3(57, Call_AT,
        inst_u8_t, ordered_arg_count,
        inst_u8_t, named_arg_count,
        inst_u8_t, need_result_count
);

XTAL_DEF_INST_3(58, CallCallee_AT,
        inst_u8_t, ordered_arg_count,
        inst_u8_t, named_arg_count,
        inst_u8_t, need_result_count
);

XTAL_DEF_INST_1(59, BlockBegin,
        inst_u16_t, core_number
);

XTAL_DEF_INST_0(60, BlockEnd);

XTAL_DEF_INST_1(61, BlockBeginDirect,
        inst_u16_t, variable_size
);

XTAL_DEF_INST_1(62, BlockEndDirect,
        inst_u16_t, variable_size
);

XTAL_DEF_INST_1(63, TryBegin,
        inst_u16_t, core_number
);

XTAL_DEF_INST_0(64, TryEnd);

XTAL_DEF_INST_1(65, PushGoto,
        inst_address_t, address
);

XTAL_DEF_INST_0(66, PopGoto);

XTAL_DEF_INST_1(67, IfEq,
        inst_address_t, address
);

XTAL_DEF_INST_1(68, IfNe,
        inst_address_t, address
);

XTAL_DEF_INST_1(69, IfLt,
        inst_address_t, address
);

XTAL_DEF_INST_1(70, IfLe,
        inst_address_t, address
);

XTAL_DEF_INST_1(71, IfGt,
        inst_address_t, address
);

XTAL_DEF_INST_1(72, IfGe,
        inst_address_t, address
);

XTAL_DEF_INST_1(73, IfRawEq,
        inst_address_t, address
);

XTAL_DEF_INST_1(74, IfRawNe,
        inst_address_t, address
);

XTAL_DEF_INST_1(75, IfIs,
        inst_address_t, address
);

XTAL_DEF_INST_1(76, IfNis,
        inst_address_t, address
);

XTAL_DEF_INST_2(77, IfArgIsNull,
	inst_u8_t, arg,
	inst_address_t, address
);

XTAL_DEF_INST_2(78, IfArgIsNullDirect,
	inst_u8_t, arg,
	inst_address_t, address
);

XTAL_DEF_INST_0(79, Pos);
XTAL_DEF_INST_0(80, Neg);
XTAL_DEF_INST_0(81, Com);
XTAL_DEF_INST_0(82, Not);

XTAL_DEF_INST_0(83, At);
XTAL_DEF_INST_0(84, SetAt);
XTAL_DEF_INST_0(85, Add);
XTAL_DEF_INST_0(86, Sub);
XTAL_DEF_INST_0(87, Cat);
XTAL_DEF_INST_0(88, Mul);
XTAL_DEF_INST_0(89, Div);
XTAL_DEF_INST_0(90, Mod);
XTAL_DEF_INST_0(91, And);
XTAL_DEF_INST_0(92, Or);
XTAL_DEF_INST_0(93, Xor);
XTAL_DEF_INST_0(94, Shl);
XTAL_DEF_INST_0(95, Shr);
XTAL_DEF_INST_0(96, Ushr);

XTAL_DEF_INST_0(97, Eq);
XTAL_DEF_INST_0(98, Ne);
XTAL_DEF_INST_0(99, Lt);
XTAL_DEF_INST_0(100, Le);
XTAL_DEF_INST_0(101, Gt);
XTAL_DEF_INST_0(102, Ge);
XTAL_DEF_INST_0(103, RawEq);
XTAL_DEF_INST_0(104, RawNe);
XTAL_DEF_INST_0(105, Is);
XTAL_DEF_INST_0(106, Nis);

XTAL_DEF_INST_0(107, Inc);
XTAL_DEF_INST_0(108, Dec);
XTAL_DEF_INST_0(109, AddAssign);
XTAL_DEF_INST_0(110, SubAssign);
XTAL_DEF_INST_0(111, CatAssign);
XTAL_DEF_INST_0(112, MulAssign);
XTAL_DEF_INST_0(113, DivAssign);
XTAL_DEF_INST_0(114, ModAssign);
XTAL_DEF_INST_0(115, AndAssign);
XTAL_DEF_INST_0(116, OrAssign);
XTAL_DEF_INST_0(117, XorAssign);
XTAL_DEF_INST_0(118, ShlAssign);
XTAL_DEF_INST_0(119, ShrAssign);
XTAL_DEF_INST_0(120, UshrAssign);

/**
* @brief グローバル変数を取り出す。
*
* stack [] -> [value]
*/
XTAL_DEF_INST_1(121, GlobalVariable,
        inst_u16_t, symbol_number
);

/**
* @brief グローバル変数に設定する。
*
* stack [value] -> []
*/
XTAL_DEF_INST_1(122, SetGlobalVariable,
        inst_u16_t, symbol_number
);

/**
* @brief グローバル変数を定義する。
*
* stack [value] -> []
*/
XTAL_DEF_INST_1(123, DefineGlobalVariable,
        inst_u16_t, symbol_number
);

/**
* @brief オブジェクトのメンバを取り出す。
*
* stack [object] -> [result]
*/
XTAL_DEF_INST_1(124, Member,
        inst_u16_t, symbol_number
);

/**
* @brief オブジェクトのメンバを取り出す。
*
* stack [object] -> [result]
*/
XTAL_DEF_INST_1(125, MemberIfDefined,
        inst_u16_t, symbol_number
);

/**
* @brief オブジェクトのメンバを定義する。
*
* stack [object, value] -> []
*/
XTAL_DEF_INST_1(126, DefineMember,
        inst_u16_t, symbol_number
);

XTAL_DEF_INST_3(127, DefineClassMember,
        inst_u16_t, number,
		inst_u16_t, symbol_number,
		inst_u8_t, accessibility
);

XTAL_DEF_INST_1(128, SetName,
        inst_u16_t, symbol_number
);

XTAL_DEF_INST_2(129, SetMultipleLocalVariable2Direct,
        inst_u8_t, local_variable1,
        inst_u8_t, local_variable2
);

XTAL_DEF_INST_3(130, SetMultipleLocalVariable3Direct,
        inst_u8_t, local_variable1,
        inst_u8_t, local_variable2,
        inst_u8_t, local_variable3
);

XTAL_DEF_INST_4(131, SetMultipleLocalVariable4Direct,
        inst_u8_t, local_variable1,
        inst_u8_t, local_variable2,
        inst_u8_t, local_variable3,
        inst_u8_t, local_variable4
);

XTAL_DEF_INST_2(132, Once,
        inst_address_t, address,
        inst_u16_t, value_number
);

XTAL_DEF_INST_1(133, SetOnce,
        inst_u16_t, value_number
);

XTAL_DEF_INST_2(134, ClassBegin,
        inst_u16_t, core_number,
		inst_u8_t, mixins
);

XTAL_DEF_INST_0(135, ClassEnd);

XTAL_DEF_INST_0(136, MakeArray);

XTAL_DEF_INST_0(137, ArrayAppend);

XTAL_DEF_INST_0(138, MakeMap);

XTAL_DEF_INST_0(139, MapInsert);

XTAL_DEF_INST_2(140, MakeFun,
        inst_u16_t, core_number,
		inst_address_t, address
);

XTAL_DEF_INST_3(141, MakeInstanceVariableAccessor,
        inst_u8_t, type,
		inst_u8_t, number,
        inst_u16_t, core_number
);

XTAL_DEF_INST_0(142, Throw);

XTAL_DEF_INST_0(143, ThrowUnsupportedError);

XTAL_DEF_INST_0(144, ThrowNull);

XTAL_DEF_INST_0(145, Assert);

XTAL_DEF_INST_1(146, BreakPoint,
        inst_u8_t, type
);

XTAL_DEF_INST_0(147, MAX);

}
