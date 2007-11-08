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

}
