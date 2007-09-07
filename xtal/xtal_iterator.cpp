#include "xtal.h"

#include "xtal_iterator.h"
#include "xtal_macro.h"

namespace xtal{
	
namespace{

ClassCore iterator_core(0, 2);

}

const ArrayPtr& PseudoArray_arrayize(const AnyPtr& v){
	if(type(v)==TYPE_BASE){
		pvalue(v)->make_instance_variables();
		InstanceVariables* iv = pvalue(v)->instance_variables();
		if(!iv->is_included(&iterator_core)){
			iv->init_variables(&iterator_core);
			ArrayPtr values = xnew<Array>();
			Xfor(it, v){
				values->push_back(it);
			}
			iv->set_variable(0, &iterator_core, values);
		}
		return static_ptr_cast<Array>(iv->variable(0, &iterator_core));
	}else{
		return static_ptr_cast<Array>(null);
	}
}

const ArrayPtr& PseudoArray_extarct_array(const AnyPtr& v){
	if(type(v)==TYPE_BASE){
		if(InstanceVariables* iv = pvalue(v)->instance_variables()){
			if(iv->is_included(&iterator_core)){
				return static_ptr_cast<Array>(iv->variable(0, &iterator_core));
			}
		}
	}
	return static_ptr_cast<Array>(null);
}

namespace{

int_t PseudoArray_length(const AnyPtr& self){
	return PseudoArray_arrayize(self)->length();
}

void PseudoArray_resize(const AnyPtr& self, int_t size){
	PseudoArray_arrayize(self)->resize(size);
}

bool PseudoArray_empty(const AnyPtr& self, int_t index){
	return PseudoArray_arrayize(self)->empty();
}

const AnyPtr& PseudoArray_op_at(const AnyPtr& self, int_t index){
	return PseudoArray_arrayize(self)->op_at(index);
}

void PseudoArray_op_set_at(const AnyPtr& self, int_t index, const AnyPtr& value){
	PseudoArray_arrayize(self)->op_set_at(index, value);
}

AnyPtr PseudoArray_slice(const AnyPtr& self, int_t i, int_t n){
	return PseudoArray_arrayize(self)->slice(i, n);
}

AnyPtr PseudoArray_splice(const AnyPtr& self, int_t i, int_t n){
	return PseudoArray_arrayize(self)->splice(i, n);
}

void PseudoArray_pop_back(const AnyPtr& self){
	PseudoArray_arrayize(self)->pop_back();
}

void PseudoArray_push_back(const AnyPtr& self, const AnyPtr& value){
	PseudoArray_arrayize(self)->push_back(value);
}

void PseudoArray_pop_front(const AnyPtr& self){
	PseudoArray_arrayize(self)->pop_front();
}

void PseudoArray_push_front(const AnyPtr& self, const AnyPtr& value){
	PseudoArray_arrayize(self)->push_front(value);
}

StringPtr PseudoArray_to_s(const AnyPtr& self){
	return PseudoArray_arrayize(self)->to_s();
}

void PseudoArray_erase(const AnyPtr& self, int_t i, int_t n){
	PseudoArray_arrayize(self)->erase(i, n);
}

void PseudoArray_insert(const AnyPtr& self, int_t i, const AnyPtr& value){
	PseudoArray_arrayize(self)->insert(i, value);
}

AnyPtr PseudoArray_cat(const AnyPtr& self, const ArrayPtr& other){
	return PseudoArray_arrayize(self)->cat(other);
}

AnyPtr PseudoArray_cat_assign(const AnyPtr& self, const ArrayPtr& other){
	return PseudoArray_arrayize(self)->cat_assign(other);
}

AnyPtr PseudoArray_r_each(const AnyPtr& self){
	return PseudoArray_arrayize(self)->r_each();
}

AnyPtr PseudoArray_clone(const AnyPtr& self){
	return PseudoArray_arrayize(self)->clone();
}

const AnyPtr& PseudoArray_front(const AnyPtr& self){
	return PseudoArray_arrayize(self)->front();
}

const AnyPtr& PseudoArray_back(const AnyPtr& self){
	return PseudoArray_arrayize(self)->back();
}

void PseudoArray_clear(const AnyPtr& self){
	PseudoArray_arrayize(self)->clear();
}

bool PseudoArray_op_eq(const AnyPtr& self, const ArrayPtr& other){
	return PseudoArray_arrayize(self)->op_eq(other);
}

void PseudoArray_reverse(const AnyPtr& self){
	PseudoArray_arrayize(self)->reverse();
}

AnyPtr PseudoArray_reversed(const AnyPtr& self){
	return PseudoArray_arrayize(self)->reversed();
}

void PseudoArray_assign(const AnyPtr& self, const ArrayPtr& other){
	PseudoArray_arrayize(self)->assign(other);
}

AnyPtr PseudoArray_each(const AnyPtr& self){
	if(const AnyPtr& ary = PseudoArray_extarct_array(self)){
		return static_ptr_cast<Array>(ary)->each();
	}else{
		return self;
	}
}

}

void initialize_iterator(){
	{
		ClassPtr p = PseudoArray();
		p->method("arrayize", &PseudoArray_arrayize);

		p->method("size", &PseudoArray_length);
		p->method("length", &PseudoArray_length);
		p->method("resize", &PseudoArray_resize);
		p->method("empty", &PseudoArray_empty);
		p->method("op_at", &PseudoArray_op_at);
		p->method("op_set_at", &PseudoArray_op_set_at);
		p->method("slice", &PseudoArray_slice)->param("i", Named("n", 1));
		p->method("splice", &PseudoArray_splice)->param("i", Named("n", 1));
		p->method("pop_back", &PseudoArray_pop_back);
		p->method("push_back", &PseudoArray_push_back);
		p->method("pop_front", &PseudoArray_pop_front);
		p->method("push_front", &PseudoArray_push_front);
		p->method("erase", &PseudoArray_erase)->param("i", Named("n", 1));
		p->method("insert", &PseudoArray_insert);
		p->method("cat", &PseudoArray_cat);
		p->method("cat_assign", &PseudoArray_cat_assign);
		p->method("op_cat", &PseudoArray_cat);
		p->method("op_cat_assign", &PseudoArray_cat_assign);
		p->method("to_s", &PseudoArray_to_s);
		p->method("r_each", &PseudoArray_r_each);
		p->method("clone", &PseudoArray_clone);
		p->method("front", &PseudoArray_front);
		p->method("back", &PseudoArray_back);
		p->method("clear", &PseudoArray_clear);
		p->method("op_eq", &PseudoArray_op_eq);
		p->method("reverse", &PseudoArray_reverse);
		p->method("reversed", &PseudoArray_reversed);
		p->method("assign", &PseudoArray_assign);
		p->method("each", &PseudoArray_each);
	}
}

void DelegateToIterator::call(const VMachinePtr& vm){
	vm->get_arg_this()->send(Xid(each))->rawsend(vm, member_);
}

void DelegateToIteratorOrArray::call(const VMachinePtr& vm){
	if(const ArrayPtr& ary = PseudoArray_extarct_array(vm->get_arg_this())){
		ary->each()->rawsend(vm, member_);
	}else{
		Iterator()->member(member_)->call(vm);
	}
}

IterBreaker::~IterBreaker(){
	const VMachinePtr& vm = vmachine();
	vm->setup_call(0);
	target->rawsend(vm, Xid(block_break));
	if(!vm->processed()){
		vm->return_result();
	}
	vm->cleanup_call();
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
