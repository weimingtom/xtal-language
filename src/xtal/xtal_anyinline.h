
#pragma once

namespace xtal{

inline const VMachinePtr& ThreadMgr::vmachine(){
	return vmachine_;
}

inline const ClassPtr& Base::get_class(){ 
	return *static_cast<ClassPtr*>(static_cast<Any*>(class_)); 
}

//{REPEAT{{
/*
/// @brief primary_keyメソッドを呼び出す
template<class A0 #COMMA_REPEAT#class A`i+1`#>
inline AnyPtr Any::send(const IDPtr& primary_key, const A0& a0 #COMMA_REPEAT#const A`i+1`& a`i+1`#) const{
	const VMachinePtr& vm = vmachine();
	vm->setup_call(1, a0 #COMMA_REPEAT#a`i+1`#);
	rawsend(vm, primary_key);
	return vm->result_and_cleanup_call();
}

/// @brief primary_key#secondary_keyメソッドを呼び出す
template<class A0 #COMMA_REPEAT#class A`i+1`#>
inline AnyPtr Any::send2(const IDPtr& primary_key, const AnyPtr& secondary_key, const A0& a0 #COMMA_REPEAT#const A`i+1`& a`i+1`#) const{
	const VMachinePtr& vm = vmachine();
	vm->setup_call(1, a0 #COMMA_REPEAT#a`i+1`#);
	rawsend(vm, primary_key, secondary_key);
	return vm->result_and_cleanup_call();
}

/// @brief 関数を呼び出す
template<class A0 #COMMA_REPEAT#class A`i+1`#>
inline AnyPtr Any::call(const A0& a0 #COMMA_REPEAT#const A`i+1`& a`i+1`#) const{
	const VMachinePtr& vm = vmachine();
	vm->setup_call(1, a0 #COMMA_REPEAT#a`i+1`#);
	rawcall(vm);
	return vm->result_and_cleanup_call();
}
*/

/// @brief primary_keyメソッドを呼び出す
template<class A0 >
inline AnyPtr Any::send(const IDPtr& primary_key, const A0& a0 ) const{
	const VMachinePtr& vm = vmachine();
	vm->setup_call(1, a0 );
	rawsend(vm, primary_key);
	return vm->result_and_cleanup_call();
}

/// @brief primary_key#secondary_keyメソッドを呼び出す
template<class A0 >
inline AnyPtr Any::send2(const IDPtr& primary_key, const AnyPtr& secondary_key, const A0& a0 ) const{
	const VMachinePtr& vm = vmachine();
	vm->setup_call(1, a0 );
	rawsend(vm, primary_key, secondary_key);
	return vm->result_and_cleanup_call();
}

/// @brief 関数を呼び出す
template<class A0 >
inline AnyPtr Any::call(const A0& a0 ) const{
	const VMachinePtr& vm = vmachine();
	vm->setup_call(1, a0 );
	rawcall(vm);
	return vm->result_and_cleanup_call();
}

/// @brief primary_keyメソッドを呼び出す
template<class A0 , class A1>
inline AnyPtr Any::send(const IDPtr& primary_key, const A0& a0 , const A1& a1) const{
	const VMachinePtr& vm = vmachine();
	vm->setup_call(1, a0 , a1);
	rawsend(vm, primary_key);
	return vm->result_and_cleanup_call();
}

/// @brief primary_key#secondary_keyメソッドを呼び出す
template<class A0 , class A1>
inline AnyPtr Any::send2(const IDPtr& primary_key, const AnyPtr& secondary_key, const A0& a0 , const A1& a1) const{
	const VMachinePtr& vm = vmachine();
	vm->setup_call(1, a0 , a1);
	rawsend(vm, primary_key, secondary_key);
	return vm->result_and_cleanup_call();
}

/// @brief 関数を呼び出す
template<class A0 , class A1>
inline AnyPtr Any::call(const A0& a0 , const A1& a1) const{
	const VMachinePtr& vm = vmachine();
	vm->setup_call(1, a0 , a1);
	rawcall(vm);
	return vm->result_and_cleanup_call();
}

/// @brief primary_keyメソッドを呼び出す
template<class A0 , class A1, class A2>
inline AnyPtr Any::send(const IDPtr& primary_key, const A0& a0 , const A1& a1, const A2& a2) const{
	const VMachinePtr& vm = vmachine();
	vm->setup_call(1, a0 , a1, a2);
	rawsend(vm, primary_key);
	return vm->result_and_cleanup_call();
}

/// @brief primary_key#secondary_keyメソッドを呼び出す
template<class A0 , class A1, class A2>
inline AnyPtr Any::send2(const IDPtr& primary_key, const AnyPtr& secondary_key, const A0& a0 , const A1& a1, const A2& a2) const{
	const VMachinePtr& vm = vmachine();
	vm->setup_call(1, a0 , a1, a2);
	rawsend(vm, primary_key, secondary_key);
	return vm->result_and_cleanup_call();
}

/// @brief 関数を呼び出す
template<class A0 , class A1, class A2>
inline AnyPtr Any::call(const A0& a0 , const A1& a1, const A2& a2) const{
	const VMachinePtr& vm = vmachine();
	vm->setup_call(1, a0 , a1, a2);
	rawcall(vm);
	return vm->result_and_cleanup_call();
}

/// @brief primary_keyメソッドを呼び出す
template<class A0 , class A1, class A2, class A3>
inline AnyPtr Any::send(const IDPtr& primary_key, const A0& a0 , const A1& a1, const A2& a2, const A3& a3) const{
	const VMachinePtr& vm = vmachine();
	vm->setup_call(1, a0 , a1, a2, a3);
	rawsend(vm, primary_key);
	return vm->result_and_cleanup_call();
}

/// @brief primary_key#secondary_keyメソッドを呼び出す
template<class A0 , class A1, class A2, class A3>
inline AnyPtr Any::send2(const IDPtr& primary_key, const AnyPtr& secondary_key, const A0& a0 , const A1& a1, const A2& a2, const A3& a3) const{
	const VMachinePtr& vm = vmachine();
	vm->setup_call(1, a0 , a1, a2, a3);
	rawsend(vm, primary_key, secondary_key);
	return vm->result_and_cleanup_call();
}

/// @brief 関数を呼び出す
template<class A0 , class A1, class A2, class A3>
inline AnyPtr Any::call(const A0& a0 , const A1& a1, const A2& a2, const A3& a3) const{
	const VMachinePtr& vm = vmachine();
	vm->setup_call(1, a0 , a1, a2, a3);
	rawcall(vm);
	return vm->result_and_cleanup_call();
}

/// @brief primary_keyメソッドを呼び出す
template<class A0 , class A1, class A2, class A3, class A4>
inline AnyPtr Any::send(const IDPtr& primary_key, const A0& a0 , const A1& a1, const A2& a2, const A3& a3, const A4& a4) const{
	const VMachinePtr& vm = vmachine();
	vm->setup_call(1, a0 , a1, a2, a3, a4);
	rawsend(vm, primary_key);
	return vm->result_and_cleanup_call();
}

/// @brief primary_key#secondary_keyメソッドを呼び出す
template<class A0 , class A1, class A2, class A3, class A4>
inline AnyPtr Any::send2(const IDPtr& primary_key, const AnyPtr& secondary_key, const A0& a0 , const A1& a1, const A2& a2, const A3& a3, const A4& a4) const{
	const VMachinePtr& vm = vmachine();
	vm->setup_call(1, a0 , a1, a2, a3, a4);
	rawsend(vm, primary_key, secondary_key);
	return vm->result_and_cleanup_call();
}

/// @brief 関数を呼び出す
template<class A0 , class A1, class A2, class A3, class A4>
inline AnyPtr Any::call(const A0& a0 , const A1& a1, const A2& a2, const A3& a3, const A4& a4) const{
	const VMachinePtr& vm = vmachine();
	vm->setup_call(1, a0 , a1, a2, a3, a4);
	rawcall(vm);
	return vm->result_and_cleanup_call();
}

/// @brief primary_keyメソッドを呼び出す
template<class A0 , class A1, class A2, class A3, class A4, class A5>
inline AnyPtr Any::send(const IDPtr& primary_key, const A0& a0 , const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5) const{
	const VMachinePtr& vm = vmachine();
	vm->setup_call(1, a0 , a1, a2, a3, a4, a5);
	rawsend(vm, primary_key);
	return vm->result_and_cleanup_call();
}

/// @brief primary_key#secondary_keyメソッドを呼び出す
template<class A0 , class A1, class A2, class A3, class A4, class A5>
inline AnyPtr Any::send2(const IDPtr& primary_key, const AnyPtr& secondary_key, const A0& a0 , const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5) const{
	const VMachinePtr& vm = vmachine();
	vm->setup_call(1, a0 , a1, a2, a3, a4, a5);
	rawsend(vm, primary_key, secondary_key);
	return vm->result_and_cleanup_call();
}

/// @brief 関数を呼び出す
template<class A0 , class A1, class A2, class A3, class A4, class A5>
inline AnyPtr Any::call(const A0& a0 , const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5) const{
	const VMachinePtr& vm = vmachine();
	vm->setup_call(1, a0 , a1, a2, a3, a4, a5);
	rawcall(vm);
	return vm->result_and_cleanup_call();
}

//}}REPEAT}

}
