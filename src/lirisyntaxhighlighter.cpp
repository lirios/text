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
            QRegularExpression endRegex = QRegularExpression(container->endPattern);
            if(container->endPattern == "")
                end = -1;
            else
                end = endRegex.match(text, start).capturedEnd();

            QTextBlock nextBlock = currentBlock().next();
            if(nextBlock.isValid()) {
                if(end == -1) {
                    end = text.length();
                } else {
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
            QRegularExpression kwRegexp("\\b(" + kw->keyword + ")\\b");
            QRegularExpressionMatchIterator kwI = kwRegexp.globalMatch(text);
            while (kwI.hasNext()) {
                QRegularExpressionMatch kwMatch = kwI.next();
                setFormat(kwMatch.capturedStart(), kwMatch.capturedLength(), keywordFormat);
            }
        }
        if(ce->type == LanguageContextElement::Simple) {
            LanguageContextElementSimple *simple = static_cast<LanguageContextElementSimple *>(ce);
            QRegularExpression matchRegex = QRegularExpression(simple->matchPattern);
            QRegularExpressionMatchIterator matchI = matchRegex.globalMatch(text);
            while (matchI.hasNext()) {
                QRegularExpressionMatch match = matchI.next();
                setFormat(match.capturedStart(), match.capturedLength(), simpleFormat);
            }
        }
        if(ce->type == LanguageContextElement::Container) {
            LanguageContextElementContainer *container = static_cast<LanguageContextElementContainer *>(ce);
            QRegularExpression startRegex = QRegularExpression(container->startPattern);
            QRegularExpression endRegex = QRegularExpression(container->endPattern);
            QRegularExpressionMatch startMatch;
            QRegularExpressionMatch endMatch;
            while ((startMatch = startRegex.match(text, endMatch.hasMatch() ? endMatch.capturedEnd() : 0)).hasMatch()) {
                if(container->endPattern == "") {
                    setFormat(startMatch.capturedStart(), text.length() - startMatch.capturedStart(), containerFormat);
                    break;
                }

                endMatch = endRegex.match(text, startMatch.capturedEnd());
                if(endMatch.hasMatch()) {
                    setFormat(startMatch.capturedStart(), endMatch.capturedEnd() - startMatch.capturedStart(), containerFormat);
                } else {
                    nextStateData->containers.insert(0, container);
                    QTextBlock nextBlock = currentBlock().next();
                    if(nextBlock.isValid()) {
                        state++;
                    }
                    setFormat(startMatch.capturedStart(), text.length() - startMatch.capturedStart(), containerFormat);
                    break;
                }
            }
        }
    }
    setCurrentBlockState(state);
}
