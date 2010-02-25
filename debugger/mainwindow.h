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
		QHBoxLayout* main_layout = new QHBoxLayout();
		setLayout(main_layout);

		QTextEdit* edit = new QTextEdit();
		QPushButton* cancel = new QPushButton(tr("cancel"));

		main_layout->addWidget(edit);
		main_layout->addWidget(cancel);
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
		QHBoxLayout* main_layout = new QHBoxLayout();
		setLayout(main_layout);

		QLabel* label = new QLabel(tr("ip address"));
		QLineEdit* line_edit = new QLineEdit();
		label->setBuddy(line_edit);

		QPushButton* ok = new QPushButton(tr("ok"));
		QPushButton* cancel = new QPushButton(tr("cancel"));

		main_layout->addWidget(label);
		main_layout->addWidget(line_edit);
		main_layout->addWidget(ok);
		main_layout->addWidget(cancel);

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

	void create_actions();

	void view(const VMachinePtr& vm){
		callstack_->view(vm);
	}

protected:

	void set_gui_enabled(bool b);

	void set_actions_enabled(bool b);

	void set_step_actions_enabled(bool b);

	void update_expr_view();

	void update_call_stack_view();

public slots:

	void new_project();

	void save_project();

	void load_project();

	void add_file(const QString& filename);
	
	void add_file();

	void view_option();

	void view_source(const QString& file);

	void expr_changed(int i, const QString& expr);

	void breaked();

	void required();

	void connected();

	void disconnected();

	void run();

	void step_over();

	void step_into();

	void step_out();

	void on_update();

	void on_breakpoint_changed(const QString& path, int n, bool b);

	void print(const QString& mes);

private:
	EvalExprView* evalexpr_;
	CallStackView* callstack_;
	CodeEditor* code_editor_;
	ProjectView* project_;
	QTextEdit* messages_;
	Debugger debugger_;

	QStringList required_files_;
	Document document_;
	QString project_filename_;

private:
	QToolBar* tool_bar_;
	QAction* run_action_;
	QAction* step_into_action_;
	QAction* step_over_action_;
	QAction* step_out_action_;
	QAction* update_action_;

	QMenu* file_menu_;

private:
	int stopped_line_;

	enum{
		STATE_NONE,
		STATE_REQUIRING,
	};

	int state_;
};


#endif // MAINWINDOW_H
