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
	:filelocal_(xnew<Class>()), source_file_name_(XTAL_STRING("<noname>")){
	filelocal_->set_singleton();
	filelocal_->set_object_temporary_name(Xid(filelocal));
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

void Code::find_near_variable_inner(const IDPtr& primary_key, const ScopeInfo& info, IDPtr& pick, int_t& minv){
	for(uint_t j=0; j<info.variable_size; ++j){
		IDPtr id = identifier(info.variable_identifier_offset+j);
		int_t dist = edit_distance(primary_key, id);
		if(dist!=0 && dist<minv){
			minv = dist;
			pick = id;
		}
	}
}
	
IDPtr Code::find_near_variable(const IDPtr& primary_key){
	int_t minv = 0xffffff;
	IDPtr pick;

	if(const ClassPtr& klass = filelocal()){
		pick = klass->find_near_member(primary_key, undefined, minv);
	}

	for(uint_t i=0; i<xfun_info_table_.size(); ++i){
		find_near_variable_inner(primary_key, xfun_info_table_[i], pick, minv);
	}

	for(uint_t i=0; i<scope_info_table_.size(); ++i){
		find_near_variable_inner(primary_key, scope_info_table_[i], pick, minv);
	}

	for(uint_t i=0; i<class_info_table_.size(); ++i){
		find_near_variable_inner(primary_key, class_info_table_[i], pick, minv);
	}

	return pick;
}

void Code::check_implicit_lookup(){
	ArrayPtr ary;
	for(uint_t i=0; i<implicit_table_.size(); ++i){
		const IDPtr& id = unchecked_ptr_cast<ID>(identifier_table_->at(implicit_table_[i].id));
		const AnyPtr& ret = filelocal_->member(id);
		if(raweq(undefined, ret)){
			if(!ary){
				ary = xnew<Array>();
			}

			ary->push_back(Xf("%s(%d)")->call(filelocal_unsupported_error(to_smartptr(this), id), implicit_table_[i].lineno));
		}
	}

	if(ary){
		XTAL_SET_EXCEPT(cpp_class<RuntimeError>()->call(Xt("Xtal Runtime Error 1030")->call(Named(Xid(name), ary))));
	}
}

StringPtr Code::inspect(){

#ifdef XTAL_DEBUG

	MemoryStreamPtr ms(xnew<MemoryStream>());

	ms->put_s(XTAL_STRING("identifier_table\n"));
	for(uint_t i=0; i<identifier_table_->size(); ++i){
		Xf("\t%04d:%s\n")->call(i, identifier_table_->at(i));
		ms->put_s(Xf("\t%04d:%s\n")->call(i, identifier_table_->at(i))->to_s());
	}

	ms->put_s(XTAL_STRING("value_table\n"));
	for(uint_t i=0; i<value_table_->size(); ++i){
		ms->put_s(Xf("\t%04d:%s\n")->call(i, value_table_->at(i))->to_s());
	}
	
	ms->put_s(XTAL_STRING("\n"));
	ms->put_s(inspect_range(0, size()));

	ms->seek(0);
	return ms->get_s(ms->size());
	
#endif

	return empty_string;
}

StringPtr Code::inspect_range(int_t start, int_t end){
	return xtal::inspect_range(to_smartptr(this), data()+start, data()+end);
}

}
