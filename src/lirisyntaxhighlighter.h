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
    void setLanguage(QSharedPointer<LanguageContextReference> l, const QHash<QString, QString> &map);
    void setDefaultStyle(QSharedPointer<LanguageDefaultStyles> def);

protected:
    struct Match {
        QRegularExpressionMatch match;
        QSharedPointer<LanguageContextReference> contextRef;

        inline bool operator <(const Match &other);
    };

    void highlightBlock(const QString &text);

    // Returns container length
    int highlightTillContainerEnd(const QString &text, int offset, HighlightData::ContainerInfo containerInfo,
                                  HighlightData *stateData, int startLength = 0);

    // Returns match for context end or empty match if it wasn't found
    // end is set to ending position of the context or to text.length + 1 if context doesn't end in current block
    QRegularExpressionMatch highlightPart(int &end, const QString &text, int offset,
                                          HighlightData::ContainerInfo &currentContainerInfo, HighlightData *stateData);

    QSharedPointer<LanguageContextReference> lang;
    QSharedPointer<LanguageDefaultStyles> defStyles;
    QHash<QString, QString> styleMap;
};

#endif // LIRISYNTAXHIGHLIGHTER_H
