
#include "xtal.h"
#include "xtal_marshal.h"
#include "xtal_any.h"
#include "xtal_marshal.h"
#include "xtal_array.h"
#include "xtal_frame.h"
#include "xtal_cast.h"
#include "xtal_macro.h"
#include "xtal_map.h"
#include "xtal_codeimpl.h"
#include "xtal_fun.h"

namespace xtal{

enum{
	MARSHAL_VERSION1 = 1,
	MARSHAL_VERSION2 = 0,
};

Marshal::Marshal(const Stream& s)
	:stream_(s){
}

void Marshal::dump(const Any& v){
	inner_dump(v);
}

Any Marshal::load(){
	return inner_load();
}

bool Marshal::check_id(const ID& id){
	const char_t* str = id.c_str();
	if(str[0]=='l' && str[1]=='i' && str[2]=='b' && str[3]==':'){
		return true;
	}
	XTAL_THROW(builtin().member("RuntimeError")(Xt("Xtal Runtime Error 1008")));
	return false;
}

void Marshal::to_script(const Any& v){

	/*
	Stream temp(stream_);
	stream_ = MemoryStream();
	*/

	stream_.put_s("export ");
	inner_to_script(v, 0);
	stream_.put_s(";\n");

	/*
	stream_.swap(temp);

	stream_.write("values: ");
	inner_to_script(dvalues_, 0);
	stream_.write(";\n");
	*/
}

void Marshal::inner_dump(const Any& v){
	if(!v){
		stream_.put_i8(TNULL);
		return;
	}

	const Class& cls = v.get_class();
	if(cls.raweq(TClass<Int>::get())){
		stream_.put_i8(INT);
		stream_.put_i32(v.ivalue());
		return;
	}else if(cls.raweq(TClass<Float>::get())){
		stream_.put_i8(FLOAT);
		stream_.put_f32(v.fvalue());
		return;
	}else if(cls.raweq(TClass<True>::get())){
		stream_.put_i8(TTRUE);
		return;
	}else if(cls.raweq(TClass<False>::get())){
		stream_.put_i8(TFALSE);
		return;
	}

	bool added = false;
	int_t num = register_dvalue(v, added);
	if(added){

		if(cls.raweq(TClass<Array>::get())){
			const Array& a = (const Array&)v;
			stream_.put_i8(ARRAY);
			stream_.put_i32(a.size());
			for(int_t i=0; i<a.size(); ++i){
				inner_dump(a.at(i));
			}
			return;
		}else if(cls.raweq(TClass<String>::get())){
			const String& a = (const String&)v;
			if(a.is_interned()){
				stream_.put_i8(TID);
			}else{
				stream_.put_i8(STRING);
			}
			stream_.put_i32(a.size());
			for(size_t i=0; i<a.size(); ++i){
				stream_.put_i8(a.c_str()[i]);
			}
			return;
		}else if(cls.raweq(TClass<Map>::get())){
			const Map& a = (const Map&)v;
			stream_.put_i8(MAP);
			stream_.put_i32(a.size());
			Xfor2(key, value, a.each_pair()){
				inner_dump(key);
				inner_dump(value);
			}
			return;
		}else if(cls.raweq(TClass<Fun>::get()) && v.object_name()==String("<TopLevel>")){
			CodeImpl* p = (CodeImpl*)((Fun*)&v)->code().impl();
			stream_.put_i8('x'); stream_.put_i8('t'); stream_.put_i8('a'); stream_.put_i8('l');
			stream_.put_i8(MARSHAL_VERSION1); stream_.put_i8(MARSHAL_VERSION2); 
			stream_.put_i8(0); 
			stream_.put_i8(0);
			
			int_t sz;

			sz = p->code_.size();
			stream_.put_i32(sz);
			for(int_t i=0; i<sz; ++i){
				stream_.put_i8(p->code_[i]);
			}
			
			sz = p->frame_core_table_.size();
			stream_.put_i32(sz);
			for(int_t i=0; i<sz; ++i){
				stream_.put_i8(p->frame_core_table_[i].kind);
				stream_.put_u16(p->frame_core_table_[i].variable_symbol_offset);
				stream_.put_u16(p->frame_core_table_[i].variable_size);
				stream_.put_u16(p->frame_core_table_[i].instance_variable_symbol_offset);
				stream_.put_u16(p->frame_core_table_[i].instance_variable_size);
				stream_.put_u16(p->frame_core_table_[i].line_number);			
			}

			sz = p->xfun_core_table_.size();
			stream_.put_i32(sz);
			for(int_t i=0; i<sz; ++i){
				stream_.put_i8(p->frame_core_table_[i].kind);
				stream_.put_u16(p->xfun_core_table_[i].variable_symbol_offset);
				stream_.put_u16(p->xfun_core_table_[i].variable_size);
				stream_.put_u16(p->xfun_core_table_[i].instance_variable_symbol_offset);
				stream_.put_u16(p->xfun_core_table_[i].instance_variable_size);
				stream_.put_u16(p->xfun_core_table_[i].line_number);			

				stream_.put_u16(p->xfun_core_table_[i].pc);			
				stream_.put_u16(p->xfun_core_table_[i].max_stack);			
				stream_.put_i8(p->xfun_core_table_[i].min_param_count);
				stream_.put_i8(p->xfun_core_table_[i].max_param_count);
				stream_.put_i8(p->xfun_core_table_[i].used_args_object);
			}

			sz = p->line_number_table_.size();
			stream_.put_i32(sz);
			for(int_t i=0; i<sz; ++i){
				stream_.put_u16(p->line_number_table_[i].start_pc);
				stream_.put_u16(p->line_number_table_[i].line_number);
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
			stream_.put_i8(VALUE);
			check_id(id);
			inner_dump(id); // クラスの名前を埋め込む
			inner_dump(v.send(Xid(marshal_dump)));					
		}else{
			ID id = v.object_name();
			check_id(id);
			stream_.put_i8(LIB);
			inner_dump(id);
		}
	}else{
		// 既に保存されているオブジェクトなので参照位置だけ保存する
		stream_.put_i8(REF);
		stream_.put_i32(num);
	}
}

Any Marshal::inner_load(){
	int_t op = stream_.get_u8();
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
			return lvalues_[stream_.get_u32()];
		}

		XTAL_CASE(TNULL){
			return null;
		}

		XTAL_CASE(INT){
			return stream_.get_i32();
		}

		XTAL_CASE(FLOAT){
			return stream_.get_f32();
		}

		XTAL_CASE2(STRING, TID){
			int_t sz = stream_.get_i32();
			char* p = (char*)user_malloc(sz);
			for(int_t i = 0; i<sz; ++i){
				p[i] = stream_.get_i8();
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
			int_t sz = stream_.get_i32();
			Array ret(sz);
			register_lvalue(ret);
			for(int_t i = 0; i<sz; ++i){
				ret.set_at(i, inner_load());
			}				
			return ret;
		}
		
		XTAL_CASE(MAP){
			int_t sz = stream_.get_i32();
			Map ret;
			register_lvalue(ret);
			Any key;
			for(int_t i = 0; i<sz; ++i){
				key = inner_load();
				ret.set_at(key, inner_load());
			}				
			return ret;
		}
		
		XTAL_CASE(TTRUE){
			return true;
		}
		
		XTAL_CASE(TFALSE){
			return false;
		}

		XTAL_CASE('x'){
			Code guard(null);
			CodeImpl* p = new(guard) CodeImpl();

			if(stream_.get_u8()!='t' || stream_.get_u8()!='a' || stream_.get_u8()!='l'){
				XTAL_THROW(builtin().member("RuntimeError")(Xt("Xtal Runtime Error 1009")));
			}

			xtal::u8 version1 = stream_.get_u8(), version2 = stream_.get_u8();
			if(version1!=MARSHAL_VERSION1 || version2!=MARSHAL_VERSION2){
				XTAL_THROW(builtin().member("RuntimeError")(Xt("Xtal Runtime Error 1009")));
			}
			
			stream_.get_u8();
			stream_.get_u8();
		
			int_t sz;

			sz = stream_.get_u32();
			p->code_.resize(sz);
			for(int_t i=0; i<sz; ++i){
				p->code_[i] = stream_.get_u8();
			}
			
			sz = stream_.get_u32();
			p->frame_core_table_.resize(sz);
			for(int_t i=0; i<sz; ++i){
				p->frame_core_table_[i].kind = stream_.get_u8();
				p->frame_core_table_[i].variable_symbol_offset = stream_.get_u16();
				p->frame_core_table_[i].variable_size = stream_.get_u16();
				p->frame_core_table_[i].instance_variable_symbol_offset = stream_.get_u16();
				p->frame_core_table_[i].instance_variable_size = stream_.get_u16();
				p->frame_core_table_[i].line_number = stream_.get_u16();			
			}

			sz = stream_.get_u32();
			p->xfun_core_table_.resize(sz);
			for(int_t i=0; i<sz; ++i){
				p->frame_core_table_[i].kind = stream_.get_u8();
				p->xfun_core_table_[i].variable_symbol_offset = stream_.get_u16();
				p->xfun_core_table_[i].variable_size = stream_.get_u16();
				p->xfun_core_table_[i].instance_variable_symbol_offset = stream_.get_u16();
				p->xfun_core_table_[i].instance_variable_size = stream_.get_u16();
				p->xfun_core_table_[i].line_number = stream_.get_u16();		

				p->xfun_core_table_[i].pc = stream_.get_u16();		
				p->xfun_core_table_[i].max_stack = stream_.get_u16();			
				p->xfun_core_table_[i].min_param_count = stream_.get_u8();
				p->xfun_core_table_[i].max_param_count = stream_.get_u8();
				p->xfun_core_table_[i].used_args_object = stream_.get_u8();
			}

			sz = stream_.get_u32();
			p->line_number_table_.resize(sz);
			for(int_t i=0; i<sz; ++i){
				p->line_number_table_[i].start_pc = stream_.get_u16();
				p->line_number_table_[i].line_number = stream_.get_u16();
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
		XTAL_THROW(builtin().member("RuntimeError")(Xt("Xtal Runtime Error 1008")(Named("name", lvalues_[n]))));
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
		stream_.put_s(v.to_s());
		return;
	}else if(cls.raweq(TClass<Float>::get())){
		stream_.put_s(v.to_s());
		return;
	}else if(cls.raweq(TClass<True>::get())){
		stream_.put_s(Xid(true));
		return;
	}else if(cls.raweq(TClass<False>::get())){
		stream_.put_s(Xid(false));
		return;
	}

	bool added = false;
	int_t num = register_dvalue(v, added);
	/*if(added)*/{
		if(cls.raweq(TClass<Array>::get())){
			const Array& a = (const Array&)v;
			if(a.empty()){
				stream_.put_s("[]");
			}else{
				stream_.put_s("[\n");
				tab++;
				for(int_t i=0; i<a.size(); ++i){
					put_tab(tab);
					inner_to_script(a.at(i), tab);
					stream_.put_s(",\n");
				}
				tab--;
				put_tab(tab);
				stream_.put_s("]");
			}
			return;
		}else if(cls.raweq(TClass<String>::get())){
			const String& a = (const String&)v;
			stream_.put_i8('"');
			for(size_t i=0; i<a.size(); ++i){
				u8 ch = a.c_str()[i];
				switch(ch){
					XTAL_DEFAULT{ stream_.put_i8(ch); }
					XTAL_CASE('"'){ stream_.put_i8('\\'); stream_.put_i8('"'); }
					XTAL_CASE('\n'){ stream_.put_i8('\\'); stream_.put_i8('n'); }
					XTAL_CASE('\t'){ stream_.put_i8('\\'); stream_.put_i8('t'); }
					XTAL_CASE('\a'){ stream_.put_i8('\\'); stream_.put_i8('a'); }
					XTAL_CASE('\b'){ stream_.put_i8('\\'); stream_.put_i8('b'); }
					XTAL_CASE('\r'){ stream_.put_i8('\\'); stream_.put_i8('r'); }
					XTAL_CASE('\f'){ stream_.put_i8('\\'); stream_.put_i8('f'); }
					XTAL_CASE('\v'){ stream_.put_i8('\\'); stream_.put_i8('v'); }
				}

			}
			stream_.put_i8('"');
			return;
		}else if(cls.raweq(TClass<Map>::get())){
			const Map& a = (const Map&)v;
			if(a.empty()){
				stream_.put_s("[:]");
			}else{
				stream_.put_s("[\n");
				tab++;
				Xfor2(key, value, a.each_pair()){
					put_tab(tab);
					inner_to_script(key, tab);
					stream_.put_s(": ");
					inner_to_script(value, tab);
					stream_.put_s(",\n");
				}
				tab--;
				put_tab(tab);
				stream_.put_s("]");
			}
			return;
		}

		// 所属クラスにmarshal_dump関数が定義されている
		if(v.get_class().member(Xid(marshal_dump))){
			ID id = v.get_class().object_name();
			check_id(id);
			stream_.put_s(id);
			stream_.put_s(".marshal_new(");
			inner_to_script(v.send(Xid(marshal_dump)), tab);
			stream_.put_s(")");
		}else{
			ID id = v.object_name();
			check_id(id);
			stream_.put_s(id);
		}
	}/*else{
		// 既に保存されているオブジェクトなので参照位置だけ保存する
		stream_.write("values[");
		stream_.write(Any(num).to_s());
		stream_.write("]");
	}*/
}

void Marshal::put_tab(int_t tab){
	while(tab--)
		stream_.put_i8('\t');
}


}
