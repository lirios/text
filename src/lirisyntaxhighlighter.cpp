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
      lang() { }

LiriSyntaxHighlighter::~LiriSyntaxHighlighter() { }

void LiriSyntaxHighlighter::setLanguage(QSharedPointer<LanguageContextContainer> l) {
    lang = l;
    if(defStyles)
        rehighlight();
}

void LiriSyntaxHighlighter::setDefaultStyle(QSharedPointer<LanguageDefaultStyles> def) {
    defStyles = def;
    if(lang)
        rehighlight();
}

void LiriSyntaxHighlighter::highlightBlock(const QString &text) {
    if(lang == nullptr)
        return;

    HighlightData *previousStateData = nullptr;
    if(currentBlock().previous().isValid())
        previousStateData = static_cast<HighlightData *>(currentBlock().previous().userData());
    if(!previousStateData) {
        previousStateData = new HighlightData();
        previousStateData->containers = QList<QSharedPointer<LanguageContextContainer>>({lang});
        previousStateData->startMatches = QList<QRegularExpressionMatch>({QRegularExpressionMatch()});
    }

    HighlightData *currentStateData = static_cast<HighlightData *>(currentBlockUserData());
    if(!currentStateData) {
        currentStateData = new HighlightData();
        setCurrentBlockUserData(currentStateData);
    }
    currentStateData->containers = QList<QSharedPointer<LanguageContextContainer>>();
    currentStateData->startMatches = QList<QRegularExpressionMatch>();

    int start = 0;
    for (int i = 0; i < previousStateData->containers.length(); ++i) {
        auto container = previousStateData->containers[i];

        int len = highlightTillContainerEnd(text, start, container, currentStateData, previousStateData->startMatches[i], false);
        start += len;
        if(start <= text.length()) {
            while(container->endParent) {
                i++;
                container = previousStateData->containers[i];
            }
        }
    }

    setCurrentBlockState(qHash(currentStateData->containers));
}

int LiriSyntaxHighlighter::highlightTillContainerEnd(const QString &text, int offset, QSharedPointer<LanguageContextContainer> container,
                                                     HighlightData *stateData, QRegularExpressionMatch startMatch, bool startAtCurrentBlock) {
    int innerStart = (startAtCurrentBlock && startMatch.hasMatch()) ? startMatch.capturedEnd() : offset;
    int end;
    // Highlight whole received text
    if(container->style)
        setFormat(offset, text.length() - offset, defStyles->styles[container->style->defaultId]);
    QString endPattern = container->endPattern;
    if(startMatch.hasMatch()) {
        QRegularExpression startRefRegex = QRegularExpression("\\\\%{(.+)@start}");
        QRegularExpressionMatch startRefMatch;
        while((startRefMatch = startRefRegex.match(endPattern)).hasMatch()) {
            QString groupName = startRefMatch.captured(1);
            bool isId;
            int id = groupName.toInt(&isId);
            endPattern.replace(startRefMatch.capturedStart(), startRefMatch.capturedLength(),
                               QRegularExpression::escape(isId ? startMatch.captured(id) : startMatch.captured(groupName)));
        }
    }
    QRegularExpression endRegex = QRegularExpression(endPattern);
    QRegularExpressionMatch endMatch = highlightPart(end, text, innerStart, container, endRegex, stateData);
    // Clear style of block ending
    if(container->style)
        setFormat(end, text.length() - end, QTextCharFormat());

    if(end > text.length()) {
        stateData->containers.append(container);
        stateData->startMatches.append(startMatch);
    }

    if(startMatch.hasMatch() || endMatch.hasMatch()) {
        for (ContextDPtr inc : container->includes) {
            if(inc->data()->type == LanguageContext::SubPattern) {
                QSharedPointer<LanguageContextSubPattern> subpattern = inc->staticCast<LanguageContextSubPattern>();
                if(subpattern->where == LanguageContextSubPattern::Start && startMatch.hasMatch()) {
                    int startStart = subpattern->groupName.isNull() ? startMatch.capturedStart(subpattern->groupId) :
                                                                      startMatch.capturedStart(subpattern->groupName);
                    if(startStart >= 0) {
                        int startLen = subpattern->groupName.isNull() ? startMatch.capturedLength(subpattern->groupId) :
                                                                        startMatch.capturedLength(subpattern->groupName);
                        if(subpattern->style)
                            setFormat(offset + startStart - startMatch.capturedStart(),
                                      startLen, defStyles->styles[subpattern->style->defaultId]);
                    }
                }

                if(subpattern->where == LanguageContextSubPattern::End && endMatch.hasMatch()) {
                    int endStart = subpattern->groupName.isNull() ? endMatch.capturedStart(subpattern->groupId) :
                                                                    endMatch.capturedStart(subpattern->groupName);
                    if(endStart >= 0) {
                        int endLen = subpattern->groupName.isNull() ? endMatch.capturedLength(subpattern->groupId) :
                                                                      endMatch.capturedLength(subpattern->groupName);
                        if(subpattern->style)
                            setFormat(offset + endStart - startMatch.capturedStart(),
                                      endLen, defStyles->styles[subpattern->style->defaultId]);
                    }
                }
            }
        }
    }

    return end - offset;
}

QRegularExpressionMatch LiriSyntaxHighlighter::highlightPart(int &end, const QString &text, int offset,
                                                             QSharedPointer<LanguageContextContainer> currentContainer,
                                                             QRegularExpression containerEndRegex, HighlightData *stateData) {
    QList<Match> matches;
    QRegularExpressionMatchIterator containerEndIter;
    if(currentContainer->endPattern != "")
        containerEndIter = containerEndRegex.globalMatch(text, offset);
    QList<ContextDPtr> extendedContainer = currentContainer->includes;
    for (int i = 0; i < extendedContainer.length(); ++i) {
        QSharedPointer<LanguageContext> context = *extendedContainer[i].data();
        if(!context)
            continue;

        switch (context->type) {
        case LanguageContext::Keyword: {
            QSharedPointer<LanguageContextKeyword> kw = context.staticCast<LanguageContextKeyword>();
            for (QString keyword : kw->keywords) {
                QRegularExpression kwRegexp("\\b(" + keyword + ")\\b");
                QRegularExpressionMatchIterator kwI = kwRegexp.globalMatch(text, offset);
                while (kwI.hasNext()) {
                    QRegularExpressionMatch kwMatch = kwI.next();
                    matches.append({kwMatch, context});
                }
            }
            break;
        }
        case LanguageContext::Simple: {
            QSharedPointer<LanguageContextSimple> simple = context.staticCast<LanguageContextSimple>();
            QRegularExpression matchRegex = QRegularExpression(simple->matchPattern);
            QRegularExpressionMatchIterator matchI = matchRegex.globalMatch(text, offset);
            while (matchI.hasNext()) {
                QRegularExpressionMatch match = matchI.next();
                matches.append({match, context});
            }
            break;
        }
        case LanguageContext::Container: {
            QSharedPointer<LanguageContextContainer> container = context.staticCast<LanguageContextContainer>();
            if(container->startPattern == "") {
                extendedContainer.append(container->includes);
            } else {
                QRegularExpression startRegex = QRegularExpression(container->startPattern);
                QRegularExpressionMatchIterator matchI = startRegex.globalMatch(text, offset);
                while (matchI.hasNext()) {
                    QRegularExpressionMatch startMatch = matchI.next();
                    matches.append({startMatch, context});
                }
            }
            break;
        }
        }
    }
    std::sort(matches.begin(), matches.end());

    QRegularExpressionMatch containerEndMatch;
    if(containerEndIter.hasNext())
        containerEndMatch = containerEndIter.next();
    else if(currentContainer->endAtLineEnd)
        containerEndMatch = QRegularExpression("$").match(text, offset);

    end = offset;
    for (auto m : matches) {
        if(m.match.capturedStart() >= end) {
            if(containerEndMatch.hasMatch() && containerEndMatch.capturedStart() < m.match.capturedStart())
                break;
            switch (m.context->type) {
            case LanguageContext::Keyword: {
                QSharedPointer<LanguageContextKeyword> kw = m.context.staticCast<LanguageContextKeyword>();

                if(!kw->extendParent && containerEndMatch.hasMatch() && containerEndMatch.capturedStart() < m.match.capturedEnd())
                    continue;

                if(m.context->style)
                    setFormat(m.match.capturedStart(), m.match.capturedLength(),
                              defStyles->styles[m.context->style->defaultId]);
                end = m.match.capturedEnd();
                if(kw->endParent)
                    return QRegularExpressionMatch();
                break;
            }
            case LanguageContext::Simple: {
                QSharedPointer<LanguageContextSimple> simple = m.context.staticCast<LanguageContextSimple>();

                if(!simple->extendParent && containerEndMatch.hasMatch() && containerEndMatch.capturedStart() < m.match.capturedEnd())
                    continue;

                if(m.context->style)
                    setFormat(m.match.capturedStart(), m.match.capturedLength(),
                              defStyles->styles[m.context->style->defaultId]);
                end = m.match.capturedEnd();
                for (ContextDPtr inc : simple->includes) {
                    if(inc->data()->type == LanguageContext::SubPattern) {
                        QSharedPointer<LanguageContextSubPattern> subpattern = inc->staticCast<LanguageContextSubPattern>();
                        int mStart = subpattern->groupName.isNull() ? m.match.capturedStart(subpattern->groupId) :
                                                                      m.match.capturedStart(subpattern->groupName);
                        int mLen = subpattern->groupName.isNull() ? m.match.capturedLength(subpattern->groupId) :
                                                                    m.match.capturedLength(subpattern->groupName);
                        setFormat(mStart,
                                  mLen, defStyles->styles[subpattern->style->defaultId]);
                    }
                }
                if(simple->endParent)
                    return QRegularExpressionMatch();
                break;
            }
            case LanguageContext::Container: {
                QSharedPointer<LanguageContextContainer> container = m.context.staticCast<LanguageContextContainer>();

                if(!container->extendParent && containerEndMatch.hasMatch() && containerEndMatch.capturedStart() < m.match.capturedEnd())
                    continue;

                int start = m.match.capturedStart();
                int len = highlightTillContainerEnd(text, start, container, stateData, m.match, true);
                end = start + len;

                if(container->endParent)
                    return QRegularExpressionMatch();
                break;
            }
            }

            // If we went past container ending, try to find next one
            while(containerEndMatch.hasMatch() && containerEndMatch.capturedStart() < end) {
                if(containerEndIter.hasNext())
                    containerEndMatch = containerEndIter.next();
                else if(currentContainer->endAtLineEnd && end < text.length())
                    containerEndMatch = QRegularExpression("$").match(text, offset);
                else
                    containerEndMatch = QRegularExpressionMatch();
            }
        }
    }

    if(containerEndMatch.hasMatch())
        end = containerEndMatch.capturedEnd();
    else
        end = text.length() + 1;

    return containerEndMatch;
}

bool LiriSyntaxHighlighter::Match::operator <(const LiriSyntaxHighlighter::Match &other) {
    if(this->match.capturedStart() == other.match.capturedStart())
        return this->match.capturedEnd() > other.match.capturedEnd();
    else
        return this->match.capturedStart() < other.match.capturedStart();
}
