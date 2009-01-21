#include "xtal.h"

namespace xtal{

namespace{

AnyPtr abs(const AnyPtr& a){
	switch(type(a)){XTAL_DEFAULT;
		XTAL_CASE(TYPE_INT){ return ivalue(a)<0 ? -ivalue(a) : a; }
		XTAL_CASE(TYPE_FLOAT){ return fvalue(a)<0 ? -fvalue(a) : a; }
	}
	return 0;
}

AnyPtr max_(const AnyPtr& a, const AnyPtr& b){
	switch(type(a)){XTAL_DEFAULT;
		XTAL_CASE(TYPE_INT){
			switch(type(b)){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ return ivalue(a)<ivalue(b) ? ivalue(b) : ivalue(a); }
				XTAL_CASE(TYPE_FLOAT){ return ivalue(a)<fvalue(b) ? fvalue(b) : ivalue(a); }
			}
		}
		XTAL_CASE(TYPE_FLOAT){
			switch(type(b)){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ return fvalue(a)<ivalue(b) ? ivalue(b) : fvalue(a); }
				XTAL_CASE(TYPE_FLOAT){ return fvalue(a)<fvalue(b) ? fvalue(b) : fvalue(a); }
			}
		}
	}
	return null;
}

AnyPtr min_(const AnyPtr& a, const AnyPtr& b){
	switch(type(a)){XTAL_DEFAULT;
		XTAL_CASE(TYPE_INT){
			switch(type(b)){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ return ivalue(a)<ivalue(b) ? ivalue(a) : ivalue(b); }
				XTAL_CASE(TYPE_FLOAT){ return ivalue(a)<fvalue(b) ? fvalue(a) : ivalue(b); }
			}
		}
		XTAL_CASE(TYPE_FLOAT){
			switch(type(b)){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ return fvalue(a)<ivalue(b) ? ivalue(a) : fvalue(b); }
				XTAL_CASE(TYPE_FLOAT){ return fvalue(a)<fvalue(b) ? fvalue(a) : fvalue(b); }
			}
		}
	}
	return null;
}

class Random{
public:

	Random(int_t seed=std::time(0)){ 
		set_seed(seed); 
	}
	
	void set_seed(int_t seed){
		idum_=seed==0 ? 1 : seed; 
	} 

	float_t operator ()(){
		int_t idum = idum_;
		int_t k = idum/IQ;
		idum = IA*(idum-k*IQ)-IR*k;
		idum &= IM;
		idum_ = idum;
		return ((float_t)1.0/IM)*idum;
	}
	
	float_t operator ()(float_t max){
		return (*this)()*max; 
	}

	const float_t operator ()(float_t min, float_t max){
		return (*this)()*(max-min)+min; 
	}	

private:
	enum{ IA=16807,IM=2147483647,IQ=127773,IR=2836 };
	int_t idum_;
};
	
Random random_instance;

float_t random(){
	return random_instance();
}

float_t random_range(float_t in, float_t ax){
	return random_instance(in, ax);
}

}

template<class T>
struct Math{
	static ClassPtr make(float*){
		using namespace std;

		ClassPtr math = xnew<Singleton>("math");
		
		math->fun("acos", (float (*)(float))&acosf);
		math->fun("asin", (float (*)(float))&asinf);
		math->fun("atan", (float (*)(float))&atanf);
		math->fun("atan2", (float (*)(float, float))&atan2f);
		math->fun("ceil", (float (*)(float))&ceilf);
		math->fun("cos", (float (*)(float))&cosf);
		math->fun("exp", (float (*)(float))&expf);
		math->fun("floor", (float (*)(float))&floorf);
		math->fun("log", (float (*)(float))&logf);
		math->fun("pow", (float (*)(float, float))&powf);
		math->fun("sin", (float (*)(float))&sinf);
		math->fun("sqrt", (float (*)(float))&sqrtf);
		math->fun("tan", (float (*)(float))&tanf);
		math->def("PI", (float_t)3.14159265358979);
		math->def("E", (float_t)2.71828182845905);

		return math;
	}

	static ClassPtr make(double*){
		using namespace std;

		ClassPtr math = xnew<Singleton>("math");

		math->fun("acos", (double (*)(double))&acos);
		math->fun("asin", (double (*)(double))&asin);
		math->fun("atan", (double (*)(double))&atan);
		math->fun("atan2", (double (*)(double, double))&atan2);
		math->fun("ceil", (double (*)(double))&ceil);
		math->fun("cos", (double (*)(double))&cos);
		math->fun("exp", (double (*)(double))&exp);
		math->fun("floor", (double (*)(double))&floor);
		math->fun("log", (double (*)(double))&log);
		math->fun("pow", (double (*)(double, double))&pow);
		math->fun("sin", (double (*)(double))&sin);
		math->fun("sqrt", (double (*)(double))&sqrt);
		math->fun("tan", (double (*)(double))&tan);
		math->def("PI", (float_t)3.14159265358979);
		math->def("E", (float_t)2.71828182845905);

		return math;
	}
};

void initialize_math(){
	ClassPtr math(Math<float_t>::make((float_t*)0));
	math->fun("abs", &abs);
	math->fun("max", &max_);
	math->fun("min", &min_);
	math->fun("random", &random);
	math->fun("random_range", &random_range);
	builtin()->def("math", math);
}

}
