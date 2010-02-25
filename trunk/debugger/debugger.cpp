#include "debugger.h"

Debugger::Debugger(){
	connect(&server_, SIGNAL(newConnection()), SLOT(on_connected()));
	server_.listen(QHostAddress::Any, 13245);
	state_ = STATE_NONE;

	prev_command_ = "run";
}

void Debugger::set_source(const QString& source){
	source_ = source;
}

void Debugger::set_eval_expr(int n, const StringPtr& expr){
	if(exprs_.size()<=n){
		exprs_.resize(n+1);
	}

	exprs_[n].expr = expr;
	exprs_[n].result = null;

	if(state_==STATE_CONNECTED){
		send_command("return");
	}
}

ArrayPtr Debugger::eval_result(int n){
	if(exprs_.size()<=n){
		exprs_.resize(n+1);
	}

	return exprs_[n].result;
}

int Debugger::eval_expr_size(){
	return exprs_.size();
}

int Debugger::call_stack_size(){
	return call_stack_.size();
}

StringPtr Debugger::call_stack_fun_name(int n){
	return call_stack_[n].fun_name;
}

StringPtr Debugger::call_stack_file_name(int n){
	return call_stack_[n].file_name;
}

int Debugger::call_stack_lineno(int n){
	return call_stack_[n].lineno;
}

QString Debugger::required_file(){
	return required_file_;
}

void Debugger::send_required_source(const CodePtr& code){
	ArrayPtr a = xnew<Array>();
	a->push_back(Xid(required_source));
	a->push_back(code);
	stream_->serialize(a);
}

void Debugger::run(){
	send_command("run");
}

void Debugger::step_over(){
	send_command("step_over");
}

void Debugger::step_into(){
	send_command("step_into");
}

void Debugger::step_out(){
	send_command("step_out");
}

void Debugger::redo(){
	send_command(prev_command_);
}

void Debugger::send_breakpoint(const QString& path, int n, bool b){
	ArrayPtr a = xnew<Array>();
	a->push_back(Xid(breakpoint));
	a->push_back(path.toStdString().c_str());
	a->push_back(n);
	a->push_back(b);
	stream_->serialize(a);
}

bool Debugger::is_connected(){
	return state_!=STATE_NONE;
}

void Debugger::send_command(const IDPtr& id){
	prev_command_ = id;

	ArrayPtr a = xnew<Array>();
	a->push_back(id);

	ArrayPtr exprs = xnew<Array>(exprs_.size());
	for(int i=0; i<exprs_.size(); ++i){
		exprs->set_at(i, exprs_[i].expr);
	}
	a->push_back(exprs);

	stream_->serialize(a);
}

void Debugger::on_recv(){
	if(stream_->rawsocket()->bytesAvailable()==0){
		return;
	}

	ArrayPtr command = ptr_cast<Array>(stream_->deserialize());
	AnyPtr type = command->at(0);

	if(raweq(type, Xid(break))){
		ArrayPtr exprs = ptr_cast<Array>(command->at(1));
		ArrayPtr call_stack = ptr_cast<Array>(command->at(2));

		for(uint_t i=0; i<exprs->size(); ++i){
			exprs_[i].result = ptr_cast<Array>(exprs->at(i));
		}

		call_stack_.resize(call_stack->size());
		for(uint_t i=0; i<call_stack->size(); ++i){
			ArrayPtr record = ptr_cast<Array>(call_stack->at(i));
			call_stack_[i].fun_name = record->at(0)->to_s();
			call_stack_[i].file_name = record->at(1)->to_s();
			call_stack_[i].lineno = record->at(2)->to_i();
		}

		emit breaked();
	}
	else if(raweq(type, Xid(require))){
		required_file_ = command->at(1)->to_s()->c_str();
		emit required();
	}
}

void Debugger::on_closed(){
	state_ = STATE_NONE;
	emit disconnected();
}

void Debugger::on_connected(){
	stream_ = xnew<TCPStream>(server_.nextPendingConnection());
	connect(stream_->rawsocket(), SIGNAL(readyRead()), SLOT(on_recv()));
	connect(stream_->rawsocket(), SIGNAL(readChannelFinished()), SLOT(on_closed()));
	state_ = STATE_CONNECTED;
	emit connected();
	on_recv();
}


