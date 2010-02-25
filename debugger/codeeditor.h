#ifndef CODEEDITOR_H
#define CODEEDITOR_H

#include "xtalhighlighter.h"

class LineData{
public:
	LineData(){ breakpoint = false; }
	bool breakpoint;
};

/**
  * \brief コードエディタ部分
  */
class CodeEditorPage : public QPlainTextEdit{
	Q_OBJECT
public:
	CodeEditorPage(QWidget *parent = 0);

	void lineNumberAreaPaintEvent(QPaintEvent *event);

	int lineNumberAreaWidth();

	QTextBlock block(int line);

	void set_cursor_line(int line);

	void click_event(QMouseEvent* event);

	void set_breakpoint(int n);

	void save();

	int line_count(){
		return line_data_.size();
	}

	bool breakpoint(int n){
		return line_data_[n-1].breakpoint;
	}

	const QString& source_path(){
		return source_path_;
	}

	void set_source_path(const QString& file){
		source_path_ = file;
	}

	void set_plain_text(const QString& text){
		text_->setPlainText(text);
		changed_ = false;
	}

protected:
	void resizeEvent(QResizeEvent *event);

private slots:

	void updateLineNumberAreaWidth(int newBlockCount);
	void highlightCurrentLine();
	void updateLineNumberArea(const QRect &, int);

	void on_text_changed();

	void on_block_count_changed(int block_count);

signals:

	void text_changed(CodeEditorPage* p);

	void breakpoint_changed(const QString& path, int n, bool b);

private:
	QTextDocument* text_;
	QWidget *lineNumberArea;
	QString source_path_;
	QVector<LineData> line_data_;
	int block_count_;
	bool changed_;
	XtalHighlighter* highlighter_;
};

class CodeEditor : public QTabWidget{
   Q_OBJECT
public:

	CodeEditor(QWidget *parent = 0);

public slots:

	void close_page(int index);

	void on_text_changed(CodeEditorPage* p);

	void on_breakpoint_changed(const QString& path, int n, bool b);

signals:

	void breakpoint_changed(const QString& path, int n, bool b);

public:

	void add_page(const QString& file, const QString& path);

	CodeEditorPage* widget(int index);

	CodeEditorPage* current_page();

	bool set_pos(const QString& filename, int lineno);

	void set_cursor_line(int n);

	int find_widget(const QString& path);

	int find_widget(CodeEditorPage* a);

	bool set_source_file(const QString& path);

	void clear_cursor_line();

	void save_all();
};

class LineNumberArea : public QWidget{
public:
	LineNumberArea(CodeEditorPage *editor) : QWidget(editor) {
		codeEditor = editor;
	}

	QSize sizeHint() const {
		return QSize(codeEditor->lineNumberAreaWidth(), 0);
	}

protected:

	void paintEvent(QPaintEvent *event) {
		codeEditor->lineNumberAreaPaintEvent(event);
	}

	void mouseReleaseEvent(QMouseEvent* event){
		codeEditor->click_event(event);
	}

private:
	CodeEditorPage *codeEditor;
};

#endif // CODEEDITOR_H
