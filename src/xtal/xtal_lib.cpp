#include "xtal.h"
#include "xtal_macro.h"
#include "xtal_codebuilder.h"

namespace xtal{

//AnyPtr load_lib_member(){

//}

Lib::Lib(most_top_level_t){
	set_object_temporary_name(XTAL_STRING("lib"));
	set_object_force(1000);
	load_path_list_ = xnew<Array>();
}

Lib::Lib(){
	load_path_list_ = xnew<Array>();
}

const AnyPtr& Lib::rawmember(const IDPtr& primary_key, const AnyPtr& secondary_key, bool inherited_too, int_t& accessibility, bool& nocache){
	const AnyPtr& ret = Class::rawmember(primary_key, secondary_key, inherited_too, accessibility, nocache);
	if(rawne(ret, undefined)){
		return ret;
	}
	else{

		AnyPtr value;
		Xfor(var, load_path_list_){
			if(CodePtr code = require_source(Xf("%s/%s")->call(var, primary_key)->to_s())){
				value = code->call();
			}
			else{
				XTAL_CATCH_EXCEPT(e){
					if(e->is(cpp_class<CompileError>())){
						XTAL_SET_EXCEPT(e);
						return null;
					}
				}
			}

			XTAL_CATCH_EXCEPT(e){ 
				return null;
			}

			if(!raweq(value, undefined)){
				def(primary_key, value, secondary_key, accessibility);
				return Class::rawmember(primary_key, secondary_key, inherited_too, accessibility, nocache);
			}
		}

		nocache = true;
		return undefined;

		/* 指定した名前をフォルダーとみなす
		ArrayPtr next = path_.clone();
		next.push_back(name);
		AnyPtr lib = xnew<Lib>(next);
		return rawdef(name, lib, secondary_key);
		*/
	}
}

}
