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

#ifndef LIRISYNTAXHIGHLIGHTER_H
#define LIRISYNTAXHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QRegularExpressionMatch>
#include "languagecontext.h"
#include "highlightdata.h"
#include "languagedefaultstyles.h"

class LiriSyntaxHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT
public:
    LiriSyntaxHighlighter(QTextDocument *parent);
    ~LiriSyntaxHighlighter();
    void setLanguage(QSharedPointer<LanguageContext> lang, const QHash<QString, QString> &styleMap);
    void setDefaultStyles(QSharedPointer<LanguageDefaultStyles> defStyles);

    QString highlightedFragment(int position, int blockCount, QFont font);

protected:
    struct Match {
        QRegularExpressionMatch match;
        QSharedPointer<LanguageContext> context;

        inline bool operator <(const Match &other);
    };

    void highlightBlock(const QString &text);

    void endNthContainer(QList<HighlightData::ContainerInfo> &containers,
                         int n, int offset, int length, QRegularExpressionMatch endMatch = QRegularExpressionMatch());

    void startContainer(QList<HighlightData::ContainerInfo> &containers, QSharedPointer<LanguageContext> container,
                        int offset, int length, QRegularExpressionMatch startMatch = QRegularExpressionMatch());

    Match findMatch(const QString &text, int offset, int potentialEnd,
                    QSharedPointer<LanguageContext> context,
                    HighlightData::ContainerInfo &currentContainerInfo, bool rootContext = true);

    QSharedPointer<LanguageContext> m_lang;
    QSharedPointer<LanguageDefaultStyles> m_defStyles;
    QHash<QString, QString> m_styleMap;
};

#endif // LIRISYNTAXHIGHLIGHTER_H
