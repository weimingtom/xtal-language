#include "xtal.h"
#include "xtal_macro.h"

namespace xtal{

Null null;
Nop nop;

namespace{

	int_t Int_to_i(const AnyPtr& p){
		if(type(p)==TYPE_INT){
			return ivalue(p);
		}

		XTAL_THROW(cast_error(p->get_class()->object_name(), "Int"), return 0);
	}

	float_t Int_to_f(const AnyPtr& p){
		if(type(p)==TYPE_INT){
			return (float_t)ivalue(p);
		}

		XTAL_THROW(cast_error(p->get_class()->object_name(), "Int"), return 0);
	}

	StringPtr Int_to_s(const AnyPtr& p){
		if(type(p)==TYPE_INT){
			return Xf("%d")(p)->to_s();
		}

		XTAL_THROW(cast_error(p->get_class()->object_name(), "Int"), return null);
	}

	int_t Float_to_i(const AnyPtr& p){
		if(type(p)==TYPE_FLOAT){
			return (int_t)fvalue(p);
		}

		XTAL_THROW(cast_error(p->get_class()->object_name(), "Float"), return 0);
	}

	float_t Float_to_f(const AnyPtr& p){
		if(type(p)==TYPE_FLOAT){
			return fvalue(p);
		}

		XTAL_THROW(cast_error(p->get_class()->object_name(), "Float"), return 0);
	}

	StringPtr Float_to_s(const AnyPtr& p){
		if(type(p)==TYPE_FLOAT){
			return Xf("%g")(p)->to_s();
		}

		XTAL_THROW(cast_error(p->get_class()->object_name(), "Float"), return null);
	}

	IntRangePtr range_Int(const AnyPtr& left, const AnyPtr& right, int_t kind){
		return xnew<IntRange>(left->to_i(), right->to_i(), kind);	
	}

	FloatRangePtr range_Float(const AnyPtr& left, const AnyPtr& right, int_t kind){
		return xnew<FloatRange>(left->to_f(), right->to_f(), kind);	
	}

	class IntRangeIter : public Base{
	public:

		IntRangeIter(const IntRangePtr& range)
			:it_(range->begin()), end_(range->end()){
		}

		void block_next(const VMachinePtr& vm){
			if(it_<end_){
				vm->return_result(SmartPtr<IntRangeIter>(this), it_);
				++it_;
			}else{
				vm->return_result(null);
			}
		}

	private:
		int_t it_, end_;
	};
}

AnyPtr IntRange::each(){
	return xnew<IntRangeIter>(IntRangePtr(this));
}



void initialize_basic_type(){

	builtin()->def("MissingNS", new_cpp_class<MissingNS>());
	builtin()->def("String", new_cpp_class<String>());
	builtin()->def("Int", new_cpp_class<Int>());
	builtin()->def("Float", new_cpp_class<Float>());
	builtin()->def("Null", new_cpp_class<Null>());
	builtin()->def("Nop", new_cpp_class<Nop>());
	builtin()->def("True", new_cpp_class<True>());
	builtin()->def("False", new_cpp_class<False>());
	builtin()->def("IntRange", new_cpp_class<IntRange>());
	builtin()->def("FloatRange", new_cpp_class<FloatRange>());
	builtin()->def("ChRange", new_cpp_class<ChRange>());

	{
		ClassPtr p = new_cpp_class<Int>("Int");	
		p->method("to_i", &Int_to_i);
		p->method("to_f", &Int_to_f);
		p->method("to_s", &Int_to_s);
		p->method("op_range", &range_Int, get_cpp_class<Int>());
		p->method("op_range", &range_Float, get_cpp_class<Float>());
	}

	{
		ClassPtr p = new_cpp_class<Float>("Float");
		p->method("to_i", &Float_to_i);
		p->method("to_f", &Float_to_f);
		p->method("to_s", &Float_to_s);
		p->method("op_range", &range_Float, get_cpp_class<Int>());
		p->method("op_range", &range_Float, get_cpp_class<Float>());
	}

	{
		ClassPtr p = new_cpp_class<IntRangeIter>("IntRangeIter");
		p->inherit(Iterator());
		p->method("block_next", &IntRangeIter::block_next);
	}

	{
		ClassPtr p = new_cpp_class<IntRange>("IntRange");
		p->inherit(Enumerator());
		p->def("new", ctor<IntRange, int_t, int_t, int_t>()->param(Named("left", null), Named("right", null), Named("kind", 0)));
		p->method("begin", &IntRange::begin);
		p->method("end", &IntRange::end);
		p->method("left", &IntRange::left);
		p->method("right", &IntRange::right);
		p->method("left_closed", &IntRange::left_closed);
		p->method("right_closed", &IntRange::right_closed);
		p->method("each", &IntRange::each);
	}

	{
		ClassPtr p = new_cpp_class<FloatRange>("FloatRange");
		p->inherit(Enumerator());
		p->def("new", ctor<FloatRange, float_t, float_t, int_t>()->param(Named("left", null), Named("right", null), Named("kind", 0)));
		p->method("left", &FloatRange::left);
		p->method("right", &FloatRange::right);
		p->method("left_closed", &FloatRange::left_closed);
		p->method("right_closed", &FloatRange::right_closed);
	}

}

}