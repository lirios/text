/*
 * Copyright © 2016-2017 Andrew Penkrat
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
#include <QTextDocumentFragment>
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

QString LiriSyntaxHighlighter::highlightedFragment(int position, int blockCount, QFont font) {
    QTextCursor cursor(document()->findBlock(position));
    cursor.movePosition(QTextCursor::Up, QTextCursor::MoveAnchor, blockCount / 2);
    cursor.movePosition(QTextCursor::Down, QTextCursor::KeepAnchor, blockCount);
    QTextDocument *tempDocument(new QTextDocument);
    Q_ASSERT(tempDocument);
    QTextCursor tempCursor(tempDocument);

    tempCursor.insertFragment(cursor.selection());
    tempCursor.select(QTextCursor::Document);
    // Set preview font
    QTextCharFormat textfmt = tempCursor.charFormat();
    textfmt.setFontFamily(font.family());
    tempCursor.setCharFormat(textfmt);

    // Apply the formats set by the syntax highlighter
    QTextBlock start = document()->findBlock(cursor.selectionStart());
    QTextBlock end = document()->findBlock(cursor.selectionEnd());
    end = end.next();
    const int selectionStart = cursor.selectionStart();
    const int endOfDocument = tempDocument->characterCount() - 1;
    for(QTextBlock current = start; current.isValid() && current != end; current = current.next()) {
        const QTextLayout *layout(current.layout());

        foreach(const QTextLayout::FormatRange &range, layout->additionalFormats()) {
            const int start = current.position() + range.start - selectionStart;
            const int end = start + range.length;
            if(end <= 0 or start >= endOfDocument)
                continue;
            tempCursor.setPosition(qMax(start, 0));
            tempCursor.setPosition(qMin(end, endOfDocument), QTextCursor::KeepAnchor);
            textfmt = range.format;
            textfmt.setFontFamily(font.family());
            tempCursor.setCharFormat(textfmt);
        }
    }

    // Reset the user states since they are not interesting
    for(QTextBlock block = tempDocument->begin(); block.isValid(); block = block.next())
        block.setUserState(-1);

    // Make sure the text appears pre-formatted
    tempCursor.select(QTextCursor::Document);
    QTextBlockFormat blockFormat = tempCursor.blockFormat();
    blockFormat.setNonBreakableLines(true);
    tempCursor.setBlockFormat(blockFormat);

    // Finally retreive the syntax higlighted html
    return tempCursor.selection().toHtml();
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
    auto containerStack = previousBlockContainers;

    int start = 0;
    bool highlightingProgresses = true;
    startContainer(containerStack, containerStack.first().containerRef, start, text.length());

    while (highlightingProgresses) {
        auto& containerInfo = containerStack.first();
        int containerIdx = 0;

        QRegularExpressionMatch containerEndMatch;
        for (int i = 0; i < containerStack.size(); ++i) {
            QRegularExpressionMatch tmp;
            if(containerStack[i].endRegex.pattern() != "")
                tmp = containerStack[i].endRegex.match(text, start);
            if(!tmp.hasMatch() && containerStack[i].containerRef->context->container.endAtLineEnd)
                tmp = QRegularExpression("$").match(text, start);
            if(tmp.hasMatch() && (!containerEndMatch.hasMatch() || tmp.capturedStart() <= containerEndMatch.capturedStart())) {
                containerIdx = i;
                containerEndMatch = tmp;
            }
            if(containerStack[i].containerRef->context->container.extendParent)
                break;
        }

        Match bestMatch = findMatch(text, start, containerEndMatch.hasMatch() ? containerEndMatch.capturedStart() : text.length(),
                                    containerInfo.containerRef, containerInfo);

        if(!bestMatch.match.hasMatch()) {
            if(!containerEndMatch.hasMatch()) {
                start = text.length();
                highlightingProgresses = false;
                continue;
            } else {
                start = containerEndMatch.capturedEnd();
                endNthContainer(containerStack, containerIdx, start, text.length(), containerEndMatch);
                continue;
            }
        } else if(containerEndMatch.hasMatch() && containerEndMatch.capturedStart() <= bestMatch.match.capturedStart()) {
            start = containerEndMatch.capturedEnd();
            endNthContainer(containerStack, containerIdx, start, text.length(), containerEndMatch);
            continue;
        }

        switch (bestMatch.contextRef->context->type) {
        case LanguageContext::Keyword: {
            auto kw = bestMatch.contextRef->context->keyword;

            if(kw.onceOnly) {
                containerInfo.forbiddenContexts.append(bestMatch.contextRef);
            }

            if(m_styleMap.keys().contains(bestMatch.contextRef->styleId))
                setFormat(bestMatch.match.capturedStart(), bestMatch.match.capturedLength(),
                          m_defStyles->styles[m_styleMap[bestMatch.contextRef->styleId]]);

            start = bestMatch.match.capturedEnd();

            if(kw.endParent)
                endNthContainer(containerStack, 0, start, text.length());
            break;
        }
        case LanguageContext::Simple: {
            auto simple = bestMatch.contextRef->context->simple;

            if(simple.onceOnly) {
                containerInfo.forbiddenContexts.append(bestMatch.contextRef);
            }

            if(m_styleMap.keys().contains(bestMatch.contextRef->styleId))
                setFormat(bestMatch.match.capturedStart(), bestMatch.match.capturedLength(),
                          m_defStyles->styles[m_styleMap[bestMatch.contextRef->styleId]]);

            start = bestMatch.match.capturedEnd();

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
                endNthContainer(containerStack, 0, start, text.length());
            break;
        }
        case LanguageContext::Container: {
            auto container = bestMatch.contextRef->context->container;

            if(container.onceOnly) {
                containerInfo.forbiddenContexts.append(bestMatch.contextRef);
            }

            start = bestMatch.match.capturedEnd();
            startContainer(containerStack, bestMatch.contextRef, start, text.length(), bestMatch.match);
            break;
        }
        }
    }

    currentStateData->containers = containerStack;
    setCurrentBlockState(qHash(currentStateData->containers));
}

void LiriSyntaxHighlighter::endNthContainer(QList<HighlightData::ContainerInfo> &containers,
                                            int n, int offset, int length, QRegularExpressionMatch endMatch) {
    for (int i = 0; i < n; ++i)
        containers.removeFirst();

    if(endMatch.hasMatch()) {
        for (auto inc : containers.first().containerRef->context->container.includes) {
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

    while (containers.first().containerRef->context->container.endParent)
        containers.removeFirst();
    containers.removeFirst();
    startContainer(containers, containers.first().containerRef, offset, length);
}

void LiriSyntaxHighlighter::startContainer(QList<HighlightData::ContainerInfo> &containers, QSharedPointer<LanguageContextReference> container,
                                           int offset, int length, QRegularExpressionMatch startMatch) {
    int start = startMatch.hasMatch() ? startMatch.capturedStart() : offset;
    // Highlight the whole text
    QTextCharFormat containerFormat;
    if(m_styleMap.keys().contains(container->styleId))
        containerFormat = m_defStyles->styles[m_styleMap[container->styleId]];
    else {
        for (int i = 0; i < containers.size(); ++i) {
            if(m_styleMap.keys().contains(containers[i].containerRef->styleId)) {
                containerFormat = m_defStyles->styles[m_styleMap[containers[i].containerRef->styleId]];
                break;
            }
        }
    }
    setFormat(container->context->container.styleInside ? offset : start, length, containerFormat);

    if(startMatch.hasMatch()) {
        // Resolve references to start subpatterns from end regex
        QRegularExpression endRegex = container->context->container.end;
        QString endPattern = endRegex.pattern();
        QRegularExpression startRefRegex = QRegularExpression("\\\\%{(.+)@start}");
        QRegularExpressionMatch startRefMatch;
        while((startRefMatch = startRefRegex.match(endPattern)).hasMatch()) {
            QString groupName = startRefMatch.captured(1);
            bool isId;
            int id = groupName.toInt(&isId);
            endPattern.replace(startRefMatch.capturedStart(), startRefMatch.capturedLength(),
                               QRegularExpression::escape(isId ? startMatch.captured(id) : startMatch.captured(groupName)));
        }
        if(endRegex.pattern() != endPattern) // Don't make regex dirty if there were no changes
            endRegex.setPattern(endPattern);

        // Highlight start subpatterns
        for (auto inc : container->context->container.includes) {
            if(inc->context->type == LanguageContext::SubPattern) {
                auto subPattern = inc->context->subPattern;
                if(subPattern.where == LanguageContextSubPattern::Start) {
                    int startStart = subPattern.groupName.isNull() ? startMatch.capturedStart(subPattern.groupId) :
                                                                     startMatch.capturedStart(subPattern.groupName);
                    if(startStart >= 0) {
                        int startLen = subPattern.groupName.isNull() ? startMatch.capturedLength(subPattern.groupId) :
                                                                       startMatch.capturedLength(subPattern.groupName);
                        if(m_styleMap.keys().contains(inc->styleId))
                            setFormat(startStart,
                                      startLen, m_defStyles->styles[m_styleMap[inc->styleId]]);
                    }
                }
            }
        }
        containers.prepend({container, endRegex, QList<QSharedPointer<LanguageContextReference>>()});
    }
}

LiriSyntaxHighlighter::Match LiriSyntaxHighlighter::findMatch(const QString &text, int offset, int potentialEnd,
                                                              QSharedPointer<LanguageContextReference> contextRef,
                                                              HighlightData::ContainerInfo &currentContainerInfo, bool rootContext) {
    QSharedPointer<LanguageContext> context = contextRef->context;

    switch (context->type) {
    case LanguageContext::Keyword: {
        if(context->keyword.firstLineOnly && currentBlock().position() != 0)
            break;
        if(currentContainerInfo.forbiddenContexts.contains(contextRef))
            break;

        QStringRef allowedText = QStringRef(&text);
        if(!context->keyword.extendParent)
            allowedText = allowedText.left(potentialEnd);
        Match bestMatch;
        for (QRegularExpression keyword : context->keyword.keywords) {
            QRegularExpressionMatch kwMatch = keyword.match(allowedText, offset);
            if(kwMatch.hasMatch()) {
                Match match = {kwMatch, contextRef};
                if(match < bestMatch)
                    bestMatch = match;
            }
        }
        return bestMatch;
    }
    case LanguageContext::Simple: {
        if(context->simple.firstLineOnly && currentBlock().position() != 0)
            break;
        if(currentContainerInfo.forbiddenContexts.contains(contextRef))
            break;

        QStringRef allowedText = QStringRef(&text);
        if(!context->simple.extendParent)
            allowedText = allowedText.left(potentialEnd);
        QRegularExpressionMatch match = context->simple.match.match(allowedText, offset);
        return {match, contextRef};
    }
    case LanguageContext::Container: {
        if(context->container.firstLineOnly && currentBlock().position() != 0)
            break;
        if(currentContainerInfo.forbiddenContexts.contains(contextRef))
            break;

        if(context->container.includesOnly || rootContext) {
            Match bestMatch;
            for (auto inc : context->container.includes) {
                Match match = findMatch(text, offset, potentialEnd, inc, currentContainerInfo, false);
                if(match < bestMatch)
                    bestMatch = match;
            }
            return bestMatch;
        } else {
            if(!context->container.start.isValid())
                qDebug() << "Regular expression error during highlighting:" << context->container.start.errorString();
            QStringRef allowedText = QStringRef(&text);
            if(!context->container.extendParent)
                allowedText = allowedText.left(potentialEnd);
            QRegularExpressionMatch startMatch = context->container.start.match(allowedText, offset);
            return {startMatch, contextRef};
        }
    }
    }

    return {QRegularExpressionMatch(), QSharedPointer<LanguageContextReference>()};
}

bool LiriSyntaxHighlighter::Match::operator <(const Match &other) {
    if(!this->match.hasMatch())
        return false;
    if(!other.match.hasMatch())
        return true;
    return this->match.capturedStart() < other.match.capturedStart();
}
