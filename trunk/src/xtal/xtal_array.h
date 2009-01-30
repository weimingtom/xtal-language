
#pragma once

namespace xtal{

/**
* @brief 配列
*
*/
class Array : public Base{
public:

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
	virtual ~Array();

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
	* @brief 配列の長さを返す
	*
	*/
	uint_t length(){
		return size_;
	}

	/**
	* @brief i番目の要素を返す
	*
	*/
	const AnyPtr& at(int_t i){
		return values_[i];
	}

	/**
	* @brief i番目の要素を設定する
	*
	*/
	void set_at(int_t i, const AnyPtr& v){
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
	* @brief この配列の文字列表現を返す
	*
	*/
	StringPtr to_s();

	/**
	* @brief 自身を返す
	*
	*/
	ArrayPtr to_a(){
		return from_this(this);
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
	* @brief 要素を全て削除する
	*
	*/
	void clear();

	/**
	* @brief 要素を最初から持つIteratorを返す
	*
	*/
	AnyPtr each();

	/**
	* @brief 中身をiteratorで取得できる要素に置き換える
	*/
	void assign(const AnyPtr& iterator);

	void concat(const AnyPtr& iterator);

	/*
	AnyPtr* data(){
		return values_;
	}
	*/
public:

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

	virtual void visit_members(Visitor& m);
};

class MultiValue : public Array{
public:

	MultiValue(int_t size = 0)
		:Array(size){}

	MultiValuePtr clone();

	MultiValuePtr flatten_mv();

	MultiValuePtr flatten_all_mv();

	StringPtr to_s();

};

inline MultiValuePtr mv(){
	return xnew<MultiValue>();
}

inline MultiValuePtr mv(const AnyPtr& v){
	MultiValuePtr ret = xnew<MultiValue>();
	ret->push_back(v);
	return ret;
}

inline MultiValuePtr mv(const AnyPtr& v1, const AnyPtr& v2){
	MultiValuePtr ret = xnew<MultiValue>();
	ret->push_back(v1);
	ret->push_back(v2);
	return ret;
}

}
