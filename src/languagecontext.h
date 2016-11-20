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

#ifndef LANGUAGECONTEXT_H
#define LANGUAGECONTEXT_H

#include <QSharedPointer>

#include "languagestyle.h"

class LanguageContext
{
public:
    enum ElementType {
        Simple
      , Container
      , SubPattern
      , Keyword
    } type;

    LanguageContext(ElementType t);
    virtual ~LanguageContext();

    QSharedPointer<LanguageStyle> style;
protected:
    void resolveCircularDeps(QList<LanguageContext *> stack);
    template<class ContextType>
    void resolveCircularDeps(ContextType *current, QList<LanguageContext *> stack);
    template<class ContextType>
    void resolveCircularDeps(ContextType *current);
};

#endif // LANGUAGECONTEXT_H
