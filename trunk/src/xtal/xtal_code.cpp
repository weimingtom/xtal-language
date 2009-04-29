#include "xtal.h"
#include "xtal_bind.h"
#include "xtal_macro.h"

namespace xtal{

namespace{

void filelocal_check_implicit_lookup(const AnyPtr& a){
	if(CodePtr code = ptr_cast<Code>(a->member(Xid(code)))){
		code->check_implicit_lookup();
	}
}

}

Code::Code()
	:filelocal_(xnew<Singleton>()), source_file_name_(XTAL_STRING("<noname>")){
	filelocal_->set_object_name(Xid(filelocal));
	filelocal_->set_object_force(500);
	filelocal_->inherit(builtin());
	filelocal_->def(Xid(filelocal), filelocal_);
	filelocal_->def(Xid(code), to_smartptr(this));
	filelocal_->def_method(Xid(check_implicit_lookup), &filelocal_check_implicit_lookup);

	identifier_table_ = xnew<Array>();
	value_table_ = xnew<Array>();

	first_fun_ = xnew<Method>(null, to_smartptr(this), (FunInfo*)0);
}

void Code::set_lineno_info(uint_t line){
	if(!lineno_table_.empty() && lineno_table_.back().lineno==line)
		return;
	LineNumberInfo lnt={(u16)code_.size(), (u16)line};
	lineno_table_.push_back(lnt);
}

int_t Code::compliant_lineno(const inst_t* p){
	if(!lineno_table_.empty()){
		LineNumberInfo* begin = &lineno_table_[0];
		LineNumberInfo* end = begin+lineno_table_.size();
		LineNumberInfo* it=
			std::lower_bound(
				begin,
				end,
				static_cast<int_t>(p-data()),
				LineNumberCmp()
			);

		if(it!=end){
			if(it==begin){
				return 1;
			}
			--it;
			return it->lineno;
		}
	}
	return 0;
}

void Code::rawcall(const VMachinePtr& vm){
	vm->set_arg_this(filelocal_);
	first_fun_->rawcall(vm);
}

void Code::insert_code(inst_t* p, inst_t* code, int_t size){
	insert_erase_common(p, size);
	code_.insert(p-&code_[0], code, size);
}

void Code::erase_code(inst_t* p, int_t size){
	insert_erase_common(p, -size);
	code_.erase(p-&code_[0], size);
}

bool Code::add_break_point(uint_t lineno){
	for(uint_t i=0; i<lineno_table_.size(); ++i){
		if(lineno_table_[i].lineno==lineno){
			if(code_[lineno_table_[i].start_pc]!=InstBreakPoint::NUMBER){
				InstBreakPoint break_point;
				insert_code(&code_[lineno_table_[i].start_pc], (inst_t*)&break_point, sizeof(InstBreakPoint));
			}
			return true;
		}
	}
	return false;
}

void Code::remove_break_point(uint_t lineno){
	for(uint_t i=0; i<lineno_table_.size(); ++i){
		if(lineno_table_[i].lineno==lineno){
			if(code_[lineno_table_[i].start_pc]==InstBreakPoint::NUMBER){
				erase_code(&code_[lineno_table_[i].start_pc], sizeof(InstBreakPoint));
			}
			return;
		}
	}
}

void Code::insert_erase_common(inst_t* p, int_t size){
	uint_t pos = p - &code_[0];
	for(uint_t i=0; i<address_jump_table_.size(); ++i){
		uint_t start = address_jump_table_[i].pos;
		inst_address_t& address = *(inst_address_t*)&code_[start];
		uint_t end = start + address;

		if(start<end){
			if(start<pos && pos<=end){
				address = address + size;
			}
		}
		else{
			if(end<=pos && pos<start){
				address = address - size;
			}
		}

		if(start>=pos){
			address_jump_table_[i].pos += size;
		}
	}

	for(uint_t i=0; i<lineno_table_.size(); ++i){
		if(lineno_table_[i].start_pc>pos){
			lineno_table_[i].start_pc += size;
		}
	}

	for(uint_t i=0; i<xfun_info_table_.size(); ++i){
		if(xfun_info_table_[i].pc>pos){
			xfun_info_table_[i].pc += size;
		}
	}

	for(uint_t i=0; i<scope_info_table_.size(); ++i){
		if(scope_info_table_[i].pc>pos){
			scope_info_table_[i].pc += size;
		}
	}

	for(uint_t i=0; i<class_info_table_.size(); ++i){
		if(class_info_table_[i].pc>pos){
			class_info_table_[i].pc += size;
		}
	}

	for(uint_t i=0; i<except_info_table_.size(); ++i){
		if(except_info_table_[i].catch_pc>pos){
			except_info_table_[i].catch_pc += size;
		}

		if(except_info_table_[i].finally_pc>pos){
			except_info_table_[i].finally_pc += size;
		}

		if(except_info_table_[i].end_pc>pos){
			except_info_table_[i].end_pc += size;
		}
	}
}
	
void Code::check_implicit_lookup(){
	ArrayPtr ary;
	for(uint_t i=0; i<implicit_table_.size(); ++i){
		IDPtr id = unchecked_ptr_cast<ID>(identifier_table_->at(implicit_table_[i].id));
		AnyPtr ret = filelocal_->member(id);
		if(raweq(undefined, ret)){
			if(!ary){
				ary = xnew<Array>();
			}

			ary->push_back(Xf("%s(%d)")->call(unsupported_error(filelocal_, id, undefined), implicit_table_[i].lineno));
		}
	}

	if(ary){
		XTAL_SET_EXCEPT(cpp_class<RuntimeError>()->call(Xt("Xtal Runtime Error 1030")->call(Named(Xid(name), ary))));
	}
}


StringPtr Code::inspect(){
	MemoryStreamPtr ms(xnew<MemoryStream>());

	ms->put_s("identifier_table\n");
	for(uint_t i=0; i<identifier_table_->size(); ++i){
		ms->put_s(Xf("\t%04d:%s\n")->call(i, identifier_table_->at(i))->to_s());
	}

	ms->put_s("value_table\n");
	for(uint_t i=0; i<value_table_->size(); ++i){
		ms->put_s(Xf("\t%04d:%s\n")->call(i, value_table_->at(i))->to_s());
	}

	ms->put_s("\n");
	ms->put_s(inspect_range(0, size()));

	ms->seek(0);
	return ms->get_s(ms->size());
}

StringPtr Code::inspect_range(int_t start, int_t end){

#ifdef XTAL_DEBUG

	int sz = 0;
	const inst_t* pc = data()+start;
	StringPtr temp;
	MemoryStreamPtr ms(xnew<MemoryStream>());
	CodePtr code(to_smartptr(this));

	for(; pc < data() + end;){switch(*pc){
		XTAL_NODEFAULT;
//{CODE_INSPECT{{
		XTAL_CASE(InstNop::NUMBER){ temp = ((InstNop*)pc)->inspect(code); sz = InstNop::ISIZE; }
		XTAL_CASE(InstPushNull::NUMBER){ temp = ((InstPushNull*)pc)->inspect(code); sz = InstPushNull::ISIZE; }
		XTAL_CASE(InstPushUndefined::NUMBER){ temp = ((InstPushUndefined*)pc)->inspect(code); sz = InstPushUndefined::ISIZE; }
		XTAL_CASE(InstPushTrue::NUMBER){ temp = ((InstPushTrue*)pc)->inspect(code); sz = InstPushTrue::ISIZE; }
		XTAL_CASE(InstPushFalse::NUMBER){ temp = ((InstPushFalse*)pc)->inspect(code); sz = InstPushFalse::ISIZE; }
		XTAL_CASE(InstPushTrueAndSkip::NUMBER){ temp = ((InstPushTrueAndSkip*)pc)->inspect(code); sz = InstPushTrueAndSkip::ISIZE; }
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
		XTAL_CASE(InstLocalVariableInc::NUMBER){ temp = ((InstLocalVariableInc*)pc)->inspect(code); sz = InstLocalVariableInc::ISIZE; }
		XTAL_CASE(InstLocalVariableDec::NUMBER){ temp = ((InstLocalVariableDec*)pc)->inspect(code); sz = InstLocalVariableDec::ISIZE; }
		XTAL_CASE(InstLocalVariableInc2Byte::NUMBER){ temp = ((InstLocalVariableInc2Byte*)pc)->inspect(code); sz = InstLocalVariableInc2Byte::ISIZE; }
		XTAL_CASE(InstLocalVariableDec2Byte::NUMBER){ temp = ((InstLocalVariableDec2Byte*)pc)->inspect(code); sz = InstLocalVariableDec2Byte::ISIZE; }
		XTAL_CASE(InstLocalVariable1Byte::NUMBER){ temp = ((InstLocalVariable1Byte*)pc)->inspect(code); sz = InstLocalVariable1Byte::ISIZE; }
		XTAL_CASE(InstLocalVariable1ByteX2::NUMBER){ temp = ((InstLocalVariable1ByteX2*)pc)->inspect(code); sz = InstLocalVariable1ByteX2::ISIZE; }
		XTAL_CASE(InstLocalVariable1ByteX3::NUMBER){ temp = ((InstLocalVariable1ByteX3*)pc)->inspect(code); sz = InstLocalVariable1ByteX3::ISIZE; }
		XTAL_CASE(InstLocalVariable2Byte::NUMBER){ temp = ((InstLocalVariable2Byte*)pc)->inspect(code); sz = InstLocalVariable2Byte::ISIZE; }
		XTAL_CASE(InstSetLocalVariable1Byte::NUMBER){ temp = ((InstSetLocalVariable1Byte*)pc)->inspect(code); sz = InstSetLocalVariable1Byte::ISIZE; }
		XTAL_CASE(InstSetLocalVariable2Byte::NUMBER){ temp = ((InstSetLocalVariable2Byte*)pc)->inspect(code); sz = InstSetLocalVariable2Byte::ISIZE; }
		XTAL_CASE(InstInstanceVariable::NUMBER){ temp = ((InstInstanceVariable*)pc)->inspect(code); sz = InstInstanceVariable::ISIZE; }
		XTAL_CASE(InstSetInstanceVariable::NUMBER){ temp = ((InstSetInstanceVariable*)pc)->inspect(code); sz = InstSetInstanceVariable::ISIZE; }
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
		XTAL_CASE(InstBlockBegin::NUMBER){ temp = ((InstBlockBegin*)pc)->inspect(code); sz = InstBlockBegin::ISIZE; }
		XTAL_CASE(InstBlockEnd::NUMBER){ temp = ((InstBlockEnd*)pc)->inspect(code); sz = InstBlockEnd::ISIZE; }
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
		XTAL_CASE(InstGlobalVariable::NUMBER){ temp = ((InstGlobalVariable*)pc)->inspect(code); sz = InstGlobalVariable::ISIZE; }
		XTAL_CASE(InstSetGlobalVariable::NUMBER){ temp = ((InstSetGlobalVariable*)pc)->inspect(code); sz = InstSetGlobalVariable::ISIZE; }
		XTAL_CASE(InstDefineGlobalVariable::NUMBER){ temp = ((InstDefineGlobalVariable*)pc)->inspect(code); sz = InstDefineGlobalVariable::ISIZE; }
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
//}}CODE_INSPECT}
	} ms->put_s(Xf("%04d:%s\n")->call((int_t)(pc-data()), temp)->to_s()); pc += sz; }

	ms->seek(0);
	return ms->get_s(ms->size());

#else

	return "no debug info";

#endif
}

}
