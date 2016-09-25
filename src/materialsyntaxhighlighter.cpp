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
    LanguageContext *root = lang->getMainContext();
    for (QString kw : root->keywords.keys()) {
        if(root->keywords[kw] == "keyword") {
            QRegExp kwRegexp("\\b(" + kw + ")\\b");
            int start = 0;
            while ((start = kwRegexp.indexIn(text, start)) != -1) {
                setFormat(start, kwRegexp.matchedLength(), keywordFormat);
                start += kwRegexp.matchedLength();
            }
        }
    }
}
