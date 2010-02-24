#include "projectview.h"

ProjectView::ProjectView(QWidget *parent)
	:QTreeView(parent){
	model_ = new QStandardItemModel();
	setModel(model_);

	setDragEnabled(true);
	setDragDropMode(QAbstractItemView::DragDrop);
	setHeaderHidden(true);

	connect(this, SIGNAL(clicked(const QModelIndex&)), this, SLOT(on_clicked(const QModelIndex&)));
}

void ProjectView::init(){
	model_->clear();
}

QStandardItem* ProjectView::make_item(const QString& text, const QString& data){
	QStandardItem* ret = new QStandardItem(text);
	ret->setData(data);
	ret->setEditable(false);
	return ret;
}

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

		//model_->invisibleRootItem()->appendRow(make_item(file));
		emit on_add_file(file);
	}
}

void ProjectView::dragMoveEvent(QDragMoveEvent *event){
	if(event->mimeData()->hasFormat("text/uri-list")){
		event->acceptProposedAction();
	}
}

void ProjectView::add_file(const QString& filename, const QString& data){
	model_->invisibleRootItem()->appendRow(make_item(filename, data));
}

void ProjectView::on_clicked(const QModelIndex& index){
	if(QStandardItem* ret = model_->item(index.row(), index.column())){
		emit on_view(ret->text());
	}
}
