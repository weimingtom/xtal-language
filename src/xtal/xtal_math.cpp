#include "xtal.h"
#include "xtal_macro.h"

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

		ClassPtr math = xnew<Singleton>(Xid(math));
		
		math->def_fun(Xid(acos), (float (*)(float))&acosf);
		math->def_fun(Xid(asin), (float (*)(float))&asinf);
		math->def_fun(Xid(atan), (float (*)(float))&atanf);
		math->def_fun(Xid(atan2), (float (*)(float, float))&atan2f);
		math->def_fun(Xid(ceil), (float (*)(float))&ceilf);
		math->def_fun(Xid(cos), (float (*)(float))&cosf);
		math->def_fun(Xid(exp), (float (*)(float))&expf);
		math->def_fun(Xid(floor), (float (*)(float))&floorf);
		math->def_fun(Xid(log), (float (*)(float))&logf);
		math->def_fun(Xid(pow), (float (*)(float, float))&powf);
		math->def_fun(Xid(sin), (float (*)(float))&sinf);
		math->def_fun(Xid(sqrt), (float (*)(float))&sqrtf);
		math->def_fun(Xid(tan), (float (*)(float))&tanf);
		math->def(Xid(PI), (float_t)3.14159265358979);
		math->def(Xid(E), (float_t)2.71828182845905);

		return math;
	}

	static ClassPtr make(double*){
		using namespace std;

		ClassPtr math = xnew<Singleton>(Xid(math));

		math->def_fun(Xid(acos), (double (*)(double))&acos);
		math->def_fun(Xid(asin), (double (*)(double))&asin);
		math->def_fun(Xid(atan), (double (*)(double))&atan);
		math->def_fun(Xid(atan2), (double (*)(double, double))&atan2);
		math->def_fun(Xid(ceil), (double (*)(double))&ceil);
		math->def_fun(Xid(cos), (double (*)(double))&cos);
		math->def_fun(Xid(exp), (double (*)(double))&exp);
		math->def_fun(Xid(floor), (double (*)(double))&floor);
		math->def_fun(Xid(log), (double (*)(double))&log);
		math->def_fun(Xid(pow), (double (*)(double, double))&pow);
		math->def_fun(Xid(sin), (double (*)(double))&sin);
		math->def_fun(Xid(sqrt), (double (*)(double))&sqrt);
		math->def_fun(Xid(tan), (double (*)(double))&tan);
		math->def(Xid(PI), (float_t)3.14159265358979);
		math->def(Xid(E), (float_t)2.71828182845905);

		return math;
	}
};

void initialize_math(){
	ClassPtr math(Math<float_t>::make((float_t*)0));
	math->def_fun(Xid(abs), &abs);
	math->def_fun(Xid(max), &max_);
	math->def_fun(Xid(min), &min_);
	math->def_fun(Xid(random), &random);
	math->def_fun(Xid(random_range), &random_range);
	builtin()->def(Xid(math), math);
}

}
