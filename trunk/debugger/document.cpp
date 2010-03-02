#include "document.h"
#include <QtXml>
#include <QDomDocument>

void Document::init(){
	files_.clear();
	evalExprs_.clear();
}

bool Document::save(const QString& filename){
	QFile file(filename);
	if(!file.open(QIODevice::WriteOnly)){
		 return false;
	}
	QTextStream ds(&file);

	QDomDocument doc;

	QDomElement root = doc.createElement("project");
	doc.appendChild(root);

	QDomElement files = doc.createElement("sourcePathList");
	root.appendChild(files);

	for(int i=0; i<files_.size(); ++i){
		QDomElement node1 = doc.createElement("path");
		QDomText node2 = doc.createTextNode(files_[i].path);
		node1.appendChild(node2);
		files.appendChild(node1);
	}

	QDomElement evalExprs = doc.createElement("evalExprList");
	root.appendChild(evalExprs);
	for(int i=0; i<evalExprs_.size(); ++i){
		QDomElement node1 = doc.createElement("evalExpr");
		QDomText node2 = doc.createTextNode(evalExprs_[i]);
		node1.appendChild(node2);
		evalExprs.appendChild(node1);
	}

	QDomElement breakpoints = doc.createElement("breakpointList");
	root.appendChild(breakpoints);
	for(int i=0; i<files_.size(); ++i){
		FileInfo* f = &files_[i];
		QMap<int, QString>::iterator it = f->breakpoints.begin();
		for(; it!=f->breakpoints.end(); ++it){
			QDomElement node1 = doc.createElement("breakpoint");
			node1.setAttribute("file", f->path);
			node1.setAttribute("lineno", it.key());
			node1.setAttribute("condition", it.value());
			breakpoints.appendChild(node1);
		}
	}

	doc.save(ds, 4);

	return true;
}

bool Document::load(const QString& filename){
	QFile file(filename);
	if(!file.open(QIODevice::ReadOnly)){
		 return false;
	}

	init();

	QDomDocument doc;
	QString errorMessage;
	int errorLine;
	int errorColumn;
	if(!doc.setContent(&file, true, &errorMessage, &errorLine, &errorColumn)){
		return false;
	}

	QDomElement root = doc.documentElement();
	if(root.tagName()!="project"){
		return false;
	}

	for(QDomNode node=root.firstChild(); !node.isNull(); node=node.nextSibling()){
		if(node.toElement().tagName()=="sourcePathList"){
			for(QDomNode node2 = node.toElement().firstChild(); !node2.isNull(); node2=node2.nextSibling()){
				QDomNode node3 = node2.toElement().firstChild();
				if(node3.nodeType()==QDomNode::TextNode){
					FileInfo fi;
					fi.path = node3.toText().data();
					files_.push_back(fi);
				}
			}
		}

		if(node.toElement().tagName()=="evalExprList"){
			for(QDomNode node2 = node.toElement().firstChild(); !node2.isNull(); node2=node2.nextSibling()){
				QDomNode node3 = node2.toElement().firstChild();
				if(node3.nodeType()==QDomNode::TextNode){
					evalExprs_.push_back(node3.toText().data());
				}
			}
		}

		if(node.toElement().tagName()=="breakpointList"){
			for(QDomNode node2 = node.toElement().firstChild(); !node2.isNull(); node2=node2.nextSibling()){
				QDomElement elem = node2.toElement();
				QString path = elem.attribute("file");
				findFile(path)->breakpoints.insert(elem.attribute("lineno").toInt(), elem.attribute("condition"));
			}
		}
	}

	return true;
}

FileInfo* Document::file(int i){
	if(i<0 || i>=files_.size()){
		return 0;
	}
	return &files_[i];
}

int Document::fileCount(){
	return files_.size();
}

bool Document::addFile(const QString& file){
	if(!findFile(file)){
		FileInfo fi;
		fi.path = file;
		files_.push_back(fi);
		return true;
	}
	return false;
}

bool Document::removeFile(const QString& file){
	for(int i=0; i<files_.size(); ++i){
		if(files_[i].path==file){
			files_.erase(files_.begin()+i);
			return true;
		}
	}
	return false;
}

FileInfo* Document::findFile(const QString& file){
	for(int i=0; i<files_.size(); ++i){
		if(files_[i].path==file){
			return &files_[i];
		}
	}
	return 0;
}

FileInfo* Document::findFileAbout(const QString& file){
	QString str = file + ".xtal";
	QRegExp r2(".*\\/(.+)$");
	r2.setMinimal(true);

	while(true){
		QRegExp r(QRegExp::escape(str));

		for(int i=0; i<files_.size(); ++i){
			FileInfo& f = files_[i];

			if(r.indexIn(f.path)>-1){
				return &f;
			}
		}

		if(r2.indexIn(str)>-1){
			str = r2.cap(1);
		}
		else{
			break;
		}
	}

	return 0;
}
