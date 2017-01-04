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

LiriSyntaxHighlighter::~LiriSyntaxHighlighter() {
    if(lang)
        lang->~LanguageContextReference();
}

void LiriSyntaxHighlighter::setLanguage(QSharedPointer<LanguageContextReference> l) {
    if(lang)
        lang->~LanguageContextReference();
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

    QList<HighlightData::ContainerInfo> previousBlockContainers;
    if(currentBlock().previous().isValid())
        previousBlockContainers = static_cast<HighlightData *>(currentBlock().previous().userData())->containers;
    else {
        previousBlockContainers = QList<HighlightData::ContainerInfo>({ {lang, QRegularExpression(),
                                                                         QList<QSharedPointer<LanguageContextReference>>()} });
    }

    HighlightData *currentStateData = static_cast<HighlightData *>(currentBlockUserData());
    if(!currentStateData) {
        currentStateData = new HighlightData();
        setCurrentBlockUserData(currentStateData);
    }
    currentStateData->containers = QList<HighlightData::ContainerInfo>();

    int start = 0;
    for (int i = 0; i < previousBlockContainers.length(); ++i) {
        auto containerInfo = previousBlockContainers[i];

        int len = highlightTillContainerEnd(text, start, containerInfo, currentStateData);
        start += len;
        if(start <= text.length()) {
            while(containerInfo.containerRef->context.staticCast<LanguageContextContainer>()->endParent) {
                i++;
                containerInfo = previousBlockContainers[i];
            }
        }
    }

    setCurrentBlockState(qHash(currentStateData->containers));
}

int LiriSyntaxHighlighter::highlightTillContainerEnd(const QString &text, int offset, HighlightData::ContainerInfo containerInfo,
                                                     HighlightData *stateData, int startLength) {
    auto container = containerInfo.containerRef->context.staticCast<LanguageContextContainer>();
    HighlightData::ContainerInfo newContainerInfo = containerInfo;
    int innerStart = offset + startLength;
    int end;
    // Highlight whole received text
    if(containerInfo.containerRef->style)
        setFormat(container->styleInside ? innerStart : offset, text.length(), defStyles->styles[containerInfo.containerRef->style->defaultId]);
    QRegularExpressionMatch endMatch = highlightPart(end, text, innerStart, newContainerInfo, stateData);
    // Clear style of block ending
    int highlightEnd = endMatch.hasMatch() ? (container->styleInside ? endMatch.capturedStart() : endMatch.capturedEnd()) : end;
    if(containerInfo.containerRef->style)
        setFormat(highlightEnd, text.length(), QTextCharFormat());

    if(end > text.length()) {
        stateData->containers.append(newContainerInfo);
    }

    if(endMatch.hasMatch()) {
        for (auto inc : container->includes) {
            if(inc->context->type == LanguageContext::SubPattern) {
                auto subpattern = inc->context.staticCast<LanguageContextSubPattern>();
                if(subpattern->where == LanguageContextSubPattern::End && endMatch.hasMatch()) {
                    int endStart = subpattern->groupName.isNull() ? endMatch.capturedStart(subpattern->groupId) :
                                                                    endMatch.capturedStart(subpattern->groupName);
                    if(endStart >= 0) {
                        int endLen = subpattern->groupName.isNull() ? endMatch.capturedLength(subpattern->groupId) :
                                                                      endMatch.capturedLength(subpattern->groupName);
                        if(inc->style)
                            setFormat(offset + endStart,
                                      endLen, defStyles->styles[inc->style->defaultId]);
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
    auto container = currentContainerInfo.containerRef->context.staticCast<LanguageContextContainer>();
    QList<Match> matches;
    QRegularExpressionMatchIterator containerEndIter;
    if(currentContainerInfo.endRegex.pattern() != "")
        containerEndIter = currentContainerInfo.endRegex.globalMatch(text, offset);
    auto extendedContainer = container->includes;
    for (int i = 0; i < extendedContainer.length(); ++i) {
        QSharedPointer<LanguageContext> context = extendedContainer[i]->context;
        if(!context)
            continue;

        switch (context->type) {
        case LanguageContext::Keyword: {
            auto kw = context.staticCast<LanguageContextKeyword>();

            if(kw->firstLineOnly && currentBlock().position() != 0)
                break;
            if(currentContainerInfo.forbiddenContexts.contains(extendedContainer[i]))
                break;

            QRegularExpression keyword = kw->keyword;
            QRegularExpressionMatchIterator kwI = keyword.globalMatch(text, offset);
            while (kwI.hasNext()) {
                QRegularExpressionMatch kwMatch = kwI.next();
                matches.append({kwMatch, extendedContainer[i]});
            }
            break;
        }
        case LanguageContext::Simple: {
            auto simple = context.staticCast<LanguageContextSimple>();

            if(simple->firstLineOnly && currentBlock().position() != 0)
                break;
            if(currentContainerInfo.forbiddenContexts.contains(extendedContainer[i]))
                break;

            QRegularExpressionMatchIterator matchI = simple->match.globalMatch(text, offset);
            while (matchI.hasNext()) {
                QRegularExpressionMatch match = matchI.next();
                matches.append({match, extendedContainer[i]});
            }
            break;
        }
        case LanguageContext::Container: {
            auto container = context.staticCast<LanguageContextContainer>();

            if(container->firstLineOnly && currentBlock().position() != 0)
                break;
            if(currentContainerInfo.forbiddenContexts.contains(extendedContainer[i]))
                break;

            if(container->start.pattern() == "") {
                extendedContainer.append(container->includes);
            } else {
                if(!container->start.isValid())
                    qDebug() << "Regular expression error during highlighting:" << container->start.errorString();
                QRegularExpressionMatchIterator matchI = container->start.globalMatch(text, offset);
                while (matchI.hasNext()) {
                    QRegularExpressionMatch startMatch = matchI.next();
                    matches.append({startMatch, extendedContainer[i]});
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
    else if(container->endAtLineEnd)
        containerEndMatch = QRegularExpression("$").match(text, offset);

    end = offset;
    for (auto m : matches) {
        if(m.match.capturedStart() >= end) {
            if(containerEndMatch.hasMatch() && containerEndMatch.capturedStart() < m.match.capturedStart())
                break;
            switch (m.contextRef->context->type) {
            case LanguageContext::Keyword: {
                auto kw = m.contextRef->context.staticCast<LanguageContextKeyword>();

                if(!kw->extendParent && containerEndMatch.hasMatch() && containerEndMatch.capturedStart() < m.match.capturedEnd())
                    continue;
                if(kw->onceOnly) {
                    if(currentContainerInfo.forbiddenContexts.contains(m.contextRef))
                        continue;
                    else
                        currentContainerInfo.forbiddenContexts.append(m.contextRef);
                }

                if(m.contextRef->style)
                    setFormat(m.match.capturedStart(), m.match.capturedLength(),
                              defStyles->styles[m.contextRef->style->defaultId]);
                end = m.match.capturedEnd();
                if(kw->endParent)
                    return QRegularExpressionMatch();
                break;
            }
            case LanguageContext::Simple: {
                auto simple = m.contextRef->context.staticCast<LanguageContextSimple>();

                if(!simple->extendParent && containerEndMatch.hasMatch() && containerEndMatch.capturedStart() < m.match.capturedEnd())
                    continue;
                if(simple->onceOnly) {
                    if(currentContainerInfo.forbiddenContexts.contains(m.contextRef))
                        continue;
                    else
                        currentContainerInfo.forbiddenContexts.append(m.contextRef);
                }

                if(m.contextRef->style)
                    setFormat(m.match.capturedStart(), m.match.capturedLength(),
                              defStyles->styles[m.contextRef->style->defaultId]);
                end = m.match.capturedEnd();
                for (auto inc : simple->includes) {
                    if(inc->context->type == LanguageContext::SubPattern) {
                        auto subpattern = inc->context.staticCast<LanguageContextSubPattern>();
                        int mStart = subpattern->groupName.isNull() ? m.match.capturedStart(subpattern->groupId) :
                                                                      m.match.capturedStart(subpattern->groupName);
                        int mLen = subpattern->groupName.isNull() ? m.match.capturedLength(subpattern->groupId) :
                                                                    m.match.capturedLength(subpattern->groupName);
                        setFormat(mStart,
                                  mLen, defStyles->styles[inc->style->defaultId]);
                    }
                }
                if(simple->endParent)
                    return QRegularExpressionMatch();
                break;
            }
            case LanguageContext::Container: {
                auto container = m.contextRef->context.staticCast<LanguageContextContainer>();

                if(!container->extendParent && containerEndMatch.hasMatch() && containerEndMatch.capturedStart() < m.match.capturedEnd())
                    continue;
                if(container->onceOnly) {
                    if(currentContainerInfo.forbiddenContexts.contains(m.contextRef))
                        continue;
                    else
                        currentContainerInfo.forbiddenContexts.append(m.contextRef);
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

                int len = highlightTillContainerEnd(text, start, {m.contextRef, endRegex, {}}, stateData, m.match.capturedLength());
                end = start + len;

                // Highlight start subpatterns
                for (auto inc : container->includes) {
                    if(inc->context->type == LanguageContext::SubPattern) {
                        auto subpattern = inc->context.staticCast<LanguageContextSubPattern>();
                        if(subpattern->where == LanguageContextSubPattern::Start) {
                            int startStart = subpattern->groupName.isNull() ? m.match.capturedStart(subpattern->groupId) :
                                                                              m.match.capturedStart(subpattern->groupName);
                            if(startStart >= 0) {
                                int startLen = subpattern->groupName.isNull() ? m.match.capturedLength(subpattern->groupId) :
                                                                                m.match.capturedLength(subpattern->groupName);
                                if(inc->style)
                                    setFormat(offset + startStart,
                                              startLen, defStyles->styles[inc->style->defaultId]);
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
                else if(container->endAtLineEnd && end < text.length())
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
