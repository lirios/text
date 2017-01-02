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
#include <QDebug>
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
        previousStateData->containers = QList<HighlightData::ContainerInfo>({ {lang, QRegularExpression(),
                                                                               QList<QSharedPointer<LanguageContext>>()} });
    }

    HighlightData *currentStateData = static_cast<HighlightData *>(currentBlockUserData());
    if(!currentStateData) {
        currentStateData = new HighlightData();
        setCurrentBlockUserData(currentStateData);
    }
    currentStateData->containers = QList<HighlightData::ContainerInfo>();

    int start = 0;
    for (int i = 0; i < previousStateData->containers.length(); ++i) {
        auto containerInfo = previousStateData->containers[i];

        int len = highlightTillContainerEnd(text, start, containerInfo, currentStateData);
        start += len;
        if(start <= text.length()) {
            while(containerInfo.container->endParent) {
                i++;
                containerInfo = previousStateData->containers[i];
            }
        }
    }

    setCurrentBlockState(qHash(currentStateData->containers));
}

int LiriSyntaxHighlighter::highlightTillContainerEnd(const QString &text, int offset, HighlightData::ContainerInfo containerInfo,
                                                     HighlightData *stateData, int startLength) {
    QSharedPointer<LanguageContextContainer> container = containerInfo.container;
    HighlightData::ContainerInfo newContainerInfo = containerInfo;
    int innerStart = offset + startLength;
    int end;
    // Highlight whole received text
    if(container->style)
        setFormat(container->styleInside ? innerStart : offset, text.length(), defStyles->styles[container->style->defaultId]);
    QRegularExpressionMatch endMatch = highlightPart(end, text, innerStart, newContainerInfo, stateData);
    // Clear style of block ending
    int highlightEnd = endMatch.hasMatch() ? (container->styleInside ? endMatch.capturedStart() : endMatch.capturedEnd()) : end;
    if(container->style)
        setFormat(highlightEnd, text.length(), QTextCharFormat());

    if(end > text.length()) {
        stateData->containers.append(newContainerInfo);
    }

    if(endMatch.hasMatch()) {
        for (ContextDPtr inc : container->includes) {
            if(inc->data()->type == LanguageContext::SubPattern) {
                QSharedPointer<LanguageContextSubPattern> subpattern = inc->staticCast<LanguageContextSubPattern>();
                if(subpattern->where == LanguageContextSubPattern::End && endMatch.hasMatch()) {
                    int endStart = subpattern->groupName.isNull() ? endMatch.capturedStart(subpattern->groupId) :
                                                                    endMatch.capturedStart(subpattern->groupName);
                    if(endStart >= 0) {
                        int endLen = subpattern->groupName.isNull() ? endMatch.capturedLength(subpattern->groupId) :
                                                                      endMatch.capturedLength(subpattern->groupName);
                        if(subpattern->style)
                            setFormat(offset + endStart,
                                      endLen, defStyles->styles[subpattern->style->defaultId]);
                    }
                }
            }
        }
    }

    return end - offset;
}

QRegularExpressionMatch LiriSyntaxHighlighter::highlightPart(int &end, const QString &text, int offset,
                                                             HighlightData::ContainerInfo &currentContainerInfo,
                                                             HighlightData *stateData) {
    QList<Match> matches;
    QRegularExpressionMatchIterator containerEndIter;
    if(currentContainerInfo.endRegex.pattern() != "")
        containerEndIter = currentContainerInfo.endRegex.globalMatch(text, offset);
    QList<ContextDPtr> extendedContainer = currentContainerInfo.container->includes;
    for (int i = 0; i < extendedContainer.length(); ++i) {
        QSharedPointer<LanguageContext> context = *extendedContainer[i].data();
        if(!context)
            continue;

        switch (context->type) {
        case LanguageContext::Keyword: {
            QSharedPointer<LanguageContextKeyword> kw = context.staticCast<LanguageContextKeyword>();

            if(kw->firstLineOnly && currentBlock().position() != 0)
                break;
            if(currentContainerInfo.forbiddenContexts.contains(context))
                break;

            QRegularExpression keyword = kw->keyword;
            QRegularExpressionMatchIterator kwI = keyword.globalMatch(text, offset);
            while (kwI.hasNext()) {
                QRegularExpressionMatch kwMatch = kwI.next();
                matches.append({kwMatch, context});
            }
            break;
        }
        case LanguageContext::Simple: {
            QSharedPointer<LanguageContextSimple> simple = context.staticCast<LanguageContextSimple>();

            if(simple->firstLineOnly && currentBlock().position() != 0)
                break;
            if(currentContainerInfo.forbiddenContexts.contains(context))
                break;

            QRegularExpressionMatchIterator matchI = simple->match.globalMatch(text, offset);
            while (matchI.hasNext()) {
                QRegularExpressionMatch match = matchI.next();
                matches.append({match, context});
            }
            break;
        }
        case LanguageContext::Container: {
            QSharedPointer<LanguageContextContainer> container = context.staticCast<LanguageContextContainer>();

            if(container->firstLineOnly && currentBlock().position() != 0)
                break;
            if(currentContainerInfo.forbiddenContexts.contains(context))
                break;

            if(container->start.pattern() == "") {
                extendedContainer.append(container->includes);
            } else {
                if(!container->start.isValid())
                    qDebug() << "Regular expression error during highlighting:" << container->start.errorString();
                QRegularExpressionMatchIterator matchI = container->start.globalMatch(text, offset);
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
    else if(currentContainerInfo.container->endAtLineEnd)
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
                if(kw->onceOnly) {
                    if(currentContainerInfo.forbiddenContexts.contains(kw))
                        continue;
                    else
                        currentContainerInfo.forbiddenContexts.append(kw);
                }

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
                if(simple->onceOnly) {
                    if(currentContainerInfo.forbiddenContexts.contains(simple))
                        continue;
                    else
                        currentContainerInfo.forbiddenContexts.append(simple);
                }

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
                if(container->onceOnly) {
                    if(currentContainerInfo.forbiddenContexts.contains(container))
                        continue;
                    else
                        currentContainerInfo.forbiddenContexts.append(container);
                }

                int start = m.match.capturedStart();

                // Resolve references to start subpatterns from end regex
                QRegularExpression endRegex = container->end;
                QString endPattern = endRegex.pattern();
                QRegularExpression startRefRegex = QRegularExpression("\\\\%{(.+)@start}");
                QRegularExpressionMatch startRefMatch;
                while((startRefMatch = startRefRegex.match(endPattern)).hasMatch()) {
                    QString groupName = startRefMatch.captured(1);
                    bool isId;
                    int id = groupName.toInt(&isId);
                    endPattern.replace(startRefMatch.capturedStart(), startRefMatch.capturedLength(),
                                       QRegularExpression::escape(isId ? m.match.captured(id) : m.match.captured(groupName)));
                }
                if(endRegex.pattern() != endPattern) // Don't make regex dirty if there were no changes
                    endRegex.setPattern(endPattern);

                int len = highlightTillContainerEnd(text, start, {container, endRegex, {}}, stateData, m.match.capturedLength());
                end = start + len;

                // Highlight start subpatterns
                for (ContextDPtr inc : container->includes) {
                    if(inc->data()->type == LanguageContext::SubPattern) {
                        QSharedPointer<LanguageContextSubPattern> subpattern = inc->staticCast<LanguageContextSubPattern>();
                        if(subpattern->where == LanguageContextSubPattern::Start) {
                            int startStart = subpattern->groupName.isNull() ? m.match.capturedStart(subpattern->groupId) :
                                                                              m.match.capturedStart(subpattern->groupName);
                            if(startStart >= 0) {
                                int startLen = subpattern->groupName.isNull() ? m.match.capturedLength(subpattern->groupId) :
                                                                                m.match.capturedLength(subpattern->groupName);
                                if(subpattern->style)
                                    setFormat(offset + startStart,
                                              startLen, defStyles->styles[subpattern->style->defaultId]);
                            }
                        }
                    }
                }

                if(container->endParent)
                    return QRegularExpressionMatch();
                break;
            }
            }

            // If we went past container ending, try to find next one
            while(containerEndMatch.hasMatch() && containerEndMatch.capturedStart() < end) {
                if(containerEndIter.hasNext())
                    containerEndMatch = containerEndIter.next();
                else if(currentContainerInfo.container->endAtLineEnd && end < text.length())
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
