#include "xtal.h"
#include "xtal_macro.h"

namespace xtal{
	


namespace{

void Iterator_each(const VMachinePtr& vm){
	vm->return_result(vm->get_arg_this());
}

void Iterator_block_first(const VMachinePtr& vm){
	vm->get_arg_this()->rawsend(vm, Xid(block_next));
}

}

class ZipIter : public Base{
public:

	ZipIter(const VMachinePtr& vm){
		next = xnew<Array>(vm->ordered_arg_count());
		for(int_t i = 0, len = next->size(); i<len; ++i){
			next->set_at(i, vm->arg(i));
		}
	}

	void common(const VMachinePtr& vm, const IDPtr& id){
		bool all = true;
		value = xnew<MultiValue>(next->size());
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
			vm->return_result(from_this(this), value);
		}
		else{
			vm->return_result(null, null);
		}
	}

	void block_first(const VMachinePtr& vm){
		common(vm, Xid(block_first));
	}
	
	void block_next(const VMachinePtr& vm){
		common(vm, Xid(block_next));
	}

	void block_break(const VMachinePtr& vm){
		IDPtr id = Xid(block_break);
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

void DelegateToIterator::rawcall(const VMachinePtr& vm){
	vm->get_arg_this()->send(Xid(each))->rawsend(vm, member_);
}

void IteratorClass::def(const IDPtr& primary_key, const AnyPtr& value, const AnyPtr& secondary_key, int_t accessibility){
	Class::def(primary_key, value, secondary_key, accessibility);
	if(rawne(Xid(p), primary_key) && rawne(Xid(each), primary_key)){
		Iterable()->def(primary_key, xnew<DelegateToIterator>(primary_key), secondary_key, accessibility);
	}
}

void block_break(AnyPtr& target){
	if(target){
		const VMachinePtr& vm = vmachine();
		vm->setup_call(0);
		target->rawsend(vm, Xid(block_break));
		if(!vm->processed()){
			vm->return_result();
		}
		vm->cleanup_call();
	}
}

bool block_next(BlockValueHolder1& holder, bool first){
	if(holder.array){
		if(holder.it==holder.array->end()){
			holder.target = null;
			holder.values[0] = null;
		}
		else{
			holder.values[0] = *holder.it;
			++holder.it;
		}
	}
	else{
		const VMachinePtr& vm = vmachine();
		vm->setup_call(2);
		holder.target->rawsend(vm, first ? Xid(block_first) : Xid(block_next));
		holder.target = vm->result(0);
		holder.values[0] = vm->result(1);
		vm->cleanup_call();
	}
	return holder.target;
}

bool block_next(BlockValueHolder2& holder, bool first){
	if(holder.map){
		if(holder.it==holder.map->end()){
			holder.target = null;
			holder.values[0] = null;
			holder.values[1] = null;
		}
		else{
			holder.values[0] = holder.it->first;
			holder.values[1] = holder.it->second;
			++holder.it;
		}
	}
	else{
		const VMachinePtr& vm = vmachine();
		vm->setup_call(3);
		holder.target->rawsend(vm, first ? Xid(block_first) : Xid(block_next));
		holder.target = vm->result(0);
		holder.values[0] = vm->result(1);
		holder.values[1] = vm->result(2);
		vm->cleanup_call();
	}
	return holder.target;
}

bool block_next(BlockValueHolder3& holder, bool first){
	const VMachinePtr& vm = vmachine();
	vm->setup_call(4);
	holder.target->rawsend(vm, first ? Xid(block_first) : Xid(block_next));
	holder.target = vm->result(0);
	holder.values[0] = vm->result(1);
	holder.values[1] = vm->result(2);
	holder.values[2] = vm->result(3);
	vm->cleanup_call();
	return holder.target;
}

BlockValueHolder1::BlockValueHolder1(const AnyPtr& tar, bool& not_end)
	:target(tar){
	not_end = tar;
	array = ptr_as<Array>(tar);
	if(array){ it = array->begin(); }
}

BlockValueHolder1::BlockValueHolder1(const ArrayPtr& tar, bool& not_end)
	:target(tar){
	not_end = tar;
	array = tar;
	if(array){ it = array->begin(); }
}

BlockValueHolder1::~BlockValueHolder1(){ 
	block_break(target); 
}
	
BlockValueHolder2::BlockValueHolder2(const AnyPtr& tar, bool& not_end)
	:target(tar){
	not_end = tar;
	map = ptr_as<Map>(tar);
	if(map){ it = map->begin(); }
}

BlockValueHolder2::BlockValueHolder2(const MapPtr& tar, bool& not_end)
	:target(tar){
	not_end = tar;
	map = tar;
	if(map){ it = map->begin(); }
}


BlockValueHolder2::~BlockValueHolder2(){ 
	block_break(target); 
}

BlockValueHolder3::BlockValueHolder3(const AnyPtr& tar, bool& not_end)
	:target(tar){
	not_end = tar;
}

BlockValueHolder3::~BlockValueHolder3(){ 
	block_break(target); 
}


///

void InitZipIter(){
	ClassPtr p = new_cpp_class<ZipIter>();
	p->inherit(Iterator());
	p->def(Xid(new), ctor<ZipIter, const VMachinePtr&>());
	p->method(Xid(block_first), &ZipIter::block_first);
	p->method(Xid(block_next), &ZipIter::block_next);
	p->method(Xid(block_break), &ZipIter::block_break);
}

void initialize_iterator(){
	{
		ClassPtr p = Iterator();
		p->inherit(Iterable());

		p->fun(Xid(each), &Iterator_each);
		p->fun(Xid(block_first), &Iterator_block_first);
	}

	InitZipIter();
	builtin()->def(Xid(zip), get_cpp_class<ZipIter>());
}

void initialize_iterator_script(){

	Xemb((

Iterator::scan: method(pattern) fiber{
	exec: xpeg::Executor(this);
	while(exec.match(pattern)){
		yield exec;
	}	
}

Iterator::p: method{
	m: MemoryStream();
	m.put_s("<[");
	a: this.take(6).to_a;
	m.put_s(a.take(5).join(","));
	if(a.length==6){
		m.put_s(" ...]>")
	}
	else{
		m.put_s("]>");
	}
	m.to_s.p;
	return chain(a.each, this);
}

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

Iterator::reverse: method{
	return this[].reverse;
}

Iterator::join: method(sep: ""){
	ret: MemoryStream();
	if(sep==""){
		this{
			ret.put_s(it.to_s);
		}
	}
	else{
		this{
			if(!first_step){
				ret.put_s(sep);
			}
			ret.put_s(it.to_s);
		}
	}
		
	return ret.to_s;
}

Iterator::with_index: method(start: 0){
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

Iterator::select: method(pred) fiber{
	this{
		if(pred(it)){
			yield it;
		}
	}
}

Iterator::filter: Iterator::select;

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

Iterator::unique: method(pred: null){
	if(pred){
		return fiber{
			prev: undefined;
			this{
				if(!pred(it, prev)){
					yield it;
					prev = it;
				}
			}
		}
	}

	return fiber{
		prev: undefined;
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

Iterator::cycle: method fiber{
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

Iterator::max_element: method(pred: null){
	item: null;
	if(pred){
		this{
			if(item){
				if(pred(it, item))
					item = it;
			}
			else{
				item = it;
			}
		}
		return item;
	}

	this{
		if(item){
			if(item<it)
				item = it;
		}
		else{
			item = it;
		}
	}
	return item;
}

Iterator::min_element: method(pred: null){
	item: null;
	if(pred){
		this{
			if(item){
				if(pred(it, item))
					item = it;
			}
			else{
				item = it;
			}
		}
		return item;
	}

	this{
		if(item){
			if(item>it)
				item = it;
		}
		else{
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

Iterator::with_prev: method fiber{
	prev: undefined;
	this{
		yield prev, it;
		prev = it;
	}
}

Iterator::flatten_param: method fiber{
	this{ yield it.flatten_mv; }
}

Iterator::flatten_all_param: method fiber{
	this{ yield it.flatten_all_mv; }
}

builtin::ClassicIterator: class{
	_current;
	_source;

	initialize: method(source){ _source, _current = source.block_first; }
	current: method _current;
	has_next: method !!_source;
	is_done: method !_source;
	next: method{ _source, _current = _source.block_next; }
	source: method _source;
}

Iterator::classic: method ClassicIterator(this);

builtin::range: fun(first, last, step:1){
	if(step==1){
		return fiber{
			for(i:first; i<last; i++){
				yield i;
			}
		}
	}
	else{
		return fiber{
			for(i:first; i<last; i+=step){
				yield i;
			}
		}
	}
}
	),
""
)->call();


}

}
