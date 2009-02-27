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

void Serializer::inner_serialize(const AnyPtr& v){
	
	// 基本型かチェック
	switch(type(v)){
		XTAL_DEFAULT;

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
	}

	const ClassPtr& cls = v->get_class();
	int_t num = register_value(v);
	if(num>=0){
		// 既に保存されているオブジェクトなので参照位置だけ保存する
		stream_->put_u8(REF);
		stream_->put_u32be(num);
		return;
	}
	
	switch(type(v)){
		XTAL_DEFAULT;

		XTAL_CASE2(TYPE_SMALL_STRING, TYPE_STRING){
			const StringPtr& a = unchecked_ptr_cast<String>(v);
			stream_->put_u8(a->is_interned() ? TID : TSTRING);
			uint_t sz = a->data_size();
			const char_t* str = a->data();
			stream_->put_i32be(sz);
			for(size_t i=0; i<sz; ++i){
				stream_->put_ch_code_be(str[i]);
			}
			return;
		}

		XTAL_CASE(TYPE_ARRAY){
			const ArrayPtr& a = unchecked_ptr_cast<Array>(v);
			stream_->put_u8(TARRAY);
			stream_->put_u32be(a->size());
			for(uint_t i=0; i<a->size(); ++i){
				inner_serialize(a->at(i));
			}
			return;
		}

		XTAL_CASE(TYPE_MULTI_VALUE){
			const MultiValuePtr& a = unchecked_ptr_cast<MultiValue>(v);
			stream_->put_u8(TMULTI_VALUE);
			inner_serialize(a->head());
			inner_serialize(a->tail());
			return;
		}
	}
	
	if(const MapPtr& a = as<MapPtr>(v)){
		stream_->put_u8(TMAP);
		stream_->put_u32be(a->size());
		Xfor2(key, value, a){
			inner_serialize(key);
			inner_serialize(value);
		}
		return;
	}

	if(raweq(cls, get_cpp_class<Code>())){
		CodePtr p = cast<CodePtr>(v);
		stream_->put_u8('x'); stream_->put_u8('t'); stream_->put_u8('a'); stream_->put_u8('l');
		stream_->put_u8(SERIALIZE_VERSION1); stream_->put_u8(SERIALIZE_VERSION2); 
		stream_->put_u8(0); 
		stream_->put_u8(0);
		
		int_t sz;
		sz = p->code_.size();
		stream_->put_u32be(sz);
		if(sz!=0){ stream_->write(&p->code_[0], sizeof(p->code_[0])*sz); }
		
		sz = p->scope_info_table_.size();
		stream_->put_u16be(sz);
		if(sz!=0){ stream_->write(&p->scope_info_table_[0], sizeof(p->scope_info_table_[0])*sz); }

		sz = p->class_info_table_.size();
		stream_->put_u16be(sz);
		if(sz!=0){ stream_->write(&p->class_info_table_[0], sizeof(p->class_info_table_[0])*sz); }

		sz = p->xfun_info_table_.size();
		stream_->put_u16be(sz);
		if(sz!=0){ stream_->write(&p->xfun_info_table_[0], sizeof(p->xfun_info_table_[0])*sz); }

		sz = p->except_info_table_.size();
		stream_->put_u16be(sz);
		if(sz!=0){ stream_->write(&p->except_info_table_[0], sizeof(p->except_info_table_[0])*sz); }

		sz = p->lineno_table_.size();
		stream_->put_u16be(sz);
		if(sz!=0){ stream_->write(&p->lineno_table_[0], sizeof(p->lineno_table_[0])*sz); }

		sz = p->once_table_->size();
		stream_->put_u16be(sz);

		MapPtr map = xnew<Map>();
		map->set_at(Xid(source), p->source_file_name_);
		map->set_at(Xid(identifiers), p->identifier_table_);
		map->set_at(Xid(values), p->value_table_);
		inner_serialize(map);

		return;
	}
	
	// 名前で保存したら読込できそうか
	ArrayPtr name_list = v->object_name_list();
	if(name_list && !name_list->is_empty()){
		stream_->put_u8(NAME);
		inner_serialize(name_list);
	}
	else{
		// serial_newで空オブジェクトを生成するコマンドを埋め込む
		stream_->put_u8(SERIAL_NEW);

		// クラスを埋め込む
		inner_serialize(v->get_class()); 

		// s_saveでserializableなオブジェクトを取り出しserializeする
		inner_serialize(v->send(Xid(s_save)));
	}
}

AnyPtr Serializer::inner_deserialize(){
	if(stream_->eos()){
		return undefined;
	}

	int_t op = stream_->get_u8();
	switch(op){
		XTAL_DEFAULT;

		XTAL_CASE(SERIAL_NEW){
			int_t num = append_value(null);

			// serial_newをするクラスを取り出す
			ClassPtr c(cast<ClassPtr>(inner_deserialize()));

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

		XTAL_CASE(NAME){
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

		XTAL_CASE2(TSTRING, TID){
			int_t sz = stream_->get_u32be();
			char_t* p = (char_t*)so_malloc(sizeof(char_t)*(sz+1));
			for(int_t i = 0; i<sz; ++i){
				p[i] = (char_t)stream_->get_ch_code_be();
			}
			p[sz] = 0;	
			IDPtr ret = xnew<ID>(p, sz);
			so_free(p, sizeof(char_t)*(sz+1));
			append_value(ret);
			return ret;
		}

		XTAL_CASE(TARRAY){
			int_t sz = stream_->get_u32be();
			ArrayPtr ret = xnew<Array>(sz);
			append_value(ret);
			for(int_t i = 0; i<sz; ++i){
				ret->set_at(i, inner_deserialize());
			}				
			return ret;
		}

		XTAL_CASE(TMULTI_VALUE){
			MultiValuePtr ret = xnew<MultiValue>(null);
			append_value(ret);
			AnyPtr head = inner_deserialize();
			AnyPtr tail = inner_deserialize();
			ret->set(head, unchecked_ptr_cast<MultiValue>(tail));		
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
				XTAL_SET_EXCEPT(RuntimeError()->call(Xt("Xtal Runtime Error 1009")));
				return null;
			}

			xtal::u8 version1 = stream_->get_u8(), version2 = stream_->get_u8();
			if(version1!=SERIALIZE_VERSION1 || version2!=SERIALIZE_VERSION2){
				XTAL_SET_EXCEPT(RuntimeError()->call(Xt("Xtal Runtime Error 1009")));
				return null;
			}
			
			stream_->get_u8();
			stream_->get_u8();
		
			int_t sz;
			sz = stream_->get_u32be();
			p->code_.resize(sz);
			if(sz!=0){ stream_->read(&p->code_[0], sizeof(p->code_[0])*sz); }

			sz = stream_->get_u16be();
			p->scope_info_table_.resize(sz);
			if(sz!=0){ stream_->read(&p->scope_info_table_[0], sizeof(p->scope_info_table_[0])*sz); }
	
			sz = stream_->get_u16be();
			p->class_info_table_.resize(sz);
			if(sz!=0){ stream_->read(&p->class_info_table_[0], sizeof(p->class_info_table_[0])*sz); }

			sz = stream_->get_u16be();
			p->xfun_info_table_.resize(sz);
			if(sz!=0){ stream_->read(&p->xfun_info_table_[0], sizeof(p->xfun_info_table_[0])*sz); }

			sz = stream_->get_u16be();
			p->except_info_table_.resize(sz);
			if(sz!=0){ stream_->read(&p->except_info_table_[0], sizeof(p->except_info_table_[0])*sz); }

			sz = stream_->get_u16be();
			p->lineno_table_.resize(sz);
			if(sz!=0){ stream_->read(&p->lineno_table_[0], sizeof(p->lineno_table_[0])*sz); }

			sz = stream_->get_u16be();
			p->once_table_ = xnew<Array>(sz);
			for(int_t i=0; i<sz; ++i){
				p->once_table_->set_at(i, undefined);
			}

			MethodPtr ret(xnew<Method>(null, p, &p->xfun_info_table_[0]));
			append_value(ret);

			MapPtr map(cast<MapPtr>(inner_deserialize()));
			p->source_file_name_ = cast<StringPtr>(map->at(Xid(source)));
			p->identifier_table_ = cast<ArrayPtr>(map->at(Xid(identifiers)));
			p->value_table_ = cast<ArrayPtr>(map->at(Xid(values)));
			p->first_fun_ = ret;
			return p;
		}
	}
	return null;
}

AnyPtr Serializer::demangle(const AnyPtr& n){
	AnyPtr ret;
	Xfor_cast(const MultiValuePtr& mv, n){
		if(first_step){
			if(raweq(mv->at(0), Xid(lib))){
				ret = lib();
			}
			else{
				break;
			}
		}
		else{
			ret = ret->member(ptr_cast<ID>(mv->at(0)), mv->at(1));
		}
	}

	if(!ret){
		XTAL_SET_EXCEPT(RuntimeError()->call(Xt("Xtal Runtime Error 1008")->call(Named(Xid(object), n))));
		return null;
	}

	return ret;
}

int_t Serializer::register_value(const AnyPtr& v){
	AnyPtr ret = map_->at(v);
	if(rawne(ret, undefined)){
		return ret->to_i();
	}
	else{
		ret = append_value(v);
		return -1;
	}
}

int_t Serializer::append_value(const AnyPtr& v){
	uint_t ret = values_->size();
	map_->set_at(v, ret);
	values_->push_back(v);
	return ret;
}

void Serializer::put_tab(int_t tab){
	while(tab--){
		stream_->put_s("\t");
	}
}


}
