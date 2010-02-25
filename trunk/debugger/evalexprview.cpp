#include "evalexprview.h"

EvalExprView::EvalExprView(QWidget *parent)
	:QTreeView(parent){
	model_ = new QStandardItemModel();
	//model_->setColumnCount(3);
	model_->setRowCount(0);
	model_->setRowCount(256);
	setModel(model_);

	model_->setHorizontalHeaderItem(0, new QStandardItem("expr"));
	model_->setHorizontalHeaderItem(1, new QStandardItem("value"));
	model_->setHorizontalHeaderItem(2, new QStandardItem("class"));

	connect(model_, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(data_changed(QStandardItem*)));

	data_changed_ = false;
}

void EvalExprView::data_changed(QStandardItem* item){
	if(item->isEditable() && item->column()==0 && item->text()!=""){
		emit expr_changed(item->row(), item->text());
	}
}

void EvalExprView::set_expr(int n, const QString& expr){
	QStandardItem* item = make_item(expr);
	item->setEditable(true);
	model_->invisibleRootItem()->setChild(n, 0, item);
	item->setEditable(false);
}

QString EvalExprView::item(int n){
	if(model_->item(n, 0)){
		return model_->item(n, 0)->text();
	}
	return QString("");
}

void EvalExprView::set_item(int n, const ArrayPtr& value){
	set_child(model_->invisibleRootItem(), n, 0, value);
}

void EvalExprView::set_child(QStandardItem* item, int n, const QString& key, const ArrayPtr& value){
	if(value){
		if(key!=""){
			item->setChild(n, 0, make_item(key));
		}
		item->setChild(n, 1, make_item(value->at(1)->to_s()->c_str()));
		item->setChild(n, 2, make_item(value->at(0)->to_s()->c_str()));
		set_child(item->child(n), value->at(2));
	}
	else{
		if(item->child(n, 1)) item->child(n, 1)->setText("");
		if(item->child(n, 2)) item->child(n, 2)->setText("");
		if(item->child(n, 0)) item->child(n, 0)->removeRows(0, item->child(n, 0)->row());
	}
}

void EvalExprView::set_child(QStandardItem* item, const AnyPtr& children){
	if(ArrayPtr ary = ptr_cast<Array>(children)){
		int i = 0;
		Xfor(val, ary){
			ArrayPtr it = ptr_cast<Array>(val);
			set_child(item, i, QString("%1").arg(i), it);
			++i;
		}
	}
	else if(MapPtr map = ptr_cast<Map>(children)){
		int i = 0;
		Xfor2(key, val, map){
			ArrayPtr it = ptr_cast<Array>(val);
			set_child(item, i, key->to_s()->c_str(), it);
			++i;
		}
	}
	else if(StringPtr str = ptr_cast<String>(children)){
		item->setChild(0, 0, make_item(str->c_str()));
	}
}

QStandardItem* EvalExprView::make_item(const QString& text){
	QStandardItem* ret = new QStandardItem(text);
	ret->setEditable(false);
	return ret;
}
