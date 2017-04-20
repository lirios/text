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

#ifndef LANGUAGELOADER_H
#define LANGUAGELOADER_H

#include <QString>
#include <QXmlStreamReader>
#include <QRegularExpression>
#include <QHash>
#include <QMimeType>

#include "languagecontextreference.h"
#include "languagedefaultstyles.h"
#include "languagemetadata.h"

class LanguageLoader
{
public:
    LanguageLoader();
    LanguageLoader(QSharedPointer<LanguageDefaultStyles> defaultStyles);
    ~LanguageLoader();

    QSharedPointer<LanguageContext> loadMainContextById(const QString &id);
    QSharedPointer<LanguageContext> loadMainContextByMimeType(const QMimeType &mimeType, const QString &filename);
    QSharedPointer<LanguageContext> loadMainContext(const QString &path);
    LanguageMetadata loadMetadata(const QString &path);
    void loadDefinitionsAndStylesById(const QString &id);
    void loadDefinitionsAndStyles(const QString &path);

    inline QHash<QString, QString> styleMap() { return m_styleMap; }
private:
    void parseMetadata(QXmlStreamReader &xml, LanguageMetadata &metadata);
    void parseStyles(QXmlStreamReader &xml, const QString &langId);
    void parseDefinitions(QXmlStreamReader &xml, const QString &langId);
    QSharedPointer<LanguageContextReference> parseContext(QXmlStreamReader &xml, const QString &langId,
                                                          const QXmlStreamAttributes &additionalAttributes = QXmlStreamAttributes());
    void parseStyle(QXmlStreamReader &xml, const QString &langId);
    QRegularExpression::PatternOptions parseRegexOptions(QXmlStreamReader &xml, const QString &langId);
    void parseDefaultRegexOptions(QXmlStreamReader &xml, const QString &langId);
    void parseDefineRegex(QXmlStreamReader &xml, const QString &langId);
    void parseWordCharClass(QXmlStreamReader &xml, const QString &langId);
    void parseReplace(QXmlStreamReader &xml, const QString &langId);
    QRegularExpression resolveRegex(const QString &pattern, QRegularExpression::PatternOptions options, const QString &langId);
    QString escapeNonExtended(const QString &pattern);
    QString applyOptionsToSubRegex(const QString &pattern, QRegularExpression::PatternOptions options);
    QSharedPointer<LanguageContext> buildContextTree(QSharedPointer<LanguageContextReference> reference);

    QHash<QString, QSharedPointer<LanguageContextReference> > m_knownContexts;
    QHash<QString, QSharedPointer<LanguageContextReference> > m_originalContexts;
    QHash<QString, QString> m_knownRegexes;
    QHash<QString, QRegularExpression::PatternOptions> m_languageDefaultOptions;
    QHash<QString, QString> m_languageLeftWordBoundary;
    QHash<QString, QString> m_languageRightWordBoundary;
    QHash<QString, QString> m_styleMap;
    QList<QString> m_themeStyles;
};

#endif // LANGUAGELOADER_H
