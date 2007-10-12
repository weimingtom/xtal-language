#include "xtal.h"

#include "xtal_proxy.h"
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

void SendProxy::execute(){
	if(name){
		const VMachinePtr& vm = vmachine();
		vm->setup_call(1);
		obj->rawsend(vm, name, ns);
		name = null;
		obj = vm->result_and_cleanup_call();
	}
}

}
