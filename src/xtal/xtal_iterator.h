
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
	virtual void def(const InternedStringPtr& name, const AnyPtr& value, int_t accessibility, const AnyPtr& ns);
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
	
	BlockValueHolder(const AnyPtr& tar=null)
		:target(tar){
		array = ptr_as<Array>(tar);
		if(array){ it = array->begin(); }
	}
	
	~BlockValueHolder(){ block_break(target); }

	AnyPtr target;
	AnyPtr values[2];

	ArrayPtr array;
	Array::iterator it;

	operator bool(){ return target; }
};

template<>
struct BlockValueHolder<2>{
	
	BlockValueHolder(const AnyPtr& tar=null)
		:target(tar){
		map = ptr_as<Map>(tar);
		if(map){ it = map->begin(); }
	}
	
	~BlockValueHolder(){ block_break(target); }

	AnyPtr target;
	AnyPtr values[2];

	MapPtr map;
	Map::iterator it;

	operator bool(){ return target; }
};

}