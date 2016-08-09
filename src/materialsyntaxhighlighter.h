#ifndef MATERIALSYNTAXHIGHLIGHTER_H
#define MATERIALSYNTAXHIGHLIGHTER_H

#include <QSyntaxHighlighter>


class MaterialSyntaxHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT
public:
    MaterialSyntaxHighlighter(QTextDocument *parent);

protected:
    void highlightBlock(const QString &text);
};

#endif // MATERIALSYNTAXHIGHLIGHTER_H
