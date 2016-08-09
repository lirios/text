#include "materialsyntaxhighlighter.h"

MaterialSyntaxHighlighter::MaterialSyntaxHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter (parent)
{

}

void MaterialSyntaxHighlighter::highlightBlock(const QString &text)
{
    QTextCharFormat myFormat;
    myFormat.setFontOverline(true);
    //setFormat(0, text.length(), myFormat);
}
