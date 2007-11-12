#include "xtal.h"
#include "xtal_macro.h"

namespace xtal{

enum{
	SERIALIZE_VERSION1 = 1,
	SERIALIZE_VERSION2 = 0,
};

Serializer::Serializer(const StreamPtr& s)
	:stream_(s){
}

void Serializer::serialize(const AnyPtr& v){
	clear();
	inner_serialize(v);
}

AnyPtr Serializer::deserialize(){
	clear();
	return inner_deserialize();
}

bool Serializer::check_id(const IDPtr& id){
	const char_t* str = id->c_str();
	if(str[0]=='l' && str[1]=='i' && str[2]=='b' && str[3]==':'){
		return true;
	}
	return false;
}

void Serializer::check_id_and_throw(const IDPtr& id){
	if(!check_id(id)){
		XTAL_THROW(builtin()->member("RuntimeError")(Xt("Xtal Runtime Error 1008")(Named("object", id))), return);
	}
}

void Serializer::inner_serialize(const AnyPtr& v){
	
	switch(type(v)){
		XTAL_NODEFAULT;

		XTAL_CASE(TYPE_NULL){
			stream_->put_u8(TNULL);
			return;
		}
		
		XTAL_CASE(TYPE_UNDEFINED){
			stream_->put_u8(TUNDEFINED);
			return;
		}

		XTAL_CASE(TYPE_BASE){}

		XTAL_CASE(TYPE_INT){
			stream_->put_u8(TINT);
			stream_->put_i32be(ivalue(v));
			return;
		}

		XTAL_CASE(TYPE_FLOAT){
			stream_->put_u8(TFLOAT);
			stream_->put_f32be(fvalue(v));
			return;
		}

		XTAL_CASE(TYPE_FALSE){
			stream_->put_u8(TFALSE);
			return;
		}

		XTAL_CASE(TYPE_TRUE){
			stream_->put_u8(TTRUE);
			return;
		}

		XTAL_CASE(TYPE_SMALL_STRING){}
	}

	const ClassPtr& cls = v->get_class();
	bool added = false;
	int_t num = register_value(v, added);
	if(added){
		if(ArrayPtr a = as<ArrayPtr>(v)){
			stream_->put_u8(TARRAY);
			stream_->put_u32be(a->size());
			for(uint_t i=0; i<a->size(); ++i){
				inner_serialize(a->at(i));
			}
			return;
		}
		else if(StringPtr a = as<StringPtr>(v)){
			stream_->put_u8(TSTRING);
			uint_t sz = a->buffer_size();
			const char* str = a->data();
			stream_->put_i32be(sz);
			for(size_t i=0; i<sz; ++i){
				stream_->put_u8(str[i]);
			}
			return;
		}
		else if(MapPtr a = as<MapPtr>(v)){
			stream_->put_u8(TMAP);
			stream_->put_u32be(a->size());
			Xfor2(key, value, a){
				inner_serialize(key);
				inner_serialize(value);
			}
			return;
		}
		else if(raweq(cls, get_cpp_class<Code>())){
			CodePtr p = cast<CodePtr>(v);
			stream_->put_u8('x'); stream_->put_u8('t'); stream_->put_u8('a'); stream_->put_u8('l');
			stream_->put_u8(SERIALIZE_VERSION1); stream_->put_u8(SERIALIZE_VERSION2); 
			stream_->put_u8(0); 
			stream_->put_u8(0);
			
			int_t sz;
			sz = p->code_.size();
			stream_->put_u32be(sz);
			if(sz!=0){ stream_->write(&p->code_[0], sizeof(p->code_[0])*sz); }
			
			sz = p->block_core_table_.size();
			stream_->put_u16be(sz);
			if(sz!=0){ stream_->write(&p->block_core_table_[0], sizeof(p->block_core_table_[0])*sz); }

			sz = p->class_core_table_.size();
			stream_->put_u16be(sz);
			if(sz!=0){ stream_->write(&p->class_core_table_[0], sizeof(p->class_core_table_[0])*sz); }

			sz = p->xfun_core_table_.size();
			stream_->put_u16be(sz);
			if(sz!=0){ stream_->write(&p->xfun_core_table_[0], sizeof(p->xfun_core_table_[0])*sz); }

			sz = p->except_core_table_.size();
			stream_->put_u16be(sz);
			if(sz!=0){ stream_->write(&p->except_core_table_[0], sizeof(p->except_core_table_[0])*sz); }

			sz = p->lineno_table_.size();
			stream_->put_u16be(sz);
			if(sz!=0){ stream_->write(&p->lineno_table_[0], sizeof(p->lineno_table_[0])*sz); }

			sz = p->once_table_->size();
			stream_->put_u16be(sz);

			MapPtr map = xnew<Map>();
			map->set_at("source", p->source_file_name_);
			map->set_at("identifiers", p->identifier_table_);
			map->set_at("values", p->value_table_);
			inner_serialize(map);

			return;
		}

		IDPtr id = v->object_name();
		if(check_id(id)){
			stream_->put_u8(LIB);
			inner_serialize(id);
		}
		else{
			id = v->get_class()->object_name();

			// serial_newで空オブジェクトを生成するコマンドを埋め込む
			stream_->put_u8(SERIAL_NEW);
			check_id_and_throw(id);
			inner_serialize(id); // クラスの名前を埋め込む

			// s_saveでserializableなオブジェクトを取り出しserializeする
			inner_serialize(v->send(Xid(s_save)));
		}
	}
	else{
		// 既に保存されているオブジェクトなので参照位置だけ保存する
		stream_->put_u8(REF);
		stream_->put_u32be(num);
	}
}

AnyPtr Serializer::inner_deserialize(){
	int_t op = stream_->get_u8();
	switch(op){
		XTAL_NODEFAULT;

		XTAL_CASE(SERIAL_NEW){
			int_t num = append_value(null);

			// serial_newをするクラスを取り出す
			ClassPtr c(cast<ClassPtr>(demangle(inner_deserialize())));

			const VMachinePtr& vm = vmachine();

			// serial_newを呼び出して、保存しておく
			vm->setup_call(1);
			c->s_new(vm);
			AnyPtr ret = vm->result();
			values_->set_at(num, ret);
			vm->cleanup_call();

			vm->setup_call(0, inner_deserialize());
			ret->rawsend(vm, Xid(s_load));
			vm->cleanup_call();

			return ret;
		}

		XTAL_CASE(LIB){
			int_t num = append_value(null);
			values_->set_at(num, demangle(inner_deserialize()));
			return values_->at(num);
		}	

		XTAL_CASE(REF){
			return values_->at(stream_->get_u32be());
		}

		XTAL_CASE(TNULL){
			return null;
		}
		
		XTAL_CASE(TUNDEFINED){
			return undefined;
		}

		XTAL_CASE(TINT){
			return stream_->get_i32be();
		}

		XTAL_CASE(TFLOAT){
			return stream_->get_f32be();
		}

		XTAL_CASE(TSTRING){
			int_t sz = stream_->get_u32be();
			char* p = (char*)user_malloc(sz+1);
			for(int_t i = 0; i<sz; ++i){
				p[i] = (char_t)stream_->get_u8();
			}
			p[sz] = 0;				
			StringPtr ret = xnew<ID>(p, sz);
			user_free(p);
			append_value(ret);
			return ret;
		}

		XTAL_CASE(TARRAY){
			int_t sz = stream_->get_u32be();
			ArrayPtr ret(xnew<Array>(sz));
			append_value(ret);
			for(int_t i = 0; i<sz; ++i){
				ret->set_at(i, inner_deserialize());
			}				
			return ret;
		}
		
		XTAL_CASE(TMAP){
			int_t sz = stream_->get_u32be();
			MapPtr ret(xnew<Map>());
			append_value(ret);
			AnyPtr key;
			for(int_t i = 0; i<sz; ++i){
				key = inner_deserialize();
				ret->set_at(key, inner_deserialize());
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
			CodePtr p = xnew<Code>();

			if(stream_->get_u8()!='t' || stream_->get_u8()!='a' || stream_->get_u8()!='l'){
				XTAL_THROW(builtin()->member("RuntimeError")(Xt("Xtal Runtime Error 1009")), return null);
			}

			xtal::u8 version1 = stream_->get_u8(), version2 = stream_->get_u8();
			if(version1!=SERIALIZE_VERSION1 || version2!=SERIALIZE_VERSION2){
				XTAL_THROW(builtin()->member("RuntimeError")(Xt("Xtal Runtime Error 1009")), return null);
			}
			
			stream_->get_u8();
			stream_->get_u8();
		
			int_t sz;
			sz = stream_->get_u32be();
			p->code_.resize(sz);
			if(sz!=0){ stream_->read(&p->code_[0], sizeof(p->code_[0])*sz); }

			sz = stream_->get_u16be();
			p->block_core_table_.resize(sz);
			if(sz!=0){ stream_->read(&p->block_core_table_[0], sizeof(p->block_core_table_[0])*sz); }
	
			sz = stream_->get_u16be();
			p->class_core_table_.resize(sz);
			if(sz!=0){ stream_->read(&p->class_core_table_[0], sizeof(p->class_core_table_[0])*sz); }

			sz = stream_->get_u16be();
			p->xfun_core_table_.resize(sz);
			if(sz!=0){ stream_->read(&p->xfun_core_table_[0], sizeof(p->xfun_core_table_[0])*sz); }

			sz = stream_->get_u16be();
			p->except_core_table_.resize(sz);
			if(sz!=0){ stream_->read(&p->except_core_table_[0], sizeof(p->except_core_table_[0])*sz); }

			sz = stream_->get_u16be();
			p->lineno_table_.resize(sz);
			if(sz!=0){ stream_->read(&p->lineno_table_[0], sizeof(p->lineno_table_[0])*sz); }

			sz = stream_->get_u16be();
			p->once_table_ = xnew<Array>(sz);
			for(int_t i=0; i<sz; ++i){
				p->once_table_->set_at(i, undefined);
			}

			FunPtr ret(xnew<Fun>(null, null, p, &p->xfun_core_table_[0]));
			append_value(ret);

			MapPtr map(cast<MapPtr>(inner_deserialize()));
			p->source_file_name_ = cast<StringPtr>(map->at("source"));
			p->identifier_table_ = cast<ArrayPtr>(map->at("identifiers"));
			p->value_table_ = cast<ArrayPtr>(map->at("values"));
			
			ret->set_object_name("<filelocal>", 1, null);	
			return ret;
		}
	}
	return null;
}

AnyPtr Serializer::demangle(const AnyPtr& n){
	AnyPtr ret = demangle_map_->at(n);
	if(ret){
		return ret;
	}

	Xfor(v, n->send("split")("::")){
		IDPtr id = ptr_cast<String>(v)->intern();
		if(!ret){
			if(raweq(id, Xid(lib))){
				ret = lib();
			}
		}
		else{
			ret = ret->member(id);
		}
	}

	if(!ret){
		XTAL_THROW(builtin()->member("RuntimeError")(Xt("Xtal Runtime Error 1008")(Named("object", n))), return null);
	}

	demangle_map_->set_at(n, ret);
	return ret;
}

int_t Serializer::register_value(const AnyPtr& v, bool& added){
	AnyPtr ret = map_->at(v);
	if(rawne(ret, undefined)){
		added = false;
	}
	else{
		ret = append_value(v);
		added = true;
	}
	return ret->to_i();
}

int_t Serializer::append_value(const AnyPtr& v){
	int_t ret = (int_t)values_->size();
	map_->set_at(v, ret);
	values_->push_back(v);
	return ret;
}

void Serializer::put_tab(int_t tab){
	while(tab--)
		stream_->put_s("\t");
}


}
