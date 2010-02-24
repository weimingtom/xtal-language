#ifndef DEBUGGER_H
#define DEBUGGER_H

#include <QTcpSocket>
#include <QTcpServer>

#include "../src/xtal/xtal.h"
#include "../src/xtal/xtal_macro.h"
using namespace xtal;

class TCPStream : public Stream{
public:

	TCPStream(QTcpSocket* socket = 0){
		socket_ = socket;
	}

	virtual ~TCPStream(){
		close();
	}

	bool is_open(){
		return socket_!=0;
	}

	virtual void close(){
		if(is_open()){
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

	void set_source(const QString& source);

public:

	void set_eval_expr(int n, const StringPtr& expr);

	ArrayPtr eval_result(int n);

	int eval_expr_size();

public:

	int call_stack_size();

	StringPtr call_stack_fun_name(int n);

	StringPtr call_stack_file_name(int n);

	int call_stack_lineno(int n);

public:

	QString required_file();

	void send_required_source(const CodePtr& code);

public:

	void add_breakpoint();
	void remove_breakpoint();
	void clear_breakpoint();

	void run();
	void step_over();
	void step_into();
	void step_out();

	void send_breakpoint(const QString& path, int n, bool b);

public:

	bool is_connected();

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
	void compile_error();

	// キャッチされない例外が起こった際のシグナル
	void uncatched_exception();

protected:

	void send_command(const IDPtr& id);

protected slots:

	void on_recv();
	void on_closed();
	void on_connected();

private:

	int socket_;
	SmartPtr<TCPStream> stream_;
	QTcpServer server_;
	QString source_;

	struct CallInfo{
		StringPtr fun_name;
		StringPtr file_name;
		int lineno;
	};

	QVector<CallInfo> call_stack_;

	struct ExprValue{
		StringPtr expr;
		ArrayPtr result;
	};

	QVector<ExprValue> exprs_;

	QString required_file_;

	int state_;

	enum{
		STATE_NONE,
		STATE_CONNECTED,
	};
};

#endif // DEBUGGER_H
