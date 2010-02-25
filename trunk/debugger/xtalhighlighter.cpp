#include "xtalhighlighter.h"

XtalHighlighter::XtalHighlighter(QTextDocument *parent)
	: QSyntaxHighlighter(parent)
{
	HighlightingRule rule;

	keyword_format_.setForeground(Qt::blue);
	keyword_format_.setFontWeight(QFont::Bold);
	QStringList keyword_patterns;

	keyword_patterns
		<< "if"
		<< "for"
		<< "else"
		<< "fun"
		<< "method"
		<< "do"
		<< "while"
		<< "continue"
		<< "break"
		<< "fiber"
		<< "yield"
		<< "return"
		<< "once"
		<< "null"
		<< "undefined"
		<< "false"
		<< "true"
		<< "xtal"
		<< "try"
		<< "catch"
		<< "finally"
		<< "throw"
		<< "class"
		<< "callee"
		<< "this"
		<< "dofun"
		<< "is"
		<< "in"
		<< "assert"
		<< "nobreak"
		<< "switch"
		<< "case"
		<< "default"
		<< "singleton"
		<< "public"
		<< "protected"
		<< "private";

	foreach(const QString &pattern, keyword_patterns) {
		rule.pattern = QRegExp("\\b" + pattern + "\\b");
		rule.format = keyword_format_;
		highlighting_rules_.append(rule);
	}

	class_format_.setFontWeight(QFont::Bold);
	class_format_.setForeground(Qt::darkMagenta);
	rule.pattern = QRegExp("\\b[A-Z][A-Za-z0-9_]+\\b");
	rule.format = class_format_;
	highlighting_rules_.append(rule);

	number_format_.setFontWeight(QFont::Bold);
	number_format_.setForeground(Qt::darkBlue);
	rule.pattern = QRegExp("\\b[0-9]([0-9_]|\\.[0-9])*[fF]?\\b");
	rule.format = number_format_;
	highlighting_rules_.append(rule);

	singleline_comment_format_.setForeground(Qt::darkGreen);
	rule.pattern = QRegExp("//[^\n]*");
	rule.format = singleline_comment_format_;
	highlighting_rules_.append(rule);

	multiline_comment_format_.setForeground(Qt::darkGreen);

	quotation_format_.setForeground(Qt::darkRed);
	rule.pattern = QRegExp("\".*\"");
	rule.format = quotation_format_;
	highlighting_rules_.append(rule);

	comment_start_expression_ = QRegExp("/\\*");
	comment_end_expression_ = QRegExp("\\*/");
}

void XtalHighlighter::highlightBlock(const QString &text){
	foreach(const HighlightingRule &rule, highlighting_rules_){
		QRegExp expression(rule.pattern);
		int index = expression.indexIn(text);
		while(index >= 0){
			int length = expression.matchedLength();
			setFormat(index, length, rule.format);
			index = expression.indexIn(text, index + length);
		}
	}
	setCurrentBlockState(0);

	int startIndex = 0;
	if(previousBlockState() != 1){
		startIndex = comment_start_expression_.indexIn(text);
	}

	while(startIndex >= 0){
		int endIndex = comment_end_expression_.indexIn(text, startIndex);
		int commentLength;
		if(endIndex == -1){
			setCurrentBlockState(1);
			commentLength = text.length() - startIndex;
		}
		else{
			commentLength = endIndex - startIndex + comment_end_expression_.matchedLength();
		}

		setFormat(startIndex, commentLength, multiline_comment_format_);
		startIndex = comment_start_expression_.indexIn(text, startIndex + commentLength);
	}
}

