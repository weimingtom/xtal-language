#include "mainwindow.h"
#include "ui_mainwindow.h"

inline QString makeRelative(const QString& source, const QString& from){
	if(source.isEmpty()){
		return QString::null;
	}

	QFileInfo fileSource(source);
	QFileInfo fileFrom((from.isEmpty() ? "." : from));
	fileSource.makeAbsolute();
	fileFrom.makeAbsolute();

	if(!fileSource.exists()){
		return source;
	}

	QString sSource = fileSource.absoluteFilePath();
	QString sFrom = fileFrom.absoluteFilePath();
	if(fileFrom.isFile()){
		sFrom = fileFrom.absolutePath();
	}

	if(sSource.left(sFrom.length()) == sFrom){
		sSource = sSource.remove(0, sFrom.length()+1);
		return sSource;
	}

#if defined(Q_OS_WIN32)
	if(sSource.left(3) != sFrom.left(3))
		return source;
	sSource = sSource.remove(0, 3);
	sFrom = sFrom.remove(0, 3);
#else
	sSource = sSource.remove(0, 1);
	sFrom = sFrom.remove(0, 1);
#endif

	QString tmp;
	int last = sFrom.length();
	for(int i = sFrom.length() -1; i > -1; i--){
		if(sFrom[i] == '/'){
			tmp += "../";
			last = i +1;
		}

		if(sSource.indexOf(sFrom.left( last ) ) != -1){
			sSource = tmp.append(sSource.mid(i +1));
			return sSource;
			break;
		}
	}

	for(int i = 0; i < sFrom.count( "/" ); i++){
		tmp += "../";
	}

	sSource = tmp.append(sSource);
	return sSource;
}

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent){
	code_editor_ = new CodeEditor();
	variables_ = new EvalExprView();
	callstack_ = new CallStackView();
	messages_ = new QTextEdit();
	project_ = new ProjectView(this);

	setCentralWidget(code_editor_);

	QDockWidget* proj_dock_widget = new QDockWidget(tr("Project"), this);
	proj_dock_widget->setWidget(project_);
	proj_dock_widget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	addDockWidget(Qt::LeftDockWidgetArea, proj_dock_widget);

	QDockWidget* expr_dock_widget = new QDockWidget(tr("Expr"), this);
	expr_dock_widget->setWidget(variables_);
	expr_dock_widget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	addDockWidget(Qt::RightDockWidgetArea, expr_dock_widget);

	QDockWidget* cs_dock_widget = new QDockWidget(tr("Call Stack"), this);
	cs_dock_widget->setWidget(callstack_);
	cs_dock_widget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	addDockWidget(Qt::RightDockWidgetArea, cs_dock_widget);

	QDockWidget* m_dock_widget = new QDockWidget(tr("Message"), this);
	m_dock_widget->setWidget(messages_);
	m_dock_widget->setAllowedAreas(Qt::BottomDockWidgetArea);
	addDockWidget(Qt::BottomDockWidgetArea, m_dock_widget);

	create_actions();

	//variables_->add_expr(0, "filelocal");
}

MainWindow::~MainWindow(){
}

void MainWindow::create_actions(){
	file_menu_ = this->menuBar()->addMenu(tr("file"));
	QAction* new_project_action = file_menu_->addAction(tr("new project"));
	QAction* load_project_action = file_menu_->addAction(tr("load project"));
	QAction* save_project_action = file_menu_->addAction(tr("save project"));
	file_menu_->addSeparator();
	QAction* add_file_action = file_menu_->addAction(tr("add file"));

	QMenu* option_menu_ = this->menuBar()->addMenu(tr("tool"));
	QAction* option_action = option_menu_->addAction(tr("option"));

	tool_bar_ = this->addToolBar(tr("debug"));
	run_action_ = tool_bar_->addAction(QIcon("data/run.png"), tr("run"));
	step_into_action_ = tool_bar_->addAction(QIcon("data/step_into.png"), tr("step into"));
	step_over_action_ = tool_bar_->addAction(QIcon("data/step_over.png"), tr("step over"));
	step_out_action_ = tool_bar_->addAction(QIcon("data/step_out.png"), tr("step out"));
	update_action_ = tool_bar_->addAction(QIcon("data/update.png"), tr("update"));

	connect(new_project_action, SIGNAL(triggered()), this, SLOT(new_project()));
	connect(save_project_action, SIGNAL(triggered()), this, SLOT(save_project()));
	connect(load_project_action, SIGNAL(triggered()), this, SLOT(load_project()));
	connect(add_file_action, SIGNAL(triggered()), this, SLOT(add_file()));
	connect(option_action, SIGNAL(triggered()), this, SLOT(view_option()));

	connect(run_action_, SIGNAL(triggered()), this, SLOT(run()));
	connect(step_into_action_, SIGNAL(triggered()), this, SLOT(step_into()));
	connect(step_over_action_, SIGNAL(triggered()), this, SLOT(step_over()));
	connect(step_out_action_, SIGNAL(triggered()), this, SLOT(step_out()));
	connect(update_action_, SIGNAL(triggered()), this, SLOT(on_update()));

	connect(&debugger_, SIGNAL(breaked()), this, SLOT(breaked()));
	connect(&debugger_, SIGNAL(required()), this, SLOT(required()));
	connect(&debugger_, SIGNAL(connected()), this, SLOT(connected()));
	connect(&debugger_, SIGNAL(disconnected()), this, SLOT(disconnected()));

	connect(variables_, SIGNAL(expr_changed(int, const QString&)), this, SLOT(expr_changed(int, const QString&)));

	connect(project_, SIGNAL(on_add_file(const QString&)), this, SLOT(add_file(const QString&)));
	connect(project_, SIGNAL(on_view(const QString&)), this, SLOT(view_source(const QString&)));

	connect(code_editor_, SIGNAL(breakpoint_changed(const QString&,int,bool)), this, SLOT(on_breakpoint_changed(const QString&,int,bool)));

	set_actions_enabled(false);
	set_gui_enabled(false);
}

void MainWindow::set_gui_enabled(bool b){
	variables_->setEnabled(b);
	callstack_->setEnabled(b);
	code_editor_->setEnabled(b);
	project_->setEnabled(b);
	messages_->setEnabled(b);
}

void MainWindow::set_actions_enabled(bool b){
	run_action_->setEnabled(b);
	step_into_action_->setEnabled(b);
	step_over_action_->setEnabled(b);
	step_out_action_->setEnabled(b);
	update_action_->setEnabled(b);
}

void MainWindow::set_step_actions_enabled(bool b){
	run_action_->setEnabled(b);
	step_into_action_->setEnabled(b);
	step_over_action_->setEnabled(b);
	step_out_action_->setEnabled(b);
}

void MainWindow::update_expr_view(){
	for(int i=0; i<debugger_.eval_expr_size(); ++i){
		variables_->set_item(i, debugger_.eval_result(i));
	}
}

void MainWindow::update_call_stack_view(){
	callstack_->clear();
	for(int i=0; i<debugger_.call_stack_size(); ++i){
		callstack_->set(i,
			debugger_.call_stack_fun_name(i),
			debugger_.call_stack_file_name(i),
			debugger_.call_stack_lineno(i));
	}
}

void MainWindow::new_project(){
	project_filename_ = QFileDialog::getSaveFileName(this, tr("New Project"), QString(), tr("xtal project(*.xproj)"));
	document_.init();
	if(document_.save(project_filename_)){
		QDir::setCurrent(QFileInfo(project_filename_).dir().absolutePath());

		//code_editor_->set_source_file(filename);

		set_gui_enabled(true);
		this->setWindowTitle(project_filename_ + " - xtal debugger");
	}
}

void MainWindow::save_project(){
	document_.save(project_filename_);
	code_editor_->save_all();
}

void MainWindow::load_project(){
	project_filename_ = QFileDialog::getOpenFileName(this, tr("Load Project"), QString(), tr("xtal project(*.xproj)"));
	if(document_.load(project_filename_)){
		QDir::setCurrent(QFileInfo(project_filename_).dir().absolutePath());

		project_->init();
		for(int i=0; i<document_.file_count(); ++i){
			QString path = document_.file(i)->path;
			project_->add_file(QFileInfo(path).fileName(), path);
		}

		set_gui_enabled(true);
		this->setWindowTitle(project_filename_ + " - xtal debugger");
		this->setWindowTitle(QDir::current().absolutePath());
	}
}

void MainWindow::add_file(const QString& filename){
	QString path = makeRelative(filename, QDir::currentPath());
	if(document_.add_file(path)){
		project_->add_file(QFileInfo(path).fileName(), path);
	}
}

void MainWindow::add_file(){
	add_file(QFileDialog::getOpenFileName(this, tr("add file"), QString(), tr("xtal(*.xtal)")));
}

void MainWindow::view_option(){
	OptionDialog* dialog = new OptionDialog(this);
	dialog->exec();
}

void MainWindow::view_source(const QString& file){
	if(Document::FileInfo* fi = document_.file(document_.find_file(file))){
		code_editor_->add_page(QFileInfo(fi->path).fileName(), fi->path);
	}
}

void MainWindow::expr_changed(int i, const QString& expr){
	document_.set_eval_expr(i, expr);
	debugger_.set_eval_expr(i, expr.toStdString().c_str());
}

void MainWindow::breaked(){
	update_expr_view();
	update_call_stack_view();

	code_editor_->set_pos(debugger_.call_stack_file_name(0)->c_str(), debugger_.call_stack_lineno(0));
}

void MainWindow::required(){
	if(Document::FileInfo* f=document_.file(document_.find_file_about(debugger_.required_file()))){
		if(CodeEditorPage* p=code_editor_->widget(code_editor_->find_widget(f->path))){
			f->breakpoints.clear();
			for(int i=1; i<=p->line_count(); ++i){
				if(p->breakpoint(i)){
					f->breakpoints.insert(i);
				}
			}

			if(CodePtr code = compile(p->toPlainText().toStdString().c_str())){
				code->set_source_file_name(f->path.toStdString().c_str());
				for(QSet<int>::iterator it=f->breakpoints.begin(); it!=f->breakpoints.end(); ++it){
					code->set_breakpoint(*it);
				}
				required_files_.push_back(f->path);
				debugger_.send_required_source(code);
				return;
			}
			else{
				XTAL_CATCH_EXCEPT(e){
					print(e->to_s()->c_str());
				}
			}
		}
		else if(CodePtr code = compile_file(f->path.toStdString().c_str())){
			code->set_source_file_name(f->path.toStdString().c_str());
			for(QSet<int>::iterator it=f->breakpoints.begin(); it!=f->breakpoints.end(); ++it){
				code->set_breakpoint(*it);
			}
			required_files_.push_back(f->path);
			debugger_.send_required_source(code);
			return;
		}
		else{
			XTAL_CATCH_EXCEPT(e){
				print(e->to_s()->c_str());
			}
		}
	}
	else{
		print(QString("Not found %1").arg(debugger_.required_file()));
	}

	state_ = STATE_REQUIRING;
	set_step_actions_enabled(false);
}

void MainWindow::connected(){
	set_actions_enabled(true);
	required_files_.clear();
	state_ = STATE_NONE;
}

void MainWindow::disconnected(){
	set_actions_enabled(false);
	code_editor_->clear_cursor_line();
	required_files_.clear();
}

void MainWindow::run(){
	debugger_.run();
}

void MainWindow::step_over(){
	debugger_.step_over();
}

void MainWindow::step_into(){
	debugger_.step_into();
}

void MainWindow::step_out(){
	debugger_.step_out();
}

void MainWindow::on_update(){
	if(state_==STATE_REQUIRING){
		state_ = STATE_NONE;
		set_step_actions_enabled(true);
		required();
	}
	else{
		if(CodeEditorPage* p=code_editor_->current_page()){
			Document::FileInfo* f=document_.file(document_.find_file(p->source_path()));

			f->breakpoints.clear();
			for(int i=1; i<=p->line_count(); ++i){
				if(p->breakpoint(i)){
					f->breakpoints.insert(i);
				}
			}

			if(CodePtr code = compile(p->toPlainText().toStdString().c_str())){
				code->set_source_file_name(f->path.toStdString().c_str());
				for(QSet<int>::iterator it=f->breakpoints.begin(); it!=f->breakpoints.end(); ++it){
					code->set_breakpoint(*it);
				}
				required_files_.push_back(f->path);
				//debugger_.send_updated_source(code);
				return;
			}
			else{
				XTAL_CATCH_EXCEPT(e){
					print(e->to_s()->c_str());
				}
			}
		}
	}
}

void MainWindow::on_breakpoint_changed(const QString& path, int n, bool b){
	foreach(QString file, required_files_){
		if(file==path){
			debugger_.send_breakpoint(path, n, b);
			return;
		}
	}
}

void MainWindow::print(const QString& mes){
	messages_->append(mes);
	messages_->append("\n\n");
}
