#include "xtal.h"

#ifndef XTAL_NO_PARSER

namespace xtal{

const ExprPtr& ep(const AnyPtr& a){
	if(type(a)==TYPE_TREE_NODE){
		return unchecked_ptr_cast<Expr>(a);
	}
	return unchecked_ptr_cast<Expr>(null);
}

const AnyPtr& Expr::at(uint_t i){
	if(size()<=i){
		resize(i+1);
	}
	return Array::at(i);
}

void Expr::set_at(uint_t i, const AnyPtr& v){
	if(size()<=i){
		resize(i+1);
	}
	Array::set_at(i, v); 
}

void ExprBuilder::splice(int_t tag, int_t num){
	splice(tag, num, num==0 ? lineno_ : linenos_[root_->size()-num]);
}

void ExprBuilder::splice(int_t tag, int_t num, int_t lineno){
	ExprPtr ret = xnew<Expr>(tag, lineno);
	for(uint_t i=root_->size()-num; i<root_->size(); ++i){
		ret->push_back(root_->at(i));
	}
	linenos_.resize(root_->size()-num);
	root_->resize(root_->size()-num);
	linenos_.push_back(lineno);
	root_->push_back(ret);
}

void ExprBuilder::push(const AnyPtr& v){
	linenos_.push_back(lineno_);
	root_->push_back(v);
}

}

#endif
