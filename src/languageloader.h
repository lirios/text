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

#ifndef LANGUAGELOADER_H
#define LANGUAGELOADER_H

#include <QString>
#include <QXmlStreamReader>
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

    QSharedPointer<LanguageContextReference> loadMainContextById(QString id);
    QSharedPointer<LanguageContextReference> loadMainContextByMimeType(QMimeType mimeType, QString filename);
    QSharedPointer<LanguageContextReference> loadMainContext(QString path);
    LanguageMetadata loadMetadata(QString path);
    void loadDefinitionsAndStylesById(QString id);
    void loadDefinitionsAndStyles(QString path);

    inline QHash<QString, QString> styleMap() { return m_styleMap; }
protected:
    void parseMetadata(QXmlStreamReader &xml, LanguageMetadata &metadata);
    void parseStyles(QXmlStreamReader &xml, QString langId);
    void parseDefinitions(QXmlStreamReader &xml, QString langId);
    QSharedPointer<LanguageContextReference> parseContext(QXmlStreamReader &xml, QString langId, QXmlStreamAttributes additionalAttributes = QXmlStreamAttributes());
    void parseStyle(QXmlStreamReader &xml, QString langId);
    QRegularExpression::PatternOptions parseRegexOptions(QXmlStreamReader &xml, QString langId);
    void parseDefaultRegexOptions(QXmlStreamReader &xml, QString langId);
    void parseDefineRegex(QXmlStreamReader &xml, QString langId);
    void parseWordCharClass(QXmlStreamReader &xml, QString langId);
    void parseReplace(QXmlStreamReader &xml, QString langId);
    QRegularExpression resolveRegex(QString pattern, QRegularExpression::PatternOptions options, QString langId);
    QString escapeNonExtended(QString pattern);
    QString applyOptionsToSubRegex(QString pattern, QRegularExpression::PatternOptions options);
    void applyStyleToContext(QSharedPointer<LanguageContextReference> context, QString styleId);

    QHash<QString, QSharedPointer<LanguageContextReference>> knownContexts;
    QHash<QString, QSharedPointer<LanguageContextReference>> originalContexts;
    QHash<QString, QString> knownRegexes;
    QHash<QString, QRegularExpression::PatternOptions> languageDefaultOptions;
    QHash<QString, QString> languageLeftWordBoundary;
    QHash<QString, QString> languageRightWordBoundary;
    QHash<QString, QString> m_styleMap;
    QList<QString> themeStyles;
};

#endif // LANGUAGELOADER_H
