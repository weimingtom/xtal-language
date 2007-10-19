#include "xtal.h"

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

StringPtr CodeLib::ch_inc(const char_t* data, int_t buffer_size){
	if(buffer_size>6){
		return xnew<InternedString>(data, buffer_size);
	}

	uchar_t buf[8] = {0};
	memcpy(buf+1, data, buffer_size*sizeof(uchar_t));

	for(int_t i=buffer_size; i>=0; --i){
		buf[i]++;
		if(buf[i]==0){
			continue;
		}
		break;
	}

	if(buf[0]==0){
		return xnew<InternedString>((char_t*)buf+1, buffer_size);
	}else{
		return xnew<InternedString>((char_t*)buf, buffer_size+1);
	}
}

int_t CodeLib::ch_cmp(const char_t* a, int_t asize, const char_t* b, int_t bsize){
	if(asize==bsize){
		for(int_t i=0; i<asize; ++i){
			if(a[i]<b[i]){
				return -1;
			}else if(a[i]>b[i]){
				return 1;
			}
		}
		return 0;
	}else if(asize<bsize){
		return -1;
	}
		
	return 1;
}

void ChMaker::add(char_t ch){
	buf[pos++] = ch;
	if(pos==1){
		len = ch_len(ch);
	}else if(pos == -len){
		len = ch_len2(buf);
	}
}

InternedStringPtr ChMaker::to_s(){
	switch(pos){
	case 1: return xnew<InternedString>(buf[0]);
	case 2: return xnew<InternedString>(buf[0], buf[1]);
	case 3: return xnew<InternedString>(buf[0], buf[1], buf[2]);
	default: return xnew<InternedString>(&buf[0], pos);
	}
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

StringPtr ch_inc(const char_t* data, int_t buffer_size){
	return code_lib_->ch_inc(data, buffer_size);
}

int_t ch_cmp(const char_t* a, uint_t asize, const char_t* b, uint_t bsize){
	return code_lib_->ch_cmp(a, asize, b, bsize);
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
