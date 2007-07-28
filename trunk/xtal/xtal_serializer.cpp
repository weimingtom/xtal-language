
#include "xtal.h"
#include "xtal_serializer.h"
#include "xtal_any.h"
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

Serializer::Serializer(const Stream& s)
	:stream_(s){
}

void Serializer::serialize(const Any& v){
	clear();
	inner_serialize(v);
}

Any Serializer::deserialize(){
	clear();
	return inner_deserialize();
}

void Serializer::xtalize(const Any& v){
	clear();
	stream_.put_s("v: [:];\n");
	stream_.put_s("d: fun(n, m){ v[n] = m; return m; }\nexport ");
	inner_xtalize(v, 0);
	stream_.put_s(";\n");
}

bool Serializer::check_id(const ID& id){
	const char_t* str = id.c_str();
	if(str[0]=='l' && str[1]=='i' && str[2]=='b' && str[3]==':'){
		return true;
	}
	XTAL_THROW(builtin().member("RuntimeError")(Xt("Xtal Runtime Error 1008")(Named("name", id))));
	return false;
}

void Serializer::inner_serialize(const Any& v){
	
	switch(v.type()){
		XTAL_NODEFAULT;

		XTAL_CASE(TYPE_NULL){
			stream_.put_u8(TNULL);
			return;
		}

		XTAL_CASE(TYPE_BASE){}

		XTAL_CASE(TYPE_INT){
			stream_.put_u8(TINT);
			stream_.put_i32(v.ivalue());
			return;
		}

		XTAL_CASE(TYPE_FLOAT){
			stream_.put_u8(TFLOAT);
			stream_.put_f32(v.fvalue());
			return;
		}

		XTAL_CASE(TYPE_FALSE){
			stream_.put_u8(TFALSE);
			return;
		}

		XTAL_CASE(TYPE_TRUE){
			stream_.put_u8(TTRUE);
			return;
		}

		XTAL_CASE(TYPE_NOP){
			stream_.put_u8(TNOP);
			return;
		}
	}

	const Class& cls = v.get_class();
	bool added = false;
	int_t num = register_value(v, added);
	if(added){

		if(cls.raweq(TClass<Array>::get())){
			const Array& a = (const Array&)v;
			stream_.put_u8(TARRAY);
			stream_.put_u32(a.size());
			for(int_t i=0; i<a.size(); ++i){
				inner_serialize(a.at(i));
			}
			return;
		}else if(cls.raweq(TClass<String>::get())){
			const String& a = (const String&)v;
			if(a.is_interned()){
				stream_.put_u8(TID);
			}else{
				stream_.put_u8(TSTRING);
			}
			stream_.put_i32(a.size());
			for(size_t i=0; i<a.size(); ++i){
				stream_.put_u8(a.c_str()[i]);
			}
			return;
		}else if(cls.raweq(TClass<Map>::get())){
			const Map& a = (const Map&)v;
			stream_.put_u8(TMAP);
			stream_.put_u32(a.size());
			Xfor2(key, value, a.each_pair()){
				inner_serialize(key);
				inner_serialize(value);
			}
			return;
		}else if(cls.raweq(TClass<Code>::get())){
			CodeImpl* p = (CodeImpl*)((Fun*)&v)->code().impl();
			stream_.put_u8('x'); stream_.put_u8('t'); stream_.put_u8('a'); stream_.put_u8('l');
			stream_.put_u8(MARSHAL_VERSION1); stream_.put_u8(MARSHAL_VERSION2); 
			stream_.put_u8(0); 
			stream_.put_u8(0);
			
			int_t sz;

			sz = p->code_.size();
			stream_.put_i32(sz);
			for(int_t i=0; i<sz; ++i){
				stream_.put_i8(p->code_[i]);
			}
			
			sz = p->block_core_table_.size();
			stream_.put_u32(sz);
			for(int_t i=0; i<sz; ++i){
				stream_.put_u16(p->block_core_table_[i].line_number);			
				stream_.put_u16(p->block_core_table_[i].variable_symbol_offset);
				stream_.put_u16(p->block_core_table_[i].variable_size);
			}

			sz = p->class_core_table_.size();
			stream_.put_u32(sz);
			for(int_t i=0; i<sz; ++i){
				stream_.put_u16(p->class_core_table_[i].line_number);			
				stream_.put_u16(p->class_core_table_[i].variable_symbol_offset);
				stream_.put_u16(p->class_core_table_[i].variable_size);

				stream_.put_u16(p->class_core_table_[i].instance_variable_symbol_offset);
				stream_.put_u16(p->class_core_table_[i].instance_variable_size);			
			}

			sz = p->xfun_core_table_.size();
			stream_.put_u32(sz);
			for(int_t i=0; i<sz; ++i){
				stream_.put_u16(p->xfun_core_table_[i].line_number);			
				stream_.put_u16(p->xfun_core_table_[i].variable_symbol_offset);
				stream_.put_u16(p->xfun_core_table_[i].variable_size);		

				stream_.put_u16(p->xfun_core_table_[i].pc);			
				stream_.put_u16(p->xfun_core_table_[i].max_stack);			
				stream_.put_u8(p->xfun_core_table_[i].min_param_count);
				stream_.put_u8(p->xfun_core_table_[i].max_param_count);
				stream_.put_u8(p->xfun_core_table_[i].used_args_object);
				stream_.put_u8(p->xfun_core_table_[i].on_heap);
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

			inner_serialize(map);

			return;
		}

		// 所属クラスにserial_save関数が定義されている
		if(v.get_class().member(Xid(serial_save))){
			ID id = v.get_class().object_name();

			// serial_newで空オブジェクトを生成するコマンドを埋め込む
			stream_.put_u8(SERIAL_NEW);
			append_value(null);
			check_id(id);
			inner_serialize(id); // クラスの名前を埋め込む

			// serial_saveでserializableなオブジェクトを取り出しserializeする
			inner_serialize(v.send(Xid(serial_save)));

		}else{
			ID id = v.object_name();
			check_id(id);
			stream_.put_u8(LIB);
			inner_serialize(id);
		}
	}else{
		// 既に保存されているオブジェクトなので参照位置だけ保存する
		stream_.put_u8(REF);
		stream_.put_u32(num);
	}
}

Any Serializer::inner_deserialize(){
	int_t op = stream_.get_u8();
	switch(op){
		XTAL_NODEFAULT;

		XTAL_CASE(SERIAL_NEW){
			int_t num = append_value(null);

			// serial_newをするクラスを取り出す
			Class c(cast<Class>(demangle(append_value(inner_deserialize()))));

			const VMachine& vm = vmachine();

			// serial_newを呼び出して、保存しておく
			vm.setup_call(1);
			c.s_new(vm);
			Any ret = vm.result();
			values_.set_at(num, ret);
			vm.cleanup_call();

			vm.setup_call(0, inner_deserialize());
			ret.rawsend(vm, Xid(serial_load));
			vm.cleanup_call();

			return ret;
		}

		XTAL_CASE(LIB){
			int_t num = append_value(null);
			values_.set_at(num, demangle(append_value(inner_deserialize())));
			return values_[num];
		}	

		XTAL_CASE(REF){
			return values_[stream_.get_u32()];
		}

		XTAL_CASE(TNULL){
			return null;
		}
		
		XTAL_CASE(TNOP){
			return nop;
		}

		XTAL_CASE(TINT){
			return stream_.get_i32();
		}

		XTAL_CASE(TFLOAT){
			return stream_.get_f32();
		}

		XTAL_CASE2(TSTRING, TID){
			int_t sz = stream_.get_u32();
			char* p = (char*)user_malloc(sz+1);
			for(int_t i = 0; i<sz; ++i){
				p[i] = (char_t)stream_.get_u8();
			}
			p[sz] = 0;
			if(op==TID){
				ID ret(p, sz);
				append_value(ret);
				user_free(p, sz);
				return ret;
			}else{
				String ret(p, sz, sz+1, String::delegate_memory_t());
				append_value(ret);
				return ret;
			}
		}

		XTAL_CASE(TARRAY){
			int_t sz = stream_.get_u32();
			Array ret(sz);
			append_value(ret);
			for(int_t i = 0; i<sz; ++i){
				ret.set_at(i, inner_deserialize());
			}				
			return ret;
		}
		
		XTAL_CASE(TMAP){
			int_t sz = stream_.get_u32();
			Map ret;
			append_value(ret);
			Any key;
			for(int_t i = 0; i<sz; ++i){
				key = inner_deserialize();
				ret.set_at(key, inner_deserialize());
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
			
			sz = stream_.get_u16();
			p->block_core_table_.resize(sz);
			for(int_t i=0; i<sz; ++i){
				p->block_core_table_[i].line_number = stream_.get_u16();			
				p->block_core_table_[i].variable_symbol_offset = stream_.get_u16();
				p->block_core_table_[i].variable_size = stream_.get_u16();
			}

			sz = stream_.get_u16();
			p->class_core_table_.resize(sz);
			for(int_t i=0; i<sz; ++i){
				p->class_core_table_[i].line_number = stream_.get_u16();			
				p->class_core_table_[i].variable_symbol_offset = stream_.get_u16();
				p->class_core_table_[i].variable_size = stream_.get_u16();

				p->class_core_table_[i].instance_variable_symbol_offset = stream_.get_u16();
				p->class_core_table_[i].instance_variable_size = stream_.get_u16();
			}

			sz = stream_.get_u16();
			p->xfun_core_table_.resize(sz);
			for(int_t i=0; i<sz; ++i){
				p->xfun_core_table_[i].line_number = stream_.get_u16();			
				p->xfun_core_table_[i].variable_symbol_offset = stream_.get_u16();
				p->xfun_core_table_[i].variable_size = stream_.get_u16();

				p->xfun_core_table_[i].pc = stream_.get_u16();		
				p->xfun_core_table_[i].max_stack = stream_.get_u16();			
				p->xfun_core_table_[i].min_param_count = stream_.get_u8();
				p->xfun_core_table_[i].max_param_count = stream_.get_u8();
				p->xfun_core_table_[i].used_args_object = stream_.get_u8();
				p->xfun_core_table_[i].on_heap = stream_.get_u8();
			}

			sz = stream_.get_u32();
			p->line_number_table_.resize(sz);
			for(int_t i=0; i<sz; ++i){
				p->line_number_table_[i].start_pc = stream_.get_u16();
				p->line_number_table_[i].line_number = stream_.get_u16();
			}

			Fun ret(null, null, guard, &p->xfun_core_table_[0]);
			append_value(ret);

			Map map(cast<Map>(inner_deserialize()));
			p->source_file_name_ = cast<String>(map.at("source"));
			p->symbol_table_ = cast<Array>(map.at("symbols"));
			p->value_table_ = cast<Array>(map.at("values"));
			
			ret.set_object_name("<TopLevel>", 1, null);	
			return ret;
		}
	}
	return null;
}

Any Serializer::demangle(int_t n){
	Any ret = map_.at(n);
	if(ret){ return ret; }
	Xfor(v, ((String&)values_[n]).split("::")){
		ID id(((String&)v).intern());
		if(!ret){
			if(id.raweq(Xid(lib))){
				ret = lib();
				map_.set_at(n, ret);
			}
		}else{
			ret = ret.member(id);
			map_.set_at(n, ret);
		}
	}
	if(!ret){
		XTAL_THROW(builtin().member("RuntimeError")(Xt("Xtal Runtime Error 1008")(Named("name", values_[n]))));
	}
	return ret;
}

int_t Serializer::register_value(const Any& v, bool& added){
	Any ret = map_.at(v);
	if(ret){
		added = false;
	}else{
		ret = append_value(v);
		added = true;
	}
	return ret.ivalue();
}

int_t Serializer::append_value(const Any& v){
	int_t ret = (int_t)values_.size();
	map_.set_at(v, ret);
	values_.push_back(v);
	return ret;
}

void Serializer::inner_xtalize(const Any& v, int_t tab){
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
	int_t num = register_value(v, added);
	if(added){

		bool processed = false;

		if(cls.raweq(TClass<Array>::get())){
			const Array& a = (const Array&)v;
			if(a.empty()){
				stream_.put_s("[]");
			}else{
				stream_.put_s("[\n");
				tab++;
				for(int_t i=0; i<a.size(); ++i){
					put_tab(tab);
					inner_xtalize(a.at(i), tab);
					stream_.put_s(",\n");
				}
				tab--;
				put_tab(tab);
				stream_.put_s("]");
			}

			processed = true;
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

			processed = true;
		}else if(cls.raweq(TClass<Map>::get())){
			const Map& a = (const Map&)v;
			if(a.empty()){
				stream_.put_s("[:]");
			}else{
				stream_.put_s("[\n");
				tab++;
				Xfor2(key, value, a.each_pair()){
					put_tab(tab);
					inner_xtalize(key, tab);
					stream_.put_s(": ");
					inner_xtalize(value, tab);
					stream_.put_s(",\n");
				}
				tab--;
				put_tab(tab);
				stream_.put_s("]");
			}

			processed = true;
		}

		if(!processed){
			// 所属クラスにserial_save関数が定義されている
			if(v.get_class().member(Xid(serial_save))){
				ID id = v.get_class().object_name();

				check_id(id);
				stream_.put_s(id);
				stream_.put_s(".serial_new).serial_load(");
				inner_xtalize(v.send(Xid(serial_save)), tab);
			}else{
				ID id = v.object_name();
				check_id(id);
				stream_.put_s(id);
			}
		}

		stream_.put_s(")");
	}else{
		// 既に保存されているオブジェクトなので参照位置だけ保存する
		stream_.put_s("v[");
		stream_.put_s(Any(num).to_s());
		stream_.put_s("]");
	}
}

void Serializer::put_tab(int_t tab){
	while(tab--)
		stream_.put_i8('\t');
}


}
