
#include "xtal.h"
#include "xtal_cast.h"
#include "xtal_frame.h"
#include "xtal_frameimpl.h"

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

	const void* fetch(const Any& a, const void* type_key){
		uint_t ia = a.rawvalue();
		uint_t hash = (((uint_t)type_key)>>3) | (ia>>2);
		Unit& unit = table_[hash & (CACHE_MAX-1)];
		if(global_mutate_count==unit.mutate_count && type_key==unit.key1 && ia==unit.key2){
			//printf("e");
			if(unit.same){
				return &a;
			}else{
				return unit.value;
			}
		}
		//printf("n");
		return 0;
	}

	void store(const Any& a, const void* value, const void* type_key){
		if(a.type()!=TYPE_BASE){
			return;
		}

		uint_t ia = a.rawvalue();
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

const void* fetch_cast_cache(const Any& a, const void* type_key){
	return cast_cache_table.fetch(a, type_key);
}

void store_cast_cache(const Any& a, const void* ret, const void* type_key){
	cast_cache_table.store(a, ret, type_key);
}


void* cast_helper_helper_extend_anyimpl(const Any& a, const Class& cls){
	if(a.is(cls)){ return (void*)a.impl(); }
	XTAL_THROW(cast_error(a, cls.object_name()));
	return 0;
}

void* cast_helper_helper_extend_any(const Any& a, const Class& cls){
	if(a.is(cls)){ return (void*)&a; }
	XTAL_THROW(cast_error(a, cls.object_name()));
	return 0;
}

void* cast_helper_helper_other(const Any& a, const Class& cls){
	if(a.is(cls)){ return ((UserData<int>&)a).get(); }
	XTAL_THROW(cast_error(a, cls.object_name()));
	return 0;
}

void* arg_cast_helper_helper_extend_anyimpl(const Any& a, int param_num, const Any& param_name, const Class& cls){
	if(a.is(cls)){ return (void*)a.impl(); }
	XTAL_THROW(argument_error(a, cls.object_name(), param_num, param_name));
	return 0;
}

void* arg_cast_helper_helper_extend_any(const Any& a, int param_num, const Any& param_name, const Class& cls){
	if(a.is(cls)){ return (void*)&a; }
	XTAL_THROW(argument_error(a, cls.object_name(), param_num, param_name));
	return 0;
}

void* arg_cast_helper_helper_other(const Any& a, int param_num, const Any& param_name, const Class& cls){
	if(a.is(cls)){ return ((UserData<int>&)a).get(); }
	XTAL_THROW(argument_error(a, cls.object_name(), param_num, param_name));
	return 0;
}

const ID* CastHelper<const ID*>::as(const Any& a){ 
	if(const String* p = xtal::as<const String*>(a)){
		if(p->is_interned()){
			return (const ID*)p;
		}
	}
	return 0;
}

const ID* CastHelper<const ID*>::cast(const Any& a){
	if(const ID* p = as(a)){
		return p;
	}
	XTAL_THROW(cast_error(a, "interned String"));
	return 0;
}

const ID* CastHelper<const ID*>::arg_cast(const Any& a, int param_num, const Any& param_name){
	if(const ID* p = as(a)){
		return p;
	}
	XTAL_THROW(argument_error(a, "interned String", param_num, param_name));
	return 0;
}


const char* CastHelper<const char*>::as(const Any& a){ 
	if(const String* p = xtal::as<const String*>(a)){
		return p->c_str();
	}
	return 0;
}

const char* CastHelper<const char*>::cast(const Any& a){
	if(const char* p = as(a)){
		return p;
	}
	XTAL_THROW(cast_error(a, TClass<String>::get().object_name()));
	return 0;
}

const char* CastHelper<const char*>::arg_cast(const Any& a, int param_num, const Any& param_name){
	if(const char* p = as(a)){
		return p;
	}
	XTAL_THROW(argument_error(a, TClass<String>::get().object_name(), param_num, param_name));
	return 0;
}


int_t CastHelper<int_t>::as(const Any& a){
	switch(a.type()){
		XTAL_DEFAULT;
		XTAL_CASE(TYPE_INT){ return a.ivalue(); }
		XTAL_CASE(TYPE_FLOAT){ return static_cast<int_t>(a.fvalue()); }
	}
	return 0;
}
	
int_t CastHelper<int_t>::cast(const Any& a){
	switch(a.type()){
		XTAL_DEFAULT{ XTAL_THROW(cast_error(a, String("Int"))); }
		XTAL_CASE(TYPE_INT){ return a.ivalue(); }
		XTAL_CASE(TYPE_FLOAT){ return static_cast<int_t>(a.fvalue()); }
	}
	return 0;
}

int_t CastHelper<int_t>::arg_cast(const Any& a, int param_num, const Any& param_name){
	switch(a.type()){
		XTAL_DEFAULT{ XTAL_THROW(argument_error(a, String("Int"), param_num, param_name)); }
		XTAL_CASE(TYPE_INT){ return a.ivalue(); }
		XTAL_CASE(TYPE_FLOAT){ return static_cast<int_t>(a.fvalue()); }
	}
	return 0;
}
	
float_t CastHelper<float_t>::as(const Any& a){
	switch(a.type()){
		XTAL_DEFAULT;
		XTAL_CASE(TYPE_INT){ return static_cast<float_t>(a.ivalue()); }
		XTAL_CASE(TYPE_FLOAT){ return a.fvalue(); }
	}
	return 0;
}
	
float_t CastHelper<float_t>::cast(const Any& a){
	switch(a.type()){
		XTAL_DEFAULT{ XTAL_THROW(cast_error(a, String("Float"))); }
		XTAL_CASE(TYPE_INT){ return static_cast<float_t>(a.ivalue()); }
		XTAL_CASE(TYPE_FLOAT){ return a.fvalue(); }
	}
	return 0;
}

float_t CastHelper<float_t>::arg_cast(const Any& a, int param_num, const Any& param_name){
	switch(a.type()){
		XTAL_DEFAULT{ XTAL_THROW(argument_error(a, String("Float"), param_num, param_name)); }
		XTAL_CASE(TYPE_INT){ return static_cast<float_t>(a.ivalue()); }
		XTAL_CASE(TYPE_FLOAT){ return a.fvalue(); }
	}
	return 0;
}

}
