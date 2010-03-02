#ifndef BREAKPOINTVIEW_H
#define BREAKPOINTVIEW_H

#include <QtGui>

#include "../src/xtal/xtal.h"
#include "../src/xtal/xtal_macro.h"
using namespace xtal;

/**
  * \brief ブレークポイントの表示ツリービュー
  */
class BreakpointView : public QTreeView{
	Q_OBJECT
public:
	BreakpointView(QWidget* parent = 0);

	void add(const QString& file, int line, const QString& cond);
	void remove(const QString& file, int line);

	void clear();

	void init(){
		model_->setRowCount(0);
	}

public slots:

	void dataChanged(QStandardItem* item);

	void onClicked(const QModelIndex & index);

signals:

	void breakpointConditionChanged(const QString& file, int line, const QString& cond);

	void viewBreakpoint(const QString& file, int line);

	void eraseBreakpoint(const QString& file, int line);

protected:

	QStandardItem* makeItem(const QString& text, bool editable = false);

private:
	QStandardItemModel* model_;
};

#endif // BREAKPOINTVIEW_H
