
#pragma once

namespace xtal{

class DelegateToIterator : public HaveName{
	IDPtr member_;
public:
	
	DelegateToIterator(const IDPtr& name)
		:member_(name){}

	virtual void call(const VMachinePtr& vm);
};

class IteratorClass : public Class{
public:
	virtual void def(const IDPtr& name, const AnyPtr& value, const AnyPtr& secondary_key, int_t accessibility);
};

struct BlockValueHolder1{
	
	BlockValueHolder1(const AnyPtr& tar = null);
	~BlockValueHolder1();

	BlockValueHolder1(const BlockValueHolder1&);
	BlockValueHolder1& operator=(const BlockValueHolder1&);

	AnyPtr target;
	AnyPtr values[1];

	ArrayPtr array;
	Array::iterator it;

	operator bool(){ return target; }
};

struct BlockValueHolder2{
	
	BlockValueHolder2(const AnyPtr& tar = null);
	~BlockValueHolder2();

	BlockValueHolder2(const BlockValueHolder2&);
	BlockValueHolder2& operator=(const BlockValueHolder2&);

	AnyPtr target;
	AnyPtr values[2];

	MapPtr map;
	Map::iterator it;

	operator bool(){ return target; }
};

struct BlockValueHolder3{
	BlockValueHolder3(const AnyPtr& tar = null);
	~BlockValueHolder3();

	BlockValueHolder3(const BlockValueHolder3&);
	BlockValueHolder3& operator=(const BlockValueHolder3&);

	AnyPtr target;
	AnyPtr values[3];

	operator bool(){ return target; }
};

void block_next(BlockValueHolder1& holder, bool first);
void block_next(BlockValueHolder2& holder, bool first);
void block_next(BlockValueHolder3& holder, bool first);
void block_break(AnyPtr& target);

}
