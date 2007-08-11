
#include "xtal.h"
#include "xtal_cast.h"
#include "xtal_frame.h"
#include "xtal_frame.h"

namespace xtal{

struct CastCacheTable{
	struct Unit{
		const void* key1;
		int_t key2;
		const void* value;
		bool same;
		uint_t mutate_count;
	};

	enum{ CACHE_MAX = 256 };

	Unit table_[CACHE_MAX];

	CastCacheTable(){
		for(int_t i=0; i<CACHE_MAX; ++i){
			table_[i].value = 0;
			table_[i].key1 = 0;
			table_[i].key2 = 0;
			table_[i].mutate_count = 0;
		}
	}

	const void* fetch(const AnyPtr& a, const void* type_key){
		uint_t ia = rawvalue(a);
		uint_t hash = (((uint_t)type_key)>>3) | (ia>>2);
		Unit& unit = table_[hash & (CACHE_MAX-1)];
		if(global_mutate_count==unit.mutate_count && type_key==unit.key1 && ia==unit.key2){
			if(unit.same){
				return &a;
			}else{
				return unit.value;
			}
		}
		return 0;
	}

	void store(const AnyPtr& a, const void* value, const void* type_key){
		if(type(a)!=TYPE_BASE){
			return;
		}

		uint_t ia = rawvalue(a);
		uint_t hash = (((uint_t)type_key)>>3) | (ia>>2);
		Unit& unit = table_[hash & (CACHE_MAX-1)];
		unit.key1 = type_key;
		unit.key2 = ia;
		unit.value = value;
		unit.same = &a==value;
		unit.mutate_count = global_mutate_count;
	}

};

namespace{
	CastCacheTable cast_cache_table;
}

const void* fetch_cast_cache(const AnyPtr& a, const void* type_key){
	return cast_cache_table.fetch(a, type_key);
}

void store_cast_cache(const AnyPtr& a, const void* ret, const void* type_key){
	cast_cache_table.store(a, ret, type_key);
}


void* cast_helper_helper_extend_anyimpl(const AnyPtr& a, const ClassPtr& cls){
	if(a->is(cls)){ return (void*)pvalue(a); }
	XTAL_THROW(cast_error(a, cls->object_name()), return 0);
}

void* cast_helper_helper_extend_any(const AnyPtr& a, const ClassPtr& cls){
	if(a->is(cls)){ return (void*)&a; }
	XTAL_THROW(cast_error(a, cls->object_name()), return 0);
}

void* cast_helper_helper_other(const AnyPtr& a, const ClassPtr& cls){
	if(a->is(cls)){ return ((SmartPtr<int>&)a).get(); }
	XTAL_THROW(cast_error(a, cls->object_name()), return 0);
}

void* arg_cast_helper_helper_extend_anyimpl(const AnyPtr& a, int_t param_num, const AnyPtr& param_name, const ClassPtr& cls){
	if(a->is(cls)){ return (void*)pvalue(a); }
	XTAL_THROW(argument_error(a, cls->object_name(), param_num, param_name), return 0);
}

void* arg_cast_helper_helper_extend_any(const AnyPtr& a, int_t param_num, const AnyPtr& param_name, const ClassPtr& cls){
	if(a->is(cls)){ return (void*)&a; }
	XTAL_THROW(argument_error(a, cls->object_name(), param_num, param_name), return 0);
}

void* arg_cast_helper_helper_other(const AnyPtr& a, int_t param_num, const AnyPtr& param_name, const ClassPtr& cls){
	if(a->is(cls)){ return ((SmartPtr<int>&)a).get(); }
	XTAL_THROW(argument_error(a, cls->object_name(), param_num, param_name), return 0);
}

const InternedStringPtr* CastHelper<const InternedStringPtr*>::as(const AnyPtr& a){ 
	if(String* p = xtal::as<String*>(a)){
		if(p->is_interned()){
			return (const InternedStringPtr*)p;
		}
	}
	return 0;
}

const InternedStringPtr* CastHelper<const InternedStringPtr*>::cast(const AnyPtr& a){
	if(const InternedStringPtr* p = as(a)){
		return p;
	}
	XTAL_THROW(cast_error(a, "interned String"), return 0);
}

const InternedStringPtr* CastHelper<const InternedStringPtr*>::arg_cast(const AnyPtr& a, int_t param_num, const AnyPtr& param_name){
	if(const InternedStringPtr* p = as(a)){
		return p;
	}
	XTAL_THROW(argument_error(a, "interned String", param_num, param_name), return 0);
}


const char* CastHelper<const char*>::as(const AnyPtr& a){ 
	if(String* p = xtal::as<String*>(a)){
		return p->c_str();
	}
	return 0;
}

const char* CastHelper<const char*>::cast(const AnyPtr& a){
	if(const char* p = as(a)){
		return p;
	}
	XTAL_THROW(cast_error(a, get_cpp_class<String>()->object_name()), return 0);
}

const char* CastHelper<const char*>::arg_cast(const AnyPtr& a, int_t param_num, const AnyPtr& param_name){
	if(const char* p = as(a)){
		return p;
	}
	XTAL_THROW(argument_error(a, get_cpp_class<String>()->object_name(), param_num, param_name), return 0);
}


int_t CastHelper<int_t>::as(const AnyPtr& a){
	switch(type(a)){
		XTAL_DEFAULT;
		XTAL_CASE(TYPE_INT){ return ivalue(a); }
		XTAL_CASE(TYPE_FLOAT){ return static_cast<int_t>(fvalue(a)); }
	}
	return 0;
}
	
int_t CastHelper<int_t>::cast(const AnyPtr& a){
	switch(type(a)){
		XTAL_DEFAULT{ XTAL_THROW(cast_error(a, xnew<String>("Int")), return 0); }
		XTAL_CASE(TYPE_INT){ return ivalue(a); }
		XTAL_CASE(TYPE_FLOAT){ return static_cast<int_t>(fvalue(a)); }
	}
	return 0;
}

int_t CastHelper<int_t>::arg_cast(const AnyPtr& a, int_t param_num, const AnyPtr& param_name){
	switch(type(a)){
		XTAL_DEFAULT{ XTAL_THROW(argument_error(a, xnew<String>("Int"), param_num, param_name), return 0); }
		XTAL_CASE(TYPE_INT){ return ivalue(a); }
		XTAL_CASE(TYPE_FLOAT){ return static_cast<int_t>(fvalue(a)); }
	}
	return 0;
}
	
float_t CastHelper<float_t>::as(const AnyPtr& a){
	switch(type(a)){
		XTAL_DEFAULT;
		XTAL_CASE(TYPE_INT){ return static_cast<float_t>(ivalue(a)); }
		XTAL_CASE(TYPE_FLOAT){ return fvalue(a); }
	}
	return 0;
}
	
float_t CastHelper<float_t>::cast(const AnyPtr& a){
	switch(type(a)){
		XTAL_DEFAULT{ XTAL_THROW(cast_error(a, xnew<String>("Float")), return 0); }
		XTAL_CASE(TYPE_INT){ return static_cast<float_t>(ivalue(a)); }
		XTAL_CASE(TYPE_FLOAT){ return fvalue(a); }
	}
	return 0;
}

float_t CastHelper<float_t>::arg_cast(const AnyPtr& a, int_t param_num, const AnyPtr& param_name){
	switch(type(a)){
		XTAL_DEFAULT{ XTAL_THROW(argument_error(a, xnew<String>("Float"), param_num, param_name), return 0); }
		XTAL_CASE(TYPE_INT){ return static_cast<float_t>(ivalue(a)); }
		XTAL_CASE(TYPE_FLOAT){ return fvalue(a); }
	}
	return 0;
}

}
