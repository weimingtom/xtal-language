#include "xtal.h"
#include "xtal_macro.h"

namespace xtal{

class StringMgr : public GCObserver{
public:

	struct Key{
		const char_t* str;
		uint_t size;
		uint_t hashcode;
	};

	struct Fun{
		static uint_t hash(const Key& key){
			return key.hashcode;
		}

		static bool eq(const Key& a, const Key& b){
			return a.hashcode==b.hashcode && a.size==b.size && std::memcmp(a.str, b.str, a.size*sizeof(char_t))==0;
		}
	};

	typedef Hashtable<Key, IDPtr, Fun> table_t; 
	table_t table_;

	struct Fun2{
		static uint_t hash(const void* key){
			return ((uint_t)key)>>3;
		}

		static bool eq(const void* a, const void* b){
			return a==b;
		}
	};

	typedef Hashtable<const void*, IDPtr, Fun2> table2_t; 
	table2_t table2_;

	StringMgr(){
		guard_ = 0;
	}

protected:

	int_t guard_;

	struct Guard{
		int_t& count;
		Guard(int_t& c):count(c){ count++; }
		~Guard(){ count--; }
	private:
		void operator=(const Guard&);
	};

	virtual void visit_members(Visitor& m){
		GCObserver::visit_members(m);
		for(table_t::iterator it = table_.begin(); it!=table_.end(); ++it){
			m & it->second;
		}		
	}

public:

	const IDPtr& insert(const char_t* str, uint_t size){
		uint_t hashcode, length;
		string_hashcode_and_length(str, size, hashcode, length);
		return insert(str, size, hashcode, length);
	}

	const IDPtr& insert(const char_t* str){
		uint_t hashcode, length, size;
		string_data_size_and_hashcode_and_length(str, size, hashcode, length);
		return insert(str, size, hashcode, length);
	}

	const IDPtr& insert(const char_t* str, uint_t size, uint_t hash, uint_t length);

	const IDPtr& insert_literal(const char_t* str){
		IDPtr& ret = table2_[str];
		if(!ret){
			uint_t hashcode, length, size;
			string_data_size_and_hashcode_and_length(str, size, hashcode, length);
			ret = insert(str, size, hashcode, length);
		}
		return ret;
	}

	virtual void before_gc();
};

const IDPtr& StringMgr::insert(const char_t* str, uint_t size, uint_t hashcode, uint_t length){
	Guard guard(guard_);

	Key key = {str, size, hashcode};
	table_t::iterator it = table_.find(key, hashcode);
	if(it!=table_.end()){
		return it->second;
	}

	it = table_.insert(key, xnew<ID>(str, size, hashcode, length), hashcode).first;
	it->first.str = it->second->data();
	return it->second;
}

void StringMgr::before_gc(){
	return;

	if(guard_){
		return;
	}
}

const IDPtr& intern_literal(const char_t* str){
	return unchecked_ptr_cast<StringMgr>(environment()->string_mgr_)->insert_literal(str);
}

const IDPtr& intern(const char_t* str){
	return unchecked_ptr_cast<StringMgr>(environment()->string_mgr_)->insert(str);
}

const IDPtr& intern(const char_t* str, uint_t data_size){
	return unchecked_ptr_cast<StringMgr>(environment()->string_mgr_)->insert(str, data_size);
}

const IDPtr& intern(const char_t* str, uint_t data_size, uint_t hash, uint_t length){
	return unchecked_ptr_cast<StringMgr>(environment()->string_mgr_)->insert(str, data_size, hash, length);
}

class InternedStringIter : public Base{
	StringMgr::table_t::iterator iter_, last_;
public:

	InternedStringIter(StringMgr::table_t::iterator begin, StringMgr::table_t::iterator end)
		:iter_(begin), last_(end){
	}
			
	void block_next(const VMachinePtr& vm){
		if(iter_!=last_){
			vm->return_result(from_this(this), iter_->second);
			++iter_;
		}
		else{
			vm->return_result(null, null);
		}
	}
};

AnyPtr interned_strings(){
	return xnew<InternedStringIter>(
		unchecked_ptr_cast<StringMgr>(environment()->string_mgr_)->table_.begin(), 
		unchecked_ptr_cast<StringMgr>(environment()->string_mgr_)->table_.end());
}



void Environment::initialize(){

	global_mutate_count_ = 0;

	ClassPtr* holders[] = { 
		&cpp_class_map_.insert(&CppClassSymbol<Any>::value, null).first->second,
		&cpp_class_map_.insert(&CppClassSymbol<Class>::value, null).first->second,
		&cpp_class_map_.insert(&CppClassSymbol<CppClass>::value, null).first->second,
		&cpp_class_map_.insert(&CppClassSymbol<Array>::value, null).first->second,
	};

	for(int i=0; i<sizeof(holders)/sizeof(holders[0]); ++i){
		*holders[i] = (ClassPtr&)ap(Any((Class*)Base::operator new(sizeof(CppClass))));
	}
	
	for(int i=0; i<sizeof(holders)/sizeof(holders[0]); ++i){
		Base* p = pvalue(*holders[i]);
		new(p) CppClass();
	}

	for(int i=0; i<sizeof(holders)/sizeof(holders[0]); ++i){
		Base* p = pvalue(*holders[i]);
		p->set_class(get_cpp_class<CppClass>());
	}
	
	for(int i=0; i<sizeof(holders)/sizeof(holders[0]); ++i){
		Base* p = pvalue(*holders[i]);
		register_gc(p);
	}

//////////

	set_cpp_class<Base>(get_cpp_class<Any>());
	set_cpp_class<Singleton>(get_cpp_class<CppClass>());
	set_cpp_class<IteratorClass>(get_cpp_class<CppClass>());

	builtin_ = xnew<Singleton>();
	lib_ = xnew<Lib>();
	Iterator_ = xnew<IteratorClass>();
	Iterable_ = xnew<Class>();

	vm_list_ = xnew<Array>();

	string_mgr_ = xnew<StringMgr>();
}

void Environment::uninitialize(){
	for(cpp_class_map_t::iterator it=cpp_class_map_.begin(); it!=cpp_class_map_.end(); ++it){
		it->second = null;
	}

	cpp_class_map_.destroy();
}

const ClassPtr& Environment::new_cpp_class(const StringPtr& name, void* key){
	ClassPtr& p = cpp_class_map_[key];
	if(!p){ p = xnew<CppClass>(name); }
	return p;
}
	
VMachinePtr Environment::vm_take_over(){
	if(vm_list_->empty()){
		vm_list_->push_back(xnew<VMachine>());
	}
	VMachinePtr vm = unchecked_ptr_cast<VMachine>(vm_list_->back());
	vm_list_->pop_back();
	return vm;
}

void Environment::vm_take_back(const VMachinePtr& vm){
	vm->reset();
	vm_list_->push_back(vm);
}

const AnyPtr& Environment::MemberCacheTable::cache(const Any& target_class, const IDPtr& primary_key, const Any& secondary_key, const Any& self, bool inherited_too, uint_t global_mutate_count){
	uint_t itarget_class = rawvalue(target_class) | (uint_t)inherited_too;
	uint_t iprimary_key = rawvalue(primary_key);
	uint_t ins = rawvalue(secondary_key);

	uint_t hash = itarget_class ^ (iprimary_key>>2) ^ ins + iprimary_key ^ type(primary_key);
	Unit& unit = table_[hash & CACHE_MASK];
	if(global_mutate_count==unit.mutate_count && itarget_class==unit.target_class && raweq(primary_key, unit.primary_key) && ins==unit.secondary_key){
		hit_++;
		return ap(unit.member);
	}
	else{
		miss_++;

		if(type(target_class)!=TYPE_BASE){
			return undefined;
		}

		bool nocache = false;
		unit.member = pvalue(target_class)->do_member(primary_key, ap(secondary_key), ap(self), inherited_too, &nocache);
		if(!nocache){
			unit.target_class = itarget_class;
			unit.primary_key = primary_key;
			unit.secondary_key = ins;
			unit.mutate_count = global_mutate_count;
		}
		return ap(unit.member);
	}
}

bool Environment::IsInheritedCacheTable::cache_is(const Any& target_class, const Any& klass, uint_t global_mutate_count){
	uint_t itarget_class = rawvalue(target_class);
	uint_t iklass = rawvalue(klass);

	uint_t hash = (itarget_class>>3) ^ (iklass>>2);
	Unit& unit = table_[hash & CACHE_MASK];

	if(global_mutate_count==unit.mutate_count && itarget_class==unit.target_class && iklass==unit.klass){
		hit_++;
		return unit.result;
	}
	else{
		miss_++;
		// キャッシュに保存
		unit.target_class = itarget_class;
		unit.klass = iklass;
		unit.mutate_count = global_mutate_count;
		unit.result = unchecked_ptr_cast<Class>(ap(target_class))->is_inherited(ap(klass));

		return unit.result;
	}
}

bool Environment::IsInheritedCacheTable::cache_is_inherited(const Any& target_class, const Any& klass, uint_t global_mutate_count){
	uint_t itarget_class = rawvalue(target_class);
	uint_t iklass = rawvalue(klass);

	uint_t hash = (itarget_class>>3) ^ (iklass>>2);
	Unit& unit = table_[hash & CACHE_MASK];

	if(global_mutate_count==unit.mutate_count && itarget_class==unit.target_class && iklass==unit.klass){
		hit_++;
		return unit.result;
	}
	else{
		miss_++;
		// キャッシュに保存
		unit.target_class = itarget_class;
		unit.klass = iklass;
		unit.mutate_count = global_mutate_count;

		if(const ClassPtr& cls = ptr_as<Class>(ap(target_class))){
			unit.result = cls->is_inherited(ap(klass));
		}
		else{
			unit.result = false;
		}

		return unit.result;
	}
}


const ClassPtr& RuntimeError(){
	return ptr_cast<Class>(builtin()->member(Xid(RuntimeError)));
}

const ClassPtr& CompileError(){
	return ptr_cast<Class>(builtin()->member(Xid(CompileError)));
}

const ClassPtr& UnsupportedError(){
	return ptr_cast<Class>(builtin()->member(Xid(UnsupportedError)));
}

const ClassPtr& ArgumentError(){
	return ptr_cast<Class>(builtin()->member(Xid(ArgumentError)));
}

const StreamPtr& stdin_stream(){
	return ptr_cast<Stream>(builtin()->member(Xid(stdin)));
}

const StreamPtr& stdout_stream(){
	return ptr_cast<Stream>(builtin()->member(Xid(stdout)));
}

const StreamPtr& stderr_stream(){
	return ptr_cast<Stream>(builtin()->member(Xid(stderr)));
}

}
