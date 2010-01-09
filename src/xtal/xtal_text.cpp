#include "xtal.h"
#include "xtal_macro.h"

namespace xtal{

void assign_text_map(const AnyPtr& map){
	text_map()->assign(map);
}

void append_text_map(const AnyPtr& map){
	text_map()->append(map);
}

TextPtr text(const StringPtr& text){
	return xnew<Text>(text);
}

TextPtr text(const StringLiteral& text){
	return xnew<Text>(text);
}

FormatPtr format(const StringPtr& text){
	return xnew<Format>(text);
}

FormatPtr format(const StringLiteral& text){
	return xnew<Format>(text);
}

bool FormatSpecifier::is_int_type(){
	switch(type_){
	case 'i': case 'd': case 'x': case 'X':
		return true;
	
	default:
		return false;
	}
}

bool FormatSpecifier::is_float_type(){
	switch(type_){
	case 'e': case 'E': case 'g':
	case 'G': case 'f':
		return true;
	
	default:
		return false;
	}
}

char_t FormatSpecifier::change_int_type(){
	if(is_int_type()){
		return type_;
	}
	return 'd';
}

char_t FormatSpecifier::change_float_type(){
	if(is_float_type()){
		return type_;
	}
	return 'g';
}
	
int_t FormatSpecifier::type(){
	return type_;
}

int_t FormatSpecifier::max_buffer_size(){
	return width_ + precision_;
}

void FormatSpecifier::make_format_specifier(char_t* dest, char_t type, bool int_type){
	*dest++ = '%';

	{ // 書式指定子を埋め込む
		const char_t* src = buf_;
		while((*dest++ = *src++)!=0){}
		--dest;
	}

	if(int_type){ // int_t が64-bitの時に特別な書式指定子を埋め込む
		const char_t* src = XTAL_INT_FMT.str();
		while((*dest++ = *src++)!=0){}
		--dest;
	}

	*dest++ = type;
	*dest++ = 0;
}

uint_t FormatSpecifier::parse_format(const char_t* const str, uint_t i, uint_t sz){
	width_ = 0;
	precision_ = 0;
	pos_ = 0;		
	i = parse_format_inner(str, i, sz);
	buf_[pos_] = '\0';
	return i;
}

uint_t FormatSpecifier::parse_format_digit(const char_t* const str, uint_t i, uint_t sz, int_t& digit){
	digit = 0;
	while(i!=sz){
		if(str[i]>='0' && str[i]<= '9'){
			digit *= 10;
			digit += str[i]-'0';
			buf_[pos_++] = str[i++]; 
			if(pos_ == BUF_MAX){
				break;
			}
		}
		else{
			break;
		}
	}

	return i;
}

uint_t FormatSpecifier::parse_format_inner(const char_t* const str, uint_t i, uint_t sz){

	while(i!=sz){
		switch(str[i]){
		case '-': case '+': case '0': case ' ': case '#':
			buf_[pos_++] = str[i++];
			if(pos_ == BUF_MAX){
				return i;
			}
			continue;
		}
		break;
	}
	
	i = parse_format_digit(str, i, sz, width_);
	if(pos_ == BUF_MAX || i==sz){ return i; }

	if(str[i]=='.'){	
		buf_[pos_++] = str[i++]; 
		if(pos_ == BUF_MAX || i==sz){ return i; }
		
		i = parse_format_digit(str, i, sz, precision_);
		if(pos_ == BUF_MAX || i==sz){ return i; }
	}

	char_t type = str[i];

	switch(type){
	case 'i': case 'd': case 'x': case 'X':
	case 'e': case 'E': case 'g':
	case 'G': case 'f':
	case 's':
		i++;
		type_ = type;
		break;
	
	default:
		type_ = 's';
		break;
	}

	return i;
}

Format::Format(const StringPtr& str){
	set(str);
}

void Format::set(const StringPtr& original){
	original_ = original;

	const char_t* str = original_->data();
	for(int_t i=0, size=original_->data_size()-2; i<size; ++i){
		if(str[i]=='%' && str[i+1]=='(' && !('0'<=str[i+2] && str[i+2]<='9')){
			have_named_ = true;
			break;
		}
	}
}

void Format::rawcall(const VMachinePtr& vm){
	//if(!have_named_){
	//	vm->flatten_args();
	//}

	MemoryStreamPtr ms = xnew<MemoryStream>();
	enum{
		LIMIT = 255
	};

	char_t cbuf[LIMIT+1];
	char_t spec[FormatSpecifier::FORMAT_SPECIFIER_MAX];

	const char_t* const str = original_->data();
	const char_t* begin = str;
	int_t n = 0;		
	
	AnyPtr value;
	FormatSpecifier fs;

	uint_t i = 0;
	uint_t sz = original_->data_size();

	while(true){
		if(i==sz){
			ms->put_s(begin, str+i);
			break; 
		}

		if(str[i]=='%'){
			ms->put_s(begin, str+i);
			i++;
			if(i==sz){ break; }
			
			if(str[i]=='%'){
				begin = str+i;
				i++;
			}
			else{
				if(str[i]=='('){
					i++;
					if(i==sz){ break; }

					begin = str+i;
					
					bool number = false;
					if('0'<=str[i] && str[i]<='9'){
						number = true;
					}

					while(str[i]!=')'){
						i++;
						if(i==sz){ break; }
					}

					if(number){
						value = vm->arg(ivalue(xnew<String>(begin, str+i)->to_i()));
					}
					else{
						value = vm->arg(xnew<ID>(begin, str+i));
					}

					if(str[i]==')'){
						i++;
					}
				}
				else{
					value = vm->arg(n);
					++n;
				}

				i = fs.parse_format(str, i, sz);
				begin = str + i;

				switch(type(value)){
					XTAL_DEFAULT{
						StringPtr str = value->to_s();
						ms->put_s(str);
					}

					XTAL_CASE(TYPE_NULL){
						ms->put_s(XTAL_STRING("null"));
					}

					XTAL_CASE(TYPE_INT){
						if(fs.is_float_type()){
							fs.make_format_specifier(spec, fs.change_float_type(), true);
							XTAL_SPRINTF(cbuf, LIMIT, spec, (double)ivalue(value));
						}
						else{
							fs.make_format_specifier(spec, fs.change_int_type(), true);
							XTAL_SPRINTF(cbuf, LIMIT, spec, ivalue(value));
						}
						ms->put_s(cbuf);
					}
					
					XTAL_CASE(TYPE_FLOAT){
						if(fs.is_int_type()){
							fs.make_format_specifier(spec, fs.change_int_type(), true);
							XTAL_SPRINTF(cbuf, LIMIT, spec, (int_t)ivalue(value));
						}
						else{			
							fs.make_format_specifier(spec, fs.change_float_type());
							XTAL_SPRINTF(cbuf, LIMIT, spec, (double)fvalue(value));
						}
						ms->put_s(cbuf);
					}
				}
			}
		}
		else{
			i++;
		}
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
	set(v);
}

Text::Text(const IDPtr& key)
	:key_(key){}

void Text::rawcall(const VMachinePtr& vm){
	MapPtr m = text_map();
	if(m){
		if(AnyPtr value = m->at(key_)){
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

void Text::serial_load(const IDPtr& v){
	key_ = v;
}

}
