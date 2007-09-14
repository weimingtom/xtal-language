
#include "xtal.h"

#include "xtal_stream.h"
#include "xtal_macro.h"
#include "xtal_code.h"
#include "xtal_fun.h"

namespace xtal{

FunCore empty_xfun_core;
BlockCore empty_block_core;
ClassCore empty_class_core;
ExceptCore empty_except_core;

AnyPtr once_value_none_;

namespace{

void uninitialize_code(){
	once_value_none_ = null;
}

}

void initialize_code(){
	register_uninitializer(&uninitialize_code);

	{
		ClassPtr p = new_cpp_class<Code>("Code");
		p->inherit(get_cpp_class<Fun>());
		p->method("inspect", &Code::inspect);
	}

	builtin()->def("Code", get_cpp_class<Code>());

	once_value_none_ = xnew<Base>();
}


Code::Code()
	:filelocal_(xnew<Singleton>("filelocal")), source_file_name_("<noname>"){
	set_object_name("<filelocal>", 1, null);
	filelocal_->inherit(builtin());
	filelocal_->def("filelocal", filelocal_);

	symbol_table_ = xnew<Array>();
	value_table_ = xnew<Array>();

	first_fun_ = xnew<Fun>(null, null, CodePtr::from_this(this), (FunCore*)0);
	first_fun_->set_object_name("<toplevel>", 1, null);
}

void Code::reset_core(){
	first_fun_->set_core(&xfun_core_table_[0]);
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

	int sz = 0;
	const inst_t* pc = data();
	StringPtr temp;
	MemoryStreamPtr ms(xnew<MemoryStream>());

	for(; pc < data() + size();){switch(*pc){
		XTAL_NODEFAULT;

//{CODE_INSPECT{{
		XTAL_CASE(InstNop::NUMBER){ temp = ((InstNop*)pc)->inspect(); sz = InstNop::ISIZE; }
		XTAL_CASE(InstPushNull::NUMBER){ temp = ((InstPushNull*)pc)->inspect(); sz = InstPushNull::ISIZE; }
		XTAL_CASE(InstPushTrue::NUMBER){ temp = ((InstPushTrue*)pc)->inspect(); sz = InstPushTrue::ISIZE; }
		XTAL_CASE(InstPushFalse::NUMBER){ temp = ((InstPushFalse*)pc)->inspect(); sz = InstPushFalse::ISIZE; }
		XTAL_CASE(InstPushInt1Byte::NUMBER){ temp = ((InstPushInt1Byte*)pc)->inspect(); sz = InstPushInt1Byte::ISIZE; }
		XTAL_CASE(InstPushInt2Byte::NUMBER){ temp = ((InstPushInt2Byte*)pc)->inspect(); sz = InstPushInt2Byte::ISIZE; }
		XTAL_CASE(InstPushFloat1Byte::NUMBER){ temp = ((InstPushFloat1Byte*)pc)->inspect(); sz = InstPushFloat1Byte::ISIZE; }
		XTAL_CASE(InstPushFloat2Byte::NUMBER){ temp = ((InstPushFloat2Byte*)pc)->inspect(); sz = InstPushFloat2Byte::ISIZE; }
		XTAL_CASE(InstPushCallee::NUMBER){ temp = ((InstPushCallee*)pc)->inspect(); sz = InstPushCallee::ISIZE; }
		XTAL_CASE(InstPushArgs::NUMBER){ temp = ((InstPushArgs*)pc)->inspect(); sz = InstPushArgs::ISIZE; }
		XTAL_CASE(InstPushThis::NUMBER){ temp = ((InstPushThis*)pc)->inspect(); sz = InstPushThis::ISIZE; }
		XTAL_CASE(InstPushCurrentContext::NUMBER){ temp = ((InstPushCurrentContext*)pc)->inspect(); sz = InstPushCurrentContext::ISIZE; }
		XTAL_CASE(InstPop::NUMBER){ temp = ((InstPop*)pc)->inspect(); sz = InstPop::ISIZE; }
		XTAL_CASE(InstDup::NUMBER){ temp = ((InstDup*)pc)->inspect(); sz = InstDup::ISIZE; }
		XTAL_CASE(InstInsert1::NUMBER){ temp = ((InstInsert1*)pc)->inspect(); sz = InstInsert1::ISIZE; }
		XTAL_CASE(InstInsert2::NUMBER){ temp = ((InstInsert2*)pc)->inspect(); sz = InstInsert2::ISIZE; }
		XTAL_CASE(InstInsert3::NUMBER){ temp = ((InstInsert3*)pc)->inspect(); sz = InstInsert3::ISIZE; }
		XTAL_CASE(InstAdjustResult::NUMBER){ temp = ((InstAdjustResult*)pc)->inspect(); sz = InstAdjustResult::ISIZE; }
		XTAL_CASE(InstIf::NUMBER){ temp = ((InstIf*)pc)->inspect(); sz = InstIf::ISIZE; }
		XTAL_CASE(InstUnless::NUMBER){ temp = ((InstUnless*)pc)->inspect(); sz = InstUnless::ISIZE; }
		XTAL_CASE(InstGoto::NUMBER){ temp = ((InstGoto*)pc)->inspect(); sz = InstGoto::ISIZE; }
		XTAL_CASE(InstLocalVariableInc::NUMBER){ temp = ((InstLocalVariableInc*)pc)->inspect(); sz = InstLocalVariableInc::ISIZE; }
		XTAL_CASE(InstLocalVariableIncDirect::NUMBER){ temp = ((InstLocalVariableIncDirect*)pc)->inspect(); sz = InstLocalVariableIncDirect::ISIZE; }
		XTAL_CASE(InstLocalVariableDec::NUMBER){ temp = ((InstLocalVariableDec*)pc)->inspect(); sz = InstLocalVariableDec::ISIZE; }
		XTAL_CASE(InstLocalVariableDecDirect::NUMBER){ temp = ((InstLocalVariableDecDirect*)pc)->inspect(); sz = InstLocalVariableDecDirect::ISIZE; }
		XTAL_CASE(InstLocalVariableInc2Byte::NUMBER){ temp = ((InstLocalVariableInc2Byte*)pc)->inspect(); sz = InstLocalVariableInc2Byte::ISIZE; }
		XTAL_CASE(InstLocalVariableDec2Byte::NUMBER){ temp = ((InstLocalVariableDec2Byte*)pc)->inspect(); sz = InstLocalVariableDec2Byte::ISIZE; }
		XTAL_CASE(InstLocalVariable1Byte::NUMBER){ temp = ((InstLocalVariable1Byte*)pc)->inspect(); sz = InstLocalVariable1Byte::ISIZE; }
		XTAL_CASE(InstLocalVariable1ByteDirect::NUMBER){ temp = ((InstLocalVariable1ByteDirect*)pc)->inspect(); sz = InstLocalVariable1ByteDirect::ISIZE; }
		XTAL_CASE(InstLocalVariable2Byte::NUMBER){ temp = ((InstLocalVariable2Byte*)pc)->inspect(); sz = InstLocalVariable2Byte::ISIZE; }
		XTAL_CASE(InstSetLocalVariable1Byte::NUMBER){ temp = ((InstSetLocalVariable1Byte*)pc)->inspect(); sz = InstSetLocalVariable1Byte::ISIZE; }
		XTAL_CASE(InstSetLocalVariable1ByteDirect::NUMBER){ temp = ((InstSetLocalVariable1ByteDirect*)pc)->inspect(); sz = InstSetLocalVariable1ByteDirect::ISIZE; }
		XTAL_CASE(InstSetLocalVariable2Byte::NUMBER){ temp = ((InstSetLocalVariable2Byte*)pc)->inspect(); sz = InstSetLocalVariable2Byte::ISIZE; }
		XTAL_CASE(InstInstanceVariable::NUMBER){ temp = ((InstInstanceVariable*)pc)->inspect(); sz = InstInstanceVariable::ISIZE; }
		XTAL_CASE(InstSetInstanceVariable::NUMBER){ temp = ((InstSetInstanceVariable*)pc)->inspect(); sz = InstSetInstanceVariable::ISIZE; }
		XTAL_CASE(InstCleanupCall::NUMBER){ temp = ((InstCleanupCall*)pc)->inspect(); sz = InstCleanupCall::ISIZE; }
		XTAL_CASE(InstReturn0::NUMBER){ temp = ((InstReturn0*)pc)->inspect(); sz = InstReturn0::ISIZE; }
		XTAL_CASE(InstReturn1::NUMBER){ temp = ((InstReturn1*)pc)->inspect(); sz = InstReturn1::ISIZE; }
		XTAL_CASE(InstReturn2::NUMBER){ temp = ((InstReturn2*)pc)->inspect(); sz = InstReturn2::ISIZE; }
		XTAL_CASE(InstReturn::NUMBER){ temp = ((InstReturn*)pc)->inspect(); sz = InstReturn::ISIZE; }
		XTAL_CASE(InstYield::NUMBER){ temp = ((InstYield*)pc)->inspect(); sz = InstYield::ISIZE; }
		XTAL_CASE(InstExit::NUMBER){ temp = ((InstExit*)pc)->inspect(); sz = InstExit::ISIZE; }
		XTAL_CASE(InstValue::NUMBER){ temp = ((InstValue*)pc)->inspect(); sz = InstValue::ISIZE; }
		XTAL_CASE(InstCheckUnsupported::NUMBER){ temp = ((InstCheckUnsupported*)pc)->inspect(); sz = InstCheckUnsupported::ISIZE; }
		XTAL_CASE(InstSend::NUMBER){ temp = ((InstSend*)pc)->inspect(); sz = InstSend::ISIZE; }
		XTAL_CASE(InstSendIfDefined::NUMBER){ temp = ((InstSendIfDefined*)pc)->inspect(); sz = InstSendIfDefined::ISIZE; }
		XTAL_CASE(InstCall::NUMBER){ temp = ((InstCall*)pc)->inspect(); sz = InstCall::ISIZE; }
		XTAL_CASE(InstCallCallee::NUMBER){ temp = ((InstCallCallee*)pc)->inspect(); sz = InstCallCallee::ISIZE; }
		XTAL_CASE(InstSend_A::NUMBER){ temp = ((InstSend_A*)pc)->inspect(); sz = InstSend_A::ISIZE; }
		XTAL_CASE(InstSendIfDefined_A::NUMBER){ temp = ((InstSendIfDefined_A*)pc)->inspect(); sz = InstSendIfDefined_A::ISIZE; }
		XTAL_CASE(InstCall_A::NUMBER){ temp = ((InstCall_A*)pc)->inspect(); sz = InstCall_A::ISIZE; }
		XTAL_CASE(InstCallCallee_A::NUMBER){ temp = ((InstCallCallee_A*)pc)->inspect(); sz = InstCallCallee_A::ISIZE; }
		XTAL_CASE(InstSend_T::NUMBER){ temp = ((InstSend_T*)pc)->inspect(); sz = InstSend_T::ISIZE; }
		XTAL_CASE(InstSendIfDefined_T::NUMBER){ temp = ((InstSendIfDefined_T*)pc)->inspect(); sz = InstSendIfDefined_T::ISIZE; }
		XTAL_CASE(InstCall_T::NUMBER){ temp = ((InstCall_T*)pc)->inspect(); sz = InstCall_T::ISIZE; }
		XTAL_CASE(InstCallCallee_T::NUMBER){ temp = ((InstCallCallee_T*)pc)->inspect(); sz = InstCallCallee_T::ISIZE; }
		XTAL_CASE(InstSend_AT::NUMBER){ temp = ((InstSend_AT*)pc)->inspect(); sz = InstSend_AT::ISIZE; }
		XTAL_CASE(InstSendIfDefined_AT::NUMBER){ temp = ((InstSendIfDefined_AT*)pc)->inspect(); sz = InstSendIfDefined_AT::ISIZE; }
		XTAL_CASE(InstCall_AT::NUMBER){ temp = ((InstCall_AT*)pc)->inspect(); sz = InstCall_AT::ISIZE; }
		XTAL_CASE(InstCallCallee_AT::NUMBER){ temp = ((InstCallCallee_AT*)pc)->inspect(); sz = InstCallCallee_AT::ISIZE; }
		XTAL_CASE(InstBlockBegin::NUMBER){ temp = ((InstBlockBegin*)pc)->inspect(); sz = InstBlockBegin::ISIZE; }
		XTAL_CASE(InstBlockBeginDirect::NUMBER){ temp = ((InstBlockBeginDirect*)pc)->inspect(); sz = InstBlockBeginDirect::ISIZE; }
		XTAL_CASE(InstBlockEnd::NUMBER){ temp = ((InstBlockEnd*)pc)->inspect(); sz = InstBlockEnd::ISIZE; }
		XTAL_CASE(InstBlockEndDirect::NUMBER){ temp = ((InstBlockEndDirect*)pc)->inspect(); sz = InstBlockEndDirect::ISIZE; }
		XTAL_CASE(InstTryBegin::NUMBER){ temp = ((InstTryBegin*)pc)->inspect(); sz = InstTryBegin::ISIZE; }
		XTAL_CASE(InstTryEnd::NUMBER){ temp = ((InstTryEnd*)pc)->inspect(); sz = InstTryEnd::ISIZE; }
		XTAL_CASE(InstPushGoto::NUMBER){ temp = ((InstPushGoto*)pc)->inspect(); sz = InstPushGoto::ISIZE; }
		XTAL_CASE(InstPopGoto::NUMBER){ temp = ((InstPopGoto*)pc)->inspect(); sz = InstPopGoto::ISIZE; }
		XTAL_CASE(InstIfEq::NUMBER){ temp = ((InstIfEq*)pc)->inspect(); sz = InstIfEq::ISIZE; }
		XTAL_CASE(InstIfNe::NUMBER){ temp = ((InstIfNe*)pc)->inspect(); sz = InstIfNe::ISIZE; }
		XTAL_CASE(InstIfLt::NUMBER){ temp = ((InstIfLt*)pc)->inspect(); sz = InstIfLt::ISIZE; }
		XTAL_CASE(InstIfLe::NUMBER){ temp = ((InstIfLe*)pc)->inspect(); sz = InstIfLe::ISIZE; }
		XTAL_CASE(InstIfGt::NUMBER){ temp = ((InstIfGt*)pc)->inspect(); sz = InstIfGt::ISIZE; }
		XTAL_CASE(InstIfGe::NUMBER){ temp = ((InstIfGe*)pc)->inspect(); sz = InstIfGe::ISIZE; }
		XTAL_CASE(InstIfRawEq::NUMBER){ temp = ((InstIfRawEq*)pc)->inspect(); sz = InstIfRawEq::ISIZE; }
		XTAL_CASE(InstIfRawNe::NUMBER){ temp = ((InstIfRawNe*)pc)->inspect(); sz = InstIfRawNe::ISIZE; }
		XTAL_CASE(InstIfIs::NUMBER){ temp = ((InstIfIs*)pc)->inspect(); sz = InstIfIs::ISIZE; }
		XTAL_CASE(InstIfNis::NUMBER){ temp = ((InstIfNis*)pc)->inspect(); sz = InstIfNis::ISIZE; }
		XTAL_CASE(InstIfArgIsNull::NUMBER){ temp = ((InstIfArgIsNull*)pc)->inspect(); sz = InstIfArgIsNull::ISIZE; }
		XTAL_CASE(InstIfArgIsNullDirect::NUMBER){ temp = ((InstIfArgIsNullDirect*)pc)->inspect(); sz = InstIfArgIsNullDirect::ISIZE; }
		XTAL_CASE(InstPos::NUMBER){ temp = ((InstPos*)pc)->inspect(); sz = InstPos::ISIZE; }
		XTAL_CASE(InstNeg::NUMBER){ temp = ((InstNeg*)pc)->inspect(); sz = InstNeg::ISIZE; }
		XTAL_CASE(InstCom::NUMBER){ temp = ((InstCom*)pc)->inspect(); sz = InstCom::ISIZE; }
		XTAL_CASE(InstNot::NUMBER){ temp = ((InstNot*)pc)->inspect(); sz = InstNot::ISIZE; }
		XTAL_CASE(InstAt::NUMBER){ temp = ((InstAt*)pc)->inspect(); sz = InstAt::ISIZE; }
		XTAL_CASE(InstSetAt::NUMBER){ temp = ((InstSetAt*)pc)->inspect(); sz = InstSetAt::ISIZE; }
		XTAL_CASE(InstAdd::NUMBER){ temp = ((InstAdd*)pc)->inspect(); sz = InstAdd::ISIZE; }
		XTAL_CASE(InstSub::NUMBER){ temp = ((InstSub*)pc)->inspect(); sz = InstSub::ISIZE; }
		XTAL_CASE(InstCat::NUMBER){ temp = ((InstCat*)pc)->inspect(); sz = InstCat::ISIZE; }
		XTAL_CASE(InstMul::NUMBER){ temp = ((InstMul*)pc)->inspect(); sz = InstMul::ISIZE; }
		XTAL_CASE(InstDiv::NUMBER){ temp = ((InstDiv*)pc)->inspect(); sz = InstDiv::ISIZE; }
		XTAL_CASE(InstMod::NUMBER){ temp = ((InstMod*)pc)->inspect(); sz = InstMod::ISIZE; }
		XTAL_CASE(InstAnd::NUMBER){ temp = ((InstAnd*)pc)->inspect(); sz = InstAnd::ISIZE; }
		XTAL_CASE(InstOr::NUMBER){ temp = ((InstOr*)pc)->inspect(); sz = InstOr::ISIZE; }
		XTAL_CASE(InstXor::NUMBER){ temp = ((InstXor*)pc)->inspect(); sz = InstXor::ISIZE; }
		XTAL_CASE(InstShl::NUMBER){ temp = ((InstShl*)pc)->inspect(); sz = InstShl::ISIZE; }
		XTAL_CASE(InstShr::NUMBER){ temp = ((InstShr*)pc)->inspect(); sz = InstShr::ISIZE; }
		XTAL_CASE(InstUshr::NUMBER){ temp = ((InstUshr*)pc)->inspect(); sz = InstUshr::ISIZE; }
		XTAL_CASE(InstEq::NUMBER){ temp = ((InstEq*)pc)->inspect(); sz = InstEq::ISIZE; }
		XTAL_CASE(InstNe::NUMBER){ temp = ((InstNe*)pc)->inspect(); sz = InstNe::ISIZE; }
		XTAL_CASE(InstLt::NUMBER){ temp = ((InstLt*)pc)->inspect(); sz = InstLt::ISIZE; }
		XTAL_CASE(InstLe::NUMBER){ temp = ((InstLe*)pc)->inspect(); sz = InstLe::ISIZE; }
		XTAL_CASE(InstGt::NUMBER){ temp = ((InstGt*)pc)->inspect(); sz = InstGt::ISIZE; }
		XTAL_CASE(InstGe::NUMBER){ temp = ((InstGe*)pc)->inspect(); sz = InstGe::ISIZE; }
		XTAL_CASE(InstRawEq::NUMBER){ temp = ((InstRawEq*)pc)->inspect(); sz = InstRawEq::ISIZE; }
		XTAL_CASE(InstRawNe::NUMBER){ temp = ((InstRawNe*)pc)->inspect(); sz = InstRawNe::ISIZE; }
		XTAL_CASE(InstIs::NUMBER){ temp = ((InstIs*)pc)->inspect(); sz = InstIs::ISIZE; }
		XTAL_CASE(InstNis::NUMBER){ temp = ((InstNis*)pc)->inspect(); sz = InstNis::ISIZE; }
		XTAL_CASE(InstInc::NUMBER){ temp = ((InstInc*)pc)->inspect(); sz = InstInc::ISIZE; }
		XTAL_CASE(InstDec::NUMBER){ temp = ((InstDec*)pc)->inspect(); sz = InstDec::ISIZE; }
		XTAL_CASE(InstAddAssign::NUMBER){ temp = ((InstAddAssign*)pc)->inspect(); sz = InstAddAssign::ISIZE; }
		XTAL_CASE(InstSubAssign::NUMBER){ temp = ((InstSubAssign*)pc)->inspect(); sz = InstSubAssign::ISIZE; }
		XTAL_CASE(InstCatAssign::NUMBER){ temp = ((InstCatAssign*)pc)->inspect(); sz = InstCatAssign::ISIZE; }
		XTAL_CASE(InstMulAssign::NUMBER){ temp = ((InstMulAssign*)pc)->inspect(); sz = InstMulAssign::ISIZE; }
		XTAL_CASE(InstDivAssign::NUMBER){ temp = ((InstDivAssign*)pc)->inspect(); sz = InstDivAssign::ISIZE; }
		XTAL_CASE(InstModAssign::NUMBER){ temp = ((InstModAssign*)pc)->inspect(); sz = InstModAssign::ISIZE; }
		XTAL_CASE(InstAndAssign::NUMBER){ temp = ((InstAndAssign*)pc)->inspect(); sz = InstAndAssign::ISIZE; }
		XTAL_CASE(InstOrAssign::NUMBER){ temp = ((InstOrAssign*)pc)->inspect(); sz = InstOrAssign::ISIZE; }
		XTAL_CASE(InstXorAssign::NUMBER){ temp = ((InstXorAssign*)pc)->inspect(); sz = InstXorAssign::ISIZE; }
		XTAL_CASE(InstShlAssign::NUMBER){ temp = ((InstShlAssign*)pc)->inspect(); sz = InstShlAssign::ISIZE; }
		XTAL_CASE(InstShrAssign::NUMBER){ temp = ((InstShrAssign*)pc)->inspect(); sz = InstShrAssign::ISIZE; }
		XTAL_CASE(InstUshrAssign::NUMBER){ temp = ((InstUshrAssign*)pc)->inspect(); sz = InstUshrAssign::ISIZE; }
		XTAL_CASE(InstGlobalVariable::NUMBER){ temp = ((InstGlobalVariable*)pc)->inspect(); sz = InstGlobalVariable::ISIZE; }
		XTAL_CASE(InstSetGlobalVariable::NUMBER){ temp = ((InstSetGlobalVariable*)pc)->inspect(); sz = InstSetGlobalVariable::ISIZE; }
		XTAL_CASE(InstDefineGlobalVariable::NUMBER){ temp = ((InstDefineGlobalVariable*)pc)->inspect(); sz = InstDefineGlobalVariable::ISIZE; }
		XTAL_CASE(InstMember::NUMBER){ temp = ((InstMember*)pc)->inspect(); sz = InstMember::ISIZE; }
		XTAL_CASE(InstMemberIfDefined::NUMBER){ temp = ((InstMemberIfDefined*)pc)->inspect(); sz = InstMemberIfDefined::ISIZE; }
		XTAL_CASE(InstDefineMember::NUMBER){ temp = ((InstDefineMember*)pc)->inspect(); sz = InstDefineMember::ISIZE; }
		XTAL_CASE(InstDefineClassMember::NUMBER){ temp = ((InstDefineClassMember*)pc)->inspect(); sz = InstDefineClassMember::ISIZE; }
		XTAL_CASE(InstSetName::NUMBER){ temp = ((InstSetName*)pc)->inspect(); sz = InstSetName::ISIZE; }
		XTAL_CASE(InstOnce::NUMBER){ temp = ((InstOnce*)pc)->inspect(); sz = InstOnce::ISIZE; }
		XTAL_CASE(InstSetOnce::NUMBER){ temp = ((InstSetOnce*)pc)->inspect(); sz = InstSetOnce::ISIZE; }
		XTAL_CASE(InstClassBegin::NUMBER){ temp = ((InstClassBegin*)pc)->inspect(); sz = InstClassBegin::ISIZE; }
		XTAL_CASE(InstClassEnd::NUMBER){ temp = ((InstClassEnd*)pc)->inspect(); sz = InstClassEnd::ISIZE; }
		XTAL_CASE(InstMakeArray::NUMBER){ temp = ((InstMakeArray*)pc)->inspect(); sz = InstMakeArray::ISIZE; }
		XTAL_CASE(InstArrayAppend::NUMBER){ temp = ((InstArrayAppend*)pc)->inspect(); sz = InstArrayAppend::ISIZE; }
		XTAL_CASE(InstMakeMap::NUMBER){ temp = ((InstMakeMap*)pc)->inspect(); sz = InstMakeMap::ISIZE; }
		XTAL_CASE(InstMapInsert::NUMBER){ temp = ((InstMapInsert*)pc)->inspect(); sz = InstMapInsert::ISIZE; }
		XTAL_CASE(InstMakeFun::NUMBER){ temp = ((InstMakeFun*)pc)->inspect(); sz = InstMakeFun::ISIZE; }
		XTAL_CASE(InstMakeInstanceVariableAccessor::NUMBER){ temp = ((InstMakeInstanceVariableAccessor*)pc)->inspect(); sz = InstMakeInstanceVariableAccessor::ISIZE; }
		XTAL_CASE(InstThrow::NUMBER){ temp = ((InstThrow*)pc)->inspect(); sz = InstThrow::ISIZE; }
		XTAL_CASE(InstThrowUnsupportedError::NUMBER){ temp = ((InstThrowUnsupportedError*)pc)->inspect(); sz = InstThrowUnsupportedError::ISIZE; }
		XTAL_CASE(InstThrowNull::NUMBER){ temp = ((InstThrowNull*)pc)->inspect(); sz = InstThrowNull::ISIZE; }
		XTAL_CASE(InstAssert::NUMBER){ temp = ((InstAssert*)pc)->inspect(); sz = InstAssert::ISIZE; }
		XTAL_CASE(InstBreakPoint::NUMBER){ temp = ((InstBreakPoint*)pc)->inspect(); sz = InstBreakPoint::ISIZE; }
		XTAL_CASE(InstMAX::NUMBER){ temp = ((InstMAX*)pc)->inspect(); sz = InstMAX::ISIZE; }
//}}CODE_INSPECT}

	} ms->put_s(Xt("%04d:%s\n")((int_t)(pc-data()), temp)->to_s()); pc += sz; }

	ms->seek(0);
	return ms->get_s(ms->size());
}

}
