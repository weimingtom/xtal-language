
#pragma once

namespace xtal{
	
/**
* @brief 先頭バイトを見て、そのマルチバイト文字が何文字かを調べる。
*
* マイナスの値が返された場合、最低文字数を返す。
* -2の場合、最低2文字以上の文字で、本当の長さは2文字目を読まないと判断できない、という意味となる。
*/
int_t ch_len(char_t lead);

/**
* @brief マルチバイト文字が何文字かを調べる。
*
* int_t ch_len(char_t lead)が呼ばれた後、マイナスの値を返した場合に続けて呼ぶ。
* ch_lenで-2の値を返した後は、strの先には最低2バイト分のデータを格納すること。
*/
int_t ch_len2(const char_t* str);


/**
* @brief 一つ先の文字を返す
*
* 例えば a を渡した場合、b が返る
*/
StringPtr ch_inc(const char_t* data, int_t buffer_size);

/**
* @brief 文字の大小判定
*
* 負の値 a の文字の方がbの文字より小さい
* 0の値 等しい
* 正の値 bの文字の方がaの文字より小さい
*/
int_t ch_cmp(const char_t* a, uint_t asize, const char_t* b, uint_t bsize);



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

void set_code_sjis();
void set_code_euc();
void set_code_utf8();

class CodeLib{
public:
	virtual ~CodeLib(){}
	virtual void initialize(){}
	virtual int_t ch_len(char_t lead) = 0;
	virtual int_t ch_len2(const char_t* str){ return ch_len(*str); }
	virtual StringPtr ch_inc(const char_t* data, int_t buffer_size);
	virtual int_t ch_cmp(const char_t* a, int_t asize, const char_t* b, int_t bsize);
};

void set_code(CodeLib& lib);


struct ChMaker{

	ChMaker(){
		pos = 0;
		len = -1;
	}

	bool is_completed(){
		return pos==len;
	}

	void add(char_t ch);

	IDPtr to_s();

	void clear(){
		pos = 0;
		len = -1;
	}

	int_t pos;
	int_t len;
	char_t buf[8];
};
}
