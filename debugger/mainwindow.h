#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui>
#include <QPlainTextEdit>
#include <QObject>
#include <QMenu>
#include <QPair>
#include <QVector>

#include <vector>
#include <map>

#include "codeeditor.h"
#include "evalexprview.h"
#include "callstackview.h"
#include "projectview.h"
#include "breakpointview.h"
#include "codeeditor.h"
#include "debugger.h"
#include "document.h"

Q_DECLARE_METATYPE(AnyPtr);

QT_BEGIN_NAMESPACE
class QDragEnterEvent;
class QDropEvent;
QT_END_NAMESPACE

class LineNumberArea;

class AlertDialog : public QDialog{
	Q_OBJECT
public:

	AlertDialog(const QString& mes, QWidget* parent = 0)
		:QDialog(parent){
		QHBoxLayout* mainLayout = new QHBoxLayout();
		setLayout(mainLayout);

		QTextEdit* edit = new QTextEdit();
		QPushButton* cancel = new QPushButton(tr("cancel"));

		mainLayout->addWidget(edit);
		mainLayout->addWidget(cancel);
		connect(cancel, SIGNAL(clicked()), this, SLOT(close()));

		edit->setText(mes);
	}
};

inline void alert(const QString& mes){
	AlertDialog* alert = new AlertDialog(mes);
	alert->exec();
}

inline void alert(int i){
	AlertDialog* alert = new AlertDialog(QString("%1").arg(i));
	alert->exec();
}

QString makeRelative(const QString& source, const QString& from);

class OptionDialog : public QDialog{
	Q_OBJECT
public:

	OptionDialog(QWidget* parent = 0)
		:QDialog(parent){
		QHBoxLayout* mainLayout = new QHBoxLayout();
		setLayout(mainLayout);

		QLabel* label = new QLabel(tr("ip address"));
		QLineEdit* lineEdit = new QLineEdit();
		label->setBuddy(lineEdit);

		QPushButton* ok = new QPushButton(tr("ok"));
		QPushButton* cancel = new QPushButton(tr("cancel"));

		mainLayout->addWidget(label);
		mainLayout->addWidget(lineEdit);
		mainLayout->addWidget(ok);
		mainLayout->addWidget(cancel);

		ok->setDefault(true);
		connect(cancel, SIGNAL(clicked()), this, SLOT(close()));
	}
};

class MainWindow : public QMainWindow{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = 0);

	~MainWindow();

public:

	void createActions();

	void view(const VMachinePtr& vm){
		callstack_->view(vm);
	}

protected:
	void closeEvent(QCloseEvent *event);

	void setGuiEnabled(bool b);

	void setActionsEnabled(bool b);

	void setStepActionsEnabled(bool b);

	void updateExprView();

	void updateCallStackView();

public slots:

	void initProject();

	void newProject();

	void saveProject();

	void loadProject();

	void addFile(const QString& filename);
	void removeFile(const QString& filename);

	void addFile();

	void viewOption();

	void viewSource(const QString& file);

	void exprChanged(int i, const QString& expr);

	void breaked();

	void required();

	void connected();

	void disconnected();

	void run();

	void stepOver();

	void stepInto();

	void stepOut();

	void onUpdate();

	void onBreakpointChanged(const QString& path, int line, bool b);
	void onBreakpointConditionChanged(const QString& path, int line, const QString& cond);
	void onViewBreakpoint(const QString& path, int line);
	void eraseBreakpoint(const QString& path, int line);

	void print(const QString& mes);

	void showProjectDock(){ projDockWidget_->show(); }
	void showEvalExprDock(){ exprDockWidget_->show(); }
	void showCSDock(){ csDockWidget_->show(); }
	void showBreakpointDock(){ breakpointDockWidget_->show(); }
	void showMessageDock(){ mesDockWidget_->show(); }

	void moveCallStack(int n);

	void showAboutQt(){
		QMessageBox::aboutQt(this, "Xtal Debugger");
	}

private:
	EvalExprView* evalexpr_;
	CallStackView* callstack_;
	CodeEditor* codeEditor_;
	ProjectView* project_;
	BreakpointView* breakpoint_;
	QTextEdit* messages_;
	Debugger debugger_;

private:
	QSet<QString> requiredFiles_;
	Document document_;
	QString projectFilename_;


private:
	QDockWidget* projDockWidget_;
	QDockWidget* exprDockWidget_;
	QDockWidget* csDockWidget_;
	QDockWidget* mesDockWidget_;
	QDockWidget* breakpointDockWidget_;

private:
	QToolBar* toolBar_;
	QAction* runAction_;
	QAction* stepIntoAction_;
	QAction* stepOverAction_;
	QAction* stepOutAction_;
	QAction* updateAction_;

	QMenu* fileMenu_;

private:
	int stoppedLine_;

	enum{
		STATE_NONE,
		STATE_REQUIRING,
		STATE_BREAKING
	};

	int state_;
};


#endif // MAINWINDOW_H
