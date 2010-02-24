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

	EvalExprView();

public slots:

	void data_changed(QStandardItem* item);

signals:

	void expr_changed(int i, const QString& expr);

public:

	void add_expr(int n, const QString& expr);

	QString item(int n);

	void set_item(int n, const ArrayPtr& value);

	void set_child(QStandardItem* item, int n, const QString& key, const ArrayPtr& value);

	void set_child(QStandardItem* item, const AnyPtr& children);

	QStandardItem* make_item(const QString& text);

private:
	QStandardItemModel* model_;
	bool data_changed_;
};

#endif // EVALEXPRVIEW_H
