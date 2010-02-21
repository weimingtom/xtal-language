/** \file src/xtal/xtal_class.h
* \brief src/xtal/xtal_class.h
*/

#ifndef XTAL_CLASS_H_INCLUDE_GUARD
#define XTAL_CLASS_H_INCLUDE_GUARD

#pragma once

namespace xtal{

struct param_types_holder_n;

class InstanceVariables{
public:

	struct uninit_t{};

	InstanceVariables(uninit_t){}

	InstanceVariables();
			
	~InstanceVariables();
		
	void init_variables(ClassInfo* class_info);

	uint_t find_class_info(ClassInfo* class_info, uint_t index){
		VariablesInfo& info = variables_info_.back();
		if(info.class_info == class_info){
			return info.pos + index;
		}
		return find_class_info_inner(class_info, index);
	}

	const AnyPtr& variable(uint_t index, ClassInfo* class_info){
		return variables_.at(find_class_info(class_info, index));
	}

	void set_variable(uint_t index, ClassInfo* class_info, const AnyPtr& value){
		variables_.set_at(find_class_info(class_info, index), value);
	}

	bool is_included(ClassInfo* class_info);

	uint_t find_class_info_inner(ClassInfo* class_info, uint_t index);

	void replace(ClassInfo* from, ClassInfo* to);

	bool empty(){
		return variables_.empty();
	}

	void clear(){
		variables_info_.clear();
		variables_.clear();
	}

	void visit_members(Visitor& m){
		for(uint_t i=0; i<variables_.size(); ++i){
			m & (AnyPtr&)variables_.at(i);
		}
	}

protected:
	
	struct VariablesInfo{
		ClassInfo* class_info;
		int_t pos;
	};

	PODArrayList<VariablesInfo> variables_info_;
	xarray variables_;
};

class EmptyInstanceVariables : public InstanceVariables{
public:

	EmptyInstanceVariables();

	~EmptyInstanceVariables();

	static VariablesInfo vi;
};

/**
* \xbind lib::builtin
* \xinherit lib::builtin::Frame
* \brief �N���X
*/
class Class : public Frame{
public:

	Class();

	Class(const IDPtr& name);

	Class(const FramePtr& outer, const CodePtr& code, ClassInfo* info);

	~Class();

	void overwrite(const ClassPtr& p);

	void overwrite_member(const IDPtr& primary_key, const AnyPtr& value, const AnyPtr& secondary_key = undefined, int_t accessibility = KIND_PUBLIC);

public:

	/**
	* \brief �V���������o���`����
	* \param primary_key �V������`���郁���o�̖��O
	* \param value �ݒ肷��l
	* \param secondary_key �Z�J���_���L�[
	* \param accessibility �I��
	*/
	void on_def(const IDPtr& primary_key, const AnyPtr& value, const AnyPtr& secondary_key, int_t accessibility = KIND_PUBLIC);

	using RefCountingBase::def;
	
	void def(const char_t* primary_key, const AnyPtr& value, const AnyPtr& secondary_key, int_t accessibility = KIND_PUBLIC);
	void def(const char8_t* primary_key, const AnyPtr& value, const AnyPtr& secondary_key, int_t accessibility = KIND_PUBLIC);

	void def(const IDPtr& primary_key, const AnyPtr& value);
	void def(const char_t* primary_key, const AnyPtr& value);
	void def(const char8_t* primary_key, const AnyPtr& value);

	/**
	* \internal
	* \brief �����o�����o��
	*
	* ���̊֐����g���̂ł͂Ȃ��AAny::member���g�����ƁB
	*/
	const AnyPtr& on_rawmember(const IDPtr& primary_key, const AnyPtr& secondary_key, bool inherited_too, int_t& accessibility, bool& nocache);

	/**
	* \brief �����o���Đݒ肷��
	* Xtal���x���ł͋֎~����Ă��鑀�삾���AC++���x���ł͉\�ɂ��Ă���
	* \param primary_key �V������`���郁���o�̖��O
	* \param value �ݒ肷��l
	* \param secondary_key �Z�J���_���L�[
	*/
	bool set_member(const IDPtr& primary_key, const AnyPtr& value, const AnyPtr& secondary_key);

	/**
	* \xbind
	* \brief �p��
	* \param cls �p������N���X�I�u�W�F�N�g
	*/
	void inherit(const ClassPtr& cls);

	/**
	* \brief �p������
	* Xtal���x���ŁA�N���X��`���Ɍp������ꍇ�ɌĂ΂��
	* \param cls �p������N���X�I�u�W�F�N�g
	*/
	void inherit_first(const ClassPtr& cls);

	/**
	* \brief �p������
	* \param cls �p������N���X�I�u�W�F�N�g
	*/
	void inherit_strict(const ClassPtr& cls);

	/**
	* \brief �p������Ă��邩���ׂ�
	* \param cls �p������Ă��钲�ׂ����N���X�I�u�W�F�N�g
	*/
	bool is_inherited(const AnyPtr& cls);

	/**
	* \brief C++�̃N���X���p������Ă��邩���ׂ�
	*/
	bool is_inherited_cpp_class();

	/**
	* \xbind
	* \brief �p������Ă���N���X��񋓂���Iterator��Ԃ�
	*/
	AnyPtr inherited_classes();

	/**
	* \brief �߂����O�̃����o����������
	*/
	IDPtr find_near_member(const IDPtr& primary_key, const AnyPtr& secondary_key = undefined){
		int_t dist = 0xffffff;
		return find_near_member2(primary_key, secondary_key, dist);
	}

	IDPtr find_near_member2(const IDPtr& primary_key, const AnyPtr& secondary_key, int_t& dist);
	
	/**
	* \xbind
	* \brief �c��N���X��񋓂���Iterator��Ԃ�
	*/
	AnyPtr ancestors();

public:

	/**
	* \brief �֐����`����
	* cls->def_fun(Xid(name), &foo); �� cls->def(Xid(name), xtal::fun(&foo)); �Ɠ���
	* \param primary_key �V������`���郁���o�̖��O
	* \param f �ݒ肷��֐�
	* \param secondary_key �Z�J���_���L�[
	* \param accessibility �I��
	*/
	template<class TFun>
	const NativeFunPtr& def_fun(const IDPtr& primary_key, const TFun& f, const AnyPtr& secondary_key = undefined, int_t accessibility = KIND_PUBLIC){
		return def_and_return(primary_key, secondary_key, accessibility, fun_param_holder<dfun<TFun> >::value, &f);
	}

	/**
	* \brief ���\�b�h���`����
	* cls->def_method(Xid(name), &Klass::foo); �� cls->def(Xid(name), xtal::method(&Klass::foo)); �Ɠ���
	* \param primary_key �V������`���郁���o�̖��O
	* \param f �ݒ肷��֐�
	* \param secondary_key �Z�J���_���L�[
	* \param accessibility �I��
	*/
	template<class TFun>
	const NativeFunPtr& def_method(const IDPtr& primary_key, const TFun& f, const AnyPtr& secondary_key = undefined, int_t accessibility = KIND_PUBLIC){
		return def_and_return(primary_key, secondary_key, accessibility, fun_param_holder<dmemfun<TFun> >::value, &f);
	}

	/**
	* \brief �����o�ϐ��ւ̃|�C���^����Q�b�^�[�𐶐����A��`����
	* \param primary_key �V������`���郁���o�̖��O
	* \param value �ݒ肷��l
	* \param secondary_key �Z�J���_���L�[
	* \param accessibility �I��
	*/
	template<class T, class C>
	const NativeFunPtr& def_getter(const IDPtr& primary_key, T C::* v, const AnyPtr& secondary_key = undefined, int_t accessibility = KIND_PUBLIC){
		return def_and_return(primary_key, secondary_key, accessibility, fun_param_holder<dmemfun<getter_functor<C, T> > >::value, &v);
	}
	
	/**
	* \brief �����o�ϐ��ւ̃|�C���^����Z�b�^�[�𐶐����A��`����
	* Xtal�ł́Aobj.name = 10; �Ƃ���ɂ�set_name��set_��O�u�������\�b�h���`����K�v�����邽�߁A
	* �P���ȃZ�b�^�[���`�������ꍇ�Aset_xxx�Ƃ��邱�Ƃ�Y��Ȃ����ƁB
	* \param primary_key �V������`���郁���o�̖��O
	* \param v �ݒ肷�郁���o�ϐ��ւ̃|�C���^
	* \param secondary_key �Z�J���_���L�[
	* \param accessibility �I��
	*/
	template<class T, class C>
	const NativeFunPtr& def_setter(const IDPtr& primary_key, T C::* v, const AnyPtr& secondary_key = undefined, int_t accessibility = KIND_PUBLIC){
		return def_and_return(primary_key, secondary_key, accessibility, fun_param_holder<dmemfun<setter_functor<C, T> > >::value, &v);
	}
	
	/**
	* \brief �����o�ϐ��ւ̃|�C���^����Q�b�^�[�A�Z�b�^�[�𗼕��������A��`����
	* cls->def_getter(primary_key, v, policy);
	* cls->def_setter(StringPtr("set_")->cat(primary_key), v, policy);
	* �Ɠ�����	
	* \param primary_key �V������`���郁���o�̖��O
	* \param v �ݒ肷�郁���o�ϐ��ւ̃|�C���^
	* \param secondary_key �Z�J���_���L�[
	* \param accessibility �I��
	*/	
	template<class T, class U>
	void def_var(const IDPtr& primary_key, T U::* v, const AnyPtr& secondary_key = undefined, int_t accessibility = KIND_PUBLIC){
		def_getter(primary_key, v, secondary_key, accessibility);
		def_setter(StringPtr("set_")->cat(primary_key), v, secondary_key, accessibility);
	}

	/**
	* \brief 2�d�f�B�X�p�b�`���\�b�h���`����B
	* \param primary_key �V������`���郁���o�̖��O
	* \param accessibility �I��
	*/
	void def_double_dispatch_method(const IDPtr& primary_key, int_t accessibility = KIND_PUBLIC);

	/**
	* \brief 2�d�f�B�X�p�b�`�֐����`����B
	* \param primary_key �V������`���郁���o�̖��O
	* \param accessibility �I��
	*/
	void def_double_dispatch_fun(const IDPtr& primary_key, int_t accessibility = KIND_PUBLIC);

public:

	/**
	* \brief �R���X�g���N�^�֐���o�^����
	*/
	const NativeFunPtr& def_ctor(const NativeFunPtr& ctor_func);

	/**
	* \brief �o�^���ꂽ�R���X�g���N�^�֐���Ԃ�
	*/
	const NativeFunPtr& ctor();

	/**
	* \brief �V���A���C�Y�Ɏg����R���X�g���N�^�֐���o�^����
	*/
	const NativeFunPtr& def_serial_ctor(const NativeFunPtr& ctor_func);

	/**
	* \brief �o�^���ꂽ�V���A���C�Y�Ɏg����R���X�g���N�^�֐���Ԃ�
	*/	
	const NativeFunPtr& serial_ctor();

//{REPEAT{{
/*
	/// `n`�����̃R���X�g���N�^�֐���o�^����
	template<class T #COMMA_REPEAT#class A`i`#>
	const NativeFunPtr& def_ctor`n`(){
		return def_ctor(xtal::ctor<T #COMMA_REPEAT#A`i`#>());
	}
*/

	/// 0�����̃R���X�g���N�^�֐���o�^����
	template<class T >
	const NativeFunPtr& def_ctor0(){
		return def_ctor(xtal::ctor<T >());
	}

	/// 1�����̃R���X�g���N�^�֐���o�^����
	template<class T , class A0>
	const NativeFunPtr& def_ctor1(){
		return def_ctor(xtal::ctor<T , A0>());
	}

	/// 2�����̃R���X�g���N�^�֐���o�^����
	template<class T , class A0, class A1>
	const NativeFunPtr& def_ctor2(){
		return def_ctor(xtal::ctor<T , A0, A1>());
	}

	/// 3�����̃R���X�g���N�^�֐���o�^����
	template<class T , class A0, class A1, class A2>
	const NativeFunPtr& def_ctor3(){
		return def_ctor(xtal::ctor<T , A0, A1, A2>());
	}

	/// 4�����̃R���X�g���N�^�֐���o�^����
	template<class T , class A0, class A1, class A2, class A3>
	const NativeFunPtr& def_ctor4(){
		return def_ctor(xtal::ctor<T , A0, A1, A2, A3>());
	}

	/// 5�����̃R���X�g���N�^�֐���o�^����
	template<class T , class A0, class A1, class A2, class A3, class A4>
	const NativeFunPtr& def_ctor5(){
		return def_ctor(xtal::ctor<T , A0, A1, A2, A3, A4>());
	}

	/// 6�����̃R���X�g���N�^�֐���o�^����
	template<class T , class A0, class A1, class A2, class A3, class A4, class A5>
	const NativeFunPtr& def_ctor6(){
		return def_ctor(xtal::ctor<T , A0, A1, A2, A3, A4, A5>());
	}

	/// 7�����̃R���X�g���N�^�֐���o�^����
	template<class T , class A0, class A1, class A2, class A3, class A4, class A5, class A6>
	const NativeFunPtr& def_ctor7(){
		return def_ctor(xtal::ctor<T , A0, A1, A2, A3, A4, A5, A6>());
	}

	/// 8�����̃R���X�g���N�^�֐���o�^����
	template<class T , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7>
	const NativeFunPtr& def_ctor8(){
		return def_ctor(xtal::ctor<T , A0, A1, A2, A3, A4, A5, A6, A7>());
	}

	/// 9�����̃R���X�g���N�^�֐���o�^����
	template<class T , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8>
	const NativeFunPtr& def_ctor9(){
		return def_ctor(xtal::ctor<T , A0, A1, A2, A3, A4, A5, A6, A7, A8>());
	}

	/// 10�����̃R���X�g���N�^�֐���o�^����
	template<class T , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9>
	const NativeFunPtr& def_ctor10(){
		return def_ctor(xtal::ctor<T , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9>());
	}

	/// 11�����̃R���X�g���N�^�֐���o�^����
	template<class T , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10>
	const NativeFunPtr& def_ctor11(){
		return def_ctor(xtal::ctor<T , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10>());
	}

	/// 12�����̃R���X�g���N�^�֐���o�^����
	template<class T , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11>
	const NativeFunPtr& def_ctor12(){
		return def_ctor(xtal::ctor<T , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11>());
	}

	/// 13�����̃R���X�g���N�^�֐���o�^����
	template<class T , class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12>
	const NativeFunPtr& def_ctor13(){
		return def_ctor(xtal::ctor<T , A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12>());
	}

//}}REPEAT}

public:

	struct cpp_class_t{};

	Class(cpp_class_t);

	void on_rawcall(const VMachinePtr& vm);
	
	void s_new(const VMachinePtr& vm);

	void init_instance(const AnyPtr& self, const VMachinePtr& vm);
	
	void set_member_direct(int_t i, const IDPtr& primary_key, const AnyPtr& value, const AnyPtr& secondary_key, int_t accessibility);

	const AnyPtr& find_member(const IDPtr& primary_key, const AnyPtr& secondary_key, int_t& accessibility, bool& nocache);

	const AnyPtr& find_member(const IDPtr& primary_key, const AnyPtr& secondary_key, bool inherited_too, int_t& accessibility, bool& nocache);

	const AnyPtr& find_member_from_inherited_classes(const IDPtr& primary_key, const AnyPtr& secondary_key, int_t& accessibility, bool& nocache);

	ValuesPtr child_object_name(const AnyPtr& a);

	void set_object_temporary_name(const IDPtr& name);

	const IDPtr& object_temporary_name();

	void on_set_object_parent(const ClassPtr& parent);

	uint_t object_force(){
		return object_force_;
	}

	void set_object_force(uint_t force){
		object_force_ = (u16)force;
	}

	ClassInfo* info(){
		return (ClassInfo*)Frame::info();
	}

	void unset_native(){
		flags_ &= ~FLAG_NATIVE;
	}
	
	bool is_native(){
		return (flags_&FLAG_NATIVE)!=0;
	}

	bool is_final(){
		return (flags_&FLAG_FINAL)!=0;
	}

	void set_final(){
		flags_ |= FLAG_FINAL;
	}

	bool is_singleton(){
		return (flags_&FLAG_SINGLETON)!=0;
	}

	void prebind();

	bool bind(int_t n = -1);

	void set_symbol_data(CppClassSymbolData* data){
		XTAL_ASSERT(!symbol_data_);
		symbol_data_ = data;
	}

	CppClassSymbolData* symbol_data(){
		return symbol_data_;
	}

	void set_singleton();

	void init_singleton(const VMachinePtr& vm);

private:

	void init();

	const NativeFunPtr& ctor(int_t type);

	const NativeFunPtr& def_ctor(int_t type, const NativeFunPtr& ctor_func);

public:

	const NativeFunPtr& def_and_return(const IDPtr& primary_key, const AnyPtr& secondary_key, int_t accessibility, const param_types_holder_n& pth, const void* val);

	void define(const char_t* primary_key, const param_types_holder_n& pth);
	void define(const char_t* primary_key, const AnyPtr& secondary_key, const param_types_holder_n& pth);
	void define_param(const char_t* name, const AnyPtr& default_value);
	void define(const char_t* primary_key, const AnyPtr& valueh);
	void define(const char_t* primary_key, const AnyPtr& value, const AnyPtr& secondary_key);

public:

	void on_visit_members(Visitor& m){
		Frame::on_visit_members(m);
		for(uint_t i=0; i<inherited_classes_.size(); ++i){
			m & inherited_classes_.at(i);
		}
	}

protected:

	void fillup_inherited_classes();
	
	const AnyPtr& def2(const IDPtr& primary_key, const AnyPtr& value, const AnyPtr& secondary_key = null, int_t accessibility = KIND_PUBLIC);

	IDPtr name_;

	NativeFunPtr ctor_[2];

	xarray inherited_classes_;
	CppClassSymbolData* symbol_data_;

	u16 object_force_;
	u16 flags_;

	enum{
		FLAG_NATIVE = 1<<0,
		FLAG_FINAL = 1<<1,
		FLAG_SINGLETON = 1<<2,
		FLAG_PREBINDED = 1<<3,
		FLAG_BINDED = 1<<4,
		FLAG_BINDED2 = 1<<5,

		FLAG_LAST_DEFINED_CTOR = 1<<7,
		FLAG_LAST_DEFINED_CTOR2 = 1<<8,

	};
};

}

#endif // XTAL_CLASS_H_INCLUDE_GUARD