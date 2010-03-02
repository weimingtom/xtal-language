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

	void addFile(const QString& filename);

signals:

	void onAddFile(const QString& filename);

	void onView(const QString& filename);

	void removeFile(const QString& filename);

public slots:

	void onClicked(const QModelIndex& index);

protected:

	void keyPressEvent(QKeyEvent* event){
		if(event->key()==Qt::Key_Delete){
			QModelIndexList list = selectedIndexes();
			for(int i=0; i<list.size(); ++i){
				removeFile(model_->item(list.at(i).row(), 0)->data().toString());
				model_->removeRow(list.at(i).row());
			}
		}
	}

private:
	QStandardItemModel* model_;
};

#endif // PROJECTVIEW_H
