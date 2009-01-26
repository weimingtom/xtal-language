#include "xtal.h"

#ifndef XTAL_NO_PARSER

namespace xtal{

const ExprPtr& ep(const AnyPtr& a){
	return ptr_cast<Expr>(a);
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
	ExprPtr ret = xnew<Expr>(tag, lineno_);
	for(uint_t i=root_->size()-num; i<root_->size(); ++i){
		ret->push_back(root_->at(i));
	}
	root_->resize(root_->size()-num);
	root_->push_back(ret);
}

}

#endif