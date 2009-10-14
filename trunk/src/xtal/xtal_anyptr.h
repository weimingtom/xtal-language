/** \file src/xtal/xtal_anyptr.h
* \brief src/xtal/xtal_anyptr.h
*/

#ifndef XTAL_ANYPTR_H_INCLUDE_GUARD
#define XTAL_ANYPTR_H_INCLUDE_GUARD

#pragma once

namespace xtal{

template<uint_t Size>
struct UserTypeBuffer{
	union{
		u8 buf[Size];
		int_t dummy;
	};

	template<class T>
	void operator()(T* p){
		p->~T();
	}
};

template<class T>
struct UserTypeHolder : public Base{
	UserTypeHolder(){}
	UserTypeHolder(T* p):ptr(p){}
	T* ptr;
};

template<class T, class Deleter = UserTypeBuffer<sizeof(T)> >
struct UserTypeHolderSub : public UserTypeHolder<T>, public Deleter{
	UserTypeHolderSub(){}
	UserTypeHolderSub(T* p, const Deleter& f):UserTypeHolder<T>(p), Deleter(f){}
	virtual ~UserTypeHolderSub(){ Deleter::operator()(this->ptr); }
};

struct undeleter_t{
	template<class T>
	void operator()(T* p){}
};

struct deleter_t{
	template<class T>
	void operator()(T* p){
		delete p;
	}
};

extern undeleter_t undeleter;
extern deleter_t deleter;

enum InheritedEnum{
	INHERITED_BASE,
	INHERITED_RCBASE,
	INHERITED_ANY,
	INHERITED_ANYPTR,
	INHERITED_OTHER
};

template<class T>
struct InheritedN{
	enum{
		value = 
			IsInherited<T, Base>::value ? INHERITED_BASE : 
			IsInherited<T, RefCountingBase>::value ? INHERITED_RCBASE : 
			IsInherited<T, AnyPtr>::value ? INHERITED_ANYPTR :
			IsInherited<T, Any>::value ? INHERITED_ANY : INHERITED_OTHER
	};
};

template<class T>
struct SmartPtrCtor1{
	struct type{};
	static int_t call(type){ return 0; };
};

template<class T>
struct SmartPtrCtor2{
	struct type{};
	static int_t call(type){ return 0; };
};

template<class T>
struct SmartPtrCtor3{
	struct type{};
	static int_t call(type){ return 0; };
};

template<class T>
struct SmartPtrCtor4{
	struct type{};
	static int_t call(type){ return 0; };
};


template <int N>
struct SmartPtrSelector{};

template<class T>
inline const ClassPtr& cpp_class();

/**
* \brief ���̌^�̃I�u�W�F�N�g�ł��ێ�������ꉻ���ꂽ�X�}�[�g�|�C���^
*/
template<>
class SmartPtr<Any> : public Any{
public:
	
	/**
	* \brief null�ō\�z����R���X�g���N�^
	*/
	SmartPtr(){}

	/**
	* \brief T�̃|�C���^�ƁA�����j�����邽�߂̊֐��I�u�W�F�N�g���󂯎���č\�z����R���X�g���N�^
	*/
	template<class T, class Deleter>
	SmartPtr(T* tp, const Deleter& deleter)
		:Any(noinit_t()){
		UserTypeHolderSub<T, Deleter>* p = new UserTypeHolderSub<T, Deleter>(tp, deleter);
		set_pvalue(*this, p);
		p->set_class(cpp_class<T>());
		register_gc(p);
	}

	SmartPtr(const SmartPtr<Any>& p);

	/// null���󂯎��R���X�g���N�^
	SmartPtr(const NullPtr&){}

	SmartPtr<Any>& operator =(const SmartPtr<Any>& p);

	explicit SmartPtr(PrimitiveType type)
		:Any(type){}
	
	~SmartPtr();

public:

	void assign_direct(const SmartPtr<Any>& a);

protected:

	struct special_ctor_t{};

	SmartPtr(const Any& a, special_ctor_t)
		:Any(a){}

	SmartPtr(RefCountingBase* p, int_t type, special_ctor_t);

	SmartPtr(Base* p, const ClassPtr& c, special_ctor_t);

public:

	/**
	* \brief boolean����\�z����R���X�g���N�^�B
	*
	*/
	SmartPtr(bool v):Any(v){}

	/**
	* \brief �����񂩂�\�z����R���X�g���N�^�B
	*
	*/
	SmartPtr(const char_t* str);

	/**
	* \brief �����񂩂�\�z����R���X�g���N�^�B
	*
	*/
	SmartPtr(const char8_t* str);

	/**
	* \brief �����񂩂�\�z����R���X�g���N�^�B
	*
	*/
	SmartPtr(const StringLiteral& str);

	// ��{�^�̐����A���������_������\�z����R���X�g���N�^
	SmartPtr(char v):Any(v){}
	SmartPtr(signed char v):Any(v){}
	SmartPtr(unsigned char v):Any(v){}
	SmartPtr(short v):Any(v){}
	SmartPtr(unsigned short v):Any(v){}
	SmartPtr(int v):Any(v){}
	SmartPtr(unsigned int v):Any(v){}
	SmartPtr(long v):Any(v){}
	SmartPtr(unsigned long v):Any(v){}

	SmartPtr(float v):Any(v){}
	SmartPtr(double v):Any(v){}
	SmartPtr(long double v):Any(v){}

public:

	/**
	* \brief Any�^�ւ̃|�C���^���擾����B
	*/
	Any* get() const{ return (Any*)this; }

	/**
	* \brief ->���Z�q
	* �X�}�[�g�|�C���^�Ƃ��Ĉ������߂ɃI�[�o�[���[�h����B
	*/
	Any* operator ->() const{ return get(); }
	
	/**
	* \brief *���Z�q
	* �X�}�[�g�|�C���^�Ƃ��Ĉ������߂ɃI�[�o�[���[�h����B
	*/
	Any& operator *() const{ return *get(); }
	
public:

#ifdef XTAL_DEBUG

	struct dummy_bool_tag{ void safe_true(){} };
	typedef void (dummy_bool_tag::*safe_bool)();

	operator safe_bool() const{
		return type(*this)>TYPE_FALSE ? &dummy_bool_tag::safe_true : (safe_bool)0;
	}

#else

	/**
	* \brief boolean�ւ̎����ϊ�
	*/
	operator bool() const{
		return type(*this)>TYPE_FALSE;
	}

#endif

	/**
	* \biref !���Z�q
	*/
	bool operator !() const{
		return type(*this)<=TYPE_FALSE;
	}

private:

	/**
	* \brief �Öق̕ϊ���}���邽�߂̃R���X�g���N�^�B
	* ���̂̒m��Ȃ��|�C���^����̍\�z�����ۂ��邽�߁A���̃R���X�g���N�^��private�Ŏ��������݂��Ȃ��B
	*/
	SmartPtr(void*);

};

void visit_members(Visitor& m, const Any& p);

class Visitor{
	int_t value_;
public:	

	Visitor(int_t value){
		value_ = value;
	}

	int_t value(){
		return value_;
	}

	template<class T> Visitor& operator &(T& value){
		visit_members(*this, value);
		return *this;
	}

	template<class T> Visitor& operator ()(T& value){
		visit_members(*this, value);
		return *this;
	}
};

inline const AnyPtr& ap(const Any& v){
	return (const AnyPtr&)v;
}

template<class F, class S>
void visit_members(Visitor& m, const std::pair<F, S>& value){
	m & value.first & value.second;
}

}//namespace 

#endif // XTAL_ANYPTR_H_INCLUDE_GUARD