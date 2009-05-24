/** \file src/xtal/xtal_basictype.h
* \brief src/xtal/xtal_basictype.h
*/

#ifndef XTAL_BASICTYPE_H_INCLUDE_GUARD
#define XTAL_BASICTYPE_H_INCLUDE_GUARD

#pragma once

namespace xtal{

/**
* \xbind lib::builtin
* \brief ヌル値
*/
class Null : public Any{
public:
	Null():Any(TYPE_NULL){} 
};

/**
* \xbind lib::builtin
* \brief 未定義値
*/
class Undefined : public Any{ 
public: 
	Undefined():Any(TYPE_UNDEFINED){} 
};

/**
* \xbind lib::builtin
* \brief 整数値
*/
class Int : public Any{
public:

	Int(int_t val = 0)
		:Any(val){}

public:
	
	int_t to_i(){
		return ivalue(*this);
	}

	float_t to_f(){
		return (float_t)ivalue(*this);
	}

	StringPtr to_s(){
		return Any::to_s();
	}

	bool op_in(const IntRangePtr& range);

	bool op_in(const FloatRangePtr& range);

	IntRangePtr op_range(int_t right, int_t kind);

	FloatRangePtr op_range(float_t right, int_t kind);
};

/**
* \xbind lib::builtin
* \brief 浮動小数点値
*/
class Float : public Any{
public:

	Float(float_t val = 0)
		:Any(val){}

public:

	int_t to_i(){
		return (int_t)fvalue(*this);
	}

	float_t to_f(){
		return fvalue(*this);
	}

	StringPtr to_s(){
		return Any::to_s();
	}

	bool op_in(const IntRangePtr& range);

	bool op_in(const FloatRangePtr& range);

	FloatRangePtr op_range(int_t right, int_t kind);

	FloatRangePtr op_range(float_t right, int_t kind);
};

/**
* \xbind lib::builtin
* \brief 真偽値
*/
class Bool : public Any{
public: 
	Bool(bool b):Any(b){} 
};

/**
* \xbind lib::builtin
* \brief 区間
*/
class Range : public Base{
public:

	Range(const AnyPtr& left, const AnyPtr& right, int_t kind = RANGE_CLOSED)
		:left_(left), right_(right), kind_(kind){}

	/**
	* \xbind
	* \brief 区間の左の要素を返す
	*/
	const AnyPtr& left(){ return left_; }

	/**
	* \xbind
	* \brief 区間の右の要素を返す
	*/
	const AnyPtr& right(){ return right_; }

	/**
	* \xbind
	* \brief 区間の種類を返す
	*/
	int_t kind(){ return kind_; }

	/**
	* \xbind
	* \brief 左が閉じた区間か返す
	*/
	bool is_left_closed(){ return (kind_&(1<<1))==0; }

	/**
	* \xbind
	* \brief 右が閉じた区間か返す
	*/
	bool is_right_closed(){ return (kind_&(1<<0))==0; }

	/**
	* \brief 区間の種類
	*/
	enum RangeKind{
		/**
		* \brief 閉区間[left, right]
		*/
		CLOSED = RANGE_CLOSED,

		/**
		* \brief 左開右閉区間 [left, right)
		*/
		LEFT_CLOSED_RIGHT_OPEN = RANGE_LEFT_CLOSED_RIGHT_OPEN,

		/**
		* \brief 左開右閉区間 (left, right]
		*/
		LEFT_OPEN_RIGHT_CLOSED = RANGE_LEFT_OPEN_RIGHT_CLOSED,

		/**
		* \brief 開区間 (left, right)
		*/
		OPEN = RANGE_OPEN
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

/**
* \xbind lib::builtin
* \brief 区間
*/
class IntRange : public Range{
public:

	IntRange(int_t left, int_t right, int_t kind = RANGE_CLOSED)
		:Range(left, right, kind){}

public:

	/**
	* \xbind
	* \brief 範囲の開始を返す
	*
	* beginは左閉右開区間 [begin, end) で得ることが出来る 
	*/
	int_t begin(){ return is_left_closed() ? left() : left()+1; }

	/**
	* \xbind
	* \brief 範囲の終端を返す
	*
	* endは左閉右開区間 [begin, end) で得ることが出来る 
	*/
	int_t end(){ return is_right_closed() ? right()+1 : right(); }

	/**
	* \xbind
	* \brief 区間の左の要素を返す
	*/
	int_t left(){ return ivalue(left_); }

	/**
	* \xbind
	* \brief 区間の左の要素を返す
	*/
	int_t right(){ return ivalue(right_); }

	AnyPtr each();
};

class IntRangeIter : public Base{
public:

	IntRangeIter(const IntRangePtr& range)
		:it_(range->begin()), end_(range->end()){
	}

	void block_next(const VMachinePtr& vm);

private:
	int_t it_, end_;
};

/**
* \xbind lib::builtin
* \brief 区間
*/
class FloatRange : public Range{
public:

	FloatRange(float_t left, float_t right, int_t kind = RANGE_CLOSED)
		:Range(left, right, kind){}

public:

	/**
	* \xbind
	* \brief 区間の左の要素を返す
	*/
	float_t left(){ return fvalue(left_); }

	/**
	* \xbind
	* \brief 区間の左の要素を返す
	*/
	float_t right(){ return fvalue(right_); }

	AnyPtr each();
};

/**
* \brief 多値
*/
class Values : public RefCountingBase{
public:
	enum{ TYPE = TYPE_VALUES };

	Values(const AnyPtr& head)
		:head_(head), tail_((ValuesPtr&)undefined){
		set_pvalue(*this, TYPE, this);

		static int n = 0;
		if(n++>1500){
			n = n;
		}
	}

	Values(const AnyPtr& head, const ValuesPtr& tail)
		:head_(head), tail_(tail){
		set_pvalue(*this, TYPE, this);
	}

	~Values(){

	}

	const AnyPtr& head(){
		return head_;
	}

	const ValuesPtr& tail(){
		return tail_;
	}

	void block_next(const VMachinePtr& vm);

	int_t size();

	const AnyPtr& at(int_t i);

	const AnyPtr& op_at(int_t i){
		return at(i);
	}

	/**
	* \xbind
	* \brief 値が等しいか調べる
	*/
	bool op_eq(const ValuesPtr& other);

	void visit_members(Visitor& m){
		m & head_ & tail_;
	}

public:

	void set(const AnyPtr& head, const ValuesPtr& tail = null){
		head_ = head;
		tail_ = tail;
	}

private:
	AnyPtr head_;
	ValuesPtr tail_;
};

inline ValuesPtr mv(const AnyPtr& v1, const AnyPtr& v2){
	return xnew<Values>(v1, xnew<Values>(v2));
}

/**
* \brief スコープ的な親を認識するクラス
*/
class HaveParent : public Base{
public:

	HaveParent()
		:parent_(0){}

	HaveParent(const HaveParent& a);

	HaveParent& operator=(const HaveParent& a);

	~HaveParent();

	virtual const ClassPtr& object_parent();

	virtual void set_object_parent(const ClassPtr& parent);

protected:

	Class* parent_;

	virtual void visit_members(Visitor& m);
};

/**
* \brief スコープ的な親を認識するクラス
*/
class RefCountingHaveParent : public RefCountingBase{
public:

	RefCountingHaveParent()
		:parent_(0){}

	RefCountingHaveParent(const RefCountingHaveParent& a);

	RefCountingHaveParent& operator=(const RefCountingHaveParent& a);

	~RefCountingHaveParent();

	const ClassPtr& object_parent();

	void set_object_parent(const ClassPtr& parent);

	void visit_members(Visitor& m);

protected:

	Class* parent_;
};

/**
* \brief GC時に通知を受け取るクラス
*/
class GCObserver : public Base{
public:
	GCObserver();
	GCObserver(const GCObserver& v);
	virtual ~GCObserver();
	virtual void before_gc(){}
	virtual void after_gc(){}
};


}

#endif // XTAL_BASICTYPE_H_INCLUDE_GUARD
