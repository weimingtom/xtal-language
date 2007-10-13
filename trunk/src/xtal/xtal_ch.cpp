
#include "xtal.h"
#include "xtal_ch.h"

namespace xtal{

////////////////////////////////////////////////
// sjis

class SJISCodeLib : public CodeLib{
public:
	int_t ch_len(char_t ch){
		u8 c = (u8)ch;
		if(c==0) return 0;
		if((c >= 0x81 && c <= 0x9F) || (c >= 0xE0 && c <= 0xFC)){
			return 2;
		}
		return 1;
	}
};

////////////////////////////////////////////////
// euc

class EUCCodeLib : public CodeLib{
public:
	int_t ch_len(char_t ch){
		u8 c = (u8)ch;
		if(c==0) return 0;
		if(c&0x80){
			return 2;
		}
		return 1;	
	}
};

////////////////////////////////////////////////
// utf8

class UTF8CodeLib : public CodeLib{
public:
	int_t ch_len(char_t ch){
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
};

////////////////////////////////////////////////

namespace{

#ifdef WIN32
	SJISCodeLib default_code_lib_;
#elif defined(__linux__)
	EUCCodeLib default_code_lib_;
#else
	UTF8CodeLib default_code_lib_;
#endif

	CodeLib* code_lib_ = &default_code_lib_;
}

int_t ch_len(char_t lead){
	return code_lib_->ch_len(lead);
}

int_t ch_len2(const char_t* str){
	return code_lib_->ch_len2(str);
}

void set_code_sjis(){
	static SJISCodeLib code_lib;
	code_lib_ = &code_lib;
}

void set_code_euc(){
	static EUCCodeLib code_lib;
	code_lib_ = &code_lib;
}

void set_code_utf8(){
	static UTF8CodeLib code_lib;
	code_lib_ = &code_lib;
}

void set_code(CodeLib& lib){
	code_lib_ = &lib;
}
	
}
