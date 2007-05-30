
#pragma once

#include "any.h"
#include "visitor.h"

namespace xtal{

template<class T>
class NullInitialized : public T{
public:
	NullInitialized():T(null){}
	NullInitialized(const T& v){ T::operator =(v); }
	NullInitialized& operator =(const T& v){ T::operator =(v); return *this; }
};

template<class T>
class LLVar : public T{
public:
	LLVar(){ add_long_life_var(this); }
	LLVar(const LLVar<T>& v):T(v){ add_long_life_var(this); }
	template<class U> LLVar(const U& v):T(v){ add_long_life_var(this); }
	~LLVar(){ remove_long_life_var(this); }
	template<class U> LLVar<T>& operator =(const U& v){ this->T::operator=(v); return *this; }
};

class GCObserverImpl : public AnyImpl{
public:
	GCObserverImpl();
	GCObserverImpl(const GCObserverImpl& v);
	virtual ~GCObserverImpl();
	virtual void before_gc(){}
	virtual void after_gc(){}
};

class HaveNameImpl : public AnyImpl{
public:

	HaveNameImpl()
		:name_(null), force_(0){}

	virtual String object_name(){
		if(!name_)
			return String("<instance of ").cat(((Any&)get_class()).object_name()).cat(">");
		if(!parent_)
			return name_;
		return parent_.object_name().cat("::").cat(name_);
	}

	virtual int_t object_name_force(){
		return force_;
	}

	virtual void set_object_name(const String& name, int_t force, const Any& parent){
		if(!name_ || force_<force){
			name_ = name;
			force_ = force;
			parent_ = parent;
		}
	}

protected:

	String name_;
	Any parent_;
	int_t force_;

	virtual void visit_members(Visitor& m){
		AnyImpl::visit_members(m);
		m & name_ & parent_;
	}	
};

}
