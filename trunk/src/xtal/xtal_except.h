/** \file src/xtal/xtal_except.h
* \brief src/xtal/xtal_except.h
*/

#ifndef XTAL_EXCEPT_H_INCLUDE_GUARD
#define XTAL_EXCEPT_H_INCLUDE_GUARD

#pragma once

namespace xtal{

/**
* \xbind lib::builtin
* \brief 例外
*/
class Exception : public Base{
public:

	Exception(const AnyPtr& message = empty_string);

	void initialize(const AnyPtr& message = empty_string);

	void append_backtrace(const AnyPtr& file, int_t line, const AnyPtr& function_name = empty_string);

	/**
	* \brief 文字列化する
	*/
	StringPtr to_s();

	/**
	* \brief メッセージを返す
	*/
	StringPtr message(){
		return message_;
	}

	/**
	* \brief バックトレースの情報を要素とするIteratorを返す
	*/
	AnyPtr backtrace(){
		return backtrace_->each();
	}

private:

	virtual void visit_members(Visitor& m){
		Base::visit_members(m);
		m & message_ & backtrace_;
	}

	StringPtr message_;
	ArrayPtr backtrace_;
};

class StandardError;
class RuntimeError;
class IOError;
class LogicError;
class CastError;
class ArgumentError;
class YieldError;
class InstanceVariableError;
class UnsupportedError;
class RedefinedError;
class AccessibilityError;
class AssertionFailed;
class CompileError;

AnyPtr cast_error(const AnyPtr& from, const AnyPtr& to);
AnyPtr argument_error(const AnyPtr& object, int_t no, const ClassPtr& required, const ClassPtr& type);
AnyPtr unsupported_error(const AnyPtr& target, const IDPtr& primary_key, const AnyPtr& secondary_key);

/**
* \brief 例外を設定する
* \hideinitializer
*/
#define XTAL_SET_EXCEPT(e) ::xtal::vmachine()->set_except(e)

/**
* \brief 例外が送出されているなら取り出す
* \hideinitializer
*/
#define XTAL_CATCH_EXCEPT(e) if(const ::xtal::AnyPtr& e = ::xtal::unchecked_ptr_cast<::xtal::Exception>(::xtal::vmachine()->catch_except()))

/**
* \brief 例外が送出されているか調べる
* \hideinitializer
*/
#define XTAL_CHECK_EXCEPT(e) if(const ::xtal::AnyPtr& e = ::xtal::unchecked_ptr_cast<::xtal::Exception>(::xtal::vmachine()->except()))

}

#endif // XTAL_EXCEPT_H_INCLUDE_GUARD
