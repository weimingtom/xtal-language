#include "xtal.h"
#include "xtal_macro.h"

namespace xtal{

void assign_text_map(const AnyPtr& map){
	text_map()->assign(map);
}

void append_text_map(const AnyPtr& map){
	text_map()->append(map);
}

AnyPtr text(const StringPtr& text){
	return xnew<Text>(text);
}

AnyPtr format(const StringPtr& text){
	return xnew<Format>(text);
}


char_t FormatSpecifier::change_int_type(){
	switch(type_){
	case 'i': case 'd': case 'x': case 'X':
		return type_;
		break;
	
	default:
		return 'd';
		break;
	}
}

char_t FormatSpecifier::change_float_type(){
	switch(type_){
	case 'e': case 'E': case 'g':
	case 'G': case 'f':
		return type_;
		break;
	
	default:
		return 'g';
		break;
	}
}
	
int_t FormatSpecifier::type(){
	return type_;
}

int_t FormatSpecifier::max_buffer_size(){
	return width_ + precision_;
}

void FormatSpecifier::make_format_specifier(char_t* dest, char_t type, bool int_type){
	*dest++ = '%';

	{ // �����w��q�𖄂ߍ���
		const char_t* src = buf_;
		while((*dest++ = *src++)!=0){}
		--dest;
	}

	if(int_type){ // int_t ��64-bit�̎��ɓ��ʂȏ����w��q�𖄂ߍ���
		const char_t* src = XTAL_INT_FMT;
		while((*dest++ = *src++)!=0){}
		--dest;
	}

	*dest++ = type;
	*dest++ = 0;
}

const char_t* FormatSpecifier::parse_format(const char_t* str){
	width_ = 0;
	precision_ = 0;
	pos_ = 0;		
	str = parse_format_inner(str);
	buf_[pos_] = '\0';
	return str;
}

const char_t* FormatSpecifier::parse_format_inner(const char_t* str){

	while(str[0]){
		switch(str[0]){
		case '-': case '+': case '0': case ' ': case '#':
			buf_[pos_++] = *str++;
			if(pos_ == BUF_MAX){
				return str;
			}
			continue;
		}
		break;
	}
	
	width_ = 0;
	while(str[0]){
		if(str[0]>='0' && str[0]<= '9'){
			width_ *= 10;
			width_ += str[0]-'0';
			buf_[pos_++] = *str++; 
			if(pos_ == BUF_MAX){
				return str;
			}
		}
		else{
			break;
		}
	}

	if(str[0]=='.'){	
		buf_[pos_++] = *str++; 
		if(pos_ == BUF_MAX){
			return str;
		}
		
		precision_ = 0;
		while(str[0]){
			if(str[0]>='0' && str[0]<= '9'){
				precision_ *= 10;
				precision_ += str[0]-'0';
				buf_[pos_++] = *str++; 
				if(pos_ == BUF_MAX){
					return str;
				}
			}
			else{
				break;
			}
		}
	}

	char_t type = *str;

	switch(type){
	case 'i': case 'd': case 'x': case 'X':
	case 'e': case 'E': case 'g':
	case 'G': case 'f':
	case 's':
		str++;
		type_ = type;
		break;
	
	default:
		type_ = 's';
		break;
	}

	return str;
}

Format::Format(const StringPtr& str){
	set(str->c_str());
}

void Format::set(const StringPtr& original){
	original_ = original;

	const char_t* str = original_->c_str();
	for(uint_t i=0, size=original_->data_size()-2; i<size; ++i){
		if(str[i]=='%' && str[i+1]=='(' && !('0'<=str[i+2] && str[i+2]<='9')){
			have_named_ = true;
			break;
		}
	}

	/*
	strings_ = xnew<Array>();

	const char_t* str = original_->c_str();
	const char_t* begin = str;
	int_t n = 0;
	while(true){
		if(str[0]=='%'){
			strings_->push_back(xnew<String>(begin, str));
			str++;
			
			if(str[0]=='%'){
				//strings_->push_back(xnew<String>('%'));
				begin = str++;
			}
			else{
				AnyPtr key;
				if(str[0]=='('){
					str++;
					begin = str;
					
					bool number = false;
					if('0'<=str[0] && str[0]<='9'){
						number = true;
					}

					while(str[0]!=0 && str[0]!=')'){
						str++;					
					}

					if(number){
						key = xnew<String>(begin, str);
						key = key->to_i();
					}
					else{
						key = xnew<ID>(begin, str);
						have_named_ = true;
					}

					if(str[0]==')'){
						str++;
					}
				}
				else{
					key = n++;
				}

				FormatSpecifier fs;
				str = fs.parse_format(str);
				strings_->push_back(mv(key, format_specifiers_.size()));
				format_specifiers_.push_back(fs);
				begin = str;
			}
		}
		else if(str[0]=='\0'){
			strings_->push_back(xnew<String>(begin, str));
			break;
		}
		else{
			str++;
		}
	}
	*/
}

void Format::rawcall(const VMachinePtr& vm){
	if(!have_named_){
		vm->flatten_args();
	}

	MemoryStreamPtr ms = xnew<MemoryStream>();
	uint_t malloc_size = 0;
	char_t cbuf[256];
	char_t spec[FormatSpecifier::FORMAT_SPECIFIER_MAX];
	char_t* pcbuf;

	const char_t* str = original_->c_str();
	const char_t* begin = str;
	int_t n = 0;		
	
	AnyPtr value;
	FormatSpecifier fs;

	while(true){
		if(str[0]=='%'){
			ms->put_s(begin, str);
			str++;
			
			if(str[0]=='%'){
				begin = str++;
			}
			else{
				if(str[0]=='('){
					str++;
					begin = str;
					
					bool number = false;
					if('0'<=str[0] && str[0]<='9'){
						number = true;
					}

					while(str[0]!=0 && str[0]!=')'){
						str++;					
					}

					if(number){
						value = vm->arg(ivalue(xnew<String>(begin, str)->to_i()));
					}
					else{
						value = vm->arg(xnew<ID>(begin, str));
					}

					if(str[0]==')'){
						str++;
					}
				}
				else{
					value = vm->arg(n);
					++n;
				}

				str = fs.parse_format(str);
				begin = str;

				switch(type(value)){
					XTAL_DEFAULT{
						StringPtr str = value->to_s();
						
						if(str->data_size()>=256){
							if(str->data_size()>malloc_size){
								if(malloc_size!=0){
									xfree(pcbuf, malloc_size);
								}
								
								malloc_size = str->data_size() + fs.max_buffer_size() + 1;
								pcbuf = (char_t*)xmalloc(malloc_size);
							}
						}
						else{
							pcbuf = cbuf;
						}

						fs.make_format_specifier(spec, 's');
						XTAL_SPRINTF(pcbuf, malloc_size ? malloc_size : 255, spec, str->c_str());
						ms->put_s(pcbuf);
					}

					XTAL_CASE(TYPE_NULL){
						ms->put_s(XTAL_STRING("null"));
					}

					XTAL_CASE(TYPE_INT){
						fs.make_format_specifier(spec, fs.change_int_type(), true);
						XTAL_SPRINTF(cbuf, malloc_size ? malloc_size : 255, spec, ivalue(value));
						ms->put_s(cbuf);
					}
					
					XTAL_CASE(TYPE_FLOAT){
						fs.make_format_specifier(spec, fs.change_float_type());
						XTAL_SPRINTF(cbuf, malloc_size ? malloc_size : 255, spec, (double)fvalue(value));
						ms->put_s(cbuf);
					}
				}
			}
		}
		else if(str[0]=='\0'){
			ms->put_s(begin, str);
			break;
		}
		else{
			str++;
		}
	}

	if(malloc_size!=0){
		xfree(pcbuf, malloc_size);
	}

	vm->return_result(ms->to_s());
}

void Format::to_s(const VMachinePtr& vm){
	rawcall(vm);
}

AnyPtr Format::serial_save(){
	return original_;
}

void Format::serial_load(const StringPtr& v){
	set(v->c_str());
}

Text::Text(const StringPtr& key)
	:key_(key){}

void Text::rawcall(const VMachinePtr& vm){
	MapPtr m = text_map();
	if(m){
		if(const AnyPtr& value=m->at(key_)){
			xnew<Format>(value->to_s())->rawcall(vm);
			return;
		}
	}
	
	vm->return_result(key_->cat(vm->make_arguments()->to_s()));
}

void Text::to_s(const VMachinePtr& vm){
	rawcall(vm);
}

AnyPtr Text::serial_save(){
	return key_;
}

void Text::serial_load(const StringPtr& v){
	key_ = v;
}

}