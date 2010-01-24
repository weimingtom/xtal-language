#include "xtal.h"
#include "xtal_macro.h"
#include "xtal_codebuilder.h"

namespace xtal{

//AnyPtr load_lib_member(){

//}

Lib::Lib(most_top_level_t){
	set_object_temporary_name(Xid(lib));
	set_object_force(1000);
	load_path_list_ = xnew<Array>();
}

Lib::Lib(){
	load_path_list_ = xnew<Array>();
}

const AnyPtr& Lib::on_rawmember(const IDPtr& primary_key, const AnyPtr& secondary_key, bool inherited_too, int_t& accessibility, bool& nocache){
	const AnyPtr& ret = Class::on_rawmember(primary_key, secondary_key, inherited_too, accessibility, nocache);
	if(rawne(ret, undefined)){
		return ret;
	}
	else{

		AnyPtr value;
		Xfor(var, load_path_list_){
			value = undefined;

			if(CodePtr code = require_source(Xf("%s/%s")->call(var, primary_key)->to_s())){
				value = code->call();
			}
			else{
				XTAL_CATCH_EXCEPT(e){
					if(e->is(cpp_class<CompileError>())){
						XTAL_SET_EXCEPT(e);
						return undefined;
					}
				}
			}

			XTAL_CATCH_EXCEPT(e){ 
				return Class::on_rawmember(primary_key, secondary_key, inherited_too, accessibility, nocache);
			}

			if(!raweq(value, undefined)){
				on_def(primary_key, value, secondary_key, accessibility);
				return Class::on_rawmember(primary_key, secondary_key, inherited_too, accessibility, nocache);
			}
		}

		nocache = true;
		return Class::on_rawmember(primary_key, secondary_key, inherited_too, accessibility, nocache);
	}
}

void Global::on_def(const IDPtr& primary_key, const AnyPtr& value, const AnyPtr& secondary_key, int_t accessibility){
	Key key = {primary_key, secondary_key};
	map_t::iterator it = map_members_->find(key);
	if(it==map_members_->end()){
		Class::on_def(primary_key, value, secondary_key, accessibility);
	}
	else{
		overwrite_member(primary_key, value, secondary_key, accessibility);
	}
}

}
