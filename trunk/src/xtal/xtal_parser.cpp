#include "xtal.h"
#include "xtal_macro.h"

#ifndef XTAL_NO_PARSER

namespace xtal{

#define c2(C1, C2) ((C2)<<8 | (C1))
#define c3(C1, C2, C3) ((C3)<<16 | (C2)<<8 | (C1))
#define c4(C1, C2, C3, C4) ((C4)<<24 | (C3)<<16 | (C2)<<8 | (C1))


void CompileError::init(const StringPtr& file_name){
	errors = xnew<Array>();
	source_file_name = file_name;
}

void CompileError::error(int_t lineno, const AnyPtr& message){
	if(errors->size()<10){
		errors->push_back(Xf("%(file)s:%(lineno)d:%(message)s")(
			Named("file", source_file_name),
			Named("lineno", lineno),
			Named("message", message)
		));
	}
}

Reader::Reader()
	:stream_(null){
	pos_ = 0;
	read_ = 0;
	recording_ = false;
}

int_t Reader::read(){
	int_t ret = peek();

	if(recording_ && ret!=-1){
		recorded_string_ += (char_t)ret;
	}

	++pos_;
	return ret;
}

int_t Reader::peek(int_t n){
	uint_t rpos = read_&BUF_MASK;
	uint_t ppos = pos_&BUF_MASK;

	while(pos_+n>=read_){
		uint_t now_read = 0;

		if(ppos>rpos){
			now_read = stream_->read(&buf_[rpos], (ppos-rpos)*sizeof(char_t));
		}
		else{
			now_read = stream_->read(&buf_[rpos], (BUF_SIZE-rpos)*sizeof(char_t));
		}

		if(now_read==0){
			return -1;
		}

		read_ += now_read/sizeof(char_t);
		rpos = read_&BUF_MASK;
	}
	return buf_[(pos_+n) & BUF_MASK];
}

bool Reader::eat(int_t ch){
	if(peek()==ch){
		read();
		return true;
	}
	return false;
}

void Reader::begin_record(){
	recording_ = true;
	recorded_string_ = XTAL_STRING("");
}

StringPtr Reader::end_record(){
	recording_ = false;
	if(!recorded_string_.empty()){
		recorded_string_.resize(recorded_string_.size()-1);
	}
	return xnew<String>(recorded_string_.c_str(), recorded_string_.size());
}

Lexer::Lexer(){
	set_lineno(1);
	read_ = 0;
	pos_ = 0;

	keyword_map_ = xnew<Map>();
	keyword_map_->set_at(Xid(if), (int_t)Token::KEYWORD_IF);
	keyword_map_->set_at(Xid(for), (int_t)Token::KEYWORD_FOR);
	keyword_map_->set_at(Xid(else), (int_t)Token::KEYWORD_ELSE);
	keyword_map_->set_at(Xid(fun), (int_t)Token::KEYWORD_FUN);
	keyword_map_->set_at(Xid(method), (int_t)Token::KEYWORD_METHOD);
	keyword_map_->set_at(Xid(do), (int_t)Token::KEYWORD_DO);
	keyword_map_->set_at(Xid(while), (int_t)Token::KEYWORD_WHILE);
	keyword_map_->set_at(Xid(continue), (int_t)Token::KEYWORD_CONTINUE);
	keyword_map_->set_at(Xid(break), (int_t)Token::KEYWORD_BREAK);
	keyword_map_->set_at(Xid(fiber), (int_t)Token::KEYWORD_FIBER);
	keyword_map_->set_at(Xid(yield), (int_t)Token::KEYWORD_YIELD);
	keyword_map_->set_at(Xid(return), (int_t)Token::KEYWORD_RETURN);
	keyword_map_->set_at(Xid(once), (int_t)Token::KEYWORD_ONCE);
	keyword_map_->set_at(Xid(null), (int_t)Token::KEYWORD_NULL);
	keyword_map_->set_at(Xid(undefined), (int_t)Token::KEYWORD_UNDEFINED);
	keyword_map_->set_at(Xid(false), (int_t)Token::KEYWORD_FALSE);
	keyword_map_->set_at(Xid(true), (int_t)Token::KEYWORD_TRUE);
	keyword_map_->set_at(Xid(xtal), (int_t)Token::KEYWORD_XTAL);
	keyword_map_->set_at(Xid(try), (int_t)Token::KEYWORD_TRY);
	keyword_map_->set_at(Xid(catch), (int_t)Token::KEYWORD_CATCH);
	keyword_map_->set_at(Xid(finally), (int_t)Token::KEYWORD_FINALLY);
	keyword_map_->set_at(Xid(throw), (int_t)Token::KEYWORD_THROW);
	keyword_map_->set_at(Xid(class), (int_t)Token::KEYWORD_CLASS);
	keyword_map_->set_at(Xid(callee), (int_t)Token::KEYWORD_CALLEE);
	keyword_map_->set_at(Xid(this), (int_t)Token::KEYWORD_THIS);
	keyword_map_->set_at(Xid(current_context), (int_t)Token::KEYWORD_CURRENT_CONTEXT);
	keyword_map_->set_at(Xid(dofun), (int_t)Token::KEYWORD_DOFUN);
	keyword_map_->set_at(Xid(is), (int_t)Token::KEYWORD_IS);
	keyword_map_->set_at(Xid(in), (int_t)Token::KEYWORD_IN);
	keyword_map_->set_at(Xid(unittest), (int_t)Token::KEYWORD_UNITTEST);
	keyword_map_->set_at(Xid(assert), (int_t)Token::KEYWORD_ASSERT);
	keyword_map_->set_at(Xid(pure), (int_t)Token::KEYWORD_PURE);
	keyword_map_->set_at(Xid(nobreak), (int_t)Token::KEYWORD_NOBREAK);
	keyword_map_->set_at(Xid(switch), (int_t)Token::KEYWORD_SWITCH);
	keyword_map_->set_at(Xid(case), (int_t)Token::KEYWORD_CASE);
	keyword_map_->set_at(Xid(default), (int_t)Token::KEYWORD_DEFAULT);
	keyword_map_->set_at(Xid(singleton), (int_t)Token::KEYWORD_SINGLETON);
}	


void Lexer::init(const StreamPtr& stream, CompileError* error){
	reader_.set_stream(stream);
	error_ = error;
}

const Token& Lexer::read(){
	const Token& ret = peek();
	++pos_;
	return ret;
}

const Token& Lexer::peek(){
	if(pos_==read_){
		do_read();
		if(pos_==read_){
			static Token end(Token::TYPE_TOKEN, (int_t)-1, (int_t)0);
			return end;
		}
	}
	return buf_[pos_ & BUF_MASK];
}

void Lexer::push_token(int_t v){
	buf_[read_ & BUF_MASK] = Token(Token::TYPE_TOKEN, v, left_space_ | test_right_space(reader_.peek()));
	read_++;
}
	
void Lexer::push_int_token(int_t v){
	buf_[read_ & BUF_MASK] = Token(Token::TYPE_INT, v, left_space_ | test_right_space(reader_.peek()));
	read_++;
}

void Lexer::push_float_token(float_t v){
	buf_[read_ & BUF_MASK] = Token(Token::TYPE_FLOAT, v, left_space_ | test_right_space(reader_.peek()));
	read_++;
}
	
void Lexer::push_keyword_token(const IDPtr& v, int_t num){
	buf_[read_ & BUF_MASK] = Token(Token::TYPE_KEYWORD, v, left_space_ | test_right_space(reader_.peek()), num);
	read_++;
}
	
void Lexer::push_identifier_token(const IDPtr& v){
	buf_[read_ & BUF_MASK] = Token(Token::TYPE_IDENTIFIER, v, left_space_ | test_right_space(reader_.peek()));
	read_++;
}

void Lexer::putback(){
	pos_--;
}

void Lexer::putback(const Token& ch){
	pos_--;
	buf_[pos_ & BUF_MASK] = ch;
}

IDPtr Lexer::parse_identifier(){
	string_t buf;

	ChMaker chm;
	while(!chm.is_completed()){
		chm.add(reader_.read());
	}
	buf.append(chm.buf, chm.buf+chm.pos);

	while(test_ident_rest(reader_.peek())){
		chm.clear();
		while(!chm.is_completed()){
			chm.add(reader_.read());
		}
		buf.append(chm.buf, chm.buf+chm.pos);
	}
	
	return xnew<ID>(buf);
}

int_t Lexer::parse_integer(){
	int_t ret = 0;
	for(;;){
		if(test_digit(reader_.peek())){
			ret *= 10;
			ret += reader_.read()-'0';
		}
		else if(reader_.peek()=='_'){
			reader_.read();
		}
		else{
			break;
		}
	}
	return ret;
}

int_t Lexer::parse_hex(){
	int_t ret = 0;
	for(;;){
		if(test_digit(reader_.peek())){
			ret *= 16;
			ret += reader_.read()-'0';
		}
		else if(test_range(reader_.peek(), 'a', 'f')){
			ret *= 16;
			ret += reader_.read()-'a';
		}
		else if(test_range(reader_.peek(), 'A', 'F')){
			ret *= 16;
			ret += reader_.read()-'A';
		}
		else if(reader_.peek()=='_'){
			reader_.read();
		}
		else{
			break;
		}
	}

	if(test_ident_rest(reader_.peek())){
		error_->error(lineno(), Xt("Xtal Compile Error 1015")(Named("n", 16)));
	}

	return ret;		
}

int_t Lexer::parse_oct(){
	int_t ret = 0;
	for(;;){
		if(test_range(reader_.peek(), '0', '7')){
			ret *= 8;
			ret += reader_.read()-'0';
		}
		else if(reader_.peek()=='_'){
			reader_.read();
		}
		else{
			break;
		}
	}

	if(test_ident_rest(reader_.peek()) || ('8'<=reader_.peek() && reader_.peek()<='9')){
		error_->error(lineno(), Xt("Xtal Compile Error 1015")(Named("n", 8)));
	}

	return ret;		
}

int_t Lexer::parse_bin(){
	int ret = 0;
	for(;;){
		if(test_range(reader_.peek(), '0', '1')){
			ret *= 2;
			ret += reader_.read()-'0';
		}
		else if(reader_.peek()=='_'){
			reader_.read();
		}
		else{
			break;
		}
	}

	if(test_ident_rest(reader_.peek()) || ('2'<=reader_.peek() && reader_.peek()<='9')){
		error_->error(lineno(), Xt("Xtal Compile Error 1015")(Named("n", 2)));
	}

	return ret;
}

void Lexer::parse_number_suffix(int_t val){
	if(reader_.eat('f') || reader_.eat('F')){
		push_float_token((float_t)val);	
	}
	else{

		if(test_ident_rest(reader_.peek())){
			error_->error(lineno(), Xt("Xtal Compile Error 1010"));
		}

		push_int_token(val);
	}
}

void Lexer::parse_number_suffix(float_t val){
	if(reader_.eat('f') || reader_.eat('F')){
		push_float_token(val);	
	}
	else{
	
		if(test_ident_rest(reader_.peek())){
			error_->error(lineno(), Xt("Xtal Compile Error 1010"));
		}

		push_float_token(val);
	}
}

void Lexer::parse_number(){
	if(reader_.eat('0')){
		if(reader_.eat('x') || reader_.eat('X')){
			push_int_token(parse_hex());
			return;
		}
		else if(reader_.eat('o')){
			push_int_token(parse_oct());
			return;
		}
		else if(reader_.eat('b') || reader_.eat('B')){
			push_int_token(parse_bin());
			return;
		}
	}
	
	int_t ival = parse_integer();
	
	if(reader_.peek()=='.'){
		if(test_digit(reader_.peek(1))){
			reader_.read();

			float_t scale = 1;
			while(reader_.eat('0')){
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
			if(reader_.eat('e') || reader_.eat('E')){
				if(reader_.eat('-')){
					e=-1;
				}
				else{
					reader_.eat('+');
				}

				if(!test_digit(reader_.peek())){
					error_->error(lineno(), Xt("Xtal Compile Error 1014"));
				}

				e *= parse_integer();

				{
					using namespace std;
					fval *= (float_t)pow((float_t)10, (float_t)e);
				}
			}
			parse_number_suffix(fval);
		}
		else{
			push_int_token(ival);
		}
	}
	else{
		parse_number_suffix(ival);
	}
}
	
void Lexer::do_read(){
	left_space_ = 0;
	
	do{

		int_t ch = reader_.peek();
		
		switch(ch){

			XTAL_DEFAULT{

				if(ch!=-1 && test_ident_first(ch)){
					IDPtr identifier = parse_identifier();
					if(const AnyPtr& key = keyword_map_->at(identifier)){
						push_keyword_token(identifier, ivalue(key));
					}
					else{
						push_identifier_token(identifier);
					}
				}
				else if(test_digit(ch)){
					parse_number();
					return;
				}
				else{
					reader_.read();
					push_token(ch);
				}
			}
			
			XTAL_CASE('+'){ reader_.read();
				if(reader_.eat('+')){ push_token(c2('+', '+')); }
				else if(reader_.eat('=')){ push_token(c2('+', '=')); }
				else{ push_token('+'); }
			}
			
			XTAL_CASE('-'){ reader_.read();
				if(reader_.eat('-')){ push_token(c2('-', '-')); }
				else if(reader_.eat('=')){ push_token(c2('-', '=')); }
				else{ push_token('-'); }
			}
			
			XTAL_CASE('~'){ reader_.read();
				if(reader_.eat('=')){ push_token(c2('~', '=')); }
				else{ push_token('~'); }
			}
			
			XTAL_CASE('*'){ reader_.read();
				if(reader_.eat('=')){ push_token(c2('*', '=')); }
				else{ push_token('*'); }
			}
			
			XTAL_CASE('/'){ reader_.read();
				if(reader_.eat('=')){
					push_token(c2('/', '='));
				}
				else if(reader_.eat('/')){
					for(;;){
						int_t ch = reader_.read();
						if(ch=='\r'){
							reader_.eat('\n');
							set_lineno(lineno()+1);
							left_space_ = Token::FLAG_LEFT_SPACE;
							break;
						}
						else if(ch=='\n'){
							set_lineno(lineno()+1);
							left_space_ = Token::FLAG_LEFT_SPACE;
							break;
						}
						else if(ch==-1){
							break;
						}
					}
					continue;
				}
				else if(reader_.eat('*')){
					for(;;){
						int_t ch = reader_.read();
						if(ch=='\r'){
							reader_.eat('\n');
							set_lineno(lineno()+1);
						}
						else if(ch=='\n'){
							set_lineno(lineno()+1);
						}
						else if(ch=='*'){
							if(reader_.eat('/')){
								left_space_ = Token::FLAG_LEFT_SPACE;
								break;
							}
						}
						else if(ch==-1){
							error_->error(lineno(), Xt("Xtal Compile Error 1021"));
							break;
						}
					}
					continue;
				}
				else{
					push_token('/');
				}
			}			
			
			XTAL_CASE('#'){
				reader_.read();
				if(reader_.eat('!')){
					for(;;){
						int_t ch = reader_.read();
						if(ch=='\r'){
							reader_.eat('\n');
							set_lineno(lineno()+1);
							left_space_ = Token::FLAG_LEFT_SPACE;
							break;
						}
						else if(ch=='\n'){
							set_lineno(lineno()+1);
							left_space_ = Token::FLAG_LEFT_SPACE;
							break;
						}
						else if(ch==-1){
							break;
						}
					}
					continue;
				}
				else{
					push_token('#');
				}
			}			
			
			XTAL_CASE('%'){ reader_.read();
				if(reader_.eat('=')){ push_token(c2('%', '=')); }
				else{ push_token('%'); }
			}
			
			XTAL_CASE('&'){ reader_.read();
				if(reader_.eat('&')){ push_token(c2('&', '&')); }
				else{ push_token('&'); }
			}
			
			XTAL_CASE('|'){ reader_.read();
				if(reader_.eat('|')){ push_token(c2('|', '|')); }
				else{ push_token('|'); }
			}
						
			XTAL_CASE('>'){ reader_.read();
				if(reader_.eat('>')){
					if(reader_.eat('>')){
						if(reader_.eat('=')){
							push_token(c4('>','>','>','='));
						}
						else{
							push_token(c3('>','>','>'));
						}
					}
					else{
						push_token(c2('>','>'));
					}
				}
				else if(reader_.eat('=')){
					push_token(c2('>', '='));
				}
				else{
					push_token('>');
				}
			}
			
			XTAL_CASE('<'){ reader_.read();
				if(reader_.eat('<')){
					if(reader_.eat('=')){
						push_token(c3('<','<','='));
					}
					else{
						push_token(c2('<','<'));
					}
				}
				else if(reader_.eat('=')){
					push_token(c2('<', '='));
				}
				else if(reader_.eat('.')){
					if(!reader_.eat('.')){
						error_->error(lineno(), Xt("Xtal Compile Error 1001"));					
					}

					if(reader_.eat('<')){
						push_token(c4('<', '.', '.', '<'));
					}
					else{
						push_token(c3('<', '.', '.'));
					}
				}
				else{
					push_token('<');
				}
			}
			
			XTAL_CASE('='){ reader_.read();
				if(reader_.eat('=')){
					if(reader_.eat('=')){
						push_token(c3('=', '=', '='));
					}
					else{
						push_token(c2('=', '='));
					}
				}
				else{
					push_token('=');
				}
			}
			
			XTAL_CASE('!'){ reader_.read();
				if(reader_.eat('=')){
					if(reader_.eat('=')){
						push_token(c3('!', '=', '='));
					}
					else{
						push_token(c2('!', '='));
					}
				}
				else if(reader_.peek()=='i'){
					if(reader_.peek(1)=='s'){
						if(!test_ident_rest(reader_.peek(2))){
							reader_.read();
							reader_.read();
							push_token(c3('!', 'i', 's'));
						}
						else{
							push_token('!');
						}
					}
					else if(reader_.peek(1)=='n'){
						if(!test_ident_rest(reader_.peek(2))){
							reader_.read();
							reader_.read();
							push_token(c3('!', 'i', 'n'));
						}
						else{
							push_token('!');
						}
					}
					else{
						push_token('!');
					}
				}
				else{
					push_token('!');
				}
			}
			
			XTAL_CASE('.'){ reader_.read();
				if(reader_.eat('.')){
					if(reader_.eat('.')){ push_token(c3('.', '.', '.')); }
					else if(reader_.eat('<')){ push_token(c3('.', '.', '<')); }
					else{ push_token(c2('.', '.')); }
				}
				else if(reader_.eat('?')){ push_token(c2('.', '?')); }
				else{ push_token('.'); }
			}
			
			XTAL_CASE(':'){ reader_.read();
				if(reader_.eat(':')){
					if(reader_.eat('?')){ push_token(c3(':', ':', '?')); }
					else{ push_token(c2(':', ':')); }
				}
				else{ push_token(':'); }
			}

			XTAL_CASE('\''){ reader_.read();
				push_identifier_token(read_string('\'', '\''));
			}

			XTAL_CASE4(' ', '\t', '\r', '\n'){
				deplete_space();
				left_space_ = Token::FLAG_LEFT_SPACE;
				continue;
			}

			XTAL_CASE((char_t)239){
				if(reader_.peek(1)==(char_t)187 && reader_.peek(2)==(char_t)191){
					reader_.read();
					reader_.read();
					reader_.read();
					continue;
				}
				push_token(ch);
			}

			XTAL_CASE(-1){
				push_token(-1);
			}
		}
			
		break;
	}while(true);

}

void Lexer::deplete_space(){
	for(;;){
		int_t ch = reader_.peek();
		if(ch=='\r'){
			reader_.read();
			reader_.eat('\n');
			set_lineno(lineno()+1);
		}
		else if(ch=='\n'){
			reader_.read();
			set_lineno(lineno()+1);
		}
		else if(ch==' ' || ch=='\t'){
			reader_.read();
		}
		else{
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

int_t Lexer::read_direct(){
	return reader_.read();
}

StringPtr Lexer::read_string(int_t open, int_t close){
	string_t str;
	int_t depth = 1;
	for(;;){
		int_t ch = reader_.read();
		if(ch==close){
			--depth;
			if(depth==0){
				break;
			}
		}
		if(ch==open){
			++depth;
		}
		if(ch==-1){
			error_->error(lineno(), Xt("Xtal Compile Error 1011"));
			break;
		}
		if(ch=='\\'){
			switch(reader_.peek()){
				XTAL_DEFAULT{ 
					str+='\\';
					str+=(char_t)reader_.peek(); 
				}
				
				XTAL_CASE('n'){ str+='\n'; }
				XTAL_CASE('r'){ str+='\r'; }
				XTAL_CASE('t'){ str+='\t'; }
				XTAL_CASE('f'){ str+='\f'; }
				XTAL_CASE('b'){ str+='\b'; }
				XTAL_CASE('\\'){ str+='\\'; }
				XTAL_CASE('"'){ str+='"'; } 
				
				XTAL_CASE('\r'){ 
					if(reader_.peek()=='\n'){
						reader_.read();
					}

					str+='\r';
					str+='\n';
					set_lineno(lineno()+1);
				}
				
				XTAL_CASE('\n'){ 
					str+='\n';
					set_lineno(lineno()+1);
				}
			}
			reader_.read();
		}
		else{
			if(ch=='\r'){
				if(reader_.peek()=='\n'){
					reader_.read();
				}

				str+='\r';
				str+='\n';
				set_lineno(lineno()+1);
			}
			else if(ch=='\n'){
				str+='\n';
				set_lineno(lineno()+1);
			}
			else{
				ChMaker chm;
				chm.add(ch);
				while(!chm.is_completed()){
					chm.add(reader_.read());
				}
				str.append(chm.buf, chm.buf+chm.pos);
			}
		}	
	}

	return xnew<ID>(str.c_str(), str.size());
}


enum{//Expressions priority
	
	PRI_BEGIN_ = 0x1000,

	PRI_LOOP,

	PRI_Q,

	PRI_OROR,
	PRI_ANDAND,

	PRI_OR,
	PRI_XOR,
	PRI_AND,

	PRI_EQ,
		PRI_NE = PRI_EQ,
		PRI_LT = PRI_EQ,
		PRI_GT = PRI_EQ,
		PRI_LE = PRI_EQ,
		PRI_GE = PRI_EQ,
		PRI_RAWEQ = PRI_EQ,
		PRI_RAWNE = PRI_EQ,
		PRI_IN = PRI_EQ,
		PRI_NIN = PRI_EQ,
		PRI_IS = PRI_EQ,
		PRI_NIS = PRI_EQ,

	PRI_SHL,
		PRI_SHR = PRI_SHL,
		PRI_USHR = PRI_SHL,

	PRI_ADD, 
		PRI_SUB = PRI_ADD, 
		PRI_CAT = PRI_ADD,
	
	PRI_MUL, 
		PRI_DIV = PRI_MUL, 
		PRI_MOD = PRI_MUL,

	PRI_NEG,
		PRI_POS = PRI_NEG,
		PRI_COM = PRI_NEG,
		PRI_NOT = PRI_NEG,

	PRI_POW,

	PRI_AT,
		PRI_SEND = PRI_AT,
		PRI_MEMBER = PRI_AT,
		PRI_CALL = PRI_AT,
		PRI_NS = PRI_AT,
		PRI_RANGE = PRI_AT,

	PRI_ONCE,

	PRI_END_,

	PRI_MAX = PRI_END_-PRI_BEGIN_
};




Parser::Parser(){
	expr_end_flag_ = false;
}

ExprPtr Parser::parse(const StreamPtr& stream, CompileError* error){
	error_ = error;
	lexer_.init(stream, error_);

	parse_toplevel();

	if(error_->errors->size()!=0){
		return null;
	}

	return ep(eb_.pop());
}

ExprPtr Parser::parse_stmt(const StreamPtr& stream, CompileError* error){
	error_ = error;
	lexer_.init(stream, error_);

	if(!parse_stmt()){
		lexer_read();
	}

	if(error_->errors->size()!=0){
		return null;
	}

	return ep(eb_.pop());
}

const Token& Parser::lexer_read(){
	eb_.set_lineno(lexer_.lineno());
	return lexer_.read();
}

const Token& Parser::lexer_peek(){
	eb_.set_lineno(lexer_.lineno());
	return lexer_.peek();
}

void Parser::expect(int_t ch){
	if(eat(ch)){
		return;
	}		
	const Token& tok = lexer_read();
	error_->error(lineno(), Xt("Xtal Compile Error 1002")(Named("char", lexer_peek().to_s())));
}

bool Parser::eat(int_t ch){
	const Token& n = lexer_peek();
	if(n.type() == Token::TYPE_TOKEN){
		if(n.ivalue()==ch){
			lexer_read();
			return true;
		}
	}
	return false;
}

bool Parser::eat(Token::Keyword kw){
	const Token& n = lexer_peek();
	if(n.type() == Token::TYPE_KEYWORD){
		if(n.keyword_number()==kw){
			lexer_read();
			return true;
		}
	}
	return false;
}

bool Parser::parse_term(){
	const Token& ch = lexer_read();
	int_t r_space = ch.right_space() ? PRI_MAX : 0;

	switch(ch.type()){
		XTAL_NODEFAULT;

		XTAL_CASE(Token::TYPE_TOKEN){
			switch(ch.ivalue()){

				XTAL_DEFAULT{}

				XTAL_CASE('('){ 
					if(parse_expr()){
						if(eat(',')){
							parse_exprs(true);
							eb_.splice(EXPR_MULTI_VALUE, 1);
						}
						else{
							if(ep(eb_.back())->tag()==EXPR_SEND){
								eb_.splice(EXPR_BRACKET, 1);
							}	
						}
					}
					else{
						eb_.splice(EXPR_UNDEFINED, 0);
					}
					expect(')'); 
					expr_end_flag_ = false; 
					return true; 
				}

				XTAL_CASE('['){ parse_array();  expr_end_flag_ = false; return true; }
				XTAL_CASE('|'){ parse_lambda(); return true; }
				XTAL_CASE(c2('|', '|')){ parse_lambda(true); return true; }

				XTAL_CASE('_'){ must_parse_identifier(); eb_.splice(EXPR_IVAR, 1); return true; }

				XTAL_CASE('"'){ 
					eb_.push(KIND_STRING);
					eb_.push(lexer_.read_string('"', '"'));
					eb_.splice(EXPR_STRING, 2);
					return true; 
				}
				
				XTAL_CASE('%'){
					int_t ch = lexer_.read_direct();
					int_t kind = KIND_STRING;

					if(ch=='t'){
						kind = KIND_TEXT;
						ch = lexer_.read_direct();
					}
					else if(ch=='f'){
						kind = KIND_FORMAT;
						ch = lexer_.read_direct();
					}

					int_t open = ch;
					int_t close = 0;

					switch(open){
						case '!': case '?': case '"': case '&': //"
						case '#': case '\'':case '|': case ':':
						case '^': case '+': case '-': case '*':
						case '/': case '@': case '$': case '.':
						case '=': case '~': case '`': case ';':
							close = open; break;

						case '(': close = ')'; break;
						case '<': close = '>'; break;
						case '{': close = '}'; break;
						case '[': close = ']'; break;

						default:
							close = open;
							error_->error(lineno(), Xt("Xtal Compile Error 1017"));
							break;
					}

					eb_.push(kind);
					eb_.push(lexer_.read_string(open, close));
					eb_.splice(EXPR_STRING, 2);
					return true; 
				}
				
				XTAL_CASE(c3('.','.','.')){ eb_.splice(EXPR_ARGS, 0); return true; }

////////////////////////////////////////////////////////////////////////////////////////

				XTAL_CASE('+'){ must_parse_expr(PRI_POS - r_space); eb_.splice(EXPR_POS, 1); return true; }
				XTAL_CASE('-'){ must_parse_expr(PRI_NEG - r_space); eb_.splice(EXPR_NEG, 1); return true; }
				XTAL_CASE('~'){ must_parse_expr(PRI_COM - r_space); eb_.splice(EXPR_COM, 1); return true; }
				XTAL_CASE('!'){ must_parse_expr(PRI_NOT - r_space); eb_.splice(EXPR_NOT, 1); return true; }
			}
		}

		XTAL_CASE(Token::TYPE_KEYWORD){
			switch(ch.keyword_number()){

				XTAL_DEFAULT{}
				
				XTAL_CASE(Token::KEYWORD_ONCE){ must_parse_expr(PRI_ONCE - r_space*2); eb_.splice(EXPR_ONCE, 1); return true; }
				XTAL_CASE(Token::KEYWORD_CLASS){ parse_class(KIND_CLASS); return true; }
				XTAL_CASE(Token::KEYWORD_SINGLETON){ parse_class(KIND_SINGLETON); return true; }
				XTAL_CASE(Token::KEYWORD_FUN){ parse_fun(KIND_FUN); return true; }
				XTAL_CASE(Token::KEYWORD_METHOD){ parse_fun(KIND_METHOD); return true; }
				XTAL_CASE(Token::KEYWORD_FIBER){ parse_fun(KIND_FIBER); return true; }
				XTAL_CASE(Token::KEYWORD_CALLEE){ eb_.splice(EXPR_CALLEE, 0); return true; }
				XTAL_CASE(Token::KEYWORD_NULL){ eb_.splice(EXPR_NULL, 0); return true; }
				XTAL_CASE(Token::KEYWORD_UNDEFINED){ eb_.splice(EXPR_UNDEFINED, 0); return true; }
				XTAL_CASE(Token::KEYWORD_TRUE){ eb_.splice(EXPR_TRUE, 0); return true; }
				XTAL_CASE(Token::KEYWORD_FALSE){ eb_.splice(EXPR_FALSE, 0); return true; }
				XTAL_CASE(Token::KEYWORD_THIS){ eb_.splice(EXPR_THIS, 0); return true; }
				XTAL_CASE(Token::KEYWORD_CURRENT_CONTEXT){ eb_.splice(EXPR_CURRENT_CONTEXT, 0); return true; }

				XTAL_CASE(Token::KEYWORD_DOFUN){ 
					parse_fun(KIND_FUN);
					eb_.push(null);
					eb_.push(null);
					eb_.splice(EXPR_CALL, 3);
					return true; 
				}
			}
		}
		
		XTAL_CASE(Token::TYPE_INT){ eb_.push(ch.ivalue()); eb_.splice(EXPR_INT, 1); return true; }
		XTAL_CASE(Token::TYPE_FLOAT){ eb_.push(ch.fvalue()); eb_.splice(EXPR_FLOAT, 1); return true; }
		XTAL_CASE(Token::TYPE_IDENTIFIER){ eb_.push(ch.identifier()); eb_.splice(EXPR_LVAR, 1); return true; }
	}

	lexer_.putback(ch);
	return false;
}

bool Parser::parse_post(int_t pri){
	if(expr_end_flag_){
		const Token& ch = lexer_peek();

		if(ch.type()==Token::TYPE_TOKEN && (ch.ivalue()=='.' || ch.ivalue()==c2('.','?'))){
			expr_end_flag_ = false;
		}
		else{
			return false;
		}
	}

	Token ch = lexer_read();
	int_t r_space = (ch.right_space()) ? PRI_MAX : 0;
	int_t l_space = (ch.left_space()) ? PRI_MAX : 0;

	switch(ch.type()){
	
		XTAL_DEFAULT{}
		
		XTAL_CASE(Token::TYPE_KEYWORD){
			switch(ch.keyword_number()){
				XTAL_DEFAULT{}
				
				XTAL_CASE(Token::KEYWORD_IS){ if(pri < PRI_IS - l_space){ must_parse_expr(PRI_IS - r_space); eb_.splice(EXPR_IS, 2); return true; } }
				XTAL_CASE(Token::KEYWORD_IN){ if(pri < PRI_IN - l_space){ must_parse_expr(PRI_IN - r_space); eb_.splice(EXPR_IN, 2); return true; } }
			}
		}
		
		XTAL_CASE(Token::TYPE_TOKEN){
			switch(ch.ivalue()){

				XTAL_DEFAULT{}
			
				XTAL_CASE('+'){ if(pri < PRI_ADD - l_space){ must_parse_expr(PRI_ADD - l_space); eb_.splice(EXPR_ADD, 2); return true; } }
				XTAL_CASE('-'){ if(pri < PRI_SUB - l_space){ must_parse_expr(PRI_SUB - l_space); eb_.splice(EXPR_SUB, 2); return true; } }
				XTAL_CASE('~'){ if(pri < PRI_CAT - l_space){ must_parse_expr(PRI_CAT - l_space); eb_.splice(EXPR_CAT, 2); return true; } } 
				XTAL_CASE('*'){ if(pri < PRI_MUL - l_space){ must_parse_expr(PRI_MUL - l_space); eb_.splice(EXPR_MUL, 2); return true; } } 
				XTAL_CASE('/'){ if(pri < PRI_DIV - l_space){ must_parse_expr(PRI_DIV - l_space); eb_.splice(EXPR_DIV, 2); return true; } } 
				XTAL_CASE('%'){ if(pri < PRI_MOD - l_space){ must_parse_expr(PRI_MOD - l_space); eb_.splice(EXPR_MOD, 2); return true; } } 
				XTAL_CASE('^'){ if(pri < PRI_XOR - l_space){ must_parse_expr(PRI_XOR - l_space); eb_.splice(EXPR_XOR, 2); return true; } } 
				XTAL_CASE(c2('&','&')){ if(pri < PRI_ANDAND - l_space){ must_parse_expr(PRI_ANDAND - l_space); eb_.splice(EXPR_ANDAND, 2); return true; } } 
				XTAL_CASE('&'){ if(pri < PRI_AND - l_space){ must_parse_expr(PRI_AND - l_space); eb_.splice(EXPR_AND, 2); return true; } } 
				XTAL_CASE(c2('|','|')){ if(pri < PRI_OROR - l_space){ must_parse_expr(PRI_OROR - l_space); eb_.splice(EXPR_OROR, 2); return true; } } 
				XTAL_CASE('|'){ if(pri < PRI_OR - l_space){ must_parse_expr(PRI_OR - l_space); eb_.splice(EXPR_OR, 2); return true; } } 
				XTAL_CASE(c2('<','<')){ if(pri < PRI_SHL - l_space){ must_parse_expr(PRI_SHL - l_space); eb_.splice(EXPR_SHL, 2); return true; } } 
				XTAL_CASE(c2('>','>')){ if(pri < PRI_SHR - l_space){ must_parse_expr(PRI_SHR - l_space); eb_.splice(EXPR_SHR, 2); return true; } } 
				XTAL_CASE(c3('>','>','>')){ if(pri < PRI_USHR - l_space){ must_parse_expr(PRI_USHR - l_space); eb_.splice(EXPR_USHR, 2); return true; } } 
				XTAL_CASE(c2('<','=')){ if(pri < PRI_LE - l_space){ must_parse_expr(PRI_LE - l_space); eb_.splice(EXPR_LE, 2); return true; } } 
				XTAL_CASE('<'){ if(pri < PRI_LT - l_space){ must_parse_expr(PRI_LT - l_space); eb_.splice(EXPR_LT, 2); return true; } } 
				XTAL_CASE(c2('>','=')){ if(pri < PRI_GE - l_space){ must_parse_expr(PRI_GE - l_space); eb_.splice(EXPR_GE, 2); return true; } } 
				XTAL_CASE('>'){ if(pri < PRI_GT - l_space){ must_parse_expr(PRI_GT - l_space); eb_.splice(EXPR_GT, 2); return true; } } 
				XTAL_CASE(c2('=','=')){ if(pri < PRI_EQ - l_space){ must_parse_expr(PRI_EQ - l_space); eb_.splice(EXPR_EQ, 2); return true; } } 
				XTAL_CASE(c2('!','=')){ if(pri < PRI_NE - l_space){ must_parse_expr(PRI_NE - l_space); eb_.splice(EXPR_NE, 2); return true; } } 
				XTAL_CASE(c3('=','=','=')){ if(pri < PRI_RAWEQ - l_space){ must_parse_expr(PRI_RAWEQ - l_space); eb_.splice(EXPR_RAWEQ, 2); return true; } } 
				XTAL_CASE(c3('!','=','=')){ if(pri < PRI_RAWNE - l_space){ must_parse_expr(PRI_RAWNE - l_space); eb_.splice(EXPR_RAWNE, 2); return true; } } 
				XTAL_CASE(c3('!','i','s')){ if(pri < PRI_NIS - l_space){ must_parse_expr(PRI_NIS - l_space); eb_.splice(EXPR_NIS, 2); return true; } }
				XTAL_CASE(c3('!','i','n')){ if(pri < PRI_NIN - l_space){ must_parse_expr(PRI_NIN - l_space); eb_.splice(EXPR_NIN, 2); return true; } }

				XTAL_CASE4(c2(':',':'), '.', c3(':',':','?'), c2('.', '?')){
					if(pri < PRI_MEMBER - l_space){
						if(eat('(')){
							must_parse_expr();
							expect(')');
						}
						else{
							parse_identifier_or_keyword();
						}

						int_t r_space = (lexer_peek().right_space() || lexer_peek().left_space()) ? PRI_MAX : 0;
						if(eat('#')){
							must_parse_expr(PRI_NS - r_space);
						}
						else{
							eb_.push(null);
						}

						switch(ch.ivalue()){
							XTAL_NODEFAULT;
							XTAL_CASE(c2(':',':')){ eb_.splice(EXPR_MEMBER, 3); }
							XTAL_CASE('.'){ eb_.splice(EXPR_SEND, 3); }
							XTAL_CASE(c3(':',':','?')){ eb_.splice(EXPR_MEMBER_Q, 3); }
							XTAL_CASE(c2('.', '?')){ eb_.splice(EXPR_SEND_Q, 3); }
						}
						return true;
					}
				}

				XTAL_CASE('?'){
					if(pri < PRI_Q - l_space){
						must_parse_expr();
						expect(':');
						must_parse_expr();
						eb_.splice(EXPR_Q, 3);
						return true;
					}
				}
				
				XTAL_CASE4(c2('.', '.'), c3('.', '.', '<'), c3('<', '.', '.'), c4('<', '.', '.', '<')){
					if(pri < PRI_RANGE - l_space){
						must_parse_expr(PRI_RANGE - r_space);
						switch(ch.ivalue()){
							XTAL_NODEFAULT;
							XTAL_CASE(c2('.', '.')){ eb_.insert(2, RANGE_CLOSED); }
							XTAL_CASE(c3('.', '.', '<')){ eb_.insert(2, RANGE_LEFT_CLOSED_RIGHT_OPEN); }
							XTAL_CASE(c3('<', '.', '.')){ eb_.insert(2, RANGE_LEFT_OPEN_RIGHT_CLOSED); }
							XTAL_CASE(c4('<', '.', '.', '<')){ eb_.insert(2, RANGE_OPEN); }
						}
						eb_.splice(EXPR_RANGE, 3);
						return true;
					}
				}

				XTAL_CASE('('){
					if(pri < PRI_CALL - l_space){
						parse_call();
						return true;
					}
				}

				XTAL_CASE('['){
					if(pri < PRI_AT - l_space){
						if(eat(':')){
							expect(']');
							eb_.push(Xid(to_m));
							eb_.push(null);
							eb_.splice(EXPR_SEND, 3);
						}
						else if(eat(']')){
							eb_.push(Xid(to_a));
							eb_.push(null);
							eb_.splice(EXPR_SEND, 3);
						}
						else{
							must_parse_expr();
							eb_.splice(EXPR_AT, 2);
							expect(']');
						}
						return true;
					}
				}
			}
		}
	}

	lexer_.putback(ch);
	return false;
}


void Parser::parse_each(){
	ExprPtr lhs = ep(eb_.pop());
	IDPtr label = ptr_as<ID>(eb_.pop());

	ExprPtr params = xnew<Expr>();

	eb_.push(Xid(iterator));
	eb_.splice(EXPR_LVAR, 1);
	params->push_back(eb_.pop());

	if(eat('|')){ // ブロックパラメータ
		for(;;){
			const Token& ch = lexer_peek();
			if(ch.type()==ch.TYPE_IDENTIFIER){
				lexer_read();
				eb_.push(ch.identifier());
				eb_.splice(EXPR_LVAR, 1);
				params->push_back(eb_.pop());
				if(!eat(',')){
					expect('|');
					break;
				}
			}
			else{
				expect('|');
				break;
			}
		}
	}
	
	if(params->size()==1){
		eb_.push(Xid(it));
		eb_.splice(EXPR_LVAR, 1);
		params->push_back(eb_.pop());
	}

	ExprPtr scope = xnew<Expr>();
	
	{
		eb_.push(params);
		eb_.push(lhs);
		eb_.push(Xid(block_first));
		eb_.push(null);
		eb_.splice(EXPR_SEND, 3);
		eb_.splice(0, 1);
		eb_.splice(EXPR_MDEFINE, 2);
	}

	scope->push_back(eb_.pop());

	{
		eb_.push(label);

		eb_.push(null);

		{
			eb_.push(Xid(iterator));
			eb_.splice(EXPR_LVAR, 1);
		}

		{
			eb_.push(params);
			eb_.push(Xid(iterator));
			eb_.splice(EXPR_LVAR, 1);
			eb_.push(Xid(block_next));
			eb_.push(null);
			eb_.splice(EXPR_SEND, 3);
			eb_.splice(0, 1);
			eb_.splice(EXPR_MASSIGN, 2);
		}

		parse_scope();

		if(eat(Token::KEYWORD_ELSE)){
			must_parse_stmt();
			eb_.push(null);
		}
		else if(eat(Token::KEYWORD_NOBREAK)){
			eb_.push(null);
			must_parse_stmt();
		}
		else{
			eb_.push(null);
			eb_.push(null);
		}

		eb_.splice(EXPR_FOR, 7);
	}

	ExprPtr loop = ep(eb_.pop());

	{
		eb_.push(Xid(iterator));
		eb_.splice(EXPR_LVAR, 1);
		eb_.push(Xid(block_catch));
		eb_.push(null);
		eb_.splice(EXPR_SEND_Q, 3);
		eb_.push(null);
		eb_.push(Xid(e));
		eb_.splice(EXPR_LVAR, 1);
		eb_.splice(0, 2);
		eb_.splice(0, 1);
		eb_.push(null);
		eb_.splice(EXPR_CALL, 3);
		eb_.splice(EXPR_NOT, 1);

		eb_.push(Xid(e));
		eb_.splice(EXPR_LVAR, 1);
		eb_.splice(EXPR_THROW, 1);

		eb_.push(null);

		eb_.splice(EXPR_IF, 3);
	}

	ExprPtr block_catch = ep(eb_.pop());

	eb_.push(loop);

	eb_.push(Xid(e));
	eb_.push(block_catch);

	eb_.push(Xid(iterator));
	eb_.splice(EXPR_LVAR, 1);
	eb_.push(Xid(block_break));
	eb_.push(null);
	eb_.splice(EXPR_SEND_Q, 3);

	eb_.splice(EXPR_TRY, 4);
	
	scope->push_back(eb_.pop());
	eb_.push(scope);
	eb_.splice(EXPR_SCOPE, 1);
}

void Parser::parse_for(){
	expect('(');
	if(!parse_assign_stmt()) eb_.push(null);
	expect(';');

	if(!parse_expr()) eb_.push(null);
	expect(';');

	if(!parse_assign_stmt()) eb_.push(null);
	expect(')');

	must_parse_stmt();

	if(eat(Token::KEYWORD_ELSE)){
		must_parse_stmt();
		eb_.push(null);
	}
	else if(eat(Token::KEYWORD_NOBREAK)){
		eb_.push(null);
		must_parse_stmt();
	}
	else{
		eb_.push(null);
		eb_.push(null);
	}

	eb_.splice(EXPR_FOR, 7);
}

void Parser::parse_while(){
	eb_.push(null);
	expect('(');

	must_parse_expr();
	expect(')');

	eb_.push(null);
	
	must_parse_stmt();
	
	if(eat(Token::KEYWORD_ELSE)){
		must_parse_stmt();
		eb_.push(null);
	}
	else if(eat(Token::KEYWORD_NOBREAK)){
		eb_.push(null);
		must_parse_stmt();
	}
	else{
		eb_.push(null);
		eb_.push(null);
	}

	eb_.splice(EXPR_FOR, 7);
}

bool Parser::parse_loop(){
	if(parse_var()){
		const Token& ch = lexer_read(); // :の次を読み取る
		if(ch.type()==Token::TYPE_KEYWORD){
			switch(ch.keyword_number()){
				XTAL_DEFAULT{}
				XTAL_CASE(Token::KEYWORD_FOR){ parse_for(); return true; }
				XTAL_CASE(Token::KEYWORD_WHILE){ parse_while(); return true; }
			}
		}

		lexer_.putback(ch);
		if(parse_expr()){
			if(!expr_end_flag_ && eat('{')){
				parse_each();
				return true;
			}
			else{
				AnyPtr temp = eb_.pop();
				eb_.splice(EXPR_LVAR, 1);
				eb_.push(temp);
				eb_.splice(EXPR_DEFINE, 2);
				return true;
			}
		}

		lexer_.putback();
		lexer_.putback();
	}

	return false;
}

bool Parser::parse_assign_stmt(){
	{
		const Token& ch = lexer_read();

		if(ch.type()==Token::TYPE_TOKEN){
			switch(ch.ivalue()){
				XTAL_DEFAULT{}
				XTAL_CASE(c2('+','+')){ must_parse_expr(); eb_.splice(EXPR_INC, 1); return true; }
				XTAL_CASE(c2('-','-')){ must_parse_expr(); eb_.splice(EXPR_DEC, 1); return true; }
			}
		}
		lexer_.putback();
	}

	if(parse_expr()){
		if(expr_end_flag_){
			return true;
		}
		
		const Token& ch = lexer_read();

		switch(ch.type()){
			XTAL_DEFAULT{}
			
			XTAL_CASE(Token::TYPE_TOKEN){
				switch(ch.ivalue()){

					XTAL_DEFAULT{
						lexer_.putback();
						return true; 
					}

					XTAL_CASE(','){
						parse_exprs(true);
						
						if(eat('=')){
							parse_exprs();
							eb_.splice(EXPR_MASSIGN, 2);
							return true;
						}
						else if(eat(':')){
							parse_exprs();
							eb_.splice(EXPR_MDEFINE, 2);
							return true;
						}
						else{
							error_->error(lineno(), Xt("Xtal Compile Error 1001"));
						}
						
						return true;
					}

					XTAL_CASE('='){ must_parse_expr(); eb_.splice(EXPR_ASSIGN, 2); return true; }
					XTAL_CASE(':'){ must_parse_expr(); eb_.splice(EXPR_DEFINE, 2); return true; }

					XTAL_CASE(c2('+','+')){ eb_.splice(EXPR_INC, 1); return true; }
					XTAL_CASE(c2('-','-')){ eb_.splice(EXPR_DEC, 1); return true; }
					
					XTAL_CASE(c2('+','=')){ must_parse_expr(); eb_.splice(EXPR_ADD_ASSIGN, 2); return true; }
					XTAL_CASE(c2('-','=')){ must_parse_expr(); eb_.splice(EXPR_SUB_ASSIGN, 2); return true; }
					XTAL_CASE(c2('~','=')){ must_parse_expr(); eb_.splice(EXPR_CAT_ASSIGN, 2); return true; }
					XTAL_CASE(c2('*','=')){ must_parse_expr(); eb_.splice(EXPR_MUL_ASSIGN, 2); return true; }
					XTAL_CASE(c2('/','=')){ must_parse_expr(); eb_.splice(EXPR_DIV_ASSIGN, 2); return true; }
					XTAL_CASE(c2('%','=')){ must_parse_expr(); eb_.splice(EXPR_MOD_ASSIGN, 2); return true; }
					XTAL_CASE(c2('^','=')){ must_parse_expr(); eb_.splice(EXPR_XOR_ASSIGN, 2); return true; }
					XTAL_CASE(c2('|','=')){ must_parse_expr(); eb_.splice(EXPR_OR_ASSIGN, 2); return true; }
					XTAL_CASE(c2('&','=')){ must_parse_expr(); eb_.splice(EXPR_AND_ASSIGN, 2); return true; }
					XTAL_CASE(c3('<','<','=')){ must_parse_expr(); eb_.splice(EXPR_SHL_ASSIGN, 2); return true; }
					XTAL_CASE(c3('>','>','=')){ must_parse_expr(); eb_.splice(EXPR_SHR_ASSIGN, 2); return true; }
					XTAL_CASE(c4('>','>','>','=')){ must_parse_expr(); eb_.splice(EXPR_USHR_ASSIGN, 2); return true; }

					XTAL_CASE('{'){
						eb_.insert(1, null);
						parse_each();
						return true;
					}
				}
			}
		}

		lexer_.putback();
		return true;
	}

	return false;
}

bool Parser::parse_stmt(){
	if(parse_loop()){
		eat(';'); 
		return true;
	}

	const Token& ch = lexer_read();

	switch(ch.type()){
		XTAL_DEFAULT{}

		XTAL_CASE(Token::TYPE_KEYWORD){
			switch(ch.keyword_number()){
				XTAL_DEFAULT{}
		
				XTAL_CASE(Token::KEYWORD_WHILE){ eb_.push(null); parse_while(); return true; }
				XTAL_CASE(Token::KEYWORD_FOR){ eb_.push(null); parse_for(); return true; }
				XTAL_CASE(Token::KEYWORD_SWITCH){ parse_switch(); return true; }
				XTAL_CASE(Token::KEYWORD_IF){ parse_if(); return true; }
				XTAL_CASE(Token::KEYWORD_TRY){ parse_try(); return true; }
				XTAL_CASE(Token::KEYWORD_THROW){ must_parse_expr(); eb_.splice(EXPR_THROW, 1); eat(';'); return true; }	
				XTAL_CASE(Token::KEYWORD_ASSERT){ parse_assert(); eat(';'); return true; }
				XTAL_CASE(Token::KEYWORD_RETURN){ parse_exprs(); eb_.splice(EXPR_RETURN, 1); eat(';'); return true; }
				XTAL_CASE(Token::KEYWORD_YIELD){ parse_exprs(); eb_.splice(EXPR_YIELD, 1); eat(';'); return true; }
				
				XTAL_CASE(Token::KEYWORD_CONTINUE){ 
					if(!parse_identifier())
						eb_.push(null); 
					eb_.splice(EXPR_CONTINUE, 1); 
					eat(';'); 
					return true; 
				}

				XTAL_CASE(Token::KEYWORD_BREAK){ 
					if(!parse_identifier())
						eb_.push(null); 
					eb_.splice(EXPR_BREAK, 1); 
					eat(';'); 
					return true; 
				}

			}
		}
		
		XTAL_CASE(Token::TYPE_TOKEN){
			switch(ch.ivalue()){
				XTAL_DEFAULT{}
				XTAL_CASE('{'){ parse_scope(); return true; }
				XTAL_CASE(';'){ eb_.splice(EXPR_NULL, 0); return true; }
			}
		}
	}
	
	lexer_.putback();
	if(parse_assign_stmt()){
		eat(';');
		return true;
	}

	return false;
}

void Parser::must_parse_stmt(){
	if(!parse_stmt()){
		error_->error(lineno(), Xt("Xtal Compile Error 1001"));
		eb_.push(null);
	}
}

void Parser::parse_assert(){
	lexer_.begin_record();
	if(parse_expr()){
		StringPtr ref_str = lexer_.end_record();
		eb_.push(KIND_STRING);
		eb_.push(ref_str);
		eb_.splice(EXPR_STRING, 2);
		if(!eat(',') || !parse_expr()){
			eb_.splice(0, 2);
		}
		else{
			eb_.splice(0, 3);
		}
	}
	else{
		eb_.splice(0, 0);
		lexer_.end_record();
	}

	eb_.splice(EXPR_ASSERT, 1);
}
	
void Parser::parse_exprs(bool one){
	ExprBuilder::State state = eb_.begin();
	if(one){
		state.pos--;
	}

	for(;;){
		if(!parse_expr() || !eat(',')){
			break;
		}
	}
	eb_.end(0, state);
}

void Parser::parse_stmts(){
	ExprBuilder::State state = eb_.begin();
	while(parse_stmt()){}
	eb_.end(0, state);
}

void Parser::must_parse_identifier(){
	if(!parse_identifier()){
		error_->error(lineno(), Xt("Xtal Compile Error 1001"));
		eb_.push(null);
	}
}

bool Parser::parse_identifier(){
	if(lexer_peek().type()==Token::TYPE_IDENTIFIER){
		eb_.push(lexer_read().identifier());
		return true;
	}
	return false;
}

void Parser::parse_identifier_or_keyword(){
	if(lexer_peek().type()==Token::TYPE_IDENTIFIER){
		eb_.push(lexer_read().identifier());
	}
	else if(lexer_peek().type()==Token::TYPE_KEYWORD){
		eb_.push(lexer_read().identifier());
	}
	else{
		expect('i');
	}
}

bool Parser::parse_var(){
	if(parse_identifier()){
		if(eat(':')){ 
			return true; 
		}
		else{
			eb_.pop();
			lexer_.putback();
		}
	}
	return false;
}
	
void Parser::parse_toplevel(){
	parse_stmts();
	eb_.splice(EXPR_TOPLEVEL, 1);
	expect(-1);
}

void Parser::parse_scope(){
	parse_stmts();
	eb_.splice(EXPR_SCOPE, 1);
	expect('}');
	expr_end_flag_ = true;
}

void Parser::parse_class(int_t kind){
	eb_.push(kind);

	if(eat('(')){
		parse_exprs();
		expect(')');
	}
	else{
		eb_.push(null);
	}

	ExprBuilder::State state = eb_.begin();

	expect('{');
	for(;;){
		
		if(eat('#')){// 可触性 protected 指定
			eb_.push(KIND_PROTECTED);
		}
		else if(eat('-')){// 可触性 private 指定
			eb_.push(KIND_PRIVATE);
		}
		else if(eat('+')){// 可触性 public 指定
			eb_.push(KIND_PUBLIC);
		}
		else{
			eb_.push(null);
		}

		if(parse_identifier()){ // メンバ定義
			if(eat('#')){
				must_parse_expr();
				expect(':');
				must_parse_expr();
			}
			else{
				eb_.push(null);
				expect(':');
				must_parse_expr();
			}
			eb_.splice(EXPR_CDEFINE_MEMBER, 4);
			eat(';');
			
		}
		else if(eat('_')){// インスタンス変数定義
			if(parse_identifier()){
				if(eat(':')){ // 初期値込み
					must_parse_expr();
				}
				else{
					eb_.push(null);
				}
				eb_.splice(EXPR_CDEFINE_IVAR, 3);
				eat(';');
			}
			else{
				eb_.pop();
				error_->error(lineno(), Xt("Xtal Compile Error 1001"));
			}
		}
		else{
			eb_.pop();
			break;
		}
	}

	eb_.end(0, state);
	eb_.splice(EXPR_CLASS, 3);

	expect('}');
	expr_end_flag_ = true;
}

void Parser::parse_try(){	
	must_parse_stmt();
	
	if(eat(Token::KEYWORD_CATCH)){
		expect('(');
		must_parse_identifier();
		expect(')');
		must_parse_stmt();
	}
	else{
		eb_.push(null);
		eb_.push(null);
	}

	if(eat(Token::KEYWORD_FINALLY)){
		must_parse_stmt();
	}
	else{
		eb_.push(null);
	}

	eb_.splice(EXPR_TRY, 4);
}

void Parser::parse_lambda(bool noparam){
	eb_.push(KIND_LAMBDA);

	if(noparam){
		eb_.push(null);
	}
	else{
		ExprBuilder::State state = eb_.begin();
		for(;;){
			if(parse_identifier()){
				eb_.splice(EXPR_LVAR, 1);
				eb_.push(null);
				eb_.splice(0, 2);
			}
			else{
				break;
			}

			if(!eat(',')){
				break;
			}
		}
		eat(',');
		expect('|');
		eb_.end(0, state);
	}

	eb_.push(null);

	if(eat('{')){
		parse_scope();
	}
	else{
		parse_exprs();
		eb_.splice(EXPR_RETURN, 1);
	}

	eb_.splice(EXPR_FUN, 4);
}

void Parser::parse_fun(int_t kind){
	eb_.push(kind);
	
	if(eat('(')){

		ExprBuilder::State state = eb_.begin();
		for(;;){
			if(eat(c3('.','.','.'))){ // extendable
				lexer_.putback();
				break;
			}
			else{
				if(parse_expr()){
					if(eat(':')){
						must_parse_expr();
					}
					else{
						eb_.push(null);
					}
					eb_.splice(0, 2);
				}
				else{
					break;
				}
			}

			if(!eat(',')){
				break;
			}
		}
		eb_.end(0, state);

		if(eat(c3('.','.','.'))){ // extendable
			if(!parse_expr()){
				eb_.splice(EXPR_ARGS, 0);
			}
		}
		else{
			eb_.push(null);
		}

		eat(',');
		expect(')');
	}
	else{
		eb_.push(null);
		eb_.push(null);
	}

	if(eat('{')){
		parse_scope();
	}
	else{
		parse_exprs();
		eb_.splice(EXPR_RETURN, 1);
	}

	eb_.splice(EXPR_FUN, 4);
}

void Parser::parse_call(){
	// 順番引数のループ
	ExprBuilder::State state = eb_.begin();
	for(;;){
		if(eat(c3('.','.','.'))){ // extendable
			lexer_.putback();
			break;
		}
		else{
			if(parse_expr()){
				if(eat(':')){
					must_parse_expr();
				}
				else{
					eb_.insert(1, null);
				}
				eb_.splice(0, 2);
			}
			else{
				break;
			}
		}

		if(!eat(',')){
			break;
		}
	}
	eb_.end(0, state);

	if(eat(c3('.','.','.'))){ // extendable
		if(!parse_expr()){
			eb_.splice(EXPR_ARGS, 0);
		}
	}
	else{
		eb_.push(null);
	}

	eat(',');
	expect(')');

	eb_.splice(EXPR_CALL, 3);
}

bool Parser::parse_expr(int_t pri){
	if(!parse_term()){
		return false;
	}
	while(parse_post(pri)){}
	return true;
}

bool Parser::parse_expr(){
	expr_end_flag_ = false;
	return parse_expr(0);
}


void Parser::must_parse_expr(int_t pri){
	if(!parse_expr(pri)){
		error_->error(lineno(), Xt("Xtal Compile Error 1001"));
		eb_.push(null);
	}
}

void Parser::must_parse_expr(){
	expr_end_flag_ = false;
	must_parse_expr(0);

}

void Parser::parse_if(){
	expect('(');

	if(parse_var()){
		eb_.splice(EXPR_LVAR, 1);
		must_parse_expr();
		eb_.splice(EXPR_DEFINE, 2);
	}
	else{
		must_parse_expr();
	}

	expect(')');
	must_parse_stmt();
	if(eat(Token::KEYWORD_ELSE)){
		must_parse_stmt();
	}
	else{
		eb_.push(null);
	}
	eb_.splice(EXPR_IF, 3);
}

void Parser::parse_switch(){
	expect('(');

	if(parse_var()){
		must_parse_expr();
		eb_.splice(EXPR_DEFINE, 2);
	}
	else{
		must_parse_expr();
	}
	
	expect(')');
	expect('{');
	
	ExprBuilder::State state = eb_.begin();
	for(;;){
		if(eat(Token::KEYWORD_CASE)){
			expect('(');
			parse_exprs();
			expect(')');
			must_parse_stmt();
			eb_.splice(EXPR_SWITCH_CASE, 2);
		}
		else if(eat(Token::KEYWORD_DEFAULT)){
			must_parse_stmt();
			eb_.splice(EXPR_SWITCH_DEFAULT, 1);
		}
		else{
			expect('}');
			expr_end_flag_ = true;
			break;
		}
	}
	eb_.end(0, state);
	
	eb_.splice(EXPR_SWITCH, 2);
}

void Parser::parse_array(){	
	if(eat(']')){//empty array
		eb_.splice(EXPR_ARRAY, 0);
		return;
	}
	
	if(eat(':')){//empty map
		expect(']');
		eb_.splice(EXPR_MAP, 0);
		return;
	}
	
	ExprBuilder::State state = eb_.begin();
	must_parse_expr();
	if(eat(':')){//map
		must_parse_expr();
		eb_.splice(0, 2);

		if(eat(',')){
			for(;;){
				if(parse_expr()){
					expect(':');
					must_parse_expr();
					eb_.splice(0, 2);
					
					if(!eat(',')){
						break;
					}
				}
				else{
					break;
				}
			}
		}

		expect(']');
		eb_.end(0, state);
		eb_.splice(EXPR_MAP, 1);
	}
	else{//array
		if(eat(',')){
			for(;;){
				if(parse_expr()){
					if(!eat(',')){
						break;
					}
				}
				else{
					break;
				}
			}
		}

		expect(']');
		eb_.end(0, state);
		eb_.splice(EXPR_ARRAY, 1);
	}
}

}

#endif

