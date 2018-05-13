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

#ifndef LANGUAGECONTEXTCONTAINER_H
#define LANGUAGECONTEXTCONTAINER_H

#include <QList>
#include <QRegularExpression>
#include <QSharedPointer>
#include "languagecontextbase.h"

class QXmlStreamAttributes;
class LanguageContext;

class LanguageContextContainer : public LanguageContextBase
{
public:
    LanguageContextContainer();
    LanguageContextContainer(const QXmlStreamAttributes &attributes);

    QRegularExpression start;
    QRegularExpression end;
    QList<QSharedPointer<LanguageContext> > includes;

    bool styleInside   = false;
    bool extendParent  = true;
    bool endAtLineEnd  = false;
    bool endParent     = false;
    bool firstLineOnly = false;
    bool onceOnly      = false;
    bool includesOnly  = true;

    void markAsInUse() override;
    void prepareForRemoval(bool ignoreUsage = false) override;
};

#endif // LANGUAGECONTEXTCONTAINER_H
