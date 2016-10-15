#include "lirisyntaxhighlighter.h"
#include "languageloader.h"
#include "languagecontextelementkeyword.h"
#include "languagecontextelementcontainer.h"
#include "languagecontextelementsimple.h"
#include "languagecontextelementsubpattern.h"

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

    QTextCharFormat simpleFormat;
    simpleFormat.setFontWeight(QFont::Cursive);
    simpleFormat.setForeground(QColor("green"));

    QTextCharFormat containerFormat;
    containerFormat.setFontWeight(QFont::Bold);
    containerFormat.setForeground(QColor("brown"));

    QTextCharFormat subPatternFormat;
    subPatternFormat.setFontWeight(QFont::Normal);
    subPatternFormat.setForeground(QColor("red"));

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
        if(ce->type == LanguageContextElement::Simple) {
            LanguageContextElementSimple *simple = static_cast<LanguageContextElementSimple *>(ce);
            int start = 0;
            while ((start = simple->match.indexIn(text, start)) != -1) {
                setFormat(start, simple->match.matchedLength(), simpleFormat);
                start += simple->match.matchedLength();
            }
        }
        if(ce->type == LanguageContextElement::Container) {
            LanguageContextElementContainer *container = static_cast<LanguageContextElementContainer *>(ce);
            int start = 0;
            while ((start = container->start.indexIn(text, start)) != -1) {
                int end = container->end.indexIn(text, start + container->start.matchedLength());
                if(end == -1)
                    end = text.length();
                else
                    end += container->end.matchedLength();
                setFormat(start, end - start, containerFormat);
                start = end;
            }
        }
    }
}
