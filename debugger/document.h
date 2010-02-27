#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <QtGui>
#include <QPlainTextEdit>
#include <QObject>

/**
  * \brief プロジェクトの情報を保持するクラス
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

public:

	FileInfo* file(int i);

	int fileCount();

	bool addFile(const QString& file);

	int findFile(const QString& file);

	int findFileAbout(const QString& file);

public:

	void setEvalExpr(int n, const QString& expr){
		if(n>=evalExprs_.size()){
			evalExprs_.resize(n+1);
		}

		evalExprs_[n] = expr;
	}

	int evalExprCount(){
		return evalExprs_.size();
	}

	QString evalExpr(int n){
		return evalExprs_[n];
	}

private:
	QVector<FileInfo> files_;
	QVector<QString> evalExprs_;
};


#endif // DOCUMENT_H
