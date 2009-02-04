#include "xtal.h"
#include "xtal_macro.h"

namespace xtal{

Null null;
Undefined undefined;
Named null_named;
IDPtr empty_id(unchecked_ptr_cast<ID>(ap(Any(TYPE_SMALL_STRING))));

FunInfo empty_xfun_info;
ScopeInfo empty_scope_info;
ClassInfo empty_class_info;
ExceptInfo empty_except_info;
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


StringPtr HaveName::object_name(int_t depth){
	if(!name_){
		return xnew<String>("instance of ")->cat(get_class()->object_name(depth));
	}

	if(!parent_ || depth==0){
		return name_;
	}

	return parent_->object_name()->cat("::")->cat(name_);
}

ArrayPtr HaveName::object_name_list(){
	if(!name_){
		return null;
	}

	ArrayPtr ret = parent_->object_name_list();
	if(ret){
		ret->push_back(name_);
	}

	return ret;
}

int_t HaveName::object_name_force(){
	return force_;
}

void HaveName::set_object_name(const StringPtr& name, int_t force, const AnyPtr& parent){
	if(!name_ || force_<force){
		name_ = name;
		force_ = force;
		parent_ = parent;
	}
}

GCObserver::GCObserver(){
	core()->register_gc_observer(this);
}

GCObserver::GCObserver(const GCObserver& v)
:Base(v){
	core()->register_gc_observer(this);
}
	
GCObserver::~GCObserver(){
	core()->unregister_gc_observer(this);
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
		p->def_method(Xid(to_mv), &to_mv_Undefined);
	}
	
	{
		ClassPtr p = new_cpp_class<Int>(Xid(Int));	
		p->def_method(Xid(to_i), &Int_to_i);
		p->def_method(Xid(to_f), &Int_to_f);
		p->def_method(Xid(to_s), &Int_to_s);
		p->def_method(Xid(op_range), &op_range_Int, get_cpp_class<Int>());
		p->def_method(Xid(op_range), &op_range_Float, get_cpp_class<Float>());
		p->def_method(Xid(op_in), &op_in_Int_IntRange, get_cpp_class<IntRange>());
		p->def_method(Xid(op_in), &op_in_Float_FloatRange, get_cpp_class<FloatRange>());
	}

	{
		ClassPtr p = new_cpp_class<Float>(Xid(Float));
		p->def_method(Xid(to_i), &Float_to_i);
		p->def_method(Xid(to_f), &Float_to_f);
		p->def_method(Xid(to_s), &Float_to_s);
		p->def_method(Xid(op_range), &op_range_Float, get_cpp_class<Int>());
		p->def_method(Xid(op_range), &op_range_Float, get_cpp_class<Float>());
		p->def_method(Xid(op_in), &op_in_Int_IntRange, get_cpp_class<IntRange>());
		p->def_method(Xid(op_in), &op_in_Float_FloatRange, get_cpp_class<FloatRange>());
	}

	{
		ClassPtr p = new_cpp_class<IntRangeIter>(Xid(IntRangeIter));
		p->inherit(Iterator());
		p->def_method(Xid(block_next), &IntRangeIter::block_next);
	}

	{
		ClassPtr p = new_cpp_class<Range>(Xid(Range));
		p->inherit(Iterable());
		p->def(Xid(new), ctor<Range, const AnyPtr&, const AnyPtr&, int_t>()->params(Xid(left), null, Xid(right), null, Xid(kind), 0));
		p->def_method(Xid(left), &Range::left);
		p->def_method(Xid(right), &Range::right);
		p->def_method(Xid(kind), &Range::kind);
	}

	{
		ClassPtr p = new_cpp_class<IntRange>(Xid(IntRange));
		p->inherit(get_cpp_class<Range>());
		p->def(Xid(new), ctor<IntRange, int_t, int_t, int_t>()->params(Xid(left), null, Xid(right), null, Xid(kind), 0));
		p->def_method(Xid(begin), &IntRange::begin);
		p->def_method(Xid(end), &IntRange::end);
		p->def_method(Xid(each), &IntRange::each);
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
"\x78\x74\x61\x6c\x01\x00\x00\x00\x00\x00\x00\x4b\x39\x00\x01\x89\x00\x01\x00\x0f\x0b\x2f\x00\x00\x00\x00\x00\x02\x0b\x25\x01\x25\x00\x37\x00\x03\x39\x00\x01\x89\x00\x02\x00\x0f\x0b\x2f\x00\x00\x00\x00\x00\x04\x01\x25\x01\x25\x00\x37\x00\x05\x39\x00\x01\x89"
"\x00\x03\x00\x0f\x0b\x2f\x00\x00\x00\x00\x00\x04\x01\x25\x01\x25\x00\x37\x00\x06\x25\x00\x8b\x00\x03\x08\x00\x00\x00\x00\x00\x02\x00\x00\x00\x12\x00\x20\x00\x00\x00\x00\x00\x04\x00\x00\x00\x12\x00\x38\x00\x00\x00\x00\x00\x06\x00\x00\x00\x12\x00\x00\x00\x00"
"\x04\x00\x00\x00\x00\x03\x06\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x08\x00\x00\x00\x05\x00\x02\x00\x00\x00\x00\x00\x00\x01\x00\x00\x20\x00\x00\x00\x05\x00\x04\x00\x00\x00\x00\x00\x00\x01\x00\x00\x38\x00\x00\x00\x05\x00\x06\x00\x00\x00\x00\x00\x00\x01\x00"
"\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x14\x00\x00\x00\x00\x11\x00\x00\x00\x00\x00\x00\x00\x06\x00\x00\x00\x00\x00\x00\x00\x03\x00\x00\x00\x03\x00\x00\x00\x06\x00\x00\x00\x08\x00\x00\x00\x04\x00\x00\x00\x10\x00\x00\x00\x05\x00\x00"
"\x00\x13\x00\x00\x00\x06\x00\x00\x00\x18\x00\x00\x00\x0b\x00\x00\x00\x18\x00\x00\x00\x08\x00\x00\x00\x1b\x00\x00\x00\x0b\x00\x00\x00\x20\x00\x00\x00\x09\x00\x00\x00\x28\x00\x00\x00\x0a\x00\x00\x00\x2b\x00\x00\x00\x0b\x00\x00\x00\x30\x00\x00\x00\x10\x00\x00"
"\x00\x30\x00\x00\x00\x0d\x00\x00\x00\x33\x00\x00\x00\x10\x00\x00\x00\x38\x00\x00\x00\x0e\x00\x00\x00\x40\x00\x00\x00\x0f\x00\x00\x00\x43\x00\x00\x00\x10\x00\x00\x00\x48\x00\x00\x00\x11\x00\x00\x00\x00\x01\x0b\x00\x00\x00\x03\x09\x00\x00\x00\x06\x73\x6f\x75"
"\x72\x63\x65\x09\x00\x00\x00\x11\x74\x6f\x6f\x6c\x2f\x74\x65\x6d\x70\x2f\x69\x6e\x2e\x78\x74\x61\x6c\x09\x00\x00\x00\x0b\x69\x64\x65\x6e\x74\x69\x66\x69\x65\x72\x73\x0a\x00\x00\x00\x07\x09\x00\x00\x00\x00\x09\x00\x00\x00\x05\x4d\x75\x74\x65\x78\x09\x00\x00"
"\x00\x04\x6c\x6f\x63\x6b\x09\x00\x00\x00\x0b\x62\x6c\x6f\x63\x6b\x5f\x66\x69\x72\x73\x74\x09\x00\x00\x00\x06\x75\x6e\x6c\x6f\x63\x6b\x09\x00\x00\x00\x0a\x62\x6c\x6f\x63\x6b\x5f\x6e\x65\x78\x74\x09\x00\x00\x00\x0b\x62\x6c\x6f\x63\x6b\x5f\x62\x72\x65\x61\x6b"
"\x09\x00\x00\x00\x06\x76\x61\x6c\x75\x65\x73\x0a\x00\x00\x00\x01\x03"
)->call();

}

}
