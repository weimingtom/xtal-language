
#include "xtal.h"

#include "xtal_codeimpl.h"

namespace xtal{

FunCore empty_xfun_core;
FrameCore empty_frame_core;
ExceptCore empty_except_core;


CodeImpl::CodeImpl()
	:toplevel_("toplevel"), source_file_name_("<noname>"){
	toplevel_.inherit(builtin());
	toplevel_.def("toplevel", toplevel_);
}

int_t CodeImpl::compliant_line_number(const inst_t* p){
	AC<LineNumberTable>::vector::const_iterator it=
		std::lower_bound(
			line_number_table_.begin(),
			line_number_table_.end(),
			static_cast<int_t>(p-data()),
			LineNumberCmp()
		);

	if(it!=line_number_table_.end()){
		if(it==line_number_table_.begin()){
			return 1;
		}
		--it;
		return it->line_number;
	}
	return 0;
}

Code::Code(){
	new(*this) CodeImpl();
}

int_t Code::compliant_line_number(const inst_t* p) const{
	return impl()->compliant_line_number(p);
}

const inst_t* Code::data() const{
	return impl()->data();
}

int_t Code::size() const{
	return impl()->size();
}

const ID& Code::symbol(int_t i) const{
	return impl()->symbol(i);
}

const Any& Code::value(int_t i) const{
	return impl()->value(i);
}

void Code::set_value(int_t i, const Any& v) const{
	impl()->set_value(i, v);
}

FrameCore* Code::frame_core(int_t i) const{
	return &impl()->frame_core_table_[i];
}

FunCore* Code::fun_core(int_t i) const{
	return &impl()->xfun_core_table_[i];
}


String Code::source_file_name() const{ 
	return impl()->source_file_name();
}

const Class& Code::toplevel() const{ 
	return impl()->toplevel();
}

String Code::inspect() const{
	return impl()->inspect();
}

String CodeImpl::inspect(){

	int size = 0;
	const inst_t* pc = data();
	String temp;
	MemoryStream ms;

	for(;;){switch(*pc){
		XTAL_NODEFAULT;

//{CODE_INSPECT{{

	
//}}CODE_INSPECT}

	} pc += size; ms.put_s(Xt("%04d:%s\n")((int_t)(data()-pc), temp).to_s()); }

	ms.seek(0);
	return ms.get_s(ms.size());
}

}
