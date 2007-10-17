#include "xtal.h"
#include "xtal_macro.h"

namespace xtal{

FunCore empty_xfun_core;
BlockCore empty_block_core;
ClassCore empty_class_core;
ExceptCore empty_except_core;

void initialize_code(){
	{
		ClassPtr p = new_cpp_class<Code>("Code");
		p->inherit(get_cpp_class<Fun>());
		p->method("inspect", &Code::inspect);
	}

	builtin()->def("Code", get_cpp_class<Code>());
}


Code::Code()
	:filelocal_(xnew<Singleton>("filelocal")), source_file_name_("<noname>"){
	set_object_name("<filelocal>", 1, null);
	filelocal_->inherit(builtin());
	filelocal_->def("filelocal", filelocal_);

	identifier_table_ = xnew<Array>();
	value_table_ = xnew<Array>();

	first_fun_ = xnew<Fun>(null, null, CodePtr(this), (FunCore*)0);
	first_fun_->set_object_name("<toplevel>", 1, null);
}

void Code::set_lineno_info(int_t line){
	if(!lineno_table_.empty() && lineno_table_.back().lineno==line)
		return;
	LineNumberTable lnt={(u16)code_.size(), (u16)line};
	lineno_table_.push_back(lnt);
}

int_t Code::compliant_lineno(const inst_t* p){
	AC<LineNumberTable>::vector::const_iterator it=
		std::lower_bound(
			lineno_table_.begin(),
			lineno_table_.end(),
			static_cast<int_t>(p-data()),
			LineNumberCmp()
		);

	if(it!=lineno_table_.end()){
		if(it==lineno_table_.begin()){
			return 1;
		}
		--it;
		return it->lineno;
	}
	return 0;
}

void Code::call(const VMachinePtr& vm){
	first_fun_->call(vm);
}

StringPtr Code::inspect(){
	return inspect_range(0, size());
}
	
StringPtr Code::inspect_range(int_t start, int_t end){

	int sz = 0;
	const inst_t* pc = data()+start;
	StringPtr temp;
	MemoryStreamPtr ms(xnew<MemoryStream>());
	CodePtr code(this);

	for(; pc < data() + end;){switch(*pc){
		XTAL_NODEFAULT;

//{CODE_INSPECT{{
		XTAL_CASE(InstNop::NUMBER){ temp = ((InstNop*)pc)->inspect(code); sz = InstNop::ISIZE; }
		XTAL_CASE(InstPushNull::NUMBER){ temp = ((InstPushNull*)pc)->inspect(code); sz = InstPushNull::ISIZE; }
		XTAL_CASE(InstPushNop::NUMBER){ temp = ((InstPushNop*)pc)->inspect(code); sz = InstPushNop::ISIZE; }
		XTAL_CASE(InstPushTrue::NUMBER){ temp = ((InstPushTrue*)pc)->inspect(code); sz = InstPushTrue::ISIZE; }
		XTAL_CASE(InstPushFalse::NUMBER){ temp = ((InstPushFalse*)pc)->inspect(code); sz = InstPushFalse::ISIZE; }
		XTAL_CASE(InstPushInt1Byte::NUMBER){ temp = ((InstPushInt1Byte*)pc)->inspect(code); sz = InstPushInt1Byte::ISIZE; }
		XTAL_CASE(InstPushInt2Byte::NUMBER){ temp = ((InstPushInt2Byte*)pc)->inspect(code); sz = InstPushInt2Byte::ISIZE; }
		XTAL_CASE(InstPushFloat1Byte::NUMBER){ temp = ((InstPushFloat1Byte*)pc)->inspect(code); sz = InstPushFloat1Byte::ISIZE; }
		XTAL_CASE(InstPushFloat2Byte::NUMBER){ temp = ((InstPushFloat2Byte*)pc)->inspect(code); sz = InstPushFloat2Byte::ISIZE; }
		XTAL_CASE(InstPushCallee::NUMBER){ temp = ((InstPushCallee*)pc)->inspect(code); sz = InstPushCallee::ISIZE; }
		XTAL_CASE(InstPushArgs::NUMBER){ temp = ((InstPushArgs*)pc)->inspect(code); sz = InstPushArgs::ISIZE; }
		XTAL_CASE(InstPushThis::NUMBER){ temp = ((InstPushThis*)pc)->inspect(code); sz = InstPushThis::ISIZE; }
		XTAL_CASE(InstPushCurrentContext::NUMBER){ temp = ((InstPushCurrentContext*)pc)->inspect(code); sz = InstPushCurrentContext::ISIZE; }
		XTAL_CASE(InstPop::NUMBER){ temp = ((InstPop*)pc)->inspect(code); sz = InstPop::ISIZE; }
		XTAL_CASE(InstDup::NUMBER){ temp = ((InstDup*)pc)->inspect(code); sz = InstDup::ISIZE; }
		XTAL_CASE(InstInsert1::NUMBER){ temp = ((InstInsert1*)pc)->inspect(code); sz = InstInsert1::ISIZE; }
		XTAL_CASE(InstInsert2::NUMBER){ temp = ((InstInsert2*)pc)->inspect(code); sz = InstInsert2::ISIZE; }
		XTAL_CASE(InstInsert3::NUMBER){ temp = ((InstInsert3*)pc)->inspect(code); sz = InstInsert3::ISIZE; }
		XTAL_CASE(InstAdjustResult::NUMBER){ temp = ((InstAdjustResult*)pc)->inspect(code); sz = InstAdjustResult::ISIZE; }
		XTAL_CASE(InstIf::NUMBER){ temp = ((InstIf*)pc)->inspect(code); sz = InstIf::ISIZE; }
		XTAL_CASE(InstUnless::NUMBER){ temp = ((InstUnless*)pc)->inspect(code); sz = InstUnless::ISIZE; }
		XTAL_CASE(InstGoto::NUMBER){ temp = ((InstGoto*)pc)->inspect(code); sz = InstGoto::ISIZE; }
		XTAL_CASE(InstLocalVariableInc::NUMBER){ temp = ((InstLocalVariableInc*)pc)->inspect(code); sz = InstLocalVariableInc::ISIZE; }
		XTAL_CASE(InstLocalVariableIncDirect::NUMBER){ temp = ((InstLocalVariableIncDirect*)pc)->inspect(code); sz = InstLocalVariableIncDirect::ISIZE; }
		XTAL_CASE(InstLocalVariableDec::NUMBER){ temp = ((InstLocalVariableDec*)pc)->inspect(code); sz = InstLocalVariableDec::ISIZE; }
		XTAL_CASE(InstLocalVariableDecDirect::NUMBER){ temp = ((InstLocalVariableDecDirect*)pc)->inspect(code); sz = InstLocalVariableDecDirect::ISIZE; }
		XTAL_CASE(InstLocalVariableInc2Byte::NUMBER){ temp = ((InstLocalVariableInc2Byte*)pc)->inspect(code); sz = InstLocalVariableInc2Byte::ISIZE; }
		XTAL_CASE(InstLocalVariableDec2Byte::NUMBER){ temp = ((InstLocalVariableDec2Byte*)pc)->inspect(code); sz = InstLocalVariableDec2Byte::ISIZE; }
		XTAL_CASE(InstLocalVariable1Byte::NUMBER){ temp = ((InstLocalVariable1Byte*)pc)->inspect(code); sz = InstLocalVariable1Byte::ISIZE; }
		XTAL_CASE(InstLocalVariable1ByteDirect::NUMBER){ temp = ((InstLocalVariable1ByteDirect*)pc)->inspect(code); sz = InstLocalVariable1ByteDirect::ISIZE; }
		XTAL_CASE(InstLocalVariable2Byte::NUMBER){ temp = ((InstLocalVariable2Byte*)pc)->inspect(code); sz = InstLocalVariable2Byte::ISIZE; }
		XTAL_CASE(InstSetLocalVariable1Byte::NUMBER){ temp = ((InstSetLocalVariable1Byte*)pc)->inspect(code); sz = InstSetLocalVariable1Byte::ISIZE; }
		XTAL_CASE(InstSetLocalVariable1ByteDirect::NUMBER){ temp = ((InstSetLocalVariable1ByteDirect*)pc)->inspect(code); sz = InstSetLocalVariable1ByteDirect::ISIZE; }
		XTAL_CASE(InstSetLocalVariable2Byte::NUMBER){ temp = ((InstSetLocalVariable2Byte*)pc)->inspect(code); sz = InstSetLocalVariable2Byte::ISIZE; }
		XTAL_CASE(InstInstanceVariable::NUMBER){ temp = ((InstInstanceVariable*)pc)->inspect(code); sz = InstInstanceVariable::ISIZE; }
		XTAL_CASE(InstSetInstanceVariable::NUMBER){ temp = ((InstSetInstanceVariable*)pc)->inspect(code); sz = InstSetInstanceVariable::ISIZE; }
		XTAL_CASE(InstCleanupCall::NUMBER){ temp = ((InstCleanupCall*)pc)->inspect(code); sz = InstCleanupCall::ISIZE; }
		XTAL_CASE(InstReturn0::NUMBER){ temp = ((InstReturn0*)pc)->inspect(code); sz = InstReturn0::ISIZE; }
		XTAL_CASE(InstReturn1::NUMBER){ temp = ((InstReturn1*)pc)->inspect(code); sz = InstReturn1::ISIZE; }
		XTAL_CASE(InstReturn2::NUMBER){ temp = ((InstReturn2*)pc)->inspect(code); sz = InstReturn2::ISIZE; }
		XTAL_CASE(InstReturn::NUMBER){ temp = ((InstReturn*)pc)->inspect(code); sz = InstReturn::ISIZE; }
		XTAL_CASE(InstYield::NUMBER){ temp = ((InstYield*)pc)->inspect(code); sz = InstYield::ISIZE; }
		XTAL_CASE(InstExit::NUMBER){ temp = ((InstExit*)pc)->inspect(code); sz = InstExit::ISIZE; }
		XTAL_CASE(InstValue::NUMBER){ temp = ((InstValue*)pc)->inspect(code); sz = InstValue::ISIZE; }
		XTAL_CASE(InstCheckUnsupported::NUMBER){ temp = ((InstCheckUnsupported*)pc)->inspect(code); sz = InstCheckUnsupported::ISIZE; }
		XTAL_CASE(InstProperty::NUMBER){ temp = ((InstProperty*)pc)->inspect(code); sz = InstProperty::ISIZE; }
		XTAL_CASE(InstPropertyNS::NUMBER){ temp = ((InstPropertyNS*)pc)->inspect(code); sz = InstPropertyNS::ISIZE; }
		XTAL_CASE(InstSetProperty::NUMBER){ temp = ((InstSetProperty*)pc)->inspect(code); sz = InstSetProperty::ISIZE; }
		XTAL_CASE(InstSetPropertyNS::NUMBER){ temp = ((InstSetPropertyNS*)pc)->inspect(code); sz = InstSetPropertyNS::ISIZE; }
		XTAL_CASE(InstCall::NUMBER){ temp = ((InstCall*)pc)->inspect(code); sz = InstCall::ISIZE; }
		XTAL_CASE(InstSend::NUMBER){ temp = ((InstSend*)pc)->inspect(code); sz = InstSend::ISIZE; }
		XTAL_CASE(InstSendNS::NUMBER){ temp = ((InstSendNS*)pc)->inspect(code); sz = InstSendNS::ISIZE; }
		XTAL_CASE(InstSendQ::NUMBER){ temp = ((InstSendQ*)pc)->inspect(code); sz = InstSendQ::ISIZE; }
		XTAL_CASE(InstSendQNS::NUMBER){ temp = ((InstSendQNS*)pc)->inspect(code); sz = InstSendQNS::ISIZE; }
		XTAL_CASE(InstMember::NUMBER){ temp = ((InstMember*)pc)->inspect(code); sz = InstMember::ISIZE; }
		XTAL_CASE(InstMemberNS::NUMBER){ temp = ((InstMemberNS*)pc)->inspect(code); sz = InstMemberNS::ISIZE; }
		XTAL_CASE(InstMemberQ::NUMBER){ temp = ((InstMemberQ*)pc)->inspect(code); sz = InstMemberQ::ISIZE; }
		XTAL_CASE(InstMemberQNS::NUMBER){ temp = ((InstMemberQNS*)pc)->inspect(code); sz = InstMemberQNS::ISIZE; }
		XTAL_CASE(InstDefineMember::NUMBER){ temp = ((InstDefineMember*)pc)->inspect(code); sz = InstDefineMember::ISIZE; }
		XTAL_CASE(InstDefineMemberNS::NUMBER){ temp = ((InstDefineMemberNS*)pc)->inspect(code); sz = InstDefineMemberNS::ISIZE; }
		XTAL_CASE(InstGlobalVariable::NUMBER){ temp = ((InstGlobalVariable*)pc)->inspect(code); sz = InstGlobalVariable::ISIZE; }
		XTAL_CASE(InstSetGlobalVariable::NUMBER){ temp = ((InstSetGlobalVariable*)pc)->inspect(code); sz = InstSetGlobalVariable::ISIZE; }
		XTAL_CASE(InstDefineGlobalVariable::NUMBER){ temp = ((InstDefineGlobalVariable*)pc)->inspect(code); sz = InstDefineGlobalVariable::ISIZE; }
		XTAL_CASE(InstOnce::NUMBER){ temp = ((InstOnce*)pc)->inspect(code); sz = InstOnce::ISIZE; }
		XTAL_CASE(InstSetOnce::NUMBER){ temp = ((InstSetOnce*)pc)->inspect(code); sz = InstSetOnce::ISIZE; }
		XTAL_CASE(InstBlockBegin::NUMBER){ temp = ((InstBlockBegin*)pc)->inspect(code); sz = InstBlockBegin::ISIZE; }
		XTAL_CASE(InstBlockBeginDirect::NUMBER){ temp = ((InstBlockBeginDirect*)pc)->inspect(code); sz = InstBlockBeginDirect::ISIZE; }
		XTAL_CASE(InstBlockEnd::NUMBER){ temp = ((InstBlockEnd*)pc)->inspect(code); sz = InstBlockEnd::ISIZE; }
		XTAL_CASE(InstBlockEndDirect::NUMBER){ temp = ((InstBlockEndDirect*)pc)->inspect(code); sz = InstBlockEndDirect::ISIZE; }
		XTAL_CASE(InstTryBegin::NUMBER){ temp = ((InstTryBegin*)pc)->inspect(code); sz = InstTryBegin::ISIZE; }
		XTAL_CASE(InstTryEnd::NUMBER){ temp = ((InstTryEnd*)pc)->inspect(code); sz = InstTryEnd::ISIZE; }
		XTAL_CASE(InstPushGoto::NUMBER){ temp = ((InstPushGoto*)pc)->inspect(code); sz = InstPushGoto::ISIZE; }
		XTAL_CASE(InstPopGoto::NUMBER){ temp = ((InstPopGoto*)pc)->inspect(code); sz = InstPopGoto::ISIZE; }
		XTAL_CASE(InstIfEq::NUMBER){ temp = ((InstIfEq*)pc)->inspect(code); sz = InstIfEq::ISIZE; }
		XTAL_CASE(InstIfNe::NUMBER){ temp = ((InstIfNe*)pc)->inspect(code); sz = InstIfNe::ISIZE; }
		XTAL_CASE(InstIfLt::NUMBER){ temp = ((InstIfLt*)pc)->inspect(code); sz = InstIfLt::ISIZE; }
		XTAL_CASE(InstIfLe::NUMBER){ temp = ((InstIfLe*)pc)->inspect(code); sz = InstIfLe::ISIZE; }
		XTAL_CASE(InstIfGt::NUMBER){ temp = ((InstIfGt*)pc)->inspect(code); sz = InstIfGt::ISIZE; }
		XTAL_CASE(InstIfGe::NUMBER){ temp = ((InstIfGe*)pc)->inspect(code); sz = InstIfGe::ISIZE; }
		XTAL_CASE(InstIfRawEq::NUMBER){ temp = ((InstIfRawEq*)pc)->inspect(code); sz = InstIfRawEq::ISIZE; }
		XTAL_CASE(InstIfRawNe::NUMBER){ temp = ((InstIfRawNe*)pc)->inspect(code); sz = InstIfRawNe::ISIZE; }
		XTAL_CASE(InstIfIs::NUMBER){ temp = ((InstIfIs*)pc)->inspect(code); sz = InstIfIs::ISIZE; }
		XTAL_CASE(InstIfNis::NUMBER){ temp = ((InstIfNis*)pc)->inspect(code); sz = InstIfNis::ISIZE; }
		XTAL_CASE(InstIfArgIsNop::NUMBER){ temp = ((InstIfArgIsNop*)pc)->inspect(code); sz = InstIfArgIsNop::ISIZE; }
		XTAL_CASE(InstIfArgIsNopDirect::NUMBER){ temp = ((InstIfArgIsNopDirect*)pc)->inspect(code); sz = InstIfArgIsNopDirect::ISIZE; }
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
		XTAL_CASE(InstEq::NUMBER){ temp = ((InstEq*)pc)->inspect(code); sz = InstEq::ISIZE; }
		XTAL_CASE(InstNe::NUMBER){ temp = ((InstNe*)pc)->inspect(code); sz = InstNe::ISIZE; }
		XTAL_CASE(InstLt::NUMBER){ temp = ((InstLt*)pc)->inspect(code); sz = InstLt::ISIZE; }
		XTAL_CASE(InstLe::NUMBER){ temp = ((InstLe*)pc)->inspect(code); sz = InstLe::ISIZE; }
		XTAL_CASE(InstGt::NUMBER){ temp = ((InstGt*)pc)->inspect(code); sz = InstGt::ISIZE; }
		XTAL_CASE(InstGe::NUMBER){ temp = ((InstGe*)pc)->inspect(code); sz = InstGe::ISIZE; }
		XTAL_CASE(InstRawEq::NUMBER){ temp = ((InstRawEq*)pc)->inspect(code); sz = InstRawEq::ISIZE; }
		XTAL_CASE(InstRawNe::NUMBER){ temp = ((InstRawNe*)pc)->inspect(code); sz = InstRawNe::ISIZE; }
		XTAL_CASE(InstIs::NUMBER){ temp = ((InstIs*)pc)->inspect(code); sz = InstIs::ISIZE; }
		XTAL_CASE(InstNis::NUMBER){ temp = ((InstNis*)pc)->inspect(code); sz = InstNis::ISIZE; }
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
		XTAL_CASE(InstClassBegin::NUMBER){ temp = ((InstClassBegin*)pc)->inspect(code); sz = InstClassBegin::ISIZE; }
		XTAL_CASE(InstClassEnd::NUMBER){ temp = ((InstClassEnd*)pc)->inspect(code); sz = InstClassEnd::ISIZE; }
		XTAL_CASE(InstDefineClassMember::NUMBER){ temp = ((InstDefineClassMember*)pc)->inspect(code); sz = InstDefineClassMember::ISIZE; }
		XTAL_CASE(InstMakeRange::NUMBER){ temp = ((InstMakeRange*)pc)->inspect(code); sz = InstMakeRange::ISIZE; }
		XTAL_CASE(InstMakeArray::NUMBER){ temp = ((InstMakeArray*)pc)->inspect(code); sz = InstMakeArray::ISIZE; }
		XTAL_CASE(InstArrayAppend::NUMBER){ temp = ((InstArrayAppend*)pc)->inspect(code); sz = InstArrayAppend::ISIZE; }
		XTAL_CASE(InstMakeMap::NUMBER){ temp = ((InstMakeMap*)pc)->inspect(code); sz = InstMakeMap::ISIZE; }
		XTAL_CASE(InstMapInsert::NUMBER){ temp = ((InstMapInsert*)pc)->inspect(code); sz = InstMapInsert::ISIZE; }
		XTAL_CASE(InstMakeFun::NUMBER){ temp = ((InstMakeFun*)pc)->inspect(code); sz = InstMakeFun::ISIZE; }
		XTAL_CASE(InstMakeInstanceVariableAccessor::NUMBER){ temp = ((InstMakeInstanceVariableAccessor*)pc)->inspect(code); sz = InstMakeInstanceVariableAccessor::ISIZE; }
		XTAL_CASE(InstThrow::NUMBER){ temp = ((InstThrow*)pc)->inspect(code); sz = InstThrow::ISIZE; }
		XTAL_CASE(InstThrowUnsupportedError::NUMBER){ temp = ((InstThrowUnsupportedError*)pc)->inspect(code); sz = InstThrowUnsupportedError::ISIZE; }
		XTAL_CASE(InstThrowNop::NUMBER){ temp = ((InstThrowNop*)pc)->inspect(code); sz = InstThrowNop::ISIZE; }
		XTAL_CASE(InstAssert::NUMBER){ temp = ((InstAssert*)pc)->inspect(code); sz = InstAssert::ISIZE; }
		XTAL_CASE(InstBreakPoint::NUMBER){ temp = ((InstBreakPoint*)pc)->inspect(code); sz = InstBreakPoint::ISIZE; }
		XTAL_CASE(InstMAX::NUMBER){ temp = ((InstMAX*)pc)->inspect(code); sz = InstMAX::ISIZE; }
//}}CODE_INSPECT}

	} ms->put_s(Xf("%04d:%s\n")((int_t)(pc-data()), temp)->to_s()); pc += sz; }

	ms->seek(0);
	return ms->get_s(ms->size());
}

AnyPtr inst_inspect(inst_address_t value, Inst* inst, const CodePtr& code){
	return (int_t)(((inst_t*)inst - code->data()) + value);
}

}
