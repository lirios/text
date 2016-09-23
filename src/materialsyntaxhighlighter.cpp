#include "materialsyntaxhighlighter.h"
#include "languageloader.h"

MaterialSyntaxHighlighter::MaterialSyntaxHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter (parent),
      lang(nullptr) { }

void MaterialSyntaxHighlighter::setLanguage(LanguageSpecification *l) {
    lang = l;
    rehighlight();
}

void MaterialSyntaxHighlighter::highlightBlock(const QString &text) {
    if(lang == nullptr)
        return;

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
