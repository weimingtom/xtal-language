
#include "xtal.h"
#include "xtal_utilimpl.h"
#include "xtal_stream.h"

namespace xtal{

class ArrayImpl : public AnyImpl{
public:

	ArrayImpl(int_t size)
		:values_(size){
		set_class(TClass<Array>::get());	
	}

	int_t size() const{
		return values_.size();
	}

	void resize(int_t sz){
		values_.resize(sz);
	}

	int_t length() const{
		return values_.size();
	}

	const Any& at(int_t i) const{
		if(i>=0){
			if(i<(int_t)values_.size()){
				return values_[i];
			}else{
				return null;
			}
		}else{
			if(-i-1<(int_t)values_.size()){
				return values_[values_.size()+i];
			}else{
				return null;
			}
		}
	}

	void set_at(int_t i, const Any& v){
		if(i>=0){
			if(i<(int_t)values_.size()){
				values_[i] = v;
			}else{
				values_.resize((int_t)values_.size());
				values_[i] = v;
			}
		}else{
			if(-i-1<(int_t)values_.size()){
				values_[values_.size()+i] = v;
			}else{
				values_.resize((int_t)values_.size());
				values_[i] = v;				
			}
		}
	}

	void push_front(const Any& v){
		values_.insert(values_.begin(), v);
	}

	void pop_front(){
		values_.erase(values_.begin());
	}

	void push_back(const Any& v){
		values_.push_back(v);
	}

	void pop_back(){
		values_.pop_back();
	}

	Array slice(int_t first, int_t last) const{
		Array ret(last-first);
		for(int_t i = first; i!=last; ++i){
			ret.set_at(i-first, at(i));
		}
		return ret;
	}

	void erase(int_t i){
		values_.erase(values_.begin()+i);
	}

	void insert(int_t i, const Any& v){
		values_.insert(values_.begin()+i, v);
	}

	void reverse(){
		int_t sz = size();
		for(int_t i=0; i<sz/2; ++i){
			values_[sz-1-i].swap(values_[i]);
		}
	}

	Array reversed(){
		int_t sz = size();
		Array ret(size());
		for(int_t i=0; i<sz; ++i){
			ret.impl()->values_[sz-1-i] = values_[i];
		}
		return ret;
	}

	Array clone() const{
		Array ret(null); new(ret) ArrayImpl(*this);
		return ret;
	}

	Array cat(const Array& a){
		Array ret(clone());
		ret.cat_assign(a);
		return ret;
	}

	Array cat_assign(const Array& a){
		for(int_t i = 0, size = a.size(); i<size; ++i){
			push_back(a.at(i));
		}
		return Array(this);
	}
	
	String join(const String& sep){
		MemoryStream ret;
		for(int_t i = 0, sz = size(); i<sz; ++i){
			ret.put_s(at(i).to_s());
			if(i<sz-1){
				ret.put_s(sep);
			}
		}
		return ret.to_s();
	}

	String to_s(){
		MemoryStream ret;
		ret.put_s("[");
		ret.put_s(join(","));
		ret.put_s("]");
		return ret.to_s();
	}

	bool op_eq(const Array& other){
		if(size()!=other.size())
			return false;
		for(int_t i=0, size=other.size(); i<size; ++i){
			if(at(i)!=other.at(i)){
				return false;
			}
		}
		return true;
	}

	bool empty(){
		return values_.empty();
	}

	void clear(){
		values_.clear();
	}

	class ArrayIterImpl : public AnyImpl{
		Array array_;
		int_t index_;
		bool reverse_;

		virtual void visit_members(Visitor& m){
			AnyImpl::visit_members(m);
			m & array_;
		}

	public:

		ArrayIterImpl(const Array& a, bool reverse = false)
			:array_(a), index_(-1), reverse_(reverse){
			set_class(TClass<ArrayIterImpl>::get());
		}

		Any reset(){
			index_ = -1;
			return this;
		}
				
		void iter_next(const VMachine& vm){
			++index_;
			if(index_<array_.size()){
				vm.return_result(this, array_.at(reverse_ ? array_.size()-1-index_ : index_));
			}else{
				reset();
				vm.return_result(null, null);
			}
		}

		void remove(){
			if(-1<=index_ && index_<array_.size()-1){
				array_.erase(index_+1);
			}
		}
	};

	Any each() const{
		Any ret; new(ret) ArrayIterImpl(Array(this));
		return ret;
	}

	Any r_each() const{
		Any ret; new(ret) ArrayIterImpl(Array(this), true);
		return ret;
	}

protected:

	typedef AC<Any>::vector vector_t;
	vector_t values_;

	virtual void visit_members(Visitor& m){
		AnyImpl::visit_members(m);
		m & values_;
	}
};

}
