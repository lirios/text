#include <QTextDocument>
#include "lirisyntaxhighlighter.h"
#include "languageloader.h"
#include "languagecontextelementkeyword.h"
#include "languagecontextelementcontainer.h"
#include "languagecontextelementsimple.h"
#include "languagecontextelementsubpattern.h"
#include "highlightdata.h"

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

    int state = previousBlockState();

    HighlightData *currentStateData = static_cast<HighlightData *>(currentBlockUserData());
    HighlightData *nextStateData = static_cast<HighlightData *>(currentBlock().next().userData());
    if(!currentStateData) {
        currentStateData = new HighlightData();
        setCurrentBlockUserData(currentStateData);
    }
    if(!nextStateData) {
        nextStateData = new HighlightData();
        if(currentBlock().next().isValid())
            currentBlock().next().setUserData(nextStateData);
    }
    nextStateData->containers = currentStateData->containers;

    if(currentStateData) {
        for (int i = 0; i < currentStateData->containers.length(); ++i) {
            auto container = currentStateData->containers[i];
            int start = 0;
            int end;
            QRegExp endRegex = QRegExp(container->endPattern);
            if(container->endPattern == "")
                end = -1;
            else
                end = endRegex.indexIn(text, start);

            QTextBlock nextBlock = currentBlock().next();
            if(nextBlock.isValid()) {
                if(end == -1) {
                    end = text.length();
                } else {
                    end += endRegex.matchedLength();
                    if(nextStateData->containers.contains(container)) {
                        nextStateData->containers.removeOne(container);
                    }
                    state--;
                }
            }

            setFormat(start, end - start, containerFormat);
            start = end;
        }
    }

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
            QRegExp matchRegex = QRegExp(simple->matchPattern);
            int start = 0;
            while ((start = matchRegex.indexIn(text, start)) != -1) {
                setFormat(start, matchRegex.matchedLength(), simpleFormat);
                start += matchRegex.matchedLength();
            }
        }
        if(ce->type == LanguageContextElement::Container) {
            LanguageContextElementContainer *container = static_cast<LanguageContextElementContainer *>(ce);
            QRegExp startRegex = QRegExp(container->startPattern);
            QRegExp endRegex = QRegExp(container->endPattern);
            int start = 0;
            while ((start = startRegex.indexIn(text, start)) != -1) {
                int end;
                if(container->endPattern == "")
                    end = -1;
                else
                    end = endRegex.indexIn(text, start + startRegex.matchedLength());

                if(end == -1) {
                    end = text.length();
                    if(container->endPattern != "") {
                        nextStateData->containers.insert(0, container);
                        QTextBlock nextBlock = currentBlock().next();
                        if(nextBlock.isValid()) {
                            state++;
                        }
                    }
                }
                else
                    end += endRegex.matchedLength();

                setFormat(start, end - start, containerFormat);
                start = end;
            }
        }
    }
    setCurrentBlockState(state);
}
