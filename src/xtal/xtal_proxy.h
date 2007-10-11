
#pragma once

namespace xtal{

class AtProxy{
	AnyPtr obj;
	AnyPtr key;
public:

	AtProxy(const AnyPtr& obj, const AnyPtr& key)
		:obj(obj), key(key){}

	const AtProxy& operator =(const AnyPtr& value);

	operator const AnyPtr&();

	const AnyPtr& operator ->();
};

class SendProxy{
	AnyPtr obj;
	InternedStringPtr name;
	AnyPtr ns;
public:

	SendProxy(const AnyPtr& obj, const InternedStringPtr& name, const AnyPtr& ns)
		:obj(obj), name(name), ns(ns){}

	//const AtProxy& operator =(const AnyPtr& value);

	operator const AnyPtr&();

	const AnyPtr& operator ->();

public:

	const AnyPtr& operator ()();

	template<class A0>
	const AnyPtr& operator ()(const A0& a0);

	template<class A0, class A1>
	const AnyPtr& operator ()(const A0& a0, const A1& a1);

	template<class A0, class A1, class A2>
	const AnyPtr& operator ()(const A0& a0, const A1& a1, const A2& a2);

	template<class A0, class A1, class A2, class A3>
	const AnyPtr& operator ()(const A0& a0, const A1& a1, const A2& a2, const A3& a3);

	template<class A0, class A1, class A2, class A3, class A4>
	const AnyPtr& operator ()(const A0& a0, const A1& a1, const A2& a2, const A3& a3, const A4& a4);
};

template<class A0>
const AnyPtr& SendProxy::operator ()(const A0& a0){
	const VMachinePtr& vm = vmachine();
	vm->setup_call(1, a0);
	obj->rawsend(vm, name, ns);
	return obj = vm->result_and_cleanup_call();
}

template<class A0, class A1>
const AnyPtr& SendProxy::operator ()(const A0& a0, const A1& a1){
	const VMachinePtr& vm = vmachine();
	vm->setup_call(1, a0, a1);
	obj->rawsend(vm, name, ns);
	return obj = vm->result_and_cleanup_call();
}

template<class A0, class A1, class A2>
const AnyPtr& SendProxy::operator ()(const A0& a0, const A1& a1, const A2& a2){
	const VMachinePtr& vm = vmachine();
	vm->setup_call(1, a0, a1, a2);
	obj->rawsend(vm, name, ns);
	return obj = vm->result_and_cleanup_call();
}

template<class A0, class A1, class A2, class A3>
const AnyPtr& SendProxy::operator ()(const A0& a0, const A1& a1, const A2& a2, const A3& a3){
	const VMachinePtr& vm = vmachine();
	vm->setup_call(1, a0, a1, a2, a3);
	obj->rawsend(vm, name, ns);
	return obj = vm->result_and_cleanup_call();
}


template<class A0, class A1, class A2, class A3, class A4>
const AnyPtr& SendProxy::operator ()(const A0& a0, const A1& a1, const A2& a2, const A3& a3, const A4& a4){
	const VMachinePtr& vm = vmachine();
	vm->setup_call(1, a0, a1, a2, a3, a4);
	obj->rawsend(vm, name, ns);
	return obj = vm->result_and_cleanup_call();
}

}
