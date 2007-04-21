
#include "marshal.h"

#pragma once

#include "any.h"
#include "marshal.h"
#include "array.h"
#include "frame.h"
#include "cast.h"
#include "xmacro.h"
#include "map.h"
#include "codeimpl.h"
#include "fun.h"
#include "xtal.h"

namespace xtal{

Marshal::Marshal(const Stream& s)
	:stream_(s){
}

void Marshal::dump(const Any& v){
	inner_dump(v);
}

Any Marshal::load(){
	return inner_load();
}

void Marshal::inner_dump(const Any& v){
	if(!v){
		stream_.p1(TNULL);
		return;
	}

	const Class& cls = v.get_class();
	if(cls.raweq(TClass<Int>::get())){
		stream_.p1(INT);
		stream_.p4(v.ivalue());
		return;
	}else if(cls.raweq(TClass<Float>::get())){
		stream_.p1(FLOAT);
		union{ int_t i; float_t f; } u;
		u.f = v.fvalue();
		stream_.p4(u.i);
		return;
	}

	bool added = false;
	int_t num = register_dvalue(v, added);
	if(added){

		if(cls.raweq(TClass<Array>::get())){
			const Array& a = (const Array&)v;
			stream_.p1(ARRAY);
			stream_.p4(a.size());
			for(int_t i=0; i<a.size(); ++i){
				inner_dump(a.at(i));
			}
			return;
		}else if(cls.raweq(TClass<String>::get())){
			const String& a = (const String&)v;
			if(a.is_interned()){
				stream_.p1(TID);
			}else{
				stream_.p1(STRING);
			}
			stream_.p4(a.size());
			for(int_t i=0; i<a.size(); ++i){
				stream_.p1(a.c_str()[i]);
			}
			return;
		}else if(cls.raweq(TClass<Map>::get())){
			const Map& a = (const Map&)v;
			stream_.p1(MAP);
			stream_.p4(a.size());
			Xfor2(key, value, a.pairs()){
				inner_dump(key);
				inner_dump(value);
			}
			return;
		}else if(cls.raweq(TClass<Fun>::get()) && v.object_name()==String("<TopLevel>")){
			CodeImpl* p = (CodeImpl*)((Fun*)&v)->code().impl();
			stream_.p1('X'); stream_.p1('T'); stream_.p1('A'); stream_.p1('L');
			stream_.p1(VERSION1); stream_.p1(VERSION2); stream_.p1(VERSION3); stream_.p1(VERSION4);
			
			int_t sz;

			sz = p->code_.size();
			stream_.p4(sz);
			for(int_t i=0; i<sz; ++i){
				stream_.p1(p->code_[i]);
			}
			
			sz = p->frame_core_table_.size();
			stream_.p4(sz);
			for(int_t i=0; i<sz; ++i){
				stream_.p1(p->frame_core_table_[i].kind);
				stream_.p2(p->frame_core_table_[i].variable_symbol_offset);
				stream_.p2(p->frame_core_table_[i].variable_size);
				stream_.p2(p->frame_core_table_[i].instance_variable_symbol_offset);
				stream_.p2(p->frame_core_table_[i].instance_variable_size);
				stream_.p2(p->frame_core_table_[i].line_number);			
			}

			sz = p->xfun_core_table_.size();
			stream_.p4(sz);
			for(int_t i=0; i<sz; ++i){
				stream_.p1(p->frame_core_table_[i].kind);
				stream_.p2(p->xfun_core_table_[i].variable_symbol_offset);
				stream_.p2(p->xfun_core_table_[i].variable_size);
				stream_.p2(p->xfun_core_table_[i].instance_variable_symbol_offset);
				stream_.p2(p->xfun_core_table_[i].instance_variable_size);
				stream_.p2(p->xfun_core_table_[i].line_number);			

				stream_.p2(p->xfun_core_table_[i].pc);			
				stream_.p2(p->xfun_core_table_[i].max_stack);			
				stream_.p1(p->xfun_core_table_[i].min_param_count);
				stream_.p1(p->xfun_core_table_[i].max_param_count);
				stream_.p1(p->xfun_core_table_[i].used_args_object);
			}

			sz = p->line_number_table_.size();
			stream_.p4(sz);
			for(int_t i=0; i<sz; ++i){
				stream_.p2(p->line_number_table_[i].start_pc);
				stream_.p2(p->line_number_table_[i].line_number);
			}

			Map map;
			map.set_at("source", p->source_file_name_);
			map.set_at("symbols", p->symbol_table_);
			map.set_at("values", p->value_table_);

			inner_dump(map);

			return;
		}

		ID id = v.get_class().object_name();
		num = register_dvalue(id, added);
		// 所属クラスにmarshal_dump関数が定義されている
		if(v.get_class().member(Xid(marshal_dump))){
			// 直列化する
			if(added){
				stream_.p1(VALUE2);
				inner_dump(id);
				inner_dump(v.send(Xid(marshal_dump)));					
			}else{
				stream_.p1(VALUE);
				stream_.p4(num); // クラスの名前を埋め込む
				inner_dump(v.send(Xid(marshal_dump)));
			}
		}else{
			// 名前だけを保存する
			if(added){
				stream_.p1(LIB2);
				inner_dump(id.send(Xid(marshal_dump)));
			}else{
				stream_.p1(LIB);
				stream_.p4(num);
			}
		}
	}else{
		// 既に保存されているオブジェクトなので参照位置だけ保存する
		stream_.p1(REF);
		stream_.p4(num);
	}
}

Any Marshal::inner_load(){
	int_t op = stream_.u1();
	switch(op){
		XTAL_NODEFAULT;

		XTAL_CASE(VALUE){
			int_t num = register_lvalue(null);
			Class c(cast<Class>(demangle(stream_.u4())));
			lvalues_.set_at(num, c.member(Xid(marshal_new))());
			lvalues_[num].send(Xid(marshal_load), inner_load());
			return lvalues_[num];
		}

		XTAL_CASE(VALUE2){
			int_t num = register_lvalue(null);
			Class c(cast<Class>(demangle(register_lvalue(inner_load()))));
			lvalues_.set_at(num, c.member(Xid(marshal_new))());
			lvalues_[num].send(Xid(marshal_load), inner_load());
			return lvalues_[num];
		}

		XTAL_CASE(LIB){
			int_t num = register_lvalue(null);
			lvalues_.set_at(num, demangle(stream_.u4()));
			return lvalues_[num];
		}

		XTAL_CASE(LIB2){
			int_t num = register_lvalue(null);
			lvalues_.set_at(num, demangle(register_lvalue(inner_load())));
			return lvalues_[num];
		}	

		XTAL_CASE(REF){
			return lvalues_[stream_.u4()];
		}

		XTAL_CASE(TNULL){
			return null;
		}

		XTAL_CASE(INT){
			return stream_.s4();
		}

		XTAL_CASE(FLOAT){
			union{ int_t i; float_t f; } u;
			u.i = stream_.s4();
			return u.f;
		}

		XTAL_CASE2(STRING, TID){
			int_t sz = stream_.s4();
			char* p = (char*)user_malloc(sz);
			for(int_t i = 0; i<sz; ++i){
				p[i] = stream_.s1();
			}
			if(op==TID){
				ID ret(p, sz);
				register_lvalue(ret);
				user_free(p, sz);
				return ret;
			}else{
				String ret(p, sz);
				register_lvalue(ret);
				user_free(p, sz);
				return ret;
			}
		}

		XTAL_CASE(ARRAY){
			int_t sz = stream_.s4();
			Array ret(sz);
			register_lvalue(ret);
			for(int_t i = 0; i<sz; ++i){
				ret.set_at(i, inner_load());
			}				
			return ret;
		}
		
		XTAL_CASE(MAP){
			int_t sz = stream_.s4();
			Map ret;
			register_lvalue(ret);
			Any key;
			for(int_t i = 0; i<sz; ++i){
				key = inner_load();
				ret.set_at(key, inner_load());
			}				
			return ret;
		}

		XTAL_CASE('X'){
			Code guard(null);
			CodeImpl* p = new(guard) CodeImpl();

			if(stream_.u1()!='T' || stream_.u1()!='A' || stream_.u1()!='L'){
				throw builtin().member("RuntimeError")(Xt("不正なコンパイル済みXtalファイルです"));
			}

			stream_.u1(); stream_.u1(); stream_.u1(); stream_.u1();
		
			int_t sz;

			sz = stream_.u4();
			p->code_.resize(sz);
			for(int_t i=0; i<sz; ++i){
				p->code_[i] = stream_.u1();
			}
			
			sz = stream_.u4();
			p->frame_core_table_.resize(sz);
			for(int_t i=0; i<sz; ++i){
				p->frame_core_table_[i].kind = stream_.u1();
				p->frame_core_table_[i].variable_symbol_offset = stream_.u2();
				p->frame_core_table_[i].variable_size = stream_.u2();
				p->frame_core_table_[i].instance_variable_symbol_offset = stream_.u2();
				p->frame_core_table_[i].instance_variable_size = stream_.u2();
				p->frame_core_table_[i].line_number = stream_.u2();			
			}

			sz = stream_.u4();
			p->xfun_core_table_.resize(sz);
			for(int_t i=0; i<sz; ++i){
				p->frame_core_table_[i].kind = stream_.u1();
				p->xfun_core_table_[i].variable_symbol_offset = stream_.u2();
				p->xfun_core_table_[i].variable_size = stream_.u2();
				p->xfun_core_table_[i].instance_variable_symbol_offset = stream_.u2();
				p->xfun_core_table_[i].instance_variable_size = stream_.u2();
				p->xfun_core_table_[i].line_number = stream_.u2();		

				p->xfun_core_table_[i].pc = stream_.u2();		
				p->xfun_core_table_[i].max_stack = stream_.u2();			
				p->xfun_core_table_[i].min_param_count = stream_.u1();
				p->xfun_core_table_[i].max_param_count = stream_.u1();
				p->xfun_core_table_[i].used_args_object = stream_.u1();
			}

			sz = stream_.u4();
			p->line_number_table_.resize(sz);
			for(int_t i=0; i<sz; ++i){
				p->line_number_table_[i].start_pc = stream_.u2();
				p->line_number_table_[i].line_number = stream_.u2();
			}

			Fun ret(null, null, guard, &p->xfun_core_table_[0]);
			register_lvalue(ret);

			Map map(cast<Map>(inner_load()));
			p->source_file_name_ = cast<String>(map.at("source"));
			p->symbol_table_ = cast<Array>(map.at("symbols"));
			p->value_table_ = cast<Array>(map.at("values"));
			
			ret.set_object_name("<TopLevel>", 1, null);	
			return ret;
		}
	}
	return null;
}

Any Marshal::demangle(int_t n){
	Any ret = lmap_.at(n);
	if(ret){ return ret; }
	Xfor(v, ((String&)lvalues_[n]).split("::")){
		ID id(((String&)v).intern());
		if(!ret){
			if(id.raweq(Xid(lib))){
				ret = lib();
				lmap_.set_at(n, ret);
			}
		}else{
			ret = ret.member(id);
			lmap_.set_at(n, ret);
		}
	}
	if(!ret){
		throw builtin().member("RuntimeError")(Xt("%sにアクセスできません。")(lvalues_[n]));
	}
	return ret;
}

int_t Marshal::register_dvalue(const Any& v, bool& added){
	Any ret = dmap_.at(v);
	if(ret){
		added = false;
	}else{
		ret = (int_t)dvalues_.size();
		dmap_.set_at(v, ret);
		dvalues_.push_back(v);
		added = true;
	}
	return ret.ivalue();
}

int_t Marshal::register_lvalue(const Any& v){
	lvalues_.push_back(v);
	return lvalues_.size()-1;
}

}
