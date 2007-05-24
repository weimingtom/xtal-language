#include "xtal.h"

#include <iostream>
#include <iomanip>

#include <cmath>
#include <stdio.h>

#include "codebuilder.h"
#include "fun.h"
#include "codeimpl.h"
#include "xmacro.h"
#include "marshal.h"
#include "frameimpl.h"
#include "constant.h"

//#include "message_jp_sjis.txt"

namespace xtal{

#ifndef XTAL_NO_PARSER

Any compile_file(const String& file_name){
	CodeBuilder cb;
	FileStream fs(file_name, "r");
	if(Any fun = cb.compile(fs, file_name)){
		fs.close();
		return fun;
	}
	fs.close();
	XTAL_THROW(builtin().member("CompileError")(Xt("Xtal Runtime Error 1016")(file_name), cb.errors()));
	return null;
}

Any compile(const String& source){
	CodeBuilder cb;
	StringStream ms(source.c_str(), source.size());
	if(Any fun =  cb.compile(ms, "<eval>")){
		return fun;
	}
	XTAL_THROW(builtin().member("CompileError")(Xt("Xtal Runtime Error 1002")(), cb.errors()));
	return null;
}

Any load(const String& file_name){
	Any ret = compile_file(file_name)();
	full_gc();
	return ret;
}

Any load_and_save(const String& file_name){
	Any ret = compile_file(file_name);
	FileStream fs(file_name.cat("c"), "w");
	object_dump(ret, fs);
	fs.close();
	full_gc();
	return ret();
}

Any source(const char* src, int_t size, const char* file){
	CodeBuilder cb;
	StringStream ms(src, size);
	if(Any fun = cb.compile(ms, file)){
		return fun;
	}
	XTAL_THROW(builtin().member("CompileError")(Xt("Xtal Runtime Error 1010")(), cb.errors()));
	return null;
}

void ix(){
	CodeBuilder cb;
	cb.interactive_compile();
}

#else

Any compile_file(const String& file_name){
	throw unsupported_error("", "");
}

Any compile(const String& source){
	throw unsupported_error("", "");
}

Any load(const String& file_name){
	throw unsupported_error("", "");
}

Any load_and_save(const String& file_name){
	throw unsupported_error("", "");
}

Any source(const char* src, int_t size, const char* file){
	throw unsupported_error("", "");
}

void ix(){
	throw unsupported_error("", "");
}

#endif

void print(const VMachine& vm){
	for(int_t i = 0; i<vm.ordered_arg_count(); ++i){
		String str = vm.arg(i).to_s();
		std::cout << str.c_str();
	}	
	vm.return_result();
}

void println(const VMachine& vm){
	print(vm);
	std::cout << std::endl;
}

void object_dump(const Any& v, const Stream& out){
	Marshal m(out);
	m.dump(v);
}

Any object_load(const Stream& in){
	Marshal m(in);
	return m.load();
}

void object_to_script(const Any& v, const Stream& out){
	Marshal m(out);
	m.to_script(v);
}

Result result;
ReturnThis return_this;
ReturnVoid return_void;


static Any abs(const Any& a){
	switch(a.type()){XTAL_DEFAULT;
		XTAL_CASE(TYPE_INT){ return a.ivalue()<0 ? -a.ivalue() : a; }
		XTAL_CASE(TYPE_FLOAT){ return a.fvalue()<0 ? -a.fvalue() : a; }
	}
	return 0;
}

static Any max_(const Any& a, const Any& b){
	switch(a.type()){XTAL_DEFAULT;
		XTAL_CASE(TYPE_INT){
			switch(b.type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ return a.ivalue()<b.ivalue() ? b.ivalue() : a.ivalue(); }
				XTAL_CASE(TYPE_FLOAT){ return a.ivalue()<b.fvalue() ? b.fvalue() : a.ivalue(); }
			}
		}
		XTAL_CASE(TYPE_FLOAT){
			switch(b.type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ return a.fvalue()<b.ivalue() ? b.ivalue() : a.fvalue(); }
				XTAL_CASE(TYPE_FLOAT){ return a.fvalue()<b.fvalue() ? b.fvalue() : a.fvalue(); }
			}
		}
	}
	return a<b ? b : a;
}

static Any min_(const Any& a, const Any& b){
	switch(a.type()){XTAL_DEFAULT;
		XTAL_CASE(TYPE_INT){
			switch(b.type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ return a.ivalue()<b.ivalue() ? a.ivalue() : b.ivalue(); }
				XTAL_CASE(TYPE_FLOAT){ return a.ivalue()<b.fvalue() ? a.fvalue() : b.ivalue(); }
			}
		}
		XTAL_CASE(TYPE_FLOAT){
			switch(b.type()){XTAL_DEFAULT;
				XTAL_CASE(TYPE_INT){ return a.fvalue()<b.ivalue() ? a.ivalue() : b.fvalue(); }
				XTAL_CASE(TYPE_FLOAT){ return a.fvalue()<b.fvalue() ? a.fvalue() : b.fvalue(); }
			}
		}
	}
	return a<b ? a : b;
}

class Random{
public:

	Random(int_t seed=1){ 
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
	
static Random random_instance;

static float_t random(){
	return random_instance();
}

static float_t random_range(float_t in, float_t ax){
	return random_instance(in, ax);
}

template<class T>
struct Math{
	static Class make(float*){
		using namespace std;

		Class math("math");
		
		math.fun("acos", (float (*)(float))&acosf);
		math.fun("asin", (float (*)(float))&asinf);
		math.fun("atan", (float (*)(float))&atanf);
		math.fun("atan2", (float (*)(float, float))&atan2f);
		math.fun("ceil", (float (*)(float))&ceilf);
		math.fun("cos", (float (*)(float))&cosf);
		math.fun("exp", (float (*)(float))&expf);
		math.fun("floor", (float (*)(float))&floorf);
		math.fun("log", (float (*)(float))&logf);
		math.fun("pow", (float (*)(float, float))&powf);
		math.fun("sin", (float (*)(float))&sinf);
		math.fun("sqrt", (float (*)(float))&sqrtf);
		math.fun("tan", (float (*)(float))&tanf);
		math.def("PI", (float_t)3.14159265358979);
		math.def("E", (float_t)2.71828182845905);

		return math;
	}

	static Class make(double*){
		using namespace std;

		Class math("math");

		math.fun("acos", (double (*)(double))&acos);
		math.fun("asin", (double (*)(double))&asin);
		math.fun("atan", (double (*)(double))&atan);
		math.fun("atan2", (double (*)(double))&atan2);
		math.fun("ceil", (double (*)(double))&ceil);
		math.fun("cos", (double (*)(double))&cos);
		math.fun("exp", (double (*)(double))&exp);
		math.fun("floor", (double (*)(double))&floor);
		math.fun("log", (double (*)(double))&log);
		math.fun("pow", (double (*)(double))&pow);
		math.fun("sin", (double (*)(double))&sin);
		math.fun("sqrt", (double (*)(double))&sqrt);
		math.fun("tan", (double (*)(double))&tan);
		math.def("PI", (float_t)3.14159265358979);
		math.def("E", (float_t)2.71828182845905);

		return math;
	}
};

class Int : public Any{
public:
	int_t to_i(){ return ivalue(); }
	float_t to_f(){ return static_cast<float_t>(ivalue()); }
	String to_s(){
		char buf[256];
		sprintf(buf, "%d", ivalue());
		return String(buf);
	}
};

void InitInt(){
	TClass<Int> p("Int");
	
	p.method("to_i", &Int::to_i);
	p.method("to_f", &Int::to_f);
	p.method("to_s", &Int::to_s);
}

class Float : public Any{
public:
	int_t to_i(){ return static_cast<int_t>(fvalue()); }
	float_t to_f(){ return fvalue(); }
	String to_s(){
		char buf[256];
		sprintf(buf, "%g", fvalue());
		return String(buf);
	}
};

void InitFloat(){
	TClass<Float> p("Float");
	
	p.method("to_i", &Float::to_i);
	p.method("to_f", &Float::to_f);
	p.method("to_s", &Float::to_s);
}
	
class ZipIterImpl : public AnyImpl{
public:

	ZipIterImpl(const VMachine& vm)
		:next(vm.ordered_arg_count()), value(null){
		set_class(TClass<ZipIterImpl>::get());
		for(int_t i = 0, len = next.size(); i<len; ++i){
			next.set_at(i, vm.arg(i));
		}
	}

	void common(const VMachine& vm, const ID& id){
		bool all = true;
		value = Array(next.size());
		for(int_t i = 0, len = next.size(); i<len; ++i){
			vm.setup_call(2);
			next.at(i).send(id, vm);
			next.set_at(i, vm.result(0));
			value.set_at(i, vm.result(1));
			vm.cleanup_call();
			if(!next.at(i))
				all = false;
		}
		if(all){
			vm.return_result(this, value);
		}else{
			vm.return_result(null);
		}
	}

	void iter_first(const VMachine& vm){
		common(vm, Xid(iter_first));
	}
	
	void iter_next(const VMachine& vm){
		common(vm, Xid(iter_next));
	}

	void iter_break(const VMachine& vm){
		ID id = Xid(iter_break);
		for(int_t i = 0, len = next.size(); i<len; ++i){
			vm.setup_call(0);
			next.at(i).send(id, vm);
			if(!vm.processed()){
				vm.return_result();	
			}
			vm.cleanup_call();
		}
		vm.return_result();
	}

	virtual void visit_members(Visitor& m){
		AnyImpl::visit_members(m);
		m & next & value;
	}

	Array next;
	Array value;
};

void InitZipIter(){
	TClass<ZipIterImpl> cls;
	cls.inherit(Iterator());
	cls.def("new", New<ZipIterImpl, const VMachine&>());
	cls.method("iter_first", &ZipIterImpl::iter_next);
	cls.method("iter_next", &ZipIterImpl::iter_next);
	cls.method("iter_break", &ZipIterImpl::iter_break);
}

IterBreaker::~IterBreaker(){
	const VMachine& vm = vmachine();
	vm.setup_call(0);
	target.send(Xid(iter_break), vm);
	if(!vm.processed()){
		vm.return_result();
	}
	vm.cleanup_call();
}

void iter_next(Any& target, Any& value1, bool first){
	const VMachine& vm = vmachine();
	vm.setup_call(2);
	target.send(first ? Xid(iter_first) : Xid(iter_next), vm);
	target = vm.result(0);
	value1 = vm.result(1);
	vm.cleanup_call();
}

void iter_next(Any& target, Any& value1, Any& value2, bool first){
	const VMachine& vm = vmachine();
	vm.setup_call(3);
	target.send(first ? Xid(iter_first) : Xid(iter_next), vm);
	target = vm.result(0);
	value1 = vm.result(1);
	value2 = vm.result(2);
	vm.cleanup_call();
}

void iter_next(Any& target, Any& value1, Any& value2, Any& value3, bool first){
	const VMachine& vm = vmachine();
	vm.setup_call(4);
	target.send(first ? Xid(iter_first) : Xid(iter_next), vm);
	target = vm.result(0);
	value1 = vm.result(1);
	value2 = vm.result(2);
	value3 = vm.result(3);
	vm.cleanup_call();
}

const Class& Iterator(){
	static LLVar<Class> p("Iterator");
	return p;
}

const Class& Enumerable(){
	static LLVar<Class> p("Enumerable");
	return p;
}

const Class& builtin(){
	static LLVar<Class> p("builtin");
	return p;
}

const Any& lib(){
	static LLVar<Any> lib;
	if(!lib){ new(lib) LibImpl(); }
	return lib;
}

class NopClassImpl : public ClassImpl{
public:
	virtual const Any& member(const ID& name){
		return nop();
	}
	virtual void call(const VMachine& vm){
		vm.return_result(nop());
	}
	virtual void def(const ID& name, const Any& v){
	}
};

class NopImpl : public AnyImpl{
public:

	NopImpl(){
		set_class(TClass<Nop>::get());
	}

	virtual void call(const VMachine& vm){
		vm.return_result(nop());
	}
	
	virtual const Any& member(const ID& name){
		return nop();
	}

	virtual void def(const ID& name, const Any& v){
	}
};

const Any& nop(){
	static LLVar<Any> obj;
	if(!obj){
		TClass<Nop> p(null);
		new(p) NopClassImpl();
		TClass<Nop>::set(p);
		p.set_object_name("Nop", 1, null);
		new(obj) NopImpl();
	}
	return obj;
}

Any bad_cast_error(const Any& from, const Any& to){
	return builtin().member("BadCastError")(Xt("Xtal Runtime Error 1004")(
		Xid(type)=from.get_class().object_name(), Xid(required)=to
	));
}

Any invalid_argument_error(const Any& from, const Any& to, int param_num, const Any& param_name){
	if(param_name.to_b()){
		return builtin().member("InvalidArgumentError")(Xt("Xtal Runtime Error 1001")(
			Xid(n)=param_num+1, 
			Xid(param_name)=param_name, 
			Xid(type)=from.get_class().object_name(), 
			Xid(required)=to
		));
	}else{
		return builtin().member("InvalidArgumentError")(Xt("Xtal Runtime Error 1001")(
			Xid(n)=param_num+1, 
			Xid(param_name)=String(""), 
			Xid(type)=from.get_class().object_name(), 
			Xid(required)=to
		));	}
}

Any unsupported_error(const Any& name, const Any& member){
	return builtin().member("UnsupportedError")(Xt("Xtal Runtime Error 1015")(
		Xid(object)=name, Xid(name)=(member ? member : Any("()"))
	));
}

namespace{
	void default_except_handler(const Any& except, const char* file, int line){
#ifdef XTAL_NO_EXCEPT
		printf("%s(%d):%s\n", file, line, except.to_s().c_str());
		exit(1);
#endif
	}

	except_handler_t except_handler_ = &default_except_handler;
	Any except_;
}

except_handler_t except_handler(){
	return except_handler_;
}

void set_except_handler(except_handler_t handler){
	except_handler_ = handler;
}

Any except(){
	Any ret = except_;
	except_ = null;
	return ret;
}

void set_except(const Any& except){
	static bool init = false;
	if(!init){
		init = true;
		add_long_life_var(&except_);
	}
	except_ = except;
}

struct FormatString : public AnyImpl{

	enum{ BUF_MAX = 14, REAL_BUF_MAX = BUF_MAX + 2 };
	
	char buf_[REAL_BUF_MAX];
	int_t code_pos_, width_, precision_;
	
public:

	FormatString(const char*& str){
		code_pos_ = 0;
		buf_[code_pos_++] = '%';
		width_ = 0;
		precision_ = 0;
		
		parse_fmt(str);
	
		buf_[code_pos_] = str[0]; 
		buf_[code_pos_+1] = '\0';
		
		if(str[0]){
			++str;
		}

	}
	
	void change_int_code(){
		switch(code()){
		case 'i': case 'd': case 'x': case 'X':
			break;
		
		default:
			set_code('i');
			break;
		}
	}
	
	void change_float_code(){
		switch(code()){
		case 'e': case 'E': case 'g':
		case 'G': case 'f':
			break;
		
		default:
			set_code('g');
			break;
		}
	}
	
	void change_string_code(){
		set_code('s');
	}
	
	int_t code(){
		return buf_[code_pos_];
	}
	
	void set_code(int_t v){
		buf_[code_pos_] = v;
	}
	
	int_t max_buf_size(){
		return width_ + precision_;
	}
	
	const char* format_string(){
		return buf_;
	}
	
private:

	void parse_fmt(const char*& str){
		while(str[0]){
			switch(str[0]){
			case '-': case '+': case '0': case ' ': case '#':
				buf_[code_pos_++] = *str++;
				if(code_pos_ == BUF_MAX){
					return;
				}
				continue;
			}
			break;
		}
		
		width_ = 0;
		while(str[0]){
			if(str[0]>='0' && str[0]<= '9'){
				width_ *= 10;
				width_ += str[0]-'0';
				buf_[code_pos_++] = *str++; 
				if(code_pos_ == BUF_MAX){
					return;
				}
			}else{
				break;
			}
		}

		if(str[0]=='.'){	
			buf_[code_pos_++] = *str++; 
			if(code_pos_ == BUF_MAX){
				return;
			}
			
			precision_ = 0;
			while(str[0]){
				if(str[0]>='0' && str[0]<= '9'){
					precision_ *= 10;
					precision_ += str[0]-'0';
					buf_[code_pos_++] = *str++; 
					if(code_pos_ == BUF_MAX){
						return;
					}
				}else{
					break;
				}
			}
		}
	}

};

class Format : public AnyImpl{
public:

	Format(const String& str){
		set_class(TClass<Format>::get());
		set(str.c_str());
	}

	Format(const char* str){
		set_class(TClass<Format>::get());
		set(str);
	}

	void set(const char* str){

		original_ = String(str);

		const char* begin = str;
		char buf[256];
		int bufpos = 0, n = 0;
		while(true){
			if(str[0]=='%'){
				values_.push_back(Pair(null, String(begin, str)));
				str++;
				
				if(str[0]=='%'){
					begin = str++;
				}else{
					if(str[0]=='('){
						str++;
						bufpos = 0;
						while(str[0]!=0 && str[0]!=')' && bufpos!=255){
							buf[bufpos++] = str[0];
							str++;					
						}
						if(str[0]==')'){
							str++;
						}
						buf[bufpos++] = 0;
					}else{
						bufpos = sprintf(buf, "%d", n++);
					}
					Any ret; new(ret) FormatString(str);
					values_.push_back(Pair(ID(buf), ret));
					begin = str;
				}
			}else if(str[0]=='\0'){
				values_.push_back(Pair(null, String(begin, str)));
				break;
			}else{
				str++;
			}
		}
	}

	virtual void call(const VMachine& vm){
		string_t buf;
		char cbuf[256];
		char* pcbuf;
			
		for(int i = 0, size = values_.size(); i<size; ++i){
			if(!values_[i].key){
				buf += cast<String>(values_[i].value).c_str();
			}else{
				Any a = vm.arg(values_[i].key);
				if(a==null){
					a = vm.arg(values_[i].key.to_i());
				}
				
				FormatString* fs = (FormatString*)values_[i].value.impl();
				int_t malloc_size = 0;
				if(fs->max_buf_size()>=256){
					malloc_size = fs->max_buf_size() + 1;
					pcbuf = (char*)user_malloc(malloc_size);
				}else{
					pcbuf = cbuf;
				}
				
				switch(a.type()){
					XTAL_DEFAULT{
						String str = a.to_s();
						
						if(str.size()>=256){
							if(str.size()>malloc_size){
								if(malloc_size!=0){
									user_free(pcbuf, malloc_size);
								}
								
								malloc_size = str.size() + fs->max_buf_size() + 1;
								pcbuf = (char*)user_malloc(malloc_size);
							}
						}
						fs->change_string_code();
						sprintf(pcbuf, fs->format_string(), str.c_str());
						buf += pcbuf;
					}

					XTAL_CASE(TYPE_NULL){
						buf += "<null>";
					}

					XTAL_CASE(TYPE_INT){
						fs->change_int_code();
						sprintf(pcbuf, fs->format_string(), a.ivalue());
						buf += pcbuf;
					}
					
					XTAL_CASE(TYPE_FLOAT){
						fs->change_float_code();
						sprintf(pcbuf, fs->format_string(), a.fvalue());
						buf += pcbuf;
					}
				}
				
				if(malloc_size!=0){
					user_free(pcbuf, fs->max_buf_size());
				}
			}
		}
		vm.return_result(String(buf));
	}

	void to_s(const VMachine& vm){
		call(vm);
	}

	Any marshal_dump(){
		return original_;
	}

private:

	struct Pair{
		ID key;
		Any value;
		Pair(const ID& k = null, const Any& v = null)
			:key(k), value(v){}
	};
	
	friend void visit_members(Visitor& m, const Pair& p){
		m & p.key & p.value;
	}

	AC<Pair>::vector values_;
	String original_;
	
	virtual void visit_members(Visitor& m){
		AnyImpl::visit_members(m);
		m & values_ & original_;
	}
};

namespace{
	Any user_get_text_map_(null);
}

void InitFormat(){
	TClass<Format> p("Format");
	p.method("to_s", &Format::to_s);
	p.method("marshal_dump", &Format::marshal_dump);
	p.def("marshal_new", New<Format, const String&>());

	add_long_life_var(&user_get_text_map_);
	user_get_text_map_ = Map();
}

void set_get_text_map(const Any& map){
	user_get_text_map_ = map;
}

void add_get_text_map(const Any& map){
	user_get_text_map_ = user_get_text_map_.cat(map);
}

Any get_get_text_map(){
	return user_get_text_map_;
}

Any get_text(const char* text){
	if(user_get_text_map_){
		String key(text);
		if(const Any& value=user_get_text_map_[key]){
			Any ret; new(ret) Format(value.to_s().c_str());
			return ret;
		}
	}
	Any ret; new(ret) Format(text);
	return ret;
}

Any format(const char* text){
	Any ret; new(ret) Format(text);
	return ret;
}

namespace debug{

class InfoImpl : public AnyImpl{
public:

	InfoImpl(){
		set_class(TClass<Info>::get());
	}

	void visit_members(Visitor& m){
		AnyImpl::visit_members(m);
		m & file_name & fun_name & local_variables;
	}

	int_t kind;
	int_t line;
	String file_name;
	String fun_name;
	Frame local_variables;
};

Info::Info()	
	:Any(null){
	new(*this) InfoImpl();
}

int_t Info::kind() const{
	return impl()->kind;
}

int_t Info::line() const{
	return impl()->line;
}

String Info::file_name() const{
	return impl()->file_name;
}

String Info::fun_name() const{
	return impl()->fun_name;
}

Frame Info::local_variables() const{
	return impl()->local_variables;
}

void Info::set_kind(int_t v) const{
	impl()->kind = v;
}

void Info::set_line(int_t v) const{
	impl()->line = v;
}

void Info::set_file_name(const String& v) const{
	impl()->file_name = v;
}

void Info::set_fun_name(const String& v) const{
	impl()->fun_name = v;
}

void Info::set_local_variables(const Frame& v) const{
	impl()->local_variables = v;
}



namespace{
	int_t enable_count_;
	Any line_hook_;
	Any call_hook_;
	Any return_hook_;
}

void enable(){
	enable_count_++;
	if(enable_count_>1)
		enable_count_ = 1;
}

void disable(){
	enable_count_--;
}

bool is_enabled(){
	return enable_count_>0;
}

void set_line_hook(const Any& hook){
	line_hook_ = hook;
}

void set_call_hook(const Any& hook){
	call_hook_ = hook;
}

void set_return_hook(const Any& hook){
	return_hook_ = hook;
}

Any line_hook(){
	return line_hook_;
}

Any call_hook(){
	return call_hook_;
}

Any return_hook(){
	return return_hook_;
}

}

void InitDebug(){
	using namespace debug;

	add_long_life_var(&line_hook_);
	add_long_life_var(&call_hook_);
	add_long_life_var(&return_hook_);
	enable_count_ = 0;

	TClass<Info> cls;
	cls.method("kind", &Info::kind);
	cls.method("line", &Info::line);
	cls.method("fun_name", &Info::fun_name);
	cls.method("file_name", &Info::file_name);
	cls.method("set_kind", &Info::set_kind);
	cls.method("set_line", &Info::line);
	cls.method("set_fun_name", &Info::set_fun_name);
	cls.method("set_file_name", &Info::set_file_name);

	cls.def("LINE", BREAKPOINT_LINE);
	cls.def("CALL", BREAKPOINT_CALL);
	cls.def("RETURN", BREAKPOINT_RETURN);
}

void initialize_lib(){

	Class builtin = xtal::builtin();

	Array lib_path;
	lib_path.push_back(".");
	builtin.def("lib_path", lib_path);

	builtin.def("Any", TClass<Any>::get());
	builtin.def("Array", TClass<Array>::get());
	builtin.def("Map", TClass<Map>::get());
	builtin.def("String", TClass<String>::get());
	builtin.def("Int", TClass<Int>::get());
	builtin.def("Float", TClass<Float>::get());
	builtin.def("Arguments", TClass<Arguments>::get());
	builtin.def("Iterator", Iterator());
	builtin.def("Enumerable", Enumerable());
	builtin.def("Null", TClass<Null>::get());
	builtin.def("Class", TClass<Class>::get());
	builtin.def("Fun", TClass<Fun>::get());
	builtin.def("Fiber", TClass<Fiber>::get());
	builtin.def("Stream", TClass<Stream>::get());
	builtin.def("FileStream", TClass<FileStream>::get());
	builtin.def("StringStream", TClass<StringStream>::get());
	builtin.def("Thread", TClass<Thread>::get());
	builtin.def("Mutex", TClass<Mutex>::get());
	builtin.def("Format", TClass<Format>::get());
	
	builtin.fun("print", &print);
	builtin.fun("println", &println);
	builtin.fun("compile_file", &compile_file);
	builtin.fun("compile", &compile);
	builtin.fun("gc", &gc);
	builtin.fun("full_gc", &full_gc);
	builtin.fun("disable_gc", &disable_gc);
	builtin.fun("enable_gc", &enable_gc);
	builtin.fun("object_dump", &object_dump);
	builtin.fun("object_load", &object_load);
	builtin.fun("object_to_script", &object_to_script);

	Class math(Math<float_t>::make((float_t*)0));
	math.fun("abs", &abs);
	math.fun("max", &max_);
	math.fun("min", &min_);
	math.fun("random", &random);
	math.fun("random_range", &random_range);
	builtin.def("math", math);

	builtin.def("builtin", builtin);

	Any lib = xtal::lib();
	lib.def("builtin", builtin);		
	builtin.def("lib", lib);

	InitZipIter();
	builtin.def("zip", TClass<ZipIterImpl>::get());

	Xsrc((
builtin::Exception: class{
	@message;
	@backtrace;

	initialize: method(message:""){
		@message = message;
		@backtrace = [];
	}

	backtrace: @backtrace;

	message: @message;

	append_backtrace: method(file, line, name){
		if(name){
			@backtrace.push_back(%t"\t%(file)s:%(line)d: in %(name)s"(
				file:file,
				line:line,
				name:name));
		}else{
			@backtrace.push_back(%t"\t%(file)s:%(line)d:"(
				file:file,
				line:line));
		}
	}

	to_s: method{
		return this.class.object_name ~ ": " ~ @message.to_s ~ "\n" ~ @backtrace.each.join("\n");
	}
}

builtin::RuntimeError: class(Exception){}
builtin::IOError: class(Exception){}
builtin::LogicError: class(Exception){}
builtin::BadCastError: class(Exception){}
builtin::InvalidArgumentError: class(Exception){}
builtin::BadYieldError: class(Exception){}
builtin::BadInstanceVariableError: class(Exception){}
builtin::UnsupportedError: class(Exception){}
builtin::BadRedefineError: class(Exception){}
builtin::AssertionFailed: class(Exception){
	initialize: method(message, expr){
		Exception::initialize(%t"'%s':%s"(expr, message));
	}
}
builtin::CompileError: class(Exception){
	initialize: method(message, errors){
		Exception::initialize(%t"%s\n%s"(message, errors.join("\t\n")));
	}		
}		
	))();


	Xsrc((
Enumerable::collect: method(conv){ return this.each.collect(conv); }
Enumerable::map: method(conv){ return this.each.map(conv); }
Enumerable::select: method(pred){ return this.each.select(pred); }
Enumerable::filter: method(pred){ return this.each.filter(pred); }
Enumerable::to_a: method(){ return this.each.to_a; }
Enumerable::join: method(sep:","){ return this.each.join(sep); }
Enumerable::with_index: method(start:0){ return this.each.with_index(start); }
Enumerable::iter_first: method(){ return this.each.iter_first; }
Enumerable::cycle: method(){ return this.each.cycle; }
Enumerable::break_if: method(pred){ return this.each.break_if(pred); }
Enumerable::take: method(times){ return this.each.take(times); }
Enumerable::zip: method(...){ return this.each.zip(...); }
Enumerable::unique: method(pred:null){ return this.each.unique(pred); }
Enumerable::chain: method(other){ return this.each.chain(other); }
Enumerable::max_element: method(pred:null){ return this.each.max_element(pred); }
Enumerable::min_element: method(pred:null){ return this.each.min_element(pred); }
Enumerable::find: method(pred){ return this.each.find(pred); }
Enumerable::inject: method(init, fn){ return this.each.inject(init, fn); }
	))();


	Xsrc((
Iterator::to_a: method{
	ret: [];
	this{
		ret.push_back(it); 
	}
	return ret;
}

Iterator::join: method(sep:","){
	ret: StringStream();
	first: true;
	this{
		if(first){
			first = false;
		}else{
			ret.write(sep);
		}
		ret.write(it.to_s);
	}
	return ret.to_s;
}

Iterator::with_index: method(start:0){
	return fiber{		
		this{
			yield start, it;
			++start;
		}
	}
}

Iterator::collect: method(conv){
	return fiber{
		this{
			yield conv(it);
		}
	}
}

Iterator::map: Iterator::collect;

Iterator::select: method(pred){
	return fiber{
		this{
			if(pred(it)){
				yield it;
			}
		}
	}
}

Iterator::filter: Iterator::select;

Iterator::each: method{ 
	return this; 
}
	))();

	Xsrc((
Iterator::break_if: method(pred){
	return fiber{
		this{
			if(pred(it))
				break;
			yield it;
		}
	}
}

Iterator::take: method(times){
	return fiber{
		i: 0;
		this{
			if(i>=times)
				break;
			yield it;
			i++;
		}
	}
}

Iterator::zip: method(...){
	return builtin::zip(this, ...);
}

Iterator::unique: method(pred:null){
	prev: once class{}
	if(pred){
		return fiber{
			this{
				if(pred(it, prev)){
					yield it;
					prev = it;
				}
			}
		}
	}

	return fiber{
		this{
			if(prev!=it){
				yield it;
				prev = it;
			}
		}
	}
}

builtin::chain: fun(...){
	arg: ...;
	return fiber{
		arg.each_ordered{
			it{
				yield it;
			}
		}
	}
}

Iterator::chain: method(...){
	return builtin::chain(this, ...);
}

Iterator::cycle: method{
	return fiber{
		temp: [];
		this{
			temp.push_back(it);
			yield it;
		}
		
		for(;;){
			temp{
				yield it;
			}
		}
	}
}
	))();

	Xsrc((
Iterator::max_element: method(pred:null){
	item: null;
	if(pred){
		this{
			if(item){
				if(pred(it, item))
					item = it;
			}else{
				item = it;
			}
		}
		return item;
	}

	this{
		if(item){
			if(item<it)
				item = it;
		}else{
			item = it;
		}
	}
	return item;
}

Iterator::min_element: method(pred:null){
	item: null;
	if(pred){
		this{
			if(item){
				if(pred(it, item))
					item = it;
			}else{
				item = it;
			}
		}
		return item;
	}

	this{
		if(item){
			if(item>it)
				item = it;
		}else{
			item = it;
		}
	}
	return item;
}

Iterator::find: method(pred){
	this{
		if(pred(it)){
			return it;
		}
	}
}

Iterator::inject: method(init, fn){
	result: init;
	this{
		init = fn(result, it);
	}
	return init;
}
	))();

	Xsrc((

Any::p: method{
	println(this.to_s);
	return this;
}

Any::to_s: method{
	return this.object_name;
}

Int::times: method{
	return fiber{
		for(i: 0; i<this; ++i){
			yield i;
		}
	}
}

Null::to_s: method{
	return "null";
}

Null::iter_first: method{
	return null;
}

builtin::range: fun(first, last, step:1){
	if(step==1){
		return fiber{
			while(first<last){
				yield first;
				first++;
			}
		}
	}else{
		return fiber{
			while(first<last){
				yield first;
				first += step;
			}
		}
	}
}

builtin::load: fun(file_name, ...){
	code: compile_file(file_name);
	return code(...);
}

Arguments::each: method{
	return fiber{ 
		this.each_ordered_arg.with_index{ |i, v|
			yield i, v;
		}
		this.each_named_arg{ |i, v|
			yield i, v;
		}
	}
}

Arguments::each_pair: Arguments::each;

Arguments::pairs: method(){
	return this.each.to_a;
}

Arguments::ordered_args: method(){
	return this.each_ordered_arg.to_a;
}

Arguments::named_args: method(){
	return this.each_named_arg.to_a;
}

Map::pairs: method(){
	return this.each_pair.to_a;
}

Map::values: method(){
	return this.each_value.to_a;
}

Map::keys: method(){
	return this.each_key.to_a;
}

Fun::call: method(...){
	return this(...);
}

builtin::open: fun(file_name, mode: "r"){
	ret: null;
	XTAL_TRY{
		ret = FileStream(file_name, mode);
	}XTAL_CATCH(e){
		ret = null;
	}
	return ret;
}

Mutex::iter_first: method{
	this.lock;
	return this;
}

Mutex::iter_next: method{
	this.unlock;
	return null;
}

Mutex::iter_break: method{
	this.unlock;
	return null;
}
	))();

	Xsrc((

Int::op_add: method(rhs){ return rhs.op_add_r_Int(this); }
Int::op_add_assign: Int::op_add;
Int::op_sub: method(rhs){ return rhs.op_sub_r_Int(this); }
Int::op_sub_assign: Int::op_sub;
Int::op_cat: method(rhs){ return rhs.op_cat_r_Int(this); }
Int::op_cat_assign: Int::op_cat;
Int::op_mul: method(rhs){ return rhs.op_mul_r_Int(this); }
Int::op_mul_assign: Int::op_mul;
Int::op_div: method(rhs){ return rhs.op_div_r_Int(this); }
Int::op_div_assign: Int::op_div;
Int::op_mod: method(rhs){ return rhs.op_mod_r_Int(this); }
Int::op_mod_assign: Int::op_mod;
Int::op_and: method(rhs){ return rhs.op_and_r_Int(this); }
Int::op_and_assign: Int::op_and;
Int::op_or: method(rhs){ return rhs.op_or_r_Int(this); }
Int::op_or_assign: Int::op_or;
Int::op_xor: method(rhs){ return rhs.op_xor_r_Int(this); }
Int::op_xor_assign: Int::op_xor;
Int::op_shr: method(rhs){ return rhs.op_shr_r_Int(this); }
Int::op_shr_assign: Int::op_shr;
Int::op_shl: method(rhs){ return rhs.op_shl_r_Int(this); }
Int::op_shl_assign: Int::op_shl;
Int::op_ushr: method(rhs){ return rhs.op_ushr_r_Int(this); }
Int::op_ushr_assign: Int::op_ushr;
Int::op_eq: method(rhs){ return rhs.op_eq_r_Int(this); }
Int::op_eq_assign: Int::op_eq;
Int::op_lt: method(rhs){ return rhs.op_lt_r_Int(this); }
Int::op_lt_assign: Int::op_lt;
	))();

	Xsrc((
Float::op_add: method(rhs){ return rhs.op_add_r_Float(this); }
Float::op_add_assign: Float::op_add;
Float::op_sub: method(rhs){ return rhs.op_sub_r_Float(this); }
Float::op_sub_assign: Float::op_sub;
Float::op_cat: method(rhs){ return rhs.op_cat_r_Float(this); }
Float::op_cat_assign: Float::op_cat;
Float::op_mul: method(rhs){ return rhs.op_mul_r_Float(this); }
Float::op_mul_assign: Float::op_mul;
Float::op_div: method(rhs){ return rhs.op_div_r_Float(this); }
Float::op_div_assign: Float::op_div;
Float::op_mod: method(rhs){ return rhs.op_mod_r_Float(this); }
Float::op_mod_assign: Float::op_mod;
Float::op_and: method(rhs){ return rhs.op_and_r_Float(this); }
Float::op_and_assign: Float::op_and;
Float::op_or: method(rhs){ return rhs.op_or_r_Float(this); }
Float::op_or_assign: Float::op_or;
Float::op_xor: method(rhs){ return rhs.op_xor_r_Float(this); }
Float::op_xor_assign: Float::op_xor;
Float::op_shr: method(rhs){ return rhs.op_shr_r_Float(this); }
Float::op_shr_assign: Float::op_shr;
Float::op_shl: method(rhs){ return rhs.op_shl_r_Float(this); }
Float::op_shl_assign: Float::op_shl;
Float::op_ushr: method(rhs){ return rhs.op_ushr_r_Float(this); }
Float::op_ushr_assign: Float::op_ushr;
Float::op_eq: method(rhs){ return rhs.op_eq_r_Float(this); }
Float::op_eq_assign: Float::op_eq;
Float::op_lt: method(rhs){ return rhs.op_lt_r_Float(this); }
Float::op_lt_assign: Float::op_lt;
	))();

	Xsrc((
Int::op_pos: method(){ return this; }
Int::op_neg: method(){ return -this; }
Int::op_com: method(){ return ~this; }
Int::op_inc: method(){ return this + 1; }
Int::op_dec: method(){ return this - 1; }

Int::op_add_r_Int: method(lhs){ return lhs + this; }
Int::op_sub_r_Int: method(lhs){ return lhs - this; }
Int::op_mul_r_Int: method(lhs){ return lhs * this; }
Int::op_div_r_Int: method(lhs){ return lhs / this; }
Int::op_mod_r_Int: method(lhs){ return lhs % this; }
Int::op_or_r_Int: method(lhs){ return lhs | this; }
Int::op_and_r_Int: method(lhs){ return lhs & this; }
Int::op_xor_r_Int: method(lhs){ return lhs ^ this; }
Int::op_shr_r_Int: method(lhs){ return lhs >> this; }
Int::op_shl_r_Int: method(lhs){ return lhs << this; }
Int::op_ushr_r_Int: method(lhs){ return lhs >>> this; }
Int::op_eq_r_Int: method(lhs){ return lhs == this; }
Int::op_lt_r_Int: method(lhs){ return lhs < this; }

Int::op_add_r_Float: method(lhs){ return lhs + this; }
Int::op_sub_r_Float: method(lhs){ return lhs - this; }
Int::op_mul_r_Float: method(lhs){ return lhs * this; }
Int::op_div_r_Float: method(lhs){ return lhs / this; }
Int::op_mod_r_Float: method(lhs){ return lhs % this; }
Int::op_eq_r_Float: method(lhs){ return lhs == this; }
Int::op_lt_r_Float: method(lhs){ return lhs < this; }
	))();

	Xsrc((
Float::op_pos: method(){ return this; }
Float::op_neg: method(){ return -this; }
Float::op_inc: method(){ return this + 1; }
Float::op_dec: method(){ return this - 1; }

Float::op_add_r_Int: method(lhs){ return lhs + this; }
Float::op_sub_r_Int: method(lhs){ return lhs - this; }
Float::op_mul_r_Int: method(lhs){ return lhs * this; }
Float::op_div_r_Int: method(lhs){ return lhs / this; }
Float::op_mod_r_Int: method(lhs){ return lhs % this; }
Float::op_eq_r_Int: method(lhs){ return lhs == this; }
Float::op_lt_r_Int: method(lhs){ return lhs < this; }

Float::op_add_r_Float: method(lhs){ return lhs + this; }
Float::op_sub_r_Float: method(lhs){ return lhs - this; }
Float::op_mul_r_Float: method(lhs){ return lhs * this; }
Float::op_div_r_Float: method(lhs){ return lhs / this; }
Float::op_mod_r_Float: method(lhs){ return lhs % this; }
Float::op_eq_r_Float: method(lhs){ return lhs == this; }
Float::op_lt_r_Float: method(lhs){ return lhs < this; }
	))();
///////////////////////////////////////////////////////////

	add_get_text_map(Xsrc((
export [
	"Xtal Compile Error 1001":"構文エラーです。",
	"Xtal Compile Error 1002":"予期せぬ文字 '%(char)s' が検出されました。",
	"Xtal Compile Error 1003":"';' がありません。",
	"Xtal Compile Error 1004":"continue文に対応するループ文がありません。",
	"Xtal Compile Error 1005":"break文に対応するループ文がありません。",
	"Xtal Compile Error 1006":"不正なcontinue文です。",
	"Xtal Compile Error 1007":"不正なbreak文です。",
	"Xtal Compile Error 1008":"不正な多重代入文です。",
	"Xtal Compile Error 1009":"定義されていない変数 '%(name)s' に代入しようとしました 。",
	"Xtal Compile Error 1010":"不正な数字リテラルのサフィックスです。",
	"Xtal Compile Error 1011":"文字列リテラルの途中でファイルが終わりました。",
	"Xtal Compile Error 1012":"不正な代入文の左辺です。",
	"Xtal Compile Error 1013":"比較演算式の結果を演算しようとしています。",
	"Xtal Compile Error 1014":"不正な浮動小数点数リテラルです。",
	"Xtal Compile Error 1015":"不正な16進数値リテラルのサフィックスです。",
	"Xtal Compile Error 1016":"assert文の引数の数が不正です。",
	"Xtal Compile Error 1017":"不正な%%記法リテラルです。",
	"Xtal Compile Error 1018":"default節が重複定義されました。",
	"Xtal Compile Error 1019":"exportは一つのファイルで一つのみ許可されます。",
	"Xtal Compile Error 1020":"不正な2進数値リテラルのサフィックスです。",
	"Xtal Compile Error 1021":"コメントの途中でファイルが終わりました。",
	"Xtal Compile Error 1022":"関数から返せる多値の最大は255個です。",
	"Xtal Compile Error 1023":"定義されていないインスタンス変数名 '%(name)s' を参照しています。",
	"Xtal Compile Error 1024":"同名のインスタンス変数名 '%(name)s' が既に定義されています。",
	"Xtal Compile Error 1025":"比較演算式の結果を最比較しようとしています。",
	"Xtal Compile Error 1026":"同じスコープ内で、同じ変数名 '%(name)s' が重複定義されました",
];
	))());

	add_get_text_map(Xsrc((
	export [
	"Xtal Runtime Error 1001":"'%(n)d'番目の引数%(param_name)の型エラーです。 '%(required)s'型を要求していますが、'%(type)s'型の値が渡されました。",
	"Xtal Runtime Error 1002":"evalに渡されたソースのコンパイル中、コンパイルエラーが発生しました。",
	"Xtal Runtime Error 1003":"不正なインスタンス変数の参照です。",
	"Xtal Runtime Error 1004":"型エラーです。 '%(required)s'型を要求していますが、'%(type)s'型の値が渡されました。",
	"Xtal Runtime Error 1005":"'%(name)s'関数呼び出しの引数の数が不正です。%(min)s以上の引数を受け取る関数に、%(value)s個の引数を渡しました。",
	"Xtal Runtime Error 1006":"'%(name)s'関数呼び出しの引数の数が不正です。%(min)s以上、%(max)以下の引数を受け取る関数に、%(value)s個の引数を渡しました。",
	"Xtal Runtime Error 1007":"'%(name)s'関数呼び出しの引数の数が不正です。引数を取らない関数に、%(value)s個の引数を渡しました。",
	"Xtal Runtime Error 1008":"%(name)sにアクセスできず、marshal loadに失敗しました。",
	"Xtal Runtime Error 1009":"不正なコンパイル済みXtalファイルです。",
	"Xtal Runtime Error 1010":"コンパイルエラーが発生しました。",
	"Xtal Runtime Error 1011":"%(object)s :: '%(name)s' は既に定義されています。",
	"Xtal Runtime Error 1012":"yieldがfiberの非実行中に実行されました。",
	"Xtal Runtime Error 1013":"%(object)s :: new 関数が登録されていないため、インスタンスを生成できません。",
	"Xtal Runtime Error 1014":"ファイル '%(name)s' を開けません。",
	"Xtal Runtime Error 1015":"%(object)s :: '%(name)s' は定義されていません。",
	"Xtal Runtime Error 1016":"ファイル '%(name)s' のコンパイル中、コンパイルエラーが発生しました。",
];
	))());
}

}
