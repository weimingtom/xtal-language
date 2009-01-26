#include "xtal.h"
#include "xtal_macro.h"

namespace xtal{

Null null;
Undefined undefined;
Named null_named;
IDPtr empty_id((IDPtr&)Innocence(TYPE_SMALL_STRING));

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

void initialize_basic_type(){

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
		p->def(Xid(new), ctor<Range, const AnyPtr&, const AnyPtr&, int_t>()->param(Named(Xid(left), null), Named(Xid(right), null), Named(Xid(kind), 0)));
		p->method(Xid(left), &Range::left);
		p->method(Xid(right), &Range::right);
		p->method(Xid(kind), &Range::kind);
	}

	{
		ClassPtr p = new_cpp_class<IntRange>(Xid(IntRange));
		p->inherit(get_cpp_class<Range>());
		p->def(Xid(new), ctor<IntRange, int_t, int_t, int_t>()->param(Named(Xid(left), null), Named(Xid(right), null), Named(Xid(kind), 0)));
		p->method(Xid(begin), &IntRange::begin);
		p->method(Xid(end), &IntRange::end);
		p->method(Xid(each), &IntRange::each);
	}

	{
		ClassPtr p = new_cpp_class<FloatRange>(Xid(FloatRange));
		p->inherit(get_cpp_class<Range>());
		p->def(Xid(new), ctor<FloatRange, float_t, float_t, int_t>()->param(Named(Xid(left), null), Named(Xid(right), null), Named(Xid(kind), 0)));
	}

}

}
