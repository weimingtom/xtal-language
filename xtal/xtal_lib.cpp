
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
			vm->return_result(SmartPtr<ZipIter>::from_this(this), value);
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
	p->method("block_first", &ZipIter::block_next);
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
	return a.chain(this);
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
		prev: null;
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

Any::serial_save: method{
	ret: [:];
	klass: this.class;

	klass.ancestors.to_a.r_each{
		if(n: this.instance_serial_save(it))
			ret[it.object_name] = n;
	}

	if(n: this.instance_serial_save(klass)){
		ret[klass.object_name] = n;
	}

	return ret;
}

Any::serial_load: method(v){
	klass: this.class;

	klass.ancestors.to_a.r_each{
		if(n: v[it.object_name]){
			this.instance_serial_load(it, n);
		}
	}

	if(n: v[klass.object_name]){
		this.instance_serial_load(klass, n);
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
