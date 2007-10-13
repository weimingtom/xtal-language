
#pragma once

namespace xtal{

TextPtr text(const StringPtr& text);

FormatPtr format(const StringPtr& text);

void set_text_map(const MapPtr& map);

void add_text_map(const MapPtr& map);

MapPtr get_text_map();


struct FormatSpecifier{

	enum{ BUF_MAX = 20, REAL_BUF_MAX = BUF_MAX + 2 };
	
	char_t buf_[REAL_BUF_MAX];
	char_t type_;
	int_t pos_, width_, precision_;
	
public:

	enum{ FORMAT_SPECIFIER_MAX = 20 };
	
	int_t type();
	int_t max_buffer_size();

	char_t change_int_type();
	char_t change_float_type();
	
	void make_format_specifier(char_t* dest, char_t type);

	const char_t* parse_format(const char_t* str);

private:
	
	const char_t* parse_format_inner(const char_t* str);

};

class Format : public Base{
public:

	Format(const StringPtr& str = "");
	void set(const char* str);
	virtual void call(const VMachinePtr& vm);
	void to_s(const VMachinePtr& vm);
	AnyPtr instance_serial_save();
	void instance_serial_load(const StringPtr& v);

private:

	MapPtr values_;
	StringPtr original_;
	int_t param_count_;
	bool have_named_;
	
	virtual void visit_members(Visitor& m){
		Base::visit_members(m);
		m & values_ & original_;
	}
};

class Text : public Base{
public:

	Text(const StringPtr& key = "");
	virtual void call(const VMachinePtr& vm);
	void to_s(const VMachinePtr& vm);
	AnyPtr instance_serial_save();
	void instance_serial_load(const StringPtr& v);

private:	
	StringPtr key_;
};

}
