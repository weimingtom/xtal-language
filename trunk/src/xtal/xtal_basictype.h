
#pragma once

namespace xtal{

class Int : public Any{};
class Float : public Any{};
class True : public Any{};
class False : public Any{};

class Range : public Base{
public:

	Range(const AnyPtr& left, const AnyPtr& right, int_t kind = CLOSED)
		:left_(left), right_(right), kind_(kind){}

	const AnyPtr& left(){ return left_; }

	const AnyPtr& right(){ return right_; }

	int_t kind(){ return kind_; }

	bool is_left_closed(){ return (kind_&(1<<1))==0; }

	bool is_right_closed(){ return (kind_&(1<<0))==0; }

	enum{
		CLOSED = (0<<1) | (0<<0),
		LEFT_CLOSED_RIGHT_OPEN = (0<<1) | (1<<0),
		LEFT_OPEN_RIGHT_CLOSED = (1<<1) | (0<<0),
		OPEN = (1<<1) | (1<<0)
	};

protected:

	virtual void visit_members(Visitor& m){
		Base::visit_members(m);
		m & left_ & right_;
	}

	AnyPtr left_;
	AnyPtr right_;
	int_t kind_;
};

class IntRange : public Range{
public:

	IntRange(int_t left, int_t right, int_t kind = CLOSED)
		:Range(left, right, kind){}

public:

	/**
	* @brief 範囲の開始
	*
	* beginは左閉右開区間 [begin, end) で得ることが出来る 
	*/
	int_t begin(){ return is_left_closed() ? left() : left()+1; }

	/**
	* @brief 範囲の終端
	*
	* endは左閉右開区間 [begin, end) で得ることが出来る 
	*/
	int_t end(){ return is_right_closed() ? right()+1 : right(); }

	int_t left(){ return ivalue(left_); }

	int_t right(){ return ivalue(right_); }

	AnyPtr each();
};

class FloatRange : public Range{
public:

	FloatRange(float_t left, float_t right, int_t kind = CLOSED)
		:Range(left, right, kind){}

public:

	float_t left(){ return fvalue(left_); }

	float_t right(){ return fvalue(right_); }

	AnyPtr each();
};

}
