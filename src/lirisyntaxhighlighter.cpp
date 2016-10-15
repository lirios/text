#include "lirisyntaxhighlighter.h"
#include "languageloader.h"
#include "languagecontextelementkeyword.h"

LiriSyntaxHighlighter::LiriSyntaxHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter (parent),
      lang(nullptr) { }

void LiriSyntaxHighlighter::setLanguage(LanguageSpecification *l) {
    lang = l;
    rehighlight();
}

void LiriSyntaxHighlighter::highlightBlock(const QString &text) {
    if(lang == nullptr)
        return;

    QTextCharFormat keywordFormat;
    keywordFormat.setFontWeight(QFont::Bold);
    keywordFormat.setForeground(QColor("blue"));
    QList<LanguageContextElement *> root = lang->mainContext;
    for (LanguageContextElement *ce : root) {
        if(ce->type == LanguageContextElement::Keyword) {
            LanguageContextElementKeyword *kw = static_cast<LanguageContextElementKeyword *>(ce);
            QRegExp kwRegexp("\\b(" + kw->keyword + ")\\b");
            int start = 0;
            while ((start = kwRegexp.indexIn(text, start)) != -1) {
                setFormat(start, kwRegexp.matchedLength(), keywordFormat);
                start += kwRegexp.matchedLength();
            }
        }
    }
}
