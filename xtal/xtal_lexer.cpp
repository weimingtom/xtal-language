
#include "xtal.h"

#ifndef XTAL_NO_PARSER

#include <math.h>

#include "xtal_lexer.h"
#include "xtal_macro.h"

namespace xtal{

#define c2(C1, C2) ((C2)<<8 | (C1))
#define c3(C1, C2, C3) ((C3)<<16 | (C2)<<8 | (C1))
#define c4(C1, C2, C3, C4) ((C4)<<24 | (C3)<<16 | (C2)<<8 | (C1))


LPCCommon::LPCCommon()
	:errors(null), ident_table(null), ident_map(null), value_table(null), source_file_name(null){}
	
void LPCCommon::init(const String& file_name){
	errors = Array();
	ident_table = Array();
	ident_map = Map();
	value_table = Array();
	source_file_name = file_name;

	register_ident(ID(""));
	value_table.push_back(null);
}

void LPCCommon::error(int_t line, const Any& message){
	if(errors.size()<10){
		errors.push_back(Xf("%(file)s:%(line)d:%(message)")(
			Named("file", source_file_name),
			Named("line", line),
			Named("message", message)
		));
	}
}

int_t LPCCommon::register_ident(const ID& ident){
	if(Any pos = ident_map[ident]){ return pos.to_i(); }
	ident_table.push_back(ident);
	ident_map.set_at(ident, ident_table.size()-1);
	return ident_table.size()-1;
}


int_t LPCCommon::register_value(const Any& v){
	if(String str = as<String>(v)){
		value_table.push_back(str.intern());
	}else{
		value_table.push_back(v);
	}
	return value_table.size()-1;
}

int_t LPCCommon::append_ident(const ID& ident){
	ident_table.push_back(ident);
	return ident_table.size()-1;
}

int_t LPCCommon::append_value(const Any& v){
	value_table.push_back(v);
	return value_table.size()-1;
}

Lexer::Lexer(){
	set_line(1);
	read_ = 0;
	pos_ = 0;
	mode_ = NORMAL_MODE;
}

void Lexer::init(const Stream& stream, const String& source_file_name){
	reader_.set_stream(stream);
	com_.init(source_file_name);
	
	keyword_map_.set_at(ID("if"), (int_t)Token::KEYWORD_IF);
	keyword_map_.set_at(ID("for"), (int_t)Token::KEYWORD_FOR);
	keyword_map_.set_at(ID("else"), (int_t)Token::KEYWORD_ELSE);
	keyword_map_.set_at(ID("fun"), (int_t)Token::KEYWORD_FUN);
	keyword_map_.set_at(ID("method"), (int_t)Token::KEYWORD_METHOD);
	keyword_map_.set_at(ID("do"), (int_t)Token::KEYWORD_DO);
	keyword_map_.set_at(ID("while"), (int_t)Token::KEYWORD_WHILE);
	keyword_map_.set_at(ID("continue"), (int_t)Token::KEYWORD_CONTINUE);
	keyword_map_.set_at(ID("break"), (int_t)Token::KEYWORD_BREAK);
	keyword_map_.set_at(ID("fiber"), (int_t)Token::KEYWORD_FIBER);
	keyword_map_.set_at(ID("yield"), (int_t)Token::KEYWORD_YIELD);
	keyword_map_.set_at(ID("return"), (int_t)Token::KEYWORD_RETURN);
	keyword_map_.set_at(ID("once"), (int_t)Token::KEYWORD_ONCE);
	keyword_map_.set_at(ID("null"), (int_t)Token::KEYWORD_NULL);
	keyword_map_.set_at(ID("false"), (int_t)Token::KEYWORD_FALSE);
	keyword_map_.set_at(ID("true"), (int_t)Token::KEYWORD_TRUE);
	keyword_map_.set_at(ID("nop"), (int_t)Token::KEYWORD_NOP);
	keyword_map_.set_at(ID("xtal"), (int_t)Token::KEYWORD_XTAL);
	keyword_map_.set_at(ID("try"), (int_t)Token::KEYWORD_TRY);
	keyword_map_.set_at(ID("catch"), (int_t)Token::KEYWORD_CATCH);
	keyword_map_.set_at(ID("finally"), (int_t)Token::KEYWORD_FINALLY);
	keyword_map_.set_at(ID("throw"), (int_t)Token::KEYWORD_THROW);
	keyword_map_.set_at(ID("class"), (int_t)Token::KEYWORD_CLASS);
	keyword_map_.set_at(ID("import"), (int_t)Token::KEYWORD_IMPORT);
	keyword_map_.set_at(ID("callee"), (int_t)Token::KEYWORD_CALLEE);
	keyword_map_.set_at(ID("this"), (int_t)Token::KEYWORD_THIS);
	keyword_map_.set_at(ID("current_context"), (int_t)Token::KEYWORD_CURRENT_CONTEXT);
	keyword_map_.set_at(ID("dofun"), (int_t)Token::KEYWORD_DOFUN);
	keyword_map_.set_at(ID("is"), (int_t)Token::KEYWORD_IS);
	keyword_map_.set_at(ID("export"), (int_t)Token::KEYWORD_EXPORT);
	keyword_map_.set_at(ID("unittest"), (int_t)Token::KEYWORD_UNITTEST);
	keyword_map_.set_at(ID("assert"), (int_t)Token::KEYWORD_ASSERT);
	keyword_map_.set_at(ID("pure"), (int_t)Token::KEYWORD_PURE);
	keyword_map_.set_at(ID("nobreak"), (int_t)Token::KEYWORD_NOBREAK);
	keyword_map_.set_at(ID("switch"), (int_t)Token::KEYWORD_SWITCH);
	keyword_map_.set_at(ID("case"), (int_t)Token::KEYWORD_CASE);
	keyword_map_.set_at(ID("default"), (int_t)Token::KEYWORD_DEFAULT);
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
		read_ += stream_.read(&buf_[pos_ & BUF_MASK], BUF_SIZE-(pos_ & BUF_MASK));
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

	
String Lexer::token2str(const Token& t){
	switch(t.type()){
		XTAL_NODEFAULT;
		
		XTAL_CASE(Token::TYPE_KEYWORD){
			Xfor2(key, value, keyword_map_.send("each_pair")){
				if(value.ivalue()==t.ivalue()){
					return String("Keyword");
				}
			}
			return String("Unknown Keyword");
		}

		XTAL_CASE(Token::TYPE_IDENT){
			return cast<String>(Xf("Identifier %s")(com_.ident_table[t.ivalue()]));
		}
				
		XTAL_CASE(Token::TYPE_INT){
			return cast<String>(Xf("Number %s")(t.ivalue()));
		}
		
		XTAL_CASE(Token::TYPE_FLOAT){
			return cast<String>(Xf("Number %s")(t.fvalue()));
		}

		XTAL_CASE(Token::TYPE_TOKEN){
			if(t.ivalue()==-1){
				return String("End of File");
			}else{
				char_t buf[5];
				buf[0] = (char_t)(t.ivalue() & 0xff);
				buf[1] = (char_t)(t.ivalue()>>8 & 0xff);
				buf[2] = (char_t)(t.ivalue()>>16 & 0xff);
				buf[3] = (char_t)(t.ivalue()>>24 & 0xff);
				buf[4] = 0;
				return String(buf);
			}
		}
	}

	return String();
}

ID Lexer::keyword2id(int_t v){
	Xfor2(key, value, keyword_map_.each_pair()){
		if(value.ivalue()==v){
			return (const String&)key;
		}
	}
	return ID("");
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
	ID id(buf);
	if(Any ret = keyword_map_[id]){
		return -ret.ivalue();
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

