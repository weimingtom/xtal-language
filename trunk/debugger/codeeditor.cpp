#include "codeeditor.h"

CodeEditorPage::CodeEditorPage(QWidget *parent)
:QPlainTextEdit(parent){
	block_count_ = 0;

	lineNumberArea = new LineNumberArea(this);

	connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(on_block_count_changed(int)));
	connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth(int)));
	connect(this, SIGNAL(updateRequest(const QRect &, int)), this, SLOT(updateLineNumberArea(const QRect &, int)));
	//connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLine()));

	updateLineNumberAreaWidth(0);
	highlightCurrentLine();

	line_data_.clear();

	text_ = new QTextDocument();
	highlighter_ = new XtalHighlighter(text_);
	text_->setDocumentLayout(new QPlainTextDocumentLayout(text_));
	setDocument(text_);
	setLineWrapMode(QPlainTextEdit::NoWrap);

	QFont font = text_->defaultFont();
	font.setFamily("Courier");
	font.setFixedPitch(true);
	font.setKerning(false);
	font.setPointSize(10);
	text_->setDefaultFont(font);

	setTabStopWidth(20);

	connect(this, SIGNAL(textChanged()), this, SLOT(on_text_changed()));
	changed_ = false;

}

int CodeEditorPage::lineNumberAreaWidth(){
	int space = 3 + fontMetrics().width(QLatin1Char('9')) * 5;
	return space;
}

void CodeEditorPage::updateLineNumberAreaWidth(int /* newBlockCount */){
	setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void CodeEditorPage::updateLineNumberArea(const QRect &rect, int dy){
	if (dy)
		lineNumberArea->scroll(0, dy);
	else
		lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());

	if (rect.contains(viewport()->rect()))
		updateLineNumberAreaWidth(0);
}

void CodeEditorPage::resizeEvent(QResizeEvent *e){
	QPlainTextEdit::resizeEvent(e);

	QRect cr = contentsRect();
	lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void CodeEditorPage::highlightCurrentLine(){
	QList<QTextEdit::ExtraSelection> extraSelections;

	if (!isReadOnly()) {
		QTextEdit::ExtraSelection selection;

		QColor lineColor = QColor(Qt::yellow).lighter(120);

		selection.format.setBackground(lineColor);
		selection.format.setProperty(QTextFormat::FullWidthSelection, true);
		selection.cursor = textCursor();
		selection.cursor.clearSelection();
		extraSelections.append(selection);
	}

	setExtraSelections(extraSelections);
}

void CodeEditorPage::lineNumberAreaPaintEvent(QPaintEvent *event){
	QPainter painter(lineNumberArea);
	painter.fillRect(event->rect(), Qt::lightGray);

	QTextBlock block = firstVisibleBlock();
	int blockNumber = block.blockNumber();
	int top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
	int bottom = top + (int) blockBoundingRect(block).height();

	while (block.isValid() && top <= event->rect().bottom()) {
		if (block.isVisible() && bottom >= event->rect().top()) {
			QString number = QString::number(blockNumber + 1);
			painter.setPen(Qt::black);
			painter.drawText(0, top, lineNumberArea->width(), fontMetrics().height(),
							 Qt::AlignRight, number);

			if(block.blockNumber()<line_data_.size()){
				LineData& data = line_data_[block.blockNumber()];
				if(data.breakpoint){
					painter.setPen(Qt::red);
					painter.setBrush(Qt::red);
					painter.drawEllipse(QPoint(4, (top+bottom)/2-2), 4, 4);
				}
			}
		}

		block = block.next();
		top = bottom;
		bottom = top + (int) blockBoundingRect(block).height();
		++blockNumber;
	}
}

void CodeEditorPage::on_text_changed(){
	if(!changed_){
		emit text_changed(this);
		changed_ = true;
	}
}

QTextBlock CodeEditorPage::block(int line){
	QTextBlock block = firstVisibleBlock();
	while(block.blockNumber()+1<line && block.isValid()){
		block = block.next();
	}
	return block;
}

void CodeEditorPage::set_cursor_line(int line){
	QTextBlock b = text_->findBlockByLineNumber(line-1);
	setTextCursor(QTextCursor(b));
	highlightCurrentLine();
}

void CodeEditorPage::click_event(QMouseEvent* event){
	QTextBlock block = firstVisibleBlock();
	int blockNumber = block.blockNumber();
	int y = this->mapFromGlobal(event->globalPos()).y();
	int top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
	int bottom = top + (int) blockBoundingRect(block).height();

	while(block.isValid()) {
		if(block.isVisible() && top<y && y<bottom) {
			set_breakpoint(block.blockNumber()+1);
			return;
		}

		block = block.next();
		top = bottom;
		bottom = top + (int) blockBoundingRect(block).height();
		++blockNumber;
	}
}

void CodeEditorPage::set_breakpoint(int n){
	line_data_[n-1].breakpoint = !line_data_[n-1].breakpoint;
	this->update();

	emit breakpoint_changed(source_path_, n, line_data_[n-1].breakpoint);
}

void CodeEditorPage::save(){
	QFile data(source_path_);
	if(data.open(QFile::WriteOnly | QFile::Truncate)){
		QTextStream out(&data);
		out << text_->toPlainText();
		changed_ = false;
	}
}

void CodeEditorPage::on_block_count_changed(int block_count){
	QList<QTextEdit::ExtraSelection> sel = this->extraSelections();
	if(line_data_.size()!=0){
		if(sel.size()>0){
			QTextCursor cur = textCursor();
			int diff = block_count - line_data_.size();
			int n = cur.block().blockNumber();
			if(diff>0){
				line_data_.insert(line_data_.begin()+n+1-diff, diff, LineData());
			}
			else{
				line_data_.erase(line_data_.begin()+n+1, line_data_.begin()+n+1-diff);
			}

		}
	}
	line_data_.resize(block_count);
}

//////////////////////

CodeEditor::CodeEditor(){
	CodeEditorPage* page = new CodeEditorPage();
	setTabsClosable(true);
	page->setEnabled(false);

	connect(this, SIGNAL(tabCloseRequested(int)), this, SLOT(close_page(int)));
}

void CodeEditor::close_page(int index){
	if(widget(index)->source_path()!=""){
		this->removeTab(index);
	}
}

void CodeEditor::on_text_changed(CodeEditorPage* p){
	int i = find_widget(p);
	this->setTabText(i, "*" + this->tabText(i));
}

void CodeEditor::on_breakpoint_changed(const QString& path, int n, bool b){
	emit breakpoint_changed(path, n, b);
}
void CodeEditor::add_page(const QString& file, const QString& path){
	if(CodeEditorPage* p = widget(find_widget(path))){
		this->setCurrentWidget(p);
		return;
	}

	CodeEditorPage* page = new CodeEditorPage();
	page->set_source_path(path);
	addTab(page, file);
	this->setCurrentWidget(page);

	QFile f(path);
	if(f.open(QIODevice::ReadOnly | QIODevice::Text)){
		QTextStream ts(&f);
		page->set_plain_text(ts.readAll());
	}

	connect(page, SIGNAL(text_changed(CodeEditorPage*)), this, SLOT(on_text_changed(CodeEditorPage*)));
	connect(page, SIGNAL(breakpoint_changed(const QString&,int,bool)), this, SLOT(on_breakpoint_changed(const QString&,int,bool)));
}

CodeEditorPage* CodeEditor::widget(int index){
	return (CodeEditorPage*)QTabWidget::widget(index);
}

CodeEditorPage* CodeEditor::current_page(){
	return (CodeEditorPage*)currentWidget();
}

void CodeEditor::set_pos(const QString& filename, int lineno){
	set_source_file(filename);
	set_cursor_line(lineno);
}

void CodeEditor::set_cursor_line(int n){
	if(current_page()){
		current_page()->set_cursor_line(n);
	}
}

int CodeEditor::find_widget(const QString& path){
	for(int i=0; i<this->count(); ++i){
		CodeEditorPage* p = (CodeEditorPage*)widget(i);
		if(p->source_path()==path){
			return i;
		}
	}
	return -1;
}

int CodeEditor::find_widget(CodeEditorPage* a){
	for(int i=0; i<this->count(); ++i){
		CodeEditorPage* p = (CodeEditorPage*)widget(i);
		if(p==a){
			return i;
		}
	}
	return -1;
}

void CodeEditor::set_source_file(const QString& path){
	if(CodeEditorPage* p = widget(find_widget(path))){
		this->setCurrentWidget(p);
	}
}

void CodeEditor::clear_cursor_line(){
	for(int i=0; i<this->count(); ++i){
		CodeEditorPage* p = (CodeEditorPage*)widget(i);
		p->setExtraSelections(QList<QTextEdit::ExtraSelection>());
	}
}

void CodeEditor::save_all(){
	for(int i=0; i<this->count(); ++i){
		CodeEditorPage* p = (CodeEditorPage*)widget(i);
		p->save();

		if(this->tabText(i).left(1)=="*"){
			this->setTabText(i, this->tabText(i).right(this->tabText(i).size()-1));
		}
	}
}
