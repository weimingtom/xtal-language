#ifndef XTAL_LIB_H_INCLUDE_GUARD
#define XTAL_LIB_H_INCLUDE_GUARD

#pragma once

namespace xtal{

class Lib : public Class{
public:

	Lib(bool most_top_level=false);
	
	virtual const AnyPtr& do_member(const IDPtr& primary_key, const AnyPtr& secondary_key, bool inherited_too, int_t& accessibility, bool& nocache);

	void append_load_path(const StringPtr& path){
		load_path_list_->push_back(path);
	}

private:

	ArrayPtr load_path_list_;
	MapPtr path_map_;

	virtual void visit_members(Visitor& m){
		Class::visit_members(m);
		m & load_path_list_ & path_map_;
	}
};

}


#endif // XTAL_LIB_H_INCLUDE_GUARD
