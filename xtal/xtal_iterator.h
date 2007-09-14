
#pragma once

namespace xtal{

class DelegateToIterator : public HaveName{
	InternedStringPtr member_;
public:
	
	DelegateToIterator(const InternedStringPtr& name)
		:member_(name){}

	virtual void call(const VMachinePtr& vm);
};

class IteratorClass : public Class{
public:
	virtual void def(const InternedStringPtr& name, const AnyPtr& value, int_t accessibility, const AnyPtr& ns){
		Class::def(name, value, accessibility, ns);
		Enumerator()->def(name, xnew<DelegateToIterator>(name), accessibility, ns);
	}
};

void block_next(AnyPtr& target, AnyPtr& value, bool first);
void block_next(AnyPtr& target, AnyPtr& value1, AnyPtr& value2, bool first);
void block_next(AnyPtr& target, AnyPtr& value1, AnyPtr& value2, AnyPtr& value3, bool first);
void block_break(AnyPtr& target);

template<int N>
struct BlockValueHolder{
	BlockValueHolder(const AnyPtr& tar=null):target(tar){}
	~BlockValueHolder(){ block_break(target); }

	AnyPtr target;
	AnyPtr values[N];

	operator bool(){ return target; }
};

}
