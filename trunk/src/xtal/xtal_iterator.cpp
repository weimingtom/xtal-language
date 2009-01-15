#include "xtal.h"
#include "xtal_macro.h"

namespace xtal{
	


namespace{

void Iterator_each(const VMachinePtr& vm){
	vm->return_result(vm->get_arg_this());
}

void Iterator_block_first(const VMachinePtr& vm){
	vm->get_arg_this()->rawsend(vm, Xid(block_next));
}

}

class ZipIter : public Base{
public:

	ZipIter(const VMachinePtr& vm){
		next = xnew<Array>(vm->ordered_arg_count());
		for(int_t i = 0, len = next->size(); i<len; ++i){
			next->set_at(i, vm->arg(i));
		}
	}

	void common(const VMachinePtr& vm, const IDPtr& id){
		bool all = true;
		value = xnew<MultiValue>(next->size());
		for(int_t i = 0, len = next->size(); i<len; ++i){
			vm->setup_call(2);
			next->at(i)->rawsend(vm, id);
			next->set_at(i, vm->result(0));
			value->set_at(i, vm->result(1));
			vm->cleanup_call();
			if(!next->at(i))
				all = false;
		}
		if(all){
			vm->return_result(from_this(this), value);
		}
		else{
			vm->return_result(null, null);
		}
	}

	void block_first(const VMachinePtr& vm){
		common(vm, Xid(block_first));
	}
	
	void block_next(const VMachinePtr& vm){
		common(vm, Xid(block_next));
	}

	void block_break(const VMachinePtr& vm){
		IDPtr id = Xid(block_break);
		for(int_t i = 0, len = next->size(); i<len; ++i){
			vm->setup_call(0);
			next->at(i)->rawsend(vm, id);
			if(!vm->processed()){
				vm->return_result();	
			}
			vm->cleanup_call();
		}
		vm->return_result();
	}

	virtual void visit_members(Visitor& m){
		Base::visit_members(m);
		m & next & value;
	}

	ArrayPtr next;
	ArrayPtr value;
};

void InitZipIter(){
	ClassPtr p = new_cpp_class<ZipIter>("ZipIter");
	p->inherit(Iterator());
	p->def("new", ctor<ZipIter, const VMachinePtr&>());
	p->method("block_first", &ZipIter::block_first);
	p->method("block_next", &ZipIter::block_next);
	p->method("block_break", &ZipIter::block_break);
}

void initialize_iterator(){
	{
		ClassPtr p = Iterator();
		p->inherit(Iterable());

		p->fun("each", &Iterator_each);
		p->fun("block_first", &Iterator_block_first);
	}

	InitZipIter();
	builtin()->def("zip", get_cpp_class<ZipIter>());

}

void DelegateToIterator::call(const VMachinePtr& vm){
	vm->get_arg_this()->send(Xid(each))->rawsend(vm, member_);
}

void IteratorClass::def(const IDPtr& primary_key, const AnyPtr& value, const AnyPtr& secondary_key, int_t accessibility){
	Class::def(primary_key, value, secondary_key, accessibility);
	if(rawne(Xid(p), primary_key) && rawne(Xid(each), primary_key)){
		Iterable()->def(primary_key, xnew<DelegateToIterator>(primary_key), secondary_key, accessibility);
	}
}

void block_break(AnyPtr& target){
	if(target){
		const VMachinePtr& vm = vmachine();
		vm->setup_call(0);
		target->rawsend(vm, Xid(block_break));
		if(!vm->processed()){
			vm->return_result();
		}
		vm->cleanup_call();
	}
}

void block_next(BlockValueHolder1& holder, bool first){
	if(holder.array){
		if(holder.it==holder.array->end()){
			holder.target = null;
			holder.values[0] = null;
		}
		else{
			holder.values[0] = *holder.it;
			++holder.it;
		}
	}
	else{
		const VMachinePtr& vm = vmachine();
		vm->setup_call(2);
		holder.target->rawsend(vm, first ? Xid(block_first) : Xid(block_next));
		holder.target = vm->result(0);
		holder.values[0] = vm->result(1);
		vm->cleanup_call();
	}
}

void block_next(BlockValueHolder2& holder, bool first){
	if(holder.map){
		if(holder.it==holder.map->end()){
			holder.target = null;
			holder.values[0] = null;
			holder.values[1] = null;
		}
		else{
			holder.values[0] = holder.it->first;
			holder.values[1] = holder.it->second;
			++holder.it;
		}
	}
	else{
		const VMachinePtr& vm = vmachine();
		vm->setup_call(3);
		holder.target->rawsend(vm, first ? Xid(block_first) : Xid(block_next));
		holder.target = vm->result(0);
		holder.values[0] = vm->result(1);
		holder.values[1] = vm->result(2);
		vm->cleanup_call();
	}
}

void block_next(BlockValueHolder3& holder, bool first){
	const VMachinePtr& vm = vmachine();
	vm->setup_call(4);
	holder.target->rawsend(vm, first ? Xid(block_first) : Xid(block_next));
	holder.target = vm->result(0);
	holder.values[0] = vm->result(1);
	holder.values[1] = vm->result(2);
	holder.values[2] = vm->result(3);
	vm->cleanup_call();
}

BlockValueHolder1::BlockValueHolder1(const AnyPtr& tar)
	:target(tar){
	array = ptr_as<Array>(tar);
	if(array){ it = array->begin(); }
}

BlockValueHolder1::~BlockValueHolder1(){ 
	block_break(target); 
}

BlockValueHolder1::BlockValueHolder1(const BlockValueHolder1& a){
	target = a.target;
	values[0] = a.values[0];

	array = a.array;
	it = a.it;
}

BlockValueHolder1& BlockValueHolder1::operator=(const BlockValueHolder1& a){
	target = a.target;
	values[0] = a.values[0];

	array = a.array;
	it = a.it;
	return *this;
}
	
BlockValueHolder2::BlockValueHolder2(const AnyPtr& tar)
	:target(tar){
	map = ptr_as<Map>(tar);
	if(map){ it = map->begin(); }
}

BlockValueHolder2::~BlockValueHolder2(){ 
	block_break(target); 
}

BlockValueHolder2::BlockValueHolder2(const BlockValueHolder2& a){
	target = a.target;
	values[0] = a.values[0];
	values[1] = a.values[1];

	map = a.map;
	it = a.it;
}

BlockValueHolder2& BlockValueHolder2::operator=(const BlockValueHolder2& a){
	target = a.target;
	values[0] = a.values[0];
	values[1] = a.values[1];

	map = a.map;
	it = a.it;
	return *this;
}

BlockValueHolder3::BlockValueHolder3(const AnyPtr& tar)
	:target(tar){
}

BlockValueHolder3::~BlockValueHolder3(){ 
	block_break(target); 
}

BlockValueHolder3::BlockValueHolder3(const BlockValueHolder3& a){
	target = a.target;
	values[0] = a.values[0];
	values[1] = a.values[1];
	values[2] = a.values[2];
}

BlockValueHolder3& BlockValueHolder3::operator=(const BlockValueHolder3& a){
	target = a.target;
	values[0] = a.values[0];
	values[1] = a.values[1];
	values[2] = a.values[2];
	return *this;
}


}
