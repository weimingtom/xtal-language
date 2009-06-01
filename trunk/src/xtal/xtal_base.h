/** \file src/xtal/xtal_base.h
* \brief src/xtal/xtal_base.h
*/

#ifndef XTAL_BASE_H_INCLUDE_GUARD
#define XTAL_BASE_H_INCLUDE_GUARD

#pragma once

namespace xtal{

void* xmalloc(size_t);
void xfree(void*p, size_t);
void register_gc(RefCountingBase* p);
void register_gc_observer(GCObserver* p);
void unregister_gc_observer(GCObserver* p);

/**
* \brief �Q�ƃJ�E���^�̋@�\��L����N���X
*/
class RefCountingBase : public Any{
public:
	
	RefCountingBase()
		:Any(noinit_t()){}

	virtual ~RefCountingBase(){}

public:

	/**
	* \brief �֐��I�u�W�F�N�g�Ƃ݂Ȃ��A�֐��Ăяo��������B
	* ������߂�l��vm��ʂ��Ă���肷��B
	*/
	virtual void rawcall(const VMachinePtr& vm);

	/**
	* \brief name�����o�������lvalue�Œ�`����B
	*/
	virtual void def(const IDPtr& primary_key, const AnyPtr& value, const AnyPtr& secondary_key = (const AnyPtr&)undefined, int_t accessibility = 0);

	/**
	* \brief name�����o���擾����B
	* \retval null ���̃����o�͑��݂��Ȃ�
	* \retval ��null name�ɑΉ����������o  
	*/
	virtual const AnyPtr& rawmember(const IDPtr& primary_key, const AnyPtr& secondary_key, bool inherited_too, int_t& accessibility, bool& nocache);

	/**
	* \brief ���̃I�u�W�F�N�g�������o�ƂȂ��Ă���e�̃N���X��Ԃ��B
	*/
	virtual const ClassPtr& object_parent();
	
	/**
	* \brief ���̃I�u�W�F�N�g�ɐe��ݒ肷��B
	* �e�����ĂȂ��I�u�W�F�N�g��A�O�ɕt����ꂽ�e�̕��������ꍇ���������B
	* \param parent �e
	*/
	virtual void set_object_parent(const ClassPtr& parent);

	/**
	* \brief �t�@�C�i���C�U
	* full_gc���ɁA���ʗ\��̃I�u�W�F�N�g�ƂȂ������ɔj�������邽�߂̊֐��B
	* �������Aset_finalizer_flag()���Ă�ł��Ȃ���ΌĂ΂�邱�Ƃ͂Ȃ��B
	* �j������Xtal�̃����o�ϐ���G�邱�Ƃ�����̂ł���΂�����g���B
	* �����łȂ��̂ł���΁A�f�X�g���N�^�ɔj���������������ƁB
	*/
	virtual void finalize();

	virtual void visit_members(Visitor& m){}

public:

	enum{
		HAVE_FINALIZER_FLAG_SHIFT = TYPE_SHIFT+1,
		HAVE_FINALIZER_FLAG_BIT = 1<<HAVE_FINALIZER_FLAG_SHIFT,

		GENE_COUNT_BIT_SHIFT = HAVE_FINALIZER_FLAG_SHIFT+1,
		GENE_COUNT_BIT_MASK = (1<<GENE_COUNT_BIT_SHIFT) | (1<<(GENE_COUNT_BIT_SHIFT+1)),

		REF_COUNT_SHIFT = GENE_COUNT_BIT_SHIFT+2,
		REF_COUNT_MASK = ~((1<<REF_COUNT_SHIFT)-1)
	};

	bool have_finalizer(){ return (type_ & HAVE_FINALIZER_FLAG_BIT)!=0; }
	void set_finalizer_flag(){ type_ |= HAVE_FINALIZER_FLAG_BIT; }

	uint_t ref_count(){ return (type_ & REF_COUNT_MASK)>>REF_COUNT_SHIFT; }
	void add_ref_count(int_t rc){ type_ += rc<<REF_COUNT_SHIFT; }
	void inc_ref_count(){ type_ += 1<<REF_COUNT_SHIFT; }
	void dec_ref_count(){ type_ -= 1<<REF_COUNT_SHIFT; }

	uint_t ungc(){ return type_&(REF_COUNT_MASK|HAVE_FINALIZER_FLAG_BIT); }

	uint_t gene_count(){ return type_&GENE_COUNT_BIT_MASK; }
	void inc_gene_count(){ if(gene_count()!=GENE_COUNT_BIT_MASK)type_+=1<<GENE_COUNT_BIT_SHIFT; }

	void destroy(){ delete this; }

	void object_free(){ xfree(this, value_.uvalue); }

public:

	static void* operator new(size_t size){ 
		return xmalloc(size);
	}

	static void operator delete(void* p, size_t size){
		if(p){ ((RefCountingBase*)p)->value_.ivalue = (int_t)size; }
	}
	
	static void* operator new(size_t, void* p){ return p; }
	static void operator delete(void*, void*){}

};

/**
* @brief ���N���X
*/
class Base : public RefCountingBase{
public:

	/**
	* \brief ���̃I�u�W�F�N�g����������N���X��Ԃ��B
	*/
	const ClassPtr& get_class();

public:
	
	Base();

protected:

	Base(const Base& b);

	Base& operator =(const Base& b);

public:

	virtual ~Base();
			
public:

	InstanceVariables* instance_variables(){ return instance_variables_; }

	void make_instance_variables();

	void set_class(const ClassPtr& c);

	virtual void visit_members(Visitor& m);

protected:

	// �Q�ƃJ�E���^�l
	//uint_t ref_count_;

	// �C���X�^���X�ϐ��e�[�u��
	InstanceVariables* instance_variables_;

	// �����N���X
	Class* class_;
};

}


#endif // XTAL_BASE_H_INCLUDE_GUARD