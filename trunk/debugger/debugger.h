#ifndef DEBUGGER_H
#define DEBUGGER_H

#include <QTcpSocket>
#include <QTcpServer>

#include "xtal_src.h"

class TCPStream : public Stream{
public:

	TCPStream(QTcpSocket* socket = 0){
		socket_ = socket;
	}

	virtual ~TCPStream(){
		close();
	}

	bool isOpen(){
		return socket_!=0;
	}

	virtual void close(){
		if(isOpen()){
			socket_->close();
			socket_ = 0;
		}
	}

	virtual uint_t read(void* dest, uint_t size){
		uint_t read = 0;

		while(true){
			int temp = socket_->read((char*)dest+read, size-read);
			if(temp<0){
				close();
				return read;
			}
			read += temp;
			if(read==size){
				break;
			}

			socket_->waitForReadyRead();
		}

		return read;
	}

	virtual uint_t write(const void* src, uint_t size){
		int temp = socket_->write((char*)src, size);
		if(temp<0){
			close();
			return 0;
		}
		return temp;
	}

	QTcpSocket* rawsocket(){
		return socket_;
	}

private:
	QTcpSocket* socket_;
};

class Debugger : public QObject{
	Q_OBJECT
public:

	Debugger();

	void setSource(const QString& source);

public:

	void addEvalExpr(const QString& expr);

	void removeEvalExpr(const QString& expr);

	ArrayPtr evalExprResult(const QString& expr);

public:

	int callStackSize();

	StringPtr callStackFunName(int n);
	StringPtr callStackFunName();

	StringPtr callStackFileName(int n);
	StringPtr callStackFileName();

	int callStackLineno(int n);
	int callStackLineno();

public:

	QString requiredFile();

	void sendRequiredSource(const CodePtr& code);

	int level(){ return level_;	}

public:

	void addBreakpoint();
	void removeBreakpoint();
	void clearBreakpoint();

	void run();
	void stepOver();
	void stepInto();
	void stepOut();
	void redo();

	void sendAddBreakpoint(const QString& path, int n);
	void sendRemoveBreakpoint(const QString& path, int n);
	void sendMoveCallStack(int n);
	void sendNostep();
	void sendStart();

public:

	bool isConnected();

signals:

	// リモートデバック先と接続された際のシグナル
	void connected();

	// リモートデバック先と接続された際のシグナル
	void disconnected();

	// ブレークした際のシグナル
	void breaked();

	// ブレークした際のシグナル
	void required();

	// コンパイルエラーが起こった際のシグナル
	void compileError();

	// キャッチされない例外が起こった際のシグナル
	void uncatchedException();

protected:

	void sendCommand(const IDPtr& id);
	void sendAddEvalExpr(const QString& expr);
	void sendRemoveEvalExpr(const QString& expr);

protected slots:

	void onRecv();
	void onClosed();
	void onConnected();

private:

	int socket_;
	SmartPtr<TCPStream> stream_;
	QTcpServer server_;
	QString source_;

	struct CallInfo{
		StringPtr funName;
		StringPtr fileName;
		int lineno;
	};

	QVector<CallInfo> callStack_;

	struct ExprValue{
		ExprValue(){
			count = 0;
		}

		int count;
		CodePtr code;
		ArrayPtr result;
	};

	QMap<QString, ExprValue> exprs_;
	QString requiredFile_;
	int level_;

	IDPtr prevCommand_;

	int state_;

	enum{
		STATE_NONE,
		STATE_CONNECTED,
	};
};

#endif // DEBUGGER_H
