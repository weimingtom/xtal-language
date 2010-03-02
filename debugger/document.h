#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <QtGui>
#include <QPlainTextEdit>
#include <QObject>

struct FileInfo{
	QString path;
	QMap<int, QString> breakpoints;
};

/**
  * \brief プロジェクトの情報を保持するクラス
　 * ソースパス、ブレークポイントの位置を保持している
  */
class Document{
public:

	void init();

	bool save(const QString& filename);

	bool load(const QString& filename);

public:

	/**
	  * \brief i番目のファイル情報を取り出す
	  */
	FileInfo* file(int i);

	/**
	  * \brief ファイル情報が何個あるか返す
	  */
	int fileCount();

	/**
	  * \brief プロジェクトにfileを追加する
	  */
	bool addFile(const QString& file);

	bool removeFile(const QString& file);

	/**
	  * \brief 文字列fileにマッチするファイル情報を取り出す
	  */
	FileInfo* findFile(const QString& file);

	/**
	  * \brief 部分文字列fileにマッチするファイル情報を取り出す
	  */
	FileInfo* findFileAbout(const QString& file);

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

	void removeEvalExpr(int n);

	void insertEvaExpr(int n);

private:

	QVector<FileInfo> files_;
	QVector<QString> evalExprs_;
};


#endif // DOCUMENT_H
