
#include "xtal.h"
#include "utilimpl.h"

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
		String ret;
		for(int_t i = 0, sz = size(); i<sz; ++i){
			ret = ret.cat(at(i).to_s());
			if(i<sz-1){
				ret = ret.cat(sep);
			}
		}
		return ret;
	}

	String to_s(){
		String str("[");
		str = str.cat(join(","));
		str = str.cat(String("]"));
		return str;
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

		virtual void visit_members(Visitor& m){
			AnyImpl::visit_members(m);
			m & array_;
		}

	public:

		ArrayIterImpl(const Array& a)
			:array_(a), index_(-1){
			set_class(TClass<ArrayIterImpl>::get());
		}
		
		void iter_first(const VMachine& vm){
			index_ = -1;
			iter_next(vm);
		}

		void iter_next(const VMachine& vm){
			++index_;
			if(index_<array_.size()){
				vm.return_result(this, array_.at(index_));
			}else{
				vm.return_result(null);
			}
		}
	};

	Any each() const{
		Any ret; new(ret) ArrayIterImpl(Array(this));
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
