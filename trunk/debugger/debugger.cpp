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

void Debugger::addEvalExpr(const QString& expr){
	ExprValue& ev = exprs_[expr];
	ev.count++;
	if(ev.count==1){
		ev.code = eval_compile(qstr2xstr(expr));
		if(isConnected()){
			sendAddEvalExpr(expr);
		}
	}
}

void Debugger::removeEvalExpr(const QString& expr){
	ExprValue& ev = exprs_[expr];
	ev.count--;
	if(ev.count==0){
		ev.code = null;
		ev.result = null;
		if(isConnected()){
			sendRemoveEvalExpr(expr);
		}
	}
}

ArrayPtr Debugger::evalExprResult(const QString& expr){
	return exprs_[expr].result;
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

void Debugger::sendAddBreakpoint(const QString& path, int n){
	ArrayPtr a = xnew<Array>();
	a->push_back(Xid(add_breakpoint));
	a->push_back(path.toStdString().c_str());
	a->push_back(n);
	a->push_back(null);
	stream_->serialize(a);
}

void Debugger::sendRemoveBreakpoint(const QString& path, int n){
	ArrayPtr a = xnew<Array>();
	a->push_back(Xid(remove_breakpoint));
	a->push_back(path.toStdString().c_str());
	a->push_back(n);
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

void Debugger::sendStart(){
	ArrayPtr a = xnew<Array>();
	a->push_back(Xid(start));
	stream_->serialize(a);
}

void Debugger::sendRequiredSource(const CodePtr& code){
	ArrayPtr a = xnew<Array>();
	a->push_back(Xid(required_source));
	a->push_back(code);
	stream_->serialize(a);
}

void Debugger::sendAddEvalExpr(const QString& expr){
	ArrayPtr a = xnew<Array>();
	a->push_back(Xid(add_eval_expr));
	a->push_back(qstr2xid(expr));
	a->push_back(exprs_[expr].code);
	XTAL_CATCH_EXCEPT(e){}
	stream_->serialize(a);
}

void Debugger::sendRemoveEvalExpr(const QString& expr){
	ArrayPtr a = xnew<Array>();
	a->push_back(Xid(remove_eval_expr));
	a->push_back(qstr2xid(expr));
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
			MapPtr exprs = ptr_cast<Map>(command->at(1));
			ArrayPtr callStack = ptr_cast<Array>(command->at(2));

			Xfor2(key, value, exprs){
				ExprValue& ev = exprs_[xstr2qstr(key->to_s())];
				ev.result = ptr_cast<Array>(value);
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

	QMap<QString, ExprValue>::const_iterator it = exprs_.constBegin();
	for(; it!=exprs_.constEnd(); ++it){
		sendAddEvalExpr(it.key());
	}

	sendCommand(Xid(start));

	emit connected();
	onRecv();
}


