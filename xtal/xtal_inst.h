
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
* @brief スタックにtrue値をプッシュする。
*
* stack [] -> [value]
*/
XTAL_DEF_INST_0(2, PushTrue);

/**
* @brief スタックにfalse値をプッシュする。
*
* stack [] -> [value]
*/
XTAL_DEF_INST_0(3, PushFalse);

/**
* @brief スタックにint値をプッシュする。
*
* stack [] -> [value]
*/
XTAL_DEF_INST_1(4, PushInt1Byte,
        inst_i8_t, value
);

/**
* @brief スタックにint値をプッシュする。
*
* stack [] -> [value]
*/
XTAL_DEF_INST_1(5, PushInt2Byte,
        inst_i16_t, value
);

/**
* @brief スタックにfloat値をプッシュする。
*
* stack [] -> [value]
*/
XTAL_DEF_INST_1(6, PushFloat1Byte,
        inst_i8_t, value
);

/**
* @brief スタックにfloat値をプッシュする。
*
* stack [] -> [value]
*/
XTAL_DEF_INST_1(7, PushFloat2Byte,
        inst_i16_t, value
);

/**
* @brief スタックに現在実行中の関数をプッシュする。
*
* stack [] -> [value]
*/
XTAL_DEF_INST_0(8, PushCallee);

/**
* @brief スタックにArgumentsオブジェクトをプッシュする。
*
* stack [] -> [value]
*/
XTAL_DEF_INST_0(9, PushArgs);

/**
* @brief 現在のthisをスタックに積む。
*
* stack [] -> [value]
*/
XTAL_DEF_INST_0(10, PushThis);

/**
* @brief 現在の文脈をスタックに積む。
*
* stack [] -> [value]
*/
XTAL_DEF_INST_0(11, PushCurrentContext);

/**
* @brief スタックトップをポップする
*
* stack [value] -> []
*/
XTAL_DEF_INST_0(12, Pop);

/**
* @brief スタックトップの値をプッシュする
*
* stack [value] -> [value, value2]
*/
XTAL_DEF_INST_0(13, Dup);

/**
* @brief スタックトップの値を一つ前に入れる。
*
* stack [value1, value2] -> [value2, value1]
*/
XTAL_DEF_INST_0(14, Insert1);

/**
* @brief スタックトップの値を二つ前に入れる。
*
* stack [value1, value2, value3] -> [value3, value1, value2]
*/
XTAL_DEF_INST_0(15, Insert2);

/**
* @brief スタックトップの値を三つ前に入れる。
*
* stack [value1, value2, value3, value4] -> [value4, value1, value2, value3]
*/
XTAL_DEF_INST_0(16, Insert3);

/**
* @brief 値の数を調整する
*
* stack [] -> []
*/
XTAL_DEF_INST_2(17, AdjustResult,
        inst_u8_t, result_count,
        inst_u8_t, need_result_count
);

/**
* @brief 条件分岐
*
* stack [value] -> []
*/
XTAL_DEF_INST_1(18, If,
        inst_address_t, address
);

/**
* @brief 条件分岐
*
* stack [value] -> []
*/
XTAL_DEF_INST_1(19, Unless,
        inst_address_t, address
);

/**
* @brief 無条件分岐
*
* stack [] -> []
*/
XTAL_DEF_INST_1(20, Goto,
        inst_address_t, address
);

XTAL_DEF_INST_1(21, LocalVariableInc,
		inst_u16_t, number
);

XTAL_DEF_INST_1(22, LocalVariableDec,
		inst_u16_t, number
);

XTAL_DEF_INST_1(23, LocalVariableIncDirect,
		inst_u8_t, number
);

XTAL_DEF_INST_1(24, LocalVariableDecDirect,
		inst_u8_t, number
);

/**
* @brief ローカル変数を取り出す。
*
* stack [] -> [value]
*/
XTAL_DEF_INST_1(25, LocalVariable1ByteDirect,
        inst_u8_t, number
);

/**
* @brief ローカル変数を取り出す。
*
* stack [] -> [value]
*/
XTAL_DEF_INST_1(26, LocalVariable1Byte,
        inst_u8_t, number
);

/**
* @brief ローカル変数を取り出す。
*
* stack [] -> [value]
*/
XTAL_DEF_INST_1(27, LocalVariable2Byte,
        inst_u16_t, number
);

/**
* @brief ローカル変数に値を設定する。
*
* stack [value] -> []
*/
XTAL_DEF_INST_1(28, SetLocalVariable1ByteDirect,
        inst_u8_t, number
);

/**
* @brief ローカル変数に値を設定する。
*
* stack [value] -> []
*/
XTAL_DEF_INST_1(29, SetLocalVariable1Byte,
        inst_u8_t, number
);

/**
* @brief ローカル変数に値を設定する。
*
* stack [value] -> []
*/
XTAL_DEF_INST_1(30, SetLocalVariable2Byte,
        inst_u16_t, number
);

/**
* @brief インスタンス変数を取り出す。
*
* stack [] -> [value]
*/
XTAL_DEF_INST_2(31, InstanceVariable,
        inst_u8_t, number,
        inst_u16_t, core_number
);

/**
* @brief インスタンス変数に値を設定する。
*
* stack [value] -> []
*/
XTAL_DEF_INST_2(32, SetInstanceVariable,
        inst_u8_t, number,
        inst_u16_t, core_number
);

/**
* @brief 関数呼び出しの後始末をする
*
* stack [] -> []
*/
XTAL_DEF_INST_0(33, CleanupCall);

/**
* @brief 関数から抜ける
*
* stack [] -> []
*/
XTAL_DEF_INST_0(34, Return0);

/**
* @brief 関数から抜ける
*
* stack [value1] -> []
*/
XTAL_DEF_INST_0(35, Return1);

/**
* @brief 関数から抜ける
*
* stack [value1, value2] -> []
*/
XTAL_DEF_INST_0(36, Return2);

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

// A=args flag, T=tail flag

XTAL_DEF_INST_4(42, Send,
        inst_u8_t, ordered_arg_count,
        inst_u8_t, named_arg_count,
        inst_u8_t, need_result_count,
        inst_u16_t, symbol_number
);

XTAL_DEF_INST_4(43, SendIfDefined,
        inst_u8_t, ordered_arg_count,
        inst_u8_t, named_arg_count,
        inst_u8_t, need_result_count,
        inst_u16_t, symbol_number
);

XTAL_DEF_INST_3(44, Call,
        inst_u8_t, ordered_arg_count,
        inst_u8_t, named_arg_count,
        inst_u8_t, need_result_count
);

XTAL_DEF_INST_3(45, CallCallee,
        inst_u8_t, ordered_arg_count,
        inst_u8_t, named_arg_count,
        inst_u8_t, need_result_count
);

XTAL_DEF_INST_4(46, Send_A,
        inst_u8_t, ordered_arg_count,
        inst_u8_t, named_arg_count,
        inst_u8_t, need_result_count,
        inst_u16_t, symbol_number
);

XTAL_DEF_INST_4(47, SendIfDefined_A,
        inst_u8_t, ordered_arg_count,
        inst_u8_t, named_arg_count,
        inst_u8_t, need_result_count,
        inst_u16_t, symbol_number
);

XTAL_DEF_INST_3(48, Call_A,
        inst_u8_t, ordered_arg_count,
        inst_u8_t, named_arg_count,
        inst_u8_t, need_result_count
);

XTAL_DEF_INST_3(49, CallCallee_A,
        inst_u8_t, ordered_arg_count,
        inst_u8_t, named_arg_count,
        inst_u8_t, need_result_count
);

XTAL_DEF_INST_4(50, Send_T,
        inst_u8_t, ordered_arg_count,
        inst_u8_t, named_arg_count,
        inst_u8_t, need_result_count,
        inst_u16_t, symbol_number
);

XTAL_DEF_INST_4(51, SendIfDefined_T,
        inst_u8_t, ordered_arg_count,
        inst_u8_t, named_arg_count,
        inst_u8_t, need_result_count,
        inst_u16_t, symbol_number
);

XTAL_DEF_INST_3(52, Call_T,
        inst_u8_t, ordered_arg_count,
        inst_u8_t, named_arg_count,
        inst_u8_t, need_result_count
);

XTAL_DEF_INST_3(53, CallCallee_T,
        inst_u8_t, ordered_arg_count,
        inst_u8_t, named_arg_count,
        inst_u8_t, need_result_count
);

XTAL_DEF_INST_4(54, Send_AT,
        inst_u8_t, ordered_arg_count,
        inst_u8_t, named_arg_count,
        inst_u8_t, need_result_count,
        inst_u16_t, symbol_number
);

XTAL_DEF_INST_4(55, SendIfDefined_AT,
        inst_u8_t, ordered_arg_count,
        inst_u8_t, named_arg_count,
        inst_u8_t, need_result_count,
        inst_u16_t, symbol_number
);

XTAL_DEF_INST_3(56, Call_AT,
        inst_u8_t, ordered_arg_count,
        inst_u8_t, named_arg_count,
        inst_u8_t, need_result_count
);

XTAL_DEF_INST_3(57, CallCallee_AT,
        inst_u8_t, ordered_arg_count,
        inst_u8_t, named_arg_count,
        inst_u8_t, need_result_count
);

XTAL_DEF_INST_1(58, BlockBegin,
        inst_u16_t, core_number
);

XTAL_DEF_INST_0(59, BlockEnd);

XTAL_DEF_INST_1(60, BlockBeginDirect,
        inst_u16_t, variable_size
);

XTAL_DEF_INST_1(61, BlockEndDirect,
        inst_u16_t, variable_size
);

XTAL_DEF_INST_1(62, TryBegin,
        inst_u16_t, core_number
);

XTAL_DEF_INST_0(63, TryEnd);

XTAL_DEF_INST_1(64, PushGoto,
        inst_address_t, address
);

XTAL_DEF_INST_0(65, PopGoto);

XTAL_DEF_INST_1(66, IfEq,
        inst_address_t, address
);

XTAL_DEF_INST_1(67, IfNe,
        inst_address_t, address
);

XTAL_DEF_INST_1(68, IfLt,
        inst_address_t, address
);

XTAL_DEF_INST_1(69, IfLe,
        inst_address_t, address
);

XTAL_DEF_INST_1(70, IfGt,
        inst_address_t, address
);

XTAL_DEF_INST_1(71, IfGe,
        inst_address_t, address
);

XTAL_DEF_INST_1(72, IfRawEq,
        inst_address_t, address
);

XTAL_DEF_INST_1(73, IfRawNe,
        inst_address_t, address
);

XTAL_DEF_INST_1(74, IfIs,
        inst_address_t, address
);

XTAL_DEF_INST_1(75, IfNis,
        inst_address_t, address
);

XTAL_DEF_INST_2(76, IfArgIsNull,
	inst_u8_t, arg,
	inst_address_t, address
);

XTAL_DEF_INST_2(77, IfArgIsNullDirect,
	inst_u8_t, arg,
	inst_address_t, address
);

XTAL_DEF_INST_0(78, Pos);
XTAL_DEF_INST_0(79, Neg);
XTAL_DEF_INST_0(80, Com);
XTAL_DEF_INST_0(81, Not);

XTAL_DEF_INST_0(82, At);
XTAL_DEF_INST_0(83, SetAt);
XTAL_DEF_INST_0(84, Add);
XTAL_DEF_INST_0(85, Sub);
XTAL_DEF_INST_0(86, Cat);
XTAL_DEF_INST_0(87, Mul);
XTAL_DEF_INST_0(88, Div);
XTAL_DEF_INST_0(89, Mod);
XTAL_DEF_INST_0(90, And);
XTAL_DEF_INST_0(91, Or);
XTAL_DEF_INST_0(92, Xor);
XTAL_DEF_INST_0(93, Shl);
XTAL_DEF_INST_0(94, Shr);
XTAL_DEF_INST_0(95, Ushr);

XTAL_DEF_INST_0(96, Eq);
XTAL_DEF_INST_0(97, Ne);
XTAL_DEF_INST_0(98, Lt);
XTAL_DEF_INST_0(99, Le);
XTAL_DEF_INST_0(100, Gt);
XTAL_DEF_INST_0(101, Ge);
XTAL_DEF_INST_0(102, RawEq);
XTAL_DEF_INST_0(103, RawNe);
XTAL_DEF_INST_0(104, Is);
XTAL_DEF_INST_0(105, Nis);

XTAL_DEF_INST_0(106, Inc);
XTAL_DEF_INST_0(107, Dec);
XTAL_DEF_INST_0(108, AddAssign);
XTAL_DEF_INST_0(109, SubAssign);
XTAL_DEF_INST_0(110, CatAssign);
XTAL_DEF_INST_0(111, MulAssign);
XTAL_DEF_INST_0(112, DivAssign);
XTAL_DEF_INST_0(113, ModAssign);
XTAL_DEF_INST_0(114, AndAssign);
XTAL_DEF_INST_0(115, OrAssign);
XTAL_DEF_INST_0(116, XorAssign);
XTAL_DEF_INST_0(117, ShlAssign);
XTAL_DEF_INST_0(118, ShrAssign);
XTAL_DEF_INST_0(119, UshrAssign);

/**
* @brief グローバル変数を取り出す。
*
* stack [] -> [value]
*/
XTAL_DEF_INST_1(120, GlobalVariable,
        inst_u16_t, symbol_number
);

/**
* @brief グローバル変数に設定する。
*
* stack [value] -> []
*/
XTAL_DEF_INST_1(121, SetGlobalVariable,
        inst_u16_t, symbol_number
);

/**
* @brief グローバル変数を定義する。
*
* stack [value] -> []
*/
XTAL_DEF_INST_1(122, DefineGlobalVariable,
        inst_u16_t, symbol_number
);

/**
* @brief オブジェクトのメンバを取り出す。
*
* stack [object] -> [result]
*/
XTAL_DEF_INST_1(123, Member,
        inst_u16_t, symbol_number
);

/**
* @brief オブジェクトのメンバを取り出す。
*
* stack [object] -> [result]
*/
XTAL_DEF_INST_1(124, MemberIfDefined,
        inst_u16_t, symbol_number
);

/**
* @brief オブジェクトのメンバを定義する。
*
* stack [object, value] -> []
*/
XTAL_DEF_INST_1(125, DefineMember,
        inst_u16_t, symbol_number
);

XTAL_DEF_INST_3(126, DefineClassMember,
        inst_u16_t, number,
		inst_u16_t, symbol_number,
		inst_u8_t, accessibility
);

XTAL_DEF_INST_1(127, SetName,
        inst_u16_t, symbol_number
);

XTAL_DEF_INST_2(128, SetMultipleLocalVariable2Direct,
        inst_u8_t, local_variable1,
        inst_u8_t, local_variable2
);

XTAL_DEF_INST_3(129, SetMultipleLocalVariable3Direct,
        inst_u8_t, local_variable1,
        inst_u8_t, local_variable2,
        inst_u8_t, local_variable3
);

XTAL_DEF_INST_4(130, SetMultipleLocalVariable4Direct,
        inst_u8_t, local_variable1,
        inst_u8_t, local_variable2,
        inst_u8_t, local_variable3,
        inst_u8_t, local_variable4
);

XTAL_DEF_INST_2(131, Once,
        inst_address_t, address,
        inst_u16_t, value_number
);

XTAL_DEF_INST_1(132, SetOnce,
        inst_u16_t, value_number
);

XTAL_DEF_INST_2(133, ClassBegin,
        inst_u16_t, core_number,
		inst_u8_t, mixins
);

XTAL_DEF_INST_0(134, ClassEnd);

XTAL_DEF_INST_0(135, MakeArray);

XTAL_DEF_INST_0(136, ArrayAppend);

XTAL_DEF_INST_0(137, MakeMap);

XTAL_DEF_INST_0(138, MapInsert);

XTAL_DEF_INST_2(139, MakeFun,
        inst_u16_t, core_number,
		inst_address_t, address
);

XTAL_DEF_INST_3(140, MakeInstanceVariableAccessor,
        inst_u8_t, type,
		inst_u8_t, number,
        inst_u16_t, core_number
);

XTAL_DEF_INST_0(141, Throw);

XTAL_DEF_INST_0(142, ThrowUnsupportedError);

XTAL_DEF_INST_0(143, ThrowNull);

XTAL_DEF_INST_0(144, Assert);

XTAL_DEF_INST_1(145, BreakPoint,
        inst_u8_t, type
);

XTAL_DEF_INST_0(146, MAX);

}
