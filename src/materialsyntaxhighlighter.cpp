#include "materialsyntaxhighlighter.h"
#include "languageloader.h"

MaterialSyntaxHighlighter::MaterialSyntaxHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter (parent)
{
    LanguageLoader *ll = new LanguageLoader();
    ll->loadFromFile("/usr/share/gtksourceview-3.0/language-specs/cpp.lang");
    delete ll;
}

void MaterialSyntaxHighlighter::highlightBlock(const QString &text)
{
    QTextCharFormat myFormat;
    myFormat.setFontOverline(true);
    //setFormat(0, text.length(), myFormat);
}
