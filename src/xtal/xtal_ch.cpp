
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


AnyPtr CodeLib::make_range(const char_t* begin, int_t begin_size, const char_t* end, int_t end_size){
	SetPtr set = xnew<Set>();

	u8* a = (u8*)begin;
	u8* b = (u8*)end;
	int_t as = begin_size;
	int_t bs = end_size;
	u8 buf[16] = {0};
	
	int_t offset = bs - as;
	memcpy(buf+offset, a, as);

	set->set_at((char_t*)(buf+offset), true);
	while(memcmp(buf, b, bs)!=0){
		for(int_t i=bs-1; i>=0; --i){
			buf[i]++;
			if(buf[i]==0){
				offset = bs - i - 1;
				continue;
			}
			break;
		}

		set->set_at((char_t*)(buf+offset), true);
	}

	return set;
}

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

AnyPtr make_range(const char_t* begin, int_t begin_size, const char_t* end, int_t end_size){
	return code_lib_->make_range(begin, begin_size, end, end_size);
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
