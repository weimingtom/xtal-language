
#pragma once

namespace xtal{

class Lib : public Class{
public:

	Lib(bool most_top_level=false);

	Lib(const ArrayPtr& path);
	
	virtual const AnyPtr& do_member(const IDPtr& primary_key, const AnyPtr& secondary_key, const AnyPtr& self, bool inherited_too, bool* nocache);

	virtual void def(const IDPtr& primary_key, const AnyPtr& value, const AnyPtr& secondary_key = null, int_t accessibility = KIND_PUBLIC);

	void append_load_path(const StringPtr& path){
		load_path_list_->push_back(path);
	}

private:

	const AnyPtr& rawdef(const IDPtr& primary_key, const AnyPtr& value, const AnyPtr& secondary_key);

	StringPtr join_path(const StringPtr& sep);

private:

	ArrayPtr load_path_list_;
	ArrayPtr path_;
	bool most_top_level_;

	virtual void visit_members(Visitor& m){
		Class::visit_members(m);
		m & path_ & load_path_list_;
	}
};

}

