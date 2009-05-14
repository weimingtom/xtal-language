#include "xtal.h"
#include "xtal_macro.h"

namespace xtal{

Lib::Lib(most_top_level_t){
	set_object_name(XTAL_STRING("lib"));
	set_object_force(1000);
	load_path_list_ = xnew<Array>();
}

Lib::Lib(){
	load_path_list_ = xnew<Array>();
}

const AnyPtr& Lib::rawmember(const IDPtr& primary_key, const AnyPtr& secondary_key, bool inherited_too, int_t& accessibility, bool& nocache){
	XTAL_ASSERT(!raweq(secondary_key, null)); // セカンダリキーが無いときはnullでなくundefinedを指定するようになったので、検出用assert	
	const AnyPtr& ret = Class::rawmember(primary_key, secondary_key, inherited_too, accessibility, nocache);
	if(rawne(ret, undefined)){
		return ret;
	}
	else{

		Xfor(var, load_path_list_){
			StringPtr file_name = Xf("%s/%s.xtal")->call(var, primary_key)->to_s();
			def(primary_key, load(file_name), secondary_key, accessibility);
			return Class::rawmember(primary_key, secondary_key, inherited_too, accessibility, nocache);
		}

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
