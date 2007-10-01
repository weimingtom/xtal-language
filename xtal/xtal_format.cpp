#include "xtal.h"
#include "xtal_macro.h"

namespace xtal{

namespace{
	MapPtr user_text_map_;

	void uninitialize_format(){
		user_text_map_ = null;
	}
}

void initialize_format(){
	register_uninitializer(&uninitialize_format);
	user_text_map_ = xnew<Map>();

	{
		ClassPtr p = new_cpp_class<Format>("Format");
		p->method("to_s", &Format::to_s);
		p->method("instance_serial_save", &Format::instance_serial_save);
		p->method("instance_serial_load", &Format::instance_serial_load);
		p->def("serial_new", ctor<Format>());
	}

	builtin()->def("Format", get_cpp_class<Format>());
}


void set_text_map(const MapPtr& map){
	user_text_map_ = map;
}

void add_text_map(const MapPtr& map){
	user_text_map_ = user_text_map_->cat(map);
}

MapPtr get_text_map(){
	return user_text_map_;
}

TextPtr text(const char* text){
	return xnew<Text>(text);
}

TextPtr text(const StringPtr& text){
	return xnew<Text>(text->c_str());
}

FormatPtr format(const char* text){
	return xnew<Format>(text);
}

FormatPtr format(const StringPtr& text){
	return xnew<Format>(text->c_str());
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

void FormatSpecifier::make_format_specifier(char_t* dest, char_t type){
	*dest++ = '%';

	{ // 書式指定子を埋め込む
		const char_t* src = buf_;
		while(*dest++ = *src++){}
		--dest;
	}

	{ // int_t が64-bitの時に特別な書式指定子を埋め込む
		const char* src = XTAL_INT_FMT;
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
		}else{
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
			}else{
				break;
			}
		}
	}

	return str;
}

Format::Format(){
	set("");
}

Format::Format(const StringPtr& str){
	set(str->c_str());
}

Format::Format(const char* str){
	set(str);
}

void Format::set(const char* str){

	values_ = xnew<Map>();
	original_ = xnew<String>(str);

	const char* begin = str;
	char buf[256];
	int bufpos = 0, n = 0;
	while(true){
		if(str[0]=='%'){
			values_->set_at((int_t)values_->size(), xnew<String>(begin, str));
			str++;
			
			if(str[0]=='%'){
				begin = str++;
			}else{
				if(str[0]=='('){
					str++;
					bufpos = 0;
					while(str[0]!=0 && str[0]!=')' && bufpos!=255){
						buf[bufpos++] = str[0];
						str++;					
					}

					if(str[0]==')'){
						str++;
					}

					buf[bufpos++] = 0;
				}else{
					bufpos = sprintf(buf, "%d", n++);
				}

				SmartPtr<FormatSpecifier> ret = xnew<FormatSpecifier>();
				str = ret->parse_format(str);
				values_->set_at(buf, ret);
				begin = str;
			}
		}else if(str[0]=='\0'){
			values_->set_at((int_t)values_->size(), xnew<String>(begin, str));
			break;
		}else{
			str++;
		}
	}
}

void Format::call(const VMachinePtr& vm){
	string_t buf;
	char_t cbuf[256];
	char_t spec[FormatSpecifier::FORMAT_SPECIFIER_MAX];
	char_t* pcbuf;
	
	Xfor2(k, v, values_){
		if(type(k)==TYPE_INT){
			buf += v->to_s()->c_str();
		}else{
			AnyPtr a = vm->arg(k->to_s()->intern());
			if(!a){
				a = vm->arg(k->to_i());
			}
			
			SmartPtr<FormatSpecifier> fs = xnew<FormatSpecifier>(*ptr_cast<FormatSpecifier>(v));
			size_t malloc_size = 0;
			if(fs->max_buffer_size()>=256){
				malloc_size = fs->max_buffer_size() + 1;
				pcbuf = (char_t*)user_malloc(malloc_size);
			}else{
				pcbuf = cbuf;
			}
			
			switch(type(a)){
				XTAL_DEFAULT{
					StringPtr str = a->to_s();
					
					if(str->buffer_size()>=256){
						if(str->buffer_size()>malloc_size){
							if(malloc_size!=0){
								user_free(pcbuf);
							}
							
							malloc_size = str->buffer_size() + fs->max_buffer_size() + 1;
							pcbuf = (char*)user_malloc(malloc_size);
						}
					}
					fs->make_format_specifier(spec, 's');
					sprintf(pcbuf, spec, str->c_str());
					buf += pcbuf;
				}

				XTAL_CASE(TYPE_NULL){
					buf += "<null>";
				}

				XTAL_CASE(TYPE_INT){
					fs->make_format_specifier(spec, fs->change_int_type());
					sprintf(pcbuf, spec, ivalue(a));
					buf += pcbuf;
				}
				
				XTAL_CASE(TYPE_FLOAT){
					fs->make_format_specifier(spec, fs->change_float_type());
					sprintf(pcbuf, spec, fvalue(a));
					buf += pcbuf;
				}
			}
			
			if(malloc_size!=0){
				user_free(pcbuf);
			}
		}
	}

	vm->return_result(xnew<String>(buf));
}

void Format::to_s(const VMachinePtr& vm){
	call(vm);
}

AnyPtr Format::instance_serial_save(){
	return original_;
}

void Format::instance_serial_load(const StringPtr& v){
	set(v->c_str());
}

Text::Text(const StringPtr& key)
	:key_(key){}

void Text::call(const VMachinePtr& vm){
	if(user_text_map_){
		if(const AnyPtr& value=user_text_map_->at(key_)){
			xnew<Format>(value->to_s())->call(vm);
			return;
		}
	}
}

void Text::to_s(const VMachinePtr& vm){
	call(vm);
}

AnyPtr Text::instance_serial_save(){
	return key_;
}

void Text::instance_serial_load(const StringPtr& v){
	key_ = v;
}

}
