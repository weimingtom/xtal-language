#ifndef XTALSRC_H
#define XTALSRC_H

#include "../src/xtal/xtal.h"
#include "../src/xtal/xtal_macro.h"
using namespace xtal;

inline StringPtr qstr2xstr(const QString& str){
	return str.toStdString().c_str();
}

inline StringPtr qstr2xid(const QString& str){
	return intern(str.toStdString().c_str());
}

inline QString xstr2qstr(const StringPtr& str){
	return str->c_str();
}

#endif // XTALSRC_H
