#include "xtal.h"

namespace xtal{


struct FormatString : public Base{

	enum{ BUF_MAX = 14, REAL_BUF_MAX = BUF_MAX + 2 };
	
	char buf_[REAL_BUF_MAX];
	int_t code_pos_, width_, precision_;
	
public:

	FormatString(const char_t*& str){
		code_pos_ = 0;
		buf_[code_pos_++] = '%';
		width_ = 0;
		precision_ = 0;
		
		parse_fmt(str);
	
		buf_[code_pos_] = str[0]; 
		buf_[code_pos_+1] = '\0';
		
		if(str[0]){
			++str;
		}

	}
	
	void change_int_code(){
		switch(code()){
		case 'i': case 'd': case 'x': case 'X':
			break;
		
		default:
			set_code('i');
			break;
		}
	}
	
	void change_float_code(){
		switch(code()){
		case 'e': case 'E': case 'g':
		case 'G': case 'f':
			break;
		
		default:
			set_code('g');
			break;
		}
	}
	
	void change_string_code(){
		set_code('s');
	}
	
	int_t code(){
		return buf_[code_pos_];
	}
	
	void set_code(int_t v){
		buf_[code_pos_] = v;
	}
	
	int_t max_buf_size(){
		return width_ + precision_;
	}
	
	const char* format_string(){
		return buf_;
	}
	
private:

	void parse_fmt(const char_t*& str){
		while(str[0]){
			switch(str[0]){
			case '-': case '+': case '0': case ' ': case '#':
				buf_[code_pos_++] = *str++;
				if(code_pos_ == BUF_MAX){
					return;
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
				buf_[code_pos_++] = *str++; 
				if(code_pos_ == BUF_MAX){
					return;
				}
			}else{
				break;
			}
		}

		if(str[0]=='.'){	
			buf_[code_pos_++] = *str++; 
			if(code_pos_ == BUF_MAX){
				return;
			}
			
			precision_ = 0;
			while(str[0]){
				if(str[0]>='0' && str[0]<= '9'){
					precision_ *= 10;
					precision_ += str[0]-'0';
					buf_[code_pos_++] = *str++; 
					if(code_pos_ == BUF_MAX){
						return;
					}
				}else{
					break;
				}
			}
		}
	}

};

class Format : public Base{
public:

	Format(){
		set("");
	}

	Format(const StringPtr& str){
		set(str->c_str());
	}

	Format(const char* str){
		set(str);
	}

	void set(const char* str){

		original_ = xnew<String>(str);

		const char* begin = str;
		char buf[256];
		int bufpos = 0, n = 0;
		while(true){
			if(str[0]=='%'){
				values_.push_back(Pair(null, xnew<String>(begin, str)));
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
					AnyPtr ret = xnew<FormatString>(ref(str));
					values_.push_back(Pair(InternedStringPtr(buf), ret));
					begin = str;
				}
			}else if(str[0]=='\0'){
				values_.push_back(Pair(null, xnew<String>(begin, str)));
				break;
			}else{
				str++;
			}
		}
	}

	virtual void call(const VMachinePtr& vm){
		string_t buf;
		char_t cbuf[256];
		char_t* pcbuf;
			
		for(int i = 0, size = values_.size(); i<size; ++i){
			if(!values_[i].key){
				buf += cast<StringPtr>(values_[i].value)->c_str();
			}else{
				AnyPtr a = vm->arg(values_[i].key);
				if(!a){
					a = vm->arg(values_[i].key->to_i());
				}
				
				SmartPtr<FormatString> fs = cast<SmartPtr<FormatString> >(values_[i].value);
				size_t malloc_size = 0;
				if(fs->max_buf_size()>=256){
					malloc_size = fs->max_buf_size() + 1;
					pcbuf = (char*)user_malloc(malloc_size);
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
								
								malloc_size = str->buffer_size() + fs->max_buf_size() + 1;
								pcbuf = (char*)user_malloc(malloc_size);
							}
						}
						fs->change_string_code();
						sprintf(pcbuf, fs->format_string(), str->c_str());
						buf += pcbuf;
					}

					XTAL_CASE(TYPE_NULL){
						buf += "<null>";
					}

					XTAL_CASE(TYPE_INT){
						fs->change_int_code();
						sprintf(pcbuf, fs->format_string(), ivalue(a));
						buf += pcbuf;
					}
					
					XTAL_CASE(TYPE_FLOAT){
						fs->change_float_code();
						sprintf(pcbuf, fs->format_string(), fvalue(a));
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

	void to_s(const VMachinePtr& vm){
		call(vm);
	}

	AnyPtr serial_save(){
		return original_;
	}

	void serial_load(const StringPtr& v){
		set(v->c_str());
	}

private:

	struct Pair{
		InternedStringPtr key;
		AnyPtr value;
		Pair(const InternedStringPtr& k = null, const AnyPtr& v = null)
			:key(k), value(v){}
	};
	
	friend void visit_members(Visitor& m, const Pair& p){
		m & p.key & p.value;
	}

	AC<Pair>::vector values_;
	StringPtr original_;
	
	virtual void visit_members(Visitor& m){
		Base::visit_members(m);
		m & values_ & original_;
	}
};

namespace{
	MapPtr user_get_text_map_;

	void uninitialize_format(){
		user_get_text_map_ = null;
	}
}

void initialize_format(){
	register_uninitializer(&uninitialize_format);
	user_get_text_map_ = xnew<Map>();

	{
		ClassPtr p = new_cpp_class<Format>("Format");
		p->method("to_s", &Format::to_s);
		p->method("serial_save", &Format::serial_save);
		p->method("serial_load", &Format::serial_load);
		p->def("serial_new", ctor<Format, const StringPtr&>());
	}

	builtin()->def("Format", get_cpp_class<Format>());
}


void set_get_text_map(const MapPtr& map){
	user_get_text_map_ = map;
}

void add_get_text_map(const MapPtr& map){
	user_get_text_map_ = user_get_text_map_->cat(map);
}

MapPtr get_get_text_map(){
	return user_get_text_map_;
}

AnyPtr get_text(const char* text){
	if(user_get_text_map_){
		StringPtr key(text);
		if(const AnyPtr& value=user_get_text_map_->at(key)){
			return xnew<Format>(value->to_s());
		}
	}
	return xnew<Format>(text);
}

AnyPtr format(const char* text){
	return xnew<Format>(text);
}

}
