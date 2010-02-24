#include "callstackview.h"

CallStackView::CallStackView(){
	model_ = new QStandardItemModel();
	setModel(model_);

	model_->setHorizontalHeaderItem(0, new QStandardItem("function"));
	model_->setHorizontalHeaderItem(1, new QStandardItem("file"));
	model_->setHorizontalHeaderItem(2, new QStandardItem("line"));
}

QStandardItem* CallStackView::make_item(const QString& text){
	QStandardItem* ret = new QStandardItem(text);
	ret->setEditable(false);
	return ret;
}

void CallStackView::view(const VMachinePtr& vm){
	for(int i=0;; ++i){
		if(debug::CallerInfoPtr caller = vm->caller(i)){
			model_->setItem(i, 0, make_item(caller->fun_name()->c_str()));
			model_->setItem(i, 1, make_item(caller->file_name()->c_str()));
			if(caller->line()){
				model_->setItem(i, 2, make_item(QString("%1").arg(caller->line())));
			}
		}
		else{
			break;
		}
	}
}

void CallStackView::set(int i, const StringPtr& fun, const StringPtr& file, int line){
	model_->setItem(i, 0, make_item(fun->c_str()));
	model_->setItem(i, 1, make_item(file->c_str()));
	if(line){
		model_->setItem(i, 2, make_item(QString("%1").arg(line)));
	}
}

void CallStackView::clear(){
	model_->setRowCount(0);
}
