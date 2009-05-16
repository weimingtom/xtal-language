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
//{INST_INSPECT{{
		XTAL_CASE(InstNop::NUMBER){ temp = ((InstNop*)pc)->inspect(code); sz = InstNop::ISIZE; }
		XTAL_CASE(InstPushNull::NUMBER){ temp = ((InstPushNull*)pc)->inspect(code); sz = InstPushNull::ISIZE; }
		XTAL_CASE(InstPushUndefined::NUMBER){ temp = ((InstPushUndefined*)pc)->inspect(code); sz = InstPushUndefined::ISIZE; }
		XTAL_CASE(InstPushTrue::NUMBER){ temp = ((InstPushTrue*)pc)->inspect(code); sz = InstPushTrue::ISIZE; }
		XTAL_CASE(InstPushFalse::NUMBER){ temp = ((InstPushFalse*)pc)->inspect(code); sz = InstPushFalse::ISIZE; }
		XTAL_CASE(InstPushTrueAndSkip::NUMBER){ temp = ((InstPushTrueAndSkip*)pc)->inspect(code); sz = InstPushTrueAndSkip::ISIZE; }
		XTAL_CASE(InstPushInt1Byte::NUMBER){ temp = ((InstPushInt1Byte*)pc)->inspect(code); sz = InstPushInt1Byte::ISIZE; }
		XTAL_CASE(InstPushFloat1Byte::NUMBER){ temp = ((InstPushFloat1Byte*)pc)->inspect(code); sz = InstPushFloat1Byte::ISIZE; }
		XTAL_CASE(InstPushCallee::NUMBER){ temp = ((InstPushCallee*)pc)->inspect(code); sz = InstPushCallee::ISIZE; }
		XTAL_CASE(InstPushThis::NUMBER){ temp = ((InstPushThis*)pc)->inspect(code); sz = InstPushThis::ISIZE; }
		XTAL_CASE(InstPushCurrentContext::NUMBER){ temp = ((InstPushCurrentContext*)pc)->inspect(code); sz = InstPushCurrentContext::ISIZE; }
		XTAL_CASE(InstPop::NUMBER){ temp = ((InstPop*)pc)->inspect(code); sz = InstPop::ISIZE; }
		XTAL_CASE(InstDup::NUMBER){ temp = ((InstDup*)pc)->inspect(code); sz = InstDup::ISIZE; }
		XTAL_CASE(InstInsert1::NUMBER){ temp = ((InstInsert1*)pc)->inspect(code); sz = InstInsert1::ISIZE; }
		XTAL_CASE(InstInsert2::NUMBER){ temp = ((InstInsert2*)pc)->inspect(code); sz = InstInsert2::ISIZE; }
		XTAL_CASE(InstAdjustResult::NUMBER){ temp = ((InstAdjustResult*)pc)->inspect(code); sz = InstAdjustResult::ISIZE; }
		XTAL_CASE(InstLocalVariableInc1Byte::NUMBER){ temp = ((InstLocalVariableInc1Byte*)pc)->inspect(code); sz = InstLocalVariableInc1Byte::ISIZE; }
		XTAL_CASE(InstLocalVariableDec1Byte::NUMBER){ temp = ((InstLocalVariableDec1Byte*)pc)->inspect(code); sz = InstLocalVariableDec1Byte::ISIZE; }
		XTAL_CASE(InstLocalVariable1Byte::NUMBER){ temp = ((InstLocalVariable1Byte*)pc)->inspect(code); sz = InstLocalVariable1Byte::ISIZE; }
		XTAL_CASE(InstLocalVariable1ByteX2::NUMBER){ temp = ((InstLocalVariable1ByteX2*)pc)->inspect(code); sz = InstLocalVariable1ByteX2::ISIZE; }
		XTAL_CASE(InstLocalVariable2Byte::NUMBER){ temp = ((InstLocalVariable2Byte*)pc)->inspect(code); sz = InstLocalVariable2Byte::ISIZE; }
		XTAL_CASE(InstSetLocalVariable1Byte::NUMBER){ temp = ((InstSetLocalVariable1Byte*)pc)->inspect(code); sz = InstSetLocalVariable1Byte::ISIZE; }
		XTAL_CASE(InstSetLocalVariable1ByteX2::NUMBER){ temp = ((InstSetLocalVariable1ByteX2*)pc)->inspect(code); sz = InstSetLocalVariable1ByteX2::ISIZE; }
		XTAL_CASE(InstSetLocalVariable2Byte::NUMBER){ temp = ((InstSetLocalVariable2Byte*)pc)->inspect(code); sz = InstSetLocalVariable2Byte::ISIZE; }
		XTAL_CASE(InstInstanceVariable::NUMBER){ temp = ((InstInstanceVariable*)pc)->inspect(code); sz = InstInstanceVariable::ISIZE; }
		XTAL_CASE(InstSetInstanceVariable::NUMBER){ temp = ((InstSetInstanceVariable*)pc)->inspect(code); sz = InstSetInstanceVariable::ISIZE; }
		XTAL_CASE(InstFilelocalVariable::NUMBER){ temp = ((InstFilelocalVariable*)pc)->inspect(code); sz = InstFilelocalVariable::ISIZE; }
		XTAL_CASE(InstCleanupCall::NUMBER){ temp = ((InstCleanupCall*)pc)->inspect(code); sz = InstCleanupCall::ISIZE; }
		XTAL_CASE(InstReturn::NUMBER){ temp = ((InstReturn*)pc)->inspect(code); sz = InstReturn::ISIZE; }
		XTAL_CASE(InstYield::NUMBER){ temp = ((InstYield*)pc)->inspect(code); sz = InstYield::ISIZE; }
		XTAL_CASE(InstExit::NUMBER){ temp = ((InstExit*)pc)->inspect(code); sz = InstExit::ISIZE; }
		XTAL_CASE(InstValue::NUMBER){ temp = ((InstValue*)pc)->inspect(code); sz = InstValue::ISIZE; }
		XTAL_CASE(InstCheckUnsupported::NUMBER){ temp = ((InstCheckUnsupported*)pc)->inspect(code); sz = InstCheckUnsupported::ISIZE; }
		XTAL_CASE(InstProperty::NUMBER){ temp = ((InstProperty*)pc)->inspect(code); sz = InstProperty::ISIZE; }
		XTAL_CASE(InstSetProperty::NUMBER){ temp = ((InstSetProperty*)pc)->inspect(code); sz = InstSetProperty::ISIZE; }
		XTAL_CASE(InstCall::NUMBER){ temp = ((InstCall*)pc)->inspect(code); sz = InstCall::ISIZE; }
		XTAL_CASE(InstSend::NUMBER){ temp = ((InstSend*)pc)->inspect(code); sz = InstSend::ISIZE; }
		XTAL_CASE(InstMember::NUMBER){ temp = ((InstMember*)pc)->inspect(code); sz = InstMember::ISIZE; }
		XTAL_CASE(InstDefineMember::NUMBER){ temp = ((InstDefineMember*)pc)->inspect(code); sz = InstDefineMember::ISIZE; }
		XTAL_CASE(InstScopeBegin::NUMBER){ temp = ((InstScopeBegin*)pc)->inspect(code); sz = InstScopeBegin::ISIZE; }
		XTAL_CASE(InstScopeEnd::NUMBER){ temp = ((InstScopeEnd*)pc)->inspect(code); sz = InstScopeEnd::ISIZE; }
		XTAL_CASE(InstTryBegin::NUMBER){ temp = ((InstTryBegin*)pc)->inspect(code); sz = InstTryBegin::ISIZE; }
		XTAL_CASE(InstTryEnd::NUMBER){ temp = ((InstTryEnd*)pc)->inspect(code); sz = InstTryEnd::ISIZE; }
		XTAL_CASE(InstPushGoto::NUMBER){ temp = ((InstPushGoto*)pc)->inspect(code); sz = InstPushGoto::ISIZE; }
		XTAL_CASE(InstPopGoto::NUMBER){ temp = ((InstPopGoto*)pc)->inspect(code); sz = InstPopGoto::ISIZE; }
		XTAL_CASE(InstGoto::NUMBER){ temp = ((InstGoto*)pc)->inspect(code); sz = InstGoto::ISIZE; }
		XTAL_CASE(InstIf::NUMBER){ temp = ((InstIf*)pc)->inspect(code); sz = InstIf::ISIZE; }
		XTAL_CASE(InstIfEq::NUMBER){ temp = ((InstIfEq*)pc)->inspect(code); sz = InstIfEq::ISIZE; }
		XTAL_CASE(InstIfLt::NUMBER){ temp = ((InstIfLt*)pc)->inspect(code); sz = InstIfLt::ISIZE; }
		XTAL_CASE(InstIfRawEq::NUMBER){ temp = ((InstIfRawEq*)pc)->inspect(code); sz = InstIfRawEq::ISIZE; }
		XTAL_CASE(InstIfIn::NUMBER){ temp = ((InstIfIn*)pc)->inspect(code); sz = InstIfIn::ISIZE; }
		XTAL_CASE(InstIfIs::NUMBER){ temp = ((InstIfIs*)pc)->inspect(code); sz = InstIfIs::ISIZE; }
		XTAL_CASE(InstIfArgIsUndefined::NUMBER){ temp = ((InstIfArgIsUndefined*)pc)->inspect(code); sz = InstIfArgIsUndefined::ISIZE; }
		XTAL_CASE(InstPos::NUMBER){ temp = ((InstPos*)pc)->inspect(code); sz = InstPos::ISIZE; }
		XTAL_CASE(InstNeg::NUMBER){ temp = ((InstNeg*)pc)->inspect(code); sz = InstNeg::ISIZE; }
		XTAL_CASE(InstCom::NUMBER){ temp = ((InstCom*)pc)->inspect(code); sz = InstCom::ISIZE; }
		XTAL_CASE(InstNot::NUMBER){ temp = ((InstNot*)pc)->inspect(code); sz = InstNot::ISIZE; }
		XTAL_CASE(InstAt::NUMBER){ temp = ((InstAt*)pc)->inspect(code); sz = InstAt::ISIZE; }
		XTAL_CASE(InstSetAt::NUMBER){ temp = ((InstSetAt*)pc)->inspect(code); sz = InstSetAt::ISIZE; }
		XTAL_CASE(InstAdd::NUMBER){ temp = ((InstAdd*)pc)->inspect(code); sz = InstAdd::ISIZE; }
		XTAL_CASE(InstSub::NUMBER){ temp = ((InstSub*)pc)->inspect(code); sz = InstSub::ISIZE; }
		XTAL_CASE(InstCat::NUMBER){ temp = ((InstCat*)pc)->inspect(code); sz = InstCat::ISIZE; }
		XTAL_CASE(InstMul::NUMBER){ temp = ((InstMul*)pc)->inspect(code); sz = InstMul::ISIZE; }
		XTAL_CASE(InstDiv::NUMBER){ temp = ((InstDiv*)pc)->inspect(code); sz = InstDiv::ISIZE; }
		XTAL_CASE(InstMod::NUMBER){ temp = ((InstMod*)pc)->inspect(code); sz = InstMod::ISIZE; }
		XTAL_CASE(InstAnd::NUMBER){ temp = ((InstAnd*)pc)->inspect(code); sz = InstAnd::ISIZE; }
		XTAL_CASE(InstOr::NUMBER){ temp = ((InstOr*)pc)->inspect(code); sz = InstOr::ISIZE; }
		XTAL_CASE(InstXor::NUMBER){ temp = ((InstXor*)pc)->inspect(code); sz = InstXor::ISIZE; }
		XTAL_CASE(InstShl::NUMBER){ temp = ((InstShl*)pc)->inspect(code); sz = InstShl::ISIZE; }
		XTAL_CASE(InstShr::NUMBER){ temp = ((InstShr*)pc)->inspect(code); sz = InstShr::ISIZE; }
		XTAL_CASE(InstUshr::NUMBER){ temp = ((InstUshr*)pc)->inspect(code); sz = InstUshr::ISIZE; }
		XTAL_CASE(InstInc::NUMBER){ temp = ((InstInc*)pc)->inspect(code); sz = InstInc::ISIZE; }
		XTAL_CASE(InstDec::NUMBER){ temp = ((InstDec*)pc)->inspect(code); sz = InstDec::ISIZE; }
		XTAL_CASE(InstAddAssign::NUMBER){ temp = ((InstAddAssign*)pc)->inspect(code); sz = InstAddAssign::ISIZE; }
		XTAL_CASE(InstSubAssign::NUMBER){ temp = ((InstSubAssign*)pc)->inspect(code); sz = InstSubAssign::ISIZE; }
		XTAL_CASE(InstCatAssign::NUMBER){ temp = ((InstCatAssign*)pc)->inspect(code); sz = InstCatAssign::ISIZE; }
		XTAL_CASE(InstMulAssign::NUMBER){ temp = ((InstMulAssign*)pc)->inspect(code); sz = InstMulAssign::ISIZE; }
		XTAL_CASE(InstDivAssign::NUMBER){ temp = ((InstDivAssign*)pc)->inspect(code); sz = InstDivAssign::ISIZE; }
		XTAL_CASE(InstModAssign::NUMBER){ temp = ((InstModAssign*)pc)->inspect(code); sz = InstModAssign::ISIZE; }
		XTAL_CASE(InstAndAssign::NUMBER){ temp = ((InstAndAssign*)pc)->inspect(code); sz = InstAndAssign::ISIZE; }
		XTAL_CASE(InstOrAssign::NUMBER){ temp = ((InstOrAssign*)pc)->inspect(code); sz = InstOrAssign::ISIZE; }
		XTAL_CASE(InstXorAssign::NUMBER){ temp = ((InstXorAssign*)pc)->inspect(code); sz = InstXorAssign::ISIZE; }
		XTAL_CASE(InstShlAssign::NUMBER){ temp = ((InstShlAssign*)pc)->inspect(code); sz = InstShlAssign::ISIZE; }
		XTAL_CASE(InstShrAssign::NUMBER){ temp = ((InstShrAssign*)pc)->inspect(code); sz = InstShrAssign::ISIZE; }
		XTAL_CASE(InstUshrAssign::NUMBER){ temp = ((InstUshrAssign*)pc)->inspect(code); sz = InstUshrAssign::ISIZE; }
		XTAL_CASE(InstRange::NUMBER){ temp = ((InstRange*)pc)->inspect(code); sz = InstRange::ISIZE; }
		XTAL_CASE(InstOnce::NUMBER){ temp = ((InstOnce*)pc)->inspect(code); sz = InstOnce::ISIZE; }
		XTAL_CASE(InstSetOnce::NUMBER){ temp = ((InstSetOnce*)pc)->inspect(code); sz = InstSetOnce::ISIZE; }
		XTAL_CASE(InstClassBegin::NUMBER){ temp = ((InstClassBegin*)pc)->inspect(code); sz = InstClassBegin::ISIZE; }
		XTAL_CASE(InstClassEnd::NUMBER){ temp = ((InstClassEnd*)pc)->inspect(code); sz = InstClassEnd::ISIZE; }
		XTAL_CASE(InstDefineClassMember::NUMBER){ temp = ((InstDefineClassMember*)pc)->inspect(code); sz = InstDefineClassMember::ISIZE; }
		XTAL_CASE(InstMakeArray::NUMBER){ temp = ((InstMakeArray*)pc)->inspect(code); sz = InstMakeArray::ISIZE; }
		XTAL_CASE(InstArrayAppend::NUMBER){ temp = ((InstArrayAppend*)pc)->inspect(code); sz = InstArrayAppend::ISIZE; }
		XTAL_CASE(InstMakeMap::NUMBER){ temp = ((InstMakeMap*)pc)->inspect(code); sz = InstMakeMap::ISIZE; }
		XTAL_CASE(InstMapInsert::NUMBER){ temp = ((InstMapInsert*)pc)->inspect(code); sz = InstMapInsert::ISIZE; }
		XTAL_CASE(InstMapSetDefault::NUMBER){ temp = ((InstMapSetDefault*)pc)->inspect(code); sz = InstMapSetDefault::ISIZE; }
		XTAL_CASE(InstMakeFun::NUMBER){ temp = ((InstMakeFun*)pc)->inspect(code); sz = InstMakeFun::ISIZE; }
		XTAL_CASE(InstMakeInstanceVariableAccessor::NUMBER){ temp = ((InstMakeInstanceVariableAccessor*)pc)->inspect(code); sz = InstMakeInstanceVariableAccessor::ISIZE; }
		XTAL_CASE(InstThrow::NUMBER){ temp = ((InstThrow*)pc)->inspect(code); sz = InstThrow::ISIZE; }
		XTAL_CASE(InstThrowUnsupportedError::NUMBER){ temp = ((InstThrowUnsupportedError*)pc)->inspect(code); sz = InstThrowUnsupportedError::ISIZE; }
		XTAL_CASE(InstIfDebug::NUMBER){ temp = ((InstIfDebug*)pc)->inspect(code); sz = InstIfDebug::ISIZE; }
		XTAL_CASE(InstAssert::NUMBER){ temp = ((InstAssert*)pc)->inspect(code); sz = InstAssert::ISIZE; }
		XTAL_CASE(InstBreakPoint::NUMBER){ temp = ((InstBreakPoint*)pc)->inspect(code); sz = InstBreakPoint::ISIZE; }
		XTAL_CASE(InstMAX::NUMBER){ temp = ((InstMAX*)pc)->inspect(code); sz = InstMAX::ISIZE; }
//}}INST_INSPECT}
	} ms->put_s(Xf("%04d:%s\n")->call((int_t)(pc-start), temp)->to_s()); pc += sz; }

	ms->seek(0);
	return ms->get_s(ms->size());

#else

	return "no debug info";

#endif
}

u8 inst_size(uint_t no){
	static u8 sizelist[] = {
//{INST_SIZE{{
	InstNop::ISIZE,
	InstPushNull::ISIZE,
	InstPushUndefined::ISIZE,
	InstPushTrue::ISIZE,
	InstPushFalse::ISIZE,
	InstPushTrueAndSkip::ISIZE,
	InstPushInt1Byte::ISIZE,
	InstPushFloat1Byte::ISIZE,
	InstPushCallee::ISIZE,
	InstPushThis::ISIZE,
	InstPushCurrentContext::ISIZE,
	InstPop::ISIZE,
	InstDup::ISIZE,
	InstInsert1::ISIZE,
	InstInsert2::ISIZE,
	InstAdjustResult::ISIZE,
	InstLocalVariableInc1Byte::ISIZE,
	InstLocalVariableDec1Byte::ISIZE,
	InstLocalVariable1Byte::ISIZE,
	InstLocalVariable1ByteX2::ISIZE,
	InstLocalVariable2Byte::ISIZE,
	InstSetLocalVariable1Byte::ISIZE,
	InstSetLocalVariable1ByteX2::ISIZE,
	InstSetLocalVariable2Byte::ISIZE,
	InstInstanceVariable::ISIZE,
	InstSetInstanceVariable::ISIZE,
	InstFilelocalVariable::ISIZE,
	InstCleanupCall::ISIZE,
	InstReturn::ISIZE,
	InstYield::ISIZE,
	InstExit::ISIZE,
	InstValue::ISIZE,
	InstCheckUnsupported::ISIZE,
	InstProperty::ISIZE,
	InstSetProperty::ISIZE,
	InstCall::ISIZE,
	InstSend::ISIZE,
	InstMember::ISIZE,
	InstDefineMember::ISIZE,
	InstScopeBegin::ISIZE,
	InstScopeEnd::ISIZE,
	InstTryBegin::ISIZE,
	InstTryEnd::ISIZE,
	InstPushGoto::ISIZE,
	InstPopGoto::ISIZE,
	InstGoto::ISIZE,
	InstIf::ISIZE,
	InstIfEq::ISIZE,
	InstIfLt::ISIZE,
	InstIfRawEq::ISIZE,
	InstIfIn::ISIZE,
	InstIfIs::ISIZE,
	InstIfArgIsUndefined::ISIZE,
	InstPos::ISIZE,
	InstNeg::ISIZE,
	InstCom::ISIZE,
	InstNot::ISIZE,
	InstAt::ISIZE,
	InstSetAt::ISIZE,
	InstAdd::ISIZE,
	InstSub::ISIZE,
	InstCat::ISIZE,
	InstMul::ISIZE,
	InstDiv::ISIZE,
	InstMod::ISIZE,
	InstAnd::ISIZE,
	InstOr::ISIZE,
	InstXor::ISIZE,
	InstShl::ISIZE,
	InstShr::ISIZE,
	InstUshr::ISIZE,
	InstInc::ISIZE,
	InstDec::ISIZE,
	InstAddAssign::ISIZE,
	InstSubAssign::ISIZE,
	InstCatAssign::ISIZE,
	InstMulAssign::ISIZE,
	InstDivAssign::ISIZE,
	InstModAssign::ISIZE,
	InstAndAssign::ISIZE,
	InstOrAssign::ISIZE,
	InstXorAssign::ISIZE,
	InstShlAssign::ISIZE,
	InstShrAssign::ISIZE,
	InstUshrAssign::ISIZE,
	InstRange::ISIZE,
	InstOnce::ISIZE,
	InstSetOnce::ISIZE,
	InstClassBegin::ISIZE,
	InstClassEnd::ISIZE,
	InstDefineClassMember::ISIZE,
	InstMakeArray::ISIZE,
	InstArrayAppend::ISIZE,
	InstMakeMap::ISIZE,
	InstMapInsert::ISIZE,
	InstMapSetDefault::ISIZE,
	InstMakeFun::ISIZE,
	InstMakeInstanceVariableAccessor::ISIZE,
	InstThrow::ISIZE,
	InstThrowUnsupportedError::ISIZE,
	InstIfDebug::ISIZE,
	InstAssert::ISIZE,
	InstBreakPoint::ISIZE,
	InstMAX::ISIZE,
//}}INST_SIZE}
	};

	return sizelist[no];
}

}
