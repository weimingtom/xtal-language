
#include "xtal.h"
#include "xtal_ch.h"

namespace xtal{

int_t ch_len_sjis(char ch){
	unsigned char c = (unsigned char)ch;
	if((c >= 0x81 && c <= 0x9F) || (c >= 0xE0 && c <= 0xFC)){
		return 2;
	}
	return 1;
}

int_t ch_len_euc(char_t ch){
	unsigned char c = (unsigned char)ch;
	if(c&0x80){
		return 2;
	}
	return 1;	
}

int_t ch_len_utf8(char_t ch){
	unsigned char c = (unsigned char)ch;
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

namespace{
	int_t (*ch_len_)(char_t lead) = 

#ifdef WIN32
	&ch_len_sjis;
#elif defined(__linux__)
	&ch_len_euc;
#else
	&ch_len_utf8;
#endif

}

int_t ch_len(char_t lead){
	return ch_len_(lead);
}

void set_code_sjis(){
	ch_len_ = &ch_len_sjis;
}

void set_code_euc(){
	ch_len_ = &ch_len_euc;
}

void set_code_utf8(){
	ch_len_ = &ch_len_utf8;
}
	
}
