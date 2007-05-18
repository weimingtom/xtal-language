
#pragma once

#include "string.h"
#include "utilimpl.h"
#include "marshal.h"

namespace xtal{

uint_t make_hashcode(const char* str, int_t size);

class StringImpl : public AnyImpl{
public:

	
	void common_init(int_t len);
	
	StringImpl(const char* str){
		common_init(strlen(str));
		memcpy(str_, str, size_);
		hashcode_ = make_hashcode(c_str(), size());
	}

	StringImpl(const char* str, int_t len){
		common_init(len);
		memcpy(str_, str, size_);
		hashcode_ = make_hashcode(c_str(), size());
	}

	StringImpl(const char* begin, const char* last){
		common_init(last-begin);
		memcpy(str_, begin, size_);
		hashcode_ = make_hashcode(c_str(), size());
	}

	StringImpl(const char* str1, int_t size1, const char* str2, int_t size2){
		common_init(size1 + size2);
		memcpy(str_, str1, size1);
		memcpy(str_+size1, str2, size2);
		hashcode_ = make_hashcode(c_str(), size());
	}

	StringImpl(const string_t& str){
		common_init(str.size());
		memcpy(str_, str.c_str(), size_);
		hashcode_ = make_hashcode(c_str(), size());
	}

	StringImpl(const char* str, int_t len, uint_t hashcode){
		common_init(len);
		memcpy(str_, str, size_);
		hashcode_ = hashcode;
		intern_ = true;
	}

	StringImpl(char* str, int_t len, int_t buffer_size, String::delegate_memory_t){
		set_class(TClass<String>::get());
		size_ = len;
		str_ = str;
		intern_ = false;
		hashcode_ = make_hashcode(c_str(), size());
	}

	virtual ~StringImpl(){
		user_free(str_, size_+1);
	}
		
	const char* c_str(){ return str_; }
	int_t size(){ return size_; }
	int_t length(){ return size_; }
	String slice(int_t first, int_t last){ return String(c_str()+first, last-first); }
	uint_t hashcode(){ return hashcode_; }
	String clone(){ return String(this); }
	ID intern(){ return String(this); }
	bool is_interned(){ return intern_; }

	int_t to_i(){ return atoi(c_str()); }
	float_t to_f(){ return (float_t)atof(c_str()); }
	String to_s(){ return String(this); }
	
	class StringSplitImpl : public AnyImpl{
		String str_, sep_;
		int_t index_;

		virtual void visit_members(Visitor& m){
			AnyImpl::visit_members(m);
			m & str_ & sep_;
		}

	public:

		StringSplitImpl(const String& str, const String& sep)
			:str_(str), sep_(sep), index_(0){
			set_class(TClass<StringSplitImpl>::get());
		}
		
		Any restart(){
			index_ = 0;
			return this;
		}

		void iter_next(const VMachine& vm){
			if(str_.size()<=index_){
				vm.return_result(null);
				return;
			}
			const char* sep = sep_.c_str();
			const char* str = str_.c_str();

			if(sep[0]==0){
				vm.return_result(this, String(&str[index_], 1));
				index_ += 1;
				return;
			}

			for(int_t j=index_, jsz=str_.size(); j<jsz; ++j){
				for(int_t i=0, sz=sep_.size()+1; i<sz; ++i){
					if(sep[i]==0){
						vm.return_result(this, String(&str[index_], j-index_));
						index_ = j+i;
						return;
					}
					if(str[j+i]==0){
						vm.return_result(this, String(&str[index_], j-index_));
						index_ = j+i;
						return;
					}
					if(str[j+i]!=sep[i]){
						break;
					}
				}
			}
			vm.return_result(this, String(&str[index_], str_.size()-index_));
			index_ = str_.size();
		}
	};

	Any split(const String& sep){
		Any ret; new(ret) StringSplitImpl(String(this), sep);
		return ret;
	}

	String op_cat_String(const String& v){
		if(v){
			return String(c_str(), size(), v.c_str(), v.size());
		}
		return String(this);
	}
	
	bool op_eq_r_String(const String& v){ 
		return v.size()==size() && memcmp(v.c_str(), c_str(), size())==0; 
	}

	bool op_lt_r_String(const String& v){ 
		return strcmp(v.c_str(), c_str())<0; 
	}

private:

	char* str_;
	int_t size_;
	int_t hashcode_;
	bool intern_;

};

}

