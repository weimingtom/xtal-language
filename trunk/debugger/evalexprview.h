#ifndef EVALEXPRVIEW_H
#define EVALEXPRVIEW_H

#include <QtGui>

#include "../src/xtal/xtal.h"
#include "../src/xtal/xtal_macro.h"
using namespace xtal;

/**
  * \brief 変数の表示ツリービュー
  */
class EvalExprView : public QTreeView{
	Q_OBJECT
public:

	EvalExprView(QWidget *parent);

	void init(){
		model_->setRowCount(0);
		model_->setRowCount(256);
	}

public slots:

	void dataChanged(QStandardItem* item);

signals:

	void exprChanged(int i, const QString& expr);

public:

	void setExpr(int n, const QString& expr);

	void setExprResult(int n, const ArrayPtr& value);

	QString expr(int n);

	int exprCount(){ return exprCount_; }

public:

	void setChild(QStandardItem* item, int n, const QString& key, const ArrayPtr& value);

	void setChild(QStandardItem* item, const AnyPtr& children);

	QStandardItem* makeItem(const QString& text, bool editable = false);

private:
	QStandardItemModel* model_;
	int exprCount_;
	bool dataChanged_;
};

#endif // EVALEXPRVIEW_H
