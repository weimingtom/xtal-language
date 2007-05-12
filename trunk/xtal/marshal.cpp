
#include "xtal.h"
#include "marshal.h"
#include "any.h"
#include "marshal.h"
#include "array.h"
#include "frame.h"
#include "cast.h"
#include "xmacro.h"
#include "map.h"
#include "codeimpl.h"
#include "fun.h"

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

void Marshal::to_script(const Any& v){
	stream_.write("export ");
	inner_to_script(v, 0);
	stream_.write(";\n");
}

void Marshal::inner_dump(const Any& v){
	if(!v){
		stream_.p8(TNULL);
		return;
	}

	const Class& cls = v.get_class();
	if(cls.raweq(TClass<Int>::get())){
		stream_.p8(INT);
		stream_.p32(v.ivalue());
		return;
	}else if(cls.raweq(TClass<Float>::get())){
		stream_.p8(FLOAT);
		union{ int i; float f; } u;
		u.f = v.fvalue();
		stream_.p32(u.i);
		return;
	}

	bool added = false;
	int_t num = register_dvalue(v, added);
	if(added){

		if(cls.raweq(TClass<Array>::get())){
			const Array& a = (const Array&)v;
			stream_.p8(ARRAY);
			stream_.p32(a.size());
			for(int_t i=0; i<a.size(); ++i){
				inner_dump(a.at(i));
			}
			return;
		}else if(cls.raweq(TClass<String>::get())){
			const String& a = (const String&)v;
			if(a.is_interned()){
				stream_.p8(TID);
			}else{
				stream_.p8(STRING);
			}
			stream_.p32(a.size());
			for(int_t i=0; i<a.size(); ++i){
				stream_.p8(a.c_str()[i]);
			}
			return;
		}else if(cls.raweq(TClass<Map>::get())){
			const Map& a = (const Map&)v;
			stream_.p8(MAP);
			stream_.p32(a.size());
			Xfor2(key, value, a.pairs()){
				inner_dump(key);
				inner_dump(value);
			}
			return;
		}else if(cls.raweq(TClass<Fun>::get()) && v.object_name()==String("<TopLevel>")){
			CodeImpl* p = (CodeImpl*)((Fun*)&v)->code().impl();
			stream_.p8('X'); stream_.p8('T'); stream_.p8('A'); stream_.p8('L');
			stream_.p8(VERSION1); stream_.p8(VERSION2); stream_.p8(VERSION3); stream_.p8(VERSION4);
			
			int_t sz;

			sz = p->code_.size();
			stream_.p32(sz);
			for(int_t i=0; i<sz; ++i){
				stream_.p8(p->code_[i]);
			}
			
			sz = p->frame_core_table_.size();
			stream_.p32(sz);
			for(int_t i=0; i<sz; ++i){
				stream_.p8(p->frame_core_table_[i].kind);
				stream_.p16(p->frame_core_table_[i].variable_symbol_offset);
				stream_.p16(p->frame_core_table_[i].variable_size);
				stream_.p16(p->frame_core_table_[i].instance_variable_symbol_offset);
				stream_.p16(p->frame_core_table_[i].instance_variable_size);
				stream_.p16(p->frame_core_table_[i].line_number);			
			}

			sz = p->xfun_core_table_.size();
			stream_.p32(sz);
			for(int_t i=0; i<sz; ++i){
				stream_.p8(p->frame_core_table_[i].kind);
				stream_.p16(p->xfun_core_table_[i].variable_symbol_offset);
				stream_.p16(p->xfun_core_table_[i].variable_size);
				stream_.p16(p->xfun_core_table_[i].instance_variable_symbol_offset);
				stream_.p16(p->xfun_core_table_[i].instance_variable_size);
				stream_.p16(p->xfun_core_table_[i].line_number);			

				stream_.p16(p->xfun_core_table_[i].pc);			
				stream_.p16(p->xfun_core_table_[i].max_stack);			
				stream_.p8(p->xfun_core_table_[i].min_param_count);
				stream_.p8(p->xfun_core_table_[i].max_param_count);
				stream_.p8(p->xfun_core_table_[i].used_args_object);
			}

			sz = p->line_number_table_.size();
			stream_.p32(sz);
			for(int_t i=0; i<sz; ++i){
				stream_.p16(p->line_number_table_[i].start_pc);
				stream_.p16(p->line_number_table_[i].line_number);
			}

			Map map;
			map.set_at("source", p->source_file_name_);
			map.set_at("symbols", p->symbol_table_);
			map.set_at("values", p->value_table_);

			inner_dump(map);

			return;
		}

		// 所属クラスにmarshal_dump関数が定義されている
		if(v.get_class().member(Xid(marshal_dump))){
			ID id = v.get_class().object_name();
			stream_.p8(VALUE);
			inner_dump(id); // クラスの名前を埋め込む
			inner_dump(v.send(Xid(marshal_dump)));					
		}else{
			ID id = v.object_name();
			stream_.p8(LIB);
			inner_dump(id);
		}
	}else{
		// 既に保存されているオブジェクトなので参照位置だけ保存する
		stream_.p8(REF);
		stream_.p32(num);
	}
}

Any Marshal::inner_load(){
	int_t op = stream_.u8();
	switch(op){
		XTAL_NODEFAULT;

		XTAL_CASE(VALUE){
			int_t num = register_lvalue(null);
			Class c(cast<Class>(demangle(register_lvalue(inner_load()))));
			const VMachine& vm = vmachine();
			vm.setup_call(1, inner_load());
			c.marshal_new(vm);
			lvalues_.set_at(num, vm.result());
			vm.cleanup_call();
			return lvalues_[num];
		}

		XTAL_CASE(LIB){
			int_t num = register_lvalue(null);
			lvalues_.set_at(num, demangle(register_lvalue(inner_load())));
			return lvalues_[num];
		}	

		XTAL_CASE(REF){
			return lvalues_[stream_.u32()];
		}

		XTAL_CASE(TNULL){
			return null;
		}

		XTAL_CASE(INT){
			return stream_.s32();
		}

		XTAL_CASE(FLOAT){
			union{ int i; float f; } u;
			u.i = stream_.s32();
			return u.f;
		}

		XTAL_CASE2(STRING, TID){
			int_t sz = stream_.s32();
			char* p = (char*)user_malloc(sz);
			for(int_t i = 0; i<sz; ++i){
				p[i] = stream_.s8();
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
			int_t sz = stream_.s32();
			Array ret(sz);
			register_lvalue(ret);
			for(int_t i = 0; i<sz; ++i){
				ret.set_at(i, inner_load());
			}				
			return ret;
		}
		
		XTAL_CASE(MAP){
			int_t sz = stream_.s32();
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

			if(stream_.u8()!='T' || stream_.u8()!='A' || stream_.u8()!='L'){
				throw builtin().member("RuntimeError")(Xt("Xtal Runtime Error 1009"));
			}

			stream_.u8(); stream_.u8(); stream_.u8(); stream_.u8();
		
			int_t sz;

			sz = stream_.u32();
			p->code_.resize(sz);
			for(int_t i=0; i<sz; ++i){
				p->code_[i] = stream_.u8();
			}
			
			sz = stream_.u32();
			p->frame_core_table_.resize(sz);
			for(int_t i=0; i<sz; ++i){
				p->frame_core_table_[i].kind = stream_.u8();
				p->frame_core_table_[i].variable_symbol_offset = stream_.u16();
				p->frame_core_table_[i].variable_size = stream_.u16();
				p->frame_core_table_[i].instance_variable_symbol_offset = stream_.u16();
				p->frame_core_table_[i].instance_variable_size = stream_.u16();
				p->frame_core_table_[i].line_number = stream_.u16();			
			}

			sz = stream_.u32();
			p->xfun_core_table_.resize(sz);
			for(int_t i=0; i<sz; ++i){
				p->frame_core_table_[i].kind = stream_.u8();
				p->xfun_core_table_[i].variable_symbol_offset = stream_.u16();
				p->xfun_core_table_[i].variable_size = stream_.u16();
				p->xfun_core_table_[i].instance_variable_symbol_offset = stream_.u16();
				p->xfun_core_table_[i].instance_variable_size = stream_.u16();
				p->xfun_core_table_[i].line_number = stream_.u16();		

				p->xfun_core_table_[i].pc = stream_.u16();		
				p->xfun_core_table_[i].max_stack = stream_.u16();			
				p->xfun_core_table_[i].min_param_count = stream_.u8();
				p->xfun_core_table_[i].max_param_count = stream_.u8();
				p->xfun_core_table_[i].used_args_object = stream_.u8();
			}

			sz = stream_.u32();
			p->line_number_table_.resize(sz);
			for(int_t i=0; i<sz; ++i){
				p->line_number_table_[i].start_pc = stream_.u16();
				p->line_number_table_[i].line_number = stream_.u16();
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
		throw builtin().member("RuntimeError")(Xt("Xtal Runtime Error 1008")(Xid(name)=lvalues_[n]));
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

void Marshal::inner_to_script(const Any& v, int_t tab){
	if(!v){
		stream_.write("null", 4);
		return;
	}

	const Class& cls = v.get_class();
	if(cls.raweq(TClass<Int>::get())){
		stream_.write(v.to_s());
		return;
	}else if(cls.raweq(TClass<Float>::get())){
		stream_.write(v.to_s());
		return;
	}

	bool added = false;
	int_t num = register_dvalue(v, added);
	/*if(added)*/{
		if(cls.raweq(TClass<Array>::get())){
			const Array& a = (const Array&)v;
			if(a.empty()){
				stream_.write("[]");
			}else{
				stream_.write("[\n");
				tab++;
				for(int_t i=0; i<a.size(); ++i){
					put_tab(tab);
					inner_to_script(a.at(i), tab);
					stream_.write(",\n");
				}
				tab--;
				put_tab(tab);
				stream_.write("]");
			}
			return;
		}else if(cls.raweq(TClass<String>::get())){
			const String& a = (const String&)v;
			stream_.p8('"');
			for(int_t i=0; i<a.size(); ++i){
				u8 ch = a.c_str()[i];
				switch(ch){
					XTAL_DEFAULT{ stream_.p8(ch); }
					XTAL_CASE('"'){ stream_.p8('\\'); stream_.p8('"'); }
					XTAL_CASE('\n'){ stream_.p8('\\'); stream_.p8('n'); }
					XTAL_CASE('\t'){ stream_.p8('\\'); stream_.p8('t'); }
					XTAL_CASE('\a'){ stream_.p8('\\'); stream_.p8('a'); }
					XTAL_CASE('\b'){ stream_.p8('\\'); stream_.p8('b'); }
					XTAL_CASE('\r'){ stream_.p8('\\'); stream_.p8('r'); }
					XTAL_CASE('\f'){ stream_.p8('\\'); stream_.p8('f'); }
					XTAL_CASE('\v'){ stream_.p8('\\'); stream_.p8('v'); }
				}

			}
			stream_.p8('"');
			return;
		}else if(cls.raweq(TClass<Map>::get())){
			const Map& a = (const Map&)v;
			if(a.empty()){
				stream_.write("[:]");
			}else{
				stream_.write("[\n");
				tab++;
				Xfor2(key, value, a.pairs()){
					put_tab(tab);
					inner_to_script(key, tab);
					stream_.write(": ");
					inner_to_script(value, tab);
					stream_.write(",\n");
				}
				tab--;
				put_tab(tab);
				stream_.write("]");
			}
			return;
		}

		// 所属クラスにmarshal_dump関数が定義されている
		if(v.get_class().member(Xid(marshal_dump))){
			ID id = v.get_class().object_name();
			// クラスの名前を埋め込む
			for(int_t i=0; i<id.size(); ++i){
				stream_.p8(id.c_str()[i]);
			}
			stream_.write("::marshal_new(");
			inner_to_script(v.send(Xid(marshal_dump)), tab);
			stream_.write(")");
		}else{
			ID id = v.object_name();
			for(int_t i=0; i<id.size(); ++i){
				stream_.p8(id.c_str()[i]);
			}
		}
	}/*else{
		// 既に保存されているオブジェクトなので参照位置だけ保存する
		stream_.write("value[");
		stream_.write(Any(num).to_s());
		stream_.write("]");
	}*/
}

void Marshal::put_tab(int_t tab){
	while(tab--)
		stream_.p8('\t');
}


}
