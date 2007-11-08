
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


template<int N>
struct BlockValueHolder;

void block_next(BlockValueHolder<1>& holder, bool first);
void block_next(BlockValueHolder<2>& holder, bool first);
void block_next(BlockValueHolder<3>& holder, bool first);
void block_break(AnyPtr& target);

template<int N>
struct BlockValueHolder{
	BlockValueHolder(const AnyPtr& tar=null):target(tar){}
	~BlockValueHolder(){ block_break(target); }

	AnyPtr target;
	AnyPtr values[N];

	operator bool(){ return target; }
};

template<>
struct BlockValueHolder<1>{
	
	BlockValueHolder(const AnyPtr& tar=null);
	~BlockValueHolder(){ block_break(target); }

	AnyPtr target;
	AnyPtr values[2];

	ArrayPtr array;
	Array::iterator it;

	operator bool(){ return target; }
};

template<>
struct BlockValueHolder<2>{
	
	BlockValueHolder(const AnyPtr& tar=null);
	~BlockValueHolder(){ block_break(target); }

	AnyPtr target;
	AnyPtr values[2];

	MapPtr map;
	Map::iterator it;

	operator bool(){ return target; }
};

}
