/*
 * Copyright © 2016 Andrew Penkrat
 *
 * This file is part of Liri Text.
 *
 * Liri Text is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Liri Text is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Liri Text.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QTextDocument>
#include <QRegularExpression>
#include "lirisyntaxhighlighter.h"
#include "languageloader.h"
#include "languagecontextkeyword.h"
#include "languagecontextcontainer.h"
#include "languagecontextsimple.h"
#include "languagecontextsubpattern.h"

LiriSyntaxHighlighter::LiriSyntaxHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter (parent),
      lang(nullptr) {

    keywordFormat.setFontWeight(QFont::Bold);
    keywordFormat.setForeground(QColor("blue"));

    simpleFormat.setFontWeight(QFont::Cursive);
    simpleFormat.setForeground(QColor("green"));

    containerFormat.setFontWeight(QFont::Bold);
    containerFormat.setForeground(QColor("brown"));

    subPatternFormat.setFontWeight(QFont::Normal);
    subPatternFormat.setForeground(QColor("red"));
}

void LiriSyntaxHighlighter::setLanguage(LanguageSpecification *l) {
    lang = l;
    rehighlight();
}

void LiriSyntaxHighlighter::highlightBlock(const QString &text) {
    if(lang == nullptr)
        return;

    int state = previousBlockState();

    HighlightData *currentStateData = static_cast<HighlightData *>(currentBlockUserData());
    HighlightData *nextStateData = static_cast<HighlightData *>(currentBlock().next().userData());
    if(!currentStateData) {
        currentStateData = new HighlightData();
        setCurrentBlockUserData(currentStateData);
    }
    if(!nextStateData) {
        // FIXME: properly handle last block without memory allocation on every rehighlight
        nextStateData = new HighlightData();
        if(currentBlock().next().isValid())
            currentBlock().next().setUserData(nextStateData);
    }
    nextStateData->containers = currentStateData->containers;

    {
        int start = 0;
        for (auto container : currentStateData->containers) {
            int end;
            QRegularExpressionMatch endMatch = QRegularExpression(container->endPattern).match(text, start);
            if(container->endPattern == "")
                end = text.length();
            else
                end = endMatch.capturedEnd();

            QTextBlock nextBlock = currentBlock().next();
            if(nextBlock.isValid()) {
                if(end == -1) {
                    end = text.length();
                } else {
                    if(nextStateData->containers.contains(container)) {
                        nextStateData->containers.removeOne(container);
                    }
                    state++;
                }
            }

            setFormat(start, end - start, containerFormat);
            if(endMatch.hasMatch()) {
                highlightPart(text, container->includes, start, end, nullptr);
                start = end;
            } else {
                bool change = highlightPart(text, container->includes, start, end, nextStateData);
                start = end;
                if(change)
                    state++;
                break;
            }
        }
        bool change = highlightPart(text, lang->mainContext->includes, start, text.length(), nextStateData);
        if(change)
            state++;
    }

    setCurrentBlockState(state);
}

bool LiriSyntaxHighlighter::highlightPart(const QString &text, QList<LanguageContext *> currentContainer, int start, int end, HighlightData *nextStateData) {
    bool change = false;

    QList<Match> matches;
    for (LanguageContext *ce : currentContainer) {
        switch (ce->type) {
        case LanguageContext::Keyword: {
            LanguageContextKeyword *kw = static_cast<LanguageContextKeyword *>(ce);
            for (QString keyword : kw->keywords) {
                QRegularExpression kwRegexp("\\b(" + keyword + ")\\b");
                QRegularExpressionMatchIterator kwI = kwRegexp.globalMatch(text);
                while (kwI.hasNext()) {
                    QRegularExpressionMatch kwMatch = kwI.next();
                    if(kwMatch.capturedStart() >= start && kwMatch.capturedEnd() <= end)
                        matches.append({kwMatch, ce});
                }
            }
            break;
        }
        case LanguageContext::Simple: {
            LanguageContextSimple *simple = static_cast<LanguageContextSimple *>(ce);
            if(simple->matchPattern == "")
                change |= highlightPart(text, simple->includes, start, end, nextStateData);
            else {
                QRegularExpression matchRegex = QRegularExpression(simple->matchPattern);
                QRegularExpressionMatchIterator matchI = matchRegex.globalMatch(text);
                while (matchI.hasNext()) {
                    QRegularExpressionMatch match = matchI.next();
                    if(match.capturedStart() >= start && match.capturedEnd() <= end)
                        matches.append({match, ce});
                }
            }
            break;
        }
        case LanguageContext::Container: {
            LanguageContextContainer *container = static_cast<LanguageContextContainer *>(ce);
            QRegularExpression startRegex = QRegularExpression(container->startPattern);
            QRegularExpressionMatchIterator matchI = startRegex.globalMatch(text);
            while (matchI.hasNext()) {
                QRegularExpressionMatch startMatch = matchI.next();
                if(startMatch.capturedStart() >= start && startMatch.capturedEnd() <= end)
                    matches.append({startMatch, ce});
            }
            break;
        }
        }
    }
    std::sort(matches.begin(), matches.end());

    int position = 0;
    for (auto m : matches) {
        if(m.match.capturedStart() >= position) {
            switch (m.context->type) {
            case LanguageContext::Keyword:
                setFormat(m.match.capturedStart(), m.match.capturedLength(), keywordFormat);
                position = m.match.capturedEnd();
                break;
            case LanguageContext::Simple:
                setFormat(m.match.capturedStart(), m.match.capturedLength(), simpleFormat);
                position = m.match.capturedEnd();
                break;
            case LanguageContext::Container:
                LanguageContextContainer *container = static_cast<LanguageContextContainer *>(m.context);
                if(container->endPattern == "") {
                    setFormat(m.match.capturedStart(), end - m.match.capturedStart(), containerFormat);
                    highlightPart(text, container->includes, m.match.capturedEnd(), end, nullptr);
                    position = end;
                    break;
                }
                QRegularExpression endRegex = QRegularExpression(container->endPattern);
                QRegularExpressionMatch endMatch = endRegex.match(text, m.match.capturedEnd());
                if(endMatch.hasMatch()) {
                    setFormat(m.match.capturedStart(), endMatch.capturedEnd() - m.match.capturedStart(), containerFormat);
                    highlightPart(text, container->includes, m.match.capturedEnd(), endMatch.capturedStart(), nullptr);
                    position = endMatch.capturedEnd();
                    break;
                } else {
                    if(nextStateData) {
                        nextStateData->containers.insert(0, container);
                        change = true;
                    }
                    setFormat(m.match.capturedStart(), end - m.match.capturedStart(), containerFormat);
                    highlightPart(text, container->includes, m.match.capturedEnd(), end, nextStateData);
                    position = end;
                    break;
                }
                break;
            }
        }
    }

    return change;
}
