
#pragma once

namespace xtal{

class DelegateToIterator : public HaveParent{
	IDPtr member_;
public:
	
	DelegateToIterator(const IDPtr& name)
		:member_(name){}

	virtual void rawcall(const VMachinePtr& vm);
};

class Iterator : public Class{
public:
	virtual void def(const IDPtr& name, const AnyPtr& value, const AnyPtr& secondary_key, int_t accessibility);
};

class Iterable;

class ZipIter : public Base{
public:

	ZipIter(const VMachinePtr& vm);

	void common(const VMachinePtr& vm, const IDPtr& id);

	void block_first(const VMachinePtr& vm);
	
	void block_next(const VMachinePtr& vm);

	void block_break(const VMachinePtr& vm);

	virtual void visit_members(Visitor& m);

	ArrayPtr next_;
};

struct BlockValueHolder1{
	
	BlockValueHolder1(const AnyPtr& tar, bool& not_end);
	BlockValueHolder1(const ArrayPtr& tar, bool& not_end);
	~BlockValueHolder1();

	AnyPtr target;
	AnyPtr values[1];

	SmartPtr<ArrayIter> it;
};

struct BlockValueHolder2{
	
	BlockValueHolder2(const AnyPtr& tar, bool& not_end);
	BlockValueHolder2(const MapPtr& tar, bool& not_end);
	~BlockValueHolder2();

	AnyPtr target;
	AnyPtr values[2];

	SmartPtr<MapIter> it;
};

struct BlockValueHolder3{
	BlockValueHolder3(const AnyPtr& tar, bool& not_end);
	~BlockValueHolder3();

	AnyPtr target;
	AnyPtr values[3];
};

bool block_next(BlockValueHolder1& holder, bool first);
bool block_next(BlockValueHolder2& holder, bool first);
bool block_next(BlockValueHolder3& holder, bool first);
void block_break(AnyPtr& target);

}
