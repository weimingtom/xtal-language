#include "xtal.h"

#include "xtal_iterator.h"
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

void block_next(AnyPtr& target, AnyPtr& value1, bool first){
	const VMachinePtr& vm = vmachine();
	vm->setup_call(2);
	target->rawsend(vm, first ? Xid(block_first) : Xid(block_next));
	target = vm->result(0);
	value1 = vm->result(1);
	vm->cleanup_call();
}

void block_next(AnyPtr& target, AnyPtr& value1, AnyPtr& value2, bool first){
	const VMachinePtr& vm = vmachine();
	vm->setup_call(3);
	target->rawsend(vm, first ? Xid(block_first) : Xid(block_next));
	target = vm->result(0);
	value1 = vm->result(1);
	value2 = vm->result(2);
	vm->cleanup_call();
}

void block_next(AnyPtr& target, AnyPtr& value1, AnyPtr& value2, AnyPtr& value3, bool first){
	const VMachinePtr& vm = vmachine();
	vm->setup_call(4);
	target->rawsend(vm, first ? Xid(block_first) : Xid(block_next));
	target = vm->result(0);
	value1 = vm->result(1);
	value2 = vm->result(2);
	value3 = vm->result(3);
	vm->cleanup_call();
}


}
