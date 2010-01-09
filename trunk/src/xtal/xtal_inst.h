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

StringPtr make_inst_string(const StringLiteral& InstName);

StringPtr make_inst_string(const StringLiteral& InstName, 
						const StringLiteral& MemberName1, int_t MemberValue1);

StringPtr make_inst_string(const StringLiteral& InstName, 
						const StringLiteral& MemberName1, int_t MemberValue1,
						const StringLiteral& MemberName2, int_t MemberValue2);

StringPtr make_inst_string(const StringLiteral& InstName, 
						const StringLiteral& MemberName1, int_t MemberValue1,
						const StringLiteral& MemberName2, int_t MemberValue2,
						const StringLiteral& MemberName3, int_t MemberValue3);
						
StringPtr make_inst_string(const StringLiteral& InstName, 
						const StringLiteral& MemberName1, int_t MemberValue1,
						const StringLiteral& MemberName2, int_t MemberValue2,
						const StringLiteral& MemberName3, int_t MemberValue3,
						const StringLiteral& MemberName4, int_t MemberValue4);

StringPtr make_inst_string(const StringLiteral& InstName, 
						const StringLiteral& MemberName1, int_t MemberValue1,
						const StringLiteral& MemberName2, int_t MemberValue2,
						const StringLiteral& MemberName3, int_t MemberValue3,
						const StringLiteral& MemberName4, int_t MemberValue4,
						const StringLiteral& MemberName5, int_t MemberValue5);

StringPtr inspect_range(const CodePtr& code, const inst_t* start, const inst_t* end);

int_t inst_size(uint_t no);

struct Inst{
	inst_t op;

	Inst(inst_t v = 0)
		:op(v){}

	operator const inst_t*() const{
		return &this->op;
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
	struct InstName : public Inst{\
		enum{ \
			NUMBER = N,\
			SIZE = sizeof(Inst),\
			ISIZE = SIZE/sizeof(inst_t)\
		};\
		InstName(){\
			op = N;\
		}\
		StringPtr inspect(const CodePtr&){\
			return make_inst_string(\
					XTAL_STRING(#InstName));\
		}\
	}

#define XTAL_DEF_INST_1(N, InstName, MemberType1, MemberName1) \
	struct InstName : public Inst{\
		enum{ \
			NUMBER = N,\
			SIZE = sizeof(Inst) + sizeof(MemberType1),\
			ISIZE = SIZE/sizeof(inst_t),\
			OFFSET_##MemberName1 = sizeof(inst_t)\
		};\
		MemberType1 MemberName1;\
		InstName(){\
			op = N;\
		}\
		InstName(int m1){\
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
	struct InstName : public Inst{\
		enum{ \
			NUMBER = N,\
			SIZE = sizeof(Inst) + sizeof(MemberType1) + sizeof(MemberType2),\
			ISIZE = SIZE/sizeof(inst_t),\
			OFFSET_##MemberName1 = sizeof(inst_t),\
			OFFSET_##MemberName2 = OFFSET_##MemberName1 + sizeof(MemberType1)\
		};\
		MemberType1 MemberName1;\
		MemberType2 MemberName2;\
		InstName(){\
			op = N;\
		}\
		InstName(int m1, int m2){\
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
	struct InstName : public Inst{\
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
		InstName(){\
			op = N;\
		}\
		InstName(int m1, int m2, int m3){\
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
	struct InstName : public Inst{\
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
		InstName(){\
			op = N;\
		}\
		InstName(int m1, int m2, int m3, int m4){\
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
	struct InstName : public Inst{\
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
		InstName(){\
			op = N;\
		}\
		InstName(int m1, int m2, int m3, int m4, int m5){\
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

#define XTAL_DEF_INST_6(N, InstName, MemberType1, MemberName1, MemberType2, MemberName2, MemberType3, MemberName3, MemberType4, MemberName4, MemberType5, MemberName5, MemberType6, MemberName6) \
	struct InstName : public Inst{\
		enum{ \
			NUMBER = N,\
			SIZE = sizeof(Inst) + sizeof(MemberType1) + sizeof(MemberType2) + sizeof(MemberType3) + sizeof(MemberType4) + sizeof(MemberType5) + sizeof(MemberType6),\
			ISIZE = SIZE/sizeof(inst_t),\
			OFFSET_##MemberName1 = sizeof(inst_t),\
			OFFSET_##MemberName2 = OFFSET_##MemberName1 + sizeof(MemberType1),\
			OFFSET_##MemberName3 = OFFSET_##MemberName2 + sizeof(MemberType2),\
			OFFSET_##MemberName4 = OFFSET_##MemberName3 + sizeof(MemberType3),\
			OFFSET_##MemberName5 = OFFSET_##MemberName4 + sizeof(MemberType4),\
			OFFSET_##MemberName6 = OFFSET_##MemberName5 + sizeof(MemberType5)\
		};\
		MemberType1 MemberName1;\
		MemberType2 MemberName2;\
		MemberType3 MemberName3;\
		MemberType4 MemberName4;\
		MemberType5 MemberName5;\
		MemberType6 MemberName6;\
		InstName(){\
			op = N;\
		}\
		InstName(int m1, int m2, int m3, int m4, int m5, int m6){\
			op = N;\
			checked_assign(MemberName1, m1);\
			checked_assign(MemberName2, m2);\
			checked_assign(MemberName3, m3);\
			checked_assign(MemberName4, m4);\
			checked_assign(MemberName5, m5);\
			checked_assign(MemberName6, m6);\
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

#define XTAL_DEF_INST_7(N, InstName, MemberType1, MemberName1, MemberType2, MemberName2, MemberType3, MemberName3, MemberType4, MemberName4, MemberType5, MemberName5, MemberType6, MemberName6, MemberType7, MemberName7) \
	struct InstName : public Inst{\
		enum{ \
			NUMBER = N,\
			SIZE = sizeof(Inst) + sizeof(MemberType1) + sizeof(MemberType2) + sizeof(MemberType3) + sizeof(MemberType4) + sizeof(MemberType5) + sizeof(MemberType6) + sizeof(MemberType7),\
			ISIZE = SIZE/sizeof(inst_t),\
			OFFSET_##MemberName1 = sizeof(inst_t),\
			OFFSET_##MemberName2 = OFFSET_##MemberName1 + sizeof(MemberType1),\
			OFFSET_##MemberName3 = OFFSET_##MemberName2 + sizeof(MemberType2),\
			OFFSET_##MemberName4 = OFFSET_##MemberName3 + sizeof(MemberType3),\
			OFFSET_##MemberName5 = OFFSET_##MemberName4 + sizeof(MemberType4),\
			OFFSET_##MemberName6 = OFFSET_##MemberName5 + sizeof(MemberType5),\
			OFFSET_##MemberName7 = OFFSET_##MemberName6 + sizeof(MemberType6)\
		};\
		MemberType1 MemberName1;\
		MemberType2 MemberName2;\
		MemberType3 MemberName3;\
		MemberType4 MemberName4;\
		MemberType5 MemberName5;\
		MemberType6 MemberName6;\
		MemberType7 MemberName7;\
		InstName(){\
			op = N;\
		}\
		InstName(int m1, int m2, int m3, int m4, int m5, int m6, int m7){\
			op = N;\
			checked_assign(MemberName1, m1);\
			checked_assign(MemberName2, m2);\
			checked_assign(MemberName3, m3);\
			checked_assign(MemberName4, m4);\
			checked_assign(MemberName5, m5);\
			checked_assign(MemberName6, m6);\
			checked_assign(MemberName7, m7);\
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

#define XTAL_DEF_INST_8(N, InstName, MemberType1, MemberName1, MemberType2, MemberName2, MemberType3, MemberName3, MemberType4, MemberName4, MemberType5, MemberName5, MemberType6, MemberName6, MemberType7, MemberName7, MemberType8, MemberName8) \
	struct InstName : public Inst{\
		enum{ \
			NUMBER = N,\
			SIZE = sizeof(Inst) + sizeof(MemberType1) + sizeof(MemberType2) + sizeof(MemberType3) + sizeof(MemberType4) + sizeof(MemberType5) + sizeof(MemberType6) + sizeof(MemberType7) + sizeof(MemberType8),\
			ISIZE = SIZE/sizeof(inst_t),\
			OFFSET_##MemberName1 = sizeof(inst_t),\
			OFFSET_##MemberName2 = OFFSET_##MemberName1 + sizeof(MemberType1),\
			OFFSET_##MemberName3 = OFFSET_##MemberName2 + sizeof(MemberType2),\
			OFFSET_##MemberName4 = OFFSET_##MemberName3 + sizeof(MemberType3),\
			OFFSET_##MemberName5 = OFFSET_##MemberName4 + sizeof(MemberType4),\
			OFFSET_##MemberName6 = OFFSET_##MemberName5 + sizeof(MemberType5),\
			OFFSET_##MemberName7 = OFFSET_##MemberName6 + sizeof(MemberType6),\
			OFFSET_##MemberName8 = OFFSET_##MemberName7 + sizeof(MemberType7)\
		};\
		MemberType1 MemberName1;\
		MemberType2 MemberName2;\
		MemberType3 MemberName3;\
		MemberType4 MemberName4;\
		MemberType5 MemberName5;\
		MemberType6 MemberName6;\
		MemberType7 MemberName7;\
		MemberType8 MemberName8;\
		InstName(){\
			op = N;\
		}\
		InstName(int m1, int m2, int m3, int m4, int m5, int m6, int m7, int m8){\
			op = N;\
			checked_assign(MemberName1, m1);\
			checked_assign(MemberName2, m2);\
			checked_assign(MemberName3, m3);\
			checked_assign(MemberName4, m4);\
			checked_assign(MemberName5, m5);\
			checked_assign(MemberName6, m6);\
			checked_assign(MemberName7, m7);\
			checked_assign(MemberName8, m8);\
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

#define XTAL_DEF_INST_9(N, InstName, MemberType1, MemberName1, MemberType2, MemberName2, MemberType3, MemberName3, MemberType4, MemberName4, MemberType5, MemberName5, MemberType6, MemberName6, MemberType7, MemberName7, MemberType8, MemberName8, MemberType9, MemberName9) \
	struct InstName : public Inst{\
		enum{ \
			NUMBER = N,\
			SIZE = sizeof(Inst) + sizeof(MemberType1) + sizeof(MemberType2) + sizeof(MemberType3) + sizeof(MemberType4) + sizeof(MemberType5) + sizeof(MemberType6) + sizeof(MemberType7) + sizeof(MemberType8) + sizeof(MemberType9),\
			ISIZE = SIZE/sizeof(inst_t),\
			OFFSET_##MemberName1 = sizeof(inst_t),\
			OFFSET_##MemberName2 = OFFSET_##MemberName1 + sizeof(MemberType1),\
			OFFSET_##MemberName3 = OFFSET_##MemberName2 + sizeof(MemberType2),\
			OFFSET_##MemberName4 = OFFSET_##MemberName3 + sizeof(MemberType3),\
			OFFSET_##MemberName5 = OFFSET_##MemberName4 + sizeof(MemberType4),\
			OFFSET_##MemberName6 = OFFSET_##MemberName5 + sizeof(MemberType5),\
			OFFSET_##MemberName7 = OFFSET_##MemberName6 + sizeof(MemberType6),\
			OFFSET_##MemberName8 = OFFSET_##MemberName7 + sizeof(MemberType7),\
			OFFSET_##MemberName9 = OFFSET_##MemberName8 + sizeof(MemberType8)\
		};\
		MemberType1 MemberName1;\
		MemberType2 MemberName2;\
		MemberType3 MemberName3;\
		MemberType4 MemberName4;\
		MemberType5 MemberName5;\
		MemberType6 MemberName6;\
		MemberType7 MemberName7;\
		MemberType8 MemberName8;\
		MemberType8 MemberName9;\
		InstName(){\
			op = N;\
		}\
		InstName(int m1, int m2, int m3, int m4, int m5, int m6, int m7, int m8, int m9){\
			op = N;\
			checked_assign(MemberName1, m1);\
			checked_assign(MemberName2, m2);\
			checked_assign(MemberName3, m3);\
			checked_assign(MemberName4, m4);\
			checked_assign(MemberName5, m5);\
			checked_assign(MemberName6, m6);\
			checked_assign(MemberName7, m7);\
			checked_assign(MemberName8, m8);\
			checked_assign(MemberName9, m9);\
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
* \brief 行が変わるごとに挟まれる命令
*
*/
XTAL_DEF_INST_0(0, InstLine);

enum{
	LOAD_NULL,
	LOAD_UNDEFINED,
	LOAD_FALSE,
	LOAD_TRUE,
};

XTAL_DEF_INST_2(1, InstLoadValue,
	inst_i8_t, result,
	inst_u8_t, value);

/**
* \internal
* \brief 値テーブルから値を取り出す
*
*/
XTAL_DEF_INST_2(2, InstLoadConstant,
		inst_i8_t, result,
        inst_u16_t, value_number
);

/**
* \internal
* \brief スタックにint値をプッシュする。
*
*/
XTAL_DEF_INST_2(3, InstLoadInt1Byte,
		inst_i8_t, result,
        inst_i8_t, value
);

/**
* \internal
* \brief スタックにfloat値をプッシュする。
*
*/
XTAL_DEF_INST_2(4, InstLoadFloat1Byte,
		inst_i8_t, result,
        inst_i8_t, value
);

/**
* \internal
* \brief スタックに現在実行中の関数をプッシュする。
*
*/
XTAL_DEF_INST_1(5, InstLoadCallee,
	inst_i8_t, result);

/**
* \internal
* \brief 現在のthisをスタックに積む。
*
*/
XTAL_DEF_INST_1(6, InstLoadThis,
	inst_i8_t, result);

/**
* \internal
* \brief コピーする
*
*/
XTAL_DEF_INST_2(7, InstCopy,
	inst_i8_t, result,
	inst_i8_t, target);

/**
* \internal
* \brief オブジェクトのメンバを取り出す。
*
*/
XTAL_DEF_INST_5(8, InstMember,
		inst_i8_t, result,
		inst_i8_t, target,
		inst_i16_t, primary,
		inst_i8_t, secondary,
		inst_u8_t, flags
);

enum{
	UNA_INC,
	UNA_DEC,
	UNA_POS,
	UNA_NEG,
	UNA_COM
};

XTAL_DEF_INST_4(9, InstUna,
	inst_i8_t, result,
	inst_u8_t, kind,
	inst_i8_t, target,
	inst_i8_t, stack_base);

enum{
	ARITH_ADD,
	ARITH_SUB,
	ARITH_CAT,
	ARITH_MUL,
	ARITH_DIV,
	ARITH_MOD,

	ARITH_ADD_ASSIGN = 8,
	ARITH_SUB_ASSIGN,
	ARITH_CAT_ASSIGN,
	ARITH_MUL_ASSIGN,
	ARITH_DIV_ASSIGN,
	ARITH_MOD_ASSIGN,
};

XTAL_DEF_INST_5(10, InstArith,
	inst_i8_t, result,
	inst_u8_t, kind,
	inst_i8_t, lhs,
	inst_i8_t, rhs,
	inst_i8_t, stack_base
	);

enum{
	BITWISE_AND,
	BITWISE_OR,
	BITWISE_XOR,
	BITWISE_SHL,
	BITWISE_SHR,
	BITWISE_USHR,

	BITWISE_AND_ASSIGN = 8,
	BITWISE_OR_ASSIGN,
	BITWISE_XOR_ASSIGN,
	BITWISE_SHL_ASSIGN,
	BITWISE_SHR_ASSIGN,
	BITWISE_USHR_ASSIGN,
};

XTAL_DEF_INST_5(11, InstBitwise,
	inst_i8_t, result,
	inst_u8_t, kind,
	inst_i8_t, lhs,
	inst_i8_t, rhs,
	inst_i8_t, stack_base
	);


XTAL_DEF_INST_4(12, InstAt,
	inst_i8_t, result,
	inst_i8_t, target,
	inst_i8_t, index,
	inst_i8_t, stack_base
);

XTAL_DEF_INST_4(13, InstSetAt,
	inst_i8_t, target,
	inst_i8_t, index,
	inst_i8_t, value,
	inst_i8_t, stack_base
);

/**
* \internal
* \brief 無条件分岐
*
*/
XTAL_DEF_INST_1(14, InstGoto,
        inst_address_t, address
);

XTAL_DEF_INST_2(15, InstNot,
	inst_i8_t, result,
	inst_i8_t, target);

/**
* \internal
* \brief 条件分岐
*
*/
XTAL_DEF_INST_3(16, InstIf,
		inst_i8_t, target,
        inst_address_t, address_true,
		inst_address_t, address_false
);

enum{
	IF_COMP_EQ,
	IF_COMP_LT,
	IF_COMP_IN
};

XTAL_DEF_INST_4(17, InstIfComp,
		inst_u8_t, kind,
		inst_i8_t, lhs,
		inst_i8_t, rhs,
		inst_i8_t, stack_base
	);

XTAL_DEF_INST_3(18, InstIfRawEq,
		inst_i8_t, lhs,
		inst_i8_t, rhs,
		inst_i8_t, stack_base);

XTAL_DEF_INST_3(19, InstIfIs,
		inst_i8_t, lhs,
		inst_i8_t, rhs,
		inst_i8_t, stack_base);

XTAL_DEF_INST_3(20, InstIfUndefined,
	inst_i8_t, target,
    inst_address_t, address_true,
	inst_address_t, address_false
);

XTAL_DEF_INST_1(21, InstIfDebug,
		inst_address_t, address);


XTAL_DEF_INST_1(22, InstPush,
	inst_i8_t, target);

XTAL_DEF_INST_1(23, InstPop,
	inst_i8_t, result);

/**
* \internal
* \brief 値の数を調整する
*
*/
XTAL_DEF_INST_3(24, InstAdjustValues,
		inst_u8_t, stack_base,
        inst_u8_t, result_count,
        inst_u8_t, need_result_count
);

/**
* \internal
* \brief ローカル変数を取り出す。
*
*/
XTAL_DEF_INST_3(25, InstLocalVariable,
		inst_i8_t, result,
        inst_u16_t, number,
		inst_u8_t, depth
);

/**
* \internal
* \brief ローカル変数に値を設定する。
*
*/
XTAL_DEF_INST_3(26, InstSetLocalVariable,
		inst_i8_t, target,
		inst_u16_t, number,
		inst_u8_t, depth
);

/**
* \internal
* \brief インスタンス変数を取り出す。
*
*/
XTAL_DEF_INST_3(27, InstInstanceVariable,
		inst_i8_t, result,
        inst_u8_t, number,
        inst_u16_t, info_number
);

/**
* \internal
* \brief インスタンス変数に値を設定する。
*
*/
XTAL_DEF_INST_3(28, InstSetInstanceVariable,
		inst_i8_t, value,
        inst_u8_t, number,
        inst_u16_t, info_number
);

/**
* \internal
* \brief ファイルローカル変数を取り出す。
*
*/
XTAL_DEF_INST_2(29, InstFilelocalVariable,
		inst_i8_t, result,
        inst_u16_t, identifier_number
);

enum{
	CALL_FLAG_ARGS = 0,
	CALL_FLAG_THIS = 1,

	CALL_FLAG_ARGS_BIT = 1<<CALL_FLAG_ARGS,
	CALL_FLAG_THIS_BIT = 1<<CALL_FLAG_THIS,
};

enum{
	MEMBER_FLAG_P = 2,
	MEMBER_FLAG_S = 3,
	MEMBER_FLAG_Q = 4,

	MEMBER_FLAG_P_BIT = 1<<MEMBER_FLAG_P,
	MEMBER_FLAG_S_BIT = 1<<MEMBER_FLAG_S,
	MEMBER_FLAG_Q_BIT = 1<<MEMBER_FLAG_Q,
};

XTAL_DEF_INST_8(30, InstCall,
		inst_i8_t, result,
        inst_u8_t, need_result,
		inst_i8_t, target,
		inst_i8_t, self,
		inst_i8_t, stack_base,
        inst_u8_t, ordered,
        inst_u8_t, named,
        inst_u8_t, flags
);

XTAL_DEF_INST_9(31, InstSend,
		inst_i8_t, result,
        inst_u8_t, need_result,
		inst_i8_t, target,
		inst_i16_t, primary,
		inst_i8_t, secondary,
		inst_i8_t, stack_base,
        inst_u8_t, ordered,
        inst_u8_t, named,
        inst_u8_t, flags
);

XTAL_DEF_INST_4(32, InstProperty,
		inst_i8_t, result,
		inst_i8_t, target,
        inst_i16_t, primary,
		inst_i8_t, stack_base
);

XTAL_DEF_INST_3(33, InstSetProperty,
		inst_i8_t, target,
        inst_i16_t, primary,
		inst_i8_t, stack_base
);

XTAL_DEF_INST_1(34, InstScopeBegin,
        inst_u16_t, info_number
);

XTAL_DEF_INST_0(35, InstScopeEnd);

/**
* \internal
* \brief 関数から抜ける
*
*/
XTAL_DEF_INST_1(36, InstReturn,
        inst_u8_t, result_count
);

/**
* \internal
* \brief fiberの実行を一時中断する
*
*/
XTAL_DEF_INST_3(37, InstYield,
		inst_i8_t, result,
		inst_u8_t, need_result_count,
        inst_u8_t, target_count
);

/**
* \internal
* \brief 仮想マシンのループから脱出する。
*
*/
XTAL_DEF_INST_0(38, InstExit);

XTAL_DEF_INST_5(39, InstRange,
	inst_i8_t, result,
	inst_u8_t, kind,
	inst_i8_t, lhs,
	inst_i8_t, rhs,
	inst_i8_t, stack_base
);

XTAL_DEF_INST_3(40, InstOnce,
		inst_i8_t, result,
        inst_address_t, address,
        inst_u16_t, value_number
);

XTAL_DEF_INST_2(41, InstSetOnce,
		inst_i8_t, target,
        inst_u16_t, value_number
);

XTAL_DEF_INST_1(42, InstMakeArray,
		inst_i8_t, result
);

XTAL_DEF_INST_2(43, InstArrayAppend,
		inst_i8_t, target,
		inst_i8_t, value
);

XTAL_DEF_INST_1(44, InstMakeMap,
		inst_i8_t, result);

XTAL_DEF_INST_3(45, InstMapInsert,
		inst_i8_t, target,
		inst_i8_t, key,
		inst_i8_t, value
);

XTAL_DEF_INST_2(46, InstMapSetDefault,
		inst_i8_t, target,
		inst_i8_t, value);

XTAL_DEF_INST_2(47, InstClassBegin,
        inst_u16_t, info_number,
		inst_i8_t, mixin_base
);

XTAL_DEF_INST_1(48, InstClassEnd,
		inst_i8_t, result);

XTAL_DEF_INST_5(49, InstDefineClassMember,
        inst_u16_t, number,
		inst_u16_t, primary,
		inst_i8_t, secondary,
		inst_u8_t, accessibility,
		inst_i8_t, value
);

/**
* \internal
* \brief オブジェクトのメンバを定義する。
*
*/
XTAL_DEF_INST_5(50, InstDefineMember,
		inst_i8_t, target,
		inst_i16_t, primary,
		inst_i8_t, secondary,
		inst_u8_t, flags,
		inst_i8_t, value
);

XTAL_DEF_INST_3(51, InstMakeFun,
		inst_i8_t, result,
        inst_u16_t, info_number,
		inst_address_t, address
);

XTAL_DEF_INST_4(52, InstMakeInstanceVariableAccessor,
		inst_i8_t, result,        
		inst_u8_t, type,
		inst_u8_t, number,
        inst_u16_t, info_number
);

XTAL_DEF_INST_1(53, InstTryBegin,
        inst_u16_t, info_number
);

XTAL_DEF_INST_0(54, InstTryEnd);

XTAL_DEF_INST_1(55, InstPushGoto,
        inst_address_t, address
);

XTAL_DEF_INST_0(56, InstPopGoto);

XTAL_DEF_INST_0(57, InstThrow);

XTAL_DEF_INST_1(58, InstAssert,
		inst_i8_t, message);

XTAL_DEF_INST_0(59, InstBreakPoint);

XTAL_DEF_INST_0(60, InstMAX);

}

#endif // XTAL_INST_H_INCLUDE_GUARD
