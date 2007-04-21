
#pragma once

#include "any.h"
#include "marshal.h"
#include "array.h"
#include "frame.h"
#include "xmacro.h"
#include "cast.h"
#include "xmacro.h"
#include "map.h"

namespace xtal{

class MarshalImpl : public AnyImpl{
public:

	MarshalImpl()
		:pos_(0){
		set_class(TClass<Marshal>::get());
	}

	void w8(int_t v){
		data_.push_back(v);
	}

	void w16(int_t v){
		data_.push_back((v>>8)&0xff);
		data_.push_back((v>>0)&0xff);
	}

	void w32(int_t v){
		data_.push_back((v>>24)&0xff);
		data_.push_back((v>>16)&0xff);
		data_.push_back((v>>8)&0xff);
		data_.push_back((v>>0)&0xff);
	}

	void write(const String& v);

	int_t s8(){
		if(pos_+1>data_.size()){ return 0; }
		int_t ret = (xtal::s8)data_[pos_];
		pos_+= 1;
		return ret;
	}

	int_t s16(){
		if(pos_+2>data_.size()){ return 0; }
		int_t ret = (xtal::s16)((data_[pos_]<<8) | data_[pos_+1]);
		pos_+= 2;
		return ret;
	}

	int_t s32(){
		if(pos_+4>data_.size()){ return 0; }
		int_t ret = (xtal::s16)((data_[pos_]<<24) | (data_[pos_+1]<<16) | (data_[pos_+2]<<8) | data_[pos_+3]);
		pos_+= 4;
		return ret;
	}

	uint_t u8(){
		if(pos_+1>data_.size()){ return 0; }
		uint_t ret = data_[pos_];
		pos_+= 1;
		return ret;
	}

	uint_t u16(){
		if(pos_+2>data_.size()){ return 0; }
		uint_t ret = ((data_[pos_]<<8) | data_[pos_+1]);
		pos_+= 2;
		return ret;
	}

	uint_t u32(){
		if(pos_+4>data_.size()){ return 0; }
		uint_t ret = ((data_[pos_]<<24) | (data_[pos_+1]<<16) | (data_[pos_+2]<<8) | data_[pos_+3]);
		pos_+= 4;
		return ret;
	}

	enum{ VALUE, VALUE2, LIB, LIB2, REF, TNULL, INT, FLOAT, STRING, TID, ARRAY, MAP };

	/*
	static Any marshal_new(const Marshal& m){
		union{ int_t i; float_t f; } u;
		u.i = m.s32();
		return u.f;
	}
	*/

	void dump(const Any& v){
		if(!v){
			w8(TNULL);
			return;
		}

		const Class& cls = v.get_class();
		if(cls.raweq(TClass<Int>::get())){
			w8(INT);
			w32(v.ivalue());
			return;
		}else if(cls.raweq(TClass<Float>::get())){
			w8(FLOAT);
			union{ int_t i; float_t f; } u;
			u.f = v.fvalue();
			w32(u.i);
			return;
		}

		bool added = false;
		int_t num = register_dvalue(v, added);
		if(added){

			if(cls.raweq(TClass<Array>::get())){
				const Array& a = (const Array&)v;
				w8(ARRAY);
				w32(a.size());
				for(int_t i=0; i<a.size(); ++i){
					dump(a.at(i));
				}
				return;
			}else if(cls.raweq(TClass<String>::get())){
				const String& a = (const String&)v;
				if(a.is_interned()){
					w8(TID);
				}else{
					w8(STRING);
				}
				w32(a.size());
				for(int_t i=0; i<a.size(); ++i){
					w8(a.c_str()[i]);
				}
				return;
			}else if(cls.raweq(TClass<Map>::get())){
				const Map& a = (const Map&)v;
				w8(MAP);
				w32(a.size());
				Xfor2(key, value, a.each()){
					dump(key);
					dump(value);
				}
				return;
			}

			ID id = v.get_class().object_name();
			num = register_dvalue(id, added);
			// 所属クラスにmarshal_dump関数が定義されている
			if(v.get_class().member(Xid(marshal_dump))){
				// 直列化する
				if(added){
					w8(VALUE2);
					dump(id);
					dump(v.send(Xid(marshal_dump), this));					
				}else{
					w8(VALUE);
					w32(num); // クラスの名前を埋め込む
					dump(v.send(Xid(marshal_dump), this));
				}
			}else{
				// 名前だけを保存する
				if(added){
					w8(LIB2);
					dump(id.send(Xid(marshal_dump), this));
				}else{
					w8(LIB);
					w32(num);
				}
			}
		}else{
			// 既に保存されているオブジェクトなので参照位置だけ保存する
			w8(REF);
			w32(num);
		}
	}

	Any load(){
		int_t op = u8();
		switch(op){
			XTAL_NODEFAULT;

			XTAL_CASE(VALUE){
				int_t num = register_lvalue(null);
				Class c(cast<Class>(demangle(u32())));
				lvalues_.set_at(num, c.member(Xid(marshal_new))(this));
				return lvalues_[num];
			}

			XTAL_CASE(VALUE2){
				int_t num = register_lvalue(null);
				Class c(cast<Class>(demangle(register_lvalue(load()))));
				lvalues_.set_at(num, c.member(Xid(marshal_new))(this));
				return lvalues_[num];
			}

			XTAL_CASE(LIB){
				int_t num = register_lvalue(null);
				lvalues_.set_at(num, demangle(u32()));
				return lvalues_[num];
			}

			XTAL_CASE(LIB2){
				int_t num = register_lvalue(null);
				lvalues_.set_at(num, demangle(register_lvalue(load())));
				return lvalues_[num];
			}	

			XTAL_CASE(REF){
				return lvalues_[u32()];
			}

			XTAL_CASE(TNULL){
				return null;
			}

			XTAL_CASE(INT){
				return s32();
			}

			XTAL_CASE(FLOAT){
				union{ int_t i; float_t f; } u;
				u.i = s32();
				return u.f;
			}

			XTAL_CASE2(STRING, TID){
				int_t sz = s32();
				char* p = (char*)user_malloc(sz);
				for(int_t i = 0; i<sz; ++i){
					p[i] = s8();
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
				int_t sz = s32();
				Array ret(sz);
				register_lvalue(ret);
				for(int_t i = 0; i<sz; ++i){
					ret.set_at(i, load());
				}				
				return ret;
			}
			XTAL_CASE(MAP){
				int_t sz = s32();
				Map ret;
				register_lvalue(ret);
				Any key;
				for(int_t i = 0; i<sz; ++i){
					key = load();
					ret.set_at(key, load());
				}				
				return ret;
			}
		}
		return null;
	}

private:

	Any demangle(int_t n){
		Any& ret = lmap_[n];
		if(ret){ return ret; }
		Xfor(v, ((String&)lvalues_[n]).split("::")){
			ID id(((String&)v).intern());
			if(!ret){
				if(id.raweq(Xid(lib))){
					ret = lib();
				}
			}else{
				ret = ret.member(id);
			}
		}
		if(!ret){
			throw builtin().member("RuntimeError")(Xt("%sにアクセスできません。")(lvalues_[n]));
		}
		return ret;
	}

	int_t register_dvalue(const Any& v, bool& added){
		Info& info = dmap_[v];
		if(info.num<0){
			info.num = dvalues_.size();
			dvalues_.push_back(v);
			added = true;
		}else{
			added = false;
		}
		return info.num;
	}

	int_t register_lvalue(const Any& v){
		lvalues_.push_back(v);
		return lvalues_.size()-1;
	}

	struct Cmp{
		bool operator()(const Any& a, const Any& b){
			return a.rawlt(b);
		}
	};

	struct Info{
		int_t num;
		Info():num(-1){}
	};

	AC<Any, Info, Cmp>::map dmap_;
	Array dvalues_;

	AC<int_t, Any>::map lmap_;
	Array lvalues_;

	AC<xtal::u8>::vector data_;
	uint_t pos_;
};

}

