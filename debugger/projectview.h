#ifndef PROJECTVIEW_H
#define PROJECTVIEW_H

#include <QtGui>

class ProjectView : public QTreeView{
	Q_OBJECT
public:

	ProjectView(QWidget *parent = 0);

public:

	void init();

	QStandardItem* make_item(const QString& text, const QString& data);

	void dragEnterEvent(QDragEnterEvent *event);

	void dropEvent(QDropEvent *event);

	void dragMoveEvent(QDragMoveEvent *event);

	void add_file(const QString& filename, const QString& data);

signals:

	void on_add_file(const QString& filename);
	void on_view(const QString& filename);

public slots:

	void on_clicked(const QModelIndex& index);

private:
	QStandardItemModel* model_;
};

#endif // PROJECTVIEW_H
