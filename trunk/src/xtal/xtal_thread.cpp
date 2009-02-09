#include "xtal.h"
#include "xtal_macro.h"

namespace xtal{

const VMachinePtr& ThreadMgr::vmachine(){
	return vmachine_;
}

ThreadMgr::ThreadMgr(ThreadLib* lib){
	thread_enabled_ = false;

	thread_count_ = 1;
	thread_locked_count_ = 0;
	thread_unlocked_count_ = 0;
	stop_the_world_ = false;
	current_thread_recursive_ = 0;

	thread_lib_ = lib;

	if(lib){
		mutex_ = new_mutex(); 
		mutex2_ = new_mutex(); 

		thread_enabled_ = true;
		global_interpreter_lock();

		register_vmachine();
	}
	else{
		vmachine_ = xnew<VMachine>();
	}
	thread_enabled_ = false;
}

ThreadMgr::~ThreadMgr(){
	vmachine_ = null;
}

void ThreadMgr::destroy(){
	global_interpreter_unlock();
	thread_enabled_ = false;
	thread_lib_ = 0;
	table.clear();

	mutex_ = null;
	mutex2_ = null;
}

void ThreadMgr::change_vmachine(const Thread::ID& id){
	if(!current_vmachine_id_.is_valid() || !thread_lib_->equal_thread_id(current_vmachine_id_, id)){
		for(uint_t i=0; i<table.size(); ++i){
			VMachineTableUnit& unit = table[i];
			if(thread_lib_->equal_thread_id(unit.id, id)){
				vmachine_ = (VMachinePtr&)unit.vm;
				current_vmachine_id_ = id;
				break;
			}
		}
	}
	current_thread_recursive_ = 1;
	current_thread_id_ = id;
}

void ThreadMgr::visit_members(Visitor& m){
	Base::visit_members(m);

	m & mutex_ & mutex2_ & vmachine_;
	for(uint_t i=0; i<table.size(); ++i){
		m & table[i].vm;
	}
}

void ThreadMgr::register_vmachine(){
	Thread::ID id;
	thread_lib_->current_thread_id(id);

	VMachineTableUnit unit;
	vmachine_ = xnew<VMachine>();
	unit.vm = vmachine_;
	unit.id = id;
	table.push_back(unit);	
}

void ThreadMgr::remove_vmachine(){
	Thread::ID id;
	thread_lib_->current_thread_id(id);

	for(uint_t i=0; i<table.size(); ++i){
		VMachineTableUnit& unit = table[i];
		if(thread_lib_->equal_thread_id(unit.id, id)){
			unit = table[table.size()-1];
			table.pop_back();
			break;
		}
	}
}

void ThreadMgr::global_interpreter_lock(){
	Thread::ID id;
	thread_lib_->current_thread_id(id);
	if(current_thread_recursive_==0){
		mutex_->lock();
		change_vmachine(id);
	}
	else{
		current_thread_recursive_++;
	}
}

void ThreadMgr::global_interpreter_unlock(){
	current_thread_recursive_--;
	if(current_thread_recursive_==0){
		current_thread_id_.invalid();
		mutex_->unlock();
	}
}

void ThreadMgr::xlock(){
	if(!thread_enabled_){
		return;
	}

	Thread::ID id;
	thread_lib_->current_thread_id(id);
	if(current_thread_recursive_==0){
		mutex_->lock();
		change_vmachine(id);
	}
	else{
		current_thread_recursive_++;
	}
}

void ThreadMgr::xunlock(){
	if(!thread_enabled_){
		return;
	}

	current_thread_recursive_--;
	if(current_thread_recursive_==0){
		current_thread_id_.invalid();
		mutex_->unlock();
	}
}

void ThreadMgr::thread_entry(const ThreadPtr& thread){
	register_thread();

	register_vmachine();
	const VMachinePtr& vm(vmachine_);

	XTAL_TRY{
		vm->setup_call(0);
		thread->callback()->rawcall(vm);
		vm->cleanup_call();
	}
	XTAL_CATCH(e){
		fprintf(stderr, "%s\n", e->to_s()->c_str());
	}

	vm->reset();	

	unregister_thread();
}

void ThreadMgr::register_thread(){
	if(!thread_enabled_)
		return;

	mutex_->lock();
	thread_count_++;
	mutex_->unlock();
	
	register_vmachine();

	global_interpreter_lock();
}

void ThreadMgr::unregister_thread(){
	if(!thread_enabled_)
		return;

	remove_vmachine();
	thread_count_--;

	global_interpreter_unlock();
}

void ThreadMgr::check_yield_thread(){
	if(thread_count_==1){
		return;
	}

	XTAL_UNLOCK{
		thread_lib_->yield();
	}
}

void ThreadMgr::yield_thread(){
	XTAL_UNLOCK{
		thread_lib_->yield();
	}
}

void ThreadMgr::sleep_thread(float_t sec){
	if(!thread_lib_)
		return;

	XTAL_UNLOCK{
		thread_lib_->sleep(sec);
	}
}

ThreadPtr ThreadMgr::new_thread(const AnyPtr& fun){
	ThreadPtr ret = thread_lib_->new_thread();
	ret->set_callback(fun);
	ret->start();
	return ret;
}

MutexPtr ThreadMgr::new_mutex(){
	return thread_lib_->new_mutex();
}

void ThreadMgr::lock_mutex(const MutexPtr& self){
	XTAL_UNLOCK{
		self->lock();
	}
}


#ifdef XTAL_USE_THREAD_MODEL_2

bool stop_the_world(){
	if(!thread_lib_){
		return true;
	}

	if(stop_the_world_){
		return false;
	}

	stop_the_world_ = true;
	thread_lib_->current_thread_id(stop_the_world_thread_id_);
	mutex2_.pvalue()->lock();	
	mutex_->unlock();
	yield_thread();
	
	int count = 0;
	while(true){
		mutex_->lock();

		if(thread_locked_count_+thread_unlocked_count_==thread_count_-1){
			if(count!=0){
				printf("ok locked=%d, unlocked=%d, count%d\n", thread_locked_count_, thread_unlocked_count_, thread_count_);
			}
			break;
		}
		else{
			printf("locked=%d, unlocked=%d, count%d\n", thread_locked_count_, thread_unlocked_count_, thread_count_);
			yield_thread();
			count++;
		}

		mutex_->unlock();
		yield_thread();
	}
	printf("start locked=%d, unlocked=%d, count%d\n", thread_locked_count_, thread_unlocked_count_, thread_count_);
	return true;
}

void restart_the_world(){
	if(!thread_lib_){
		return;
	}
	printf("restart the world\n");

	XTAL_ASSERT(stop_the_world_);

	stop_the_world_ = false;
	mutex2_.pvalue()->unlock();
}

void global_interpreter_lock(){
	Thread::ID id;
	thread_lib_->current_thread_id(id);
	if(!current_thread_id_.is_valid() || !thread_lib_->equal_thread_id(current_thread_id_, id)){

		mutex_->lock();
		for(;;){
			if(stop_the_world_){ // lockを獲得できたのが、世界が止まった所為なら
				thread_locked_count_++;

				mutex_->unlock(); // lockを開放し
				mutex2_.pvalue()->lock(); // 世界が動き出すまで待つ
				mutex2_.pvalue()->unlock();

				thread_locked_count_--;
				mutex_->lock();
			}
			else{
				break;
			}
		}

		// locked状態
		change_vmachine(id);
	}
	else{
		current_thread_recursive_++;
	}
}

void global_interpreter_unlock(){
	current_thread_recursive_--;
	if(current_thread_recursive_==0){
		current_thread_id_.invalid();
		mutex_->unlock();
	}
}

void xlock(){
	Thread::ID id;
	thread_lib_->current_thread_id(id);
	if(!current_thread_id_.is_valid() || !thread_lib_->equal_thread_id(current_thread_id_, id)){

		mutex_->lock();
		for(;;){
			if(stop_the_world_){ // lockを獲得できたのが、世界が止まった所為なら
				thread_locked_count_++;

				mutex_->unlock(); // lockを開放し
				mutex2_.pvalue()->lock(); // 世界が動き出すまで待つ
				mutex2_.pvalue()->unlock();

				mutex_->lock();
				thread_locked_count_--;
			}
			else{
				thread_unlocked_count_--;
				break;
			}
		}
		
		// locked状態
		change_vmachine(id);
	}
	else{
		current_thread_recursive_++;
	}
}

void xunlock(){
	current_thread_recursive_--;
	if(current_thread_recursive_==0){
		thread_unlocked_count_++;
		current_thread_id_.invalid();
		mutex_->unlock();
	}
}

#else

bool stop_the_world(){
	return true;
}

void restart_the_world(){

}

void global_interpreter_lock(){
	core()->thread_mgr()->global_interpreter_lock();
}

void global_interpreter_unlock(){
	core()->thread_mgr()->global_interpreter_unlock();
}

void xlock(){
	core()->thread_mgr()->xlock();
}

void xunlock(){
	core()->thread_mgr()->xunlock();
}

#endif

void thread_entry(const ThreadPtr& thread){
	core()->thread_mgr()->thread_entry(thread);
}

void register_thread(){
	core()->thread_mgr()->register_thread();
}

void unregister_thread(){
	core()->thread_mgr()->unregister_thread();
}


Thread::Thread(){
	inc_ref_count();
}

Mutex::Mutex(){
}

void Thread::visit_members(Visitor& m){
	Base::visit_members(m);
	m & callback_;
}

void Thread::begin_thread(){
	thread_entry(from_this(this));
	callback_ = null;	
	dec_ref_count();
}

}
