/** \file src/xtal/xtal_array.h
* \brief src/xtal/xtal_array.h
*/

#ifndef XTAL_ARRAY_H_INCLUDE_GUARD
#define XTAL_ARRAY_H_INCLUDE_GUARD

#pragma once

namespace xtal{

/**
* \xbind lib::builtin
* \xinherit lib::builtin::Any lib::builtin::Iterable
* \brief 配列
*/
class Array : public RefCountingBase{
public:
	enum{ TYPE = TYPE_ARRAY };

	/**
	* \xbind
	* \brief sizeの長さの配列を生成する 
	*/
	Array(uint_t size = 0);

	Array(const AnyPtr* first, const AnyPtr* end);

	/**
	* \brief コピーコンストラクタを備える
	*/
	Array(const Array& v);

	/**
	* \brief 代入演算子を備える
	*/
	Array& operator =(const Array& v);

	/**
	* \brief デストラクタ
	*/
	~Array();

	/**
	* \xbind
	* \brief 配列の長さを返す
	*/
	uint_t length(){
		return size_;
	}

	/**
	* \xbind
	* \brief 配列の長さを返す
	*/
	uint_t size(){
		return size_;
	}

	/**
	* \xbind
	* \brief 配列の長さを変更する
	*/
	void resize(uint_t sz);

	/**
	* \xbind
	* \brief 配列をsz分長くする
	*/
	void upsize(uint_t sz);

	/**
	* \xbind
	* \brief 配列をsz分短くする
	*/
	void downsize(uint_t sz);

	/**
	* \brief i番目の要素を返す
	*/
	const AnyPtr& at(int_t i){
		XTAL_ASSERT(0<=i && (uint_t)i<size_);
		return values_[i];
	}

	/**
	* \brief i番目の要素を設定する
	*/
	void set_at(int_t i, const AnyPtr& v){
		XTAL_ASSERT(0<=i && (uint_t)i<size_);
		values_[i] = v;
	}

	/**
	* \xbind
	* \brief i番目の要素を返す
	*/
	const AnyPtr& op_at(int_t i);

	/**
	* \xbind
	* \brief i番目の要素を設定する
	*/
	void op_set_at(int_t i, const AnyPtr& v);

	/**
	* \xbind
	* \brief 先頭に要素を追加する
	*/
	void push_front(const AnyPtr& v){
		insert(0, v);
	}

	/**
	* \xbind
	* \brief 先頭の要素を削除する
	*/
	void pop_front(){
		erase(0);
	}

	/**
	* \xbind
	* \brief 末尾に要素を追加する
	*/
	void push_back(const AnyPtr& v);

	/**
	* \xbind
	* \brief 末尾の要素を削除する
	*/
	void pop_back();

	/**
	* \xbind
	* \brief 先頭の要素を返す
	*/
	const AnyPtr& front(){
		return op_at(0);
	}

	/**
	* \xbind
	* \brief 末尾の要素を返す
	*/
	const AnyPtr& back(){
		return op_at(size()-1);
	}

	/**
	* \xbind
	* \brief i番目からn個の部分配列を返す
	*/
	ArrayPtr slice(int_t i, int_t n = 1);
	
	/**
	* \xbind
	* \brief indexからn個の部分配列を削除し、その部分配列を返す
	*/
	ArrayPtr splice(int_t i, int_t n = 1);

	/**
	* \xbind
	* \brief i番目のn個の要素を削除する
	*/
	void erase(int_t i, int_t n = 1);

	/**
	* \xbind
	* \brief i番目に要素を追加する
	*/
	void insert(int_t i, const AnyPtr& v);

	/**
	* \xbind
	* \brief 配列の要素を逆順にするイテレータを返す
	*/
	AnyPtr reverse();
	
	/**
	* \xbind
	* \brief 浅いコピーを返す
	*/
	ArrayPtr clone();

	/**
	* \brief 連結した配列を返す
	*/
	ArrayPtr op_cat(const ArrayPtr& a);

	/**
	* \brief 自身を連結し、自身を返す
	*/
	ArrayPtr op_cat_assign(const ArrayPtr& a);
	
	/**
	* \xbind
	* \brief 要素を文字列として連結した結果を返す
	* \param sep 要素と要素の区切り文字列
	*/
	StringPtr join(const StringPtr& sep);

	/**
	* \xbind
	* \brief 自身を返す
	*/
	ArrayPtr op_to_array(){
		return to_smartptr(this);
	}

	/**
	* \xbind
	* \brief 値が等しいか調べる
	*/
	bool op_eq(const ArrayPtr& other);

	/**
	* \xbind
	* \brief 空か調べる
	*/
	bool empty(){
		return size_ == 0;
	}

	/**
	* \xbind
	* \brief 空か調べる
	*/
	bool is_empty(){
		return size_ == 0;
	}

	/**
	* \xbind
	* \brief 要素を全て削除する
	*/
	void clear();

	/**
	* \xbind
	* \brief 要素を最初から反復できるIteratorを返す
	*/
	AnyPtr each();

	/**
	* \xbind
	* \brief 中身をiteratorで取得できる要素に置き換える
	*/
	void assign(const AnyPtr& iterator);

	/**
	* \xbind
	* \brief iteratorで取得できる要素を追加する
	*/
	void append(const AnyPtr& iterator);

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

	const AnyPtr* data(){
		return values_;
	}

	void attach(AnyPtr* data, uint_t size){
		values_ = data;
		size_ = size;
		capa_ = size;
	}

	void detach(){
		values_ = 0;
		size_ = 0;
		capa_ = 0;
	}

	void set_at_unref(int_t i, const Any& value){
		(Any&)values_[i] = value;
	}

	void resize_unref(uint_t size);

	void clear_unref();

protected:

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

}

#endif // XTAL_ARRAY_H_INCLUDE_GUARD
