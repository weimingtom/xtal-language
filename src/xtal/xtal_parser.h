/** \file src/xtal/xtal_parser.h
* \brief src/xtal/xtal_parser.h
*/

#ifndef XTAL_PARSER_H_INCLUDE_GUARD
#define XTAL_PARSER_H_INCLUDE_GUARD

#pragma once

#ifndef XTAL_NO_PARSER

namespace xtal{

class Token : public ImmediateValue{
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
		KEYWORD_DOFUN,
		KEYWORD_IS,
		KEYWORD_IN,
		KEYWORD_ASSERT,
		KEYWORD_NOBREAK,
		KEYWORD_SWITCH,
		KEYWORD_CASE,
		KEYWORD_DEFAULT,
		KEYWORD_SINGLETON,
		KEYWORD_PUBLIC,
		KEYWORD_PROTECTED,
		KEYWORD_PRIVATE,

		KEYWORD_MAX
	};

	Token()
		:ImmediateValue(0, 0){}
	
	Token(int_t type, int_t flags, int_t value)
		:ImmediateValue(type | (flags<<4), value){}

	Token(int_t type, int_t flags, float_t value)
		:ImmediateValue(type | (flags<<4), value){}

	int_t type() const{ return first()&0xf; }

	bool left_space() const{ return (((first()>>4)&0xf) & FLAG_LEFT_SPACE) != 0; }
	
	bool right_space() const{ return (((first()>>4)&0xf) & FLAG_RIGHT_SPACE) != 0; }
	
	int_t ivalue() const{ return second(); }
	
	float_t fvalue() const{ return secondf(); }
	
	int_t identifier_number() const{ return second(); }
	
	int_t keyword_number() const{ return second(); }
};

class Parser{
public:

	Parser();

	ExprPtr parse(const xpeg::ExecutorPtr& scanner);

	ExprPtr parse_eval(const xpeg::ExecutorPtr& scanner);

private:
	
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
	void parse_class2(int_t kind);
	void parse_scope();
	void parse_lambda(bool noparam = false);
	void parse_fun(int_t kind);
	void parse_fun2(int_t kind);
	void parse_call();		
	void parse_toplevel();

	void parse_expr_statement();
	bool expr_end();
	void expect_stmt_end();

private:

	const Token& read_token();
	const Token& peek_token(int_t n = 0);
	void putback_token();
	void putback_token(const Token& ch);
	StringPtr token_to_string(const Token& ch);
		
	StringPtr read_string(int_t open, int_t close);

	const IDPtr& identifier(int_t n){
		return unchecked_ptr_cast<ID>(identifiers_->at(n));
	}

private:

	void tokenize();
	void push_token(int_t v);
	void push_int_token(int_t v);
	void push_float_token(float_t v);
	void push_keyword_token(int_t num);
	void push_identifier_token(int_t num);

	void deplete_space();

	float_t read_finteger();
	int_t read_integer(int_t base);
	bool is_integer_literal();
	void tokenize_number();
	
	int_t test_right_space(int_t ch);

private:

	MapPtr identifier_map_;
	ArrayPtr identifiers_;
	MemoryStreamPtr ms_;

	enum{ TOKEN_BUF_SIZE = 8, TOKEN_BUF_MASK = TOKEN_BUF_SIZE-1 };
	Token token_buf_[TOKEN_BUF_SIZE];
	int_t token_pos_;
	int_t token_read_;

	uint_t left_space_;

public:

	xpeg::ExecutorPtr reader_;
	ExprBuilder eb_;
};

}

#endif


#endif // XTAL_PARSER_H_INCLUDE_GUARD
