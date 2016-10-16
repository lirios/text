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

#include "languagespecification.h"

class LanguageLoader
{
public:
    LanguageLoader();
    LanguageSpecification *loadById(QString name);
    LanguageSpecification *loadFromFile(QString path);
protected:
    void parseMetadata(LanguageSpecification *lang, QXmlStreamReader *xml);
    LanguageContext *parseContext(QXmlStreamReader &xml, QString langId);
    void parseDefineRegex(QXmlStreamReader &xml);
    QString resolveRegex(QString pattern);
    QHash<QString, LanguageContext *> knownContexts;
    QHash<QString, QString> knownRegexes;
};

#endif // LANGUAGELOADER_H
