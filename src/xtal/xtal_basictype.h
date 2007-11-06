
#pragma once

namespace xtal{

class Int : public Any{};
class Float : public Any{};
class True : public Any{};
class False : public Any{};


class IntRange : public Base{
public:

	IntRange(int_t left, int_t right, int_t kind = 0)
		:left_(left), right_(right), kind_(kind){}

public:

	/**
	* @brief 範囲の開始
	*
	* beginは左閉右開区間 [begin, end) で得ることが出来る 
	*/
	int_t begin(){ return left_closed() ? left_ : left_+1; }

	/**
	* @brief 範囲の終端
	*
	* endは左閉右開区間 [begin, end) で得ることが出来る 
	*/
	int_t end(){ return right_closed() ? right_+1 : right_; }

	int_t left(){ return left_; }

	int_t right(){ return right_; }

	bool left_closed(){ return (kind_&(1<<1))==0; }

	bool right_closed(){ return (kind_&(1<<0))==0; }

	AnyPtr each();

private:

	int_t left_;
	int_t right_;
	int_t kind_;
};

class FloatRange : public Base{
public:

	FloatRange(float_t left, float_t right, int_t kind = 0)
		:left_(left), right_(right), kind_(kind){}

public:

	float_t left(){ return left_; }

	float_t right(){ return right_; }

	bool left_closed(){ return (kind_&(1<<1))==0; }

	bool right_closed(){ return (kind_&(1<<0))==0; }

	AnyPtr each();

private:

	float_t left_;
	float_t right_;
	int_t kind_;
};


struct Lazy : public Base{
	AnyPtr value;
	AnyPtr ret;
	Lazy(const AnyPtr& v):value(v){}
};

}
