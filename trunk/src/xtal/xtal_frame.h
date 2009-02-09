
#pragma once

namespace xtal{

class Frame : public HaveName{
public:
	
	Frame(const FramePtr& outer, const CodePtr& code, ScopeInfo* core);
	
	Frame();

	Frame(const Frame& frame);

	Frame& operator=(const Frame& frame);
		
	~Frame();

public:
	
	/**
	* @brief 外側のスコープを表すFrameオブジェクトを返す。
	*
	*/
	const FramePtr& outer(){ 
		return outer_; 
	}

	const CodePtr& code(){ 
		return code_; 
	}

	/**
	* @brief リテラル時に定義された要素の数を返す。
	*
	*/
	int_t block_size(){ 
		return core_->variable_size; 
	}

	/**
	* @brief i番目のメンバーをダイレクトに取得。
	*
	*/
	const AnyPtr& member_direct(int_t i){
		return members_->at(i);
	}

	/**
	* @brief i番目のメンバーをダイレクトに設定。
	*
	*/
	void set_member_direct(int_t i, const AnyPtr& value){
		members_->set_at(i, value);
	}

	void set_class_member(int_t i, const IDPtr& primary_key, const AnyPtr& value, const AnyPtr& secondary_key, int_t accessibility);
		
	void set_object_name(const StringPtr& name, int_t force, const AnyPtr& parent);

	virtual StringPtr object_name(int_t depth = -1);

public:

	/**
	* @brief メンバが格納された、Iteratorを返す
	*
	* @code
	* Xfor2(key, value, frame.members()){
	*   puts(Xf("%s %s")(key, value).to_s().c_str());
	* }
	* @endcode
	*/
	AnyPtr members();

protected:

	void make_map_members();

	friend class MembersIter;

	FramePtr outer_;
	CodePtr code_;
	ScopeInfo* core_;
	
	ArrayPtr members_;

	struct Key{
		IDPtr primary_key;
		AnyPtr secondary_key;

		friend void visit_members(Visitor& m, const Key& a){
			m & a.primary_key & a.secondary_key;
		}
	};

	struct Value{
		u16 num;
		u16 flags;

		friend void visit_members(Visitor& m, const Value&){}
	};

	struct Fun{
		static uint_t hash(const Key& key){
			return (rawvalue(key.primary_key)>>3) ^ rawvalue(key.secondary_key);
		}

		static bool eq(const Key& a, const Key& b){
			return raweq(a.primary_key, b.primary_key) && raweq(a.secondary_key, b.secondary_key);
		}
	};

	typedef Hashtable<Key, Value, Fun> map_t; 
	//map_t table_;

	map_t* map_members_;


	virtual void visit_members(Visitor& m){
		HaveName::visit_members(m);
		m & outer_ & code_ & members_;
		if(map_members_){
			m & *map_members_;
		}
	}
};

class MembersIter : public Base{
	FramePtr frame_;
	Frame::map_t::iterator it_;

	virtual void visit_members(Visitor& m);

public:

	MembersIter(const FramePtr& a)
		:frame_(a), it_(frame_->map_members_->begin()){
	}

	void block_next(const VMachinePtr& vm);
};

}
