
#pragma once

namespace xtal{

const ArrayPtr& PseudoArray_arrayize(const AnyPtr& v);
const ArrayPtr& PseudoArray_extarct_array(const AnyPtr& v);

class DelegateToIterator : public HaveName{
	InternedStringPtr member_;
public:
	
	DelegateToIterator(const InternedStringPtr& name)
		:member_(name){}

	virtual void call(const VMachinePtr& vm);
};

class DelegateToIteratorOrArray : public HaveName{
	InternedStringPtr member_;
public:

	DelegateToIteratorOrArray(const InternedStringPtr& name)
		:member_(name){}

	virtual void call(const VMachinePtr& vm);
};

class IteratorClass : public Class{
public:
	virtual void def(const InternedStringPtr& name, const AnyPtr& value, int_t accessibility, const AnyPtr& ns){
		Class::def(name, value, accessibility, ns);
		Enumerator()->def(name, xnew<DelegateToIterator>(name), accessibility, ns);
		PseudoArray()->def(name, xnew<DelegateToIteratorOrArray>(name), accessibility, ns);
	}
};

}
