#include "xtal.h"
#include "xtal_macro.h"

namespace xtal{

#ifdef XTAL_DEBUG

int_t inst_inspect(inst_address_t& value, Inst* inst, const CodePtr& code){
	int_t pc = (inst_t*)inst - code->data();
	return pc + value;
}

StringPtr make_inst_string(const StringLiteral& InstName){
	StringPtr temp = format(XTAL_STRING("%s:"))->call(InstName)->to_s();
	return format(temp)->call()->to_s();
}

StringPtr make_inst_string(const StringLiteral& InstName, 
						const StringLiteral& MemberName1, int_t MemberValue1){
	StringPtr temp = format(XTAL_STRING("%s: %s=%%s"))->call(InstName, MemberName1)->to_s();
	return format(temp)->call(MemberValue1)->to_s();
}

StringPtr make_inst_string(const StringLiteral& InstName, 
						const StringLiteral& MemberName1, int_t MemberValue1,
						const StringLiteral& MemberName2, int_t MemberValue2){
	StringPtr temp = format(XTAL_STRING("%s: %s=%%s, %s=%%s"))->call(InstName, MemberName1, MemberName2)->to_s();
	return format(temp)->call(MemberValue1, MemberValue2)->to_s();
}

StringPtr make_inst_string(const StringLiteral& InstName, 
						const StringLiteral& MemberName1, int_t MemberValue1,
						const StringLiteral& MemberName2, int_t MemberValue2,
						const StringLiteral& MemberName3, int_t MemberValue3){
	StringPtr temp = format(XTAL_STRING("%s: %s=%%s, %s=%%s, %s=%%s"))->call(InstName, MemberName1, MemberName2, MemberName3)->to_s();
	return format(temp)->call(MemberValue1, MemberValue2, MemberValue3)->to_s();
}

StringPtr make_inst_string(const StringLiteral& InstName, 
						const StringLiteral& MemberName1, int_t MemberValue1,
						const StringLiteral& MemberName2, int_t MemberValue2,
						const StringLiteral& MemberName3, int_t MemberValue3,
						const StringLiteral& MemberName4, int_t MemberValue4){
	StringPtr temp = format(XTAL_STRING("%s: %s=%%s, %s=%%s, %s=%%s, %s=%%s"))->call(InstName, MemberName1, MemberName2, MemberName3, MemberName4)->to_s();
	return format(temp)->call(MemberValue1, MemberValue2, MemberValue3, MemberValue4)->to_s();
}

StringPtr make_inst_string(const StringLiteral& InstName, 
						const StringLiteral& MemberName1, int_t MemberValue1,
						const StringLiteral& MemberName2, int_t MemberValue2,
						const StringLiteral& MemberName3, int_t MemberValue3,
						const StringLiteral& MemberName4, int_t MemberValue4,
						const StringLiteral& MemberName5, int_t MemberValue5){					
	StringPtr temp = format(XTAL_STRING("%s: %s=%%s, %s=%%s, %s=%%s, %s=%%s, %s=%%s"))->call(InstName, MemberName1, MemberName2, MemberName3, MemberName4, MemberName5)->to_s();
	return format(temp)->call(MemberValue1, MemberValue2, MemberValue3, MemberValue4, MemberValue5)->to_s();
}

#endif

StringPtr inspect_range(const CodePtr& code, const inst_t* start, const inst_t* end){

#ifdef XTAL_DEBUG

	int sz = 0;
	const inst_t* pc = start;
	StringPtr temp;
	MemoryStreamPtr ms = xnew<MemoryStream>();

	for(; pc < end;){switch(*pc){
		XTAL_NODEFAULT;

#define XTAL_INST_CASE(x) XTAL_CASE(x::NUMBER){ temp = ((x*)pc)->inspect(code); sz = x::ISIZE; }

//{INST_INSPECT{{
		XTAL_INST_CASE(InstLine);
		XTAL_INST_CASE(InstLoadValue);
		XTAL_INST_CASE(InstLoadConstant);
		XTAL_INST_CASE(InstLoadInt1Byte);
		XTAL_INST_CASE(InstLoadFloat1Byte);
		XTAL_INST_CASE(InstLoadCallee);
		XTAL_INST_CASE(InstLoadThis);
		XTAL_INST_CASE(InstCopy);
		XTAL_INST_CASE(InstMember);
		XTAL_INST_CASE(InstUna);
		XTAL_INST_CASE(InstArith);
		XTAL_INST_CASE(InstBitwise);
		XTAL_INST_CASE(InstAt);
		XTAL_INST_CASE(InstSetAt);
		XTAL_INST_CASE(InstGoto);
		XTAL_INST_CASE(InstNot);
		XTAL_INST_CASE(InstIf);
		XTAL_INST_CASE(InstIfComp);
		XTAL_INST_CASE(InstIfRawEq);
		XTAL_INST_CASE(InstIfIs);
		XTAL_INST_CASE(InstIfUndefined);
		XTAL_INST_CASE(InstIfDebug);
		XTAL_INST_CASE(InstPush);
		XTAL_INST_CASE(InstPop);
		XTAL_INST_CASE(InstAdjustValues);
		XTAL_INST_CASE(InstLocalVariable);
		XTAL_INST_CASE(InstSetLocalVariable);
		XTAL_INST_CASE(InstInstanceVariable);
		XTAL_INST_CASE(InstSetInstanceVariable);
		XTAL_INST_CASE(InstInstanceVariableByName);
		XTAL_INST_CASE(InstSetInstanceVariableByName);
		XTAL_INST_CASE(InstFilelocalVariable);
		XTAL_INST_CASE(InstSetFilelocalVariable);
		XTAL_INST_CASE(InstFilelocalVariableByName);
		XTAL_INST_CASE(InstSetFilelocalVariableByName);
		XTAL_INST_CASE(InstCall);
		XTAL_INST_CASE(InstSend);
		XTAL_INST_CASE(InstProperty);
		XTAL_INST_CASE(InstSetProperty);
		XTAL_INST_CASE(InstScopeBegin);
		XTAL_INST_CASE(InstScopeEnd);
		XTAL_INST_CASE(InstReturn);
		XTAL_INST_CASE(InstYield);
		XTAL_INST_CASE(InstExit);
		XTAL_INST_CASE(InstRange);
		XTAL_INST_CASE(InstOnce);
		XTAL_INST_CASE(InstSetOnce);
		XTAL_INST_CASE(InstMakeArray);
		XTAL_INST_CASE(InstArrayAppend);
		XTAL_INST_CASE(InstMakeMap);
		XTAL_INST_CASE(InstMapInsert);
		XTAL_INST_CASE(InstMapSetDefault);
		XTAL_INST_CASE(InstClassBegin);
		XTAL_INST_CASE(InstClassEnd);
		XTAL_INST_CASE(InstDefineClassMember);
		XTAL_INST_CASE(InstDefineMember);
		XTAL_INST_CASE(InstMakeFun);
		XTAL_INST_CASE(InstMakeInstanceVariableAccessor);
		XTAL_INST_CASE(InstTryBegin);
		XTAL_INST_CASE(InstTryEnd);
		XTAL_INST_CASE(InstPushGoto);
		XTAL_INST_CASE(InstPopGoto);
		XTAL_INST_CASE(InstThrow);
		XTAL_INST_CASE(InstAssert);
		XTAL_INST_CASE(InstBreakPoint);
		XTAL_INST_CASE(InstMAX);
//}}INST_INSPECT}
	} ms->put_s(Xf("%04d(%04d):%s\n")->call((int_t)(pc-start), code->compliant_lineno(pc), temp)->to_s()); pc += sz; }

	ms->seek(0);
	return ms->get_s(ms->size());

#else

	return XTAL_STRING("");

#endif

}

int_t inst_size(uint_t no){
	static u8 sizelist[] = {
//{INST_SIZE{{
	InstLine::ISIZE,
	InstLoadValue::ISIZE,
	InstLoadConstant::ISIZE,
	InstLoadInt1Byte::ISIZE,
	InstLoadFloat1Byte::ISIZE,
	InstLoadCallee::ISIZE,
	InstLoadThis::ISIZE,
	InstCopy::ISIZE,
	InstMember::ISIZE,
	InstUna::ISIZE,
	InstArith::ISIZE,
	InstBitwise::ISIZE,
	InstAt::ISIZE,
	InstSetAt::ISIZE,
	InstGoto::ISIZE,
	InstNot::ISIZE,
	InstIf::ISIZE,
	InstIfComp::ISIZE,
	InstIfRawEq::ISIZE,
	InstIfIs::ISIZE,
	InstIfUndefined::ISIZE,
	InstIfDebug::ISIZE,
	InstPush::ISIZE,
	InstPop::ISIZE,
	InstAdjustValues::ISIZE,
	InstLocalVariable::ISIZE,
	InstSetLocalVariable::ISIZE,
	InstInstanceVariable::ISIZE,
	InstSetInstanceVariable::ISIZE,
	InstInstanceVariableByName::ISIZE,
	InstSetInstanceVariableByName::ISIZE,
	InstFilelocalVariable::ISIZE,
	InstSetFilelocalVariable::ISIZE,
	InstFilelocalVariableByName::ISIZE,
	InstSetFilelocalVariableByName::ISIZE,
	InstCall::ISIZE,
	InstSend::ISIZE,
	InstProperty::ISIZE,
	InstSetProperty::ISIZE,
	InstScopeBegin::ISIZE,
	InstScopeEnd::ISIZE,
	InstReturn::ISIZE,
	InstYield::ISIZE,
	InstExit::ISIZE,
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

	if(no>=InstMAX::NUMBER){
		return 0;
	}

	return sizelist[no];
}

}