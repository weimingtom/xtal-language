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

	CallStackView();

public:

	QStandardItem* make_item(const QString& text);

	void view(const VMachinePtr& vm);

	void set(int i, const StringPtr& fun, const StringPtr& file, int line);

	void clear();

private:
	QStandardItemModel* model_;
};

#endif // CALLSTACKVIEW_H
