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

	class RangeIter{
	public:

		RangeIter(const RangePtr& range)
			:it_(range->begin()), end_(range->end()){
			upto_ = true;
			first_ = true;
			last_ = false;
			type_ = 0;
			if(type(it_)==TYPE_INT && type(end_)==TYPE_INT){
				type_ = 1;
				upto_ = ivalue(it_) < ivalue(end_);
			}else if(type(it_)==TYPE_FLOAT && type(end_)==TYPE_FLOAT){
				type_ = 2;
				upto_ = fvalue(it_) < fvalue(end_);
			}
		}

		void block_next(const VMachinePtr& vm){
			if(type_==1){
				if(upto_){
					if(ivalue(it_) < ivalue(end_)){
						vm->return_result(SmartPtr<RangeIter>(this), it_);
						it_ = ivalue(it_) + 1;
					}else{
						vm->return_result(null);
					}
				}else{
					if(ivalue(it_) > ivalue(end_)){
						vm->return_result(SmartPtr<RangeIter>(this), it_);
						it_ = ivalue(it_) - 1;
					}else{
						vm->return_result(null);
					}
				}
			}else if(type_==2){
				if(upto_){
					if(fvalue(it_) < fvalue(end_)){
						vm->return_result(SmartPtr<RangeIter>(this), it_);
						it_ = fvalue(it_) + 1;
					}else{
						vm->return_result(null);
					}
				}else{
					if(fvalue(it_) > fvalue(end_)){
						vm->return_result(SmartPtr<RangeIter>(this), it_);
						it_ = fvalue(it_) - 1;
					}else{
						vm->return_result(null);
					}
				}
			}else{
				if(last_){
					vm->return_result(null);
					return;
				}

				if(first_){
					first_ = false;

					vm->setup_call(1, end_);
					it_->rawsend(vm, Xid(op_lt), end_->get_class());
					upto_ = vm->result_and_cleanup_call();
				}else{
					vm->setup_call(1);
					it_->rawsend(vm, upto_ ? Xid(op_inc) : Xid(op_dec));
					it_ = vm->result_and_cleanup_call();
				}

				if(upto_){
					vm->setup_call(1, end_);
					it_->rawsend(vm, Xid(op_lt), end_->get_class());
				}else{
					vm->setup_call(1, it_);
					end_->rawsend(vm, Xid(op_lt), it_->get_class());
				}

				if(vm->result_and_cleanup_call()){
					vm->return_result(SmartPtr<RangeIter>(this), it_);
				}else{
					last_ = true;
					vm->return_result(null);
				}
			}
		}

	private:
		AnyPtr it_, end_;
		bool upto_, first_, last_;
		byte_t type_;
	};
}

Range::Range(const AnyPtr& begin, const AnyPtr& end, bool exclude)
	:begin_(begin), end_(end){
	if(exclude){
		end_ = end_->send(Xid(op_inc));	
	}
}

AnyPtr Range::each(){
	return xnew<RangeIter>(RangePtr(this));
}



void initialize_basic_type(){
	{
		ClassPtr p = new_cpp_class<Int>("Int");	
		p->method("to_i", &Int_to_i);
		p->method("to_f", &Int_to_f);
		p->method("to_s", &Int_to_s);
	}

	{
		ClassPtr p = new_cpp_class<Float>("Float");
		p->method("to_i", &Float_to_i);
		p->method("to_f", &Float_to_f);
		p->method("to_s", &Float_to_s);
	}

	{
		ClassPtr p = new_cpp_class<RangeIter>("RangeIter");
		p->inherit(Iterator());
		p->method("block_next", &RangeIter::block_next);
	}

	{
		ClassPtr p = new_cpp_class<Range>("Range");
		p->inherit(Enumerator());
		p->def("new", ctor<Range, const AnyPtr&, const AnyPtr&, bool>()->param(Named("begin", null), Named("end", null), Named("exclude", false)));
		p->method("begin", &Range::begin);
		p->method("end", &Range::end);
		p->method("each", &Range::each);
	}


	builtin()->def("MissingNS", new_cpp_class<MissingNS>());
	builtin()->def("String", new_cpp_class<String>());
	builtin()->def("Int", new_cpp_class<Int>());
	builtin()->def("Float", new_cpp_class<Float>());
	builtin()->def("Null", new_cpp_class<Null>());
	builtin()->def("Nop", new_cpp_class<Nop>());
	builtin()->def("True", new_cpp_class<True>());
	builtin()->def("False", new_cpp_class<False>());
	builtin()->def("Range", new_cpp_class<Range>());
}

}
