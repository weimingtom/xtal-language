#include "xtal.h"

#include "xtal_proxy.h"
#include "xtal_macro.h"

namespace xtal{

const AtProxy& AtProxy::operator =(const AnyPtr& value){
	obj->send(Xid(set_at))(key, value);
	return *this;
}

AtProxy Innocence::operator[](const AnyPtr& key) const{
	return AtProxy(ap(*this), key);
}

AtProxy::operator const AnyPtr&(){
	return obj = obj->send(Xid(at))(key);
}

const AnyPtr& AtProxy::operator ->(){
	return obj = obj->send(Xid(at))(key);
}


SendProxy::operator const AnyPtr&(){
	return operator()();
}

const AnyPtr& SendProxy::operator ->(){
	return operator()();
}

const AnyPtr& SendProxy::operator()(){
	const VMachinePtr& vm = vmachine();
	vm->setup_call(1);
	obj->rawsend(vm, name, ns);
	return obj = vm->result_and_cleanup_call();
}

}
