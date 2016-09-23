#ifndef MATERIALSYNTAXHIGHLIGHTER_H
#define MATERIALSYNTAXHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include "languagespecification.h"

class MaterialSyntaxHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT
public:
    MaterialSyntaxHighlighter(QTextDocument *parent);

protected:
    void highlightBlock(const QString &text);
    LanguageSpecification *lang;
};

#endif // MATERIALSYNTAXHIGHLIGHTER_H
