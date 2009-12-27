/** \file src/xtal/xtal_text.h
* \brief src/xtal/xtal_text.h
*/

#ifndef XTAL_TEXT_H_INCLUDE_GUARD
#define XTAL_TEXT_H_INCLUDE_GUARD

#pragma once

namespace xtal{

class FormatSpecifier{
public:

	FormatSpecifier(){}

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
	bool is_int_type();
	bool is_float_type();

	void make_format_specifier(char_t* dest, char_t type, bool int_type = false);

	uint_t parse_format(const char_t* str, uint_t i, uint_t sz);

private:
	
	uint_t parse_format_inner(const char_t* str, uint_t i, uint_t sz);

	uint_t parse_format_digit(const char_t* str, uint_t i, uint_t sz, int_t& digit);
};

class Format : public Base{
public:

	Format(const StringPtr& str = empty_string);
	void set(const StringPtr& str);
	virtual void rawcall(const VMachinePtr& vm);
	void to_s(const VMachinePtr& vm);
	AnyPtr serial_save();
	void serial_load(const StringPtr& v);

private:

	StringPtr original_;
	bool have_named_;
	
	virtual void visit_members(Visitor& m){
		Base::visit_members(m);
		m & original_;
	}
};

class Text : public Base{
public:

	Text(const IDPtr& key = empty_id);
	virtual void rawcall(const VMachinePtr& vm);
	void to_s(const VMachinePtr& vm);
	AnyPtr serial_save();
	void serial_load(const IDPtr& v);

private:	
	IDPtr key_;
};

TextPtr text(const StringPtr& text);
TextPtr text(const StringLiteral& text);

FormatPtr format(const StringPtr& text);
FormatPtr format(const StringLiteral& text);

void assign_text_map(const AnyPtr& map_iter);
void append_text_map(const AnyPtr& map_iter);

}

#endif // XTAL_TEXT_H_INCLUDE_GUARD
