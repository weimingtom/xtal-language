#ifndef PROJECTVIEW_H
#define PROJECTVIEW_H

#include <QtGui>

class ProjectView : public QTreeView{
	Q_OBJECT
public:

	ProjectView(QWidget *parent = 0);

public:

	void init();

	QStandardItem* makeItem(const QString& text, const QString& data);

	void dragEnterEvent(QDragEnterEvent *event);

	void dropEvent(QDropEvent *event);

	void dragMoveEvent(QDragMoveEvent *event);

	void addFile(const QString& filename, const QString& data);

signals:

	void onAddFile(const QString& filename);
	void onView(const QString& filename);

public slots:

	void onClicked(const QModelIndex& index);

private:
	QStandardItemModel* model_;
};

#endif // PROJECTVIEW_H
