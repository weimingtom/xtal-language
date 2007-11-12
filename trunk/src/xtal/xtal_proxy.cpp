#include "xtal.h"
#include "xtal_macro.h"

namespace xtal{

const AtProxy& AtProxy::operator =(const AnyPtr& value){
	obj->send(Xid(set_at), key->get_class())(key, value);
	return *this;
}

AtProxy Innocence::operator[](const AnyPtr& key) const{
	return AtProxy(ap(*this), key);
}

AtProxy::operator const AnyPtr&(){
	return obj = obj->send(Xid(op_at), key->get_class())(key);
}

const AnyPtr& AtProxy::operator ->(){
	return obj = obj->send(Xid(op_at), key->get_class())(key);
}

SendProxy::SendProxy(const AnyPtr& obj, const IDPtr& primary_key, const AnyPtr& secondary_key)
	:obj(obj), primary_key(primary_key), secondary_key(secondary_key){}

SendProxy::~SendProxy(){
	execute();
}

void SendProxy::execute(){
	if(primary_key){
		const VMachinePtr& vm = vmachine();
		vm->setup_call(1);
		obj->rawsend(vm, primary_key, secondary_key);
		primary_key = null;
		obj = vm->result_and_cleanup_call();
	}
}

const AnyPtr& SendProxy::operator ()(const Param& a0){
	if(!primary_key) return obj;
	const VMachinePtr& vm = vmachine();
	vm->setup_call(1, a0);
	obj->rawsend(vm, primary_key, secondary_key);
	primary_key = null;
	return obj = vm->result_and_cleanup_call();
}

const AnyPtr& SendProxy::operator ()(const Param& a0, const Param& a1){
	if(!primary_key) return obj;
	const VMachinePtr& vm = vmachine();
	vm->setup_call(1, a0, a1);
	obj->rawsend(vm, primary_key, secondary_key);
	primary_key = null;
	return obj = vm->result_and_cleanup_call();
}

const AnyPtr& SendProxy::operator ()(const Param& a0, const Param& a1, const Param& a2){
	if(!primary_key) return obj;
	const VMachinePtr& vm = vmachine();
	vm->setup_call(1, a0, a1, a2);
	obj->rawsend(vm, primary_key, secondary_key);
	primary_key = null;
	return obj = vm->result_and_cleanup_call();
}

const AnyPtr& SendProxy::operator ()(const Param& a0, const Param& a1, const Param& a2, const Param& a3){
	if(!primary_key) return obj;
	const VMachinePtr& vm = vmachine();
	vm->setup_call(1, a0, a1, a2, a3);
	obj->rawsend(vm, primary_key, secondary_key);
	primary_key = null;
	return obj = vm->result_and_cleanup_call();
}

const AnyPtr& SendProxy::operator ()(const Param& a0, const Param& a1, const Param& a2, const Param& a3, const Param& a4){
	if(!primary_key) return obj;
	const VMachinePtr& vm = vmachine();
	vm->setup_call(1, a0, a1, a2, a3, a4);
	obj->rawsend(vm, primary_key, secondary_key);
	primary_key = null;
	return obj = vm->result_and_cleanup_call();
}

AnyPtr Innocence::operator()() const{
	const VMachinePtr& vm = vmachine();
	vm->setup_call(1);
	ap(*this)->call(vm);
	return vm->result_and_cleanup_call();
}

AnyPtr Innocence::operator()(const Param& a0) const{
	const VMachinePtr& vm = vmachine();
	vm->setup_call(1, a0);
	ap(*this)->call(vm);
	return vm->result_and_cleanup_call();
}

AnyPtr Innocence::operator()(const Param& a0, const Param& a1) const{
	const VMachinePtr& vm = vmachine();
	vm->setup_call(1, a0, a1);
	ap(*this)->call(vm);
	return vm->result_and_cleanup_call();
}

AnyPtr Innocence::operator()(const Param& a0, const Param& a1, const Param& a2) const{
	const VMachinePtr& vm = vmachine();
	vm->setup_call(1, a0, a1, a2);
	ap(*this)->call(vm);
	return vm->result_and_cleanup_call();
}

AnyPtr Innocence::operator()(const Param& a0, const Param& a1, const Param& a2, const Param& a3) const{
	const VMachinePtr& vm = vmachine();
	vm->setup_call(1, a0, a1, a2, a3);
	ap(*this)->call(vm);
	return vm->result_and_cleanup_call();
}

AnyPtr Innocence::operator()(const Param& a0, const Param& a1, const Param& a2, const Param& a3, const Param& a4) const{
	const VMachinePtr& vm = vmachine();
	vm->setup_call(1, a0, a1, a2, a3, a4);
	ap(*this)->call(vm);
	return vm->result_and_cleanup_call();
}

}
