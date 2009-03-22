
#pragma once

namespace xtal{

/**
* @brief 配列
*
*/
class Array : public RefCountingBase{
public:
	enum{ TYPE = TYPE_ARRAY };

	/**
	* @brief sizeの長さの配列を生成する 
	*
	*/
	Array(uint_t size = 0);

	Array(const AnyPtr* first, const AnyPtr* end);

	/**
	* @brief コピーコンストラクタを備える
	*/
	Array(const Array& v);

	/**
	* @brief 代入演算子を備える
	*/
	Array& operator =(const Array& v);

	/**
	* @brief デストラクタ
	*/
	~Array();


	/**
	* @brief 配列の長さを返す
	*
	*/
	uint_t length(){
		return size_;
	}

	/**
	* @brief 配列の長さを返す
	*
	*/
	uint_t size(){
		return size_;
	}

	/**
	* @brief 配列の長さを変更する
	*/
	void resize(uint_t sz);

	/**
	* @brief 配列をsz分長くする
	*/
	void upsize(uint_t sz);

	/**
	* @brief 配列をsz分短くする
	*/
	void downsize(uint_t sz);

	/**
	* @brief i番目の要素を返す
	*
	*/
	const AnyPtr& at(int_t i){
		XTAL_ASSERT(0<=i && (uint_t)i<size_);
		return values_[i];
	}

	/**
	* @brief i番目の要素を設定する
	*
	*/
	void set_at(int_t i, const AnyPtr& v){
		XTAL_ASSERT(0<=i && (uint_t)i<size_);
		values_[i] = v;
	}

	/**
	* @brief i番目の要素を返す
	*
	*/
	const AnyPtr& op_at(int_t i);

	/**
	* @brief i番目の要素を設定する
	*
	*/
	void op_set_at(int_t i, const AnyPtr& v);

	/**
	* @brief 先頭に要素を追加する
	*
	*/
	void push_front(const AnyPtr& v){
		insert(0, v);
	}

	/**
	* @brief 先頭の要素を削除する
	*
	*/
	void pop_front(){
		erase(0);
	}

	/**
	* @brief 末尾に要素を追加する
	*
	*/
	void push_back(const AnyPtr& v);

	/**
	* @brief 末尾の要素を削除する
	*
	*/
	void pop_back();

	/**
	* @brief 先頭の要素を返す
	*
	*/
	const AnyPtr& front(){
		return op_at(0);
	}

	/**
	* @brief 末尾の要素を返す
	*
	*/
	const AnyPtr& back(){
		return op_at(size()-1);
	}

	/**
	* @brief i番目からn個の部分配列を返す
	*
	*/
	ArrayPtr slice(int_t i, int_t n = 1);
	
	/**
	* @brief indexからn個の部分配列を削除し、その部分配列を返す
	*
	*/
	ArrayPtr splice(int_t i, int_t n = 1);

	/**
	* @brief i番目のn個の要素を削除する
	*
	*/
	void erase(int_t i, int_t n = 1);

	/**
	* @brief i番目に要素を追加する
	*
	*/
	void insert(int_t i, const AnyPtr& v);

	/**
	* @brief 配列の要素を逆順にするイテレータを返す
	*
	*/
	AnyPtr reverse();
	
	/**
	* @brief 浅いコピーを返す
	*
	*/
	ArrayPtr clone();

	/**
	* @brief 連結した配列を返す
	*
	*/
	ArrayPtr cat(const ArrayPtr& a);

	/**
	* @brief 自身を連結し、自身を返す
	*
	*/
	ArrayPtr cat_assign(const ArrayPtr& a);
	
	/**
	* @brief 要素を文字列として連結した結果を返す
	*
	* @param sep 要素と要素の区切り文字列
	*/
	StringPtr join(const StringPtr& sep);

	/**
	* @brief 自身を返す
	*
	*/
	ArrayPtr to_a(){
		return to_smartptr(this);
	}

	bool op_eq(const ArrayPtr& other);

	/**
	* @brief 空か調べる
	*
	*/
	bool empty(){
		return size_ == 0;
	}

	/**
	* @brief 空か調べる
	*
	*/
	bool is_empty(){
		return size_ == 0;
	}

	/**
	* @brief 要素を全て削除する
	*
	*/
	void clear();

	/**
	* @brief 要素を最初から反復できるIteratorを返す
	*
	*/
	AnyPtr each();

	/**
	* @brief 中身をiteratorで取得できる要素に置き換える
	*/
	void assign(const AnyPtr& iterator);

	void concat(const AnyPtr& iterator);

public:

	void visit_members(Visitor& m);

	class iterator{
	public:
		
		iterator(AnyPtr* p = 0)
			:p_(p){}
		
		AnyPtr& operator *() const{
			return *p_;
		}

		AnyPtr* operator ->() const{
			return p_;
		}

		iterator& operator ++(){
			++p_;
			return *this;
		}

		iterator operator ++(int){
			iterator temp(*this);
			++p_;
			return *this;
		}

		friend bool operator ==(iterator a, iterator b){
			return a.p_ == b.p_;
		}

		friend bool operator !=(iterator a, iterator b){
			return a.p_ != b.p_;
		}

	private:
		AnyPtr* p_;
	};

	iterator begin(){
		return iterator(values_);
	}

	iterator end(){
		return iterator(values_ + size_);
	}


protected:

	int_t calc_offset(int_t i);
	void throw_index_error();

	AnyPtr* values_;
	uint_t size_;
	uint_t capa_;
};

class ArrayIter : public Base{
public:

	ArrayIter(const ArrayPtr& a, bool reverse = false);
			
	void block_next(const VMachinePtr& vm);

public:

	bool block_next_direct(AnyPtr& ret);

private:
	ArrayPtr array_;
	uint_t index_;
	bool reverse_;

	virtual void visit_members(Visitor& m);
};

class MultiValue : public RefCountingBase{
public:
	enum{ TYPE = TYPE_MULTI_VALUE };

	MultiValue(const AnyPtr& head, const MultiValuePtr& tail = null)
		:head_(head), tail_(tail){
		set_pvalue(*this, TYPE, this);
	}

	const AnyPtr& head(){
		return head_;
	}

	const MultiValuePtr& tail(){
		return tail_;
	}

	void block_next(const VMachinePtr& vm);

	int_t size();

	const AnyPtr& at(int_t i);

	void visit_members(Visitor& m){
		m & head_ & tail_;
	}

public:

	void set(const AnyPtr& head, const MultiValuePtr& tail = null){
		head_ = head;
		tail_ = tail;
	}

private:
	AnyPtr head_;
	MultiValuePtr tail_;
};

void bind(MultiValue*);

inline MultiValuePtr mv(const AnyPtr& v1, const AnyPtr& v2){
	return xnew<MultiValue>(v1, xnew<MultiValue>(v2));
}

}
