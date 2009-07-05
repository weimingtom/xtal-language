#include "xtal.h"
#include "xtal_macro.h"

namespace xtal{

int_t inst_inspect(inst_address_t& value, Inst* inst, const CodePtr& code){
	int_t pc = (inst_t*)inst - code->data();
	return pc + value;
}

StringPtr make_inst_string(const char_t* InstName){
	StringPtr temp = format(XTAL_STRING("%s:"))->call(InstName)->to_s();
	return format(temp)->call()->to_s();
}

StringPtr make_inst_string(const char_t* InstName, 
						const char_t* MemberName1, int_t MemberValue1){
	StringPtr temp = format(XTAL_STRING("%s: %s=%%s"))->call(InstName, MemberName1)->to_s();
	return format(temp)->call(MemberValue1)->to_s();
}

StringPtr make_inst_string(const char_t* InstName, 
						const char_t* MemberName1, int_t MemberValue1,
						const char_t* MemberName2, int_t MemberValue2){
	StringPtr temp = format(XTAL_STRING("%s: %s=%%s, %s=%%s"))->call(InstName, MemberName1, MemberName2)->to_s();
	return format(temp)->call(MemberValue1, MemberValue2)->to_s();
}

StringPtr make_inst_string(const char_t* InstName, 
						const char_t* MemberName1, int_t MemberValue1,
						const char_t* MemberName2, int_t MemberValue2,
						const char_t* MemberName3, int_t MemberValue3){
	StringPtr temp = format(XTAL_STRING("%s: %s=%%s, %s=%%s, %s=%%s"))->call(InstName, MemberName1, MemberName2, MemberName3)->to_s();
	return format(temp)->call(MemberValue1, MemberValue2, MemberValue3)->to_s();
}

StringPtr make_inst_string(const char_t* InstName, 
						const char_t* MemberName1, int_t MemberValue1,
						const char_t* MemberName2, int_t MemberValue2,
						const char_t* MemberName3, int_t MemberValue3,
						const char_t* MemberName4, int_t MemberValue4){
	StringPtr temp = format(XTAL_STRING("%s: %s=%%s, %s=%%s, %s=%%s, %s=%%s"))->call(InstName, MemberName1, MemberName2, MemberName3, MemberName4)->to_s();
	return format(temp)->call(MemberValue1, MemberValue2, MemberValue3, MemberValue4)->to_s();
}

StringPtr make_inst_string(const char_t* InstName, 
						const char_t* MemberName1, int_t MemberValue1,
						const char_t* MemberName2, int_t MemberValue2,
						const char_t* MemberName3, int_t MemberValue3,
						const char_t* MemberName4, int_t MemberValue4,
						const char_t* MemberName5, int_t MemberValue5){					
	StringPtr temp = format(XTAL_STRING("%s: %s=%%s, %s=%%s, %s=%%s, %s=%%s, %s=%%s"))->call(InstName, MemberName1, MemberName2, MemberName3, MemberName4, MemberName5)->to_s();
	return format(temp)->call(MemberValue1, MemberValue2, MemberValue3, MemberValue4, MemberValue5)->to_s();
}

StringPtr inspect_range(const CodePtr& code, const inst_t* start, const inst_t* end){

#ifdef XTAL_DEBUG

	int sz = 0;
	const inst_t* pc = start;
	StringPtr temp;
	MemoryStreamPtr ms(xnew<MemoryStream>());

	for(; pc < end;){switch(*pc){
		XTAL_NODEFAULT;

#define XTAL_INST_CASE(x) XTAL_CASE(Inst##x::NUMBER){ temp = ((Inst##x*)pc)->inspect(code); sz = Inst##x::ISIZE; }

//{INST_INSPECT{{
		XTAL_INST_CASE(Nop);
		XTAL_INST_CASE(LoadNull);
		XTAL_INST_CASE(LoadUndefined);
		XTAL_INST_CASE(LoadTrue);
		XTAL_INST_CASE(LoadFalse);
		XTAL_INST_CASE(LoadTrueAndSkip);
		XTAL_INST_CASE(LoadInt1Byte);
		XTAL_INST_CASE(LoadFloat1Byte);
		XTAL_INST_CASE(LoadCallee);
		XTAL_INST_CASE(LoadThis);
		XTAL_INST_CASE(LoadCurrentContext);
		XTAL_INST_CASE(Copy);
		XTAL_INST_CASE(Push);
		XTAL_INST_CASE(Pop);
		XTAL_INST_CASE(AdjustValues);
		XTAL_INST_CASE(LocalVariable2Byte);
		XTAL_INST_CASE(SetLocalVariable2Byte);
		XTAL_INST_CASE(InstanceVariable);
		XTAL_INST_CASE(SetInstanceVariable);
		XTAL_INST_CASE(FilelocalVariable);
		XTAL_INST_CASE(Return);
		XTAL_INST_CASE(Yield);
		XTAL_INST_CASE(Exit);
		XTAL_INST_CASE(Value);
		XTAL_INST_CASE(Call);
		XTAL_INST_CASE(Send);
		XTAL_INST_CASE(Property);
		XTAL_INST_CASE(SetProperty);
		XTAL_INST_CASE(Member);
		XTAL_INST_CASE(ScopeBegin);
		XTAL_INST_CASE(ScopeEnd);
		XTAL_INST_CASE(Pos);
		XTAL_INST_CASE(Neg);
		XTAL_INST_CASE(Com);
		XTAL_INST_CASE(Not);
		XTAL_INST_CASE(Inc);
		XTAL_INST_CASE(Dec);
		XTAL_INST_CASE(Arith);
		XTAL_INST_CASE(Bitwise);
		XTAL_INST_CASE(At);
		XTAL_INST_CASE(SetAt);
		XTAL_INST_CASE(Goto);
		XTAL_INST_CASE(If);
		XTAL_INST_CASE(IfEq);
		XTAL_INST_CASE(IfLt);
		XTAL_INST_CASE(IfRawEq);
		XTAL_INST_CASE(IfIn);
		XTAL_INST_CASE(IfIs);
		XTAL_INST_CASE(IfUndefined);
		XTAL_INST_CASE(IfDebug);
		XTAL_INST_CASE(Range);
		XTAL_INST_CASE(Once);
		XTAL_INST_CASE(SetOnce);
		XTAL_INST_CASE(MakeArray);
		XTAL_INST_CASE(ArrayAppend);
		XTAL_INST_CASE(MakeMap);
		XTAL_INST_CASE(MapInsert);
		XTAL_INST_CASE(MapSetDefault);
		XTAL_INST_CASE(ClassBegin);
		XTAL_INST_CASE(ClassEnd);
		XTAL_INST_CASE(DefineClassMember);
		XTAL_INST_CASE(DefineMember);
		XTAL_INST_CASE(MakeFun);
		XTAL_INST_CASE(MakeInstanceVariableAccessor);
		XTAL_INST_CASE(TryBegin);
		XTAL_INST_CASE(TryEnd);
		XTAL_INST_CASE(PushGoto);
		XTAL_INST_CASE(PopGoto);
		XTAL_INST_CASE(Throw);
		XTAL_INST_CASE(Assert);
		XTAL_INST_CASE(BreakPoint);
		XTAL_INST_CASE(MAX);
//}}INST_INSPECT}
	} ms->put_s(Xf("%04d:%s\n")->call((int_t)(pc-start), temp)->to_s()); pc += sz; }

	ms->seek(0);
	return ms->get_s(ms->size());

#else

	return "no debug info";

#endif
}

int_t inst_size(uint_t no){
	static u8 sizelist[] = {
//{INST_SIZE{{
	InstNop::ISIZE,
	InstLoadNull::ISIZE,
	InstLoadUndefined::ISIZE,
	InstLoadTrue::ISIZE,
	InstLoadFalse::ISIZE,
	InstLoadTrueAndSkip::ISIZE,
	InstLoadInt1Byte::ISIZE,
	InstLoadFloat1Byte::ISIZE,
	InstLoadCallee::ISIZE,
	InstLoadThis::ISIZE,
	InstLoadCurrentContext::ISIZE,
	InstCopy::ISIZE,
	InstPush::ISIZE,
	InstPop::ISIZE,
	InstAdjustValues::ISIZE,
	InstLocalVariable2Byte::ISIZE,
	InstSetLocalVariable2Byte::ISIZE,
	InstInstanceVariable::ISIZE,
	InstSetInstanceVariable::ISIZE,
	InstFilelocalVariable::ISIZE,
	InstReturn::ISIZE,
	InstYield::ISIZE,
	InstExit::ISIZE,
	InstValue::ISIZE,
	InstCall::ISIZE,
	InstSend::ISIZE,
	InstProperty::ISIZE,
	InstSetProperty::ISIZE,
	InstMember::ISIZE,
	InstScopeBegin::ISIZE,
	InstScopeEnd::ISIZE,
	InstPos::ISIZE,
	InstNeg::ISIZE,
	InstCom::ISIZE,
	InstNot::ISIZE,
	InstInc::ISIZE,
	InstDec::ISIZE,
	InstArith::ISIZE,
	InstBitwise::ISIZE,
	InstAt::ISIZE,
	InstSetAt::ISIZE,
	InstGoto::ISIZE,
	InstIf::ISIZE,
	InstIfEq::ISIZE,
	InstIfLt::ISIZE,
	InstIfRawEq::ISIZE,
	InstIfIn::ISIZE,
	InstIfIs::ISIZE,
	InstIfUndefined::ISIZE,
	InstIfDebug::ISIZE,
	InstRange::ISIZE,
	InstOnce::ISIZE,
	InstSetOnce::ISIZE,
	InstMakeArray::ISIZE,
	InstArrayAppend::ISIZE,
	InstMakeMap::ISIZE,
	InstMapInsert::ISIZE,
	InstMapSetDefault::ISIZE,
	InstClassBegin::ISIZE,
	InstClassEnd::ISIZE,
	InstDefineClassMember::ISIZE,
	InstDefineMember::ISIZE,
	InstMakeFun::ISIZE,
	InstMakeInstanceVariableAccessor::ISIZE,
	InstTryBegin::ISIZE,
	InstTryEnd::ISIZE,
	InstPushGoto::ISIZE,
	InstPopGoto::ISIZE,
	InstThrow::ISIZE,
	InstAssert::ISIZE,
	InstBreakPoint::ISIZE,
	InstMAX::ISIZE,
//}}INST_SIZE}
	};

	if(no>=InstMAX::ISIZE){
		return 0;
	}

	return sizelist[no];
}

}
