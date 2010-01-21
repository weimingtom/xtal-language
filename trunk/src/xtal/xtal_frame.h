/** \file src/xtal/xtal_frame.h
* \brief src/xtal/xtal_frame.h
*/

#ifndef XTAL_FRAME_H_INCLUDE_GUARD
#define XTAL_FRAME_H_INCLUDE_GUARD

#pragma once

namespace xtal{

/**
* \xbind lib::builtin
* \xinherit lib::builtin::Any
* \brief フレーム
*/
class Frame : public HaveParentBase{
public:
	
	Frame(const FramePtr& outer, const CodePtr& code, ScopeInfo* info);
	
	Frame();

	~Frame();

public:
	
	/**
	* \xbind
	* \brief 外側のスコープを表すFrameオブジェクトを返す。
	*/
	const FramePtr& outer(){ 
		return outer_; 
	}

	void set_outer(const FramePtr& outer){
		outer_ = outer;
	}

	const CodePtr& code(){ 
		return code_; 
	}

	void set_code(const CodePtr& code){
		code_ = code;
	}

	ScopeInfo* info(){
		return scope_info_;
	}

	void set_info(ScopeInfo* info){
		scope_info_ = info;
	}

	/**
	* \brief リテラル時に定義された要素の数を返す。
	*/
	int_t block_size(){ 
		return scope_info_->variable_size; 
	}

	/**
	* \xbind
	* \brief メンバが格納された、Iteratorを返す
	* ブロックパラメータは(primary_key, secondary_key, value)
	*/
	AnyPtr members();

public:

	/**
	* \brief i番目のメンバーをダイレクトに取得。
	*/
	AnyPtr& member_direct(int_t i){
		return (AnyPtr&)members_.at(i);
	}

	/**
	* \brief i番目のメンバーをダイレクトに設定。
	*/
	void set_member_direct(int_t i, const AnyPtr& value){
		if(orphan_){
			members_.set_at(i, value);
		}
		else{
			//members_.set_at(i, value);
			members_.set_at_unref(i, value);
		}
	}

	void resize_member_direct(int_t size){
		members_.resize(size);
	}

	bool replace_member(const IDPtr& primary_key, const AnyPtr& value);

private:

	/**
	* \brief i番目のメンバーを設定。
	*/
	void set_member_direct_unref(int_t i, const Any& value){
		//members_.set_at(i, (AnyPtr&)value);
		members_.set_at_unref(i, value);
	}

	void add_ref_count_members(int_t i);

protected:

	void make_map_members();
	void make_members();
	void make_members_force(int_t flags);

	void push_back_member(const AnyPtr& value);

	uint_t member_size(){
		return members_.size();
	}

private:

	FramePtr outer_;
	CodePtr code_;
	ScopeInfo* scope_info_;
	
	xarray members_;

public:

	struct Key{
		IDPtr primary_key;
		AnyPtr secondary_key;

		friend void visit_members(Visitor& m, Key& a){
			m & a.primary_key & a.secondary_key;
		}
	};

	enum{
		FLAG_ACCESSIBILITY_MASK = 0x3,
		FLAG_NOCACHE = 1<<3
	};

	struct Value{
		u16 num;
		u16 flag;

		int_t accessibility() const{
			return flag & FLAG_ACCESSIBILITY_MASK;
		}

		bool nocache() const{
			return (flag&FLAG_NOCACHE)==0;
		}

		friend void visit_members(Visitor& m, const Value&){}
	};

	struct Fun{
		static uint_t hash(const Key& key){
			return (rawvalue(key.primary_key).u()>>3) ^ rawvalue(key.secondary_key).u();
		}

		static bool eq(const Key& a, const Key& b){
			return raweq(a.primary_key, b.primary_key) && raweq(a.secondary_key, b.secondary_key);
		}
	};

protected:

	typedef Hashtable<Key, Value, Fun> map_t; 
	map_t* map_members_;

	bool orphan_;
	bool initialized_members_;

public:
	
	const AnyPtr& on_rawmember(const IDPtr& primary_key, const AnyPtr& secondary_key, bool inherited_too, int_t& accessibility, bool& nocache);

	void on_visit_members(Visitor& m){
		HaveParentBase::on_visit_members(m);
		m & outer_ & code_;

		if(orphan_){
			for(uint_t i=0; i<members_.size(); ++i){
				m & members_.at(i);
			}
		}

		if(map_members_){
			m & *map_members_;
		}
	}

private:
	XTAL_DISALLOW_COPY_AND_ASSIGN(Frame);

	friend class MembersIter;
	friend class MembersIter2;
	friend class VMachine;
};

class MembersIter : public Base{
public:

	MembersIter(const FramePtr& a);

	void block_next(const VMachinePtr& vm);

	void on_visit_members(Visitor& m);

private:
	FramePtr frame_;
	Frame::map_t::iterator it_;
};

class MembersIter2 : public Base{
public:

	MembersIter2(const FramePtr& a);

	void block_next(const VMachinePtr& vm);

	void on_visit_members(Visitor& m);

private:
	FramePtr frame_;
	int_t it_;
};

}

#endif // XTAL_FRAME_H_INCLUDE_GUARD
