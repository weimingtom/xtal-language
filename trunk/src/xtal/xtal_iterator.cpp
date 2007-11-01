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

void initialize_iterator(){
	{
		ClassPtr p = Iterator();
		p->fun("each", &Iterator_each);
		p->fun("block_first", &Iterator_block_first);
	}
}

void DelegateToIterator::call(const VMachinePtr& vm){
	vm->get_arg_this()->send(Xid(each))->rawsend(vm, member_);
}

void IteratorClass::def(const IDPtr& primary_key, const AnyPtr& value, const AnyPtr& secondary_key, int_t accessibility){
	Class::def(primary_key, value, secondary_key, accessibility);
	if(rawne(Xid(p), primary_key) && rawne(Xid(each), primary_key)){
		Enumerator()->def(primary_key, xnew<DelegateToIterator>(primary_key), secondary_key, accessibility);
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

void block_next(BlockValueHolder<1>& holder, bool first){
	if(holder.array){
		if(holder.it==holder.array->end()){
			holder.target = null;
			holder.values[0] = null;
		}else{
			holder.values[0] = *holder.it;
			++holder.it;
		}
	}else{
		const VMachinePtr& vm = vmachine();
		vm->setup_call(2);
		holder.target->rawsend(vm, first ? Xid(block_first) : Xid(block_next));
		holder.target = vm->result(0);
		holder.values[0] = vm->result(1);
		vm->cleanup_call();
	}
}

void block_next(BlockValueHolder<2>& holder, bool first){
	if(holder.map){
		if(holder.it==holder.map->end()){
			holder.target = null;
			holder.values[0] = null;
			holder.values[1] = null;
		}else{
			holder.values[0] = holder.it->first;
			holder.values[1] = holder.it->second;
			++holder.it;
		}
	}else{
		const VMachinePtr& vm = vmachine();
		vm->setup_call(3);
		holder.target->rawsend(vm, first ? Xid(block_first) : Xid(block_next));
		holder.target = vm->result(0);
		holder.values[0] = vm->result(1);
		holder.values[1] = vm->result(2);
		vm->cleanup_call();
	}
}

void block_next(BlockValueHolder<3>& holder, bool first){
	const VMachinePtr& vm = vmachine();
	vm->setup_call(4);
	holder.target->rawsend(vm, first ? Xid(block_first) : Xid(block_next));
	holder.target = vm->result(0);
	holder.values[0] = vm->result(1);
	holder.values[1] = vm->result(2);
	holder.values[2] = vm->result(3);
	vm->cleanup_call();
}


}
