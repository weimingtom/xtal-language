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
	codeEditor_ = new CodeEditor(this);
	evalexpr_ = new EvalExprView(this);
	callstack_ = new CallStackView(this);
	messages_ = new QTextEdit(this);
	project_ = new ProjectView(this);
	breakpoint_ = new BreakpointView(this);

	state_ = STATE_NONE;

	setCentralWidget(codeEditor_);

	projDockWidget_ = new QDockWidget(tr("Project"), this);
	projDockWidget_->setObjectName("Project");
	projDockWidget_->setWidget(project_);
	projDockWidget_->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	addDockWidget(Qt::LeftDockWidgetArea, projDockWidget_);

	exprDockWidget_ = new QDockWidget(tr("Expr"), this);
	exprDockWidget_->setObjectName("Expr");
	exprDockWidget_->setWidget(evalexpr_);
	exprDockWidget_->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	addDockWidget(Qt::RightDockWidgetArea, exprDockWidget_);

	csDockWidget_ = new QDockWidget(tr("Call Stack"), this);
	csDockWidget_->setObjectName("Call Stack");
	csDockWidget_->setWidget(callstack_);
	csDockWidget_->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	addDockWidget(Qt::RightDockWidgetArea, csDockWidget_);

	breakpointDockWidget_ = new QDockWidget(tr("Breakpoint"), this);
	breakpointDockWidget_->setObjectName("Breakpoint");
	breakpointDockWidget_->setWidget(breakpoint_);
	breakpointDockWidget_->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	addDockWidget(Qt::RightDockWidgetArea, breakpointDockWidget_);

	mesDockWidget_ = new QDockWidget(tr("Message"), this);
	mesDockWidget_->setObjectName("Message");
	mesDockWidget_->setWidget(messages_);
	mesDockWidget_->setAllowedAreas(Qt::BottomDockWidgetArea);
	addDockWidget(Qt::BottomDockWidgetArea, mesDockWidget_);

	createActions();

	//evalexpr_->addExpr(0, "filelocal");

	QSettings settings("xtal", "debugger");

	QByteArray geoData = settings.value("geometry").toByteArray();
	QByteArray layoutData = settings.value("windowState").toByteArray();

	if(geoData.size() > 1){
		restoreGeometry(geoData);
	}

	if(layoutData.size() > 1){
		restoreState(layoutData);
	}
}

MainWindow::~MainWindow(){
}

void MainWindow::closeEvent(QCloseEvent *event){
	QSettings settings("xtal", "debugger");
	settings.setValue("geometry", saveGeometry());
	settings.setValue("windowState", saveState());
	QMainWindow::closeEvent(event);
}

void MainWindow::createActions(){
	fileMenu_ = this->menuBar()->addMenu(tr("&File"));
	QAction* newProjectAction = fileMenu_->addAction(tr("&New Project"));
	QAction* loadProjectAction = fileMenu_->addAction(tr("&Load Project"));
	QAction* saveProjectAction = fileMenu_->addAction(tr("&Save Project"));
	//fileMenu_->addSeparator();
	//QAction* addFileAction = fileMenu_->addAction(tr("add file"));

	//QMenu* optionMenu_ = this->menuBar()->addMenu(tr("&Tool"));
	//QAction* optionAction = optionMenu_->addAction(tr("&Option"));

	toolBar_ = this->addToolBar(tr("debug"));
	toolBar_->setObjectName("debug");
	runAction_ = toolBar_->addAction(QIcon("data/run.png"), tr("run"));
	stepIntoAction_ = toolBar_->addAction(QIcon("data/step_into.png"), tr("step into"));
	stepOverAction_ = toolBar_->addAction(QIcon("data/step_over.png"), tr("step over"));
	stepOutAction_ = toolBar_->addAction(QIcon("data/step_out.png"), tr("step out"));
	updateAction_ = toolBar_->addAction(QIcon("data/update.png"), tr("update"));

	connect(newProjectAction, SIGNAL(triggered()), this, SLOT(newProject()));
	connect(saveProjectAction, SIGNAL(triggered()), this, SLOT(saveProject()));
	connect(loadProjectAction, SIGNAL(triggered()), this, SLOT(loadProject()));
//	connect(addFileAction, SIGNAL(triggered()), this, SLOT(addFile()));
//	connect(optionAction, SIGNAL(triggered()), this, SLOT(viewOption()));

	connect(runAction_, SIGNAL(triggered()), this, SLOT(run()));
	connect(stepIntoAction_, SIGNAL(triggered()), this, SLOT(stepInto()));
	connect(stepOverAction_, SIGNAL(triggered()), this, SLOT(stepOver()));
	connect(stepOutAction_, SIGNAL(triggered()), this, SLOT(stepOut()));
	connect(updateAction_, SIGNAL(triggered()), this, SLOT(onUpdate()));

	QMenu* windowMenu = this->menuBar()->addMenu(tr("&Window"));
	connect(windowMenu->addAction(tr("Project")), SIGNAL(triggered()), this, SLOT(showProjectDock()));
	connect(windowMenu->addAction(tr("Expr")), SIGNAL(triggered()), this, SLOT(showEvalExprDock()));
	connect(windowMenu->addAction(tr("CallStack")), SIGNAL(triggered()), this, SLOT(showCSDock()));
	connect(windowMenu->addAction(tr("Breakpoint")), SIGNAL(triggered()), this, SLOT(showBreakpointDock()));
	connect(windowMenu->addAction(tr("Message")), SIGNAL(triggered()), this, SLOT(showMessageDock()));

	QMenu* helpMenu_ = this->menuBar()->addMenu(tr("&Help"));
	QAction* aboutqtAction = helpMenu_->addAction(tr("About Qt"));
	connect(aboutqtAction, SIGNAL(triggered()), this, SLOT(showAboutQt()));

	connect(&debugger_, SIGNAL(breaked()), this, SLOT(breaked()));
	connect(&debugger_, SIGNAL(required()), this, SLOT(required()));
	connect(&debugger_, SIGNAL(connected()), this, SLOT(connected()));
	connect(&debugger_, SIGNAL(disconnected()), this, SLOT(disconnected()));

	connect(evalexpr_, SIGNAL(exprChanged(int, const QString&)), this, SLOT(exprChanged(int, const QString&)));

	connect(callstack_, SIGNAL(moveCallStack(int)), this, SLOT(moveCallStack(int)));

	connect(project_, SIGNAL(onAddFile(const QString&)), this, SLOT(addFile(const QString&)));
	connect(project_, SIGNAL(onView(const QString&)), this, SLOT(viewSource(const QString&)));
	connect(project_, SIGNAL(removeFile(const QString&)), this, SLOT(removeFile(const QString&)));

	connect(codeEditor_, SIGNAL(breakpointChanged(const QString&,int,bool)), this, SLOT(onBreakpointChanged(const QString&,int,bool)));
	connect(breakpoint_, SIGNAL(breakpointConditionChanged(const QString&,int,const QString&)), this, SLOT(onBreakpointConditionChanged(const QString&,int,const QString&)));
	connect(breakpoint_, SIGNAL(viewBreakpoint(const QString&,int)), this, SLOT(onViewBreakpoint(const QString&,int)));

	setActionsEnabled(false);
	setGuiEnabled(false);
}

void MainWindow::setGuiEnabled(bool b){
	evalexpr_->setEnabled(b);
	callstack_->setEnabled(b);
	codeEditor_->setEnabled(b);
	project_->setEnabled(b);
	breakpoint_->setEnabled(b);
	messages_->setEnabled(b);
}

void MainWindow::setActionsEnabled(bool b){
	runAction_->setEnabled(b);
	stepIntoAction_->setEnabled(b);
	stepOverAction_->setEnabled(b);
	stepOutAction_->setEnabled(b);
	updateAction_->setEnabled(b);
}

void MainWindow::setStepActionsEnabled(bool b){
	runAction_->setEnabled(b);
	stepIntoAction_->setEnabled(b);
	stepOverAction_->setEnabled(b);
	stepOutAction_->setEnabled(b);
}

void MainWindow::updateExprView(){
	for(int i=0; i<evalexpr_->exprCount(); ++i){
		evalexpr_->setExprResult(i, debugger_.evalExprResult(evalexpr_->expr(i)));
	}
}

void MainWindow::updateCallStackView(){
	callstack_->clear();
	for(int i=0; i<debugger_.callStackSize(); ++i){
		callstack_->set(i,
			debugger_.callStackFunName(i),
			debugger_.callStackFileName(i),
			debugger_.callStackLineno(i));
	}
	callstack_->setLevel(debugger_.level());
}

void MainWindow::initProject(){
	document_.init();

	evalexpr_->init();
	callstack_->init();
	codeEditor_->init();
	project_->init();
	breakpoint_->init();
	debugger_.init();

	setActionsEnabled(false);
	setGuiEnabled(false);
	state_ = STATE_NONE;
}

void MainWindow::newProject(){
	initProject();

	projectFilename_ = QFileDialog::getSaveFileName(this, tr("New Project"), QString(), tr("xtal project(*.xproj)"));
	if(document_.save(projectFilename_)){
		QDir::setCurrent(QFileInfo(projectFilename_).dir().absolutePath());
		setGuiEnabled(true);
		this->setWindowTitle(projectFilename_ + " - xtal debugger");
	}
}

void MainWindow::saveProject(){
	document_.save(projectFilename_);
	codeEditor_->saveAll();
}

void MainWindow::loadProject(){
	initProject();

	projectFilename_ = QFileDialog::getOpenFileName(this, tr("Load Project"), QString(), tr("xtal project(*.xproj)"));
	if(document_.load(projectFilename_)){
		QDir::setCurrent(QFileInfo(projectFilename_).dir().absolutePath());

		project_->init();
		for(int i=0; i<document_.fileCount(); ++i){
			FileInfo* f = document_.file(i);
			QString path = f->path;
			//project_->addFile(path);

			QMap<int, QString>::iterator it = f->breakpoints.begin();
			for(; it!=f->breakpoints.end(); ++it){
				breakpoint_->add(f->path, it.key(), it.value());
				codeEditor_->setBreakpoint(f->path, it.key());
			}
		}

		for(int i=0; i<document_.evalExprCount(); ++i){
			QString expr = document_.evalExpr(i);
			evalexpr_->setExpr(i, expr);
			debugger_.addEvalExpr(expr);
		}

		setGuiEnabled(true);
		this->setWindowTitle(projectFilename_ + " - xtal debugger");
		this->setWindowTitle(QDir::current().absolutePath());
	}
}

void MainWindow::addFile(const QString& filename){
	QString path = makeRelative(filename, QDir::currentPath());
	if(document_.addFile(path)){
	//	project_->addFile(path);
	}
}

void MainWindow::removeFile(const QString& filename){
	document_.removeFile(filename);
}

void MainWindow::addFile(){
	addFile(QFileDialog::getOpenFileName(this, tr("add file"), QString(), tr("xtal(*.xtal)")));
}

void MainWindow::viewOption(){
	OptionDialog* dialog = new OptionDialog(this);
	dialog->exec();
}

void MainWindow::viewSource(const QString& file){
	if(FileInfo* f = document_.findFile(file)){
		codeEditor_->addPage(f->path);

		QMap<int, QString>::iterator it = f->breakpoints.begin();
		for(; it!=f->breakpoints.end(); ++it){
			codeEditor_->setBreakpoint(f->path, it.key());
		}
	}
}

void MainWindow::exprChanged(int i, const QString& expr){
	document_.setEvalExpr(i, expr);

	if(debugger_.isConnected()){
		debugger_.addEvalExpr(expr);
		if(state_==STATE_BREAKING){
			debugger_.sendNostep();
		}
	}
}

void MainWindow::breaked(){
	QString path = debugger_.callStackFileName()->c_str();
	if(FileInfo* f=document_.findFile(path)){
		codeEditor_->addPage(f->path);
	}

	codeEditor_->setPos(path, debugger_.callStackLineno());
	updateExprView();
	updateCallStackView();

	state_ = STATE_BREAKING;
	setStepActionsEnabled(true);

	activateWindow();
	raise();
}

void MainWindow::required(){
	print(QString("Required %1").arg(debugger_.requiredFile()));

	if(FileInfo* f=document_.findFileAbout(debugger_.requiredFile())){
		if(CodeEditorPage* p = codeEditor_->findPage(f->path)){
			if(CodePtr code = compile(p->toPlainText().toStdString().c_str())){
				code->set_source_file_name(f->path.toStdString().c_str());
				requiredFiles_.insert(f->path);
				debugger_.sendRequiredSource(code);

				for(QMap<int, QString>::iterator it=f->breakpoints.begin(); it!=f->breakpoints.end(); ++it){
					debugger_.sendAddBreakpoint(f->path, it.key(), it.value());
				}

				debugger_.sendStart();
				return;
			}
			else{
				XTAL_CATCH_EXCEPT(e){
					print(e->to_s()->c_str());
					print("コンパイルエラーが発生しました。修正してUpdateボタンを押してください");
				}
			}
		}
		else if(CodePtr code = compile_file(f->path.toStdString().c_str())){
			code->set_source_file_name(f->path.toStdString().c_str());
			requiredFiles_.insert(f->path);
			debugger_.sendRequiredSource(code);

			for(QMap<int, QString>::iterator it=f->breakpoints.begin(); it!=f->breakpoints.end(); ++it){
				debugger_.sendAddBreakpoint(f->path, it.key(), it.value());
			}

			debugger_.sendStart();
			return;
		}
		else{
			XTAL_CATCH_EXCEPT(e){
				print(e->to_s()->c_str());
				print("コンパイルエラーが発生しました。修正してUpdateボタンを押してください");
			}
		}
	}
	else{
		debugger_.sendRequiredSource(null);
		print(QString("Not found %1").arg(debugger_.requiredFile()));
		return;
	}

	state_ = STATE_REQUIRING;
	setStepActionsEnabled(false);
}

void MainWindow::connected(){
	setActionsEnabled(true);
	setStepActionsEnabled(false);
	requiredFiles_.clear();
	state_ = STATE_NONE;
}

void MainWindow::disconnected(){
	setActionsEnabled(false);
	codeEditor_->clearCursorLine();
	requiredFiles_.clear();
}

void MainWindow::run(){
	codeEditor_->clearCursorLine();
	debugger_.run();
	setStepActionsEnabled(false);
	state_ = STATE_NONE;
}

void MainWindow::stepOver(){
	codeEditor_->clearCursorLine();
	debugger_.stepOver();
	setStepActionsEnabled(false);
	state_ = STATE_NONE;
}

void MainWindow::stepInto(){
	codeEditor_->clearCursorLine();
	debugger_.stepInto();
	setStepActionsEnabled(false);
	state_ = STATE_NONE;
}

void MainWindow::stepOut(){
	codeEditor_->clearCursorLine();
	debugger_.stepOut();
	setStepActionsEnabled(false);
	state_ = STATE_NONE;
}

void MainWindow::onUpdate(){
	if(state_==STATE_REQUIRING){
		state_ = STATE_NONE;
		setStepActionsEnabled(true);
		required();
	}

	/*else{
		if(CodeEditorPage* p=codeEditor_->currentPage()){
			FileInfo* f=document_.findFile(p->sourcePath());

			if(CodePtr code = compile(p->toPlainText().toStdString().c_str())){
				code->set_source_file_name(f->path.toStdString().c_str());
				requiredFiles_.insert(f->path);
				//debugger_.sendUpdatedSource(code);
				return;
			}
			else{
				XTAL_CATCH_EXCEPT(e){
					print(e->to_s()->c_str());
				}
			}
		}
	}*/
}

void MainWindow::onBreakpointChanged(const QString& path, int line, bool b){
	// 接続されていて、既にrequireされているファイルであるならば、debuggerに伝える
	if(debugger_.isConnected()){
		QSet<QString>::const_iterator it = requiredFiles_.find(path);
		if(it!=requiredFiles_.constEnd()){
			if(b){
				debugger_.sendAddBreakpoint(path, line, "");
			}
			else{
				debugger_.sendRemoveBreakpoint(path, line);
			}
		}
	}

	if(FileInfo* f=document_.findFile(path)){
		if(b){
			f->breakpoints.insert(line, "");
			breakpoint_->add(path, line, "");
		}
		else{
			f->breakpoints.erase(f->breakpoints.find(line));
			breakpoint_->remove(path, line);
		}
	}
}

void MainWindow::onBreakpointConditionChanged(const QString& path, int line, const QString& cond){
	if(debugger_.isConnected()){
		QSet<QString>::const_iterator it = requiredFiles_.find(path);
		if(it!=requiredFiles_.constEnd()){
			debugger_.sendAddBreakpoint(path, line, cond);
		}
	}

	if(FileInfo* f=document_.findFile(path)){
		f->breakpoints.insert(line, cond);
	}
}

void MainWindow::onViewBreakpoint(const QString& path, int line){
	codeEditor_->setPos(path, line);
}

void MainWindow::eraseBreakpoint(const QString& path, int line){
	onBreakpointChanged(path, line, false);
}

void MainWindow::print(const QString& mes){
	messages_->append(mes);
}

void MainWindow::moveCallStack(int n){
	if(debugger_.isConnected()){
		debugger_.sendMoveCallStack(n);
	}
}

