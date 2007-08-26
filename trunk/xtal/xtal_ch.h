
#pragma once

#include "xtal.h"

namespace xtal{
	
/**
* @brief 先頭バイトから、そのマルチバイト文字が何文字かを調べる。
* マイナスの値が返された場合、最低文字数を返す。
* -2の場合、2文字目を読まないと判断できない、という意味となる。
*/
int_t ch_len(char_t lead);

/**
* @brief マルチバイト文字が何文字かを調べる。
* int_t ch_len(char_t lead)が呼ばれた後、マイナスの値を返した場合に続けて呼ばれる。
* ch_lenで-2の値を返した後は、strの先には2バイト分のデータが格納されている。
*/
int_t ch_len2(const char_t* str);


void set_code_sjis();
void set_code_euc();
void set_code_utf8();

inline bool test_range(int ch, int begin, int end){
	return begin<=ch && ch<=end;
}

inline bool test_digit(int ch){
	return test_range(ch, '0', '9');
}

inline bool test_lalpha(int ch){
	return test_range(ch, 'a', 'z');
}

inline bool test_ualpha(int ch){
	return test_range(ch, 'A', 'Z');
}

inline bool test_alpha(int ch){
	return test_lalpha(ch) || test_ualpha(ch);
}

inline bool test_space(int ch){
	return ch==' ' || ch=='\t' || ch=='\n' || ch=='\r';
}

inline bool test_ident_first(int ch){
	return test_alpha(ch) || ch_len(ch)>1;
}

inline bool test_ident_rest(int ch){
	return test_ident_first(ch) || test_digit(ch) || ch=='_';
}

inline bool test_delim(int ch){
	return ch==';' || ch==':' || ch=='}' || ch==']' || ch==')' || ch==',';
}

inline bool test_xml_tag_name(int ch){
	return ch==';' || ch==':' || ch=='}' || ch==']' || ch==')' || ch==',';
}

inline int to_lalpha(int ch){
	using namespace std;
	return tolower(ch);
}

}
