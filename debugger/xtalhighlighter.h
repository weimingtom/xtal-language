#ifndef XTALHIGHLIGHTER_H
#define XTALHIGHLIGHTER_H

#include <QtGui>
#include <QPlainTextEdit>
#include <QObject>

/**
  * \brief Xtalのシンタックスハイライタ
  */
class XtalHighlighter : public QSyntaxHighlighter{
	Q_OBJECT

public:

	XtalHighlighter(QTextDocument *parent = 0);

protected:

	void highlightBlock(const QString &text);

private:

	struct HighlightingRule{
		QRegExp pattern;
		QTextCharFormat format;
	};

	QVector<HighlightingRule> highlighting_rules_;

	QRegExp comment_start_expression_;
	QRegExp comment_end_expression_;

	QTextCharFormat keyword_format_;
	QTextCharFormat class_format_;
	QTextCharFormat singleline_comment_format_;
	QTextCharFormat multiline_comment_format_;
	QTextCharFormat quotation_format_;
	QTextCharFormat number_format_;
};

#endif // XTALHIGHLIGHTER_H
