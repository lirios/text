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

    HighlightData *previousStateData = static_cast<HighlightData *>(currentBlock().previous().userData());
    HighlightData *currentStateData = static_cast<HighlightData *>(currentBlockUserData());
    if(!previousStateData && currentBlock().previous().isValid()) {
        previousStateData = new HighlightData();
        currentBlock().previous().setUserData(previousStateData);
    }
    if(!currentStateData) {
        currentStateData = new HighlightData();
        setCurrentBlockUserData(currentStateData);
    }
    currentStateData->containers = QList<LanguageContextContainer *>();

    int start = 0;
    if(previousStateData) {
        for (int i = 0; i < previousStateData->containers.length(); ++i) {
            auto container = previousStateData->containers[i];

            int len = highlightTillContainerEnd(text.midRef(start), container, currentStateData);
            if(len < 0) {
                if(len == -2)
                    state++;
                start = text.length();
                currentStateData->containers.append(previousStateData->containers.mid(i + 1));
                break;
            } else {
                start += len;
                state++;
            }
        }
    }
    state += highlightPart(text.midRef(start), lang->mainContext->includes, currentStateData);

    setCurrentBlockState(state);
}

int LiriSyntaxHighlighter::highlightTillContainerEnd(const QStringRef &text, LanguageContextContainer *container,
                                                     HighlightData *stateData, QRegularExpressionMatch startMatch) {
    int innerStart = startMatch.hasMatch() ? startMatch.capturedLength() : 0;

    if(container->endPattern == "") {
        setFormat(text.position(), text.length(), containerFormat);
        QStringRef part = text.mid(innerStart);
        highlightPart(part, container->includes);
        return text.length();
    }
    QRegularExpression endRegex = QRegularExpression(container->endPattern);
    QRegularExpressionMatch endMatch = endRegex.match(text, innerStart);

    for (LanguageContext *inc : container->includes) {
        if(inc->type == LanguageContext::SubPattern) {
            LanguageContextSubPattern *subpattern = static_cast<LanguageContextSubPattern *>(inc);
            if(startMatch.hasMatch() && subpattern->where == LanguageContextSubPattern::Start)
                setFormat(text.position() + startMatch.capturedStart(subpattern->group), startMatch.capturedLength(subpattern->group), subPatternFormat);
            if(endMatch.hasMatch() && subpattern->where == LanguageContextSubPattern::End)
                setFormat(text.position() + endMatch.capturedStart(subpattern->group), endMatch.capturedLength(subpattern->group), subPatternFormat);
        }
    }

    if(endMatch.hasMatch()) {
        setFormat(text.position(), endMatch.capturedEnd(), containerFormat);
        QStringRef part = text.mid(innerStart, endMatch.capturedStart() - innerStart);
        highlightPart(part, container->includes);
        return endMatch.capturedEnd();
    } else {
        if(stateData) {
            stateData->containers.insert(0, container);
        }
        setFormat(text.position(), text.length(), containerFormat);
        QStringRef part = text.mid(innerStart);
        bool change = highlightPart(part, container->includes, stateData);
        return -1 - change;
    }
}

bool LiriSyntaxHighlighter::highlightPart(const QStringRef &text, QList<LanguageContext *> currentContainer, HighlightData *stateData) {
    QList<Match> matches;
    for (LanguageContext *context : currentContainer) {
        switch (context->type) {
        case LanguageContext::Keyword: {
            LanguageContextKeyword *kw = static_cast<LanguageContextKeyword *>(context);
            for (QString keyword : kw->keywords) {
                QRegularExpression kwRegexp("\\b(" + keyword + ")\\b");
                QRegularExpressionMatchIterator kwI = kwRegexp.globalMatch(text);
                while (kwI.hasNext()) {
                    QRegularExpressionMatch kwMatch = kwI.next();
                    matches.append({kwMatch, context});
                }
            }
            break;
        }
        case LanguageContext::Simple: {
            LanguageContextSimple *simple = static_cast<LanguageContextSimple *>(context);
            if(simple->matchPattern == "") {
                if(highlightPart(text, simple->includes, stateData))
                    return true;
            } else {
                QRegularExpression matchRegex = QRegularExpression(simple->matchPattern);
                QRegularExpressionMatchIterator matchI = matchRegex.globalMatch(text);
                while (matchI.hasNext()) {
                    QRegularExpressionMatch match = matchI.next();
                    matches.append({match, context});
                }
            }
            break;
        }
        case LanguageContext::Container: {
            LanguageContextContainer *container = static_cast<LanguageContextContainer *>(context);
            QRegularExpression startRegex = QRegularExpression(container->startPattern);
            QRegularExpressionMatchIterator matchI = startRegex.globalMatch(text);
            while (matchI.hasNext()) {
                QRegularExpressionMatch startMatch = matchI.next();
                matches.append({startMatch, context});
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
                setFormat(text.position() + m.match.capturedStart(), m.match.capturedLength(), keywordFormat);
                position = m.match.capturedEnd();
                break;
            case LanguageContext::Simple:
                setFormat(text.position() + m.match.capturedStart(), m.match.capturedLength(), simpleFormat);
                position = m.match.capturedEnd();
                for (LanguageContext *inc : static_cast<LanguageContextSimple *>(m.context)->includes) {
                    if(inc->type == LanguageContext::SubPattern) {
                        LanguageContextSubPattern *subpattern = static_cast<LanguageContextSubPattern *>(inc);
                        setFormat(text.position() + m.match.capturedStart(subpattern->group), m.match.capturedLength(subpattern->group), subPatternFormat);
                    }
                }
                break;
            case LanguageContext::Container:
                LanguageContextContainer *container = static_cast<LanguageContextContainer *>(m.context);
                int start = m.match.capturedStart();
                int len = highlightTillContainerEnd(text.mid(start), container, stateData, m.match);
                if(len < 0) {
                    return true;
                } else {
                    position = start + len;
                }
                break;
            }
        }
    }

    return false;
}
