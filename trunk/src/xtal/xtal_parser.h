#ifndef XTAL_PARSER_H_INCLUDE_GUARD
#define XTAL_PARSER_H_INCLUDE_GUARD

#pragma once

#ifndef XTAL_NO_PARSER

namespace xtal{

class Token{
public:

	enum Flag{
		FLAG_LEFT_SPACE = 1<<0,
		FLAG_RIGHT_SPACE = 1<<1
	};
	
	enum Type{	
		TYPE_TOKEN = 0,
		TYPE_INT = 1,
		TYPE_FLOAT = 2,
		TYPE_IDENTIFIER = 3,
		TYPE_KEYWORD = 4
	};
	
	enum Keyword{
		KEYWORD_NONE,
		
		KEYWORD_IF,
		KEYWORD_FOR,
		KEYWORD_ELSE,
		KEYWORD_FUN,
		KEYWORD_METHOD,
		KEYWORD_DO,
		KEYWORD_WHILE,
		KEYWORD_CONTINUE,
		KEYWORD_BREAK,
		KEYWORD_FIBER,
		KEYWORD_YIELD,
		KEYWORD_RETURN,
		KEYWORD_ONCE,
		KEYWORD_NULL,
		KEYWORD_UNDEFINED,
		KEYWORD_FALSE,
		KEYWORD_TRUE,
		KEYWORD_XTAL,
		KEYWORD_TRY,
		KEYWORD_CATCH,
		KEYWORD_FINALLY,
		KEYWORD_THROW,
		KEYWORD_CLASS,
		KEYWORD_CALLEE,
		KEYWORD_THIS,
		KEYWORD_CURRENT_CONTEXT,
		KEYWORD_DOFUN,
		KEYWORD_IS,
		KEYWORD_IN,
		KEYWORD_ASSERT,
		KEYWORD_NOBREAK,
		KEYWORD_SWITCH,
		KEYWORD_CASE,
		KEYWORD_DEFAULT,
		KEYWORD_SINGLETON,
		KEYWORD_DEBUG,
		KEYWORD_PUBLIC,
		KEYWORD_PROTECTED,
		KEYWORD_PRIVATE,

		KEYWORD_MAX
	};

	Token(){}
	
	Token(int_t type, const AnyPtr& value, int_t flags, int_t keyword_number = 0)
		:type_((u8)type), flags_((u8)flags), keyword_number_(keyword_number), value_(value){}

	int_t type() const{ return type_; }

	bool left_space() const{ return (flags_ & FLAG_LEFT_SPACE) != 0; }
	
	bool right_space() const{ return (flags_ & FLAG_RIGHT_SPACE) != 0; }
	
	int_t ivalue() const{ return xtal::ivalue((Any&)value_); }
	
	float_t fvalue() const{ return xtal::fvalue(value_); }
	
	const IDPtr& identifier() const{ return *(IDPtr*)&value_; }
	
	u16 keyword_number() const{ return keyword_number_; }

	StringPtr to_s() const{ 
		if(type_==TYPE_TOKEN){
			char_t buf[4] = {((ivalue()>>0)&0xff), ((ivalue()>>8)&0xff), ((ivalue()>>16)&0xff), 0};
			return buf;
		}
		else{
			return value_->to_s();
		}
	}

private:

	u8 type_;
	u8 flags_;
	u16 keyword_number_;
	AnyPtr value_;
	
};

class Reader{
public:

	Reader();

	void set_stream(const StreamPtr& stream){
		stream_ = stream;
	}

	/**
	* @brief 読み進める。
	*/
	int_t read();

	/**
	* @brief 次の要素を読む。
	*/
	int_t peek(int_t n=0);

	/**
	* @brief 次の要素が引数chと同じだったら読み進める。
	* @param ch この値と次の要素が等しい場合に読み進める。
	* @retval true 次の要素はchと同じで、読み進めた。
	* @retval false 次の要素はchと異なり、読み進められなかった。
	*/
	bool eat(int_t ch);

	/**
	* @brief 文字列の記録を開始する
	*/
	void begin_record();

	/**
	* @brief 文字列の記録を終了して、それを返す。
	*/
	StringPtr end_record();

private:

	enum{ BUF_SIZE = 1024, BUF_MASK = BUF_SIZE-1 };

	StreamPtr stream_;

	char_t buf_[BUF_SIZE];

	uint_t pos_;
	uint_t read_;

	MemoryStreamPtr recorded_ms_;
	bool recording_;
};
	
class CompileErrors{
public:	

	void init(const StringPtr& source_file_name);
	void error(int_t line, const AnyPtr& message);

	ArrayPtr errors;
	StringPtr source_file_name;
};

/*
* XTALプログラムソースをトークン列に変換して取り出す
*/
class Lexer{
public:

	Lexer();
	
	/**
	* @brief 初期化
	*/
	void init(const StreamPtr& stream, CompileErrors* error);
	
	/**
	* @brief 読み進める
	*/
	const Token& read();

	/**
	* @brief 次の要素を読む
	*/
	const Token& peek();

	/**
	* @brief 読み込んだ要素を一つ戻す
	*/
	void putback();
	
	/**
	* @brief 指定したトークンを一つ戻す
	* 次のreadやpeekでは、これで戻した値が得られる
	*/
	void putback(const Token& ch);
	
	/**
	* @brief 現在の行数を返す
	*/
	int_t lineno(){ return lineno_; }
	
	/**
	* @brief 現在の行数を設定する
	*/
	void set_lineno(int_t v){ lineno_ = v; }

	int_t read_direct();

	StringPtr read_string(int_t open, int_t close);

	/**
	* @brief 文字列の記録を開始する
	*/
	void begin_record(){
		reader_.begin_record();
	}

	/**
	* @brief 文字列の記録を終了して、それを返す。
	*/
	StringPtr end_record(){
		return reader_.end_record();
	}

private:
	
	void do_read();

	void push_token(int_t v);
	void push_int_token(int_t v);
	void push_float_token(float_t v);
	void push_keyword_token(const IDPtr& v, int_t num);
	void push_identifier_token(const IDPtr& v);

	void deplete_space();

	IDPtr parse_identifier();
	int_t parse_integer();
	int_t parse_hex();
	int_t parse_oct();
	int_t parse_bin();

	void parse_number_suffix(int_t val);
	void parse_number_suffix(float_t val);
	void parse_number();
	
	int_t test_right_space(int_t ch);

private:

	Reader reader_;
	MapPtr keyword_map_;
	MemoryStreamPtr ms_;

	enum{ BUF_SIZE = 64, BUF_MASK = BUF_SIZE-1 };
	Token buf_[BUF_SIZE];

	uint_t pos_;
	uint_t read_;
	uint_t left_space_;
	uint_t lineno_;

	CompileErrors* error_;
};

class Parser{
public:

	Parser();

	ExprPtr parse(const StreamPtr& stream, CompileErrors* error);

	ExprPtr parse_stmt(const StreamPtr& stream, CompileErrors* error);

private:

	int_t lineno(){ return lexer_.lineno(); }

	bool eos(){
		return eat(-1);
	}
	
	const Token& lexer_read();
	const Token& lexer_peek();

	void expect(int_t ch);
	bool eat(int_t ch);
	bool eat(Token::Keyword kw);

	bool cmp_pri(int_t pri, int_t op, int_t r_space, int_t l_space);

	void parse_secondary_key();
	bool parse_term();
	bool parse_expr(int_t pri, int_t space);
	bool parse_expr();
	void expect_parse_expr(int_t pri, int_t space);
	bool parse_post(int_t pri, int_t space);
	void expect_parse_expr();
	void expect_parse_identifier();
	bool parse_identifier();
	void parse_identifier_or_keyword();
	bool parse_var();
	void parse_if();
	void expect_parse_stmt();
	void parse_else_or_nobreak();
	void parse_try();
	bool parse_stmt();
	void parse_stmts();
	void parse_assert();
	void parse_exprs(bool one = false);
	bool parse_assign_stmt();
	void parse_array();
	void parse_each();
	void parse_while();
	void parse_for();
	bool parse_loop();
	void parse_switch();
	void parse_class(int_t kind);
	void parse_scope();
	void parse_lambda(bool noparam = false);
	void parse_fun(int_t kind);
	void parse_call();		
	void parse_toplevel();

public:

	ExprBuilder eb_;
	Lexer lexer_;
	bool expr_end_flag_;
	CompileErrors* error_;
};


}

#endif


#endif // XTAL_PARSER_H_INCLUDE_GUARD
