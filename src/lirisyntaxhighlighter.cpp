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
    HighlightData *currentStateData = static_cast<HighlightData *>(currentBlockUserData());

    if(currentBlock().previous().isValid())
        previousStateData = static_cast<HighlightData *>(currentBlock().previous().userData());
    if(!previousStateData) {
        previousStateData = new HighlightData();
        previousStateData->containers = QList<QSharedPointer<LanguageContextContainer>>({lang});
    }

    if(!currentStateData) {
        currentStateData = new HighlightData();
        setCurrentBlockUserData(currentStateData);
    }
    currentStateData->containers = QList<QSharedPointer<LanguageContextContainer>>();

    int start = 0;
    for (int i = 0; i < previousStateData->containers.length(); ++i) {
        auto container = previousStateData->containers[i];

        int len = highlightTillContainerEnd(text.midRef(start), container, currentStateData);
        start += len;
    }

    setCurrentBlockState(qHash(currentStateData->containers));
}

int LiriSyntaxHighlighter::highlightTillContainerEnd(const QStringRef &text, QSharedPointer<LanguageContextContainer> container,
                                                     HighlightData *stateData, QRegularExpressionMatch startMatch) {
    int innerStart = startMatch.hasMatch() ? startMatch.capturedLength() : 0;
    int len;
    // Highlight whole received text
    if(container->style)
        setFormat(text.position(), text.length(), defStyles->styles[container->style->defaultId]);
    QStringRef part = text.mid(innerStart);
    QRegularExpressionMatch endMatch = highlightPart(len, part, container, stateData);
    len += innerStart;
    // Clear style of block ending
    if(container->style)
        setFormat(text.position() + len, text.length() - len, QTextCharFormat());

    if(!endMatch.hasMatch())
        stateData->containers.append(container);

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
                            setFormat(text.position() + startStart - startMatch.capturedStart(),
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
                            setFormat(text.position() + endStart - startMatch.capturedStart(),
                                      endLen, defStyles->styles[subpattern->style->defaultId]);
                    }
                }
            }
        }
    }

    return len;
}

QRegularExpressionMatch LiriSyntaxHighlighter::highlightPart(int &length, const QStringRef &text,
                                                             QSharedPointer<LanguageContextContainer> currentContainer,
                                                             HighlightData *stateData) {
    QList<Match> matches;
    QRegularExpressionMatchIterator containerEndIter;
    if(currentContainer->endPattern != "")
        containerEndIter = QRegularExpression(currentContainer->endPattern).globalMatch(text);
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
                QRegularExpressionMatchIterator kwI = kwRegexp.globalMatch(text);
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
            QRegularExpressionMatchIterator matchI = matchRegex.globalMatch(text);
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
                QRegularExpressionMatchIterator matchI = startRegex.globalMatch(text);
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
        containerEndMatch = QRegularExpression("$").match(text);

    length = 0;
    for (auto m : matches) {
        if(m.match.capturedStart() >= length) {
            if(containerEndMatch.hasMatch() && containerEndMatch.capturedStart() < m.match.capturedStart())
                break;
            switch (m.context->type) {
            case LanguageContext::Keyword: {
                QSharedPointer<LanguageContextKeyword> kw = m.context.staticCast<LanguageContextKeyword>();
                if(containerEndMatch.hasMatch() && containerEndMatch.capturedStart() < m.match.capturedEnd()) {
                    if(kw->extendParent) {
                        if(containerEndIter.hasNext())
                            containerEndMatch = containerEndIter.next();
                        else if(currentContainer->endAtLineEnd)
                            containerEndMatch = QRegularExpression("$").match(text);
                        else
                            containerEndMatch = QRegularExpressionMatch();
                    } else
                        break; // Move to next match, maybe it has extendParent attribute
                }

                if(m.context->style)
                    setFormat(text.position() + m.match.capturedStart(), m.match.capturedLength(),
                              defStyles->styles[m.context->style->defaultId]);
                length = m.match.capturedEnd();
                if(kw->endParent)
                    return QRegularExpressionMatch();
                break;
            }
            case LanguageContext::Simple: {
                QSharedPointer<LanguageContextSimple> simple = m.context.staticCast<LanguageContextSimple>();
                if(containerEndMatch.hasMatch() && containerEndMatch.capturedStart() < m.match.capturedEnd()) {
                    if(simple->extendParent) {
                        if(containerEndIter.hasNext())
                            containerEndMatch = containerEndIter.next();
                        else if(currentContainer->endAtLineEnd)
                            containerEndMatch = QRegularExpression("$").match(text);
                        else
                            containerEndMatch = QRegularExpressionMatch();
                    } else
                        break; // Move to next match, maybe it has extendParent attribute
                }

                if(m.context->style)
                    setFormat(text.position() + m.match.capturedStart(), m.match.capturedLength(),
                              defStyles->styles[m.context->style->defaultId]);
                length = m.match.capturedEnd();
                for (ContextDPtr inc : simple->includes) {
                    if(inc->data()->type == LanguageContext::SubPattern) {
                        QSharedPointer<LanguageContextSubPattern> subpattern = inc->staticCast<LanguageContextSubPattern>();
                        int mStart = subpattern->groupName.isNull() ? m.match.capturedStart(subpattern->groupId) :
                                                                      m.match.capturedStart(subpattern->groupName);
                        int mLen = subpattern->groupName.isNull() ? m.match.capturedLength(subpattern->groupId) :
                                                                    m.match.capturedLength(subpattern->groupName);
                        setFormat(text.position() + mStart,
                                  mLen, defStyles->styles[subpattern->style->defaultId]);
                    }
                }
                if(simple->endParent)
                    return QRegularExpressionMatch();
                break;
            }
            case LanguageContext::Container: {
                QSharedPointer<LanguageContextContainer> container = m.context.staticCast<LanguageContextContainer>();

                if(containerEndMatch.hasMatch() && containerEndMatch.capturedStart() < m.match.capturedEnd()) {
                    if(container->extendParent) {
                        if(containerEndIter.hasNext())
                            containerEndMatch = containerEndIter.next();
                        else if(currentContainer->endAtLineEnd)
                            containerEndMatch = QRegularExpression("$").match(text);
                        else
                            containerEndMatch = QRegularExpressionMatch();
                    } else
                        break; // Move to next match, maybe it has extendParent attribute
                }

                int start = m.match.capturedStart();
                int len = highlightTillContainerEnd(text.mid(start), container, stateData, m.match);
                length = start + len;
                if(container->endParent)
                    return QRegularExpressionMatch();
                break;
            }
            }
        }
    }

    if(containerEndMatch.hasMatch())
        length = containerEndMatch.capturedEnd();
    else
        length = text.length();

    return containerEndMatch;
}

bool LiriSyntaxHighlighter::Match::operator <(const LiriSyntaxHighlighter::Match &other) {
    if(this->match.capturedStart() == other.match.capturedStart())
        return this->match.capturedEnd() > other.match.capturedEnd();
    else
        return this->match.capturedStart() < other.match.capturedStart();
}
