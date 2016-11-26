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

#ifndef LIRISYNTAXHIGHLIGHTER_H
#define LIRISYNTAXHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QRegularExpressionMatch>
#include "languagecontextcontainer.h"
#include "languagecontextsimple.h"
#include "highlightdata.h"
#include "languagedefaultstyles.h"

class LiriSyntaxHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT
public:
    LiriSyntaxHighlighter(QTextDocument *parent);
    ~LiriSyntaxHighlighter();
    void setLanguage(QSharedPointer<LanguageContextSimple> l);
    void setDefaultStyle(QSharedPointer<LanguageDefaultStyles> def);

protected:
    struct Match {
        QRegularExpressionMatch match;
        QSharedPointer<LanguageContext> context;

        inline bool operator <(const Match &other);
    };

    void highlightBlock(const QString &text);

    // Returns length. If container's end was not found, returns negative number
    int highlightTillContainerEnd(const QStringRef &text, QSharedPointer<LanguageContextContainer> container,
                                  HighlightData *stateData = nullptr, QRegularExpressionMatch startMatch = QRegularExpressionMatch());

    // Returns true if stateData's changed
    bool highlightPart(const QStringRef &text, QList<QSharedPointer<LanguageContext> > currentContainer, HighlightData *stateData = nullptr);

    QSharedPointer<LanguageContextSimple> lang;
    QSharedPointer<LanguageDefaultStyles> defStyles;
};

#endif // LIRISYNTAXHIGHLIGHTER_H
