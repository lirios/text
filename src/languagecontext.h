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

#ifndef LANGUAGECONTEXT_H
#define LANGUAGECONTEXT_H

#include <QString>
#include <QSharedPointer>
#include "languagecontextbase.h"

class QXmlStreamAttributes;

class LanguageContext
{
public:
    enum ElementType {
        Simple
      , Container
      , SubPattern
      , Keyword
      , Undefined
    } type;

    QSharedPointer<LanguageContextBase> base;

    LanguageContext();
    LanguageContext(const LanguageContext &other);
    virtual ~LanguageContext();
    void init(ElementType t);
    void init(ElementType t, QXmlStreamAttributes attributes);
    LanguageContext& operator =(const LanguageContext &other);

    QString styleId;
};

#endif // LANGUAGECONTEXT_H
