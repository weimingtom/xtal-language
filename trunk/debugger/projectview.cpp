#include "projectview.h"

ProjectView::ProjectView(QWidget *parent)
	:QTreeView(parent){
	model_ = new ProjectTreeModel(this);
	model_->setRoot(new ProjectTreeNode(ProjectTreeNode::TYPE_FOLDER, "", 0));
	setModel(model_);

	setDragEnabled(true);
	setDragDropMode(QAbstractItemView::DragDrop);
	setHeaderHidden(true);

	connect(this, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(onClicked(const QModelIndex&)));
}

void ProjectView::init(){
//	model_->clear();
}
/*
QStandardItem* ProjectView::makeItem(const QString& text, const QString& data){
	QStandardItem* ret = new QStandardItem(text);
	ret->setData(data);
	ret->setEditable(false);
	return ret;
}
*/
void ProjectView::dragEnterEvent(QDragEnterEvent *event){
	dragMoveEvent(event);
}

void ProjectView::dropEvent(QDropEvent *event){
	QList<QUrl> urls = event->mimeData()->urls();
	if(urls.empty()){
		return;
	}

	foreach(QUrl url, urls){
		QString file = url.toLocalFile();
		if(file.isEmpty()){
			continue;
		}

		model_->addNode(file);
	}
}

void ProjectView::dragMoveEvent(QDragMoveEvent *event){
	if(event->mimeData()->hasFormat("text/uri-list")){
		event->acceptProposedAction();
	}
}
/*
void ProjectView::addFile(const QString& filename){
	model_->invisibleRootItem()->appendRow(makeItem(QFile(filename).fileName(), filename));
}
*/
void ProjectView::onClicked(const QModelIndex& index){
	//if(QStandardItem* ret = model_->item(index.row(), index.column())){
	//	emit onView(ret->text());
	//}
}
