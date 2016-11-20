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

void LiriSyntaxHighlighter::setLanguage(QSharedPointer<LanguageSpecification> l) {
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
    currentStateData->containers = QList<QSharedPointer<LanguageContextContainer>>();

    int start = 0;
    if(previousStateData) {
        for (int i = 0; i < previousStateData->containers.length(); ++i) {
            auto container = previousStateData->containers[i];

            int len = highlightTillContainerEnd(text.midRef(start), container, currentStateData);
            if(len < 0) {
                start = text.length();
                currentStateData->containers.append(previousStateData->containers.mid(i + 1));
                break;
            } else {
                start += len;
            }
        }
    }
    highlightPart(text.midRef(start), lang->mainContext->includes, currentStateData);

    setCurrentBlockState(qHash(currentStateData->containers));
}

int LiriSyntaxHighlighter::highlightTillContainerEnd(const QStringRef &text, QSharedPointer<LanguageContextContainer> container,
                                                     HighlightData *stateData, QRegularExpressionMatch startMatch) {
    int innerStart = startMatch.hasMatch() ? startMatch.capturedLength() : 0;
    int len;
    QRegularExpressionMatch endMatch;

    if(container->endPattern == "") {
        if(container->style)
            setFormat(text.position(), text.length(), defStyles->styles[container->style->defaultId]);
        QStringRef part = text.mid(innerStart);
        highlightPart(part, container->includes);
        len = text.length();
    } else {
        QRegularExpression endRegex = QRegularExpression(container->endPattern);

        endMatch = endRegex.match(text, innerStart);
        if(endMatch.hasMatch()) {
            if(container->style)
                setFormat(text.position(), endMatch.capturedEnd(), defStyles->styles[container->style->defaultId]);
            QStringRef part = text.mid(innerStart, endMatch.capturedStart() - innerStart);
            highlightPart(part, container->includes);
            len = endMatch.capturedEnd();
        } else {
            if(stateData) {
                stateData->containers.insert(0, container);
            }
            if(container->style)
                setFormat(text.position(), text.length(), defStyles->styles[container->style->defaultId]);
            QStringRef part = text.mid(innerStart);
            highlightPart(part, container->includes, stateData);
            len = -1;
        }
    }

    for (QSharedPointer<LanguageContext> inc : container->includes) {
        if(inc->type == LanguageContext::SubPattern) {
            QSharedPointer<LanguageContextSubPattern> subpattern = inc.staticCast<LanguageContextSubPattern>();
            if(subpattern->where == LanguageContextSubPattern::Start &&
                   startMatch.hasMatch() && startMatch.capturedStart(subpattern->group) >= 0)
                if(subpattern->style)
                    setFormat(text.position() + startMatch.capturedStart(subpattern->group) - startMatch.capturedStart(),
                              startMatch.capturedLength(subpattern->group), defStyles->styles[subpattern->style->defaultId]);

            if(subpattern->where == LanguageContextSubPattern::End &&
                     endMatch.hasMatch() && endMatch.capturedStart(subpattern->group) >= 0)
                if(subpattern->style)
                    setFormat(text.position() + endMatch.capturedStart(subpattern->group) - startMatch.capturedStart(),
                              endMatch.capturedLength(subpattern->group), defStyles->styles[subpattern->style->defaultId]);
        }
    }

    return len;
}

bool LiriSyntaxHighlighter::highlightPart(const QStringRef &text, QList<QSharedPointer<LanguageContext>> currentContainer, HighlightData *stateData) {
    QList<Match> matches;
    QList<QSharedPointer<LanguageContext>> extendedContainer = currentContainer;
    for (int i = 0; i < extendedContainer.length(); ++i) {
        QSharedPointer<LanguageContext> context = extendedContainer[i];
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
            if(simple->matchPattern == "") {
                extendedContainer.append(simple->includes);
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
            QSharedPointer<LanguageContextContainer> container = context.staticCast<LanguageContextContainer>();
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
                if(m.context->style)
                    setFormat(text.position() + m.match.capturedStart(), m.match.capturedLength(),
                              defStyles->styles[m.context->style->defaultId]);
                position = m.match.capturedEnd();
                break;
            case LanguageContext::Simple:
                if(m.context->style)
                    setFormat(text.position() + m.match.capturedStart(), m.match.capturedLength(),
                              defStyles->styles[m.context->style->defaultId]);
                position = m.match.capturedEnd();
                for (QSharedPointer<LanguageContext> inc : m.context.staticCast<LanguageContextSimple>()->includes) {
                    if(inc->type == LanguageContext::SubPattern) {
                        QSharedPointer<LanguageContextSubPattern> subpattern = inc.staticCast<LanguageContextSubPattern>();
                        setFormat(text.position() + m.match.capturedStart(subpattern->group),
                                  m.match.capturedLength(subpattern->group), defStyles->styles[subpattern->style->defaultId]);
                    }
                }
                break;
            case LanguageContext::Container:
                QSharedPointer<LanguageContextContainer> container = m.context.staticCast<LanguageContextContainer>();
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

bool LiriSyntaxHighlighter::Match::operator <(const LiriSyntaxHighlighter::Match &other) {
    if(this->match.capturedStart() == other.match.capturedStart())
        return this->match.capturedEnd() > other.match.capturedEnd();
    else
        return this->match.capturedStart() < other.match.capturedStart();
}
