#include "debugger.h"

Debugger::Debugger(){
	connect(&server_, SIGNAL(newConnection()), SLOT(onConnected()));
	server_.listen(QHostAddress::Any, 13245);
	state_ = STATE_NONE;
	level_ = 0;

	prevCommand_ = "run";
}

void Debugger::setSource(const QString& source){
	source_ = source;
}

void Debugger::setEvalExpr(int n, const StringPtr& expr){
	if(exprs_.size()<=n){
		exprs_.resize(n+1);
	}

	exprs_[n].expr = expr;
	exprs_[n].result = null;

	if(isConnected()){
		sendEvalExpr(n, expr);
	}
}

StringPtr Debugger::evalExpr(int n){
	if(exprs_.size()<=n){
		exprs_.resize(n+1);
	}

	return exprs_[n].expr;
}

ArrayPtr Debugger::evalResult(int n){
	if(exprs_.size()<=n){
		exprs_.resize(n+1);
	}

	return exprs_[n].result;
}

int Debugger::evalExprSize(){
	return exprs_.size();
}

int Debugger::callStackSize(){
	return callStack_.size();
}

StringPtr Debugger::callStackFunName(int n){
	return callStack_[n].funName;
}

StringPtr Debugger::callStackFileName(int n){
	return callStack_[n].fileName;
}

int Debugger::callStackLineno(int n){
	return callStack_[n].lineno;
}

StringPtr Debugger::callStackFunName(){
	return callStack_[level_].funName;
}

StringPtr Debugger::callStackFileName(){
	return callStack_[level_].fileName;
}

int Debugger::callStackLineno(){
	return callStack_[level_].lineno;
}

QString Debugger::requiredFile(){
	return requiredFile_;
}

void Debugger::run(){
	sendCommand("run");
}

void Debugger::stepOver(){
	sendCommand("step_over");
}

void Debugger::stepInto(){
	sendCommand("step_into");
}

void Debugger::stepOut(){
	sendCommand("step_out");
}

void Debugger::redo(){
	sendCommand(prevCommand_);
}

void Debugger::sendBreakpoint(const QString& path, int n, bool b){
	ArrayPtr a = xnew<Array>();
	a->push_back(Xid(breakpoint));
	a->push_back(path.toStdString().c_str());
	a->push_back(n);
	a->push_back(b);
	stream_->serialize(a);
}

void Debugger::sendMoveCallStack(int n){
	ArrayPtr a = xnew<Array>();
	a->push_back(Xid(move_callstack));
	a->push_back(n);
	stream_->serialize(a);
}

void Debugger::sendNostep(){
	ArrayPtr a = xnew<Array>();
	a->push_back(Xid(nostep));
	stream_->serialize(a);
}

void Debugger::sendRequiredSource(const CodePtr& code){
	ArrayPtr a = xnew<Array>();
	a->push_back(Xid(required_source));
	a->push_back(code);
	stream_->serialize(a);
}

void Debugger::sendEvalExpr(int n, const StringPtr& expr){
	ArrayPtr a = xnew<Array>();
	a->push_back(Xid(eval_expr));
	a->push_back(n);
	a->push_back(expr);
	stream_->serialize(a);
}

void Debugger::sendCommand(const IDPtr& id){
	prevCommand_ = id;
	ArrayPtr a = xnew<Array>();
	a->push_back(id);
	stream_->serialize(a);
}

bool Debugger::isConnected(){
	return state_!=STATE_NONE;
}

void Debugger::onRecv(){
	while(stream_->rawsocket()->bytesAvailable()!=0){
		ArrayPtr command = ptr_cast<Array>(stream_->deserialize());
		AnyPtr type = command->at(0);

		if(raweq(type, Xid(break))){
			ArrayPtr exprs = ptr_cast<Array>(command->at(1));
			ArrayPtr callStack = ptr_cast<Array>(command->at(2));

			for(uint_t i=0; i<exprs->size(); ++i){
				exprs_[i].result = ptr_cast<Array>(exprs->at(i));
			}

			callStack_.resize(callStack->size());
			for(uint_t i=0; i<callStack->size(); ++i){
				ArrayPtr record = ptr_cast<Array>(callStack->at(i));
				callStack_[i].funName = record->at(0)->to_s();
				callStack_[i].fileName = record->at(1)->to_s();
				callStack_[i].lineno = record->at(2)->to_i();
			}

			level_ = command->at(3).to_i();

			emit breaked();
			continue;
		}

		if(raweq(type, Xid(require))){
			requiredFile_ = command->at(1)->to_s()->c_str();
			emit required();
			continue;
		}
	}
}

void Debugger::onClosed(){
	state_ = STATE_NONE;
	emit disconnected();
}

void Debugger::onConnected(){
	stream_ = xnew<TCPStream>(server_.nextPendingConnection());
	connect(stream_->rawsocket(), SIGNAL(readyRead()), SLOT(onRecv()));
	connect(stream_->rawsocket(), SIGNAL(readChannelFinished()), SLOT(onClosed()));
	state_ = STATE_CONNECTED;

	for(int i=0; i<exprs_.size(); ++i){
		sendEvalExpr(i, exprs_[i].expr);
	}

	sendCommand(Xid(start));

	emit connected();
	onRecv();
}


