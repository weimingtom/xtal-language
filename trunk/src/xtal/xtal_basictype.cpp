#include "xtal.h"
#include "xtal_macro.h"

namespace xtal{

Null null;
Undefined undefined;
Named null_named;
IDPtr empty_id(unchecked_ptr_cast<ID>(ap(Any(TYPE_SMALL_STRING))));

FunCore empty_xfun_core;
ScopeCore empty_scope_core;
ClassCore empty_class_core;
ExceptCore empty_except_core;
EmptyInstanceVariables empty_instance_variables;

undeleter_t undeleter;
deleter_t deleter;

namespace{

	int_t Int_to_i(const AnyPtr& p){
		if(type(p)==TYPE_INT){
			return ivalue(p);
		}

		XTAL_THROW(cast_error(p->get_class()->object_name(), Xid(Int)), return 0);
	}

	float_t Int_to_f(const AnyPtr& p){
		if(type(p)==TYPE_INT){
			return (float_t)ivalue(p);
		}

		XTAL_THROW(cast_error(p->get_class()->object_name(), Xid(Int)), return 0);
	}

	StringPtr Int_to_s(const AnyPtr& p){
		if(type(p)==TYPE_INT){
			return Xf("%d")->call(p)->to_s();
		}

		XTAL_THROW(cast_error(p->get_class()->object_name(), Xid(Int)), return null);
	}

	int_t Float_to_i(const AnyPtr& p){
		if(type(p)==TYPE_FLOAT){
			return (int_t)fvalue(p);
		}

		XTAL_THROW(cast_error(p->get_class()->object_name(), Xid(Float)), return 0);
	}

	float_t Float_to_f(const AnyPtr& p){
		if(type(p)==TYPE_FLOAT){
			return fvalue(p);
		}

		XTAL_THROW(cast_error(p->get_class()->object_name(), Xid(Float)), return 0);
	}

	StringPtr Float_to_s(const AnyPtr& p){
		if(type(p)==TYPE_FLOAT){
			return Xf("%g")->call(p)->to_s();
		}

		XTAL_THROW(cast_error(p->get_class()->object_name(), Xid(Float)), return null);
	}

	IntRangePtr op_range_Int(const AnyPtr& left, const AnyPtr& right, int_t kind){
		return xnew<IntRange>(left->to_i(), right->to_i(), kind);	
	}

	FloatRangePtr op_range_Float(const AnyPtr& left, const AnyPtr& right, int_t kind){
		return xnew<FloatRange>(left->to_f(), right->to_f(), kind);	
	}

	bool op_in_Int_IntRange(int_t v, const IntRangePtr& range){
		return range->begin() <= v && v < range->end();
	}

	bool op_in_Float_IntRange(float_t v, const IntRangePtr& range){
		return range->begin() <= v && v < range->end();
	}

	bool op_in_Float_FloatRange(float_t v, const FloatRangePtr& range){
		return (range->is_left_closed() ? (range->left() <= v) : (range->left() < v)) && (range->is_right_closed() ? (v <= range->right()) : (v < range->right()));
	}

	AnyPtr to_mv_Undefined(const AnyPtr& v){
		return xnew<MultiValue>();
	}
	
	class IntRangeIter : public Base{
	public:

		IntRangeIter(const IntRangePtr& range)
			:it_(range->begin()), end_(range->end()){
		}

		void block_next(const VMachinePtr& vm){
			if(it_<end_){
				vm->return_result(from_this(this), it_);
				++it_;
			}
			else{
				vm->return_result(null, null);
			}
		}

	private:
		int_t it_, end_;
	};
}

AnyPtr IntRange::each(){
	return xnew<IntRangeIter>(from_this(this));
}


///////////////////////////////////

void initialize_basictype(){

	builtin()->def(Xid(String), new_cpp_class<String>());
	builtin()->def(Xid(Int), new_cpp_class<Int>());
	builtin()->def(Xid(Float), new_cpp_class<Float>());
	builtin()->def(Xid(Null), new_cpp_class<Null>());
	builtin()->def(Xid(Undefined), new_cpp_class<Undefined>());
	builtin()->def(Xid(True), new_cpp_class<True>());
	builtin()->def(Xid(False), new_cpp_class<False>());
	builtin()->def(Xid(IntRange), new_cpp_class<IntRange>());
	builtin()->def(Xid(FloatRange), new_cpp_class<FloatRange>());
	builtin()->def(Xid(ChRange), new_cpp_class<ChRange>());


	{
		ClassPtr p = new_cpp_class<Undefined>(Xid(Undefined));	
		p->method(Xid(to_mv), &to_mv_Undefined);
	}
	
	{
		ClassPtr p = new_cpp_class<Int>(Xid(Int));	
		p->method(Xid(to_i), &Int_to_i);
		p->method(Xid(to_f), &Int_to_f);
		p->method(Xid(to_s), &Int_to_s);
		p->method(Xid(op_range), &op_range_Int, get_cpp_class<Int>());
		p->method(Xid(op_range), &op_range_Float, get_cpp_class<Float>());
		p->method(Xid(op_in), &op_in_Int_IntRange, get_cpp_class<IntRange>());
		p->method(Xid(op_in), &op_in_Float_FloatRange, get_cpp_class<FloatRange>());
	}

	{
		ClassPtr p = new_cpp_class<Float>(Xid(Float));
		p->method(Xid(to_i), &Float_to_i);
		p->method(Xid(to_f), &Float_to_f);
		p->method(Xid(to_s), &Float_to_s);
		p->method(Xid(op_range), &op_range_Float, get_cpp_class<Int>());
		p->method(Xid(op_range), &op_range_Float, get_cpp_class<Float>());
		p->method(Xid(op_in), &op_in_Int_IntRange, get_cpp_class<IntRange>());
		p->method(Xid(op_in), &op_in_Float_FloatRange, get_cpp_class<FloatRange>());
	}

	{
		ClassPtr p = new_cpp_class<IntRangeIter>(Xid(IntRangeIter));
		p->inherit(Iterator());
		p->method(Xid(block_next), &IntRangeIter::block_next);
	}

	{
		ClassPtr p = new_cpp_class<Range>(Xid(Range));
		p->inherit(Iterable());
		p->def(Xid(new), ctor<Range, const AnyPtr&, const AnyPtr&, int_t>()->params(Xid(left), null, Xid(right), null, Xid(kind), 0));
		p->method(Xid(left), &Range::left);
		p->method(Xid(right), &Range::right);
		p->method(Xid(kind), &Range::kind);
	}

	{
		ClassPtr p = new_cpp_class<IntRange>(Xid(IntRange));
		p->inherit(get_cpp_class<Range>());
		p->def(Xid(new), ctor<IntRange, int_t, int_t, int_t>()->params(Xid(left), null, Xid(right), null, Xid(kind), 0));
		p->method(Xid(begin), &IntRange::begin);
		p->method(Xid(end), &IntRange::end);
		p->method(Xid(each), &IntRange::each);
	}

	{
		ClassPtr p = new_cpp_class<FloatRange>(Xid(FloatRange));
		p->inherit(get_cpp_class<Range>());
		p->def(Xid(new), ctor<FloatRange, float_t, float_t, int_t>()->params(Xid(left), null, Xid(right), null, Xid(kind), 0));
	}

}

void initialize_basictype_script(){
	Xemb((

Any::p: method{
	println(this.to_s);
	return this;
}

Any::to_s: method this.object_name;

Int::times: method fiber{
	for(i: 0; i<this; ++i){
		yield i;
	}
}

Null::to_s: method "null";
Null::to_a: method [];
Null::to_m: method [:];
Null::block_first: method null;
Undefined::to_s: method "undefined";
True::to_s: method "true";
False::to_s: method "false";

Int::block_next: method{
	return (this==0 ? null : this-1), this;
}

Int::block_first: Int::block_next;

op_add: method(v){ return this + v; }
Int::op_add#Int: op_add;
Int::op_add#Float: op_add;
Float::op_add#Int: op_add;
Float::op_add#Float: op_add;
Int::op_add_assign#Int: op_add;
Int::op_add_assig#Float: op_add;
Float::op_add_assig#Int: op_add;
Float::op_add_assig#Float: op_add;

op_sub: method(v){ return this - v; }
Int::op_sub#Int: op_sub;
Int::op_sub#Float: op_sub;
Float::op_sub#Int: op_sub;
Float::op_sub#Float: op_sub;
Int::op_sub_assign#Int: op_sub;
Int::op_sub_assign#Float: op_sub;
Float::op_sub_assign#Int: op_sub;
Float::op_sub_assign#Float: op_sub;

op_mul: method(v){ return this * v; }
Int::op_mul#Int: op_mul;
Int::op_mul#Float: op_mul;
Float::op_mul#Int: op_mul;
Float::op_mul#Float: op_mul;
Int::op_mul_assign#Int: op_mul;
Int::op_mul_assign#Float: op_mul;
Float::op_mul_assign#Int: op_mul;
Float::op_mul_assign#Float: op_mul;

op_div: method(v){ return this / v; }
Int::op_div#Int: op_div;
Int::op_div#Float: op_div;
Float::op_div#Int: op_div;
Float::op_div#Float: op_div;
Int::op_div_assign#Int: op_div;
Int::op_div_assign#Float: op_div;
Float::op_div_assign#Int: op_div;
Float::op_div_assign#Float: op_div;

op_mod: method(v){ return this % v; }
Int::op_mod#Int: op_mod;
Int::op_mod#Float: op_mod;
Float::op_mod#Int: op_mod;
Float::op_mod#Float: op_mod;
Int::op_mod_assign#Int: op_mod;
Int::op_mod_assign#Float: op_mod;
Float::op_mod_assign#Int: op_mod;
Float::op_mod_assign#Float: op_mod;

op_and: method(v){ return this & v; }
Int::op_and#Int: op_and;
Int::op_and_assign#Int: op_and;

op_or: method(v){ return this | v; }
Int::op_or#Int: op_or;
Int::op_or_assign#Int: op_or;

op_xor: method(v){ return this ^ v; }
Int::op_xor#Int: op_xor;
Int::op_xor_assign#Int: op_xor;

op_shr: method(v){ return this >> v; }
Int::op_shr#Int: op_shr;
Int::op_shr_assign#Int: op_shr;

op_ushr: method(v){ return this >>> v; }
Int::op_ushr#Int: op_ushr;
Int::op_ushr_assign#Int: op_ushr;

op_shl: method(v){ return this << v; }
Int::op_shl#Int: op_shl;
Int::op_shl_assign#Int: op_shl;

op_inc: method{ return this+1; }
Int::op_inc: op_inc;
Float::op_inc: op_inc;

op_dec: method{ return this-1; }
Int::op_dec: op_dec;
Float::op_dec: op_dec;

op_pos: method{ return +this; }
Int::op_pos: op_pos;
Float::op_pos: op_pos;

op_neg: method{ return -this; }
Int::op_neg: op_neg;
Float::op_neg: op_neg;

op_com: method{ return ~this; }
Int::op_com: op_com;


op_lt: method(v){ return this < v; }
Int::op_lt#Int: op_lt;
Int::op_lt#Float: op_lt;
Float::op_lt#Int: op_lt;
Float::op_lt#Float: op_lt;
Int::op_lt_assign#Int: op_lt;
Int::op_lt_assign#Float: op_lt;
Float::op_lt_assign#Int: op_lt;
Float::op_lt_assign#Float: op_lt;

op_eq: method(v){ return this == v; }
Int::op_eq#Int: op_eq;
Int::op_eq#Float: op_eq;
Float::op_eq#Int: op_eq;
Float::op_eq#Float: op_eq;
Int::op_eq_assign#Int: op_eq;
Int::op_eq_assign#Float: op_eq;
Float::op_eq_assign#Int: op_eq;
Float::op_eq_assign#Float: op_eq;

	),
"\x78\x74\x61\x6c\x01\x00\x00\x00\x00\x00\x01\x7e\x89\x00\x01\x00\x0a\x0b\x05\x01\x5a\x25\x01\x25\x00\x3b\x00\x01\x39\x00\x02\x39\x00\x01\x37\x00\x01\x39\x00\x03\x39\x00\x01\x37\x00\x01\x89\x00\x02\x00\x0a\x0b\x05\x01\x5b\x25\x01\x25\x00\x3b\x00\x04\x39\x00"
"\x02\x39\x00\x04\x37\x00\x04\x39\x00\x03\x39\x00\x04\x37\x00\x04\x89\x00\x03\x00\x08\x0b\x54\x25\x01\x25\x00\x3b\x00\x05\x39\x00\x02\x39\x00\x05\x37\x00\x05\x39\x00\x03\x39\x00\x05\x37\x00\x05\x89\x00\x04\x00\x08\x0b\x55\x25\x01\x25\x00\x3b\x00\x06\x39\x00"
"\x02\x39\x00\x06\x37\x00\x06\x39\x00\x03\x39\x00\x06\x37\x00\x06\x89\x00\x05\x00\x08\x0b\x56\x25\x01\x25\x00\x3b\x00\x07\x39\x00\x02\x39\x00\x07\x37\x00\x07\x89\x00\x06\x00\x0a\x0b\x1d\x00\x68\x25\x01\x25\x00\x3b\x00\x09\x39\x00\x02\x39\x00\x09\x39\x00\x02"
"\x38\x00\x09\x39\x00\x02\x39\x00\x09\x39\x00\x03\x38\x00\x09\x39\x00\x03\x39\x00\x09\x39\x00\x02\x38\x00\x09\x39\x00\x03\x39\x00\x09\x39\x00\x03\x38\x00\x09\x39\x00\x02\x39\x00\x09\x39\x00\x02\x38\x00\x0a\x39\x00\x02\x39\x00\x09\x39\x00\x03\x38\x00\x0a\x39"
"\x00\x03\x39\x00\x09\x39\x00\x02\x38\x00\x0a\x39\x00\x03\x39\x00\x09\x39\x00\x03\x38\x00\x0a\x89\x00\x07\x00\x0a\x0b\x1d\x00\x66\x25\x01\x25\x00\x3b\x00\x0c\x39\x00\x02\x39\x00\x0c\x39\x00\x02\x38\x00\x0c\x39\x00\x02\x39\x00\x0c\x39\x00\x03\x38\x00\x0c\x39"
"\x00\x03\x39\x00\x0c\x39\x00\x02\x38\x00\x0c\x39\x00\x03\x39\x00\x0c\x39\x00\x03\x38\x00\x0c\x39\x00\x02\x39\x00\x0c\x39\x00\x02\x38\x00\x0d\x39\x00\x02\x39\x00\x0c\x39\x00\x03\x38\x00\x0d\x39\x00\x03\x39\x00\x0c\x39\x00\x02\x38\x00\x0d\x39\x00\x03\x39\x00"
"\x0c\x39\x00\x03\x38\x00\x0d\x25\x00\x8b\x00\x07\x05\x00\x00\x00\x00\x00\x01\x00\x00\x00\x37\x00\x27\x00\x00\x00\x00\x00\x04\x00\x00\x00\x36\x00\x49\x00\x00\x00\x00\x00\x05\x00\x00\x00\x36\x00\x69\x00\x00\x00\x00\x00\x06\x00\x00\x00\x36\x00\x89\x00\x00\x00"
"\x00\x00\x07\x00\x00\x00\x37\x00\xa0\x00\x00\x00\x00\x01\x09\x00\x00\x00\x36\x00\x10\x01\x00\x00\x00\x01\x0c\x00\x00\x00\x36\x00\x00\x00\x00\x08\x00\x00\x00\x00\x03\x06\x00\x00\x00\x00\xcc\xcc\x00\x01\x00\x00\x05\x00\x00\x00\x05\x00\x01\x00\x00\x00\x12\x00"
"\x00\x01\x00\x00\x27\x00\x00\x00\x05\x00\x04\x00\x00\x00\x12\x00\x00\x01\x00\x00\x49\x00\x00\x00\x05\x00\x05\x00\x00\x00\x12\x00\x00\x01\x00\x00\x69\x00\x00\x00\x05\x00\x06\x00\x00\x00\x12\x00\x00\x01\x00\x00\x89\x00\x00\x00\x05\x00\x07\x00\x00\x00\x12\x00"
"\x00\x01\x00\x00\xa0\x00\x00\x00\x05\x00\x08\x00\x01\x00\x12\x00\x00\x01\x01\x01\x10\x01\x00\x00\x05\x00\x0b\x00\x01\x00\x12\x00\x00\x01\x01\x01\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x22\x00\x00\x00\x00\x2b\x00\x00\x00\x00\x00\x00\x00"
"\x03\x00\x00\x00\x10\x00\x00\x00\x04\x00\x00\x00\x19\x00\x00\x00\x05\x00\x00\x00\x22\x00\x00\x00\x07\x00\x00\x00\x32\x00\x00\x00\x08\x00\x00\x00\x3b\x00\x00\x00\x09\x00\x00\x00\x44\x00\x00\x00\x0b\x00\x00\x00\x52\x00\x00\x00\x0c\x00\x00\x00\x5b\x00\x00\x00"
"\x0d\x00\x00\x00\x64\x00\x00\x00\x0f\x00\x00\x00\x72\x00\x00\x00\x10\x00\x00\x00\x7b\x00\x00\x00\x11\x00\x00\x00\x84\x00\x00\x00\x13\x00\x00\x00\x92\x00\x00\x00\x14\x00\x00\x00\x9b\x00\x00\x00\x17\x00\x00\x00\xab\x00\x00\x00\x18\x00\x00\x00\xb7\x00\x00\x00"
"\x19\x00\x00\x00\xc3\x00\x00\x00\x1a\x00\x00\x00\xcf\x00\x00\x00\x1b\x00\x00\x00\xdb\x00\x00\x00\x1c\x00\x00\x00\xe7\x00\x00\x00\x1d\x00\x00\x00\xf3\x00\x00\x00\x1e\x00\x00\x00\xff\x00\x00\x00\x1f\x00\x00\x00\x0b\x01\x00\x00\x21\x00\x00\x00\x1b\x01\x00\x00"
"\x22\x00\x00\x00\x27\x01\x00\x00\x23\x00\x00\x00\x33\x01\x00\x00\x24\x00\x00\x00\x3f\x01\x00\x00\x25\x00\x00\x00\x4b\x01\x00\x00\x26\x00\x00\x00\x57\x01\x00\x00\x27\x00\x00\x00\x63\x01\x00\x00\x28\x00\x00\x00\x6f\x01\x00\x00\x29\x00\x00\x00\x7b\x01\x00\x00"
"\x2b\x00\x00\x00\x00\x01\x0b\x00\x00\x00\x03\x09\x00\x00\x00\x06\x73\x6f\x75\x72\x63\x65\x09\x00\x00\x00\x11\x74\x6f\x6f\x6c\x2f\x74\x65\x6d\x70\x2f\x69\x6e\x2e\x78\x74\x61\x6c\x09\x00\x00\x00\x0b\x69\x64\x65\x6e\x74\x69\x66\x69\x65\x72\x73\x0a\x00\x00\x00"
"\x0e\x09\x00\x00\x00\x00\x09\x00\x00\x00\x06\x6f\x70\x5f\x69\x6e\x63\x09\x00\x00\x00\x03\x49\x6e\x74\x09\x00\x00\x00\x05\x46\x6c\x6f\x61\x74\x09\x00\x00\x00\x06\x6f\x70\x5f\x64\x65\x63\x09\x00\x00\x00\x06\x6f\x70\x5f\x70\x6f\x73\x09\x00\x00\x00\x06\x6f\x70"
"\x5f\x6e\x65\x67\x09\x00\x00\x00\x06\x6f\x70\x5f\x63\x6f\x6d\x09\x00\x00\x00\x01\x76\x09\x00\x00\x00\x05\x6f\x70\x5f\x6c\x74\x09\x00\x00\x00\x0c\x6f\x70\x5f\x6c\x74\x5f\x61\x73\x73\x69\x67\x6e\x02\x00\x00\x00\x0e\x09\x00\x00\x00\x05\x6f\x70\x5f\x65\x71\x09"
"\x00\x00\x00\x0c\x6f\x70\x5f\x65\x71\x5f\x61\x73\x73\x69\x67\x6e\x09\x00\x00\x00\x06\x76\x61\x6c\x75\x65\x73\x0a\x00\x00\x00\x01\x03"
)->call();

}

}
