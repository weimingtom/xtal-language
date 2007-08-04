
#include "xtal.h"

#ifndef XTAL_NO_PARSER

#include <math.h>

#include "xtal_lexer.h"
#include "xtal_macro.h"
#include "xtal_array.h"
#include "xtal_stream.h"

namespace xtal{

#define c2(C1, C2) ((C2)<<8 | (C1))
#define c3(C1, C2, C3) ((C3)<<16 | (C2)<<8 | (C1))
#define c4(C1, C2, C3, C4) ((C4)<<24 | (C3)<<16 | (C2)<<8 | (C1))


LPCCommon::LPCCommon()
	:errors(null), ident_table(null), ident_map(null), value_table(null), source_file_name(null){}
	
void LPCCommon::init(const StringPtr& file_name){
	errors = xnew<Array>();
	ident_table = xnew<Array>();
	ident_map = xnew<Map>();
	value_table = xnew<Array>();
	once_table = xnew<Array>();
	source_file_name = file_name;

	register_ident(InternedStringPtr(""));
	value_table->push_back(null);
	once_table->push_back(nop);
}

void LPCCommon::error(int_t line, const AnyPtr& message){
	if(errors->size()<10){
		errors->push_back(Xf("%(file)s:%(line)d:%(message)")(
			Named("file", source_file_name),
			Named("line", line),
			Named("message", message)
		));
	}
}

int_t LPCCommon::register_ident(const InternedStringPtr& ident){
	if(AnyPtr pos = ident_map->at(ident)){ return pos->to_i(); }
	ident_table->push_back(ident);
	ident_map->set_at(ident, ident_table->size()-1);
	return ident_table->size()-1;
}


int_t LPCCommon::register_value(const AnyPtr& v){
	if(StringPtr str = as<StringPtr>(v)){
		value_table->push_back(str->intern());
	}else{
		value_table->push_back(v);
	}
	return value_table->size()-1;
}

int_t LPCCommon::append_ident(const InternedStringPtr& ident){
	ident_table->push_back(ident);
	return ident_table->size()-1;
}

int_t LPCCommon::append_value(const AnyPtr& v){
	value_table->push_back(v);
	return value_table->size()-1;
}

int_t LPCCommon::append_once(){
	once_table->push_back(nop);
	return once_table->size()-1;
}

Lexer::Lexer(){
	set_line(1);
	read_ = 0;
	pos_ = 0;
	mode_ = NORMAL_MODE;
}

void Lexer::init(const StreamPtr& stream, const StringPtr& source_file_name){
	reader_.set_stream(stream);
	com_.init(source_file_name);
	
	keyword_map_ = xnew<Map>();
	keyword_map_->set_at(InternedStringPtr("if"), (int_t)Token::KEYWORD_IF);
	keyword_map_->set_at(InternedStringPtr("for"), (int_t)Token::KEYWORD_FOR);
	keyword_map_->set_at(InternedStringPtr("else"), (int_t)Token::KEYWORD_ELSE);
	keyword_map_->set_at(InternedStringPtr("fun"), (int_t)Token::KEYWORD_FUN);
	keyword_map_->set_at(InternedStringPtr("method"), (int_t)Token::KEYWORD_METHOD);
	keyword_map_->set_at(InternedStringPtr("do"), (int_t)Token::KEYWORD_DO);
	keyword_map_->set_at(InternedStringPtr("while"), (int_t)Token::KEYWORD_WHILE);
	keyword_map_->set_at(InternedStringPtr("continue"), (int_t)Token::KEYWORD_CONTINUE);
	keyword_map_->set_at(InternedStringPtr("break"), (int_t)Token::KEYWORD_BREAK);
	keyword_map_->set_at(InternedStringPtr("fiber"), (int_t)Token::KEYWORD_FIBER);
	keyword_map_->set_at(InternedStringPtr("yield"), (int_t)Token::KEYWORD_YIELD);
	keyword_map_->set_at(InternedStringPtr("return"), (int_t)Token::KEYWORD_RETURN);
	keyword_map_->set_at(InternedStringPtr("once"), (int_t)Token::KEYWORD_ONCE);
	keyword_map_->set_at(InternedStringPtr("null"), (int_t)Token::KEYWORD_NULL);
	keyword_map_->set_at(InternedStringPtr("false"), (int_t)Token::KEYWORD_FALSE);
	keyword_map_->set_at(InternedStringPtr("true"), (int_t)Token::KEYWORD_TRUE);
	keyword_map_->set_at(InternedStringPtr("nop"), (int_t)Token::KEYWORD_NOP);
	keyword_map_->set_at(InternedStringPtr("xtal"), (int_t)Token::KEYWORD_XTAL);
	keyword_map_->set_at(InternedStringPtr("try"), (int_t)Token::KEYWORD_TRY);
	keyword_map_->set_at(InternedStringPtr("catch"), (int_t)Token::KEYWORD_CATCH);
	keyword_map_->set_at(InternedStringPtr("finally"), (int_t)Token::KEYWORD_FINALLY);
	keyword_map_->set_at(InternedStringPtr("throw"), (int_t)Token::KEYWORD_THROW);
	keyword_map_->set_at(InternedStringPtr("class"), (int_t)Token::KEYWORD_CLASS);
	keyword_map_->set_at(InternedStringPtr("import"), (int_t)Token::KEYWORD_IMPORT);
	keyword_map_->set_at(InternedStringPtr("callee"), (int_t)Token::KEYWORD_CALLEE);
	keyword_map_->set_at(InternedStringPtr("this"), (int_t)Token::KEYWORD_THIS);
	keyword_map_->set_at(InternedStringPtr("current_context"), (int_t)Token::KEYWORD_CURRENT_CONTEXT);
	keyword_map_->set_at(InternedStringPtr("dofun"), (int_t)Token::KEYWORD_DOFUN);
	keyword_map_->set_at(InternedStringPtr("is"), (int_t)Token::KEYWORD_IS);
	keyword_map_->set_at(InternedStringPtr("export"), (int_t)Token::KEYWORD_EXPORT);
	keyword_map_->set_at(InternedStringPtr("unittest"), (int_t)Token::KEYWORD_UNITTEST);
	keyword_map_->set_at(InternedStringPtr("assert"), (int_t)Token::KEYWORD_ASSERT);
	keyword_map_->set_at(InternedStringPtr("pure"), (int_t)Token::KEYWORD_PURE);
	keyword_map_->set_at(InternedStringPtr("nobreak"), (int_t)Token::KEYWORD_NOBREAK);
	keyword_map_->set_at(InternedStringPtr("switch"), (int_t)Token::KEYWORD_SWITCH);
	keyword_map_->set_at(InternedStringPtr("case"), (int_t)Token::KEYWORD_CASE);
	keyword_map_->set_at(InternedStringPtr("default"), (int_t)Token::KEYWORD_DEFAULT);
}

LPCCommon* Lexer::common(){
	return &com_;
}


Reader::Reader()
	:stream_(null){
	pos_ = 0;
	read_ = 0;
	marked_=(uint_t)-1;
}

int_t Reader::read(){
	int_t ret = peek();
	++pos_;
	return ret;
}

int_t Reader::peek(){
	if(pos_==read_){
		read_ += stream_->read(&buf_[pos_ & BUF_MASK], BUF_SIZE-(pos_ & BUF_MASK));
		if(pos_==read_){
			return -1;
		}
	}
	return buf_[pos_ & BUF_MASK];
}

bool Reader::eat(int_t ch){
	if(peek()==ch){
		read();
		return true;
	}
	return false;
}

void Reader::putback(int_t ch){
	pos_--;
	buf_[pos_ & BUF_MASK]=ch;
}

int_t Reader::position(){
	return pos_;
}

void Reader::set_position(int_t pos){
	XTAL_ASSERT((int_t)pos_ >= pos);
	XTAL_ASSERT(pos_-pos < BUF_SIZE);
	pos_ = pos;
}

	
StringPtr Lexer::token2str(const Token& t){
	switch(t.type()){
		XTAL_NODEFAULT;
		
		XTAL_CASE(Token::TYPE_KEYWORD){
			Xfor2(key, value, keyword_map_->each_pair()){
				if(ivalue(value)==t.ivalue()){
					return xnew<String>("Keyword");
				}
			}
			return xnew<String>("Unknown Keyword");
		}

		XTAL_CASE(Token::TYPE_IDENT){
			return cast<StringPtr>(Xf("Identifier %s")(com_.ident_table->at(t.ivalue())));
		}
				
		XTAL_CASE(Token::TYPE_INT){
			return cast<StringPtr>(Xf("Number %s")(t.ivalue()));
		}
		
		XTAL_CASE(Token::TYPE_FLOAT){
			return cast<StringPtr>(Xf("Number %s")(t.fvalue()));
		}

		XTAL_CASE(Token::TYPE_TOKEN){
			if(t.ivalue()==-1){
				return xnew<String>("End of File");
			}else{
				char_t buf[5];
				buf[0] = (char_t)(t.ivalue() & 0xff);
				buf[1] = (char_t)(t.ivalue()>>8 & 0xff);
				buf[2] = (char_t)(t.ivalue()>>16 & 0xff);
				buf[3] = (char_t)(t.ivalue()>>24 & 0xff);
				buf[4] = 0;
				return xnew<String>(buf);
			}
		}
	}

	return xnew<String>("");
}

InternedStringPtr Lexer::keyword2id(int_t v){
	Xfor2(key, value, keyword_map_->each_pair()){
		if(ivalue(value)==v){
			return cast<StringPtr>(key);
		}
	}
	return InternedStringPtr("");
}

Token Lexer::read(){
	Token ret = peek();
	++pos_;
	return ret;
}

Token Lexer::peek(){
	if(pos_==read_){
		do_read();
		if(pos_==read_){
			return Token(Token::TYPE_TOKEN, (int_t)-1, (int_t)0);
		}
	}
	return buf_[pos_ & BUF_MASK];
}

void Lexer::push(int_t v){
	buf_[read_ & BUF_MASK] = Token(Token::TYPE_TOKEN, v, left_space_ | test_right_space(reader_.peek()));
	read_++;
}
	
void Lexer::push_int(int_t v){
	buf_[read_ & BUF_MASK] = Token(Token::TYPE_INT, v, left_space_ | test_right_space(reader_.peek()));
	read_++;
}

void Lexer::push_float(float_t v){
	buf_[read_ & BUF_MASK] = Token(Token::TYPE_FLOAT, v, left_space_ | test_right_space(reader_.peek()));
	read_++;
}
	
void Lexer::push_keyword(int_t v){
	buf_[read_ & BUF_MASK] = Token(Token::TYPE_KEYWORD, v, left_space_ | test_right_space(reader_.peek()));
	read_++;
}
	
void Lexer::push_ident(int_t v){
	buf_[read_ & BUF_MASK] = Token(Token::TYPE_IDENT, v, left_space_ | test_right_space(reader_.peek()));
	read_++;
}

void Lexer::push_direct(int_t v){
	buf_[read_ & BUF_MASK] = Token(Token::TYPE_TOKEN, v, 0);
	read_++;
}

void Lexer::putback(){
	pos_--;
}

void Lexer::putback(const Token& ch){
	pos_--;
	buf_[pos_ & BUF_MASK] = ch;
}

void Lexer::set_string_mode(){
	mode_ = STRING_MODE;
}
	
void Lexer::set_normal_mode(){
	mode_ = NORMAL_MODE;
}
	
int_t Lexer::parse_ident(){
	string_t buf;

	int len = ch_len(reader_.peek());
	while(len--)
		buf += read_from_reader();
	while(test_ident_rest(reader_.peek())){
		len = ch_len(reader_.peek());
		while(len--)
			buf += read_from_reader();
	}
	InternedStringPtr id(buf.c_str());
	if(AnyPtr ret = keyword_map_->at(id)){
		return -ret->to_i();
	}
	return com_.register_ident(id);
}

int_t Lexer::parse_integer(){
	int_t ret = 0;
	while(1){
		if(test_digit(reader_.peek())){
			ret *= 10;
			ret += read_from_reader()-'0';
		}else if(reader_.peek()=='_'){
			read_from_reader();
		}else{
			break;
		}
	}
	return ret;
}

int_t Lexer::parse_hex(){
	int_t ret = 0;
	while(1){
		if(test_digit(reader_.peek())){
			ret *= 16;
			ret += read_from_reader()-'0';
		}else if(test_range(reader_.peek(), 'a', 'f')){
			ret *= 16;
			ret += read_from_reader()-'a';
		}else if(test_range(reader_.peek(), 'A', 'F')){
			ret *= 16;
			ret += read_from_reader()-'A';
		}else if(reader_.peek()=='_'){
			read_from_reader();
		}else{
			break;
		}
	}

	if(test_ident_rest(reader_.peek())){
		com_.error(line(), Xt("Xtal Compile Error 1015"));
	}

	return ret;		
}

int_t Lexer::parse_bin(){
	int ret = 0;
	while(1){
		if(test_range(reader_.peek(), '0', '1')){
			ret *= 2;
			ret += read_from_reader()-'0';
		}else if(reader_.peek()=='_'){
			read_from_reader();
		}else{
			break;
		}
	}

	if(test_ident_rest(reader_.peek()) || ('2'<=reader_.peek() && reader_.peek()<='9')){
		com_.error(line(), Xt("Xtal Compile Error 1020"));
	}

	return ret;
}

void Lexer::parse_number_suffix(int_t val){
	if(eat_from_reader('K')){
		push_int(val*1024);
	}else if(eat_from_reader('M')){
		push_int(val*1024*1024);	
	}else if(eat_from_reader('G')){
		push_int(val*1024*1024*1024);	
	}else if(eat_from_reader('f') || eat_from_reader('F')){
		push_float((float_t)val);	
	}else{

		if(test_ident_rest(reader_.peek())){
			com_.error(line(), Xt("Xtal Compile Error 1010"));
		}

		push_int(val);
	}
}

void Lexer::parse_number_suffix(float_t val){
	if(eat_from_reader('K')){
		push_float(val*1024);	
	}else if(eat_from_reader('M')){
		push_float(val*1024*1024);	
	}else if(eat_from_reader('G')){
		push_float(val*1024*1024*1024);	
	}else if(eat_from_reader('f') || eat_from_reader('F')){
		push_float(val);	
	}else{
	
		if(test_ident_rest(reader_.peek())){
			com_.error(line(), Xt("Xtal Compile Error 1010"));
		}

		push_float(val);
	}
}

void Lexer::parse_number(){
	if(eat_from_reader('0')){
		if(eat_from_reader('x') || eat_from_reader('X')){
			push_int(parse_hex());
			return;
		}else if(eat_from_reader('b') || eat_from_reader('B')){
			push_int(parse_bin());
			return;
		}
	}
	
	int_t ival = parse_integer();
	
	if(eat_from_reader('.')){
		if(test_digit(reader_.peek())){
			float_t scale = 1;
			while(eat_from_reader('0')){
				scale /= 10;
			}
			float_t fval = 0;
			int_t after_the_decimal_point = parse_integer();
			while(after_the_decimal_point!=0){
				fval+=after_the_decimal_point%10;
				fval/=10;
				after_the_decimal_point/=10;
			}
			fval *= scale;
			fval += ival;
			int_t e = 1;
			if(eat_from_reader('e') || eat_from_reader('E')){
				if(eat_from_reader('-')){
					e=-1;
				}else{
					eat_from_reader('+');
				}
				if(!test_digit(reader_.peek())){
					com_.error(line(), Xt("Xtal Compile Error 1014"));
				}
				e *= parse_integer();
				{
					using namespace std;
					fval *= (float_t)pow((float_t)10, (float_t)e);
				}
			}
			parse_number_suffix(fval);
		}else{
			putback_to_reader('.');
			push_int(ival);
		}
	}else{
		parse_number_suffix(ival);
	}
}
	
int_t Lexer::read_from_reader(){
	int_t ch = reader_.read();
	if(recording_){
		recorded_string_ += (char)ch;
	}
	return ch;
}

bool Lexer::eat_from_reader(int_t ch){
	if(reader_.eat(ch)){
		if(recording_){
			recorded_string_ += (char)ch;
		}
		return true;
	}
	return false;
}

void Lexer::putback_to_reader(int_t ch){
	if(recording_){
		recorded_string_.resize(recorded_string_.size()-1);
	}
	reader_.putback(ch);
}

void Lexer::do_read(){
	left_space_ = 0;
	
	if(mode_==STRING_MODE){
		int_t ch = read_from_reader();
		if(ch=='\r'){
			eat_from_reader('\n');
			set_line(line()+1);
			ch = '\n';
		}else if(ch=='\n'){
			set_line(line()+1);
		}
		push_direct(ch);
		return;
	}

	do{

		int_t ch = read_from_reader();
		
		switch(ch){

			XTAL_DEFAULT{
				if(test_ident_first(ch)){
					putback_to_reader(ch);
					int_t ident = parse_ident();
					if(ident<0){
						push_keyword(-ident);
					}else{
						push_ident(ident);
					}
				}else if(test_digit(ch)){
					putback_to_reader(ch);
					parse_number();
					return;
				}else if(ch=='_'){
					push('_');
					int_t ident = parse_ident();
					if(ident<0){
						push_keyword(-ident);
					}else{
						push_ident(ident);
					}
					return;
				}else{
					push(ch);
					//com_.error(line(), "不正な文字です");
				}
			}
			
			XTAL_CASE('+'){
				if(eat_from_reader('+')){
					push(c2('+', '+'));
				}else if(eat_from_reader('=')){
					push(c2('+', '='));
				}else{
					push('+');
				}
			}
			
			XTAL_CASE('-'){
				if(eat_from_reader('-')){
					push(c2('-', '-'));
				}else if(eat_from_reader('=')){
					push(c2('-', '='));
				}else{
					push('-');
				}
			}
			
			XTAL_CASE('~'){
				if(eat_from_reader('=')){
					push(c2('~', '='));
				}else{
					push('~');
				}
			}
			
			XTAL_CASE('*'){
				if(eat_from_reader('=')){
					push(c2('*', '='));
				}else{
					push('*');
				}
			}
			
			XTAL_CASE('/'){
				if(eat_from_reader('=')){
					push(c2('/', '='));
				}else if(eat_from_reader('/')){
					while(1){
						int_t ch = read_from_reader();
						if(ch=='\r'){
							eat_from_reader('\n');
							set_line(line()+1);
							left_space_ = Token::FLAG_LEFT_SPACE;
							break;
						}else if(ch=='\n'){
							set_line(line()+1);
							left_space_ = Token::FLAG_LEFT_SPACE;
							break;
						}else if(ch==-1){
							break;
						}
					}
					continue;
				}else if(eat_from_reader('*')){
					while(1){
						int_t ch = read_from_reader();
						if(ch=='\r'){
							eat_from_reader('\n');
							set_line(line()+1);
						}else if(ch=='\n'){
							set_line(line()+1);
						}else if(ch=='*'){
							if(eat_from_reader('/')){
								left_space_ = Token::FLAG_LEFT_SPACE;
								break;
							}
						}else if(ch==-1){
							com_.error(line(), Xt("Xtal Compile Error 1021"));
							break;
						}
					}
					continue;
				}else{
					push('/');
				}
			}			
			
			XTAL_CASE('#'){
				if(eat_from_reader('!')){
					while(1){
						int_t ch = read_from_reader();
						if(ch=='\r'){
							eat_from_reader('\n');
							set_line(line()+1);
							left_space_ = Token::FLAG_LEFT_SPACE;
							break;
						}else if(ch=='\n'){
							set_line(line()+1);
							left_space_ = Token::FLAG_LEFT_SPACE;
							break;
						}else if(ch==-1){
							break;
						}
					}
					continue;
				}else{
					push('#');
				}
			}			
			
			XTAL_CASE('%'){
				if(eat_from_reader('=')){
					push(c2('%', '='));
				}else{
					push('%');
				}
			}
			
			XTAL_CASE('&'){
				if(eat_from_reader('&')){
					push(c2('&', '&'));
				}else{
					push('&');
				}
			}
			
			XTAL_CASE('|'){
				if(eat_from_reader('|')){
					push(c2('|', '|'));
				}else{
					push('|');
				}
			}
						
			XTAL_CASE('>'){
				if(eat_from_reader('>')){
					if(eat_from_reader('>')){
						if(eat_from_reader('=')){
							push(c3('>','3','='));
						}else{
							push(c3('>','>','>'));
						}
					}else{
						push(c2('>','>'));
					}
				}else if(eat_from_reader('=')){
					push(c2('>', '='));
				}else{
					push('>');
				}
			}
			
			XTAL_CASE('<'){
				if(eat_from_reader('<')){
					if(eat_from_reader('=')){
						push(c3('<','<','='));
					}else{
						push(c2('<','<'));
					}
				}else if(eat_from_reader('=')){
					push(c2('<', '='));
				}else{
					push('<');
				}
			}
			
			XTAL_CASE('='){
				if(eat_from_reader('=')){
					if(eat_from_reader('=')){
						push(c3('=', '=', '='));
					}else{
						push(c2('=', '='));
					}
				}else{
					push('=');
				}
			}
			
			XTAL_CASE('!'){
				if(eat_from_reader('=')){
					if(eat_from_reader('=')){
						push(c3('!', '=', '='));
					}else{
						push(c2('!', '='));
					}
				}else if(eat_from_reader('i')){
					if(eat_from_reader('s')){
						if(!test_ident_rest(reader_.peek())){
							push(c3('!', 'i', 's'));
						}else{
							putback_to_reader('s');
							putback_to_reader('i');
							push('!');
						}
					}else{
						putback_to_reader(ch);
						push('!');
					}
				}else{
					push('!');
				}
			}
			
			XTAL_CASE('.'){
				if(eat_from_reader('.')){
					if(eat_from_reader('.')){
						push(c3('.', '.', '.'));
					}else{
						push(c2('.', '.'));
					}
				}else if(eat_from_reader('?')){
					push(c2('.', '?'));
				}else{
					push('.');
				}
			}

			XTAL_CASE('"'){ //"
				push('"'); //"
			}
			
			XTAL_CASE(':'){
				if(eat_from_reader(':')){
					if(eat_from_reader('?')){
						push(c3(':', ':', '?'));
					}else{
						push(c2(':', ':'));
					}
				}else{
					push(':');
				}
			}

			XTAL_CASE(';'){ push(';'); }
			XTAL_CASE('{'){ push('{'); }
			XTAL_CASE('}'){ push('}'); push('\a'); }
			XTAL_CASE('['){ push('['); }
			XTAL_CASE(']'){ push(']'); }
			XTAL_CASE('('){ push('('); }
			XTAL_CASE(')'){ push(')'); }

			XTAL_CASE4(' ', '\t', '\r', '\n'){
				putback_to_reader(ch);
				deplete_space();
				left_space_ = Token::FLAG_LEFT_SPACE;
				continue;
			}

			XTAL_CASE(-1){
				push(-1);
			}
		}
			
		break;
	}while(1);

}

void Lexer::deplete_space(){
	while(1){
		int_t ch = read_from_reader();
		if(ch=='\r'){
			eat_from_reader('\n');
			set_line(line()+1);
		}else if(ch=='\n'){
			set_line(line()+1);
		}else if(ch==' ' || ch=='\t'){
			
		}else{
			putback_to_reader(ch);
			return;
		}
	}
}

int_t Lexer::test_right_space(int_t ch){
	if(test_space(ch)){
		return Token::FLAG_RIGHT_SPACE;
	}
	return 0;
}
	
void Lexer::begin_record(){
	recording_ = true;
	recorded_string_ = "";
}

string_t Lexer::end_record(){
	recording_ = false;
	if(!recorded_string_.empty()){
		recorded_string_.resize(recorded_string_.size()-1);
	}
	return recorded_string_;
}

}

#endif

