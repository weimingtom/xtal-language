
#include "xtal.h"
#include "xtal_ch.h"

namespace xtal{

int_t ch_len_sjis(char_t ch){
	u8 c = (u8)ch;
	if(c==0) return 0;
	if((c >= 0x81 && c <= 0x9F) || (c >= 0xE0 && c <= 0xFC)){
		return 2;
	}
	return 1;
}

int_t ch_len_euc(char_t ch){
	u8 c = (u8)ch;
	if(c==0) return 0;
	if(c&0x80){
		return 2;
	}
	return 1;	
}

int_t ch_len_utf8(char_t ch){
	u8 c = (u8)ch;
	if(c==0) return 0;
	if((c&0x80) && (c&0x40)){
		if(c&0x20){
			if(c&0x10){
				if(c&0x8){
					if(c&0x4){
						return 6;
					}
					return 5;		
				}
				return 4;
			}
			return 3;
		}
		return 2;	
	}
	return 1;
}

int_t ch_len_sjis2(const char_t* str){
	return ch_len_sjis(*str);
}

int_t ch_len_euc2(const char_t* str){
	return ch_len_euc(*str);
}

int_t ch_len_utf82(const char_t* str){
	return ch_len_utf8(*str);
}


namespace{
	int_t (*ch_len_)(char_t lead) = 

#ifdef WIN32
	&ch_len_sjis;
#elif defined(__linux__)
	&ch_len_euc;
#else
	&ch_len_utf8;
#endif

	int_t (*ch_len2_)(const char_t* str) = 

#ifdef WIN32
	&ch_len_sjis2;
#elif defined(__linux__)
	&ch_len_euc2;
#else
	&ch_len_utf82;
#endif
}

int_t ch_len(char_t lead){
	return ch_len_(lead);
}

int_t ch_len2(const char_t* str){
	return ch_len2_(str);
}

void set_code_sjis(){
	ch_len_ = &ch_len_sjis;
	ch_len2_ = &ch_len_sjis2;
}

void set_code_euc(){
	ch_len_ = &ch_len_euc;
	ch_len2_ = &ch_len_euc2;
}

void set_code_utf8(){
	ch_len_ = &ch_len_utf8;
	ch_len2_ = &ch_len_utf82;
}
	
}
