#include "xtal.h"

namespace xtal{

AnyPtr inst_inspect(inst_address_t& value, Inst* inst, const CodePtr& code){
	return (int_t)(((inst_t*)inst - code->data()) + value + ((u8)&value - (u8)inst));
}

StringPtr make_inst_string(const char* InstName){
	StringPtr temp = format(XTAL_STRING("%s:"))(InstName)->to_s();
	return format(temp)()->to_s();
}

StringPtr make_inst_string(const char* InstName, 
						const char* MemberName1, const AnyPtr& MemberValue1){
	StringPtr temp = format(XTAL_STRING("%s: %s=%%s"))(InstName, MemberName1)->to_s();
	return format(temp)(MemberValue1)->to_s();
}

StringPtr make_inst_string(const char* InstName, 
						const char* MemberName1, const AnyPtr& MemberValue1,
						const char* MemberName2, const AnyPtr& MemberValue2){
	StringPtr temp = format(XTAL_STRING("%s: %s=%%s, %s=%%s"))(InstName, MemberName1, MemberName2)->to_s();
	return format(temp)(MemberValue1, MemberValue2)->to_s();
}

StringPtr make_inst_string(const char* InstName, 
						const char* MemberName1, const AnyPtr& MemberValue1,
						const char* MemberName2, const AnyPtr& MemberValue2,
						const char* MemberName3, const AnyPtr& MemberValue3){
	StringPtr temp = format(XTAL_STRING("%s: %s=%%s, %s=%%s, %s=%%s"))(InstName, MemberName1, MemberName2, MemberName3)->to_s();
	return format(temp)(MemberValue1, MemberValue2, MemberValue3)->to_s();
}

StringPtr make_inst_string(const char* InstName, 
						const char* MemberName1, const AnyPtr& MemberValue1,
						const char* MemberName2, const AnyPtr& MemberValue2,
						const char* MemberName3, const AnyPtr& MemberValue3,
						const char* MemberName4, const AnyPtr& MemberValue4){
	StringPtr temp = format(XTAL_STRING("%s: %s=%%s, %s=%%s, %s=%%s, %s=%%s"))(InstName, MemberName1, MemberName2, MemberName3, MemberName4)->to_s();
	return format(temp)(MemberValue1, MemberValue2, MemberValue3, MemberValue4)->to_s();
}

StringPtr make_inst_string(const char* InstName, 
						const char* MemberName1, const AnyPtr& MemberValue1,
						const char* MemberName2, const AnyPtr& MemberValue2,
						const char* MemberName3, const AnyPtr& MemberValue3,
						const char* MemberName4, const AnyPtr& MemberValue4,
						const char* MemberName5, const AnyPtr& MemberValue5){					
	StringPtr temp = format(XTAL_STRING(": %s=%%s, %s=%%s, %s=%%s, %s=%%s, %s=%%s"))(MemberName1, MemberName2, MemberName3, MemberName4, MemberName5)->to_s();
	return StringPtr(InstName)->cat(format(temp)(MemberValue1, MemberValue2, MemberValue3, MemberValue4, MemberValue5)->to_s());
}

}