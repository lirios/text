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
      m_lang(),
      m_defStyles() { }

LiriSyntaxHighlighter::~LiriSyntaxHighlighter() {
    if(m_lang)
        m_lang->context->prepareForRemoval(true);
}

void LiriSyntaxHighlighter::setLanguage(QSharedPointer<LanguageContextReference> lang, const QHash<QString, QString> &styleMap) {
    if(m_lang)
        m_lang->context->prepareForRemoval(true);
    m_lang = lang;
    m_styleMap = styleMap;
    if(m_defStyles)
        rehighlight();
}

void LiriSyntaxHighlighter::setDefaultStyles(QSharedPointer<LanguageDefaultStyles> defStyles) {
    m_defStyles = defStyles;
    if(m_lang)
        rehighlight();
}

void LiriSyntaxHighlighter::highlightBlock(const QString &text) {
    if(!m_lang || !m_defStyles)
        return;

    QList<HighlightData::ContainerInfo> previousBlockContainers;
    if(currentBlock().previous().isValid())
        previousBlockContainers = static_cast<HighlightData *>(currentBlock().previous().userData())->containers;
    else {
        previousBlockContainers = QList<HighlightData::ContainerInfo>({ {m_lang, QRegularExpression(),
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
            while(containerInfo.containerRef->context->container.endParent) {
                i++;
                containerInfo = previousBlockContainers[i];
            }
        }
    }

    setCurrentBlockState(qHash(currentStateData->containers));
}

int LiriSyntaxHighlighter::highlightTillContainerEnd(const QString &text, int offset, HighlightData::ContainerInfo containerInfo,
                                                     HighlightData *stateData, int startLength) {
    auto container = containerInfo.containerRef->context->container;
    HighlightData::ContainerInfo newContainerInfo = containerInfo;
    int innerStart = offset + startLength;
    int end;
    // Highlight whole received text
    if(m_styleMap.keys().contains(containerInfo.containerRef->styleId))
        setFormat(container.styleInside ? innerStart : offset, text.length(), m_defStyles->styles[m_styleMap[containerInfo.containerRef->styleId]]);
    QRegularExpressionMatch endMatch = highlightPart(end, text, innerStart, newContainerInfo, stateData);
    // Clear style of block ending
    int highlightEnd = endMatch.hasMatch() ? (container.styleInside ? endMatch.capturedStart() : endMatch.capturedEnd()) : end;
    if(m_styleMap.keys().contains(containerInfo.containerRef->styleId))
        setFormat(highlightEnd, text.length(), QTextCharFormat());

    if(end > text.length()) {
        stateData->containers.append(newContainerInfo);
    }

    if(endMatch.hasMatch()) {
        for (auto inc : container.includes) {
            if(inc->context->type == LanguageContext::SubPattern) {
                auto subPattern = inc->context->subPattern;
                if(subPattern.where == LanguageContextSubPattern::End && endMatch.hasMatch()) {
                    int endStart = subPattern.groupName.isNull() ? endMatch.capturedStart(subPattern.groupId) :
                                                                   endMatch.capturedStart(subPattern.groupName);
                    if(endStart >= 0) {
                        int endLen = subPattern.groupName.isNull() ? endMatch.capturedLength(subPattern.groupId) :
                                                                     endMatch.capturedLength(subPattern.groupName);
                        if(m_styleMap.keys().contains(inc->styleId))
                            setFormat(endStart,
                                      endLen, m_defStyles->styles[m_styleMap[inc->styleId]]);
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
    auto currentContainer = currentContainerInfo.containerRef->context->container;
    QList<Match> matches;
    QRegularExpressionMatch containerEndMatch;
    if(currentContainerInfo.endRegex.pattern() != "")
        containerEndMatch = currentContainerInfo.endRegex.match(text, offset);
    if(!containerEndMatch.hasMatch() && currentContainer.endAtLineEnd)
        containerEndMatch = QRegularExpression("$").match(text, offset);

    auto extendedContainer = currentContainer.includes;
    for (int i = 0; i < extendedContainer.length(); ++i) {
        QSharedPointer<LanguageContext> context = extendedContainer[i]->context;
        if(!context)
            continue;

        switch (context->type) {
        case LanguageContext::Keyword: {
            if(context->keyword.firstLineOnly && currentBlock().position() != 0)
                break;
            if(currentContainerInfo.forbiddenContexts.contains(extendedContainer[i]))
                break;

            QRegularExpression keyword = context->keyword.keyword;
            QRegularExpressionMatch kwMatch = keyword.match(text, offset);
            if(kwMatch.hasMatch())
                matches.append({kwMatch, extendedContainer[i]});
            break;
        }
        case LanguageContext::Simple: {
            if(context->simple.firstLineOnly && currentBlock().position() != 0)
                break;
            if(currentContainerInfo.forbiddenContexts.contains(extendedContainer[i]))
                break;

            QRegularExpressionMatch match = context->simple.match.match(text, offset);
            if(match.hasMatch())
                matches.append({match, extendedContainer[i]});
            break;
        }
        case LanguageContext::Container: {
            if(context->container.firstLineOnly && currentBlock().position() != 0)
                break;
            if(currentContainerInfo.forbiddenContexts.contains(extendedContainer[i]))
                break;

            if(context->container.start.pattern() == "") {
                extendedContainer.append(context->container.includes);
            } else {
                if(!context->container.start.isValid())
                    qDebug() << "Regular expression error during highlighting:" << context->container.start.errorString();
                QRegularExpressionMatch startMatch = context->container.start.match(text, offset);
                if(startMatch.hasMatch())
                    matches.append({startMatch, extendedContainer[i]});
            }
            break;
        }
        }
    }


    Match bestMatch;
    for (Match m : matches) {
        if(m < bestMatch)
            bestMatch = m;
    }

    end = offset;
    if(!bestMatch.match.hasMatch()) {
        if(!containerEndMatch.hasMatch()) {
            end = text.length() + 1;
            return containerEndMatch;
        } else {
            end = containerEndMatch.capturedEnd();
            return containerEndMatch;
        }
    } else if(containerEndMatch.hasMatch() && containerEndMatch.capturedStart() <= bestMatch.match.capturedStart()) {
        end = containerEndMatch.capturedEnd();
        return containerEndMatch;
    }

    switch (bestMatch.contextRef->context->type) {
    case LanguageContext::Keyword: {
        auto kw = bestMatch.contextRef->context->keyword;

        if(!kw.extendParent && containerEndMatch.hasMatch() && containerEndMatch.capturedStart() < bestMatch.match.capturedEnd()) {
            end = containerEndMatch.capturedEnd();
            return containerEndMatch;
        }
        if(kw.onceOnly) {
            currentContainerInfo.forbiddenContexts.append(bestMatch.contextRef);
        }

        if(m_styleMap.keys().contains(bestMatch.contextRef->styleId))
            setFormat(bestMatch.match.capturedStart(), bestMatch.match.capturedLength(),
                      m_defStyles->styles[m_styleMap[bestMatch.contextRef->styleId]]);
        end = bestMatch.match.capturedEnd();
        if(kw.endParent)
            return QRegularExpressionMatch();
        break;
    }
    case LanguageContext::Simple: {
        auto simple = bestMatch.contextRef->context->simple;

        if(!simple.extendParent && containerEndMatch.hasMatch() && containerEndMatch.capturedStart() < bestMatch.match.capturedEnd()) {
            end = containerEndMatch.capturedEnd();
            return containerEndMatch;
        }
        if(simple.onceOnly) {
            currentContainerInfo.forbiddenContexts.append(bestMatch.contextRef);
        }

        if(m_styleMap.keys().contains(bestMatch.contextRef->styleId))
            setFormat(bestMatch.match.capturedStart(), bestMatch.match.capturedLength(),
                      m_defStyles->styles[m_styleMap[bestMatch.contextRef->styleId]]);
        end = bestMatch.match.capturedEnd();
        for (auto inc : simple.includes) {
            if(inc->context->type == LanguageContext::SubPattern) {
                auto subPattern = inc->context->subPattern;
                int mStart = subPattern.groupName.isNull() ? bestMatch.match.capturedStart(subPattern.groupId) :
                                                             bestMatch.match.capturedStart(subPattern.groupName);
                int mLen = subPattern.groupName.isNull() ? bestMatch.match.capturedLength(subPattern.groupId) :
                                                           bestMatch.match.capturedLength(subPattern.groupName);
                if(m_styleMap.keys().contains(inc->styleId))
                    setFormat(mStart,
                              mLen, m_defStyles->styles[m_styleMap[inc->styleId]]);
            }
        }
        if(simple.endParent)
            return QRegularExpressionMatch();
        break;
    }
    case LanguageContext::Container: {
        auto container = bestMatch.contextRef->context->container;

        if(!container.extendParent && containerEndMatch.hasMatch() && containerEndMatch.capturedStart() < bestMatch.match.capturedEnd()) {
            end = containerEndMatch.capturedEnd();
            return containerEndMatch;
        }
        if(container.onceOnly) {
            currentContainerInfo.forbiddenContexts.append(bestMatch.contextRef);
        }

        int start = bestMatch.match.capturedStart();

        // Resolve references to start subpatterns from end regex
        QRegularExpression endRegex = container.end;
        QString endPattern = endRegex.pattern();
        QRegularExpression startRefRegex = QRegularExpression("\\\\%{(.+)@start}");
        QRegularExpressionMatch startRefMatch;
        while((startRefMatch = startRefRegex.match(endPattern)).hasMatch()) {
            QString groupName = startRefMatch.captured(1);
            bool isId;
            int id = groupName.toInt(&isId);
            endPattern.replace(startRefMatch.capturedStart(), startRefMatch.capturedLength(),
                               QRegularExpression::escape(isId ? bestMatch.match.captured(id) : bestMatch.match.captured(groupName)));
        }
        if(endRegex.pattern() != endPattern) // Don't make regex dirty if there were no changes
            endRegex.setPattern(endPattern);

        int len = highlightTillContainerEnd(text, start, {bestMatch.contextRef, endRegex, {}}, stateData, bestMatch.match.capturedLength());
        end = start + len;

        // Highlight start subpatterns
        for (auto inc : container.includes) {
            if(inc->context->type == LanguageContext::SubPattern) {
                auto subPattern = inc->context->subPattern;
                if(subPattern.where == LanguageContextSubPattern::Start) {
                    int startStart = subPattern.groupName.isNull() ? bestMatch.match.capturedStart(subPattern.groupId) :
                                                                     bestMatch.match.capturedStart(subPattern.groupName);
                    if(startStart >= 0) {
                        int startLen = subPattern.groupName.isNull() ? bestMatch.match.capturedLength(subPattern.groupId) :
                                                                       bestMatch.match.capturedLength(subPattern.groupName);
                        if(m_styleMap.keys().contains(inc->styleId))
                            setFormat(startStart,
                                      startLen, m_defStyles->styles[m_styleMap[inc->styleId]]);
                    }
                }
            }
        }

        if(container.endParent)
            return QRegularExpressionMatch();
        break;
    }
    }
    return highlightPart(end, text, end, currentContainerInfo, stateData);
}

bool LiriSyntaxHighlighter::Match::operator <(const LiriSyntaxHighlighter::Match &other) {
    if(!this->match.hasMatch())
        return false;
    if(!other.match.hasMatch())
        return true;
    return this->match.capturedStart() < other.match.capturedStart();
}
