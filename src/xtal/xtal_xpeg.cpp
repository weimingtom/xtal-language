#include "xtal.h"
#include "xtal_bind.h"
#include "xtal_macro.h"

namespace xtal{ namespace xpeg{

enum{ NFA_STATE_START = 0, NFA_STATE_FINAL = 1 };

void Executor::reset(const AnyPtr& stream_or_iterator){
	if(stream_or_iterator->is(cpp_class<Stream>())){
		scanner_ = xnew<StreamScanner>(ptr_cast<Stream>(stream_or_iterator));
	}
	else if(stream_or_iterator->is(cpp_class<String>())){
		scanner_ = xnew<StreamScanner>(xnew<StringStream>(ptr_cast<String>(stream_or_iterator)));
	}
	else{
		scanner_ = xnew<IteratorScanner>(stream_or_iterator);
	}

	cap_ = xnew<Map>();
	tree_ = xnew<TreeNode>();
	errors_ = null;
	begin_ = 0;
	match_begin_ = 0;
	match_end_ = 0;
}

bool Executor::match(const AnyPtr& pattern){
	const NFAPtr& nfa = fetch_nfa(elem(pattern));
	begin_ = scanner_->pos();
	for(;;){
		match_begin_ = scanner_->pos();
		if(match_inner(nfa)){
			match_end_ = scanner_->pos();
			return !errors_;
		}

		if(scanner_->eos()){
			return false;
		}

		scanner_->read();
	}
}

bool Executor::parse(const AnyPtr& pattern){
	return match_inner(fetch_nfa(elem(pattern))) && (!errors_ || errors_->empty());
}

AnyPtr Executor::captures(){
	MapPtr ret = xnew<Map>();

	ret->set_at(empty_id, at(empty_string));
	Xfor2(k, v, cap_){
		XTAL_UNUSED_VAR(v);
		const StringPtr& k2 = unchecked_ptr_cast<String>(k);
		ret->set_at(k2, at(k2));
	}

	return ret->each();
}

AnyPtr Executor::captures_values(){
	MapPtr ret = xnew<Map>();

	ret->set_at(empty_id, at(empty_string));
	Xfor2(k, v, cap_){
		XTAL_UNUSED_VAR(v);
		const StringPtr& k2 = unchecked_ptr_cast<String>(k);
		ret->set_at(k2, call(k2));
	}

	return ret->each();
}

StringPtr Executor::at(const StringPtr& key){
	if(raweq(key, empty_id)){
		return unchecked_ptr_cast<Scanner>(scanner_)->capture(match_begin_, match_end_);
	}
	else{
		const SmartPtr<Cap>& temp = unchecked_ptr_cast<Cap>(cap_->at(key));

		if(temp->end>=0 && temp->end-temp->begin>0){
			return scanner_->capture(temp->begin, temp->end);
		}
		else{
			if(temp->end==temp->begin){
				return empty_string;
			}
			else{
				return null;
			}
		}
	}
}

AnyPtr Executor::call(const StringPtr& key){
	if(raweq(key, empty_id)){
		return unchecked_ptr_cast<Scanner>(scanner_)->capture_values(match_begin_, match_end_);
	}
	else{		
		const SmartPtr<Cap>& temp = unchecked_ptr_cast<Cap>(cap_->at(key));

		if(temp->end>=0 && temp->end-temp->begin>0){
			return scanner_->capture_values(temp->begin, temp->end);
		}
		else{
			if(temp->end==temp->begin){
				return xnew<Array>();
			}
			else{
				return null;
			}
		}
	}
}

StringPtr Executor::prefix(){
	return unchecked_ptr_cast<Scanner>(scanner_)->capture(begin_, match_begin_);
}

StringPtr Executor::suffix(){
	return unchecked_ptr_cast<Scanner>(scanner_)->capture(match_end_);
}

AnyPtr Executor::prefix_values(){
	return unchecked_ptr_cast<Scanner>(scanner_)->capture_values(begin_, match_begin_)->each();
}

AnyPtr Executor::suffix_values(){
	return unchecked_ptr_cast<Scanner>(scanner_)->capture_values(match_end_)->each();
}

const AnyPtr& Executor::read(){
	return scanner_->read();
}

const AnyPtr& Executor::peek(uint_t n){
	return scanner_->peek(n);
}

const StringPtr& Executor::peek_s(uint_t n){
	const AnyPtr& ret = peek(n);
	if(raweq(ret, undefined)){
		return empty_string;
	}
	return unchecked_ptr_cast<String>(ret);
}

int_t Executor::peek_ascii(uint_t n){
	const AnyPtr& ret = peek(n);
	if(raweq(ret, undefined)){
		return -1;
	}
	return unchecked_ptr_cast<String>(ret)->ascii();
}

const StringPtr& Executor::read_s(){
	const AnyPtr& ret = read();
	if(raweq(ret, undefined)){
		return empty_string;
	}
	return unchecked_ptr_cast<String>(ret);
}

int_t Executor::read_ascii(){
	const AnyPtr& ret = read();
	if(raweq(ret, undefined)){
		return -1;
	}
	return unchecked_ptr_cast<String>(ret)->ascii();		
}

uint_t Executor::lineno(){
	return scanner_->lineno();
}

bool Executor::bos(){
	return scanner_->bos();
}

bool Executor::eos(){
	return scanner_->eos();
}

bool Executor::bol(){
	return scanner_->bol();
}

bool Executor::eol(){
	return scanner_->eol();
}

bool Executor::eat(const AnyPtr& v){
	if(raweq(peek(), v)){
		skip();
		return true;
	}
	return false;
}

bool Executor::eat_ascii(int_t ch){
	if(peek_s()->ascii()==ch){
		skip();
		return true;
	}
	return false;
}

void Executor::skip_eol(){
	return scanner_->skip_eol();
}

void Executor::skip(uint_t n){
	scanner_->skip(n);
}

void Executor::skip(){
	scanner_->read();
}

void Executor::error(const AnyPtr& message, int_t line){
	if(!errors_){
		errors_ = xnew<Array>();
	}

	if(errors_->size()<10){
		errors_->push_back(Xf("%(lineno)d:%(message)s")->call(
			Named(Xid(lineno), line==0 ? lineno() : line),
			Named(Xid(message), message)
		));
	}
}

void Executor::push(uint_t mins, uint_t cur_state, uint_t nodes, const State& pos){
	for(uint_t i=mins, sz=stack_.size(); i<sz; ++i){
		if(stack_.reverse_at(i).pos.pos != pos.pos){
			break;
		}
		else if(stack_.reverse_at(i).state == cur_state){
			return;
		}
	}

	StackInfo temp = {cur_state, nodes, pos};
	stack_.push(temp);
}

bool Executor::match_inner(const AnyPtr& anfa){
	const NFAPtr& nfa = unchecked_ptr_cast<NFA>(anfa);

	// メモ化したい
	// (スキャナーの位置、NFAのポインタ値) がキー
	Key key;
	key.pos = scanner_->pos();
	key.ptr = nfa.get();

	memotable_t::iterator it=memotable_.find(key);
	if(it!=memotable_.end()){
		scanner_->load(it->second.state);
		tree_->op_cat_assign(it->second.tree);
		return true;
	}

	int_t nodenum = tree_->size();
	uint_t mins = stack_.size();

	for(uint_t i=0, sz=nfa->cap_list_->size(); i<sz; ++i){
		cap_->set_at(nfa->cap_list_->at(i), xnew<Cap>());
	}

	bool match = false;
	State match_pos = {0, 0};
	State first_pos = scanner_->save();

	push(mins, NFA_STATE_START, nodenum, first_pos);

	while(stack_.size()>mins){
		StackInfo se = stack_.pop();
		int cur_state = se.state;
		
		State& pos = se.pos;
		NFA::State& state = nfa->states_[cur_state];

		if(cur_state==NFA_STATE_FINAL){
			if(match_pos.pos <= pos.pos){
				match_pos = pos;
			}
			match = true;
		}

		bool fail = true;
		for(const TransPtr* tr=&state.trans; *tr; tr=&(*tr)->next){
			scanner_->load(pos);

			if(se.nodes<tree_->size()){
				tree_->resize(se.nodes);
			}

			// パース成功
			if(test((*tr)->ch)){
				push(mins, (*tr)->to, tree_->size(), scanner_->save());
				fail = false;
			}
		}

		if(fail){
			if(match){
				break;
			}
		}
		else{
			switch(state.capture_kind){
				XTAL_NODEFAULT;
				XTAL_CASE(NFA::CAPTURE_NONE){}

				XTAL_CASE(NFA::CAPTURE_BEGIN){
					const SmartPtr<Cap>& temp = unchecked_ptr_cast<Cap>(cap_->at(state.capture_name));
					temp->begin = pos.pos;
					temp->end = -1;
				}

				XTAL_CASE(NFA::CAPTURE_END){
					const SmartPtr<Cap>& temp = unchecked_ptr_cast<Cap>(cap_->at(state.capture_name));
					temp->end = pos.pos;
				}
			}
		}
	}

	stack_.downsize_n(mins);

	if(match){
		Value value;
		value.state = match_pos;
		value.tree = tree_->slice(nodenum, tree_->size()-nodenum);
		memotable_[key] = value;
		scanner_->load(match_pos);
		return true;
	}

	tree_->resize(nodenum);

	scanner_->load(first_pos);
	return false;
}

bool Executor::test(const AnyPtr& ae){
	const ElementPtr& e = unchecked_ptr_cast<Element>(ae);

	switch(e->type){
		XTAL_NODEFAULT;

		case Element::TYPE_CONCAT:
		case Element::TYPE_OR:
		case Element::TYPE_MORE0:
		case Element::TYPE_MORE1:
		case Element::TYPE_01:
		case Element::TYPE_EMPTY:
		case Element::TYPE_CAP:
		case Element::TYPE_DECL:
			XTAL_ASSERT(false);
			break;

		XTAL_CASE(Element::TYPE_INVALID){
			return true;
		}

		XTAL_CASE(Element::TYPE_ANY){
			if(scanner_->eos()){ return false; }
			scanner_->read();
			return !e->inv;
		}

		XTAL_CASE(Element::TYPE_BOS){
			return scanner_->bos()!=e->inv;
		}

		XTAL_CASE(Element::TYPE_EOS){
			return scanner_->eos()!=e->inv;
		}

		XTAL_CASE(Element::TYPE_BOL){
			return scanner_->bol()!=e->inv;
		}

		XTAL_CASE(Element::TYPE_EOL){
			if(!scanner_->eol()){ return e->inv; }
			scanner_->skip_eol();
			return !e->inv;
		}

		XTAL_CASE(Element::TYPE_EQL){
			if(scanner_->eos()){ return false; }
			if(raweq(scanner_->read(), e->param1)){ return !e->inv; }
			return e->inv;
		}

		XTAL_CASE(Element::TYPE_INT_RANGE){
			if(scanner_->eos()){ return false; }
			const AnyPtr& a = scanner_->peek();
			if(rawtype(a)==TYPE_INT){
				return unchecked_ptr_cast<Int>(a)->op_in(unchecked_ptr_cast<IntRange>(e->param1))!=e->inv;
			}
			return e->inv;
		}

		XTAL_CASE(Element::TYPE_FLOAT_RANGE){
			if(scanner_->eos()){ return false; }
			const AnyPtr& a = scanner_->peek();
			if(rawtype(a)==TYPE_FLOAT){
				return unchecked_ptr_cast<Float>(a)->op_in(unchecked_ptr_cast<FloatRange>(e->param1))!=e->inv;
			}
			return e->inv;
		}

		XTAL_CASE(Element::TYPE_CH_RANGE){
			if(scanner_->eos()){ return false; }
			const AnyPtr& a = scanner_->read();
			if(rawtype(a)==TYPE_SMALL_STRING){
				return unchecked_ptr_cast<String>(a)->op_in(unchecked_ptr_cast<ChRange>(e->param1))!=e->inv;
			}
			return e->inv;
		}

		XTAL_CASE(Element::TYPE_PRED){
			if(scanner_->eos()){ return false; }
			if(e->param1->call(scanner_->read())){ return !e->inv; }
			return e->inv;
		}

		XTAL_CASE(Element::TYPE_CH_SET){
			if(scanner_->eos()){ return false; }
			const MapPtr& data = unchecked_ptr_cast<Map>(e->param1);
			if(data->at(scanner_->read())){ return !e->inv; }
			return e->inv;
		}

		XTAL_CASE(Element::TYPE_CALL){
			if(scanner_->eos()){ return false; }
			AnyPtr ret = e->param1->call(to_smartptr(this));
			return (ret || raweq(ret, undefined))!=e->inv;
		}

		XTAL_CASE(Element::TYPE_GREED){
			const NFAPtr& nfa = fetch_nfa(unchecked_ptr_cast<Element>(e->param1));
			return match_inner(nfa)!=e->inv;
		}

		XTAL_CASE(Element::TYPE_LOOKAHEAD){
			const NFAPtr& nfa = fetch_nfa(unchecked_ptr_cast<Element>(e->param1));
			Scanner::State state = scanner_->save();
			bool ret = match_inner(nfa);
			scanner_->load(state);
			return ret!=e->inv;
		}

		XTAL_CASE(Element::TYPE_LOOKBEHIND){
			const NFAPtr& nfa = fetch_nfa(unchecked_ptr_cast<Element>(e->param1));
			Scanner::State state = scanner_->save();
			Scanner::State fict_state = state;
			fict_state.pos = fict_state.pos > (uint_t)e->param3 ? fict_state.pos-e->param3 : 0;
			scanner_->load(fict_state);
			bool ret = match_inner(nfa);
			scanner_->load(state);
			return ret!=e->inv;
		}

		XTAL_CASE(Element::TYPE_LEAF){
			const NFAPtr& nfa = fetch_nfa(unchecked_ptr_cast<Element>(e->param1));
			int_t pos = scanner_->pos();
			if(match_inner(nfa)){
				if(tree_){
					if(e->param3!=0){
						tree_->push_back(scanner_->capture_values(pos, scanner_->pos()));
					}
					else{
						tree_->push_back(scanner_->capture(pos, scanner_->pos()));
					}
				}
				return !e->inv;
			}
			return e->inv;
		}

		XTAL_CASE(Element::TYPE_NODE){
			const NFAPtr& nfa = fetch_nfa(unchecked_ptr_cast<Element>(e->param1));
			//int_t pos = scanner_->pos();
			if(tree_){
				int_t nodenum = tree_->size() - e->param3;
				if(nodenum<0){ nodenum = 0; }

				if(match_inner(nfa)){
					TreeNodePtr node = xnew<TreeNode>();
					node->set_tag(e->param2->to_s()->intern());
					node->set_lineno(scanner_->lineno());
					node->assign(tree_->splice(nodenum, tree_->size()-nodenum));
					tree_->push_back(node);
					return !e->inv;
				}
			}
			else{
				if(match_inner(nfa)){
					return !e->inv;
				}
			}
			return e->inv;
		}

		XTAL_CASE(Element::TYPE_BACKREF){
			const SmartPtr<Cap>& temp = unchecked_ptr_cast<Cap>(cap_->at(e->param1));
			if(temp && temp->end>=0 && temp->end-temp->begin>0){
				return (scanner_->eat_capture(temp->begin, temp->end))!=e->inv;
			}
			return e->inv;
		}

		XTAL_CASE(Element::TYPE_ERROR){
			if(errors_){
				errors_->push_back(e->param1->call(Named(Xid(line), scanner_->lineno())));
			}
			return !e->inv;
		}
	}

	return false;
}

////////////////////////////////////////////////////////////////////////

Scanner::Scanner(){
	num_ = 0;
	begin_ = 0;
	max_ = 0;

	mm_ = xnew<MemoryStream>();
	pos_ = 0;
	read_ = 0;
	base_ = 0;
	record_pos_ = -1;

	n_ch_ = XTAL_STRING("\n");
	r_ch_ = XTAL_STRING("\r");

	lineno_ = 1;

	expand();
}

Scanner::~Scanner(){
	for(uint_t i=base_; i<num_; ++i){
		for(int j=0, sz=ONE_BLOCK_SIZE; j<sz; ++j){
			begin_[i-base_][j] = null;
		}

		xfree(begin_[i-base_], sizeof(AnyPtr)*ONE_BLOCK_SIZE);
	}
	xfree(begin_, sizeof(AnyPtr*)*max_);
}

const AnyPtr& Scanner::peek(uint_t n){
	while(pos_+n >= read_){
		uint_t now_read = 0;

		if((read_>>ONE_BLOCK_SHIFT)==num_){
			expand();
		}

		now_read = do_read(&access(read_), ONE_BLOCK_SIZE-(read_&ONE_BLOCK_MASK));

		if(now_read==0){
			return undefined;
		}

		read_ += now_read;
	}
	
	return access(pos_+n);
}

const AnyPtr& Scanner::read(){
	const AnyPtr& ret = peek();
	if(raweq(ret, r_ch_)){
		if(rawne(peek(1), n_ch_)){
			lineno_++;
		}
	}
	else if(raweq(ret, n_ch_)){
		lineno_++;
	}

	pos_ += 1;
	return  ret;
}

bool Scanner::bol(){
	if(pos_==0){
		return true;
	}

	const AnyPtr& ch = access(pos_-1);
	return raweq(ch, n_ch_) || raweq(ch, r_ch_);
}

bool Scanner::eol(){
	const AnyPtr& ch = peek();
	return raweq(ch, r_ch_) || raweq(ch, n_ch_);
}

void Scanner::skip(uint_t n){
	for(uint_t i=0; i<n; ++i){
		read();
	}
}

void Scanner::skip_eol(){
	const AnyPtr& ch = peek();
	if(raweq(ch, r_ch_)){
		if(raweq(peek(1), n_ch_)){
			skip(2);
		}
		else{
			skip(1);
		}
	}
	else if(raweq(ch, n_ch_)){
		skip(1);
	}
}

ArrayPtr Scanner::capture_values(int_t begin, int_t end){
	ArrayPtr ret = xnew<Array>();
	for(int_t i=begin; i<end; ++i){
		ret->push_back(access(i));
	}
	return ret;
}

ArrayPtr Scanner::capture_values(int_t begin){
	ArrayPtr ret = xnew<Array>();
	int_t saved = pos_;
	pos_ = begin;
	while(!eos()){
		ret->push_back(read());
	}
	pos_ = saved;
	return ret;
}
	
StringPtr Scanner::capture(int_t begin, int_t end){
	mm_->clear();
	for(int_t i=begin; i<end; ++i){
		mm_->put_s(access(i)->to_s());
	}
	return mm_->to_s();
}

StringPtr Scanner::capture(int_t begin){
	mm_->clear();
	int_t saved = pos_;
	pos_ = begin;
	while(!eos()){
		mm_->put_s(read()->to_s());
	}
	pos_ = saved;
	return mm_->to_s();
}

bool Scanner::eat_capture(int_t begin, int_t end){
	for(int_t i=begin; i<end; ++i){
		if(rawne(peek(i-begin), access(i))){
			return false;
		}
	}
	skip(end-begin);
	return true;
}

void Scanner::begin_record(){
	record_pos_ = pos_;
}

StringPtr Scanner::end_record(){
	if(record_pos_<0){
		return empty_string;
	}

	int_t begin = record_pos_;
	record_pos_ = -1;
	return capture(begin, pos_);
}

void Scanner::expand(){
	if(max_==num_){
		uint_t newmax = max_ + max_/2 + 4;
		AnyPtr** newp = (AnyPtr**)xmalloc(sizeof(AnyPtr*)*newmax);

		if(begin_){
			std::memcpy(newp, begin_, sizeof(AnyPtr*)*num_);
		}

		for(int i=num_; i<newmax; ++i){
			newp[i] = 0;
		}

		xfree(begin_, sizeof(AnyPtr*)*max_);
		begin_ = newp;
		max_ = newmax;
	}

	//XTAL_ASSERT(begin_[num_-base_]==0);

	begin_[num_-base_] = (AnyPtr*)xmalloc(sizeof(AnyPtr)*ONE_BLOCK_SIZE);
	std::memset(begin_[num_-base_], 0, sizeof(AnyPtr)*ONE_BLOCK_SIZE);
	num_++;
}
	
void Scanner::bin(){
		return;
	if(record_pos_<0){
	}

	int n = (int)((pos_>>ONE_BLOCK_SHIFT)-base_);

	if(n>0){
		for(int i=0; i<num_; ++i){
			if((int)num_<=n+i){
				break;
			}

			std::swap(begin_[i], begin_[n+i]);
		}

		base_ += n;
	}
}

IteratorScanner::IteratorScanner(const AnyPtr& iter)
	:iter_(iter->send(Xid(each))){}

int_t IteratorScanner::do_read(AnyPtr* buffer, int_t max){
	if(!iter_){
		return 0;
	}

	const VMachinePtr& vm = vmachine();
	for(int_t i=0; i<max; ++i){
		vm->setup_call(2);
		iter_->rawsend(vm, Xid(block_next));
		iter_ = vm->result(0);
		if(!iter_){
			vm->cleanup_call();
			return i;
		}
		buffer[i] = vm->result(1);
		vm->cleanup_call();
	}
	return max;
}
	
NFA::NFA(const ElementPtr& node){
	e_ = xnew<Element>(Element::TYPE_INVALID);

	root_node_ = node;
	cap_max_ = 0;
	cap_list_ = xnew<Array>();

	gen_state(); // NFA_STATE_START
	gen_state(); // NFA_STATE_FINAL

	gen_nfa(NFA_STATE_START, node, NFA_STATE_FINAL, 0);
}

int NFA::gen_state(){
	State state;
	state.capture_kind = 0;
	states_.push_back(state);
	return states_.size() - 1;
}

void NFA::add_transition(int from, const AnyPtr& ch, int to){
	TransPtr x = xnew<Trans>();
	x->to = to;
	x->ch = ptr_cast<Element>(ch);

	/*
	if(states_[from].trans){
		TransPtr cur = states_[from].trans;
		while(cur->next){
			cur = cur->next;
		}
		cur->next = x;
	}
	else{
		states_[from].trans = x;
	}
	*/

	x->next = states_[from].trans;
	states_[from].trans = x;
}

void NFA::gen_nfa(int entry, const AnyPtr& a, int exit, int depth){
	const ElementPtr& t = unchecked_ptr_cast<Element>(a);

	switch(t->type){
		XTAL_DEFAULT{
			printn("DEFAULT", depth);
			//         ch
			//  entry -----> exit
			add_transition(entry, t, exit);
		}

		XTAL_CASE(Element::TYPE_CONCAT){
			printn("CONCAT", depth);
			//         left         right
			//  entry ------> step -------> exit
			int step = gen_state();
			gen_nfa(entry, t->param1, step, depth+1);
			gen_nfa(step, t->param2, exit, depth+1);
		}

		XTAL_CASE(Element::TYPE_OR){
			printn("OR", depth);
			//               left
			//        ----------------->
			//  entry -----> step -----> exit
			//          e         right
			int step = gen_state();
			gen_nfa(entry, t->param1, exit, depth+1);
			add_transition(entry, e_, step);
			gen_nfa(step, t->param2, exit, depth+1);
		}

		XTAL_CASE2(Element::TYPE_MORE0, Element::TYPE_MORE1){
			printn("MORE", depth);
			//                       e
			//         e          <------        e
			//  entry ---> before ------> after ---> exit
			//    |                left                ^
			//    >------->------------------->------>-|
			//                      e
			//----------------------------------------------
			//                       e
			//         e          <------        e
			//  entry ---> before ------> after ---> exit
			//                     left
			int before = gen_state();
			int after = gen_state();
			
			if(t->param3==1){
				// exitに向かう方が優先
				if(t->type == Element::TYPE_MORE0){ add_transition(entry, e_, exit); }
				add_transition(entry, e_, before);

				// exitに向かう方が優先
				add_transition(after, e_, exit);
				add_transition(after, e_, before);
			}
			else{
				// beforeに向かう方が優先
				add_transition(entry, e_, before);
				if(t->type == Element::TYPE_MORE0){ add_transition(entry, e_, exit); }

				// beforeに向かう方が優先
				add_transition(after, e_, before);
				add_transition(after, e_, exit);
			}

			// beforeからafterへの遷移 
			gen_nfa(before, t->param1, after, depth+1);
		}

		XTAL_CASE(Element::TYPE_01){
			printn("01", depth);
			//           e
			//        ------>
			//  entry ------> exit
			//         left
			if(t->param3==1){
				// eを経由する方が優先
				add_transition(entry, e_, exit);
				gen_nfa(entry, t->param1, exit, depth+1);
			}
			else{
				// leftを経由する方が優先
				gen_nfa(entry, t->param1, exit, depth+1);
				add_transition(entry, e_, exit);
			}
		}

		XTAL_CASE(Element::TYPE_EMPTY){
			printn("EMPTY", depth);
			//         e
			//  entry ---> exit
			add_transition(entry, e_, exit);
		}

		XTAL_CASE(Element::TYPE_CAP){
			printn("CAP", depth);
			int before = gen_state();
			int after = gen_state();

			states_[before].capture_kind = CAPTURE_BEGIN;
			states_[after].capture_kind = CAPTURE_END;
			states_[before].capture_name = unchecked_ptr_cast<ID>(t->param2);
			states_[after].capture_name = unchecked_ptr_cast<ID>(t->param2);
			cap_list_->push_back(t->param2);
			cap_max_++;

			add_transition(entry, e_, before);
			gen_nfa(before, t->param1, after, depth+1);
			add_transition(after, e_, exit);
		}

		XTAL_CASE(Element::TYPE_DECL){
			printn("DECL", depth);
			add_transition(entry, t, exit);
			//gen_nfa(entry, t->param1, exit, depth+1);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////

ElementPtr elem(const AnyPtr& a){
	if(const ElementPtr& p = ptr_cast<Element>(a)){
		return p;
	}

	if(can_cast<IntRange>(a)){
		return xnew<Element>(Element::TYPE_INT_RANGE, a);
	}

	if(can_cast<FloatRange>(a)){
		return xnew<Element>(Element::TYPE_FLOAT_RANGE, a);
	}

	if(can_cast<ChRange>(a)){
		return xnew<Element>(Element::TYPE_CH_RANGE, a);
	}

	if(const StringPtr& p = ptr_cast<String>(a)){
		if(p->length()==0){
			return xnew<Element>(Element::TYPE_EMPTY);
		}
		
		if(p->length()==1){
			return xnew<Element>(Element::TYPE_EQL, p);
		}

		AnyPtr str = null;
		Xfor(v, p->each()){
			if(str){
				str = concat(str, v);
			}
			else{
				str = v;
			}
		}
		
		return elem(str);
	}

	if(const FunPtr& p = ptr_cast<Fun>(a)){
		return xnew<Element>(Element::TYPE_CALL, p);
	}

	XTAL_SET_EXCEPT(cpp_class<RuntimeError>()->call(Xt("XRE1026")));
	return null;
}

AnyPtr set(const StringPtr& str){
	SetPtr chset = xnew<Set>();
	Xfor(v, str){
		chset->set_at(v, true);
	}
	return xnew<Element>(Element::TYPE_CH_SET, chset);
}

AnyPtr call(const AnyPtr& fun){
	return xnew<Element>(Element::TYPE_CALL, fun);
}

AnyPtr select(const AnyPtr& left, const AnyPtr& right){
	ElementPtr eleft = elem(left);
	ElementPtr eright = elem(right);

	if((eleft->type==Element::TYPE_EQL || eleft->type==Element::TYPE_CH_SET) && 
		(eright->type==Element::TYPE_EQL || eright->type==Element::TYPE_CH_SET)){

		SetPtr chset = xnew<Set>();

		if(eleft->type==Element::TYPE_EQL){
			chset->set_at(eleft->param1, true);
		}

		if(eleft->type==Element::TYPE_CH_SET){
			Xfor(v, eleft->param1){
				chset->set_at(v, true);
			}
		}

		if(eright->type==Element::TYPE_EQL){
			chset->set_at(eright->param1, true);
		}

		if(eright->type==Element::TYPE_CH_SET){
			Xfor(v, eright->param1){
				chset->set_at(v, true);
			}
		}
		return xnew<Element>(Element::TYPE_CH_SET, chset);
	}

	return xnew<Element>(Element::TYPE_OR, eleft, eright); 
}

AnyPtr concat(const AnyPtr& left, const AnyPtr& right){ 
	ElementPtr eleft = elem(left);
	ElementPtr eright = elem(right);

	if(eleft->type==Element::TYPE_CONCAT){
		return xnew<Element>(Element::TYPE_CONCAT, eleft->param1, xnew<Element>(Element::TYPE_CONCAT, eleft->param2, eright));
	}

	return xnew<Element>(Element::TYPE_CONCAT, eleft, eright); 
}

AnyPtr more_Int(const AnyPtr& left, int_t n, int_t kind){
	if(n==0){ return xnew<Element>(Element::TYPE_MORE0, elem(left), null, kind); }
	else if(n==1){ return xnew<Element>(Element::TYPE_MORE1, elem(left), null, kind); }
	else if(n==-1){ return xnew<Element>(Element::TYPE_01, elem(left), null, kind); }

	if(n>0){ return concat(left, more_Int(left, n-1, kind)); }
	else{ return concat(more_Int(left, -1, kind), more_Int(left, n+1, kind)); }
}

AnyPtr more_IntRange(const AnyPtr& left, const IntRangePtr& range, int_t kind){
	if(range->begin()<=0){
		int n = -(range->end()-1);
		return n < 0 ? more_Int(left, n, kind) : xnew<Element>(Element::TYPE_EMPTY);
	}

	return concat(left, more_IntRange(left, xnew<IntRange>(range->begin()-1, range->end()-1, RANGE_LEFT_CLOSED_RIGHT_OPEN), kind));
}

AnyPtr more_normal_Int(const AnyPtr& left, int_t n){ return more_Int(left, n, 0); }
AnyPtr more_shortest_Int(const AnyPtr& left, int_t n){ return more_Int(left, n, 1); }
AnyPtr more_greed_Int(const AnyPtr& left, int_t n){ return xnew<Element>(Element::TYPE_GREED, more_normal_Int(left, n)); }
AnyPtr more_normal_IntRange(const AnyPtr& left, const IntRangePtr& range){ return more_IntRange(left, range, 0); }
AnyPtr more_shortest_IntRange(const AnyPtr& left, const IntRangePtr& range){ return more_IntRange(left, range, 1); }
AnyPtr more_greed_IntRange(const AnyPtr& left, const IntRangePtr& range){ return xnew<Element>(Element::TYPE_GREED, more_normal_IntRange(left, range)); }

AnyPtr inv(const AnyPtr& left){
	ElementPtr e = elem(left);
	ElementPtr ret = xnew<Element>(e->type, e->param1, e->param2, e->param3);
	ret->inv = !e->inv;
	return ret;
}

AnyPtr lookahead(const AnyPtr& left){ return xnew<Element>(Element::TYPE_LOOKAHEAD, elem(left)); }
AnyPtr lookbehind(const AnyPtr& left, int_t back){ return xnew<Element>(Element::TYPE_LOOKBEHIND, elem(left), null, back); }

AnyPtr cap(const IDPtr& name, const AnyPtr& left){ return xnew<Element>(Element::TYPE_CAP, elem(left), name, 1); }

void cap_vm(const VMachinePtr& vm){
	if(vm->named_arg_count()==1 && vm->ordered_arg_count()==0){ 
		vm->return_result(cap(vm->arg_name(0), vm->arg(vm->arg_name(0)))); 
		return;
	}
	
	if(vm->ordered_arg_count()==2 && vm->named_arg_count()==0){ 
		vm->return_result(cap(ptr_cast<ID>(vm->arg(0)), vm->arg(1)));
		return;
	}

	XTAL_SET_EXCEPT(cpp_class<ArgumentError>()->call(Xt("XRE1027")));
}

AnyPtr node(const AnyPtr& left){ return xnew<Element>(Element::TYPE_NODE, elem(left)); }
AnyPtr node(const IDPtr& name, const AnyPtr& left){ return xnew<Element>(Element::TYPE_NODE, elem(left), name); }

void node_vm(const VMachinePtr& vm){
	if(vm->named_arg_count()!=0){ vm->return_result(node(vm->arg_name(0), vm->arg(vm->arg_name(0)))); }
	else{ 
		if(vm->ordered_arg_count()==2){ vm->return_result(node(ptr_cast<ID>(vm->arg(0)), vm->arg(1))); }
		else{ vm->return_result(node(vm->arg(0))); }
	}
}

AnyPtr splice_node(int_t num, const AnyPtr& left){ return xnew<Element>(Element::TYPE_NODE, elem(left), null, num); }
AnyPtr splice_node(int_t num, const IDPtr& name, const AnyPtr& left){ return xnew<Element>(Element::TYPE_NODE, elem(left), name, num); }

void splice_node_vm(const VMachinePtr& vm){
	if(vm->named_arg_count()!=0){ vm->return_result(splice_node(vm->arg(0)->to_i(), vm->arg_name(0), vm->arg(vm->arg_name(0)))); }
	else{ 
		if(vm->ordered_arg_count()==3){ vm->return_result(splice_node(vm->arg(0)->to_i(), ptr_cast<ID>(vm->arg(1)), vm->arg(2))); }
		else{ vm->return_result(splice_node(vm->arg(0)->to_i(), vm->arg(0))); }
	}
}

AnyPtr leaf(const AnyPtr& left){ return xnew<Element>(Element::TYPE_LEAF, elem(left)); }
AnyPtr leafs(const AnyPtr& left){ return xnew<Element>(Element::TYPE_LEAF, elem(left), null, 1); }
AnyPtr back_ref(const AnyPtr& n){ return xnew<Element>(Element::TYPE_BACKREF, n); }

AnyPtr decl(){ return xnew<Element>(Element::TYPE_DECL); }

void set_body(const ElementPtr& x, const AnyPtr& term){ 
	if(x->type==Element::TYPE_DECL){
		x->type = Element::TYPE_GREED;
		x->param1 = elem(term);
	}
}

AnyPtr bound(const AnyPtr& body, const AnyPtr& sep){ return concat(concat(lookbehind(sep, 1), body), lookahead(sep)); }
AnyPtr error(const AnyPtr& fn){ return xnew<Element>(Element::TYPE_ERROR, fn); }
AnyPtr pred(const AnyPtr& e){ return xnew<Element>(Element::TYPE_PRED, e); }
	
}

class XpegOperator;

XTAL_PREBIND(XpegOperator){
	it->unset_native();
}

XTAL_BIND(XpegOperator){
	using namespace xpeg;

	it->def_method(Xid(op_div), &more_shortest_Int, cpp_class<Int>());
	it->def_method(Xid(op_div), &more_shortest_IntRange, cpp_class<IntRange>());
	it->def_method(Xid(op_mod), &more_normal_Int, cpp_class<Int>());
	it->def_method(Xid(op_mod), &more_normal_IntRange, cpp_class<IntRange>());
	it->def_method(Xid(op_mul), &more_greed_Int, cpp_class<Int>());
	it->def_method(Xid(op_mul), &more_greed_IntRange, cpp_class<IntRange>());
	it->def_method(Xid(op_com), &inv);
	
	it->def_method(Xid(op_or), &select, cpp_class<Element>());
	it->def_method(Xid(op_or), &select, cpp_class<String>());
	it->def_method(Xid(op_or), &select, cpp_class<ChRange>());
	it->def_method(Xid(op_or), &select, cpp_class<Fun>());
	it->def_method(Xid(op_shr), &concat, cpp_class<Element>());
	it->def_method(Xid(op_shr), &concat, cpp_class<String>());
	it->def_method(Xid(op_shr), &concat, cpp_class<ChRange>());
	it->def_method(Xid(op_shr), &concat, cpp_class<Fun>());
}

XTAL_BIND(xpeg::Element){
	using namespace xpeg;
	it->def_method(Xid(set_body), &set_body);
}

XTAL_PREBIND(xpeg::TreeNode){
	using namespace xpeg;
	it->set_final();
	it->inherit(cpp_class<Array>());
}

XTAL_BIND(xpeg::TreeNode){
	using namespace xpeg;
	it->def_method(Xid(tag), &TreeNode::tag);
	it->def_method(Xid(lineno), &TreeNode::lineno);
	it->def_method(Xid(set_tag), &TreeNode::set_tag);
	it->def_method(Xid(set_lineno), &TreeNode::set_lineno);
}

XTAL_PREBIND(xpeg::Executor){
	using namespace xpeg;
	it->def_ctor(ctor<Executor, const AnyPtr&>()->param(1, Xid(stream_or_iterator), empty_string));
}

XTAL_BIND(xpeg::Executor){
	using namespace xpeg;
	it->def_method("reset", &Executor::reset);
	it->def_method("parse", &Executor::parse);
	it->def_method("match", &Executor::match);

	it->def_method("captures", &Executor::captures);
	it->def_method("captures_values", &Executor::captures_values);		
	it->def_method("op_at", &Executor::at, cpp_class<String>());
	it->def_method("op_call", &Executor::call, cpp_class<String>());
	it->def_method("prefix", &Executor::prefix);
	it->def_method("suffix", &Executor::suffix);
	it->def_method("prefix_values", &Executor::prefix_values);
	it->def_method("suffix_values", &Executor::suffix_values);
	it->def_method("errors", &Executor::errors);
	it->def_method("read", &Executor::read);
	it->def_method("peek", &Executor::peek)->param(1, "n", 0);
	it->def_method("tree", &Executor::tree);
	it->def_method("bos", &Executor::bos);
	it->def_method("eos", &Executor::eos);
}

class Xpeg;

XTAL_BIND(Xpeg){
	using namespace xpeg;
	it->unset_native();

	AnyPtr any = xnew<Element>(Element::TYPE_ANY);
	AnyPtr bos = xnew<Element>(Element::TYPE_BOS);
	AnyPtr eos = xnew<Element>(Element::TYPE_EOS);
	AnyPtr bol = xnew<Element>(Element::TYPE_BOL);
	AnyPtr eol = xnew<Element>(Element::TYPE_EOL);
	AnyPtr empty = xnew<Element>(Element::TYPE_EMPTY);
	
	AnyPtr degit = elem(xnew<ChRange>("0", "9"));
	AnyPtr lalpha = elem(xnew<ChRange>("a", "z"));
	AnyPtr ualpha = elem(xnew<ChRange>("A", "Z"));
	AnyPtr alpha = select(lalpha, ualpha);
	AnyPtr word = select(select(alpha, degit), "_");
	AnyPtr ascii = elem(xnew<ChRange>(xnew<ID>((char_t)1), xnew<ID>((char_t)127)));

	it->def("any", any);
	it->def("bos", bos);
	it->def("eos", eos);
	it->def("bol", bol);
	it->def("eol", eol);
	it->def("empty", empty);
	it->def("degit", degit);
	it->def("lalpha", lalpha);
	it->def("ualpha", ualpha);
	it->def("alpha", alpha);
	it->def("word", word);
	it->def("ascii", ascii);
	
	it->def_fun("set", &set);
	it->def_fun("back_ref", &back_ref);
	it->def_fun("lookahead", &lookahead);
	it->def_fun("lookbehind", &lookbehind);
	it->def_fun("leaf", &leaf);
	it->def_fun("leafs", &leafs);
	it->def_fun("node", &node_vm);
	it->def_fun("splice_node", &splice_node_vm);
	it->def_fun("cap", &cap_vm);
	it->def_fun("bound", &bound);
	it->def_fun("pred", &pred);
	it->def_fun("error", &error);

	it->def_fun("decl", &decl);

	it->def("Executor", cpp_class<Executor>());
}

void initialize_xpeg(){
	using namespace xpeg;

	ClassPtr classes[4] = {cpp_class<Element>(), cpp_class<ChRange>(), cpp_class<String>(), cpp_class<Fun>()};
	for(int i=0; i<4; ++i){
		classes[i]->inherit(cpp_class<XpegOperator>());
	}

	builtin()->def("xpeg", cpp_class<Xpeg>());
}


}
