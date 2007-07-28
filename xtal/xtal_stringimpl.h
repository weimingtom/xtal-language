
#pragma once

#include "xtal_string.h"
#include "xtal_utilimpl.h"
#include "xtal_serializer.h"
#include "xtal_ch.h"

namespace xtal{

uint_t make_hashcode(const char* str, uint_t size);

class StringImpl : public AnyImpl{

	StringImpl(const StringImpl&);
	StringImpl& operator=(const StringImpl&);

public:
	
	void common_init(uint_t len);
	
	StringImpl(const char* str){
		common_init(strlen(str));
		memcpy(str_.p, str, size_);
	}

	StringImpl(const char* str, uint_t len){
		common_init(len);
		memcpy(str_.p, str, size_);
	}

	StringImpl(const char* begin, const char* last){
		common_init(last-begin);
		memcpy(str_.p, begin, size_);
	}

	StringImpl(const char* str1, uint_t size1, const char* str2, uint_t size2){
		common_init(size1 + size2);
		memcpy(str_.p, str1, size1);
		memcpy(str_.p+size1, str2, size2);
	}

	StringImpl(const string_t& str){
		common_init(str.size());
		memcpy(str_.p, str.c_str(), size_);
	}

	StringImpl(const char* str, uint_t len, uint_t hashcode){
		common_init(len);
		memcpy(str_.p, str, size_);
		str_.hashcode = hashcode;
		flags_ = INTERNED | HASHED;
	}

	StringImpl(char* str, uint_t len, uint_t /*buffer_size*/, String::delegate_memory_t){
		set_class(TClass<String>::get());
		size_ = len;
		str_.p = str;
		flags_ = 0;
	}

	StringImpl(StringImpl* left, StringImpl* right){
		set_class(TClass<String>::get());
		left->inc_ref_count();
		right->inc_ref_count();
		rope_.left = left;
		rope_.right = right;
		size_ = left->size() + right->size();
		flags_ = ROPE;
	}

	virtual ~StringImpl(){
		if((flags_ & ROPE)==0){
			if((flags_ & NOFREE)==0){
				user_free(str_.p, size_+1);
			}
		}else{
			rope_.left->dec_ref_count();
			rope_.right->dec_ref_count();
		}
	}

	void became_unified();
	static void write_to_memory(StringImpl* p, char_t* memory, uint_t& pos);

	const char* debug_c_str(){
		if((flags_ & ROPE)!=0){
			return "rope";
		}
		return str_.p;
	}

	const char* c_str(){ became_unified(); return str_.p; }
	uint_t size(){ return size_; }
	uint_t length(){ return size_; }
	String slice(int_t first, int_t last){ return String(c_str()+first, last-first); }

	uint_t hashcode(){ 
		if((flags_ & HASHED)!=0)
			return str_.hashcode;		
		became_unified(); 
		str_.hashcode = make_hashcode(str_.p, size_);
		flags_ |= HASHED;
		return str_.hashcode; 
	}

	String clone(){ return String(this); }
	ID intern(){ return String(this); }
	bool is_interned(){ return (flags_ & INTERNED)!=0; }

	int_t to_i(){ return atoi(c_str()); }
	float_t to_f(){ return (float_t)atof(c_str()); }
	String to_s(){ return String(this); }
	
	class StringSplitImpl : public AnyImpl{
		String str_, sep_;
		uint_t index_;

		virtual void visit_members(Visitor& m){
			AnyImpl::visit_members(m);
			m & str_ & sep_;
		}

	public:

		StringSplitImpl(const String& str, const String& sep)
			:str_(str), sep_(sep), index_(0){
			set_class(TClass<StringSplitImpl>::get());
		}
		
		Any reset(){
			index_ = 0;
			return this;
		}

		void iter_next(const VMachine& vm){
			if(str_.size()<=index_){
				reset();
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

			for(int_t j=index_, jsz=str_.size(); j<jsz; j+=ch_len(str[j])){
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
		if(size_+v.impl()->size_ <= 16)
			return String(c_str(), size(), v.c_str(), v.size());
		String ret(null);
		new(ret) StringImpl(this, v.impl());
		return ret;
	}
	
	bool op_eq_r_String(const String& v){ 
		return v.size()==size() && memcmp(v.c_str(), c_str(), size())==0; 
	}

	bool op_lt_r_String(const String& v){ 
		return strcmp(v.c_str(), c_str())<0; 
	}

	virtual void visit_members(Visitor& m){
		AnyImpl::visit_members(m);
		if((flags_ & ROPE)!=0){
			m & rope_.left & rope_.right;
		}
	}

private:

	enum{
		ROPE = 1<<0,
		INTERNED = 1<<1,
		NOFREE = 1<<2,
		HASHED = 1<<3
	};
	uint_t flags_;

	struct Str{
		char* p;
		uint_t hashcode;
	};

	struct Rope{
		StringImpl* left;
		StringImpl* right;
	};

	union{
		Str str_;
		Rope rope_;
	};

	uint_t size_;
};

}

