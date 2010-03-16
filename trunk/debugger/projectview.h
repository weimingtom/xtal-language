#ifndef PROJECTVIEW_H
#define PROJECTVIEW_H

#include <QtGui>

class ProjectTreeNode{
public:
	enum{
		TYPE_FOLDER,
		TYPE_FILE,
	};

	ProjectTreeNode(int type, const QString& name, ProjectTreeNode* parent)
		:type(type), name(name), parent(parent){}

	~ProjectTreeNode(){
		qDeleteAll(children);
	}

	int type;
	QString name;
	ProjectTreeNode* parent;
	QList<ProjectTreeNode*> children;
};

class ProjectTreeModel : public QAbstractItemModel{
	Q_OBJECT
public:

	ProjectTreeModel(QObject* parent=0)
		:QAbstractItemModel(parent){
		root_ = 0;
	}

	~ProjectTreeModel(){
		delete root_;
	}

	void setRoot(ProjectTreeNode* p){
		delete root_;
		root_ = p;
		reset();
	}

	void addNode(const QString& name){
		root_->children.push_back(new ProjectTreeNode(ProjectTreeNode::TYPE_FILE, name,root_));
		reset();
	}

	ProjectTreeNode* root(){
		return root_;
	}

	QModelIndex index(int row, int column, const QModelIndex& parent) const{
		if(ProjectTreeNode* p=toNode(parent)){
			return createIndex(row, column, p->children[row]);
		}

		return QModelIndex();
	}

	QModelIndex parent(const QModelIndex& child) const{
		if(ProjectTreeNode* c=toNode(child)){
			if(ProjectTreeNode* p=c->parent){
				if(ProjectTreeNode* pp=p->parent){
					return createIndex(pp->children.indexOf(p), 0, p);
				}
			}
		}
		return QModelIndex();
	}

	int rowCount(const QModelIndex& parent) const{
		if(ProjectTreeNode* p=toNode(parent)){
			return p->children.count();
		}
		return 0;
	}

	int columnCount(const QModelIndex& parent) const{
		return 1;
	}

	QVariant data(const QModelIndex& index, int role) const{
		if(role!=Qt::DisplayRole){
			return QVariant();
		}

		if(ProjectTreeNode* p=toNode(index)){
			if(index.column()==0){
				return p->name;
			}
			else if(index.column()==1){
				return p->name;
			}
		}

		return QVariant();
	}

private:

	ProjectTreeNode* toNode(const QModelIndex& index) const{
		if(index.isValid()){
			return (ProjectTreeNode*)index.internalPointer();
		}
		else{
			return root_;
		}
	}

	ProjectTreeNode* root_;
};

class ProjectView : public QTreeView{
	Q_OBJECT
public:

	ProjectView(QWidget *parent = 0);

public:

	void init();

	void setRoot(ProjectTreeNode* p){
		model_->setRoot(p);
	}

	void dragEnterEvent(QDragEnterEvent *event);

	void dropEvent(QDropEvent *event);

	void dragMoveEvent(QDragMoveEvent *event);

signals:

	void onView(const QString& filename);

	void removeFile(const QString& filename);

public slots:

	void onClicked(const QModelIndex& index);

protected:

	/*
	void keyPressEvent(QKeyEvent* event){
		if(event->key()==Qt::Key_Delete){
			QModelIndexList list = selectedIndexes();
			for(int i=0; i<list.size(); ++i){
				removeFile(model_->item(list.at(i).row(), 0)->data().toString());
				model_->removeRow(list.at(i).row());
			}
		}
	}
	*/

private:
	ProjectTreeModel* model_;
};

#endif // PROJECTVIEW_H
