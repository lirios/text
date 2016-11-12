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
#include "languagecontextelementkeyword.h"
#include "languagecontextelementcontainer.h"
#include "languagecontextelementsimple.h"
#include "languagecontextelementsubpattern.h"

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
                highlightPart(text, container, start, end, nullptr);
                start = end;
            } else {
                bool change = highlightPart(text, container, start, end, nextStateData);
                start = end;
                if(change)
                    state++;
                break;
            }
        }
        bool change = highlightPart(text, lang->mainContext, start, text.length(), nextStateData);
        if(change)
            state++;
    }

    setCurrentBlockState(state);
}

bool LiriSyntaxHighlighter::highlightPart(const QString &text, const LanguageContextElementContainer *currentContainer, int start, int end, HighlightData *nextStateData) {
    bool change = false;

    QList<Match> matches;
    for (LanguageContextElement *ce : currentContainer->includes) {
        switch (ce->type) {
        case LanguageContextElement::Keyword: {
            LanguageContextElementKeyword *kw = static_cast<LanguageContextElementKeyword *>(ce);
            QRegularExpression kwRegexp("\\b(" + kw->keyword + ")\\b");
            QRegularExpressionMatchIterator kwI = kwRegexp.globalMatch(text);
            while (kwI.hasNext()) {
                QRegularExpressionMatch kwMatch = kwI.next();
                if(kwMatch.capturedStart() >= start && kwMatch.capturedEnd() <= end)
                    matches.append({kwMatch, ce});
            }
            break;
        }
        case LanguageContextElement::Simple: {
            LanguageContextElementSimple *simple = static_cast<LanguageContextElementSimple *>(ce);
            QRegularExpression matchRegex = QRegularExpression(simple->matchPattern);
            QRegularExpressionMatchIterator matchI = matchRegex.globalMatch(text);
            while (matchI.hasNext()) {
                QRegularExpressionMatch match = matchI.next();
                if(match.capturedStart() >= start && match.capturedEnd() <= end)
                    matches.append({match, ce});
            }
            break;
        }
        case LanguageContextElement::Container: {
            LanguageContextElementContainer *container = static_cast<LanguageContextElementContainer *>(ce);
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
            case LanguageContextElement::Keyword:
                setFormat(m.match.capturedStart(), m.match.capturedLength(), keywordFormat);
                position = m.match.capturedEnd();
                break;
            case LanguageContextElement::Simple:
                setFormat(m.match.capturedStart(), m.match.capturedLength(), simpleFormat);
                position = m.match.capturedEnd();
                break;
            case LanguageContextElement::Container:
                LanguageContextElementContainer *container = static_cast<LanguageContextElementContainer *>(m.context);
                if(container->endPattern == "") {
                    setFormat(m.match.capturedStart(), end - m.match.capturedStart(), containerFormat);
                    highlightPart(text, container, m.match.capturedEnd(), end, nullptr);
                    position = end;
                    break;
                }
                QRegularExpression endRegex = QRegularExpression(container->endPattern);
                QRegularExpressionMatch endMatch = endRegex.match(text, m.match.capturedEnd());
                if(endMatch.hasMatch()) {
                    setFormat(m.match.capturedStart(), endMatch.capturedEnd() - m.match.capturedStart(), containerFormat);
                    highlightPart(text, container, m.match.capturedEnd(), endMatch.capturedStart(), nullptr);
                    position = endMatch.capturedEnd();
                    break;
                } else {
                    if(nextStateData) {
                        nextStateData->containers.insert(0, container);
                        change = true;
                    }
                    setFormat(m.match.capturedStart(), end - m.match.capturedStart(), containerFormat);
                    highlightPart(text, container, m.match.capturedEnd(), end, nextStateData);
                    position = end;
                    break;
                }
                break;
            }
        }
    }

    return change;
}
