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
		if(c==0) return 1;
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
		if(c==0) return 1;
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
// wchar

class WCharCodeLib : public CodeLib{
public:
	int_t ch_len(char_t ch){
		return 1;
	}
};


////////////////////////////////////////////////

StringPtr CodeLib::ch_inc(const char_t* data, int_t buffer_size){
	if(buffer_size>6){
		return xnew<ID>(data, buffer_size);
	}

	uchar_t buf[8] = {0};
	std::memcpy(buf+1, data, buffer_size*sizeof(uchar_t));

	for(int_t i=buffer_size; i>=0; --i){
		buf[i]++;
		if(buf[i]==0){
			continue;
		}
		break;
	}

	if(buf[0]==0){
		return xnew<ID>((char_t*)buf+1, buffer_size);
	}
	else{
		return xnew<ID>((char_t*)buf, buffer_size+1);
	}
}

int_t CodeLib::ch_cmp(const char_t* a, int_t asize, const char_t* b, int_t bsize){
	if(asize==bsize){
		for(int_t i=0; i<asize; ++i){
			if((uchar_t)a[i]<(uchar_t)b[i]){
				return -1;
			}
			else if((uchar_t)a[i]>(uchar_t)b[i]){
				return 1;
			}
		}
		return 0;
	}
	else if(asize<bsize){
		return -1;
	}
		
	return 1;
}

void ChMaker::add(char_t ch){
	buf[pos++] = ch;
	if(pos==1){
		len = ch_len(ch);
	}
	else if(pos == -len){
		len = ch_len2(buf);
	}
}

IDPtr ChMaker::to_s(){
	switch(pos){
	case 1: return xnew<ID>(buf[0]);
	case 2: return xnew<ID>(buf[0], buf[1]);
	case 3: return xnew<ID>(buf[0], buf[1], buf[2]);
	default: return xnew<ID>(&buf[0], pos);
	}
}

namespace{

#ifdef XTAL_USE_WCHAR
	WCharCodeLib default_code_lib_;
#else
#	ifdef WIN32
	SJISCodeLib default_code_lib_;
#	elif defined(__linux__)
	EUCCodeLib default_code_lib_;
#	else
	UTF8CodeLib default_code_lib_;
#	endif
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

void edit_distance_helper(const void* data1, uint_t size1, const void* data2, uint_t size2, int* buf, uint_t k, int offset){
#ifdef max
#	undef max
#endif
	uint_t x = std::max(buf[k-1+offset]+1, buf[k+1+offset]);
	uint_t y = x-k;
	while(x<size1 && y<size2 && ((u8*)data1)[x-1]==((u8*)data2)[y-1]){
		++x;
		++y;
	}
	buf[k+offset] = x;
}

uint_t edit_distance(const void* data1, uint_t data_size1, const void* data2, uint_t data_size2){
	if(data_size1<data_size2){
		return edit_distance(data2, data_size2, data1, data_size1);
	}

	uint_t size1 = data_size1 + 1;
	uint_t size2 = data_size2 + 1;

	int buf_size = size1+size2+6;
	int* buf = (int*)user_malloc(sizeof(int)*buf_size);
	
	for(int i=0; i<buf_size; ++i){
		buf[i] = 0;
	}

	int offset = size2+1, delta = size1-size2;
	for(int p=0;;++p){
		for(int k=-p; k<delta; ++k){
			edit_distance_helper(data1, size1, data2, size2, buf, k, offset);
		}

		for(int k=delta+p; k>delta; --k){
			edit_distance_helper(data1, size1, data2, size2, buf, k, offset);
		}

		edit_distance_helper(data1, size1, data2, size2, buf, delta, offset);

		if(buf[delta+offset]==size1){
			user_free(buf);
			return p+delta;
		}
	}
	return 0;
}

}
