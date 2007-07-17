
#pragma once

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
	};

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
		Inst##InstName(MemberType1 m1){\
			op = N;\
			MemberName1 = m1;\
		}\
	};

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
		Inst##InstName(MemberType1 m1, MemberType2 m2){\
			op = N;\
			MemberName1 = m1;\
			MemberName2 = m2;\
		}\
	};
	
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
		Inst##InstName(MemberType1 m1, MemberType2 m2, MemberType3 m3){\
			op = N;\
			MemberName1 = m1;\
			MemberName2 = m2;\
			MemberName3 = m3;\
		}\
	};
	
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
		Inst##InstName(MemberType1 m1, MemberType2 m2, MemberType3 m3, MemberType4 m4){\
			op = N;\
			MemberName1 = m1;\
			MemberName2 = m2;\
			MemberName3 = m3;\
			MemberName4 = m4;\
		}\
	};
	
#define XTAL_DEF_INST_5(N, InstName, MemberType1, MemberName1, MemberType2, MemberName2, MemberType3, MemberName3, MemberType4, MemberName4, MemberType5, MemberName5) \
	struct Inst##InstName : public Inst{\
		enum{ \
			NUMBER = N,\
			SIZE = sizeof(Inst) + sizeof(MemberType1) + sizeof(MemberType2) + sizeof(MemberType3) + sizeof(MemberType4) + sizeof(MemberType5),\
			ISIZE = SIZE/sizeof(inst_t)\
		};\
		MemberType1 MemberName1;\
		MemberType2 MemberName2;\
		MemberType3 MemberName3;\
		MemberType4 MemberName4;\
		MemberType5 MemberName5;\
		Inst##InstName(){\
			op = N;\
		}\
		Inst##InstName(MemberType1 m1, MemberType2 m2, MemberType3 m3, MemberType4 m4, MemberType5 m5){\
			op = N;\
			MemberName1 = m1;\
			MemberName2 = m2;\
			MemberName3 = m3;\
			MemberName4 = m4;\
			MemberName5 = m5;\
		}\
	};
	
#define XTAL_DEF_INST_6(N, InstName, MemberType1, MemberName1, MemberType2, MemberName2, MemberType3, MemberName3, MemberType4, MemberName4, MemberType5, MemberName5, MemberType6, MemberName6) \
	struct Inst##InstName : public Inst{\
		enum{ \
			NUMBER = N,\
			SIZE = sizeof(Inst) + sizeof(MemberType1) + sizeof(MemberType2) + sizeof(MemberType3) + sizeof(MemberType4) + sizeof(MemberType5) + sizeof(MemberType6),\
			ISIZE = SIZE/sizeof(inst_t)\
		};\
		MemberType1 MemberName1;\
		MemberType2 MemberName2;\
		MemberType3 MemberName3;\
		MemberType4 MemberName4;\
		MemberType5 MemberName5;\
		MemberType6 MemberName6;\
		Inst##InstName(){\
			op = N;\
		}\
		Inst##InstName(MemberType1 m1, MemberType2 m2, MemberType3 m3, MemberType4 m4, MemberType5 m5, MemberType6 m6){\
			op = N;\
			MemberName1 = m1;\
			MemberName2 = m2;\
			MemberName3 = m3;\
			MemberName4 = m4;\
			MemberName5 = m5;\
			MemberName6 = m6;\
		}\
	};
	

XTAL_DEF_INST_0(0, Nop);

XTAL_DEF_INST_0(1, PushNull);
XTAL_DEF_INST_0(2, PushNop);
XTAL_DEF_INST_0(3, PushTrue);
XTAL_DEF_INST_0(4, PushFalse);

XTAL_DEF_INST_1(5, PushInt1Byte,
        inst_u8_t, value
);
XTAL_DEF_INST_1(6, PushInt2Byte,
        inst_u16_t, value
);

XTAL_DEF_INST_1(7, PushFloat1Byte,
        inst_u8_t, value
);
XTAL_DEF_INST_1(8, PushFloat2Byte,
        inst_u16_t, value
);

XTAL_DEF_INST_0(9, PushCallee);

XTAL_DEF_INST_0(10, PushArgs);
XTAL_DEF_INST_0(11, PushThis);
XTAL_DEF_INST_0(12, PushCurrentContext);
XTAL_DEF_INST_0(13, Pop);
XTAL_DEF_INST_0(14, Dup);
XTAL_DEF_INST_0(15, Insert1);
XTAL_DEF_INST_0(16, Insert2);
XTAL_DEF_INST_0(17, Insert3);

XTAL_DEF_INST_2(18, AdjustResult,
        inst_u8_t, result_count,
        inst_u8_t, need_result_count
);

XTAL_DEF_INST_1(19, If,
        inst_address_t, address
);
XTAL_DEF_INST_1(20, Unless,
        inst_address_t, address
);
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
XTAL_DEF_INST_1(26, LocalVariable1ByteDirect,
        inst_u8_t, number
);
XTAL_DEF_INST_1(27, LocalVariable1Byte,
        inst_u8_t, number
);
XTAL_DEF_INST_1(28, LocalVariable2Byte,
        inst_u16_t, number
);

XTAL_DEF_INST_1(29, SetLocalVariable1ByteDirect,
        inst_u8_t, number
);
XTAL_DEF_INST_1(30, SetLocalVariable1Byte,
        inst_u8_t, number
);
XTAL_DEF_INST_1(31, SetLocalVariable2Byte,
        inst_u16_t, number
);

XTAL_DEF_INST_2(32, InstanceVariable,
        inst_u8_t, number,
        inst_u16_t, core_number
);
XTAL_DEF_INST_2(33, SetInstanceVariable,
        inst_u8_t, number,
        inst_u16_t, core_number
);

XTAL_DEF_INST_0(34, CleanupCall);

XTAL_DEF_INST_0(35, Return0);
XTAL_DEF_INST_0(36, Return1);
XTAL_DEF_INST_0(37, Return2);
XTAL_DEF_INST_1(38, Return,
        inst_u8_t, results
);

XTAL_DEF_INST_1(39, Yield,
        inst_u16_t, results
);
XTAL_DEF_INST_0(40, Exit);

XTAL_DEF_INST_1(41, Value,
        inst_u16_t, value_number
);
XTAL_DEF_INST_1(42, SetValue,
        inst_u16_t, value_number
);

XTAL_DEF_INST_0(43, CheckUnsupported);

// A=args flag, T=tail flag

XTAL_DEF_INST_4(44, Send,
        inst_u8_t, ordered_arg_count,
        inst_u8_t, named_arg_count,
        inst_u8_t, need_result_count,
        inst_u16_t, symbol_number
);

XTAL_DEF_INST_4(45, SendIfDefined,
        inst_u8_t, ordered_arg_count,
        inst_u8_t, named_arg_count,
        inst_u8_t, need_result_count,
        inst_u16_t, symbol_number
);

XTAL_DEF_INST_3(46, Call,
        inst_u8_t, ordered_arg_count,
        inst_u8_t, named_arg_count,
        inst_u8_t, need_result_count
);

XTAL_DEF_INST_3(47, CallCallee,
        inst_u8_t, ordered_arg_count,
        inst_u8_t, named_arg_count,
        inst_u8_t, need_result_count
);

XTAL_DEF_INST_4(48, Send_A,
        inst_u8_t, ordered_arg_count,
        inst_u8_t, named_arg_count,
        inst_u8_t, need_result_count,
        inst_u16_t, symbol_number
);

XTAL_DEF_INST_4(49, SendIfDefined_A,
        inst_u8_t, ordered_arg_count,
        inst_u8_t, named_arg_count,
        inst_u8_t, need_result_count,
        inst_u16_t, symbol_number
);

XTAL_DEF_INST_3(50, Call_A,
        inst_u8_t, ordered_arg_count,
        inst_u8_t, named_arg_count,
        inst_u8_t, need_result_count
);

XTAL_DEF_INST_3(51, CallCallee_A,
        inst_u8_t, ordered_arg_count,
        inst_u8_t, named_arg_count,
        inst_u8_t, need_result_count
);

XTAL_DEF_INST_4(52, Send_T,
        inst_u8_t, ordered_arg_count,
        inst_u8_t, named_arg_count,
        inst_u8_t, need_result_count,
        inst_u16_t, symbol_number
);

XTAL_DEF_INST_4(53, SendIfDefined_T,
        inst_u8_t, ordered_arg_count,
        inst_u8_t, named_arg_count,
        inst_u8_t, need_result_count,
        inst_u16_t, symbol_number
);

XTAL_DEF_INST_3(54, Call_T,
        inst_u8_t, ordered_arg_count,
        inst_u8_t, named_arg_count,
        inst_u8_t, need_result_count
);

XTAL_DEF_INST_3(55, CallCallee_T,
        inst_u8_t, ordered_arg_count,
        inst_u8_t, named_arg_count,
        inst_u8_t, need_result_count
);

XTAL_DEF_INST_4(56, Send_AT,
        inst_u8_t, ordered_arg_count,
        inst_u8_t, named_arg_count,
        inst_u8_t, need_result_count,
        inst_u16_t, symbol_number
);

XTAL_DEF_INST_4(57, SendIfDefined_AT,
        inst_u8_t, ordered_arg_count,
        inst_u8_t, named_arg_count,
        inst_u8_t, need_result_count,
        inst_u16_t, symbol_number
);

XTAL_DEF_INST_3(58, Call_AT,
        inst_u8_t, ordered_arg_count,
        inst_u8_t, named_arg_count,
        inst_u8_t, need_result_count
);

XTAL_DEF_INST_3(59, CallCallee_AT,
        inst_u8_t, ordered_arg_count,
        inst_u8_t, named_arg_count,
        inst_u8_t, need_result_count
);

XTAL_DEF_INST_1(60, BlockBegin,
        inst_u16_t,core_number
);
XTAL_DEF_INST_0(61, BlockEnd);
XTAL_DEF_INST_1(62, BlockBeginDirect,
        inst_u16_t,core_number
);
XTAL_DEF_INST_0(63, BlockEndDirect);

XTAL_DEF_INST_1(64, TryBegin,
        inst_u16_t, core_number
);
XTAL_DEF_INST_0(65, TryEnd);

XTAL_DEF_INST_1(66, PushGoto,
        inst_address_t, address
);
XTAL_DEF_INST_0(67, PopGoto);

XTAL_DEF_INST_1(68, IfEq,
        inst_address_t, address
);
XTAL_DEF_INST_1(69, IfNe,
        inst_address_t, address
);
XTAL_DEF_INST_1(70, IfLt,
        inst_address_t, address
);
XTAL_DEF_INST_1(71, IfLe,
        inst_address_t, address
);
XTAL_DEF_INST_1(72, IfGt,
        inst_address_t, address
);
XTAL_DEF_INST_1(73, IfGe,
        inst_address_t, address
);
XTAL_DEF_INST_1(74, IfRawEq,
        inst_address_t, address
);
XTAL_DEF_INST_1(75, IfRawNe,
        inst_address_t, address
);
XTAL_DEF_INST_1(76, IfIs,
        inst_address_t, address
);
XTAL_DEF_INST_1(77, IfNis,
        inst_address_t, address
);

XTAL_DEF_INST_2(78, IfArgIsNull,
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

XTAL_DEF_INST_1(121, GlobalVariable,
        inst_u16_t, symbol_number
);
XTAL_DEF_INST_1(122, SetGlobalVariable,
        inst_u16_t, symbol_number
);
XTAL_DEF_INST_1(123, DefineGlobalVariable,
        inst_u16_t, symbol_number
);

XTAL_DEF_INST_1(124, Member,
        inst_u16_t, symbol_number
);
XTAL_DEF_INST_1(125, MemberIfDefined,
        inst_u16_t, symbol_number
);
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

XTAL_DEF_INST_2(133, ClassBegin,
        inst_u16_t, core_number,
		inst_u8_t, mixins
);
XTAL_DEF_INST_0(134, ClassEnd);

XTAL_DEF_INST_0(135, MakeArray);
XTAL_DEF_INST_0(136, ArrayAppend);

XTAL_DEF_INST_0(137, MakeMap);
XTAL_DEF_INST_0(138, MapInsert);

XTAL_DEF_INST_3(139, MakeFun,
        inst_u8_t, type,
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

}
