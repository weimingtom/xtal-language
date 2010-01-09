#include "xtal.h"
#include "xtal_macro.h"

#include "xtal_serializer.h"

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
	AnyPtr ret = inner_deserialize();
	XTAL_CHECK_EXCEPT(e){ return undefined; }
	return ret;
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
			if(sizeof(int_t)==4){
				stream_->put_u8(TINT32);
				stream_->put_i32be(ivalue(v));
			}
			else{
				stream_->put_u8(TINT64);
				stream_->put_i64be(ivalue(v));
			}
			return;
		}

		XTAL_CASE(TYPE_FLOAT){
			if(sizeof(float_t)==4){
				stream_->put_u8(TFLOAT32);
				stream_->put_f32be(fvalue(v));
			}
			else{
				stream_->put_u8(TFLOAT64);
				stream_->put_f64be(fvalue(v));
			}
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

		XTAL_CASE4(TYPE_SMALL_STRING, TYPE_STRING_LITERAL, TYPE_ID_LITERAL, TYPE_STRING){
			const StringPtr& a = unchecked_ptr_cast<String>(v);
			uint_t sz = a->data_size();
			const char_t* str = a->data();

			stream_->put_u8(a->is_interned() ? TID : TSTRING);
			stream_->put_u32be(sz);
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

		XTAL_CASE(TYPE_VALUES){
			const ValuesPtr& a = unchecked_ptr_cast<Values>(v);
			stream_->put_u8(TVALUES);
			inner_serialize(a->head());
			inner_serialize(a->tail());
			return;
		}
	}
	
	if(const MapPtr& a = ptr_cast<Map>(v)){
		stream_->put_u8(TMAP);
		stream_->put_u32be(a->size());
		Xfor2(key, value, a){
			inner_serialize(key);
			inner_serialize(value);
		}
		return;
	}

	if(raweq(cls, cpp_class<Code>())){
		CodePtr p = cast<CodePtr>(v);
		stream_->put_u8('x'); stream_->put_u8('t'); stream_->put_u8('a'); stream_->put_u8('l');
		stream_->put_u8(SERIALIZE_VERSION1); stream_->put_u8(SERIALIZE_VERSION2); 
		stream_->put_u8(0); 
		stream_->put_u8(0);
		
		uint_t sz;
		sz = p->code_.size();
		stream_->put_u32be(sz);
		if(sz!=0){ stream_->write(&p->code_[0], sz); }	
		
		sz = p->scope_info_table_.size();
		stream_->put_u16be(sz);
		for(uint_t i=0; i<sz; ++i){
			ScopeInfo& info = p->scope_info_table_[i];
			stream_->put_u32be(info.pc);
			stream_->put_u8(info.kind);
			stream_->put_u8(info.flags);
			stream_->put_u16be(info.variable_identifier_offset);
			stream_->put_u16be(info.variable_size);
		}
		
		sz = p->class_info_table_.size();
		stream_->put_u16be(sz);
		for(uint_t i=0; i<sz; ++i){
			ClassInfo& info = p->class_info_table_[i];
			stream_->put_u32be(info.pc);
			stream_->put_u8(info.kind);
			stream_->put_u8(info.flags);
			stream_->put_u16be(info.variable_identifier_offset);
			stream_->put_u16be(info.variable_size);

			stream_->put_u16be(info.instance_variable_size);
			stream_->put_u16be(info.instance_variable_identifier_offset);
			stream_->put_u16be(info.name_number);
			stream_->put_u8(info.mixins);
		}

		sz = p->xfun_info_table_.size();
		stream_->put_u16be(sz);
		for(uint_t i=0; i<sz; ++i){
			FunInfo& info = p->xfun_info_table_[i];
			stream_->put_u32be(info.pc);
			stream_->put_u8(info.kind);
			stream_->put_u8(info.flags);
			stream_->put_u16be(info.variable_identifier_offset);
			stream_->put_u16be(info.variable_size);
			
			stream_->put_u16be(info.max_stack);
			stream_->put_u16be(info.max_variable);
			stream_->put_u16be(info.name_number);
			stream_->put_u8(info.min_param_count);
			stream_->put_u8(info.max_param_count);
		}

		sz = p->except_info_table_.size();
		stream_->put_u16be(sz);
		for(uint_t i=0; i<sz; ++i){
			ExceptInfo& info = p->except_info_table_[i];
			stream_->put_u32be(info.catch_pc);
			stream_->put_u32be(info.finally_pc);
			stream_->put_u32be(info.end_pc);
		}
		
		sz = p->lineno_table_.size();
		stream_->put_u16be(sz);
		for(uint_t i=0; i<sz; ++i){
			Code::LineNumberInfo& info = p->lineno_table_[i];
			stream_->put_u32be(info.start_pc);
			stream_->put_u16be(info.lineno);
			stream_->put_u8(info.op);
			stream_->put_u8(info.breakpoint);
		}
			
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

		Xfor_cast(const ValuesPtr& mv, name_list){
			if(first_step){
				if(raweq(mv->at(0), Xid(lib))){
					break;
				}
				else{
					break;
				}
			}
		}
	}
	else{
		// serial_newで空オブジェクトを生成するコマンドを埋め込む
		stream_->put_u8(SERIAL_NEW);

		// クラスを埋め込む
		inner_serialize(v->get_class()); 

		// s_saveでserializableなオブジェクトを取り出しserializeする
		inner_serialize(v->s_save());
	}
}

AnyPtr Serializer::inner_deserialize_serial_new(){
	int_t num = append_value(null);

	// serial_newをするクラスを取り出す
	if(ClassPtr c = (ptr_cast<Class>(inner_deserialize()))){
		const VMachinePtr& vm = vmachine();

		// serial_newを呼び出して、保存しておく
		vm->setup_call(1);
		c->s_new(vm);
		AnyPtr ret = vm->result();
		values_.set_at(num, ret);
		vm->cleanup_call();

		ret->s_load(inner_deserialize());

		return ret;
	}

	return undefined;
}

AnyPtr Serializer::inner_deserialize_name(){
	int_t num = append_value(null);
	values_.set_at(num, demangle(inner_deserialize()));
	return values_.at(num);
}

AnyPtr Serializer::inner_deserialize_string(bool intern){
	uint_t sz = stream_->get_u32be();
	XMallocGuard guard(sizeof(char_t)*sz);
	char_t* p = (char_t*)guard.get();
	for(uint_t i = 0; i<sz; ++i){
		p[i] = stream_->get_ch_code_be();
	}

	AnyPtr ret;
	if(intern){
		ret = xnew<ID>(p, sz);
	}
	else{
		ret = xnew<String>(p, sz);
	}
	append_value(ret);
	return ret;
}

AnyPtr Serializer::inner_deserialize_array(){
	uint_t sz = stream_->get_u32be();
	ArrayPtr ret = xnew<Array>(sz);
	append_value(ret);
	for(uint_t i = 0; i<sz; ++i){
		ret->set_at(i, inner_deserialize());
	}				
	return ret;
}

AnyPtr Serializer::inner_deserialize_values(){
	ValuesPtr ret = xnew<Values>(null);
	append_value(ret);
	AnyPtr head = inner_deserialize();
	AnyPtr tail = inner_deserialize();
	ret->set(head, unchecked_ptr_cast<Values>(tail));		
	return ret;
}

AnyPtr Serializer::inner_deserialize_map(){
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

AnyPtr Serializer::inner_deserialize_code(){
	CodePtr p = xnew<Code>();

	if(stream_->get_u8()!='t' || stream_->get_u8()!='a' || stream_->get_u8()!='l'){
		XTAL_SET_EXCEPT(cpp_class<RuntimeError>()->call(Xt("XRE1009")));
		return null;
	}

	xtal::u8 version1 = stream_->get_u8(), version2 = stream_->get_u8();
	if(version1!=SERIALIZE_VERSION1 || version2!=SERIALIZE_VERSION2){
		XTAL_SET_EXCEPT(cpp_class<RuntimeError>()->call(Xt("XRE1009")));
		return null;
	}
	
	stream_->get_u8();
	stream_->get_u8();

	uint_t sz;
	sz = stream_->get_u32be();
	p->code_.resize(sz);
	if(sz){ stream_->read(&p->code_[0], sz); }
	
	sz = stream_->get_u16be();
	p->scope_info_table_.resize(sz);
	for(uint_t i=0; i<sz; ++i){
		ScopeInfo& info = p->scope_info_table_[i];
		info.pc = stream_->get_u32be();
		info.kind = stream_->get_u8();
		info.flags = stream_->get_u8();
		info.variable_identifier_offset = stream_->get_u16be();
		info.variable_size = stream_->get_u16be();
	}
	
	sz = stream_->get_u16be();
	p->class_info_table_.resize(sz);
	for(uint_t i=0; i<sz; ++i){
		ClassInfo& info = p->class_info_table_[i];
		info.pc = stream_->get_u32be();
		info.kind = stream_->get_u8();
		info.flags = stream_->get_u8();
		info.variable_identifier_offset = stream_->get_u16be();
		info.variable_size = stream_->get_u16be();

		info.instance_variable_size = stream_->get_u16be();
		info.instance_variable_identifier_offset = stream_->get_u16be();
		info.name_number = stream_->get_u16be();
		info.mixins = stream_->get_u8();
	}

	sz = stream_->get_u16be();
	p->xfun_info_table_.resize(sz);
	for(uint_t i=0; i<sz; ++i){
		FunInfo& info = p->xfun_info_table_[i];
		info.pc = stream_->get_u32be();
		info.kind = stream_->get_u8();
		info.flags = stream_->get_u8();
		info.variable_identifier_offset = stream_->get_u16be();
		info.variable_size = stream_->get_u16be();

		info.max_stack = stream_->get_u16be();
		info.max_variable = stream_->get_u16be();
		info.name_number = stream_->get_u16be();
		info.min_param_count = stream_->get_u8();
		info.max_param_count = stream_->get_u8();
	}

	sz = stream_->get_u16be();
	p->except_info_table_.resize(sz);
	for(uint_t i=0; i<sz; ++i){
		ExceptInfo& info = p->except_info_table_[i];
		info.catch_pc = stream_->get_u32be();
		info.finally_pc = stream_->get_u32be();
		info.end_pc = stream_->get_u32be();
	}
	
	sz = stream_->get_u16be();
	p->lineno_table_.resize(sz);
	for(uint_t i=0; i<sz; ++i){
		Code::LineNumberInfo& info = p->lineno_table_[i];
		info.start_pc = stream_->get_u32be();
		info.lineno = stream_->get_u16be();
		info.op = stream_->get_u8();
		info.breakpoint = stream_->get_u8();
	}

	sz = stream_->get_u16be();
	p->once_table_ = xnew<Array>(sz);
	for(uint_t i=0; i<sz; ++i){
		p->once_table_->set_at(i, undefined);
	}

	MethodPtr ret(xnew<Method>(nul<Frame>(), p, &p->xfun_info_table_[0]));
	append_value(ret);

	AnyPtr mapd = inner_deserialize();
	MapPtr map(ptr_cast<Map>(mapd));
	
	XTAL_ASSERT(map);
	
	p->source_file_name_ = ptr_cast<String>(map->at(Xid(source)));
	p->identifier_table_ = ptr_cast<Array>(map->at(Xid(identifiers)));
	p->value_table_ = ptr_cast<Array>(map->at(Xid(values)));
	p->first_fun_ = ret;
	return p;
}

AnyPtr Serializer::inner_deserialize(){
	if(stream_->eos()){
		return undefined;
	}

	XTAL_CHECK_EXCEPT(e){
		return undefined;
	}

	int_t op = stream_->get_u8();
	switch(op){
		XTAL_DEFAULT;

		XTAL_CASE(SERIAL_NEW){
			return inner_deserialize_serial_new();
		}

		XTAL_CASE(NAME){
			return inner_deserialize_name();
		}	

		XTAL_CASE(REF){
			return values_.at(stream_->get_u32be());
		}

		XTAL_CASE(TNULL){
			return null;
		}
		
		XTAL_CASE(TUNDEFINED){
			return undefined;
		}

		XTAL_CASE(TINT32){
			return (int_t)stream_->get_i32be();
		}

		XTAL_CASE(TFLOAT32){
			return (float_t)stream_->get_f32be();
		}

		XTAL_CASE(TINT64){
			return (int_t)stream_->get_i64be();
		}

		XTAL_CASE(TFLOAT64){
			return (float_t)stream_->get_f64be();
		}

		XTAL_CASE(TSTRING){
			return inner_deserialize_string(false);
		}

		XTAL_CASE(TID){
			return inner_deserialize_string(true);
		}

		XTAL_CASE(TARRAY){
			return inner_deserialize_array();
		}

		XTAL_CASE(TVALUES){
			return inner_deserialize_values();
		}
		
		XTAL_CASE(TMAP){
			return inner_deserialize_map();
		}
		
		XTAL_CASE(TTRUE){
			return true;
		}
		
		XTAL_CASE(TFALSE){
			return false;
		}

		XTAL_CASE('x'){
			return inner_deserialize_code();
		}
	}
	return null;
}

AnyPtr Serializer::demangle(const AnyPtr& n){
	AnyPtr ret;
	Xfor_cast(const ValuesPtr& mv, n){
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
		XTAL_SET_EXCEPT(cpp_class<RuntimeError>()->call(Xt("XRE1008")->call(Named(Xid(object), n))));
		return null;
	}

	return ret;
}

int_t Serializer::register_value(const AnyPtr& v){
	table_t::iterator it=map_.find(v);
	if(it!=map_.end()){
		return it->second;
	}
	append_value(v);
	return -1;
}

int_t Serializer::append_value(const AnyPtr& v){
	uint_t ret = values_.size();
	map_.insert(v, ret);
	values_.push_back(v);
	return ret;
}

void Serializer::put_tab(int_t tab){
	while(tab--){
		stream_->put_s("\t");
	}
}


}
