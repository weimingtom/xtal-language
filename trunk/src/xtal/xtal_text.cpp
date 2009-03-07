#include "xtal.h"
#include "xtal_macro.h"

namespace xtal{

void assign_text_map(const AnyPtr& map){
	MapPtr m = ptr_cast<Map>(builtin()->member(Xid(_text_map)));
	m->assign(map);
}

void append_text_map(const AnyPtr& map){
	MapPtr m = ptr_cast<Map>(builtin()->member(Xid(_text_map)));
	m->concat(map);
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
		return 'i';
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

	{ // 書式指定子を埋め込む
		const char_t* src = buf_;
		while(*dest++ = *src++){}
		--dest;
	}

	if(int_type){ // int_t が64-bitの時に特別な書式指定子を埋め込む
		const char_t* src = XTAL_INT_FMT;
		while(*dest++ = *src++){}
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
	if(*str) type_ = *str++;
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

	return str;
}

Format::Format(const StringPtr& str){
	set(str->c_str());
}

void Format::set(const char_t* str){

	values_ = xnew<Map>();
	original_ = xnew<String>(str);
	have_named_ = false;

	const char_t* begin = str;
	char_t buf[256];
	int_t bufpos = 0, n = 0;
	while(true){
		if(str[0]=='%'){
			values_->set_at((int_t)values_->size(), xnew<String>(begin, str));
			str++;
			
			if(str[0]=='%'){
				begin = str++;
			}
			else{
				if(str[0]=='('){
					str++;
					bufpos = 0;
					
					if(!('0'<=str[0] && str[0]<='9')){
						have_named_ = true;
					}

					while(str[0]!=0 && str[0]!=')' && bufpos!=255){
						buf[bufpos++] = str[0];
						str++;					
					}

					if(str[0]==')'){
						str++;
					}

					buf[bufpos++] = 0;
				}
				else{
					bufpos = XTAL_SPRINTF(buf, 256-bufpos, XTAL_STRING("%d"), (u32)n++);
				}

				SmartPtr<FormatSpecifier> ret = xnew<FormatSpecifier>();
				str = ret->parse_format(str);
				values_->set_at(buf, ret);
				begin = str;
			}
		}
		else if(str[0]=='\0'){
			values_->set_at((int_t)values_->size(), xnew<String>(begin, str));
			break;
		}
		else{
			str++;
		}
	}
}

void Format::rawcall(const VMachinePtr& vm){

	if(!have_named_){
		vm->flatten_arg();
	}

	MemoryStreamPtr ms = xnew<MemoryStream>();
	char_t cbuf[256];
	char_t spec[FormatSpecifier::FORMAT_SPECIFIER_MAX];
	char_t* pcbuf;
	
	Xfor2(k, v, values_){
		if(type(k)==TYPE_INT){
			ms->put_s(v->to_s());
		}
		else{
			AnyPtr a = vm->arg(k->to_s()->intern());
			if(!a){
				a = !have_named_ ? vm->arg(k->to_i()) : undefined;
			}
			
			SmartPtr<FormatSpecifier> fs = xnew<FormatSpecifier>(*ptr_cast<FormatSpecifier>(v));
			size_t malloc_size = 0;
			if(fs->max_buffer_size()>=256){
				malloc_size = fs->max_buffer_size() + 1;
				pcbuf = (char_t*)so_malloc(malloc_size);
			}
			else{
				pcbuf = cbuf;
			}
			
			switch(type(a)){
				XTAL_DEFAULT{
					StringPtr str = a->to_s();
					
					if(str->data_size()>=256){
						if(str->data_size()>malloc_size){
							if(malloc_size!=0){
								user_free(pcbuf);
							}
							
							malloc_size = str->data_size() + fs->max_buffer_size() + 1;
							pcbuf = (char_t*)so_malloc(malloc_size);
						}
					}
					fs->make_format_specifier(spec, 's');
					XTAL_SPRINTF(pcbuf, malloc_size ? malloc_size : 255, spec, str->c_str());
					ms->put_s(pcbuf);
				}

				XTAL_CASE(TYPE_NULL){
					ms->put_s(XTAL_STRING("null"));
				}

				XTAL_CASE(TYPE_INT){
					fs->make_format_specifier(spec, fs->change_int_type(), true);
					XTAL_SPRINTF(pcbuf, malloc_size ? malloc_size : 255, spec, ivalue(a));
					ms->put_s(pcbuf);
				}
				
				XTAL_CASE(TYPE_FLOAT){
					fs->make_format_specifier(spec, fs->change_float_type());
					XTAL_SPRINTF(pcbuf, malloc_size ? malloc_size : 255, spec, (double)fvalue(a));
					ms->put_s(pcbuf);
				}
			}
			
			if(malloc_size!=0){
				so_free(pcbuf, malloc_size);
			}
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
	set(v->c_str());
}

Text::Text(const StringPtr& key)
	:key_(key){}

void Text::rawcall(const VMachinePtr& vm){
	MapPtr m = ptr_cast<Map>(builtin()->member(Xid(_text_map)));
	if(m){
		if(const AnyPtr& value=m->at(key_)){
			xnew<Format>(value->to_s())->rawcall(vm);
			return;
		}
	}
	
	vm->return_result(key_);
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
