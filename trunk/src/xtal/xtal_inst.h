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

int_t inst_size(uint_t no);

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

#define XTAL_DEF_INST_6(N, InstName, MemberType1, MemberName1, MemberType2, MemberName2, MemberType3, MemberName3, MemberType4, MemberName4, MemberType5, MemberName5, MemberType6, MemberName6) \
	struct Inst##InstName : public Inst{\
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
		Inst##InstName(){\
			op = N;\
		}\
		Inst##InstName(int m1, int m2, int m3, int m4, int m5, int m6){\
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
	struct Inst##InstName : public Inst{\
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
		Inst##InstName(){\
			op = N;\
		}\
		Inst##InstName(int m1, int m2, int m3, int m4, int m5, int m6, int m7){\
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
	struct Inst##InstName : public Inst{\
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
		Inst##InstName(){\
			op = N;\
		}\
		Inst##InstName(int m1, int m2, int m3, int m4, int m5, int m6, int m7, int m8){\
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
	struct Inst##InstName : public Inst{\
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
		Inst##InstName(){\
			op = N;\
		}\
		Inst##InstName(int m1, int m2, int m3, int m4, int m5, int m6, int m7, int m8, int m9){\
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
XTAL_DEF_INST_1(1, LoadNull,
	inst_i8_t, result);

/**
* \internal
* \brief スタックにundefined値をプッシュする。
*
* stack [] -> [value]
*/
XTAL_DEF_INST_1(2, LoadUndefined,
	inst_i8_t, result);

/**
* \internal
* \brief スタックにtrue値をプッシュする。
*
* stack [] -> [value]
*/
XTAL_DEF_INST_1(3, LoadTrue,
	inst_i8_t, result);

/**
* \internal
* \brief スタックにfalse値をプッシュする。
*
* stack [] -> [value]
*/
XTAL_DEF_INST_1(4, LoadFalse,
	inst_i8_t, result);

/**
* \internal
* \brief スタックにtrue値をプッシュする。
*
* stack [] -> [value]
*/
XTAL_DEF_INST_1(5, LoadTrueAndSkip,
		inst_i8_t, result);

/**
* \internal
* \brief スタックにint値をプッシュする。
*
* stack [] -> [value]
*/
XTAL_DEF_INST_2(6, LoadInt1Byte,
		inst_i8_t, result,
        inst_i8_t, value
);

/**
* \internal
* \brief スタックにfloat値をプッシュする。
*
* stack [] -> [value]
*/
XTAL_DEF_INST_2(7, LoadFloat1Byte,
		inst_i8_t, result,
        inst_i8_t, value
);

/**
* \internal
* \brief スタックに現在実行中の関数をプッシュする。
*
* stack [] -> [value]
*/
XTAL_DEF_INST_1(8, LoadCallee,
	inst_i8_t, result);

/**
* \internal
* \brief 現在のthisをスタックに積む。
*
* stack [] -> [value]
*/
XTAL_DEF_INST_1(9, LoadThis,
	inst_i8_t, result);

/**
* \internal
* \brief 現在の文脈をスタックに積む。
*
* stack [] -> [value]
*/
XTAL_DEF_INST_1(10, LoadCurrentContext,
	inst_i8_t, result);

/**
* \internal
* \brief スタックトップをコピーする
*
*/
XTAL_DEF_INST_2(11, Copy,
	inst_i8_t, result,
	inst_i8_t, target);

XTAL_DEF_INST_1(12, Push,
	inst_i8_t, target);

XTAL_DEF_INST_1(13, Pop,
	inst_i8_t, result);

/**
* \internal
* \brief 値の数を調整する
*
* stack [] -> []
*/
XTAL_DEF_INST_3(14, AdjustValues,
		inst_u8_t, stack_base,
        inst_u8_t, result_count,
        inst_u8_t, need_result_count
);

/**
* \internal
* \brief ローカル変数を取り出す。
*
* stack [] -> [value]
*/
XTAL_DEF_INST_3(15, LocalVariable2Byte,
		inst_i8_t, result,
        inst_u16_t, number,
		inst_u8_t, depth
);

/**
* \internal
* \brief ローカル変数に値を設定する。
*
* stack [value] -> []
*/
XTAL_DEF_INST_3(16, SetLocalVariable2Byte,
		inst_i8_t, target,
		inst_u16_t, number,
		inst_u8_t, depth
);

/**
* \internal
* \brief インスタンス変数を取り出す。
*
* stack [] -> [value]
*/
XTAL_DEF_INST_3(17, InstanceVariable,
		inst_i8_t, result,
        inst_u8_t, number,
        inst_u16_t, info_number
);

/**
* \internal
* \brief インスタンス変数に値を設定する。
*
* stack [value] -> []
*/
XTAL_DEF_INST_3(18, SetInstanceVariable,
		inst_i8_t, value,
        inst_u8_t, number,
        inst_u16_t, info_number
);

/**
* \internal
* \brief ファイルローカル変数を取り出す。
*
* stack [] -> [value]
*/
XTAL_DEF_INST_2(19, FilelocalVariable,
		inst_i8_t, result,
        inst_u16_t, identifier_number
);

/**
* \internal
* \brief 関数から抜ける
*
* stack [value1, value2, ...valueN] -> []
*/
XTAL_DEF_INST_1(20, Return,
        inst_u8_t, result_count
);

/**
* \internal
* \brief fiberの実行を一時中断する
*
* stack [value1, value2, ...valueN] -> []
*/
XTAL_DEF_INST_3(21, Yield,
		inst_i8_t, result,
		inst_u8_t, need_result_count,
        inst_u8_t, target_count
);

/**
* \internal
* \brief 仮想マシンのループから脱出する。
*
* stack [] -> []
*/
XTAL_DEF_INST_0(22, Exit);

/**
* \internal
* \brief 値テーブルから値を取り出す
*
* stack [] -> [value]
*/
XTAL_DEF_INST_2(23, Value,
		inst_i8_t, result,
        inst_u16_t, value_number
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

XTAL_DEF_INST_8(24, Call,
		inst_i8_t, result,
        inst_u8_t, need_result,
		inst_i8_t, target,
		inst_i8_t, self,
		inst_i8_t, stack_base,
        inst_u8_t, ordered,
        inst_u8_t, named,
        inst_u8_t, flags
);

XTAL_DEF_INST_9(25, Send,
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

XTAL_DEF_INST_4(26, Property,
		inst_i8_t, result,
		inst_i8_t, target,
        inst_i16_t, primary,
		inst_i8_t, stack_base
);

XTAL_DEF_INST_3(27, SetProperty,
		inst_i8_t, target,
        inst_i16_t, primary,
		inst_i8_t, stack_base
);

/**
* \internal
* \brief オブジェクトのメンバを取り出す。
*
* stack [object] -> [result]
*/
XTAL_DEF_INST_5(28, Member,
		inst_i8_t, result,
		inst_i8_t, target,
		inst_i16_t, primary,
		inst_i8_t, secondary,
		inst_u8_t, flags
);

XTAL_DEF_INST_1(29, ScopeBegin,
        inst_u16_t, info_number
);

XTAL_DEF_INST_0(30, ScopeEnd);

XTAL_DEF_INST_3(31, Pos,
	inst_i8_t, result,
	inst_i8_t, target,
	inst_i8_t, stack_base);

XTAL_DEF_INST_3(32, Neg,
	inst_i8_t, result,
	inst_i8_t, target,
		inst_i8_t, stack_base);

XTAL_DEF_INST_3(33, Com,
	inst_i8_t, result,
	inst_i8_t, target,
		inst_i8_t, stack_base);

XTAL_DEF_INST_3(34, Not,
	inst_i8_t, result,
	inst_i8_t, target,
		inst_i8_t, stack_base);

XTAL_DEF_INST_3(35, Inc,
	inst_i8_t, result,
	inst_i8_t, target,
	inst_i8_t, stack_base);

XTAL_DEF_INST_3(36, Dec,
	inst_i8_t, result,
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

XTAL_DEF_INST_5(37, Arith,
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

XTAL_DEF_INST_5(38, Bitwise,
	inst_i8_t, result,
	inst_u8_t, kind,
	inst_i8_t, lhs,
	inst_i8_t, rhs,
	inst_i8_t, stack_base
	);


XTAL_DEF_INST_4(39, At,
	inst_i8_t, result,
	inst_i8_t, target,
	inst_i8_t, index,
	inst_i8_t, stack_base
);

XTAL_DEF_INST_4(40, SetAt,
	inst_i8_t, target,
	inst_i8_t, index,
	inst_i8_t, value,
	inst_i8_t, stack_base
);

/**
* \internal
* \brief 無条件分岐
*
* stack [] -> []
*/
XTAL_DEF_INST_1(41, Goto,
        inst_address_t, address
);

/**
* \internal
* \brief 条件分岐
*
* stack [value] -> []
*/
XTAL_DEF_INST_3(42, If,
		inst_i8_t, target,
        inst_address_t, address_true,
		inst_address_t, address_false
);

XTAL_DEF_INST_3(43, IfEq,
		inst_i8_t, lhs,
		inst_i8_t, rhs,
		inst_i8_t, stack_base
	);

XTAL_DEF_INST_3(44, IfLt,
		inst_i8_t, lhs,
		inst_i8_t, rhs,
		inst_i8_t, stack_base);

XTAL_DEF_INST_3(45, IfRawEq,
		inst_i8_t, lhs,
		inst_i8_t, rhs,
		inst_i8_t, stack_base);

XTAL_DEF_INST_3(46, IfIn,
		inst_i8_t, lhs,
		inst_i8_t, rhs,
		inst_i8_t, stack_base);

XTAL_DEF_INST_3(47, IfIs,
		inst_i8_t, lhs,
		inst_i8_t, rhs,
		inst_i8_t, stack_base);

XTAL_DEF_INST_3(48, IfUndefined,
	inst_i8_t, target,
    inst_address_t, address_true,
	inst_address_t, address_false
);

XTAL_DEF_INST_1(49, IfDebug,
		inst_address_t, address);


XTAL_DEF_INST_5(50, Range,
	inst_i8_t, result,
	inst_u8_t, kind,
	inst_i8_t, lhs,
	inst_i8_t, rhs,
	inst_i8_t, stack_base
);

XTAL_DEF_INST_3(51, Once,
		inst_i8_t, result,
        inst_address_t, address,
        inst_u16_t, value_number
);

XTAL_DEF_INST_2(52, SetOnce,
		inst_i8_t, target,
        inst_u16_t, value_number
);

XTAL_DEF_INST_1(53, MakeArray,
		inst_i8_t, result
);

XTAL_DEF_INST_2(54, ArrayAppend,
		inst_i8_t, target,
		inst_i8_t, value
);

XTAL_DEF_INST_1(55, MakeMap,
		inst_i8_t, result);

XTAL_DEF_INST_3(56, MapInsert,
		inst_i8_t, target,
		inst_i8_t, key,
		inst_i8_t, value
);

XTAL_DEF_INST_2(57, MapSetDefault,
		inst_i8_t, target,
		inst_i8_t, value);

XTAL_DEF_INST_2(58, ClassBegin,
        inst_u16_t, info_number,
		inst_i8_t, mixin_base
);

XTAL_DEF_INST_1(59, ClassEnd,
		inst_i8_t, result);

XTAL_DEF_INST_5(60, DefineClassMember,
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
* stack [object, value] -> []
*/
XTAL_DEF_INST_5(61, DefineMember,
		inst_i8_t, target,
		inst_i16_t, primary,
		inst_i8_t, secondary,
		inst_u8_t, flags,
		inst_i8_t, value
);

XTAL_DEF_INST_3(62, MakeFun,
		inst_i8_t, result,
        inst_u16_t, info_number,
		inst_address_t, address
);

XTAL_DEF_INST_4(63, MakeInstanceVariableAccessor,
		inst_i8_t, result,        
		inst_u8_t, type,
		inst_u8_t, number,
        inst_u16_t, info_number
);

XTAL_DEF_INST_1(64, TryBegin,
        inst_u16_t, info_number
);

XTAL_DEF_INST_0(65, TryEnd);

XTAL_DEF_INST_1(66, PushGoto,
        inst_address_t, address
);

XTAL_DEF_INST_0(67, PopGoto);

XTAL_DEF_INST_0(68, Throw);

XTAL_DEF_INST_1(69, Assert,
		inst_i8_t, message);

XTAL_DEF_INST_0(70, BreakPoint);

XTAL_DEF_INST_0(71, MAX);

}

#endif // XTAL_INST_H_INCLUDE_GUARD
