#include "materialsyntaxhighlighter.h"
#include "languageloader.h"

MaterialSyntaxHighlighter::MaterialSyntaxHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter (parent)
{
    LanguageLoader *ll = new LanguageLoader();
    lang = ll->loadFromFile("/usr/share/gtksourceview-3.0/language-specs/cpp.lang");
    delete ll;
}

void MaterialSyntaxHighlighter::highlightBlock(const QString &text)
{
    QTextCharFormat keywordFormat;
    keywordFormat.setFontWeight(QFont::Bold);
    keywordFormat.setForeground(QColor("blue"));
    for (QRegExp kwRegexp : lang->keywords.keys()) {
        if(lang->keywords[kwRegexp] == "keyword") {
            int start = 0;
            while ((start = kwRegexp.indexIn(text, start)) != -1) {
                setFormat(start, kwRegexp.matchedLength(), keywordFormat);
                start += kwRegexp.matchedLength();
            }
        }
    }
}
