#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <QtGui>
#include <QPlainTextEdit>
#include <QObject>

/**
  * \brief
　　* ソースパス、ブレークポイントの位置を保持している
  */
class Document{
public:

	struct FileInfo{
		QString path;
		QSet<int> breakpoints;
	};

	void init();

	bool save(const QString& filename);

	bool load(const QString& filename);

	FileInfo* file(int i);

	int file_count();

	bool add_file(const QString& file);

	int find_file(const QString& file);

	int find_file_about(const QString& file);

	void set_eval_expr(int n, const QString& expr){
		if(n>=eval_exprs_.size()){
			eval_exprs_.resize(n+1);
		}

		eval_exprs_[n] = expr;
	}

	int eval_expr_num(){
		return eval_exprs_.size();
	}

	QString eval_expr(int n){
		return eval_exprs_[n];
	}

private:
	QVector<FileInfo> files_;
	QVector<QString> eval_exprs_;
};


#endif // DOCUMENT_H
