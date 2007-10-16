#include "xtal.h"
#include "xtal_macro.h"

#include <stdio.h>

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

	XTAL_THROW(builtin()->member("CompileError")(Xt("Xtal Runtime Error 1016")(file_name), cb.errors()->to_a()), return null);
}

CodePtr compile(const StringPtr& source){
	CodeBuilder cb;
	StringStreamPtr ms(xnew<StringStream>(source));
	if(CodePtr fun =  cb.compile(ms, "<eval>")){
		return fun;
	}

	XTAL_THROW(builtin()->member("CompileError")(Xt("Xtal Runtime Error 1002")(), cb.errors()->to_a()), return null);
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

	XTAL_THROW(builtin()->member("CompileError")(Xt("Xtal Runtime Error 1010")(), cb.errors()->to_a()), return null);
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



static float_t clock_(){
	return clock()/(float_t)CLOCKS_PER_SEC;
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
			vm->return_result(SmartPtr<ZipIter>(this), value);
		}else{
			vm->return_result(null);
		}
	}

	void block_first(const VMachinePtr& vm){
		common(vm, Xid(block_first));
	}
	
	void block_next(const VMachinePtr& vm){
		common(vm, Xid(block_next));
	}

	void block_break(const VMachinePtr& vm){
		InternedStringPtr id = Xid(block_break);
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
	p->method("block_next", &ZipIter::block_next);
	p->method("block_break", &ZipIter::block_break);
}

void initialize_builtin(){

	builtin()->def("builtin", builtin());

	builtin()->def("Iterator", Iterator());
	builtin()->def("Enumerator", Enumerator());
	
	builtin()->fun("compile_file", &compile_file);
	builtin()->fun("compile", &compile);
	builtin()->fun("gc", &gc);
	builtin()->fun("full_gc", &full_gc);
	builtin()->fun("disable_gc", &disable_gc);
	builtin()->fun("enable_gc", &enable_gc);
	builtin()->fun("clock", &clock_);

	lib()->def("builtin", builtin());
	
	InitZipIter();
	builtin()->def("zip", get_cpp_class<ZipIter>());

	Xsrc((

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

Iterator::p: method(){
	m: MemoryStream();
	m.put_s("<[");
	a: this.take(6).to_a;
	m.put_s(a.take(5).join(","));
	if(a.length==6){
		m.put_s(" ...]>")
	}else{
		m.put_s("]>");
	}
	m.to_s.p;
	return chain(a.each, this);
}

Iterator::to_s: method(){
	return "<Iterator>";
}

Iterator::to_a: method(){
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

Iterator::reverse: method(){
	return this[].reverse;
}

Iterator::join: method(sep:","){
	ret: MemoryStream();
	if(sep==""){
		this{
			ret.put_s(it.to_s);
		}
	}else{
		this{
			if(!first_step){
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
		return null;

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
	....ordered_arguments{
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
		arg.ordered_arguments{
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

Int::block_next: method{
	return (this==0 ? null : this-1), this;
}

Int::block_first: Int::block_next;

	))();

	Xsrc((

Class::ancestors: method fiber{
	this.inherited_classes{
		yield it;
		it.ancestors{
			yield it;
		}
	}
}

Array::block_first: method{
	return this.each.block_first;
}

Map::block_first: method{
	return this.each.block_first;
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

Null::block_first: method{
	return null;
}

Nop::to_s: method{
	return "nop";
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
		this.ordered_arguments.with_index{ |i, v|
			yield i, v;
		}
		this.named_arguments{ |i, v|
			yield i, v;
		}
	}
}

Arguments::pairs: Arguments::each;

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

Mutex::block_first: method{
	this.lock;
	return this;
}

Mutex::block_next: method{
	this.unlock;
	return null;
}

Mutex::block_break: method{
	this.unlock;
	return null;
}
	))();

}

}
