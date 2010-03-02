#ifndef CALLSTACKVIEW_H
#define CALLSTACKVIEW_H

#include <QtGui>

#include "../src/xtal/xtal.h"
#include "../src/xtal/xtal_macro.h"
using namespace xtal;

/**
  * \breif コールスタックの表示のツリービュー
  */
class CallStackView : public QTreeView{
	Q_OBJECT
public:

	CallStackView(QWidget *parent = 0);

public:

	void init(){
		clear();
	}

	QStandardItem* makeItem(const QString& text);

	void view(const VMachinePtr& vm);

	void set(int i, const StringPtr& fun, const StringPtr& file, int line);

	void clear();

	void setLevel(int n);

public slots:

	void onClicked(const QModelIndex & index);

signals:

	void moveCallStack(int n);

private:
	QStandardItemModel* model_;
};

#endif // CALLSTACKVIEW_H
