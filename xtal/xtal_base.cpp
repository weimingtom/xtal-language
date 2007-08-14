#include "xtal.h"

#include "xtal_base.h"
#include "xtal_macro.h"

namespace xtal{


void InitAny();
void InitDebug();
void InitString();
void InitInternedString();
void InitAny();
void InitInt();
void InitFloat();
void InitArray();
void InitMap();
void InitArguments();
void InitFun();
void InitFormat();
void InitClass();
void InitStream();
void InitCode();


namespace{

Base** objects_begin_ = 0;
Base** objects_current_ = 0;
Base** objects_end_ = 0;

GCObserver** gcobservers_begin_ = 0;
GCObserver** gcobservers_current_ = 0;
GCObserver** gcobservers_end_ = 0;

AnyPtr** llvars_begin_ = 0;
AnyPtr** llvars_current_ = 0;
AnyPtr** llvars_end_ = 0;

AnyPtr** place_begin_ = 0;
AnyPtr** place_current_ = 0;
AnyPtr** place_end_ = 0;

uint_t cycle_count_ = 0;
uint_t objects_gene_line_ = 0;

void print_alive_objects(){
	for(Base** it = objects_begin_; it!=objects_current_; ++it){
		//String* str = dynamic_cast<String*>(*it);
		//fprintf(stderr, " %s rc=%d %s\n", typeid(**it).name(), (*it)->ref_count(), str ? str->debug_c_str() : "");
		fprintf(stderr, " rc=%d\n", (*it)->ref_count());
	}
}

}

void enable_gc(){
	cycle_count_++;
}

void disable_gc(){
	cycle_count_--;
}

bool initialized(){
	return objects_begin_ ? true : false;
}

void initialize(){
	if(initialized()){ return; } 

	set_thread();

	disable_gc();

	expand_simple_dynamic_pointer_array((void**&)objects_begin_, (void**&)objects_end_, (void**&)objects_current_);

	empty_have_instance_variables.init();

	CppClassHolder<Any>::value = make_place();
	CppClassHolder<Class>::value = make_place();

	*CppClassHolder<Any>::value = ap(Innocence((Class*)Base::operator new(sizeof(Class))));
	*CppClassHolder<Class>::value = ap(Innocence((Class*)Base::operator new(sizeof(Class))));

	pvalue(get_cpp_class<Any>())->set_ref_count(1);
	pvalue(get_cpp_class<Class>())->set_ref_count(1);
	
	int_t temp_ref_count;
	
	temp_ref_count = pvalue(get_cpp_class<Any>())->ref_count(); 
	new(pvalue(get_cpp_class<Any>())) Class(Class::cpp_class_t());
	pvalue(get_cpp_class<Any>())->add_ref_count(temp_ref_count-1);
		
	temp_ref_count = pvalue(get_cpp_class<Class>())->ref_count(); 
	new(pvalue(get_cpp_class<Class>())) Class(Class::cpp_class_t());
	pvalue(get_cpp_class<Class>())->add_ref_count(temp_ref_count-1);
	
	pvalue(get_cpp_class<Any>())->set_class(get_cpp_class<Class>());
	pvalue(get_cpp_class<Class>())->set_class(get_cpp_class<Class>());

	new_cpp_class<String>();
	new_cpp_class<Null>();
	new_cpp_class<Nop>();
	new_cpp_class<True>();
	new_cpp_class<False>();
	new_cpp_class<Int>();
	new_cpp_class<Float>();
	new_cpp_class<Array>();
	new_cpp_class<Map>();
	new_cpp_class<Arguments>();
	new_cpp_class<Fiber>();
	new_cpp_class<VMachine>();
	new_cpp_class<CFun>();
	new_cpp_class<CFunArgsImpl>();
	new_cpp_class<Fiber>();
	new_cpp_class<CppClass>();
	new_cpp_class<Lib>();
	
	InitDebug();

	//finalize_id = InternedStringPtr("finalize");
	
	InitString();
	InitInternedString();
	InitAny();
	InitThread();
	InitInt();
	InitFloat();
	InitArray();
	InitMap();
	InitArguments();
	InitFun();
	InitFormat();
	InitClass();
	InitStream();
	InitCode();
	
	atexit(&uninitialize); // uninitialize

	initialize_lib();

	enable_gc();
}

void uninitialize(){
	//print_alive_objects();

	UninitThread();

	full_gc();

	for(AnyPtr** p = llvars_begin_; p!=llvars_current_; ++p){
		**p = null;
	}
	
	for(AnyPtr** p = place_begin_; p!=place_current_; ++p){
		**p = null;
	}
		
	full_gc();
	
	if(objects_current_-objects_begin_ != 0){
		//fprintf(stderr, "finished gc\n");
		//fprintf(stderr, " alive object = %d\n", objects_current_-objects_begin_);
		//print_alive_objects();
		XTAL_ASSERT(false); // 全部開放できてない

		// 強制的に全部開放する

		for(Base** it = objects_begin_; it!=objects_current_; ++it){
			delete *it;
		}

		for(Base** it = objects_begin_; it!=objects_current_; ++it){
			user_free(*it, 0);
		}

		objects_current_ = objects_begin_;
	}
	
	for(AnyPtr** p = place_begin_; p!=place_current_; ++p){
		user_free(*p, sizeof(AnyPtr));
	}
	
	llvars_current_ = llvars_begin_;
	place_current_ = place_begin_;

	fit_simple_dynamic_pointer_array((void**&)llvars_begin_, (void**&)llvars_end_, (void**&)llvars_current_);
	fit_simple_dynamic_pointer_array((void**&)gcobservers_begin_, (void**&)gcobservers_end_, (void**&)gcobservers_current_);
	fit_simple_dynamic_pointer_array((void**&)objects_begin_, (void**&)objects_end_, (void**&)objects_current_);
	fit_simple_dynamic_pointer_array((void**&)place_begin_, (void**&)place_end_, (void**&)place_current_);

	//
}

void add_long_life_var(AnyPtr* a, int_t n){
	for(AnyPtr** p = llvars_begin_; p!=llvars_current_; ++p){
		if(*p==a){
			return;
		}
	}

	for(int_t i = 0; i<n; ++i){
		if(llvars_current_==llvars_end_){
			expand_simple_dynamic_pointer_array((void**&)llvars_begin_, (void**&)llvars_end_, (void**&)llvars_current_);
		}
		*llvars_current_++ = a++;
	}
}

void remove_long_life_var(AnyPtr* a, int_t n){
	for(AnyPtr** p = llvars_begin_; p!=llvars_current_; ++p){
		if(*p==a){
			std::memmove(p, p+n, sizeof(AnyPtr*)*n);
			llvars_current_ -= n;
			break;
		}
	}
}

AnyPtr* make_place(){
	if(place_current_==place_end_){
		expand_simple_dynamic_pointer_array((void**&)place_begin_, (void**&)place_end_, (void**&)place_current_);
	}
	*place_current_ = new(user_malloc(sizeof(AnyPtr))) AnyPtr();
	return *place_current_++;
}

struct CycleCounter{
	uint_t* p;
	CycleCounter(uint_t* p):p(p){ *p+=1; }
	~CycleCounter(){ *p-=1; }
};


void gc(){
	if(cycle_count_!=0){ return; }
	if(stop_the_world()){
		const VMachinePtr& vm = vmachine();

		CycleCounter cc(&cycle_count_);

		for(GCObserver** it = gcobservers_begin_; it!=gcobservers_current_; ++it){
			(*it)->before_gc();
		}

		if((int)objects_gene_line_>objects_current_-objects_begin_){
			objects_gene_line_ = 0;
		}

		Base** objects_alive = objects_begin_+objects_gene_line_;

		for(Base** it = objects_alive; it!=objects_current_; ++it){
			if((*it)->ref_count()!=0){
				std::swap(*it, *objects_alive++);
			}
		}

		for(GCObserver** it = gcobservers_begin_; it!=gcobservers_current_; ++it){
			(*it)->after_gc();
		}

		for(Base** it = objects_alive; it!=objects_current_; ++it){
			delete *it;
		}

		for(Base** it = objects_alive; it!=objects_current_; ++it){
			user_free(*it, 0);
		}
		objects_current_ = objects_alive;

		// 2/3
		objects_gene_line_ = (objects_current_-objects_begin_)*2/3;

		//fprintf(stderr, "finished gc\n");
		//fprintf(stderr, " alive object = %d\n", objects_current_-objects_begin_);

		restart_the_world();
	}
}

void full_gc(){
	if(cycle_count_!=0){ return; }
	if(stop_the_world()){
		const VMachinePtr& vm = vmachine();

		CycleCounter cc(&cycle_count_);
		
		Base** prev_oc;
		do{
			
			for(GCObserver** it = gcobservers_begin_; it!=gcobservers_current_; ++it){
				(*it)->before_gc();
			}

			prev_oc = objects_current_;

			{
				Visitor m(-1);	
				for(Base** it = objects_begin_; it!=objects_current_; ++it){
					(*it)->visit_members(m);
				}
			}

			{
				Base** objects_alive = objects_begin_;

				{
					Visitor m(1);
					bool end = false;
					while(!end){
						end = true;
						for(Base** it = objects_alive; it!=objects_current_; ++it){
							if((*it)->ref_count()!=0){
								end = false;
								(*it)->visit_members(m);
								std::swap(*it, *objects_alive++);
							}
						}
					}
				}
	

				{// 
					Visitor m(1);
					for(Base** it = objects_alive; it!=objects_current_; ++it){
						(*it)->visit_members(m);
					}
				}

				for(GCObserver** it = gcobservers_begin_; it!=gcobservers_current_; ++it){
					(*it)->after_gc();
				}

				for(Base** it = objects_alive; it!=objects_current_; ++it){
					delete *it;
				}

				for(Base** it = objects_alive; it!=objects_current_; ++it){
					user_free(*it, 0);
				}
				objects_current_ = objects_alive;
			}

		}while(prev_oc!=objects_current_);
		
		//fprintf(stderr, "finished full_gc\n");
		//fprintf(stderr, " alive object = %d\n", objects_current_-objects_begin_);

		restart_the_world();
	}
}

	
void* Base::operator new(size_t size){
	if(objects_current_==objects_end_){
		expand_simple_dynamic_pointer_array((void**&)objects_begin_, (void**&)objects_end_, (void**&)objects_current_);
	}	
	Base* p = static_cast<Base*>(user_malloc(size));
	*objects_current_++=p;
	
	p->ref_count_ = 1;
	p->class_ = null;
	
	return p;
}
	
void Base::operator delete(void* p){

}


int_t Base::to_i(){
	return cast<int_t>(send(Xid(to_i)));
}

float_t Base::to_f(){
	return cast<float_t>(send(Xid(to_f)));
}

StringPtr Base::to_s(){
	return cast<StringPtr>(send(Xid(to_s)));
}

AnyPtr Base::p(){
	return send(Xid(p));
}


void Base::set_class(const ClassPtr& c){
	class_ = c;
}

void Base::visit_members(Visitor& m){
	//m & class_;
}
	
void Base::call(const VMachinePtr& vm){
	ap(Innocence(this))->rawsend(vm, Xid(op_call));
}

const AnyPtr& Base::member(const InternedStringPtr& name, const AnyPtr& self, const AnyPtr& ns){ 
	return null; 
}

void Base::def(const InternedStringPtr& name, const AnyPtr& value, int_t accessibility, const AnyPtr& ns){

}

AnyPtr Base::send(const InternedStringPtr& name){
	const VMachinePtr& vm = vmachine();
	vm->setup_call(1);
	rawsend(vm, name);
	return vm->result_and_cleanup_call();
}

void Base::rawsend(const VMachinePtr& vm, const InternedStringPtr& name, const AnyPtr& self, const AnyPtr& ns){
	ap(Innocence(this))->rawsend(vm, name, self, ns);
}

HaveInstanceVariables* Base::have_instance_variables(){ 
	return &empty_have_instance_variables; 
}

StringPtr Base::object_name(){ 
	return xnew<String>("instance of ")->cat(get_class()->object_name());
}

int_t Base::object_name_force(){ 
	return 0;
}

void Base::set_object_name(const StringPtr& name, int_t force, const AnyPtr& parent){

}

bool Base::is(const ClassPtr& v){
	return get_class()->is_inherited(v);
}

uint_t Base::hashcode(){
	return (uint_t)this;
}


GCObserver::GCObserver(){
	if(gcobservers_current_==gcobservers_end_){
		expand_simple_dynamic_pointer_array((void**&)gcobservers_begin_, (void**&)gcobservers_end_, (void**&)gcobservers_current_);
	}
	*gcobservers_current_++ = this;
}

GCObserver::GCObserver(const GCObserver& v)
:Base(v){
	if(gcobservers_current_==gcobservers_end_){
		expand_simple_dynamic_pointer_array((void**&)gcobservers_begin_, (void**&)gcobservers_end_, (void**&)gcobservers_current_);
	}
	*gcobservers_current_++ = this;
}
	
GCObserver::~GCObserver(){
	for(GCObserver** p = gcobservers_begin_; p!=gcobservers_current_; ++p){
		if(*p==this){
			std::swap(*p, *--gcobservers_current_);
			break;
		}
	}
}

StringPtr HaveName::object_name(){
	if(!name_)
		return xnew<String>("<instance of ")->cat(get_class()->object_name())->cat(">");
	if(!parent_)
		return name_;
	return parent_->object_name()->cat("::")->cat(name_);
}

int_t HaveName::object_name_force(){
	return force_;
}

void HaveName::set_object_name(const StringPtr& name, int_t force, const AnyPtr& parent){
	if(!name_ || force_<force){
		name_ = name;
		force_ = force;
		parent_ = parent;
	}
}


}

