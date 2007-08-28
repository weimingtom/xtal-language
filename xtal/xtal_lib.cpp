
#include "xtal.h"

#include <iostream>
#include <iomanip>

#include <cmath>
#include <stdio.h>

#include "xtal_codebuilder.h"
#include "xtal_fun.h"
#include "xtal_code.h"
#include "xtal_macro.h"
#include "xtal_serializer.h"
#include "xtal_frame.h"
#include "xtal_stream.h"
#include "xtal_smartptr.h"

namespace xtal{

#ifndef XTAL_NO_PARSER

CodePtr compile_file(const StringPtr& file_name){
	CodeBuilder cb;
	FileStreamPtr fs(xnew<FileStream>(file_name, "r"));
	if(CodePtr fun = cb.compile(fs, file_name)){
		fs->close();
		return fun;
	}
	fs->close();

	XTAL_THROW(builtin()->member("CompileError")(Xt("Xtal Runtime Error 1016")(file_name), cb.errors()), return null);
}

CodePtr compile(const StringPtr& source){
	CodeBuilder cb;
	MemoryStreamPtr ms(xnew<MemoryStream>(source->c_str(), source->buffer_size()));
	if(CodePtr fun =  cb.compile(ms, "<eval>")){
		return fun;
	}

	XTAL_THROW(builtin()->member("CompileError")(Xt("Xtal Runtime Error 1002")(), cb.errors()), return null);
}

AnyPtr load(const StringPtr& file_name){
	AnyPtr ret = compile_file(file_name)();
	gc();
	return ret;
}

AnyPtr load_and_save(const StringPtr& file_name){
	AnyPtr ret = compile_file(file_name);
	FileStreamPtr fs(xnew<FileStream>(file_name->cat("c"), "w"));
	fs->serialize(ret);
	fs->close();
	gc();
	return ret();
}

AnyPtr source(const char_t* src, int_t size, const char_t* file){
	CodeBuilder cb;
	MemoryStreamPtr ms(xnew<MemoryStream>(src, size));
	if(AnyPtr fun = cb.compile(ms, file)){
		return fun;
	}

	XTAL_THROW(builtin()->member("CompileError")(Xt("Xtal Runtime Error 1010")(), cb.errors()), return null);
}

void ix(){
	CodeBuilder cb;
	cb.interactive_compile();
}

#else

Code compile_file(const StringPtr& file_name){
	return null;
}

Code compile(const StringPtr& source){
	return null;
}

AnyPtr load(const StringPtr& file_name){
	return null;
}

AnyPtr load_and_save(const StringPtr& file_name){
	return null;
}

AnyPtr source(const char* src, int_t size, const char* file){
	return null;
}

void ix(){

}

#endif

static AnyPtr abs(const AnyPtr& a){
	switch(type(a)){XTAL_DEFAULT;
		XTAL_CASE(TYPE_INT){ return ivalue(a)<0 ? -ivalue(a) : a; }
		XTAL_CASE(TYPE_FLOAT){ return fvalue(a)<0 ? -fvalue(a) : a; }
	}
	return 0;
}

static AnyPtr max_(const AnyPtr& a, const AnyPtr& b){
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
	return null;// a->send(Xid(op_lt), a, b) ? b : a;
}

static AnyPtr min_(const AnyPtr& a, const AnyPtr& b){
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
	return null;//a->send(Xid(op_lt), a, b) ? a : b;
}

class Random{
public:

	Random(int_t seed=time(0)){ 
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

static float_t clock_(){
	return clock()/(float_t)CLOCKS_PER_SEC;
}

template<class T>
struct Math{
	static ClassPtr make(float*){
		using namespace std;

		ClassPtr math = xnew<Class>("math");
		
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

		ClassPtr math = xnew<Class>("math");

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
			char buf[32];
			sprintf(buf, "%d", ivalue(p));
			return xnew<String>(buf);
		}

		XTAL_THROW(cast_error(p->get_class()->object_name(), "Int"), return null);
	}


	int_t Float_to_i(const AnyPtr& p){
		if(type(p)==TYPE_FLOAT){
			return (int_t)fvalue(p);
		}

		XTAL_THROW(cast_error(p->get_class()->object_name(), "Int"), return 0);
	}

	float_t Float_to_f(const AnyPtr& p){
		if(type(p)==TYPE_FLOAT){
			return fvalue(p);
		}

		XTAL_THROW(cast_error(p->get_class()->object_name(), "Int"), return 0);
	}

	StringPtr Float_to_s(const AnyPtr& p){
		if(type(p)==TYPE_FLOAT){
			char buf[32];
			sprintf(buf, "%g", fvalue(p));
			return xnew<String>(buf);
		}

		XTAL_THROW(cast_error(p->get_class()->object_name(), "Int"), return null);
	}
}

void InitInt(){
	ClassPtr p = new_cpp_class<Int>("Int");	
	p->method("to_i", &Int_to_i);
	p->method("to_f", &Int_to_f);
	p->method("to_s", &Int_to_s);
}

void InitFloat(){
	ClassPtr p = new_cpp_class<Float>("Float");		
	p->method("to_i", &Float_to_i);
	p->method("to_f", &Float_to_f);
	p->method("to_s", &Float_to_s);
}
	
class ZipIter : public Base{
public:

	ZipIter(const VMachinePtr& vm){
		next = xnew<Array>(vm->ordered_arg_count());
		for(int_t i = 0, len = next->size(); i<len; ++i){
			next->set_at(i, vm->arg(i));
		}
	}

	void common(const VMachinePtr& vm, const InternedStringPtr& id){
		bool all = true;
		value = xnew<Array>(next->size());
		for(int_t i = 0, len = next->size(); i<len; ++i){
			vm->setup_call(2);
			next->at(i)->rawsend(vm, id);
			next->set_at(i, vm->result(0));
			value->set_at(i, vm->result(1));
			vm->cleanup_call();
			if(!next->at(i))
				all = false;
		}
		if(all){
			vm->return_result(SmartPtr<ZipIter>::from_this(this), value);
		}else{
			vm->return_result(null);
		}
	}

	void iter_first(const VMachinePtr& vm){
		common(vm, Xid(iter_first));
	}
	
	void iter_next(const VMachinePtr& vm){
		common(vm, Xid(iter_next));
	}

	void iter_break(const VMachinePtr& vm){
		InternedStringPtr id = Xid(iter_break);
		for(int_t i = 0, len = next->size(); i<len; ++i){
			vm->setup_call(0);
			next->at(i)->rawsend(vm, id);
			if(!vm->processed()){
				vm->return_result();	
			}
			vm->cleanup_call();
		}
		vm->return_result();
	}

	virtual void visit_members(Visitor& m){
		Base::visit_members(m);
		m & next & value;
	}

	ArrayPtr next;
	ArrayPtr value;
};

void InitZipIter(){
	ClassPtr p = new_cpp_class<ZipIter>("ZipIter");
	p->inherit(Iterator());
	p->def("new", ctor<ZipIter, const VMachinePtr&>());
	p->method("iter_first", &ZipIter::iter_next);
	p->method("iter_next", &ZipIter::iter_next);
	p->method("iter_break", &ZipIter::iter_break);
}

IterBreaker::~IterBreaker(){
	const VMachinePtr& vm = vmachine();
	vm->setup_call(0);
	target->rawsend(vm, Xid(iter_break));
	if(!vm->processed()){
		vm->return_result();
	}
	vm->cleanup_call();
}

void iter_next(AnyPtr& target, AnyPtr& value1, bool first){
	const VMachinePtr& vm = vmachine();
	vm->setup_call(2);
	target->rawsend(vm, first ? Xid(iter_first) : Xid(iter_next));
	target = vm->result(0);
	value1 = vm->result(1);
	vm->cleanup_call();
}

void iter_next(AnyPtr& target, AnyPtr& value1, AnyPtr& value2, bool first){
	const VMachinePtr& vm = vmachine();
	vm->setup_call(3);
	target->rawsend(vm, first ? Xid(iter_first) : Xid(iter_next));
	target = vm->result(0);
	value1 = vm->result(1);
	value2 = vm->result(2);
	vm->cleanup_call();
}

void iter_next(AnyPtr& target, AnyPtr& value1, AnyPtr& value2, AnyPtr& value3, bool first){
	const VMachinePtr& vm = vmachine();
	vm->setup_call(4);
	target->rawsend(vm, first ? Xid(iter_first) : Xid(iter_next));
	target = vm->result(0);
	value1 = vm->result(1);
	value2 = vm->result(2);
	value3 = vm->result(3);
	vm->cleanup_call();
}

const ClassPtr& Iterator(){
	static LLVar<ClassPtr> p = xnew<Class>("Iterator");
	return p;
}

const ClassPtr& Enumerator(){
	static LLVar<ClassPtr> p = xnew<Class>("Enumerator");
	return p;
}

const ClassPtr& builtin(){
	static LLVar<ClassPtr> p = xnew<Class>("builtin");
	return p;
}

const ClassPtr& lib(){
	static LLVar<ClassPtr> p = xnew<Lib>();
	return p;
}

const StreamPtr& stdin_stream(){
	static LLVar<StreamPtr> p = xnew<StdioStream>(stdin);
	return p;
}

const StreamPtr& stdout_stream(){
	static LLVar<StreamPtr> p = xnew<StdioStream>(stdout);
	return p;
}

const StreamPtr& stderr_stream(){
	static LLVar<StreamPtr> p = xnew<StdioStream>(stderr);
	return p;
}

AnyPtr cast_error(const AnyPtr& from, const AnyPtr& to){
	return builtin()->member("CastError")(Xt("Xtal Runtime Error 1004")(
		Named("type", from->get_class()->object_name()), Named("required", to)
	));
}

AnyPtr argument_error(const AnyPtr& from, const AnyPtr& to, int_t param_num, const AnyPtr& param_name){
	if(param_name){
		return builtin()->member("ArgumentError")(Xt("Xtal Runtime Error 1001")(
			Named("n", param_num+1), 
			Named("param_name", param_name), 
			Named("type", from->get_class()->object_name()), 
			Named("required", to)
		));
	}else{
		return builtin()->member("ArgumentError")(Xt("Xtal Runtime Error 1001")(
			Named("n", param_num+1), 
			Named("param_name", xnew<String>("")), 
			Named("type", from->get_class()->object_name()), 
			Named("required", to)
		));	}
}

AnyPtr unsupported_error(const AnyPtr& name, const AnyPtr& member){
	return builtin()->member("UnsupportedError")(Xt("Xtal Runtime Error 1015")(
		Named("object", name), Named("name", (member ? member : AnyPtr("()")))
	));
}
namespace{
	void default_except_handler(const AnyPtr& except, const char* file, int line){
#ifdef XTAL_NO_EXCEPT
		printf("%s(%d):%s\n", file, line, except->to_s()->c_str());
		exit(1);
#endif
	}

	except_handler_t except_handler_ = &default_except_handler;
	AnyPtr except_;
}


except_handler_t except_handler(){
	return except_handler_;
}

void set_except_handler(except_handler_t handler){
	except_handler_ = handler;
}

struct FormatString : public Base{

	enum{ BUF_MAX = 14, REAL_BUF_MAX = BUF_MAX + 2 };
	
	char buf_[REAL_BUF_MAX];
	int_t code_pos_, width_, precision_;
	
public:

	FormatString(const char_t*& str){
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

	void parse_fmt(const char_t*& str){
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

class Format : public Base{
public:

	Format(){
		set("");
	}

	Format(const StringPtr& str){
		set(str->c_str());
	}

	Format(const char* str){
		set(str);
	}

	void set(const char* str){

		original_ = xnew<String>(str);

		const char* begin = str;
		char buf[256];
		int bufpos = 0, n = 0;
		while(true){
			if(str[0]=='%'){
				values_.push_back(Pair(null, xnew<String>(begin, str)));
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
					AnyPtr ret = xnew<FormatString>(ref(str));
					values_.push_back(Pair(InternedStringPtr(buf), ret));
					begin = str;
				}
			}else if(str[0]=='\0'){
				values_.push_back(Pair(null, xnew<String>(begin, str)));
				break;
			}else{
				str++;
			}
		}
	}

	virtual void call(const VMachinePtr& vm){
		string_t buf;
		char_t cbuf[256];
		char_t* pcbuf;
			
		for(int i = 0, size = values_.size(); i<size; ++i){
			if(!values_[i].key){
				buf += cast<StringPtr>(values_[i].value)->c_str();
			}else{
				AnyPtr a = vm->arg(values_[i].key);
				if(!a){
					a = vm->arg(values_[i].key->to_i());
				}
				
				SmartPtr<FormatString> fs = cast<SmartPtr<FormatString> >(values_[i].value);
				size_t malloc_size = 0;
				if(fs->max_buf_size()>=256){
					malloc_size = fs->max_buf_size() + 1;
					pcbuf = (char*)user_malloc(malloc_size);
				}else{
					pcbuf = cbuf;
				}
				
				switch(type(a)){
					XTAL_DEFAULT{
						StringPtr str = a->to_s();
						
						if(str->buffer_size()>=256){
							if(str->buffer_size()>malloc_size){
								if(malloc_size!=0){
									user_free(pcbuf);
								}
								
								malloc_size = str->buffer_size() + fs->max_buf_size() + 1;
								pcbuf = (char*)user_malloc(malloc_size);
							}
						}
						fs->change_string_code();
						sprintf(pcbuf, fs->format_string(), str->c_str());
						buf += pcbuf;
					}

					XTAL_CASE(TYPE_NULL){
						buf += "<null>";
					}

					XTAL_CASE(TYPE_INT){
						fs->change_int_code();
						sprintf(pcbuf, fs->format_string(), ivalue(a));
						buf += pcbuf;
					}
					
					XTAL_CASE(TYPE_FLOAT){
						fs->change_float_code();
						sprintf(pcbuf, fs->format_string(), fvalue(a));
						buf += pcbuf;
					}
				}
				
				if(malloc_size!=0){
					user_free(pcbuf);
				}
			}
		}
		vm->return_result(xnew<String>(buf));
	}

	void to_s(const VMachinePtr& vm){
		call(vm);
	}

	AnyPtr serial_save(){
		return original_;
	}

	void serial_load(const StringPtr& v){
		set(v->c_str());
	}

private:

	struct Pair{
		InternedStringPtr key;
		AnyPtr value;
		Pair(const InternedStringPtr& k = null, const AnyPtr& v = null)
			:key(k), value(v){}
	};
	
	friend void visit_members(Visitor& m, const Pair& p){
		m & p.key & p.value;
	}

	AC<Pair>::vector values_;
	StringPtr original_;
	
	virtual void visit_members(Visitor& m){
		Base::visit_members(m);
		m & values_ & original_;
	}
};

namespace{
	MapPtr user_get_text_map_;
}

void InitFormat(){
	ClassPtr p = new_cpp_class<Format>("Format");
	p->method("to_s", &Format::to_s);
	p->method("serial_save", &Format::serial_save);
	p->method("serial_load", &Format::serial_load);
	p->def("serial_new", ctor<Format, const StringPtr&>());

	add_long_life_var(&user_get_text_map_);
	user_get_text_map_ = xnew<Map>();
}

void set_get_text_map(const MapPtr& map){
	user_get_text_map_ = map;
}

void add_get_text_map(const MapPtr& map){
	user_get_text_map_ = user_get_text_map_->cat(map);
}

MapPtr get_get_text_map(){
	return user_get_text_map_;
}

AnyPtr get_text(const char* text){
	if(user_get_text_map_){
		StringPtr key(text);
		if(const AnyPtr& value=user_get_text_map_->at(key)){
			return xnew<Format>(value->to_s());
		}
	}
	return xnew<Format>(text);
}

AnyPtr format(const char* text){
	return xnew<Format>(text);
}

namespace debug{

void Info::visit_members(Visitor& m){
	Base::visit_members(m);
	m & file_name_ & fun_name_ & local_variables_;
}

namespace{
	int_t enable_count_;
	AnyPtr line_hook_;
	AnyPtr call_hook_;
	AnyPtr return_hook_;
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

void set_line_hook(const AnyPtr& hook){
	line_hook_ = hook;
}

void set_call_hook(const AnyPtr& hook){
	call_hook_ = hook;
}

void set_return_hook(const AnyPtr& hook){
	return_hook_ = hook;
}

AnyPtr line_hook(){
	return line_hook_;
}

AnyPtr call_hook(){
	return call_hook_;
}

AnyPtr return_hook(){
	return return_hook_;
}

}

void InitDebug(){
	using namespace debug;

	add_long_life_var(&line_hook_);
	add_long_life_var(&call_hook_);
	add_long_life_var(&return_hook_);
	enable_count_ = 0;

	{
		ClassPtr p = new_cpp_class<Info>("Info");
		p->method("clone", &Info::clone);
		p->method("kind", &Info::kind);
		p->method("line", &Info::line);
		p->method("fun_name", &Info::fun_name);
		p->method("file_name", &Info::file_name);
		p->method("set_kind", &Info::set_kind);
		p->method("set_line", &Info::line);
		p->method("set_fun_name", &Info::set_fun_name);
		p->method("set_file_name", &Info::set_file_name);

		p->def("LINE", BREAKPOINT_LINE);
		p->def("CALL", BREAKPOINT_CALL);
		p->def("RETURN", BREAKPOINT_RETURN);
	}
}

void initialize_lib(){

	ClassPtr builtin = xtal::builtin();

	builtin->def("Any", get_cpp_class<Any>());
	builtin->def("Array", get_cpp_class<Array>());
	builtin->def("Map", get_cpp_class<Map>());
	builtin->def("String", get_cpp_class<String>());
	builtin->def("Int", get_cpp_class<Int>());
	builtin->def("Float", get_cpp_class<Float>());
	builtin->def("Arguments", get_cpp_class<Arguments>());
	builtin->def("Iterator", Iterator());
	builtin->def("Enumerator", Enumerator());
	builtin->def("Null", get_cpp_class<Null>());
	builtin->def("True", get_cpp_class<True>());
	builtin->def("False", get_cpp_class<False>());
	builtin->def("Class", get_cpp_class<Class>());
	builtin->def("CppClass", get_cpp_class<CppClass>());
	builtin->def("Fun", get_cpp_class<Fun>());
	builtin->def("Fiber", get_cpp_class<Fiber>());
	builtin->def("Stream", get_cpp_class<Stream>());
	builtin->def("FileStream", get_cpp_class<FileStream>());
	builtin->def("MemoryStream", get_cpp_class<MemoryStream>());
	builtin->def("StringStream", get_cpp_class<StringStream>());
	builtin->def("Thread", get_cpp_class<Thread>());
	builtin->def("Mutex", get_cpp_class<Mutex>());
	builtin->def("Format", get_cpp_class<Format>());
	builtin->def("Code", get_cpp_class<Code>());
	builtin->def("Instance", get_cpp_class<Instance>());

	builtin->def("stdin", stdin_stream());
	builtin->def("stdout", stdout_stream());
	builtin->def("stderr", stderr_stream());
	
	builtin->fun("compile_file", &compile_file);
	builtin->fun("compile", &compile);
	builtin->fun("gc", &gc);
	builtin->fun("full_gc", &full_gc);
	builtin->fun("disable_gc", &disable_gc);
	builtin->fun("enable_gc", &enable_gc);

	builtin->fun("clock", &clock_);

	ClassPtr math(Math<float_t>::make((float_t*)0));
	math->fun("abs", &abs);
	math->fun("max", &max_);
	math->fun("min", &min_);
	math->fun("random", &random);
	math->fun("random_range", &random_range);
	builtin->def("math", math);

	builtin->def("builtin", builtin);

	ClassPtr lib = xtal::lib();
	lib->def("builtin", builtin);		
	builtin->def("lib", lib);
	builtin->def("Lib", get_cpp_class<Lib>());

	InitZipIter();
	builtin->def("zip", get_cpp_class<ZipIter>());

	
	Xsrc((

builtin::Exception: class{
	+ _backtrace;
	+ _message;

	initialize: method(message:""){
		_message = message;
		_backtrace = [];
	}

	append_backtrace: method(file, line, function_name: null){
		if(function_name){
			_backtrace.push_back(%t"\t%(file)s:%(line)d: in %(function_name)s"(
				file:file,
				line:line,
				function_name:function_name));
		}else{
			_backtrace.push_back(%t"\t%(file)s:%(line)d:"(
				file:file,
				line:line));
		}
	}

	to_s: method{
		return this.class.object_name ~ ": " ~ _message.to_s ~ "\n" ~ _backtrace.each.join("\n");
	}
}

builtin::StandardError: class(Exception){}
builtin::RuntimeError: class(StandardError){}
builtin::IOError: class(StandardError){}
builtin::LogicError: class(StandardError){}
builtin::CastError: class(StandardError){}
builtin::ArgumentError: class(StandardError){}
builtin::YieldError: class(StandardError){}
builtin::InstanceVariableError: class(StandardError){}
builtin::UnsupportedError: class(StandardError){}
builtin::RedefinedError: class(StandardError){}
builtin::AccessibilityError: class(StandardError){}
builtin::AssertionFailed: class(StandardError){
	initialize: method(message, expr){
		Exception::initialize(%f"'%s':%s"(expr, message));
	}
}
builtin::CompileError: class(StandardError){
	initialize: method(message, errors){
		Exception::initialize(%f"%s\n%s"(message, errors.join("\t\n")));
	}		
}		

	))();

	Xsrc((
		lib.append_load_path(".");
	))();

	Xsrc((
Enumerator::reset: method(){ return this.each.reset; }
Enumerator::collect: method(conv){ return this.each.collect(conv); }
Enumerator::map: method(conv){ return this.each.map(conv); }
Enumerator::select: method(pred){ return this.each.select(pred); }
Enumerator::filter: method(pred){ return this.each.filter(pred); }
Enumerator::to_a: method(){ return this.each.to_a; }
Enumerator::to_m: method(){ return this.each.to_m; }
Enumerator::join: method(sep:","){ return this.each.join(sep); }
Enumerator::with_index: method(start:0){ return this.each.with_index(start); }
Enumerator::iter_first: method(){ return this.each.iter_first; }
Enumerator::cycle: method(){ return this.each.cycle; }
Enumerator::break_if: method(pred){ return this.each.break_if(pred); }
Enumerator::take: method(times){ return this.each.take(times); }
Enumerator::zip: method(...){ return this.each.zip(...); }
Enumerator::unique: method(pred:null){ return this.each.unique(pred); }
Enumerator::chain: method(other){ return this.each.chain(other); }
Enumerator::max_element: method(pred:null){ return this.each.max_element(pred); }
Enumerator::min_element: method(pred:null){ return this.each.min_element(pred); }
Enumerator::find: method(pred){ return this.each.find(pred); }
Enumerator::inject: method(init, fn){ return this.each.inject(init, fn); }
	))();


	Xsrc((
Iterator::to_a: method{
	ret: [];
	this{
		ret.push_back(it); 
	}
	return ret;
}

Iterator::to_m: method{
	ret: [:];
	this{ |key, value|
		ret[key] = value; 
	}
	return ret;
}

Iterator::join: method(sep:","){
	ret: MemoryStream();

	if(sep==""){
		this{
			ret.put_s(it.to_s);
		}
	}else{
		first: true;
		this{
			if(first){
				first = false;
			}else{
				ret.put_s(sep);
			}
			ret.put_s(it.to_s);
		}
	}
		
	return ret.to_s;
}

Iterator::with_index: method(start:0){
	return fiber{
		i: start;
		this{
			yield i, it;
			++i;
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
	if(times<=0)
		return [];

	return fiber{
		i: 0;
		this{
			yield it;
			i++;

			if(i>=times)
				break;
		}
	}
}

Iterator::zip: method(...){
	return builtin::zip(this, ...);
}

Iterator::unique: method(pred:null){
	tag: once class{}
	if(pred){
		return fiber{
			prev: tag;
			this{
				if(pred(it, prev)){
					yield it;
					prev = it;
				}
			}
		}
	}

	return fiber{
		prev: tag;
		this{
			if(prev!=it){
				yield it;
				prev = it;
			}
		}
	}
}

builtin::print: fun(...){
	....each_ordered_arg{
		stdout.put_s(it.to_s);
	}
}

builtin::println: fun(...){
	print(...);
	print("\n");
}

builtin::chain: fun(...){
	arg: ...;
	return fiber{
		arg.each_ordered_arg{
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
		result = fn(result, it);
	}
	return result;
}

Iterator::with_prev: method{
	return fiber{
		prev: nop;
		this{
			yield prev, it;
			prev = it;
		}
	}
}

Int::iter_next: method{
	return (this==0 ? null : this-1), this;
}

Int::iter_first: Int::iter_next;

	))();

	Xsrc((

Class::each_ancestor: method fiber{
	this.each_inherited_class{
		yield it;
		it.each_ancestor{
			yield it;
		}
	}
}

Instance::serial_save: method{
	ret: [:];
	klass: this.class;
	if(n: this.instance_serial_save(klass))
		ret[klass.object_name] = n;

	klass.each_ancestor{
		if(n: this.instance_serial_save(it))
			ret[it.object_name] = n;
	}
	return ret;
}

Instance::serial_load: method(v){
	klass: this.class;
	if(n: v[klass.object_name])
		this.instance_serial_load(klass, n);

	klass.each_ancestor{
		if(n: v[it.object_name])
			this.instance_serial_load(it, n);
	}
}

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

String::gsub: method(pattern, fn){
	elem: (peg::anych - pattern)*0;
	split: elem >> (pattern[|| [fn(it)]] >> elem)*0;
	ret: [];
	peg::join(split).parse_string(this, ret);
	return ret[0];
}

Null::to_s: method{
	return "null";
}

Null::iter_first: method{
	return null;
}

True::to_s: method{
	return "true";
}

False::to_s: method{
	return "false";
}

builtin::range: fun(first, last, step:1){
	if(step==1){
		return fiber{
			for(i:first; i<last; i++){
				yield i;
			}
		}
	}else{
		return fiber{
			for(i:first; i<last; i+=step){
				yield i;
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
	try{
		ret = FileStream(file_name, mode);
	}catch(e){
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
}

}
